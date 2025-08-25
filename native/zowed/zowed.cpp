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
#include "extern/picojson.h"
#include "worker.hpp"

struct IoserverOptions
{
  int numWorkers = 10;
  bool verbose = false;
};

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

  void printReadyMessage()
  {
    picojson::object data;
    // TODO: Load checksums similar to Go implementation
    data["checksums"] = picojson::value(picojson::object());

    picojson::object readyMsg;
    readyMsg["status"] = picojson::value(std::string("ready"));
    readyMsg["message"] = picojson::value(std::string("zowed is ready to accept input"));
    readyMsg["data"] = picojson::value(data);

    std::string jsonString = serializeJson(picojson::value(readyMsg));
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

  IoserverOptions parseOptions(int argc, char *argv[])
  {
    IoserverOptions opts;

    // Simple argument parsing without getopt_long
    for (int i = 1; i < argc; i++)
    {
      std::string arg = argv[i];

      if (arg == "-w" || arg == "--num-workers" || arg == "-num-workers")
      {
        // TODO Should we support single hyphen with long option names?
        if (i + 1 < argc)
        {
          opts.numWorkers = std::stoi(argv[++i]);
          if (opts.numWorkers <= 0)
          {
            std::cerr << "Number of workers must be greater than 0" << std::endl;
            exit(1);
          }
        }
        else
        {
          std::cerr << "Option " << arg << " requires an argument" << std::endl;
          exit(1);
        }
      }
      else if (arg == "-v" || arg == "--verbose")
      {
        opts.verbose = true;
      }
      else if (arg == "-h" || arg == "--help")
      {
        std::cout << "Usage: " << argv[0] << " [OPTIONS]\n"
                  << "  -w, --num-workers NUM  Number of worker threads (default: 10)\n"
                  << "  -v, --verbose          Enable verbose logging\n"
                  << "  -h, --help             Show this help message\n";
        exit(0);
      }
      else
      {
        std::cerr << "Unknown option: " << arg << std::endl;
        exit(1);
      }
    }

    return opts;
  }

  void run(int argc, char *argv[])
  {
    options = parseOptions(argc, argv);

    // Initialize logger (placeholder for now)
    if (options.verbose)
    {
      std::cerr << "Verbose logging enabled" << std::endl;
    }

    // Set up signal handling
    setupSignalHandlers();

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

int main(int argc, char *argv[])
{
  try
  {
    ZowedServer server;
    server.run(argc, argv);
  }
  catch (const std::exception &e)
  {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
