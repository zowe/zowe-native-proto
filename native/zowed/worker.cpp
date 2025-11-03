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

constexpr size_t kMaxReplacementAttempts = 5;
constexpr std::chrono::milliseconds kBaseReplacementBackoff(200);
constexpr std::chrono::milliseconds kMaxReplacementBackoff(5000);

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
    : id(worker_id)
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
  update_heartbeat();
  LOG_DEBUG("Worker %d state -> %s (start invoked)", id, worker_state_to_string(WorkerState::Starting));

  auto self = shared_from_this();

  worker_thread = std::thread(&Worker::worker_loop, self);
  state.store(WorkerState::Idle, std::memory_order_release);
  update_heartbeat();
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

  if (state.load(std::memory_order_acquire) != WorkerState::Faulted)
    LOG_DEBUG("Worker %d stopped", id);
  else
    LOG_DEBUG("Worker %d stop requested while faulted/detached", id);
}

void Worker::add_request(const RequestMetadata &request)
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
      RequestMetadata request_metadata;

      {
        std::unique_lock<std::mutex> lock(queue_mutex);
        state.store(WorkerState::Idle, std::memory_order_release);
        update_heartbeat();
        queue_condition.wait(lock, [this]
                             { return stop_requested.load(std::memory_order_acquire) || !request_queue.empty(); });

        if (stop_requested.load(std::memory_order_acquire))
        {
          state.store(WorkerState::Stopping, std::memory_order_release);
          update_heartbeat();
          LOG_DEBUG("Worker %d state -> %s (stop signaled)", id, worker_state_to_string(WorkerState::Stopping));
          break;
        }

        if (request_queue.empty())
        {
          update_heartbeat();
          continue;
        }

        request_metadata = request_queue.front();
        request_queue.pop();
        state.store(WorkerState::Running, std::memory_order_release);
        update_heartbeat();
      }

      // Track current request for potential recovery
      {
        std::lock_guard<std::mutex> lock(current_request_mutex);
        current_request_data = request_metadata.data;
      }

      process_request(request_metadata.data);
      update_heartbeat();

      // Clear current request after successful processing
      {
        std::lock_guard<std::mutex> lock(current_request_mutex);
        current_request_data.clear();
      }
    }

    state.store(WorkerState::Exited, std::memory_order_release);
    update_heartbeat();
    LOG_DEBUG("Worker %d state -> %s (worker loop exit)", id, worker_state_to_string(WorkerState::Exited));
  }
  catch (const std::exception &e)
  {
    state.store(WorkerState::Faulted, std::memory_order_release);
    update_heartbeat();
    LOG_DEBUG("Worker %d state -> %s (exception)", id, worker_state_to_string(WorkerState::Faulted));
    stop_requested.store(true, std::memory_order_release);
    LOG_ERROR("Worker %d encountered fatal error: %s", id, e.what());
  }
  catch (...)
  {
    state.store(WorkerState::Faulted, std::memory_order_release);
    update_heartbeat();
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

void Worker::update_heartbeat()
{
  last_heartbeat_ms.store(steady_clock_now_ms(), std::memory_order_release);
}

std::chrono::steady_clock::time_point Worker::get_last_heartbeat() const
{
  const auto stored_ms = last_heartbeat_ms.load(std::memory_order_acquire);
  const auto duration_ms = std::chrono::milliseconds(stored_ms);
  const auto steady_duration = std::chrono::duration_cast<std::chrono::steady_clock::duration>(duration_ms);
  return std::chrono::steady_clock::time_point(steady_duration);
}

void Worker::force_detach()
{
  stop_requested.store(true, std::memory_order_release);
  state.store(WorkerState::Faulted, std::memory_order_release);
  queue_condition.notify_all();
  LOG_WARN(worker_thread.joinable() ? "Worker %d forcibly detached due to heartbeat timeout" : "Worker %d marked faulted due to heartbeat timeout (thread not joinable)", id);
  update_heartbeat();
}

std::vector<RequestMetadata> Worker::drain_pending_requests()
{
  std::vector<RequestMetadata> drained_requests;

  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    while (!request_queue.empty())
    {
      drained_requests.push_back(request_queue.front());
      request_queue.pop();
    }
  }

  if (!drained_requests.empty())
  {
    LOG_DEBUG("Worker %d: Drained %zu pending requests from queue", id, drained_requests.size());
  }

  return drained_requests;
}

std::string Worker::get_current_request()
{
  std::lock_guard<std::mutex> lock(current_request_mutex);
  return current_request_data;
}

// WorkerPool implementation
WorkerPool::WorkerPool(int num_workers, std::chrono::milliseconds request_timeout_param)
    : request_timeout(request_timeout_param <= std::chrono::milliseconds(0) ? std::chrono::seconds(60) : request_timeout_param)
{
  workers.reserve(num_workers);

  // Create workers
  for (int i = 0; i < num_workers; ++i)
  {
    auto worker = std::make_shared<Worker>(i);
    workers.push_back(std::move(worker));
  }
  ready_list.resize(num_workers, false);
  replacement_attempts.resize(num_workers, 0);
  next_replacement_allowed.resize(num_workers, std::chrono::steady_clock::time_point::min());

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
  // Wrap the request in metadata with retry_count = 0
  RequestMetadata metadata(request, 0);
  distribute_request_internal(metadata);
}

void WorkerPool::distribute_request_internal(const RequestMetadata &request)
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
    const auto worker_index = static_cast<size_t>(worker_id);

    // Make sure a worker still exists at that index in the pool before using it
    Worker *worker = workers[worker_index].get();
    if (!worker)
    {
      LOG_WARN("Attempted to mark worker %d as ready but worker slot is empty", worker_id);
      return;
    }

    if (worker_index >= ready_list.size())
      ready_list.resize(workers.size(), false);

    if (worker_index >= replacement_attempts.size())
    {
      replacement_attempts.resize(workers.size(), 0);
      next_replacement_allowed.resize(workers.size(), std::chrono::steady_clock::time_point::min());
    }

    if (!ready_list[worker_index] && worker->is_ready())
    {
      // Mark worker as ready if we haven't marked it in the list
      ready_list[worker_index] = true;
      ready_count.fetch_add(1);
      replacement_attempts[worker_index] = 0;
      next_replacement_allowed[worker_index] = std::chrono::steady_clock::time_point::min();
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
        if (i < next_replacement_allowed.size() &&
            std::chrono::steady_clock::now() < next_replacement_allowed[i])
        {
          // Worker is already being replaced, skip monitoring
          continue;
        }

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
      {
        replace_worker(i, "unexpected exit");
      }
      else if (worker_state == WorkerState::Running && request_timeout.count() > 0)
      {
        const auto last_heartbeat = worker->get_last_heartbeat();
        const auto now = std::chrono::steady_clock::now();
        if (last_heartbeat != std::chrono::steady_clock::time_point::min() && now > last_heartbeat)
        {
          const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_heartbeat);
          if (elapsed > request_timeout)
          {
            LOG_WARN("Worker %zu exceeded request timeout (%lld ms > %lld ms); scheduling replacement", i, static_cast<long long>(elapsed.count()), static_cast<long long>(request_timeout.count()));
            replace_worker(i, "heartbeat timeout", true);
            continue;
          }
        }
      }
    }

    // Sleep loop to avoid busy waiting on CPU
    for (int i = 0; i < 5 && supervisor_running && !is_shutting_down; i++)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void WorkerPool::replace_worker(size_t worker_index, const char *reason, bool force_detach)
{
  if (is_shutting_down.load())
    return;

  // Mark worker as not ready
  {
    std::lock_guard<std::mutex> lock(ready_mutex);

    if (worker_index >= workers.size())
    {
      LOG_ERROR("Cannot replace worker at index %zu - out of range", worker_index);
      return;
    }

    // Ensure vectors are sized appropriately
    if (worker_index >= ready_list.size())
      ready_list.resize(workers.size(), false);
    if (worker_index >= replacement_attempts.size())
    {
      replacement_attempts.resize(workers.size(), 0);
      next_replacement_allowed.resize(workers.size(), std::chrono::steady_clock::time_point::min());
    }

    // Decrement ready count if it was ready
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
  }

  std::shared_ptr<Worker> old_worker;
  bool max_attempts_reached = false;
  size_t attempt_number = 0;
  std::chrono::milliseconds applied_backoff(0);

  // Check retries, set next backoff, and get old worker
  {
    std::lock_guard<std::mutex> lock(ready_mutex);

    if (worker_index >= workers.size())
    {
      LOG_ERROR("Cannot replace worker at index %zu - out of range after lock", worker_index);
      return;
    }

    const size_t attempts = replacement_attempts[worker_index];
    const auto now = std::chrono::steady_clock::now();

    if (attempts >= kMaxReplacementAttempts)
    {
      max_attempts_reached = true;
    }
    else
    {
      // Not max attempts, so schedule the *next* backoff time
      attempt_number = attempts + 1;
      replacement_attempts[worker_index] = attempt_number;

      auto computed_backoff = kBaseReplacementBackoff * (1ULL << (attempt_number - 1));
      if (computed_backoff > kMaxReplacementBackoff)
        computed_backoff = kMaxReplacementBackoff;

      applied_backoff = computed_backoff;
      next_replacement_allowed[worker_index] = now + computed_backoff;
    }

    // Move worker out to be processed outside the lock
    old_worker = std::move(workers[worker_index]);
  }

  // Drain and stop the old worker
  std::vector<RequestMetadata> recovered_requests;
  if (old_worker)
  {
    // Get pending requests from queue
    auto pending = old_worker->drain_pending_requests();
    recovered_requests.insert(recovered_requests.end(), pending.begin(), pending.end());

    // Recover in-flight request only if it wasn't a timeout/hang
    if (!force_detach)
    {
      std::string current_req = old_worker->get_current_request();
      if (!current_req.empty())
      {
        LOG_DEBUG("Worker %zu: Recovering in-flight request due to %s", worker_index, reason);
        recovered_requests.emplace_back(RequestMetadata(current_req, 0));
      }
    }
    else
    {
      std::string current_req = old_worker->get_current_request();
      if (!current_req.empty())
      {
        LOG_DEBUG("Worker %zu: NOT recovering in-flight request due to timeout/hang - request may still complete", worker_index);
      }
    }

    // Stop/detach the old worker
    if (force_detach)
      old_worker->force_detach();
    old_worker->stop();
  }

  // Don't create new worker if we exceeded max attempts
  if (max_attempts_reached)
  {
    LOG_ERROR("Worker %zu hit maximum replacement attempts (%zu) after %s; leaving worker offline", worker_index, kMaxReplacementAttempts, reason);
    // Still try to redistribute any recovered requests
    if (!recovered_requests.empty())
      redistribute_requests(recovered_requests, worker_index, reason);
    return; // EXIT: Do not replace.
  }

  if (attempt_number > 0)
  {
    LOG_WARN("Replacing worker %zu due to %s (attempt %zu, next backoff %lld ms)", worker_index, reason, attempt_number, static_cast<long long>(applied_backoff.count()));
  }

  if (is_shutting_down.load())
    return;

  // Create and spawn the new worker
  auto new_worker = std::make_shared<Worker>(static_cast<int>(worker_index));

  {
    std::lock_guard<std::mutex> lock(ready_mutex);

    // Re-check sizes in case of resize
    if (worker_index >= workers.size())
      workers.resize(worker_index + 1);
    if (worker_index >= ready_list.size())
      ready_list.resize(worker_index + 1, false);
    if (worker_index >= replacement_attempts.size())
    {
      replacement_attempts.resize(worker_index + 1, 0);
      next_replacement_allowed.resize(worker_index + 1, std::chrono::steady_clock::time_point::min());
    }

    workers[worker_index] = std::move(new_worker);
    ready_list[worker_index] = false;
  }

  std::thread(&WorkerPool::initialize_worker, this, static_cast<int>(worker_index)).detach();
  LOG_DEBUG("Replacement worker %zu spawned, awaiting readiness", worker_index);

  // Redistribute recovered requests from old worker
  if (!recovered_requests.empty())
    redistribute_requests(recovered_requests, worker_index, reason);
}

void WorkerPool::redistribute_requests(std::vector<RequestMetadata> &requests, size_t worker_index, const char *reason)
{
  if (is_shutting_down.load())
    return;

  size_t redistributed_count = 0;
  size_t failed_count = 0;

  for (auto &req : requests)
  {
    // Check if request has exceeded max retry limit (poison pill protection)
    if (req.retry_count >= kMaxRequestRetries)
    {
      LOG_ERROR("Request from worker %zu discarded after %zu retry attempts (poison pill protection). Reason: %s",
                worker_index, req.retry_count, reason);
      failed_count++;
      continue;
    }

    // Increment retry count and re-distribute
    req.retry_count++;
    LOG_DEBUG("Re-routing request from worker %zu (attempt %zu/%zu). Reason: %s",
              worker_index, req.retry_count, kMaxRequestRetries, reason);

    distribute_request_internal(req);
    redistributed_count++;
  }

  if (redistributed_count > 0)
  {
    LOG_INFO("Redistributed %zu requests from failed worker %zu. Failed: %zu (poison pills)",
             redistributed_count, worker_index, failed_count);
  }
}
