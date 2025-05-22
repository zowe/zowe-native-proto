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

#include "zmetal.h"
#include "zwto.h"

int main()
{
  char mod[] = "IEFBR15";
  void *ep = load_module(mod);
  if (ep)
  {
    zwto_debug("@TEST loaded");
  }
  else
  {
    zwto_debug("@TEST not loaded");
  }
  return 5;
}