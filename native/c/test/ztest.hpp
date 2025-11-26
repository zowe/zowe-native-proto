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

#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif

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
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/select.h>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <regex>
#include <functional>

// TODO(Kelosky): handle test not run
// TODO(Kelosky): handle running individual test and/or suite

#define Expect(x) [&]() -> RESULT_CHECK<typename std::remove_reference<decltype(x)>::type> { EXPECT_CONTEXT ctx = {__LINE__, __FILE__}; return expect(x, ctx); }()
#define ExpectWithContext(x, context) [&]() -> RESULT_CHECK<typename std::remove_reference<decltype(x)>::type> { EXPECT_CONTEXT ctx = {__LINE__, __FILE__, std::string(context), true}; return expect(x, ctx); }()
#define TestLog(message) Globals::get_instance().test_log(message)
#define TrimChars(str) Globals::get_instance().trim_chars(str)

namespace ztst
{

// Forward declarations
class Globals;

struct TEST_OPTIONS
{
  bool remove_signal_handling;
};

inline std::string get_indent(int level)
{
  return std::string(level * 2, ' ');
}

// Forward declaration of signal handler
inline void signal_handler(int code, siginfo_t *info, void *context);

// ANSI color codes
struct Colors
{
  const char *green;
  const char *red;
  const char *yellow;
  const char *reset;
  const char *check; // Unicode check mark or ASCII alternative
  const char *cross; // Unicode X or ASCII alternative
  const char *warn;  // Unicode warning or ASCII alternative
  const char *arrow; // Unicode corner arrow or ASCII alternative

  Colors()
  {
    bool use_color = false;
    bool use_unicode = false;

    // Check if colors are explicitly forced on
    if (getenv("FORCE_COLOR") != nullptr)
    {
      use_color = true;
    }
    else
    {
      // Check if we're on z/OS - disable colors by default
#if defined(__MVS__) || defined(__NATIVE_EBCDIC__)
      use_color = false;
#else
      // Check if colors are explicitly disabled
      if (getenv("NO_COLOR") != nullptr)
      {
        use_color = false;
      }
      // Check if we're in a terminal that supports color
      else if (const char *term = getenv("TERM"))
      {
        use_color = isatty(STDOUT_FILENO) &&
                    (strstr(term, "xterm") != nullptr ||
                     strstr(term, "vt100") != nullptr ||
                     strstr(term, "ansi") != nullptr ||
                     strstr(term, "color") != nullptr ||
                     strstr(term, "linux") != nullptr);
      }
      // Check if we're in a CI environment that supports color
      else if (getenv("CI") != nullptr || getenv("GITHUB_ACTIONS") != nullptr)
      {
        use_color = true;
      }
#endif
    }

    // Check if we can use Unicode characters
    const char *lang = getenv("LANG");
    if (lang && (strstr(lang, "UTF-8") != nullptr || strstr(lang, "UTF8") != nullptr))
    {
      use_unicode = true;
    }

    if (use_color)
    {
      green = "\x1B[32m";
      red = "\x1B[31m";
      yellow = "\x1B[33m";
      reset = "\x1B[0m";
    }
    else
    {
      green = "";
      red = "";
      yellow = "";
      reset = "";
    }

    if (use_unicode)
    {
      check = "✓";
      cross = "✗";
      warn = "!";
      arrow = "└─";
    }
    else
    {
      check = "+";
      cross = "-";
      warn = "!";
      arrow = "|-";
    }
  }
};

static Colors colors;

struct TEST_CASE
{
  bool success;
  std::string description;
  std::string fail_message;
  std::chrono::high_resolution_clock::time_point start_time;
  std::chrono::high_resolution_clock::time_point end_time;
};

using hook_callback = std::function<void()>;
struct TEST_SUITE
{
  std::string description;
  std::vector<TEST_CASE> tests;
  int nesting_level;
  std::vector<hook_callback> before_all_hooks;
  std::vector<hook_callback> before_each_hooks;
  std::vector<hook_callback> after_each_hooks;
  std::vector<hook_callback> after_all_hooks;
  bool before_all_executed = false;
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
  int current_nesting = 0;
  jmp_buf jump_buf = {0};
  std::string matcher = "";
  std::vector<int> suite_stack;
  std::string znp_test_log = "";

  Globals()
  {
  }
  ~Globals()
  {
  }
  Globals(const Globals &) = delete;
  Globals &operator=(const Globals &) = delete;

  static void setup_signal_handlers(struct sigaction &sa)
  {
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;

#if defined(__IBMCPP__) || defined(__IBMC__)
    sigaction(SIGABND, &sa, NULL);
#endif
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
  }

  static void reset_signal_handlers(struct sigaction &sa)
  {
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;
#if defined(__IBMCPP__) || defined(__IBMC__)
    sigaction(SIGABND, &sa, NULL);
#endif
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
  }

  template <typename Callable>
  static void execute_test(Callable &test, TEST_CASE &tc, bool &abend)
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

  static std::pair<std::string, const char *> format_test_status(const TEST_CASE &tc, bool abend)
  {
    std::string icon;
    const char *color;

    if (tc.success)
    {
      icon = std::string(colors.check) + " PASS"; // Single space after status character
      color = colors.green;
    }
    else if (abend)
    {
      icon = std::string(colors.warn) + " ABEND"; // Single space after status character
      color = colors.yellow;
    }
    else
    {
      icon = std::string(colors.cross) + " FAIL"; // Single space after status character
      color = colors.red;
    }
    return std::make_pair(icon, color);
  }

  static void print_test_output(const TEST_CASE &tc, const std::string &description,
                                int current_nesting, const std::string &icon,
                                const char *color)
  {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        tc.end_time - tc.start_time);

    // Print exactly current_nesting * 2 spaces
    for (int i = 0; i < current_nesting * 2; i++)
    {
      std::cout << " ";
    }

    if (color[0] != '\0')
    { // Only use color if it's enabled
      std::cout << color << icon << colors.reset << " ";
    }
    else
    {
      std::cout << icon << " ";
    }
    std::cout << description;

    if (duration.count() >= 100)
    {
      std::cout << " (" << std::fixed << std::setprecision(3)
                << duration.count() / 1000.0 << "ms)";
    }
    std::cout << std::endl;

    if (!tc.success)
    {
      print_failure_details(tc, current_nesting, color);
    }
  }

  static void print_failure_details(const TEST_CASE &tc, int current_nesting,
                                    const char *color)
  {
    std::string main_error = tc.fail_message;
    std::string context = "";

    size_t pos = main_error.find("\n");
    if (pos != std::string::npos)
    {
      context = main_error.substr(pos);
      main_error = main_error.substr(0, pos);
    }

    // Print exactly current_nesting * 2 spaces
    Globals::get_instance().pad_nesting(current_nesting);
    std::cout << color << colors.arrow << colors.reset << " " << main_error << std::endl;

    if (!context.empty())
    {
      // Print exactly (current_nesting * 2 + 2) spaces for context
      for (int i = 0; i < current_nesting * 2 + 2; i++)
      {
        std::cout << " ";
      }
      std::cout << context.substr(1) << std::endl;
    }
  }

public:
  static Globals &get_instance()
  {
    static Globals instance;
    return instance;
  }

  std::string &get_matcher()
  {
    return matcher;
  }
  void set_matcher(const std::string &m)
  {
    matcher = m;
  }
  std::vector<TEST_SUITE> &get_suites()
  {
    return suites;
  }
  int get_suite_index()
  {
    return suite_index;
  }
  void push_suite_index(int si)
  {
    suite_stack.push_back(si);
    suite_index = si;
  }
  void pop_suite_index()
  {
    // Check if stack has elements before popping last suite index (otherwise pop_back is considered UB for empty vectors)
    if (!suite_stack.empty())
    {
      suite_stack.pop_back();
    }

    // After removing the last suite stack, re-assign the suite index to the next/parent suite if the stack still has suites
    if (!suite_stack.empty())
    {
      suite_index = suite_stack.back();
    }
  }
  jmp_buf &get_jmp_buf()
  {
    return jump_buf;
  }
  void increment_nesting()
  {
    current_nesting++;
  }
  void decrement_nesting()
  {
    if (current_nesting > 0)
      current_nesting--;
  }
  int get_nesting()
  {
    return current_nesting;
  }
  std::string &trim_chars(std::string &str, const std::string &chars = " \t\n\r\f\v")
  {
    str.erase(0, str.find_first_not_of(chars));
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
  }
  void pad_nesting(int level)
  {
    for (int i = 0; i < level * 2; i++)
    {
      std::cout << " ";
    }
  }
  void test_log(const std::string &message)
  {
    static bool show_test_log = false;
    if (znp_test_log.empty())
    {
      const char *debug = getenv("ZNP_TEST_LOG");
      znp_test_log = debug == nullptr || strstr(debug, "ON") != nullptr ? "ON" : "OFF";
      show_test_log = znp_test_log == "ON";
    }

    if (show_test_log)
    {
      pad_nesting(get_nesting());
      std::cout << "[TEST_INFO] " << message << std::endl;
    }
  }

  // Execute a vector of hooks, catching and reporting any errors
  void execute_hooks(const std::vector<hook_callback> &hooks, const std::string &hook_type, std::string &error_message)
  {
    for (const auto &hook : hooks)
    {
      try
      {
        hook();
      }
      catch (const std::exception &e)
      {
        error_message = hook_type + " hook failed: " + std::string(e.what());
        throw std::runtime_error(error_message);
      }
      catch (...)
      {
        error_message = hook_type + " hook failed with unknown error";
        throw std::runtime_error(error_message);
      }
    }
  }

  // Collect beforeEach hooks from current suite and all parent suites (parent -> child order)
  std::vector<hook_callback> collect_before_each_hooks()
  {
    std::vector<hook_callback> hooks;
    if (suite_stack.empty())
    {
      return hooks;
    }

    for (const auto &idx : suite_stack)
    {
      const auto &suite_hooks = suites[idx].before_each_hooks;
      hooks.insert(hooks.end(), suite_hooks.begin(), suite_hooks.end());
    }

    return hooks;
  }

  // Collect afterEach hooks from current suite and all parent suites (child -> parent order)
  std::vector<hook_callback> collect_after_each_hooks()
  {
    std::vector<hook_callback> hooks;
    if (suite_stack.empty())
    {
      return hooks;
    }

    for (auto it = suite_stack.rbegin(); it != suite_stack.rend(); ++it)
    {
      const auto &suite_hooks = suites[*it].after_each_hooks;
      hooks.insert(hooks.end(), suite_hooks.begin(), suite_hooks.end());
    }

    return hooks;
  }

  template <typename Callable,
            typename = typename std::enable_if<
                std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
  void run_test(const std::string &description, Callable test, TEST_OPTIONS &opts)
  {
    TEST_CASE tc = {0};
    tc.description = description;

    int current_nesting = get_nesting();

    if (get_matcher() != "")
    {
      try
      {
        std::regex pattern(get_matcher());
        if (!std::regex_search(description, pattern))
        {
          return;
        }
      }
      catch (const std::regex_error &e)
      {
        // If regex is invalid, fall back to exact string match
        if (get_matcher() != description)
        {
          return;
        }
      }
    }

    int suite_idx = get_suite_index();

    // Execute beforeAll hooks for the current suite stack (parent -> child order)
    bool before_all_failed = false;
    std::string before_all_error;
    for (const auto &idx : suite_stack)
    {
      if (idx < 0 || idx >= static_cast<int>(suites.size()))
      {
        continue;
      }

      TEST_SUITE &suite = suites[idx];
      if (!suite.before_all_executed && !suite.before_all_hooks.empty())
      {
        suite.before_all_executed = true;
        std::string error_message;
        try
        {
          execute_hooks(suite.before_all_hooks, "beforeAll", error_message);
        }
        catch (const std::exception &)
        {
          before_all_failed = true;
          before_all_error = error_message;
          break;
        }
      }
    }

    if (before_all_failed)
    {
      tc.success = false;
      tc.fail_message = before_all_error;
      auto status = format_test_status(tc, false);
      print_test_output(tc, description, current_nesting, status.first, status.second);
      if (suite_idx >= 0 && suite_idx < static_cast<int>(suites.size()))
      {
        suites[suite_idx].tests.push_back(tc);
      }
      return;
    }

    // Collect and execute beforeEach hooks
    std::vector<hook_callback> before_each_hooks = collect_before_each_hooks();
    if (!before_each_hooks.empty())
    {
      std::string error_message;
      try
      {
        execute_hooks(before_each_hooks, "beforeEach", error_message);
      }
      catch (const std::exception &)
      {
        tc.success = false;
        tc.fail_message = error_message;

        // Still run afterEach hooks even if beforeEach failed
        std::vector<hook_callback> after_each_hooks = collect_after_each_hooks();
        if (!after_each_hooks.empty())
        {
          std::string after_error;
          try
          {
            execute_hooks(after_each_hooks, "afterEach", after_error);
          }
          catch (const std::exception &)
          {
            // Append afterEach error to beforeEach error
            tc.fail_message += " (afterEach also failed: " + after_error + ")";
          }
        }

        auto status = format_test_status(tc, false);
        print_test_output(tc, description, current_nesting, status.first, status.second);
        get_suites()[suite_idx].tests.push_back(tc);
        return;
      }
    }

    bool abend = false;
    struct sigaction sa;

    if (!opts.remove_signal_handling)
    {
      setup_signal_handlers(sa);
    }

    tc.start_time = std::chrono::high_resolution_clock::now();

    if (0 != setjmp(get_jmp_buf()))
    {
      abend = true;
    }

    if (!abend)
    {
      execute_test(test, tc, abend);
    }

    tc.end_time = std::chrono::high_resolution_clock::now();

    if (!opts.remove_signal_handling)
    {
      reset_signal_handlers(sa);
    }

    if (abend)
    {
      tc.success = false;
      tc.fail_message = "unexpected ABEND occured. Add `TEST_OPTIONS.remove_signal_handling = false` to `it(...)` to capture abend dump";
    }

    // Execute afterEach hooks (always run, even if test failed)
    std::vector<hook_callback> after_each_hooks = collect_after_each_hooks();
    if (!after_each_hooks.empty())
    {
      std::string error_message;
      try
      {
        execute_hooks(after_each_hooks, "afterEach", error_message);
      }
      catch (const std::exception &)
      {
        // If test was successful but afterEach failed, mark test as failed
        if (tc.success)
        {
          tc.success = false;
          tc.fail_message = error_message;
        }
        else
        {
          // Test already failed, append afterEach error
          tc.fail_message += " (afterEach also failed: " + error_message + ")";
        }
      }
    }

    auto status = format_test_status(tc, abend);
    print_test_output(tc, description, current_nesting, status.first, status.second);

    get_suites()[get_suite_index()].tests.push_back(tc);
  }
};

// Signal handler needs to be after Globals class definition
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

  template <typename U = T>
  typename std::enable_if<std::is_arithmetic<U>::value>::type
  ToBeGreaterThanOrEqualTo(U val)
  {
    if (inverse)
    {
      if (result >= val)
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
        std::string error = "expected " + type_name + " '" + ss.str() + "' to NOT to be greater than or equal to '" + ss2.str() + "'";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
    else
    {
      if (result < val)
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
        std::string error = "expected " + type_name + " '" + ss.str() + "' to be greater than or equal to '" + ss2.str() + "'";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
  }

  template <typename U = T>
  typename std::enable_if<std::is_arithmetic<U>::value>::type
  ToBeLessThan(U val)
  {
    if (inverse)
    {
      if (result < val)
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
        std::string error = "expected " + type_name + " '" + ss.str() + "' to NOT be less than '" + ss2.str() + "'";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
    else
    {
      if (result >= val)
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
        std::string error = "expected " + type_name + " '" + ss.str() + "' to be less than '" + ss2.str() + "'";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
  }

  template <typename U = T>
  typename std::enable_if<std::is_same<U, std::string>::value>::type
  ToContain(const std::string &substring)
  {
    if (inverse)
    {
      if (result.find(substring) != std::string::npos)
      {
        std::string error = "expected string '" + result + "' to NOT contain '" + substring + "'";
        error += append_error_details();
        throw std::runtime_error(error);
      }
    }
    else
    {
      if (result.find(substring) == std::string::npos)
      {
        std::string error = "expected string '" + result + "' to contain '" + substring + "'";
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
      error = "\n" + std::string(2, ' ') + "at " + ctx.file_name + ":" + std::to_string(ctx.line_number);
      if (ctx.message.size() > 0)
      {
        error += " (" + TrimChars(ctx.message) + ")";
      }
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

typedef void (*cb)();

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void describe(const std::string &description, Callable suite)
{
  Globals &g = Globals::get_instance();
  TEST_SUITE ts;
  ts.description = description;
  ts.nesting_level = g.get_nesting();
  g.get_suites().push_back(ts);
  int current_suite_idx = static_cast<int>(g.get_suites().size()) - 1;
  g.push_suite_index(current_suite_idx);

  std::cout << get_indent(ts.nesting_level) << description << std::endl;
  g.increment_nesting();

  auto cleanup = [&]()
  {
    g.decrement_nesting();
    g.pop_suite_index();
  };

  // Execute suite (this is where beforeAll/beforeEach/afterEach/afterAll/it calls happen)
  try
  {
    suite();
  }
  catch (...)
  {
    cleanup();
    throw;
  }

  // Execute afterAll hooks for this suite
  if (current_suite_idx >= 0 && current_suite_idx < static_cast<int>(g.get_suites().size()))
  {
    TEST_SUITE &current_suite = g.get_suites()[current_suite_idx];
    const std::vector<hook_callback> &after_all_hooks = current_suite.after_all_hooks;
    if (!after_all_hooks.empty())
    {
      std::string error_message;
      try
      {
        g.execute_hooks(after_all_hooks, "afterAll", error_message);
      }
      catch (const std::exception &)
      {
        // Report afterAll hook failure
        g.pad_nesting(g.get_nesting());
        std::cout << colors.red << colors.cross << " " << error_message << colors.reset << std::endl;
      }
    }
    current_suite.before_all_hooks.clear();
    current_suite.before_each_hooks.clear();
    current_suite.after_each_hooks.clear();
    current_suite.after_all_hooks.clear();
    current_suite.before_all_executed = false;
  }

  cleanup();
}

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void xit(const std::string &description, Callable)
{
  Globals &g = Globals::get_instance();
  std::cout << get_indent(g.get_nesting()) << "/ SKIP " << description << std::endl;
}

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void xdescribe(const std::string &description, Callable)
{
  Globals &g = Globals::get_instance();
  std::cout << get_indent(g.get_nesting()) << "/ SKIP " << description << std::endl;
}

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void it(const std::string &description, Callable test)
{
  TEST_OPTIONS opts = {0};
  it(description, test, opts);
}

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void it(const std::string &description, Callable test, TEST_OPTIONS &opts)
{
  Globals::get_instance().run_test(description, test, opts);
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

// Hook registration functions
template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void beforeAll(Callable hook)
{
  Globals &g = Globals::get_instance();
  int suite_idx = g.get_suite_index();
  if (suite_idx >= 0 && suite_idx < static_cast<int>(g.get_suites().size()))
  {
    g.get_suites()[suite_idx].before_all_hooks.push_back(hook);
  }
}

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void beforeEach(Callable hook)
{
  Globals &g = Globals::get_instance();
  int suite_idx = g.get_suite_index();
  if (suite_idx >= 0 && suite_idx < static_cast<int>(g.get_suites().size()))
  {
    g.get_suites()[suite_idx].before_each_hooks.push_back(hook);
  }
}

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void afterEach(Callable hook)
{
  Globals &g = Globals::get_instance();
  int suite_idx = g.get_suite_index();
  if (suite_idx >= 0 && suite_idx < static_cast<int>(g.get_suites().size()))
  {
    g.get_suites()[suite_idx].after_each_hooks.push_back(hook);
  }
}

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void afterAll(Callable hook)
{
  Globals &g = Globals::get_instance();
  int suite_idx = g.get_suite_index();
  if (suite_idx >= 0 && suite_idx < static_cast<int>(g.get_suites().size()))
  {
    g.get_suites()[suite_idx].after_all_hooks.push_back(hook);
  }
}

inline void print_failed_tests()
{
  Globals &g = Globals::get_instance();
  bool has_failures = false;

  // First pass: check if there are any failures
  for (std::vector<TEST_SUITE>::iterator it = g.get_suites().begin(); it != g.get_suites().end(); it++)
  {
    for (std::vector<TEST_CASE>::iterator iit = it->tests.begin(); iit != it->tests.end(); iit++)
    {
      if (!iit->success)
      {
        has_failures = true;
        break;
      }
    }
    if (has_failures)
      break;
  }

  if (!has_failures)
    return;

  std::cout << "\n======== FAILED TESTS ========" << std::endl;

  // Build a map of suite paths for proper nesting display
  std::vector<std::string> suite_paths;
  std::vector<int> suite_nesting_levels;

  for (std::vector<TEST_SUITE>::iterator it = g.get_suites().begin(); it != g.get_suites().end(); it++)
  {
    bool suite_has_failures = false;

    // Check if this suite has any failures
    for (std::vector<TEST_CASE>::iterator iit = it->tests.begin(); iit != it->tests.end(); iit++)
    {
      if (!iit->success)
      {
        suite_has_failures = true;
        break;
      }
    }

    if (suite_has_failures)
    {
      // Build the complete path for this suite
      std::string full_path = it->description;
      int current_nesting = it->nesting_level;

      // Find the most recent parent suite by looking at nesting levels
      // We need to find the last suite that has a lower nesting level
      for (int i = suite_paths.size() - 1; i >= 0; i--)
      {
        if (suite_nesting_levels[i] < current_nesting)
        {
          full_path = suite_paths[i] + " > " + it->description;
          break;
        }
      }

      std::cout << colors.red << colors.cross << " FAIL " << full_path << colors.reset << std::endl;

      for (std::vector<TEST_CASE>::iterator iit = it->tests.begin(); iit != it->tests.end(); iit++)
      {
        if (!iit->success)
        {
          std::cout << "  " << colors.red << colors.cross << " FAIL " << iit->description << colors.reset << std::endl;
          if (!iit->fail_message.empty())
          {
            std::cout << "    " << colors.arrow << " " << iit->fail_message << std::endl;
          }
        }
      }

      // Store this suite for future path building
      suite_paths.push_back(full_path);
      suite_nesting_levels.push_back(current_nesting);
    }
    else
    {
      // Even if this suite doesn't have failures, we need to track it for path building
      // This is important for nested suites that don't have failures themselves
      std::string full_path = it->description;
      int current_nesting = it->nesting_level;

      // Find the most recent parent suite
      for (int i = suite_paths.size() - 1; i >= 0; i--)
      {
        if (suite_nesting_levels[i] < current_nesting)
        {
          full_path = suite_paths[i] + " > " + it->description;
          break;
        }
      }

      suite_paths.push_back(full_path);
      suite_nesting_levels.push_back(current_nesting);
    }
  }
}

inline int report()
{
  int suite_fail = 0;
  int suite_total = 0;
  int tests_total = 0;
  int tests_fail = 0;
  std::chrono::microseconds total_duration(0);

  Globals &g = Globals::get_instance();

  // Print failed tests summary before the main summary
  print_failed_tests();

  std::cout << "\n======== TESTS SUMMARY ========" << std::endl;

  for (std::vector<TEST_SUITE>::iterator it = g.get_suites().begin(); it != g.get_suites().end(); it++)
  {
    bool suite_success = true;
    bool suite_ran = false;
    for (std::vector<TEST_CASE>::iterator iit = it->tests.begin(); iit != it->tests.end(); iit++)
    {
      suite_ran = true;
      tests_total++;
      if (!iit->success)
      {
        suite_success = false;
        tests_fail++;
      }
      total_duration += std::chrono::duration_cast<std::chrono::microseconds>(iit->end_time - iit->start_time);
    }
    if (suite_ran)
    {
      suite_total++;
    }
    if (!suite_success)
    {
      suite_fail++;
    }
  }

  const int width = 13;
  std::cout << std::left << std::setw(width) << "Suites:"
            << colors.green << suite_total - suite_fail << " passed" << colors.reset << ", "
            << colors.red << suite_fail << " failed" << colors.reset << ", "
            << suite_total << " total" << std::endl;

  std::cout << std::left << std::setw(width) << "Tests:"
            << colors.green << tests_total - tests_fail << " passed" << colors.reset << ", "
            << colors.red << tests_fail << " failed" << colors.reset << ", "
            << tests_total << " total" << std::endl;

  std::cout << std::left << std::setw(width) << "Time:"
            << std::fixed << std::setprecision(3)
            << total_duration.count() / 1000.0 << "ms" << std::endl;

  return tests_fail > 0 ? 1 : 0;
}

inline void escape_xml(std::string &data)
{
  std::string::size_type pos = 0;
  for (;;)
  {
    pos = data.find_first_of("\"&'<>", pos);
    if (pos == std::string::npos)
      break;

    std::string replacement;
    switch (data[pos])
    {
    case '\"':
      replacement = "&quot;";
      break;
    case '&':
      replacement = "&amp;";
      break;
    case '\'':
      replacement = "&apos;";
      break;
    case '<':
      replacement = "&lt;";
      break;
    case '>':
      replacement = "&gt;";
      break;
    default:
      break;
    }

    data.replace(pos, 1, replacement);
    pos += replacement.length();
  }
}

inline void report_xml(const std::string &filename = "test-results.xml")
{
  Globals &g = Globals::get_instance();
  std::ofstream xml_file(filename);

  int total_tests = 0;
  int total_failures = 0;
  std::chrono::microseconds total_time(0);

  // Count totals first
  for (const auto &suite : g.get_suites())
  {
    for (const auto &test : suite.tests)
    {
      total_tests++;
      if (!test.success)
        total_failures++;
      total_time += std::chrono::duration_cast<std::chrono::microseconds>(
          test.end_time - test.start_time);
    }
  }

  // XML Header
  xml_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  xml_file << "<testsuites tests=\"" << total_tests
           << "\" failures=\"" << total_failures
           << "\" time=\"" << std::fixed << std::setprecision(3)
           << total_time.count() / 1000000.0 << "\">\n";

  // Process each test suite
  for (const auto &suite : g.get_suites())
  {
    int suite_tests = 0;
    int suite_failures = 0;
    std::chrono::microseconds suite_time(0);

    // Count suite totals
    for (const auto &test : suite.tests)
    {
      suite_tests++;
      if (!test.success)
        suite_failures++;
      suite_time += std::chrono::duration_cast<std::chrono::microseconds>(
          test.end_time - test.start_time);
    }

    std::string suite_name = suite.description;
    escape_xml(suite_name);

    // Write suite header
    xml_file << "  <testsuite name=\"" << suite_name
             << "\" tests=\"" << suite_tests
             << "\" failures=\"" << suite_failures
             << "\" time=\"" << std::fixed << std::setprecision(3)
             << suite_time.count() / 1000000.0 << "\">\n";

    // Process each test case
    for (const auto &test : suite.tests)
    {
      std::string test_name = test.description;
      std::string failure_message = test.fail_message;
      escape_xml(test_name);
      escape_xml(failure_message);

      auto test_time = std::chrono::duration_cast<std::chrono::microseconds>(
          test.end_time - test.start_time);

      xml_file << "    <testcase name=\"" << test_name
               << "\" time=\"" << std::fixed << std::setprecision(3)
               << test_time.count() / 1000000.0 << "\"";

      if (!test.success)
      {
        xml_file << ">\n";
        xml_file << "      <failure message=\"" << failure_message << "\"/>\n";
        xml_file << "    </testcase>\n";
      }
      else
      {
        xml_file << "/>\n";
      }
    }

    xml_file << "  </testsuite>\n";
  }

  xml_file << "</testsuites>\n";
  xml_file.close();
}

/**
 * Execute a command and capture stdout and stderr separately
 * @param command The command string to execute
 * @param stdout_output Reference to string that will contain stdout
 * @param stderr_output Reference to string that will contain stderr
 * @return Exit code of the executed command
 */
inline int execute_command(const std::string &command, std::string &stdout_output, std::string &stderr_output)
{
  stdout_output = "";
  stderr_output = "";

  // Create pipes for stdout and stderr
  int stdout_pipe[2], stderr_pipe[2];
  if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1)
  {
    throw std::runtime_error("Failed to create pipes");
  }

  // Fork the process
  pid_t pid = fork();
  if (pid == -1)
  {
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    close(stderr_pipe[0]);
    close(stderr_pipe[1]);
    throw std::runtime_error("Failed to fork process");
  }

  if (pid == 0)
  {
    // Child process
    close(stdout_pipe[0]); // Close read ends
    close(stderr_pipe[0]);

    // Redirect stdout and stderr to respective pipes
    dup2(stdout_pipe[1], STDOUT_FILENO);
    dup2(stderr_pipe[1], STDERR_FILENO);
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    // Execute command via shell
    execl("/bin/sh", "sh", "-c", command.c_str(), (char *)NULL);
    _exit(127); // If execl fails
  }
  else
  {
    // Parent process
    close(stdout_pipe[1]); // Close write ends
    close(stderr_pipe[1]);

    // Set pipes to non-blocking
    fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(stderr_pipe[0], F_SETFL, O_NONBLOCK);

    // Read from both pipes using select
    fd_set read_fds;
    char buffer[256];
    ssize_t bytes_read;
    bool stdout_open = true, stderr_open = true;

    while (stdout_open || stderr_open)
    {
      FD_ZERO(&read_fds);
      int max_fd = 0;

      if (stdout_open)
      {
        FD_SET(stdout_pipe[0], &read_fds);
        max_fd = std::max(max_fd, stdout_pipe[0]);
      }
      if (stderr_open)
      {
        FD_SET(stderr_pipe[0], &read_fds);
        max_fd = std::max(max_fd, stderr_pipe[0]);
      }

      struct timeval timeout = {1, 0}; // 1 second timeout
      int select_result = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

      if (select_result > 0)
      {
        // Check stdout
        if (stdout_open && FD_ISSET(stdout_pipe[0], &read_fds))
        {
          bytes_read = read(stdout_pipe[0], buffer, sizeof(buffer) - 1);
          if (bytes_read > 0)
          {
            buffer[bytes_read] = '\0';
            stdout_output += buffer;
          }
          else if (bytes_read == 0)
          {
            stdout_open = false;
          }
        }

        // Check stderr
        if (stderr_open && FD_ISSET(stderr_pipe[0], &read_fds))
        {
          bytes_read = read(stderr_pipe[0], buffer, sizeof(buffer) - 1);
          if (bytes_read > 0)
          {
            buffer[bytes_read] = '\0';
            stderr_output += buffer;
          }
          else if (bytes_read == 0)
          {
            stderr_open = false;
          }
        }
      }
      else if (select_result == 0)
      {
        // Timeout - check if child is still running
        int status;
        if (waitpid(pid, &status, WNOHANG) != 0)
        {
          // Child has exited, do final reads
          break;
        }
      }
      else
      {
        // Error in select
        break;
      }
    }

    // Final non-blocking reads to get any remaining data
    while ((bytes_read = read(stdout_pipe[0], buffer, sizeof(buffer) - 1)) > 0)
    {
      buffer[bytes_read] = '\0';
      stdout_output += buffer;
    }
    while ((bytes_read = read(stderr_pipe[0], buffer, sizeof(buffer) - 1)) > 0)
    {
      buffer[bytes_read] = '\0';
      stderr_output += buffer;
    }

    close(stdout_pipe[0]);
    close(stderr_pipe[0]);

    // Wait for child process to complete
    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
      throw std::runtime_error("Failed to wait for child process");
    }

    // Return exit code
    if (WIFEXITED(status))
    {
      return WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
      return 128 + WTERMSIG(status);
    }
    else
    {
      return -1;
    }
  }

  // Should never reach here, but add return to satisfy compiler
  return -1;
}

inline int tests(int argc, char *argv[], ztst::cb tests)
{
  std::cout << "======== TESTS ========" << std::endl;

  if (argc > 1)
  {
    std::cout << "Running tests matching: " << argv[1] << std::endl;
    Globals::get_instance().set_matcher(argv[1]);
  }
  else
  {
    std::cout << "Running all tests" << std::endl;
  }

  tests();
  int rc = report();
  report_xml();
  return rc;
}

} // namespace ztst

#endif
