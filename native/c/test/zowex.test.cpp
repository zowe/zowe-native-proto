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
#include "zowex.test.hpp"

using namespace std;
using namespace ztst;

void zowex_tests()
{

  describe("zowex tests",
           []() -> void
           {
             it("should list jobs",
                []()
                {
                  int rc = system("zowex job list > /dev/null 2>&1");
                  Expect(rc).ToBe(0);
                });
           });
}
