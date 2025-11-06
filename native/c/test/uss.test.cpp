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
#include "zutils.hpp"
#include <stdio.h>
#include "test_utils.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace ztst;

const string zowex_command = "./../build-out/zowex";
const string ussTestDir = "/tmp/zowex-uss";

void uss_tests()
{
  int rc;
  string response;
  describe("uss tests",
           [&]() -> void
           {
             // Start by creating a /tmp/zowex-uss test directory
             beforeAll([&response]() -> void
                       { execute_command_with_output(zowex_command + " uss create-dir " + ussTestDir + " --mode 777", response); });

             // Reset the RC to zero before each test
             beforeEach([&rc]() -> void
                        { rc = 0; });

             // Clean up the test directory
             afterAll([&response]() -> void
                      { execute_command_with_output(zowex_command + " uss delete /tmp/zowex-uss --recursive", response); });

             // Helper function to create a test file
             auto create_test_file_cmd = [&](const string &uss_file, const string &options = "") -> void
             {
               string command = zowex_command + " uss create-file " + uss_file + " " + options;
               rc = execute_command_with_output(command, response);
               ExpectWithContext(rc, response).ToBe(0);
               Expect(response).ToContain("USS file '" + uss_file + "' created");
             };

             // Helper function to create a test directory
             auto create_test_dir_cmd = [&](const string &uss_dir, const string &options = "") -> void
             {
               string command = zowex_command + " uss create-dir " + uss_dir + " " + options;
               rc = execute_command_with_output(command, response);
               ExpectWithContext(rc, response).ToBe(0);
               Expect(response).ToContain("USS directory '" + uss_dir + "' created");
             };

             describe("zowex uss - chmod",
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
                             string chmodFileCommand = zowex_command + " uss chmod 777 " + uss_file;
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(chmodFileCommand, response);
                             }
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Path '" + uss_file + "' does not exist");
                           });
                      });
             // Chown tests currently do not run. Will be ran once https://github.com/zowe/zowe-native-proto/issues/400 is resolved
             xdescribe("zowex uss - chown",
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
                                         Expect(response).ToContain("newUser");
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
                                         Expect(response).ToContain("newGroup");
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
                                        Expect(response).ToContain("newUser");
                                        Expect(response).ToContain("newGroup"); });
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
                                         string listUser = "ls -ld " + uss_path;

                                         rc = execute_command_with_output(newUserNewGroupChownCommand, response);
                                         ExpectWithContext(rc, response).ToBe(0);
                                         Expect(response).ToContain("USS path '" + uss_path + "' owner changed to 'newUser:newGroup'");

                                         rc = execute_command_with_output(listUser, response);
                                         ExpectWithContext(rc, response).ToBe(0);
                                         Expect(response).ToContain("newUser");
                                         Expect(response).ToContain("newGroup");
                                       });
                                  });
                       });
             describe("zowex uss - chtag",
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
                                        {
                                          test_utils::ErrorStreamCapture c;
                                          rc = execute_command_with_output(chtagCommand, response);
                                        }
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
             describe("zowex uss - create-dir",
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
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(command, response);
                             }
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Failed to create directory '" + uss_dir + "/'");
                           });
                      });
             describe("zowex uss - create-file",
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
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(command, response);
                             }
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("could not create USS file: '" + uss_file + "'");
                           });
                      });
             describe("zowex uss - delete",
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
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(deleteCommand, response);
                             }
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Path '" + uss_file + "' does not exist");
                           });

                        it("should properly handle deleting a directory without the recursive flag",
                           [&]() -> void
                           {
                             string uss_dir = get_random_uss(ussTestDir) + "_dir";
                             create_test_dir_cmd(uss_dir);
                             string deleteCommand = zowex_command + " uss delete " + uss_dir;
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(deleteCommand, response);
                             }
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Path '" + uss_dir + "' is a directory and recursive was false");
                           });
                      });

             describe("zowex uss - write and view",
                      [&]() -> void
                      {
                        string uss_path;
                        beforeEach([&]() -> void
                                   {
                                                 uss_path = get_random_uss(ussTestDir);
                                                 create_test_file_cmd(uss_path); });
                        it("should properly write and view files",
                           [&]() -> void
                           {
                             string viewCommand = zowex_command + " uss view " + uss_path + " --ec UTF-8";
                             string writeCommand = zowex_command + " uss write " + uss_path + " --ec UTF-8";
                             string view_response;

                             rc = execute_command_with_input(writeCommand, "Hello World!");
                             ExpectWithContext(rc, "Write command failed").ToBe(0);

                             rc = execute_command_with_output(viewCommand, view_response);
                             ExpectWithContext(rc, view_response).ToBe(0);

                             Expect(view_response).ToContain("Hello World!");
                           });
                        it("should successfully write using a valid etag",
                           [&]() -> void
                           {
                             string viewCommand = zowex_command + " uss view " + uss_path + " --return-etag --ec UTF-8";
                             string writeCommand = zowex_command + " uss write " + uss_path + " --ec UTF-8";
                             string view_response;

                             rc = execute_command_with_input(writeCommand, "Initial Content");
                             ExpectWithContext(rc, "Initial write failed").ToBe(0);

                             rc = execute_command_with_output(viewCommand, view_response);
                             ExpectWithContext(rc, view_response).ToBe(0);
                             Expect(view_response).ToContain("Initial Content");
                             string valid_etag = parse_etag_from_output(view_response);
                             ExpectWithContext(valid_etag, "Failed to parse ETag from view output.").Not().ToBe("");

                             string writeWithValidEtagCmd = writeCommand + " --etag " + valid_etag;
                             rc = execute_command_with_input(writeWithValidEtagCmd, "Updated Content");
                             ExpectWithContext(rc, "Write with valid etag failed").ToBe(0);

                             string final_view_response;
                             string simpleViewCmd = zowex_command + " uss view " + uss_path + " --ec UTF-8";
                             rc = execute_command_with_output(simpleViewCmd, final_view_response);
                             Expect(final_view_response).ToContain("Updated Content");
                           });
                        it("should fail to view a file that does not exist",
                           [&]() -> void
                           {
                             string viewCommand = zowex_command + " uss view /tmp/does/not/exist";
                             string view_response;
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(viewCommand, view_response);
                             }
                             ExpectWithContext(rc, view_response).ToBe(255);
                             Expect(view_response).ToContain("Path /tmp/does/not/exist does not exist");
                           });
                        it("should properly translate EBCDIC text to ASCII/UTF-8",
                           [&]() -> void
                           {
                             string ebcdic_text = "Hello World - This is a test.";

                             string expected_ascii_text =
                                 "\x48\x65\x6c\x6c\x6f\x20" // "Hello "
                                 "\x57\x6f\x72\x6c\x64\x20" // "World "
                                 "\x2d\x20"                 // "- "
                                 "\x54\x68\x69\x73\x20"     // "This "
                                 "\x69\x73\x20"             // "is "
                                 "\x61\x20"                 // "a "
                                 "\x74\x65\x73\x74\x2e";    // "test."

                             string writeCommand = zowex_command + " uss write " + uss_path;
                             rc = execute_command_with_input(writeCommand, ebcdic_text);
                             ExpectWithContext(rc, "Write command failed").ToBe(0);

                             string viewCommand = zowex_command + " uss view " + uss_path + " --rfb";
                             string view_response_hex_dump;
                             rc = execute_command_with_output(viewCommand, view_response_hex_dump);
                             ExpectWithContext(rc, view_response_hex_dump).ToBe(0);

                             // Remove Newline
                             view_response_hex_dump.pop_back();

                             // Get last 2 characters
                             string newLine = view_response_hex_dump.substr(view_response_hex_dump.length() - 2);
                             if (!view_response_hex_dump.empty() && newLine == "0a")
                             {
                               // Update Dump to not include newline
                               view_response_hex_dump = view_response_hex_dump.substr(0, view_response_hex_dump.length() - 3);
                             }

                             string parsed_response_bytes = parse_hex_dump(view_response_hex_dump);

                             Expect(parsed_response_bytes.length()).ToBe(expected_ascii_text.length());
                             ExpectWithContext(memcmp(parsed_response_bytes.data(), expected_ascii_text.data(), parsed_response_bytes.length()),
                                               "Byte-for-byte memory comparison failed.")
                                 .ToBe(0);
                           });
                        it("should handle write and view for a FIFO pipe",
                           [&]() -> void
                           {
                             string writeCommand = zowex_command + " uss write " + uss_path + " --ec binary";
                             string viewCommand = zowex_command + " uss view " + uss_path + " --ec binary";
                             string view_response;
                             mkfifo(uss_path.c_str(), 0777);

                             rc = execute_command_with_input(writeCommand, "Hello World!");
                             ExpectWithContext(rc, "Write command failed").ToBe(0);

                             rc = execute_command_with_output(viewCommand, view_response);
                             ExpectWithContext(rc, view_response).ToBe(0);

                             Expect(view_response).ToContain("Hello World!");
                           });
                        it("should handle write and view for a symlink",
                           [&]() -> void
                           {
                             // Create symlink
                             string symPath = uss_path + "_sym";
                             symlink(uss_path.c_str(), symPath.c_str());

                             // Write to sym link
                             string writeCommand = zowex_command + " uss write " + symPath + " --ec binary";

                             // Read from original path
                             string viewCommand = zowex_command + " uss view " + uss_path + " --ec binary";
                             string listCommand = zowex_command + " uss ls " + uss_path + " -l";
                             string view_response;

                             rc = execute_command_with_input(writeCommand, "Hello World!");
                             ExpectWithContext(rc, "Write command failed").ToBe(0);

                             rc = execute_command_with_output(viewCommand, view_response);
                             ExpectWithContext(rc, view_response).ToBe(0);

                             Expect(view_response).ToContain("Hello World!");
                           });
                      });

             describe("zowex uss - list (ls)",
                      [&]() -> void
                      {
                        beforeAll([&response]() -> void
                                  {
                                     execute_command_with_output(zowex_command + " uss create-dir " + ussTestDir + "/subDir1/subDir2/subDir3" + " --mode 777", response);
                                     execute_command_with_output(zowex_command + " uss create-file " + ussTestDir + "/subDir1/subFile1" + " --mode 777", response);
                                     execute_command_with_output(zowex_command + " uss create-file " + ussTestDir + "/subDir1/subDir2/subFile2" + " --mode 777", response);
                                     execute_command_with_output(zowex_command + " uss create-file " + ussTestDir + "/subDir1/subDir2/subDir3/subFile3" + " --mode 777", response); });
                        it("should properly list files",
                           [&]() -> void
                           {
                             string listCommand = zowex_command + " uss ls " + ussTestDir;
                             rc = execute_command_with_output(listCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("subDir1");
                           });
                        it("should properly list files with the --all option",
                           [&]() -> void
                           {
                             string listAllCommand = zowex_command + " uss ls " + ussTestDir + " --all";
                             rc = execute_command_with_output(listAllCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(".");
                             Expect(response).ToContain("..");
                             Expect(response).ToContain("subDir1");
                           });
                        it("should properly list files with the --depth option set to 1 and should match the output of no depth option specified",
                           [&]() -> void
                           {
                             string testResponse;
                             string listCommand = zowex_command + " uss ls " + ussTestDir;
                             string listDepthCommand = zowex_command + " uss ls " + ussTestDir + " --depth 1";
                             rc = execute_command_with_output(listDepthCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("subDir1");
                             execute_command_with_output(listCommand, testResponse);
                             Expect(response).ToBe(testResponse);
                           });
                        it("should properly list files with the --depth option set to 2",
                           [&]() -> void
                           {
                             string listDepthCommand = zowex_command + " uss ls " + ussTestDir + " --depth 2";
                             rc = execute_command_with_output(listDepthCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("subDir1/subDir2");
                             Expect(response).ToContain("subDir1/subFile1");
                           });
                        it("should properly list files with the --depth option set to 3",
                           [&]() -> void
                           {
                             string listDepthCommand = zowex_command + " uss ls " + ussTestDir + " --depth 3";
                             rc = execute_command_with_output(listDepthCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("subDir1/subDir2");
                             Expect(response).ToContain("subDir1/subDir2/subDir3");
                             Expect(response).ToContain("subDir1/subDir2/subFile2");
                             Expect(response).ToContain("subDir1/subFile1");
                           });
                        it("should properly list files with --response-format-csv",
                           [&]() -> void
                           {
                             string listDepthCommand = zowex_command + " uss ls " + ussTestDir + " -l --rfc";
                             rc = execute_command_with_output(listDepthCommand, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("drwxrwxrwx,");
                             Expect(response).ToContain(",subDir1");
                           });
                        it("should properly handle missing options",
                           [&]() -> void
                           {
                             string incompleteCommand = zowex_command + " uss ls";
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(incompleteCommand, response);
                             }
                             ExpectWithContext(rc, response).ToBe(1);
                             Expect(response).ToContain("missing required positional argument: file-path");
                           });
                        it("should properly handle listing a path that does not exist",
                           [&]() -> void
                           {
                             string incompleteCommand = zowex_command + " uss ls /does/not/exist";
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(incompleteCommand, response);
                             }
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Path '/does/not/exist' does not exist");
                           });
                      });
           });
}