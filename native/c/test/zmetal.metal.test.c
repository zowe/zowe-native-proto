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

#include "zmetal.metal.test.h"
#include "zmetal.h"

#pragma prolog(ZMTLLOAD, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZMTLLOAD, " ZWEEPILG ")
void *ZMTLLOAD(const char *name)
{
    void *ep = load_module(name);
    return ep;
}

#pragma prolog(ZMTLDEL, " ZWEPROLG NEWDSA=(YES,4) ")
#pragma epilog(ZMTLDEL, " ZWEEPILG ")
int ZMTLDEL(const char *name)
{
    return delete_module(name);
}
