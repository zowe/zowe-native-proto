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
#include "zut.hpp"
#include "ams.h"
#include <unistd.h>

// const char *data80 =
// "";

int main()
{
  int rc = 0;
  unsigned int code = 0;

  std::vector<std::string> dds;

  // dds.push_back("alloc dd(sysin) da('DKELOSKY.IO.O.ERROR(new)') shr lrecl(80) recfm(f) ");
  // dds.push_back("alloc dd(example2) da('DKELOSKY.IO.O.VB256(data)') shr lrecl(80) recfm(f) ");
  dds.push_back("alloc dd(sysin) da('DKELOSKY.IO.I.F80(data)') shr lrecl(80) recfm(f) ");
  // dds.push_back("alloc dd(sysin) da('DKELOSKY.IO.O.PS') shr lrecl(80) recfm(f) ");
  // dds.push_back("concat ddlist(sysin,sysin2)");

  // dds.push_back("alloc dd(sysprint) da('DKELOSKY.IO.O.F80(data)') shr lrecl(80) recfm(f) ");
  // dds.push_back("alloc dd(sysprint) da('DKELOSKY.IO.O.FB80(data)') shr lrecl(80) recfm(f,b)");
  // dds.push_back("alloc dd(tep) da('sys1.maclib') shr lrecl(80) recfm(f,b)");
  // dds.push_back("alloc dd(another) da('DKELOSKY.IO.O.PS') shr lrecl(80)");
  // dds.push_back("concat ddlist(sysprint,another)");
  // dds.push_back("alloc dd(sysprint) da('DKELOSKY.IO.O.ERROR(NEW)') shr lrecl(80) recfm(f,b) ");
  // dds.push_back("alloc dd(sysprint) da('DKELOSKY.IO.O.PS','DKELOSKY.IO.O.FB80(data)') shr ");
  // dds.push_back("alloc dd(sysprint) da('DKELOSKY.IO.O.FB80(data)' 'DKELOSKY.IO.O.PS') shr lrecl(80) recfm(f,b) ");
  // dds.push_back("alloc dd(sysprint) da('DKELOSKY.IO.O.FB80(data)') shr lrecl(80) blksize(160) recfm(f,b) ");

  // dds.push_back("alloc dd(sysprint) da('dkelosky.output.fixed(data)') shr lrecl(80) recfm(f) ");
  // dds.push_back("alloc dd(sysin) da('dkelosky.input(data)') shr lrecl(80) recfm(f,b) blksize(160) ");
  // dds.push_back("alloc dd(sysprint) da('dkelosky.output(test)') shr"); //  lrecl(80) recfm(f,b) blksize(160) ");
  // dds.push_back("alloc dd(sysin) lrecl(80) recfm(f,b) blksize(80)");

  std::cout << "current user: " << getlogin() << std::endl;

  ZDIAG diag = {0};
  rc = zut_loop_dynalloc(diag, dds);
  if (0 != rc)
  {
    std::cout << diag.e_msg << std::endl;
    return 1;
  }

  std::string response;
  // std::string cmd = "alloc dd(happy) da('DKELOSKY.IO.O.VB256(data)') shr lrecl(80) recfm(f,b)";

  std::string dsname = "da('DKELOSKY.IO.O.FB80(data)')";
  std::string cmd = "alloc " + dsname + " shr lrecl(80) recfm(f,b)";
  // std::string cmd = "alloc da('DKELOSKY.NOT.FOUND') shr lrecl(80) recfm(f,b)";
  // std::string cmd = "alloc da('DKELOSKY.IO.O.VB256(data)') shr lrecl(80) recfm(f,b)";
  // std::string cmd = "alloc RTDDN(NAME) da('DKELOSKY.IO.O.VB256(data)') shr lrecl(80) recfm(f,b)";
  // std::string alloc = "alloc dd(sysprint) da('DKELOSKY.IO.O.VB256(data)') shr  ";
  std::string ddname = "        ";
  rc = zut_bpxwdyn(cmd, &code, response, ddname);
  if (0 != rc)
  {
    std::cout << "alloc failed: " << response << std::endl;
    std::cout << "code: " << std::hex << code << std::dec << std::endl;
    std::cout << "rc: " << rc << std::endl;
    return -1;
  }

  std::cout << "ddname: " << ddname << std::endl;

  std::cout << "AMS started" << std::endl;
  // AMSMAIN();
  std::cout << "AMS ended" << std::endl;

  std::string full = std::string("alloc dd(" + ddname + ") " + dsname + " shr lrecl(80) recfm(f,b)");
  // dds.push_back(full);

  rc = zut_bpxwdyn("free fi(" + ddname + ")", &code, response);
  if (0 != rc)
  {
    std::cout << "free failed: " << response << std::endl;
    std::cout << "code: " << std::hex << code << std::dec << std::endl;
    std::cout << "rc: " << rc << std::endl;
    // return -1;
  }

  rc = zut_free_dynalloc_dds(diag, dds);
  if (0 != rc)
  {
    std::cout << diag.e_msg << std::endl;
    // return 1;
  }

  return 0;
}
