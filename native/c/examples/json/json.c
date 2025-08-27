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

#include "zmetal.h"
#include "zwto.h"
#include "zjsonm.h"

#pragma prolog(main, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(main, " ZWEEPILG ")

int main()
{
  PARSE_HANDLE handle = {0};
  KEY_HANDLE key_handle = {0};
  DIAG diag = {0};

  int rc = 0;
  char json[] = "{\"name\": \"John\", \"isMarried\": true, \
  \"hasKids\": false, \"age\": 30, \
  \"pets\": [\"dog\", \"cat\", \"fish\"], \
  \"address\": {\"street\": \
  \"123 Main St\", \"city\": \"Anytown\", \"state\": \"CA\", \
  \"zip\": \"12345\"}}";

  int print_length = 25;
  int print_offset = 0;
  int total_length = sizeof(json);

  zwto_debug("@TEST json:");

  while (print_offset < total_length)
  {
    zwto_debug("%.*s", print_length, json + print_offset);
    print_offset += print_length;
  }

  /**
   * initialize json services and parse json
   */

  // initialize JSON services
  rc = ZJSNMINIT(&handle, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSNMINIT error: %d - exiting...", rc);
    return -1;
  }

  // parse JSON
  rc = ZJSMPARS(&handle, json, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMPARS error: %d - exiting...", rc);
    return -1;
  }

  /**
   * search for string, get it's type, and get value
   */

  // search for string key
  char *PTR32 string_key = "name";
  rc = ZJSMSRCH(&handle, string_key, &key_handle, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMSRCH error: %d - exiting...", rc);
    return -1;
  }

  // get type of key
  // NOTE(Kelosky): types are:
  // HWTJ_OBJECT_TYPE
  // HWTJ_ARRAY_TYPE
  // HWTJ_STRING_TYPE
  // HWTJ_NUMBER_TYPE
  // HWTJ_BOOLEAN_TYPE
  // HWTJ_NULL_TYPE
  int type = 0;
  rc = ZJSNGJST(&handle, &key_handle, &type, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSNGJST error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST type of name: %d", type);

  // get value from previous search
  char *PTR32 string_value = NULL;
  int string_value_length = 0;
  rc = ZJSMGVAL(&handle, &key_handle, &string_value, &string_value_length, &diag);

  zwto_debug("@TEST result: %.*s", string_value_length, string_value);

  // search for array key
  char *PTR32 array_key = "pets";
  rc = ZJSMSRCH(&handle, array_key, &key_handle, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMSRCH error: %d - exiting...", rc);
    return -1;
  }

  rc = ZJSNGJST(&handle, &key_handle, &type, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSNGJST error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST type of pets: %d", type);

  // get number of entries
  int number_entries = 0;
  rc = ZJSMGNUE(&handle, &key_handle, &number_entries, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMGNUE error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST array number_entries of pets: %d", number_entries);

  int index = 1;
  KEY_HANDLE value = {0};
  rc = ZJSMGAEN(&handle, &key_handle, &index, &value, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMGAEN error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST array index of pets: %d", index);

  rc = ZJSMGVAL(&handle, &value, &string_value, &string_value_length, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMGVAL error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST array value of pets: %.*s", string_value_length, string_value);

  // search for object key
  char *PTR32 object_key = "address";
  rc = ZJSMSRCH(&handle, object_key, &key_handle, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMSRCH error: %d - exiting...", rc);
    return -1;
  }

  // get number of entries
  rc = ZJSMGNUE(&handle, &key_handle, &number_entries, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMGNUE error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST number_entries of address: %d", number_entries);

  char buffer[100] = {0};
  int buffer_length = (int)sizeof(buffer);
  int actual_length = 0;
  char *PTR32 buffer_ptr = buffer;
  index = 3;
  rc = ZJSMGOEN(&handle, &key_handle, &index, &buffer_ptr, &buffer_length, &value, &actual_length, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMGOEN error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST object value of address: %s and actual_length: %d", buffer, actual_length);

  rc = ZJSMGVAL(&handle, &value, &string_value, &string_value_length, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMGVAL error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST object value of address.%s: %.*s", buffer, string_value_length, string_value);

  // search for boolean key
  char *PTR32 boolean_key = "isMarried";
  rc = ZJSMSRCH(&handle, boolean_key, &key_handle, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMSRCH error: %d - exiting...", rc);
    return -1;
  }

  rc = ZJSNGJST(&handle, &key_handle, &type, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSNGJST error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST type of isMarried: %d", type);

  // get boolean value
  char boolean_value = 0x00;
  rc = ZJSMGBOV(&handle, &key_handle, &boolean_value, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMGBOV error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST boolean_value of isMarried: %x", boolean_value);

  // search for boolean key
  char *PTR32 boolean_key2 = "hasKids";
  rc = ZJSMSRCH(&handle, boolean_key2, &key_handle, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMSRCH error: %d - exiting...", rc);
    return -1;
  }

  rc = ZJSNGJST(&handle, &key_handle, &type, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSNGJST error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST type of hasKids: %d", type);

  // get boolean value
  char boolean_value2 = 0x00;
  rc = ZJSMGBOV(&handle, &key_handle, &boolean_value2, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMGBOV error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST boolean_value2 of hasKids: %x", boolean_value2);

  // char *PTR32 boolean_value3 = NULL;
  // int boolean_value3_length = 0;
  // rc = ZJSMGVAL(&handle, &key_handle, &boolean_value3, &boolean_value3_length, &diag);
  // if (0 != rc)
  // {
  //   zwto_debug("@TEST ZJSMGVAL error: %d - exiting...", rc);
  //   return -1;
  // }
  // zwto_debug("@TEST boolean_value3 result: %.*s", boolean_value3_length, boolean_value3);

  // search for number key
  char *PTR32 number_key = "age";
  rc = ZJSMSRCH(&handle, number_key, &key_handle, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMSRCH error: %d - exiting...", rc);
    return -1;
  }

  // get value from previous search
  char *PTR32 number_value = NULL;
  int number_value_length = 0;
  rc = ZJSMGVAL(&handle, &key_handle, &number_value, &number_value_length, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMGVAL error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("@TEST result of age: %.*s", number_value_length, number_value);

  // terminate JSON services
  rc = ZJSNMTERM(&handle, &diag);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSNMTERM error: %d - exiting...", rc);
    return -1;
  }

  return 0;
}