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
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include "../zbase64.h"
#include "ztest.hpp"
#include "zutils.hpp"
#include "ztype.h"
#include "zowex.test.hpp"
#include "zowex.ds.test.hpp"

using namespace std;
using namespace ztst;

// Generic helper function for creating data sets
void _create_ds(const string &ds_name, const string &ds_options = "")
{
  string response;
  string command = zowex_command + " data-set create " + ds_name + " " + ds_options;
  int rc = execute_command_with_output(command, response);
  ExpectWithContext(rc, response).ToBe(0);
  Expect(response).ToContain("Data set created");
}

void zowex_ds_tests()
{
  vector<string> _ds;
  describe("data-set",
           [&]() -> void
           {
             afterAll(
                 [&]() -> void
                 {
                   TestLog("Deleting " + to_string(_ds.size()) + " data sets...");
                   for (vector<string>::iterator it = _ds.begin(); it != _ds.end(); ++it)
                   {
                     try
                     {
                       string command = zowex_command + " data-set delete " + *it;
                       string response;
                       int rc = execute_command_with_output(command, response);
                       ExpectWithContext(rc, response).ToBe(0);
                       Expect(response).ToContain("Data set '" + *it + "' deleted"); // ds deleted
                     }
                     catch (...)
                     {
                       try
                       {
                         string response;
                         string command = zowex_command + " data-set list " + *it + " --no-warn --me 1";
                         int rc = execute_command_with_output(command, response);
                         ExpectWithContext(rc, response).ToBe(0);
                         Expect(response).Not().ToContain(*it);
                       }
                       catch (...)
                       {
                         TestLog("Failed to delete: " + *it);
                       }
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
                  Expect(response).ToContain("compress");
                  Expect(response).ToContain("create");
                  Expect(response).ToContain("create-adata");
                  Expect(response).ToContain("create-fb");
                  Expect(response).ToContain("create-loadlib");
                  Expect(response).ToContain("create-member");
                  Expect(response).ToContain("create-vb");
                  Expect(response).ToContain("delete");
                  Expect(response).ToContain("list");
                  Expect(response).ToContain("list-members");
                  Expect(response).ToContain("restore");
                  Expect(response).ToContain("view");
                  Expect(response).ToContain("write"); });

             // TODO: https://github.com/zowe/zowe-native-proto/issues/640
             xdescribe("compress",
                       [&]() -> void
                       {
                         beforeEach(
                             [&]() -> void
                             {
                               _ds.push_back(get_random_ds());
                             });
                         it("should error when the data set is PS",
                            [&]() -> void
                            {
                              string ds = _ds.back();
                              _create_ds(ds, "--dsorg PS");

                              string response;
                              string command = zowex_command + " data-set compress " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).Not().ToBe(0);
                              Expect(response).ToContain("Error: data set '" + ds + "' is not a PDS'");
                            });
                         it("should error when the data set is PDS/E",
                            [&]() -> void
                            {
                              string ds = _ds.back();
                              _create_ds(ds, "--dsorg PO --dirblk 2 --dsntype LIBRARY");

                              string response;
                              string command = zowex_command + " data-set compress " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).Not().ToBe(0);
                              Expect(response).ToContain("Error: data set '" + ds + "' is not a PDS'");
                            });

                         it("should error when the data set doesn't exist",
                            [&]() -> void
                            {
                              string ds = _ds.back() + ".GHOST";

                              string response;
                              string command = zowex_command + " data-set compress " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).Not().ToBe(0);
                              Expect(response).ToContain("Error: data set '" + ds + "' is not a PDS'");
                            });

                         xit("should compress a data set",
                             [&]() -> void
                             {
                               string ds = _ds.back();
                               _create_ds(ds, "--dsorg PO --dirblk 2");

                               string response;
                               string command = zowex_command + " data-set compress " + ds;
                               int rc = execute_command_with_output(command, response);
                               ExpectWithContext(rc, response).ToBe(0);
                               Expect(response).ToContain("Data set compressed");
                             });

                         // TODO: https://github.com/zowe/zowe-native-proto/issues/666
                         xit("should error when the data set is VSAM", []() -> void {});
                         xit("should error when the data set is GDG", []() -> void {});
                         xit("should error when the data set is ALIAS", []() -> void {});
                       });
             describe("create",
                      [&]() -> void
                      {
                        beforeEach(
                            [&]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });
                        it("should error when the data set already exists",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds);

                             string response;
                             string command = zowex_command + " data-set create " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set");
                           });

                        it("should create a data set with default attributes",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds);

                             string response;
                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = parse_rfc_response(response, ",");
                             // NOTE: Non-SMS managed systems return `--` for dsorg if not specified
                             Expect(tokens[3]).ToBe("PS");
                             Expect(tokens[4]).ToBe("FB");
                           });

                        it("should create a simple PDS/E data set - dsorg: PO and dsntype: LIBRARY",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --dirblk 2 --dsntype LIBRARY");

                             string response;
                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = parse_rfc_response(response, ",");
                             Expect(tokens[3]).ToBe("PO");
                             Expect(tokens[9]).ToBe("PDS");
                           });

                        it("should create a data set - recfm:VB dsorg:PO",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--recfm VB --dsorg PO --dirblk 2");

                             string response;

                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = parse_rfc_response(response, ",");
                             Expect(tokens[3]).ToBe("PO");
                             Expect(tokens[4]).ToBe("VB");
                           });

                        it("should create a PS data set - recfm:VB dsorg:PS",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--recfm VB --dsorg PS");

                             string response;
                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = parse_rfc_response(response, ",");
                             Expect(tokens[3]).ToBe("PS");
                             Expect(tokens[4]).ToBe("VB");
                           });

                        // TODO: https://github.com/zowe/zowe-native-proto/pull/625
                        it("should create a data set - dsorg: PO, primary: 10, secondary: 2, lrecl: 20, blksize:10, dirblk: 5, alcunit: CYL",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --primary 10 --secondary 2 --lrecl 20 --blksize 10 --dirblk 5 --alcunit CYL");

                             string response;
                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = parse_rfc_response(response, ",");
                             Expect(tokens[3]).ToBe("PO"); // dsorg
                             Expect(tokens[4]).ToBe("FB"); // recfm
                             Expect(tokens[5]).ToBe("20"); // lrecl
                             Expect(tokens[6]).ToBe("10"); // blksize
                             Expect(tokens[7]).ToBe("10"); // primary
                             Expect(tokens[8]).ToBe("2");  // secondary
                           });
                        it("should fail to create a data set if the data set name is too long",
                           []() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds(8);
                             string response;
                             string command = zowex_command + " data-set create " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set: '" + ds + "'");
                           });
                      });
             describe("create-adata",
                      [&]() -> void
                      {
                        it("should create a data set with default attributes",
                           [&]() -> void
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
                             vector<string> tokens = parse_rfc_response(response, ",");
                             Expect(tokens[3]).ToBe("PO");
                             Expect(tokens[4]).ToBe("VB");
                             Expect(tokens[9]).ToBe("PDS");
                             // lrecl = 32756
                           });

                        it("should fail to create a data set if the data set already exists",
                           [&]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create-adata " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set create-adata " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set: '" + ds + "'");
                           });
                        it("should fail to create a data set if the data set name is too long",
                           []() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds(8);
                             string response;
                             string command = zowex_command + " data-set create-adata " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set: '" + ds + "'");
                           });
                      });

             describe("create-fb",
                      [&]() -> void
                      {
                        it("should create a data set with default attributes",
                           [&]() -> void
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
                             vector<string> tokens = parse_rfc_response(response, ",");
                             Expect(tokens[3]).ToBe("PO");
                             Expect(tokens[4]).ToBe("FB");
                             Expect(tokens[9]).ToBe("PDS");
                             // lrecl = 80
                           });
                        it("should fail to create a data set if the data set already exists",
                           [&]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create-fb " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set create-fb " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set: '" + ds + "'");
                           });
                        it("should fail to create a data set if the data set name is too long",
                           []() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds(8);
                             string response;
                             string command = zowex_command + " data-set create-fb " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set: '" + ds + "'");
                           });
                      });
             describe("create-loadlib",
                      [&]() -> void
                      {
                        it("should create a data set with default attributes",
                           [&]() -> void
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
                             vector<string> tokens = parse_rfc_response(response, ",");
                             Expect(tokens[3]).ToBe("PO");
                             Expect(tokens[4]).ToBe("U");
                             Expect(tokens[5]).ToBe("0"); // lrecl
                             Expect(tokens[9]).ToBe("PDS");
                           });
                        it("should fail to create a data set if the data set already exists",
                           [&]() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds();
                             _ds.push_back(ds);

                             string response;
                             string command = zowex_command + " data-set create-loadlib " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set create-loadlib " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set: '" + ds + "'");
                           });
                        it("should fail to create a data set if the data set name is too long",
                           []() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds(8);
                             string response;
                             string command = zowex_command + " data-set create-loadlib " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set: '" + ds + "'");
                           });
                      });
             describe("create-member",
                      [&]() -> void
                      {
                        beforeEach(
                            [&]() -> void
                            {
                              string ds = get_random_ds();
                              _ds.push_back(ds);

                              string response;
                              string command = zowex_command + " data-set create-fb " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).ToBe(0);
                              Expect(response).ToContain("Data set created");
                            });
                        it("should error if no member name is specified",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             string response;
                             string command = zowex_command + " data-set create-member " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not find member name in dsn");
                           });
                        it("should error if the data set doesn't exist",
                           [&]() -> void
                           {
                             string ds = _ds.back() + ".GHOST";
                             string response;
                             string command = zowex_command + " data-set create-member '" + ds + "(TEST)'";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set member");
                             Expect(response).ToContain("Not found in catalog");
                           });
                        it("should create a member in a PDS",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             string response;
                             string command = zowex_command + " data-set create-member '" + ds + "(TEST)'";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set and/or member created");
                           });

                        // TODO: https://github.com/zowe/zowe-native-proto/issues/643
                        xit("should not overwrite existing members",
                            [&]() -> void
                            {
                              string ds = "'" + _ds.back() + "(TEST)'";
                              string response;
                              string command = zowex_command + " data-set create-member " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).ToBe(0);
                              Expect(response).ToContain("Data set and/or member created");

                              // Write "test" data
                              command = "echo test | " + zowex_command + " data-set write " + ds;
                              rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).ToBe(0);
                              Expect(response).ToContain("Wrote data to");

                              // Read "test" data to confirm
                              command = "echo test | " + zowex_command + " data-set view " + ds;
                              rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).ToBe(0);
                              Expect(response).ToContain("test");

                              // Create the same TEST member
                              command = "echo test | " + zowex_command + " data-set create-member " + ds;
                              rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).Not().ToBe(0);
                              Expect(response).ToContain("ERROR");

                              // Read "test" data to confirm
                              command = "echo test | " + zowex_command + " data-set view " + ds;
                              rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).ToBe(0);
                              Expect(response).ToContain("test");
                            });
                      });
             describe("create-vb",
                      [&]() -> void
                      {
                        beforeEach(
                            [&]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });
                        it("should create a data set with default attributes",
                           [&]() -> void
                           {
                             string ds = _ds.back();

                             string response;
                             string command = zowex_command + " data-set create-vb " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set list " + ds + " -a --rfc";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = parse_rfc_response(response, ",");
                             Expect(tokens[3]).ToBe("PO");
                             Expect(tokens[4]).ToBe("VB");
                             Expect(tokens[9]).ToBe("PDS");
                             Expect(tokens[5]).ToBe("255"); // lrecl
                           });
                        it("should error when the data set already exists",
                           [&]() -> void
                           {
                             string ds = _ds.back();

                             string response;
                             string command = zowex_command + " data-set create-vb " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set create-vb " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set: '" + ds + "'");
                           });
                        it("should fail to create a data set if the data set name is too long",
                           []() -> void
                           {
                             int rc = 0;
                             string ds = get_random_ds(8);
                             string response;
                             string command = zowex_command + " data-set create-vb " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set: '" + ds + "'");
                           });
                      });
             describe("delete",
                      [&]() -> void
                      {
                        beforeEach(
                            [&]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });

                        it("should delete a sequential data set",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS");

                             string response;
                             string command = zowex_command + " data-set delete " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set '" + ds + "' deleted");
                           });
                        it("should delete a partitioned data set (PDS)",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --dirblk 2");

                             string response;
                             string command = zowex_command + " data-set delete " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set '" + ds + "' deleted");
                           });
                        it("should delete a partitioned data set extended (PDSE)",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --dirblk 2 --dsntype LIBRARY");

                             string response;
                             string command = zowex_command + " data-set delete " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set '" + ds + "' deleted");
                           });

                        it("should fail to delete a non-existent data set",
                           [&]() -> void
                           {
                             string ds = _ds.back();

                             string response;
                             string command = zowex_command + " data-set delete " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not delete data set: '" + ds + "'");
                           });
                        it("should fail to delete a data set if the data set name is too long",
                           []() -> void
                           {
                             string ds = get_random_ds(8);

                             string response;
                             string command = zowex_command + " data-set delete " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not delete data set: '" + ds + "'");
                           });

                        // TODO: What do?
                        xit("should fail to delete a data set if not authorized", []() -> void {});
                        // TODO: What do?
                        xit("should fail to delete a data set that is currently in use", []() -> void {});

                        // TODO: https://github.com/zowe/zowe-native-proto/issues/665
                        xit("should delete multiple data sets specified in a list", []() -> void {});

                        // TODO: https://github.com/zowe/zowe-native-proto/issues/664
                        xit("should delete a data set using the force option even if it has members", []() -> void {});
                        xit("should not delete a data set with the force option if it is in use", []() -> void {});

                        // TODO: https://github.com/zowe/zowe-native-proto/issues/666
                        xit("should delete a VSAM KSDS data set", []() -> void {});
                        xit("should delete a VSAM ESDS data set", []() -> void {});
                        xit("should delete a VSAM RRDS data set", []() -> void {});
                        xit("should delete a VSAM LDS data set", []() -> void {});

                        // TODO: https://github.com/zowe/zowe-native-proto/issues/666
                        xit("should delete a generation data group (GDG) base when empty", []() -> void {});
                        xit("should delete a generation data group (GDG) base and all its generations", []() -> void {});
                        xit("should delete a specific generation of a GDG", []() -> void {});
                        xit("should error when attempting to delete a GDG base with generations without the PURGE or FORCE option", []() -> void {});
                      });
             describe("list",
                      [&]() -> void
                      {
                        beforeEach(
                            [&]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });
                        it("should list a data set",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds);

                             string response;
                             string command = zowex_command + " data-set list " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(ds);
                           });
                        it("should list data sets based on pattern and warn about listing too many data sets",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds);
                             _create_ds(ds + ".T00");
                             _ds.push_back(ds + ".T00");

                             string response;
                             string pattern = ds;
                             string command = zowex_command + " data-set list " + pattern + " --me 1";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                             Expect(response).ToContain(ds);
                             Expect(response).Not().ToContain(ds + ".T00");
                           });

                        it("should list up to the max entries specified and not warn",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds);
                             _create_ds(ds + ".T00");
                             _ds.push_back(ds + ".T00");

                             string response;
                             string pattern = ds;
                             string command = zowex_command + " data-set list " + pattern + " --no-warn --me 1";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(ds);
                             Expect(response).Not().ToContain(ds + ".T00");
                           });

                        it("should warn when listing a non-existent data set",
                           [&]() -> void
                           {
                             string ds = _ds.back();

                             string response;
                             string command = zowex_command + " data-set list " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                             Expect(response).ToContain("Warning: no matching results found");
                           });
                        it("should error when the data set name is too long",
                           [&]() -> void
                           {
                             string ds = get_random_ds(8);

                             string response;
                             string command = zowex_command + " data-set list " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: data set pattern exceeds 44 character length limit");
                           });

                        // TODO: https://github.com/zowe/zowe-native-proto/issues/666
                        xit("should list information for a VSAM KSDS data set", []() -> void {});
                        xit("should list information for a VSAM ESDS data set", []() -> void {});
                        xit("should list information for a VSAM RRDS data set", []() -> void {});
                        xit("should list information for a VSAM LDS data set", []() -> void {});

                        // TODO: https://github.com/zowe/zowe-native-proto/issues/666
                        xit("should list generations of a generation data group (GDG) base", []() -> void {});
                        xit("should list specific generation of a GDG", []() -> void {});
                      });
             describe("list-members",
                      [&]() -> void
                      {
                        string data_set = "SYS1.MACLIB";
                        it("should list a member of a data set",
                           [&]() -> void
                           {
                             string response;
                             string command = zowex_command + " data-set lm " + data_set + " --no-warn --me 1";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                           });
                        it("should warn when listing members of a data set with many members",
                           [&]() -> void
                           {
                             int rc = 0;
                             string response;
                             string command = zowex_command + " data-set lm " + data_set + " --me 1";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                           });
                        it("should error when the data set name is too long",
                           [&]() -> void
                           {
                             string ds = get_random_ds(8);
                             string response;
                             string command = zowex_command + " data-set lm " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                           });
                        it("should fail if the data set doesn't exist",
                           [&]() -> void
                           {
                             string ds = _ds.back() + ".GHOST";
                             string response;
                             string command = zowex_command + " data-set lm " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not list members: '" + ds + "'");
                           });
                      });
             // TODO: https://github.com/zowe/zowe-native-proto/issues/380
             xdescribe("restore",
                       [&]() -> void
                       {
                         beforeEach(
                             [&]() -> void
                             {
                               _ds.push_back(get_random_ds());
                               _create_ds(_ds.back());
                             });
                         it("should restore a data set",
                            [&]() -> void
                            {
                              string ds = _ds.back();

                              string response;
                              string command = zowex_command + " data-set restore " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).ToBe(0);
                              Expect(response).ToContain("Data set '" + ds + "' restored");
                            });
                         it("should fail to restore a non-existent backup",
                            [&]() -> void
                            {
                              string ds = _ds.back() + ".GHOST";

                              string response;
                              string command = zowex_command + " data-set restore " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).Not().ToBe(0);
                              Expect(response).ToContain("Error: could not restore data set: '" + ds + "'");
                            });
                         // TODO: What do?
                         xit("should fail to restore if not authorized", [&]() -> void {});
                       });
             describe("view",
                      [&]() -> void
                      {
                        beforeEach(
                            [&]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });
                        it("should view the content of a sequential data set",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS");
                             string response;

                             string random_string = get_random_string(80, false);
                             string command = "echo " + random_string + " | " + zowex_command + " data-set write " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "'");

                             command = zowex_command + " data-set view " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(random_string);
                           });
                        it("should view the content of a PDS member",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --dirblk 2");
                             string response;

                             string random_string = get_random_string(80, false);
                             string command = "echo " + random_string + " | " + zowex_command + " data-set write '" + ds + "(TEST)'";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "(TEST)'");

                             command = zowex_command + " data-set view '" + ds + "(TEST)'";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(random_string);
                           });
                        it("should view a data set with different encoding",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS");
                             string response;
                             string command = "echo 'test!' | " + zowex_command + " data-set write " + ds + " --local-encoding IBM-1047 --encoding IBM-1147";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "'");

                             command = zowex_command + " data-set view " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("test|");

                             command = zowex_command + " data-set view " + ds + " --local-encoding IBM-1047 --encoding IBM-1047";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("test|");

                             command = zowex_command + " data-set view " + ds + " --local-encoding IBM-1047 --encoding IBM-1147";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("test!");
                           });
                        it("should fail to view a non-existent data set",
                           [&]() -> void
                           {
                             string ds = _ds.back() + ".GHOST";
                             string response;
                             string command = zowex_command + " data-set view " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not read data set: '" + ds + "'");
                           });
                        it("should error when the data set name too long",
                           [&]() -> void
                           {
                             string ds = get_random_ds(8);
                             string response;
                             string command = zowex_command + " data-set view " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not read data set: '" + ds + "'");
                           });

                        // What do?
                        xit("should fail to view a data set if not authorized", []() -> void {});

                        // TODO: https://github.com/zowe/zowe-native-proto/issues/666
                        xit("should view the content of a VSAM KSDS data set", []() -> void {});
                        xit("should view the content of a VSAM ESDS data set", []() -> void {});
                        xit("should view the content of a VSAM RRDS data set", []() -> void {});
                        xit("should view the content of a VSAM LDS data set", []() -> void {});
                      });
             describe("write",
                      [&]() -> void
                      {
                        beforeEach(
                            [&]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });
                        it("should write content to a sequential data set",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS");

                             string response;
                             string random_string = get_random_string(80, false);
                             string command = "echo " + random_string + " | " + zowex_command + " data-set write " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "'");

                             command = zowex_command + " data-set view " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(random_string);
                           });
                        it("should write content to a PDS member",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --dirblk 2");

                             string response;
                             string command = zowex_command + " data-set create-member '" + ds + "(TEST)'";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set and/or member created");

                             string random_string = get_random_string(80, false);
                             command = "echo " + random_string + " | " + zowex_command + " data-set write '" + ds + "(TEST)'";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "(TEST)'");

                             command = zowex_command + " data-set view '" + ds + "(TEST)'";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(random_string);
                           });
                        it("should overwrite content in a sequential data set",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS");

                             string response;
                             string random_string = get_random_string(80, false);
                             string random_string1 = get_random_string(80, false);
                             string random_string2 = get_random_string(80, false);
                             string command = "echo '" + random_string + "\n" + random_string1 + "' | " + zowex_command + " data-set write " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "'");

                             command = zowex_command + " data-set view " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(random_string);
                             Expect(response).ToContain(random_string1);
                             Expect(response).Not().ToContain(random_string2);

                             command = "echo " + random_string2 + " | " + zowex_command + " data-set write " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "'");

                             command = zowex_command + " data-set view " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(random_string2);
                             Expect(response).Not().ToContain(random_string);
                             Expect(response).Not().ToContain(random_string1);
                           });
                        it("should overwrite content in a PDS member",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --dirblk 2");

                             string response;
                             string command = zowex_command + " data-set create-member '" + ds + "(TEST)'";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set and/or member created");

                             string random_string = get_random_string(80, false);
                             string random_string1 = get_random_string(80, false);
                             string random_string2 = get_random_string(80, false);
                             command = "echo '" + random_string + "\n" + random_string1 + "' | " + zowex_command + " data-set write '" + ds + "(TEST)'";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "(TEST)'");

                             command = zowex_command + " data-set view '" + ds + "(TEST)'";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(random_string);
                             Expect(response).ToContain(random_string1);
                             Expect(response).Not().ToContain(random_string2);

                             command = "echo " + random_string2 + " | " + zowex_command + " data-set write '" + ds + "(TEST)'";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "(TEST)'");

                             command = zowex_command + " data-set view '" + ds + "(TEST)'";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(random_string2);
                             Expect(response).Not().ToContain(random_string);
                             Expect(response).Not().ToContain(random_string1);
                           });

                        it("should only print the etag when requested",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS");
                             string response;
                             string command = "echo 'test' | " + zowex_command + " data-set write " + ds + " --etag-only";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("etag: 8890283"); // etag for "test"
                             Expect(response).Not().ToContain("Wrote data to '" + ds + "'");
                           });

                        it("should fail if the provided etag is different from the current etag",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS");
                             string response;
                             string command = "echo 'test' | " + zowex_command + " data-set write " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "'");

                             command = "echo 'zowe' | " + zowex_command + " data-set write " + ds + " --etag 8bb0280"; // etag for "zowe"
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Etag mismatch: expected 8bb0280, actual 8890283");
                             Expect(response).Not().ToContain("Wrote data to '" + ds + "'");
                           });

                        // TODO: https://github.com/zowe/zowe-native-proto/issues/676
                        xit("should fail if the provided etag is different and evaluates to a number",
                            [&]() -> void
                            {
                              string ds = _ds.back();
                              _create_ds(ds, "--dsorg PS");
                              string response;
                              string command = "echo 'zowe' | " + zowex_command + " data-set write " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).ToBe(0);
                              Expect(response).ToContain("Wrote data to '" + ds + "'");

                              command = "echo 'test' | " + zowex_command + " data-set write " + ds + " --etag 8890283"; // etag for "test"
                              rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).Not().ToBe(0);
                              Expect(response).ToContain("Etag mismatch: expected 8890283, actual 8bb0280");
                              Expect(response).Not().ToContain("Wrote data to '" + ds + "'");
                            });

                        it("should write content to a data set with different encoding",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS");
                             string response;
                             string command = "echo 'test!' | " + zowex_command + " data-set write " + ds + " --local-encoding IBM-1047 --encoding IBM-1147";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "'");

                             command = zowex_command + " data-set view " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("test|");

                             command = zowex_command + " data-set view " + ds + " --rfb";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("a3 85 a2 a3 4f 15");
                           });

                        it("should fail to write to a non-existent data set",
                           [&]() -> void
                           {
                             string ds = _ds.back() + ".GHOST";
                             string response;
                             string command = "echo 'test' | " + zowex_command + " data-set write " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not write to data set: '" + ds + "'");
                           });
                        it("should error when the data set name is too long",
                           []() -> void
                           {
                             string ds = get_random_ds(8);
                             string response;
                             string command = "echo 'test' | " + zowex_command + " data-set write " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not write to data set: '" + ds + "'");
                           });
                        it("should write content from a USS file to a sequential data set",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS --lrecl 120 --blksize 18480");
                             string response;
                             string file = "./makefile";
                             string get_uss_file_command = zowex_command + " uss view '" + file + "'";
                             string command = get_uss_file_command + " | " + zowex_command + " data-set write " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Wrote data to '" + ds + "'");

                             ifstream in(file.c_str(), ifstream::in);
                             Expect(in.is_open()).ToBe(true);

                             in.seekg(0, ios::end);
                             size_t size = in.tellg();
                             in.seekg(0, ios::beg);

                             vector<char> raw_data(size);
                             in.read(&raw_data[0], size);
                             in.close();

                             string makefile_content;
                             makefile_content.assign(raw_data.begin(), raw_data.end());

                             command = zowex_command + " data-set view " + ds;
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain(makefile_content);
                           });

                        describe("BPAM member writes",
                                 [&]() -> void
                                 {
                                   it("should write and read a PDS member using BPAM",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PO --dirblk 2");

                                        string response;
                                        string command = zowex_command + " data-set create-member '" + ds + "(PDS1)'";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Data set and/or member created");

                                        string payload = "PDSDATA";
                                        command = "echo " + payload + " | " + zowex_command + " data-set write '" + ds + "(PDS1)'";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "(PDS1)'");

                                        command = zowex_command + " data-set view '" + ds + "(PDS1)'";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain(payload);
                                      });
                                   it("should write and read a PDSE member using BPAM",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PO --dirblk 2 --dsntype LIBRARY");

                                        string response;
                                        string command = zowex_command + " data-set create-member '" + ds + "(MEM1)'";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Data set and/or member created");

                                        string payload = "MEMBERDATA";
                                        command = "echo " + payload + " | " + zowex_command + " data-set write '" + ds + "(MEM1)'";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "(MEM1)'");

                                        command = zowex_command + " data-set view '" + ds + "(MEM1)'";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain(payload);
                                      });

                                   it("should write multi-line content to a PDSE member",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PO --dirblk 2 --dsntype LIBRARY");

                                        string response;
                                        string command = zowex_command + " data-set create-member '" + ds + "(MEM2)'";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Data set and/or member created");

                                        string command_payload = "printf 'LINEA\\nLINEB\\n' | " + zowex_command + " data-set write '" + ds + "(MEM2)'";
                                        rc = execute_command_with_output(command_payload, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "(MEM2)'");

                                        command = zowex_command + " data-set view '" + ds + "(MEM2)'";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("LINEA");
                                        Expect(response).ToContain("LINEB");
                                      });

                                   it("should overwrite a PDSE member using BPAM",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PO --dirblk 2 --dsntype LIBRARY");

                                        string response;
                                        string command = zowex_command + " data-set create-member '" + ds + "(MEM3)'";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Data set and/or member created");

                                        string command_payload = "echo FIRSTDATA | " + zowex_command + " data-set write '" + ds + "(MEM3)'";
                                        rc = execute_command_with_output(command_payload, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "(MEM3)'");

                                        command_payload = "echo SECONDDATA | " + zowex_command + " data-set write '" + ds + "(MEM3)'";
                                        rc = execute_command_with_output(command_payload, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "(MEM3)'");

                                        command = zowex_command + " data-set view '" + ds + "(MEM3)'";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("SECONDDATA");
                                        Expect(response).Not().ToContain("FIRSTDATA");
                                      });

                                   it("should warn when a PDSE member line exceeds LRECL",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PO --dirblk 2 --dsntype LIBRARY --lrecl 10 --blksize 80");

                                        string response;
                                        string command = zowex_command + " data-set create-member '" + ds + "(MEM4)'";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Data set and/or member created");

                                        string long_line = "ABCDEFGHIJKLMNOPQRST";
                                        command = "echo " + long_line + " | " + zowex_command + " data-set write '" + ds + "(MEM4)'";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Warning:");
                                        Expect(response).ToContain("truncated");
                                      });
                                 });

                        describe("ASA control characters",
                                 [&]() -> void
                                 {
                                   it("should add ASA control characters for a PDS member",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PO --dirblk 2 --recfm FBA --lrecl 81");

                                        string response;
                                        string command = zowex_command + " data-set create-member '" + ds + "(ASA2)'";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Data set and/or member created");

                                        command = "printf 'AAA\\nBBB\\n' | " + zowex_command +
                                                 " data-set write '" + ds + "(ASA2)' --local-encoding UTF-8 --encoding IBM-1047";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "(ASA2)'");

                                        command = zowex_command + " data-set view '" + ds + "(ASA2)' --encoding binary --rfb";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("40 c1 c1 c1");
                                        Expect(response).ToContain("40 c2 c2 c2");
                                      });
                                   it("should add ASA control characters for FBA data sets",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PS --recfm FBA --lrecl 81");

                                        string response;
                                        string command = "printf 'AAA\\nBBB\\n' | " + zowex_command +
                                                         " data-set write " + ds + " --local-encoding UTF-8 --encoding IBM-1047";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "'");

                                        command = zowex_command + " data-set view " + ds + " --encoding binary --rfb";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("40 c1 c1 c1");
                                        Expect(response).ToContain("40 c2 c2 c2");
                                      });

                                   it("should convert a single blank line into ASA double space",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PS --recfm FBA --lrecl 81");

                                        string response;
                                        string command = "printf 'AAA\\n\\nBBB\\n' | " + zowex_command +
                                                         " data-set write " + ds + " --local-encoding UTF-8 --encoding IBM-1047";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "'");

                                        command = zowex_command + " data-set view " + ds + " --encoding binary --rfb";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("f0 c2 c2 c2");
                                      });

                                   it("should handle multiple blank lines with ASA overflow",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PS --recfm FBA --lrecl 81");

                                        string response;
                                        string command = "printf 'AAA\\n\\n\\n\\nDDD\\n' | " + zowex_command +
                                                         " data-set write " + ds + " --local-encoding UTF-8 --encoding IBM-1047";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "'");

                                        command = zowex_command + " data-set view " + ds + " --encoding binary --rfb";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("60");
                                        Expect(response).ToContain("f0 c4 c4 c4");
                                      });

                                   it("should convert form feed to ASA page break",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PS --recfm FBA --lrecl 81");

                                        string response;
                                        string command = "printf '\\fEEE\\n' | " + zowex_command +
                                                         " data-set write " + ds + " --local-encoding UTF-8 --encoding IBM-1047";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "'");

                                        command = zowex_command + " data-set view " + ds + " --encoding binary --rfb";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("f1 c5 c5 c5");
                                      });

                                   it("should honor input-asa when writing ASA records",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PS --recfm FBA --lrecl 81");

                                        string response;
                                        string command = "printf '0ABC\\n1DEF\\n' | " + zowex_command +
                                                         " data-set write " + ds + " --input-asa --local-encoding UTF-8 --encoding IBM-1047";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "'");

                                        command = zowex_command + " data-set view " + ds + " --encoding binary --rfb";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("f0 c1 c2 c3");
                                        Expect(response).ToContain("f1 c4 c5 c6");
                                      });

                                   it("should honor input-asa for a PDS member via BPAM",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PO --dirblk 2 --dsntype LIBRARY --recfm FBA --lrecl 81");

                                        string response;
                                        string command = zowex_command + " data-set create-member '" + ds + "(ASA1)'";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Data set and/or member created");

                                        command = "printf '0ABC\\n1DEF\\n' | " + zowex_command +
                                                 " data-set write '" + ds + "(ASA1)' --input-asa --local-encoding UTF-8 --encoding IBM-1047";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "(ASA1)'");

                                        command = zowex_command + " data-set view '" + ds + "(ASA1)' --encoding binary --rfb";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("f0 c1 c2 c3");
                                        Expect(response).ToContain("f1 c4 c5 c6");
                                      });

                                   it("should stream ASA conversion to a member via pipe-path",
                                      [&]() -> void
                                      {
                                        string ds = _ds.back();
                                        _create_ds(ds, "--dsorg PO --dirblk 2 --dsntype LIBRARY --recfm FBA --lrecl 81");

                                        string response;
                                        string command = zowex_command + " data-set create-member '" + ds + "(ASA3)'";
                                        int rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Data set and/or member created");

                                        const string pipe_path = "/tmp/zowex_ds_pipe_" + get_random_string(10);
                                        mkfifo(pipe_path.c_str(), 0777);

                                        const string payload = "AAA\n\nBBB\n";
                                        const auto encoded = zbase64::encode(payload.c_str(), payload.size());
                                        const string encoded_payload(encoded.begin(), encoded.end());

                                        std::thread writer([&]() -> void
                                                          {
                                                            int fd = -1;
                                                            for (int attempt = 0; attempt < 100 && fd == -1; ++attempt)
                                                            {
                                                              fd = open(pipe_path.c_str(), O_WRONLY | O_NONBLOCK);
                                                              if (fd == -1)
                                                              {
                                                                usleep(10000);
                                                              }
                                                            }
                                                            if (fd != -1)
                                                            {
                                                              write(fd, encoded_payload.data(), encoded_payload.size());
                                                              close(fd);
                                                            }
                                                          });

                                        command = zowex_command + " data-set write '" + ds + "(ASA3)' --pipe-path " + pipe_path +
                                                 " --local-encoding UTF-8 --encoding IBM-1047";
                                        rc = execute_command_with_output(command, response);
                                        writer.join();
                                        unlink(pipe_path.c_str());

                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("Wrote data to '" + ds + "(ASA3)'");

                                        command = zowex_command + " data-set view '" + ds + "(ASA3)' --encoding binary --rfb";
                                        rc = execute_command_with_output(command, response);
                                        ExpectWithContext(rc, response).ToBe(0);
                                        Expect(response).ToContain("40 c1 c1 c1");
                                        Expect(response).ToContain("f0 c2 c2 c2");
                                        Expect(response).ToContain("40 c2 c2 c2");
                                      });
                                 });

                        it("should fail to write a member in a sequential data set",
                           [&]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PS");

                             string response;
                             string command = "echo TESTDATA | " + zowex_command + " data-set write '" + ds + "(BAD1)'";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not write to data set");
                           });

                        // TODO: What do?
                        xit("should fail if the data set is deleted before the write operation is completed", []() -> void {});
                        // TODO: What do?
                        xit("should fail to write to a data set if not authorized", []() -> void {});
                      });
           });
}
