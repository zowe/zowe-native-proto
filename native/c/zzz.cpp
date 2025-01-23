/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0 & Apache-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

// int zdsReadDynalloc(string ddname, string dsname, string member, string &data)
// {
//   int rc = 0;
//   string content;

//   __dyn_t ip;
//   rc = dyninit(&ip);
//   if (0 != rc)
//   {
//     cerr << "Error: dyninit failed with " << rc << endl; // TODO(Kelosky): better error handling scheme
//     return -1;
//   }
//   ip.__ddname = (char *)ddname.c_str();
//   ip.__dsname = (char *)dsname.c_str();
//   ip.__member = (char *)member.c_str();
//   ip.__status = __DISP_SHR;

//   rc = dynalloc(&ip);
//   if (0 != rc)
//   {
//     cerr << "Error: dynalloc failed with " << rc << endl; // TODO(Kelosky): better error handling scheme
//     return -1;
//   }

//   char buffer[80] = {0};

//   FILE *fp = fopen(string("DD:" + ddname).c_str(), "r");
//   int len = 0;
//   while ((len = fread(buffer, 1, sizeof(buffer), fp)) > 0)
//   {
//     printf("read %s", buffer);
//   }
//   fclose(fp);

//   rc = dynfree(&ip);
//   if (0 != rc)
//   {
//     cerr << "Error: dynfree failed with " << rc << endl; // TODO(Kelosky): better error handling scheme
//     return -1;
//   }

//   return rc;
// }

// #pragma prolog(ZDSSMSAT, "&CCN_MAIN SETB 1 \n MYPROLOG")
// int ZDSSMSAT(ZDS *zds, const char *dsn)
// {
//   int rc = 0;
//   int rsn = 0;
//   int problem_data[2] = {0};
//   ZSMS_DATA sms_data[3];
//   int dsn_type = 0;
//   int dsn_len = (int)strlen(dsn);

//   // load our service
//   IGWASMS asms = (IGWASMS)load_module31("IGWASMS"); // EP which doesn't require R0 == 0
//   if (!asms)
//   {
//     return RTNCD_FAILURE;
//   }

//   rc = asms(&rc, &rsn, &problem_data[0], &dsn_len, dsn, &sms_data[0], &dsn_type);

//   delete_module("IGWASMS");

//   // https://www.ibm.com/docs/en/zos/3.1.0?topic=attributes-igwasys-igwasms-igwabwo-igwlshr-return-reason-codes#retig__xrrcode
//   if (0 != rc)
//   {
//     strcpy(zds->diag.service_name, "IGWASMS");
//     zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "IGWASMS rc was: '%d', rsn was: '%d'", rc, rsn);
//     zds->diag.detail_rc = ZDS_RTNCD_SERVICE_FAILURE;
//     return RTNCD_FAILURE;
//   }

//   // TODO(Kelosky): finish implementation when useful/needed

//   return rc;
// }
