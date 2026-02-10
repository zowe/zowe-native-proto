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
             it("should run shell command",
                []() -> void
                {
                  string output;
                  zut_run_shell_command("echo 'hello world'", output);
                  expect(output).ToBe("'hello world'");
                });
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

             describe("zut_list_parmlib",
                      []() -> void
                      {
                        it("should list parmlib data sets",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             vector<string> parmlibs;
                             zut_list_parmlib(diag, parmlibs);
                             expect(parmlibs.size()).ToBeGreaterThan(0);
                           });
                      });
             describe("zut_bpxwdyn",
                      []() -> void
                      {
                        it("should allocate a sysout data set and get the DS name",
                           []() -> void
                           {
                             std::string cmd = "ALLOC SYSOUT";
                             unsigned int code = 0;
                             std::string dsname = "";
                             std::string resp = "";
                             int rc = zut_bpxwdyn_rtdsn(cmd, &code, resp, dsname);
                             expect(rc).ToBe(0);
                             expect(dsname.size()).ToBeGreaterThan(0);
                             expect(code).ToBe(0);
                           });

                        it("should allocate a data set, get the DD name, and free it",
                           []() -> void
                           {
                             std::string cmd = "ALLOC DA('SYS1.MACLIB') SHR";
                             unsigned int code = 0;
                             std::string ddname = "";
                             std::string resp = "";
                             int rc = zut_bpxwdyn_rtdd(cmd, &code, resp, ddname);
                             expect(rc).ToBe(0);
                             expect(ddname.size()).ToBeGreaterThan(0);
                             expect(code).ToBe(0);

                             cmd = "FREE DD(" + ddname + ")";
                             rc = zut_bpxwdyn(cmd, &code, resp);
                             expect(rc).ToBe(0);
                             expect(code).ToBe(0);
                           });
                      });

             describe("zut_encode",
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

                        it("should handle UTF-8 to EBCDIC conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};

                             // Create UTF-8 "Hello, world!" using known byte values
                             // UTF-8 bytes for "Hello, world!": 0x48 0x65 0x6C 0x6C 0x6F 0x2C 0x20 0x77 0x6F 0x72 0x6C 0x64 0x21
                             vector<char> utf8_hello_world = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21};

                             // Convert UTF-8 to EBCDIC (IBM-1047)
                             vector<char> ebcdic_result = zut_encode(utf8_hello_world.data(), utf8_hello_world.size(), "UTF-8", "IBM-1047", diag);

                             expect(diag.e_msg_len).ToBe(0);
                             expect(ebcdic_result.size()).ToBe(13); // "Hello, world!" is 13 characters

                             // Convert back to UTF-8 to verify round-trip
                             vector<char> utf8_roundtrip = zut_encode(ebcdic_result.data(), ebcdic_result.size(), "IBM-1047", "UTF-8", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(utf8_roundtrip.size()).ToBe(13);

                             // Verify original UTF-8 bytes are preserved
                             for (size_t i = 0; i < utf8_hello_world.size(); i++)
                             {
                               expect(utf8_roundtrip[i]).ToBe(utf8_hello_world[i]);
                             }
                           });

                        it("should handle UTF-8 to UCS-2 conversion with emoji",
                           []() -> void
                           {
                             ZDIAG diag = {0};

                             // UTF-8 bytes for emoji characters:
                             vector<char> utf8_emoji = {
                                 0xF0, 0x9F, 0xA6, 0x80, // 🦀
                                 0xF0, 0x9F, 0x8F, 0x86, // 🏆
                                 0xF0, 0x9F, 0x98, 0x8F, // 😏
                                 0xE2, 0x98, 0x95,       // ☕
                                 0xF0, 0x9F, 0x92, 0xA4  // 💤
                             };

                             // Convert UTF-8 to UCS-2
                             vector<char> ucs2_result = zut_encode(utf8_emoji.data(), utf8_emoji.size(), "UTF-8", "UCS-2", diag);

                             expect(diag.e_msg_len).ToBe(0);
                             expect(ucs2_result.size()).ToBeGreaterThan(0); // UCS-2 will have different size

                             // Convert back to UTF-8 to verify round-trip
                             vector<char> utf8_roundtrip = zut_encode(ucs2_result.data(), ucs2_result.size(), "UCS-2", "UTF-8", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(utf8_roundtrip.size()).ToBe(utf8_emoji.size());

                             // Verify original UTF-8 bytes are preserved
                             for (size_t i = 0; i < utf8_emoji.size(); i++)
                             {
                               expect(utf8_roundtrip[i]).ToBe(utf8_emoji[i]);
                             }
                           });

                        it("should handle IBM-1047 to IBM-037 conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};

                             // Note: String literals on z/OS are typically IBM-1047 by default
                             string input_str = "It's a clean machine";

                             // Convert IBM-1047 to IBM-037
                             string result = zut_encode(input_str, "IBM-1047", "IBM-037", diag);

                             expect(diag.e_msg_len).ToBe(0);
                             expect(result.length()).ToBe(input_str.length());

                             // Convert back to IBM-1047 to verify round-trip
                             string roundtrip = zut_encode(result, "IBM-037", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip.length()).ToBe(input_str.length());

                             // Verify original string is preserved
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 273 (German EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Muller & Sohne GmbH - Grosse: 15cm"; // Using basic ASCII chars for compatibility

                             // Convert IBM-1047 to CCSID 273 (German EBCDIC)
                             string result = zut_encode(input_str, "IBM-1047", "IBM-273", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             // Convert back to verify round-trip
                             string roundtrip = zut_encode(result, "IBM-273", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 500 (International EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "International characters & symbols";

                             // Convert IBM-1047 to CCSID 500
                             string result = zut_encode(input_str, "IBM-1047", "IBM-500", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             // Convert back to verify round-trip
                             string roundtrip = zut_encode(result, "IBM-500", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 285 (UK English EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "50.00 - proper behaviour"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-285", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-285", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 297 (French EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Cafe francais - resume naif"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-297", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-297", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 284 (Spanish EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Nino espanol - ano manana"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-284", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-284", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 280 (Italian EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Citta italiana - piu cosi"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-280", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-280", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 1140 (US EBCDIC with Euro) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Price: 125.50 USD 150.00"; // Using basic chars, Euro symbol handling may vary

                             string result = zut_encode(input_str, "IBM-037", "IBM-1140", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-1140", "IBM-037", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 1141 (German EBCDIC with Euro) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Preis: 99,99 - Grosse: Muller"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-273", "IBM-1141", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-1141", "IBM-273", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 819 (ISO 8859-1) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "ASCII + Latin extras: cafe resume";

                             string result = zut_encode(input_str, "IBM-1047", "ISO8859-1", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "ISO8859-1", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 1252 (Windows Latin-1) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Windows smart quotes - em-dash"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-1252", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-1252", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 290 (Japanese EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Katakana Test"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-290", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-290", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 935 (Simplified Chinese EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Simplified Chinese Test"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-935", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-935", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 937 (Traditional Chinese EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Traditional Chinese Test"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-937", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-937", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 420 (Arabic EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Arabic Test"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-420", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-420", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 424 (Hebrew EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Hebrew Test"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-424", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-424", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });

                        it("should handle CCSID 870 (Latin-2 EBCDIC) conversion",
                           []() -> void
                           {
                             ZDIAG diag = {0};
                             string input_str = "Pristup cestina - Laszlo"; // Using basic chars for compatibility

                             string result = zut_encode(input_str, "IBM-1047", "IBM-870", diag);
                             expect(diag.e_msg_len).ToBe(0);

                             string roundtrip = zut_encode(result, "IBM-870", "IBM-1047", diag);
                             expect(diag.e_msg_len).ToBe(0);
                             expect(roundtrip).ToBe(input_str);
                           });
                      });
           });
}
