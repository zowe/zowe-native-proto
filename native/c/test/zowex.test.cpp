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

std::string &trim_chars(std::string &str, const std::string &chars = " \t\n\r\f\v")
{
  str.erase(0, str.find_first_not_of(chars));
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

int execute_command_with_output(const std::string &command, std::string &output)
{
  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe)
  {
    throw std::runtime_error("Failed to open pipe");
  }

  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
  {
    output += buffer;
  }

  int exit_code = pclose(pipe);
  return WEXITSTATUS(exit_code);
}

void zowex_tests()
{

  describe("zowex tests",
           []() -> void
           {
             describe("job tests",
                      []() -> void
                      {
                        it("should list jobs",
                           []()
                           {
                             int rc = system("zowex job list > /dev/null 2>&1");
                             Expect(rc).ToBe(0);
                           });
                      });
             describe("data set tests",
                      []() -> void
                      {
                        it("should create a data set",
                           []()
                           {
                             string user;
                             execute_command_with_output("whoami", user);
                             string data_set = trim_chars(user) + ".test.temp.jcl";
                             string response;
                             cout << "@TEST user: " << user << endl;
                             cout << "@TEST data_set: " << data_set << endl;
                             int rc = execute_command_with_output("zowex data-set create-fb " + data_set, response);
                             Expect(rc).ToBe(0);
                           });
                      });
           });
}
