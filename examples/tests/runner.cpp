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
#include "more.hpp"

using namespace std;
using namespace ztst;

// @TEST TODO(Kelosky): fix this global variable
std::string matcher = "";

int main()
{
  return tests(
      []() -> void
      {
        describe("test suite",
                 []() -> void
                 {
                   it("test",
                      [&]() -> void
                      {
                        // expect(1).ToBe(1);
                        Expect(1).ToBe(1);
                      });

                   more_tests();
                 });
      });
}
