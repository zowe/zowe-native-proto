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

#include "zlogger_metal.h"

#ifndef __IBM_METAL__
/* Include the full C logger core when not in Metal C */
#include "zlogger_core.h"
#endif

#ifdef __IBM_METAL__

/* Metal C implementation - simplified for Metal C environment */

int zlog_metal_init(const char *log_file_path, int min_level)
{
  /* In Metal C, we have limited capabilities */
  /* For now, this would need to interface with the C core through */
  /* cross-memory services or other z/OS mechanisms */

  /* This is a placeholder - full implementation would require */
  /* setting up communication with the non-Metal C logger */
  return 0;
}

int zlog_metal_write(int level, const char *message)
{
  /* Metal C logging implementation */
  /* This could write directly to a dataset or use WTO */
  /* For now, this is a placeholder */
  return 0;
}

int zlog_metal_write_formatted(int level, const char *prefix, const char *message, const char *suffix)
{
  char formatted_msg[ZLOG_METAL_MAX_MSG];
  int pos = 0;

  if (!message)
  {
    return -1;
  }

  /* Simple string concatenation since Metal C has limited printf */
  if (prefix)
  {
    int prefix_len = strlen(prefix);
    if (pos + prefix_len < ZLOG_METAL_MAX_MSG - 1)
    {
      strcpy(formatted_msg + pos, prefix);
      pos += prefix_len;
      if (pos < ZLOG_METAL_MAX_MSG - 2)
      {
        formatted_msg[pos] = ' ';
        pos++;
      }
    }
  }

  int msg_len = strlen(message);
  if (pos + msg_len < ZLOG_METAL_MAX_MSG - 1)
  {
    strcpy(formatted_msg + pos, message);
    pos += msg_len;
  }

  if (suffix && pos < ZLOG_METAL_MAX_MSG - 1)
  {
    if (pos < ZLOG_METAL_MAX_MSG - 2)
    {
      formatted_msg[pos] = ' ';
      pos++;
    }
    int suffix_len = strlen(suffix);
    if (pos + suffix_len < ZLOG_METAL_MAX_MSG - 1)
    {
      strcpy(formatted_msg + pos, suffix);
      pos += suffix_len;
    }
  }

  formatted_msg[pos] = '\0';

  return zlog_metal_write(level, formatted_msg);
}

void zlog_metal_set_level(int level)
{
  /* Set level in Metal C context */
  /* Placeholder implementation */
}

int zlog_metal_get_level(void)
{
  /* Get level in Metal C context */
  /* Placeholder implementation */
  return ZLOG_METAL_INFO;
}

void zlog_metal_cleanup(void)
{
  /* Cleanup in Metal C context */
  /* Placeholder implementation */
}

#else

/* Non-Metal C implementation - bridges to the C core */

int zlog_metal_init(const char *log_file_path, int min_level)
{
  /* Convert Metal C level to core level */
  zlog_level_t core_level = (zlog_level_t)min_level;
  return zlog_init(log_file_path, core_level);
}

int zlog_metal_write(int level, const char *message)
{
  /* Convert Metal C level to core level and write */
  zlog_level_t core_level = (zlog_level_t)level;
  return zlog_write_msg(core_level, message);
}

int zlog_metal_write_formatted(int level, const char *prefix, const char *message, const char *suffix)
{
  char formatted_msg[ZLOG_METAL_MAX_MSG];
  int pos = 0;

  if (!message)
  {
    return -1;
  }

  /* Format the message */
  if (prefix)
  {
    int prefix_len = strlen(prefix);
    if (pos + prefix_len < ZLOG_METAL_MAX_MSG - 1)
    {
      strcpy(formatted_msg + pos, prefix);
      pos += prefix_len;
      if (pos < ZLOG_METAL_MAX_MSG - 2)
      {
        formatted_msg[pos] = ' ';
        pos++;
      }
    }
  }

  int msg_len = strlen(message);
  if (pos + msg_len < ZLOG_METAL_MAX_MSG - 1)
  {
    strcpy(formatted_msg + pos, message);
    pos += msg_len;
  }

  if (suffix && pos < ZLOG_METAL_MAX_MSG - 1)
  {
    if (pos < ZLOG_METAL_MAX_MSG - 2)
    {
      formatted_msg[pos] = ' ';
      pos++;
    }
    int suffix_len = strlen(suffix);
    if (pos + suffix_len < ZLOG_METAL_MAX_MSG - 1)
    {
      strcpy(formatted_msg + pos, suffix);
      pos += suffix_len;
    }
  }

  formatted_msg[pos] = '\0';

  return zlog_metal_write(level, formatted_msg);
}

void zlog_metal_set_level(int level)
{
  zlog_level_t core_level = (zlog_level_t)level;
  zlog_set_level(core_level);
}

int zlog_metal_get_level(void)
{
  return (int)zlog_get_level();
}

void zlog_metal_cleanup(void)
{
  zlog_cleanup();
}

#endif /* __IBM_METAL__ */