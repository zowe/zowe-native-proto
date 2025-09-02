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

  char json[] = "{\"name\": \"John\", \"isMarried\": true, \
  \"hasKids\": false, \"age\": 30, \
  \"pets\": [\"dog\", \"cat\", \"fish\"], \
  \"address\": {\"street\": \
  \"123 Main St\", \"city\": \"Anytown\", \"state\": \"CA\", \
  \"zip\": \"12345\"}}";

  printf("instance handle: %d\n", instance.handle);
  rc = ZJSMINIT(&instance);
  if (0 != rc)
  {
    std::cout << "Error ZJSMINIT: " << rc << std::endl;
    return -1;
  }

  std::cout << "ZJSMINIT: " << rc << std::endl;
  printf("instance handle: %d\n", instance.handle);

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

  KEY_HANDLE key_handle = {0};
  std::string string_key = "name";
  printf("first char of name is: %c\n", string_key[0]);
  rc = ZJSMSRCH(&instance, string_key.c_str(), &key_handle);
  if (0 != rc)
  {
    std::cout << "Error ZJSMSRCH: " << rc << std::endl;
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
