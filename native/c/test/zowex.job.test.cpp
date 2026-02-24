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
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include "ztest.hpp"
#include "zutils.hpp"
#include "zowex.test.hpp"
#include "zowex.job.test.hpp"
#include "zowex.job.list.test.hpp"
#include "zowex.job.submit.test.hpp"
#include "zowex.job.manage.test.hpp"

using namespace std;
using namespace ztst;

void zowex_job_tests()
{
  static vector<string> _jobs;
  static vector<string> _ds;
  static vector<string> _files;

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

             TEST_OPTIONS long_test_opts = {false, 30};

             afterAll(
                 [&]() -> void
                 {
                   _cleanup_jobs();
                   _cleanup_ds();
                   _cleanup_files();
                 },
                 long_test_opts);

             zowex_job_list_tests(_jobs, _ds, _files);
             zowex_job_submit_tests(_jobs, _ds, _files);
             zowex_job_manage_tests(_jobs, _ds, _files);
           });
}