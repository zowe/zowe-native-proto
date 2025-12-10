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
#include "zutils.hpp"

using namespace std;
using namespace ztst;

void zoweax_console_tests()
{
  describe("console issue command tests", [&]() -> void
           {
        it("should display help", []() -> void
        {
            string response;
            string command = zoweax_command + " console";
            int rc = execute_command_with_output(command, response);

            ExpectWithContext(rc, response).ToBe(0);
            Expect(response).ToContain("issue");
        });

        it("should issue console command successfully", []() -> void
        {
            string response;
            string command = zoweax_command + " console issue \"D T\"";
            int rc = execute_command_with_output(command, response);

            ExpectWithContext(rc, response).ToBe(0);
            Expect(response.find("IEE136I LOCAL: ")).Not().ToBe(std::string::npos);
        });

        it("should error when the console name is invalid", []() -> void
        {
            string response;
            string command = zoweax_command +
                            " console issue \"D IPLINFO\" --console-name 1Invalid";
            int rc = execute_command_with_output(command, response);

            ExpectWithContext(rc, response).Not().ToBe(0);
            Expect(response).ToContain("Error: could not activate console:");
        });

        it("should successfully issue a command when the console does not already exist", []() -> void
        {
            string response;
            string command = zoweax_command +
                            " console issue \"D IPLINFO\" --console-name newConsoleName";
            int rc = execute_command_with_output(command, response);

            ExpectWithContext(rc, response).ToBe(0);
            Expect(response.find("IEE254I")).Not().ToBe(std::string::npos);
        });

        it("should issue without waiting when boolean is set to false", []() -> void
        {
            string response;
            string command = zoweax_command + " console issue \"D IPLINFO\" --wait false";
            int rc = execute_command_with_output(command, response);

            ExpectWithContext(rc, response).ToBe(0);
        });

        it("should fail when using a non-APF authorized binary", []() -> void
        {
            string response;
            string command = zowex_command + " console issue \"D T\"";
            int rc = execute_command_with_output(command, response);

            ExpectWithContext(rc, response).Not().ToBe(0);
            Expect(response).ToContain("Error: could not activate console:");

        }); });
}
