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

using namespace std;

string ztst::RESULT_CHECK::append_error_details()
{
  string error = "";
  if (ctx.initialized)
  {
    error += "\n    at " + ctx.file_name + ":" + to_string(ctx.line_number);
    if (ctx.message.size() > 0)
      error += " (" + ctx.message + ")";
  }
  return error;
}

// template<typename T>
void ztst::RESULT_CHECK::ToBe(int val)
{
  if (inverse)
  {
    if (int_result == val)
    {
      string error = "expected int '" + to_string(int_result) + "' NOT to be '" + to_string(val) + "'";
      error += append_error_details();
      throw runtime_error(error);
    }
  }
  else
  {
    if (int_result != val)
    {
      string error = "expected int '" + to_string(int_result) + "' to be '" + to_string(val) + "'";
      error += append_error_details();
      throw runtime_error(error);
    }
  }
}

void ztst::RESULT_CHECK::ToBeGreaterThan(int val)
{
  if (inverse)
  {
    if (int_result > val)
    {
      string error = "expected int '" + to_string(int_result) + "' to NOT to be greater than '" + to_string(val) + "'";
      error += append_error_details();
      throw runtime_error(error);
    }
  }
  else
  {
    if (int_result <= val)
    {
      string error = "expected int '" + to_string(int_result) + "' to be greater than '" + to_string(val) + "'";
      error += append_error_details();
      throw runtime_error(error);
    }
  }
}

// void ztst::RESULT_CHECK::ToBe(string val)
// {
//   if (!inverse)
//   {
//     if (string_result != val)
//     {
//       string error = "expected string '" + string_result + "' to be '" + val + "'";
//       error += append_error_details();
//       throw runtime_error(error);
//     }
//   }
//   else
//   {
//     if (string_result == val)
//     {
//       string error = "expected string '" + string_result + "' NOT to be '" + val + "'";
//       error += append_error_details();
//       throw runtime_error(error);
//     }
//   }
// }

void ztst::RESULT_CHECK::ToBeNull()
{
  if (!inverse)
  {
    if (NULL != pointer_result)
    {
      stringstream ss;
      ss << pointer_result;
      string error = "expected pointer '" + ss.str() + "' to be null";
      error += append_error_details();
      throw runtime_error(error);
    }
  }
  else
  {
    if (NULL == pointer_result)
    {
      stringstream ss;
      ss << pointer_result;
      string error = "expected '" + ss.str() + "' NOT to be null";
      error += append_error_details();
      throw runtime_error(error);
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

vector<ztst::TEST_SUITE> ztst::ztst_suites;
int ztst::ztst_suite_index = -1;
jmp_buf ztst::ztst_jmp_buf = {0};

void ztst::describe(std::string description, ztst::cb suite)
{
  TEST_SUITE ts = {0};
  ts.description = description;
  ztst::ztst_suites.push_back(ts);
  ztst::ztst_suite_index++;
  cout << description << endl;
  suite();
}

void ztst::signal_handler(int code, siginfo_t *info, void *context)
{
  longjmp(ztst::ztst_jmp_buf, 1);
}

int ztst::report()
{
  int suite_fail = 0;
  int tests_total = 0;
  int tests_fail = 0;

  cout << "======== TESTS SUMMARY ========" << endl;

  for (vector<TEST_SUITE>::iterator it = ztst::ztst_suites.begin(); it != ztst::ztst_suites.end(); it++)
  {
    bool suite_success = true;
    for (vector<TEST_CASE>::iterator iit = it->tests.begin(); iit != it->tests.end(); iit++)
    {
      tests_total++;
      if (!iit->success)
      {
        suite_success = false;
        tests_fail++;
      }
    }
    if (!suite_success)
    {
      suite_fail++;
    }
  }

  cout << "Total Suites: " << ztst::ztst_suites.size() - suite_fail << " passed, " << suite_fail << " failed, " << ztst::ztst_suites.size() << " total" << endl;
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
