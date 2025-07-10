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

// template<typename T>
// string ztst::RESULT_CHECK<T>::append_error_details()
// {
//   string error = "";
//   if (ctx.initialized)
//   {
//     error += "\n    at " + ctx.file_name + ":" + to_string(ctx.line_number);
//     if (ctx.message.size() > 0)
//       error += " (" + ctx.message + ")";
//   }
//   return error;
// }


// void ztst::RESULT_CHECK::ToBeGreaterThan(int val)
// {
//   if (inverse)
//   {
//     if (int_result > val)
//     {
//       string error = "expected int '" + to_string(int_result) + "' to NOT to be greater than '" + to_string(val) + "'";
//       error += append_error_details();
//       throw runtime_error(error);
//     }
//   }
//   else
//   {
//     if (int_result <= val)
//     {
//       string error = "expected int '" + to_string(int_result) + "' to be greater than '" + to_string(val) + "'";
//       error += append_error_details();
//       throw runtime_error(error);
//     }
//   }
// }

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

// void ztst::RESULT_CHECK::ToBeNull()
// {
//   if (!inverse)
//   {
//     if (NULL != pointer_result)
//     {
//       stringstream ss;
//       ss << pointer_result;
//       string error = "expected pointer '" + ss.str() + "' to be null";
//       error += append_error_details();
//       throw runtime_error(error);
//     }
//   }
//   else
//   {
//     if (NULL == pointer_result)
//     {
//       stringstream ss;
//       ss << pointer_result;
//       string error = "expected '" + ss.str() + "' NOT to be null";
//       error += append_error_details();
//       throw runtime_error(error);
//     }
//   }
// }

// ztst::RESULT_CHECK ztst::RESULT_CHECK::Not()
// {
//   RESULT_CHECK copy;
//   copy.set_inverse(true);
//   copy.pointer_result = pointer_result;
//   copy.int_result = int_result;
//   copy.string_result = string_result;
//   set_inverse(false);
//   return copy;
// }
