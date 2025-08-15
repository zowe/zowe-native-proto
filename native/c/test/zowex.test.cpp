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

// Helper function to convert string to hex format
string string_to_hex(const string &input)
{
  string hex_output;
  for (char c : input)
  {
    char hex_byte[3];
    sprintf(hex_byte, "%02x", static_cast<unsigned char>(c));
    hex_output += hex_byte;
  }
  return hex_output;
}

int execute_command_with_output(const std::string &command, std::string &output)
{
  // string env_command = "_BPXK_AUTOCVT=ON _CEE_RUNOPTS=\"$_CEE_RUNOPTS FILETAG(AUTOCVT,AUTOTAG) POSIX(ON)\" _TAG_REDIR_ERR=txt _TAG_REDIR_IN=txt _TAG_REDIR_OUT=txt " + command;
  output = "";
  TestLog("Running: " + command);

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
                                        int rc = 0;
                                        string user;
                                        execute_command_with_output("whoami", user);
                                        string data_set = TrimChars(user) + ".temp.temp.temp.temp.temp.temp.tmp";
                                        string response;
                                        string del_command = "zowex data-set delete " + data_set;
                                        execute_command_with_output(del_command, response);
                                        string command = "zowex data-set create-fb " + data_set;
                                        rc = execute_command_with_output(command, response);
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
                                        int rc = 0;
                                        string data_set = "SYS1.MACLIB";
                                        string response;
                                        string command = "zowex data-set list " + data_set;
                                        rc = execute_command_with_output(command, response);
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
                                        int rc = 0;
                                        string data_set = "SYS1.MACLIB";
                                        string response;
                                        string command = "zowex data-set lm " + data_set + " --me 1";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                                      });
                                 });
                        describe("data set i/o tests",
                                 []() -> void
                                 {
                                   it("should write and read from a data set",
                                      []()
                                      {
                                        int rc = 0;
                                        string user;
                                        execute_command_with_output("whoami", user);
                                        string data_set = TrimChars(user) + ".temp.temp.temp.temp.temp.temp.tmp";
                                        string member = "IEFBR14";
                                        string data_set_member = "\"" + data_set + "(" + member + ")\"";
                                        string response;

                                        // delete the data set if it exists
                                        string del_command = "zowex data-set delete " + data_set;
                                        execute_command_with_output(del_command, response);

                                        // create the data set
                                        string command = "zowex data-set create-fb " + data_set;
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);

                                        string jcl = "//IEFBR14$ JOB (IZUACCT),TEST,REGION=0m\n//RUN EXEC PGM=IEFBR14";

                                        // Convert JCL to hex format and write to the data set
                                        string hex_jcl = string_to_hex(jcl);
                                        string write_command = "printf \"" + hex_jcl + "\" | zowex data-set write " + data_set_member;
                                        rc = execute_command_with_output(write_command, response);
                                        ExpectWithContext(rc, response).ToBe(0);

                                        // read from the data set
                                        string read_command = "zowex data-set view " + data_set_member;
                                        rc = execute_command_with_output(read_command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(TrimChars(response)).ToBe(jcl);

                                        // delete the data set
                                        execute_command_with_output(del_command, response);
                                      });
                                 });
                      });
           });
}
