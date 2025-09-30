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

// Forward declarations
class Worker;
class WorkerPool;

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

  void workerLoop();
  void processRequest(const std::string &data);

public:
  Worker(int workerId);
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
