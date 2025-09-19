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

#include "worker.hpp"
#include "cmds/ds.hpp"
#include <iostream>

// CommandDispatcher implementation
bool CommandDispatcher::registerHandler(const std::string &command, CommandHandler handler)
{
  std::lock_guard<std::mutex> lock(handlersMutex);
  auto result = handlers.emplace(command, handler);
  return result.second; // true if insertion took place
}

CommandHandler CommandDispatcher::getHandler(const std::string &command)
{
  std::lock_guard<std::mutex> lock(handlersMutex);
  auto it = handlers.find(command);
  if (it != handlers.end())
  {
    return it->second;
  }
  return nullptr;
}

void CommandDispatcher::initializeCoreHandlers()
{
  // Register placeholder handlers for core commands
  // These would be implemented to call the actual zowex processes

  // Dataset handlers
  registerHandler("listDatasets", HandleListDatasetsRequest);
  registerHandler("readDataset", HandleReadDatasetRequest);
  registerHandler("writeDataset", HandleWriteDatasetRequest);
  registerHandler("listDsMembers", HandleListDsMembersRequest);
  registerHandler("createDataset", [](const zjson::Value &params)
                  {
// Placeholder implementation
return zjson::Value::create_array(); });
  registerHandler("deleteDataset", [](const zjson::Value &params)
                  {
// Placeholder implementation
return zjson::Value::create_array(); });
  registerHandler("createMember", [](const zjson::Value &params)
                  {
// Placeholder implementation
return zjson::Value::create_array(); });
  registerHandler("restoreDataset", [](const zjson::Value &params)
                  {
// Placeholder implementation
return zjson::Value::create_array(); });

  // USS handlers
  registerHandler("listUssFiles", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        return zjson::Value::create_array(); });

  registerHandler("readUssFile", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("content", zjson::Value(std::string("")));
        return result; });

  registerHandler("writeUssFile", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  registerHandler("createUssFile", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  registerHandler("deleteUssFile", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  registerHandler("chmodUss", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  registerHandler("chownUss", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  registerHandler("chtagUss", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  // Job handlers
  registerHandler("listJobs", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        return zjson::Value::create_array(); });

  registerHandler("getJcl", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("jcl", zjson::Value(std::string("")));
        return result; });

  registerHandler("getJobStatus", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("status", zjson::Value(std::string("UNKNOWN")));
        return result; });

  registerHandler("listSpools", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        return zjson::Value::create_array(); });

  registerHandler("readSpool", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("content", zjson::Value(std::string("")));
        return result; });

  registerHandler("submitJob", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("jobid", zjson::Value(std::string("JOB00000")));
        return result; });

  registerHandler("submitJcl", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("jobid", zjson::Value(std::string("JOB00000")));
        return result; });

  registerHandler("submitUss", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("jobid", zjson::Value(std::string("JOB00000")));
        return result; });

  registerHandler("cancelJob", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  registerHandler("deleteJob", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  registerHandler("holdJob", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  registerHandler("releaseJob", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("success", zjson::Value(true));
        return result; });

  // Console handler
  registerHandler("consoleCommand", [](const zjson::Value &params)
                  {
        // Placeholder implementation
        zjson::Value result = zjson::Value::create_object();
        result.add_to_object("response", zjson::Value(std::string("")));
        return result; });
}

// Worker implementation
Worker::Worker(int workerId, std::shared_ptr<CommandDispatcher> disp, std::mutex *respMutex)
    : id(workerId), ready(false), shouldStop(false), dispatcher(disp), responseMutex(respMutex)
{
}

Worker::~Worker()
{
  stop();
}

void Worker::start()
{
  workerThread = std::thread(&Worker::workerLoop, this);
  ready = true;
}

void Worker::stop()
{
  shouldStop = true;
  queueCondition.notify_all();
  if (workerThread.joinable())
  {
    workerThread.join();
  }
}

void Worker::addRequest(const std::string &request)
{
  {
    std::lock_guard<std::mutex> lock(queueMutex);
    requestQueue.push(request);
  }
  queueCondition.notify_one();
}

void Worker::workerLoop()
{
  while (!shouldStop)
  {
    std::unique_lock<std::mutex> lock(queueMutex);
    queueCondition.wait(lock, [this]
                        { return !requestQueue.empty() || shouldStop; });

    if (shouldStop)
      break;

    if (!requestQueue.empty())
    {
      std::string request = requestQueue.front();
      requestQueue.pop();
      lock.unlock();

      processRequest(request);
    }
  }
}

void Worker::processRequest(const std::string &data)
{
  try
  {
    // Parse the JSON request
    auto parse_result = zjson::from_str<zjson::Value>(data);

    if (!parse_result.has_value())
    {
      ErrorDetails error{
          -32700,
          std::string("Failed to parse command request: ") + parse_result.error().what(),
          zstd::optional<zjson::Value>()};
      printErrorResponse(error, 0); // No request ID available
      return;
    }

    zjson::Value requestJson = parse_result.value();

    RpcRequest request = parseRpcRequest(requestJson);

    // Get the command handler
    CommandHandler handler = dispatcher->getHandler(request.method);
    if (!handler)
    {
      ErrorDetails error{
          -32601,
          "Unrecognized command " + request.method,
          zstd::optional<zjson::Value>()};
      printErrorResponse(error, request.id);
      return;
    }

    // Execute the command
    try
    {
      zjson::Value params = request.params.has_value() ? request.params.value() : zjson::Value::create_object();
      zjson::Value result = handler(params);
      printCommandResponse(result, request.id);
    }
    catch (const std::exception &e)
    {
      std::string errMsg = e.what();
      std::string errData;

      // Parse error message similar to Go implementation
      if (errMsg.substr(0, 7) == "Error: ")
      {
        errMsg = errMsg.substr(7);
      }

      size_t colonPos = errMsg.find(": ");
      if (colonPos != std::string::npos)
      {
        errData = errMsg.substr(colonPos + 2);
        errMsg = errMsg.substr(0, colonPos);
      }

      ErrorDetails error{
          -32603, // Internal error
          errMsg,
          errData.empty() ? zstd::optional<zjson::Value>() : zstd::optional<zjson::Value>(zjson::Value(errData))};
      printErrorResponse(error, request.id);
    }
  }
  catch (const std::exception &e)
  {
    ErrorDetails error{
        -32700,
        "Failed to parse command request: " + std::string(e.what()),
        zstd::optional<zjson::Value>()};
    printErrorResponse(error, 0); // No request ID available
  }
}

void Worker::printErrorResponse(const ErrorDetails &error, int requestId)
{
  std::lock_guard<std::mutex> lock(*responseMutex);

  RpcResponse response;
  response.jsonrpc = "2.0";
  response.result = zstd::optional<zjson::Value>();
  response.error = zstd::optional<ErrorDetails>(error);
  response.id = requestId;

  std::string jsonString = serializeJson(rpcResponseToJson(response));
  std::cout << jsonString << std::endl;
}

void Worker::printCommandResponse(const zjson::Value &result, int requestId)
{
  std::lock_guard<std::mutex> lock(*responseMutex);

  RpcResponse response;
  response.jsonrpc = "2.0";
  response.result = zstd::optional<zjson::Value>(result);
  response.error = zstd::optional<ErrorDetails>();
  response.id = requestId;

  std::string jsonString = serializeJson(rpcResponseToJson(response));
  std::cout << jsonString << std::endl;
}

// WorkerPool implementation
WorkerPool::WorkerPool(int numWorkers) : readyCount(0), isShuttingDown(false)
{
  dispatcher = std::make_shared<CommandDispatcher>();
  dispatcher->initializeCoreHandlers();

  workers.reserve(numWorkers);

  // Create workers
  for (int i = 0; i < numWorkers; ++i)
  {
    std::unique_ptr<Worker> worker(new Worker(i, dispatcher, &responseMutex));
    workers.push_back(std::move(worker));
  }

  // Initialize workers asynchronously
  for (int i = 0; i < numWorkers; ++i)
  {
    std::thread(&WorkerPool::initializeWorker, this, i).detach();
  }
}

WorkerPool::~WorkerPool()
{
  shutdown();
}

void WorkerPool::initializeWorker(int workerId)
{
  if (workerId < 0 || workerId >= static_cast<int>(workers.size()))
  {
    return;
  }

  // Start the worker
  workers[workerId]->start();

  // Mark worker as ready
  setWorkerReady(workerId);
}

void WorkerPool::distributeRequest(const std::string &request)
{
  if (isShuttingDown)
    return;

  // Simple round-robin distribution to ready workers
  Worker *worker = getReadyWorker();
  if (worker)
  {
    worker->addRequest(request);
  }
}

Worker *WorkerPool::getReadyWorker()
{
  std::unique_lock<std::mutex> lock(readyMutex);
  readyCondition.wait(lock, [this]
                      { return readyCount > 0 || isShuttingDown; });

  if (isShuttingDown)
    return nullptr;

  // Find a ready worker
  for (auto &worker : workers)
  {
    if (worker->isReady())
    {
      return worker.get();
    }
  }

  return nullptr;
}

void WorkerPool::setWorkerReady(int workerId)
{
  std::lock_guard<std::mutex> lock(readyMutex);

  if (workerId >= 0 && workerId < static_cast<int>(workers.size()))
  {
    if (workers[workerId]->isReady())
    {
      readyCount++;
      readyCondition.notify_one();
    }
  }
}

int32_t WorkerPool::getAvailableWorkersCount()
{
  return readyCount.load();
}

void WorkerPool::shutdown()
{
  isShuttingDown = true;
  readyCondition.notify_all();

  for (auto &worker : workers)
  {
    worker->stop();
  }
}
