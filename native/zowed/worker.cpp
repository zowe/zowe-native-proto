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

namespace
{

const char *worker_state_to_string(WorkerState state)
{
  switch (state)
  {
  case WorkerState::Starting:
    return "Starting";
  case WorkerState::Idle:
    return "Idle";
  case WorkerState::Running:
    return "Running";
  case WorkerState::Stopping:
    return "Stopping";
  case WorkerState::Faulted:
    return "Faulted";
  case WorkerState::Exited:
    return "Exited";
  default:
    return "Unknown";
  }
}

} // namespace

// Worker implementation
Worker::Worker(int worker_id)
    : id(worker_id),
      stop_requested(false),
      state(WorkerState::Starting)
{
  LOG_DEBUG("Worker %d state -> %s (constructor)", id, worker_state_to_string(WorkerState::Starting));
}

Worker::~Worker()
{
  stop();
}

void Worker::start()
{
  stop_requested.store(false, std::memory_order_release);
  state.store(WorkerState::Starting, std::memory_order_release);
  LOG_DEBUG("Worker %d state -> %s (start invoked)", id, worker_state_to_string(WorkerState::Starting));
  worker_thread = std::thread(&Worker::worker_loop, this);
  state.store(WorkerState::Idle, std::memory_order_release);
  LOG_DEBUG("Worker %d state -> %s (worker thread started)", id, worker_state_to_string(WorkerState::Idle));
  LOG_DEBUG("Worker %d started", id);
}

void Worker::stop()
{
  WorkerState current_state = state.load(std::memory_order_acquire);
  if (current_state == WorkerState::Exited)
    return;

  stop_requested.store(true, std::memory_order_release);

  if (current_state != WorkerState::Faulted)
  {
    state.store(WorkerState::Stopping, std::memory_order_release);
    LOG_DEBUG("Worker %d state -> %s (stop requested)", id, worker_state_to_string(WorkerState::Stopping));
  }

  queue_condition.notify_all();
  if (worker_thread.joinable())
    worker_thread.join();

  if (state.load(std::memory_order_acquire) != WorkerState::Faulted)
  {
    state.store(WorkerState::Exited, std::memory_order_release);
    LOG_DEBUG("Worker %d state -> %s (stop complete)", id, worker_state_to_string(WorkerState::Exited));
  }

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
          LOG_DEBUG("Worker %d state -> %s (stop signaled)", id, worker_state_to_string(WorkerState::Stopping));
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
    LOG_DEBUG("Worker %d state -> %s (worker loop exit)", id, worker_state_to_string(WorkerState::Exited));
  }
  catch (const std::exception &e)
  {
    state.store(WorkerState::Faulted, std::memory_order_release);
    LOG_DEBUG("Worker %d state -> %s (exception)", id, worker_state_to_string(WorkerState::Faulted));
    stop_requested.store(true, std::memory_order_release);
    LOG_ERROR("Worker %d encountered fatal error: %s", id, e.what());
  }
  catch (...)
  {
    state.store(WorkerState::Faulted, std::memory_order_release);
    LOG_DEBUG("Worker %d state -> %s (unknown exception)", id, worker_state_to_string(WorkerState::Faulted));
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
  ready_list.resize(num_workers, false);

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
  if (worker_id < 0)
  {
    LOG_ERROR("Attempted to mark invalid worker %d as ready (negative index)", worker_id);
    return;
  }

  bool notify_ready = false;

  {
    std::lock_guard<std::mutex> lock(ready_mutex);

    // Bounds checking for worker ID to make sure the pool hasn't shifted the index out-of-bounds
    if (worker_id >= static_cast<int>(workers.size()))
    {
      LOG_ERROR("Attempted to mark worker %d as ready but index is out of range", worker_id);
      return;
    }

    // Make sure a worker still exists at that index in the pool before using it
    Worker *worker = workers[worker_id].get();
    if (!worker)
    {
      LOG_WARN("Attempted to mark worker %d as ready but worker slot is empty", worker_id);
      return;
    }

    if (worker_id >= static_cast<int>(ready_list.size()))
      ready_list.resize(workers.size(), false);

    if (!ready_list[worker_id] && worker->is_ready())
    {
      // Mark worker as ready if we haven't marked it in the list
      ready_list[worker_id] = true;
      ready_count.fetch_add(1);
      notify_ready = true;
    }
  }

  if (notify_ready)
  {
    // Notify `get_ready_worker` if its waiting for a worker to become available
    LOG_DEBUG("Worker %d marked as ready. Ready workers: %d", worker_id, ready_count.load());
    ready_condition.notify_one();
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
  // Scan over the workers periodically to check their heartbeat (worker state)
  while (supervisor_running && !is_shutting_down)
  {
    for (size_t i = 0; i < workers.size(); ++i)
    {
      if (!supervisor_running || is_shutting_down)
        break;

      Worker *worker = nullptr;
      {
        std::lock_guard<std::mutex> lock(ready_mutex);
        if (i < workers.size())
        {
          worker = workers[i].get();
        }
      }

      if (worker == nullptr)
        continue;

      // If this worker has faulted, replace it
      const auto worker_state = worker->get_state();
      if (worker_state == WorkerState::Faulted)
      {
        replace_worker(i, "fault");
        continue;
      }

      // If the worker has already crashed/exited, replace it
      if (worker_state == WorkerState::Exited && !worker->is_stop_requested())
        replace_worker(i, "unexpected exit");
    }

    // Sleep loop to avoid busy waiting on CPU
    for (int i = 0; i < 5 && supervisor_running && !is_shutting_down; i++)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void WorkerPool::replace_worker(size_t worker_index, const char *reason)
{
  LOG_WARN("Replacing worker %zu due to %s", worker_index, reason);
  // Don't replace any workers if we're just shutting down the thread pool
  if (is_shutting_down.load())
    return;

  std::unique_ptr<Worker> old_worker;

  {
    std::lock_guard<std::mutex> lock(ready_mutex);

    if (worker_index >= workers.size())
    {
      // Ensure that the worker index is still valid in case multiple workers have failed
      LOG_ERROR("Cannot replace worker at index %zu - out of range", worker_index);
      return;
    }

    if (worker_index >= ready_list.size())
      ready_list.resize(workers.size(), false);

    if (ready_list[worker_index])
    {
      int32_t previous_ready = ready_count.fetch_sub(1);
      if (previous_ready <= 0)
      {
        ready_count.fetch_add(1);
        LOG_WARN("Ready count underflow while replacing worker %zu", worker_index);
      }

      ready_list[worker_index] = false;
    }

    // Prepare to shut down the old worker
    old_worker = std::move(workers[worker_index]);
  }

  if (old_worker)
    old_worker->stop();

  // Don't replace any workers if we're just shutting down the thread pool
  // Checking again in case the worker pool was shut down while a worker was being replaced
  if (is_shutting_down.load())
    return;

  std::unique_ptr<Worker> new_worker(new Worker(static_cast<int>(worker_index)));

  {
    // Prepare the new worker
    std::lock_guard<std::mutex> lock(ready_mutex);

    if (worker_index >= workers.size())
      workers.resize(worker_index + 1);

    if (worker_index >= ready_list.size())
      ready_list.resize(worker_index + 1, false);

    workers[worker_index] = std::move(new_worker);
    ready_list[worker_index] = false;
  }

  std::thread(&WorkerPool::initialize_worker, this, static_cast<int>(worker_index)).detach();
  LOG_DEBUG("Replacement worker %zu spawned, awaiting readiness", worker_index);
}
