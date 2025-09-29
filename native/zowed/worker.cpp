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
#include <iostream>

// Worker implementation
Worker::Worker(int workerId, std::mutex *respMutex)
    : id(workerId), ready(false), shouldStop(false), responseMutex(respMutex)
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

    // Use RpcDispatcher singleton to handle the command
    RpcDispatcher &dispatcher = RpcDispatcher::getInstance();

    // Check if command is registered
    if (!dispatcher.has_command(request.method))
    {
      ErrorDetails error{
          -32601,
          "Unrecognized command " + request.method,
          zstd::optional<zjson::Value>()};
      printErrorResponse(error, request.id);
      return;
    }

    // Create MiddlewareContext for the command
    plugin::ArgumentMap args;

    // Convert JSON params to ArgumentMap
    if (request.params.has_value())
    {
      zjson::Value params = request.params.value();
      if (!params.is_object())
      {
        ErrorDetails error{
            -32602,
            "Invalid parameters",
            zstd::optional<zjson::Value>()};
      }

      // Convert JSON object to ArgumentMap
      for (const auto &pair : params.as_object())
      {
        const std::string &key = pair.first;
        const zjson::Value &value = pair.second;

        if (value.is_bool())
        {
          args[key] = plugin::Argument(value.as_bool());
        }
        else if (value.is_number())
        {
          args[key] = plugin::Argument(static_cast<long long>(value.as_number()));
        }
        else if (value.is_string())
        {
          args[key] = plugin::Argument(value.as_string());
        }
        else if (value.is_array())
        {
          // Convert array to vector<string>
          std::vector<std::string> stringArray;
          for (const auto &arrayItem : value.as_array())
          {
            if (arrayItem.is_string())
            {
              stringArray.push_back(arrayItem.as_string());
            }
            else
            {
              // TODO Handle non-string values in arrays
            }
          }
          args[key] = plugin::Argument(stringArray);
        }
        // For other types (null, object), convert to string representation
        else
        {
          args[key] = plugin::Argument(zjson::to_string(value).value_or(""));
        }
      }
    }

    MiddlewareContext context(request.method, args);

    // Dispatch the command
    int result = dispatcher.dispatch(request.method, context);

    if (result == 0)
    {
      // Success - get output and convert to JSON
      std::string output = context.get_output_content();
      zjson::Value resultJson;

      if (!output.empty())
      {
        // Try to parse output as JSON, fallback to string
        auto parse_result = zjson::from_str<zjson::Value>(output);
        if (parse_result.has_value())
        {
          resultJson = parse_result.value();
        }
        else
        {
          resultJson = zjson::Value(output);
        }
      }
      else
      {
        resultJson = zjson::Value::create_object();
      }

      printCommandResponse(resultJson, request.id);
    }
    else
    {
      // Error occurred
      std::string errorOutput = context.get_error_content();
      ErrorDetails error{
          -32603, // Internal error
          "Command execution failed",
          errorOutput.empty() ? zstd::optional<zjson::Value>() : zstd::optional<zjson::Value>(zjson::Value(errorOutput))};
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
  workers.reserve(numWorkers);

  // Create workers
  for (int i = 0; i < numWorkers; ++i)
  {
    std::unique_ptr<Worker> worker(new Worker(i, &responseMutex));
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
