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

#include "ds.hpp"
#include "common_args.hpp"
#include "../zds.hpp"
#include "../zut.hpp"
#include <string>
#include <vector>
#include <unistd.h>

using namespace ast;
using namespace parser;
using namespace std;
using namespace commands::common;

namespace ds
{
int process_data_set_create_result(InvocationContext &context, ZDS *zds, int rc, string dsn, string response)
{
  if (0 != rc)
  {
    context.error_stream() << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details:\n"
                           << response << endl;
    return RTNCD_FAILURE;
  }

  // Handle member creation if specified
  size_t start = dsn.find_first_of('(');
  size_t end = dsn.find_last_of(')');
  if (start != string::npos && end != string::npos && end > start)
  {
    string member_name = dsn.substr(start + 1, end - start - 1);
    string data = "";
    rc = zds_write_to_dsn(zds, dsn, data);
    if (0 != rc)
    {
      context.output_stream() << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
      context.output_stream() << "  Details: " << zds->diag.e_msg << endl;
      return RTNCD_FAILURE;
    }
    context.output_stream() << "Data set and/or member created: '" << dsn << "'" << endl;
  }
  else
  {
    context.output_stream() << "Data set created: '" << dsn << "'" << endl;
  }

  return rc;
}

int create_with_attributes(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  DS_ATTRIBUTES attributes = {0};

  // Extract all the optional creation attributes
  if (context.has("alcunit"))
  {
    attributes.alcunit = context.get<string>("alcunit", "");
  }
  if (context.has("blksize"))
  {
    attributes.blksize = context.get<long long>("blksize", 0);
  }
  if (context.has("dirblk"))
  {
    attributes.dirblk = context.get<long long>("dirblk", 0);
  }
  if (context.has("dsorg"))
  {
    attributes.dsorg = context.get<string>("dsorg", "");
  }
  if (context.has("primary"))
  {
    attributes.primary = context.get<long long>("primary", 0);
  }
  if (context.has("recfm"))
  {
    attributes.recfm = context.get<string>("recfm", "");
  }
  if (context.has("lrecl"))
  {
    attributes.lrecl = context.get<long long>("lrecl", 0);
  }
  if (context.has("dataclass"))
  {
    attributes.dataclass = context.get<string>("dataclass", "");
  }
  if (context.has("unit"))
  {
    attributes.unit = context.get<string>("unit", "");
  }
  if (context.has("dsntype"))
  {
    attributes.dsntype = context.get<string>("dsntype", "");
  }
  if (context.has("mgntclass"))
  {
    attributes.mgntclass = context.get<string>("mgntclass", "");
  }
  if (context.has("dsname"))
  {
    attributes.dsname = context.get<string>("dsname", "");
  }
  if (context.has("avgblk"))
  {
    attributes.avgblk = context.get<long long>("avgblk", 0);
  }
  if (context.has("secondary"))
  {
    attributes.secondary = context.get<long long>("secondary", 0);
  }
  if (context.has("size"))
  {
    attributes.size = context.get<long long>("size", 0);
  }
  if (context.has("storclass"))
  {
    attributes.storclass = context.get<string>("storclass", "");
  }
  if (context.has("vol"))
  {
    attributes.vol = context.get<string>("vol", "");
  }

  string response;
  rc = zds_create_dsn(&zds, dsn, attributes, response);
  return process_data_set_create_result(context, &zds, rc, dsn, response);
}

int handle_data_set_create_fb(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_fb(&zds, dsn, response);
  return process_data_set_create_result(context, &zds, rc, dsn, response);
}

int handle_data_set_create_vb(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_vb(&zds, dsn, response);
  return process_data_set_create_result(context, &zds, rc, dsn, response);
}

int handle_data_set_create_adata(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_adata(&zds, dsn, response);
  return process_data_set_create_result(context, &zds, rc, dsn, response);
}

int handle_data_set_create_loadlib(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_loadlib(&zds, dsn, response);
  return process_data_set_create_result(context, &zds, rc, dsn, response);
}

int handle_data_set_create_member(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  string response;
  vector<ZDSEntry> entries;

  size_t start = dsn.find_first_of('(');
  size_t end = dsn.find_last_of(')');
  string member_name;
  if (start != string::npos && end != string::npos && end > start)
  {
    member_name = dsn.substr(start + 1, end - start - 1);
    string dataset_name = dsn.substr(0, start);

    rc = zds_list_data_sets(&zds, dataset_name, entries);
    if (RTNCD_WARNING < rc || entries.size() == 0)
    {
      context.output_stream() << "Error: could not create data set member: '" << dataset_name << "' rc: '" << rc << "'" << endl;
      context.output_stream() << "  Details:\n"
                              << zds.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }

    string data = "";
    rc = zds_write_to_dsn(&zds, dsn, data);
    if (0 != rc)
    {
      context.output_stream() << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
      context.output_stream() << "  Details: " << zds.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }
    context.output_stream() << "Data set and/or member created: '" << dsn << "'" << endl;
  }
  else
  {
    context.output_stream() << "Error: could not find member name in dsn: '" << dsn << "'" << endl;
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_data_set_view(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  vector<string> dds;

  if (context.has("encoding"))
  {
    zut_prepare_encoding(context.get<string>("encoding", ""), &zds.encoding_opts);
  }
  if (context.has("local-encoding"))
  {
    const auto source_encoding = context.get<string>("local-encoding", "");
    if (!source_encoding.empty() && source_encoding.size() < sizeof(zds.encoding_opts.source_codepage))
    {
      memcpy(zds.encoding_opts.source_codepage, source_encoding.data(), source_encoding.length() + 1);
    }
  }

  if (context.has("volser"))
  {
    string volser_value = context.get<string>("volser", "");
    if (!volser_value.empty())
    {
      dds.push_back("alloc dd(input) da('" + dsn + "') shr vol(" + volser_value + ")");
      rc = zut_loop_dynalloc(dds);
      if (0 != rc)
      {
        return RTNCD_FAILURE;
      }
      strcpy(zds.ddname, "INPUT");
    }
  }

  bool has_pipe_path = context.has("pipe-path");
  string pipe_path = context.get<string>("pipe-path", "");
  const auto result = obj();
  result->set("dataset", str(dsn));

  if (has_pipe_path && !pipe_path.empty())
  {
    size_t content_len = 0;
    rc = zds_read_from_dsn_streamed(&zds, dsn, pipe_path, &content_len);

    if (context.get<bool>("return-etag", false))
    {
      string temp_content;
      auto read_rc = zds_read_from_dsn(&zds, dsn, temp_content);
      if (read_rc == 0)
      {
        const auto etag = zut_calc_adler32_checksum(temp_content);
        stringstream etag_stream;
        etag_stream << hex << etag << dec;
        if (!context.is_redirecting_output())
        {
          context.output_stream() << "etag: " << etag_stream.str() << endl;
        }
        else
        {
          result->set("etag", str(etag_stream.str()));
        }
      }
    }

    if (!context.is_redirecting_output())
    {
      context.output_stream() << "size: " << content_len << endl;
    }
    result->set("contentLen", i64(content_len));
  }
  else
  {
    string response;
    rc = zds_read_from_dsn(&zds, dsn, response);
    if (0 != rc)
    {
      context.error_stream() << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
      context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }

    if (context.get<bool>("return-etag", false))
    {
      const auto etag = zut_calc_adler32_checksum(response);
      stringstream etag_stream;
      etag_stream << hex << etag << dec;
      if (!context.is_redirecting_output())
      {
        context.output_stream() << "etag: " << etag_stream.str() << endl;
        context.output_stream() << "data: ";
      }
      else
      {
        result->set("etag", str(etag_stream.str()));
      }
    }

    bool has_encoding = context.has("encoding");
    bool response_format_bytes = context.get<bool>("response-format-bytes", false);

    if (has_encoding && response_format_bytes)
    {
      zut_print_string_as_bytes(response, &context.output_stream());
    }
    else
    {
      context.output_stream() << response;
    }

    context.set_object(result);
  }

  if (dds.size() > 0)
  {
    zut_free_dynalloc_dds(dds, &context.error_stream());
  }

  return rc;
}

int handle_data_set_list(InvocationContext &context)
{
  int rc = 0;
  ZLOG_DEBUG("[>] handle_data_set_list");
  string dsn = context.get<string>("dsn", "");

  if (dsn.length() > MAX_DS_LENGTH)
  {
    context.error_stream() << "Error: data set pattern exceeds 44 character length limit" << endl;
    return RTNCD_FAILURE;
  }

  dsn += ".**";

  long long max_entries = context.get<long long>("max-entries", 0);
  bool warn = context.get<bool>("warn", true);
  bool attributes = context.get<bool>("attributes", false);

  ZDS zds = {0};
  if (max_entries > 0)
  {
    zds.max_entries = max_entries;
  }
  vector<ZDSEntry> entries;

  bool emit_csv = context.get<bool>("response-format-csv", false);
  rc = zds_list_data_sets(&zds, dsn, entries);
  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    vector<string> fields;
    fields.reserve(attributes ? 5 : 1);
    const auto entries_array = arr();

    for (vector<ZDSEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
    {
      if (emit_csv)
      {
        fields.push_back(it->name);
        if (attributes)
        {
          fields.push_back(it->dsorg);
          fields.push_back(it->volser);
          fields.push_back(it->migr ? "true" : "false");
          fields.push_back(it->recfm);
        }
        context.output_stream() << zut_format_as_csv(fields) << endl;
        fields.clear();
      }
      else
      {
        if (attributes)
        {
          context.output_stream() << left << setw(44) << it->name << " " << it->volser << " " << setw(4) << it->dsorg << " " << setw(6) << it->recfm << endl;
        }
        else
        {
          context.output_stream() << left << setw(44) << it->name << endl;
        }
      }

      const auto entry = obj();
      string trimmed_name = it->name;
      zut_rtrim(trimmed_name);
      entry->set("name", str(trimmed_name));
      entry->set("dsorg", str(it->dsorg));
      entry->set("volser", str(it->volser));
      entry->set("migr", boolean(it->migr));
      entry->set("recfm", str(it->recfm));
      entries_array->push(entry);
    }

    const auto result = obj();
    result->set("items", entries_array);
    result->set("returnedRows", i64(entries.size()));
    context.set_object(result);
  }
  if (RTNCD_WARNING == rc)
  {
    if (warn)
    {
      if (ZDS_RSNCD_MAXED_ENTRIES_REACHED == zds.diag.detail_rc)
      {
        context.error_stream() << "Warning: results truncated" << endl;
      }
      else if (ZDS_RSNCD_NOT_FOUND == zds.diag.detail_rc)
      {
        context.error_stream() << "Warning: no matching results found" << endl;
      }
    }
  }

  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    context.error_stream() << "Error: could not list data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return (!warn && rc == RTNCD_WARNING) ? RTNCD_SUCCESS : rc;
}

int handle_data_set_list_members(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  long long max_entries = context.get<long long>("max-entries", 0);
  bool warn = context.get<bool>("warn", true);

  ZDS zds = {0};
  if (max_entries > 0)
  {
    zds.max_entries = max_entries;
  }
  vector<ZDSMem> members;
  rc = zds_list_members(&zds, dsn, members);

  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    const auto entries_array = arr();
    for (vector<ZDSMem>::iterator it = members.begin(); it != members.end(); ++it)
    {
      context.output_stream() << left << setw(12) << it->name << endl;
      const auto entry = obj();
      string trimmed_name = it->name;
      zut_rtrim(trimmed_name);
      entry->set("name", str(trimmed_name));
      entries_array->push(entry);
    }
    const auto result = obj();
    result->set("items", entries_array);
    result->set("returnedRows", i64(members.size()));
    context.set_object(result);
  }
  if (RTNCD_WARNING == rc)
  {
    if (warn)
    {
      if (ZDS_RSNCD_MAXED_ENTRIES_REACHED == zds.diag.detail_rc)
      {
        context.error_stream() << "Warning: results truncated" << endl;
      }
    }
  }
  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    context.error_stream() << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return (!warn && rc == RTNCD_WARNING) ? RTNCD_SUCCESS : rc;
}

int handle_data_set_write(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  vector<string> dds;

  if (context.has("encoding"))
  {
    zut_prepare_encoding(context.get<string>("encoding", ""), &zds.encoding_opts);
  }
  if (context.has("local-encoding"))
  {
    const auto source_encoding = context.get<string>("local-encoding", "");
    if (!source_encoding.empty() && source_encoding.size() < sizeof(zds.encoding_opts.source_codepage))
    {
      memcpy(zds.encoding_opts.source_codepage, source_encoding.data(), source_encoding.length() + 1);
    }
  }

  if (context.has("etag"))
  {
    string etag_value = context.get<string>("etag", "");
    if (!etag_value.empty())
    {
      strcpy(zds.etag, etag_value.c_str());
    }
  }

  if (context.has("volser"))
  {
    string volser_value = context.get<string>("volser", "");
    if (!volser_value.empty())
    {
      dds.push_back("alloc dd(output) da('" + dsn + "') shr vol(" + volser_value + ")");
      rc = zut_loop_dynalloc(dds);
      if (0 != rc)
      {
        return RTNCD_FAILURE;
      }
      strcpy(zds.ddname, "OUTPUT");
    }
  }

  bool has_pipe_path = context.has("pipe-path");
  string pipe_path = context.get<string>("pipe-path", "");
  size_t content_len = 0;
  const auto result = obj();

  if (has_pipe_path && !pipe_path.empty())
  {
    rc = zds_write_to_dsn_streamed(&zds, dsn, pipe_path, &content_len);
    result->set("contentLen", i64(content_len));
  }
  else
  {
    string data;
    string line;

    if (!isatty(fileno(stdout)))
    {
      istreambuf_iterator<char> begin(context.input_stream());
      istreambuf_iterator<char> end;

      if (!context.is_redirecting_input())
      {
        vector<char> input(begin, end);
        const auto temp = string(input.begin(), input.end());
        input.clear();
        const auto bytes = zut_get_contents_as_bytes(temp);

        data.assign(bytes.begin(), bytes.end());
      }
      else
      {
        data.assign(begin, end);
      }
    }
    else
    {
      while (getline(context.input_stream(), line))
      {
        data += line;
        data.push_back('\n');
      }
    }

    rc = zds_write_to_dsn(&zds, dsn, data);
  }

  if (dds.size() > 0)
  {
    zut_free_dynalloc_dds(dds, &context.error_stream());
  }

  if (0 != rc)
  {
    context.error_stream() << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (context.get<bool>("etag-only", false))
  {
    context.output_stream() << "etag: " << zds.etag << endl;
    if (content_len > 0)
      context.output_stream() << "size: " << content_len << endl;
  }
  else
  {
    context.output_stream() << "Wrote data to '" << dsn << "'" << endl;
  }

  result->set("dataset", str(dsn));
  result->set("etag", str(zds.etag));
  context.set_object(result);

  return rc;
}

int handle_data_set_delete(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  rc = zds_delete_dsn(&zds, dsn);

  if (0 != rc)
  {
    context.error_stream() << "Error: could not delete data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  context.output_stream() << "Data set '" << dsn << "' deleted" << endl;

  return rc;
}

int handle_data_set_restore(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");
  ZDS zds = {0};
  string response;
  unsigned int code = 0;

  // perform dynalloc
  vector<string> dds;
  dds.reserve(2);
  dds.push_back("alloc da('" + dsn + "') shr");
  dds.push_back("free da('" + dsn + "')");

  rc = zut_loop_dynalloc(dds);
  if (0 != rc)
  {
    return RTNCD_FAILURE;
  }

  context.output_stream() << "Data set '" << dsn << "' restored" << endl;

  return rc;
}

int handle_data_set_compress(InvocationContext &context)
{
  int rc = 0;
  string dsn = context.get<string>("dsn", "");

  transform(dsn.begin(), dsn.end(), dsn.begin(), ::toupper);

  bool is_pds = false;

  string dsn_formatted = "//'" + dsn + "'";
  FILE *dir = fopen(dsn_formatted.c_str(), "r");
  if (dir)
  {
    fldata_t file_info = {0};
    char file_name[64] = {0};
    if (0 == fldata(dir, file_name, &file_info))
    {
      if (file_info.__dsorgPDSdir && !file_info.__dsorgPDSE)
      {
        is_pds = true;
      }
      fclose(dir);
    }
  }

  if (!is_pds)
  {
    context.error_stream() << "Error: data set'" << dsn << "' is not a PDS'" << endl;
    return RTNCD_FAILURE;
  }

  // perform dynalloc
  vector<string> dds;
  dds.reserve(4);
  dds.push_back("alloc dd(a) da('" + dsn + "') shr");
  dds.push_back("alloc dd(b) da('" + dsn + "') shr");
  dds.push_back("alloc dd(sysprint) lrecl(80) recfm(f,b) blksize(80)");
  dds.push_back("alloc dd(sysin) lrecl(80) recfm(f,b) blksize(80)");

  rc = zut_loop_dynalloc(dds);
  if (0 != rc)
  {
    return RTNCD_FAILURE;
  }

  // write control statements
  ZDS zds = {0};
  zds_write_to_dd(&zds, "sysin", "        COPY OUTDD=B,INDD=A");
  if (0 != rc)
  {
    context.error_stream() << "Error: could not write to dd: '" << "sysin" << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // perform compress
  rc = zut_run("IEBCOPY");
  if (RTNCD_SUCCESS != rc)
  {
    context.error_stream() << "Error: could error invoking IEBCOPY rc: '" << rc << "'" << endl;
  }

  // read output from iebcopy
  string output;
  rc = zds_read_from_dd(&zds, "sysprint", output);
  if (0 != rc)
  {
    context.error_stream() << "Error: could not read from dd: '" << "sysprint" << "' rc: '" << rc << "'" << endl;
    context.error_stream() << "  Details: " << zds.diag.e_msg << endl;
    context.error_stream() << output << endl;
    return RTNCD_FAILURE;
  }
  context.output_stream() << "Data set '" << dsn << "' compressed" << endl;

  // free dynalloc dds
  zut_free_dynalloc_dds(dds, &context.error_stream());

  return RTNCD_SUCCESS;
}

void register_commands(parser::Command &root_command)
{
  // Data set command group
  auto data_set_cmd = command_ptr(new Command("data-set", "z/OS data set operations"));
  data_set_cmd->add_alias("ds");

  // Create subcommand
  auto ds_create_cmd = command_ptr(new Command("create", "create data set"));
  ds_create_cmd->add_alias("cre");
  ds_create_cmd->add_positional_arg(DSN);

  // Data set creation attributes
  ds_create_cmd->add_keyword_arg("alcunit", make_aliases("--alcunit"), "Allocation unit", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("blksize", make_aliases("--blksize"), "Block size", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dirblk", make_aliases("--dirblk"), "Directory blocks", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dsorg", make_aliases("--dsorg"), "Data set organization", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("primary", make_aliases("--primary"), "Primary space", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("recfm", make_aliases("--recfm"), "Record format", ArgType_Single, false, ArgValue(std::string("FB")));
  ds_create_cmd->add_keyword_arg("lrecl", make_aliases("--lrecl"), "Record length", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dataclass", make_aliases("--dataclass"), "Data class", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("unit", make_aliases("--unit"), "Device type", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dsntype", make_aliases("--dsntype"), "Data set type", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("mgntclass", make_aliases("--mgntclass"), "Management class", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("dsname", make_aliases("--dsname"), "Data set name", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("avgblk", make_aliases("--avgblk"), "Average block length", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("secondary", make_aliases("--secondary"), "Secondary space", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("size", make_aliases("--size"), "Size", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("storclass", make_aliases("--storclass"), "Storage class", ArgType_Single, false);
  ds_create_cmd->add_keyword_arg("vol", make_aliases("--vol"), "Volume serial", ArgType_Single, false);
  ds_create_cmd->set_handler(create_with_attributes);
  data_set_cmd->add_command(ds_create_cmd);

  // Create-fb subcommand
  auto ds_create_fb_cmd = command_ptr(new Command("create-fb", "create FB data set using defaults: DSORG=PO, RECFM=FB, LRECL=80 "));
  ds_create_fb_cmd->add_alias("cre-fb");
  ds_create_fb_cmd->add_positional_arg(DSN);
  ds_create_fb_cmd->set_handler(handle_data_set_create_fb);
  data_set_cmd->add_command(ds_create_fb_cmd);

  // Create-vb subcommand
  auto ds_create_vb_cmd = command_ptr(new Command("create-vb", "create VB data set using defaults: DSORG=PO, RECFM=VB, LRECL=255"));
  ds_create_vb_cmd->add_alias("cre-vb");
  ds_create_vb_cmd->add_positional_arg(DSN);
  ds_create_vb_cmd->set_handler(handle_data_set_create_vb);
  data_set_cmd->add_command(ds_create_vb_cmd);

  // Create-adata subcommand
  auto ds_create_adata_cmd = command_ptr(new Command("create-adata", "create VB data set using defaults: DSORG=PO, RECFM=VB, LRECL=32756"));
  ds_create_adata_cmd->add_alias("cre-a");
  ds_create_adata_cmd->add_positional_arg(DSN);
  ds_create_adata_cmd->set_handler(handle_data_set_create_adata);
  data_set_cmd->add_command(ds_create_adata_cmd);

  // Create-loadlib subcommand
  auto ds_create_loadlib_cmd = command_ptr(new Command("create-loadlib", "create loadlib data set using defaults: DSORG=PO, RECFM=U, LRECL=0"));
  ds_create_loadlib_cmd->add_alias("cre-u");
  ds_create_loadlib_cmd->add_positional_arg(DSN);
  ds_create_loadlib_cmd->set_handler(handle_data_set_create_loadlib);
  data_set_cmd->add_command(ds_create_loadlib_cmd);

  // Create-member subcommand
  auto ds_create_member_cmd = command_ptr(new Command("create-member", "create member in data set"));
  ds_create_member_cmd->add_alias("cre-m");
  ds_create_member_cmd->add_positional_arg("dsn", "data set name with member specified", ArgType_Single, true);
  ds_create_member_cmd->set_handler(handle_data_set_create_member);
  data_set_cmd->add_command(ds_create_member_cmd);

  // View subcommand
  auto ds_view_cmd = command_ptr(new Command("view", "view data set"));
  ds_view_cmd->add_positional_arg(DSN);
  ds_view_cmd->add_keyword_arg(ENCODING);
  ds_view_cmd->add_keyword_arg(LOCAL_ENCODING);
  ds_view_cmd->add_keyword_arg(RESPONSE_FORMAT_BYTES);
  ds_view_cmd->add_keyword_arg(RETURN_ETAG);
  ds_view_cmd->add_keyword_arg(PIPE_PATH);
  ds_view_cmd->add_keyword_arg(VOLSER);
  ds_view_cmd->set_handler(handle_data_set_view);
  data_set_cmd->add_command(ds_view_cmd);

  // List subcommand
  auto ds_list_cmd = command_ptr(new Command("list", "list data sets"));
  ds_list_cmd->add_alias("ls");
  ds_list_cmd->add_positional_arg(DSN_PATTERN);
  ds_list_cmd->add_keyword_arg("attributes", make_aliases("--attributes", "-a"), "display data set attributes", ArgType_Flag, false, ArgValue(false));
  ds_list_cmd->add_keyword_arg(MAX_ENTRIES);
  ds_list_cmd->add_keyword_arg(WARN);
  ds_list_cmd->add_keyword_arg(RESPONSE_FORMAT_CSV);
  ds_list_cmd->set_handler(handle_data_set_list);
  ds_list_cmd->add_example("List SYS1.* with all attributes", "zowex ds ls 'sys1.*' -a");
  data_set_cmd->add_command(ds_list_cmd);

  // List-members subcommand
  auto ds_list_members_cmd = command_ptr(new Command("list-members", "list data set members"));
  ds_list_members_cmd->add_alias("lm");
  ds_list_members_cmd->add_positional_arg(DSN);
  ds_list_members_cmd->add_keyword_arg(MAX_ENTRIES);
  ds_list_members_cmd->add_keyword_arg(WARN);
  ds_list_members_cmd->set_handler(handle_data_set_list_members);
  data_set_cmd->add_command(ds_list_members_cmd);

  // Write subcommand
  auto ds_write_cmd = command_ptr(new Command("write", "write to data set"));
  ds_write_cmd->add_positional_arg(DSN);
  ds_write_cmd->add_keyword_arg(ENCODING);
  ds_write_cmd->add_keyword_arg(LOCAL_ENCODING);
  ds_write_cmd->add_keyword_arg(ETAG);
  ds_write_cmd->add_keyword_arg(ETAG_ONLY);
  ds_write_cmd->add_keyword_arg(PIPE_PATH);
  ds_write_cmd->add_keyword_arg(VOLSER);
  ds_write_cmd->set_handler(handle_data_set_write);
  data_set_cmd->add_command(ds_write_cmd);

  // Delete subcommand
  auto ds_delete_cmd = command_ptr(new Command("delete", "delete data set"));
  ds_delete_cmd->add_alias("del");
  ds_delete_cmd->add_positional_arg(DSN);
  ds_delete_cmd->set_handler(handle_data_set_delete);
  data_set_cmd->add_command(ds_delete_cmd);

  // Restore subcommand
  auto ds_restore_cmd = command_ptr(new Command("restore", "restore/recall data set"));
  ds_restore_cmd->add_positional_arg(DSN);
  ds_restore_cmd->set_handler(handle_data_set_restore);
  data_set_cmd->add_command(ds_restore_cmd);

  // Compress subcommand
  auto ds_compress_cmd = command_ptr(new Command("compress", "compress data set"));
  ds_compress_cmd->add_positional_arg("dsn", "data set to compress", ArgType_Single, true);
  ds_compress_cmd->set_handler(handle_data_set_compress);
  data_set_cmd->add_command(ds_compress_cmd);

  root_command.add_command(data_set_cmd);
}
} // namespace ds