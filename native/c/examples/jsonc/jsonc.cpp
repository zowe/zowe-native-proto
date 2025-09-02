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

  char *PTR32 json = "{\"name\": \"John\", \"isMarried\": true, \"hasKids\": false, \"age\": 30, \"pets\": [\"dog\", \"cat\", \"fish\"], \"address\": {\"street\": \"123 Main St\", \"city\": \"Anytown\", \"state\": \"CA\", \"zip\": \"12345\"}}";

  memset(&instance, 0, sizeof(JSON_INSTANCE));
  rc = ZJSMINIT(&instance);
  if (0 != rc)
  {
    std::cout << "Error ZJSMINIT: " << rc << std::endl;
    return -1;
  }

  std::cout << "ZJSMINIT: " << rc << std::endl;
  printf("instance handle: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", instance.handle.x[0], instance.handle.x[1], instance.handle.x[2], instance.handle.x[3], instance.handle.x[4], instance.handle.x[5], instance.handle.x[6], instance.handle.x[7], instance.handle.x[8], instance.handle.x[9], instance.handle.x[10], instance.handle.x[11]);

  rc = ZJSMPARS(&instance, json);
  if (0 != rc)
  {
    std::cout << "Error ZJSMPARS: " << rc << std::endl;
    return -1;
  }

  std::cout << "ZJSMPARS: " << rc << std::endl;

  int encoding = 0;
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
