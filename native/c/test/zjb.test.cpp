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

                  int rc = zjb_submit(&zjb, jcl, jobid);
                  expect(rc).ToBe(0);
                  expect(jobid).Not().ToBe("");
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
                  expect(jobid).Not().ToBe("");

                  ZJob zjob;

                  memset(&zjb, 0, sizeof(zjb));
                  rc = zjb_view(&zjb, "JOB00880", zjob);
                  expect(zjb_view(&zjb, jobid, zjob)).ToBe(0);
                });
           });
}
