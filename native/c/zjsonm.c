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

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjsenc-get-json-encoding
#pragma prolog(ZJSMSENC, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMSENC, " ZWEEPILG ")
int ZJSMSENC(JSON_INSTANCE *PTR64 instance, int *PTR64 encoding)
{
  return 0;

  int rc = 0;
  int encoding31 = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  encoding31 = *encoding;

  rc = ZJSMSENC31(&instance31, &encoding31);

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

  instance->json_length = (int)strlen(json) + 1;
  instance->json = storage_obtain31(instance->json_length);

  memcpy(instance->json, json, instance->json_length - 1);
  instance->json[instance->json_length - 1] = '\0';

  rc = ZJSMPARS31(&instance31, (const char *PTR32)instance->json);

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

  rc = ZJSMSRCH31(&instance31, key31, &key_handle31);

  memcpy(key_handle, &key_handle31, sizeof(KEY_HANDLE));

  storage_release(length, key31);
  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjseri-serialize-json
#pragma prolog(ZJSMSERI, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMSERI, " ZWEEPILG ")
int ZJSMSERI(JSON_INSTANCE *PTR64 instance, char *PTR64 buffer, int *PTR64 buffer_length, int *PTR64 buffer_length_actual)
{
  int rc = 0;
  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  int buffer_length31 = *buffer_length;
  char *PTR32 buffer31 = storage_obtain31(buffer_length31);
  int buffer_length_actual31 = 0;

  rc = ZJSMSERI31(&instance31, buffer31, &buffer_length31, &buffer_length_actual31);

  *buffer_length_actual = buffer_length_actual31;
  memcpy(buffer, buffer31, buffer_length31);

  storage_release(buffer_length31, buffer31);

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgjst-get-type
#pragma prolog(ZJSNGJST, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSNGJST, " ZWEEPILG ")
int ZJSNGJST(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 type)
{
  int rc = 0;
  KEY_HANDLE key_handle31 = {0};
  memcpy(&key_handle31, key_handle, sizeof(KEY_HANDLE));
  int type31 = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = ZJSNGJST31(&instance31, &key_handle31, &type31);

  *type = type31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgval-get-value
#pragma prolog(ZJSMGVAL, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGVAL, " ZWEEPILG ")
int ZJSMGVAL(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, char *PTR64 *PTR64 value, int *PTR64 value_length)
{
  int rc = 0;

  KEY_HANDLE key_handle31 = {0};
  memcpy(&key_handle31, key_handle, sizeof(KEY_HANDLE));
  char *PTR32 value31 = NULL;
  int value_length31 = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = ZJSMGVAL31(&instance31, &key_handle31, &value31, &value_length31);

  *value = value31;
  *value_length = value_length31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgnue-get-number-entries
#pragma prolog(ZJSMGNUE, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGNUE, " ZWEEPILG ")
int ZJSMGNUE(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 number_entries)
{
  int rc = 0;

  int number_entries31 = 0;

  KEY_HANDLE key_handle31 = {0};
  memcpy(&key_handle31, key_handle, sizeof(KEY_HANDLE));

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = ZJSMGNUE31(&instance31, &key_handle31, &number_entries31);

  *number_entries = number_entries31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgbov-get-boolean-value
#pragma prolog(ZJSMGBOV, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGBOV, " ZWEEPILG ")
int ZJSMGBOV(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, char *PTR64 value)
{
  int rc = 0;

  KEY_HANDLE key_handle31 = {0};
  memcpy(&key_handle31, key_handle, sizeof(KEY_HANDLE));

  char value31 = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = ZJSMGBOV31(&instance31, &key_handle31, &value31);
  *value = value31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
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

  if (instance->json)
  {
    storage_release(instance->json_length, instance->json);
    instance->json = NULL;
    instance->json_length = 0;
  }

  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjgoen-get-object-element
#pragma prolog(ZJSMGOEN, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGOEN, " ZWEEPILG ")
int ZJSMGOEN(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 index, char *PTR64 *PTR64 value, int *PTR64 value_length, KEY_HANDLE *PTR64 value_handle, int *PTR64 actual_length)
{
  return 0;
}
