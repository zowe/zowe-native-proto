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

#include "ztest.hpp"
#include <cstddef>
#include <ctime>
#include <stdlib.h>
#include <string>
#include "zowex.test.hpp"
// #include "commands.hpp"

using namespace std;
using namespace ztst;

const string zowex_command = "./../build-out/zowex";

void uss_tests()
{
  describe("uss tests",
           []() -> void
           {
             describe("create",
                      []() -> void
                      {
                        it("should create a uss file",
                           []() -> void
                           {
                             int rc = 0;
                             string uss_file = get_random_uss("/tmp");
                             string response;
                             string command = zowex_command + " uss create-file " + uss_file;
                             //  string command = zowex_command + " uss create-h";

                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS file '" + uss_file + "' created");

                             //  command = zowex_command + " uss list " + uss_file + " -a --rfc";
                             //  rc = execute_command_with_output(command, response);
                             //  ExpectWithContext(rc, response).ToBe(0);

                             //  command = zowex_command + " uss delete " + uss_file;
                             //  rc = execute_command_with_output(command, response);
                             //  ExpectWithContext(rc, response).ToBe(0);
                             //  Expect(response).ToContain("Uss file '" + uss_file + "' deleted");
                           });

                        it("handle_uss_create_file1",
                           []() -> void
                           {
                             int test1 = 1 + 2;
                             int test2 = 3;
                             Expect(test1).ToBe(test2);
                           });
                      });
             describe("create",
                      []() -> void {});
           });
}