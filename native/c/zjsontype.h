
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