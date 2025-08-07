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

#ifndef ZBASE64_H
#define ZBASE64_H

#include <string>
#include <vector>
#include <stdexcept>

namespace zbase64
{

// Standard Base64 alphabet (what we want to output)
static const char encode_table_ascii[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'};

// Get the decode table
static const unsigned char *get_ebcdic_decode_table()
{
  static unsigned char table[256];
  static bool initialized = false;

  if (!initialized)
  {
    // Initialize all to invalid
    for (int i = 0; i < 256; ++i)
    {
      table[i] = 255;
    }

    // A-I: EBCDIC 193-201 -> Base64 0-8
    for (int i = 0; i < 9; ++i)
    {
      table[193 + i] = i;
    }

    // J-R: EBCDIC 209-217 -> Base64 9-17
    for (int i = 0; i < 9; ++i)
    {
      table[209 + i] = 9 + i;
    }

    // S-Z: EBCDIC 226-233 -> Base64 18-25
    for (int i = 0; i < 8; ++i)
    {
      table[226 + i] = 18 + i;
    }

    // a-i: EBCDIC 129-137 -> Base64 26-34
    for (int i = 0; i < 9; ++i)
    {
      table[129 + i] = 26 + i;
    }

    // j-r: EBCDIC 145-153 -> Base64 35-43
    for (int i = 0; i < 9; ++i)
    {
      table[145 + i] = 35 + i;
    }

    // s-z: EBCDIC 162-169 -> Base64 44-51
    for (int i = 0; i < 8; ++i)
    {
      table[162 + i] = 44 + i;
    }

    // 0-9: EBCDIC 240-249 -> Base64 52-61
    for (int i = 0; i < 10; ++i)
    {
      table[240 + i] = 52 + i;
    }

    // Special characters
    table[78] = 62;   // + -> 62
    table[97] = 63;   // / -> 63
    table[126] = 254; // = -> padding marker

    initialized = true;
  }

  return table;
}

// Fast inline function to calculate encoded size
inline size_t encoded_size(size_t input_size)
{
  return ((input_size + 2) / 3) * 4;
}

// Fast inline function to calculate maximum decoded size
inline size_t max_decoded_size(size_t input_size)
{
  return (input_size / 4) * 3;
}

// High-performance encode function (produces ASCII Base64 output)
inline std::vector<char> encode(const char *input, size_t input_len)
{
  if (input_len == 0)
  {
    return std::vector<char>();
  }

  const size_t output_len = encoded_size(input_len);

  std::vector<char> output;
  output.resize(output_len); // Pre-allocate exact size
  char *dst = &output[0];    // Direct pointer to output buffer

  const unsigned char *src = reinterpret_cast<const unsigned char *>(input);
  const size_t full_blocks = input_len / 3;

  // Process blocks of 12 bytes (4 x 3-byte groups) at a time
  size_t i = 0;
  for (; i + 4 <= full_blocks; i += 4)
  {
    for (size_t j = 0; j < 4; j++)
    {
      const unsigned char *block = src + (i + j) * 3;
      const unsigned int combined = (block[0] << 16) | (block[1] << 8) | block[2];

      dst[0] = encode_table_ascii[(combined >> 18) & 0x3F];
      dst[1] = encode_table_ascii[(combined >> 12) & 0x3F];
      dst[2] = encode_table_ascii[(combined >> 6) & 0x3F];
      dst[3] = encode_table_ascii[combined & 0x3F];
      dst += 4;
    }
  }

  // Process remaining full blocks
  for (; i < full_blocks; i++)
  {
    const unsigned char *block = src + i * 3;
    const unsigned int combined = (block[0] << 16) | (block[1] << 8) | block[2];

    dst[0] = encode_table_ascii[(combined >> 18) & 0x3F];
    dst[1] = encode_table_ascii[(combined >> 12) & 0x3F];
    dst[2] = encode_table_ascii[(combined >> 6) & 0x3F];
    dst[3] = encode_table_ascii[combined & 0x3F];
    dst += 4;
  }

  // Handle remaining bytes (0-2)
  const size_t remaining = input_len - (full_blocks * 3);
  if (remaining > 0)
  {
    unsigned int combined = src[full_blocks * 3] << 16;
    if (remaining > 1)
    {
      combined |= src[full_blocks * 3 + 1] << 8;
    }

    dst[0] = encode_table_ascii[(combined >> 18) & 0x3F];
    dst[1] = encode_table_ascii[(combined >> 12) & 0x3F];
    dst[2] = (remaining > 1) ? encode_table_ascii[(combined >> 6) & 0x3F] : '=';
    dst[3] = '=';
  }

  return output;
}

inline std::vector<char> encode(const char *input, size_t input_len, std::vector<char> *left_over)
{
  char *temp_input = const_cast<char *>(input);
  if (left_over != nullptr && left_over->size() > 0)
  {
    std::vector<char> combined_input;
    combined_input.reserve(left_over->size() + input_len);
    combined_input.insert(combined_input.end(), left_over->begin(), left_over->end());
    combined_input.insert(combined_input.end(), input, input + input_len);
    temp_input = &combined_input[0];
    input_len = combined_input.size();
    left_over->clear();
  }
  else if (input_len == 0)
  {
    return std::vector<char>();
  }

  // Calculate how many bytes are leftover
  size_t extra_count = input_len % 3;
  size_t total_bytes = input_len - extra_count;

  // Copy leftover bytes to the left_over parameter
  if (left_over != nullptr && extra_count > 0)
  {
    left_over->resize(extra_count);
    for (size_t i = 0; i < extra_count; i++)
    {
      (*left_over)[i] = temp_input[total_bytes + i];
    }
  }

  // Encode only the complete part (multiple of 3 bytes)
  return encode(temp_input, total_bytes);
}

// Convenience overload for string input
inline std::string encode(const std::string &input)
{
  std::vector<char> result = encode(&input[0], input.size());
  return std::string(result.begin(), result.end());
}

// High-performance decode function (handles EBCDIC Base64 input)
inline std::vector<char> decode(const char *input, size_t input_len)
{
  if (input_len == 0)
  {
    return std::vector<char>();
  }

  // Validate input length (must be multiple of 4)
  if (input_len % 4 != 0)
  {
    throw std::invalid_argument("Invalid base64 input length");
  }

  const unsigned char *decode_table = get_ebcdic_decode_table();

  // Count padding characters (EBCDIC '=' is 126)
  size_t padding = 0;
  if (input_len >= 2)
  {
    if (static_cast<unsigned char>(input[input_len - 1]) == 126)
      padding++;
    if (static_cast<unsigned char>(input[input_len - 2]) == 126)
      padding++;
  }

  const size_t output_len = max_decoded_size(input_len) - padding;
  std::vector<char> output;
  output.reserve(output_len);

  const unsigned char *src = reinterpret_cast<const unsigned char *>(input);
  const unsigned char *const src_end = src + input_len;

  // Process 4 characters at a time for optimal performance
  while (src + 4 <= src_end)
  {
    const unsigned char c0 = decode_table[src[0]];
    const unsigned char c1 = decode_table[src[1]];
    const unsigned char c2 = (src[2] == 126) ? 0 : decode_table[src[2]]; // 126 = EBCDIC '='
    const unsigned char c3 = (src[3] == 126) ? 0 : decode_table[src[3]]; // 126 = EBCDIC '='

    // Validate non-padding characters
    if ((c0 | c1) & 0x80)
    {
      throw std::invalid_argument("Invalid base64 character");
    }
    if (src[2] != 126 && (c2 & 0x80))
    {
      throw std::invalid_argument("Invalid base64 character");
    }
    if (src[3] != 126 && (c3 & 0x80))
    {
      throw std::invalid_argument("Invalid base64 character");
    }

    // Combine 4 x 6-bit values into 24 bits, then extract 3 bytes
    const unsigned int combined = (c0 << 18) | (c1 << 12) | (c2 << 6) | c3;

    output.push_back(static_cast<char>((combined >> 16) & 0xFF));

    if (src[2] != 126)
    {
      output.push_back(static_cast<char>((combined >> 8) & 0xFF));

      if (src[3] != 126)
      {
        output.push_back(static_cast<char>(combined & 0xFF));
      }
    }

    src += 4;
  }

  return output;
}

// Convenience overload for string input
inline std::string decode(const std::string &input)
{
  std::vector<char> result = decode(&input[0], input.size());
  return std::string(result.begin(), result.end());
}

} // namespace zbase64

#endif // ZBASE64_H
