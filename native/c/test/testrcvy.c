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

#include "zrecovery.h"
#include "zwto.h"

#pragma prolog(ZJBMTEST, "&CCN_MAIN SETB 1 \n MYPROLOG")
int ZJBMTEST()
{
  zwto_debug("@TEST routine called under recovery");
  return 0;
}

int main()
{
  zwto_debug("@TEST calling recovery");
  set_recovery(ZJBMTEST);
}
