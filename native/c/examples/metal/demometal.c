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

#pragma prolog(main, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(main, " ZWEEPILG ")

void test()
{
  zwto_debug("test called");
}

int main()
{
  PSW psw = {0};
  get_psw(&psw);
  int mode_switch = psw.p ? 0 : 1;

  return 0;
}
