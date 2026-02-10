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

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
 
#define _OPEN_SYS_EXT
#include <sys/ps.h>
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
#include "zuttype.h"
#include <_Nascii.h>

using namespace std;

int zut_run_shell_command(string command, string &response)
{
  int rc = 0;
  string response_raw;
  FILE *cmd = popen(command.c_str(), "r");
  if (nullptr == cmd)
  {
    return RTNCD_FAILURE;
  }

  char buffer[256] = {0};
  while (fgets(buffer, sizeof(buffer), cmd) != nullptr)
  {
    response_raw += string(buffer);
  }

  stringstream response_ss(response_raw);

  string line;
  auto index = 0;

  while (getline(response_ss, line))
  {
    index++;
    if (index > 1)
    {
      response += line + '\n';
    }
  }

  rc = pclose(cmd);
  if (0 != rc)
  {
    return WEXITSTATUS(rc);
  }

  return rc;
}

int zut_search(string parms)
{
  return ZUTSRCH(parms.c_str());
}

int zut_run(ZDIAG &diag, string program, string parms)
{
  return ZUTRUN(&diag, program.c_str(), parms.c_str());
}

int zut_run(string program)
{
  ZDIAG diag = {};
  return ZUTRUN(&diag, program.c_str(), NULL);
}

unsigned char zut_get_key()
{
  return ZUTMGKEY();
}

int zut_substitute_symbol(string pattern, string &result)
{
  SYMBOL_DATA *parms = (SYMBOL_DATA *)__malloc31(sizeof(SYMBOL_DATA));
  if (parms == nullptr)
  {
    return RTNCD_FAILURE;
  }
  memset(parms, 0x00, sizeof(SYMBOL_DATA));

  if (pattern.size() > sizeof(parms->input))
  {
    free(parms);
    return RTNCD_FAILURE;
  }

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

// https://www.ibm.com/docs/en/zos/3.2.0?topic=output-requesting-dynamic-allocation
int zut_bpxwdyn_common(string parm, unsigned int *code, string &resp, string &ddname, string &dsname)
{
  char bpx_response[RET_ARG_MAX_LEN * MSG_ENTRIES + 1] = {0};

  unsigned char *p = (unsigned char *)__malloc31(sizeof(BPXWDYN_PARM) + sizeof(BPXWDYN_RESPONSE));
  if (p == nullptr)
  {
    return RTNCD_FAILURE;
  }
  memset(p, 0x00, sizeof(BPXWDYN_PARM) + sizeof(BPXWDYN_RESPONSE));

  BPXWDYN_PARM *bparm = (BPXWDYN_PARM *)p;
  BPXWDYN_RESPONSE *response = (BPXWDYN_RESPONSE *)(p + sizeof(BPXWDYN_PARM));

  if (ddname == "RTDDN")
  {
    bparm->rtdd = 1; // set bit flag indicating we want to return the DD name
  }
  else if (dsname == "RTDSN")
  {
    bparm->rtdsn = 1; // set bit flag indicating we want to return the DS name
  }

  bparm->len = sprintf(bparm->str, "%s", parm.c_str());
  int rc = ZUTWDYN(bparm, response);

  if (bparm->rtdd)
  {
    ddname = string(response->ddname);
  }
  else if (bparm->rtdsn)
  {
    dsname = string(response->dsname);
  }

  resp = string(response->response);
  *code = response->code;

  free(p);

  return rc;
}

int zut_bpxwdyn(string parm, unsigned int *code, string &resp)
{
  string ddname = "";
  string dsname = "";
  return zut_bpxwdyn_common(parm, code, resp, ddname, dsname);
}

int zut_bpxwdyn_rtdd(string parm, unsigned int *code, string &resp, string &ddname)
{
  ddname = "RTDDN";
  string dsname = "";
  return zut_bpxwdyn_common(parm, code, resp, ddname, dsname);
}

int zut_bpxwdyn_rtdsn(string parm, unsigned int *code, string &resp, string &dsname)
{
  string ddname = "";
  dsname = "RTDSN";
  return zut_bpxwdyn_common(parm, code, resp, ddname, dsname);
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

  rc = __getuserid(user, sizeof(user));
  if (0 != rc)
    return rc;

  struser = string(user);
  zut_rtrim(struser);
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

int zut_list_parmlib(ZDIAG &diag, std::vector<std::string> &parmlibs)
{
  int rc = 0;
  PARMLIB_DSNS dsns = {0};
  int num_dsns = 0;

  rc = ZUTMLPLB(&diag, &num_dsns, &dsns);
  if (0 != rc)
  {
    return rc;
  }

  parmlibs.reserve(num_dsns);
  for (int i = 0; i < num_dsns; i++)
  {
    parmlibs.push_back(string(dsns.dsn[i].val, sizeof(dsns.dsn[i].val)));
  }

  return rc;
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
 * Prints the input string as bytes to the specified output stream.
 * @param input The input string to be printed.
 * @param output_stream Pointer to output stream (nullptr uses std::cout).
 */
void zut_print_string_as_bytes(string &input, std::ostream *out_stream)
{
  std::ostream &output_stream = out_stream ? *out_stream : std::cout;
  char buf[4];
  for (char *p = (char *)input.data(); p < (input.data() + input.length()); p++)
  {
    if (p == (input.data() + input.length() - 1))
    {
      sprintf(buf, "%02x", (unsigned char)*p);
    }
    else
    {
      sprintf(buf, "%02x ", (unsigned char)*p);
    }
    output_stream << buf;
  }
  output_stream << endl;
}

/**
 * Converts a zero-padded hex string to a vector of bytes (e.g. "010203" -> [0x01, 0x02, 0x03]).
 * @param hex_string The hex string to convert.
 * @return A vector of bytes representing the hex string.
 */
vector<uint8_t> zut_get_contents_as_bytes(const string &hex_string)
{
  vector<uint8_t> bytes;
  bytes.reserve(hex_string.length() / 2);

  // If the hex string is not zero-padded, return an empty vector
  if (hex_string.length() % 2 != 0)
  {
    return bytes;
  }

  for (auto i = 0u; i < hex_string.size(); i += 2u)
  {
    const auto byte_str = hex_string.substr(i, 2);
    bytes.push_back(strtoul(byte_str.c_str(), nullptr, 16));
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
 * @param flush_state If true, flush the shift state for stateful encodings (e.g., IBM-939). Set to true on the last chunk.
 *
 * @return return code from `iconv`
 */
size_t zut_iconv(iconv_t cd, ZConvData &data, ZDIAG &diag, bool flush_state)
{
  size_t input_bytes_remaining = data.input_size;
  size_t output_bytes_remaining = data.max_output_size;

  size_t rc = iconv(cd, &data.input, &input_bytes_remaining, &data.output_iter, &output_bytes_remaining);

  // If an error occurred, throw an exception with iconv's return code and errno
  if (-1 == rc)
  {
    diag.e_msg_len = sprintf(diag.e_msg, "[zut_iconv] Error when converting characters. rc=%zu,errno=%d", rc, errno);
    return -1;
  }

  // "If the input conversion is stopped... the value pointed to by inbytesleft will be nonzero and errno is set to indicate the condition"
  if (0 != input_bytes_remaining)
  {
    diag.e_msg_len = sprintf(diag.e_msg, "[zut_iconv] Failed to convert all input bytes. rc=%zu,errno=%d", rc, errno);
    return -1;
  }

  // Flush the shift state for stateful encodings (e.g., IBM-939 with SI/SO sequences)
  if (flush_state)
  {
    size_t flush_rc = iconv(cd, NULL, NULL, &data.output_iter, &output_bytes_remaining);
    if (-1 == flush_rc)
    {
      diag.e_msg_len = sprintf(diag.e_msg, "[zut_iconv] Error flushing shift state. rc=%zu,errno=%d", flush_rc, errno);
      return -1;
    }
  }

  return rc;
}

/**
 * Flushes the shift state for stateful encodings (e.g., IBM-939 with SI/SO sequences).
 * This should be called after all chunks have been processed.
 *
 * @param cd `iconv` conversion descriptor
 * @param diag diagnostic structure to store error information
 *
 * @return vector containing the flushed bytes (empty on error)
 */
vector<char> zut_iconv_flush(iconv_t cd, ZDIAG &diag)
{
  const size_t max_output_size = 16; // Small buffer for shift state flush (SI/SO sequences are typically 1-2 bytes)
  vector<char> output_buffer(max_output_size, 0);
  char *output_iter = &output_buffer[0];
  size_t output_bytes_remaining = max_output_size;

  char *start_pos = output_iter;
  size_t flush_rc = iconv(cd, NULL, NULL, &output_iter, &output_bytes_remaining);
  if (-1 == flush_rc)
  {
    diag.e_msg_len = sprintf(diag.e_msg, "[zut_iconv_flush] Error flushing shift state. rc=%zu,errno=%d", flush_rc, errno);
    return vector<char>(); // Return empty vector on error
  }

  // Resize to actual bytes written
  size_t flush_bytes = output_iter - start_pos;
  output_buffer.resize(flush_bytes);
  return output_buffer;
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
  if (from_encoding == to_encoding)
  {
    return input_str;
  }

  vector<char> result = zut_encode(input_str.data(), input_str.size(), from_encoding, to_encoding, diag);
  return string(result.begin(), result.end());
}

/**
 * Converts the encoding for a string from one codepage to another.
 * @param input_str input data to convert
 * @param input_size size of the input data in bytes
 * @param from_encoding current codepage for the input data
 * @param to_encoding desired codepage for the data
 * @param diag diagnostic structure to store error information
 */
vector<char> zut_encode(const char *input_str, const size_t input_size, const string &from_encoding, const string &to_encoding, ZDIAG &diag)
{
  if (from_encoding == to_encoding)
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

/**
 * Converts the encoding for a string using an existing iconv descriptor.
 * @param input_str input data to convert
 * @param cd iconv descriptor (caller manages opening, flushing, and closing)
 * @param diag diagnostic structure to store error information
 */
string zut_encode(const string &input_str, iconv_t cd, ZDIAG &diag)
{
  vector<char> result = zut_encode(input_str.data(), input_str.size(), cd, diag);
  return string(result.begin(), result.end());
}

/**
 * Converts the encoding for a string using an existing iconv descriptor.
 * @param input_str input data to convert
 * @param input_size size of the input data in bytes
 * @param cd iconv descriptor (caller manages opening, flushing, and closing)
 * @param diag diagnostic structure to store error information
 */
vector<char> zut_encode(const char *input_str, const size_t input_size, iconv_t cd, ZDIAG &diag)
{
  const size_t max_output_size = input_size * 4;
  vector<char> output_buffer(max_output_size, 0);

  char *input = const_cast<char *>(input_str);
  char *output_iter = &output_buffer[0];

  ZConvData data = {input, input_size, max_output_size, &output_buffer[0], output_iter};

  size_t iconv_rc = zut_iconv(cd, data, diag, false);
  if (-1 == iconv_rc)
  {
    throw std::runtime_error(diag.e_msg);
  }

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

// Helper template for integer to string conversion
template <typename T>
static inline string int_to_string_impl(T value, bool is_hex, const char *dec_fmt, const char *hex_fmt)
{
  char buffer[32];
  sprintf(buffer, is_hex ? hex_fmt : dec_fmt, value);
  return string(buffer);
}

string zut_int_to_string(int value, bool is_hex)
{
  return int_to_string_impl(value, is_hex, "%d", "%X");
}

string zut_int_to_string(unsigned int value, bool is_hex)
{
  return int_to_string_impl(value, is_hex, "%u", "%X");
}

string zut_int_to_string(long value, bool is_hex)
{
  return int_to_string_impl(value, is_hex, "%ld", "%lX");
}

string zut_int_to_string(long long value, bool is_hex)
{
  return int_to_string_impl(value, is_hex, "%lld", "%llX");
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

void zut_debug_message(const char *message)
{
  fprintf(stderr, "%s", message);
}

bool zut_string_compare_c(const std::string &a, const std::string &b)
{
  return strcmp(a.c_str(), b.c_str()) < 0;
}

int zut_loop_dynalloc(ZDIAG &diag, vector<string> &list)
{
  int rc = 0;
  unsigned int code = 0;
  string response;

  for (vector<string>::iterator it = list.begin(); it != list.end(); it++)
  {
    rc = zut_bpxwdyn(*it, &code, response);

    if (0 != rc)
    {
      diag.detail_rc = ZUT_RTNCD_SERVICE_FAILURE;
      diag.service_rc = rc;
      strcpy(diag.service_name, "bpxwdyn");
      diag.e_msg_len = sprintf(diag.e_msg, "bpxwdyn failed with '%s' rc: '%d', emsg: '%s'", diag.service_name, rc, (*it).c_str());
      return RTNCD_FAILURE;
    }
  }

  return rc;
}

int zut_free_dynalloc_dds(ZDIAG &diag, vector<string> &list)
{
  vector<string> free_dds;
  free_dds.reserve(list.size());

  for (vector<string>::iterator it = list.begin(); it != list.end(); it++)
  {
    string alloc_dd = *it;
    size_t start = alloc_dd.find(" ");
    size_t end = alloc_dd.find(")", start);
    if (start == string::npos || end == string::npos)
    {
      diag.e_msg_len = sprintf(diag.e_msg, "Invalid format in DD alloc string: %s", (*it).c_str());
      return RTNCD_FAILURE;
    }
    else
    {
      free_dds.push_back("free " + alloc_dd.substr(start + 1, end - start));
    }
  }

  return zut_loop_dynalloc(diag, free_dds);
}

AutocvtGuard::AutocvtGuard(bool enabled)
    : old_state(0)
{
  old_state = __ae_autoconvert_state(enabled ? _CVTSTATE_ON : _CVTSTATE_OFF);
}

AutocvtGuard::~AutocvtGuard()
{
  __ae_autoconvert_state(old_state);
}

FileGuard::FileGuard(const char *filename, const char *mode)
    : fp()
{
  fp = fopen(filename, mode);
}

FileGuard::FileGuard(int fd, const char *mode)
    : fp()
{
  fp = fdopen(fd, mode);
}

FileGuard::~FileGuard()
{
  if (fp)
  {
    fclose(fp);
  }
}

FileGuard::operator FILE *() const
{
  return fp;
}

FileGuard::operator bool() const
{
  return fp != nullptr;
}
