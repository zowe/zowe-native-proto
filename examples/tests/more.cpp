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
#include "ztest.hpp"

using namespace std;
using namespace ztst;

void more_tests()
{
  describe("more test suite",
            []() -> void
            {
              it("more test",
                [&]() -> void
                {
                  expect(3).ToBe(3);
                  // Expect(3).ToBe(3);

                  expect(3).ToBe(5);
                  // Expect(3).ToBe(5);

                });

                it("even more test",
                [&]() -> void
                {
                  // EXPECT_CONTEXT ctx = {0};
                  // ctx.message = "extra error messages";
                  // expect(3, ctx).ToBe(7);
                  ExpectWithContext(3, "extra error messages").ToBe(7);

                });
            });
}
