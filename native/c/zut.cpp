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
#include <string>
#include <iomanip>
#include <algorithm>
#include "zut.hpp"
#include "zutm.h"
#include "zutm31.h"
#include <ios>
#include "zdyn.h"

using namespace std;

int zut_test()
{
  return 0;
}

int zut_convert_dsect()
{
  return ZUTEDSCT();
}

void zut_uppercase_pad_truncate(string source, char *target, int len)
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

void zut_dump_storage(string title, const void *data, size_t size)
{
  ios_base::fmtflags f(cout.flags());
  printf("--- Dumping storage for '%s' at x'%016llx' ---\n", title.c_str(), data);

  unsigned char *ptr = (unsigned char *)data;

#define BYTES_PER_LINE 32

  int index = 0;
  bool end = false;
  char *spaces = "                                ";
  char buf[BYTES_PER_LINE + 1] = {0};

  int lines = size / BYTES_PER_LINE;
  int remainder = size % BYTES_PER_LINE;
  char unknown = '.';

  for (int x = 0; x < lines; x++)
  {
    printf("%016llx", ptr);
    cout << " | ";
    for (int y = 0; y < BYTES_PER_LINE; y++)
    {
      unsigned char p = isprint(ptr[y]) ? ptr[y] : unknown;
      cout << setw(1) << setfill(' ') << p;
    }
    cout << " | ";

    for (int y = 0; y < BYTES_PER_LINE; y++)
    {
      std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ptr[y]);

      if ((y + 1) % 4 == 0)
      {
        std::cout << " ";
      }
      if ((y + 1) % 16 == 0)
      {
        std::cout << "    ";
      }
    }
    cout << endl;
    ptr = ptr + BYTES_PER_LINE;
  }

  printf("%016llx", ptr);
  cout << " | ";
  for (int y = 0; y < remainder; y++)
  {
    unsigned char p = isprint(ptr[y]) ? ptr[y] : unknown;
    cout << setw(1) << setfill(' ') << p;
  }
  memset(buf, 0x00, sizeof(buf));
  sprintf(buf, "%.*s", BYTES_PER_LINE - remainder, spaces);
  cout << buf << " | ";
  for (int y = 0; y < remainder; y++)
  {
    cout << hex << setw(2) << setfill('0') << static_cast<int>(ptr[y]);

    if ((y + 1) % 4 == 0)
    {
      std::cout << " ";
    }
    if ((y + 1) % 16 == 0)
    {
      std::cout << "    ";
    }
  }
  cout << endl;
  cout << "--- END ---" << endl;

  cout.flags(f);
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
  if (!opts)
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
  if (rc == -1)
  {
    diag.e_msg_len = sprintf(diag.e_msg, "[zut_iconv] Error when converting characters. rc=%lu,errno=%d", rc, errno);
    return -1;
  }

  // "If the input conversion is stopped... the value pointed to by inbytesleft will be nonzero and errno is set to indicate the condition"
  if (input_bytes_remaining != 0)
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
std::string zut_encode(const string &input_str, const string &from_encoding, const string &to_encoding, ZDIAG &diag)
{
  iconv_t cd = iconv_open(to_encoding.c_str(), from_encoding.c_str());
  if (cd == (iconv_t)(-1))
  {
    diag.e_msg_len = sprintf(diag.e_msg, "Cannot open converter from %s to %s", from_encoding.c_str(), to_encoding.c_str());
    return "";
  }

  const size_t input_size = input_str.size();
  // maximum possible size assumes UTF-8 data with 4-byte character sequences
  const size_t max_output_size = input_size * 4;

  // Create a contiguous memory region to store the output w/ new encoding
  // There is no guarantee that the memory is contiguous when using an empty std::string here (as xlc does not completely implement the C++11 standard),
  // so we'll handle the memory ourselves
  char *output_buffer = new char[max_output_size];
  std::fill(output_buffer, output_buffer + max_output_size, 0);

  // Prepare iconv parameters (copy output_buffer ptr to output_iter to cache start and end positions)
  char *input = (char *)input_str.data();
  char *output_iter = output_buffer;

  string result;

  ZConvData data = {input, input_size, max_output_size, output_buffer, output_iter};
  size_t iconv_rc = zut_iconv(cd, data, diag);
  iconv_close(cd);
  if (iconv_rc == -1)
  {
    throw std::exception(diag.e_msg);
  }

  // Copy converted input into a new string and return it to the caller
  result.assign(output_buffer, data.output_iter - data.output_buffer);
  delete[] data.output_buffer;

  return result;
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