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

#include <cstddef>
#include <ctime>
#include <stdlib.h>
#include <string>
#include "ztest.hpp"
#include "zutils.hpp"

using namespace std;
using namespace ztst;

const string zowex_command = "./../build-out/zowex";

void zowex_console_tests()
{
  describe("console", [&]() -> void
           {
        it("should display help", []() -> void
        {
            string response;
            string command = zowex_command + " console issue command";
            int rc = execute_command_with_output(command, response);

            ExpectWithContext(rc, response).ToBe(0);
            Expect(response).ToContain("issue");
            Expect(response).ToContain("help");
        });

        it("should issue console command successfully", []() -> void
        {
            string response;
            string command = zowex_command + " console issue command \"D T\"";
            int rc = execute_command_with_output(command, response);

            ExpectWithContext(rc, response).ToBe(0);
        });

        it("should error when the console name does not exist", []() -> void
        {
            string response;
            string command = zowex_command +
                            " console issue command \"D IPLINFO\" --console-name invalidConsoleName";
            int rc = execute_command_with_output(command, response);

            ExpectWithContext(rc, response).Not().ToBe(0);
            Expect(response).ToContain("Error: could not activate console:");
            Expect(response).ToContain("invalidConsoleName");
        }); });
}
