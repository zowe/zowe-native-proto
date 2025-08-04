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

#ifndef ZLOGGER_CORE_H
#define ZLOGGER_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifdef __MVS__
#include <dynit.h>
/* z/OS specific headers */
#else
/* Unix/Linux headers */
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  /* Log levels - must match the C++ enum */
  typedef enum
  {
    ZLOG_TRACE = 0,
    ZLOG_DEBUG = 1,
    ZLOG_INFO = 2,
    ZLOG_WARN = 3,
    ZLOG_ERROR = 4,
    ZLOG_FATAL = 5,
    ZLOG_OFF = 6
  } zlog_level_t;

/* Maximum message length */
#define ZLOG_MAX_MSG_LEN 4096
#define ZLOG_MAX_PATH_LEN 256
#define ZLOG_MAX_TIMESTAMP_LEN 32
#define ZLOG_MAX_LEVEL_STR_LEN 8

/* DCB-related structures for z/OS */
#ifdef __MVS__
  typedef struct
  {
    char dcbname[8]; /* DCB name */
    char dsname[44]; /* Dataset name */
    char ddname[8];  /* DD name */
    int lrecl;       /* Logical record length */
    char recfm;      /* Record format */
    int blksize;     /* Block size */
    void *dcb_ptr;   /* Pointer to actual DCB */
  } zlog_dcb_t;
#endif

  /* Logger state structure */
  typedef struct
  {
    char log_file_path[ZLOG_MAX_PATH_LEN];
    zlog_level_t min_level;
    int initialized;
    volatile unsigned int lock_word; /* For CS instruction locking */

#ifdef __MVS__
    zlog_dcb_t dcb_info;
    int use_dcb; /* Flag to use DCB vs Unix I/O */
#endif

    int fd; /* Unix file descriptor (fallback) */
  } zlog_state_t;

  /* Function prototypes */

  /**
   * Initialize the logger core
   * @param log_file_path Path to the log file
   * @param min_level Minimum log level to write
   * @return 0 on success, -1 on error
   */
  int zlog_init(const char *log_file_path, zlog_level_t min_level);

  /**
   * Write a log message at the specified level
   * @param level Log level
   * @param format Printf-style format string
   * @param ... Arguments for format string
   * @return 0 on success, -1 on error
   */
  int zlog_write(zlog_level_t level, const char *format, ...);

  /**
   * Write a log message with explicit arguments (for Metal C compatibility)
   * @param level Log level
   * @param message Pre-formatted message
   * @return 0 on success, -1 on error
   */
  int zlog_write_msg(zlog_level_t level, const char *message);

  /**
   * Set the minimum log level
   * @param level New minimum log level
   */
  void zlog_set_level(zlog_level_t level);

  /**
   * Get the current minimum log level
   * @return Current minimum log level
   */
  zlog_level_t zlog_get_level(void);

  /**
   * Cleanup the logger core
   */
  void zlog_cleanup(void);

  /**
   * Convert log level to string
   * @param level Log level
   * @return String representation of log level
   */
  const char *zlog_level_to_str(zlog_level_t level);

  /* Internal utility functions */

  /**
   * Acquire lock using compare-and-swap
   * @param lock_word Pointer to lock word
   * @return 0 on success (lock acquired), -1 on failure
   */
  int zlog_acquire_lock(volatile unsigned int *lock_word);

  /**
   * Release lock
   * @param lock_word Pointer to lock word
   */
  void zlog_release_lock(volatile unsigned int *lock_word);

  /**
   * Format timestamp
   * @param buffer Buffer to write timestamp to
   * @param buffer_size Size of buffer
   * @return 0 on success, -1 on error
   */
  int zlog_format_timestamp(char *buffer, size_t buffer_size);

#ifdef __MVS__
  /**
   * Initialize DCB for z/OS dataset I/O
   * @param state Logger state
   * @return 0 on success, -1 on error
   */
  int zlog_init_dcb(zlog_state_t *state);

  /**
   * Write to z/OS dataset using DCB
   * @param state Logger state
   * @param message Message to write
   * @return 0 on success, -1 on error
   */
  int zlog_write_dcb(zlog_state_t *state, const char *message);

  /**
   * Close DCB
   * @param state Logger state
   */
  void zlog_close_dcb(zlog_state_t *state);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZLOGGER_CORE_H */