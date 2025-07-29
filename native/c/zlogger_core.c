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

#include "zlogger_core.h"
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Global logger state */
static zlog_state_t g_logger_state = {0};

/* Level strings */
static const char *level_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF"};

/* Compare-and-swap implementation for z/OS locking */
#ifdef __MVS__
#define CS_INSTRUCTION(old_val, new_val, mem_addr) old_val = __plo_CS(mem_addr, (unsigned int *)mem_addr, old_val, (unsigned int *)&new_val)
#else
#define CS_INSTRUCTION(old_val, new_val, mem_addr)
#endif

int zlog_acquire_lock(volatile unsigned int *lock_word)
{
  int old_val = 0;
  unsigned int new_val = 1;
  int attempts = 0;
  const int max_attempts = 10000;

  while (attempts < max_attempts)
  {
    old_val = 0;
    CS_INSTRUCTION(old_val, new_val, (void *)lock_word);

    if (old_val == 0)
    {
      /* Lock acquired successfully */
      return 0;
    }

    /* Brief pause before retry */
    for (unsigned int i = 0; i < 100; i++)
      ;
    attempts++;
  }

  /* Failed to acquire lock after max attempts */
  return -1;
}

void zlog_release_lock(volatile unsigned int *lock_word)
{
  *lock_word = 0;
}

int zlog_format_timestamp(char *buffer, size_t buffer_size)
{
  time_t now;
  struct tm *timeinfo;

  if (!buffer || buffer_size < ZLOG_MAX_TIMESTAMP_LEN)
  {
    return -1;
  }

  time(&now);
  timeinfo = localtime(&now);

  if (!timeinfo)
  {
    return -1;
  }

  if (strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", timeinfo) == 0)
  {
    return -1;
  }

  return 0;
}

const char *zlog_level_to_str(zlog_level_t level)
{
  if (level >= ZLOG_TRACE && level <= ZLOG_OFF)
  {
    return level_strings[level];
  }
  return "UNKNOWN";
}

#ifdef __MVS__
int zlog_init_dcb(zlog_state_t *state)
{
  /* Initialize DCB for z/OS dataset I/O */
  /* This is a simplified implementation - full DCB setup would require */
  /* more detailed z/OS system programming */

  strncpy(state->dcb_info.dcbname, "ZLOGDCB", 8);
  strncpy(state->dcb_info.dsname, "ZOWE.LOG.DATASET", 44);
  strncpy(state->dcb_info.ddname, "ZLOGDD", 8);
  state->dcb_info.lrecl = 80;
  state->dcb_info.recfm = 'F'; /* Fixed length records */
  state->dcb_info.blksize = 800;
  state->dcb_info.dcb_ptr = NULL; /* Would point to actual DCB control block */

  /* For now, we'll use Unix I/O as fallback since full DCB implementation */
  /* requires extensive z/OS system programming knowledge */
  state->use_dcb = 0;

  return 0;
}

// ISGENQ
// setlock
// attach task (spawning a new thread)

int zlog_write_dcb(zlog_state_t *state, const char *message)
{
  /* Write using DCB - simplified implementation */
  /* In a full implementation, this would use QSAM or BSAM macros */

  if (!state->use_dcb)
  {
    /* Fall back to Unix I/O */
    return write(state->fd, message, strlen(message));
  }

  /* DCB write implementation would go here */
  /* For now, fall back to Unix I/O */
  return write(state->fd, message, strlen(message));
}

void zlog_close_dcb(zlog_state_t *state)
{
  if (state->dcb_info.dcb_ptr)
  {
    /* Close DCB - would use CLOSE macro in full implementation */
    state->dcb_info.dcb_ptr = NULL;
  }
  state->use_dcb = 0;
}
#endif

int zlog_init(const char *log_file_path, zlog_level_t min_level)
{
  if (!log_file_path)
  {
    return -1;
  }

  if (zlog_acquire_lock(&g_logger_state.lock_word) != 0)
  {
    return -1;
  }

  /* Initialize state */
  strncpy(g_logger_state.log_file_path, log_file_path, ZLOG_MAX_PATH_LEN - 1);
  g_logger_state.log_file_path[ZLOG_MAX_PATH_LEN - 1] = '\0';
  g_logger_state.min_level = min_level;

#ifdef __MVS__
  /* Try to initialize DCB first */
  if (zlog_init_dcb(&g_logger_state) == 0)
  {
    g_logger_state.use_dcb = 1;
  }
  else
  {
    g_logger_state.use_dcb = 0;
  }
#endif

  /* Open file for Unix I/O (fallback or primary) */
  g_logger_state.fd = open(log_file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (g_logger_state.fd == -1)
  {
    zlog_release_lock(&g_logger_state.lock_word);
    return -1;
  }

  g_logger_state.initialized = 1;
  zlog_release_lock(&g_logger_state.lock_word);

  return 0;
}

void zlog_set_level(zlog_level_t level)
{
  if (zlog_acquire_lock(&g_logger_state.lock_word) == 0)
  {
    g_logger_state.min_level = level;
    zlog_release_lock(&g_logger_state.lock_word);
  }
}

zlog_level_t zlog_get_level(void)
{
  zlog_level_t level;
  if (zlog_acquire_lock(&g_logger_state.lock_word) == 0)
  {
    level = g_logger_state.min_level;
    zlog_release_lock(&g_logger_state.lock_word);
    return level;
  }
  return ZLOG_OFF;
}

int zlog_write_msg(zlog_level_t level, const char *message)
{
  char timestamp[ZLOG_MAX_TIMESTAMP_LEN];
  char formatted_msg[ZLOG_MAX_MSG_LEN];
  const char *level_str;
  int result = 0;

  if (!g_logger_state.initialized || !message)
  {
    return -1;
  }

  /* Check if we should log this level */
  if (level < g_logger_state.min_level || level == ZLOG_OFF)
  {
    return 0;
  }

  /* Acquire lock for thread/process safety */
  if (zlog_acquire_lock(&g_logger_state.lock_word) != 0)
  {
    return -1;
  }

  /* Format timestamp */
  if (zlog_format_timestamp(timestamp, sizeof(timestamp)) != 0)
  {
    strcpy(timestamp, "UNKNOWN");
  }

  /* Get level string */
  level_str = zlog_level_to_str(level);

  /* Format complete message */
  snprintf(formatted_msg, sizeof(formatted_msg), "[%s] [%s] %s\n",
           timestamp, level_str, message);

#ifdef __MVS__
  if (g_logger_state.use_dcb)
  {
    result = zlog_write_dcb(&g_logger_state, formatted_msg);
  }
  else
  {
    result = write(g_logger_state.fd, formatted_msg, strlen(formatted_msg));
  }
#else
  result = write(g_logger_state.fd, formatted_msg, strlen(formatted_msg));
#endif

  /* Force write to disk */
  if (result > 0)
  {
    fsync(g_logger_state.fd);
  }

  zlog_release_lock(&g_logger_state.lock_word);

  return (result > 0) ? 0 : -1;
}

int zlog_write(zlog_level_t level, const char *format, ...)
{
  char message[ZLOG_MAX_MSG_LEN];
  va_list args;

  if (!format)
  {
    return -1;
  }

  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);

  return zlog_write_msg(level, message);
}

void zlog_cleanup(void)
{
  if (zlog_acquire_lock(&g_logger_state.lock_word) == 0)
  {
    if (g_logger_state.fd != -1)
    {
      close(g_logger_state.fd);
      g_logger_state.fd = -1;
    }

#ifdef __MVS__
    if (g_logger_state.use_dcb)
    {
      zlog_close_dcb(&g_logger_state);
    }
#endif

    g_logger_state.initialized = 0;
    zlog_release_lock(&g_logger_state.lock_word);
  }
}