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
    // Check environment variable for log level
    const char *env_level = std::getenv("ZOWEX_LOG_LEVEL");
    if (env_level)
    {
      set_level_from_str(env_level);
    }
    else
    {
      std::cout << "[*] ZOWEX_LOG_LEVEL not set, using default level: " << default_level_ << std::endl;
    }

    // Create logs directory if it doesn't exist
    create_logs_dir();

    std::cout << "ZLogger: calling ZLGINIT" << std::endl;

    // Initialize Metal C logger with default path
    if (ZLGINIT("logs/zowex.log", &default_level_) == 0)
    {
      metal_c_initialized_ = true;
      trace("ZLogger: Metal C DD logging initialized successfully.");
    }
    else
    {
      metal_c_initialized_ = false;
      // Log to stderr if Metal C init fails
      std::cerr << "ZLogger: Metal C initialization failed" << std::endl;
    }
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
    std::cout << "[*] default_level_: " << default_level_ << std::endl;
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
   * Initialize logger with specific DD path (called after DD allocation)
   */
  auto initialize_with_dd_path(const std::string &log_path) -> void
  {
    // Cleanup existing Metal C logger
    if (metal_c_initialized_)
    {
      ZLGCLEAN();
    }

    // Re-initialize with new path
    // if (zlg_init(log_path.c_str(), default_level_) == 0)
    // {
    //   metal_c_initialized_ = true;
    // }
    // else
    // {
    //   metal_c_initialized_ = false;
    // }
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
  auto log(LogLevel level, const char *format, ...) -> void
  {
    if (level == ZLOGLEVEL_OFF || !metal_c_initialized_)
    {
      return;
    }

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ZLGWRITE(level, buffer);
  }

  /**
   * Log helper functions for each level
   */
  auto trace(const char *format, ...) -> void
  {
    if (!metal_c_initialized_)
    {
      return;
    }

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ZLGWRITE(ZLOGLEVEL_TRACE, buffer);
  }

  auto debug(const char *format, ...) -> void
  {
    if (!metal_c_initialized_)
    {
      return;
    }

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ZLGWRITE(ZLOGLEVEL_DEBUG, buffer);
  }

  auto info(const char *format, ...) -> void
  {
    if (!metal_c_initialized_)
    {
      return;
    }

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ZLGWRITE(ZLOGLEVEL_INFO, buffer);
  }

  auto warn(const char *format, ...) -> void
  {
    if (!metal_c_initialized_)
    {
      return;
    }

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ZLGWRITE(ZLOGLEVEL_WARN, buffer);
  }

  auto error(const char *format, ...) -> void
  {
    if (!metal_c_initialized_)
    {
      return;
    }

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ZLGWRITE(ZLOGLEVEL_ERROR, buffer);
  }

  auto fatal(const char *format, ...) -> void
  {
    if (!metal_c_initialized_)
    {
      return;
    }

    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ZLGWRITE(ZLOGLEVEL_FATAL, buffer);
  }
};

/**
 * Convenience macros for easier logging usage
 */
#define ZLOG_TRACE(...) ZLogger::get_instance().trace(__VA_ARGS__)
#define ZLOG_DEBUG(...) ZLogger::get_instance().debug(__VA_ARGS__)
#define ZLOG_INFO(...) ZLogger::get_instance().info(__VA_ARGS__)
#define ZLOG_WARN(...) ZLogger::get_instance().warn(__VA_ARGS__)
#define ZLOG_ERROR(...) ZLogger::get_instance().error(__VA_ARGS__)
#define ZLOG_FATAL(...) ZLogger::get_instance().fatal(__VA_ARGS__)

#endif // ZLOGGER_HPP