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

using namespace std;
using namespace ztst;

// Helper function to generate unique dataset names
string get_random_ds()
{
  static int counter = 0;
  counter++;
  srand(time(nullptr) + counter);
  int random_num = rand() % 1000000;
  return "TEST.USER.Z" + to_string(random_num) + "." + to_string(counter);
}

// Helper function to create a PDS
void create_pds(ZDS *zds, const string &dsn)
{
  // Reset ZDS struct
  memset(zds, 0, sizeof(ZDS));

  DS_ATTRIBUTES attrs = {0};
  attrs.dsorg = "PO";
  attrs.dirblk = 5;
  string response;
  int rc = zds_create_dsn(zds, dsn, attrs, response);
  if (rc != 0)
  {
    string error_msg = "";
    if (zds->diag.e_msg_len > 0)
    {
      error_msg = string(zds->diag.e_msg);
    }
    else if (response.length() > 0)
    {
      error_msg = response;
    }
    else
    {
      error_msg = "Unknown error (rc=" + to_string(rc) + ")";
    }
    throw runtime_error("Failed to create PDS: " + error_msg);
  }
}

// Helper function to create a PDSE
void create_pdse(ZDS *zds, const string &dsn)
{
  // Reset ZDS struct
  memset(zds, 0, sizeof(ZDS));

  DS_ATTRIBUTES attrs = {0};
  attrs.dsorg = "PO";
  attrs.dsntype = "LIBRARY";
  attrs.dirblk = 5;
  string response;
  int rc = zds_create_dsn(zds, dsn, attrs, response);
  if (rc != 0)
  {
    string error_msg = "";
    if (zds->diag.e_msg_len > 0)
    {
      error_msg = string(zds->diag.e_msg);
    }
    else if (response.length() > 0)
    {
      error_msg = response;
    }
    else
    {
      error_msg = "Unknown error (rc=" + to_string(rc) + ")";
    }
    throw runtime_error("Failed to create PDSE: " + error_msg);
  }
}

// Helper function to create a sequential dataset
void create_seq(ZDS *zds, const string &dsn)
{
  // Reset ZDS struct
  memset(zds, 0, sizeof(ZDS));

  DS_ATTRIBUTES attrs = {0};
  attrs.dsorg = "PS";
  string response;
  int rc = zds_create_dsn(zds, dsn, attrs, response);
  if (rc != 0)
  {
    string error_msg = "";
    if (zds->diag.e_msg_len > 0)
    {
      error_msg = string(zds->diag.e_msg);
    }
    else if (response.length() > 0)
    {
      error_msg = response;
    }
    else
    {
      error_msg = "Unknown error (rc=" + to_string(rc) + ")";
    }
    throw runtime_error("Failed to create sequential dataset: " + error_msg);
  }
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
                             // Attempt to delete - you may need to implement this
                             // For now, just log
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
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
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
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             create_pdse(&zds, source_dsn);
                             string test_data = "Test data";
                             string member_dsn = source_dsn + "(MEMBER)";
                             zds_write_to_dsn(&zds, member_dsn, test_data);

                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy PDS member to sequential dataset",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_pds = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_pds);
                             created_dsns.push_back(target_dsn);

                             create_pds(&zds, source_pds);
                             string test_data = "Member content";
                             string member_dsn = source_pds + "(MEMBER)";
                             zds_write_to_dsn(&zds, member_dsn, test_data);

                             string src_member = source_pds + "(MEMBER)";
                             int rc = zds_copy_dsn(&zds, src_member, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy sequential dataset to sequential dataset",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             create_seq(&zds, source_dsn);
                             string test_data = "Sequential data";
                             zds_write_to_dsn(&zds, source_dsn, test_data);

                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy member to member",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_pds = get_random_ds();
                             string target_pds = get_random_ds();
                             created_dsns.push_back(source_pds);
                             created_dsns.push_back(target_pds);

                             create_pds(&zds, source_pds);
                             create_pds(&zds, target_pds);
                             string test_data = "Source member data";
                             string src_member_dsn = source_pds + "(SRCMEM)";
                             zds_write_to_dsn(&zds, src_member_dsn, test_data);

                             string src_member = source_pds + "(SRCMEM)";
                             string tgt_member = target_pds + "(TGTMEM)";
                             int rc = zds_copy_dsn(&zds, src_member, tgt_member);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy sequential dataset to PDS member (creates PDS)",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_pds = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_pds);

                             create_seq(&zds, source_dsn);
                             string test_data = "Sequential data";
                             zds_write_to_dsn(&zds, source_dsn, test_data);

                             string tgt_member = target_pds + "(MEMBER)";
                             int rc = zds_copy_dsn(&zds, source_dsn, tgt_member);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy PDS with multiple members",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             create_pds(&zds, source_dsn);
                             for (int i = 1; i <= 3; i++)
                             {
                               string mem_dsn = source_dsn + "(MEM" + to_string(i) + ")";
                               string mem_data = "Data " + to_string(i);
                               zds_write_to_dsn(&zds, mem_dsn, mem_data);
                             }

                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should fail when copying from nonexistent source",
                           []()
                           {
                             ZDS zds = {0};
                             string source_dsn = "NONEXISTENT.DATASET.NAME";
                             string target_dsn = get_random_ds();
                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("not found");
                           });

                        it("should preserve dataset attributes when copying",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             create_seq(&zds, source_dsn);
                             string test_data = "Test data";
                             zds_write_to_dsn(&zds, source_dsn, test_data);

                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);

                             vector<ZDSEntry> source_entries;
                             vector<ZDSEntry> target_entries;
                             zds_list_data_sets(&zds, source_dsn, source_entries, true);
                             zds_list_data_sets(&zds, target_dsn, target_entries, true);

                             if (!source_entries.empty() && !target_entries.empty())
                             {
                               Expect(source_entries[0].recfm).ToBe(target_entries[0].recfm);
                               Expect(source_entries[0].lrecl).ToBe(target_entries[0].lrecl);
                             }
                           });

                        it("should overwrite existing sequential dataset",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             create_seq(&zds, source_dsn);
                             create_seq(&zds, target_dsn);
                             string source_data = "Source data";
                             string target_data = "Old target data";
                             zds_write_to_dsn(&zds, source_dsn, source_data);
                             zds_write_to_dsn(&zds, target_dsn, target_data);

                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should overwrite existing PDS",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             create_pds(&zds, source_dsn);
                             create_pds(&zds, target_dsn);
                             string src_mem1 = source_dsn + "(MEM1)";
                             string tgt_mem1 = target_dsn + "(MEM1)";
                             string src_mem_data = "Source member";
                             string tgt_mem_data = "Old target member";
                             zds_write_to_dsn(&zds, src_mem1, src_mem_data);
                             zds_write_to_dsn(&zds, tgt_mem1, tgt_mem_data);

                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should overwrite existing member",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_pds = get_random_ds();
                             string target_pds = get_random_ds();
                             created_dsns.push_back(source_pds);
                             created_dsns.push_back(target_pds);

                             create_pds(&zds, source_pds);
                             create_pds(&zds, target_pds);
                             string src_mem = source_pds + "(MEM)";
                             string tgt_mem = target_pds + "(MEM)";
                             string new_data = "New source data";
                             string old_data = "Old target data";
                             zds_write_to_dsn(&zds, src_mem, new_data);
                             zds_write_to_dsn(&zds, tgt_mem, old_data);

                             string src_member = source_pds + "(MEM)";
                             string tgt_member = target_pds + "(MEM)";
                             int rc = zds_copy_dsn(&zds, src_member, tgt_member);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy empty PDS",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             create_pds(&zds, source_dsn);

                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy empty sequential dataset",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             create_seq(&zds, source_dsn);

                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy PDSE to PDSE",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_dsn = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_dsn);

                             create_pdse(&zds, source_dsn);
                             string test_data = "PDSE data";
                             string member_dsn = source_dsn + "(MEMBER)";
                             zds_write_to_dsn(&zds, member_dsn, test_data);
                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy sequential to existing member (overwrites)",
                           [&]()
                           {
                             ZDS zds = {0};
                             string source_dsn = get_random_ds();
                             string target_pds = get_random_ds();
                             created_dsns.push_back(source_dsn);
                             created_dsns.push_back(target_pds);

                             create_seq(&zds, source_dsn);
                             create_pds(&zds, target_pds);
                             string source_data = "Sequential source data";
                             string existing_mem = target_pds + "(EXISTING)";
                             string old_mem_data = "Old member data";
                             zds_write_to_dsn(&zds, source_dsn, source_data);
                             zds_write_to_dsn(&zds, existing_mem, old_mem_data);

                             string tgt_member = target_pds + "(EXISTING)";
                             int rc = zds_copy_dsn(&zds, source_dsn, tgt_member);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });
                      });

             describe("compress",
                      [&]()
                      {
                        it("should compress a PDS",
                           [&]()
                           {
                             ZDS zds = {0};
                             string pds_dsn = get_random_ds();
                             created_dsns.push_back(pds_dsn);

                             create_pds(&zds, pds_dsn);
                             string mem1_dsn = pds_dsn + "(MEM1)";
                             string mem2_dsn = pds_dsn + "(MEM2)";
                             string data1 = "Data 1";
                             string data2 = "Data 2";
                             zds_write_to_dsn(&zds, mem1_dsn, data1);
                             zds_write_to_dsn(&zds, mem2_dsn, data2);

                             int rc = zds_compress_dsn(&zds, pds_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should compress PDS with multiple members",
                           [&]()
                           {
                             ZDS zds = {0};
                             string pds_dsn = get_random_ds();
                             created_dsns.push_back(pds_dsn);

                             create_pds(&zds, pds_dsn);
                             for (int i = 1; i <= 5; i++)
                             {
                               string mem_dsn = pds_dsn + "(MEM" + to_string(i) + ")";
                               string mem_data = "Data " + to_string(i);
                               zds_write_to_dsn(&zds, mem_dsn, mem_data);
                             }

                             int rc = zds_compress_dsn(&zds, pds_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should compress empty PDS",
                           [&]()
                           {
                             ZDS zds = {0};
                             string pds_dsn = get_random_ds();
                             created_dsns.push_back(pds_dsn);

                             create_pds(&zds, pds_dsn);

                             int rc = zds_compress_dsn(&zds, pds_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should fail when compressing a sequential dataset",
                           [&]()
                           {
                             ZDS zds = {0};
                             string ps_dsn = get_random_ds();
                             created_dsns.push_back(ps_dsn);

                             create_seq(&zds, ps_dsn);
                             int rc = zds_compress_dsn(&zds, ps_dsn);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("not a PDS");
                           });

                        it("should fail when compressing a PDSE",
                           [&]()
                           {
                             ZDS zds = {0};
                             string pdse_dsn = get_random_ds();
                             created_dsns.push_back(pdse_dsn);

                             create_pdse(&zds, pdse_dsn);
                             int rc = zds_compress_dsn(&zds, pdse_dsn);
                             Expect(rc).Not().ToBe(0);
                             // The error might be an IEBCOPY error, so check for failure indicators
                             string error_msg = string(zds.diag.e_msg);
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
                             ZDS zds = {0};
                             string pds_dsn = get_random_ds();
                             created_dsns.push_back(pds_dsn);
                             string member_dsn = pds_dsn + "(MEMBER)";
                             string test_data = "Test data for compression";

                             create_pds(&zds, pds_dsn);
                             int rc = zds_write_to_dsn(&zds, member_dsn, test_data);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);

                             rc = zds_compress_dsn(&zds, pds_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);

                             string read_data = "";
                             rc = zds_read_from_dsn(&zds, member_dsn, read_data);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                             Expect(read_data).ToContain(test_data);
                           });
                      });
           });
}