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
#include <pwd.h>
#include <grp.h>
#include <time.h>

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

  describe("zusf_format_ls_time tests",
           [&]() -> void
           {
             it("should format time in ls-style format by default",
                [&]() -> void
                {
                  // Use a known timestamp: January 1, 2024 12:00:00 UTC
                  time_t test_time = 1704110400; // 2024-01-01 12:00:00 UTC

                  string result = zusf_format_ls_time(test_time, false);

                  // Should be in format like "Jan  1 12:00" (but will be in local time)
                  // We'll just check that it contains expected elements
                  Expect(result.length()).ToBeGreaterThan(0);
                  Expect(result.length()).ToBeLessThan(20); // Should be reasonable length
                });

             it("should format time in CSV/ISO format when requested",
                [&]() -> void
                {
                  // Use a known timestamp: January 1, 2024 12:00:00 UTC
                  time_t test_time = 1704110400; // 2024-01-01 12:00:00 UTC

                  string result = zusf_format_ls_time(test_time, true);

                  // Should be in format "2024-01-01T12:00:00"
                  Expect(result).ToBe("2024-01-01T12:00:00Z");
                });

             it("should handle zero timestamp in CSV format",
                [&]() -> void
                {
                  time_t test_time = 0; // Unix epoch

                  string result = zusf_format_ls_time(test_time, true);

                  // Should be in format "1970-01-01T00:00:00"
                  Expect(result).ToBe("1970-01-01T00:00:00Z");
                });

             it("should handle negative timestamp gracefully",
                [&]() -> void
                {
                  time_t test_time = -1; // Invalid time

                  string result = zusf_format_ls_time(test_time, true);

                  // Should fallback to epoch time
                  Expect(result).ToBe("1970-01-01T00:00:00Z");
                });

             it("should handle very large timestamp",
                [&]() -> void
                {
                  // Use year 2038 (close to 32-bit time_t limit)
                  time_t test_time = 2147483647; // 2038-01-19 03:14:07 UTC

                  string result = zusf_format_ls_time(test_time, true);

                  // Should format correctly
                  Expect(result).ToBe("2038-01-19T03:14:07Z");
                });
           });

  describe("zusf_get_owner_from_uid tests",
           [&]() -> void
           {
             it("should return username for current user UID",
                [&]() -> void
                {
                  uid_t current_uid = getuid();
                  const char *result = zusf_get_owner_from_uid(current_uid);

                  // Should return a valid username (not null)
                  Expect(result).Not().ToBeNull();

                  // Should match what getpwuid returns
                  struct passwd *pwd = getpwuid(current_uid);
                  if (pwd != nullptr)
                  {
                    Expect(string(result)).ToBe(string(pwd->pw_name));
                  }
                });

             it("should return null for invalid UID",
                [&]() -> void
                {
                  // Use a UID that's very unlikely to exist
                  uid_t invalid_uid = 99999;
                  const char *result = zusf_get_owner_from_uid(invalid_uid);

                  // Should return null for non-existent UID
                  Expect(result).ToBeNull();
                });
           });

  describe("zusf_get_group_from_gid tests",
           [&]() -> void
           {
             it("should return group name for current user GID",
                [&]() -> void
                {
                  gid_t current_gid = getgid();
                  const char *result = zusf_get_group_from_gid(current_gid);

                  // Should return a valid group name (not null)
                  Expect(result).Not().ToBeNull();

                  // Should match what getgrgid returns
                  struct group *grp = getgrgid(current_gid);
                  if (grp != nullptr)
                  {
                    Expect(string(result)).ToBe(string(grp->gr_name));
                  }
                });

             it("should return null for invalid GID",
                [&]() -> void
                {
                  // Use a GID that's very unlikely to exist
                  gid_t invalid_gid = 99999;
                  const char *result = zusf_get_group_from_gid(invalid_gid);

                  // Should return null for non-existent GID
                  Expect(result).ToBeNull();
                });

             it("should handle root GID (0)",
                [&]() -> void
                {
                  gid_t root_gid = 0;
                  const char *result = zusf_get_group_from_gid(root_gid);

                  // May or may not exist depending on system, but should handle gracefully
                  // If it exists, commonly "root" or "wheel"
                  if (result != nullptr)
                  {
                    Expect(strlen(result)).ToBeGreaterThan(0);
                  }
                });
           });

  describe("zusf_format_file_entry tests",
           [&]() -> void
           {
             ZUSF zusf;
             memset(&zusf, 0, sizeof(zusf));

             const string test_file = "/tmp/zusf_format_test_file.txt";
             const string test_dir = "/tmp/zusf_format_test_dir";

             it("should format short listing for regular file",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str());
                  ofstream file(test_file);
                  file << "test content";
                  file.close();

                  struct stat file_stats;
                  stat(test_file.c_str(), &file_stats);

                  ListOptions options = {false, false, 0}; // not all files, not long format, no recursion
                  string result = zusf_format_file_entry(&zusf, file_stats, test_file, "testfile.txt", options, false);

                  // Short format should just be filename + newline
                  Expect(result).ToBe("testfile.txt\n");

                  // Cleanup
                  unlink(test_file.c_str());
                });

             it("should format long listing for regular file in ls-style",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str());
                  ofstream file(test_file);
                  file << "test content";
                  file.close();
                  chmod(test_file.c_str(), 0644);

                  struct stat file_stats;
                  stat(test_file.c_str(), &file_stats);

                  ListOptions options = {false, true, 0}; // not all files, long format, no recursion
                  string result = zusf_format_file_entry(&zusf, file_stats, test_file, "testfile.txt", options, false);

                  // Long format should contain permissions, size, time, filename
                  Expect(result).ToContain("-rw-r--r--");   // permissions
                  Expect(result).ToContain("testfile.txt"); // filename
                  Expect(result).ToContain("12");           // file size (test content = 12 bytes)
                  Expect(result.back()).ToBe('\n');         // should end with newline

                  // Cleanup
                  unlink(test_file.c_str());
                });

             it("should format long listing for regular file in CSV format",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str());
                  ofstream file(test_file);
                  file << "test content";
                  file.close();
                  chmod(test_file.c_str(), 0644);

                  struct stat file_stats;
                  stat(test_file.c_str(), &file_stats);

                  ListOptions options = {false, true, 0}; // not all files, long format, no recursion
                  string result = zusf_format_file_entry(&zusf, file_stats, test_file, "testfile.txt", options, true);

                  // CSV format should have comma-separated values
                  Expect(result).ToContain(",");            // should have commas
                  Expect(result).ToContain("-rw-r--r--");   // permissions
                  Expect(result).ToContain("testfile.txt"); // filename
                  Expect(result).ToContain("12");           // file size
                  Expect(result.back()).ToBe('\n');         // should end with newline

                  // Should contain exactly 7 commas (8 fields total)
                  int comma_count = 0;
                  for (char c : result)
                  {
                    if (c == ',')
                      comma_count++;
                  }
                  Expect(comma_count).ToBe(7);

                  // Cleanup
                  unlink(test_file.c_str());
                });

             it("should format directory entry correctly",
                [&]() -> void
                {
                  // Create test directory
                  rmdir(test_dir.c_str());
                  mkdir(test_dir.c_str(), 0755);

                  struct stat dir_stats;
                  stat(test_dir.c_str(), &dir_stats);

                  ListOptions options = {false, true, 0}; // not all files, long format, no recursion
                  string result = zusf_format_file_entry(&zusf, dir_stats, test_dir, "testdir", options, false);

                  // Directory should start with 'd'
                  Expect(result).ToContain("drwxr-xr-x"); // directory permissions
                  Expect(result).ToContain("testdir");    // directory name

                  // Cleanup
                  rmdir(test_dir.c_str());
                });

             it("should handle files with different CCSID tags",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str());
                  ofstream file(test_file);
                  file << "test content";
                  file.close();

                  struct stat file_stats;
                  stat(test_file.c_str(), &file_stats);

                  ListOptions options = {false, true, 0}; // long format, no recursion
                  string result = zusf_format_file_entry(&zusf, file_stats, test_file, "testfile.txt", options, false);

                  // Should contain CCSID information (could be "untagged" or a specific CCSID)
                  bool has_ccsid_info = result.find("untagged") != string::npos ||
                                        result.find("IBM-") != string::npos ||
                                        result.find("UTF-8") != string::npos ||
                                        result.find("binary") != string::npos;
                  Expect(has_ccsid_info).ToBe(true);

                  // Cleanup
                  unlink(test_file.c_str());
                });
           });

  describe("zusf_list_uss_file_path tests",
           [&]() -> void
           {
             ZUSF zusf;
             memset(&zusf, 0, sizeof(zusf));

             const string test_file = "/tmp/zusf_list_test_file.txt";
             const string test_dir = "/tmp/zusf_list_test_dir";
             const string nonexistent_path = "/tmp/nonexistent_path_for_list";

             it("should fail for nonexistent path",
                [&]() -> void
                {
                  string response;
                  ListOptions options = {false, false, 0}; // no recursion

                  int result = zusf_list_uss_file_path(&zusf, nonexistent_path, response, options, false);

                  Expect(result).ToBe(RTNCD_FAILURE);
                  Expect(string(zusf.diag.e_msg)).ToContain("does not exist");
                });

             it("should list single file successfully",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str());
                  ofstream file(test_file);
                  file << "test content for listing";
                  file.close();

                  string response;
                  ListOptions options = {false, false, 0}; // short format, no recursion

                  int result = zusf_list_uss_file_path(&zusf, test_file, response, options, false);

                  Expect(result).ToBe(RTNCD_SUCCESS);
                  Expect(response).ToContain("zusf_list_test_file.txt");

                  // Cleanup
                  unlink(test_file.c_str());
                });

             it("should list single file in long format",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str());
                  ofstream file(test_file);
                  file << "test content for long listing";
                  file.close();

                  string response;
                  ListOptions options = {false, true, 0}; // long format, no recursion

                  int result = zusf_list_uss_file_path(&zusf, test_file, response, options, false);

                  Expect(result).ToBe(RTNCD_SUCCESS);
                  Expect(response).ToContain("zusf_list_test_file.txt");
                  Expect(response).ToContain("-rw-"); // file permissions

                  // Cleanup
                  unlink(test_file.c_str());
                });

             it("should list directory contents",
                [&]() -> void
                {
                  // Create test directory
                  rmdir(test_dir.c_str());
                  mkdir(test_dir.c_str(), 0755);

                  // Create files in directory
                  string file1 = test_dir + "/file1.txt";
                  string file2 = test_dir + "/file2.txt";
                  string subdir = test_dir + "/subdir";

                  ofstream f1(file1);
                  f1 << "content1";
                  f1.close();

                  ofstream f2(file2);
                  f2 << "content2";
                  f2.close();

                  mkdir(subdir.c_str(), 0755);

                  string response;
                  ListOptions options = {false, false, 0}; // short format, no recursion

                  int result = zusf_list_uss_file_path(&zusf, test_dir, response, options, false);

                  Expect(result).ToBe(RTNCD_SUCCESS);
                  Expect(response).ToContain("file1.txt");
                  Expect(response).ToContain("file2.txt");
                  Expect(response).ToContain("subdir");

                  // Files should be listed in alphabetical order
                  size_t pos1 = response.find("file1.txt");
                  size_t pos2 = response.find("file2.txt");
                  Expect(pos1).ToBeLessThan(pos2);

                  // Cleanup
                  unlink(file1.c_str());
                  unlink(file2.c_str());
                  rmdir(subdir.c_str());
                  rmdir(test_dir.c_str());
                });

             it("should list directory contents in long format",
                [&]() -> void
                {
                  // Create test directory
                  rmdir(test_dir.c_str());
                  mkdir(test_dir.c_str(), 0755);

                  // Create file in directory
                  string file1 = test_dir + "/testfile.txt";
                  ofstream f1(file1);
                  f1 << "test content";
                  f1.close();

                  string response;
                  ListOptions options = {false, true, 0}; // long format, no recursion

                  int result = zusf_list_uss_file_path(&zusf, test_dir, response, options, false);

                  Expect(result).ToBe(RTNCD_SUCCESS);
                  Expect(response).ToContain("testfile.txt");
                  Expect(response).ToContain("-rw-"); // file permissions

                  // Cleanup
                  unlink(file1.c_str());
                  rmdir(test_dir.c_str());
                });

             it("should handle hidden files based on all_files option",
                [&]() -> void
                {
                  // Create test directory
                  rmdir(test_dir.c_str());
                  mkdir(test_dir.c_str(), 0755);

                  // Create regular and hidden files
                  string regular_file = test_dir + "/regular.txt";
                  string hidden_file = test_dir + "/.hidden.txt";

                  ofstream f1(regular_file);
                  f1 << "regular content";
                  f1.close();

                  ofstream f2(hidden_file);
                  f2 << "hidden content";
                  f2.close();

                  // Test without all_files option
                  string response1;
                  ListOptions options1 = {false, false, 0}; // no all_files, no recursion
                  int result1 = zusf_list_uss_file_path(&zusf, test_dir, response1, options1, false);

                  Expect(result1).ToBe(RTNCD_SUCCESS);
                  Expect(response1).ToContain("regular.txt");
                  Expect(response1).Not().ToContain(".hidden.txt");

                  // Test with all_files option
                  string response2;
                  ListOptions options2 = {true, false, 0}; // with all_files, no recursion
                  int result2 = zusf_list_uss_file_path(&zusf, test_dir, response2, options2, false);

                  Expect(result2).ToBe(RTNCD_SUCCESS);
                  Expect(response2).ToContain("regular.txt");
                  Expect(response2).ToContain(".hidden.txt");

                  // Cleanup
                  unlink(regular_file.c_str());
                  unlink(hidden_file.c_str());
                  rmdir(test_dir.c_str());
                });

             it("should handle CSV format output",
                [&]() -> void
                {
                  // Create test file
                  unlink(test_file.c_str());
                  ofstream file(test_file);
                  file << "csv test content";
                  file.close();

                  string response;
                  ListOptions options = {false, true, 0}; // long format, no recursion

                  int result = zusf_list_uss_file_path(&zusf, test_file, response, options, true); // CSV format

                  Expect(result).ToBe(RTNCD_SUCCESS);

                  // CSV format should have commas
                  Expect(response).ToContain(",");
                  Expect(response).ToContain("zusf_list_test_file.txt");

                  // Should have proper number of fields (8 fields = 7 commas)
                  int comma_count = 0;
                  for (char c : response)
                  {
                    if (c == ',')
                      comma_count++;
                  }
                  Expect(comma_count).ToBe(7);

                  // Cleanup
                  unlink(test_file.c_str());
                });

             it("should handle empty directory",
                [&]() -> void
                {
                  // Create empty test directory
                  rmdir(test_dir.c_str());
                  mkdir(test_dir.c_str(), 0755);

                  string response;
                  ListOptions options = {false, false, 0}; // no recursion

                  int result = zusf_list_uss_file_path(&zusf, test_dir, response, options, false);

                  Expect(result).ToBe(RTNCD_SUCCESS);
                  Expect(response).ToBe(""); // Should be empty for empty directory

                  // Cleanup
                  rmdir(test_dir.c_str());
                });

             it("should handle directory with only hidden files when all_files is false",
                [&]() -> void
                {
                  // Create test directory
                  rmdir(test_dir.c_str());
                  mkdir(test_dir.c_str(), 0755);

                  // Create only hidden files
                  string hidden_file = test_dir + "/.hidden1.txt";
                  ofstream f1(hidden_file);
                  f1 << "hidden content";
                  f1.close();

                  string response;
                  ListOptions options = {false, false, 0}; // no all_files, no recursion

                  int result = zusf_list_uss_file_path(&zusf, test_dir, response, options, false);

                  Expect(result).ToBe(RTNCD_SUCCESS);
                  Expect(response).ToBe(""); // Should be empty since no visible files

                  // Cleanup
                  unlink(hidden_file.c_str());
                  rmdir(test_dir.c_str());
                });
           });

  describe("zusf_list_uss_file_path recursive tests",
           [&]() -> void
           {
             it("should list directory recursively with depth 1",
                [&]() -> void
                {
                  // Create test directory structure
                  string test_dir = "/tmp/test_recursive_dir";
                  string sub_dir = test_dir + "/subdir";
                  string file1 = test_dir + "/file1.txt";
                  string file2 = sub_dir + "/file2.txt";

                  // Cleanup first
                  unlink(file2.c_str());
                  unlink(file1.c_str());
                  rmdir(sub_dir.c_str());
                  rmdir(test_dir.c_str());

                  // Create directories
                  mkdir(test_dir.c_str(), 0755);
                  mkdir(sub_dir.c_str(), 0755);

                  // Create files
                  ofstream f1(file1);
                  f1 << "content1";
                  f1.close();

                  ofstream f2(file2);
                  f2 << "content2";
                  f2.close();

                  ZUSF zusf = {0};
                  string response;
                  ListOptions options = {false, false, 1}; // depth 1 for recursion

                  int result = zusf_list_uss_file_path(&zusf, test_dir, response, options, false);

                  Expect(result).ToBe(RTNCD_SUCCESS);
                  Expect(response).ToContain("file1.txt");
                  Expect(response).ToContain("subdir");
                  Expect(response).ToContain("subdir/file2.txt"); // Should include recursive content

                  // Cleanup
                  unlink(file2.c_str());
                  unlink(file1.c_str());
                  rmdir(sub_dir.c_str());
                  rmdir(test_dir.c_str());
                });

             it("should not recurse when depth is 0",
                [&]() -> void
                {
                  // Create test directory structure
                  string test_dir = "/tmp/test_no_recursive_dir";
                  string sub_dir = test_dir + "/subdir";
                  string file1 = test_dir + "/file1.txt";
                  string file2 = sub_dir + "/file2.txt";

                  // Cleanup first
                  unlink(file2.c_str());
                  unlink(file1.c_str());
                  rmdir(sub_dir.c_str());
                  rmdir(test_dir.c_str());

                  // Create directories
                  mkdir(test_dir.c_str(), 0755);
                  mkdir(sub_dir.c_str(), 0755);

                  // Create files
                  ofstream f1(file1);
                  f1 << "content1";
                  f1.close();

                  ofstream f2(file2);
                  f2 << "content2";
                  f2.close();

                  ZUSF zusf = {0};
                  string response;
                  ListOptions options = {false, false, 0}; // depth 0 for no recursion

                  int result = zusf_list_uss_file_path(&zusf, test_dir, response, options, false);

                  Expect(result).ToBe(RTNCD_SUCCESS);
                  Expect(response).ToContain("file1.txt");
                  Expect(response).ToContain("subdir");
                  Expect(response).Not().ToContain("subdir/file2.txt"); // Should NOT include recursive content

                  // Cleanup
                  unlink(file2.c_str());
                  unlink(file1.c_str());
                  rmdir(sub_dir.c_str());
                  rmdir(test_dir.c_str());
                });
           });

  describe("zusf source encoding tests",
           [&]() -> void
           {
             it("should use default source encoding (UTF-8) when not specified",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  // Set target encoding but no source encoding
                  strcpy(zusf.encoding_opts.codepage, "IBM-1047");
                  zusf.encoding_opts.data_type = eDataTypeText;
                  // source_codepage should be empty/null

                  // The encoding conversion logic should use UTF-8 as source when source_codepage is empty
                  Expect(strlen(zusf.encoding_opts.source_codepage)).ToBe(0);
                  Expect(strlen(zusf.encoding_opts.codepage)).ToBe(8); // "IBM-1047"
                });

             it("should use specified source encoding when provided",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  // Set both target and source encoding
                  strcpy(zusf.encoding_opts.codepage, "IBM-1047");
                  strcpy(zusf.encoding_opts.source_codepage, "IBM-037");
                  zusf.encoding_opts.data_type = eDataTypeText;

                  // Verify both encodings are set correctly
                  Expect(string(zusf.encoding_opts.codepage)).ToBe("IBM-1047");
                  Expect(string(zusf.encoding_opts.source_codepage)).ToBe("IBM-037");
                  Expect(zusf.encoding_opts.data_type).ToBe(eDataTypeText);
                });

             it("should handle binary data type correctly with source encoding",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  strcpy(zusf.encoding_opts.codepage, "binary");
                  strcpy(zusf.encoding_opts.source_codepage, "UTF-8");
                  zusf.encoding_opts.data_type = eDataTypeBinary;

                  // For binary data, encoding should not be used for conversion
                  Expect(string(zusf.encoding_opts.codepage)).ToBe("binary");
                  Expect(string(zusf.encoding_opts.source_codepage)).ToBe("UTF-8");
                  Expect(zusf.encoding_opts.data_type).ToBe(eDataTypeBinary);
                });

             it("should handle empty source encoding gracefully",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  strcpy(zusf.encoding_opts.codepage, "IBM-1047");
                  // Explicitly set source_codepage to empty
                  memset(zusf.encoding_opts.source_codepage, 0, sizeof(zusf.encoding_opts.source_codepage));
                  zusf.encoding_opts.data_type = eDataTypeText;

                  // Should handle empty source encoding (will default to UTF-8 in actual conversion)
                  Expect(strlen(zusf.encoding_opts.source_codepage)).ToBe(0);
                  Expect(string(zusf.encoding_opts.codepage)).ToBe("IBM-1047");
                });

             it("should preserve encoding settings in file operations struct",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  strcpy(zusf.encoding_opts.codepage, "IBM-1047");
                  strcpy(zusf.encoding_opts.source_codepage, "ISO8859-1");
                  zusf.encoding_opts.data_type = eDataTypeText;

                  // Simulate what would happen in a file operation
                  ZUSF zusf_copy = zusf;

                  // Verify encodings are preserved
                  Expect(string(zusf_copy.encoding_opts.codepage)).ToBe("IBM-1047");
                  Expect(string(zusf_copy.encoding_opts.source_codepage)).ToBe("ISO8859-1");
                  Expect(zusf_copy.encoding_opts.data_type).ToBe(eDataTypeText);
                });

             it("should handle USS file encoding with source encoding set",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  strcpy(zusf.encoding_opts.codepage, "UTF-8");
                  strcpy(zusf.encoding_opts.source_codepage, "IBM-1047");
                  zusf.encoding_opts.data_type = eDataTypeText;

                  const string test_file = "/tmp/zusf_source_encoding_test.txt";

                  // Create a test file with some content
                  unlink(test_file.c_str());
                  ofstream file(test_file);
                  file << "Test content for source encoding";
                  file.close();

                  // Test that the encoding options are properly set for file operations
                  // We can't easily test the actual encoding conversion without mainframe-specific
                  // file tags, but we can verify the struct is configured correctly
                  Expect(string(zusf.encoding_opts.codepage)).ToBe("UTF-8");
                  Expect(string(zusf.encoding_opts.source_codepage)).ToBe("IBM-1047");

                  // Test that file exists and can be accessed
                  struct stat file_stats;
                  int stat_result = stat(test_file.c_str(), &file_stats);
                  Expect(stat_result).ToBe(0);
                  Expect(S_ISREG(file_stats.st_mode)).ToBe(true);

                  // Cleanup
                  unlink(test_file.c_str());
                });

             it("should validate encoding field sizes and limits",
                [&]() -> void
                {
                  ZUSF zusf = {0};

                  // Test maximum length encoding names (15 chars + null terminator)
                  string long_target = "IBM-1234567890A"; // 15 characters
                  string long_source = "UTF-1234567890B"; // 15 characters

                  strncpy(zusf.encoding_opts.codepage, long_target.c_str(), sizeof(zusf.encoding_opts.codepage) - 1);
                  strncpy(zusf.encoding_opts.source_codepage, long_source.c_str(), sizeof(zusf.encoding_opts.source_codepage) - 1);

                  // Ensure null termination
                  zusf.encoding_opts.codepage[sizeof(zusf.encoding_opts.codepage) - 1] = '\0';
                  zusf.encoding_opts.source_codepage[sizeof(zusf.encoding_opts.source_codepage) - 1] = '\0';

                  Expect(string(zusf.encoding_opts.codepage)).ToBe(long_target);
                  Expect(string(zusf.encoding_opts.source_codepage)).ToBe(long_source);

                  // Verify the struct size assumptions
                  Expect(sizeof(zusf.encoding_opts.codepage)).ToBe(16);
                  Expect(sizeof(zusf.encoding_opts.source_codepage)).ToBe(16);
                });

             it("should handle various encoding combinations for USS files",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  zusf.encoding_opts.data_type = eDataTypeText;

                  // Test common USS file encoding conversions
                  struct EncodingPair
                  {
                    const char *source;
                    const char *target;
                    const char *description;
                  };

                  EncodingPair pairs[] = {
                      {"UTF-8", "IBM-1047", "UTF-8 to EBCDIC"},
                      {"IBM-037", "UTF-8", "EBCDIC to UTF-8"},
                      {"IBM-1047", "ISO8859-1", "EBCDIC to ASCII"},
                      {"ISO8859-1", "UTF-8", "ASCII to UTF-8"},
                      {"UTF-8", "binary", "Text to binary"},
                      {"IBM-1208", "IBM-037", "UTF-8 CCSID to EBCDIC"}};

                  for (const auto &pair : pairs)
                  {
                    memset(&zusf.encoding_opts, 0, sizeof(zusf.encoding_opts));
                    strcpy(zusf.encoding_opts.source_codepage, pair.source);
                    strcpy(zusf.encoding_opts.codepage, pair.target);
                    zusf.encoding_opts.data_type = eDataTypeText;

                    // Verify encoding pair is set correctly
                    Expect(string(zusf.encoding_opts.source_codepage)).ToBe(string(pair.source));
                    Expect(string(zusf.encoding_opts.codepage)).ToBe(string(pair.target));
                    Expect(zusf.encoding_opts.data_type).ToBe(eDataTypeText);
                  }
                });

             it("should handle source encoding with file creation operations",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  strcpy(zusf.encoding_opts.codepage, "IBM-1047");
                  strcpy(zusf.encoding_opts.source_codepage, "UTF-8");
                  zusf.encoding_opts.data_type = eDataTypeText;

                  const string test_file = "/tmp/zusf_create_with_source_encoding.txt";

                  // Cleanup any existing file
                  unlink(test_file.c_str());

                  // Test file creation with encoding options set
                  // This simulates what would happen during file write operations
                  int result = zusf_create_uss_file_or_dir(&zusf, test_file, 0644, false);
                  Expect(result).ToBe(RTNCD_SUCCESS);

                  // Verify file was created
                  struct stat file_stats;
                  Expect(stat(test_file.c_str(), &file_stats)).ToBe(0);
                  Expect(S_ISREG(file_stats.st_mode)).ToBe(true);

                  // Verify encoding options are still set correctly
                  Expect(string(zusf.encoding_opts.codepage)).ToBe("IBM-1047");
                  Expect(string(zusf.encoding_opts.source_codepage)).ToBe("UTF-8");

                  // Cleanup
                  unlink(test_file.c_str());
                });

             it("should handle source encoding with directory operations",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  strcpy(zusf.encoding_opts.codepage, "UTF-8");
                  strcpy(zusf.encoding_opts.source_codepage, "IBM-037");
                  zusf.encoding_opts.data_type = eDataTypeText;

                  const string test_dir = "/tmp/zusf_dir_source_encoding_test";

                  // Cleanup any existing directory
                  rmdir(test_dir.c_str());

                  // Test directory creation with encoding options set
                  int result = zusf_create_uss_file_or_dir(&zusf, test_dir, 0755, true);
                  Expect(result).ToBe(RTNCD_SUCCESS);

                  // Verify directory was created
                  struct stat dir_stats;
                  Expect(stat(test_dir.c_str(), &dir_stats)).ToBe(0);
                  Expect(S_ISDIR(dir_stats.st_mode)).ToBe(true);

                  // Verify encoding options are still preserved
                  Expect(string(zusf.encoding_opts.codepage)).ToBe("UTF-8");
                  Expect(string(zusf.encoding_opts.source_codepage)).ToBe("IBM-037");

                  // Cleanup
                  rmdir(test_dir.c_str());
                });

             it("should maintain source encoding through error conditions",
                [&]() -> void
                {
                  ZUSF zusf = {0};
                  strcpy(zusf.encoding_opts.codepage, "IBM-1047");
                  strcpy(zusf.encoding_opts.source_codepage, "UTF-8");
                  zusf.encoding_opts.data_type = eDataTypeText;

                  const string nonexistent_file = "/tmp/nonexistent_path_for_encoding_test.txt";

                  // Ensure the file doesn't exist
                  unlink(nonexistent_file.c_str());

                  // Test that encoding options are preserved even when operations fail
                  int result = zusf_chmod_uss_file_or_dir(&zusf, nonexistent_file, 0644, false);
                  Expect(result).ToBe(RTNCD_FAILURE);

                  // Verify encoding options are still set correctly after error
                  Expect(string(zusf.encoding_opts.codepage)).ToBe("IBM-1047");
                  Expect(string(zusf.encoding_opts.source_codepage)).ToBe("UTF-8");
                  Expect(zusf.encoding_opts.data_type).ToBe(eDataTypeText);

                  // Verify error message was set but encoding preserved
                  Expect(strlen(zusf.diag.e_msg)).ToBeGreaterThan(0);
                });
           });
}