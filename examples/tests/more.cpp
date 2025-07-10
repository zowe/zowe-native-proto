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

#include "ztest.hpp"

using namespace std;
using namespace ztst;

void more_tests()
{
  describe("more test suite",
           []() -> void
           {
             describe("nested describe",
                      []() -> void
                      {
                        it("should pass with more tests",
                           [&]() -> void
                           {
                             expect(3).ToBe(3);
                           });
                        it("should pass with more tests",
                           [&]() -> void
                           {
                             expect(3).ToBe(3);
                           });
                        it("should fail",
                           [&]() -> void
                           {
                             ExpectWithContext(3, "extra error messages").ToBe(5);
                           });
                      });

             it("should pass with more tests",
                [&]() -> void
                {
                  expect(3).ToBe(3);
                  expect(3).ToBe(5);
                });

             it("this test to fail",
                [&]() -> void
                {
                  // EXPECT_CONTEXT ctx = {0};
                  // ctx.message = "extra error messages";
                  // expect(3, ctx).ToBe(7);
                  ExpectWithContext(3, "extra error messages").ToBe(7);
                });

             it("should be greater than",
                [&]() -> void
                {
                  // EXPECT_CONTEXT ctx = {0};
                  // ctx.message = "extra error messages";
                  // expect(3, ctx).ToBe(7);
                  Expect(3).ToBeGreaterThan(2);
                });

             it("should be less than",
                [&]() -> void
                {
                  Expect(3).Not().ToBeGreaterThan(4);
                });

             it("should be null",
                [&]() -> void
                {
                  int *ptr = nullptr;
                  Expect(ptr).ToBeNull();
                });

             it("should not be null",
                [&]() -> void
                {
                  int *ptr = new int(1);

                  Expect(ptr).Not().ToBeNull();
                });
           });
}
