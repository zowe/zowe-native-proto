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

#ifndef ZPRMTYPE_H
#define ZPRMTYPE_H

#define MAX_PARMLIB_DSN_LEN 44
#define MAX_PARMLIB_DSNS 11

typedef struct
{
  char val[MAX_PARMLIB_DSN_LEN];
} PARMLIB_DSN;

typedef struct
{
  PARMLIB_DSN dsn[MAX_PARMLIB_DSNS];
} PARMLIB_DSNS;

#endif
