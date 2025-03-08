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

// TODO(Kelosky): more testing on this approach
#pragma prolog(ZJBMTEST, "&CCN_MAIN SETB 1 \n MYPROLOG")
#pragma epilog(ZJBMTEST, "&CCN_MAIN SETB 1 \n MYEPILOG")
int ZJBMTEST()
{
  zwto_debug("@TEST routine called under recovery");
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
  ZRCVY_ENV zenv = {0};

  zwto_debug("@TEST main");

  if (0 == set_env(&zenv))
  {
    zwto_debug("@TEST in if");
    // s0c3_abend(2);
  }
  else
  {
    zwto_debug("@TEST in else");
  }
  zwto_debug("@TEST outside of if/else");

  recovery_drop(&zenv);

  zwto_debug("@TEST exiting");

  return 0;
}
