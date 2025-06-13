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
#include "zrecovery.metal.test.h"
#include "zrecovery.h"

#pragma prolog(ZRCVYEN, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZRCVYEN, " ZWEEPILG ")
int ZRCVYEN()
{
  ZRCVY_ENV zenv = {0};
  if (0 == enable_recovery(&zenv))
  {
    s0c3_abend(0);
  }
  else
  {
  }
  zwto_debug("@TEST outside of if/else");

  disable_recovery(&zenv);
  return 0;
}
