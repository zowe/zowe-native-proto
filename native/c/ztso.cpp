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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <env.h>
#include <iconv.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#include <ceaytsor.h>
#include <ceaxrdef.h>

using namespace std;

#define _XOPEN_SOURCE
#define _POSIX1_SOURCE 2

zts_test()
{

  CEATsoRequestStruct_t ceatso_request = {0};
  CEATsoQueryStruct_t ceatso_query = {0};
  CEATsoError_t ceatso_error = {0};

  strcpy(ceatso_request.ceatso_eyecatcher, CEATSOREQUEST_EYECATCHER);
  ceatso_request.ceatso_version = CEATSOREQUEST_CURRENTVERSION;
  ceatso_request.ceatso_requesttype = 0;

  /*
    ceatso_request.ceatso_asid  =  0;
  */

  strcpy(ceatso_request.ceatso_userid, "DKELOSKY");
  strcpy(ceatso_request.ceatso_logonproc, "IZUFPROC");
  memset(&ceatso_request.ceatso_command, ' ', 80);

  /*
    ceatso_request.ceatso_numqueryreq  = 12;
    ceatso_request.ceatso_numqueryrslt = 12;
    ceatso_request.ceatso_duration     =  0;
    ceatso_request.ceatso_msgqueueid   =  0;
  */

  ceatso_request.ceatso_charset = 697;
  ceatso_request.ceatso_codepage = 1047;
  ceatso_request.ceatso_screenrows = 24;
  ceatso_request.ceatso_screencols = 80;
  // memset(ceatso_request.ceatso_account, '0', 40);
  strcpy(ceatso_request.ceatso_account, "IZUACCT#                                ");
  memset(ceatso_request.ceatso_group, ' ', 8);
  strcpy(ceatso_request.ceatso_region, "2000000");

  /*
    memset(ceatso_request.ceatso_instance, ' ', 1);
  */

  strcpy(ceatso_request.ceatso_apptag, "IZUIS   ");
  ceatso_request.ceatso_flags = CEATSO_ABLOGOFF;

  /*
    memset(ceatso_request.ceatso_stoken, 0xFF, 8);
    ceatso_request.ceatso_ascbaddr  =  0;
    ceatso_request.ceatso_index  =  0;
  */

  // query
  strcpy(ceatso_query.ceatsoq_eyecatcher, CEATSOQUERY_EYECATCHER);
  memset(&ceatso_request.ceatso_command, ' ', 40);

  // error
  strcpy(ceatso_error.eyeCatcher, CEATSOERROR_EYECATCHER);
  ceatso_error.version = CEATSOREQUEST_CURRENTVERSION;

  // request
  ceatso_request.ceatso_requesttype = CeaTsoStart;

  CEATsoRequest(&ceatso_request, &ceatso_query, &ceatso_error);

  if (ceatso_error.returnCode == CEASUCCESS &&
      ceatso_error.reasonCode == 0 &&
      ceatso_error.diag.diag1 == 0 &&
      ceatso_error.diag.diag2 == 0 &&
      ceatso_error.diag.diag3 == 0 &&
      ceatso_error.diag.diag4 == 0)
    // printf("  Verifying logon messages.\n\n");
    cout << "wow started" << endl;

  else
  {
    cout << "Starting TSO Failed: " << endl;
    cout << ceatso_error.returnCode << endl;
    printf("%x\n", ceatso_error.reasonCode);
    // cout << ceatso_error.reasonCode << endl;
    cout << ceatso_error.diag.diag1 << endl;
    cout << ceatso_error.diag.diag2 << endl;
    cout << ceatso_error.diag.diag3 << endl;
    cout << ceatso_error.diag.diag4 << endl;
    // error_counter = error_counter + 1;
    // printf("CEATsoRequest( )  Start session failed.\n\n\n");
    // print_error_struct();
    // print_request_struct();
    // printf("\nVariation  %d  failed.\n\n\n", variation_id);
    // printf("\n\n");
    return -1;
  }

  return 3;
}