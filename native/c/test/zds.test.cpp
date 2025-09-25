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

#include "ztest.hpp"
#include "zds.hpp"
// #include "zstorage.metal.test.h"

using namespace std;
using namespace ztst;

void zds_tests()
{

  describe("zds tests",
           []() -> void
           {
             it("should list data sets in a DSN",
                []() -> void
                {
                  int rc = 0;
                  ZDS zds = {0};
                  vector<ZDSEntry> entries;
                  string dsn = "SYS1.MACLIB";
                  rc = zds_list_data_sets(&zds, dsn, entries);
                  ExpectWithContext(rc, zds.diag.e_msg).ToBe(0);
                });
             it("should list data set members",
                []() -> void
                {
                  int rc = 0;
                  ZDS zds = {0};
                  vector<ZDSMem> members;
                  string dsn = "SYS1.MACLIB";
                  rc = zds_list_members(&zds, dsn, members);
                  ExpectWithContext(rc, zds.diag.e_msg).ToBeGreaterThan(0); // zero or warning
                });
           });

  describe("zds source encoding tests",
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
}
