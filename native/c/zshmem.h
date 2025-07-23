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

#define _XOPEN_SOURCE 600
#define __SUSV3_XSI 1

#ifndef _XPG4
#define _XPG4 1
#endif

#ifndef __UU
#define __UU
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <le/pthread.h>
#include <sys/mman.h>
#include <le/sys/stat.h>
#include <le/fcntl.h>
#include <le/unistd.h>
#include <iostream>

using namespace std;

#ifndef _PTHREAD_H
struct pthread_mutex_t
{
  char __data[68];
};

struct pthread_mutexattr_t
{
  char __data[16];
};

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
typedef struct SharedMemory
{
  pthread_mutex_t mutex;
  int animal_count;
  char raw_data[4096];
} ZSharedRegion;

#define SHM_SIZE sizeof(ZSharedRegion)

struct ZShmContext
{
  ZSharedRegion *shm_ptr;
  int fd;
  char file_path[256];
  size_t size;
};

// Cleanup shared memory
inline void cleanup_shared_memory(int shm_id, ZSharedRegion *shm_ptr)
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
}

// Create a new shared memory segment
inline int create_shared_memory(ZSharedRegion **shm_ptr)
{
  char temp_path[256];
  snprintf(temp_path, sizeof(temp_path), "/tmp/zowe_shm_%d", getpid());

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

  *shm_ptr = static_cast<ZSharedRegion *>(addr);

  // Initialize mutex and data
  pthread_mutexattr_t mutex_attr;
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);

  if (pthread_mutex_init(&(*shm_ptr)->mutex, &mutex_attr) != 0)
  {
    cerr << "Error: could not initialize mutex: " << strerror(errno) << endl;
    pthread_mutexattr_destroy(&mutex_attr);
    cleanup_shared_memory(fd, *shm_ptr);
    unlink(temp_path);
    return -1;
  }

  (*shm_ptr)->animal_count = 0;
  memset((*shm_ptr)->raw_data, 0, sizeof((*shm_ptr)->raw_data));
  strcpy((*shm_ptr)->raw_data, "Initial shared data");

  pthread_mutexattr_destroy(&mutex_attr);

  // Unlink the file for automatic cleanup
  unlink(temp_path);

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

// Legacy function for backward compatibility - now creates shared memory
inline int init_shared_memory(ZSharedRegion **shm_ptr)
{
  return create_shared_memory(shm_ptr);
}

inline void print_shared_memory_status(ZSharedRegion *shm_ptr)
{
  cout << "Animal count: " << shm_ptr->animal_count << endl;
  cout << "Raw data: " << shm_ptr->raw_data << endl;
}

inline void increment_animal_count(ZSharedRegion *shm_ptr)
{
  pthread_mutex_lock(&shm_ptr->mutex);
  shm_ptr->animal_count++;
  pthread_mutex_unlock(&shm_ptr->mutex);
}

inline void decrement_animal_count(ZSharedRegion *shm_ptr)
{
  pthread_mutex_lock(&shm_ptr->mutex);
  shm_ptr->animal_count--;
  pthread_mutex_unlock(&shm_ptr->mutex);
}

inline void set_raw_data(ZSharedRegion *shm_ptr, const char *data, size_t len)
{
  pthread_mutex_lock(&shm_ptr->mutex);
  size_t copy_len = min(len, sizeof(shm_ptr->raw_data) - 1);
  strncpy(shm_ptr->raw_data, data, copy_len);
  shm_ptr->raw_data[copy_len] = '\0';
  pthread_mutex_unlock(&shm_ptr->mutex);
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
  increment_animal_count(shm_ptr);
  set_raw_data(shm_ptr, "Hello from pthread test", 23);
  print_shared_memory_status(shm_ptr);

  cleanup_shared_memory(shm_id, shm_ptr);
  return 0;
}

#endif