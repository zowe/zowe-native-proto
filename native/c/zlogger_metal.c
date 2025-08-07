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

  if (rc != 0 || logger_data == NULL)
  {
    /* Create new logger instance */
    logger_data = (zlogger_metal_t *)storage_get64(sizeof(zlogger_metal_t));
    if (logger_data)
    {
      memset(logger_data, 0, sizeof(zlogger_metal_t));

      /* Store in named token for future access */
      znts_create(3, &name, &token, 0);
    }
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

/* Record area for output formatting */
static char g_output_record[133] = {0};

/**
 * Write message to DD using BSAM I/O
 */
static int ZLGWRDD(int level, const char *message)
{
  zwto_debug("[>] ZLGWRDD called: level: %d, message: %s", level, message);
  zlogger_metal_t *logger = get_metal_logger();
  if (!logger || !logger->log_io)
  {
    /* DD not available or not opened */
    zwto_debug("[-] ZLGWRDD: DD not available or not opened");
    return -1;
  }

  /* Additional safety checks for Metal C */
  if (!message)
  {
    zwto_debug("[-] ZLGWRDD: message is NULL");
    return -1;
  }

  zwto_debug("[*] ZLGWRDD: message: %s", message);
  /* Format the message as a fixed-length record */
  unsigned long long msg_len = strlen(message);
  if (msg_len > 131)
    msg_len = 131; /* Leave room for newline and null terminator */

  /* Initialize record with spaces and ensure null termination */
  memset(g_output_record, ' ', 132);
  g_output_record[132] = '\0'; /* Null terminate the buffer */

  /* Copy message data safely */
  if (msg_len > 0)
  {
    memcpy(g_output_record, message, msg_len);
  }
  g_output_record[131] = '\n'; /* Add newline before null terminator */
  zwto_debug("[*] ZLGWRDD: g_output_record: %s", g_output_record);

  /* Write using BSAM synchronous write with additional safety check */
  int write_rc = writeSync(logger->log_io, g_output_record);

  return (write_rc == 0) ? 0 : -1;
}

#pragma prolog(ZLGINIT, " ZWEPROLG NEWDSA=(YES,512)")
#pragma epilog(ZLGINIT, " ZWEEPILG ")
int ZLGINIT(const char *log_file_path, int *min_level)
{
  zwto_debug("[>] ZLGINIT called");
  if (!log_file_path)
  {
    return -1;
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
  zwto_debug("[*] log_file_path: %s", log_file_path);
  if (min_level)
  {
    zwto_debug("[*] min_level: %d", *min_level);
  }

  strcpy(logger->log_path, log_file_path);

  logger->min_level = *min_level;
  logger->use_wto = 1;

  char alloc_cmd[364] = {0};
  int cmd_len = sprintf(alloc_cmd,
                        "alloc file(ZWXLOGDD) path('%.256s') pathopts(owronly,ocreat) %s",
                        logger->log_path, "pathmode(sirusr,siwusr,sirgrp) filedata(text)");
  zwto_debug("[*] alloc_cmd: %s", alloc_cmd);
  if (cmd_len >= sizeof(alloc_cmd) || cmd_len < 0)
  {
    zwto_debug("[-] alloc_cmd too long");
    return -1;
  }

  int size = sizeof(BPXWDYN_PARM) + sizeof(BPXWDYN_RESPONSE);
  unsigned char *p = (unsigned char *)storage_obtain31(size);
  memset(p, 0x00, sizeof(BPXWDYN_PARM) + sizeof(BPXWDYN_RESPONSE));

  BPXWDYN_PARM *bparm = (BPXWDYN_PARM *)p;
  BPXWDYN_RESPONSE *response = (BPXWDYN_RESPONSE *)(p + sizeof(BPXWDYN_PARM));

  bparm->len = sprintf(bparm->str, "%s", alloc_cmd);
  int rc = ZUTWDYN(bparm, response);
  zwto_debug("[*] ZUTWDYN rc: %d", rc);

  if (rc != 0)
  {
    zwto_debug("[-] ZUTWDYN failed");
    storage_release(size, p);
    return rc;
  }

  logger->log_io = open_output_assert("ZWXLOGDD", 132, 132, dcbrecf + dcbrecbr);
  logger->use_dd = (logger->log_io != NULL) ? 1 : 0;
  logger->initialized = 1;
  zwto_debug("[<] ZLGINIT success, use_dd: %d", logger->use_dd);
  storage_release(size, p);
  return 0;
}

int ZLGWRITE(int *level, const char *message)
{
  char formatted_msg[ZLOG_MAX_MSG] = {0};
  char timestamp[32] = {0};
  zwto_debug("[>] ZLGWRITE called: level: %d, message: %s", *level, message);
  const char *level_str = NULL;
  int result = 0;

  zlogger_metal_t *logger = get_metal_logger();
  if (!logger || !logger->initialized || !message)
  {
    zwto_debug("[-] ZLGWRITE: logger is NULL or not initialized or message is NULL");
    return -1;
  }

  /* Check if we should log this level */
  if (*level < logger->min_level || *level == ZLOGLEVEL_OFF)
  {
    zwto_debug("[-] ZLGWRITE: level is less than min_level or level is OFF");
    return 0;
  }

  /* Get timestamp and level string */
  ZLGTIME(timestamp, sizeof(timestamp));
  level_str = (*level >= ZLOGLEVEL_TRACE && *level <= ZLOGLEVEL_OFF)
                  ? g_level_strings[*level]
                  : "UNKNOWN";
  zwto_debug("[*] ZLGWRITE: level_str: %s", level_str);

  /* Format complete message */
  sprintf(formatted_msg, "[%s] [%s] %s",
          timestamp, level_str, message);

  zwto_debug("[*] ZLGWRITE: formatted_msg: %s", formatted_msg);
  /* Try DD writing first if available */
  if (logger->use_dd)
  {
    zwto_debug("[*] ZLGWRITE: using DD");
    result = ZLGWRDD(*level, formatted_msg);
    if (result == 0)
    {
      return 0; /* Successfully wrote to DD */
    }
  }

  /* Fall back to WTO */
  if (logger->use_wto)
  {
    zwto_debug("[*] ZLGWRITE: using WTO");
    result = ZLGWRWTO(*level, formatted_msg);
  }

  return result;
}

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

void ZLGSTLVL(int level)
{
  zlogger_metal_t *logger = get_metal_logger();
  if (logger && logger->initialized)
  {
    logger->min_level = level;
  }
}

int ZLGGTLVL(void)
{
  zlogger_metal_t *logger = get_metal_logger();
  if (logger && logger->initialized)
  {
    return logger->min_level;
  }
  return ZLOGLEVEL_OFF;
}

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
#endif

    /* Reset state and delete named token */
    memset(logger, 0, sizeof(*logger));
    znts_delete(3, &name);
  }
}