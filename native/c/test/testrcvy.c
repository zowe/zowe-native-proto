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

#include "ihasdwa.h"
#include "zmetal.h"
#include "zrecovery.h"
#include "zwto.h"

#pragma prolog(ZJBMTEST, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZJBMTEST()
{
  zwto_debug("@TEST routine called under recovery");
  s0c3_abend(1);
  return 0;
}

#pragma prolog(ZJBMARR, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZJBMARR(SDWA *sdwa)
{
  // TODO(Kelosky): check r0 for 12, meaning no SDWA
  zwto_debug("@TEST recovery routine called");
  unsigned long long int reg = get_prev_r2();
  // __asm(" exrl 0,*");

  zwto_debug("@TEST %llx", reg);

  return 4;
}

int main()
{
  char *data = "hello";
  zwto_debug("@TEST data %x", data);
  zwto_debug("@TEST calling recovery");
  set_recovery(ZJBMTEST, ZJBMARR, data);
}
