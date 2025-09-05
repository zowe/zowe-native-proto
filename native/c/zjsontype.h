
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

#ifndef ZJSYTYPE_H
#define ZJSYTYPE_H

#include "ztype.h"

#ifdef TARGET_OS_MAC
#define HWTJ_SEARCHTYPE_SHALLOW 3
#define HWTJ_OBJECT_TYPE 1
#define HWTJ_ARRAY_TYPE 2
#define HWTJ_STRING_TYPE 3
#define HWTJ_NUMBER_TYPE 4
#define HWTJ_BOOLEAN_TYPE 5
#define HWTJ_NULL_TYPE 6
#define HWTJ_TRUE 1
#define HWTJ_FALSE 0
#define HWTJ_BUFFER_TOO_SMALL 8
#endif

typedef struct
{
  char x[12];
} PARSE_HANDLE;

typedef struct
{
  int x;
} KEY_HANDLE;

typedef struct
{
  char msg[132];
} DIAG;

typedef struct
{
  PARSE_HANDLE handle;
  DIAG diag;
  char *PTR64 json;
  int json_length;
} JSON_INSTANCE;

#endif