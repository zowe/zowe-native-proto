/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#include "worker.hpp"
#include "server.hpp"
#include "logger.hpp"
#include <thread>
#include <chrono>

using std::string;

// Worker implementation
Worker::Worker(int worker_id)
    : id(worker_id),
      stop_requested(false),
      state(WorkerState::Starting)
{
}

Worker::~Worker()
{
  stop();
}

void Worker::start()
{
  stop_requested.store(false, std::memory_order_release);
  state.store(WorkerState::Starting, std::memory_order_release);
  worker_thread = std::thread(&Worker::worker_loop, this);
  state.store(WorkerState::Idle, std::memory_order_release);
  LOG_DEBUG("Worker %d started", id);
}

void Worker::stop()
{
  WorkerState current_state = state.load(std::memory_order_acquire);
  if (current_state == WorkerState::Exited)
    return;

  stop_requested.store(true, std::memory_order_release);

  if (current_state != WorkerState::Faulted)
    state.store(WorkerState::Stopping, std::memory_order_release);

  queue_condition.notify_all();
  if (worker_thread.joinable())
    worker_thread.join();

  if (state.load(std::memory_order_acquire) != WorkerState::Faulted)
    state.store(WorkerState::Exited, std::memory_order_release);

  LOG_DEBUG("Worker %d stopped", id);
}

void Worker::add_request(const string &request)
{
  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    request_queue.push(request);
  }
  queue_condition.notify_one();
}

void Worker::worker_loop()
{
  try
  {
    while (true)
    {
      string request;

      {
        std::unique_lock<std::mutex> lock(queue_mutex);
        state.store(WorkerState::Idle, std::memory_order_release);
        queue_condition.wait(lock, [this]
                             { return stop_requested.load(std::memory_order_acquire) || !request_queue.empty(); });

        if (stop_requested.load(std::memory_order_acquire))
        {
          state.store(WorkerState::Stopping, std::memory_order_release);
          break;
        }

        if (request_queue.empty())
          continue;

        request = request_queue.front();
        request_queue.pop();
        state.store(WorkerState::Running, std::memory_order_release);
      }

      process_request(request);
    }

    state.store(WorkerState::Exited, std::memory_order_release);
  }
  catch (const std::exception &e)
  {
    state.store(WorkerState::Faulted, std::memory_order_release);
    stop_requested.store(true, std::memory_order_release);
    LOG_ERROR("Worker %d encountered fatal error: %s", id, e.what());
  }
  catch (...)
  {
    state.store(WorkerState::Faulted, std::memory_order_release);
    stop_requested.store(true, std::memory_order_release);
    LOG_ERROR("Worker %d encountered unknown fatal error", id);
  }
}

bool Worker::is_ready() const
{
  WorkerState current = state.load(std::memory_order_acquire);
  return current == WorkerState::Idle || current == WorkerState::Running;
}

bool Worker::is_running() const
{
  return state.load(std::memory_order_acquire) == WorkerState::Running;
}

bool Worker::has_fault() const
{
  return state.load(std::memory_order_acquire) == WorkerState::Faulted;
}

bool Worker::is_stop_requested() const
{
  WorkerState current = state.load(std::memory_order_acquire);
  if (current == WorkerState::Stopping || current == WorkerState::Exited || current == WorkerState::Faulted)
    return true;

  return stop_requested.load(std::memory_order_acquire);
}

WorkerState Worker::get_state() const
{
  return state.load(std::memory_order_acquire);
}

void Worker::process_request(const string &data)
{
  // Delegate JSON-RPC processing to the RpcServer singleton
  RpcServer &server = RpcServer::get_instance();
  server.process_request(data);
}

// WorkerPool implementation
WorkerPool::WorkerPool(int num_workers)
    : ready_count(0), is_shutting_down(false), next_worker_index(0), supervisor_running(false)
{
  workers.reserve(num_workers);

  // Create workers
  for (int i = 0; i < num_workers; ++i)
  {
    std::unique_ptr<Worker> worker(new Worker(i));
    workers.push_back(std::move(worker));
  }
  worker_ready_flags.resize(num_workers, false);

  // Initialize workers asynchronously
  for (int i = 0; i < num_workers; ++i)
    std::thread(&WorkerPool::initialize_worker, this, i).detach();

  if (num_workers > 0)
  {
    supervisor_running = true;
    supervisor_thread = std::thread(&WorkerPool::monitor_workers, this);
  }
}

WorkerPool::~WorkerPool()
{
  shutdown();
}

void WorkerPool::initialize_worker(int worker_id)
{
  if (worker_id < 0 || worker_id >= static_cast<int>(workers.size()))
  {
    LOG_ERROR("Invalid worker ID: %d", worker_id);
    return;
  }

  LOG_DEBUG("Initializing worker %d", worker_id);

  // Start the worker
  workers[worker_id]->start();

  // Mark worker as ready
  set_worker_ready(worker_id);
}

void WorkerPool::distribute_request(const string &request)
{
  if (is_shutting_down)
    return;

  // Simple round-robin distribution to ready workers
  Worker *worker = get_ready_worker();
  if (worker)
    worker->add_request(request);
}

Worker *WorkerPool::get_ready_worker()
{
  std::unique_lock<std::mutex> lock(ready_mutex);
  ready_condition.wait(lock, [this]
                       { return ready_count > 0 || is_shutting_down; });

  if (is_shutting_down)
    return nullptr;

  // Round-robin selection for O(1) access
  size_t workers_size = workers.size();
  if (workers_size == 0)
    return nullptr;

  size_t start_index = next_worker_index.fetch_add(1) % workers_size;

  // Try to find a ready worker starting from the next index
  for (size_t i = 0; i < workers_size; ++i)
  {
    size_t index = (start_index + i) % workers_size;
    if (workers[index] && workers[index]->is_ready())
      return workers[index].get();
  }

  return nullptr;
}

void WorkerPool::set_worker_ready(int worker_id)
{
  std::lock_guard<std::mutex> lock(ready_mutex);

  if (worker_id >= 0 && worker_id < static_cast<int>(workers.size()))
  {
    if (workers[worker_id] && workers[worker_id]->is_ready())
    {
      // TODO: set worker as ready
    }
  }
}

int32_t WorkerPool::get_available_workers_count()
{
  return ready_count.load();
}

void WorkerPool::shutdown()
{
  LOG_DEBUG("Shutting down worker pool");
  is_shutting_down = true;
  ready_condition.notify_all();
  supervisor_running = false;

  if (supervisor_thread.joinable())
    supervisor_thread.join();

  for (auto &worker : workers)
  {
    if (worker)
      worker->stop();
  }
  LOG_DEBUG("Worker pool shutdown complete");
}

void WorkerPool::monitor_workers()
{
  while (supervisor_running && !is_shutting_down)
  {
    for (size_t i = 0; i < workers.size(); ++i)
    {
      if (!supervisor_running || is_shutting_down)
      {
        break;
      }

      Worker *worker = nullptr;
      {
        std::lock_guard<std::mutex> lock(ready_mutex);
        if (i < workers.size())
        {
          worker = workers[i].get();
        }
      }

      if (worker == nullptr)
      {
        continue;
      }

      const auto worker_state = worker->get_state();
      if (worker_state == WorkerState::Faulted)
      {
        replace_worker(i, "fault");
        continue;
      }

      if (worker_state == WorkerState::Exited && !worker->is_stop_requested())
        replace_worker(i, "unexpected exit");
    }

    for (int i = 0; i < 5 && supervisor_running && !is_shutting_down; i++)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void WorkerPool::replace_worker(size_t worker_index, const char *reason)
{
  LOG_WARN("Worker %zu recovered after %s, respawning", worker_index, reason);
  // TODO: replace actual worker
}
