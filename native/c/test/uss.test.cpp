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
const string ussTestDir = "/tmp";

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
                             //  int rc = 0;
                             //  string uss_file = get_random_uss("/tmp/zowex_test");
                             //  string response;
                             //  string command = zowex_command + " uss create-file " + uss_file;
                             //  //  string command = zowex_command + " uss create-h";

                             //  rc = execute_command_with_output(command, response);
                             //  ExpectWithContext(rc, response).ToBe(0);
                             //  Expect(response).ToContain("USS file '" + uss_file + "' created");

                             //  command = zowex_command + " uss list " + uss_file + " -a --rfc";
                             //  rc = execute_command_with_output(command, response);
                             //  ExpectWithContext(rc, response).ToBe(0);

                             //  command = zowex_command + " uss delete " + uss_file;
                             //  rc = execute_command_with_output(command, response);
                             //  ExpectWithContext(rc, response).ToBe(0);
                             //  Expect(response).ToContain("Uss file '" + uss_file + "' deleted");
                           });
                      });
             describe("chmod",
                      []() -> void
                      {
                        it("should properly chmod a file",
                           []() -> void
                           {
                             int rc = 0;
                             string uss_file = get_random_uss(ussTestDir);
                             string response;
                             string command = zowex_command + " uss create-file " + uss_file;
                             string getPermissionsCommand = "ls -l " + uss_file;
                             string chmodFileCommand = zowex_command + " uss chmod 777 " + uss_file;

                             // Create file
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS file '" + uss_file + "' created");

                             // Get Default Perms
                             rc = execute_command_with_output(getPermissionsCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(("-rw-r--r--"));

                             // chmod 777 the test file and get permissions
                             rc = execute_command_with_output(chmodFileCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             rc = execute_command_with_output(getPermissionsCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(("-rwxrwxrwx"));
                           });
                        it("should properly chmod a directory",
                           []() -> void
                           {
                             int rc = 0;
                             string uss_dir = get_random_uss(ussTestDir) + "_dir";
                             string response;
                             string command = zowex_command + " uss create-dir " + uss_dir;
                             string getPermissionsCommand = "ls -ld " + uss_dir;
                             string chmodFileCommand = zowex_command + " uss chmod 777 " + uss_dir + " -r";

                             // Create directory
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS directory '" + uss_dir + "' created");

                             // Get default directory perms
                             rc = execute_command_with_output(getPermissionsCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(("drwxr-xr-x"));

                             // `chmod 777 -r` the test directory and get permissions
                             rc = execute_command_with_output(chmodFileCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             rc = execute_command_with_output(getPermissionsCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(("drwxrwxrwx"));
                           });
                        it("should properly handle calling chmod on a file that does not exist",
                           []() -> void
                           {
                             int rc = 0;
                             string uss_file = ussTestDir + "/test_does_not_exist";
                             string response;
                             string chmodFileCommand = zowex_command + " uss chmod 777 " + uss_file;

                             // chmod 777 the test file and get permissions
                             rc = execute_command_with_output(chmodFileCommand, response);
                             ExpectWithContext(rc, response).ToBe(1);
                             Expect(response).ToContain("Path `" + uss_file + "` does not exist");
                           });
                      });
           });
}