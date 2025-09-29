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
#include "../c/types/common.h"
#include "worker.hpp"
#include "zowed.hpp"
#include "dispatcher.hpp"
#include "../c/commands/ds.hpp"
#include "../c/zbase64.h"

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
    data.add_to_object("checksums", checksumsObj);

    StatusMessage statusMsg{
        .status = "ready",
        .message = "zowed is ready to accept input",
        .data = checksums.empty() ? zstd::optional<zjson::Value>() : zstd::optional<zjson::Value>(checksumsObj),
    };

    std::string jsonString = serializeJson(zjson::to_value(statusMsg).value());
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

    // Initialize RpcDispatcher singleton
    RpcDispatcher &dispatcher = RpcDispatcher::getInstance();

    // Register core command handlers
    dispatcher.register_command("listDatasets", ds::handle_data_set_list);
    dispatcher.register_command("listDsMembers", ds::handle_data_set_list_members);
    dispatcher.register_command("readDataset", ds::handle_data_set_view);

    // Register writeDataset with input handler for "data" parameter
    dispatcher.register_command("writeDataset", ds::handle_data_set_write,
                                [](MiddlewareContext &context)
                                {
                                  // Check if "data" argument exists
                                  const plugin::Argument *dataArg = context.find("data");
                                  if (dataArg && dataArg->is_string())
                                  {
                                    try
                                    {
                                      // Get the base64 encoded data
                                      std::string base64Data = dataArg->get_string_value();

                                      // Base64 decode the data and write to input stream
                                      std::string decodedData = zbase64::decode(base64Data);
                                      context.set_input_content(decodedData);

                                      // Remove "data" from arguments since it's now in input stream
                                      plugin::ArgumentMap &args = const_cast<plugin::ArgumentMap &>(context.arguments());
                                      args.erase("data");
                                    }
                                    catch (const std::exception &e)
                                    {
                                      // If base64 decode fails, write error to error stream
                                      context.errln("Failed to decode base64 data");
                                    }
                                  }
                                });

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
