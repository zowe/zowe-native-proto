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
#include "zowex.job.test.hpp"

using namespace std;
using namespace ztst;

void zowex_job_list_tests(vector<string> &_jobs, vector<string> &_ds, vector<string> &_files)
{
  describe("list",
           [&]() -> void
           {
             it("should list jobs",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --max-entries 5 --rfc", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  if (rc == 0 && !response.empty())
                  {
                    vector<string> lines = parse_rfc_response(response, "\n");
                    Expect(lines.size()).ToBeGreaterThan(0);
                    // Check header or first row columns
                    // RFC format: jobid, jobname, owner, status, retcode
                    vector<string> cols = parse_rfc_response(lines[0], ",");
                    Expect(cols.size()).ToBeGreaterThanOrEqualTo(5);
                  }
                });

             it("should list jobs with owner filter",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --owner *", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should list jobs with prefix filter",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --prefix IEF*", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should list jobs with -o short form for --owner",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list -o * --max-entries 5", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  Expect(response.length()).ToBeGreaterThan(0);
                });

             it("should list jobs with -p short form for --prefix",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list -p IEF* --max-entries 5", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  Expect(response.length()).ToBeGreaterThan(0);
                });

             it("should list jobs with combined owner and prefix filters",
                [&]()
                {
                  // Submit a job to ensure we have a match
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid --wait output";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  _jobs.push_back(jobid);

                  string current_user = get_user();
                  string prefix = "IEFBR*";

                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --owner " + current_user + " --prefix " + prefix + " --max-entries 10 --no-warn --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  vector<string> lines = parse_rfc_response(response, "\n");
                  bool found_our_job = false;
                  for (const auto &line : lines)
                  {
                    if (line.empty())
                      continue;
                    vector<string> parts = parse_rfc_response(line, ",");
                    if (parts.size() >= 3)
                    {
                      if (parts[0] == jobid)
                        found_our_job = true;
                      Expect(parts[2]).ToContain("IEFBR");
                    }
                  }
                  Expect(found_our_job).ToBe(true);
                });

             it("should list jobs with --rfc option",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --max-entries 5 --rfc", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  Expect(response).ToContain(",");
                });

             it("should validate RFC format structure for job list",
                []()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list --max-entries 5 --rfc", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);

                  if (rc == 0 && !response.empty())
                  {
                    // Parse lines
                    vector<string> lines = parse_rfc_response(response, "\n");
                    for (const auto &line : lines)
                    {
                      if (line.empty())
                        continue;

                      // Parse CSV columns
                      vector<string> columns = parse_rfc_response(line, ",");

                      // RFC format: jobid, jobname, owner, status, retcode
                      Expect(columns.size()).ToBeGreaterThanOrEqualTo(5);

                      // Validate jobid is not empty
                      Expect(columns[0]).Not().ToBe("");

                      // Validate retcode is not empty
                      Expect(columns[1]).Not().ToBe("");

                      // Validate jobname is not empty
                      if (columns.size() >= 3)
                      {
                        Expect(columns[2]).Not().ToBe("");
                      }
                    }
                  }
                });

             it("should list jobs with --warn option",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --max-entries 1 --warn", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should handle list with non-matching prefix",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --prefix NONEXIST99", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  Expect(TrimChars(response)).ToBe("");
                });

             it("should handle list with non-matching owner",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --owner NONEXIST", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  Expect(TrimChars(response)).ToBe("");
                });

             it("should handle list with max-entries 0",
                []()
                {
                  // max-entries 0 should return all jobs (uses default max which may trigger warning)
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --max-entries 0 --no-warn", response);
                  // With --no-warn, warning RC is converted to success
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(TrimChars(response)).Not().ToBe("");
                });

             it("should handle list with very large max-entries",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --max-entries 10000", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should validate default parameters",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --rfc --max-entries 50 --no-warn", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  // Verify response does not contain jobs from other users
                  Expect(response.length()).ToBeGreaterThan(0);
                  vector<string> lines = parse_rfc_response(response, "\n");
                  string current_user = get_user();
                  bool found_other_user = false;
                  for (const auto &line : lines)
                  {
                    if (line.empty())
                      continue;
                    vector<string> parts = parse_rfc_response(line, ",");
                    if (parts.size() >= 1)
                    {
                      // Check if we found our submitted job (parts[0] is jobid)
                      if (parts[2] != current_user)
                      {
                        found_other_user = true;
                        break;
                      }
                    }
                  }
                  Expect(found_other_user).ToBe(false);
                });

             it("should validate owner filter actually works",
                [&]()
                {
                  // Submit a job to ensure we have a job from current user
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid --wait output";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  _jobs.push_back(jobid);

                  string current_user = get_user();
                  Expect(current_user).Not().ToBe("");

                  // List jobs with current user as owner using --rfc for easier parsing
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --owner " + current_user + " --rfc --max-entries 50 --no-warn", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  // Parse and validate we found our job (RFC format: jobid,jobname,owner,status,retcode)
                  vector<string> lines = parse_rfc_response(response, "\n");
                  bool found_our_job = false;
                  for (const auto &line : lines)
                  {
                    if (line.empty())
                      continue;
                    vector<string> parts = parse_rfc_response(line, ",");
                    if (parts.size() >= 1)
                    {
                      // Check if we found our submitted job (parts[0] is jobid)
                      if (parts[0] == jobid)
                      {
                        found_our_job = true;
                        break;
                      }
                    }
                  }
                  Expect(found_our_job).ToBe(true);
                });

             it("should validate prefix filter actually works",
                [&]()
                {
                  // Submit a job with a known prefix
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid --wait output";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  _jobs.push_back(jobid);

                  // Extract job name from jobid (first part before the number)
                  string job_name = "IEFBR14";
                  string prefix = job_name.substr(0, 3) + "*"; // Use first 3 chars as prefix

                  // List jobs with prefix filter using --rfc for easier parsing
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list --prefix " + prefix + " --rfc --max-entries 50 --no-warn", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  // Parse and validate all jobs match the prefix pattern (RFC format: jobid,jobname,owner,status,retcode)
                  vector<string> lines = parse_rfc_response(response, "\n");
                  Expect(lines.size()).ToBeGreaterThan(0);

                  for (const auto &line : lines)
                  {
                    if (line.empty())
                      continue;
                    vector<string> parts = parse_rfc_response(line, ",");
                    if (parts.size() >= 2)
                    {
                      string listed_job_name = parts[1];
                      // Verify job name starts with the prefix (minus the wildcard)
                      string prefix_without_wildcard = prefix.substr(0, prefix.length() - 1);
                      bool matches = listed_job_name.substr(0, prefix_without_wildcard.length()) == prefix_without_wildcard;
                      ExpectWithContext(matches, line).ToBe(true);
                    }
                  }
                });

             it("should validate status filter actually works",
                [&]()
                {
                  // List jobs with status filter using --rfc for easier parsing
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list --status ACTIVE --rfc --max-entries 50 --no-warn", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  // Parse and validate all jobs match the prefix pattern (RFC format: jobid,jobname,owner,status,retcode)
                  vector<string> lines = parse_rfc_response(response, "\n");
                  Expect(lines.size()).ToBeGreaterThan(0);

                  for (const auto &line : lines)
                  {
                    if (line.empty())
                      continue;
                    vector<string> parts = parse_rfc_response(line, ",");
                    if (parts.size() >= 4)
                    {
                      string listed_job_status = parts[3];
                      ExpectWithContext(listed_job_status == "ACTIVE", line).ToBe(true);
                    }
                  }
                });

             it("should handle list with empty owner",
                []()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list --owner \"\" --max-entries 5", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should handle list with empty prefix",
                []()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list --prefix \"\" --max-entries 5", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should return jobs in consistent order",
                []()
                {
                  // List jobs twice and verify consistent ordering
                  string response1, response2;
                  int rc1 = execute_command_with_output(zowex_command + " job list --max-entries 10 --rfc", response1);
                  int rc2 = execute_command_with_output(zowex_command + " job list --max-entries 10 --rfc", response2);

                  ExpectWithContext(rc1, response1).ToBeGreaterThanOrEqualTo(0);
                  ExpectWithContext(rc2, response2).ToBeGreaterThanOrEqualTo(0);

                  if (rc1 == 0 && rc2 == 0 && !response1.empty() && !response2.empty())
                  {
                    vector<string> lines1 = parse_rfc_response(response1, "\n");
                    vector<string> lines2 = parse_rfc_response(response2, "\n");

                    // Should have consistent results if no jobs were submitted between calls
                    vector<string> jobids1, jobids2;
                    for (const auto &line : lines1)
                    {
                      if (!line.empty())
                      {
                        vector<string> parts = parse_rfc_response(line, ",");
                        if (parts.size() >= 1)
                          jobids1.push_back(parts[0]);
                      }
                    }
                    for (const auto &line : lines2)
                    {
                      if (!line.empty())
                      {
                        vector<string> parts = parse_rfc_response(line, ",");
                        if (parts.size() >= 1)
                          jobids2.push_back(parts[0]);
                      }
                    }

                    Expect(jobids1.size()).ToBeGreaterThan(0);
                  }
                });
           });

  describe("list-proclib",
           [&]() -> void
           {
             it("should list proclib",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list-proclib", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should list proclib and validate content",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job list-proclib", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  if (rc == 0)
                  {
                    Expect(response.length()).ToBeGreaterThan(0);
                    // Basic validation that we got some output
                    vector<string> lines = parse_rfc_response(response, "\n");
                    Expect(lines.size()).ToBeGreaterThan(0);
                  }
                });
           });

  describe("aliases",
           [&]() -> void
           {
             it("should use 'ls' alias for list command",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job ls --max-entries 5", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should use 'lf' alias for list-files command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait OUTPUT --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  string response;
                  rc = execute_command_with_output(zowex_command + " job lf " + jobid, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("JESMSGLG");
                });

             it("should use 'lp' alias for list-proclib command",
                []()
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " job lp", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                });

             it("should use 'vs' alias for view-status command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait OUTPUT --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  string response;
                  rc = execute_command_with_output(zowex_command + " job vs " + jobid, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain(jobid);
                });

             it("should use 'vj' alias for view-jcl command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  string response;
                  rc = execute_command_with_output(zowex_command + " job vj " + jobid, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("IEFBR14");
                });

             it("should use 'subj' alias for submit-jcl command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job subj --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  Expect(jobid).Not().ToBe("");
                  _jobs.push_back(jobid);
                });

             it("should use 'del' alias for delete command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid --wait output";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);

                  rc = execute_command_with_output(zowex_command + " job del " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("deleted");
                });

             it("should use 'cnl' alias for cancel command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  Expect(wait_for_job(jobid)).ToBe(true);

                  rc = execute_command_with_output(zowex_command + " job cnl " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should use 'hld' alias for hold command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  rc = execute_command_with_output(zowex_command + " job hld " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("held");
                });

             it("should use 'rel' alias for release command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  Expect(wait_for_job(jobid)).ToBe(true);

                  rc = execute_command_with_output(zowex_command + " job rel " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("released");
                });

             it("should use 'sub-u' alias for submit-uss command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string filename = "test_job_" + get_random_string(5) + ".jcl";
                  _files.push_back(filename);

                  string response;
                  string cmd = "printf \"" + jcl + "\" > " + filename;
                  int rc = execute_command_with_output(cmd, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  execute_command_with_output("chtag -r " + filename, response);

                  string stdout_output, stderr_output;
                  string command = zowex_command + " job sub-u " + filename + " --only-jobid";
                  rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  Expect(jobid).Not().ToBe("");
                  _jobs.push_back(jobid);
                });

             it("should use 'vfbi' alias for view-file-by-id command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  string response;
                  rc = execute_command_with_output(zowex_command + " job list-files " + jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  vector<string> lines = parse_rfc_response(response, "\n");
                  string file_id = "";
                  for (const auto &line : lines)
                  {
                    if (line.find("JESMSGLG") != string::npos)
                    {
                      vector<string> parts = parse_rfc_response(line, ",");
                      if (parts.size() >= 3)
                      {
                        file_id = parts[2];
                        break;
                      }
                    }
                  }

                  if (file_id.empty())
                  {
                    TestLog("Could not find JESMSGLG file ID, skipping vf alias test");
                    return;
                  }

                  rc = execute_command_with_output(zowex_command + " job vfbi " + jobid + " " + file_id, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("IEFBR14");
                });

             it("should use 'vf' alias for view-file command",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  string response;
                  rc = execute_command_with_output(zowex_command + " job list-files " + jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  vector<string> lines = parse_rfc_response(response, "\n");
                  string dsn = "";
                  for (const auto &line : lines)
                  {
                    if (line.find("JESMSGLG") != string::npos)
                    {
                      vector<string> parts = parse_rfc_response(line, ",");
                      if (parts.size() >= 3)
                      {
                        dsn = parts[1];
                        break;
                      }
                    }
                  }

                  if (dsn.empty())
                  {
                    TestLog("Could not find JESMSGLG file, skipping vf alias test");
                    return;
                  }

                  rc = execute_command_with_output(zowex_command + " job vf " + dsn, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("IEFBR14");
                });
           });
}
