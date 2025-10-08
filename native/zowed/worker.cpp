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

using std::string;

// Worker implementation
Worker::Worker(int worker_id)
    : id(worker_id), ready(false), should_stop(false)
{
}

Worker::~Worker()
{
  stop();
}

void Worker::start()
{
  worker_thread = std::thread(&Worker::worker_loop, this);
  ready = true;
  LOG_DEBUG("Worker %d started", id);
}

void Worker::stop()
{
  should_stop = true;
  queue_condition.notify_all();
  if (worker_thread.joinable())
  {
    worker_thread.join();
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
  while (!should_stop)
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    queue_condition.wait(lock, [this]
                         { return !request_queue.empty() || should_stop; });

    if (should_stop)
      break;

    if (!request_queue.empty())
    {
      string request = request_queue.front();
      request_queue.pop();
      lock.unlock();

      process_request(request);
    }
  }
}

void Worker::process_request(const string &data)
{
  // Delegate JSON-RPC processing to the RpcServer singleton
  RpcServer &server = RpcServer::get_instance();
  server.process_request(data);
}

// WorkerPool implementation
WorkerPool::WorkerPool(int num_workers) : ready_count(0), is_shutting_down(false), next_worker_index(0)
{
  workers.reserve(num_workers);

  // Create workers
  for (int i = 0; i < num_workers; ++i)
  {
    std::unique_ptr<Worker> worker(new Worker(i));
    workers.push_back(std::move(worker));
  }

  // Initialize workers asynchronously
  for (int i = 0; i < num_workers; ++i)
  {
    std::thread(&WorkerPool::initialize_worker, this, i).detach();
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
  {
    worker->add_request(request);
  }
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
    if (workers[index]->is_ready())
    {
      return workers[index].get();
    }
  }

  return nullptr;
}

void WorkerPool::set_worker_ready(int worker_id)
{
  std::lock_guard<std::mutex> lock(ready_mutex);

  if (worker_id >= 0 && worker_id < static_cast<int>(workers.size()))
  {
    if (workers[worker_id]->is_ready())
    {
      ready_count++;
      ready_condition.notify_one();
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

  for (auto &worker : workers)
  {
    if (worker)
    {
      worker->stop();
    }
  }
  LOG_DEBUG("Worker pool shutdown complete");
}
