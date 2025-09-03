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

// function to print hex bytes from a char array
void print_hex_bytes(char *bytes, int length)
{
  for (int i = 0; i < length; i++)
  {
    printf("%02x ", (unsigned char)bytes[i]);
  }
}

int main()
{
  JSON_INSTANCE instance = {0};
  int rc = 0;

  // std::string json = "{\"name\": \"John\", \"isMarried\": true, \"hasKids\": false, \"age\": 30, \"pets\": [\"dog\", \"cat\", \"fish\"], \"address\": {\"street\": \"123 Main St\", \"city\": \"Anytown\", \"state\": \"CA\", \"zip\": \"12345\"}}";

  // char json[] = "{\"name\": \"John\", \"isMarried\": true, \
  // \"hasKids\": false, \"age\": 30, \
  // \"pets\": [\"dog\", \"cat\", \"fish\"], \
  // \"address\": {\"street\": \
  // \"123 Main St\", \"city\": \"Anytown\", \"state\": \"CA\", \
  // \"zip\": \"12345\"}}";

  // #if defined(__IBMC__) || defined(__IBMCPP__)
  // #pragma convert(1208)
  // #endif
  // char *json = "{\"name\": \"John\"}"; //

  char *json = "{\"name\": \"John\", \"isMarried\": true, \"hasKids\": false, \"age\": 30, \"pets\": [\"dog\", \"cat\", \"fish\"], \"address\": {\"street\": \"123 Main St\", \"city\": \"Anytown\", \"state\": \"CA\", \"zip\": \"12345\"}}";
  // printf("json: %s\n", json);
  // printf("json in hex: ");
  // print_hex_bytes(json, strlen(json));
  // printf("\n");

  // #if defined(__IBMC__) || defined(__IBMCPP__)
  // #pragma convert(0)
  // #endif

  // memset(&instance, 0, sizeof(JSON_INSTANCE));
  rc = ZJSMINIT(&instance);
  if (0 != rc)
  {
    std::cout << "Error ZJSMINIT: " << rc << std::endl;
    return -1;
  }

  std::cout << "ZJSMINIT: " << rc << std::endl;
  // printf("instance handle: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", instance.handle.x[0], instance.handle.x[1], instance.handle.x[2], instance.handle.x[3], instance.handle.x[4], instance.handle.x[5], instance.handle.x[6], instance.handle.x[7], instance.handle.x[8], instance.handle.x[9], instance.handle.x[10], instance.handle.x[11]);

  int encoding = 2;
  rc = ZJSMSENC(&instance, &encoding);
  if (0 != rc)
  {
    std::cout << "Error ZJSMSENC: " << rc << std::endl;
    return -1;
  }

  printf("encoding: %d\n", encoding);

  rc = ZJSMPARS(&instance, json);
  if (0 != rc)
  {
    std::cout << "Error ZJSMPARS: " << rc << std::endl;
    return -1;
  }

  std::cout << "ZJSMPARS: " << rc << std::endl;

  encoding = 0;
  rc = ZJSMGENC(&instance, &encoding);
  if (0 != rc)
  {
    std::cout << "Error ZJSMGENC: " << rc << std::endl;
    return -1;
  }

  std::cout << "ZJSMGENC: " << encoding << std::endl;

  int number_entries = 0;
  KEY_HANDLE key_handle_zero = {0};
  rc = ZJSMGNUE(&instance, &key_handle_zero, &number_entries);
  if (0 != rc)
  {
    std::cout << "Error ZJSMGNUE: " << rc << std::endl;
    return -1;
  }

  std::cout << "ZJSMGNUE: " << number_entries << std::endl;

  // serialize JSON
  char serialized_json[1024] = {0};
  int serialized_json_length = (int)sizeof(serialized_json);
  int serialized_json_length_actual = 0;
  rc = ZJSMSERI(&instance, serialized_json, &serialized_json_length, &serialized_json_length_actual);
  if (0 != rc)
  {
    std::cout << "Error ZJSMSERI: " << rc << std::endl;
    return -1;
  }

  std::cout << "ZJSMSERI: " << serialized_json_length_actual << std::endl;

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
  char *PTR32 string_key = "address";
  // char *PTR32 string
  std::cout << "@TEST string_key: " << string_key << std::endl;
  rc = ZJSMSRCH(&instance, string_key, &key_handle);
  if (0 != rc)
  {
    printf("Error ZJSMSRCH: x'%x'\n", rc);
    return -1;
  }

  std::cout << "ZJSMSRCH: " << rc << std::endl;

  rc = ZJSMTERM(&instance);
  if (0 != rc)
  {
    std::cout << "Error ZJSMTERM: " << rc << std::endl;
    return -1;
  }

  std::cout << "ZJSMTERM: " << rc << std::endl;

  return 0;
}
