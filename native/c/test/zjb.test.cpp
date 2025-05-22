#include <iostream>
#include <stdexcept>

#include "ztest.hpp"
#include "zjb.hpp"
// #include "zstorage.metal.test.h"

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
                  // zut_uppercase_pad_truncate(buffer, data, sizeof(buffer) - 1);
                  int rc = zjb_submit(&zjb, jcl, jobid);
                  expect(rc).ToBe(0);
                  expect(jobid).Not().ToBe("");
                });
           });
}
