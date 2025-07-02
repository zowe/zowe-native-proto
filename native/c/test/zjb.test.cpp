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
#include <thread>
#include <chrono>

using namespace std;
using namespace ztst;

void wait_for_conversion(string correlator, string status)
{
  int index = 0;
  while (true)
  {
    ZJB zjb = {0};
    ZJob zjob = {0};
    int rc = zjb_view(&zjb, correlator, zjob);
    const int max_retries = 1000;

    cout << "@TEST index is " << index << " status is " << zjob.status << " full status " << zjob.full_status
         << " comparing " << status << endl;

    if (rc != RTNCD_SUCCESS)
    {
      string error =
          "Error: could not view job: '" + correlator + "' rc: " + to_string(rc) + "\n'  " + string(zjb.diag.e_msg) + "'";
      throw runtime_error(error);
    }

    if (index >= max_retries)
    {
      string error =
          "Error: for job: '" + correlator + "' reached max retries of " + to_string(max_retries);
      throw runtime_error(error);
    }
    if (zjob.full_status == status)
    {
      this_thread::sleep_for(chrono::milliseconds(10 * 5)); // wait for job to exit INPUT
    }
    else
    {
      break;
    }
    index++;
  }
}

void zjb_tests()
{

  describe("zjb tests", []() -> void
           {
    // it("should be able to list a job", []() -> void {
    //   ZJB zjb = {0};
    //   string owner = "*";  // all owners
    //   string prefix = "*"; // any prefix
    //   zjb.jobs_max = 1;    // limit to one
    //   vector<ZJob> jobs;
    //   int rc = zjb_list_by_owner(&zjb, owner, prefix, jobs);
    //   ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_WARNING); // expect truncated list returned
    // });

    // it("should be able to submit JCL", []() -> void {
    //   ZJB zjb = {0};
    //   string jobid;
    //   string jcl = "//IEFBR14$ JOB IZUACCT\n"
    //                "//RUNBR14  EXEC PGM=IEFBR14\n";

    //   int rc = zjb_submit(&zjb, jcl, jobid);
    //   ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
    //   Expect(jobid).Not().ToBe("");

    //   ZJob zjob;
    //   string correlator = string(zjb.correlator, 64);

    //   memset(&zjb, 0, sizeof(zjb));
    //   rc = zjb_view(&zjb, jobid, zjob);
    //   cout << "@TEST: " << jobid << " " << zjob.correlator << " " << zjob.full_status << " " << zjob.jobid << " "
    //        << zjob.jobname << " " << zjob.owner << " " << zjob.retcode << " " << zjob.status << endl;

    //   // wait_for_conversion(correlator, "INPUT");
    //   // sleep(2);

    //   memset(&zjb, 0, sizeof(zjb));
    //   rc = zjb_cancel(&zjb, correlator);
    //   cout << "@TEST cancel job rc was " << rc << " with message " << string(zjb.diag.e_msg) << endl;

    //   memset(&zjb, 0, sizeof(zjb));
    //   rc = zjb_delete(&zjb, jobid);
    //   cout << "@TEST delete job rc was " << rc << " with message " << string(zjb.diag.e_msg) << endl;
    //   // ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
    //   sleep(2);

    //   memset(&zjb, 0, sizeof(zjb));
    //   rc = zjb_view(&zjb, correlator, zjob);
    //   cout << "@TEST: " << zjob.correlator << " " << zjob.full_status << " " << zjob.jobid << " " << zjob.jobname << " "
    //        << zjob.owner << " " << zjob.retcode << " " << zjob.status << endl;
    // });

    // it("should be able to view a submitted job", []() -> void {
    //   ZJB zjb = {0};
    //   string jobid;
    //   string jcl = "//IEFBR14$ JOB IZUACCT\n"
    //                "//RUNBR14  EXEC PGM=IEFBR14\n";

    //   int rc = zjb_submit(&zjb, jcl, jobid);
    //   ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

    //   ZJob zjob;
    //   string correlator = string(zjb.correlator, 64);

    //   memset(&zjb, 0, sizeof(zjb));
    //   rc = zjb_view(&zjb, correlator, zjob);
    //   ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

    //   Expect(zjob.correlator).ToBe(correlator); // vefify submit correlator matches view status correlator

    //   // wait_for_conversion(correlator, "INPUT");

    //   memset(&zjb, 0, sizeof(zjb));
    //   rc = zjb_delete(&zjb, correlator);

    //   ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
    // });

    // it("should be able to delete a submitted job", []() -> void {
    //   ZJB zjb = {0};
    //   string jobid;
    //   string jcl = "//IEFBR14$ JOB IZUACCT\n"
    //                "//RUNBR14  EXEC PGM=IEFBR14\n";

    //   int rc = zjb_submit(&zjb, jcl, jobid);
    //   ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

    //   string correlator = string(zjb.correlator, 64);

    //   // wait_for_conversion(correlator, "INPUT");

    //   memset(&zjb, 0, sizeof(zjb));
    //   rc = zjb_delete(&zjb, correlator);

    //   ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
    // });

    // it("should be able to read job JCL", []() -> void {
    //   ZJB zjb = {0};
    //   string jobid;
    //   string jcl = "//IEFBR14$ JOB IZUACCT\n"
    //                "//RUNBR14  EXEC PGM=IEFBR14\n";

    //   int rc = zjb_submit(&zjb, jcl, jobid);
    //   ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

    //   string correlator = string(zjb.correlator, 64);
    //   string returned_jcl;

    //   // wait_for_conversion(correlator, "INPUT");

    //   memset(&zjb, 0, sizeof(zjb));
    //   rc = zjb_read_job_jcl(&zjb, correlator, returned_jcl);

    //   ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
    // });

    it("should be able to list and view SYSOUT files for INPUT jobs", []() -> void {
      ZJB zjb = {0};
      string jobid;
      string jcl = "//IEFBR14$ JOB IZUACCT,TYPRUN=HOLD\n"
                   "//RUNBR14  EXEC PGM=IEFBR14\n";

      int rc = zjb_submit(&zjb, jcl, jobid);
      ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

      // string correlator = string(zjb.correlator, 64);

      // wait_for_conversion(correlator, "INPUT");


      ZJob zjob;
      string correlator = string(zjb.correlator, 64);

      memset(&zjb, 0, sizeof(zjb));
      rc = zjb_view(&zjb, jobid, zjob);
      cout << "@TEST: " << jobid << " " << zjob.correlator << " " << zjob.full_status << " " << zjob.jobid << " "
           << zjob.jobname << " " << zjob.owner << " " << zjob.retcode << " " << zjob.status << endl;


      vector<ZJobDD> dds;
      memset(&zjb, 0, sizeof(zjb));
      rc = zjb_list_dds(&zjb, correlator, dds);
      ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
      Expect(dds.size()).ToBeGreaterThan(0); // expect at least one DD returned

      string content;
      memset(&zjb, 0, sizeof(zjb));
      rc = zjb_read_jobs_output_by_key(&zjb, correlator, dds[0].key, content);
      ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
      Expect(content).Not().ToBe(""); // expect some content returned

      memset(&zjb, 0, sizeof(zjb));
      rc = zjb_delete(&zjb, correlator);
      ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
    });

    it("should be able to cancel a JCL", []() -> void {
      ZJB zjb = {0};
      string jobid;
      string jcl = "//IEFBR14$ JOB IZUACCT,TYPRUN=HOLD\n"
                   "//RUNBR14  EXEC PGM=IEFBR14\n";

      int rc = zjb_submit(&zjb, jcl, jobid);
      ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

      string correlator = string(zjb.correlator, 64);

      // wait_for_conversion(correlator, "INPUT");
      // sleep(1);

      memset(&zjb, 0, sizeof(zjb));
      rc = zjb_cancel(&zjb, correlator);

      ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

      ZJob zjob;

      // wait_for_conversion(correlator, "CONVERSION");
      // wait_for_conversion(correlator, "AWAIT MAIN SELECT");

      memset(&zjb, 0, sizeof(zjb));
      rc = zjb_view(&zjb, correlator, zjob);

      cout << "Job status: " << zjob.status << " " << zjob.owner << " " << zjob.jobid << " " << zjob.jobname << " "
           << zjob.full_status << " " << zjob.retcode << " " << zjob.correlator << endl;

      ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);

      Expect(zjob.retcode).ToBe("CANCELED");

      memset(&zjb, 0, sizeof(zjb));
      rc = zjb_delete(&zjb, correlator);
      ExpectWithContext(rc, zjb.diag.e_msg).ToBe(RTNCD_SUCCESS);
    }); });
}
