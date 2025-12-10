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

#include <stdlib.h>
#include <string>
#include "ztest.hpp"
#include "ztype.h"
#include "zutils.hpp"
#include "zowex.test.hpp"
#include "zowex.ds.test.hpp"
#include "zowex.uss.test.hpp"
#include "zowex.job.test.hpp"
#include "zoweax.console.test.hpp"
#include "zowex.tso.test.hpp"

using namespace std;
using namespace ztst;

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
             zowex_ds_tests();
             zowex_uss_tests();
             zowex_job_tests();
             zowex_tso_tests();
           });

  describe("zoweax", []() -> void
           { zoweax_console_tests(); });
}
