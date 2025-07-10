
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

#ifndef ZDBG_H
#define ZDBG_H

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include "zutm.h"

typedef int (*zut_print_func)(const char *fmt);

/**
 * @brief Dump a memory region to output for debugging
 * @param label Label for the dump
 * @param addr Pointer to the memory region
 * @param size Size of the memory region in bytes
 * @param cb_print Call back function to print the output
 */
void zut_dump_storage(const char *title, const void *data, int size, zut_print_func cb_print)
{
  int len = 0;
  char buf[1024] = {0};
  len += sprintf(buf + len, "--- Dumping storage for '%s' at x'%016llx' ---\n", title, (unsigned long long)data);
  cb_print(buf);
  memset(buf, 0, sizeof(buf));
  len = 0;

  unsigned char *ptr = (unsigned char *)data;

#define BYTES_PER_LINE 32

  int index = 0;
  char spaces[] = "                                ";
  char fmt_buf[BYTES_PER_LINE + 1] = {0};

  int lines = size / BYTES_PER_LINE;
  int remainder = size % BYTES_PER_LINE;
  char unknown = '.';

  for (int x = 0; x < lines; x++)
  {
    len += sprintf(buf + len, "%016llx", (unsigned long long)ptr);
    len += sprintf(buf + len, " | ");
    for (int y = 0; y < BYTES_PER_LINE; y++)
    {
      unsigned char p = isprint(ptr[y]) ? ptr[y] : unknown;
      len += sprintf(buf + len, "%c", p);
    }
    len += sprintf(buf + len, " | ");

    for (int y = 0; y < BYTES_PER_LINE; y++)
    {
      len += sprintf(buf + len, "%02x", (unsigned char)ptr[y]);

      if ((y + 1) % 4 == 0)
      {
        len += sprintf(buf + len, " ");
      }
      if ((y + 1) % 16 == 0)
      {
        len += sprintf(buf + len, "    ");
      }
    }
    len += sprintf(buf + len, "\n");
    cb_print(buf);
    memset(buf, 0, sizeof(buf));
    len = 0;
    ptr = ptr + BYTES_PER_LINE;
  }

  len += sprintf(buf + len, "%016llx", (unsigned long long)ptr);
  len += sprintf(buf + len, " | ");
  for (int y = 0; y < remainder; y++)
  {
    unsigned char p = isprint(ptr[y]) ? ptr[y] : unknown;
    len += sprintf(buf + len, "%c", p);
  }
  memset(fmt_buf, 0x00, sizeof(fmt_buf));
  sprintf(fmt_buf, "%.*s | ", BYTES_PER_LINE - remainder, spaces);
  len += sprintf(buf + len, "%s", fmt_buf);
  for (int y = 0; y < remainder; y++)
  {
    len += sprintf(buf + len, "%02x", (unsigned char)ptr[y]);

    if ((y + 1) % 4 == 0)
    {
      len += sprintf(buf + len, " ");
    }
    if ((y + 1) % 16 == 0)
    {
      len += sprintf(buf + len, "    ");
    }
  }
  len += sprintf(buf + len, "\n");
  cb_print(buf);
  memset(buf, 0, sizeof(buf));
  len = 0;

  len += sprintf(buf + len, "--- END ---\n");
  cb_print(buf);
  memset(buf, 0, sizeof(buf));
  len = 0;
}

#endif
