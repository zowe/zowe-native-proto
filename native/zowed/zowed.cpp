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

struct StatusMessage
{
  std::string status;
  std::string message;
  zstd::optional<zjson::Value> data;
};
ZJSON_DERIVE(StatusMessage, status, message, data);

class ZowedServer
{
private:
  IoserverOptions options;
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
            exit(0); });
  }

  static ZowedServer &getInstance()
  {
    static ZowedServer instance;
    return instance;
  }

  std::string getExecutableDir()
  {
    // z/OS implementation: use current working directory
    char *cwd = getcwd(nullptr, 0);
    std::string result = cwd ? std::string(cwd) : ".";
    free(cwd);
    return result;
  }

  std::map<std::string, std::string> loadChecksums()
  {
    std::map<std::string, std::string> checksums;
    std::string execDir = getExecutableDir();
    std::string checksumsFile = execDir + "/checksums.asc";

    std::ifstream file(checksumsFile);
    if (!file.is_open())
    {
      // Checksums file does not exist for dev builds
      return checksums;
    }

    std::string line;
    while (std::getline(file, line))
    {
      std::istringstream iss(line);
      std::string checksum, filename;
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
    std::map<std::string, std::string> checksums = loadChecksums();
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

    std::string jsonString = RpcServer::serializeJson(zjson::to_value(statusMsg).value());
    std::cout << jsonString << std::endl;
  }

  void logWorkerCount()
  {
    if (!options.verbose)
      return;

    std::thread([this]()
                {
            while (!shutdownRequested) {
                if (workerPool) {
                    int32_t count = workerPool->getAvailableWorkersCount();
                    std::cerr << "Available workers: " << count << "/" << options.numWorkers << std::endl;
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

  void run(const IoserverOptions &opts)
  {
    options = opts;

    // Initialize logger (placeholder for now)
    if (options.verbose)
    {
      std::cerr << "Verbose logging enabled" << std::endl;
    }

    // Set up signal handling
    setupSignalHandlers();

    // Initialize CommandDispatcher singleton
    CommandDispatcher &dispatcher = CommandDispatcher::getInstance();

    // Register all command handlers
    register_ds_commands(dispatcher);
    register_job_commands(dispatcher);
    register_uss_commands(dispatcher);
    register_cmd_commands(dispatcher);

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
    std::string line;
    while (std::getline(std::cin, line) && !shutdownRequested)
    {
      if (!line.empty())
      {
        // Distribute request to worker pool
        workerPool->distributeRequest(line);
      }
    }

    // Graceful shutdown
    requestShutdown();
  }
};

// Library implementation functions
extern "C" int run_zowed_server(const IoserverOptions &options)
{
  try
  {
    ZowedServer server;
    server.run(options);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
