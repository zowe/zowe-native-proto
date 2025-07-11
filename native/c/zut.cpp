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

#include <stdio.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include "zut.hpp"
#include "zutm.h"
#include "zutm31.h"
#include <ios>
#include "zdyn.h"

using namespace std;

int zut_search(string parms)
{
  return ZUTSRCH();
}

int zut_run(string program)
{
  return ZUTRUN(program.c_str());
}

unsigned char zut_get_key()
{
  return ZUTMGKEY();
}

int zut_substitute_symbol(string pattern, string &result)
{
  SYMBOL_DATA *parms = (SYMBOL_DATA *)__malloc31(sizeof(SYMBOL_DATA));
  memset(parms, 0x00, sizeof(SYMBOL_DATA));

  strcpy(parms->input, pattern.c_str());
  parms->length = strlen(pattern.c_str());
  int rc = ZUTSYMBP(parms);
  if (RTNCD_SUCCESS != rc)
  {
    free(parms);
    return rc;
  }
  result += string(parms->output);
  free(parms);
  return RTNCD_SUCCESS;
}

int zut_convert_dsect()
{
  return ZUTEDSCT();
}

void zut_uppercase_pad_truncate(char *target, string source, int len)
{
  memset(target, ' ', len);                                           // pad with spaces
  transform(source.begin(), source.end(), source.begin(), ::toupper); // upper case
  int length = source.size() > len ? len : source.size();             // truncate
  strncpy(target, source.c_str(), length);
}

int zut_bpxwdyn(string parm, unsigned int *code, string &resp)
{
  char bpx_response[RET_ARG_MAX_LEN * MSG_ENTRIES + 1] = {0};

  unsigned char *p = (unsigned char *)__malloc31(sizeof(BPXWDYN_PARM) + sizeof(BPXWDYN_RESPONSE));
  memset(p, 0x00, sizeof(BPXWDYN_PARM) + sizeof(BPXWDYN_RESPONSE));

  BPXWDYN_PARM *bparm = (BPXWDYN_PARM *)p;
  BPXWDYN_RESPONSE *response = (BPXWDYN_RESPONSE *)(p + sizeof(BPXWDYN_PARM));

  bparm->len = sprintf(bparm->str, "%s", parm.c_str());
  int rc = ZUTWDYN(bparm, response);

  resp = string(response->response);
  *code = response->code;

  free(p);

  return rc;
}

string zut_build_etag(const size_t mtime, const size_t byte_size)
{
  stringstream ss;
  ss << std::hex << mtime;
  ss << "-";
  ss << std::hex << byte_size;
  return ss.str();
}

int zut_get_current_user(string &struser)
{
  int rc = 0;
  char user[9] = {0};

  rc = ZUTMGUSR(user);
  if (0 != rc)
    return rc;

  for (int i = sizeof(user) - 1; i >= 0; i--)
  {
    if (user[i] == ' ' || user[i] == 0x00)
      user[i] = 0x00;
    else
      break;
  }

  struser = string(user);
  return rc;
}

int zut_hello(string name)
{
  // #if defined(__IBMC__) || defined(__IBMCPP__)
  // #pragma convert(819)
  // #endif

  if (name.empty())
    cout << "Hello world!" << endl;
  else
    cout << "Hello " << name << endl;
  return 0;

  // #if defined(__IBMC__) || defined(__IBMCPP__)
  // #pragma convert(0)
  // #endif

  return 0;
}

/**
 * Get char value from hex byte, e.g. 0x0E -> 'E'
 */
char zut_get_hex_char(int num)
{
  char val = '?';

  switch (num)
  {
  case 0:
    /* code */
    val = '0';
    break;
  case 1:
    /* code */
    val = '1';
    break;
  case 2:
    /* code */
    val = '2';
    break;
  case 3:
    /* code */
    val = '3';
    break;
  case 4:
    /* code */
    val = '4';
    break;
  case 5:
    /* code */
    val = '5';
    break;
  case 6:
    /* code */
    val = '6';
    break;
  case 7:
    /* code */
    val = '7';
    break;
  case 8:
    /* code */
    val = '8';
    break;
  case 9:
    /* code */
    val = '9';
    break;
  case 10:
    /* code */
    val = 'A';
    break;
  case 11:
    /* code */
    val = 'B';
    break;
  case 12:
    /* code */
    val = 'C';
    break;
  case 13:
    /* code */
    val = 'D';
    break;
  case 14:
    /* code */
    val = 'E';
    break;
  case 15:
    /* code */
    val = 'F';
    break;
  default:
    break;
  }

  return val;
}

// built from pseudocode in https://en.wikipedia.org/wiki/Adler-32#Calculation
// exploits SIMD for performance boosts on z13+
uint32_t zut_calc_adler32_checksum(const string &input)
{
  const uint32_t MOD_ADLER = 65521u;
  uint32_t a = 1u;
  uint32_t b = 0u;
  const size_t len = input.length();
  const char *data = input.data();

  const size_t block_size = 16;
  size_t i = 0;

  // Process data in blocks of 16 bytes
  while (i + block_size <= len)
  {
    for (size_t j = 0; j < block_size; j++)
    {
      // A_i = 1 + (D_i + D_(i+1) + ... + D_(i+n-1))
      a += (uint8_t)data[i + j];
      // B_i = A_i + B_(i-1)
      b += a;
    }

    // Apply modulo to prevent overflow
    a %= MOD_ADLER;
    b %= MOD_ADLER;
    i += block_size;
  }

  // Process remaining bytes in the input
  while (i < len)
  {
    a += (uint8_t)data[i];
    b += a;
    i++;
  }

  a %= MOD_ADLER;
  b %= MOD_ADLER;

  // Adler-32(D) = B * 65536 + A
  return (b << 16) | a;
}

/**
 * Prints the input string as bytes to stdout.
 * @param input The input string to be printed.
 */
void zut_print_string_as_bytes(string &input)
{
  for (char *p = (char *)input.data(); p < (input.data() + input.length()); p++)
  {
    if (p == (input.data() + input.length() - 1))
    {
      printf("%02x", (unsigned char)*p);
    }
    else
    {
      printf("%02x ", (unsigned char)*p);
    }
  }
  cout << endl;
}

/**
 * Converts a zero-padded hex string to a vector of bytes (e.g. "010203" -> [0x01, 0x02, 0x03]).
 * @param hex_string The hex string to convert.
 * @return A vector of bytes representing the hex string.
 */
vector<uint8_t> zut_get_contents_as_bytes(const string &hex_string)
{
  vector<uint8_t> bytes;
  // If the hex string is not zero-padded, return an empty vector
  if (hex_string.length() % 2 != 0)
  {
    return bytes;
  }

  for (auto i = 0u; i < hex_string.size(); i += 2u)
  {
    const auto byte_str = hex_string.substr(i, 2);
    const uint8_t byte = strtoul(byte_str.c_str(), nullptr, 16);
    bytes.push_back(byte);
  }

  return bytes;
}

/**
 * Prepares the encoding options.
 *
 * @param encoding_value - The value of the encoding option.
 * @param opts - Pointer to the ZEncode options.
 *
 * @return true if the encoding options are successfully prepared, false otherwise.
 */
bool zut_prepare_encoding(const std::string &encoding_value, ZEncode *opts)
{
  if (encoding_value.empty() || opts == nullptr)
  {
    return false;
  }

  if (encoding_value.size() < sizeof(opts->codepage))
  {
    memcpy(opts->codepage, encoding_value.data(), encoding_value.length() + 1);
    opts->data_type = encoding_value == "binary" ? eDataTypeBinary : eDataTypeText;
    return true;
  }

  return false;
}

/**
 * Converts a string from one encoding to another using the `iconv` function.
 *
 * @param cd `iconv` conversion descriptor
 * @param data required data (input, input size, output pointers) for conversion
 * @param diag diagnostic structure to store error information
 *
 * @return return code from `iconv`
 */
size_t zut_iconv(iconv_t cd, ZConvData &data, ZDIAG &diag)
{
  size_t input_bytes_remaining = data.input_size;
  size_t output_bytes_remaining = data.max_output_size;

  size_t rc = iconv(cd, &data.input, &input_bytes_remaining, &data.output_iter, &output_bytes_remaining);

  // If an error occurred, throw an exception with iconv's return code and errno
  if (-1 == rc)
  {
    diag.e_msg_len = sprintf(diag.e_msg, "[zut_iconv] Error when converting characters. rc=%lu,errno=%d", rc, errno);
    return -1;
  }

  // "If the input conversion is stopped... the value pointed to by inbytesleft will be nonzero and errno is set to indicate the condition"
  if (0 != input_bytes_remaining)
  {
    diag.e_msg_len = sprintf(diag.e_msg, "[zut_iconv] Failed to convert all input bytes. rc=%lu,errno=%d", rc, errno);
    return -1;
  }

  return rc;
}

/**
 * Converts the encoding for a string from one codepage to another.
 * @param input_str input data to convert
 * @param from_encoding current codepage for the input data
 * @param to_encoding desired codepage for the data
 * @param diag diagnostic structure to store error information
 */
string zut_encode(const string &input_str, const string &from_encoding, const string &to_encoding, ZDIAG &diag)
{
  if(from_encoding == to_encoding)
  {
    return input_str;
  }

  iconv_t cd = iconv_open(to_encoding.c_str(), from_encoding.c_str());
  if (cd == (iconv_t)(-1))
  {
    diag.e_msg_len = sprintf(diag.e_msg, "Cannot open converter from %s to %s", from_encoding.c_str(), to_encoding.c_str());
    return "";
  }

  const size_t input_size = input_str.size();
  // maximum possible size assumes UTF-8 data with 4-byte character sequences
  const size_t max_output_size = input_size * 4;

  vector<char> output_buffer(max_output_size, 0);

  // Prepare iconv parameters (copy output_buffer ptr to output_iter to cache start and end positions)
  char *input = (char *)input_str.data();
  char *output_iter = &output_buffer[0];

  ZConvData data = {input, input_size, max_output_size, &output_buffer[0], output_iter};
  size_t iconv_rc = zut_iconv(cd, data, diag);
  iconv_close(cd);
  if (-1 == iconv_rc)
  {
    throw std::runtime_error(diag.e_msg);
  }

  // Copy converted input into a new string and return it to the caller
  return string(&output_buffer[0], data.output_iter - data.output_buffer);
}

/**
 * Converts the encoding for a string from one codepage to another.
 * @param input_str input data to convert
 * @param input_size size of the input data in bytes
 * @param from_encoding current codepage for the input data
 * @param to_encoding desired codepage for the data
 * @param diag diagnostic structure to store error information
 */
vector<char> zut_encode(const char *input_str, size_t input_size, const string &from_encoding, const string &to_encoding, ZDIAG &diag)
{
  if(from_encoding == to_encoding)
  {
    return std::vector<char>(input_str, input_str + input_size);
  }

  iconv_t cd = iconv_open(to_encoding.c_str(), from_encoding.c_str());
  if (cd == (iconv_t)(-1))
  {
    diag.e_msg_len = sprintf(diag.e_msg, "Cannot open converter from %s to %s", from_encoding.c_str(), to_encoding.c_str());
    return vector<char>();
  }

  // maximum possible size assumes UTF-8 data with 4-byte character sequences
  const size_t max_output_size = input_size * 4;

  vector<char> output_buffer(max_output_size, 0);

  // Prepare iconv parameters (copy output_buffer ptr to output_iter to cache start and end positions)
  char *input = const_cast<char *>(input_str);
  char *output_iter = &output_buffer[0];

  ZConvData data = {input, input_size, max_output_size, &output_buffer[0], output_iter};
  size_t iconv_rc = zut_iconv(cd, data, diag);
  iconv_close(cd);
  if (-1 == iconv_rc)
  {
    throw std::runtime_error(diag.e_msg);
  }

  // Shrink output buffer and return it to the caller
  output_buffer.resize(data.output_iter - data.output_buffer);
  return output_buffer;
}

std::string &zut_rtrim(std::string &s, const char *t)
{
  return s.erase(s.find_last_not_of(t) + 1);
}

std::string &zut_ltrim(std::string &s, const char *t)
{
  return s.erase(0, s.find_first_not_of(t));
}

std::string &zut_trim(std::string &s, const char *t)
{
  return zut_ltrim(zut_rtrim(s, t), t);
}

/**
 * Formats a vector of strings as a CSV string.
 *
 * @param fields the vector of strings to format
 * @return the formatted CSV string
 */
string zut_format_as_csv(std::vector<string> &fields)
{
  string formatted;
  for (int i = 0; i < fields.size(); i++)
  {
    formatted += zut_trim(fields.at(i));
    if (i < fields.size() - 1)
    {
      formatted += ",";
    }
  }

  return formatted;
}

int zut_alloc_debug()
{
  int rc = 0;
  unsigned int code = 0;
  string response;
  string zowexdbg = "/tmp/zowex_debug.txt";

  string alloc = "alloc fi(zowexdbg) path('" + zowexdbg + "') pathopts(owronly,ocreat) pathmode(sirusr,siwusr,sirgrp) filedata(text) msg(2)";
  rc = zut_bpxwdyn(alloc, &code, response);

  return rc;
}

int zut_debug_message(const char *message)
{
  fprintf(stderr, "%s", message);
  return 0;
}
