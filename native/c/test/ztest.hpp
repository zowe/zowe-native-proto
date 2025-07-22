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
#include <fstream>
#include <unistd.h>
#include <cstdlib>
#include <regex>

// TODO(Kelosky): handle test not run
// TODO(Kelosky): handle running individual test and/or suite

#define Expect(x) [&]() -> RESULT_CHECK<typename std::remove_reference<decltype(x)>::type> { EXPECT_CONTEXT ctx = {__LINE__, __FILE__}; return expect(x, ctx); }()
#define ExpectWithContext(x, context) [&]() -> RESULT_CHECK<typename std::remove_reference<decltype(x)>::type> { EXPECT_CONTEXT ctx = {__LINE__, __FILE__, std::string(context), true}; return expect(x, ctx); }()

extern std::string matcher;

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

struct TEST_SUITE
{
  std::string description;
  std::vector<TEST_CASE> tests;
  int nesting_level;
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
    for (int i = 0; i < current_nesting * 2; i++)
    {
      std::cout << " ";
    }
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

  template <typename Callable,
            typename = typename std::enable_if<
                std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
  void run_test(std::string description, Callable test, TEST_OPTIONS &opts)
  {
    TEST_CASE tc = {0};
    tc.description = description;

    int current_nesting = get_nesting();

    if (matcher != "")
    {
      try
      {
        std::regex pattern(matcher);
        if (!std::regex_search(description, pattern))
        {
          return;
        }
      }
      catch (const std::regex_error &e)
      {
        // If regex is invalid, fall back to exact string match
        if (matcher != description)
        {
          return;
        }
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

typedef void (*cb)();

template <typename Callable,
          typename = typename std::enable_if<
              std::is_same<void, decltype(std::declval<Callable>()())>::value>::type>
void describe(std::string description, Callable suite)
{
  Globals &g = Globals::get_instance();
  TEST_SUITE ts;
  ts.description = description;
  ts.nesting_level = g.get_nesting();
  g.get_suites().push_back(ts);
  g.increment_suite_index();

  // Add newline only if we're not at the root level
  if (ts.nesting_level > 0)
  {
    std::cout << "\n";
  }
  std::cout << get_indent(ts.nesting_level) << description << std::endl;
  g.increment_nesting();
  suite();
  g.decrement_nesting();
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
            << colors.green << g.get_suites().size() - suite_fail << " passed" << colors.reset << ", "
            << colors.red << suite_fail << " failed" << colors.reset << ", "
            << g.get_suites().size() << " total" << std::endl;

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

inline int tests(ztst::cb tests)
{
  std::cout << "======== TESTS ========" << std::endl;
  tests();
  int rc = report();
  report_xml();
  return rc;
}

} // namespace ztst

#endif
