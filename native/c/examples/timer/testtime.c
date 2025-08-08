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

#pragma prolog(SOMEFUNC, " ZWEPROLG NEWDSA=(YES,8),SAVE=BAKR,SAM64=YES")
#pragma epilog(SOMEFUNC, " ZWEEPILG ")
void SOMEFUNC(void *PTR32 parameter)
{
  // int *p = (int *)parameter;
  zwto_debug("@TEST called some func"); //  %llx why %d", parameter, *((int *)parameter));
  ECB *e1 = (ECB *)parameter;
  ecb_post(e1, 7);
}

int main()
{

  PSW psw = {0};
  get_psw(&psw);
  if (psw.ba)
    zwto_debug("@TEST ba mode");
  if (psw.ea)
    zwto_debug("@TEST ea mode");

  zwto_debug("@TEST main");

  int p = 10;

  ECB e1 = {0};
  // ecb_post(&e1);
  timer(1 * 100, SOMEFUNC, &e1);

  zwto_debug("@TEST waiting on ECB");
  ecb_wait(&e1);
  zwto_debug("@TEST waiting on ECB done, code: %x", e1.ecbcc);
  zwto_debug("@TEST waiting on ECB done, code: %x", e1.ecbbyte3);

  zwto_debug("@TEST waiting for 3 seconds");
  time_wait(1 * 100 * 3);
  zwto_debug("@TEST waiting complete");

  cancel_timers();

  zwto_debug("@TEST exiting");

  return 0;
}
