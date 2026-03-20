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
           });
  describe("list-subsystems tests",
           [&]() -> void
           {
             it("should list subsystems",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system list-subsystems", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });
           });
  describe("list-proclib and validate content tests",
           [&]() -> void
           {
             it("should list proclib and validate content",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system list-proclib", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  Expect(response.length()).ToBeGreaterThan(0);
                  // Basic validation that we got some output
                  vector<string> lines = parse_rfc_response(response, "\n");
                  Expect(lines.size()).ToBeGreaterThan(0);
                });
           });
  describe("display-symbol tests",
           [&]() -> void
           {
             it("should display a system symbol",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system display-symbol YYMMDD", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  Expect(response).Not().ToContain("&");
                  Expect(response.length()).ToBeGreaterThan(0);
                });
           });
  describe("use 'lp' alias for list-proclib command tests",
           [&]() -> void
           {
             it("should use 'lp' alias for list-proclib command",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system lp", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });
           });
  describe("view syslog tests",
           [&]() -> void
           {
             it("should view syslog",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system view-syslog", response);
                  ExpectWithContext(rc, response).ToBe(0);
                  // split response into lines in an array
                  vector<string> lines = parse_rfc_response(response, "\n");
                  Expect(lines.size()).ToBeGreaterThanOrEqualTo(1);
                });

             it("should view syslog with max-lines",
                []()
                {
                  int rc = 0;
                  string response;
                  int max_lines = 3;

                  // set old date and time to orient to beginning of syslog
                  rc = execute_command_with_output(zowex_command + " system view-syslog --date 1990-01-01 --time 00:00:00.00 --max-lines " + to_string(max_lines), response);
                  ExpectWithContext(rc, response).ToBe(0);
                  // split response into lines in an array
                  std::cout << "response: " << response << std::endl;
                  vector<string> lines = parse_rfc_response(response, "\n");
                  Expect(lines.size() - 1).ToBe(max_lines); // -1 to exclude final `/n`
                });

             it("should fail with invalid max-lines",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system view-syslog --max-lines 0", response);
                  ExpectWithContext(rc, response).Not().ToBe(0);
                  rc = execute_command_with_output(zowex_command + " system view-syslog --max-lines 10001", response);
                  ExpectWithContext(rc, response).Not().ToBe(0);
                });
             it("should allow time of just hh:mm:ss",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system view-syslog --time 00:00:00", response);
                  ExpectWithContext(rc, response).ToBe(0);
                });
             it("should fail with invalid time",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system view-syslog --time 25:00:00.00", response);
                  ExpectWithContext(rc, response).Not().ToBe(0);
                  Expect(response).ToContain("Error: invalid time");
                });
             it("should fail with invalid date",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " system view-syslog --date 2026-13-01", response);
                  ExpectWithContext(rc, response).Not().ToBe(0);
                  Expect(response).ToContain("Error: invalid date");
                });
           });
}
