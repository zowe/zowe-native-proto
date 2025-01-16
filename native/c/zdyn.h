/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#ifndef ZDYN_H
#define ZDYN_H

#include <stdio.h>
#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
#include <dynit.h>
#else

static int zdummy(...)
{
  return 0;
}

typedef int __dyn_t;
#define dyninit(...) zdummy(__VA_ARGS__)
#define dynfree(...) zdummy(__VA_ARGS__)
#define dynalloc(...) zdummy(__VA_ARGS__)
#endif

#include "iefzb4d0.h"

#define DISP_OLD 0x01
#define DISP_MOD 0x02
#define DISP_NEW 0x04
#define DISP_SHR 0x08

typedef struct s99rb S99RB;
typedef struct s99tunit S99TUNIT;

struct s99tunit_x
{
  S99TUNIT s99tunit;
  unsigned char overflow[49]; // TOOD(Kelosky): dynamic size
};
typedef struct s99tunit_x S99TUNIT_X;
struct s99tunit_xl
{
  S99TUNIT_X s99tunit_x;
  unsigned char overflow[250];
};
typedef struct s99tunit_xl S99TUNIT_XL;
typedef struct s99tupl S99TUPL;

#endif