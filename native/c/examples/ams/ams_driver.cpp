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

int main()
{
  int rc = 0;
  std::string dsn = "SYS1.AMBLIST.DATA";

  std::vector<std::string> dds;
  // dds.reserve(3);
  // dds.push_back("alloc dd(syslib) da('" + dsn + "') shr");
  dds.push_back("alloc dd(sysin) da('dkelosky.input.fixed(data)') shr lrecl(80) recfm(f) ");
  dds.push_back("alloc dd(sysprint) da('dkelosky.output.fixed(data)') shr lrecl(80) recfm(f) ");
  // dds.push_back("alloc dd(sysprint) da('dkelosky.output.fixed(data)') shr lrecl(80) recfm(f) ");
  // dds.push_back("alloc dd(sysin) da('dkelosky.input(data)') shr lrecl(80) recfm(f,b) blksize(160) ");
  // dds.push_back("alloc dd(sysprint) da('dkelosky.output(test)') shr"); //  lrecl(80) recfm(f,b) blksize(160) ");
  // dds.push_back("alloc dd(sysin) lrecl(80) recfm(f,b) blksize(80)");

  rc = zut_loop_dynalloc(dds, &std::cerr);
  if (0 != rc)
  {
    std::cout << "Error: could not allocate dds" << std::endl;
    return 1;
  }

  std::cout << "AMS started" << std::endl;
  AMSMAIN();
  std::cout << "AMS ended" << std::endl;

  rc = zut_free_dynalloc_dds(dds, &std::cerr);
  if (0 != rc)
  {
    std::cout << "Error: could not free dds" << std::endl;
    return 1;
  }

  return 0;
}
