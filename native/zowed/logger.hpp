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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <string>
#include <mutex>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cstdlib>
#include <cerrno>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

namespace zowed
{

/**
 * Simple logger class for zowed middleware
 * This is a header-only implementation that provides logging functionality
 */
class Logger
{
private:
  // Maximum log file size before truncation (10MB)
  static constexpr size_t MAX_LOG_SIZE = 10 * 1024 * 1024;

  static std::ofstream &get_log_file()
  {
    static std::ofstream logFile;
    return logFile;
  }

  static bool &get_verbose_logging()
  {
    static bool verboseLogging = false;
    return verboseLogging;
  }

  static std::mutex &get_log_mutex()
  {
    static std::mutex logMutex;
    return logMutex;
  }

  static bool &get_initialized()
  {
    static bool initialized = false;
    return initialized;
  }

  static std::string &get_log_file_path()
  {
    static std::string logFilePath;
    return logFilePath;
  }

  /**
   * Get the current timestamp as a formatted string
   */
  static std::string get_current_timestamp()
  {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
  }

  /**
   * Check log file size and truncate if necessary
   */
  static void check_and_truncate_log_file()
  {
    std::string &logFilePath = get_log_file_path();
    std::ofstream &logFile = get_log_file();

    if (logFilePath.empty() || !logFile.is_open())
    {
      return;
    }

    struct stat st;
    if (stat(logFilePath.c_str(), &st) == 0)
    {
      if (static_cast<size_t>(st.st_size) > MAX_LOG_SIZE)
      {
        logFile.close();

        // Reopen the file in truncate mode
        logFile.open(logFilePath.c_str(), std::ios::out | std::ios::trunc);
        if (!logFile.is_open())
        {
          std::cerr << "Failed to truncate log file: " << logFilePath << std::endl;
          return;
        }

        logFile << get_current_timestamp() << " [INFO] Log file truncated due to size limit\n";
        logFile.flush();
      }
    }
  }

public:
  /**
   * Initialize the logger with specified options
   * @param execDir Executable directory (must be provided)
   * @param verbose Whether to enable verbose logging
   * @param truncate Whether to truncate existing log file
   */
  static void init_logger(const char *execDir, bool verbose = false, bool truncate = false)
  {
    std::mutex &logMutex = get_log_mutex();
    std::ofstream &logFile = get_log_file();
    bool &verboseLogging = get_verbose_logging();
    bool &initialized = get_initialized();
    std::string &logFilePath = get_log_file_path();

    std::lock_guard<std::mutex> lock(logMutex);

    verboseLogging = verbose;

    // Create logs directory
    std::string logsDir = std::string(execDir) + "/logs";

    if (mkdir(logsDir.c_str(), 0700) != 0 && errno != EEXIST)
    {
      std::cerr << "Failed to create logs directory: " << logsDir << std::endl;
      return;
    }

    // Set log file path
    logFilePath = logsDir + "/zowed.log";

    // Open log file
    std::ios_base::openmode mode = std::ios::out;
    if (truncate)
    {
      mode |= std::ios::trunc;
    }
    else
    {
      mode |= std::ios::app;
    }

    if (logFile.is_open())
    {
      logFile.close();
    }

    // Create/open file with restricted permissions (0600)
    int fd = open(logFilePath.c_str(), O_WRONLY | O_CREAT | (truncate ? O_TRUNC : O_APPEND), 0600);
    if (fd == -1)
    {
      std::cerr << "Failed to create log file: " << logFilePath << std::endl;
      return;
    }
    close(fd);

    // Now open with fstream
    logFile.open(logFilePath.c_str(), mode);
    if (!logFile.is_open())
    {
      std::cerr << "Failed to initialize logger: could not open " << logFilePath << std::endl;
      return;
    }

    initialized = true;

    if (verbose)
    {
      log_debug("Verbose logging enabled");
    }
  }

  /**
   * Check if verbose logging is enabled
   */
  static bool is_verbose_logging()
  {
    return get_verbose_logging();
  }

  /**
   * Log a debug message (only if verbose logging is enabled)
   */
  static void log_debug(const char *format, ...)
  {
    bool &verboseLogging = get_verbose_logging();
    bool &initialized = get_initialized();

    if (!verboseLogging || !initialized)
    {
      return;
    }

    std::mutex &logMutex = get_log_mutex();
    std::ofstream &logFile = get_log_file();
    std::lock_guard<std::mutex> lock(logMutex);

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    logFile << get_current_timestamp() << " [DEBUG] " << buffer << std::endl;
    logFile.flush();

    check_and_truncate_log_file();
  }

  /**
   * Log an error message
   */
  static void log_error(const char *format, ...)
  {
    bool &initialized = get_initialized();

    if (!initialized)
    {
      return;
    }

    std::mutex &logMutex = get_log_mutex();
    std::ofstream &logFile = get_log_file();
    std::lock_guard<std::mutex> lock(logMutex);

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    logFile << get_current_timestamp() << " [ERROR] " << buffer << std::endl;
    logFile.flush();

    check_and_truncate_log_file();
  }

  /**
   * Log a fatal error and exit the program
   */
  static void log_fatal(const char *format, ...)
  {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    bool &initialized = get_initialized();

    if (initialized)
    {
      std::mutex &logMutex = get_log_mutex();
      std::ofstream &logFile = get_log_file();
      std::lock_guard<std::mutex> lock(logMutex);
      logFile << get_current_timestamp() << " [FATAL] " << buffer << std::endl;
      logFile.flush();
    }

    // Also print to stderr
    std::cerr << "FATAL: " << buffer << std::endl;

    exit(1);
  }

  /**
   * Log an info message
   */
  static void log_info(const char *format, ...)
  {
    bool &initialized = get_initialized();

    if (!initialized)
    {
      return;
    }

    std::mutex &logMutex = get_log_mutex();
    std::ofstream &logFile = get_log_file();
    std::lock_guard<std::mutex> lock(logMutex);

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    logFile << get_current_timestamp() << " [INFO] " << buffer << std::endl;
    logFile.flush();

    check_and_truncate_log_file();
  }

  /**
   * Cleanup and close the log file
   */
  static void shutdown()
  {
    std::mutex &logMutex = get_log_mutex();
    std::ofstream &logFile = get_log_file();
    bool &initialized = get_initialized();

    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open())
    {
      logFile.close();
    }
    initialized = false;
  }
};

} // namespace zowed

// Convenience macros for easier usage
#define LOG_DEBUG(...) zowed::Logger::log_debug(__VA_ARGS__)
#define LOG_ERROR(...) zowed::Logger::log_error(__VA_ARGS__)
#define LOG_FATAL(...) zowed::Logger::log_fatal(__VA_ARGS__)
#define LOG_INFO(...) zowed::Logger::log_info(__VA_ARGS__)

#endif // LOGGER_HPP
