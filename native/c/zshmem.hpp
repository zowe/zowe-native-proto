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

#ifndef ZSHMEM_H
#define ZSHMEM_H
#include <stdio.h>

#define _XOPEN_SOURCE 600
#define __SUSV3_XSI 1

#ifndef _XPG4
#define _XPG4 1
#endif

#ifndef __UU
#define __UU
#endif

#define _ISOC99_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <le/sys/stat.h>
#include <le/fcntl.h>
#include <le/unistd.h>
#include <iostream>
#include <fstream>
using namespace std;


// Shared memory structure for inter-process communication
#pragma pack(1)
typedef struct SharedMemory
{
  _Atomic int progress;
} ZSharedRegion;
#pragma pack(reset)

#define SHM_SIZE sizeof(ZSharedRegion)

struct ZShmContext
{
  ZSharedRegion *shm_ptr;
  int fd;
  char file_path[256];
  size_t size;
};

class ZShared
{
  ZShared()
  {
  }

  ~ZShared()
  {
    // TODO: clean up memory region
  }

  static ZShared *_instance;

public:
  ZSharedRegion *region;
  static ZShared *instance()
  {
    if (_instance == nullptr)
    {
      _instance = new ZShared();
    }

    return _instance;
  }
};

// Cleanup shared memory
inline void cleanup_shared_memory(int shm_id, ZSharedRegion *shm_ptr, const char *file_path = nullptr)
{
  // Note: shm_id is now a file descriptor
  if (shm_ptr)
  {
    munmap(shm_ptr, SHM_SIZE);
  }
  if (shm_id != -1)
  {
    close(shm_id);
  }
  // Clean up the file if path is provided
  if (file_path && strlen(file_path) > 0)
  {
    unlink(file_path);
  }
}

// Create a new shared memory segment
inline int create_shared_memory(ZSharedRegion **shm_ptr, char *file_path_out = nullptr)
{
  char temp_path[256];
  sprintf(temp_path, "/tmp/zowe_shm_%d", getpid());

  int fd = open(temp_path, O_CREAT | O_RDWR | O_EXCL, 0600);
  if (fd == -1)
  {
    cerr << "Error: could not create shared memory file: " << strerror(errno) << endl;
    return -1;
  }

  // Size the file
  if (ftruncate(fd, SHM_SIZE) == -1)
  {
    cerr << "Error: could not size shared memory file: " << strerror(errno) << endl;
    close(fd);
    unlink(temp_path);
    return -1;
  }

  // Map the file into memory
  void *addr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED)
  {
    cerr << "Error: could not map shared memory: " << strerror(errno) << endl;
    close(fd);
    unlink(temp_path);
    return -1;
  }

  // Zero-initialize the shared memory region
  memset(addr, 0, SHM_SIZE);

  *shm_ptr = static_cast<ZSharedRegion *>(addr);
  ZShared::instance()->region = *shm_ptr;

  // Store the file path for later cleanup instead of unlinking immediately
  if (file_path_out)
  {
    strcpy(file_path_out, temp_path);
  }

  return fd;
}

// Attach to an existing shared memory segment (for worker processes)
inline int attach_shared_memory(int fd, ZSharedRegion **shm_ptr)
{
  if (fd <= 0)
  {
    cerr << "Error: invalid file descriptor: " << fd << endl;
    return -1;
  }

  // Map the existing file
  void *addr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED)
  {
    cerr << "Error: could not attach to shared memory segment " << fd << ": " << strerror(errno) << endl;
    return -1;
  }

  *shm_ptr = static_cast<ZSharedRegion *>(addr);
  return 0;
}

inline int init_shared_memory(ZSharedRegion **shm_ptr, char *file_path_out = nullptr)
{
  return create_shared_memory(shm_ptr, file_path_out);
}

inline void print_shared_memory_status(ZSharedRegion *shm_ptr)
{
  cout << "Animal count: " << shm_ptr->progress << endl;
}

inline void increment_progress(ZSharedRegion *shm_ptr)
{
  shm_ptr->progress++;
}

inline void decrement_progress(ZSharedRegion *shm_ptr)
{
  shm_ptr->progress--;
}

inline void set_progress(int progress)
{
  ofstream stream("./zowe-native-proto/golang/zowex_progress.log", ios_base::out | ios_base::app);
  if(!stream.good()){
    return;
  }

  auto *shared_memory_map = ZShared::instance()->region;
  shared_memory_map->progress = progress;
  stream << "set_progress: " << progress << endl;
  stream << "- C++ read from shared memory: " << shared_memory_map->progress << endl;
  stream.close();
}

inline int test_shared_memory()
{
  ZSharedRegion *shm_ptr = nullptr;
  int shm_id = init_shared_memory(&shm_ptr);

  if (shm_id == -1)
  {
    return -1;
  }

  // Test the mutex operations
  increment_progress(shm_ptr);
  print_shared_memory_status(shm_ptr);

  cleanup_shared_memory(shm_id, shm_ptr);
  return 0;
}
#endif