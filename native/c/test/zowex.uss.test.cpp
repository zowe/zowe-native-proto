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
#include <cstring>
#include <ctime>
#include <iterator>
#include <stdlib.h>
#include <string>
#include "zutils.hpp"
#include <stdio.h>
#include "test_utils.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../zut.hpp"

using namespace std;
using namespace ztst;

const string ussTestDir = "/tmp/zowex-uss_" + get_random_string(10);

void zowex_uss_tests()
{
  int rc;
  string response;
  describe("uss",
           [&]() -> void
           {
             // Start by creating a /tmp/zowex-uss test directory
             beforeAll([&response]() -> void
                       { execute_command_with_output(zowex_command + " uss create-dir " + ussTestDir + " --mode 777", response); 
                        cout << "Running USS tests in " + ussTestDir << endl;
                      });

             // Reset the RC to zero before each test
             beforeEach([&rc]() -> void
                        { rc = 0; });

             // Clean up the test directory
             afterAll([&response]() -> void
                      { execute_command_with_output(zowex_command + " uss delete " + ussTestDir + " --recursive", response); });

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



             describe("copy uss tests", 
              [&]() -> void 
              {

                // Helper function for uss copy
                auto copy_cmd = [&](const string &source, const string &dest, const string &options = "") -> std::pair<string, int>
                {  
                    int lrc; 
                    string lresponse;
                    string command = zowex_command + " uss copy " + source + " " + dest + " " + options;
                    lrc = execute_command_with_output(command, lresponse);
                    return {lresponse, lrc};
                };

                auto delete_cmd = [&](const string &file_path, const string &options = "") -> std::pair<string, int>
                {
                    int lrc; 
                    string lresponse;
                    string command = zowex_command + " uss delete " + file_path + " " + options;
                    lrc = execute_command_with_output(command, lresponse);
                    return {lresponse, lrc};
                };

                auto list_cmd = [&](const string &file_path, const string &options = "") -> std::pair<string, int>
                {
                    int lrc; 
                    string lresponse;
                    string command = zowex_command + " uss list " + file_path + " " + options;
                    lrc = execute_command_with_output(command, lresponse);
                    return {lresponse, lrc};
                };

                auto chmod_cmd = [&](const string &file_path, const string &permissions, const string &options = "") -> std::pair<string, int> 
                {
                  int lrc;
                  string lresponse;
                  string command = zowex_command + " uss chmod " + permissions + " " + file_path;
                  lrc = execute_command_with_output(command, lresponse);
                  return {lresponse, lrc};
                };

                string uss_path;
                string source_path;
                string destination_path;

                beforeEach([&]() {
                   uss_path = get_random_uss(ussTestDir);
                   create_test_dir_cmd(uss_path);
                });
                
                it("copy file->file tests", [&](){
                  // simple copy
                  string source_file = uss_path + "/test_file_a";
                  string target_file = uss_path + "/test_file_b";
                  create_test_file_cmd(uss_path + "/test_file_a");
                  std::pair<string, int> copy_result;
                  std::pair<string, int> list_file_result;

                  copy_result = copy_cmd(source_file, target_file);
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(0);
                  list_file_result = list_cmd(target_file);
                  ExpectWithContext(list_file_result.second, list_file_result.first).ToBe(0);

                  // copy when target already exists: replaces it
                  copy_result = copy_cmd(source_file, target_file);
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(0);
                  list_file_result = list_cmd(target_file);
                  ExpectWithContext(list_file_result.second, list_file_result.first).ToBe(0);

                  // copy with --no-preserve-pemissions
                  string output;
                  zut_run_shell_command("chmod 777 " + source_file, output);
                  copy_result = copy_cmd(source_file, target_file, "--no-preserve-permissions");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(0);
                  //TODO: list show permissions
                  list_file_result = list_cmd(target_file);
                  ExpectWithContext(list_file_result.second, list_file_result.first).ToBe(0);
                  Expect(list_file_result.first).ToContain("-r--r--r--");
                  
                  // copy file with special characters
                  string file_special_chars = uss_path + "/\"te[st]_?*\"";
                  // this fails?
                  // create_test_file_cmd(file_special_chars);

                  copy_result = copy_cmd(source_file, file_special_chars);
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(0);
                  list_file_result = list_cmd(target_file);
                  ExpectWithContext(list_file_result.second, list_file_result.first).ToBe(0);

                });

                it("copy dir->file tests", [&](){
                  

                });

                it("copy file->dir tests", [&](){});

                it("copy dir->dir tests", [&](){
                  string source_dir = uss_path + "/test_dir_a";
                  string target_dir = uss_path + "/test_dir_b";
                  std::pair<string, int> copy_result;
                  std::pair<string, int> list_file_result;
                  create_test_dir_cmd(source_dir);

                  // bad source
                  copy_result = copy_cmd("/noway/src/noexist", target_dir);
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(255);

                  // bad target
                  copy_result = copy_cmd(source_dir, "/noway/dest/noexist");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(255);

                  // without --recursive fails
                  copy_result = copy_cmd(source_dir, target_dir);
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(255);
                  list_file_result = list_cmd(target_dir);
                  ExpectWithContext(list_file_result.second, list_file_result.first).ToBe(255);
                  // with -r
                  copy_result = copy_cmd(source_dir, target_dir, "--recursive");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(0);
                  list_file_result = list_cmd(target_dir);
                  ExpectWithContext(list_file_result.second, list_file_result.first).ToBe(0);
                  delete_cmd(target_dir, "-r");

                  // nested directories (no symlinks)
                  string output;
                  string nested_dir =  source_dir + "/one/two/three";  
                  string target_nested_dir = target_dir + "/one/two/three";
                  // TODO: story - enhance uss create-dir with -p?
                  zut_run_shell_command("mkdir -p " + nested_dir, output);

                  // copy nested dirs
                  copy_result = copy_cmd(source_dir, target_dir, "--recursive");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(0);
                  list_file_result = list_cmd(target_nested_dir);
                  ExpectWithContext(list_file_result.second, list_file_result.first).ToBe(0);
                  delete_cmd(target_dir, "-r");  

                  // with symlinks
                  string symlink_target_dir = uss_path + "/some/other/nested/dir";
                  zut_run_shell_command("mkdir -p " + symlink_target_dir, output);
                  zut_run_shell_command("ln -s " + uss_path + "/some" + " " + nested_dir, output);
                  copy_result = copy_cmd(source_dir, target_dir, "-r -L");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(0);
                  list_file_result = list_cmd(nested_dir + "/some/other");
                  ExpectWithContext(list_file_result.second, list_file_result.first).ToBe(0);

                });

                it("illegal syntax tests", [&](){
                  std::pair<string, int> copy_result = copy_cmd("", "some-destination");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(1);
                  copy_result = copy_cmd("/some/source", "");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(1);
                  copy_result = copy_cmd("", "");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(1);
                  copy_result = copy_cmd("/some/source", "/some/dest", "--illegal-parameter");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(1);
                });

                it("illegal parameter combinations", [&](){
                  std::pair<string, int> copy_result;
                  string source_dir = uss_path + "/test_dir_a";
                  string target_dir = uss_path + "/test_dir_b";

                  create_test_dir_cmd(source_dir);

                  // requires --recursive
                  copy_result = copy_cmd(source_dir, target_dir, "--follow-symlinks");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(255);
                
                  // still requires --recursive
                  copy_result = copy_cmd(source_dir, target_dir, "--follow-symlinks --dont-preserve-permissions");
                  ExpectWithContext(copy_result.second, copy_result.first).ToBe(255);
                });

                it("running with and without preserved file attributes", [&](){
                  

                });

                it("copying with permission restrictions", [&](){

                });

              });

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
                             string chmodFileCommand = zowex_command + " uss chmod 777 " + uss_file;
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(chmodFileCommand, response);
                             }
                             ExpectWithContext(rc, response).ToBe(255);
                             Expect(response).ToContain("Path '" + uss_file + "' does not exist");
                           });
                      });
             describe("chown",
                      [&]() -> void
                      {
                        string uss_path;
                        static int testUid;
                        static int testGid;

                        beforeAll([&]() -> void
                                  {
                       string resp;
                       int rc;

                       testGid = 60000;
                       while (true)
                       {
                         string idCheck = "id -g " + to_string(testGid) + " > /dev/null 2>&1";
                         rc = execute_command_with_output(idCheck, resp);
                         if (rc != 0) 
                           break;
                         testGid++;
                         if (testGid > 65530) throw runtime_error("Could not find a free GID for testing");
                       }

                       testUid = 60000;
                       while (true)
                       {
                         string idCheck = "id -u " + to_string(testUid) + " > /dev/null 2>&1";
                         rc = execute_command_with_output(idCheck, resp);
                         if (rc != 0)
                           break;
                         testUid++;
                         if (testUid > 65530) throw runtime_error("Could not find a free UID for testing");
                       } });

                        describe("on a file",
                                 [&]() -> void
                                 {
                                   beforeEach([&]() -> void
                                              {
                                   uss_path = get_random_uss(ussTestDir);
                                   create_test_file_cmd(uss_path); });

                                   it("should properly change the group on a file using numeric GID",
                                      [&]() -> void
                                      {
                                        string newGroupChownCommand = zowex_command + " uss chown :" + to_string(testGid) + " " + uss_path;
                                        string listUser = "ls -n " + uss_path;

                                        rc = execute_command_with_output(newGroupChownCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("USS path '" + uss_path + "' owner changed to ':" + to_string(testGid) + "'");

                                        rc = execute_command_with_output(listUser, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain(to_string(testGid));
                                      });

                                   it("should properly change the group and user on a file using numeric IDs",
                                      [&]() -> void
                                      {
                                        string newUserNewGroupChownCommand = zowex_command + " uss chown " + to_string(testUid) + ":" + to_string(testGid) + " " + uss_path;
                                        string listUser = "ls -n " + uss_path;

                                        rc = execute_command_with_output(newUserNewGroupChownCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("USS path '" + uss_path + "' owner changed to '" + to_string(testUid) + ":" + to_string(testGid) + "'");

                                        rc = execute_command_with_output(listUser, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain(to_string(testUid));
                                        Expect(response).ToContain(to_string(testGid));
                                      });
                                 });

                        describe("on a directory",
                                 [&]() -> void
                                 {
                                   beforeEach([&]() -> void
                                              {
                                   uss_path = get_random_uss(ussTestDir) + "_dir";
                                   create_test_dir_cmd(uss_path); });

                                   it("should properly change the group and user on a directory using numeric IDs",
                                      [&]() -> void
                                      {
                                        string newUserNewGroupChownCommand = zowex_command + " uss chown " + to_string(testUid) + ":" + to_string(testGid) + " " + uss_path + " -r";

                                        // Use -nd to list directory details numerically without entering it
                                        string listUser = "ls -nd " + uss_path;

                                        rc = execute_command_with_output(newUserNewGroupChownCommand, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("USS path '" + uss_path + "' owner changed to '" + to_string(testUid) + ":" + to_string(testGid) + "'");

                                        rc = execute_command_with_output(listUser, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain(to_string(testUid));
                                        Expect(response).ToContain(to_string(testGid));
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
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(command, response);
                             }
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
                             {
                               test_utils::ErrorStreamCapture c;
                               rc = execute_command_with_output(command, response);
                             }
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

             describe("write and view",
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
                             string writeCommand = "echo 'Hello World!' | " + zowex_command + " uss write " + uss_path + " --ec UTF-8";
                             string view_response;

                             rc = execute_command_with_output(writeCommand, response);
                             ExpectWithContext(rc, "Write command failed").ToBe(0);

                             rc = execute_command_with_output(viewCommand, view_response);
                             ExpectWithContext(rc, view_response).ToBe(0);

                             Expect(view_response).ToContain("Hello World!");
                           });
                        it("should successfully write using a valid etag",
                           [&]() -> void
                           {
                             string viewCommand = zowex_command + " uss view " + uss_path + " --return-etag --ec UTF-8";
                             string writeCommand = "echo 'Initial Content' | " + zowex_command + " uss write " + uss_path + " --ec UTF-8";
                             string view_response;

                             rc = execute_command_with_output(writeCommand, response);
                             ExpectWithContext(rc, "Initial write failed").ToBe(0);

                             rc = execute_command_with_output(viewCommand, view_response);
                             ExpectWithContext(rc, view_response).ToBe(0);
                             Expect(view_response).ToContain("Initial Content");
                             string valid_etag = parse_etag_from_output(view_response);
                             ExpectWithContext(valid_etag, "Failed to parse ETag from view output.").Not().ToBe("");

                             string writeWithValidEtagCmd = "echo 'Updated Content' | " + zowex_command + " uss write " + uss_path + " --ec UTF-8 --etag " + valid_etag;
                             rc = execute_command_with_output(writeWithValidEtagCmd, response);
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

                             string writeCommand = "printf '%s' '" + ebcdic_text + "' | " + zowex_command + " uss write " + uss_path + " --lec IBM-1047 --ec UTF-8";
                             rc = execute_command_with_output(writeCommand, response);
                             ExpectWithContext(rc, "Write command failed").ToBe(0);

                             string viewCommand = zowex_command + " uss view " + uss_path + " --ec UTF-8";
                             string view_response_hex_dump;
                             rc = execute_command_with_output(viewCommand, view_response_hex_dump);
                             ExpectWithContext(rc, view_response_hex_dump).ToBe(0);

                             Expect(memcmp(view_response_hex_dump.data(), expected_ascii_text.data(), view_response_hex_dump.length()))
                                 .ToBe(0);
                             Expect(view_response_hex_dump.length()).ToBe(expected_ascii_text.length());
                           });
                        it("should write content to a USS file with multibyte encoding",
                           [&]() -> void
                           {
                             string response;
                             string command = "echo '\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf' | " + zowex_command + " uss write " + uss_path + " --encoding IBM-939";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + uss_path + "'");

                             command = zowex_command + " uss view " + uss_path + " --rfb --ec binary";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("0e 44 8a 44 bd 44 97 44 92 44 9d 0f");
                           });
                        it("should handle write and view for a FIFO pipe",
                           [&]() -> void
                           {
                             string writeCommand = "echo 'Hello World!' | " + zowex_command + " uss write " + uss_path + " --ec binary";
                             string viewCommand = zowex_command + " uss view " + uss_path + " --ec binary";
                             string view_response;
                             mkfifo(uss_path.c_str(), 0777);

                             rc = execute_command_with_output(writeCommand, response);
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
                             string writeCommand = "echo 'Hello World!' | " + zowex_command + " uss write " + symPath + " --ec binary";

                             // Read from original path
                             string viewCommand = zowex_command + " uss view " + uss_path + " --ec binary";
                             string listCommand = zowex_command + " uss ls " + uss_path + " -l";
                             string view_response;

                             rc = execute_command_with_output(writeCommand, response);
                             ExpectWithContext(rc, "Write command failed").ToBe(0);

                             rc = execute_command_with_output(viewCommand, view_response);
                             ExpectWithContext(rc, view_response).ToBe(0);

                             Expect(view_response).ToContain("Hello World!");
                             unlink(symPath.c_str());
                           });
                      });

             describe("list (ls)",
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