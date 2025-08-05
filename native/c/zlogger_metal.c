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
#include "zwto.h"

static struct
{
  char dd_name[8];    /* DD name for log file */
  char log_path[256]; /* Log file path */
  int min_level;      /* Minimum log level */
  int initialized;    /* Initialization flag */
  int use_wto;        /* Whether to use WTO for logging */
  int use_dd;         /* Whether DD is available for file I/O */
  IO_CTRL *log_io;    /* BSAM I/O control block for log DD */
} g_metal_logger = {0};

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
  snprintf(buffer, buffer_size, "%016llX", clock_value);
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
  WTO_BUF wto_buf = {0};

  /* Format WTO message with level prefix */
  const char *level_str = (level >= ZLOGLEVEL_TRACE && level <= ZLOGLEVEL_OFF)
                              ? g_level_strings[level]
                              : "UNKNOWN";

  wto_buf.len = snprintf(wto_buf.msg, sizeof(wto_buf.msg),
                         "ZOWEX %s: %.100s", level_str, message);

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
  if (!g_metal_logger.log_io)
  {
    /* DD not available or not opened */
    return -1;
  }

  /* Additional safety checks for Metal C */
  if (!message)
  {
    return -1;
  }

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

  /* Write using BSAM synchronous write with additional safety check */
  int write_rc = writeSync(g_metal_logger.log_io, g_output_record);

  return (write_rc == 0) ? 0 : -1;
}

int ZLGINIT(const char *log_file_path, int min_level)
{
  if (!log_file_path)
  {
    return -1;
  }

  /* Prevent double initialization which could cause S0C4 */
  if (g_metal_logger.initialized)
  {
    return 0; /* Already initialized */
  }

  /* Initialize Metal C logger state with proper bounds checking */
  size_t path_len = strlen(log_file_path);
  if (path_len >= sizeof(g_metal_logger.log_path))
  {
    path_len = sizeof(g_metal_logger.log_path) - 1;
  }
  memset(g_metal_logger.log_path, 0, sizeof(g_metal_logger.log_path));
  strncpy(g_metal_logger.log_path, log_file_path, path_len);
  g_metal_logger.log_path[path_len] = '\0';

  g_metal_logger.min_level = min_level;

  /* In Metal C, we'll use WTO as primary logging mechanism */
  g_metal_logger.use_wto = 1;

  /* Try to set up DD for file logging using BSAM */
  memset(g_metal_logger.dd_name, 0, sizeof(g_metal_logger.dd_name));
  strncpy(g_metal_logger.dd_name, "ZWXLOGDD", sizeof(g_metal_logger.dd_name) - 1);

  /* Attempt to open the DD for output using BSAM */
  /* Initialize to NULL before attempting open */
  g_metal_logger.log_io = NULL;

  /* Try to open DD - open_output_assert should return NULL on failure */
  g_metal_logger.log_io = open_output_assert("ZWXLOGDD", 132, 132, dcbrecf + dcbrecbr);

  if (g_metal_logger.log_io)
  {
    g_metal_logger.use_dd = 1;
    /* Log success via DD itself - but only after confirming it works */
    ZLGWRDD(ZLOGLEVEL_INFO, "Metal C logger DD opened successfully");
  }
  else
  {
    g_metal_logger.use_dd = 0;
    /* Log failure via WTO */
    ZLGWRWTO(ZLOGLEVEL_WARN, "Metal C logger DD open failed, using WTO only");
  }

  /* Set initialized flag only after everything is properly set up */
  g_metal_logger.initialized = 1;

  /* Log initialization message */
  if (g_metal_logger.use_dd)
  {
    ZLGWRDD(ZLOGLEVEL_INFO, "Metal C logger initialized with DD support");
  }
  else
  {
    ZLGWRWTO(ZLOGLEVEL_INFO, "Metal C logger initialized with WTO only");
  }

  return 0;
}

int ZLGWRITE(int level, const char *message)
{
  char formatted_msg[ZLOG_MAX_MSG];
  char timestamp[32];
  const char *level_str;
  int result = 0;

  if (!g_metal_logger.initialized || !message)
  {
    return -1;
  }

  /* Check if we should log this level */
  if (level < g_metal_logger.min_level || level == ZLOGLEVEL_OFF)
  {
    return 0;
  }

  /* Get timestamp and level string */
  ZLGTIME(timestamp, sizeof(timestamp));
  level_str = (level >= ZLOGLEVEL_TRACE && level <= ZLOGLEVEL_OFF)
                  ? g_level_strings[level]
                  : "UNKNOWN";

  /* Format complete message */
  snprintf(formatted_msg, sizeof(formatted_msg), "[%s] [%s] %s",
           timestamp, level_str, message);

  /* Try DD writing first if available */
  if (g_metal_logger.use_dd)
  {
    result = ZLGWRDD(level, formatted_msg);
    if (result == 0)
    {
      return 0; /* Successfully wrote to DD */
    }
  }

  /* Fall back to WTO */
  if (g_metal_logger.use_wto)
  {
    result = ZLGWRWTO(level, formatted_msg);
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

  return ZLGWRITE(level, formatted_msg);
}

void ZLGSTLVL(int level)
{
  if (g_metal_logger.initialized)
  {
    g_metal_logger.min_level = level;
    /* Log level change */
    ZLGWRWTO(ZLOGLEVEL_INFO, "Log level changed");
  }
}

int ZLGGTLVL(void)
{
  if (g_metal_logger.initialized)
  {
    return g_metal_logger.min_level;
  }
  return ZLOGLEVEL_OFF;
}

void ZLGCLEAN(void)
{
  if (g_metal_logger.initialized)
  {
    /* Log cleanup message before closing */
    if (g_metal_logger.use_dd && g_metal_logger.log_io)
    {
      ZLGWRDD(ZLOGLEVEL_INFO, "Metal C logger cleanup - closing DD");
    }
    else
    {
      ZLGWRWTO(ZLOGLEVEL_INFO, "Metal C logger cleanup");
    }

#if defined(__IBM_METAL__)
    /* Close the DD if it was opened */
    if (g_metal_logger.log_io)
    {
      close_assert(g_metal_logger.log_io);
      g_metal_logger.log_io = NULL;
    }
#endif

    /* Reset state */
    memset(&g_metal_logger, 0, sizeof(g_metal_logger));
  }
}