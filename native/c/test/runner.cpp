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
#include "zrecovery.test.hpp"
#include "zmetal.test.hpp"
#include "ztest.hpp"

using namespace std;
using namespace ztst;

int main()
{

  int rc = tests(
      []() -> void
      {
        zstorage_tests();
        zut_tests();
        zjb_tests();
        zds_tests();
        zrecovery_tests();
        zmetal_tests();
      });

  return rc;
}
