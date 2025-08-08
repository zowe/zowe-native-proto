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

#include "dcbd.h"
#include "zlogger_metal.h"
#include "zam.h"
#include "zstorage.h"
#include "zutm.h"
#include "zwto.h"
#include "znts.h"

typedef struct zlogger_metal_s
{
  char log_path[256]; /* Log file path */
  int min_level;      /* Minimum log level */
  int initialized;    /* Initialization flag */
  int use_wto;        /* Whether to use WTO for logging */
  int use_dd;         /* Whether DD is available for file I/O */
  IO_CTRL *log_io;    /* BSAM I/O control block for log DD */
} zlogger_metal_t;

static zlogger_metal_t *get_metal_logger()
{
  zlogger_metal_t *logger_data = NULL;
  int token_length = 0;

  ZNT_NAME name = {"ZWXLOGGER"};
  ZNT_TOKEN token = {0};

  /* Try to retrieve existing logger from named token */
  int rc = znts_retrieve(3, &name, &token);

  if (rc == 4)
  {
    /* Named/token pair doesn't exist - create new logger instance */
    logger_data = (zlogger_metal_t *)storage_get64(sizeof(zlogger_metal_t));
    if (logger_data)
    {
      memset(logger_data, 0, sizeof(zlogger_metal_t));

      /* Store logger_data address as the token for znts_create */
      memcpy(&token, &logger_data, sizeof(void *));
      int create_rc = znts_create(3, &name, &token, 0);
    }
  }
  else if (rc == 0)
  {
    /* Token exists - retrieve logger_data from token */
    memcpy(&logger_data, &token, sizeof(void *));
  }

  return logger_data;
}

static const char *g_level_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF"};

/**
 * Get current timestamp for Metal C using z/OS STCK instruction
 */
static void ZLGTIME(char *buffer, int buffer_size)
{
#if defined(__IBM_METAL__)
  /* Use z/OS STCK (Store Clock) instruction for high-precision timestamp */
  unsigned long long clock_value = 0;

  __asm(" STCK %0" : "=m"(clock_value) : : "cc");

  /* Convert STCK value to a simple readable format */
  /* STCK returns microseconds since 1900-01-01 00:00:00 GMT */
  /* For simplicity, we'll just show the lower bits as a hex timestamp */
  sprintf(buffer, "%016llX", clock_value);
#else
  /* Fallback for non-Metal C compilation */
  strncpy(buffer, "TIMESTAMP", buffer_size - 1);
  buffer[buffer_size - 1] = '\0';
#endif
}

/**
 * Write message using WTO (Write To Operator)
 */
static int ZLGWRWTO(int level, const char *message)
{
  /* Ensure WTO_BUF is aligned on fullword boundary for Metal C */
  WTO_BUF wto_buf __attribute__((aligned(4))) = {0};

  /* Format WTO message with level prefix */
  const char *level_str = (level >= ZLOGLEVEL_TRACE && level <= ZLOGLEVEL_OFF)
                              ? g_level_strings[level]
                              : "UNKNOWN";

  wto_buf.len = sprintf(wto_buf.msg, "ZOWEX %s: %.100s", level_str, message);

  if (wto_buf.len >= sizeof(wto_buf.msg))
  {
    wto_buf.len = sizeof(wto_buf.msg) - 1;
  }

  return wto(&wto_buf);
}

/**
 * Write message to DD using BSAM I/O
 */
#pragma prolog(ZLGWRDD, " ZWEPROLG NEWDSA=(YES,512)")
#pragma epilog(ZLGWRDD, " ZWEEPILG ")
static int ZLGWRDD(int level, const char *message)
{
  zlogger_metal_t *logger = get_metal_logger();
  if (!logger || !logger->log_io)
  {
    /* DD not available or not opened */
    return -1;
  }

  /* Additional safety checks for Metal C */
  if (!message)
  {
    return -1;
  }

  /* Format the message as a fixed-length record using local buffer */
  char writeBuf[132] = {0};
  unsigned long long msg_len = strlen(message);
  if (msg_len > 130)
    msg_len = 130; /* Leave room for newline */

  /* Initialize record with spaces like ZUTDBGMG */
  memset(writeBuf, ' ', 132);

  /* Copy message data safely */
  if (msg_len > 0)
  {
    memcpy(writeBuf, message, msg_len);
  }
  writeBuf[msg_len] = '\n'; /* Add newline after message */

  /* Write using BSAM synchronous write - writeSync already calls check() */
  int write_rc = writeSync(logger->log_io, writeBuf);

  /* Force immediate writing to USS file by closing and reopening DCB */
  /* Per IBM docs: BSAM buffers writes beyond program buffering for USS files */
  if (write_rc == 0)
  {
    /* Close DCB to force flush to USS file */
    close_assert(logger->log_io);
    /* Reopen DCB for next write */
    logger->log_io = open_output_assert("ZWXLOGDD", 132, 132, dcbrecf + dcbrecbr);
    if (!logger->log_io)
    {
      logger->use_dd = 0;
      return -1;
    }
  }

  return (write_rc == 0) ? 0 : -1;
}

#pragma prolog(ZLGINIT, " ZWEPROLG NEWDSA=(YES,512)")
#pragma epilog(ZLGINIT, " ZWEEPILG ")
int ZLGINIT(const char *log_file_path, int *min_level)
{
  if (!log_file_path)
  {
    return -1;
  }

  /* Check if token exists and delete it to force re-creation */
  ZNT_NAME name = {"ZWXLOGGER"};
  ZNT_TOKEN token = {0};
  int retrieve_rc = znts_retrieve(3, &name, &token);
  if (retrieve_rc == 0)
  {
    int delete_rc = znts_delete(3, &name);
  }

  zlogger_metal_t *logger = get_metal_logger();
  if (!logger)
  {
    return -1;
  }

  if (logger->initialized)
  {
    return 0;
  }

  strcpy(logger->log_path, log_file_path);

  logger->min_level = *min_level;
  logger->use_wto = 1;

  char alloc_cmd[364] = {0};
  int cmd_len = sprintf(alloc_cmd,
                        "alloc file(ZWXLOGDD) path('%.256s') pathopts(owronly,ocreat,osync) %s",
                        logger->log_path, "pathmode(sirusr,siwusr,sirgrp) filedata(text)");
  if (cmd_len >= sizeof(alloc_cmd) || cmd_len < 0)
  {
    return -1;
  }

  int size = sizeof(BPXWDYN_PARM) + sizeof(BPXWDYN_RESPONSE);
  unsigned char *p = (unsigned char *)storage_obtain31(size);
  memset(p, 0x00, sizeof(BPXWDYN_PARM) + sizeof(BPXWDYN_RESPONSE));

  BPXWDYN_PARM *bparm = (BPXWDYN_PARM *)p;
  BPXWDYN_RESPONSE *response = (BPXWDYN_RESPONSE *)(p + sizeof(BPXWDYN_PARM));

  bparm->len = sprintf(bparm->str, "%s", alloc_cmd);
  int rc = ZUTWDYN(bparm, response);

  if (rc != 0)
  {
    storage_release(size, p);
    return rc;
  }

  logger->log_io = open_output_assert("ZWXLOGDD", 132, 132, dcbrecf + dcbrecbr);
  logger->use_dd = (logger->log_io != NULL) ? 1 : 0;
  logger->initialized = 1;
  storage_release(size, p);
  return 0;
}

#pragma prolog(ZLGWRITE, " ZWEPROLG NEWDSA=(YES,512)")
#pragma epilog(ZLGWRITE, " ZWEEPILG ")
int ZLGWRITE(int *level, const char *message)
{
  char formatted_msg[ZLOG_MAX_MSG] = {0};
  char timestamp[32] = {0};
  const char *level_str = NULL;
  int result = 0;

  zlogger_metal_t *logger = get_metal_logger();
  if (!logger || !logger->initialized || !message)
  {
    return -1;
  }

  /* Check if we should log this level */
  if (*level < logger->min_level || *level == ZLOGLEVEL_OFF)
  {
    return 0;
  }

  /* Get timestamp and level string */
  ZLGTIME(timestamp, sizeof(timestamp));
  level_str = (*level >= ZLOGLEVEL_TRACE && *level <= ZLOGLEVEL_OFF)
                  ? g_level_strings[*level]
                  : "UNKNOWN";

  /* Format complete message */
  sprintf(formatted_msg, "[%s] [%s] %s",
          timestamp, level_str, message);

  /* Try DD writing first if available */
  if (logger->use_dd)
  {
    result = ZLGWRDD(*level, formatted_msg);
    if (result == 0)
    {
      return 0; /* Successfully wrote to DD */
    }
  }

  /* Fall back to WTO */
  if (logger->use_wto)
  {
    result = ZLGWRWTO(*level, formatted_msg);
  }

  return result;
}

#pragma prolog(ZLGWRFMT, " ZWEPROLG NEWDSA=(YES,512)")
#pragma epilog(ZLGWRFMT, " ZWEEPILG ")
int ZLGWRFMT(int level, const char *prefix, const char *message, const char *suffix)
{
  char formatted_msg[ZLOG_MAX_MSG];
  int pos = 0;

  if (!message)
  {
    return -1;
  }

  /* Simple string concatenation since Metal C has limited printf */
  if (prefix)
  {
    unsigned long long prefix_len = strlen(prefix);
    if (pos + prefix_len < ZLOG_MAX_MSG - 1)
    {
      strcpy(formatted_msg + pos, prefix);
      pos += prefix_len;
      if (pos < ZLOG_MAX_MSG - 2)
      {
        formatted_msg[pos] = ' ';
        pos++;
      }
    }
  }

  unsigned long long msg_len = strlen(message);
  if (pos + msg_len < ZLOG_MAX_MSG - 1)
  {
    strcpy(formatted_msg + pos, message);
    pos += msg_len;
  }

  if (suffix && pos < ZLOG_MAX_MSG - 1)
  {
    if (pos < ZLOG_MAX_MSG - 2)
    {
      formatted_msg[pos] = ' ';
      pos++;
    }
    unsigned long long suffix_len = strlen(suffix);
    if (pos + suffix_len < ZLOG_MAX_MSG - 1)
    {
      strcpy(formatted_msg + pos, suffix);
      pos += suffix_len;
    }
  }

  formatted_msg[pos] = '\0';

  return ZLGWRITE(&level, formatted_msg);
}

#pragma prolog(ZLGSTLVL, " ZWEPROLG NEWDSA=(YES,512)")
#pragma epilog(ZLGSTLVL, " ZWEEPILG ")
void ZLGSTLVL(int level)
{
  zlogger_metal_t *logger = get_metal_logger();
  if (logger && logger->initialized)
  {
    logger->min_level = level;
  }
}

#pragma prolog(ZLGGTLVL, " ZWEPROLG NEWDSA=(YES,512)")
#pragma epilog(ZLGGTLVL, " ZWEEPILG ")
int ZLGGTLVL(void)
{
  zlogger_metal_t *logger = get_metal_logger();
  if (logger && logger->initialized)
  {
    return logger->min_level;
  }
  return ZLOGLEVEL_OFF;
}

#pragma prolog(ZLGCLEAN, " ZWEPROLG NEWDSA=(YES,512)")
#pragma epilog(ZLGCLEAN, " ZWEEPILG ")
void ZLGCLEAN(void)
{
  zlogger_metal_t *logger = get_metal_logger();
  ZNT_NAME name = {"ZWXLOGGER"};
  if (logger && logger->initialized)
  {
    /* Log cleanup message before closing */
    if (logger->use_dd && logger->log_io)
    {
      ZLGWRDD(ZLOGLEVEL_INFO, "Metal C logger cleanup - closing DD");
    }
    else
    {
      ZLGWRWTO(ZLOGLEVEL_INFO, "Metal C logger cleanup");
    }

#if defined(__IBM_METAL__)
    /* Close the DD if it was opened */
    if (logger->log_io)
    {
      close_assert(logger->log_io);
      logger->log_io = NULL;
    }

    /* Free the DD allocation to ensure proper cleanup */
    if (logger->use_dd)
    {
      int size = sizeof(BPXWDYN_PARM) + sizeof(BPXWDYN_RESPONSE);
      unsigned char *p = (unsigned char *)storage_obtain31(size);
      if (p)
      {
        memset(p, 0x00, size);
        BPXWDYN_PARM *bparm = (BPXWDYN_PARM *)p;
        bparm->len = sprintf(bparm->str, "free file(ZWXLOGDD)");
        ZUTWDYN(bparm, (BPXWDYN_RESPONSE *)(p + sizeof(BPXWDYN_PARM)));
        storage_release(size, p);
      }
    }
#endif

    /* Reset state and delete named token */
    memset(logger, 0, sizeof(*logger));
    znts_delete(3, &name);
  }
}