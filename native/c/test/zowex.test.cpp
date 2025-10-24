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
#include "ztype.h"
#include "zowex.test.hpp"
#include "zutils.hpp"

using namespace std;
using namespace ztst;

const string zowex_command = "./../build-out/zowex";

void zowex_tests()
{

  describe("zowex",
           []() -> void
           {
             it("should list a version of the tool",
                []() -> void
                {
                  int rc = 0;
                  string response;
                  rc = execute_command_with_output(zowex_command + " --version", response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("zowex");
                  Expect(response).ToContain("Version");
                });
             it("should remain less than 10mb in size",
                []() -> void
                {
                  string response;
                  execute_command_with_output("cat ../build-out/zowex | wc -c", response);
                  int file_size = stoi(response);
                  ExpectWithContext(file_size, response).ToBeLessThan(10 * 1024 * 1024);
                });
             describe("job",
                      []() -> void
                      {
                        it("should list jobs",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0); // results might be truncated
                           });

                        it("should list proclib",
                           []()
                           {
                             int rc = 0;
                             string response;
                             rc = execute_command_with_output(zowex_command + " job list-proclib", response);
                             ExpectWithContext(rc, response).ToBeGreaterThanOrEqualTo(0);
                           });

                        it("should submit a job, view it, and delete it", []()
                           {
                           int rc = 0;
                           string stdout_output, stderr_output;

                           string jobname = "IEFBR14$";
                           string jcl = "//" + jobname + " JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";

                           // write jcl to the data set
                           string submit_command = "printf \"" + jcl + "\" | " + zowex_command + " job submit-jcl --only-jobid";
                           rc = execute_command(submit_command, stdout_output, stderr_output);
                           string job_id = TrimChars(stdout_output);
                           ExpectWithContext(rc, stderr_output).ToBe(0);
                           Expect(job_id).Not().ToBe("");

                           // view the job
                           string view_command = zowex_command + " job view-status " + job_id;
                           rc = execute_command(view_command, stdout_output, stderr_output);
                           ExpectWithContext(rc, stderr_output).ToBe(0);
                           Expect(stdout_output).ToContain(jobname);

                           // delete the job
                           string delete_command = zowex_command + " job delete " + job_id;
                           rc = execute_command(delete_command, stdout_output, stderr_output);
                           ExpectWithContext(rc, stderr_output).ToBe(0);
                           Expect(stdout_output).ToContain("deleted"); });
                      });
             describe("data-set",
                      []() -> void
                      {
                        // describe("data set create tests",
                        //          []() -> void
                        //          {
                        //            it("should create a fb data set",
                        //               []()
                        //               {
                        //                 int rc = 0;
                        //                 string user;
                        //                 execute_command_with_output("whoami", user);
                        //                 string data_set = TrimChars(user) + ".temp.temp.temp.temp.temp.temp.tmp";
                        //                 string response;
                        //                 string del_command = zowex_command + " data-set delete " + data_set;
                        //                 execute_command_with_output(del_command, response);
                        //                 string command = zowex_command + " data-set create-fb " + data_set;
                        //                 rc = execute_command_with_output(command, response);
                        //                 ExpectWithContext(rc, response).ToBe(0);
                        //                 execute_command_with_output(del_command, response);
                        //               });
                        //          });
                        describe("list",
                                 []() -> void
                                 {
                                   it("should list a data set",
                                      []()
                                      {
                                        int rc = 0;
                                        string data_set = "SYS1.MACLIB";
                                        string response;
                                        string command = zowex_command + " data-set list " + data_set;
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                      });
                                   it("should list a member of a data set",
                                      []()
                                      {
                                        string data_set = "SYS1.MACLIB";
                                        string response;
                                        string command = zowex_command + " data-set lm " + data_set + " --no-warn --me 1";
                                        // TestLog("Running: " + command);
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                      });
                                   it("should warn when listing members of a data set with many members",
                                      []()
                                      {
                                        int rc = 0;
                                        string data_set = "SYS1.MACLIB";
                                        string response;
                                        string command = zowex_command + " data-set lm " + data_set + " --me 1";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                                      });

                                   it("should list data sets based on pattern and warn about listing too many members",
                                      []()
                                      {
                                        int rc = 0;
                                        string dsn = "SYS1.CMDLIB";
                                        string pattern = "SYS1.*";
                                        string response;
                                        string command = zowex_command + " data-set list " + pattern + " --me 10";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                                        Expect(response).ToContain(dsn);
                                      });
                                 });

                        // describe("data set i/o tests",
                        //          []() -> void
                        //          {
                        //            it("should write and read from a data set",
                        //               []()
                        //               {
                        //                 int rc = 0;
                        //                 string user;
                        //                 execute_command_with_output("whoami", user);
                        //                 string data_set = TrimChars(user) + ".temp.temp.temp.temp.temp.temp.tmp";
                        //                 string member = "IEFBR14";
                        //                 string data_set_member = "\"" + data_set + "(" + member + ")\"";
                        //                 string response;

                        //                 // delete the data set if it exists
                        //                 string del_command = zowex_command + " data-set delete " + data_set;
                        //                 execute_command_with_output(del_command, response);

                        //                 // create the data set
                        //                 string command = zowex_command + " data-set create-fb " + data_set;
                        //                 rc = execute_command_with_output(command, response);
                        //                 ExpectWithContext(rc, response).ToBe(0);

                        //                 string jcl = "//IEFBR14$ JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";

                        //                 // write jcl to the data set
                        //                 string write_command = "printf \"" + jcl + "\" | " + zowex_command + " data-set write " + data_set_member;
                        //                 rc = execute_command_with_output(write_command, response);
                        //                 ExpectWithContext(rc, response).ToBe(0);

                        //                 // read from the data set
                        //                 string read_command = zowex_command + " data-set view " + data_set_member;
                        //                 rc = execute_command_with_output(read_command, response);
                        //                 ExpectWithContext(rc, response).ToBe(0);
                        //                 Expect(TrimChars(response)).ToBe(jcl);

                        //                 // delete the data set
                        //                 execute_command_with_output(del_command, response);
                        //               });
                        //          });
                      });
           });
}
