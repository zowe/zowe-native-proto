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

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <signal.h>
#include <unistd.h>
#include <memory>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <map>
#include "../c/zjson.hpp"
#include "worker.hpp"
#include "zowed.hpp"
#include "dispatcher.hpp"
#include "commands.hpp"
#include "server.hpp"
#include "logger.hpp"

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
  string execDir;
  std::unique_ptr<WorkerPool> workerPool;
  std::atomic<bool> shutdownRequested;
  std::mutex shutdownMutex;
  std::once_flag shutdownFlag;

  static void signalHandler(int sig __attribute__((unused)))
  {
    getInstance().requestShutdown();
  }

  void setupSignalHandlers()
  {
    // Set up signal handling for graceful shutdown
    signal(SIGHUP, signalHandler);
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGTERM, signalHandler);
  }

  void requestShutdown()
  {
    std::call_once(shutdownFlag, [this]()
                   {
            shutdownRequested = true;
            if (workerPool) {
                workerPool->shutdown();
            }
            close(STDIN_FILENO); });
  }

  static ZowedServer &getInstance()
  {
    static ZowedServer instance;
    return instance;
  }

  std::map<string, string> loadChecksums()
  {
    std::map<string, string> checksums;
    string checksumsFile = execDir + "/checksums.asc";

    std::ifstream file(checksumsFile);
    if (!file.is_open())
    {
      // Checksums file does not exist for dev builds
      LOG_DEBUG("Checksums file not found: %s (expected for dev builds)", checksumsFile.c_str());
      return checksums;
    }

    string line;
    while (std::getline(file, line))
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

  void printReadyMessage()
  {
    zjson::Value data = zjson::Value::create_object();

    // Load checksums similar to Go implementation
    std::map<string, string> checksums = loadChecksums();
    zjson::Value checksumsObj = zjson::Value::create_object();
    for (const auto &pair : checksums)
    {
      checksumsObj.add_to_object(pair.first, zjson::Value(pair.second));
    }
    data.add_to_object("checksums", checksums.empty() ? zjson::Value() : checksumsObj);

    StatusMessage statusMsg{
        .status = "ready",
        .message = "zowed is ready to accept input",
        .data = zstd::optional<zjson::Value>(data),
    };

    string jsonString = RpcServer::serializeJson(zjson::to_value(statusMsg).value());
    std::cout << jsonString << std::endl;
  }

  void logWorkerCount()
  {
    if (!zowed::Logger::is_verbose_logging())
      return;

    std::thread([this]()
                {
            while (!shutdownRequested) {
                if (workerPool) {
                    int32_t count = workerPool->getAvailableWorkersCount();
                    LOG_DEBUG("Available workers: %d/%d", count, options.numWorkers);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    if (count == options.numWorkers) {
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } })
        .detach();
  }

public:
  ZowedServer() : shutdownRequested(false)
  {
  }

  void run(const IoserverOptions &opts, const string &execDir)
  {
    options = opts;
    this->execDir = execDir;

    // Initialize logger with executable directory
    zowed::Logger::init_logger(execDir.c_str(), options.verbose);
    LOG_INFO("Starting zowed with %d workers (verbose=%s)", options.numWorkers, options.verbose ? "true" : "false");

    // Set up signal handling
    setupSignalHandlers();

    // Initialize CommandDispatcher singleton
    CommandDispatcher &dispatcher = CommandDispatcher::get_instance();

    // Register all command handlers
    LOG_DEBUG("Registering command handlers");
    register_all_commands(dispatcher);

    // Create worker pool
    workerPool.reset(new WorkerPool(options.numWorkers));

    // Ensure worker pool teardown on normal exit
    std::atexit([]()
                { getInstance().requestShutdown(); });

    // Log available worker count if verbose
    logWorkerCount();

    // Print ready message
    printReadyMessage();

    // Main input processing loop
    LOG_DEBUG("Entering main input processing loop");
    string line;
    while (std::getline(std::cin, line) && !shutdownRequested)
    {
      if (!line.empty())
      {
        // Distribute request to worker pool
        workerPool->distributeRequest(line);
      }
    }

    // Graceful shutdown
    LOG_INFO("Input stream closed, shutting down");
    requestShutdown();

    // Cleanup logger
    zowed::Logger::shutdown();
  }
};

// Library implementation functions
extern "C" int run_zowed_server(const IoserverOptions &options, const char *execDir)
{
  try
  {
    ZowedServer server;
    server.run(options, string(execDir ? execDir : "."));
  }
  catch (const std::exception &e)
  {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    LOG_FATAL("Fatal error: %s", e.what());
    return 1;
  }

  return 0;
}
