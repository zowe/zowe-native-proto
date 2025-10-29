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

#include "zams24.h"
#include "zwto.h"

#if defined(__IBM_METAL__)
#define OPENJ(dcb, plist, rc, mode)                           \
  __asm(                                                      \
      "*                                                  \n" \
      " OPEN (%0,(" #mode ")),"                               \
      "TYPE=J,"                                               \
      "MF=(E,%2)                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "+m"(dcb),                                            \
        "=m"(rc)                                              \
      : "m"(plist)                                            \
      : "r0", "r1", "r14", "r15");
#else
#define OPENJ(dcb, plist, rc, mode)
#endif

#pragma prolog(DCBABEND, " ZWEPROLG NEWDSA=(YES,10) ")
#pragma epilog(DCBABEND, " ZWEEPILG ")
void DCBABEND()
{
  // TODO(Kelosky): handle when this is called and ensure in 24 bit storage
  zwto_debug("@test dbcabend");
  s0c3_abend(5);
}

int open_input_j(IO_CTRL *ioc)
{
  int rc = 0;
  memcpy(&ioc->opl, 0x00, sizeof(OPEN_PL));
  ioc->opl.option = OPTION_BYTE;
  OPENJ(ioc->dcb, ioc->opl, rc, INPUT);
  return rc;
}

int open_output_j(IO_CTRL *ioc)
{
  int rc = 0;
  memcpy(&ioc->opl, 0x00, sizeof(OPEN_PL));
  ioc->opl.option = OPTION_BYTE;
  OPENJ(ioc->dcb, ioc->opl, rc, OUTPUT);
  return rc;
}

#pragma prolog(AMS24, " ZWEPROLG NEWDSA=(YES,10) ")
#pragma epilog(AMS24, " ZWEEPILG ")
int AMS24(ZAMS24_FUNCS *funcs)
{
  zwto_debug("AMS24 started");
  funcs->open_input_j = open_input_j;
  funcs->open_output_j = open_output_j;
  return 0;
}
