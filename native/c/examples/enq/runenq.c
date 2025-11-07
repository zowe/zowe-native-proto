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
#include "zenq.h"

#pragma prolog(main, " ZWEPROLG NEWDSA=(YES,24) ")
#pragma epilog(main, " ZWEEPILG ")

int main()
{
  zwto_debug("main called");

  PSW psw = {0};
  get_psw(&psw);

  // QNAME qname = {0};
  // RNAME rname = {0};

  // int rc = enq(&qname, &rname);
  // zwto_debug("enq returned %d", rc);

  // rc = deq(&qname, &rname);
  // zwto_debug("deq returned %d", rc);

  zwto_debug("main returned");

  return 0;
}
