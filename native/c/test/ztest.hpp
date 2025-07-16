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
#include <stdexcept>
#include <vector>
#include <setjmp.h>
#include <signal.h>
#include <type_traits>

// TODO(Kelosky): handle test not run
// TODO(Kelosky): handle running individual test and/or suite

#define Expect(x) [&]() -> RESULT_CHECK { EXPECT_CONTEXT ctx = {__LINE__, __FILE__}; return expect(x, ctx); }()
#define ExpectWithContext(x, context) [&]() -> RESULT_CHECK { EXPECT_CONTEXT ctx = {__LINE__, __FILE__, std::string(context)}; return expect(x, ctx); }()

extern std::string matcher;

namespace ztst
{
  struct TEST_CASE
  {
    bool success;
    std::string description;
    std::string fail_message;
  };

  struct TEST_SUITE
  {
    std::string description;
    std::vector<TEST_CASE> tests;
  };

  extern std::vector<TEST_SUITE> ztst_suites;
  extern int ztst_suite_index;
  extern jmp_buf ztst_jmp_buf;

  void signal_handler(int code, siginfo_t *info, void *context);

  struct EXPECT_CONTEXT
  {
    int line_number;
    std::string file_name;
    std::string message;
    bool initialized;
  };

  class RESULT_CHECK
  {
    int int_result;
    std::string string_result;
    bool inverse;
    void *pointer_result;
    EXPECT_CONTEXT ctx;

  public:
    void ToBeGreaterThan(int);
    void ToBeGreaterThanOrEqualTo(int);
    void ToBe(int);
    void ToBe(std::string);
    void ToBeNull();
    std::string append_error_details();
    RESULT_CHECK Not();
    RESULT_CHECK() {}
    ~RESULT_CHECK() {}

    void set_inverse(bool v)
    {
      inverse = v;
    }

    void set_context(EXPECT_CONTEXT &c)
    {
      ctx = c;
      ctx.initialized = true;
    }

    void set_result(int r)
    {
      int_result = r;
    }

    void set_result(std::string r)
    {
      string_result = r;
    }

    void set_result(void *r)
    {
      pointer_result = r;
    }
  };

  struct TEST_OPTIONS
  {
    bool remove_signal_handling;
  };

  typedef void (*cb)();

  void describe(std::string description, cb suite);
  void it(std::string description, cb test);
  void it(std::string description, cb test, TEST_OPTIONS &opts);

  // Overloads for capturing lambdas
  template<typename Callable,
           typename = typename std::enable_if<
             std::is_same<void, decltype(std::declval<Callable>()())>::value
           >::type>
  void describe(std::string description, Callable suite)
  {
    TEST_SUITE ts = {0};
    ts.description = description;
    ztst_suites.push_back(ts);
    ztst_suite_index++;
    std::cout << description << std::endl;
    suite();
  }

  template<typename Callable,
           typename = typename std::enable_if<
             std::is_same<void, decltype(std::declval<Callable>()())>::value
           >::type>
  void it(std::string description, Callable test)
  {
    TEST_OPTIONS opts = {0};
    it(description, test, opts);
  }

  template<typename Callable,
           typename = typename std::enable_if<
             std::is_same<void, decltype(std::declval<Callable>()())>::value
           >::type>
  void it(std::string description, Callable test, TEST_OPTIONS &opts)
  {
    TEST_CASE tc = {0};
    tc.description = description;

    if (matcher != "" && matcher != description)
    {
      return;
    }

    bool abend = false;
    struct sigaction sa = {0};
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;

    if (!opts.remove_signal_handling)
    {
      sigaction(SIGABND, &sa, NULL);
      sigaction(SIGABRT, &sa, NULL);
      sigaction(SIGILL, &sa, NULL);
    }

    if (0 != setjmp(ztst_jmp_buf))
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

    if (!opts.remove_signal_handling)
    {
      sa.sa_flags = 0;
      sa.sa_handler = SIG_DFL;
      sigaction(SIGABND, &sa, NULL);
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
    std::cout << "  " << icon << tc.description << std::endl;
    if (!tc.success)
    {
      std::cout << "    " << tc.fail_message << std::endl;
    }

    ztst_suites[ztst_suite_index].tests.push_back(tc);
  }

  RESULT_CHECK expect(int val);
  RESULT_CHECK expect(int val, EXPECT_CONTEXT &ctx);
  RESULT_CHECK expect(std::string val);
  RESULT_CHECK expect(std::string val, EXPECT_CONTEXT &ctx);
  RESULT_CHECK expect(void *val);
  RESULT_CHECK expect(void *val, EXPECT_CONTEXT &ctx);

  int report();

  int tests(cb tests);
}

#endif
