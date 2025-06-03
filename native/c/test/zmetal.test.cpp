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
#include "zmetal.metal.test.h"

using namespace std;
using namespace ztst;

void zmetal_tests()
{

  describe("zmetal tests",
           []() -> void
           {
             it("should obtain and free 31-bit storage",
                []()
                {
                  int rc = ZMETAL1();
                  expect(rc).ToBe(0);
                });
           });
}
