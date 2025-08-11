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

#ifndef ZLOGGER_HPP
#define ZLOGGER_HPP

#include <cstdio>
#include <cstdarg>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <iostream>
#include "singleton.hpp"
#include "zlogger_metal.h"

#ifdef __MVS__
#include <sys/stat.h>
#include <errno.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif

typedef zlog_level_t LogLevel;

/**
 * ZLogger singleton class for centralized logging
 */
class ZLogger : public Singleton<ZLogger>
{
  friend class Singleton<ZLogger>;

private:
  int default_level_;
  bool metal_c_initialized_;

protected:
  ZLogger()
      : default_level_(ZLOGLEVEL_INFO), metal_c_initialized_(false)
  {
#ifdef ZLOG_ENABLE
    // Check environment variable for log level
    const char *env_level = std::getenv("ZOWEX_LOG_LEVEL");
    if (env_level)
    {
      set_level_from_str(env_level);
    }

    // Create logs directory if it doesn't exist
    create_logs_dir();

    // Get current working directory
    char cwd[260] = {0};
    std::string log_path_str;

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      log_path_str = std::string(cwd) + "/logs/zowex.log";
    }
    else
    {
      log_path_str = "logs/zowex.log";
    }

    // Initialize Metal C logger with default path
    metal_c_initialized_ = ZLGINIT(log_path_str.c_str(), &default_level_) == 0;
#endif
  }

  auto create_logs_dir() -> void
  {
    if (mkdir("logs", 0750) == -1)
    {
      if (errno != EEXIST)
      {
        std::cerr << "Failed to create logs directory: " << strerror(errno) << std::endl;
      }
    }
  }

  auto set_level_from_str(const std::string &level_str) -> void
  {
    std::string upper_level = level_str;
    std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);

    if (upper_level == "TRACE")
    {
      default_level_ = ZLOGLEVEL_TRACE;
    }
    else if (upper_level == "DEBUG")
    {
      default_level_ = ZLOGLEVEL_DEBUG;
    }
    else if (upper_level == "INFO")
    {
      default_level_ = ZLOGLEVEL_INFO;
    }
    else if (upper_level == "WARN" || upper_level == "WARNING")
    {
      default_level_ = ZLOGLEVEL_WARN;
    }
    else if (upper_level == "ERROR")
    {
      default_level_ = ZLOGLEVEL_ERROR;
    }
    else if (upper_level == "FATAL")
    {
      default_level_ = ZLOGLEVEL_FATAL;
    }
    else if (upper_level == "OFF")
    {
      default_level_ = ZLOGLEVEL_OFF;
    }
  }

public:
  /**
   * Set the default log level for the logger
   */
  auto set_log_level(LogLevel level) -> void
  {
    default_level_ = level;
    if (metal_c_initialized_)
    {
      ZLGSTLVL(level);
    }
  }

  /**
   * Get the current default log level
   */
  auto get_log_level() const -> int
  {
    return default_level_;
  }

  /**
   * Log a message at the specified level
   */
  /**
   * Internal logging function that takes va_list
   */
  auto vlog(LogLevel level, const char *format, va_list args) -> void
  {
    if (level == ZLOGLEVEL_OFF || level < default_level_ || !metal_c_initialized_)
    {
      return;
    }

    int level_value = level;
    char buffer[4096] = {0};
    vsnprintf(buffer, sizeof(buffer), format, args);
    ZLGWRITE(&level_value, buffer);
  }

  /**
   * Main logging function with variadic arguments
   */
  auto log(LogLevel level, const char *format, ...) -> void
  {
    va_list args;
    va_start(args, format);
    vlog(level, format, args);
    va_end(args);
  }

  /**
   * Log helper functions for each level
   */
  auto trace(const char *format, ...) -> void
  {
    va_list args;
    va_start(args, format);
    vlog(ZLOGLEVEL_TRACE, format, args);
    va_end(args);
  }

  auto debug(const char *format, ...) -> void
  {
    va_list args;
    va_start(args, format);
    vlog(ZLOGLEVEL_DEBUG, format, args);
    va_end(args);
  }

  auto info(const char *format, ...) -> void
  {
    va_list args;
    va_start(args, format);
    vlog(ZLOGLEVEL_INFO, format, args);
    va_end(args);
  }

  auto warn(const char *format, ...) -> void
  {
    va_list args;
    va_start(args, format);
    vlog(ZLOGLEVEL_WARN, format, args);
    va_end(args);
  }

  auto error(const char *format, ...) -> void
  {
    va_list args;
    va_start(args, format);
    vlog(ZLOGLEVEL_ERROR, format, args);
    va_end(args);
  }

  auto fatal(const char *format, ...) -> void
  {
    va_list args;
    va_start(args, format);
    vlog(ZLOGLEVEL_FATAL, format, args);
    va_end(args);
  }

  auto fatal(const char *format, int exit_code) -> void
  {
    log(ZLOGLEVEL_FATAL, format);
    std::exit(exit_code);
  }
};

/**
 * Convenience macros for easier logging usage
 * These macros are gated by ZLOG_ENABLE - if not defined during compilation,
 * all logging operations become no-ops with zero overhead.
 */
#ifdef ZLOG_ENABLE
#define ZLOG_TRACE(...) ZLogger::get_instance().trace(__VA_ARGS__)
#define ZLOG_DEBUG(...) ZLogger::get_instance().debug(__VA_ARGS__)
#define ZLOG_INFO(...) ZLogger::get_instance().info(__VA_ARGS__)
#define ZLOG_WARN(...) ZLogger::get_instance().warn(__VA_ARGS__)
#define ZLOG_ERROR(...) ZLogger::get_instance().error(__VA_ARGS__)
#define ZLOG_FATAL(...) ZLogger::get_instance().fatal(__VA_ARGS__)
#else
/* When ZLOG_ENABLE is not defined, logging macros become no-ops */
#define ZLOG_TRACE(...) ((void)0)
#define ZLOG_DEBUG(...) ((void)0)
#define ZLOG_INFO(...) ((void)0)
#define ZLOG_WARN(...) ((void)0)
#define ZLOG_ERROR(...) ((void)0)
#define ZLOG_FATAL(...) ((void)0)
#endif /* ZLOG_ENABLE */

#endif // ZLOGGER_HPP