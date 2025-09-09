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

#include "zjsonm31.h"
#include "zjsonm.h"
#include "ztype.h"
#include "zjsontype.h"
#include "zstorage.h"

#pragma prolog(ZJSMINIT, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMINIT, " ZWEEPILG ")
int ZJSMINIT(JSON_INSTANCE *PTR64 instance)
{
  int rc = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = zjsm_init(&instance31);

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

#pragma prolog(ZJSMGENC, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGENC, " ZWEEPILG ")
int ZJSMGENC(JSON_INSTANCE *PTR64 instance, int *PTR64 encoding)
{
  int rc = 0;
  int encoding31 = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = zjsm_get_encoding(&instance31, &encoding31);

  *encoding = encoding31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

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

  rc = zjsm_set_encoding(&instance31, &encoding31);

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

#pragma prolog(ZJSMDEL, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMDEL, " ZWEEPILG ")
int ZJSMDEL(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, KEY_HANDLE *PTR64 value_handle)
{
  int rc = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  KEY_HANDLE key_handle31 = {0};
  memcpy(&key_handle31, key_handle, sizeof(KEY_HANDLE));
  KEY_HANDLE value_handle31 = {0};
  memcpy(&value_handle31, value_handle, sizeof(KEY_HANDLE));

  rc = zjsm_delete(&instance31, &key_handle31, &value_handle31);

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

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

  rc = zjsm_parse(&instance31, (const char *PTR32)instance->json);

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

#pragma prolog(ZJSMSRCH, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMSRCH, " ZWEEPILG ")
int ZJSMSRCH(JSON_INSTANCE *PTR64 instance, int *PTR64 type, const char *PTR64 key, KEY_HANDLE *PTR64 object_handle, KEY_HANDLE *PTR64 starting_handle, KEY_HANDLE *PTR64 key_handle)
{
  int rc = 0;

  KEY_HANDLE key_handle31 = {0};
  KEY_HANDLE object_handle31 = {0};
  KEY_HANDLE starting_handle31 = {0};

  memcpy(&object_handle31, object_handle, sizeof(KEY_HANDLE));
  memcpy(&starting_handle31, starting_handle, sizeof(KEY_HANDLE));

  int type31 = *type;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  int length = (int)strlen(key) + 1;
  char *PTR32 key31 = storage_obtain31(length);

  memcpy(key31, key, strlen(key));
  key31[length - 1] = '\0';

  rc = zjsm_search(&instance31, &type31, key31, &object_handle31, &starting_handle31, &key_handle31);

  memcpy(key_handle, &key_handle31, sizeof(KEY_HANDLE));

  storage_release(length, key31);
  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

#pragma prolog(ZJSMSSRC, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMSSRC, " ZWEEPILG ")
int ZJSMSSRC(JSON_INSTANCE *PTR64 instance, const char *PTR64 key, KEY_HANDLE *PTR64 key_handle)
{
  int rc = 0;

  KEY_HANDLE key_handle31 = {0};

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  int length = (int)strlen(key) + 1;
  char *PTR32 key31 = storage_obtain31(length);

  memcpy(key31, key, strlen(key));
  key31[length - 1] = '\0';

  rc = zjsm_shallow_search(&instance31, key31, &key_handle31);

  memcpy(key_handle, &key_handle31, sizeof(KEY_HANDLE));

  storage_release(length, key31);
  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

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

  rc = zjsm_serialize(&instance31, buffer31, &buffer_length31, &buffer_length_actual31);

  *buffer_length_actual = buffer_length_actual31;
  memcpy(buffer, buffer31, buffer_length31);

  storage_release(buffer_length31, buffer31);

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

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

  rc = zjsm_get_type(&instance31, &key_handle31, &type31);

  *type = type31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

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

  rc = zjsm_get_string_value(&instance31, &key_handle31, &value31, &value_length31);

  *value = value31;
  *value_length = value_length31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

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

  rc = zjsm_get_number_of_entries(&instance31, &key_handle31, &number_entries31);

  *number_entries = number_entries31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

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

  rc = zjsm_get_boolean_value(&instance31, &key_handle31, &value31);
  *value = value31;

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

#pragma prolog(ZJSMGAEN, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGAEN, " ZWEEPILG ")
int ZJSMGAEN(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 index, KEY_HANDLE *PTR64 value)
{

  int rc = 0;

  KEY_HANDLE key_handle31 = {0};
  memcpy(&key_handle31, key_handle, sizeof(KEY_HANDLE));

  int index31 = *index;
  KEY_HANDLE value31 = {0};

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = zjsm_get_array_entry(&instance31, &key_handle31, &index31, &value31);

  memcpy(value, &value31, sizeof(KEY_HANDLE));

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

#pragma prolog(ZJSMGOEN, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMGOEN, " ZWEEPILG ")
int ZJSMGOEN(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 index, char *PTR64 *PTR64 key_buffer, int *PTR64 key_buffer_length, KEY_HANDLE *PTR64 value_handle, int *PTR64 actual_length)
{
  int rc = 0;

  KEY_HANDLE key_handle31 = {0};
  memcpy(&key_handle31, key_handle, sizeof(KEY_HANDLE));

  int index31 = *index;
  char *PTR32 key_buffer31 = NULL;
  if (*key_buffer_length > 0)
  {
    key_buffer31 = storage_obtain31(*key_buffer_length);
  }
  int key_buffer_length31 = *key_buffer_length;

  KEY_HANDLE value_handle31 = {0};

  int actual_length31 = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = zjsm_get_object_entry(&instance31, &key_handle31, &index31, &key_buffer31, &key_buffer_length31, &value_handle31, &actual_length31);

  memcpy(*key_buffer, key_buffer31, key_buffer_length31);
  *actual_length = actual_length31;
  memcpy(value_handle, &value_handle31, sizeof(KEY_HANDLE));

  if (key_buffer31)
  {
    storage_release(key_buffer_length31, key_buffer31);
  }

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  return rc;
}

#pragma prolog(ZJSMCREN, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMCREN, " ZWEEPILG ")
int ZJSMCREN(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 parent_handle, const char *PTR64 entry_name, const char *PTR64 entry_value, int *PTR64 entry_type, KEY_HANDLE *PTR64 new_entry_handle)
{
  int rc = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  KEY_HANDLE parent_handle31 = {0};
  memcpy(&parent_handle31, parent_handle, sizeof(KEY_HANDLE));

  int entry_type31 = *entry_type;
  KEY_HANDLE new_entry_handle31 = {0};

  // Allocate 31-bit storage for entry name
  int entry_name_length = entry_name ? (int)strlen(entry_name) + 1 : 1;
  char *PTR32 entry_name31 = storage_obtain31(entry_name_length);
  if (entry_name)
  {
    memcpy(entry_name31, entry_name, strlen(entry_name));
    entry_name31[entry_name_length - 1] = '\0';
  }
  else
  {
    entry_name31[0] = '\0';
  }

  // Allocate 31-bit storage for entry value
  int entry_value_length = entry_value ? (int)strlen(entry_value) + 1 : 1;
  char *PTR32 entry_value31 = storage_obtain31(entry_value_length);
  if (entry_value)
  {
    memcpy(entry_value31, entry_value, strlen(entry_value));
    entry_value31[entry_value_length - 1] = '\0';
  }
  else
  {
    entry_value31[0] = '\0';
  }

  rc = zjsm_create_entry(&instance31, &parent_handle31, entry_name31, entry_value31, &entry_type31, &new_entry_handle31);

  memcpy(new_entry_handle, &new_entry_handle31, sizeof(KEY_HANDLE));
  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  // Release allocated storage
  storage_release(entry_name_length, entry_name31);
  storage_release(entry_value_length, entry_value31);

  return rc;
}

#pragma prolog(ZJSMTERM, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZJSMTERM, " ZWEEPILG ")
int ZJSMTERM(JSON_INSTANCE *PTR64 instance)
{
  int rc = 0;

  JSON_INSTANCE instance31 = {0};
  memcpy(&instance31, instance, sizeof(JSON_INSTANCE));

  rc = zjsm_term(&instance31);

  memcpy(instance, &instance31, sizeof(JSON_INSTANCE));

  if (instance->json)
  {
    storage_release(instance->json_length, instance->json);
    instance->json = NULL;
    instance->json_length = 0;
  }

  return rc;
}
