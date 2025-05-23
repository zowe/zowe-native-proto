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

#ifndef ZSTORAGE_METAL_TEST_HPP
#define ZSTORAGE_METAL_TEST_HPP

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

  void *STBT31(int size);
  int STFREE(int *size, void *data);

  void *STGET64(int size);
  int STREL(void *);

#if defined(__cplusplus)
}
#endif

#endif