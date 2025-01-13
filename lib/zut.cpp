/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
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
  int rc = 0;
  unsigned int code = 0;
  string resp;
  string parm = "free dd(minemine)";

  // rc = zut_bpxwdyn(parm, &code, resp);

  /*
 This example dynamically deallocates a data set.
 */

 __dyn_t ip;

 dyninit(&ip);
 ip.__dsname = "dkelosky.temp.test5";

 rc = dynfree(&ip);


  // cout << "resp is:\n" << resp << endl;
  // printf("code is x'%x'\n", code);

  return rc;
}

void zut_uppercase_pad_truncate(string source, char *target, int len)
{
  memset(target, ' ', len); // pad with spaces
  transform(source.begin(), source.end(), source.begin(), ::toupper); // upper case
  int length = source.size() > len ? len : source.size(); // truncate
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
  if (0 != rc) return rc;

  for (int i = sizeof(user) - 1; i >=0; i--)
  {
    if (user[i] == ' ' || user[i] == 0x00) user[i] = 0x00;
    else break;
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
  cout  << buf << " | ";
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
