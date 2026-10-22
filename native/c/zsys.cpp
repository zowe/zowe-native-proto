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

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <sstream>
#include <exception>
#include "zsys.hpp"
#include "zds.hpp"
#include "zjbm.h"
#include "zut.hpp"
#include "ihapsa.h"
#include "cvt.h"

int zsys_list_proclib(ZDIAG &diag, std::vector<std::string> &proclib)
try
{
  int rc = 0;
  ZJB zjb = {};
  zjb.diag = diag;
  int entries = 0;
  char buffer[MAX_DSN_ENTRY_SIZE] = {0};
  int buffer_size = sizeof(buffer);
  rc = ZJBMLPRC(&zjb, buffer, &buffer_size, &entries);
  diag = zjb.diag;

  if (RTNCD_SUCCESS == rc)
  {
    for (int i = 0; i < entries; i++)
    {
      std::string dsn(buffer + (i * DSN_ENTRY_SIZE), DSN_ENTRY_SIZE);
      proclib.push_back(dsn);
    }
  }

  return rc;
}

catch (...)
{
  ZDIAG_SET_MSG(&diag, "Could not allocate PROCLIB result storage");
  return RTNCD_FAILURE;
}

namespace
{
struct SpoolGuard
{
  ZDIAG &diag;
  const std::string &ddname;
  bool active = true;
  ~SpoolGuard()
  {
    if (active)
      zspool_free(diag, ddname);
  }
};
} // namespace

int zsys_read_syslog(ZDIAG &diag, const ZSyslogOptions &opts, ZSyslogResponse &response)
try
{
  int rc = 0;
  std::string ddname;
  ZDS zds = {};
  response.has_more = false;
  response.end_date.clear();
  response.end_time.clear();
  response.returned_lines = 0;

  //
  // get system name
  //
  struct psa *psa = (struct psa *)0;
  struct cvt *cvt = (struct cvt *)psa->flccvt;
  char *sysname_char = (char *)cvt->cvtsname;
  std::string sysname_str = std::string(sysname_char, sizeof(cvt->cvtsname));
  std::string dsn = zut_rtrim(sysname_str) + ".SYSLOG" + ".SYSTEM"; // https://www.ibm.com/docs/en/zos/3.1.0?topic=allocation-specifying-data-set-name-daldsnam

  //
  // get input timestamp from format HH:MM:SS.CC to binary format (low bit represents 0.01 of a second) for CONVTOD
  //
  int hh = 0, mm = 0, ss = 0, cs = 0;
  sscanf(opts.time.c_str(), "%d:%d:%d.%d", &hh, &mm, &ss, &cs);
  uint32_t ts_binary = ((uint32_t)hh * 360000) + ((uint32_t)mm * 6000) + ((uint32_t)ss * 100) + (uint32_t)cs;

  //
  // subtract to convert local time -> UTC for CONVTOD (which uses default offset=0/UTC)
  //
  int32_t tz_offset_cs = (int32_t)((double)cvt->cvttz * 1.048576 / 0.01); // documented in cvt.h
  ts_binary -= (uint32_t)tz_offset_cs;
  memcpy(&zds.ts_binary, &ts_binary, sizeof(ts_binary)); // low half is unused

  //
  // prepare date for `pack`
  //
  std::string date_compact;
  for (char c : opts.date)
  {
    if (c != '-')
      date_compact += c;
  }
  memcpy(&zds.ebcdic_date, date_compact.c_str(), sizeof(zds.ebcdic_date));
  zds.max_lines = opts.max_lines;

  ZSpoolOptions allocation = opts.allocation;
  allocation.dsname = dsn;
  rc = zspool_allocate(diag, allocation, ddname);
  if (0 != rc)
  {
    return rc;
  }

  SpoolGuard spool{diag, ddname};
  zds.encoding_opts.data_type = opts.encoding_opts.data_type;
  memcpy((void *)&zds.encoding_opts.codepage, (const void *)&opts.encoding_opts.codepage, sizeof(opts.encoding_opts.codepage));

  rc = zds_read_vsam(&zds, ddname, response.data);
  memcpy(&diag, &zds.diag, sizeof(ZDIAG));
  response.has_more = (zds.has_more != 0);
  response.returned_lines = zds.returned_lines;

  if (rc == 0 && !response.data.empty())
  {
    // Undo UTC->local in signed arithmetic; uint32 wraparound doesn't align to a day boundary.
    const int32_t CS_PER_DAY = 24 * 360000;
    uint32_t utc_cs = 0;
    memcpy(&utc_cs, &zds.ts_binary, sizeof(utc_cs));
    int32_t local_cs = (int32_t)utc_cs + tz_offset_cs;

    // Normalize into [0, CS_PER_DAY) and track midnight crossings for the date adjustment.
    int day_shift = 0;
    while (local_cs < 0)
    {
      local_cs += CS_PER_DAY;
      day_shift--;
    }
    while (local_cs >= CS_PER_DAY)
    {
      local_cs -= CS_PER_DAY;
      day_shift++;
    }

    unsigned end_hh = local_cs / 360000;
    unsigned end_mm = (local_cs / 6000) % 60;
    unsigned end_ss = (local_cs / 100) % 60;

    char ebcdic_date_formatted[8 + 1 + 1 + 1] = {}; // date + hyphens + null
    if (day_shift != 0)
    {
      // mktime normalizes the out-of-range day across month/year/leap-year boundaries.
      int year = 0, month = 0, day = 0;
      sscanf((const char *)zds.ebcdic_date, "%4d%2d%2d", &year, &month, &day);
      struct tm t = {};
      t.tm_year = year - 1900;
      t.tm_mon = month - 1;
      t.tm_mday = day + day_shift;
      t.tm_hour = 12; // noon keeps any DST/boundary effects away from the date
      mktime(&t);
      snprintf(ebcdic_date_formatted, sizeof(ebcdic_date_formatted), "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    }
    else
    {
      snprintf(ebcdic_date_formatted, sizeof(ebcdic_date_formatted), "%.4s-%.2s-%.2s", zds.ebcdic_date, 4 + zds.ebcdic_date, 6 + zds.ebcdic_date);
    }
    response.end_date = std::string(ebcdic_date_formatted);
    // NOTE(Kelosky): centiseconds is not used for the end time because it cannot be used for input
    // unsigned end_cs = local_cs % 100U;
    char end_time_buf[16] = {};
    snprintf(end_time_buf, sizeof(end_time_buf), "%02u:%02u:%02u", end_hh, end_mm, end_ss);
    response.end_time = end_time_buf;
  }

  int newrc = zspool_free(diag, ddname);
  spool.active = false;
  if (0 != newrc && 0 == rc)
  {
    return newrc;
  }

  return rc;
}

catch (...)
{
  ZDIAG_SET_MSG(&diag, "Could not allocate SYSLOG result storage");
  return RTNCD_FAILURE;
}

#define MAX_LINES 10000
#define DEFAULT_MAX_LINES_ABSOLUTE 100
#define DEFAULT_MAX_LINES_RELATIVE MAX_LINES
#define DEFAULT_SECONDS_AGO 30

int zsys_resolve_syslog(const ZSyslogRequest &request, ZSyslogOptions &opts, std::string &error)
try
{
  std::ostringstream errors;
  error.clear();
  std::string time_value = request.time;
  std::string date_value = request.date;
  auto seconds_ago = request.seconds_ago;
  bool has_explicit_date = !date_value.empty();
  bool has_explicit_time = !time_value.empty();
  bool has_seconds_ago = (seconds_ago >= 0);

  // mutual exclusivity: --seconds-ago vs --date/--time
  if (has_seconds_ago && (has_explicit_date || has_explicit_time))
  {
    errors << "Error: --seconds-ago is mutually exclusive with --date and --time." << '\n';
    errors << "Use either --seconds-ago for relative time or --date/--time for absolute time." << '\n';
    error = errors.str();
    return RTNCD_FAILURE;
  }

  // pick the right default max-lines based on query mode
  bool using_relative_time = has_seconds_ago || (!has_explicit_date && !has_explicit_time);
  long long default_max_lines = using_relative_time ? DEFAULT_MAX_LINES_RELATIVE : DEFAULT_MAX_LINES_ABSOLUTE;
  auto max_lines = (request.has_max_lines ? request.max_lines : default_max_lines);

  if (max_lines < 1 || max_lines > MAX_LINES)
  {
    errors << "Error: invalid max-lines '" << max_lines << "', expected 1-" << MAX_LINES << '\n';
    error = errors.str();
    return RTNCD_FAILURE;
  }

  if (has_seconds_ago && (seconds_ago < 1 || seconds_ago > 86400))
  {
    errors << "Error: invalid seconds-ago '" << seconds_ago << "', expected 1-86400" << '\n';
    error = errors.str();
    return RTNCD_FAILURE;
  }

  time_t now = request.has_now ? request.now : time(nullptr);
  if (request.use_system_timezone)
  {
    struct psa *psa = (struct psa *)0;
    struct cvt *cvt = (struct cvt *)psa->flccvt;
    now += static_cast<time_t>(std::llround(static_cast<double>(cvt->cvttz) * 1.048576));
  }
  struct tm tm_now_storage;
  struct tm *tm_now = request.use_system_timezone ? gmtime_r(&now, &tm_now_storage) : localtime_r(&now, &tm_now_storage);
  if (tm_now == nullptr)
  {
    errors << "Error: could not obtain local time" << '\n';
    error = errors.str();
    return RTNCD_FAILURE;
  }
  char buf[32];

  // when no args provided, default to --seconds-ago 30
  if (!has_explicit_date && !has_explicit_time && !has_seconds_ago)
  {
    seconds_ago = DEFAULT_SECONDS_AGO;
    has_seconds_ago = true;
  }

  if (has_seconds_ago)
  {
    time_t start_time = now - seconds_ago;
    struct tm tm_start_storage;
    struct tm *tm_start = request.use_system_timezone ? gmtime_r(&start_time, &tm_start_storage) : localtime_r(&start_time, &tm_start_storage);
    if (tm_start == nullptr)
    {
      errors << "Error: could not compute start time" << '\n';
      error = errors.str();
      return RTNCD_FAILURE;
    }
    strftime(buf, sizeof(buf), "%Y-%m-%d", tm_start);
    date_value = buf;
    strftime(buf, sizeof(buf), "%H:%M:%S", tm_start);
    time_value = buf;
  }
  else
  {
    // resolve defaults and validate explicit values
    if (time_value.empty())
    {
      strftime(buf, sizeof(buf), "%H:%M:%S", tm_now);
      time_value = buf;
    }
    else
    {
      // Check for invalid formats first (too many colons, dots, etc.)
      size_t colon_count = std::count(time_value.begin(), time_value.end(), ':');
      size_t dot_count = std::count(time_value.begin(), time_value.end(), '.');

      if (colon_count != 2 || dot_count > 1)
      {
        errors << "Error: invalid time format '" << time_value << "', expected hh:mm:ss e.g. 12:01:30" << '\n';
        error = errors.str();
        return RTNCD_FAILURE;
      }

      int hh = -1, mm = -1, ss = -1, cs = -1;
      int parsed = sscanf(time_value.c_str(), "%d:%d:%d.%d", &hh, &mm, &ss, &cs);

      // Reject centiseconds format since it's not used in output
      if (parsed == 4)
      {
        errors << "Error: centiseconds not supported, use hh:mm:ss format e.g. 12:01:30" << '\n';
        error = errors.str();
        return RTNCD_FAILURE;
      }

      // Validate HH:MM:SS format
      if (parsed != 3 || hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59)
      {
        errors << "Error: invalid time '" << time_value << "', expected hh:mm:ss e.g. 12:01:30" << '\n';
        error = errors.str();
        return RTNCD_FAILURE;
      }
    }

    if (date_value.empty())
    {
      strftime(buf, sizeof(buf), "%Y-%m-%d", tm_now);
      date_value = buf;
    }
    else
    {
      int yyyy = -1, mm = -1, dd = -1;
      int parsed = sscanf(date_value.c_str(), "%d-%d-%d", &yyyy, &mm, &dd);
      if (parsed != 3 || yyyy < 1900 || yyyy > 2185 || mm < 1 || mm > 12 || dd < 1 || dd > 31)
      {
        errors << "Error: invalid date '" << date_value << "', expected yyyy-mm-dd e.g. 2026-03-13" << '\n';
        error = errors.str();
        return RTNCD_FAILURE;
      }
    }
  }

  opts.date = date_value;
  opts.time = time_value;
  opts.max_lines = static_cast<int>(max_lines);
  return RTNCD_SUCCESS;
}
catch (...)
{
  error.clear();
  return RTNCD_FAILURE;
}
