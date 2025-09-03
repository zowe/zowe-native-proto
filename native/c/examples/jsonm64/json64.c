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
// #include "zjsonm31.h"

// static void print_hex_bytes(const char *bytes, int length)
// {
//   for (int i = 0; i < length; i++)
//   {
//     zwto_debug("%02x ", (unsigned char)bytes[i]);
//   }
// }

#pragma prolog(main, " ZWEPROLG NEWDSA=(YES,128) ")
#pragma epilog(main, " ZWEEPILG ")

int main()
{
  JSON_INSTANCE instance = {0};
  KEY_HANDLE key_handle = {0};

  int rc = 0;
  char json[] = "{\"name\": \"John\"}";

  ZJSMINIT(&instance);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMINIT error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("ZJSMINIT: %d", rc);

  ZJSMPARS(&instance, json);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMPARS error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("ZJSMPARS: %d", rc);

  char serialized_json[1024] = {0};
  int serialized_json_length = (int)sizeof(serialized_json);
  int serialized_json_length_actual = 0;
  ZJSMSERI(&instance, serialized_json, &serialized_json_length, &serialized_json_length_actual);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMSERI error: %d - exiting...", rc);
    return -1;
  }
  zwto_debug("ZJSMSERI: %d", rc);
  zwto_debug("ZJSMSERI: %d", serialized_json_length_actual);

  int print_length = 25;
  int print_offset = 0;
  int total_length = serialized_json_length_actual;

  zwto_debug("@TEST serialized JSON:");

  while (print_offset < total_length)
  {
    if (print_length < total_length)
    {
      zwto_debug("%.*s", print_length, serialized_json + print_offset);
    }
    else
    {
      zwto_debug("%.*s", total_length - print_offset, serialized_json + print_offset);
    }
    print_offset += print_length;
  }

  ZJSMTERM(&instance);
  if (0 != rc)
  {
    zwto_debug("@TEST ZJSMTERM error: %d - exiting...", rc);
    return -1;
  }

  zwto_debug("ZJSMTERM: %d", rc);

  return 0;
}