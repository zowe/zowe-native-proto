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

#include <string>
#include <vector>
#include <sstream>
#define _POSIX_SOURCE
#include <signal.h>
#include "ztest.hpp"
#include <setjmp.h>

void ztst::RESULT_CHECK::ToBe(int val)
{
  if (inverse)
  {
    if (int_result == val)
    {
      throw runtime_error("expected int '" + to_string(int_result) + "' NOT to be '" + to_string(val) + "'");
    }
  }
  else
  {
    if (int_result != val)
    {
      throw runtime_error("expected int '" + to_string(int_result) + "' to be '" + to_string(val) + "'");
    }
  }
}

void ztst::RESULT_CHECK::ToBe(string val)
{
  if (!inverse)
  {
    if (string_result != val)
    {
      throw runtime_error("expected string '" + string_result + "' to be '" + val + "'");
    }
  }
  else
  {
    if (string_result == val)
    {
      throw runtime_error("expected string '" + string_result + "' NOT to be '" + val + "'");
    }
  }
}

void ztst::RESULT_CHECK::ToBeNull()
{
  if (!inverse)
  {
    if (NULL != pointer_result)
    {
      stringstream ss;
      ss << pointer_result;
      throw runtime_error("expected pointer '" + ss.str() + "' to be null");
    }
  }
  else
  {
    if (NULL == pointer_result)
    {
      stringstream ss;
      ss << pointer_result;
      throw runtime_error("expected '" + ss.str() + "' NOT to be null");
    }
  }
}

ztst::RESULT_CHECK ztst::RESULT_CHECK::Not()
{
  RESULT_CHECK copy;
  copy.set_inverse(true);
  copy.pointer_result = pointer_result;
  copy.int_result = int_result;
  copy.string_result = string_result;
  set_inverse(false);
  return copy;
}

struct TEST_CASE
{
  bool success;
  string description;
  string fail_message;
};

struct TEST_SUITE
{
  string description;
  vector<TEST_CASE> tests;
};

vector<TEST_SUITE>
    ztst_suites;
int ztst_suite_index = -1;
jmp_buf ztst_jmp_buf = {0};

void ztst::describe(std::string description, ztst::cb suite)
{
  TEST_SUITE ts = {0};
  ts.description = description;
  ztst_suites.push_back(ts);
  ztst_suite_index++;
  cout << description << endl;
  suite();
}

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

  static void SIGHAND(int code, siginfo_t *info, void *context)
  {
    longjmp(ztst_jmp_buf, 1);
  }

#if defined(__cplusplus)
}
#endif

void ztst::it(string description, ztst::cb test)
{
  TEST_OPTIONS opts = {0};
  it(description, test, opts);
}

void ztst::it(string description, ztst::cb test, TEST_OPTIONS &opts)
{
  TEST_CASE tc = {0};
  tc.description = description;

  bool abend = false;
  struct sigaction sa = {0};
  sa.sa_sigaction = SIGHAND;
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
    catch (const exception &e)
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

  string icon = tc.success ? "PASS  " : "FAIL  ";
  if (abend)
  {
    icon = "ABEND ";
    tc.success = false;
    tc.fail_message = "unexpected ABEND occured.  Add `TEST_OPTIONS.remove_signal_handling = false` to `it(...)` to capture abend dump";
  }
  cout << "  " << icon << tc.description << endl;
  if (!tc.success)
  {
    cout << "    " << tc.fail_message << endl;
  }

  ztst_suites[ztst_suite_index].tests.push_back(tc);
}

ztst::RESULT_CHECK ztst::expect(int val)
{
  ztst::RESULT_CHECK result;
  result.set_result(val);
  result.set_inverse(false);
  return result;
}

ztst::RESULT_CHECK ztst::expect(string val)
{
  ztst::RESULT_CHECK result;
  result.set_result(val);
  result.set_inverse(false);
  return result;
}

ztst::RESULT_CHECK ztst::expect(void *val)
{
  ztst::RESULT_CHECK result;
  result.set_result(val);
  result.set_inverse(false);
  return result;
}

int ztst::report()
{
  int rc = 0;
  int suite_fail = 0;
  int tests_total = 0;
  int tests_fail = 0;

  cout << "======== TESTS SUMMARY ========" << endl;

  for (vector<TEST_SUITE>::iterator it = ztst_suites.begin(); it != ztst_suites.end(); it++)
  {
    for (vector<TEST_CASE>::iterator iit = it->tests.begin(); iit != it->tests.end(); iit++)
    {
      tests_total++;
      if (!iit->success)
      {
        suite_fail++;
        tests_fail++;
      }
    }
  }

  cout << "Total Suites: " << ztst_suites.size() - suite_fail << " passed, " << suite_fail << " failed, " << ztst_suites.size() << " total" << endl;
  cout << "Tests:      : " << tests_total - tests_fail << " passed, " << tests_fail << " failed, " << tests_total << " total" << endl;
  return tests_fail > 0 ? 1 : 0;
}

int ztst::tests(ztst::cb tests)
{
  cout << "======== TESTS ========" << endl;
  tests();
  int rc = ztst::report();
  return rc;
}
