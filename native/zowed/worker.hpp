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
#include "extern/picojson.h"

using picojson::value;

// Utility function to serialize JSON (picojson doesn't need special writer)
inline std::string serializeJson(const picojson::value &val, bool prettify = false)
{
  return val.serialize(prettify);
}

// Forward declarations
class Worker;
class WorkerPool;

// RPC request/response structures
struct RpcRequest
{
  std::string jsonrpc = "2.0";
  std::string method;
  picojson::value params;
  int id;

  static RpcRequest fromJson(const picojson::value &j)
  {
    RpcRequest req;
    req.jsonrpc = j.contains("jsonrpc") ? j.get("jsonrpc").get<std::string>() : "2.0";
    req.method = j.contains("method") ? j.get("method").get<std::string>() : "";
    req.params = j.contains("params") ? j.get("params") : picojson::value(picojson::object());
    req.id = j.contains("id") ? static_cast<int>(j.get("id").get<double>()) : 0;
    return req;
  }
};

struct RpcResponse
{
  std::string jsonrpc = "2.0";
  picojson::value result;
  picojson::value error;
  int id;

  picojson::value toJson() const
  {
    picojson::object obj;
    obj["jsonrpc"] = picojson::value(jsonrpc);
    obj["id"] = picojson::value(static_cast<double>(id));
    if (!result.is<picojson::null>())
    {
      obj["result"] = result;
    }
    if (!error.is<picojson::null>())
    {
      obj["error"] = error;
    }
    return picojson::value(obj);
  }
};

struct ErrorDetails
{
  int code;
  std::string message;
  picojson::value data;

  picojson::value toJson() const
  {
    picojson::object obj;
    obj["code"] = picojson::value(static_cast<double>(code));
    obj["message"] = picojson::value(message);
    if (!data.is<picojson::null>())
    {
      obj["data"] = data;
    }
    return picojson::value(obj);
  }
};

// Command handler function type
using CommandHandler = std::function<picojson::value(const picojson::value &params)>;

// Command dispatcher for managing command handlers
class CommandDispatcher
{
private:
  std::unordered_map<std::string, CommandHandler> handlers;
  std::mutex handlersMutex;

public:
  bool registerHandler(const std::string &command, CommandHandler handler);
  CommandHandler getHandler(const std::string &command);
  void initializeCoreHandlers();
};

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
  std::shared_ptr<CommandDispatcher> dispatcher;
  std::mutex *responseMutex; // Shared response mutex

  void workerLoop();
  void processRequest(const std::string &data);
  void printErrorResponse(const ErrorDetails &error, int requestId);
  void printCommandResponse(const picojson::value &result, int requestId);

public:
  Worker(int workerId, std::shared_ptr<CommandDispatcher> disp, std::mutex *respMutex);
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
  std::shared_ptr<CommandDispatcher> dispatcher;
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
