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
#include "zrecovery.metal.test.h"

using namespace std;
using namespace ztst;

void zrecovery_tests()
{

  describe("zrecovery tests",
           []() -> void
           {
             TEST_OPTIONS opts = {0};
             opts.remove_signal_handling = false;

             it("should recovery from an abend", []() -> void
                {
                  int rc = ZRCVYEN();
                expect(rc).ToBe(0); }, opts);
           });
}
