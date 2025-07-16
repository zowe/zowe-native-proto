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
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h> // For __e2a_s function

#include "ztest.hpp"
#include "../extern/zb64.h"

using namespace std;
using namespace ztst;

// Helper function to convert EBCDIC string to ASCII
std::string e2a_convert(const std::string &ebcdic_str)
{
  std::string ascii_str = ebcdic_str;
  size_t converted_len = 0;
  converted_len = __e2a_s(const_cast<char *>(ascii_str.c_str()));
  return ascii_str;
}

void zbase64_tests()
{
  describe("zb64 encode tests",
           []() -> void
           {
             it("should encode empty string",
                []() -> void
                {
                  std::string input = e2a_convert("");
                  int flen = 0;
                  char *result = base64(input.c_str(), input.length(), &flen);
                  std::string result_str(result, flen);
                  free(result);
                  Expect(result_str).ToBe("");
                });

             it("should encode 'Hello World'",
                []() -> void
                {
                  std::string input = e2a_convert("Hello World");
                  int flen = 0;
                  char *result = base64(input.c_str(), input.length(), &flen);
                  std::string result_str(result, flen);
                  free(result);
                  Expect(result_str).ToBe("SGVsbG8gV29ybGQ=");
                });

             it("should encode 'f'",
                []() -> void
                {
                  std::string input = e2a_convert("f");
                  int flen = 0;
                  char *result = base64(input.c_str(), input.length(), &flen);
                  std::string result_str(result, flen);
                  free(result);
                  Expect(result_str).ToBe("Zg==");
                });

             it("should encode 'fo'",
                []() -> void
                {
                  std::string input = e2a_convert("fo");
                  int flen = 0;
                  char *result = base64(input.c_str(), input.length(), &flen);
                  std::string result_str(result, flen);
                  free(result);
                  Expect(result_str).ToBe("Zm8=");
                });

             it("should encode 'foo'",
                []() -> void
                {
                  std::string input = e2a_convert("foo");
                  int flen = 0;
                  char *result = base64(input.c_str(), input.length(), &flen);
                  std::string result_str(result, flen);
                  free(result);
                  Expect(result_str).ToBe("Zm9v");
                });

             it("should encode 'foob'",
                []() -> void
                {
                  std::string input = e2a_convert("foob");
                  int flen = 0;
                  char *result = base64(input.c_str(), input.length(), &flen);
                  std::string result_str(result, flen);
                  free(result);
                  Expect(result_str).ToBe("Zm9vYg==");
                });

             it("should encode 'fooba'",
                []() -> void
                {
                  std::string input = e2a_convert("fooba");
                  int flen = 0;
                  char *result = base64(input.c_str(), input.length(), &flen);
                  std::string result_str(result, flen);
                  free(result);
                  Expect(result_str).ToBe("Zm9vYmE=");
                });

             it("should encode 'foobar'",
                []() -> void
                {
                  std::string input = e2a_convert("foobar");
                  int flen = 0;
                  char *result = base64(input.c_str(), input.length(), &flen);
                  std::string result_str(result, flen);
                  free(result);
                  Expect(result_str).ToBe("Zm9vYmFy");
                });

             it("should encode binary data with null bytes",
                []() -> void
                {
                  std::vector<char> input = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
                  int flen = 0;
                  char *result = base64(input.data(), input.size(), &flen);
                  std::string result_str(result, flen);
                  free(result);
                  Expect(result_str).ToBe("AAECAwQF");
                });

             it("should encode all 256 byte values",
                []() -> void
                {
                  std::vector<char> input;
                  for (int i = 0; i < 256; i++)
                  {
                    input.push_back(static_cast<char>(i));
                  }
                  int flen = 0;
                  char *result = base64(input.data(), input.size(), &flen);
                  free(result);
                  Expect(flen).ToBe(344); // (256 + 2) / 3 * 4 = 344
                });

             it("should handle large input",
                []() -> void
                {
                  std::string input(10000, 'A');
                  // Convert the entire string to ASCII
                  size_t converted_len = 0;
                  converted_len = __e2a_s(const_cast<char *>(input.c_str()));

                  int flen = 0;
                  char *result = base64(input.c_str(), input.length(), &flen);
                  free(result);
                  Expect(flen).ToBe(13336); // (10000 + 2) / 3 * 4 = 13336
                });
           });

  describe("zb64 decode tests",
           []() -> void
           {
             it("should decode empty string",
                []() -> void
                {
                  std::string input = "";
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  std::string result_str(reinterpret_cast<char *>(result), flen);
                  free(result);
                  Expect(result_str).ToBe(e2a_convert(""));
                });

             it("should decode 'SGVsbG8gV29ybGQ=' to 'Hello World'",
                []() -> void
                {
                  std::string input = "SGVsbG8gV29ybGQ=";
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  std::string result_str(reinterpret_cast<char *>(result), flen);
                  free(result);
                  Expect(result_str).ToBe(e2a_convert("Hello World"));
                });

             it("should decode 'Zg==' to 'f'",
                []() -> void
                {
                  std::string input = "Zg==";
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  std::string result_str(reinterpret_cast<char *>(result), flen);
                  free(result);
                  Expect(result_str).ToBe(e2a_convert("f"));
                });

             it("should decode 'Zm8=' to 'fo'",
                []() -> void
                {
                  std::string input = "Zm8=";
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  std::string result_str(reinterpret_cast<char *>(result), flen);
                  free(result);
                  Expect(result_str).ToBe(e2a_convert("fo"));
                });

             it("should decode 'Zm9v' to 'foo'",
                []() -> void
                {
                  std::string input = "Zm9v";
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  std::string result_str(reinterpret_cast<char *>(result), flen);
                  free(result);
                  Expect(result_str).ToBe(e2a_convert("foo"));
                });

             it("should decode 'Zm9vYg==' to 'foob'",
                []() -> void
                {
                  std::string input = "Zm9vYg==";
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  std::string result_str(reinterpret_cast<char *>(result), flen);
                  free(result);
                  Expect(result_str).ToBe(e2a_convert("foob"));
                });

             it("should decode 'Zm9vYmE=' to 'fooba'",
                []() -> void
                {
                  std::string input = "Zm9vYmE=";
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  std::string result_str(reinterpret_cast<char *>(result), flen);
                  free(result);
                  Expect(result_str).ToBe(e2a_convert("fooba"));
                });

             it("should decode 'Zm9vYmFy' to 'foobar'",
                []() -> void
                {
                  std::string input = "Zm9vYmFy";
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  std::string result_str(reinterpret_cast<char *>(result), flen);
                  free(result);
                  Expect(result_str).ToBe(e2a_convert("foobar"));
                });

             it("should handle binary data with null bytes",
                []() -> void
                {
                  std::string input = e2a_convert("AAECAwQF");
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  std::vector<char> result_vec(reinterpret_cast<char *>(result), reinterpret_cast<char *>(result) + flen);
                  free(result);
                  std::vector<char> expected = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
                  Expect(result_vec.size()).ToBe(expected.size());
                  for (size_t i = 0; i < expected.size(); i++)
                  {
                    Expect(static_cast<unsigned char>(result_vec[i])).ToBe(static_cast<unsigned char>(expected[i]));
                  }
                });

             it("should handle invalid input gracefully",
                []() -> void
                {
                  std::string input = e2a_convert("ABC@"); // @ is not valid base64
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  // With SAFEBASE64 defined, invalid input should return NULL
                  Expect(result).ToBe(nullptr);
                });

             it("should handle invalid input length",
                []() -> void
                {
                  std::string input = e2a_convert("ABC"); // Not multiple of 4
                  int flen = 0;
                  unsigned char *result = unbase64(input.c_str(), input.length(), &flen);
                  // With SAFEBASE64 defined, invalid input should return NULL
                  Expect(result).ToBe(nullptr);
                });
           });

  describe("zb64 round-trip tests",
           []() -> void
           {
             it("should encode and decode 'Hello World' correctly",
                []() -> void
                {
                  std::string original = e2a_convert("Hello World");

                  // Encode
                  int encode_len = 0;
                  char *encoded = base64(original.c_str(), original.length(), &encode_len);

                  // Decode
                  int decode_len = 0;
                  unsigned char *decoded = unbase64(encoded, encode_len, &decode_len);
                  std::string decoded_str(reinterpret_cast<char *>(decoded), decode_len);

                  free(encoded);
                  free(decoded);

                  Expect(decoded_str).ToBe(original);
                });

             it("should handle empty string round-trip",
                []() -> void
                {
                  std::string original = e2a_convert("");

                  // Encode
                  int encode_len = 0;
                  char *encoded = base64(original.c_str(), original.length(), &encode_len);

                  // Decode
                  int decode_len = 0;
                  unsigned char *decoded = unbase64(encoded, encode_len, &decode_len);
                  std::string decoded_str(reinterpret_cast<char *>(decoded), decode_len);

                  free(encoded);
                  free(decoded);

                  Expect(decoded_str).ToBe(original);
                });

             it("should handle single character round-trip",
                []() -> void
                {
                  std::string original = e2a_convert("A");

                  // Encode
                  int encode_len = 0;
                  char *encoded = base64(original.c_str(), original.length(), &encode_len);

                  // Decode
                  int decode_len = 0;
                  unsigned char *decoded = unbase64(encoded, encode_len, &decode_len);
                  std::string decoded_str(reinterpret_cast<char *>(decoded), decode_len);

                  free(encoded);
                  free(decoded);

                  Expect(decoded_str).ToBe(original);
                });

             it("should handle binary data round-trip",
                []() -> void
                {
                  std::vector<char> original;
                  for (int i = 0; i < 256; i++)
                  {
                    original.push_back(static_cast<char>(i));
                  }

                  // Encode
                  int encode_len = 0;
                  char *encoded = base64(original.data(), original.size(), &encode_len);

                  // Decode
                  int decode_len = 0;
                  unsigned char *decoded = unbase64(encoded, encode_len, &decode_len);
                  std::vector<char> decoded_vec(reinterpret_cast<char *>(decoded), reinterpret_cast<char *>(decoded) + decode_len);

                  free(encoded);
                  free(decoded);

                  Expect(decoded_vec.size()).ToBe(original.size());
                  for (size_t i = 0; i < original.size(); i++)
                  {
                    Expect(static_cast<unsigned char>(decoded_vec[i])).ToBe(static_cast<unsigned char>(original[i]));
                  }
                });

             it("should handle large data round-trip",
                []() -> void
                {
                  std::string original(1000, 'X');
                  original += e2a_convert("Hello World");
                  original += std::string(1000, 'Y');

                  // Convert the entire string to ASCII
                  size_t converted_len = 0;
                  converted_len = __e2a_s(const_cast<char *>(original.c_str()));

                  // Encode
                  int encode_len = 0;
                  char *encoded = base64(original.c_str(), original.length(), &encode_len);

                  // Decode
                  int decode_len = 0;
                  unsigned char *decoded = unbase64(encoded, encode_len, &decode_len);
                  std::string decoded_str(reinterpret_cast<char *>(decoded), decode_len);

                  free(encoded);
                  free(decoded);

                  Expect(decoded_str).ToBe(original);
                });

             it("should handle special characters round-trip",
                []() -> void
                {
                  std::string original = e2a_convert("!@#$%^&*()_+-=[]{}|;':\",./<>?");

                  // Encode
                  int encode_len = 0;
                  char *encoded = base64(original.c_str(), original.length(), &encode_len);

                  // Decode
                  int decode_len = 0;
                  unsigned char *decoded = unbase64(encoded, encode_len, &decode_len);
                  std::string decoded_str(reinterpret_cast<char *>(decoded), decode_len);

                  free(encoded);
                  free(decoded);

                  Expect(decoded_str).ToBe(original);
                });

             it("should handle binary data with control characters round-trip",
                []() -> void
                {
                  // Test with binary data containing control characters
                  std::string original = e2a_convert("Hello");
                  original += std::string("\x00\x01\x02\x03\x04\x05", 6);
                  original += e2a_convert("World");

                  // Encode
                  int encode_len = 0;
                  char *encoded = base64(original.c_str(), original.length(), &encode_len);

                  // Decode
                  int decode_len = 0;
                  unsigned char *decoded = unbase64(encoded, encode_len, &decode_len);
                  std::string decoded_str(reinterpret_cast<char *>(decoded), decode_len);

                  free(encoded);
                  free(decoded);

                  Expect(decoded_str).ToBe(original);
                });
           });

  describe("zb64 performance and edge cases",
           []() -> void
           {
             it("should handle C API correctly",
                []() -> void
                {
                  const char *input_ebcdic = "Hello World";
                  std::string input_ascii = e2a_convert(input_ebcdic);
                  size_t input_len = input_ascii.length();

                  // Encode
                  int encode_len = 0;
                  char *encoded = base64(input_ascii.c_str(), input_len, &encode_len);

                  // Decode
                  int decode_len = 0;
                  unsigned char *decoded = unbase64(encoded, encode_len, &decode_len);
                  std::string result(reinterpret_cast<char *>(decoded), decode_len);

                  free(encoded);
                  free(decoded);

                  Expect(result).ToBe(input_ascii);
                });

             it("should calculate encoded size correctly",
                []() -> void
                {
                  // Test various input sizes to verify encoded size calculation
                  for (int input_size = 0; input_size <= 6; input_size++)
                  {
                    std::string input(input_size, 'A');
                    // Convert to ASCII
                    size_t converted_len = 0;
                    converted_len = __e2a_s(const_cast<char *>(input.c_str()));

                    int flen = 0;
                    char *result = base64(input.c_str(), input.length(), &flen);
                    free(result);

                    // Expected size is (input_size + 2) / 3 * 4
                    int expected_size = ((input_size + 2) / 3) * 4;
                    Expect(flen).ToBe(expected_size);
                  }
                });

             it("should handle null bytes in middle of data",
                []() -> void
                {
                  std::vector<char> original = {'A', 'B', 0x00, 'C', 'D'};
                  // Convert ASCII characters to ASCII (A, B, C, D)
                  for (size_t i = 0; i < original.size(); i++)
                  {
                    if (original[i] != 0x00)
                    {
                      std::string temp(1, original[i]);
                      size_t converted_len = 0;
                      converted_len = __e2a_s(const_cast<char *>(temp.c_str()));
                      original[i] = temp[0];
                    }
                  }

                  // Encode
                  int encode_len = 0;
                  char *encoded = base64(original.data(), original.size(), &encode_len);

                  // Decode
                  int decode_len = 0;
                  unsigned char *decoded = unbase64(encoded, encode_len, &decode_len);
                  std::vector<char> decoded_vec(reinterpret_cast<char *>(decoded), reinterpret_cast<char *>(decoded) + decode_len);

                  free(encoded);
                  free(decoded);

                  Expect(decoded_vec.size()).ToBe(original.size());
                  for (size_t i = 0; i < original.size(); i++)
                  {
                    Expect(decoded_vec[i]).ToBe(original[i]);
                  }
                });

             it("should validate base64 integrity",
                []() -> void
                {
                  // Test valid base64 strings
                  std::string valid1 = e2a_convert("SGVsbG8=");
                  Expect(base64integrity(valid1.c_str(), valid1.length())).ToBe(1);

                  std::string valid2 = e2a_convert("SGVsbG8gV29ybGQ=");
                  Expect(base64integrity(valid2.c_str(), valid2.length())).ToBe(1);

                  // Test invalid base64 strings
                  std::string invalid1 = e2a_convert("SGVsbG8"); // Not multiple of 4
                  Expect(base64integrity(invalid1.c_str(), invalid1.length())).ToBe(0);

                  std::string invalid2 = e2a_convert("SGVs@G8="); // Invalid character
                  Expect(base64integrity(invalid2.c_str(), invalid2.length())).ToBe(0);
                });
           });
}