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
#include <map>
#include <mutex>
#include <csignal>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include "../zjson.hpp"
#include "../zusf.hpp"
#include "rpc_commands.hpp"
#include "dispatcher.hpp"
#include "logger.hpp"
#include "server.hpp"
#include "worker.hpp"
#include "server_main.hpp"

using std::string;

struct StatusMessage
{
  string status;
  string message;
  std::optional<zjson::Value> data;
};
ZJSON_DERIVE(StatusMessage, status, message, data);

class ZServer
{
private:
  server::Options options;
  std::unique_ptr<WorkerPool> worker_pool;
  std::atomic<bool> shutdown_requested{false};
  std::mutex shutdown_mutex;
  std::once_flag shutdown_flag;

  static void signal_handler(int sig __attribute__((unused)))
  {
    get_instance().request_shutdown();
  }

  void setup_signal_handlers()
  {
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

  static ZServer &get_instance()
  {
    static ZServer instance;
    return instance;
  }

  std::map<string, string> load_checksums()
  {
    std::map<string, string> checksums;
    ZUSF zusf = {.encoding_opts = {.source_codepage = "IBM-1047", .data_type = eDataTypeText}};
    string checksums_file = options.exec_dir + "/checksums.asc";
    string checksums_content;

    int rc = zusf_read_from_uss_file(&zusf, checksums_file, checksums_content);
    if (rc != 0)
    {
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
        .message = "zowex server is ready to accept input",
        .data = std::optional<zjson::Value>(data),
    };

    string json_string = RpcServer::serialize_json(zjson::to_value(status_msg).value());
    std::cout << json_string << std::endl;
  }

  void log_worker_count()
  {
    if (!server::Logger::is_verbose_logging())
      return;

    std::thread([this]()
                {
            while (!shutdown_requested) {
                if (worker_pool) {
                    int32_t count = worker_pool->get_available_workers_count();
                    LOG_DEBUG("Available workers: %d/%lld", count, options.num_workers);
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
  ZServer() = default;
  void run(const server::Options &opts)
  {
    options = opts;

    server::Logger::init_logger(options.exec_dir.c_str(), options.verbose);
    LOG_INFO("Starting zowex server with %lld workers and %lld seconds until request timeout (verbose=%s)", options.num_workers, options.request_timeout, options.verbose ? "true" : "false");

    setup_signal_handlers();

    CommandDispatcher &dispatcher = CommandDispatcher::get_instance();

    LOG_DEBUG("Registering command handlers");
    register_all_commands(dispatcher);

    worker_pool.reset(new WorkerPool(options.num_workers, std::chrono::seconds(options.request_timeout)));

    std::atexit([]()
                { get_instance().request_shutdown(); });

    log_worker_count();
    print_ready_message();

    LOG_DEBUG("Entering main input processing loop");
    string line;
    while (std::getline(std::cin, line) && !shutdown_requested)
    {
      if (!line.empty())
      {
        worker_pool->distribute_request(line);
      }
    }

    LOG_INFO("Input stream closed, shutting down");
    request_shutdown();

    server::Logger::shutdown();
  }
};

int server::run(const server::Options &options)
{
  try
  {
    ZServer srv;
    srv.run(options);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    LOG_FATAL("Fatal error: %s", e.what());
    return 1;
  }

  return 0;
}
