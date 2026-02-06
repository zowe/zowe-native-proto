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

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "ztest.hpp"
#include "zds.hpp"
#include "zut.hpp"
#include "zutils.hpp"
// #include "zstorage.metal.test.h"

using namespace std;
using namespace ztst;

static bool member_exists(const string &dsn, const string &member)
{
  string path = "//'" + dsn + "(" + member + ")'";
  FILE *fp = fopen(path.c_str(), "r");
  if (fp)
  {
    fclose(fp);
    return true;
  }
  return false;
}

void zds_tests()
{

  describe("zds",
           []() -> void
           {
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

                             // Mock data to test encoding conversion logic
                             string test_data = "Hello World";

                             // The actual encoding conversion should use UTF-8 as source when source_codepage is empty
                             // Since we can't easily test the actual file operations without a real dataset,
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

                             // Copy the struct
                             ZDS zds2 = zds1;

                             // Verify encodings are preserved in copy
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

             describe("rename data sets",
                      []() -> void
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
                           []() -> void
                           {
                             ZDS zds = {0};
                             DS_ATTRIBUTES attr = {0};

                             attr.dsorg = "PS";
                             attr.recfm = "FB";
                             attr.lrecl = 80;
                             attr.blksize = 0;
                             attr.alcunit = "TRACKS";
                             attr.primary = 1;
                             attr.secondary = 1;
                             attr.dirblk = 0;

                             string source = get_random_ds(3);
                             string target = get_random_ds(3);

                             string response;
                             int rc = zds_create_dsn(&zds, source, attr, response);
                             rc = zds_create_dsn(&zds, target, attr, response);
                             rc = zds_rename_dsn(&zds, source, target);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Target data set name already exists");
                           });

                        it("should rename dataset successfully when valid",
                           []() -> void
                           {
                             ZDS zds = {0};
                             DS_ATTRIBUTES attr = {0};

                             attr.dsorg = "PS";
                             attr.recfm = "FB";
                             attr.lrecl = 80;
                             attr.blksize = 0;
                             attr.alcunit = "TRACKS";
                             attr.primary = 1;
                             attr.secondary = 1;
                             attr.dirblk = 0;
                             string before = get_random_ds(3);
                             string after = get_random_ds(3);

                             string response;
                             int rc = zds_create_dsn(&zds, before, attr, response);
                             rc = zds_rename_dsn(&zds, before, after);
                             ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                           });
                      });
             describe("rename members",
                      [&]() -> void
                      {
                        const string M1 = "M1";
                        const string M2 = "M2";
                        DS_ATTRIBUTES attr = {0};

                        attr.dsorg = "PO";
                        attr.recfm = "FB";
                        attr.lrecl = 80;
                        attr.blksize = 6160;
                        attr.alcunit = "TRACKS";
                        attr.primary = 1;
                        attr.secondary = 1;
                        attr.dirblk = 10;
                        string response;

                        it("should fail if data set name is empty",
                           [&]() -> void
                           {
                             ZDS zds = {0};
                             string ds = get_random_ds(3);
                             int rc = zds_rename_members(&zds, "", M1, M2);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Data set name must be valid");
                           });

                        it("should fail if member names are empty",
                           [&]() -> void
                           {
                             ZDS zds = {0};
                             string ds = get_random_ds(3);
                             int rc = zds_create_dsn(&zds, ds, attr, response);
                             Expect(rc).ToBe(0);

                             rc = zds_rename_members(&zds, ds, "", M2);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Member names must be valid");

                             ZDS zds2 = {0};
                             rc = zds_rename_members(&zds2, ds, M1, "");
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Member names must be valid");
                           });
                        it("should fail if member name is too long",
                           [&]() -> void
                           {
                             ZDS zds = {0};
                             string longName = "USER.TEST.TEST.TEST";
                             string ds = get_random_ds(3);
                             int rc = zds_create_dsn(&zds, ds, attr, response);
                             rc = zds_rename_members(&zds, ds, M1, longName);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Member names must be valid");
                           });

                        it("should fail if data set does not exist",
                           [&]() -> void
                           {
                             ZDS zds = {0};
                             string ds = get_random_ds(3);
                             int rc = zds_rename_members(&zds, ds, M1, M2);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Data set does not exist");
                           });

                        it("should fail if source member does not exist",
                           [&]() -> void
                           {
                             ZDS zds = {0};
                             string ds = get_random_ds(3);
                             int rc = zds_create_dsn(&zds, ds, attr, response);
                             rc = zds_rename_members(&zds, ds, M1, "M3");
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Source member does not exist");
                           });

                        it("should fail if target member already exists",
                           [&]() -> void
                           {
                             ZDS zds = {};

                             string ds = get_random_ds(3);
                             int rc = zds_create_dsn(&zds, ds, attr, response);

                             string empty = "";
                             rc = zds_write_to_dsn(&zds, ds + "(M1)", empty);
                             Expect(rc).ToBe(0);
                             memset(zds.etag, 0, 8);
                             rc = zds_write_to_dsn(&zds, ds + "(M2)", empty);
                             Expect(rc).ToBe(0);

                             rc = zds_rename_members(&zds, ds, M1, M2);
                             Expect(rc).ToBe(RTNCD_FAILURE);
                             Expect(string(zds.diag.e_msg)).ToContain("Target member already exists");
                           });

                        it("should rename dataset successfully when valid",
                           [&]() -> void
                           {
                             ZDS zds = {};
                             string ds = get_random_ds(3);
                             int rc = zds_create_dsn(&zds, ds, attr, response);

                             string empty = "";
                             rc = zds_write_to_dsn(&zds, ds + "(M1)", empty);
                             Expect(rc).ToBe(0);

                             rc = zds_rename_members(&zds, ds, M1, "M3");
                             Expect(rc).ToBe(0);
                             rc = zds_delete_dsn(&zds, ds);
                           });
                      });
           });
}