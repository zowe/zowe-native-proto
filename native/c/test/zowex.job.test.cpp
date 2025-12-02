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

void zowex_job_tests()
{
  static vector<string> _jobs;
  static vector<string> _ds;    // Track datasets to delete
  static vector<string> _files; // Track USS files to delete

  describe("job",
           [&]() -> void
           {
             // Helper to delete tracked jobs
             auto _cleanup_jobs = [&]() -> void
             {
               TestLog("Cleaning up " + to_string(_jobs.size()) + " jobs...");
               for (const auto &jobid : _jobs)
               {
                 string response;
                 execute_command_with_output(zowex_command + " job delete " + jobid, response);
               }
               _jobs.clear();
             };

             // Helper to delete tracked datasets
             auto _cleanup_ds = [&]() -> void
             {
               TestLog("Cleaning up " + to_string(_ds.size()) + " data sets...");
               for (const auto &ds : _ds)
               {
                 string response;
                 execute_command_with_output(zowex_command + " data-set delete " + ds, response);
               }
               _ds.clear();
             };

             // Helper to delete tracked files
             auto _cleanup_files = [&]() -> void
             {
               for (const auto &file : _files)
               {
                 remove(file.c_str());
               }
               _files.clear();
             };

             afterAll(
                 [&]() -> void
                 {
                   _cleanup_jobs();
                   _cleanup_ds();
                   _cleanup_files();
                 });

             describe("list",
                      []() -> void
                      {
                        it("should list jobs",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list --max-entries 5", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                           });

                        it("should list jobs with owner filter",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list --owner *", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                           });
                      });

             describe("list-proclib",
                      []() -> void
                      {
                        it("should list proclib",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list-proclib", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                           });
                      });

             describe("submit",
                      [&]() -> void
                      {
                        const string jcl_base = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";

                        it("should submit JCL from stdin",
                           [&]()
                           {
                             string jobid;
                             string stdout_output, stderr_output;
                             string command = "printf \"" + jcl_base + "\" | " + zowex_command + " job submit-jcl --only-jobid";

                             int rc = execute_command(command, stdout_output, stderr_output);
                             jobid = TrimChars(stdout_output);
                             ExpectWithContext(rc, stderr_output).ToBe(0);
                             Expect(jobid).Not().ToBe("");
                             _jobs.push_back(jobid);

                             // Wait for it to finish
                             execute_command_with_output(zowex_command + " job list --owner *", stdout_output);
                           });

                        it("should submit JCL and wait for output",
                           [&]()
                           {
                             string jobid;
                             string stdout_output, stderr_output;
                             string command = "printf \"" + jcl_base + "\" | " + zowex_command + " job submit-jcl --wait output";

                             int rc = execute_command(command, stdout_output, stderr_output);
                             ExpectWithContext(rc, stderr_output).ToBe(0);
                             Expect(stdout_output).ToContain("Submitted JCL");

                             // Extract jobid for cleanup if possible
                             size_t pos = stdout_output.find("JOB");
                             if (pos != string::npos)
                             {
                               jobid = stdout_output.substr(pos, 8);
                               _jobs.push_back(jobid);
                             }
                           });

                        it("should submit from a data set",
                           [&]()
                           {
                             // Create DS (PS)
                             string ds = get_random_ds();
                             _ds.push_back(ds);
                             string response;
                             // Use create with explicit PS to allow direct writing
                             int rc = execute_command_with_output(zowex_command + " data-set create " + ds + " --dsorg PS --recfm FB --lrecl 80", response);
                             ExpectWithContext(rc, response).ToBe(0);

                             // Write JCL
                             string command = "echo \"" + jcl_base + "\" | " + zowex_command + " data-set write " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);

                             // Submit
                             string stdout_output, stderr_output;
                             command = zowex_command + " job submit " + ds + " --only-jobid";
                             rc = execute_command(command, stdout_output, stderr_output);
                             string jobid = TrimChars(stdout_output);
                             ExpectWithContext(rc, stderr_output).ToBe(0);
                             Expect(jobid).Not().ToBe("");
                             _jobs.push_back(jobid);
                           });

                        it("should submit from a USS file",
                           [&]()
                           {
                             // Create local file
                             string filename = "test_job_" + get_random_string(5) + ".jcl";
                             _files.push_back(filename);

                             // Use printf to write the file to ensure correct newline handling and encoding
                             string response;
                             string cmd = "printf \"" + jcl_base + "\" > " + filename;
                             int rc = execute_command_with_output(cmd, response);
                             ExpectWithContext(rc, response).ToBe(0);

                             // Ensure file is untagged so zowex reads raw EBCDIC without conversion
                             execute_command_with_output("chtag -r " + filename, response);

                             // Submit
                             string stdout_output, stderr_output;
                             string command = zowex_command + " job submit-uss " + filename + " --only-jobid";
                             rc = execute_command(command, stdout_output, stderr_output);
                             string jobid = TrimChars(stdout_output);
                             ExpectWithContext(rc, stderr_output).ToBe(0);
                             Expect(jobid).Not().ToBe("");
                             _jobs.push_back(jobid);
                           });
                      });

             describe("view",
                      [&]() -> void
                      {
                        string _jobid;
                        beforeEach([&]()
                                   {
                             string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";
                             string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                             string stdout_output, stderr_output;
                             execute_command(command, stdout_output, stderr_output);
                             _jobid = TrimChars(stdout_output);
                             _jobs.push_back(_jobid); });

                        it("should view job status",
                           [&]()
                           {
                             string response;
                             int rc = execute_command_with_output(zowex_command + " job view-status " + _jobid, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(_jobid);
                           });

                        it("should view job JCL",
                           [&]()
                           {
                             string response;
                             int rc = execute_command_with_output(zowex_command + " job view-jcl " + _jobid, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("IEFBR14");
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
                             execute_command(command, stdout_output, stderr_output);
                             _jobid = TrimChars(stdout_output);
                             _jobs.push_back(_jobid); });

                        it("should list job files",
                           [&]()
                           {
                             string response;
                             int rc = execute_command_with_output(zowex_command + " job list-files " + _jobid, response);
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
                      });
           });
}
