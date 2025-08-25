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
  registerHandler("createDataset", [](const picojson::value &params)
                  {
// Placeholder implementation
return picojson::value(picojson::array()); });
  registerHandler("deleteDataset", [](const picojson::value &params)
                  {
// Placeholder implementation
return picojson::value(picojson::array()); });
  registerHandler("createMember", [](const picojson::value &params)
                  {
// Placeholder implementation
return picojson::value(picojson::array()); });
  registerHandler("restoreDataset", [](const picojson::value &params)
                  {
// Placeholder implementation
return picojson::value(picojson::array()); });

  // USS handlers
  registerHandler("listUssFiles", [](const picojson::value &params)
                  {
        // Placeholder implementation
        return picojson::value(picojson::array()); });

  registerHandler("readUssFile", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["content"] = picojson::value(std::string(""));
        return picojson::value(result); });

  registerHandler("writeUssFile", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  registerHandler("createUssFile", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  registerHandler("deleteUssFile", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  registerHandler("chmodUss", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  registerHandler("chownUss", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  registerHandler("chtagUss", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  // Job handlers
  registerHandler("listJobs", [](const picojson::value &params)
                  {
        // Placeholder implementation
        return picojson::value(picojson::array()); });

  registerHandler("getJcl", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["jcl"] = picojson::value(std::string(""));
        return picojson::value(result); });

  registerHandler("getJobStatus", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["status"] = picojson::value(std::string("UNKNOWN"));
        return picojson::value(result); });

  registerHandler("listSpools", [](const picojson::value &params)
                  {
        // Placeholder implementation
        return picojson::value(picojson::array()); });

  registerHandler("readSpool", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["content"] = picojson::value(std::string(""));
        return picojson::value(result); });

  registerHandler("submitJob", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["jobid"] = picojson::value(std::string("JOB00000"));
        return picojson::value(result); });

  registerHandler("submitJcl", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["jobid"] = picojson::value(std::string("JOB00000"));
        return picojson::value(result); });

  registerHandler("submitUss", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["jobid"] = picojson::value(std::string("JOB00000"));
        return picojson::value(result); });

  registerHandler("cancelJob", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  registerHandler("deleteJob", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  registerHandler("holdJob", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  registerHandler("releaseJob", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["success"] = picojson::value(true);
        return picojson::value(result); });

  // Console handler
  registerHandler("consoleCommand", [](const picojson::value &params)
                  {
        // Placeholder implementation
        picojson::object result;
        result["response"] = picojson::value(std::string(""));
        return picojson::value(result); });
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
    picojson::value requestJson;
    std::string errors = picojson::parse(requestJson, data);

    if (!errors.empty())
    {
      ErrorDetails error{
          -32700,
          "Failed to parse command request: " + errors,
          picojson::value(picojson::object())};
      printErrorResponse(error, 0); // No request ID available
      return;
    }

    RpcRequest request = RpcRequest::fromJson(requestJson);

    // Get the command handler
    CommandHandler handler = dispatcher->getHandler(request.method);
    if (!handler)
    {
      ErrorDetails error{
          -32601,
          "Unrecognized command " + request.method,
          picojson::value(picojson::object())};
      printErrorResponse(error, request.id);
      return;
    }

    // Execute the command
    try
    {
      picojson::value result = handler(request.params);
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
          errData.empty() ? picojson::value(picojson::object()) : picojson::value(errData)};
      printErrorResponse(error, request.id);
    }
  }
  catch (const std::exception &e)
  {
    ErrorDetails error{
        -32700,
        "Failed to parse command request: " + std::string(e.what()),
        picojson::value(picojson::object())};
    printErrorResponse(error, 0); // No request ID available
  }
}

void Worker::printErrorResponse(const ErrorDetails &error, int requestId)
{
  std::lock_guard<std::mutex> lock(*responseMutex);

  RpcResponse response;
  response.error = error.toJson();
  response.id = requestId;

  std::string jsonString = serializeJson(response.toJson());
  std::cout << jsonString << std::endl;
}

void Worker::printCommandResponse(const picojson::value &result, int requestId)
{
  std::lock_guard<std::mutex> lock(*responseMutex);

  RpcResponse response;
  response.result = result;
  response.id = requestId;

  std::string jsonString = serializeJson(response.toJson());
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
