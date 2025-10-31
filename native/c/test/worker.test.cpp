#include "worker.test.hpp"
#include "ztest.hpp"
#include "../../zowed/worker.hpp"

#include <string>
#include <stdexcept>
#include <functional>
#include <atomic>
#include <mutex>
#include <cstdarg>
#include <cstdio>
#include <thread>
#include <chrono>

using namespace ztst;

/**
 * @brief Stub for logger.hpp
 *
 * Provides minimal logging macros (LOG_DEBUG, LOG_INFO, etc.) that
 * print to stdout. This is necessary for worker.cpp to compile.
 */
#ifndef LOGGER_HPP
#define LOGGER_HPP

// A global mutex to prevent interleaved log messages from different threads
static std::mutex g_log_mutex;

/**
 * @brief Simple thread-safe logger function for tests.
 */
inline void test_logger(const char *level, const char *format, ...)
{
  std::lock_guard<std::mutex> lock(g_log_mutex);
  std::va_list args;
  va_start(args, format);
  std::printf("[%s] ", level);
  std::vprintf(format, args);
  std::printf("\n");
  fflush(stdout); // Ensure logs are visible immediately
  va_end(args);
}

// Map the logging macros used in worker.cpp
#define LOG_DEBUG(format, ...) test_logger("DEBUG", format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) test_logger("INFO", format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) test_logger("WARN", format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) test_logger("ERROR", format, ##__VA_ARGS__)
#endif // LOGGER_HPP

/**
 * @brief Stub for server.hpp
 *
 * Provides a mock RpcServer singleton that worker.cpp depends on.
 * This mock is controllable to simulate successful work, faults (exceptions),
 * and hangs (timeouts).
 */
#ifndef SERVER_HPP
#define SERVER_HPP

class RpcServer
{
public:
  // --- Test control variables ---

  // If true, requests with data "hang" will loop until this is set to false
  std::atomic<bool> hang_request{false};

  // Counts how many requests have been fully processed
  std::atomic<int> processed_count{0};

  // Stores the last request data received
  std::string last_processed_request;
  std::mutex mtx; // Protects last_processed_request

  /**
   * @brief Get the singleton instance.
   */
  static RpcServer &get_instance()
  {
    static RpcServer instance;
    return instance;
  }

  /**
   * @brief The mock request processing logic.
   *
   * @param data The request payload.
   */
  void process_request(const std::string &data)
  {
    {
      std::lock_guard<std::mutex> lock(mtx);
      last_processed_request = data;
    }

    // --- Test Scenarios ---

    // 1. Simulate a hang (for timeout tests)
    if (data == "hang")
    {
      hang_request.store(true);
      TestLog("RpcServer: Simulating hang. Request 'hang' received.");
      while (hang_request.load())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
      TestLog("RpcServer: Hang released.");
      processed_count++; // Count it once it's "finished"
      return;
    }

    // 2. Simulate a fault (for exception tests)
    if (data == "fault")
    {
      TestLog("RpcServer: Simulating fault. Request 'fault' received.");
      processed_count++; // Count it as "processed" before throwing
      throw std::runtime_error("Simulated worker fault");
    }

    // 3. Default behavior (simulating normal work)
    // Sleep for a short duration to make state transitions observable
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    processed_count++;
  }

  /**
   * @brief Resets the mock server's state.
   * Called before tests.
   */
  void reset()
  {
    hang_request.store(false);
    processed_count.store(0);
    std::lock_guard<std::mutex> lock(mtx);
    last_processed_request.clear();
  }

private:
  // Private constructor/destructor for singleton
  RpcServer() = default;
  ~RpcServer() = default;
  RpcServer(const RpcServer &) = delete;
  RpcServer &operator=(const RpcServer &) = delete;
};
#endif // SERVER_HPP

using namespace std::chrono_literals;

/**
 * @brief Helper function to wait for an asynchronous condition with a timeout.
 *
 * @param condition A function returning true when the condition is met.
 * @param timeout The maximum time to wait.
 * @return true if the condition was met, false if it timed out.
 */
bool wait_for(std::function<bool()> condition, std::chrono::milliseconds timeout)
{
  auto start = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start < timeout)
  {
    if (condition())
    {
      return true;
    }
    std::this_thread::sleep_for(10ms); // Poll interval
  }
  return false;
}

/**
 * @brief Main function containing all test definitions for Worker and WorkerPool.
 */
void worker_tests()
{
  // Global test state shared across tests in this file
  std::shared_ptr<WorkerPool> pool;
  RpcServer &server = RpcServer::get_instance();

  // Reset the mock RpcServer before each test
  beforeEach([&]()
             {
    TestLog("Resetting RpcServer mock state...");
    server.reset(); });

  // Ensure the pool is shut down after each test to clean up threads
  afterEach([&]()
            {
    TestLog("Shutting down worker pool...");
    if (pool)
    {
      pool->shutdown();
      pool = nullptr;
    } });

  // --- Test Suite for the Worker class ---
  describe("Worker (Unit Tests)", [&]()
           {

    std::shared_ptr<Worker> worker;

    // Clean up individual worker after each 'it' block
    afterEach([&]() {
      if (worker)
      {
        worker->stop();
        worker = nullptr;
      }
    });

    it("should start and transition to Idle state", [&]() {
      worker = std::make_shared<Worker>(0);
      // Initial state is Starting (set in constructor)
      Expect(worker->get_state() == WorkerState::Starting).ToBe(true);

      worker->start();

      // After start(), the loop runs and waits, setting state to Idle
      bool became_idle = wait_for([&]() { return worker->get_state() == WorkerState::Idle; }, 500ms);
      Expect(became_idle).ToBe(true);
    });

    it("should process a request and return to Idle", [&]() {
      worker = std::make_shared<Worker>(0);
      worker->start();
      Expect(worker->get_state() == WorkerState::Idle).ToBe(true);

      server.reset();
      worker->add_request(RequestMetadata("test_request_1"));

      // Wait for the worker to pick up the request and go to Running
      bool became_running = wait_for([&]() {
        return worker->get_state() == WorkerState::Running;
      }, 500ms);
      Expect(became_running).ToBe(true);

      // Wait for it to finish processing and go back to Idle
      bool became_idle = wait_for([&]() {
        return worker->get_state() == WorkerState::Idle;
      }, 500ms);
      Expect(became_idle).ToBe(true);

      // Verify the mock server actually processed it
      Expect(server.processed_count.load()).ToBe(1);
    });

    it("should transition to Faulted state on exception", [&]() {
      worker = std::make_shared<Worker>(0);
      worker->start();
      Expect(worker->get_state() == WorkerState::Idle).ToBe(true);

      server.reset();
      // "fault" is a special command for the mock RpcServer to throw
      worker->add_request(RequestMetadata("fault"));

      // Wait for the worker's catch block to set the state
      bool faulted = wait_for([&]() {
        return worker->get_state() == WorkerState::Faulted;
      }, 500ms);

      Expect(faulted).ToBe(true);
      Expect(server.processed_count.load()).ToBe(1);
    });

    it("should stop cleanly and transition to Exited state", [&]() {
      worker = std::make_shared<Worker>(0);
      worker->start();
      Expect(worker->get_state() == WorkerState::Idle).ToBe(true);

      worker->stop();

      // Note: stop() joins the thread, so this is synchronous
      Expect(worker->get_state() == WorkerState::Exited).ToBe(true);
    }); });

  // --- Test Suite for the WorkerPool class ---
  describe("WorkerPool (Integration Tests)", [&]()
           {
             it("should initialize and all workers become ready", [&]()
                {
      int num_workers = 4;
      pool = std::make_shared<WorkerPool>(num_workers, 1000ms);

      // Wait for all workers to start and mark themselves as ready
      bool all_ready = wait_for([&]() {
        return pool->get_available_workers_count() == num_workers;
      }, 2000ms); // Give them time to start up

      Expect(all_ready).ToBe(true);
      Expect(pool->get_available_workers_count()).ToBe(num_workers); });

             it("should distribute multiple requests to workers", [&]()
                {
      int num_workers = 2;
      pool = std::make_shared<WorkerPool>(num_workers, 1000ms);

      // Wait for workers to be ready
      bool all_ready = wait_for([&]() { return pool->get_available_workers_count() == num_workers; }, 1000ms);
      Expect(all_ready).ToBe(true);

      server.reset();
      pool->distribute_request("req1");
      pool->distribute_request("req2");
      pool->distribute_request("req3");

      // Wait for all three requests to be processed
      bool all_processed = wait_for([&]() { return server.processed_count.load() == 3; }, 1000ms);

      Expect(all_processed).ToBe(true);
      Expect(server.processed_count.load()).ToBe(3); });

             it("should replace a faulted worker and redistribute its requests", [&]()
                {
      int num_workers = 1;
      // Use a long timeout so it doesn't interfere with the fault test
      pool = std::make_shared<WorkerPool>(num_workers, 5000ms);

      bool ready = wait_for([&]() { return pool->get_available_workers_count() == num_workers; }, 1000ms);
      Expect(ready).ToBe(true);

      server.reset();
      // Add some requests that will be pending in the queue
      pool->distribute_request("pending1");
      pool->distribute_request("pending2");
      // Add the request that will cause the fault
      pool->distribute_request("fault");

      // Expected sequence:
      // 1. "fault" is processed (count=1), worker 0 faults.
      // 2. Monitor thread detects fault (~500ms loop).
      // 3. `replace_worker` is called.
      // 4. "pending1" and "pending2" are drained from the queue.
      // 5. "fault" (the in-flight request) is *recovered* (since force_detach=false).
      // 6. New worker 0 is created and started.
      // 7. "pending1", "pending2", and "fault" (retry 1) are redistributed.
      // 8. "pending1" is processed (count=2).
      // 9. "pending2" is processed (count=3).
      // 10. "fault" (retry 1) is processed (count=4), new worker 0 faults.
      // 11. Monitor detects fault, replaces worker.
      // 12. "fault" (retry 2) is recovered and redistributed.
      // 13. "fault" (retry 2) is processed (count=5), new worker 0 faults.
      // 14. Monitor detects fault, replaces worker.
      // 15. "fault" (retry 3) is recovered, but `kMaxRequestRetries` (2) is exceeded.
      // 16. "fault" request is DISCARDED (poison pill).

      TestLog("Waiting for fault/recovery/redistribution/poison pill cycle...");
      // We expect 5 total requests to be processed
      bool finished_processing = wait_for([&]() {
        return server.processed_count.load() == 5; // fault, p1, p2, fault(r1), fault(r2)
      }, 5000ms); // Needs time for monitor loops + replacement backoffs

      Expect(finished_processing).ToBe(true);
      Expect(server.processed_count.load()).ToBe(5);

      // Now, send a clean request. It should be processed by a new, healthy worker.
      TestLog("Sending clean request after poison pill...");
      pool->distribute_request("clean");

      bool clean_processed = wait_for([&]() {
        return server.processed_count.load() == 6;
      }, 2000ms); // Replacement backoff might add delay

      Expect(clean_processed).ToBe(true);
      Expect(server.processed_count.load()).ToBe(6); });

             it("should replace a timed-out worker and NOT recover in-flight request", [&]()
                {
      int num_workers = 1;
      // Use a very short timeout for the test
      auto short_timeout = 250ms;
      pool = std::make_shared<WorkerPool>(num_workers, short_timeout);

      bool ready = wait_for([&]() { return pool->get_available_workers_count() == num_workers; }, 1000ms);
      Expect(ready).ToBe(true);

      server.reset();
      // Add a pending request
      pool->distribute_request("pending1");
      // Add the request that will hang
      pool->distribute_request("hang");

      // Expected sequence:
      // 1. "hang" is processed, RpcServer loop starts, worker state is "Running".
      // 2. Monitor thread detects stale heartbeat (~250ms + ~500ms loop).
      // 3. `replace_worker` is called with `force_detach=true`.
      // 4. "pending1" is drained from the queue.
      // 5. "hang" (in-flight) is *NOT* recovered due to `force_detach`.
      // 6. New worker 0 is created and started.
      // 7. "pending1" is redistributed and processed by the new worker (count=1).

      TestLog("Waiting for timeout/replacement cycle...");
      bool p1_processed = wait_for([&]() {
        return server.processed_count.load() == 1; // Only "pending1"
      }, 3000ms); // Needs time for timeout + monitor + backoff

      Expect(p1_processed).ToBe(true);
      Expect(server.processed_count.load()).ToBe(1);

      // Now, manually release the original hanging request
      TestLog("Releasing original hang request...");
      server.hang_request.store(false);

      // Wait a bit. The processed count should increase to 2 as the
      // *original detached* thread finally finishes its `process_request` call.
      // This confirms the original thread was left to run.
      bool hang_finished = wait_for([&]() {
        return server.processed_count.load() == 2;
      }, 500ms);

      Expect(hang_finished).ToBe(true);
      Expect(server.processed_count.load()).ToBe(2);

      // The pool should now have a healthy, ready worker.
      TestLog("Sending clean request after timeout recovery...");
      pool->distribute_request("clean");
      bool clean_processed = wait_for([&]() {
        return server.processed_count.load() == 3;
      }, 1000ms);

      Expect(clean_processed).ToBe(true);
      Expect(server.processed_count.load()).ToBe(3);
      Expect(pool->get_available_workers_count()).ToBe(1); }); });
}
