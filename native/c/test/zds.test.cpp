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
#include "zds.hpp"
// #include "zstorage.metal.test.h"

using namespace std;
using namespace ztst;

void zds_tests()
{

  describe("zds tests",
           []() -> void
           {
             it("should list data sets in a DSN",
                []() -> void
                {
                  int rc = 0;
                  ZDS zds = {0};
                  vector<ZDSEntry> entries;
                  string dsn = "SYS1.MACLIB";
                  rc = zds_list_data_sets(&zds, dsn, entries);
                  ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                });
             it("should list data set members",
                []() -> void
                {
                  int rc = 0;
                  ZDS zds = {0};
                  vector<ZDSMem> members;
                  string dsn = "SYS1.MACLIB";
                  rc = zds_list_members(&zds, dsn, members);
                  ExpectWithContext(rc, zds.diag.e_msg).ToBeGreaterThan(0); // zero or warning
                });
           });
}
