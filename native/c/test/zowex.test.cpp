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

#include <stdexcept>
#include "ztest.hpp"
#include "ztype.h"
#include "zowex.test.hpp"

using namespace std;
using namespace ztst;

int execute_command_with_output(const std::string &command, std::string &output)
{
  FILE *pipe = popen((command + " 2>&1").c_str(), "r");
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
                        describe("data set create tests",
                                 []() -> void
                                 {
                                   it("should create a fb data set",
                                      []()
                                      {
                                        string user;
                                        execute_command_with_output("whoami", user);
                                        string data_set = TrimChars(user) + ".temp.temp.temp.temp.temp.temp.tmp";
                                        string response;
                                        string del_command = "zowex data-set delete " + data_set;
                                        execute_command_with_output(del_command, response);
                                        string command = "zowex data-set create-fb " + data_set;
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        execute_command_with_output(del_command, response);
                                      });
                                 });
                        describe("data set list tests",
                                 []() -> void
                                 {
                                   it("should list a data set",
                                      []()
                                      {
                                        string data_set = "SYS1.MACLIB";
                                        string response;
                                        string command = "zowex data-set list " + data_set;
                                        TestLog("Running: " + command);
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                      });
                                   it("should list a member of a data set",
                                      []()
                                      {
                                        string data_set = "SYS1.MACLIB";
                                        string response;
                                        string command = "zowex data-set lm " + data_set + " --no-warn --me 1";
                                        TestLog("Running: " + command);
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                      });
                                   it("should warn when listing members of a data set with many members",
                                      []()
                                      {
                                        string data_set = "SYS1.MACLIB";
                                        string response;
                                        string command = "zowex data-set lm " + data_set + " --me 1";
                                        TestLog("Running: " + command);
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                                      });
                                 });
                      });
           });
}
