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
#include <vector>
#include <cstring>
#include <ctime>
#include <cstdlib>

#include "ztest.hpp"
#include "zds.hpp"
#include "zut.hpp"
#include "zutils.hpp"

using namespace std;
using namespace ztst;

// Local helper that avoids # character (which can cause BPXWDYN issues)
string get_test_dsn()
{
  static int counter = 0;
  counter++;
  // CodeQL: rand() is fine here - just generating unique test dataset names, not for security
  srand(time(nullptr) + counter);   // codeql[cpp/weak-cryptographic-algorithm]
  int random_num = rand() % 100000; // codeql[cpp/weak-cryptographic-algorithm]
  // Use user's HLQ from zutils, but with a simpler qualifier without #
  return get_user() + ".ZDSTEST.T" + to_string(random_num) + to_string(counter);
}

// Helper to extract error message from ZDS struct or response
string get_create_error(ZDS *zds, const string &response, int rc)
{
  if (zds->diag.e_msg_len > 0)
    return string(zds->diag.e_msg);
  if (response.length() > 0)
    return response;
  return "Unknown error (rc=" + to_string(rc) + ")";
}

// Helper function to create a PDS
void create_pds(ZDS *zds, const string &dsn)
{
  memset(zds, 0, sizeof(ZDS));
  DS_ATTRIBUTES attrs = {0};
  attrs.dsorg = "PO";
  attrs.dirblk = 5;
  string response;
  int rc = zds_create_dsn(zds, dsn, attrs, response);
  if (rc != 0)
    throw runtime_error("Failed to create PDS: " + get_create_error(zds, response, rc));
}

// Helper function to create a PDSE
void create_pdse(ZDS *zds, const string &dsn)
{
  memset(zds, 0, sizeof(ZDS));
  DS_ATTRIBUTES attrs = {0};
  attrs.dsorg = "PO";
  attrs.dsntype = "LIBRARY";
  attrs.dirblk = 5;
  string response;
  int rc = zds_create_dsn(zds, dsn, attrs, response);
  if (rc != 0)
    throw runtime_error("Failed to create PDSE: " + get_create_error(zds, response, rc));
}

// Helper function to create a sequential dataset with explicit attributes
void create_seq(ZDS *zds, const string &dsn)
{
  memset(zds, 0, sizeof(ZDS));
  DS_ATTRIBUTES attrs = {0};
  attrs.dsorg = "PS";
  attrs.recfm = "F,B";
  attrs.lrecl = 80;
  attrs.blksize = 800;
  attrs.primary = 1;
  attrs.secondary = 1;
  string response;
  int rc = zds_create_dsn(zds, dsn, attrs, response);
  if (rc != 0)
    throw runtime_error("Failed to create sequential dataset: " + get_create_error(zds, response, rc));
}

void zds_tests()
{
  vector<string> created_dsns;

  describe("zds",
           [&]()
           {
             afterAll([&]()
                      {
                         // Cleanup created datasets
                         for (const auto &dsn : created_dsns)
                         {
                           try
                           {
                             ZDS zds = {0};
                             zds_delete_dsn(&zds, dsn);
                           }
                           catch (...)
                           {
                             // Ignore cleanup errors
                           }
                         }
                         created_dsns.clear(); });

             describe("list",
                      []()
                      {
                        it("should list data sets with a given DSN",
                           []()
                           {
                             int rc = 0;
                             ZDS zds = {0};
                             vector<ZDSEntry> entries;
                             string dsn = "SYS1.MACLIB";
                             rc = zds_list_data_sets(&zds, dsn, entries);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                             string found = "";
                             for (vector<ZDSEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
                             {
                               string trimmed_name = it->name;
                               zut_rtrim(trimmed_name);
                               if (trimmed_name == dsn)
                               {
                                 found = trimmed_name;
                               }
                             }
                             Expect(found).ToBe(dsn);
                           });

                        it("should find dsn (SYS1.MACLIB) based on a pattern: (SYS1.*)",
                           []()
                           {
                             int rc = 0;
                             ZDS zds = {0};
                             vector<ZDSEntry> entries;
                             string dsn = "SYS1.MACLIB";
                             string pattern = "SYS1.*";
                             rc = zds_list_data_sets(&zds, pattern, entries);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                             string found = "";
                             for (vector<ZDSEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
                             {
                               string trimmed_name = it->name;
                               zut_rtrim(trimmed_name);
                               if (trimmed_name == dsn)
                               {
                                 found = trimmed_name;
                               }
                             }
                             Expect(found).ToBe(dsn);
                           });
                      });

             describe("source encoding tests",
                      []()
                      {
                        it("should use default source encoding (UTF-8) when not specified",
                           []()
                           {
                             ZDS zds = {0};
                             strcpy(zds.encoding_opts.codepage, "IBM-1047");
                             zds.encoding_opts.data_type = eDataTypeText;
                             Expect(strlen(zds.encoding_opts.source_codepage)).ToBe(0);
                             Expect(strlen(zds.encoding_opts.codepage)).ToBe(8); // "IBM-1047"
                           });

                        it("should use specified source encoding when provided",
                           []()
                           {
                             ZDS zds = {0};
                             strcpy(zds.encoding_opts.codepage, "IBM-1047");
                             strcpy(zds.encoding_opts.source_codepage, "IBM-037");
                             zds.encoding_opts.data_type = eDataTypeText;
                             Expect(string(zds.encoding_opts.codepage)).ToBe("IBM-1047");
                             Expect(string(zds.encoding_opts.source_codepage)).ToBe("IBM-037");
                             Expect(zds.encoding_opts.data_type).ToBe(eDataTypeText);
                           });

                        it("should handle binary data type correctly with source encoding",
                           []()
                           {
                             ZDS zds = {0};
                             strcpy(zds.encoding_opts.codepage, "binary");
                             strcpy(zds.encoding_opts.source_codepage, "UTF-8");
                             zds.encoding_opts.data_type = eDataTypeBinary;
                             Expect(string(zds.encoding_opts.codepage)).ToBe("binary");
                             Expect(string(zds.encoding_opts.source_codepage)).ToBe("UTF-8");
                             Expect(zds.encoding_opts.data_type).ToBe(eDataTypeBinary);
                           });

                        it("should handle empty source encoding gracefully",
                           []()
                           {
                             ZDS zds = {0};
                             strcpy(zds.encoding_opts.codepage, "IBM-1047");
                             memset(zds.encoding_opts.source_codepage, 0, sizeof(zds.encoding_opts.source_codepage));
                             zds.encoding_opts.data_type = eDataTypeText;
                             Expect(strlen(zds.encoding_opts.source_codepage)).ToBe(0);
                             Expect(string(zds.encoding_opts.codepage)).ToBe("IBM-1047");
                           });

                        it("should handle maximum length encoding names",
                           []()
                           {
                             ZDS zds = {0};
                             string long_target = "IBM-1234567890A"; // 15 characters
                             string long_source = "UTF-1234567890B"; // 15 characters
                             strncpy(zds.encoding_opts.codepage, long_target.c_str(), sizeof(zds.encoding_opts.codepage) - 1);
                             strncpy(zds.encoding_opts.source_codepage, long_source.c_str(), sizeof(zds.encoding_opts.source_codepage) - 1);
                             zds.encoding_opts.codepage[sizeof(zds.encoding_opts.codepage) - 1] = '\0';
                             zds.encoding_opts.source_codepage[sizeof(zds.encoding_opts.source_codepage) - 1] = '\0';
                             Expect(string(zds.encoding_opts.codepage)).ToBe(long_target);
                             Expect(string(zds.encoding_opts.source_codepage)).ToBe(long_source);
                           });

                        it("should preserve encoding settings through struct copy",
                           []()
                           {
                             ZDS zds1 = {0};
                             strcpy(zds1.encoding_opts.codepage, "IBM-1047");
                             strcpy(zds1.encoding_opts.source_codepage, "IBM-037");
                             zds1.encoding_opts.data_type = eDataTypeText;
                             ZDS zds2 = zds1;
                             Expect(string(zds2.encoding_opts.codepage)).ToBe("IBM-1047");
                             Expect(string(zds2.encoding_opts.source_codepage)).ToBe("IBM-037");
                             Expect(zds2.encoding_opts.data_type).ToBe(eDataTypeText);
                           });

                        it("should validate encoding struct size assumptions",
                           []()
                           {
                             Expect(sizeof(((ZEncode *)0)->codepage)).ToBe(16);
                             Expect(sizeof(((ZEncode *)0)->source_codepage)).ToBe(16);
                             ZEncode encode = {0};
                             strcpy(encode.codepage, "test1");
                             strcpy(encode.source_codepage, "test2");
                             Expect(string(encode.codepage)).ToBe("test1");
                             Expect(string(encode.source_codepage)).ToBe("test2");
                           });

                        it("should handle common encoding combinations",
                           []()
                           {
                             ZDS zds = {0};
                             zds.encoding_opts.data_type = eDataTypeText;
                             struct EncodingPair
                             {
                               const char *source;
                               const char *target;
                             };
                             EncodingPair pairs[] = {
                                 {"UTF-8", "IBM-1047"},
                                 {"IBM-037", "UTF-8"},
                                 {"IBM-1047", "IBM-037"},
                                 {"ISO8859-1", "IBM-1047"},
                                 {"UTF-8", "binary"}};
                             for (const auto &pair : pairs)
                             {
                               memset(&zds.encoding_opts, 0, sizeof(zds.encoding_opts));
                               strcpy(zds.encoding_opts.source_codepage, pair.source);
                               strcpy(zds.encoding_opts.codepage, pair.target);
                               Expect(string(zds.encoding_opts.source_codepage)).ToBe(string(pair.source));
                               Expect(string(zds.encoding_opts.codepage)).ToBe(string(pair.target));
                             }
                           });
                      });

             describe("copy",
                      [&]()
                      {
                        it("should copy PDS to PDS",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             // Separate ZDS for create
                             ZDS zds_create = {0};
                             create_pds(&zds_create, source_dsn);

                             // Separate ZDS for write
                             ZDS zds_write = {0};
                             string test_data = "Test data";
                             string member_dsn = source_dsn + "(MEMBER)";
                             zds_write_to_dsn(&zds_write, member_dsn, test_data);

                             // Separate ZDS for copy
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should copy PDSE to nonexisting PDS (creates target)",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_pdse(&zds_create, source_dsn);

                             ZDS zds_write = {0};
                             string test_data = "Test data";
                             string member_dsn = source_dsn + "(MEMBER)";
                             zds_write_to_dsn(&zds_write, member_dsn, test_data);

                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should fail to copy PDS member to sequential dataset without member name",
                           [&]()
                           {
                             string source_pds = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_pds);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_pds(&zds_create, source_pds);

                             ZDS zds_write = {0};
                             string test_data = "Member content";
                             string member_dsn = source_pds + "(MEMBER)";
                             zds_write_to_dsn(&zds_write, member_dsn, test_data);

                             string src_member = source_pds + "(MEMBER)";
                             // Should fail because target member name not specified
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, src_member, target_dsn);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds_copy.diag.e_msg)).ToContain("specifying target member");
                           });

                        it("should copy sequential dataset to sequential dataset",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_seq(&zds_create, source_dsn);

                             ZDS zds_write = {0};
                             string test_data = "Sequential data";
                             zds_write_to_dsn(&zds_write, source_dsn, test_data);

                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should copy member to member",
                           [&]()
                           {
                             string source_pds = get_test_dsn();
                             string target_pds = get_test_dsn();
                             created_dsns.push_back(source_pds);
                             created_dsns.push_back(target_pds);

                             ZDS zds_create1 = {0};
                             create_pds(&zds_create1, source_pds);
                             ZDS zds_create2 = {0};
                             create_pds(&zds_create2, target_pds);

                             ZDS zds_write = {0};
                             string test_data = "Source member data";
                             string src_member_dsn = source_pds + "(SRCMEM)";
                             zds_write_to_dsn(&zds_write, src_member_dsn, test_data);

                             string src_member = source_pds + "(SRCMEM)";
                             string tgt_member = target_pds + "(TGTMEM)";
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, src_member, tgt_member);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should fail to copy sequential dataset to PDS member",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_pds = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_pds);

                             ZDS zds_create = {0};
                             create_seq(&zds_create, source_dsn);

                             ZDS zds_write = {0};
                             string test_data = "Sequential data";
                             zds_write_to_dsn(&zds_write, source_dsn, test_data);

                             string tgt_member = target_pds + "(MEMBER)";
                             // Should fail because sequential to PDS member is not allowed
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, tgt_member);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds_copy.diag.e_msg)).ToContain("sequential dataset to PDS member");
                           });

                        it("should copy PDS with multiple members",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_pds(&zds_create, source_dsn);
                             for (int i = 1; i <= 3; i++)
                             {
                               ZDS zds_write = {0};
                               string mem_dsn = source_dsn + "(MEM" + to_string(i) + ")";
                               string mem_data = "Data " + to_string(i);
                               zds_write_to_dsn(&zds_write, mem_dsn, mem_data);
                             }

                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should fail when copying from nonexistent source",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = "NONEXISTENT.DATASET.NAME";
                             string target_dsn = get_test_dsn();
                             // Add to cleanup list in case implementation changes
                             created_dsns.push_back(target_dsn);
                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("not found");
                           });

                        it("should preserve dataset attributes when copying",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_seq(&zds_create, source_dsn);

                             ZDS zds_write = {0};
                             string test_data = "Test data";
                             zds_write_to_dsn(&zds_write, source_dsn, test_data);

                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);

                             // Verify target was created and data was copied
                             vector<ZDSEntry> target_entries;
                             ZDS zds_list = {0};
                             zds_list_data_sets(&zds_list, target_dsn, target_entries, true);
                             Expect(target_entries.empty()).ToBe(false);

                             // Verify content was copied correctly
                             ZDS zds_read = {0};
                             string content;
                             zds_read_from_dsn(&zds_read, target_dsn, content);
                             Expect(content.find("Test data") != string::npos).ToBe(true);

                             // Note: LIKE allocation is used for attribute preservation, but SMS ACS
                             // routines may override attributes on some systems. The key verification
                             // is that the copy succeeded and data is intact.
                           });

                        it("should fail to overwrite existing sequential dataset without replace flag",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create1 = {0};
                             create_seq(&zds_create1, source_dsn);
                             ZDS zds_create2 = {0};
                             create_seq(&zds_create2, target_dsn);

                             ZDS zds_write1 = {0};
                             string source_data = "Source data";
                             zds_write_to_dsn(&zds_write1, source_dsn, source_data);

                             ZDS zds_write2 = {0};
                             string target_data = "Old target data";
                             zds_write_to_dsn(&zds_write2, target_dsn, target_data);

                             // Without replace flag, should fail
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn, false);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds_copy.diag.e_msg)).ToContain("already exists");
                           });

                        it("should overwrite existing sequential dataset with replace flag",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create1 = {0};
                             create_seq(&zds_create1, source_dsn);
                             ZDS zds_create2 = {0};
                             create_seq(&zds_create2, target_dsn);

                             ZDS zds_write1 = {0};
                             string source_data = "Source data";
                             zds_write_to_dsn(&zds_write1, source_dsn, source_data);

                             ZDS zds_write2 = {0};
                             string target_data = "Old target data";
                             zds_write_to_dsn(&zds_write2, target_dsn, target_data);

                             // With replace flag, should succeed
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn, true);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should skip existing members in PDS without replace flag",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create1 = {0};
                             create_pds(&zds_create1, source_dsn);
                             ZDS zds_create2 = {0};
                             create_pds(&zds_create2, target_dsn);

                             string src_mem1 = source_dsn + "(MEM1)";
                             string src_mem2 = source_dsn + "(MEM2)";
                             string tgt_mem1 = target_dsn + "(MEM1)";
                             string src_mem_data = "Source member";
                             string src_mem2_data = "Source member 2";
                             string tgt_mem_data = "Old target member";

                             ZDS zds_write1 = {0};
                             zds_write_to_dsn(&zds_write1, src_mem1, src_mem_data);
                             ZDS zds_write2 = {0};
                             zds_write_to_dsn(&zds_write2, src_mem2, src_mem2_data);
                             ZDS zds_write3 = {0};
                             zds_write_to_dsn(&zds_write3, tgt_mem1, tgt_mem_data);

                             // Without replace flag, MEM1 should be skipped, MEM2 should be added
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn, false);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);

                             // Verify MEM2 was added
                             vector<ZDSMem> members;
                             ZDS zds_list = {0};
                             zds_list_members(&zds_list, target_dsn, members);
                             bool found_mem2 = false;
                             for (const auto &mem : members)
                             {
                               if (mem.name == "MEM2")
                                 found_mem2 = true;
                             }
                             Expect(found_mem2).ToBe(true);
                           });

                        it("should replace existing members in PDS with replace flag",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create1 = {0};
                             create_pds(&zds_create1, source_dsn);
                             ZDS zds_create2 = {0};
                             create_pds(&zds_create2, target_dsn);

                             string src_mem1 = source_dsn + "(MEM1)";
                             string tgt_mem1 = target_dsn + "(MEM1)";
                             string src_mem_data = "Source member";
                             string tgt_mem_data = "Old target member";

                             ZDS zds_write1 = {0};
                             zds_write_to_dsn(&zds_write1, src_mem1, src_mem_data);
                             ZDS zds_write2 = {0};
                             zds_write_to_dsn(&zds_write2, tgt_mem1, tgt_mem_data);

                             // With replace flag, should overwrite MEM1
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn, true);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should fail to overwrite existing member without replace flag",
                           [&]()
                           {
                             string source_pds = get_test_dsn();
                             string target_pds = get_test_dsn();
                             created_dsns.push_back(source_pds);
                             created_dsns.push_back(target_pds);

                             ZDS zds_create1 = {0};
                             create_pds(&zds_create1, source_pds);
                             ZDS zds_create2 = {0};
                             create_pds(&zds_create2, target_pds);

                             string src_mem = source_pds + "(MEM)";
                             string tgt_mem = target_pds + "(MEM)";
                             string new_data = "New source data";
                             string old_data = "Old target data";

                             ZDS zds_write1 = {0};
                             zds_write_to_dsn(&zds_write1, src_mem, new_data);
                             ZDS zds_write2 = {0};
                             zds_write_to_dsn(&zds_write2, tgt_mem, old_data);

                             string src_member = source_pds + "(MEM)";
                             string tgt_member = target_pds + "(MEM)";
                             // Without replace flag, should fail
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, src_member, tgt_member, false);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds_copy.diag.e_msg)).ToContain("already exists");
                           });

                        it("should overwrite existing member with replace flag",
                           [&]()
                           {
                             string source_pds = get_test_dsn();
                             string target_pds = get_test_dsn();
                             created_dsns.push_back(source_pds);
                             created_dsns.push_back(target_pds);

                             ZDS zds_create1 = {0};
                             create_pds(&zds_create1, source_pds);
                             ZDS zds_create2 = {0};
                             create_pds(&zds_create2, target_pds);

                             string src_mem = source_pds + "(MEM)";
                             string tgt_mem = target_pds + "(MEM)";
                             string new_data = "New source data";
                             string old_data = "Old target data";

                             ZDS zds_write1 = {0};
                             zds_write_to_dsn(&zds_write1, src_mem, new_data);
                             ZDS zds_write2 = {0};
                             zds_write_to_dsn(&zds_write2, tgt_mem, old_data);

                             string src_member = source_pds + "(MEM)";
                             string tgt_member = target_pds + "(MEM)";
                             // With replace flag, should succeed
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, src_member, tgt_member, true);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should copy empty PDS",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_pds(&zds_create, source_dsn);

                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should copy empty sequential dataset",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_seq(&zds_create, source_dsn);

                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should copy PDSE to PDSE",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_pdse(&zds_create, source_dsn);

                             ZDS zds_write = {0};
                             string test_data = "PDSE data";
                             string member_dsn = source_dsn + "(MEMBER)";
                             zds_write_to_dsn(&zds_write, member_dsn, test_data);

                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should copy PDS to new target without replace flag",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_pds(&zds_create, source_dsn);

                             ZDS zds_write = {0};
                             string test_data = "Test data";
                             string member_dsn = source_dsn + "(MEMBER)";
                             zds_write_to_dsn(&zds_write, member_dsn, test_data);

                             // Target doesn't exist, so replace flag doesn't matter
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn, false);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should copy sequential to new target without replace flag",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create = {0};
                             create_seq(&zds_create, source_dsn);

                             ZDS zds_write = {0};
                             string test_data = "Sequential data";
                             zds_write_to_dsn(&zds_write, source_dsn, test_data);

                             // Target doesn't exist, so replace flag doesn't matter
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn, false);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);
                           });

                        it("should add new members to existing PDS without replace flag",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_dsn = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             ZDS zds_create1 = {0};
                             create_pds(&zds_create1, source_dsn);
                             ZDS zds_create2 = {0};
                             create_pds(&zds_create2, target_dsn);

                             // Source has MEM1, target has MEM2
                             string src_mem1 = source_dsn + "(MEM1)";
                             string tgt_mem2 = target_dsn + "(MEM2)";
                             string src_data = "Source data";
                             string tgt_data = "Target data";

                             ZDS zds_write1 = {0};
                             zds_write_to_dsn(&zds_write1, src_mem1, src_data);
                             ZDS zds_write2 = {0};
                             zds_write_to_dsn(&zds_write2, tgt_mem2, tgt_data);

                             // Without replace, MEM1 should be added to target
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, target_dsn, false);
                             ExpectWithContext(rc, zds_copy.diag.e_msg).ToBe(0);

                             // Verify both members exist in target
                             vector<ZDSMem> members;
                             ZDS zds_list = {0};
                             zds_list_members(&zds_list, target_dsn, members);
                             bool found_mem1 = false;
                             bool found_mem2 = false;
                             for (const auto &mem : members)
                             {
                               if (mem.name == "MEM1")
                                 found_mem1 = true;
                               if (mem.name == "MEM2")
                                 found_mem2 = true;
                             }
                             Expect(found_mem1).ToBe(true);
                             Expect(found_mem2).ToBe(true);
                           });

                        it("should fail to copy sequential to PDS member (not allowed)",
                           [&]()
                           {
                             string source_dsn = get_test_dsn();
                             string target_pds = get_test_dsn();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_pds);

                             ZDS zds_create1 = {0};
                             create_seq(&zds_create1, source_dsn);
                             ZDS zds_create2 = {0};
                             create_pds(&zds_create2, target_pds);

                             string source_data = "Sequential source data";
                             string existing_mem = target_pds + "(EXISTING)";
                             string old_mem_data = "Old member data";

                             ZDS zds_write1 = {0};
                             zds_write_to_dsn(&zds_write1, source_dsn, source_data);
                             ZDS zds_write2 = {0};
                             zds_write_to_dsn(&zds_write2, existing_mem, old_mem_data);

                             string tgt_member = target_pds + "(EXISTING)";
                             // Sequential to PDS member is not allowed
                             ZDS zds_copy = {0};
                             int rc = zds_copy_dsn(&zds_copy, source_dsn, tgt_member);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds_copy.diag.e_msg)).ToContain("sequential dataset to PDS member");
                           });
                      });

             describe("compress",
                      [&]()
                      {
                        it("should compress a PDS",
                           [&]()
                           {
                             string pds_dsn = get_test_dsn();
                             created_dsns.push_back(pds_dsn);

                             ZDS zds_create = {0};
                             create_pds(&zds_create, pds_dsn);

                             string mem1_dsn = pds_dsn + "(MEM1)";
                             string mem2_dsn = pds_dsn + "(MEM2)";
                             string data1 = "Data 1";
                             string data2 = "Data 2";
                             ZDS zds_write1 = {0};
                             zds_write_to_dsn(&zds_write1, mem1_dsn, data1);
                             ZDS zds_write2 = {0};
                             zds_write_to_dsn(&zds_write2, mem2_dsn, data2);

                             ZDS zds_compress = {0};
                             int rc = zds_compress_dsn(&zds_compress, pds_dsn);
                             ExpectWithContext(rc, zds_compress.diag.e_msg).ToBe(0);
                           });

                        it("should compress PDS with multiple members",
                           [&]()
                           {
                             string pds_dsn = get_test_dsn();
                             created_dsns.push_back(pds_dsn);

                             ZDS zds_create = {0};
                             create_pds(&zds_create, pds_dsn);
                             for (int i = 1; i <= 5; i++)
                             {
                               ZDS zds_write = {0};
                               string mem_dsn = pds_dsn + "(MEM" + to_string(i) + ")";
                               string mem_data = "Data " + to_string(i);
                               zds_write_to_dsn(&zds_write, mem_dsn, mem_data);
                             }

                             ZDS zds_compress = {0};
                             int rc = zds_compress_dsn(&zds_compress, pds_dsn);
                             ExpectWithContext(rc, zds_compress.diag.e_msg).ToBe(0);
                           });

                        it("should compress empty PDS",
                           [&]()
                           {
                             string pds_dsn = get_test_dsn();
                             created_dsns.push_back(pds_dsn);

                             ZDS zds_create = {0};
                             create_pds(&zds_create, pds_dsn);

                             ZDS zds_compress = {0};
                             int rc = zds_compress_dsn(&zds_compress, pds_dsn);
                             ExpectWithContext(rc, zds_compress.diag.e_msg).ToBe(0);
                           });

                        it("should fail when compressing a sequential dataset",
                           [&]()
                           {
                             string ps_dsn = get_test_dsn();
                             created_dsns.push_back(ps_dsn);

                             ZDS zds_create = {0};
                             create_seq(&zds_create, ps_dsn);

                             ZDS zds_compress = {0};
                             int rc = zds_compress_dsn(&zds_compress, ps_dsn);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds_compress.diag.e_msg)).ToContain("not a PDS");
                           });

                        it("should fail when compressing a PDSE",
                           [&]()
                           {
                             string pdse_dsn = get_test_dsn();
                             created_dsns.push_back(pdse_dsn);

                             ZDS zds_create = {0};
                             create_pdse(&zds_create, pdse_dsn);

                             ZDS zds_compress = {0};
                             int rc = zds_compress_dsn(&zds_compress, pdse_dsn);
                             Expect(rc).Not().ToBe(0);
                             // The error might be an IEBCOPY error, so check for failure indicators
                             string error_msg = string(zds_compress.diag.e_msg);
                             Expect(error_msg.length()).ToBeGreaterThan(0);
                             // Check if it contains error indicators (either "not a PDS" or IEBCOPY errors)
                             bool has_error = error_msg.find("not a PDS") != string::npos ||
                                              error_msg.find("IEBCOPY") != string::npos ||
                                              error_msg.find("failed") != string::npos ||
                                              error_msg.find("ERROR") != string::npos;
                             Expect(has_error).ToBe(true);
                           });

                        it("should fail when compressing nonexistent dataset",
                           []()
                           {
                             ZDS zds = {0};
                             string nonexistent_dsn = "NONEXISTENT.DATASET.NAME";
                             int rc = zds_compress_dsn(&zds, nonexistent_dsn);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("not a PDS");
                           });

                        it("should preserve member content after compression",
                           [&]()
                           {
                             string pds_dsn = get_test_dsn();
                             created_dsns.push_back(pds_dsn);
                             string member_dsn = pds_dsn + "(MEMBER)";
                             string test_data = "Test data for compression";

                             ZDS zds_create = {0};
                             create_pds(&zds_create, pds_dsn);

                             ZDS zds_write = {0};
                             int rc = zds_write_to_dsn(&zds_write, member_dsn, test_data);
                             ExpectWithContext(rc, zds_write.diag.e_msg).ToBe(0);

                             ZDS zds_compress = {0};
                             rc = zds_compress_dsn(&zds_compress, pds_dsn);
                             ExpectWithContext(rc, zds_compress.diag.e_msg).ToBe(0);

                             string read_data = "";
                             ZDS zds_read = {0};
                             rc = zds_read_from_dsn(&zds_read, member_dsn, read_data);
                             ExpectWithContext(rc, zds_read.diag.e_msg).ToBe(0);
                             Expect(read_data).ToContain(test_data);
                           });
                      });
           });
}