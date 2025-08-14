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
#include "zut.hpp"
#include "zstorage.metal.test.h"

using namespace std;
using namespace ztst;

void zut_tests()
{

  describe("zut tests",
           []() -> void
           {
             it("should upper case and truncate a long string",
                []() -> void
                {
                  char buffer[9] = {0};
                  string data = "lowercaselongstring";
                  zut_uppercase_pad_truncate(buffer, data, sizeof(buffer) - 1);
                  expect(string(buffer)).ToBe("LOWERCAS");
                });

             it("should upper case and pad a short string",
                []() -> void
                {
                  char buffer[9] = {0};
                  string data = "abc";
                  zut_uppercase_pad_truncate(buffer, data, sizeof(buffer) - 1);
                  expect(string(buffer)).ToBe("ABC     ");
                });
           });

  describe("zut_encode tests (EBCDIC-aware)",
           []() -> void
           {
             it("should return input unchanged when source and target encodings are the same",
                []() -> void
                {
                  ZDIAG diag = {0};
                  // Default string literals are IBM-1047 on z/OS
                  string input = "Hello World";
                  string result = zut_encode(input, "IBM-1047", "IBM-1047", diag);

                  expect(result).ToBe("Hello World");
                  expect(result.length()).ToBe(input.length());
                  expect(diag.e_msg_len).ToBe(0);
                });

             it("should handle ASCII to EBCDIC conversion with known byte sequences",
                []() -> void
                {
                  ZDIAG diag = {0};

                  // Create ASCII "HELLO" using known byte values: 0x48 0x45 0x4C 0x4C 0x4F
                  vector<char> ascii_hello = {0x48, 0x45, 0x4C, 0x4C, 0x4F}; // "HELLO" in ASCII

                  // Convert ASCII to EBCDIC
                  vector<char> ebcdic_result = zut_encode(ascii_hello.data(), ascii_hello.size(), "ISO8859-1", "IBM-1047", diag);

                  expect(diag.e_msg_len).ToBe(0);
                  expect(ebcdic_result.size()).ToBe(5);

                  // Convert back to ASCII to verify round-trip
                  vector<char> ascii_roundtrip = zut_encode(ebcdic_result.data(), ebcdic_result.size(), "IBM-1047", "ISO8859-1", diag);
                  expect(diag.e_msg_len).ToBe(0);
                  expect(ascii_roundtrip.size()).ToBe(5);

                  // Verify original ASCII bytes are preserved
                  for (size_t i = 0; i < ascii_hello.size(); i++)
                  {
                    expect(ascii_roundtrip[i]).ToBe(ascii_hello[i]);
                  }
                });

             it("should handle conversion failure gracefully with invalid encoding",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string input = "Test data";
                  string result = zut_encode(input, "INVALID-ENCODING", "UTF-8", diag);

                  expect(result).ToBe("");
                  expect(diag.e_msg_len).ToBeGreaterThan(0);
                  expect(string(diag.e_msg)).ToContain("Cannot open converter");
                });

             it("should handle round-trip conversion without data loss",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string original = "Round trip test data 12345";

                  // Convert IBM-1047 -> ISO8859-1 -> IBM-1047
                  string step1 = zut_encode(original, "IBM-1047", "ISO8859-1", diag);
                  expect(diag.e_msg_len).ToBe(0);

                  string step2 = zut_encode(step1, "ISO8859-1", "IBM-1047", diag);
                  expect(diag.e_msg_len).ToBe(0);

                  expect(step2).ToBe(original);
                  expect(step2.length()).ToBe(original.length());
                });

             it("should handle empty string conversion",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string input = "";
                  string result = zut_encode(input, "UTF-8", "ISO8859-1", diag);

                  expect(result).ToBe("");
                  expect(result.length()).ToBe(0);
                  expect(diag.e_msg_len).ToBe(0);
                });

             it("should handle single character conversion",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string input = "A";
                  string result = zut_encode(input, "IBM-1047", "ISO8859-1", diag);
                  char iso8859_1_a[1] = {0x41};

                  expect(result).ToBe(iso8859_1_a);
                  expect(result.length()).ToBe(1);
                  expect(diag.e_msg_len).ToBe(0);
                });

             it("should handle numeric content conversion",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string input = "1234567890";
                  string result = zut_encode(input, "IBM-1047", "ISO8859-1", diag);
                  char iso8859_1_1234567890[10] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30};

                  expect(result).ToBe(iso8859_1_1234567890);
                  expect(result.length()).ToBe(10);
                  expect(diag.e_msg_len).ToBe(0);
                });

             it("should handle line breaks and whitespace",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string input = "Line 1\nLine 2\r\nLine 3\tTabbed";
                  string result = zut_encode(input, "IBM-1047", "ISO8859-1", diag);
                  char iso8859_1_line_breaks[29] = {0x4C, 0x69, 0x6E, 0x65, 0x20, 0x31, 0x0A, 0x4C, 0x69, 0x6E, 0x65, 0x20, 0x32, 0x0D, 0x0A, 0x4C, 0x69, 0x6E, 0x65, 0x20, 0x33, 0x09, 0x54, 0x61, 0x62, 0x62, 0x65, 0x64};

                  expect(result).ToBe(iso8859_1_line_breaks);
                  expect(result.length()).ToBe(input.length());
                  expect(diag.e_msg_len).ToBe(0);
                });

             it("should preserve data integrity for longer text",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string input = "This is a longer piece of text that contains multiple words, "
                                 "punctuation marks, numbers like 123 and 456, and various symbols "
                                 "such as @#$%^&*()_+ to test that the encoding conversion maintains "
                                 "data integrity across larger content blocks.";
                  string result = zut_encode(input, "IBM-1047", "ISO8859-1", diag);
                  string result_original = zut_encode(result, "ISO8859-1", "IBM-1047", diag);

                  expect(result_original).ToBe(input);
                  expect(result.length()).ToBe(input.length());
                  expect(diag.e_msg_len).ToBe(0);
                });

             it("should handle conversion failure gracefully with invalid encoding",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string input = "Test data";
                  string result = zut_encode(input, "INVALID-ENCODING", "UTF-8", diag);

                  expect(result).ToBe("");
                  expect(diag.e_msg_len).ToBeGreaterThan(0);
                  expect(string(diag.e_msg)).ToContain("Cannot open converter");
                });

             it("should handle target encoding failure gracefully",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string input = "Test data";
                  string result = zut_encode(input, "UTF-8", "INVALID-TARGET", diag);

                  expect(result).ToBe("");
                  expect(diag.e_msg_len).ToBeGreaterThan(0);
                  expect(string(diag.e_msg)).ToContain("Cannot open converter");
                });

             it("should handle vector conversion with same encodings",
                []() -> void
                {
                  ZDIAG diag = {0};
                  string input = "Same encoding vector test";

                  vector<char> result = zut_encode(input.data(), input.size(), "UTF-8", "UTF-8", diag);

                  expect(diag.e_msg_len).ToBe(0);
                  expect(result.size()).ToBe(input.length());

                  string result_str(result.begin(), result.end());
                  expect(result_str).ToBe(input);
                });

             it("should handle binary data through encoding conversion",
                []() -> void
                {
                  ZDIAG diag = {0};

                  // Create test data with null bytes and binary content
                  vector<char> binary_input = {'H', 'e', 'l', 'l', 'o', '\0', 'W', 'o', 'r', 'l', 'd'};

                  vector<char> result = zut_encode(binary_input.data(), binary_input.size(), "UTF-8", "UTF-8", diag);

                  expect(diag.e_msg_len).ToBe(0);
                  expect(result.size()).ToBe(binary_input.size());

                  // Verify binary content is preserved
                  for (size_t i = 0; i < binary_input.size(); i++)
                  {
                    expect(result[i]).ToBe(binary_input[i]);
                  }
                });
           });
}
