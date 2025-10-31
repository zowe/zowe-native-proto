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

/**
 * @file main.cpp
 * @brief Main entry point for the zowed binary
 *
 * This file contains the main() function that serves as the entry point
 * for the zowed executable. It delegates to the libzowed library for
 * the actual server implementation.
 *
 * This file is compiled with xlc++ while the library is compiled with xlclang++.
 */

#pragma runopts("TRAP(ON,NOSPIE)")
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#ifdef __MVS__
#define _OPEN_SYS_DLL_EXT
#include <dll.h>
#else
#include <dlfcn.h>
#endif
#include "zowed.hpp"

IoserverOptions parse_options(int argc, char *argv[])
{
  IoserverOptions opts;

  // Simple argument parsing without getopt_long
  for (int i = 1; i < argc; i++)
  {
    std::string arg = argv[i];

    if (arg == "-w" || arg == "--num-workers" || arg == "-num-workers")
    {
      if (i + 1 < argc)
      {
        opts.num_workers = atoi(argv[++i]);
        if (opts.num_workers <= 0)
        {
          std::cerr << "Number of workers must be greater than 0" << std::endl;
          exit(1);
        }
      }
      else
      {
        std::cerr << "Option " << arg << " requires an argument" << std::endl;
        exit(1);
      }
    }
    else if (arg == "-v" || arg == "--verbose" || arg == "-verbose")
    {
      opts.verbose = true;
    }
    else if (arg == "-h" || arg == "--help" || arg == "-help")
    {
      std::cout << "Usage: " << argv[0] << " [OPTIONS]\n"
                << "  -w, --num-workers NUM  Number of worker threads (default: 10)\n"
                << "  -v, --verbose          Enable verbose logging\n"
                << "  -h, --help             Show this help message\n";
      exit(0);
    }
    else
    {
      std::cerr << "Unknown option: " << arg << std::endl;
      exit(1);
    }
  }

  return opts;
}

#ifdef __MVS__
// z/OS-specific dynamic loading wrapper functions
static const char *zos_error_msg = NULL;

void *dlopen_zos(const char *filename, int flag)
{
  (void)flag; // Suppress unused parameter warning
  dllhandle *handle = dllload(filename);
  if (handle == NULL)
  {
    zos_error_msg = "Failed to load DLL on z/OS";
  }
  else
  {
    zos_error_msg = NULL;
  }
  return (void *)handle;
}

void *dlsym_zos(void *handle, const char *symbol)
{
  if (handle == NULL)
  {
    zos_error_msg = "Invalid handle for dlsym on z/OS";
    return NULL;
  }
  void *result = (void *)dllqueryfn((dllhandle *)handle, symbol);
  if (result == NULL)
  {
    zos_error_msg = "Symbol not found on z/OS";
  }
  else
  {
    zos_error_msg = NULL;
  }
  return result;
}

int dlclose_zos(void *handle)
{
  if (handle != NULL)
  {
    dllfree((dllhandle *)handle);
  }
  return 0;
}

const char *dlerror_zos()
{
  return zos_error_msg;
}

#define dlopen dlopen_zos
#define dlsym dlsym_zos
#define dlclose dlclose_zos
#define dlerror dlerror_zos
#define RTLD_LAZY 0
#endif

/**
 * @brief Main entry point for the zowed executable
 *
 * This function serves as the main entry point for the zowed binary.
 * It loads the libzowed.so shared library at runtime from the executable's
 * directory and delegates to it.
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return int Exit code (0 for success, non-zero for error)
 */
int main(int argc, char *argv[])
{
  IoserverOptions options = parse_options(argc, argv);

  // Load the shared library at runtime from executable directory
  std::string executable_dir;
  std::string full_path = argv[0];
  size_t last_slash = full_path.find_last_of('/');
  if (last_slash != std::string::npos)
  {
    executable_dir = full_path.substr(0, last_slash);
  }
  else
  {
    executable_dir = ".";
  }

  std::string lib_path = executable_dir + "/libzowed.so";
  void *handle = dlopen(lib_path.c_str(), RTLD_LAZY);
  if (handle == NULL)
  {
    const char *error_msg = dlerror();
    std::cerr << "Cannot load libzowed.so: " << (error_msg ? error_msg : "Unknown error") << std::endl;
    std::cerr << "Expected location: " << lib_path << std::endl;
    std::cerr << "Make sure libzowed.so is in the same directory as the zowed executable" << std::endl;
    return 1;
  }

  // Get the function pointer
  typedef int (*run_zowed_server_func)(const IoserverOptions &, const char *);
  void *func_ptr = dlsym(handle, "run_zowed_server");

  const char *dlsym_error = dlerror();
  if (dlsym_error || func_ptr == NULL)
  {
    std::cerr << "Cannot load symbol run_zowed_server: " << (dlsym_error ? dlsym_error : "Symbol not found") << std::endl;
    dlclose(handle);
    return 1;
  }

  run_zowed_server_func run_server = (run_zowed_server_func)func_ptr;

  // Call the function with executable directory
  int result = run_server(options, executable_dir.c_str());

  // Clean up
  dlclose(handle);
  return result;
}
