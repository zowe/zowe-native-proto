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

#ifndef ZLOGGER_METAL_H
#define ZLOGGER_METAL_H

/* Metal C compatible includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Log levels for Metal C - must match zlogger_core.h */
typedef enum
{
  ZLOG_METAL_TRACE = 0,
  ZLOG_METAL_DEBUG = 1,
  ZLOG_METAL_INFO = 2,
  ZLOG_METAL_WARN = 3,
  ZLOG_METAL_ERROR = 4,
  ZLOG_METAL_FATAL = 5,
  ZLOG_METAL_OFF = 6
} zlog_metal_level_t;

/* Maximum message length for Metal C */
#define ZLOG_METAL_MAX_MSG 1024

#ifdef __IBM_METAL__

/* External C functions callable from Metal C */
#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Initialize logger from Metal C code
   * @param log_file_path Path to log file (use Unix path format in USS)
   * @param min_level Minimum log level
   * @return 0 on success, -1 on error
   */
  int zlog_metal_init(const char *log_file_path, int min_level);

  /**
   * Write a pre-formatted message from Metal C
   * @param level Log level (use ZLOG_METAL_* constants)
   * @param message Pre-formatted message string
   * @return 0 on success, -1 on error
   */
  int zlog_metal_write(int level, const char *message);

  /**
   * Format and write a message from Metal C with simple formatting
   * Note: Metal C has limited printf support, so this provides basic formatting
   * @param level Log level
   * @param prefix Message prefix/category
   * @param message Main message
   * @param suffix Optional suffix (can be NULL)
   * @return 0 on success, -1 on error
   */
  int zlog_metal_write_formatted(int level, const char *prefix, const char *message, const char *suffix);

  /**
   * Set log level from Metal C
   * @param level New minimum log level
   */
  void zlog_metal_set_level(int level);

  /**
   * Get current log level from Metal C
   * @return Current minimum log level
   */
  int zlog_metal_get_level(void);

  /**
   * Cleanup logger from Metal C
   */
  void zlog_metal_cleanup(void);

#ifdef __cplusplus
}
#endif

/* Convenience macros for Metal C logging */
#define ZLOG_METAL_TRACE_MSG(msg) zlog_metal_write(ZLOG_METAL_TRACE, msg)
#define ZLOG_METAL_DEBUG_MSG(msg) zlog_metal_write(ZLOG_METAL_DEBUG, msg)
#define ZLOG_METAL_INFO_MSG(msg) zlog_metal_write(ZLOG_METAL_INFO, msg)
#define ZLOG_METAL_WARN_MSG(msg) zlog_metal_write(ZLOG_METAL_WARN, msg)
#define ZLOG_METAL_ERROR_MSG(msg) zlog_metal_write(ZLOG_METAL_ERROR, msg)
#define ZLOG_METAL_FATAL_MSG(msg) zlog_metal_write(ZLOG_METAL_FATAL, msg)

#define ZLOG_METAL_TRACE_FMT(prefix, msg, suffix) zlog_metal_write_formatted(ZLOG_METAL_TRACE, prefix, msg, suffix)
#define ZLOG_METAL_DEBUG_FMT(prefix, msg, suffix) zlog_metal_write_formatted(ZLOG_METAL_DEBUG, prefix, msg, suffix)
#define ZLOG_METAL_INFO_FMT(prefix, msg, suffix) zlog_metal_write_formatted(ZLOG_METAL_INFO, prefix, msg, suffix)
#define ZLOG_METAL_WARN_FMT(prefix, msg, suffix) zlog_metal_write_formatted(ZLOG_METAL_WARN, prefix, msg, suffix)
#define ZLOG_METAL_ERROR_FMT(prefix, msg, suffix) zlog_metal_write_formatted(ZLOG_METAL_ERROR, prefix, msg, suffix)
#define ZLOG_METAL_FATAL_FMT(prefix, msg, suffix) zlog_metal_write_formatted(ZLOG_METAL_FATAL, prefix, msg, suffix)

#endif /* __IBM_METAL__ */

#endif /* ZLOGGER_METAL_H */