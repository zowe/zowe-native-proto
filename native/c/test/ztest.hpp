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

#ifndef ZTEST_HPP
#define ZTEST_HPP

#define _POSIX_SOURCE
#include <iostream>
#include <vector>
#include <setjmp.h>
#include <signal.h>
#include <type_traits>
#include <cxxabi.h>
#include <typeinfo>
#include <sstream>
#include <cstring> // Required for memset
#include <chrono>
#include <iomanip>

// TODO(Kelosky): handle test not run
// TODO(Kelosky): handle running individual test and/or suite

#define Expect(x) [&]() -> RESULT_CHECK<decltype(x)> { EXPECT_CONTEXT ctx = {__LINE__, __FILE__}; return expect(x, ctx); }()
#define ExpectWithContext(x, context) [&]() -> RESULT_CHECK<decltype(x)> { EXPECT_CONTEXT ctx = {__LINE__, __FILE__, std::string(context), true}; return expect(x, ctx); }()

extern std::string matcher;

namespace ztst
{

struct TEST_CASE
{
  bool success;
  std::string description;
  std::string fail_message;
  std::chrono::high_resolution_clock::time_point start_time;
  std::chrono::high_resolution_clock::time_point end_time;
};

struct TEST_SUITE
{
  std::string description;
  std::vector<TEST_CASE> tests;
};

struct EXPECT_CONTEXT
{
  int line_number;
  std::string file_name;
  std::string message;
  bool initialized;
};

class Globals
{

private:
  std::vector<TEST_SUITE> suites;
  int suite_index = -1;
  jmp_buf jump_buf = {0};

  Globals()
  {
  }
  ~Globals()
  {
  }
  Globals(const Globals &) = delete;
  Globals &operator=(const Globals &) = delete;

public:
  static Globals &get_instance()
  {
    static Globals instance;
    return instance;
  }

  std::vector<TEST_SUITE> &get_suites()
  {
    return suites;
  }
  int get_suite_index()
  {
    return suite_index;
  }
  void set_suite_index(int si)
  {
    suite_index = si;
  }
  void increment_suite_index()
  {
    set_suite_index(get_suite_index() + 1);
  }
  jmp_buf &get_jmp_buf()
  {
    return jump_buf;
  }
};

inline void signal_handler(int code, siginfo_t *info, void *context)
{
  Globals &g = Globals::get_instance();
  longjmp(g.get_jmp_buf(), 1);
}

template <class T>
class RESULT_CHECK
{
  T result;
  bool inverse;
  EXPECT_CONTEXT ctx;

public:
  void ToBe(T val)
  {
    int status;
    char *demangled = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
    std::string name = (status == 0) ? demangled : typeid(T).name();
    if (demangled)
      free(demangled);

    if (inverse)
    {
      if (result == val)
      {
        std::stringstream ss;
        ss << result;
        std::stringstream ss2;
        ss2 << val;
        std::string error = std::string("expected ") + name + " '" + ss.str() + "' NOT to be '" + ss2.str() + "'";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
    else
    {
      if (result != val)
      {
        std::stringstream ss;
        ss << result;
        std::stringstream ss2;
        ss2 << val;
        std::string error = std::string("expected ") + name + " '" + ss.str() + "' to be '" + ss2.str() + "'";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
  }

  template <typename U = T>
  typename std::enable_if<std::is_pointer<U>::value>::type
  ToBeNull()
  {
    if (!inverse)
    {
      if (NULL != result)
      {
        std::stringstream ss;
        ss << result;
        std::string error = "expected pointer '" + ss.str() + "' to be null";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
    else
    {
      if (NULL == result)
      {
        std::stringstream ss;
        ss << result;
        std::string error = "expected '" + ss.str() + "' NOT to be null";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
  }

  template <typename U = T>
  typename std::enable_if<std::is_arithmetic<U>::value>::type
  ToBeGreaterThan(U val)
  {
    if (inverse)
    {
      if (result > val)
      {
        int status;
        char *demangled = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
        std::string type_name = (status == 0) ? demangled : typeid(T).name();
        if (demangled)
          free(demangled);

        std::stringstream ss;
        ss << result;
        std::stringstream ss2;
        ss2 << val;
        std::string error = "expected " + type_name + " '" + ss.str() + "' to NOT to be greater than '" + ss2.str() + "'";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
    else
    {
      if (result <= val)
      {
        int status;
        char *demangled = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
        std::string type_name = (status == 0) ? demangled : typeid(T).name();
        if (demangled)
          free(demangled);

        std::stringstream ss;
        ss << result;
        std::stringstream ss2;
        ss2 << val;
        std::string error = "expected " + type_name + " '" + ss.str() + "' to be greater than '" + ss2.str() + "'";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
  }

  std::string append_error_details()
  {
    std::string error = "";
    if (ctx.initialized)
    {
      error += "\n    at " + ctx.file_name + ":" + std::to_string(ctx.line_number);
      if (ctx.message.size() > 0)
        error += " (" + ctx.message + ")";
    }
    return error;
  }

  RESULT_CHECK Not()
  {
    RESULT_CHECK copy;
    copy.set_inverse(true);
    copy.result = result;
    set_inverse(false);
    copy.set_context(ctx);
    return copy;
  }
  RESULT_CHECK()
  {
  }
  ~RESULT_CHECK()
  {
  }

  void set_inverse(bool v)
  {
    inverse = v;
  }

  void set_context(EXPECT_CONTEXT &c)
  {
    ctx = c;
  }

  void set_result(T r)
  {
    result = r;
  }
};

struct TEST_OPTIONS
{
  bool remove_signal_handling;
};

typedef void (*cb)();

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void describe(std::string description, Callable suite)
{
  Globals &g = Globals::get_instance();
  TEST_SUITE ts;
  ts.description = description;
  g.get_suites().push_back(ts);
  g.increment_suite_index();
  std::cout << description << std::endl;
  suite();
}

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void it(std::string description, Callable test)
{
  TEST_OPTIONS opts = {0};
  it(description, test, opts);
}

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void it(std::string description, Callable test, TEST_OPTIONS &opts)
{
  TEST_CASE tc = {0};
  tc.description = description;

  Globals &g = Globals::get_instance();

  if (matcher != "" && matcher != description)
  {
    return;
  }

  bool abend = false;
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = signal_handler;
  sa.sa_flags = SA_SIGINFO;

  if (!opts.remove_signal_handling)
  {
#if defined(__IBMCPP__) || defined(__IBMC__)
    sigaction(SIGABND, &sa, NULL);
#endif
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
  }

  tc.start_time = std::chrono::high_resolution_clock::now();

  if (0 != setjmp(g.get_jmp_buf()))
  {
    abend = true;
  }

  if (!abend)
  {
    try
    {
      test();
      tc.success = true;
    }
    catch (const std::exception &e)
    {
      tc.success = false;
      tc.fail_message = e.what();
    }
  }

  tc.end_time = std::chrono::high_resolution_clock::now();

  if (!opts.remove_signal_handling)
  {
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;
#if defined(__IBMCPP__) || defined(__IBMC__)
    sigaction(SIGABND, &sa, NULL);
#endif
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
  }

  std::string icon = tc.success ? "PASS  " : "FAIL  ";
  if (abend)
  {
    icon = "ABEND ";
    tc.success = false;
    tc.fail_message = "unexpected ABEND occured.  Add `TEST_OPTIONS.remove_signal_handling = false` to `it(...)` to capture abend dump";
  }

  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tc.end_time - tc.start_time);
  std::cout << "  " << icon << tc.description << " (" << duration.count() / 1000.0 << "ms)" << std::endl;
  if (!tc.success)
  {
    std::cout << "    " << tc.fail_message << std::endl;
  }

  g.get_suites()[g.get_suite_index()].tests.push_back(tc);
}

template <typename T>
RESULT_CHECK<T> expect(T val, EXPECT_CONTEXT ctx = {0})
{
  RESULT_CHECK<T> result;
  result.set_result(val);
  result.set_inverse(false);
  result.set_context(ctx);
  return result;
}

inline int report()
{
  int suite_fail = 0;
  int tests_total = 0;
  int tests_fail = 0;
  std::chrono::microseconds total_duration(0);

  Globals &g = Globals::get_instance();

  std::cout << "\n======== TESTS SUMMARY ========" << std::endl;

  for (std::vector<TEST_SUITE>::iterator it = g.get_suites().begin(); it != g.get_suites().end(); it++)
  {
    bool suite_success = true;
    for (std::vector<TEST_CASE>::iterator iit = it->tests.begin(); iit != it->tests.end(); iit++)
    {
      tests_total++;
      if (!iit->success)
      {
        suite_success = false;
        tests_fail++;
      }
      total_duration += std::chrono::duration_cast<std::chrono::microseconds>(iit->end_time - iit->start_time);
    }
    if (!suite_success)
    {
      suite_fail++;
    }
  }

  const int width = 13;
  std::cout << std::left << std::setw(width) << "Suites:"
            << g.get_suites().size() - suite_fail << " passed, "
            << suite_fail << " failed, "
            << g.get_suites().size() << " total" << std::endl;

  std::cout << std::left << std::setw(width) << "Tests:"
            << tests_total - tests_fail << " passed, "
            << tests_fail << " failed, "
            << tests_total << " total" << std::endl;

  std::cout << std::left << std::setw(width) << "Time:"
            << std::fixed << std::setprecision(3)
            << total_duration.count() / 1000.0 << "ms" << std::endl;

  return tests_fail > 0 ? 1 : 0;
}

inline int tests(ztst::cb tests)
{
  std::cout << "======== TESTS ========" << std::endl;
  tests();
  int rc = report();
  return rc;
}

} // namespace ztst

#endif