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
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <vector>
#include "ztest.hpp"
#include "ztype.h"
#include "zowex.test.hpp"
#include "zutils.hpp"
#include "zowex.ds.test.hpp"
#include "zowex.uss.test.hpp"

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
             zowex_ds_tests();
             zowex_uss_tests();
           });
}
