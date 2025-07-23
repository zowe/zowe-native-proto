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

#ifndef ZSHMEM_HPP
#define ZSHMEM_HPP

#ifndef _OPEN_SYS
#define _OPEN_SYS 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/shm.h>
#include <iostream>

using namespace std;

// Shared memory structure for inter-process communication
struct SharedMemoryData
{
  pthread_mutex_t mutex;
  int animal_count;
  char raw_data[4096]; // Raw data buffer
};

// Shared memory base key (each process will add its PID)
#define SHM_BASE_KEY 0x12345000
#define SHM_SIZE sizeof(SharedMemoryData)

// Generate a unique shared memory key for each process
inline key_t generate_unique_shm_key()
{
  return SHM_BASE_KEY + getpid();
}

// Cleanup shared memory (defined first since it's used by init_shared_memory)
inline void cleanup_shared_memory(int shm_id, SharedMemoryData *shm_ptr)
{
  shmdt(shm_ptr);
  shmctl(shm_id, IPC_RMID, nullptr);
}

// Shared memory management functions
inline int init_shared_memory(SharedMemoryData **shm_ptr)
{
  key_t shm_key = generate_unique_shm_key();
  int shm_id = shmget(shm_key, SHM_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
  if (shm_id == -1)
  {
    cerr << "Error: could not create shared memory segment: " << strerror(errno) << endl;
    return -1;
  }

  void *shm_addr = shmat(shm_id, nullptr, 0);
  if (shm_addr == (void *)-1)
  {
    cerr << "Error: could not attach to shared memory segment: " << strerror(errno) << endl;
    return -1;
  }

  *shm_ptr = static_cast<SharedMemoryData *>(shm_addr);

  // Initialize mutex and data for this new shared memory segment
  pthread_mutexattr_t mutex_attr;
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);

  if (pthread_mutex_init(&(*shm_ptr)->mutex, &mutex_attr) != 0)
  {
    cerr << "Error: could not initialize mutex: " << strerror(errno) << endl;
    pthread_mutexattr_destroy(&mutex_attr);
    cleanup_shared_memory(shm_id, *shm_ptr);
    return -1;
  }

  (*shm_ptr)->animal_count = 0;
  memset((*shm_ptr)->raw_data, 0, sizeof((*shm_ptr)->raw_data));
  strcpy((*shm_ptr)->raw_data, "Initial shared data");

  pthread_mutexattr_destroy(&mutex_attr);

  return shm_id;
}

inline void print_shared_memory_status(SharedMemoryData *shm_ptr)
{
  cout << "Animal count: " << shm_ptr->animal_count << endl;
  cout << "Raw data: " << shm_ptr->raw_data << endl;
}

inline void increment_animal_count(SharedMemoryData *shm_ptr)
{
  pthread_mutex_lock(&shm_ptr->mutex);
  shm_ptr->animal_count++;
  pthread_mutex_unlock(&shm_ptr->mutex);
}

inline void decrement_animal_count(SharedMemoryData *shm_ptr)
{
  pthread_mutex_lock(&shm_ptr->mutex);
  shm_ptr->animal_count--;
  pthread_mutex_unlock(&shm_ptr->mutex);
}

inline void set_raw_data(SharedMemoryData *shm_ptr, const char *data, size_t len)
{
  pthread_mutex_lock(&shm_ptr->mutex);
  size_t copy_len = min(len, sizeof(shm_ptr->raw_data) - 1);
  strncpy(shm_ptr->raw_data, data, copy_len);
  shm_ptr->raw_data[copy_len] = '\0';
  pthread_mutex_unlock(&shm_ptr->mutex);
}

#endif // ZSHMEM_HPP
