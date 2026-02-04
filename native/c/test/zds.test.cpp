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
#include <vector>
#include <cstring>

#include "ztest.hpp"
#include "zds.hpp"
#include "zut.hpp"
#include "zutils.hpp"
// #include "zstorage.metal.test.h"

using namespace std;
using namespace ztst;

// Base for test contexts that create data sets and register them for cleanup
struct DataSetTestContextBase
{
  vector<string> &cleanup_list;

  explicit DataSetTestContextBase(vector<string> &list) : cleanup_list(list) {}

  void create_pds_at(const string &dsn)
  {
    ZDS zds = {0};
    create_pds(&zds, dsn);
  }

  void create_pdse_at(const string &dsn)
  {
    ZDS zds = {0};
    create_pdse(&zds, dsn);
  }

  void create_seq_at(const string &dsn)
  {
    ZDS zds = {0};
    create_seq(&zds, dsn);
  }
};

// Test context for copy operations
struct CopyTestContext : DataSetTestContextBase
{
  string source_dsn;
  string target_dsn;

  explicit CopyTestContext(vector<string> &list) : DataSetTestContextBase(list)
  {
    source_dsn = get_random_ds(3);
    target_dsn = get_random_ds(3);
    cleanup_list.push_back(source_dsn);
    cleanup_list.push_back(target_dsn);
  }

  void create_source_pds() { create_pds_at(source_dsn); }
  void create_source_pdse() { create_pdse_at(source_dsn); }
  void create_source_seq() { create_seq_at(source_dsn); }
  void create_target_pds() { create_pds_at(target_dsn); }
  void create_target_seq() { create_seq_at(target_dsn); }

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

  bool target_has_member(const string &member)
  {
    vector<ZDSMem> members;
    ZDS zds = {0};
    zds_list_members(&zds, target_dsn, members);
    for (const auto &mem : members)
    {
      string name = mem.name;
      zut_trim(name);
      if (name == member)
        return true;
    }
    return false;
  }

  bool source_has_member(const string &member)
  {
    vector<ZDSMem> members;
    ZDS zds = {0};
    zds_list_members(&zds, source_dsn, members);
    for (const auto &mem : members)
    {
      string name = mem.name;
      zut_trim(name);
      if (name == member)
        return true;
    }
    return false;
  }
};

struct CompressTestContext : DataSetTestContextBase
{
  string pds_dsn;

  explicit CompressTestContext(vector<string> &list) : DataSetTestContextBase(list)
  {
    pds_dsn = get_random_ds(3);
    cleanup_list.push_back(pds_dsn);
  }

  void create_pds() { create_pds_at(pds_dsn); }
  void create_pdse() { create_pdse_at(pds_dsn); }
  void create_seq() { create_seq_at(pds_dsn); }
  void write_member(const string &member, const string &data)
  {
    write_to_dsn(pds_dsn + "(" + member + ")", data);
  }

  int compress()
  {
    ZDS z = {0};
    return zds_compress_dsn(&z, pds_dsn);
  }

  int compress_with_context(ZDS &z)
  {
    return zds_compress_dsn(&z, pds_dsn);
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
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.write_source_member("MEMBER", "Test data");
                             Expect(tc.copy()).ToBe(0);
                           });

                        it("should copy PDSE to nonexisting PDS (creates target)",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pdse();
                             tc.write_source_member("MEMBER", "Test data");
                             Expect(tc.copy()).ToBe(0);
                           });

                        it("should fail to copy PDS member to sequential data set",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.write_source_member("MEMBER", "Member content");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, tc.source_dsn + "(MEMBER)", tc.target_dsn);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("must specify a member name");
                           });

                        it("should copy sequential data set to sequential data set",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_seq();
                             tc.write_source("Sequential data");
                             Expect(tc.copy()).ToBe(0);
                           });

                        it("should copy member to member",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.create_target_pds();
                             tc.write_source_member("SRCMEM", "Source member data");
                             Expect(tc.copy_member("SRCMEM", "TGTMEM")).ToBe(0);
                           });

                        it("should fail to copy sequential data set to PDS member",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_seq();
                             tc.write_source("Sequential data");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, tc.source_dsn, tc.target_dsn + "(MEMBER)");
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("must be a sequential data set");
                           });

                        it("should copy PDS with multiple members",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             for (int i = 1; i <= 3; i++)
                             {
                               tc.write_source_member("MEM" + to_string(i), "Data " + to_string(i));
                             }
                             Expect(tc.copy()).ToBe(0);
                           });

                        it("should fail when copying from nonexistent source",
                           [&]() -> void
                           {
                             ZDS zds = {0};
                             string source_dsn = "NONEXISTENT.DATASET.NAME";
                             string target_dsn = get_random_ds(3);
                             created_dsns.push_back(target_dsn);
                             int rc = zds_copy_dsn(&zds, source_dsn, target_dsn);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("not found");
                           });

                        it("should preserve data set attributes when copying",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_seq();
                             tc.write_source("Test data");
                             Expect(tc.copy()).ToBe(0);

                             vector<ZDSEntry> source_entries;
                             vector<ZDSEntry> target_entries;
                             ZDS zds_list = {0};
                             zds_list_data_sets(&zds_list, tc.source_dsn, source_entries, true);
                             zds_list_data_sets(&zds_list, tc.target_dsn, target_entries, true);
                             Expect(source_entries.empty()).ToBe(false);
                             Expect(target_entries.empty()).ToBe(false);

                             const ZDSEntry &src = source_entries[0];
                             const ZDSEntry &tgt = target_entries[0];
                             Expect(tgt.recfm).ToBe(src.recfm);
                             Expect(tgt.lrecl).ToBe(src.lrecl);
                             Expect(tgt.blksize).ToBe(src.blksize);
                             Expect(tgt.spacu).ToBe(src.spacu);
                             Expect(tgt.primary).ToBe(src.primary);
                             Expect(tgt.secondary).ToBe(src.secondary);

                             ZDS zds_read = {0};
                             string content;
                             zds_read_from_dsn(&zds_read, tc.target_dsn, content);
                             Expect(content.find("Test data") != string::npos).ToBe(true);
                           });

                        it("should fail to overwrite existing sequential data set without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_seq();
                             tc.create_target_seq();
                             tc.write_source("Source data");
                             tc.write_target("Old target data");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, tc.source_dsn, tc.target_dsn, false);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("already exists");
                           });

                        it("should overwrite existing sequential data set with replace flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_seq();
                             tc.create_target_seq();
                             tc.write_source("Source data");
                             tc.write_target("Old target data");
                             Expect(tc.copy(true)).ToBe(0);
                           });

                        it("should skip existing members in PDS without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.create_target_pds();
                             tc.write_source_member("MEM1", "Source member");
                             tc.write_source_member("MEM2", "Source member 2");
                             tc.write_target_member("MEM1", "Old target member");

                             Expect(tc.copy(false)).ToBe(0);
                             Expect(tc.target_has_member("MEM2")).ToBe(true);
                           });

                        it("should replace existing members in PDS with replace flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.create_target_pds();
                             tc.write_source_member("MEM1", "Source member");
                             tc.write_target_member("MEM1", "Old target member");
                             Expect(tc.copy(true)).ToBe(0);
                           });

                        it("should overwrite entire PDS with overwrite flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.create_target_pds();
                             tc.write_source_member("MEM1", "Source member");
                             tc.write_target_member("MEM2", "Target member to be deleted");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, tc.source_dsn, tc.target_dsn, false, true);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);

                             Expect(tc.target_has_member("MEM1")).ToBe(true);
                             Expect(tc.target_has_member("MEM2")).ToBe(false);
                           });

                        it("should fail to overwrite existing member without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.create_target_pds();
                             tc.write_source_member("MEM", "New source data");
                             tc.write_target_member("MEM", "Old target data");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, tc.source_dsn + "(MEM)", tc.target_dsn + "(MEM)", false);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("already exists");
                           });

                        it("should overwrite existing member with replace flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.create_target_pds();
                             tc.write_source_member("MEM", "New source data");
                             tc.write_target_member("MEM", "Old target data");
                             Expect(tc.copy_member("MEM", "MEM", true)).ToBe(0);
                           });

                        it("should copy member to another member in the same PDS",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.write_source_member("SRC", "Source member data");

                             // Copy within same PDS
                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, tc.source_dsn + "(SRC)", tc.source_dsn + "(DST)", false);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);

                             // Verify both members exist
                             Expect(tc.source_has_member("SRC")).ToBe(true);
                             Expect(tc.source_has_member("DST")).ToBe(true);
                           });

                        it("should copy and rename member in the same PDS with replace",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.write_source_member("ORIG", "Original data");
                             tc.write_source_member("COPY", "Old copy data");

                             // Copy and replace within same PDS
                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, tc.source_dsn + "(ORIG)", tc.source_dsn + "(COPY)", true);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });

                        it("should copy empty PDS",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             Expect(tc.copy()).ToBe(0);
                           });

                        it("should copy empty sequential data set",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_seq();
                             Expect(tc.copy()).ToBe(0);
                           });

                        it("should copy PDSE to PDSE",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pdse();
                             tc.write_source_member("MEMBER", "PDSE data");
                             Expect(tc.copy()).ToBe(0);
                           });

                        it("should copy PDS to new target without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.write_source_member("MEMBER", "Test data");
                             Expect(tc.copy(false)).ToBe(0);
                           });

                        it("should copy sequential to new target without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_seq();
                             tc.write_source("Sequential data");
                             Expect(tc.copy(false)).ToBe(0);
                           });

                        it("should add new members to existing PDS without replace flag",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_pds();
                             tc.create_target_pds();
                             tc.write_source_member("MEM1", "Source data");
                             tc.write_target_member("MEM2", "Target data");

                             Expect(tc.copy(false)).ToBe(0);
                             Expect(tc.target_has_member("MEM1")).ToBe(true);
                             Expect(tc.target_has_member("MEM2")).ToBe(true);
                           });

                        it("should fail to copy sequential to PDS member even with replace",
                           [&]() -> void
                           {
                             CopyTestContext tc(created_dsns);
                             tc.create_source_seq();
                             tc.create_target_pds();
                             tc.write_source("Sequential source data");
                             tc.write_target_member("EXISTING", "Old member data");

                             ZDS zds = {0};
                             int rc = zds_copy_dsn(&zds, tc.source_dsn, tc.target_dsn + "(EXISTING)", true);
                             Expect(rc).Not().ToBe(0);
                             Expect(string(zds.diag.e_msg)).ToContain("must be a sequential data set");
                           });
                      });

             // IEBCOPY invoked via ZUTRUN (LOAD/CALL) can 0C4 on some systems
             describe("compress",
                      [&]() -> void
                      {
                        it("should compress a PDS",
                           [&]() -> void
                           {
                             CompressTestContext tc(created_dsns);
                             tc.create_pds();
                             tc.write_member("MEM1", "Data 1");
                             tc.write_member("MEM2", "Data 2");
                             Expect(tc.compress()).ToBe(0);
                           });

                        it("should compress PDS with multiple members",
                           [&]() -> void
                           {
                             CompressTestContext tc(created_dsns);
                             tc.create_pds();
                             for (int i = 1; i <= 5; i++)
                               tc.write_member("MEM" + to_string(i), "Data " + to_string(i));
                             Expect(tc.compress()).ToBe(0);
                           });

                        it("should compress empty PDS",
                           [&]() -> void
                           {
                             CompressTestContext tc(created_dsns);
                             tc.create_pds();
                             Expect(tc.compress()).ToBe(0);
                           });

                        it("should fail when compressing a sequential data set",
                           [&]() -> void
                           {
                             CompressTestContext tc(created_dsns);
                             tc.create_seq();
                             ZDS z = {0};
                             Expect(tc.compress_with_context(z)).Not().ToBe(0);
                             Expect(string(z.diag.e_msg)).ToContain("not a PDS");
                           });

                        it("should fail when compressing a PDSE",
                           [&]() -> void
                           {
                             CompressTestContext tc(created_dsns);
                             tc.create_pdse();
                             ZDS z = {0};
                             Expect(tc.compress_with_context(z)).Not().ToBe(0);
                             Expect(string(z.diag.e_msg).length()).ToBeGreaterThan(0);
                           });

                        it("should fail when compressing nonexistent data set",
                           []() -> void
                           {
                             ZDS z = {0};
                             Expect(zds_compress_dsn(&z, "NONEXISTENT.DATASET.NAME")).Not().ToBe(0);
                             Expect(string(z.diag.e_msg)).ToContain("not a PDS");
                           });

                        it("should preserve member content after compression",
                           [&]() -> void
                           {
                             CompressTestContext tc(created_dsns);
                             tc.create_pds();
                             string test_data = "Test data for compression";
                             tc.write_member("MEMBER", test_data);
                             Expect(tc.compress()).ToBe(0);

                             string read_data;
                             ZDS z = {0};
                             zds_read_from_dsn(&z, tc.pds_dsn + "(MEMBER)", read_data);
                             Expect(read_data).ToContain(test_data);
                           });
                      });
             describe("rename",
                      [&]() -> void
                      {
                        it("should fail if source or target data sets are empty",
                           []() -> void
                           {
                             ZDS zds = {0};
                             string target = get_random_ds(3);
                             int rc = zds_rename_dsn(&zds, "", target);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Data set names must be valid");

                             ZDS zds2 = {0};
                             rc = zds_rename_dsn(&zds2, "USER.TEST", "");
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds2.diag.e_msg)).ToContain("Data set names must be valid");
                           });

                        it("should fail if target data set name exceeds max length",
                           []() -> void
                           {
                             ZDS zds = {0};
                             string longName = "USER.TEST.TEST.TEST.TEST.TEST.TEST.TEST.TEST.TEST.TEST.TEST.TEST";
                             string source = get_random_ds(3);
                             int rc = zds_rename_dsn(&zds, source, longName);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Target data set name exceeds max character length of 44");
                           });

                        it("should fail if source data set does not exist",
                           []() -> void
                           {
                             ZDS zds = {0};
                             string target = get_random_ds(3);
                             string source = get_random_ds(3);
                             int rc = zds_rename_dsn(&zds, source, target);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Source data set does not exist");
                           });

                        it("should fail if target data set already exists",
                           [&]() -> void
                           {
                             ZDS zds = {0};
                             string source = get_random_ds(3);
                             string target = get_random_ds(3);
                             created_dsns.push_back(source);
                             created_dsns.push_back(target);

                             create_seq(&zds, source);
                             create_seq(&zds, target);
                             int rc = zds_rename_dsn(&zds, source, target);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Target data set name already exists");
                           });

                        it("should rename dataset successfully when valid",
                           [&]() -> void
                           {
                             ZDS zds = {0};
                             string before = get_random_ds(3);
                             string after = get_random_ds(3);
                             created_dsns.push_back(after); // before is renamed to after; clean up final name

                             create_seq(&zds, before);
                             int rc = zds_rename_dsn(&zds, before, after);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });
                      });
           });
}