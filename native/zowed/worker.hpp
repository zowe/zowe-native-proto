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

// RPC request/response structures
struct RpcRequest
{
  std::string jsonrpc = "2.0";
  std::string method;
  zjson::Value params;
  int id;

  static RpcRequest fromJson(const zjson::Value &j)
  {
    RpcRequest req;
    if (!j.is_object())
    {
      throw std::runtime_error("JSON-RPC request must be an object");
    }
    const auto &obj = j.as_object();

    auto jsonrpc_it = obj.find("jsonrpc");
    req.jsonrpc = (jsonrpc_it != obj.end() && jsonrpc_it->second.is_string()) ? jsonrpc_it->second.as_string() : "2.0";

    auto method_it = obj.find("method");
    req.method = (method_it != obj.end() && method_it->second.is_string()) ? method_it->second.as_string() : "";

    auto params_it = obj.find("params");
    req.params = (params_it != obj.end()) ? params_it->second : zjson::Value::create_object();

    auto id_it = obj.find("id");
    req.id = (id_it != obj.end() && id_it->second.is_number()) ? id_it->second.as_int() : 0;

    return req;
  }
};

struct RpcResponse
{
  std::string jsonrpc = "2.0";
  zjson::Value result;
  zjson::Value error;
  int id;

  zjson::Value toJson() const
  {
    zjson::Value obj = zjson::Value::create_object();
    obj.add_to_object("jsonrpc", zjson::Value(jsonrpc));
    obj.add_to_object("id", zjson::Value(id));
    if (!result.is_null())
    {
      obj.add_to_object("result", result);
    }
    if (!error.is_null())
    {
      obj.add_to_object("error", error);
    }
    return obj;
  }
};

struct ErrorDetails
{
  int code;
  std::string message;
  zjson::Value data;

  zjson::Value toJson() const
  {
    zjson::Value obj = zjson::Value::create_object();
    obj.add_to_object("code", zjson::Value(code));
    obj.add_to_object("message", zjson::Value(message));
    if (!data.is_null())
    {
      obj.add_to_object("data", data);
    }
    return obj;
  }
};

// Command handler function type
using CommandHandler = std::function<zjson::Value(const zjson::Value &params)>;

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
  void printCommandResponse(const zjson::Value &result, int requestId);

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
