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

#include "zwto.h"
#include "zstorage.metal.test.h"
#include "zstorage.h"

#pragma prolog(STBT31, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(STBT31, " ZWEEPILG ")
void *STBT31(int *size)
{
  return storage_obtain31(*size);
}

#pragma prolog(STFREE, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(STFREE, " ZWEEPILG ")
int STFREE(int *size, void *data)
{
  storage_release(*size, data);
  return 0;
}

#pragma prolog(STGET64, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(STGET64, " ZWEEPILG ")
void *STGET64(int *size)
{
  return storage_get64(*size);
}

#pragma prolog(STREL, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(STREL, " ZWEEPILG ")
int STREL(void *data)
{
  storage_free64(data);
  return 0;
}