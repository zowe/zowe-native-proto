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

#ifndef ZMETAL_H
#define ZMETAL_H

#include <stdio.h>
#include <string.h>
#include "ztype.h"

#define MAX_PARM_LENGTH 100 + 1

typedef struct
{
  short int length;
  char parms[MAX_PARM_LENGTH];
} IN_DATA;

#define HI_BIT_MASK 0x7FFFFFFF
typedef struct
{
  union
  {
    IN_DATA *PTR32 addr;
    int addrValue;
  } data;
} IN_PARM;

#if defined(__IBM_METAL__)
#define S0C3(n)                                               \
  __asm(                                                      \
      "*                                                  \n" \
      " LLGF  0,%0      = Value passed by caller          \n" \
      " EXRL  0,*       Execute myself                    \n" \
      " DC    C'@S0C3'  Find by '@S0C3'                     " \
      "*                                                    " \
      :                                                       \
      : "m"(n)                                                \
      : "r0");
#else
#define S0C3(n)
#endif

static void s0c3_abend(int n)
{
  S0C3(n);
}

#if defined(__IBM_METAL__)
#define TESTAUTH(rc)                                          \
  __asm(                                                      \
      "*                                                  \n" \
      " TESTAUTH FCTN=1,"                                     \
      "RBLEVEL=1,"                                            \
      "STATE=YES,"                                            \
      "KEY=YES                                            \n" \
      "*                                                  \n" \
      " ST 15,%0               Save RC                    \n" \
      "*                                                    " \
      : "=m"(rc)::"r0", "r1", "r14", "r15");
#else
#define TESTAUTH(rc)
#endif

static int test_auth()
{
  int rc = 0;
  TESTAUTH(rc);
  return rc;
}

#if defined(__IBM_METAL__)
#define MODESET_MODE(value)                                    \
  __asm(                                                       \
      "*                                                   \n" \
      " MODESET MODE=" #value##"                           \n" \
                               "*                                                    " ::: "r0", "r1", "r14", "r15");
#else
#define MODESET_MODE(value)
#endif

#if defined(__IBM_METAL__)
#define MODESET_KEY(value)                                      \
  __asm(                                                        \
      "*                                                   \n"  \
      " MODESET KEY=" #value##"                             \n" \
                              "*                                                    " ::: "r0", "r1", "r14", "r15");
#else
#define MODESET_KEY(value)
#endif

#if defined(__IBM_METAL__)
#define LOAD(name, ep, rc)                                      \
  __asm(                                                        \
      "*                                                    \n" \
      " SLGR  2,2          Clear for PDSMAN/IEBCOPY         \n" \
      "*                                                    \n" \
      " LOAD EPLOC=%2,"                                         \
      "PLISTVER=MAX,"                                           \
      "ERRET=*+4                                            \n" \
      "*                                                    \n" \
      " STG 0,%0                                            \n" \
      " ST  15,%1                                           \n" \
      "*                                                      " \
      : "=m"(ep),                                               \
        "=m"(rc)                                                \
      : "m"(name)                                               \
      : "r0", "r1", "r2", "r14", "r15");
#else
#define LOAD(name, ep, rc)
#endif

#if defined(__IBM_METAL__)
#define DELETE(name, rc)                                         \
  __asm(                                                         \
      "*                                                     \n" \
      " DELETE EPLOC=%1                                      \n" \
      "*                                                     \n" \
      " ST 15,%0                                             \n" \
      "*                                                      "  \
      : "=m"(rc)                                                 \
      : "m"(name)                                                \
      : "r0", "r1", "r14", "r15");
#else
#define DELETE(name, rc)
#endif

/**
 * @brief Load a module into a 64-bit pointer
 *
 * @param name name of module to load
 * @return void* address of entry point or NULL if not found
 */
static void *PTR64 load_module(const char name[8])
{
  int rc = 0;

  char name_truncated[8 + 1] = {0};
  memset(name_truncated, ' ', sizeof(name_truncated) - 1);                                                             // pad with spaces
  memcpy(name_truncated, name, strlen(name) > sizeof(name_truncated) - 1 ? sizeof(name_truncated) - 1 : strlen(name)); // truncate

  void *PTR64 ep = NULL;

  LOAD(name_truncated, ep, rc);

  if (0 != rc)
  {
    return NULL;
  }

  return ep;
}

typedef void (*Z31FUNC)(void) ATTRIBUTE(amode31);
/**
 * @brief
 *
 * @param name name of module to load
 * @return Z31FUNC 31 bit function cleared for use within 64-bit routine or NULL if not found
 */
static Z31FUNC ATTRIBUTE(amode31) load_module31(const char name[8])
{

  // TODO(Kelosky): test return pointer flags to validate amode??
  void *PTR64 function = load_module(name);
  Z31FUNC z31func = NULL;
  if (function)
  {
    // make 31 bit EP pointer valid in 64 bit
    long long unsigned int ifunction = (long long unsigned int)function;
    ifunction &= 0x000000007FFFFFFF; // clear high bit
    z31func = (Z31FUNC)ifunction;
  }

  return z31func;
}

/**
 * @brief Delete a module that has been loaded
 *
 * @param name name of module to delete after a successful load
 * @return int 0 for success; non zero otherwise
 */
static int delete_module(const char name[8])
{
  int rc = 0;
  char name_truncated[9] = {0};
  memset(name_truncated, ' ', sizeof(name_truncated) - 1);                                                             // pad with spaces
  memcpy(name_truncated, name, strlen(name) > sizeof(name_truncated) - 1 ? sizeof(name_truncated) - 1 : strlen(name)); // truncate
  DELETE(name_truncated, rc);
  return rc;
}

/**
 * @brief Unconditionally attempt to enter supervisor state
 */
static void mode_sup()
{
  MODESET_MODE(SUP);
}

/**
 * @brief Unconditionally attempt to enter problem state
 */
static void mode_prob()
{
  MODESET_MODE(PROB);
}

static void mode_zero()
{
  MODESET_KEY(ZERO);
}

static void mode_nzero()
{
  MODESET_KEY(NZERO);
}

#if defined(__IBM_METAL__)
#define GET_REG64(reg, number)                                 \
  __asm(                                                       \
      "*                                                   \n" \
      " STG    " #number ",%0     Save reg                 \n" \
      "*                                                    "  \
      : "=m"(reg)                                              \
      :                                                        \
      :);
#else
#define GET_REG64(reg, number)
#endif

#if defined(__IBM_METAL__)
#define GET_PREV_REG64(reg, offset)                            \
  __asm(                                                       \
      "*                                                   \n" \
      " LG     1,128(,13)                                  \n" \
      " LG     1," #offset "(,1)                           \n" \
      " STG    1,%0                                        \n" \
      "*                                                    "  \
      : "=m"(reg)                                              \
      :                                                        \
      : "r1");
#else
#define GET_PREV_REG64(reg, offset)
#endif

#if defined(__IBM_METAL__)
#define SET_PREV_REG64(reg, offset)                            \
  __asm(                                                       \
      "*                                                   \n" \
      " LG     1,128(,13)                                  \n" \
      " STG    1," #offset "(,1)                           \n" \
      "*                                                    "  \
      :                                                        \
      : "m"(reg)                                               \
      : "r1");
#else
#define SET_PREV_REG64(reg, offset)
#endif

#if defined(__IBM_METAL__)
#define GET_KEY(key)                                 \
  __asm(                                             \
      "*                                         \n" \
      " IPK (2)                                  \n" \
      " STC  2,%0  Save                          \n" \
      "*                                          "  \
      : "=m"(key)                                    \
      :                                              \
      : "r2");
#else
#define GET_KEY(key)
#endif

#if defined(__IBM_METAL__)
#define SET_KEY(key)                                 \
  __asm(                                             \
      "*                                         \n" \
      " IC   2,%0  Load                          \n" \
      " SPKA 0(2)                                \n" \
      "*                                          "  \
      :                                              \
      : "m"(key)                                     \
      : "r2");
#else
#define SET_KEY(key)
#endif

static unsigned long long int get_r0()
{
  unsigned long long int reg = 0;
  GET_REG64(reg, 0);
  return reg;
}

static unsigned long long int get_r1()
{
  unsigned long long int reg = 0;
  GET_REG64(reg, 1);
  return reg;
}

static unsigned long long int get_r2()
{
  unsigned long long int reg = 0;
  GET_REG64(reg, 1);
  return reg;
}

static unsigned long long int get_r5()
{
  unsigned long long int reg = 0;
  GET_REG64(reg, 5);
  return reg;
}

static unsigned long long int get_r6()
{
  unsigned long long int reg = 0;
  GET_REG64(reg, 6);
  return reg;
}

static unsigned long long int get_r7()
{
  unsigned long long int reg = 0;
  GET_REG64(reg, 7);
  return reg;
}

static unsigned long long int get_r13()
{
  unsigned long long int reg = 0;
  GET_REG64(reg, 13);
  return reg;
}

static void get_r14_by_ref(unsigned long long int *reg)
{
  GET_REG64(*reg, 14);
}

static unsigned long long int get_prev_r14()
{
  unsigned long long int reg = 0;
  GET_PREV_REG64(reg, 8);
  return reg;
}

static unsigned long long int get_prev_r15()
{
  unsigned long long int reg = 0;
  GET_PREV_REG64(reg, 16);
  return reg;
}

static unsigned long long int get_prev_r0()
{
  unsigned long long int reg = 0;
  GET_PREV_REG64(reg, 24);
  return reg;
}

static unsigned long long int get_prev_r1()
{
  unsigned long long int reg = 0;
  GET_PREV_REG64(reg, 32);
  return reg;
}

static unsigned long long int get_prev_r2()
{
  unsigned long long int reg = 0;
  GET_PREV_REG64(reg, 40);
  return reg;
}

static unsigned char get_key()
{
  unsigned char key = {0};
  GET_KEY(key);
  return key;
}

static void set_key(unsigned char key)
{
  SET_KEY(key);
}

static void set_prev_r0(unsigned long long int reg)
{
  SET_PREV_REG64(reg, 24);
}

#if defined(__IBM_METAL__)
#define GET_STACK_ENV(reg)                                        \
  __asm(                                                          \
      "*                                                      \n" \
      " LG     1,128(,13)   -> PREV SA                        \n" \
      " STG    1,%0         SAVE                              \n" \
      "*                                                       "  \
      : "=m"(reg)                                                 \
      :                                                           \
      : "r1");
#else
#define GET_STACK_ENV(regs)
#endif

static unsigned long long int get_prev_r13()
{
  unsigned long long int reg = 0;
  GET_STACK_ENV(reg);
  return reg;
}

#if defined(__IBM_METAL__)
#define SET_REG(num, reg)                                       \
  __asm(                                                        \
      "*                                                    \n" \
      " L    %0," #num "  = Value passed by caller          \n" \
      "*                                                    "   \
      : "=m"(*reg)                                              \
      :                                                         \
      : "#num");
#else
#define SET_REG(num, reg)
#endif

#endif
