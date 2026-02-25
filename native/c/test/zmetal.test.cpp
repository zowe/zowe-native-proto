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

using namespace ztst;

void zmetal_tests()
{

  describe("zmetal tests",
           []() -> void
           {
             it("should load a program",
                []()
                {
                  std::string name = "IEFBR14";
                  void *ep = ZMTLLOAD(name.c_str());
                  Expect(ep).Not().ToBeNull();
                  int rc = ZMTLDEL(name.c_str());
                  Expect(rc).ToBe(0);
                });

             it("should not load a program that does not exist",
                []()
                {
                  std::string name = "IEFBR15";
                  void *ep = ZMTLLOAD(name.c_str());
                  Expect(ep).ToBeNull();
                });

             it("should not delete a program that does not exist",
                []()
                {
                  std::string name = "IEFBR15";
                  int rc = ZMTLDEL(name.c_str());
                  Expect(rc).ToBe(4);
                });
           });
}
