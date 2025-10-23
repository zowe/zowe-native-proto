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

#ifndef WORKER_HPP
#define WORKER_HPP

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <cstdint>

// Forward declarations
class Worker;
class WorkerPool;

enum class WorkerState
{
  Starting,
  Idle,
  Running,
  Stopping,
  Faulted,
  Exited
};

// Request metadata for tracking retry attempts
struct RequestMetadata
{
  std::string data;       // The actual request payload
  size_t retry_count;     // Number of times this request has been attempted
  std::string request_id; // Optional: for logging/debugging

  RequestMetadata() : retry_count(0)
  {
  }
  RequestMetadata(const std::string &req_data, size_t retries = 0, const std::string &id = "")
      : data(req_data), retry_count(retries), request_id(id)
  {
  }
};

// Worker class that processes command requests
class Worker
{
private:
  int id;
  std::thread worker_thread;
  std::queue<RequestMetadata> request_queue;
  std::mutex queue_mutex;
  std::condition_variable queue_condition;
  std::atomic<bool> stop_requested;
  std::atomic<WorkerState> state;
  std::atomic<int64_t> last_heartbeat_ms;

  // Track the currently processing request for recovery
  std::string current_request_data;
  std::mutex current_request_mutex;

  void worker_loop();
  void process_request(const std::string &data);
  void update_heartbeat();

public:
  Worker(int worker_id);
  ~Worker();

  void start();
  void stop();
  void add_request(const RequestMetadata &request);
  bool is_ready() const;
  int get_id() const
  {
    return id;
  }
  bool is_running() const;
  bool has_fault() const;
  bool is_stop_requested() const;
  WorkerState get_state() const;
  std::chrono::steady_clock::time_point get_last_heartbeat() const;
  void force_detach();

  // Request recovery methods
  std::vector<RequestMetadata> drain_pending_requests();
  std::string get_current_request();
};

// Worker pool that manages multiple workers
class WorkerPool
{
private:
  static constexpr size_t kMaxRequestRetries = 2; // Maximum retry attempts for poison pill protection

  std::atomic<size_t> next_worker_index;
  std::vector<std::unique_ptr<Worker>> workers;

  // Track ready state variables and total number of ready workers
  std::vector<bool> ready_list;
  std::mutex ready_mutex;
  std::condition_variable ready_condition;
  std::atomic<int32_t> ready_count;
  std::vector<size_t> replacement_attempts;
  std::vector<std::chrono::steady_clock::time_point> next_replacement_allowed;
  std::chrono::milliseconds request_timeout;

  // Whether the pool is shutting down
  std::atomic<bool> is_shutting_down;

  // State variables for supervisor/monitor thread
  std::thread supervisor_thread;
  std::atomic<bool> supervisor_running;

  void initialize_worker(int worker_id);
  void monitor_workers();
  /**
   * @brief Replace a worker at the given index with the reason for replacement
   *
   * @param worker_index The ID of the worker to replace
   * @param reason The reason why the worker is being replaced
   */
  void replace_worker(size_t worker_index, const char *reason, bool force_detach = false);
  /**
   * @brief Re-distribute recovered requests from a failed worker
   *
   * @param requests Vector of requests to re-distribute
   * @param worker_index The index of the worker that failed
   * @param reason The reason for re-distribution
   */
  void redistribute_requests(std::vector<RequestMetadata> &requests, size_t worker_index, const char *reason);
  void distribute_request_internal(const RequestMetadata &request);

public:
  explicit WorkerPool(int num_workers, std::chrono::milliseconds request_timeout = std::chrono::seconds(60));
  ~WorkerPool();

  void distribute_request(const std::string &request);
  int32_t get_available_workers_count();
  void shutdown();
  /**
   * @brief Get the next available worker from the pool
   *
   * @return `Worker*` A pointer to the available worker
   */
  Worker *get_ready_worker();
  /**
   * @brief Marks the given worker ID as ready in the pool
   *
   * @param worker_id The ID of the worker to mark as ready
   */
  void set_worker_ready(int worker_id);
};

#endif
