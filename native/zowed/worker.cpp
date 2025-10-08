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
#include "server.hpp"
#include "logger.hpp"

using std::string;

// Worker implementation
Worker::Worker(int workerId)
    : id(workerId), ready(false), shouldStop(false)
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
  LOG_DEBUG("Worker %d started", id);
}

void Worker::stop()
{
  shouldStop = true;
  queueCondition.notify_all();
  if (workerThread.joinable())
  {
    workerThread.join();
  }
  LOG_DEBUG("Worker %d stopped", id);
}

void Worker::addRequest(const string &request)
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
      string request = requestQueue.front();
      requestQueue.pop();
      lock.unlock();

      processRequest(request);
    }
  }
}

void Worker::processRequest(const string &data)
{
  // Delegate JSON-RPC processing to the RpcServer singleton
  RpcServer &server = RpcServer::get_instance();
  server.processRequest(data);
}

// WorkerPool implementation
WorkerPool::WorkerPool(int numWorkers) : readyCount(0), isShuttingDown(false)
{
  workers.reserve(numWorkers);

  // Create workers
  for (int i = 0; i < numWorkers; ++i)
  {
    std::unique_ptr<Worker> worker(new Worker(i));
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
    LOG_ERROR("Invalid worker ID: %d", workerId);
    return;
  }

  LOG_DEBUG("Initializing worker %d", workerId);

  // Start the worker
  workers[workerId]->start();

  // Mark worker as ready
  setWorkerReady(workerId);
}

void WorkerPool::distributeRequest(const string &request)
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
  LOG_DEBUG("Shutting down worker pool");
  isShuttingDown = true;
  readyCondition.notify_all();

  for (auto &worker : workers)
  {
    if (worker)
    {
      worker->stop();
    }
  }
  LOG_DEBUG("Worker pool shutdown complete");
}
