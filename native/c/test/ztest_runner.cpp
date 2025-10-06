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
#include "zstorage.test.hpp"
#include "zut.test.hpp"
#include "zjb.test.hpp"
#include "zds.test.hpp"
#include "zcn.test.hpp"
#include "zrecovery.test.hpp"
#include "zmetal.test.hpp"
#include "zusf.test.hpp"
#include "zbase64.test.hpp"
#include "zowex.test.hpp"
#include "zlogger.test.hpp"
#include "parser.test.hpp"
#include "zstd.test.hpp"
#include "zjson.test.hpp"
#include "ztest.hpp"

using namespace std;
using namespace ztst;

int main(int argc, char *argv[])
{
  int rc = tests(
      argc, argv,
      []() -> void
      {
        zut_tests();
        zjb_tests();
        zds_tests();
        zcn_tests();
        zstorage_tests();
        zrecovery_tests();
        zmetal_tests();
        zusf_tests();
        zbase64_tests();
        zowex_tests();
        zlogger_tests();
        parser_tests();
        zstd_tests();
        zjson_tests();
      });

  return rc;
}
