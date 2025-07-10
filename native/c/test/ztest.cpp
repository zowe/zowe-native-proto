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
