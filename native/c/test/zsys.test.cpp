/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 */

#include "ztest.hpp"
#include "../zsys.hpp"
#include "../zjb.hpp"
#include "../zjbm.h"
#include <cstring>

using namespace ztst;

static ZSyslogRequest fixed_request()
{
  struct tm clock = {};
  clock.tm_year = 2024 - 1900;
  clock.tm_mon = 2;
  clock.tm_mday = 1;
  clock.tm_sec = 10;
  clock.tm_isdst = -1;
  ZSyslogRequest request;
  request.has_now = true;
  request.now = mktime(&clock);
  return request;
}

void zsys_tests()
{
  describe("zsys resolution", []()
  {
    it("resolves the default across leap-day midnight from one clock snapshot", []()
    {
      ZSyslogOptions opts;
      std::string error;
      Expect(zsys_resolve_syslog(fixed_request(), opts, error)).ToBe(0);
      Expect(opts.date).ToBe("2024-02-29");
      Expect(opts.time).ToBe("23:59:40");
      Expect(opts.max_lines).ToBe(10000);
      Expect(error.empty()).ToBe(true);
    });
    it("resolves partial absolute options and preserves permissive CLI parsing", []()
    {
      auto request = fixed_request();
      ZSyslogOptions opts;
      std::string error;
      request.date = "2024-02-31";
      Expect(zsys_resolve_syslog(request, opts, error)).ToBe(0);
      Expect(opts.date).ToBe(request.date);
      Expect(opts.time).ToBe("00:00:10");
      Expect(opts.max_lines).ToBe(100);
      request.date.clear();
      request.time = "1:2:3suffix";
      Expect(zsys_resolve_syslog(request, opts, error)).ToBe(0);
      Expect(opts.date).ToBe("2024-03-01");
      Expect(opts.time).ToBe(request.time);
      request.seconds_ago = -2;
      Expect(zsys_resolve_syslog(request, opts, error)).ToBe(0);
    });
    it("enforces relative bounds, explicit limits, and option exclusion", []()
    {
      auto request = fixed_request();
      ZSyslogOptions opts;
      std::string error;
      request.seconds_ago = 86400;
      request.has_max_lines = true;
      request.max_lines = 1;
      Expect(zsys_resolve_syslog(request, opts, error)).ToBe(0);
      Expect(opts.date).ToBe("2024-02-29");
      Expect(opts.time).ToBe("00:00:10");
      Expect(opts.max_lines).ToBe(1);
      request.max_lines = 0;
      Expect(zsys_resolve_syslog(request, opts, error)).ToBe(-1);
      Expect(error).ToContain("invalid max-lines");
      request.max_lines = 10000;
      request.seconds_ago = 0;
      Expect(zsys_resolve_syslog(request, opts, error)).ToBe(-1);
      request.seconds_ago = 86401;
      Expect(zsys_resolve_syslog(request, opts, error)).ToBe(-1);
      request.seconds_ago = 1;
      request.date = "2024-03-01";
      Expect(zsys_resolve_syslog(request, opts, error)).ToBe(-1);
      Expect(error).ToContain("mutually exclusive");
    });
    it("rejects the same invalid date and time ranges as the CLI", []()
    {
      auto request = fixed_request();
      ZSyslogOptions opts;
      std::string error;
      for (const auto *value : {"24:00:00", "12:00:00.01", "12:00", "12:00:00:00"})
      {
        request.time = value;
        Expect(zsys_resolve_syslog(request, opts, error)).ToBe(-1);
      }
      request.time.clear();
      for (const auto *value : {"1899-01-01", "2186-01-01", "2024-13-01", "2024-01-32"})
      {
        request.date = value;
        Expect(zsys_resolve_syslog(request, opts, error)).ToBe(-1);
      }
    });
  });
  describe("zsys PROCLIB capacity", []()
  {
    it("accepts exactly 100 entries and rejects entry 101 without touching the guard", []()
    {
      char buffer[MAX_DSN_ENTRY_SIZE + DSN_ENTRY_SIZE];
      memset(buffer, '?', sizeof(buffer));
      char dsn[DSN_ENTRY_SIZE];
      memset(dsn, 'A', sizeof(dsn));
      int entries = 0;
      for (int i = 0; i < MAX_DSN_ENTRIES; ++i)
        Expect(zjb_append_proclib(buffer, &entries, dsn)).ToBe(0);
      Expect(entries).ToBe(MAX_DSN_ENTRIES);
      Expect(zjb_append_proclib(buffer, &entries, dsn)).ToBe(-1);
      Expect(entries).ToBe(MAX_DSN_ENTRIES);
      Expect(std::string(buffer + MAX_DSN_ENTRY_SIZE, DSN_ENTRY_SIZE)).ToBe(std::string(DSN_ENTRY_SIZE, '?'));
      Expect(std::string(buffer + MAX_DSN_ENTRY_SIZE - DSN_ENTRY_SIZE, DSN_ENTRY_SIZE)).ToBe(std::string(DSN_ENTRY_SIZE, 'A'));
    });
  });
  describe("zsys compatibility", []()
  {
    it("preserves PROCLIB legacy results and append behavior", []()
    {
      ZDIAG diag = {};
      ZJB job = {};
      std::vector<std::string> current{"existing"}, legacy{"existing"};
      const int rc = zsys_list_proclib(diag, current);
      Expect(zjb_list_proclib(&job, legacy)).ToBe(rc);
      Expect(current == legacy).ToBe(true);
      Expect(current.front()).ToBe("existing");
      Expect(job.diag.detail_rc).ToBe(diag.detail_rc);
      Expect(job.diag.service_rc).ToBe(diag.service_rc);
      Expect(std::string(job.diag.e_msg)).ToBe(std::string(diag.e_msg));
    });
    it("preserves legacy SYSLOG buffer reuse, metadata, and diagnostics", []()
    {
      ZSyslogRequest request;
      request.seconds_ago = 3600;
      request.has_max_lines = true;
      request.max_lines = 1;
      ZSyslogOptions opts;
      std::string error;
      Expect(zsys_resolve_syslog(request, opts, error)).ToBe(0);
      ZJBSyslogOptions old_opts;
      old_opts.date = opts.date;
      old_opts.time = opts.time;
      old_opts.max_lines = opts.max_lines;
      ZSyslogResponse response;
      ZJBSyslogResponse old_response;
      response.data = old_response.data = "existing";
      ZDIAG diag = {};
      ZJB job = {};
      const int rc = zsys_read_syslog(diag, opts, response);
      Expect(zjb_read_syslog(&job, old_opts, old_response)).ToBe(rc);
      Expect(old_response.data).ToBe(response.data);
      Expect(old_response.has_more).ToBe(response.has_more);
      Expect(old_response.returned_lines).ToBe(response.returned_lines);
      Expect(old_response.end_date).ToBe(response.end_date);
      Expect(old_response.end_time).ToBe(response.end_time);
      Expect(job.diag.service_rc).ToBe(diag.service_rc);
      Expect(job.diag.detail_rc).ToBe(diag.detail_rc);
    });
  });
}
