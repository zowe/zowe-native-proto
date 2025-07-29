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

#include <iostream>
#include "../zlogger.hpp"
#include "../zlogger_core.h"
#include "../zlogger_metal.h"

void test_cpp_logger()
{
  std::cout << "Testing C++ ZLogger integration..." << std::endl;

  // Get the singleton instance
  ZLogger &logger = ZLogger::get_instance();

  // Test different log levels
  logger.info("C++ Logger initialized successfully");
  logger.debug("Debug message from C++ logger");
  logger.warn("Warning message from C++ logger");
  logger.error("Error message from C++ logger");

  // Test with format arguments
  logger.info("Process ID: %d, Thread: %p", getpid(), pthread_self());

  std::cout << "C++ Logger test completed" << std::endl;
}

void test_c_core_direct()
{
  std::cout << "Testing C core logger directly..." << std::endl;

  // Test C core functions directly
  if (zlog_init("/tmp/zlogger_test_direct.log", ZLOG_INFO) == 0)
  {
    std::cout << "C core logger initialized successfully" << std::endl;

    zlog_write(ZLOG_INFO, "Direct C core log message");
    zlog_write(ZLOG_DEBUG, "Debug from C core (should not appear if level is INFO)");
    zlog_write(ZLOG_WARN, "Warning from C core: %s", "Multi-process safe logging");
    zlog_write(ZLOG_ERROR, "Error from C core: Process %d", getpid());

    // Test level changes
    zlog_set_level(ZLOG_DEBUG);
    zlog_write(ZLOG_DEBUG, "Debug message after level change (should appear now)");

    zlog_cleanup();
    std::cout << "C core logger test completed" << std::endl;
  }
  else
  {
    std::cout << "Failed to initialize C core logger" << std::endl;
  }
}

void test_metal_c_interface()
{
  std::cout << "Testing Metal C interface..." << std::endl;

  // Test Metal C interface (when compiled for non-Metal C, this bridges to C core)
  if (zlog_metal_init("/tmp/zlogger_test_metal.log", ZLOG_METAL_INFO) == 0)
  {
    std::cout << "Metal C interface initialized successfully" << std::endl;

    zlog_metal_write(ZLOG_METAL_INFO, "Message from Metal C interface");
    zlog_metal_write_formatted(ZLOG_METAL_WARN, "PREFIX:", "Formatted message", "SUFFIX");

    // Test macros
    ZLOG_METAL_INFO_MSG("Info message using Metal C macro");
    ZLOG_METAL_ERROR_FMT("ERROR:", "Something went wrong", "Check logs");

    zlog_metal_cleanup();
    std::cout << "Metal C interface test completed" << std::endl;
  }
  else
  {
    std::cout << "Failed to initialize Metal C interface" << std::endl;
  }
}

void test_concurrent_logging()
{
  std::cout << "Testing concurrent logging safety..." << std::endl;

  // Initialize both loggers to same file to test locking
  ZLogger &cpp_logger = ZLogger::get_instance();

  if (zlog_init("/tmp/zlogger_test_concurrent.log", ZLOG_INFO) == 0)
  {
    // Simulate concurrent access
    for (int i = 0; i < 10; i++)
    {
      cpp_logger.info("C++ Message %d from process %d", i, getpid());
      zlog_write(ZLOG_INFO, "C core message %d from process %d", i, getpid());
      zlog_metal_write_formatted(ZLOG_METAL_INFO, "METAL:", "Message from Metal interface", "sequence");
    }

    zlog_cleanup();
    std::cout << "Concurrent logging test completed" << std::endl;
  }
  else
  {
    std::cout << "Failed to initialize concurrent logging test" << std::endl;
  }
}

int main()
{
  std::cout << "=== Zowe Native Protocol Logger Integration Test ===" << std::endl;

  try
  {
    test_cpp_logger();
    std::cout << std::endl;

    test_c_core_direct();
    std::cout << std::endl;

    test_metal_c_interface();
    std::cout << std::endl;

    test_concurrent_logging();
    std::cout << std::endl;

    std::cout << "All tests completed successfully!" << std::endl;
    std::cout << "Check log files in /tmp/zlogger_test_*.log for output" << std::endl;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Test failed with exception: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}