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
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <iostream>

namespace zowed
{

/**
 * Simple logger class for zowed middleware
 * This is a header-only implementation that provides logging functionality
 * similar to the Go middleware's utils/log.go
 */
class Logger
{
private:
  // Maximum log file size before truncation (10MB)
  static constexpr size_t MAX_LOG_SIZE = 10 * 1024 * 1024;

  // Meyers Singleton pattern - thread-safe in C++11, no ODR violations
  static std::ofstream &getLogFile()
  {
    static std::ofstream logFile;
    return logFile;
  }

  static bool &getVerboseLogging()
  {
    static bool verboseLogging = false;
    return verboseLogging;
  }

  static std::mutex &getLogMutex()
  {
    static std::mutex logMutex;
    return logMutex;
  }

  static bool &getInitialized()
  {
    static bool initialized = false;
    return initialized;
  }

  static std::string &getLogFilePath()
  {
    static std::string logFilePath;
    return logFilePath;
  }

  /**
   * Get the current timestamp as a formatted string
   */
  static std::string getCurrentTimestamp()
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
  static void checkAndTruncateLogFile()
  {
    std::string &logFilePath = getLogFilePath();
    std::ofstream &logFile = getLogFile();
    bool &verboseLogging = getVerboseLogging();

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
        initLogger(true, verboseLogging);
        logFile << getCurrentTimestamp() << " [INFO] Log file truncated due to size limit\n";
        logFile.flush();
      }
    }
  }

public:
  /**
   * Initialize the logger with specified options
   * @param truncate Whether to truncate existing log file
   * @param verbose Whether to enable verbose logging
   */
  static void initLogger(bool truncate = false, bool verbose = false)
  {
    std::mutex &logMutex = getLogMutex();
    std::ofstream &logFile = getLogFile();
    bool &verboseLogging = getVerboseLogging();
    bool &initialized = getInitialized();
    std::string &logFilePath = getLogFilePath();

    std::lock_guard<std::mutex> lock(logMutex);

    verboseLogging = verbose;

    // Get executable directory
    char exePath[1024];
    ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (len == -1)
    {
      // Fallback for systems without /proc/self/exe (like z/OS)
      char *cwd = getcwd(nullptr, 0);
      if (cwd)
      {
        snprintf(exePath, sizeof(exePath), "%s", cwd);
        free(cwd);
      }
      else
      {
        snprintf(exePath, sizeof(exePath), ".");
      }
    }
    else
    {
      exePath[len] = '\0';
      // Get directory name from path
      char *dir = dirname(exePath);
      snprintf(exePath, sizeof(exePath), "%s", dir);
    }

    // Create logs directory
    std::string logsDir = std::string(exePath) + "/logs";
    mkdir(logsDir.c_str(), 0700);

    // Get executable name for log file
    char execName[256];
    char *programPath = strdup(getenv("_") ? getenv("_") : "zowed");
    char *baseName = basename(programPath);
    snprintf(execName, sizeof(execName), "%s", baseName);
    free(programPath);

    logFilePath = logsDir + "/" + std::string(execName) + ".log";

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

    logFile.open(logFilePath.c_str(), mode);
    if (!logFile.is_open())
    {
      std::cerr << "Failed to initialize logger: could not open " << logFilePath << std::endl;
      return;
    }

    initialized = true;

    if (verbose)
    {
      logDebug("Verbose logging enabled");
    }
  }

  /**
   * Check if verbose logging is enabled
   */
  static bool isVerboseLogging()
  {
    return getVerboseLogging();
  }

  /**
   * Log a debug message (only if verbose logging is enabled)
   */
  static void logDebug(const char *format, ...)
  {
    bool &verboseLogging = getVerboseLogging();
    bool &initialized = getInitialized();

    if (!verboseLogging || !initialized)
    {
      return;
    }

    std::mutex &logMutex = getLogMutex();
    std::ofstream &logFile = getLogFile();
    std::lock_guard<std::mutex> lock(logMutex);

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    logFile << getCurrentTimestamp() << " [DEBUG] " << buffer << std::endl;
    logFile.flush();

    checkAndTruncateLogFile();
  }

  /**
   * Log an error message
   */
  static void logError(const char *format, ...)
  {
    bool &initialized = getInitialized();

    if (!initialized)
    {
      return;
    }

    std::mutex &logMutex = getLogMutex();
    std::ofstream &logFile = getLogFile();
    std::lock_guard<std::mutex> lock(logMutex);

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    logFile << getCurrentTimestamp() << " [ERROR] " << buffer << std::endl;
    logFile.flush();

    checkAndTruncateLogFile();
  }

  /**
   * Log a fatal error and exit the program
   */
  static void logFatal(const char *format, ...)
  {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    bool &initialized = getInitialized();

    if (initialized)
    {
      std::mutex &logMutex = getLogMutex();
      std::ofstream &logFile = getLogFile();
      std::lock_guard<std::mutex> lock(logMutex);
      logFile << getCurrentTimestamp() << " [FATAL] " << buffer << std::endl;
      logFile.flush();
    }

    // Also print to stderr
    std::cerr << "FATAL: " << buffer << std::endl;

    exit(1);
  }

  /**
   * Log an info message
   */
  static void logInfo(const char *format, ...)
  {
    bool &initialized = getInitialized();

    if (!initialized)
    {
      return;
    }

    std::mutex &logMutex = getLogMutex();
    std::ofstream &logFile = getLogFile();
    std::lock_guard<std::mutex> lock(logMutex);

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    logFile << getCurrentTimestamp() << " [INFO] " << buffer << std::endl;
    logFile.flush();

    checkAndTruncateLogFile();
  }

  /**
   * Cleanup and close the log file
   */
  static void shutdown()
  {
    std::mutex &logMutex = getLogMutex();
    std::ofstream &logFile = getLogFile();
    bool &initialized = getInitialized();

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
#define LOG_DEBUG(...) zowed::Logger::logDebug(__VA_ARGS__)
#define LOG_ERROR(...) zowed::Logger::logError(__VA_ARGS__)
#define LOG_FATAL(...) zowed::Logger::logFatal(__VA_ARGS__)
#define LOG_INFO(...) zowed::Logger::logInfo(__VA_ARGS__)

#endif // LOGGER_HPP
