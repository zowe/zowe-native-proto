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
#include "zecb.h"

#pragma prolog(ABEXIT, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(ABEXIT, " ZWEEPILG ")
void ABEXIT(SDWA *sdwa, void *abexit_data)
{
  zwto_debug("@TEST called on abend");
}

#pragma prolog(PERCEXIT, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(PERCEXIT, " ZWEEPILG ")
void PERCEXIT(void *perc_exit_data)
{
  zwto_debug("@TEST called to percolate");
}

#pragma prolog(SOMEFUNC, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(SOMEFUNC, " ZWEEPILG ")
int SOMEFUNC()
{
  zwto_debug("@TEST called some func");
  return 0;
}

int main()
{
  ZRCVY_ENV zenv = {0};
  zenv.abexit = ABEXIT;
  zenv.perc_exit = PERCEXIT;

  int val = SOMEFUNC();

  zwto_debug("@TEST main");

  ECB e1 = {0};
  ecb_post(&e1);
  ecb_wait(&e1);

  // timer(1 * 100, SOMEFUNC, NULL);

  // zwto_debug("@TEST waiting for 3 seconds");
  // time_wait(1 * 100 * 3);
  // zwto_debug("@TEST waiting complete");

  timer(1 * 100, SOMEFUNC, NULL);

  zwto_debug("@TEST waiting for 3 seconds");
  time_wait(1 * 100 * 3);
  // cancel_timers();
  zwto_debug("@TEST waiting complete");

  if (0 == enable_recovery(&zenv))
  {
    zwto_debug("@TEST in if");
    // s0c3_abend(2);
  }
  else
  {
    zwto_debug("@TEST in else");
    // s0c3_abend(2);
  }
  zwto_debug("@TEST outside of if/else");

  disable_recovery(&zenv);

  zwto_debug("@TEST exiting");

  return 0;
}
