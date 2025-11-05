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

#include <atomic>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <signal.h>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include "../c/zjson.hpp"
#include "../c/zusf.hpp"
#include "commands.hpp"
#include "dispatcher.hpp"
#include "logger.hpp"
#include "server.hpp"
#include "worker.hpp"
#include "zowed.hpp"

using std::string;

struct StatusMessage
{
  string status;
  string message;
  zstd::optional<zjson::Value> data;
};
ZJSON_DERIVE(StatusMessage, status, message, data);

class ZowedServer
{
private:
  IoserverOptions options;
  string exec_dir;
  std::unique_ptr<WorkerPool> worker_pool;
  std::atomic<bool> shutdown_requested;
  std::mutex shutdown_mutex;
  std::once_flag shutdown_flag;

  static void signal_handler(int sig __attribute__((unused)))
  {
    get_instance().request_shutdown();
  }

  void setup_signal_handlers()
  {
    // Set up signal handling for graceful shutdown
    signal(SIGHUP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGTERM, signal_handler);
  }

  void request_shutdown()
  {
    std::call_once(shutdown_flag, [this]()
                   {
            shutdown_requested = true;
            if (worker_pool) {
                worker_pool->shutdown();
            }
            close(STDIN_FILENO); });
  }

  static ZowedServer &get_instance()
  {
    static ZowedServer instance;
    return instance;
  }

  const std::unordered_map<string, string> load_checksums()
  {
    std::unordered_map<string, string> checksums;
    ZUSF zusf = {.encoding_opts = {.data_type = eDataTypeText, .source_codepage = "IBM-1047"}};
    string checksums_file = exec_dir + "/checksums.asc";
    string checksums_content;

    int rc = zusf_read_from_uss_file(&zusf, checksums_file, checksums_content);
    if (rc != 0)
    {
      // Checksums file does not exist for dev builds
      LOG_DEBUG("Failed to read checksums file: %s (expected for dev builds)", checksums_file.c_str());
      return checksums;
    }

    std::istringstream infile(checksums_content);
    string line;
    while (std::getline(infile, line))
    {
      std::istringstream iss(line);
      string checksum, filename;
      if (iss >> checksum >> filename)
      {
        checksums[filename] = checksum;
      }
    }

    return checksums;
  }

  void print_ready_message()
  {
    zjson::Value data = zjson::Value::create_object();
    const auto checksums = load_checksums();
    zjson::Value checksums_obj = zjson::Value::create_object();
    for (const auto &pair : checksums)
    {
      checksums_obj.add_to_object(pair.first, zjson::Value(pair.second));
    }
    data.add_to_object("checksums", checksums.empty() ? zjson::Value() : checksums_obj);

    StatusMessage status_msg{
        .status = "ready",
        .message = "zowed is ready to accept input",
        .data = zstd::optional<zjson::Value>(data),
    };

    string json_string = RpcServer::serialize_json(zjson::to_value(status_msg).value());
    std::cout << json_string << std::endl;
  }

  void log_worker_count()
  {
    if (!zowed::Logger::is_verbose_logging())
      return;

    std::thread([this]()
                {
            while (!shutdown_requested) {
                if (worker_pool) {
                    int32_t count = worker_pool->get_available_workers_count();
                    LOG_DEBUG("Available workers: %d/%d", count, options.num_workers);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    if (count == options.num_workers) {
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } })
        .detach();
  }

public:
  ZowedServer() : shutdown_requested(false)
  {
  }

  void run(const IoserverOptions &opts, const string &exec_dir_param)
  {
    options = opts;
    this->exec_dir = exec_dir_param;

    // Initialize logger with executable directory
    zowed::Logger::init_logger(exec_dir.c_str(), options.verbose);
    LOG_INFO("Starting zowed with %d workers (verbose=%s)", options.num_workers, options.verbose ? "true" : "false");

    // Set up signal handling
    setup_signal_handlers();

    // Initialize CommandDispatcher singleton
    CommandDispatcher &dispatcher = CommandDispatcher::get_instance();

    // Register all command handlers
    LOG_DEBUG("Registering command handlers");
    register_all_commands(dispatcher);

    // Create worker pool
    worker_pool.reset(new WorkerPool(options.num_workers));

    // Ensure worker pool teardown on normal exit
    std::atexit([]()
                { get_instance().request_shutdown(); });

    // Log available worker count if verbose
    log_worker_count();

    // Print ready message
    print_ready_message();

    // Main input processing loop
    LOG_DEBUG("Entering main input processing loop");
    string line;
    while (std::getline(std::cin, line) && !shutdown_requested)
    {
      if (!line.empty())
      {
        // Distribute request to worker pool
        worker_pool->distribute_request(line);
      }
    }

    // Graceful shutdown
    LOG_INFO("Input stream closed, shutting down");
    request_shutdown();

    // Cleanup logger
    zowed::Logger::shutdown();
  }
};

// Library implementation functions
extern "C" int run_zowed_server(const IoserverOptions &options, const char *exec_dir)
{
  try
  {
    ZowedServer server;
    server.run(options, string(exec_dir ? exec_dir : "."));
  }
  catch (const std::exception &e)
  {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    LOG_FATAL("Fatal error: %s", e.what());
    return 1;
  }

  return 0;
}
