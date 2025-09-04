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
#include "zjsonm.h"
#include "zjsontype.h"
#include "zjson.hpp"

int run_low_level_json();

// NOTE(Kelosky): this file is build and run with `xlclang++` but `xlc++` is used to build the `zjsonm.o` file
// `xlc++` can also be used
int main()
{
  try
  {
    ZJson json;
    json.parse("{\n"
               "  \"name\": \"John\",\n"
               "  \"isMarried\": true,\n"
               "  \"hasKids\": false,\n"
               "  \"age\": 30,\n"
               "  \"pets\": [\"dog\", \"cat\", \"fish\"],\n"
               "  \"address\": {\n"
               "    \"street\": \"123 Main St\",\n"
               "    \"city\": \"Anytown\",\n"
               "    \"state\": \"CA\",\n"
               "    \"zip\": \"12345\"\n"
               "  },\n"
               "  \"work\": {\n"
               "    \"company\": \"MegaCorp\",\n"
               "    \"office\": {\n"
               "      \"building\": \"Tower 1\",\n"
               "      \"location\": {\n"
               "        \"floor\": 42,\n"
               "        \"desk\": \"A1\"\n"
               "      }\n"
               "    }\n"
               "  }\n"
               "}");

    std::string serialized_json = json.to_string();

    std::cout << "Serialized JSON:\n"
              << serialized_json << std::endl;

    std::cout << "Name: " << json["name"] << std::endl;
    std::cout << "Is Married: " << json["isMarried"] << std::endl;
    std::cout << "Has Kids: " << json["hasKids"] << std::endl;
    std::cout << "Age: " << json["age"] << std::endl;
    std::cout << "Pets: " << json["pets"][1] << std::endl;
    std::cout << "Address Street: " << json["address"]["street"] << std::endl;
    std::cout << "Work Desk: " << json["work"]["office"]["location"]["desk"] << std::endl;
  }
  catch (const std::runtime_error &e)
  {
    std::cout << "Error: " << e.what() << std::endl;
  }

  return 0;
}

int run_low_level_json()
{
  JSON_INSTANCE instance = {0};
  int rc = 0;

  char *json = "{\n"
               "  \"name\": \"John\",\n"
               "  \"isMarried\": true,\n"
               "  \"hasKids\": false,\n"
               "  \"age\": 30,\n"
               "  \"pets\": [\"dog\", \"cat\", \"fish\"],\n"
               "  \"address\": {\n"
               "    \"street\": \"123 Main St\",\n"
               "    \"city\": \"Anytown\",\n"
               "    \"state\": \"CA\",\n"
               "    \"zip\": \"12345\"\n"
               "  },\n"
               "  \"work\": {\n"
               "    \"company\": \"MegaCorp\",\n"
               "    \"office\": {\n"
               "      \"building\": \"Tower 1\",\n"
               "      \"location\": {\n"
               "        \"floor\": 42,\n"
               "        \"desk\": \"A1\"\n"
               "      }\n"
               "    }\n"
               "  }\n"
               "}";

  rc = ZJSMINIT(&instance);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMPARS rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  // printf("instance handle: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", instance.handle.x[0], instance.handle.x[1], instance.handle.x[2], instance.handle.x[3], instance.handle.x[4], instance.handle.x[5], instance.handle.x[6], instance.handle.x[7], instance.handle.x[8], instance.handle.x[9], instance.handle.x[10], instance.handle.x[11]);

  int encoding = 2;
  rc = ZJSMSENC(&instance, &encoding);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSENC rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  printf("Encoding was set to: %d\n", encoding);

  rc = ZJSMPARS(&instance, json);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMPARS rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  encoding = 0;
  rc = ZJSMGENC(&instance, &encoding);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMGENC rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "Encoding that was received was: " << encoding << std::endl;

  int number_entries = 0;
  KEY_HANDLE key_handle_zero = {0};
  rc = ZJSMGNUE(&instance, &key_handle_zero, &number_entries);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMGNUE rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "Number of entries was: " << number_entries << std::endl;

  // serialize JSON
  char serialized_json[1024] = {0};
  int serialized_json_length = (int)sizeof(serialized_json);
  int serialized_json_length_actual = 0;
  rc = ZJSMSERI(&instance, serialized_json, &serialized_json_length, &serialized_json_length_actual);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSERI rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "Serialized JSON length was: " << serialized_json_length_actual << std::endl;

  // printf("Serialized JSON in hex: ");
  // print_hex_bytes(serialized_json, serialized_json_length_actual);
  // printf("\n");

  // print serialized JSON
  printf("Serialized JSON:\n%.*s\n", serialized_json_length_actual, serialized_json);
  // std::cout << "Serialized JSON:\n"
  // << serialized_json << std::endl;

  KEY_HANDLE key_handle = {0};
  // std::string string_key = "name";
  // printf("first char of name is: %c\n", string_key[0]);
  std::string string_key = "name";
  // char *PTR32 string
  std::cout << "Searching for key: " << string_key << std::endl;
  rc = ZJSMSSRC(&instance, string_key.c_str(), &key_handle);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSSRC rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  int type = 0;
  rc = ZJSNGJST(&instance, &key_handle, &type);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSNGJST rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "Type was: " << type << std::endl;

  char *string_value = NULL;
  int string_value_length = 0;
  rc = ZJSMGVAL(&instance, &key_handle, &string_value, &string_value_length);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMGVAL rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }
  std::cout << "String value length was: " << string_value_length << std::endl;
  printf("String value: %.*s\n", string_value_length, string_value);

  string_key = "isMarried";
  std::cout << "Searching for key: " << string_key << std::endl;
  rc = ZJSMSSRC(&instance, string_key.c_str(), &key_handle);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSSRC rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  rc = ZJSNGJST(&instance, &key_handle, &type);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSNGJST rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "Type was: " << type << std::endl;

  char boolean_value = 0;
  rc = ZJSMGBOV(&instance, &key_handle, &boolean_value);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSGBOV rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  printf("Boolean value was: %x\n", boolean_value);

  // find pets
  std::cout << "Searching for key: pets" << std::endl;
  rc = ZJSMSSRC(&instance, "pets", &key_handle);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSSRC rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  // get number of entries in pets array
  rc = ZJSMGNUE(&instance, &key_handle, &number_entries);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSGNUE rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "Number of entries was: " << number_entries << std::endl;

  // get handle of second entry in pets array
  int index = 2;
  KEY_HANDLE value = {0};
  rc = ZJSMGAEN(&instance, &key_handle, &index, &value);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSGAEN rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "Index was: " << index << std::endl;

  // get value of second entry in pets array
  rc = ZJSMGVAL(&instance, &value, &string_value, &string_value_length);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSGVAL rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "String value length was: " << string_value_length << std::endl;

  printf("String value: %.*s\n", string_value_length, string_value);

  // find address
  std::cout << "Searching for key: address" << std::endl;
  rc = ZJSMSSRC(&instance, "address", &key_handle);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSSRC rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  // get number of entries in address object
  rc = ZJSMGNUE(&instance, &key_handle, &number_entries);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSGNUE rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "Number of entries was: " << number_entries << std::endl;

  // get value of third entry in address object
  index = 3;
  int actual_length = 0;

  char key_buffer[100] = {0};
  int key_buffer_length = (int)sizeof(key_buffer);
  // int actual_length = 0;
  char *key_buffer_ptr = key_buffer;

  rc = ZJSMGOEN(&instance, &key_handle, &index, &key_buffer_ptr, &key_buffer_length, &value, &actual_length);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSGOEN rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  std::cout << "Key buffer length was: " << key_buffer_length << std::endl;

  printf("Key buffer: %.*s\n", key_buffer_length, key_buffer);

  rc = ZJSMGVAL(&instance, &value, &string_value, &string_value_length);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMSGVAL rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }
  std::cout << "String value length was: " << string_value_length << std::endl;
  printf("String value: %.*s\n", string_value_length, string_value);

  // get handle of third entry in address object
  rc = ZJSMTERM(&instance);
  if (0 != rc)
  {
    std::cout << __FILE__ << ":" << __LINE__ << " ZJSMTERM rc=x'" << std::hex << rc << std::dec << "'" << std::endl;
    std::cout << "Error, exiting..." << std::endl;
    return -1;
  }

  return 0;
}
