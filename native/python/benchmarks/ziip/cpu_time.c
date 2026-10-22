/*
 * SPDX-License-Identifier: EPL-2.0
 * Copyright Contributors to the Zowe Project.
 */

#define _OPEN_SYS_EXT 1
#include <stdlib.h>
#include <string.h>

extern int znp_timeused(unsigned long long *work);

/* TIMEUSED ECT requires 32 result bytes and a 144-byte F4SA below 2 GB.
 * The four results are total normalized, CP, raw offload, and offload-on-CP
 * time, in TOD units (4096 units per microsecond). This measures this task.
 */
#pragma export(znp_cpu_time)
int znp_cpu_time(unsigned long long *result)
{
  int rc = 0;
  unsigned long long *work;
  if (result == NULL)
    return 8;
  work = __malloc31(32 + 144);
  if (work == NULL)
    return 12;
  memset(work, 0, 32 + 144);

  rc = znp_timeused(work);
  if (rc == 0)
    memcpy(result, work, 32);
  free(work);
  return rc;
}
