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

#include "zwto.h"
#include "zmetal.h"
#include "zsetjmp.h"
#include "ztime.h"
#include "zwto.h"
#include "zdbg.h"

#pragma prolog(main, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(main, " ZWEEPILG ")

int main()
{
  unsigned long long tod = 0;
  __asm(" STCK %0" : "=m"(tod));
  zut_dump_storage_wto("tod", &tod, sizeof(tod));

  // demo_setjmp();
  return 0;
}

void test(ZSETJMP_ENV *zenv)
{
  zwto_debug("test called");
  zlongjmp(zenv);
}

int demo_time()
{
  unsigned long long tod = 0;
  time(&tod);
  zwto_debug("time: %llx", tod);
  return 0;
}

int demo_setjmp()
{
  PSW psw = {0};
  get_psw(&psw);
  int mode_switch = psw.data.bits.p ? 0 : 1;

  ZSETJMP_ENV zenv = {0};

  int rc = zsetjmp(&zenv);
  if (rc == 0)
  {
    zwto_debug("zsetjmp returned 0");
    test(&zenv);
    ZLONGJMP(&zenv);
  }
  else
  {
    zwto_debug("zsetjmp returned %d", rc);
  }

  return rc;
}