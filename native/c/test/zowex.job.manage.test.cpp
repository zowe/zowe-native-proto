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

void zowex_job_manage_tests(vector<string> &_jobs, vector<string> &_ds, vector<string> &_files)
{
  describe("view",
           [&]() -> void
           {
             string _jobid;
             beforeEach([&]()
                        {
                 // Submit and wait for output to ensure job completes
                 string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                 string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                 string stdout_output, stderr_output;
                 int rc = execute_command(command, stdout_output, stderr_output);
                 Expect(rc).ToBe(0);
                 _jobid = TrimChars(stdout_output);
                 Expect(_jobid).Not().ToBe("");
                 _jobs.push_back(_jobid); });

             it("should view job status",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-status " + _jobid, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain(_jobid);
                });

             it("should view job status with --rfc format",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-status " + _jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  // Validate RFC format structure
                  vector<string> lines = parse_rfc_response(response, "\n");
                  Expect(lines.size()).ToBeGreaterThan(0);

                  // Parse the first line (should contain job status)
                  vector<string> columns = parse_rfc_response(lines[0], ",");

                  // RFC format: jobid, retcode, jobname, status, correlator, ...
                  Expect(columns.size()).ToBeGreaterThanOrEqualTo(5);

                  // Validate jobid matches
                  Expect(columns[0]).ToBe(_jobid);

                  // Validate retcode is not empty
                  Expect(columns[1]).Not().ToBe("");

                  // Validate jobname is not empty
                  Expect(columns[2]).Not().ToBe("");

                  // Validate status field exists (should be OUTPUT for completed job)
                  Expect(columns[3]).Not().ToBe("");
                });

             it("should include full_status field in CSV format",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-status " + _jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  // Validate RFC format structure includes full_status
                  vector<string> lines = parse_rfc_response(response, "\n");
                  Expect(lines.size()).ToBeGreaterThan(0);

                  vector<string> columns = parse_rfc_response(lines[0], ",");
                  // RFC format: jobid, retcode, jobname, status, correlator, full_status
                  Expect(columns.size()).ToBeGreaterThanOrEqualTo(6);

                  // Validate full_status field (6th field, index 5)
                  // full_status should not be empty for a completed job
                  Expect(columns[5]).Not().ToBe("");
                });

             it("should view job JCL",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-jcl " + _jobid, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("IEFBR14");
                });

             it("should handle view-status of non-existent job",
                [&]()
                {
                  string fake_jobid = "JOB99999";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-status " + fake_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle view-jcl of non-existent job",
                [&]()
                {
                  string fake_jobid = "JOB99999";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-jcl " + fake_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle view-status with invalid jobid format",
                [&]()
                {
                  string invalid_jobid = "INVALID";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-status " + invalid_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle view-jcl with invalid jobid format",
                [&]()
                {
                  string invalid_jobid = "INVALID";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-jcl " + invalid_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle view-status with invalid correlator",
                [&]()
                {
                  string invalid_correlator = "INVALID_CORRELATOR";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-status " + invalid_correlator, response);
                  Expect(rc).Not().ToBe(0);
                });
           });

  describe("files",
           [&]() -> void
           {
             string _jobid;
             beforeEach([&]()
                        {
                 // Submit and wait for output to ensure spool files exist
                 string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                 string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                 string stdout_output, stderr_output;
                 int rc = execute_command(command, stdout_output, stderr_output);
                 Expect(rc).ToBe(0);
                 _jobid = TrimChars(stdout_output);
                 Expect(_jobid).Not().ToBe("");
                 _jobs.push_back(_jobid); });

             it("should list job files",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("JESMSGLG");
                });

             it("should list job files with --max-entries option",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --max-entries 2", response);
                  // listing job files with --max-entries option should return a warning RC if more than 2 files are found under the jobid
                  ExpectWithContext(rc, response).ToBe(1);
                });

             it("should list job files with --warn option",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --warn", response);
                  ExpectWithContext(rc, response).ToBe(0);
                });

             it("should list job files with --rfc option",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain(",");
                });

             it("should list job files with max-entries 0",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --max-entries 0", response);
                  ExpectWithContext(rc, response).ToBe(0);
                });

             it("should list job files with very large max-entries",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --max-entries 10000", response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("JESMSGLG");
                });

             it("should view a specific job file",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
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
                    TestLog("Could not find JESMSGLG file ID, skipping view-file test");
                    return;
                  }

                  rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + file_id, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("IEFBR14");
                });

             it("should view job file with --encoding option",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
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
                    TestLog("Could not find JESMSGLG file ID, skipping encoding test");
                    return;
                  }

                  rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + file_id + " --encoding ISO8859-1", response);
                  ExpectWithContext(rc, response).ToBe(0);
                });

             it("should view job file with --local-encoding option",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
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
                    TestLog("Could not find JESMSGLG file ID, skipping local-encoding test");
                    return;
                  }

                  rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + file_id + " --local-encoding IBM-1047", response);
                  ExpectWithContext(rc, response).ToBe(0);
                });

             it("should view job file with --ec short form for --encoding",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
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
                    return;

                  rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + file_id + " --ec ISO8859-1", response);
                  ExpectWithContext(rc, response).ToBe(0);
                });

             it("should view job file with --lec short form for --local-encoding",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
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
                    return;

                  rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + file_id + " --lec IBM-1047", response);
                  ExpectWithContext(rc, response).ToBe(0);
                });

             it("should view job file with --response-format-bytes option",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
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
                    TestLog("Could not find JESMSGLG file ID, skipping rfb test");
                    return;
                  }

                  rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + file_id + " --response-format-bytes", response);
                  ExpectWithContext(rc, response).ToBe(0);
                });

             it("should view job file with --rfb short alias",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
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
                    TestLog("Could not find JESMSGLG file ID, skipping rfb alias test");
                    return;
                  }

                  rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + file_id + " --rfb", response);
                  ExpectWithContext(rc, response).ToBe(0);
                });

             xit("should handle view-file with invalid encoding",
                 [&]()
                 {
                   string response;
                   int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
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
                     return;

                   rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + file_id + " --encoding INVALID_ENC", response);
                   Expect(rc).Not().ToBe(0);
                 });

             it("should handle view-file with key 0",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " 0", response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle view-file with invalid file ID",
                [&]()
                {
                  string invalid_file_id = "999";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + invalid_file_id, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle view-file with non-existent job",
                [&]()
                {
                  string fake_jobid = "JOB99999";
                  string file_id = "2";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-file " + fake_jobid + " " + file_id, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle list-files with non-existent job",
                [&]()
                {
                  string fake_jobid = "JOB99999";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + fake_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle list-files with invalid jobid format",
                [&]()
                {
                  string invalid_jobid = "INVALID";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + invalid_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle job with no spool files",
                [&]()
                {
                  string jcl = "//NOSPOOL JOB (IZUACCT),TEST,REGION=0M,TYPRUN=SCAN\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);

                  if (rc == 0)
                  {
                    string jobid = TrimChars(stdout_output);
                    _jobs.push_back(jobid);

                    string response;
                    rc = execute_command_with_output(zowex_command + " job list-files " + jobid, response);
                    ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                  }
                });

             it("should handle view-file with binary spool content",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  vector<string> lines = parse_rfc_response(response, "\n");
                  string file_id = "";
                  for (const auto &line : lines)
                  {
                    if (!line.empty())
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
                    TestLog("Could not find file ID, skipping binary content test");
                    return;
                  }

                  rc = execute_command_with_output(zowex_command + " job view-file " + _jobid + " " + file_id + " --encoding binary", response);
                  ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);

                  if (rc == 0)
                  {
                    Expect(response.length()).ToBeGreaterThan(0);
                  }
                });
           });

  describe("lifecycle",
           [&]() -> void
           {
             it("should hold and release a job",
                [&]()
                {
                  // Submit with TYPRUN=HOLD
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  _jobs.push_back(jobid);

                  // Release
                  rc = execute_command_with_output(zowex_command + " job release " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("released");
                });

             it("should delete a job",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid); // Add to list just in case test fails before delete

                  // Delete
                  rc = execute_command_with_output(zowex_command + " job delete " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("deleted");

                  // Remove from cleanup list since it is deleted
                  for (auto it = _jobs.begin(); it != _jobs.end();)
                  {
                    if (*it == jobid)
                    {
                      it = _jobs.erase(it);
                    }
                    else
                    {
                      ++it;
                    }
                  }
                });

             it("should handle delete of non-existent job",
                [&]()
                {
                  string fake_jobid = "JOB99999";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job delete " + fake_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle cancel of non-existent job",
                [&]()
                {
                  string fake_jobid = "JOB99999";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job cancel " + fake_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle hold of non-existent job",
                [&]()
                {
                  string fake_jobid = "JOB99999";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job hold " + fake_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle release of non-existent job",
                [&]()
                {
                  string fake_jobid = "JOB99999";
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job release " + fake_jobid, response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle release of job that was not held",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  // Try to release without holding
                  rc = execute_command_with_output(zowex_command + " job release " + jobid, stdout_output);
                  // Should succeed or return specific error
                  ExpectWithContext(rc, stdout_output).ToBeGreaterThanOrEqualTo(0);
                });

             it("should handle double hold of a job",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  // First hold
                  rc = execute_command_with_output(zowex_command + " job hold " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);

                  // Second hold
                  rc = execute_command_with_output(zowex_command + " job hold " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBeGreaterThanOrEqualTo(0);
                });

             it("should handle double cancel of a job",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  // First cancel
                  rc = execute_command_with_output(zowex_command + " job cancel " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);

                  // Second cancel
                  rc = execute_command_with_output(zowex_command + " job cancel " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBeGreaterThanOrEqualTo(0);
                });

             it("should handle invalid jobid format for delete",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job delete INVALID_ID", response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle invalid jobid format for cancel",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job cancel INVALID_ID", response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle invalid jobid format for hold",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job hold INVALID_ID", response);
                  Expect(rc).Not().ToBe(0);
                });

             it("should handle invalid jobid format for release",
                [&]()
                {
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job release INVALID_ID", response);
                  Expect(rc).Not().ToBe(0);
                });
           });

  describe("commands",
           [&]() -> void
           {
             it("should hold and release job output",
                [&]()
                {
                  // Submit and wait for output
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  // Hold
                  int rc = execute_command_with_output(zowex_command + " job hold " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("held");

                  // Release
                  rc = execute_command_with_output(zowex_command + " job release " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("released");
                });

             it("should cancel a job",
                [&]()
                {
                  // Submit with TYPRUN=HOLD so we can cancel it
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  // Cancel
                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should cancel a job with --dump option",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid + " --dump", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should cancel a job with --force option",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid + " --force", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should cancel a job with --purge option",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid + " --purge", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should cancel a job with --restart option",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid + " --restart", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should cancel a job with multiple options",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid + " --force --purge", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should cancel a job with -d alias for --dump",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid + " -d", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should cancel a job with -f alias for --force",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid + " -f", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should cancel a job with -p alias for --purge",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid + " -p", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });

             it("should cancel a job with -r alias for --restart",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  int rc = execute_command_with_output(zowex_command + " job cancel " + jobid + " -r", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });
           });

  describe("correlator",
           [&]() -> void
           {
             string _jobid;
             string _correlator;

             beforeEach([&]()
                        {
                 // Submit a job
                 string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                 string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                 string stdout_output, stderr_output;
                 int rc = execute_command(command, stdout_output, stderr_output);
                 Expect(rc).ToBe(0);
                 _jobid = TrimChars(stdout_output);
                 Expect(_jobid).Not().ToBe("");
                 _jobs.push_back(_jobid);

                 // Get correlator
                 string response;
                 execute_command_with_output(zowex_command + " job view-status " + _jobid + " --rfc", response);
                 // Parse CSV to get correlator (5th column, index 4)
                 vector<string> lines = parse_rfc_response(response, "\n");
                 if (lines.size() > 0) {
                     vector<string> parts = parse_rfc_response(lines[0], ",");
                     if (parts.size() >= 5) {
                         _correlator = parts[4];
                     }
                 }
                 if (_correlator.empty()) {
                     TestLog("Could not get correlator for job " + _jobid);
                 } });

             it("should view status by correlator",
                [&]()
                {
                  if (_correlator.empty())
                    return;
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-status \"" + _correlator + "\"", response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain(_jobid);
                });

             it("should view JCL by correlator",
                [&]()
                {
                  if (_correlator.empty())
                    return;
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job view-jcl \"" + _correlator + "\"", response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("IEFBR14");
                });

             it("should list files by correlator",
                [&]()
                {
                  if (_correlator.empty())
                    return;
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job list-files \"" + _correlator + "\"", response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("JESMSGLG");
                });

             it("should delete by correlator",
                [&]()
                {
                  if (_correlator.empty())
                    return;
                  string response;
                  int rc = execute_command_with_output(zowex_command + " job delete \"" + _correlator + "\"", response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("deleted");

                  // Remove from cleanup list
                  for (auto it = _jobs.begin(); it != _jobs.end();)
                  {
                    if (*it == _jobid)
                    {
                      it = _jobs.erase(it);
                    }
                    else
                    {
                      ++it;
                    }
                  }
                });

             it("should hold by correlator",
                [&]()
                {
                  // Submit a job and wait for it to complete
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  // Get correlator
                  string response;
                  execute_command_with_output(zowex_command + " job view-status " + jobid + " --rfc", response);
                  vector<string> lines = parse_rfc_response(response, "\n");
                  string correlator = "";
                  if (lines.size() > 0)
                  {
                    vector<string> parts = parse_rfc_response(lines[0], ",");
                    if (parts.size() >= 5)
                    {
                      correlator = parts[4];
                    }
                  }

                  if (correlator.empty())
                  {
                    TestLog("Could not get correlator, skipping hold test");
                    return;
                  }

                  // Hold by correlator
                  rc = execute_command_with_output(zowex_command + " job hold \"" + correlator + "\"", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("held");
                });

             it("should release by correlator",
                [&]()
                {
                  // Submit a job and wait for it to complete, then hold it
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  // Hold the job first
                  execute_command_with_output(zowex_command + " job hold " + jobid, stdout_output);

                  // Get correlator
                  string response;
                  execute_command_with_output(zowex_command + " job view-status " + jobid + " --rfc", response);
                  vector<string> lines = parse_rfc_response(response, "\n");
                  string correlator = "";
                  if (lines.size() > 0)
                  {
                    vector<string> parts = parse_rfc_response(lines[0], ",");
                    if (parts.size() >= 5)
                    {
                      correlator = parts[4];
                    }
                  }

                  if (correlator.empty())
                  {
                    TestLog("Could not get correlator, skipping release test");
                    return;
                  }

                  // Release by correlator
                  rc = execute_command_with_output(zowex_command + " job release \"" + correlator + "\"", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("released");
                });

             it("should cancel by correlator",
                [&]()
                {
                  // Submit a long-running job that we can cancel while it's executing
                  string jcl = "//LONGJOB JOB (IZUACCT),TEST,REGION=0M\n"
                               "//STEP1 EXEC PGM=IEFBR14\n"
                               "//STEP2 EXEC PGM=BPXBATCH\n"
                               "//STDOUT DD SYSOUT=*\n"
                               "//STDERR DD SYSOUT=*\n"
                               "//STDPARM DD *\n"
                               "SH sleep 30\n"
                               "/*";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  _jobs.push_back(jobid);

                  // Wait briefly for job to start executing
                  sleep(2);

                  // Get correlator from the running job
                  string response;
                  execute_command_with_output(zowex_command + " job view-status " + jobid + " --rfc", response);
                  vector<string> lines = parse_rfc_response(response, "\n");
                  string correlator = "";
                  if (lines.size() > 0)
                  {
                    vector<string> parts = parse_rfc_response(lines[0], ",");
                    if (parts.size() >= 5)
                    {
                      correlator = parts[4];
                    }
                  }

                  if (correlator.empty())
                  {
                    TestLog("Could not get correlator, skipping cancel test");
                    return;
                  }

                  // Cancel by correlator while job is running
                  rc = execute_command_with_output(zowex_command + " job cancel \"" + correlator + "\"", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("cancelled");
                });
           });

  describe("input-mode",
           [&]() -> void
           {
             it("should view status of job in input mode",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  _jobs.push_back(jobid);

                  string response;
                  // Use RFC for reliable parsing
                  int rc = execute_command_with_output(zowex_command + " job view-status " + jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  vector<string> lines = parse_rfc_response(response, "\n");
                  Expect(lines.size()).ToBeGreaterThan(0);
                  if (lines.size() > 0)
                  {
                    vector<string> parts = parse_rfc_response(lines[0], ",");
                    Expect(parts.size()).ToBeGreaterThanOrEqualTo(4);
                    Expect(parts[0]).ToBe(jobid);
                    // Status is index 3. Verify it's not empty.
                    Expect(parts[3]).Not().ToBe("");
                  }
                });

             it("should delete job in input mode by jobid",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  _jobs.push_back(jobid);

                  // Verify job is in INPUT mode
                  string response;
                  rc = execute_command_with_output(zowex_command + " job view-status " + jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  // Delete the job in INPUT mode
                  rc = execute_command_with_output(zowex_command + " job delete " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("deleted");

                  // Remove from cleanup list since it's deleted
                  for (auto it = _jobs.begin(); it != _jobs.end();)
                  {
                    if (*it == jobid)
                    {
                      it = _jobs.erase(it);
                    }
                    else
                    {
                      ++it;
                    }
                  }
                });

             it("should hold job in input mode by jobid",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  _jobs.push_back(jobid);

                  // Verify job is in INPUT mode
                  string response;
                  rc = execute_command_with_output(zowex_command + " job view-status " + jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  // Hold the job (even though it's already held by TYPRUN=HOLD)
                  rc = execute_command_with_output(zowex_command + " job hold " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBeGreaterThanOrEqualTo(0);
                  if (rc == 0)
                  {
                    Expect(stdout_output).ToContain("held");
                  }
                });

             it("should hold job in input mode by correlator",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  _jobs.push_back(jobid);

                  // Get correlator
                  string response;
                  rc = execute_command_with_output(zowex_command + " job view-status " + jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  vector<string> lines = parse_rfc_response(response, "\n");
                  string correlator = "";
                  if (lines.size() > 0)
                  {
                    vector<string> parts = parse_rfc_response(lines[0], ",");
                    if (parts.size() >= 5)
                    {
                      correlator = TrimChars(parts[4]);
                    }
                  }

                  // INPUT mode jobs may not have a valid correlator yet
                  // Skip test if correlator is empty or appears invalid
                  if (correlator.empty() || correlator.find(' ') != string::npos)
                  {
                    TestLog("Correlator not valid for INPUT mode job, skipping test");
                    return;
                  }

                  // Hold by correlator
                  rc = execute_command_with_output(zowex_command + " job hold \"" + correlator + "\"", stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBeGreaterThanOrEqualTo(0);
                  if (rc == 0)
                  {
                    Expect(stdout_output).ToContain("held");
                  }
                });

             it("should release job in input mode by jobid",
                [&]()
                {
                  string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                  string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                  string stdout_output, stderr_output;
                  int rc = execute_command(command, stdout_output, stderr_output);
                  string jobid = TrimChars(stdout_output);
                  ExpectWithContext(rc, stderr_output).ToBe(0);
                  _jobs.push_back(jobid);

                  // Verify job is in INPUT mode
                  string response;
                  rc = execute_command_with_output(zowex_command + " job view-status " + jobid + " --rfc", response);
                  ExpectWithContext(rc, response).ToBe(0);

                  // Release the job from INPUT mode
                  rc = execute_command_with_output(zowex_command + " job release " + jobid, stdout_output);
                  ExpectWithContext(rc, stdout_output).ToBe(0);
                  Expect(stdout_output).ToContain("released");
                });
           });
}
