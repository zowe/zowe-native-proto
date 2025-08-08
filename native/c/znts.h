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

#ifndef ZNTS_H
#define ZNTS_H

#include <string.h>
#include "zmetal.h"

/* Named Token Service function codes */
#define NTS_CREATE_TOKEN 1
#define NTS_RETRIEVE_TOKEN 2
#define NTS_DELETE_TOKEN 3

/* Named Token Service return codes */
#define NTS_RC_OK 0
#define NTS_RC_NOT_FOUND 4
#define NTS_RC_EXISTS 8
#define NTS_RC_ERROR 12

// IRST64

typedef struct
{
  unsigned char value[16];
} ZNT_NAME;
typedef struct
{
  unsigned char value[16];
} ZNT_TOKEN;

typedef int (*PTR32 iean4cr_fn)(int *PTR64, ZNT_NAME *PTR64, ZNT_TOKEN *PTR64, int *PTR64, int *PTR64) ATTRIBUTE(amode64);
typedef int (*PTR32 iean4dl_fn)(int *PTR64, ZNT_NAME *PTR64, int *PTR64) ATTRIBUTE(amode64);
typedef int (*PTR32 iean4rt_fn)(int *PTR64, ZNT_NAME *PTR64, ZNT_TOKEN *PTR64, int *PTR64) ATTRIBUTE(amode64);

#if defined(__IBM_METAL__)
#define IEAN4CR(func)                                              \ 
            __asm(                                                     \ 
              "*                                                  \n"\ 
              " LLGT  15,X'10'                                    \n"\ 
              " L     15,X'220'(15,0)                             \n"\ 
              " L     15,X'14'(15,0)                              \n"\ 
              " L     15,X'7C'(15,0)  -> IEAN4CR                  \n"\ 
              " ST    15,%0           Save entry point            \n"\ 
              "*                                                    "\ 
              : "=m"(*func)                                          \ 
              :                                                      \ 
              : "r15");
#define IEAN4RT(func)                                              \ 
              __asm(                                                     \ 
                "*                                                  \n"\ 
                " LLGT  15,X'10'                                    \n"\ 
                " L     15,X'220'(15,0)                             \n"\ 
                " L     15,X'14'(15,0)                              \n"\ 
                " L     15,X'80'(15,0)  -> IEAN4RT                  \n"\ 
                " ST    15,%0           Save entry point            \n"\ 
                "*                                                    "\ 
                : "=m"(*func)                                          \ 
                :                                                      \ 
                : "r15");
#define IEAN4DL(func)                                              \ 
              __asm(                                                     \ 
                "*                                                  \n"\ 
                " LLGT  15,X'10'                                    \n"\ 
                " L     15,X'220'(15,0)                             \n"\ 
                " L     15,X'14'(15,0)                              \n"\ 
                " L     15,X'84'(15,0)  -> IEAN4DL                  \n"\ 
                " ST    15,%0           Save entry point            \n"\ 
                "*                                                    "\ 
                : "=m"(*func)                                          \ 
                :                                                      \ 
                : "r15");
#else
#define IEAN4CR(func)
#define IEAN4RT(func)
#define IEAN4DL(func)
#endif

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

  static int znts_create(
      int level, ZNT_NAME *name,
      ZNT_TOKEN *token, int persist)
  {
    int rc = 0;
    iean4cr_fn iean4cr = NULL;

    IEAN4CR(&iean4cr);
    return iean4cr(&level, name, token, &persist, &rc);
  }

  static int znts_retrieve(
      int level, ZNT_NAME *name,
      ZNT_TOKEN *token)
  {
    int rc = 0;
    iean4rt_fn iean4rt = NULL;

    IEAN4RT(&iean4rt);
    return iean4rt(&level, name, token, &rc);
  }

  static int znts_delete(
      int level, ZNT_NAME *name)
  {
    int rc = 0;
    iean4dl_fn iean4dl = NULL;

    IEAN4DL(&iean4dl);
    return iean4dl(&level, name, &rc);
  }

#if defined(__cplusplus)
}
#endif

#endif