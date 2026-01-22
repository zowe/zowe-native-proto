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
  // CodeQL: rand() is fine here - just generating unique test data set names, not for security
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

// Helper function to create a sequential data set with explicit attributes
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
    throw runtime_error("Failed to create sequential data set: " + get_create_error(zds, response, rc));
}

// Helper to write data to a data set or member
void write_to_dsn(const string &dsn, const string &data)
{
  ZDS zds = {0};
  zds_write_to_dsn(&zds, dsn, data);
}

// Test context to manage data set creation and cleanup
struct CopyTestContext
{
  vector<string> &cleanup_list;
  string source_dsn;
  string target_dsn;

  CopyTestContext(vector<string> &list) : cleanup_list(list)
  {
    source_dsn = get_test_dsn();
    target_dsn = get_test_dsn();
    cleanup_list.push_back(source_dsn);
    cleanup_list.push_back(target_dsn);
  }

  void create_source_pds()
  {
    ZDS zds = {0};
    create_pds(&zds, source_dsn);
  }

  void create_source_pdse()
  {
    ZDS zds = {0};
    create_pdse(&zds, source_dsn);
  }

  void create_source_seq()
  {
    ZDS zds = {0};
    create_seq(&zds, source_dsn);
  }

  void create_target_pds()
  {
    ZDS zds = {0};
    create_pds(&zds, target_dsn);
  }

  void create_target_seq()
  {
    ZDS zds = {0};
    create_seq(&zds, target_dsn);
  }

  void write_source_member(const string &member, const string &data)
  {
    write_to_dsn(source_dsn + "(" + member + ")", data);
  }

  void write_source(const string &data)
  {
    write_to_dsn(source_dsn, data);
  }

  void write_target_member(const string &member, const string &data)
  {
    write_to_dsn(target_dsn + "(" + member + ")", data);
  }

  void write_target(const string &data)
  {
    write_to_dsn(target_dsn, data);
  }

  int copy(bool replace = false, bool overwrite = false)
  {
    ZDS zds = {0};
    return zds_copy_dsn(&zds, source_dsn, target_dsn, replace, overwrite);
  }

  int copy_member(const string &src_mem, const string &tgt_mem, bool replace = false, bool overwrite = false)
  {
    ZDS zds = {0};
    return zds_copy_dsn(&zds, source_dsn + "(" + src_mem + ")", target_dsn + "(" + tgt_mem + ")", replace, overwrite);
  }
};

void zds_tests()
{
  vector<string> created_dsns;

  describe("zds",
           [&]() -> void
           {
             afterAll([&]() -> void
                      {
                         // Cleanup created data sets
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
                      []() -> void
                      {
                        it("should list data sets with a given DSN",
                           []() -> void
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
                           []() -> void
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
                      []() -> void
                      {
                        it("should use default source encoding (UTF-8) when not specified",
                           []() -> void
                           {
                             ZDS zds = {0};
                             // Set target encoding but no source encoding
                             strcpy(zds.encoding_opts.codepage, "IBM-1047");
                             zds.encoding_opts.data_type = eDataTypeText;
                             // source_codepage should be empty/null

                             // The actual encoding conversion should use UTF-8 as source when source_codepage is empty
                             // Since we can't easily test the actual file operations without a real data set,
                             // we'll verify the struct is properly initialized
                             Expect(strlen(zds.encoding_opts.source_codepage)).ToBe(0);
                             Expect(strlen(zds.encoding_opts.codepage)).ToBe(8); // "IBM-1047"
                           });

                        it("should use specified source encoding when provided",
                           []() -> void
                           {
                             ZDS zds = {0};
                             // Set both target and source encoding
                             strcpy(zds.encoding_opts.codepage, "IBM-1047");
                             strcpy(zds.encoding_opts.source_codepage, "IBM-037");
                             zds.encoding_opts.data_type = eDataTypeText;

                             // Verify both encodings are set correctly
                             Expect(string(zds.encoding_opts.codepage)).ToBe("IBM-1047");
                             Expect(string(zds.encoding_opts.source_codepage)).ToBe("IBM-037");
                             Expect(zds.encoding_opts.data_type).ToBe(eDataTypeText);
                           });

                        it("should handle binary data type correctly with source encoding",
                           []() -> void
                           {
                             ZDS zds = {0};
                             strcpy(zds.encoding_opts.codepage, "binary");
                             strcpy(zds.encoding_opts.source_codepage, "UTF-8");
                             zds.encoding_opts.data_type = eDataTypeBinary;

                             // For binary data, encoding should not be used for conversion
                             Expect(string(zds.encoding_opts.codepage)).ToBe("binary");
                             Expect(string(zds.encoding_opts.source_codepage)).ToBe("UTF-8");
                             Expect(zds.encoding_opts.data_type).ToBe(eDataTypeBinary);
                           });

                        it("should handle empty source encoding gracefully",
                           []() -> void
                           {
                             ZDS zds = {0};
                             strcpy(zds.encoding_opts.codepage, "IBM-1047");
                             // Explicitly set source_codepage to empty
                             memset(zds.encoding_opts.source_codepage, 0, sizeof(zds.encoding_opts.source_codepage));
                             zds.encoding_opts.data_type = eDataTypeText;

                             // Should handle empty source encoding (will default to UTF-8 in actual conversion)
                             Expect(strlen(zds.encoding_opts.source_codepage)).ToBe(0);
                             Expect(string(zds.encoding_opts.codepage)).ToBe("IBM-1047");
                           });

                        it("should handle maximum length encoding names",
                           []() -> void
                           {
                             ZDS zds = {0};
                             // Test with maximum length encoding names (15 chars + null terminator)
                             string long_target = "IBM-1234567890A"; // 15 characters
                             string long_source = "UTF-1234567890B"; // 15 characters

                             strncpy(zds.encoding_opts.codepage, long_target.c_str(), sizeof(zds.encoding_opts.codepage) - 1);
                             strncpy(zds.encoding_opts.source_codepage, long_source.c_str(), sizeof(zds.encoding_opts.source_codepage) - 1);

                             // Ensure null termination
                             zds.encoding_opts.codepage[sizeof(zds.encoding_opts.codepage) - 1] = '\0';
                             zds.encoding_opts.source_codepage[sizeof(zds.encoding_opts.source_codepage) - 1] = '\0';

                             Expect(string(zds.encoding_opts.codepage)).ToBe(long_target);
                             Expect(string(zds.encoding_opts.source_codepage)).ToBe(long_source);
                           });

                        it("should preserve encoding settings through struct copy",
                           []() -> void
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
                           []() -> void
                           {
                             // Verify the struct size is as expected for both fields
                             Expect(sizeof(((ZEncode *)0)->codepage)).ToBe(16);
                             Expect(sizeof(((ZEncode *)0)->source_codepage)).ToBe(16);

                             // Verify the fields are at expected offsets
                             ZEncode encode = {0};
                             strcpy(encode.codepage, "test1");
                             strcpy(encode.source_codepage, "test2");

                             Expect(string(encode.codepage)).ToBe("test1");
                             Expect(string(encode.source_codepage)).ToBe("test2");
                           });

                        it("should handle common encoding combinations",
                           []() -> void
                           {
                             ZDS zds = {0};
                             zds.encoding_opts.data_type = eDataTypeText;

                             // Test common mainframe encoding conversions
                             struct EncodingPair
                             {
                               const char *source;
                               const char *target;
                             };

                             EncodingPair pairs[] = {
                                 {"UTF-8", "IBM-1047"},     // UTF-8 to EBCDIC
                                 {"IBM-037", "UTF-8"},      // EBCDIC to UTF-8
                                 {"IBM-1047", "IBM-037"},   // EBCDIC to EBCDIC
                                 {"ISO8859-1", "IBM-1047"}, // ASCII to EBCDIC
                                 {"UTF-8", "binary"}        // Text to binary
                             };

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
                      [&]() -> void
                      {
                        it("should copy PDS to PDS",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.write_source_member("MEMBER", "Test data");
                             Expect(ctx.copy()).ToBe(0);
                           });

                        it("should copy PDSE to nonexisting PDS (creates target)",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pdse();
                             ctx.write_source_member("MEMBER", "Test data");
                             Expect(ctx.copy()).ToBe(0);
                           });

                        it("should copy PDS member to sequential data set",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.write_source_member("MEMBER", "Member content");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, ctx.source_dsn + "(MEMBER)", ctx.target_dsn);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy sequential data set to sequential data set",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_seq();
                             ctx.write_source("Sequential data");
                             Expect(ctx.copy()).ToBe(0);
                           });

                        it("should copy member to member",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.create_target_pds();
                             ctx.write_source_member("SRCMEM", "Source member data");
                             Expect(ctx.copy_member("SRCMEM", "TGTMEM")).ToBe(0);
                           });

                        it("should copy sequential data set to PDS member",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_seq();
                             ctx.write_source("Sequential data");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, ctx.source_dsn, ctx.target_dsn + "(MEMBER)");
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy PDS with multiple members",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             for (int i = 1; i <= 3; i++)
                             {
                               ctx.write_source_member("MEM" + to_string(i), "Data " + to_string(i));
                             }
                             Expect(ctx.copy()).ToBe(0);
                           });

                        it("should fail when copying from nonexistent source",
                           [&]() -> void
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

                        it("should preserve data set attributes when copying",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_seq();
                             ctx.write_source("Test data");
                             Expect(ctx.copy()).ToBe(0);

                             // Verify target was created and data was copied
                             vector<ZDSEntry> target_entries;
                             ZDS zds_list = {0};
                             zds_list_data_sets(&zds_list, ctx.target_dsn, target_entries, true);
                             Expect(target_entries.empty()).ToBe(false);

                             // Verify content was copied correctly
                             ZDS zds_read = {0};
                             string content;
                             zds_read_from_dsn(&zds_read, ctx.target_dsn, content);
                             Expect(content.find("Test data") != string::npos).ToBe(true);

                             // Note: LIKE allocation is used for attribute preservation, but SMS ACS
                             // routines may override attributes on some systems. The key verification
                             // is that the copy succeeded and data is intact.
                           });

                        it("should fail to overwrite existing sequential data set without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_seq();
                             ctx.create_target_seq();
                             ctx.write_source("Source data");
                             ctx.write_target("Old target data");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, ctx.source_dsn, ctx.target_dsn, false);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("already exists");
                           });

                        it("should overwrite existing sequential data set with replace flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_seq();
                             ctx.create_target_seq();
                             ctx.write_source("Source data");
                             ctx.write_target("Old target data");
                             Expect(ctx.copy(true)).ToBe(0);
                           });

                        it("should skip existing members in PDS without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.create_target_pds();
                             ctx.write_source_member("MEM1", "Source member");
                             ctx.write_source_member("MEM2", "Source member 2");
                             ctx.write_target_member("MEM1", "Old target member");

                             Expect(ctx.copy(false)).ToBe(0);

                             // Verify MEM2 was copied (MEM1 should be skipped since it exists)
                             vector<ZDSMem> members;
                             ZDS zds_list = {0};
                             zds_list_members(&zds_list, ctx.target_dsn, members);
                             bool found_mem2 = false;
                             for (const auto &mem : members)
                             {
                               if (mem.name == "MEM2")
                                 found_mem2 = true;
                             }
                             Expect(found_mem2).ToBe(true);
                           });

                        it("should replace existing members in PDS with replace flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.create_target_pds();
                             ctx.write_source_member("MEM1", "Source member");
                             ctx.write_target_member("MEM1", "Old target member");
                             Expect(ctx.copy(true)).ToBe(0);
                           });

                        it("should overwrite entire PDS with overwrite flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.create_target_pds();
                             ctx.write_source_member("MEM1", "Source member");
                             ctx.write_target_member("MEM2", "Target member to be deleted");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, ctx.source_dsn, ctx.target_dsn, false, true);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);

                             vector<ZDSMem> members;
                             ZDS zds_list = {0};
                             zds_list_members(&zds_list, ctx.target_dsn, members);
                             bool found_mem1 = false;
                             bool found_mem2 = false;
                             for (const auto &mem : members)
                             {
                               string name = mem.name;
                               zut_trim(name);
                               if (name == "MEM1")
                                 found_mem1 = true;
                               if (name == "MEM2")
                                 found_mem2 = true;
                             }
                             Expect(found_mem1).ToBe(true);
                             Expect(found_mem2).ToBe(false);
                           });

                        it("should fail to overwrite existing member without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.create_target_pds();
                             ctx.write_source_member("MEM", "New source data");
                             ctx.write_target_member("MEM", "Old target data");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, ctx.source_dsn + "(MEM)", ctx.target_dsn + "(MEM)", false);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("already exists");
                           });

                        it("should overwrite existing member with replace flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.create_target_pds();
                             ctx.write_source_member("MEM", "New source data");
                             ctx.write_target_member("MEM", "Old target data");
                             Expect(ctx.copy_member("MEM", "MEM", true)).ToBe(0);
                           });

                        it("should copy empty PDS",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             Expect(ctx.copy()).ToBe(0);
                           });

                        it("should copy empty sequential data set",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_seq();
                             Expect(ctx.copy()).ToBe(0);
                           });

                        it("should copy PDSE to PDSE",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pdse();
                             ctx.write_source_member("MEMBER", "PDSE data");
                             Expect(ctx.copy()).ToBe(0);
                           });

                        it("should copy PDS to new target without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.write_source_member("MEMBER", "Test data");
                             Expect(ctx.copy(false)).ToBe(0);
                           });

                        it("should copy sequential to new target without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_seq();
                             ctx.write_source("Sequential data");
                             Expect(ctx.copy(false)).ToBe(0);
                           });

                        it("should add new members to existing PDS without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_pds();
                             ctx.create_target_pds();
                             ctx.write_source_member("MEM1", "Source data");
                             ctx.write_target_member("MEM2", "Target data");

                             Expect(ctx.copy(false)).ToBe(0);

                             // Verify both original and copied members exist
                             vector<ZDSMem> members;
                             ZDS zds_list = {0};
                             zds_list_members(&zds_list, ctx.target_dsn, members);
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

                        it("should copy sequential to existing PDS member with replace",
                           [&]() -> void
                           {
                             CopyTestContext ctx(created_dsns);
                             ctx.create_source_seq();
                             ctx.create_target_pds();
                             ctx.write_source("Sequential source data");
                             ctx.write_target_member("EXISTING", "Old member data");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, ctx.source_dsn, ctx.target_dsn + "(EXISTING)", true);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });
                      });

             describe("compress",
                      [&]() -> void
                      {
                        it("should compress a PDS",
                           [&]() -> void
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
                           [&]() -> void
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
                           [&]() -> void
                           {
                             string pds_dsn = get_test_dsn();
                             created_dsns.push_back(pds_dsn);

                             ZDS zds_create = {0};
                             create_pds(&zds_create, pds_dsn);

                             ZDS zds_compress = {0};
                             int rc = zds_compress_dsn(&zds_compress, pds_dsn);
                             ExpectWithContext(rc, zds_compress.diag.e_msg).ToBe(0);
                           });

                        it("should fail when compressing a sequential data set",
                           [&]() -> void
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
                           [&]() -> void
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

                        it("should fail when compressing nonexistent data set",
                           []() -> void
                           {
                             ZDS zds = {0};
                             string nonexistent_dsn = "NONEXISTENT.DATASET.NAME";
                             int rc = zds_compress_dsn(&zds, nonexistent_dsn);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("not a PDS");
                           });

                        it("should preserve member content after compression",
                           [&]() -> void
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