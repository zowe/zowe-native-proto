#ifndef ZLOGGER_HPP
#define ZLOGGER_HPP

#include <cstdio>
#include <cstdarg>
#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include "singleton.hpp"

/**
 * Log levels supported by ZLogger
 */
enum class LogLevel
{
  TRACE = 0,
  DEBUG = 1,
  INFO = 2,
  WARN = 3,
  ERROR = 4,
  FATAL = 5,
  OFF = 6
};

/**
 * File transport for logging to specific files with their own log levels
 */
class FileTransport
{
private:
  std::string filename_;
  LogLevel level_;
  std::ofstream file_;

public:
  explicit FileTransport(const std::string &filename, LogLevel level = LogLevel::INFO)
      : filename_(filename), level_(level)
  {
    file_.open(filename_.c_str(), std::ios::app);
    if (!file_.is_open())
    {
      std::cerr << "Failed to open log file: " << filename_ << std::endl;
    }
  }

  ~FileTransport()
  {
    if (file_.is_open())
    {
      file_.close();
    }
  }

  bool should_log(LogLevel level) const
  {
    return level >= level_ && level != LogLevel::OFF;
  }

  void write(LogLevel level, const std::string &message)
  {
    if (!should_log(level) || !file_.is_open())
    {
      return;
    }

    file_ << message << std::endl;
    file_.flush();
  }

  void set_level(LogLevel level)
  {
    level_ = level;
  }

  LogLevel get_level() const
  {
    return level_;
  }

  const std::string &get_filename() const
  {
    return filename_;
  }
};

/**
 * ZLogger singleton class for centralized logging
 */
class ZLogger : public Singleton<ZLogger>
{
  friend class Singleton<ZLogger>;

private:
  LogLevel default_level_;
  std::vector<FileTransport *> transports_;

protected:
  ZLogger()
      : default_level_(LogLevel::INFO)
  {
    // Check environment variable for log level
    const char *env_level = std::getenv("ZOWEX_LOG_LEVEL");
    if (env_level)
    {
      set_level_from_str(env_level);
    }

    // Create logs directory if it doesn't exist
    create_logs_dir();

    // Add default file transport
    add_file_transport("logs/zowex.log", default_level_);
  }

  void create_logs_dir()
  {
    if (mkdir("logs", 0750) == -1)
    {
      if (errno != EEXIST)
      {
        std::cerr << "Failed to create logs directory: " << strerror(errno) << std::endl;
      }
    }
  }

  void set_level_from_str(const std::string &level_str)
  {
    std::string upper_level = level_str;
    std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);

    if (upper_level == "TRACE")
    {
      default_level_ = LogLevel::TRACE;
    }
    else if (upper_level == "DEBUG")
    {
      default_level_ = LogLevel::DEBUG;
    }
    else if (upper_level == "INFO")
    {
      default_level_ = LogLevel::INFO;
    }
    else if (upper_level == "WARN" || upper_level == "WARNING")
    {
      default_level_ = LogLevel::WARN;
    }
    else if (upper_level == "ERROR")
    {
      default_level_ = LogLevel::ERROR;
    }
    else if (upper_level == "FATAL")
    {
      default_level_ = LogLevel::FATAL;
    }
    else if (upper_level == "OFF")
    {
      default_level_ = LogLevel::OFF;
    }
  }

  std::string format_msg(LogLevel level, const char *format, va_list args)
  {
    // Get current timestamp
    time_t now = time(0);
    char timestamp[100];
    struct tm *timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Get log level string
    const char *level_str = get_level_str(level);

    // Format the user message
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);

    // Combine timestamp, level, and message
    char final_message[4096 + 200];
    snprintf(final_message, sizeof(final_message), "[%s] [%s] %s", timestamp, level_str, buffer);

    return std::string(final_message);
  }

  const char *get_level_str(LogLevel level)
  {
    switch (level)
    {
    case LogLevel::TRACE:
      return "TRACE";
    case LogLevel::DEBUG:
      return "DEBUG";
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::WARN:
      return "WARN";
    case LogLevel::ERROR:
      return "ERROR";
    case LogLevel::FATAL:
      return "FATAL";
    default:
      return "UNKNOWN";
    }
  }

public:
  /**
   * Set the default log level for the logger
   */
  void set_log_level(LogLevel level)
  {
    default_level_ = level;
  }

  /**
   * Get the current default log level
   */
  LogLevel get_log_level() const
  {
    return default_level_;
  }

  /**
   * Add a file transport with specified filename and log level
   */
  void add_file_transport(const std::string &filename, LogLevel level = LogLevel::INFO)
  {
    transports_.push_back(new FileTransport(filename, level));
  }

  /**
   * Remove all file transports
   */
  void clear_transports()
  {
    transports_.clear();
  }

  /**
   * Log a message at the specified level
   */
  void log(LogLevel level, const char *format, ...)
  {
    if (level == LogLevel::OFF)
    {
      return;
    }

    va_list args;
    va_start(args, format);
    std::string message = format_msg(level, format, args);
    va_end(args);

    for (auto i = 0; i < transports_.size(); i++)
    {
      transports_[i]->write(level, message);
    }
  }

  /**
   * Log helper functions for each level
   */
  void trace(const char *format, ...)
  {
    va_list args;
    va_start(args, format);
    std::string message = format_msg(LogLevel::TRACE, format, args);
    va_end(args);

    for (auto i = 0; i < transports_.size(); i++)
    {
      transports_[i]->write(LogLevel::TRACE, message);
    }
  }

  void debug(const char *format, ...)
  {
    va_list args;
    va_start(args, format);
    std::string message = format_msg(LogLevel::DEBUG, format, args);
    va_end(args);

    for (auto i = 0; i < transports_.size(); i++)
    {
      transports_[i]->write(LogLevel::DEBUG, message);
    }
  }

  void info(const char *format, ...)
  {
    va_list args;
    va_start(args, format);
    std::string message = format_msg(LogLevel::INFO, format, args);
    va_end(args);

    for (auto i = 0; i < transports_.size(); i++)
    {
      transports_[i]->write(LogLevel::INFO, message);
    }
  }

  void warn(const char *format, ...)
  {
    va_list args;
    va_start(args, format);
    std::string message = format_msg(LogLevel::WARN, format, args);
    va_end(args);

    for (auto i = 0; i < transports_.size(); i++)
    {
      transports_[i]->write(LogLevel::WARN, message);
    }
  }

  void error(const char *format, ...)
  {
    va_list args;
    va_start(args, format);
    std::string message = format_msg(LogLevel::ERROR, format, args);
    va_end(args);

    for (auto i = 0; i < transports_.size(); i++)
    {
      transports_[i]->write(LogLevel::ERROR, message);
    }
  }

  void fatal(const char *format, ...)
  {
    va_list args;
    va_start(args, format);
    std::string message = format_msg(LogLevel::FATAL, format, args);
    va_end(args);

    for (auto i = 0; i < transports_.size(); i++)
    {
      transports_[i]->write(LogLevel::FATAL, message);
    }
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