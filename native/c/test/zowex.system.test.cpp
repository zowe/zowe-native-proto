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

#include <cstddef>
#include <ctime>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include "ztest.hpp"
#include "zutils.hpp"
#include "zowex.test.hpp"
#include "zowex.system.test.hpp"

using namespace std;
using namespace ztst;

void zowex_system_tests()
{
  describe("list-proclib tests",
           [&]() -> void
           {
             it("should list proclib",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system list-proclib", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should list proclib and validate content",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system list-proclib", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  if (rc == 0)
                  {
                    Expect(response.length()).ToBeGreaterThan(0);
                    // Basic validation that we got some output
                    vector<string> lines = parse_rfc_response(response, "\n");
                    Expect(lines.size()).ToBeGreaterThan(0);
                  }
                });

             it("should use 'lp' alias for list-proclib command",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system lp", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });
           });
}