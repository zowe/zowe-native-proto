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
#include <stdio.h>

using namespace std;
using namespace ztst;

const string zowex_command = "./../build-out/zowex";
const string ussTestDir = "/tmp";
void uss_tests()
{
  describe("uss tests",
           []() -> void
           {
             int rc;
             string response;
             beforeEach([&response, &rc]() -> void
                        { rc = 0; });

             describe("chmod",
                      [&response, &rc]() -> void
                      {
                        it("should properly chmod a file",
                           [&response, &rc]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir);

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
                           [&rc, &response]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir) + "_dir";
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
                           [&rc, &response]() -> void
                           {
                             string uss_file = ussTestDir + "/test_does_not_exist";
                             string response;
                             string chmodFileCommand = zowex_command + " uss chmod 777 " + uss_file;

                             // chmod 777 the test file and get permissions
                             rc = execute_command_with_output(chmodFileCommand, response);
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Path '" + uss_file + "' does not exist");
                           });
                      });
             describe("chown",
                      [&response, &rc]() -> void
                      {
                        beforeEach([&response, &rc]() -> void {});
                        it("should properly change the user on a file",
                           [&response, &rc]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir);
                             string command = zowex_command + " uss create-file " + uss_file;
                             string newUserChownCommand = zowex_command + " uss chown newUser " + uss_file;
                             string listUser = "ls -l " + uss_file;

                             // Create file
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS file '" + uss_file + "' created");

                             rc = execute_command_with_output(newUserChownCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS path '" + uss_file + "' owner changed to 'newUser'");

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             //  Expect(response).ToContain("newUser");
                           });
                        it("should properly change the group on a file",
                           [&response, &rc]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir);
                             string command = zowex_command + " uss create-file " + uss_file;
                             string newGroupChownCommand = zowex_command + " uss chown :newGroup " + uss_file;
                             string listUser = "ls -l " + uss_file;

                             // Create file
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS file '" + uss_file + "' created");

                             rc = execute_command_with_output(newGroupChownCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS path '" + uss_file + "' owner changed to ':newGroup'");

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             //  Expect(response).ToContain("newGroup");
                           });
                        it("should properly change the group and user on a file",
                           [&response, &rc]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir);
                             string command = zowex_command + " uss create-file " + uss_file;
                             string newUserNewGroupChownCommand = zowex_command + " uss chown newUser:newGroup " + uss_file;
                             string listUser = "ls -l " + uss_file;

                             // Create file
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS file '" + uss_file + "' created");

                             rc = execute_command_with_output(newUserNewGroupChownCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS path '" + uss_file + "' owner changed to 'newUser:newGroup'");

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             //  Expect(response).ToContain("newUser");
                             //  Expect(response).ToContain("newGroup");
                           });
                        it("should properly change the group and user on a directory",
                           [&response, &rc]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir) + "_dir";
                             string command = zowex_command + " uss create-dir " + uss_dir;
                             string newUserNewGroupChownCommand = zowex_command + " uss chown newUser:newGroup " + uss_dir + " -r";
                             string listUser = "ls -l " + uss_dir;

                             // Create file
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS directory '" + uss_dir + "' created");

                             rc = execute_command_with_output(newUserNewGroupChownCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS path '" + uss_dir + "' owner changed to 'newUser:newGroup'");

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             //  Expect(response).ToContain("newUser");
                             //  Expect(response).ToContain("newGroup");
                           });
                      });
             describe("chtag",
                      [&response, &rc]() -> void
                      {
                        it("should properly change the file tag on a file",
                           [&response, &rc]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir);
                             string command = zowex_command + " uss create-file " + uss_file;
                             string chtagCommand = zowex_command + " uss chtag " + uss_file + " IBM-1047";
                             string listUser = "ls -alT " + uss_file;

                             // Create file
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS file '" + uss_file + "' created");

                             rc = execute_command_with_output(chtagCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS path '" + uss_file + "' tag changed to 'IBM-1047'");

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("IBM-1047");
                           });
                        it("should properly change the file tag on a directory",
                           [&response, &rc]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir) + "_dir";
                             string command = zowex_command + " uss create-file " + uss_dir;
                             string chtagCommand = zowex_command + " uss chtag " + uss_dir + " IBM-1047 -r";
                             string listUser = "ls -alT " + uss_dir;

                             // Create file
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS file '" + uss_dir + "' created");

                             rc = execute_command_with_output(chtagCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS path '" + uss_dir + "' tag changed to 'IBM-1047'");

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("IBM-1047");
                           });
                        it("should properly handle setting an invalid file tag",
                           [&response, &rc]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir);
                             string command = zowex_command + " uss create-file " + uss_file;
                             string chtagCommand = zowex_command + " uss chtag " + uss_file + " bad-tag";
                             string listUser = "ls -alT " + uss_file;

                             // Create file
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS file '" + uss_file + "' created");

                             rc = execute_command_with_output(chtagCommand, response);
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Invalid tag 'bad-tag' - not a valid CCSID or display name");
                           });
                      });
             describe("create-dir",
                      [&response, &rc]() -> void
                      {
                        it("should properly create a directory with mode specified",
                           [&response, &rc]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir) + "_dir";
                             string command = zowex_command + " uss create-dir " + uss_dir + " --mode 777";
                             string newUserNewGroupChownCommand = zowex_command + " uss chown newUser:newGroup " + uss_dir + " -r";
                             string listUser = "ls -ld " + uss_dir;

                             // Create file
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS directory '" + uss_dir + "' created");

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(("drwxrwxrwx"));
                           });
                      });
           });
}