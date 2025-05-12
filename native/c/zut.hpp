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

#ifndef ZUT_HPP
#define ZUT_HPP

#include <iconv.h>
#include <iostream>
#include <vector>
#include <string>
#include "zcntype.h"

typedef struct ZConvData
{
    char *input;
    size_t input_size;
    size_t max_output_size;
    char *output_buffer;
    char *output_iter;
} ZConvData;

class Base64Decoder
{
    std::string leftover;
    static const std::string base64_chars;

    static inline bool is_base64(unsigned char c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

public:
    // Decodes input (may have leftover from previous chunk)
    std::vector<unsigned char> decode(const std::string &input)
    {
        std::string data = leftover + input;
        size_t valid_len = (data.size() / 4) * 4;
        leftover = data.substr(valid_len);
        std::string to_decode = data.substr(0, valid_len);

        std::vector<unsigned char> ret;
        int in_len = to_decode.size();
        int i = 0;
        unsigned char char_array_4[4], char_array_3[3];

        while (in_len-- && (to_decode[i] != '=') && is_base64(to_decode[i]))
        {
            char_array_4[i % 4] = to_decode[i];
            if (++i % 4 == 0)
            {
                for (int j = 0; j < 4; j++)
                    char_array_4[j] = base64_chars.find(char_array_4[j]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                ret.insert(ret.end(), char_array_3, char_array_3 + 3);
            }
        }
        return ret;
    }

    // Call at EOF to flush any remaining data
    std::vector<unsigned char> flush()
    {
        std::vector<unsigned char> ret;
        if (leftover.size() >= 4)
        {
            ret = decode("");
        }
        leftover.clear();
        return ret;
    }
};
const std::string Base64Decoder::base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

class Base64Encoder
{
    std::vector<unsigned char> leftover;
    static const std::string base64_chars;

public:
    // Feed in a chunk of bytes, get a base64-encoded string chunk out
    std::string encode(const std::vector<unsigned char> &input)
    {
        std::vector<unsigned char> data = leftover;
        data.insert(data.end(), input.begin(), input.end());
        size_t valid_len = (data.size() / 3) * 3;
        leftover.assign(data.begin() + valid_len, data.end());

        std::string out;
        for (size_t i = 0; i < valid_len; i += 3)
        {
            unsigned char a = data[i];
            unsigned char b = data[i + 1];
            unsigned char c = data[i + 2];
            out += base64_chars[(a & 0xfc) >> 2];
            out += base64_chars[((a & 0x03) << 4) | ((b & 0xf0) >> 4)];
            out += base64_chars[((b & 0x0f) << 2) | ((c & 0xc0) >> 6)];
            out += base64_chars[c & 0x3f];
        }
        return out;
    }

    // Call at EOF to flush any remaining bytes (with padding)
    std::string flush()
    {
        std::string out;
        if (!leftover.empty())
        {
            unsigned char a = leftover[0];
            unsigned char b = leftover.size() > 1 ? leftover[1] : 0;
            out += base64_chars[(a & 0xfc) >> 2];
            if (leftover.size() == 1)
            {
                out += base64_chars[((a & 0x03) << 4)];
                out += "==";
            }
            else if (leftover.size() == 2)
            {
                out += base64_chars[((a & 0x03) << 4) | ((b & 0xf0) >> 4)];
                out += base64_chars[((b & 0x0f) << 2)];
                out += "=";
            }
            leftover.clear();
        }
        return out;
    }
};
const std::string Base64Encoder::base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

int zut_search(std::string);
int zut_run(std::string);
int zut_substitute_sybmol(std::string, std::string &);
int zut_bpxwdyn(std::string, unsigned int *, std::string &);
void zut_dump_storage(std::string, const void *, size_t);
int zut_hello(std::string);
char zut_get_hex_char(int);
int zut_get_current_user(std::string &);
void zut_uppercase_pad_truncate(char *, std::string, int);
int zut_convert_dsect();
bool zut_prepare_encoding(const std::string &encoding_value, ZEncode *opts);
void zut_print_string_as_bytes(std::string &input);
std::vector<uint8_t> zut_get_contents_as_bytes(const std::string &hex_string);
uint32_t zut_calc_adler32_checksum(const std::string &input);

size_t zut_iconv(iconv_t cd, ZConvData &data, ZDIAG &diag);
std::string zut_build_etag(const size_t mtime, const size_t byte_size);
std::string zut_encode(const std::string &input_str, const std::string &from_encoding, const std::string &to_encoding, ZDIAG &diag);
std::string zut_format_as_csv(std::vector<std::string> &fields);
std::string &zut_rtrim(std::string &s, const char *t = " ");
std::string &zut_ltrim(std::string &s, const char *t = " ");
std::string &zut_trim(std::string &s, const char *t = " ");

#endif