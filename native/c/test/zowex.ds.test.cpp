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
#include <vector>
#include "ztest.hpp"
#include "ztest.utils.hpp"
#include "ztype.h"
#include "zowex.test.hpp"
#include "zowex.ds.test.hpp"

using namespace std;
using namespace ztst;

const string zowex_command = "./../build-out/zowex";

void zowex_ds_tests()
{
  vector<string> _ds;
  describe("data-set",
           [&_ds]() -> void
           {
             afterAll(
                 [&_ds]() -> void
                 {
                   TestLog("Deleting " + to_string(_ds.size()) + " data sets!");
                   for (vector<string>::iterator it = _ds.begin(); it != _ds.end(); ++it)
                   {
                     try
                     {
                       string command = zowex_command + " data-set delete " + *it;
                       string response;
                       TestLog(command);
                       int rc = execute_command_with_output(command, response);
                       ExpectWithContext(rc, response).ToBe(0);
                       TestLog(response);
                       Expect(response).ToContain("Data set '" + *it + "' deleted"); // ds deleted
                     }
                     catch (...)
                     {
                       TestLog("Failed to delete: " + *it);
                     }
                   }
                 });
             it("should display help", []() -> void
                {
                  int rc = 0;
                  string response;
                  string command = zowex_command + " data-set";
                  rc = execute_command_with_output(command, response);
                  ExpectWithContext(rc, response).ToBe(0);
                  Expect(response).ToContain("create");
                  Expect(response).ToContain("delete");
                  Expect(response).ToContain("list"); // done
                });
             describe("compress",
                      []() -> void
                      {
                        it("should compress a data set", []() -> void {});
                      });
             describe("create",
                      [&_ds]() -> void
                      {
                        it("should create a data set with default attributes",
                           [&_ds]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set list " + ds + " -a --rfc";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToBe("PS");
                             Expect(tokens[4]).ToBe("FB");
                           });
                        it("should create a data set - recfm:VB dsorg:PO",
                           [&_ds]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create " + ds + " --recfm VB --dsorg PO";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set list " + ds + " -a --rfc";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToBe("PO");
                             Expect(tokens[4]).ToBe("VB");
                           });

                        it("should create a data set - dsorg: PO, primary: 10, secondary: 2, lrecl: 20, blksize:10, dirblk: 5, alcunit: CYL",
                           [&_ds]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create " + ds + " --dsorg PO --primary 10 --secondary 2 --lrecl 20 --blksize 10 --dirblk 5 --alcunit CYL";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set list " + ds + " -a --rfc";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToBe("PO");
                             Expect(tokens[4]).ToBe("FB");
                           });
                      });
             describe("create-adata",
                      [&_ds]() -> void
                      {
                        it("should create a data set with default attributes",
                           [&_ds]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create-adata " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set list " + ds + " -a --rfc";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToContain("PO");
                             Expect(tokens[4]).ToBe("VB");
                             // lrecl = 32756
                           });
                      });

             describe("create-fb",
                      [&_ds]() -> void
                      {
                        it("should create a data set with default attributes",
                           [&_ds]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create-fb " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set list " + ds + " -a --rfc";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToContain("PO");
                             Expect(tokens[4]).ToBe("FB");
                             // lrecl = 80
                           });
                      });
             describe("create-loadlib",
                      [&_ds]() -> void
                      {
                        it("should create a data set with default attributes",
                           [&_ds]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create-loadlib " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set list " + ds + " -a --rfc";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToContain("PO");
                             Expect(tokens[4]).ToBe("U");
                             // lrecl = 0
                           });
                      });
             describe("create-member",
                      []() -> void
                      {
                        it("should create a data set with default attributes", []() -> void {});
                      });
             describe("create-vb",
                      [&_ds]() -> void
                      {
                        it("should create a data set with default attributes",
                           [&_ds]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create-vb " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set list " + ds + " -a --rfc";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToContain("PO");
                             Expect(tokens[4]).ToBe("VB");
                             // lrecl = 255
                           });
                      });
             describe("delete",
                      []() -> void {});
             describe("list",
                      []() -> void
                      {
                        it("should list a data set",
                           []()
                           {
                             string data_set = "SYS1.MACLIB";
                             string response;
                             string command = zowex_command + " data-set list " + data_set;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                           });
                        it("should list data sets based on pattern and warn about listing too many members",
                           []()
                           {
                             string dsn = "SYS1.CMDLIB";
                             string response;
                             string pattern = "SYS1.*";
                             string command = zowex_command + " data-set list " + pattern + " --me 10";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                             Expect(response).ToContain(dsn);
                           });
                      });
             describe("list-members",
                      []() -> void
                      {
                        string data_set = "SYS1.MACLIB";
                        it("should list a member of a data set",
                           [data_set]()
                           {
                             string response;
                             string command = zowex_command + " data-set lm " + data_set + " --no-warn --me 1";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                           });
                        it("should warn when listing members of a data set with many members",
                           [data_set]()
                           {
                             int rc = 0;
                             string response;
                             string command = zowex_command + " data-set lm " + data_set + " --me 1";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                           });
                      });
             describe("restore",
                      []() -> void {});
             describe("view",
                      []() -> void {});
             describe("write",
                      []() -> void {});
             // describe("data set i/o tests",
             //          []() -> void
             //          {
             //            it("should write and read from a data set",
             //               []()
             //               {
             //                 int rc = 0;
             //                 string user;
             //                 execute_command_with_output("whoami", user);
             //                 string data_set = TrimChars(user) + ".temp.temp.temp.temp.temp.temp.tmp";
             //                 string member = "IEFBR14";
             //                 string data_set_member = "\"" + data_set + "(" + member + ")\"";
             //                 string response;

             //                 // delete the data set if it exists
             //                 string del_command = zowex_command + " data-set delete " + data_set;
             //                 execute_command_with_output(del_command, response);

             //                 // create the data set
             //                 string command = zowex_command + " data-set create-fb " + data_set;
             //                 rc = execute_command_with_output(command, response);
             //                 ExpectWithContext(rc, response).ToBe(0);

             //                 string jcl = "//IEFBR14$ JOB (IZUACCT),TEST,REGION=0M\n//RUN EXEC PGM=IEFBR14";

             //                 // Convert JCL to hex format and write to the data set
             //                 string hex_jcl = string_to_hex(jcl);
             //                 string write_command = "printf \"" + hex_jcl + "\" | " + zowex_command + " data-set write " + data_set_member;
             //                 rc = execute_command_with_output(write_command, response);
             //                 ExpectWithContext(rc, response).ToBe(0);

             //                 // read from the data set
             //                 string read_command = zowex_command + " data-set view " + data_set_member;
             //                 rc = execute_command_with_output(read_command, response);
             //                 ExpectWithContext(rc, response).ToBe(0);
             //                 Expect(TrimChars(response)).ToBe(jcl);

             //                 // delete the data set
             //                 execute_command_with_output(del_command, response);
             //               });
             //          });
           });
}
