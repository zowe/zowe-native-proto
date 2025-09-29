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

#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include "../c/zjson.hpp"
#include "../c/types/common.h"
#include "dispatcher.hpp"

using zjson::Value;

// Utility function to serialize JSON
inline std::string serializeJson(const zjson::Value &val, bool prettify = false)
{
  auto result = prettify ? zjson::to_string_pretty(val) : zjson::to_string(val);
  if (!result.has_value())
  {
    throw std::runtime_error(std::string("Failed to serialize JSON: ") + result.error().what());
  }
  return result.value();
}

// Forward declarations
class Worker;
class WorkerPool;

// Helper function for parsing RPC requests from JSON
inline RpcRequest parseRpcRequest(const zjson::Value &j)
{
  if (!j.is_object())
  {
    throw std::runtime_error("JSON-RPC request must be an object");
  }

  auto result = zjson::from_value<RpcRequest>(j);
  if (!result.has_value())
  {
    throw std::runtime_error(std::string("Failed to parse RPC request: ") + result.error().what());
  }
  return result.value();
}

// Helper functions for working with common types
inline zjson::Value rpcResponseToJson(const RpcResponse &response)
{
  return zjson::to_value(response).value_or(zjson::Value::create_object());
}

inline zjson::Value errorDetailsToJson(const ErrorDetails &error)
{
  return zjson::to_value(error).value_or(zjson::Value::create_object());
}

// Command handler function type
using CommandHandler = std::function<zjson::Value(const zjson::Value &params)>;

// Worker class that processes command requests
class Worker
{
private:
  int id;
  std::thread workerThread;
  std::queue<std::string> requestQueue;
  std::mutex queueMutex;
  std::condition_variable queueCondition;
  std::atomic<bool> ready;
  std::atomic<bool> shouldStop;
  std::mutex *responseMutex; // Shared response mutex

  void workerLoop();
  void processRequest(const std::string &data);
  void printErrorResponse(const ErrorDetails &error, int requestId);
  void printCommandResponse(const zjson::Value &result, int requestId);

public:
  Worker(int workerId, std::mutex *respMutex);
  ~Worker();

  void start();
  void stop();
  void addRequest(const std::string &request);
  bool isReady() const
  {
    return ready.load();
  }
  int getId() const
  {
    return id;
  }
};

// Worker pool that manages multiple workers
class WorkerPool
{
private:
  std::vector<std::unique_ptr<Worker>> workers;
  std::mutex responseMutex;
  std::mutex readyMutex;
  std::condition_variable readyCondition;
  std::atomic<int32_t> readyCount;
  std::atomic<bool> isShuttingDown;

  void initializeWorker(int workerId);

public:
  explicit WorkerPool(int numWorkers);
  ~WorkerPool();

  void distributeRequest(const std::string &request);
  int32_t getAvailableWorkersCount();
  void shutdown();
  Worker *getReadyWorker();
  void setWorkerReady(int workerId);
};
