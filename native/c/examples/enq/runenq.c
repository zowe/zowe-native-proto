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
#include "zdbg.h"
#include "zecb.h"

#pragma prolog(main, " ZWEPROLG NEWDSA=(YES,24) ")
#pragma epilog(main, " ZWEEPILG ")

int main()
{
  zwto_debug("main called");

  WTO_BUF buf = {0};
  buf.len = sprintf(buf.msg, "TESTQ");

  WTOR_REPLY_BUF reply = {0};

  ECB ecb = {0};

  PSW psw = {0};
  get_psw(&psw);

  QNAME qname = {0};
  RNAME rname = {0};

  strcpy(qname.value, "KELOSKY$");
  rname.rlen = sprintf(rname.value, "ABC123HAPPY");

  zwto_debug("qname: %s and length: %d", qname.value, rname.rlen);

  int rc = enq(&qname, &rname);
  zwto_debug("enq returned %d", rc);

  // rc = wtor(&buf, &reply, &ecb);
  zwto_debug("wtor returned %d", rc);
  // ecb_wait(&ecb);
  zwto_debug("wtor reply: %s", reply.msg);

  zwto_debug("@TEST qname before deq: %s and rname: %s and length: %d", qname.value, rname.value, rname.rlen);

  rc = deq(&qname, &rname);
  zwto_debug("deq returned %d", rc);

  zwto_debug("main returned");

  return 0;
}
