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

#define SLEEPY_TIME 0

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
                  int rc = zjb_list_by_owner(&zjb, owner, prefix, jobs);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_WARNING); // expect truncated list returned
                });

             it("should be able to submit JCL",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
                  Expect(jobid).Not().ToBe("");

                  string correlator = string(zjb.correlator, 64);
                  memset(&zjb, 0, sizeof(zjb));
                  sleep(SLEEPY_TIME); // wait for job to complete
                  rc = zjb_delete(&zjb, correlator);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
                });

             it("should be able to view a submitted job",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

                  ZJob zjob;

                  string correlator = string(zjb.correlator, 64);

                  memset(&zjb, 0, sizeof(zjb));
                  rc = zjb_view(&zjb, correlator, zjob);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

                  Expect(zjob.correlator).ToBe(correlator); // vefify submit correlator matches view status correlator

                  cout << "@TEST line:76 jobname " << zjob.jobname << " jobid " << zjob.jobid << " status " << zjob.status << " owner " << zjob.owner << " full status " << zjob.full_status << " retcode " << zjob.retcode << " correlator " << zjob.correlator << "\n";

                  memset(&zjb, 0, sizeof(zjb));
                  sleep(SLEEPY_TIME); // wait for job to complete
                  rc = zjb_delete(&zjb, correlator);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
                });

             it("should be able to delete a submitted job",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

                  string correlator = string(zjb.correlator, 64);

                  memset(&zjb, 0, sizeof(zjb));
                  sleep(SLEEPY_TIME); // wait for job to complete
                  rc = zjb_delete(&zjb, correlator);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
                });

             it("should be able to read job JCL",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

                  string correlator = string(zjb.correlator, 64);
                  string returned_jcl;

                  memset(&zjb, 0, sizeof(zjb));
                  sleep(SLEEPY_TIME); // wait for job to complete
                  rc = zjb_read_job_jcl(&zjb, correlator, returned_jcl);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
                });

             it("should be able to list and view SYSOUT files for INPUT jobs",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT,TYPRUN=HOLD\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

                  string correlator = string(zjb.correlator, 64);

                  memset(&zjb, 0, sizeof(zjb));
                  sleep(SLEEPY_TIME); // wait for job to complete
                  vector<ZJobDD> dds;
                  rc = zjb_list_dds(&zjb, correlator, dds);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
                  Expect(dds.size()).ToBeGreaterThan(0); // expect at least one DD returned

                  memset(&zjb, 0, sizeof(zjb));
                  string content;
                  rc = zjb_read_jobs_output_by_key(&zjb, correlator, dds[0].key, content);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
                  Expect(content).Not().ToBe(""); // expect some content returned

                  memset(&zjb, 0, sizeof(zjb));
                  rc = zjb_delete(&zjb, correlator);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
                });

             it("should be able to cancel a JCL",
                []() -> void
                {
                  ZJB zjb = {0};
                  string jobid;
                  string jcl = "//IEFBR14$ JOB IZUACCT,TYPRUN=HOLD\n"
                               "//RUNBR14  EXEC PGM=IEFBR14\n";

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

                  string correlator = string(zjb.correlator, 64);

                  memset(&zjb, 0, sizeof(zjb));
                  sleep(SLEEPY_TIME); // wait for job to complete
                  rc = zjb_cancel(&zjb, correlator);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

                  ZJob zjob;

                  memset(&zjb, 0, sizeof(zjb));
                  rc = zjb_view(&zjb, correlator, zjob);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

                  Expect(zjob.retcode).ToBe("CANCELED");

                  memset(&zjb, 0, sizeof(zjb));
                  rc = zjb_delete(&zjb, correlator);
                  ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
                });
           });
}
