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
  // s0c3_abend(1);
  // __asm(" PR ");
  return 0;
}

#pragma prolog(ZJBMARR, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZJBMARR(SDWA *sdwa)
{
  unsigned long long int r0 = get_prev_r0();
  unsigned long long int r2 = get_prev_r2(); // TODO(Kelosky): this should not be prev

  if (NO_SDWA == r0)
  {
    return RTNCD_PERCOLATE; // TODO(Kelosky): handle no SDWA, for now percolate
  }
  // TODO(Kelosky): check r0 for 12, meaning no SDWA
  zwto_debug("@TEST recovery routine called");
  // __asm(" exrl 0,*");

  zwto_debug("@TEST %llx and %llx", r0, r2);

  return RTNCD_PERCOLATE;
}

int main()
{
  char *data = "hello";
  zwto_debug("@TEST data %x", data);
  zwto_debug("@TEST calling recovery");

  // return 7;

  SAVF4SA f4sa = {0};
  ZRCVY_ENV zenv = {0};
  // unsigned long long int all_regs[16] = {0};

  __asm(" LGHI 5,5 " ::: "r5");
  __asm(" LGHI 6,6 " ::: "r6");
  __asm(" LGHI 7,7 " ::: "r7");

  // unsigned long long int regs[5] = {0};
  // regs[0] = get_r5();
  // regs[1] = get_r6();
  // regs[2] = get_r7();
  // regs[3] = get_r13();

  // zwto_debug("@TEST outside %llx %llx %llx %llx", regs[0], regs[1], regs[2], regs[3]);

  zwto_debug("@TEST from main current %llx", get_r13());

  // __asm(" svc 199 ");
  if (0 == set_env(&f4sa))
  {
    zwto_debug("@TEST in if");
    // s0c3_abend(2);
  }
  else
  {
    zwto_debug("@TEST in else");
  }

  // @TEST @HERE we need to find IEAARR and br14 to it.
  // recovery_drop();

  zwto_debug("@TEST exiting");
  // set_recovery(ZJBMTEST, ZJBMARR, data); // run under recovery
}
