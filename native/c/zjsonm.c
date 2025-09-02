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

#include <hwtjic.h> // ensure to include /usr/include
#include "zjsonm31.h"
#include "zjsonm.h"
#include "ztype.h"
#include "zjsontype.h"
#include "zstorage.h"
#include "zwto.h"

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjinit-initialize-instance
#pragma prolog(ZJSMINIT, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMINIT, " ZWEEPILG ")
int ZJSMINIT(JSON_INSTANCE *PTR64 instance)
{
  int rc = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = ZJSMINIT31(&instance31);

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgenc-get-json-encoding
#pragma prolog(ZJSMGENC, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGENC, " ZWEEPILG ")
int ZJSMGENC(JSON_INSTANCE *PTR64 instance, int *PTR64 encoding)
{
  int rc = 0;
  int encoding31 = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = ZJSMGENC31(&instance31, &encoding31);

  *encoding = encoding31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjpars-parse-json-string
#pragma prolog(ZJSMPARS, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMPARS, " ZWEEPILG ")
int ZJSMPARS(JSON_INSTANCE *PTR64 instance, const char *PTR64 json)
{
  int rc = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));
  zwto_debug("@TEST instance handle: %d", instance31.handle);

  int json_length = (int)strlen(json) + 1;
  char *PTR32 json31 = storage_obtain31(json_length);

  memcpy(json31, json, strlen(json));
  json31[json_length - 1] = '\0';

  rc = ZJSMPARS31(&instance31, json31);

  storage_release(json_length, json31);
  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjsrch-search
#pragma prolog(ZJSMSRCH, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMSRCH, " ZWEEPILG ")
int ZJSMSRCH(JSON_INSTANCE *PTR64 instance, const char *PTR64 key, KEY_HANDLE *PTR64 key_handle)
{
  int rc = 0;

  KEY_HANDLE key_handle31 = {0};

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  int length = (int)strlen(key) + 1;
  char *PTR32 key31 = storage_obtain31(length);

  memcpy(key31, key, strlen(key));
  key31[length - 1] = '\0';

  zwto_debug("@TEST key: '%s'", key31);
  zwto_debug("@TEST instance handle: %d", instance31.handle);
  zwto_debug("@TEST key length: %d", length);

  char *PTR32 name = "name";
  rc = ZJSMSRCH31(&instance31, name, &key_handle31);
  zwto_debug("@TEST rc: %d", rc);

  memcpy(key_handle, &key_handle31, sizeof(KEY_HANDLE));

  storage_release(length, key31);
  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgjst-get-type
#pragma prolog(ZJSNGJST, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSNGJST, " ZWEEPILG ")
int ZJSNGJST(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 type)
{
  return 0;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgval-get-value
#pragma prolog(ZJSMGVAL, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGVAL, " ZWEEPILG ")
int ZJSMGVAL(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, char *PTR64 *PTR64 value, int *PTR64 value_length)
{
  return 0;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgnue-get-number-entries
#pragma prolog(ZJSMGNUE, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGNUE, " ZWEEPILG ")
int ZJSMGNUE(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 number_entries)
{
  return 0;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgbov-get-boolean-value
#pragma prolog(ZJSMGBOV, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGBOV, " ZWEEPILG ")
int ZJSMGBOV(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, char *PTR64 value)
{
  return 0;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgaen-get-array-element
#pragma prolog(ZJSMGAEN, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGAEN, " ZWEEPILG ")
int ZJSMGAEN(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 index, KEY_HANDLE *PTR64 value)
{
  return 0;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjterm-terminate-instance
#pragma prolog(ZJSMTERM, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMTERM, " ZWEEPILG ")
int ZJSMTERM(JSON_INSTANCE *PTR64 instance)
{
  int rc = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = ZJSMTERM31(&instance31);

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgoen-get-object-element
#pragma prolog(ZJSMGOEN, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGOEN, " ZWEEPILG ")
int ZJSMGOEN(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 index, char *PTR64 *PTR64 value, int *PTR64 value_length, KEY_HANDLE *PTR64 value_handle, int *PTR64 actual_length)
{
  return 0;
}
