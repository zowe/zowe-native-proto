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
#include <stdexcept>

#include "ztest.hpp"
#include "zjb.hpp"
#include <unistd.h>

using namespace std;
using namespace ztst;

void zjb_tests()
{

  describe("zjb tests",
           []() -> void
           {
             it("should be able to list a job",
                []() -> void
                {
                  ZJB zjb = {0};
                  string owner = "*";  // all owners
                  string prefix = "*"; // any prefix
                  zjb.jobs_max = 1;    // limit to one
                  vector<ZJob> jobs;
                  expect(zjb_list_by_owner(&zjb, owner, prefix, jobs)).ToBe(RTNCD_WARNING); // expect truncated list returned
                });

             it("should be able to submit JCL",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  expect(rc).ToBe(0);
                  expect(jobid).Not().ToBe("");

                  string correlator = string(zjb.job_correlator, 64);
                  memset(&zjb, 0, sizeof(zjb));
                  rc = zjb_delete(&zjb, correlator);
                  expect(rc).ToBe(0);
                });

             it("should be able to view a submitted job",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  expect(rc).ToBe(0);

                  ZJob zjob;

                  string correlator = string(zjb.job_correlator, 64);

                  memset(&zjb, 0, sizeof(zjb));
                  rc = zjb_view(&zjb, correlator, zjob);
                  expect(rc).ToBe(0);

                  expect(correlator).ToBe(zjob.job_correlator); // vefify submit correlator matches view status correlator

                  memset(&zjb, 0, sizeof(zjb));
                  rc = zjb_delete(&zjb, correlator);
                  expect(rc).ToBe(0);
                });

             it("should be able to delete a submitted job",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  expect(rc).ToBe(0);

                  string correlator = string(zjb.job_correlator, 64);

                  memset(&zjb, 0, sizeof(zjb));
                  rc = zjb_delete(&zjb, correlator);
                  expect(rc).ToBe(0);
                });

             it("should be able to read job JCL",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  expect(rc).ToBe(0);

                  string correlator = string(zjb.job_correlator, 64);
                  string returned_jcl;

                  memset(&zjb, 0, sizeof(zjb));
                  sleep(1); // wait for job to complete
                  rc = zjb_read_job_jcl(&zjb, correlator, returned_jcl);
                  expect(rc).ToBe(0);
                });

             it("should fail",
                []() -> void
                {
                  int rc = 1;
                  expect(rc).ToBe(0);
                });
           });
}
