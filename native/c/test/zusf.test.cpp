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

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "ztest.hpp"
#include "zusf.hpp"
#include "ztype.h"

using namespace std;
using namespace ztst;

void zusf_tests()
{
  describe("zusf_chmod_uss_file_or_dir tests",
           [&]() -> void
           {
             // Setup test environment
             ZUSF zusf;
             memset(&zusf, 0, sizeof(zusf));

             const string test_file = "/tmp/zusf_test_file.txt";
             const string test_dir = "/tmp/zusf_test_dir";
             const string nonexistent_file = "/tmp/nonexistent_file.txt";

             it("should fail when file does not exist",
                [&]() -> void
                {
                  // Ensure the file doesn't exist
                  unlink(nonexistent_file.c_str());

                  int result = zusf_chmod_uss_file_or_dir(&zusf, nonexistent_file, 0644, false);
                  Expect(result).ToBe(RTNCD_FAILURE);
                  Expect(string(zusf.diag.e_msg)).ToBe("Path '/tmp/nonexistent_file.txt' does not exist");
                });

             it("should fail when trying to chmod directory without recursive flag",
                [&]() -> void
                {
                  // Create test directory
                  rmdir(test_dir.c_str()); // Remove if exists
                  mkdir(test_dir.c_str(), 0755);

                  int result = zusf_chmod_uss_file_or_dir(&zusf, test_dir, 0644, false);
                  Expect(result).ToBe(RTNCD_FAILURE);
                  Expect(string(zusf.diag.e_msg)).ToBe("Path '/tmp/zusf_test_dir' is a folder and recursive is false");

                  // Cleanup
                  rmdir(test_dir.c_str());
                });

             it("should successfully chmod a regular file",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str()); // Remove if exists
                  ofstream file(test_file);
                  file << "test content";
                  file.close();

                  // Set initial permissions
                  chmod(test_file.c_str(), 0600);

                  // Test chmod function
                  int result = zusf_chmod_uss_file_or_dir(&zusf, test_file, 0644, false);
                  Expect(result).ToBe(RTNCD_SUCCESS);

                  // Verify permissions were changed
                  struct stat file_stat;
                  stat(test_file.c_str(), &file_stat);
                  mode_t actual_mode = file_stat.st_mode & 0777; // Mask to get only permission bits
                  Expect((int)actual_mode).ToBe(0644);

                  // Cleanup
                  unlink(test_file.c_str());
                });

             it("should successfully chmod a directory with recursive flag",
                [&]() -> void
                {
                  // Create test directory structure
                  rmdir(test_dir.c_str()); // Remove if exists
                  mkdir(test_dir.c_str(), 0700);

                  // Create a file inside the directory to test recursive behavior
                  string inner_file = test_dir + "/inner_file.txt";
                  ofstream file(inner_file);
                  file << "inner content";
                  file.close();
                  chmod(inner_file.c_str(), 0600);

                  // Test chmod function with recursive flag
                  int result = zusf_chmod_uss_file_or_dir(&zusf, test_dir, 0755, true);
                  Expect(result).ToBe(RTNCD_SUCCESS);

                  // Verify directory permissions were changed
                  struct stat dir_stat;
                  stat(test_dir.c_str(), &dir_stat);
                  mode_t actual_dir_mode = dir_stat.st_mode & 0777;
                  Expect((int)actual_dir_mode).ToBe(0755);

                  // Verify file permissions were also changed (recursive)
                  struct stat file_stat;
                  stat(inner_file.c_str(), &file_stat);
                  mode_t actual_file_mode = file_stat.st_mode & 0777;
                  Expect((int)actual_file_mode).ToBe(0755);

                  // Cleanup
                  unlink(inner_file.c_str());
                  rmdir(test_dir.c_str());
                });

             it("should handle different permission modes correctly",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str());
                  ofstream file(test_file);
                  file << "test content";
                  file.close();

                  // Test different permission modes
                  mode_t test_modes[] = {0600, 0644, 0755, 0777};

                  for (mode_t mode : test_modes)
                  {
                    int result = zusf_chmod_uss_file_or_dir(&zusf, test_file, mode, false);
                    Expect(result).ToBe(RTNCD_SUCCESS);

                    struct stat file_stat;
                    stat(test_file.c_str(), &file_stat);
                    mode_t actual_mode = file_stat.st_mode & 0777;
                    Expect((int)actual_mode).ToBe((int)mode);
                  }

                  // Cleanup
                  unlink(test_file.c_str());
                });
           });

  describe("zusf_get_file_ccsid tests",
           [&]() -> void
           {
             // Setup test environment
             ZUSF zusf;
             memset(&zusf, 0, sizeof(zusf));

             const string test_file = "/tmp/zusf_ccsid_test_file.txt";
             const string test_dir = "/tmp/zusf_ccsid_test_dir";
             const string nonexistent_file = "/tmp/nonexistent_ccsid_file.txt";

             it("should fail when file does not exist",
                [&]() -> void
                {
                  // Ensure the file doesn't exist
                  unlink(nonexistent_file.c_str());

                  int result = zusf_get_file_ccsid(&zusf, nonexistent_file);
                  Expect(result).ToBe(RTNCD_FAILURE);
                  Expect(string(zusf.diag.e_msg)).ToBe("Path '/tmp/nonexistent_ccsid_file.txt' does not exist");
                });

             it("should fail when path is a directory",
                [&]() -> void
                {
                  // Create test directory
                  rmdir(test_dir.c_str()); // Remove if exists
                  mkdir(test_dir.c_str(), 0755);

                  int result = zusf_get_file_ccsid(&zusf, test_dir);
                  Expect(result).ToBe(RTNCD_FAILURE);
                  Expect(string(zusf.diag.e_msg)).ToBe("Path '/tmp/zusf_ccsid_test_dir' is a directory");

                  // Cleanup
                  rmdir(test_dir.c_str());
                });

             it("should return CCSID for a regular file",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str()); // Remove if exists
                  ofstream file(test_file);
                  file << "test content";
                  file.close();

                  int result = zusf_get_file_ccsid(&zusf, test_file);
                  // The result should be a valid CCSID (non-negative) or 0 for untagged
                  Expect(result).ToBeGreaterThanOrEqualTo(0);

                  // Cleanup
                  unlink(test_file.c_str());
                });
           });

  describe("zusf_get_ccsid_display_name tests",
           [&]() -> void
           {
             it("should return 'untagged' for CCSID 0",
                [&]() -> void
                {
                  string result = zusf_get_ccsid_display_name(0);
                  Expect(result).ToBe("untagged");
                });

             it("should return 'untagged' for negative CCSID",
                [&]() -> void
                {
                  string result = zusf_get_ccsid_display_name(-1);
                  Expect(result).ToBe("untagged");
                });

             it("should return 'binary' for CCSID 65535",
                [&]() -> void
                {
                  string result = zusf_get_ccsid_display_name(65535);
                  Expect(result).ToBe("binary");
                });

             it("should return known CCSID display names",
                [&]() -> void
                {
                  // Test some well-known CCSIDs
                  string result37 = zusf_get_ccsid_display_name(37);
                  Expect(result37).ToBe("IBM-037");

                  string result819 = zusf_get_ccsid_display_name(819);
                  Expect(result819).ToBe("ISO8859-1");

                  string result1047 = zusf_get_ccsid_display_name(1047);
                  Expect(result1047).ToBe("IBM-1047");
                });

             it("should return CCSID number as string for unknown CCSIDs",
                [&]() -> void
                {
                  // Test with an unknown CCSID
                  string result = zusf_get_ccsid_display_name(99999);
                  Expect(result).ToBe("99999");

                  // Test with another unknown CCSID
                  string result2 = zusf_get_ccsid_display_name(12345);
                  Expect(result2).ToBe("12345");
                });

             it("should handle unrealistic inputs",
                [&]() -> void
                {
                  // We don't support these inputs, but we should handle them gracefully
                  // and leave it up to the caller to decide what to do with them.

                  // Test maximum positive CCSID
                  string result = zusf_get_ccsid_display_name(2147483647); // INT_MAX
                  Expect(result).ToBe("2147483647");

                  // Test very negative CCSID
                  string result2 = zusf_get_ccsid_display_name(-999);
                  Expect(result2).ToBe("untagged");
                });
           });
}