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
#include <le/pthread.h>
#include <sys/mman.h>
#include <le/sys/stat.h>
#include <le/fcntl.h>
#include <le/unistd.h>
#include <iostream>

using namespace std;

#ifndef _PTHREAD_H
#ifndef __pthread_mutex_t
#define __pthread_mutex_t 1
#ifndef __OE_7
typedef struct
{
  unsigned long __m;
} pthread_mutex_t;
#else
typedef union
{
  unsigned long __m;
  double __d[8];
} pthread_mutex_t;
#endif
#endif

#ifndef __pthread_mutexattr_t
#define __pthread_mutexattr_t 1
typedef struct
{
#ifndef _LP64
  char __[0x04];
#else
  char __[0x08];
#endif
} pthread_mutexattr_t;
#endif

// pthread function declarations
extern "C"
{
  int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
  int pthread_mutex_destroy(pthread_mutex_t *mutex);
  int pthread_mutex_lock(pthread_mutex_t *mutex);
  int pthread_mutex_unlock(pthread_mutex_t *mutex);
  int pthread_mutexattr_init(pthread_mutexattr_t *attr);
  int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
  int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
}

// pthread constants
#define PTHREAD_PROCESS_SHARED 8
#endif

// Shared memory structure for inter-process communication
#pragma pack(1)
typedef struct SharedMemory
{
  pthread_mutex_t mutex;
  int progress;
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

  pthread_mutex_t mutex;

  // Initialize mutex and data
  pthread_mutexattr_t mutex_attr;
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);

  if (pthread_mutex_init(&mutex, &mutex_attr) != 0)
  {
    cerr << "Error: could not initialize mutex: " << strerror(errno) << endl;
    pthread_mutexattr_destroy(&mutex_attr);
    cleanup_shared_memory(fd, *shm_ptr);
    unlink(temp_path);
    return -1;
  }

  memcpy((void *)&ZShared::instance()->region->mutex, (void *)&mutex, sizeof(pthread_mutex_t));

  (*shm_ptr)->progress = 0;

  pthread_mutexattr_destroy(&mutex_attr);

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
  pthread_mutex_lock(&shm_ptr->mutex);
  shm_ptr->progress++;
  pthread_mutex_unlock(&shm_ptr->mutex);
}

inline void decrement_progress(ZSharedRegion *shm_ptr)
{
  pthread_mutex_lock(&shm_ptr->mutex);
  shm_ptr->progress--;
  pthread_mutex_unlock(&shm_ptr->mutex);
}

inline void set_progress(int progress)
{
  auto *shared_memory_map = ZShared::instance()->region;
  pthread_mutex_lock(&shared_memory_map->mutex);
  shared_memory_map->progress = progress;
  pthread_mutex_unlock(&shared_memory_map->mutex);
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