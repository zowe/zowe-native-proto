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

#include <iostream>
#include <string>
#include <vector>
#include "ztype.h"
#include <unistd.h>
#include "zds.hpp"
#include "zut.hpp"

// TODO(Kelosky): test on archived data set
int main()
{
  int rc = 0;
  unsigned int code = 0;

  std::string dsname = "DKELOSKY.JCL";
  std::string alloc_cmd = "ALLOC DA('" + dsname + "') SHR";
  std::string resp = "";
  std::string ddname = "";

  rc = zut_bpxwdyn_rtdd(alloc_cmd, &code, resp, ddname);
  std::cout << "ddname: '" << ddname << "'" << std::endl;
  std::cout << "resp: '" << resp << "'" << std::endl;
  std::cout << "code: '" << code << "'" << std::endl;
  std::cout << "rc: '" << rc << "'" << std::endl;

  std::cout << "--------------------------------" << std::endl;
  alloc_cmd = "ALLOC SYSOUT";
  rc = zut_bpxwdyn_rtdsn(alloc_cmd, &code, resp, dsname);
  std::cout << "dsname: '" << dsname << "'" << std::endl;
  std::cout << "resp: '" << resp << "'" << std::endl;
  std::cout << "code: '" << code << "'" << std::endl;
  std::cout << "rc: '" << rc << "'" << std::endl;

  std::cout << "--------------------------------" << std::endl;
  alloc_cmd = "ALLOC FI(SYSIN) DA('" + std::string("SYS1.MACLIB") + "') SHR";
  ddname = "";
  rc = zut_bpxwdyn(alloc_cmd, &code, resp);
  std::cout << "ddname: '" << ddname << "'" << std::endl;
  std::cout << "resp: '" << resp << "'" << std::endl;
  std::cout << "code: '" << code << "'" << std::endl;
  std::cout << "rc: '" << rc << "'" << std::endl;

  return 0;
}
