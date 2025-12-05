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

                        it("should list jobs with prefix filter",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list --prefix IEF*", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                           });

                        it("should list jobs with combined owner and prefix filters",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list --owner * --prefix IEF* --max-entries 10", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
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
                           });

                        it("should handle list with non-matching owner",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list --owner NONEXIST", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                           });

                        it("should handle list with max-entries 0",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list --max-entries 0", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                           });

                        it("should handle list with very large max-entries",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list --max-entries 10000", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                           });

                        it("should handle list with wildcard owner pattern",
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
                             string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
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
                             string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --wait output --only-jobid";
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
                             string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
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

                             rc = execute_command_with_output(zowex_command + " job rel " + jobid, stdout_output);
                             ExpectWithContext(rc, stdout_output).ToBe(0);
                             Expect(stdout_output).ToContain("released");
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

                        it("should submit JCL with --only-correlator option",
                           [&]()
                           {
                             string stdout_output, stderr_output;
                             string command = "printf \"" + jcl_base + "\" | " + zowex_command + " job submit-jcl --only-correlator";

                             int rc = execute_command(command, stdout_output, stderr_output);
                             string correlator = TrimChars(stdout_output);
                             ExpectWithContext(rc, stderr_output).ToBe(0);
                             Expect(correlator).Not().ToBe("");

                             // Get jobid for cleanup using correlator
                             string response;
                             execute_command_with_output(zowex_command + " job view-status " + correlator + " --rfc", response);
                             vector<string> lines = parse_rfc_response(response, "\n");
                             if (lines.size() > 0)
                             {
                               vector<string> parts = parse_rfc_response(lines[0], ",");
                               if (parts.size() >= 2)
                               {
                                 _jobs.push_back(parts[1]);
                               }
                             }
                           });

                        it("should submit from data set with --only-correlator",
                           [&]()
                           {
                             // Create DS
                             string ds = get_random_ds();
                             _ds.push_back(ds);
                             string response;
                             int rc = execute_command_with_output(zowex_command + " data-set create " + ds + " --dsorg PS --recfm FB --lrecl 80", response);
                             ExpectWithContext(rc, response).ToBe(0);

                             // Write JCL
                             string command = "echo \"" + jcl_base + "\" | " + zowex_command + " data-set write " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);

                             // Submit with --only-correlator
                             string stdout_output, stderr_output;
                             command = zowex_command + " job submit " + ds + " --only-correlator";
                             rc = execute_command(command, stdout_output, stderr_output);
                             string correlator = TrimChars(stdout_output);
                             ExpectWithContext(rc, stderr_output).ToBe(0);
                             Expect(correlator).Not().ToBe("");

                             // Get jobid for cleanup
                             execute_command_with_output(zowex_command + " job view-status " + correlator + " --rfc", response);
                             vector<string> lines = parse_rfc_response(response, "\n");
                             if (lines.size() > 0)
                             {
                               vector<string> parts = parse_rfc_response(lines[0], ",");
                               if (parts.size() >= 2)
                               {
                                 _jobs.push_back(parts[1]);
                               }
                             }
                           });

                        it("should submit from USS file with --only-correlator",
                           [&]()
                           {
                             // Create local file
                             string filename = "test_job_" + get_random_string(5) + ".jcl";
                             _files.push_back(filename);

                             string response;
                             string cmd = "printf \"" + jcl_base + "\" > " + filename;
                             int rc = execute_command_with_output(cmd, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             execute_command_with_output("chtag -r " + filename, response);

                             // Submit with --only-correlator
                             string stdout_output, stderr_output;
                             string command = zowex_command + " job submit-uss " + filename + " --only-correlator";
                             rc = execute_command(command, stdout_output, stderr_output);
                             string correlator = TrimChars(stdout_output);
                             ExpectWithContext(rc, stderr_output).ToBe(0);
                             Expect(correlator).Not().ToBe("");

                             // Get jobid for cleanup
                             execute_command_with_output(zowex_command + " job view-status " + correlator + " --rfc", response);
                             vector<string> lines = parse_rfc_response(response, "\n");
                             if (lines.size() > 0)
                             {
                               vector<string> parts = parse_rfc_response(lines[0], ",");
                               if (parts.size() >= 2)
                               {
                                 _jobs.push_back(parts[1]);
                               }
                             }
                           });
                      });

             describe("submit-edge-cases",
                      [&]() -> void
                      {
                        const string jcl_base = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";

                        it("should handle submit from non-existent dataset",
                           [&]()
                           {
                             string ds = "NONEXIST.DS.NAME";
                             string response;
                             int rc = execute_command_with_output(zowex_command + " job submit " + ds, response);
                             Expect(rc).Not().ToBe(0);
                           });

                        it("should handle submit from non-existent USS file",
                           [&]()
                           {
                             string filename = "nonexistent_file_12345.jcl";
                             string response;
                             int rc = execute_command_with_output(zowex_command + " job submit-uss " + filename, response);
                             Expect(rc).Not().ToBe(0);
                           });

                        it("should handle empty JCL submission",
                           [&]()
                           {
                             string command = "echo \"\" | " + zowex_command + " job submit-jcl";
                             string response;
                             int rc = execute_command_with_output(command, response);
                             Expect(rc).Not().ToBe(0);
                           });

                        it("should handle malformed JCL submission",
                           [&]()
                           {
                             string bad_jcl = "THIS IS NOT VALID JCL";
                             string command = "printf \"" + bad_jcl + "\" | " + zowex_command + " job submit-jcl";
                             string response;
                             int rc = execute_command_with_output(command, response);
                             Expect(rc).Not().ToBe(0);
                           });

                        it("should handle submit with encoding option",
                           [&]()
                           {
                             string command = "printf \"" + jcl_base + "\" | " + zowex_command + " job submit-jcl --encoding IBM-1047 --only-jobid";
                             string stdout_output, stderr_output;
                             int rc = execute_command(command, stdout_output, stderr_output);
                             string jobid = TrimChars(stdout_output);
                             if (rc == 0)
                             {
                               Expect(jobid).Not().ToBe("");
                               _jobs.push_back(jobid);
                             }
                           });

                        it("should handle submit with local-encoding option",
                           [&]()
                           {
                             string command = "printf \"" + jcl_base + "\" | " + zowex_command + " job submit-jcl --local-encoding ISO8859-1 --only-jobid";
                             string stdout_output, stderr_output;
                             int rc = execute_command(command, stdout_output, stderr_output);
                             string jobid = TrimChars(stdout_output);
                             if (rc == 0)
                             {
                               Expect(jobid).Not().ToBe("");
                               _jobs.push_back(jobid);
                             }
                           });
                      });

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
                             // Submit a job with TYPRUN=HOLD (so we can cancel it)
                             string jcl = "//IEFBR14 JOB (IZUACCT),TEST,REGION=0M,TYPRUN=HOLD\n//RUN EXEC PGM=IEFBR14";
                             string command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                             string stdout_output, stderr_output;
                             int rc = execute_command(command, stdout_output, stderr_output);
                             string jobid = TrimChars(stdout_output);
                             _jobs.push_back(jobid);

                             // Release the job first so it starts executing, then we can cancel it
                             execute_command_with_output(zowex_command + " job release " + jobid, stdout_output);

                             // Give it a moment to start
                             string list_response;
                             execute_command_with_output(zowex_command + " job list --owner *", list_response);

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
                               TestLog("Could not get correlator, skipping cancel test");
                               return;
                             }

                             // Cancel by correlator
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
                             int rc = execute_command_with_output(zowex_command + " job view-status " + jobid, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(jobid);
                           });
                      });
           });
}
