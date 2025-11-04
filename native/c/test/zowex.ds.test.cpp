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
             // Define a generic helper lambda for creating data sets
             auto _create_ds = [](const string &ds_name, const string &ds_options = "") -> void
             {
               string response;
               string command = zowex_command + " data-set create " + ds_name + " " + ds_options;
               int rc = execute_command_with_output(command, response);
               ExpectWithContext(rc, response).ToBe(0);
               Expect(response).ToContain("Data set created");
             };

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

             // https://github.com/zowe/zowe-native-proto/issues/640
             xdescribe("compress",
                       [&_ds, _create_ds]() -> void
                       {
                         beforeEach(
                             [&_ds]() -> void
                             {
                               _ds.push_back(get_random_ds());
                             });
                         it("should error when the data set is PS",
                            [_ds, _create_ds]() -> void
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
                            [_ds, _create_ds]() -> void
                            {
                              string ds = _ds.back();
                              _create_ds(ds, "--dsorg PO --dsntype LIBRARY");

                              string response;
                              string command = zowex_command + " data-set compress " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).Not().ToBe(0);
                              Expect(response).ToContain("Error: data set '" + ds + "' is not a PDS'");
                            });

                         it("should error when the data set doesn't exist",
                            [_ds]() -> void
                            {
                              string ds = _ds.back() + ".GHOST";

                              string response;
                              string command = zowex_command + " data-set compress " + ds;
                              int rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).Not().ToBe(0);
                              Expect(response).ToContain("Error: data set '" + ds + "' is not a PDS'");
                            });

                         xit("should compress a data set",
                             [_ds, _create_ds]() -> void
                             {
                               string ds = _ds.back();
                               _create_ds(ds, "--dsorg PO");

                               string response;
                               string command = zowex_command + " data-set compress " + ds;
                               int rc = execute_command_with_output(command, response);
                               ExpectWithContext(rc, response).ToBe(0);
                               Expect(response).ToContain("Data set compressed");
                             });

                         xit("should error when the data set is VSAM", []() -> void {});
                         xit("should error when the data set is GDG", []() -> void {});
                         xit("should error when the data set is ALIAS", []() -> void {});
                       });
             describe("create",
                      [&_ds, _create_ds]() -> void
                      {
                        beforeEach(
                            [&_ds]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });
                        it("should error when the data set already exists",
                           [&_ds, _create_ds]() -> void
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
                           [&_ds, _create_ds]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds);

                             string response;
                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToBe("PS");
                             Expect(tokens[4]).ToBe("FB");
                           });

                        it("should create a simple PDS/E data set - dsorg: PO-E",
                           [&_ds, _create_ds]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --dsntype LIBRARY");

                             string response;
                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToBe("PO-E");
                           });

                        it("should create a data set - recfm:VB dsorg:PO",
                           [&_ds, _create_ds]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--recfm VB --dsorg PO");

                             string response;

                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToBe("PO");
                             Expect(tokens[4]).ToBe("VB");
                           });

                        it("should create a PS data set - recfm:VB dsorg:PS",
                           [&_ds, _create_ds]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--recfm VB --dsorg PS");

                             string response;
                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToBe("PS");
                             Expect(tokens[4]).ToBe("VB");
                           });

                        // https://github.com/zowe/zowe-native-proto/pull/625
                        it("should create a data set - dsorg: PO, primary: 10, secondary: 2, lrecl: 20, blksize:10, dirblk: 5, alcunit: CYL",
                           [&_ds, _create_ds]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --primary 10 --secondary 2 --lrecl 20 --blksize 10 --dirblk 5 --alcunit CYL");

                             string response;
                             string command = zowex_command + " data-set list " + ds + " -a --rfc";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToBe("PO");
                             Expect(tokens[4]).ToBe("FB");
                             // primary = 10
                             // secondary = 2
                             // lrecl = 20
                             // blksize = 10
                             // dirblk = 5
                             // alcunit = CYL
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
                             Expect(tokens[1]).ToBe("PO-E");
                             Expect(tokens[4]).ToBe("VB");
                             // lrecl = 32756
                           });

                        it("should fail to create a data set if the data set already exists",
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
                             Expect(tokens[1]).ToBe("PO-E");
                             Expect(tokens[4]).ToBe("FB");
                             // lrecl = 80
                           });
                        it("should fail to create a data set if the data set already exists",
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
                             Expect(tokens[1]).ToBe("PO-E");
                             Expect(tokens[4]).ToBe("U");
                             // lrecl = 0
                           });
                        it("should fail to create a data set if the data set already exists",
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
                      [&_ds]() -> void
                      {
                        beforeEach(
                            [&_ds]() -> void
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
                           [_ds]() -> void
                           {
                             string ds = _ds.back();
                             string response;
                             string command = zowex_command + " data-set create-member " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not find member name in dsn");
                           });
                        it("should error if the data set doesn't exist",
                           [_ds]() -> void
                           {
                             string ds = _ds.back() + ".GHOST";
                             string response;
                             string command = zowex_command + " data-set create-member \"" + ds + "(TEST)\"";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: could not create data set member");
                             Expect(response).ToContain("Not found in catalog");
                           });
                        it("should create a member in a PDS",
                           [_ds]() -> void
                           {
                             string ds = _ds.back();
                             string response;
                             string command = zowex_command + " data-set create-member \"" + ds + "(TEST)\"";
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set and/or member created");
                           });

                        // https://github.com/zowe/zowe-native-proto/issues/643
                        xit("should not overwrite existing members",
                            [_ds]() -> void
                            {
                              string ds = "\"" + _ds.back() + "(TEST)\"";
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
                              Expect(response).ToContain("ERROR"); // TODO: check for specific error message

                              // Read "test" data to confirm
                              command = "echo test | " + zowex_command + " data-set view " + ds;
                              rc = execute_command_with_output(command, response);
                              ExpectWithContext(rc, response).ToBe(0);
                              Expect(response).ToContain("test");
                            });
                      });
             describe("create-vb",
                      [&_ds]() -> void
                      {
                        beforeEach(
                            [&_ds]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });
                        it("should create a data set with default attributes",
                           [_ds]() -> void
                           {
                             string ds = _ds.back();
                             TestLog("ds: " + ds);

                             string response;
                             string command = zowex_command + " data-set create-vb " + ds;
                             TestLog(command);
                             int rc = execute_command_with_output(command, response);
                             TestLog("rc: " + to_string(rc));
                             TestLog("response: " + response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set created");

                             command = zowex_command + " data-set list " + ds + " -a --rfc";
                             rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             vector<string> tokens = split_rfc_response(response, ",");
                             Expect(tokens[1]).ToBe("PO-E");
                             Expect(tokens[4]).ToBe("VB");
                             // lrecl = 255
                           });
                        it("should error when the data set already exists",
                           [_ds]() -> void
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
                      [&_ds, _create_ds]() -> void
                      {
                        beforeEach(
                            [&_ds]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });

                        it("should delete a sequential data set",
                           [_ds, _create_ds]() -> void
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
                           [_ds, _create_ds]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO");

                             string response;
                             string command = zowex_command + " data-set delete " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set '" + ds + "' deleted");
                           });
                        it("should delete a partitioned data set extended (PDSE)",
                           [_ds, _create_ds]() -> void
                           {
                             string ds = _ds.back();
                             _create_ds(ds, "--dsorg PO --dsntype LIBRARY");

                             string response;
                             string command = zowex_command + " data-set delete " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(0);
                             Expect(response).ToContain("Data set '" + ds + "' deleted");
                           });

                        it("should fail to delete a non-existent data set",
                           [&_ds, _create_ds]() -> void
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
                        it("should fail to delete a data set if not authorized",
                           []() -> void {});
                        it("should fail to delete a data set that is currently in use",
                           []() -> void {});

                        // https://github.com/zowe/zowe-native-proto/issues/665
                        xit("should delete multiple data sets specified in a list", []() -> void {});

                        // https://github.com/zowe/zowe-native-proto/issues/664
                        xit("should delete a data set using the force option even if it has members", []() -> void {});
                        xit("should not delete a data set with the force option if it is in use", []() -> void {});

                        xit("should delete a VSAM KSDS data set", []() -> void {});
                        xit("should delete a VSAM ESDS data set", []() -> void {});
                        xit("should delete a VSAM RRDS data set", []() -> void {});
                        xit("should delete a VSAM LDS data set", []() -> void {});

                        xit("should delete a generation data group (GDG) base when empty", []() -> void {});
                        xit("should delete a generation data group (GDG) base and all its generations", []() -> void {});
                        xit("should delete a specific generation of a GDG", []() -> void {});
                        xit("should error when attempting to delete a GDG base with generations without the PURGE or FORCE option", []() -> void {});
                      });
             describe("list",
                      [&_ds, _create_ds]() -> void
                      {
                        beforeEach(
                            [&_ds]() -> void
                            {
                              _ds.push_back(get_random_ds());
                            });
                        it("should list a data set",
                           [_ds, _create_ds]() -> void
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
                           [&_ds, _create_ds]() -> void
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
                           [&_ds, _create_ds]() -> void
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
                           [&_ds, _create_ds]() -> void
                           {
                             string ds = _ds.back();

                             string response;
                             string command = zowex_command + " data-set list " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).ToBe(RTNCD_WARNING);
                             Expect(response).ToContain("Warning: no matching results found");
                           });
                        it("should error when the data set name is too long",
                           [&_ds, _create_ds]() -> void
                           {
                             string ds = get_random_ds(8);

                             string response;
                             string command = zowex_command + " data-set list " + ds;
                             int rc = execute_command_with_output(command, response);
                             ExpectWithContext(rc, response).Not().ToBe(0);
                             Expect(response).ToContain("Error: data set pattern exceeds 44 character length limit");
                           });

                        xit("should list information for a VSAM KSDS data set", []() -> void {});
                        xit("should list information for a VSAM ESDS data set", []() -> void {});
                        xit("should list information for a VSAM RRDS data set", []() -> void {});
                        xit("should list information for a VSAM LDS data set", []() -> void {});

                        xit("should list generations of a generation data group (GDG) base", []() -> void {});
                        xit("should list specific generation of a GDG", []() -> void {});
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
                      []() -> void
                      {
                        it("should restore a data set",
                           []() -> void {});
                        it("should fail to restore a non-existent backup",
                           []() -> void {});
                        it("should fail to restore if not authorized",
                           []() -> void {});
                      });
             describe("view",
                      []() -> void
                      {
                        it("should view the content of a sequential data set",
                           []() -> void {});
                        it("should view the content of a PDS member",
                           []() -> void {});
                        it("should view a specific range of lines from a data set",
                           []() -> void {});
                        xit("should view the content of a VSAM KSDS data set", []() -> void {});
                        xit("should view the content of a VSAM ESDS data set", []() -> void {});
                        xit("should view the content of a VSAM RRDS data set", []() -> void {});
                        xit("should view the content of a VSAM LDS data set", []() -> void {});
                        it("should fail to view a non-existent data set",
                           []() -> void {});
                        it("should fail to view a data set if not authorized",
                           []() -> void {});
                        it("should view a data set with different encoding",
                           []() -> void {});
                        it("should error when the data set name is invalid",
                           []() -> void {});
                      });
             describe("write",
                      []() -> void
                      {
                        it("should write content to a sequential data set",
                           []() -> void {});
                        it("should write content to a PDS member",
                           []() -> void {});
                        it("should overwrite content in a data set",
                           []() -> void {});
                        it("should append content to a data set",
                           []() -> void {});
                        it("should fail to write to a non-existent data set",
                           []() -> void {});
                        it("should fail to write to a data set if not authorized",
                           []() -> void {});
                        it("should write content to a data set with different encoding",
                           []() -> void {});
                        it("should write content from a local file to a data set",
                           []() -> void {});
                        it("should error when the data set name is too long",
                           []() -> void {});
                        it("should fail if the data set is deleted before the write operation is completed",
                           []() -> void {});
                      });
           });
}
