#include <iostream>
#include <stdexcept>

#include "ztest.hpp"
#include "zds.hpp"
// #include "zstorage.metal.test.h"

using namespace std;
using namespace ztst;

void zds_tests()
{

  describe("zds tests",
           []() -> void
           {
             //  it("should be able to list a job",
             //     []() -> void
             //     {
             //       ZJB zjb = {0};
             //       string owner = "*";  // all owners
             //       string prefix = "*"; // any prefix
             //       zjb.jobs_max = 1;    // limit to one
             //       vector<ZJob> jobs;
             //       expect(zjb_list_by_owner(&zjb, owner, prefix, jobs)).ToBe(RTNCD_WARNING); // expect truncated list returned
             //     });
           });
}
