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
const string ussTestDir = "/tmp/zowex-uss";
void uss_tests()
{
  describe("uss tests",
           []() -> void
           {
             int rc;
             string response;
             beforeAll([&response]() -> void
                       { execute_command_with_output(zowex_command + " uss create-dir " + ussTestDir + " --mode 777", response); });
             afterEach([&rc]() -> void
                       { rc = 0; });
             afterAll([&response]() -> void
                      { execute_command_with_output(zowex_command + " uss delete " + ussTestDir + " -r", response); });

             auto create_test_file_cmd = [&](const string &uss_file, const string &options = "") -> void
             {
               string command = zowex_command + " uss create-file " + uss_file + " " + options;
               rc = execute_command_with_output(command, response);
               ExpectWithContext(rc, response).ToBe(0);
               Expect(response).ToContain("USS file '" + uss_file + "' created");
             };

             auto create_test_dir_cmd = [&](const string &uss_dir, const string &options = "") -> void
             {
               string command = zowex_command + " uss create-dir " + uss_dir + " " + options;
               rc = execute_command_with_output(command, response);
               ExpectWithContext(rc, response).ToBe(0);
               Expect(response).ToContain("USS directory '" + uss_dir + "' created");
             };

             describe("chmod",
                      [&]() -> void
                      {
                        string uss_path;

                        describe("on a file",
                                 [&]() -> void
                                 {
                                   beforeEach([&]() -> void
                                              {
                                                uss_path = get_random_uss(ussTestDir);
                                                create_test_file_cmd(uss_path); });

                                   it("should properly chmod a file",
                                      [&]() -> void
                                      {
                                        string getPermissionsCommand = "ls -l " + uss_path;
                                        string chmodFileCommand = zowex_command + " uss chmod 777 " + uss_path;

                                        rc = execute_command_with_output(getPermissionsCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain(("-rw-r--r--"));

                                        rc = execute_command_with_output(chmodFileCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        rc = execute_command_with_output(getPermissionsCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain(("-rwxrwxrwx"));
                                      });
                                 });

                        describe("on a directory",
                                 [&]() -> void
                                 {
                                   beforeEach([&]() -> void
                                              {
                                                uss_path = get_random_uss(ussTestDir) + "_dir";
                                                create_test_dir_cmd(uss_path); });

                                   it("should properly chmod a directory",
                                      [&]() -> void
                                      {
                                        string getPermissionsCommand = "ls -ld " + uss_path;
                                        string chmodFileCommand = zowex_command + " uss chmod 777 " + uss_path + " -r";

                                        rc = execute_command_with_output(getPermissionsCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain(("drwxr-xr-x"));

                                        rc = execute_command_with_output(chmodFileCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        rc = execute_command_with_output(getPermissionsCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain(("drwxrwxrwx"));
                                      });
                                 });

                        it("should properly handle calling chmod on a file that does not exist",
                           [&]() -> void
                           {
                             string uss_file = ussTestDir + "/test_does_not_exist";
                             string response;
                             string chmodFileCommand = zowex_command + " uss chmod 777 " + uss_file;

                             rc = execute_command_with_output(chmodFileCommand, response);
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Path '" + uss_file + "' does not exist");
                           });
                      });
             describe("chown",
                      [&]() -> void
                      {
                        string uss_path;

                        describe("on a file",
                                 [&]() -> void
                                 {
                                   beforeEach([&]() -> void
                                              {
                                                uss_path = get_random_uss(ussTestDir);
                                                create_test_file_cmd(uss_path); });

                                   it("should properly change the user on a file",
                                      [&]() -> void
                                      {
                                        string newUserChownCommand = zowex_command + " uss chown newUser " + uss_path;
                                        string listUser = "ls -l " + uss_path;

                                        rc = execute_command_with_output(newUserChownCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("USS path '" + uss_path + "' owner changed to 'newUser'");

                                        rc = execute_command_with_output(listUser, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                      });
                                   it("should properly change the group on a file",
                                      [&]() -> void
                                      {
                                        string newGroupChownCommand = zowex_command + " uss chown :newGroup " + uss_path;
                                        string listUser = "ls -l " + uss_path;

                                        rc = execute_command_with_output(newGroupChownCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("USS path '" + uss_path + "' owner changed to ':newGroup'");

                                        rc = execute_command_with_output(listUser, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                      });
                                   it("should properly change the group and user on a file",
                                      [&]() -> void
                                      {
                                        string newUserNewGroupChownCommand = zowex_command + " uss chown newUser:newGroup " + uss_path;
                                        string listUser = "ls -l " + uss_path;

                                        rc = execute_command_with_output(newUserNewGroupChownCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("USS path '" + uss_path + "' owner changed to 'newUser:newGroup'");

                                        rc = execute_command_with_output(listUser, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                      });
                                 });
                        describe("on a directory",
                                 [&]() -> void
                                 {
                                   beforeEach([&]() -> void
                                              {
                                                uss_path = get_random_uss(ussTestDir) + "_dir";
                                                create_test_dir_cmd(uss_path); });

                                   it("should properly change the group and user on a directory",
                                      [&]() -> void
                                      {
                                        string newUserNewGroupChownCommand = zowex_command + " uss chown newUser:newGroup " + uss_path + " -r";
                                        string listUser = "ls -l " + uss_path;

                                        rc = execute_command_with_output(newUserNewGroupChownCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("USS path '" + uss_path + "' owner changed to 'newUser:newGroup'");

                                        rc = execute_command_with_output(listUser, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                      });
                                 });
                      });
             describe("chtag",
                      [&]() -> void
                      {
                        string uss_file;

                        describe("on a file",
                                 [&]() -> void
                                 {
                                   beforeEach([&]() -> void
                                              {
                                                uss_file = get_random_uss(ussTestDir);
                                                create_test_file_cmd(uss_file); });

                                   it("should properly change the file tag on a file",
                                      [&]() -> void
                                      {
                                        string chtagCommand = zowex_command + " uss chtag " + uss_file + " IBM-1047";
                                        string listUser = "ls -alT " + uss_file;

                                        rc = execute_command_with_output(chtagCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("USS path '" + uss_file + "' tag changed to 'IBM-1047'");

                                        rc = execute_command_with_output(listUser, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("IBM-1047");
                                      });

                                   it("should properly handle setting an invalid file tag",
                                      [&]() -> void
                                      {
                                        string chtagCommand = zowex_command + " uss chtag " + uss_file + " bad-tag";
                                        string listUser = "ls -alT " + uss_file;

                                        rc = execute_command_with_output(chtagCommand, response);
                                        ExpectWithContext(rc, response).ToBe(255);
                                        Expect(response).ToContain("Invalid tag 'bad-tag' - not a valid CCSID or display name");
                                      });
                                 });

                        it("should properly change the file tag recursively (on a file)",
                           [&]() -> void
                           {
                             string uss_dir_like_file = get_random_uss(ussTestDir) + "_dir";
                             create_test_file_cmd(uss_dir_like_file);
                             string chtagCommand = zowex_command + " uss chtag " + uss_dir_like_file + " IBM-1047 -r";
                             string listUser = "ls -alT " + uss_dir_like_file;

                             rc = execute_command_with_output(chtagCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS path '" + uss_dir_like_file + "' tag changed to 'IBM-1047'");

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("IBM-1047");
                           });
                      });
             describe("create-dir",
                      [&]() -> void
                      {
                        it("should properly create a directory with mode 777 specified",
                           [&]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir) + "_dir";
                             create_test_dir_cmd(uss_dir, "--mode 777");
                             string listUser = "ls -ld " + uss_dir;

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("drwxrwxrwx");
                           });
                        it("should properly create a directory with mode 700 specified",
                           [&]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir) + "_dir";
                             create_test_dir_cmd(uss_dir, "--mode 700");
                             string listUser = "ls -ld " + uss_dir;

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("drwx------");
                           });
                        it("should properly handle recursive directory creation",
                           [&]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir) + "_dir/recursive_test";
                             create_test_dir_cmd(uss_dir);
                             string listUser = "ls -ld " + uss_dir;

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(uss_dir);
                           });
                        it("should properly handle an invalid directory path creation",
                           [&]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir);
                             string command = zowex_command + " uss create-dir " + uss_dir + "//";

                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Failed to create directory '" + uss_dir + "/'");
                           });
                      });
             describe("create-file",
                      [&]() -> void
                      {
                        it("should properly create a file with mode 777 specified",
                           [&]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir);
                             create_test_file_cmd(uss_file, "--mode 777");
                             string listUser = "ls -l " + uss_file;

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("-rwxrwxrwx");
                           });
                        it("should properly create a file with mode 644 by default",
                           [&]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir);
                             create_test_file_cmd(uss_file);
                             string listUser = "ls -l " + uss_file;

                             rc = execute_command_with_output(listUser, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("-rw-r--r--");
                           });
                        it("should properly handle a creating a file in a non-existant directory",
                           [&]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir) + "/does_not_exist";
                             string command = zowex_command + " uss create-file " + uss_file;

                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("could not create USS file: '" + uss_file + "'");
                           });
                      });
             describe("delete",
                      [&]() -> void
                      {
                        it("should properly delete a file",
                           [&]() -> void
                           {
                             string uss_file = get_random_uss(ussTestDir);
                             create_test_file_cmd(uss_file);

                             string deleteCommand = zowex_command + " uss delete " + uss_file;
                             rc = execute_command_with_output(deleteCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS item '" + uss_file + "' deleted");

                             string listCommand = "ls " + uss_file;
                             rc = execute_command_with_output(listCommand, response);
                             Expect(rc).ToBe(1);
                           });

                        it("should properly delete a directory recursively",
                           [&]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir) + "_dir";
                             create_test_dir_cmd(uss_dir);
                             string uss_file_in_dir = uss_dir + "/test_file.txt";
                             create_test_file_cmd(uss_file_in_dir);

                             string deleteCommand = zowex_command + " uss delete " + uss_dir + " -r";
                             rc = execute_command_with_output(deleteCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("USS item '" + uss_dir + "' deleted");

                             string listCommand = "ls -d " + uss_dir;
                             rc = execute_command_with_output(listCommand, response);
                             Expect(rc).ToBe(1);
                           });

                        it("should properly handle deleting a non-existent item",
                           [&]() -> void
                           {
                             string uss_file = ussTestDir + "/test_does_not_exist";
                             string deleteCommand = zowex_command + " uss delete " + uss_file;

                             rc = execute_command_with_output(deleteCommand, response);
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Path '" + uss_file + "' does not exist");
                           });
                      });

             //  describe("list (ls)",
             //           [&]() -> void
             //           {
             //             it("should properly delete a file",
             //                [&]() -> void {});
             //           });
           });
}