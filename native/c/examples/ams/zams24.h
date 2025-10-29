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
#ifndef ZAMS24_H
#define ZAMS24_H

#include "zamstypes.h"

typedef int (*PTR32 open_input_j_fn)(IO_CTRL *ioc);
typedef int (*PTR32 open_output_j_fn)(IO_CTRL *ioc);
typedef struct
{
  open_input_j_fn open_input_j;
  open_output_j_fn open_output_j;
} ZAMS24_FUNCS;

typedef int (*PTR32 AMS24_fn)(ZAMS24_FUNCS *funcs);
int AMS24(ZAMS24_FUNCS *funcs);

#endif