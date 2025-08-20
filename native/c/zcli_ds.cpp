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

#include "parser.hpp"
#include "zds.hpp"
#include "zut.hpp"
#include "zcli.hpp"

using namespace parser;
using namespace std;

int process_data_set_create_result(ZDS *zds, int rc, string dsn, string response)
{
  if (0 != rc)
  {
    cerr << "Error: could not create data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
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
      cout << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
      cout << "  Details: " << zds->diag.e_msg << endl;
      return RTNCD_FAILURE;
    }
    cout << "Data set and/or member created: '" << dsn << "'" << endl;
  }
  else
  {
    cout << "Data set created: '" << dsn << "'" << endl;
  }

  return rc;
}

int handle_data_set_create(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  DS_ATTRIBUTES attributes = {0};

  // Extract all the optional creation attributes
  if (result.has_kw_arg("alcunit"))
  {
    attributes.alcunit = result.find_kw_arg_string("alcunit");
  }
  if (result.has_kw_arg("blksize"))
  {
    string blksize_str = result.find_kw_arg_string("blksize");
    if (!blksize_str.empty())
    {
      attributes.blksize = std::strtoul(blksize_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("dirblk"))
  {
    string dirblk_str = result.find_kw_arg_string("dirblk");
    if (!dirblk_str.empty())
    {
      attributes.dirblk = std::strtoul(dirblk_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("dsorg"))
  {
    attributes.dsorg = result.find_kw_arg_string("dsorg");
  }
  if (result.has_kw_arg("primary"))
  {
    string primary_str = result.find_kw_arg_string("primary");
    if (!primary_str.empty())
    {
      attributes.primary = std::strtoul(primary_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("recfm"))
  {
    attributes.recfm = result.find_kw_arg_string("recfm");
  }
  if (result.has_kw_arg("lrecl"))
  {
    string lrecl_str = result.find_kw_arg_string("lrecl");
    if (!lrecl_str.empty())
    {
      attributes.lrecl = std::strtoul(lrecl_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("dataclass"))
  {
    attributes.dataclass = result.find_kw_arg_string("dataclass");
  }
  if (result.has_kw_arg("unit"))
  {
    attributes.unit = result.find_kw_arg_string("unit");
  }
  if (result.has_kw_arg("dsntype"))
  {
    attributes.dsntype = result.find_kw_arg_string("dsntype");
  }
  if (result.has_kw_arg("mgntclass"))
  {
    attributes.mgntclass = result.find_kw_arg_string("mgntclass");
  }
  if (result.has_kw_arg("dsname"))
  {
    attributes.dsname = result.find_kw_arg_string("dsname");
  }
  if (result.has_kw_arg("avgblk"))
  {
    string avgblk_str = result.find_kw_arg_string("avgblk");
    if (!avgblk_str.empty())
    {
      attributes.avgblk = std::strtoul(avgblk_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("secondary"))
  {
    string secondary_str = result.find_kw_arg_string("secondary");
    if (!secondary_str.empty())
    {
      attributes.secondary = std::strtoul(secondary_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("size"))
  {
    string size_str = result.find_kw_arg_string("size");
    if (!size_str.empty())
    {
      attributes.size = std::strtoul(size_str.c_str(), nullptr, 10);
    }
  }
  if (result.has_kw_arg("storclass"))
  {
    attributes.storclass = result.find_kw_arg_string("storclass");
  }
  if (result.has_kw_arg("vol"))
  {
    attributes.vol = result.find_kw_arg_string("vol");
  }

  string response;
  rc = zds_create_dsn(&zds, dsn, attributes, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_fb(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_fb(&zds, dsn, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_vb(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_vb(&zds, dsn, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_adata(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_adata(&zds, dsn, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_loadlib(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  rc = zds_create_dsn_loadlib(&zds, dsn, response);
  return process_data_set_create_result(&zds, rc, dsn, response);
}

int handle_data_set_create_member(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
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
      cout << "Error: could not create data set member: '" << dataset_name << "' rc: '" << rc << "'" << endl;
      cout << "  Details:\n"
           << zds.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }

    string data = "";
    rc = zds_write_to_dsn(&zds, dsn, data);
    if (0 != rc)
    {
      cout << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
      cout << "  Details: " << zds.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }
    cout << "Data set and/or member created: '" << dsn << "'" << endl;
  }
  else
  {
    cout << "Error: could not find member name in dsn: '" << dsn << "'" << endl;
    return RTNCD_FAILURE;
  }

  return rc;
}

int handle_data_set_view(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};

  if (result.has_kw_arg("encoding"))
  {
    zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zds.encoding_opts);
  }

  bool has_pipe_path = result.has_kw_arg("pipe-path");
  string pipe_path = result.find_kw_arg_string("pipe-path");

  if (has_pipe_path && !pipe_path.empty())
  {
    size_t content_len = 0;
    rc = zds_read_from_dsn_streamed(&zds, dsn, pipe_path, &content_len);

    if (result.find_kw_arg_bool("return-etag"))
    {
      string temp_content;
      auto read_rc = zds_read_from_dsn(&zds, dsn, temp_content);
      if (read_rc == 0)
      {
        const auto etag = zut_calc_adler32_checksum(temp_content);
        cout << "etag: " << std::hex << etag << std::dec << endl;
      }
      cout << "size: " << content_len << endl;
    }
  }
  else
  {
    string response;
    rc = zds_read_from_dsn(&zds, dsn, response);
    if (0 != rc)
    {
      cerr << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
      cerr << "  Details: " << zds.diag.e_msg << endl;
      return RTNCD_FAILURE;
    }

    if (result.find_kw_arg_bool("return-etag"))
    {
      const auto etag = zut_calc_adler32_checksum(response);
      cout << "etag: " << std::hex << etag << std::dec << endl;
      cout << "data: ";
    }

    bool has_encoding = result.has_kw_arg("encoding");
    bool response_format_bytes = result.find_kw_arg_bool("response-format-bytes");

    if (has_encoding && response_format_bytes)
    {
      zut_print_string_as_bytes(response);
    }
    else
    {
      cout << response << endl;
    }
  }

  return rc;
}

int handle_data_set_list(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

  if (dsn.length() > MAX_DS_LENGTH)
  {
    cerr << "Error: data set pattern exceeds 44 character length limit" << endl;
    return RTNCD_FAILURE;
  }

  dsn += ".**";

  int max_entries = result.find_kw_arg_int("max-entries");
  bool warn = result.find_kw_arg_bool("warn", true);
  bool attributes = result.find_kw_arg_bool("attributes");

  ZDS zds = {0};
  if (max_entries > 0)
  {
    zds.max_entries = max_entries;
  }
  vector<ZDSEntry> entries;

  bool emit_csv = result.find_kw_arg_bool("response-format-csv");
  rc = zds_list_data_sets(&zds, dsn, entries);
  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    vector<string> fields;
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
        cout << zut_format_as_csv(fields) << endl;
        fields.clear();
      }
      else
      {
        if (attributes)
        {
          cout << left << setw(44) << it->name << " " << it->volser << " " << setw(4) << it->dsorg << " " << setw(6) << it->recfm << endl;
        }
        else
        {
          cout << left << setw(44) << it->name << endl;
        }
      }
    }
  }
  if (RTNCD_WARNING == rc)
  {
    if (warn)
    {
      if (ZDS_RSNCD_MAXED_ENTRIES_REACHED == zds.diag.detail_rc)
      {
        cerr << "Warning: results truncated" << endl;
      }
      else if (ZDS_RSNCD_NOT_FOUND == zds.diag.detail_rc)
      {
        cerr << "Warning: no matching results found" << endl;
      }
    }
  }

  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    cerr << "Error: could not list data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return (!warn && rc == RTNCD_WARNING) ? RTNCD_SUCCESS : rc;
}

int handle_data_set_list_members(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  int max_entries = result.find_kw_arg_int("max-entries");
  bool warn = result.find_kw_arg_bool("warn", true);

  ZDS zds = {0};
  if (max_entries > 0)
  {
    zds.max_entries = max_entries;
  }
  vector<ZDSMem> members;
  rc = zds_list_members(&zds, dsn, members);

  if (RTNCD_SUCCESS == rc || RTNCD_WARNING == rc)
  {
    for (vector<ZDSMem>::iterator it = members.begin(); it != members.end(); ++it)
    {
      cout << left << setw(12) << it->name << endl;
    }
  }
  if (RTNCD_WARNING == rc)
  {
    if (warn)
    {
      if (ZDS_RSNCD_MAXED_ENTRIES_REACHED == zds.diag.detail_rc)
      {
        cerr << "Warning: results truncated" << endl;
      }
    }
  }
  if (RTNCD_SUCCESS != rc && RTNCD_WARNING != rc)
  {
    cerr << "Error: could not read data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  return (!warn && rc == RTNCD_WARNING) ? RTNCD_SUCCESS : rc;
}

int handle_data_set_write(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};

  if (result.has_kw_arg("encoding"))
  {
    zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zds.encoding_opts);
  }

  if (result.has_kw_arg("etag"))
  {
    string etag_value = result.find_kw_arg_string("etag");
    if (!etag_value.empty())
    {
      strcpy(zds.etag, etag_value.c_str());
    }
  }

  bool has_pipe_path = result.has_kw_arg("pipe-path");
  string pipe_path = result.find_kw_arg_string("pipe-path");
  size_t content_len = 0;

  if (has_pipe_path && !pipe_path.empty())
  {
    rc = zds_write_to_dsn_streamed(&zds, dsn, pipe_path, &content_len);
  }
  else
  {
    string data;
    string line;

    if (!isatty(fileno(stdout)))
    {
      std::istreambuf_iterator<char> begin(std::cin);
      std::istreambuf_iterator<char> end;

      vector<char> input(begin, end);
      const auto temp = string(input.begin(), input.end());
      input.clear();
      const auto bytes = zut_get_contents_as_bytes(temp);

      data.assign(bytes.begin(), bytes.end());
    }
    else
    {
      while (getline(cin, line))
      {
        data += line;
        data.push_back('\n');
      }
    }

    rc = zds_write_to_dsn(&zds, dsn, data);
  }

  if (0 != rc)
  {
    cerr << "Error: could not write to data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (result.find_kw_arg_bool("etag-only"))
  {
    cout << "etag: " << zds.etag << endl;
    if (content_len > 0)
      cout << "size: " << content_len << endl;
  }
  else
  {
    cout << "Wrote data to '" << dsn << "'" << endl;
  }

  return rc;
}

int handle_data_set_delete(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  rc = zds_delete_dsn(&zds, dsn);

  if (0 != rc)
  {
    cerr << "Error: could not delete data set: '" << dsn << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }
  cout << "Data set '" << dsn << "' deleted" << endl;

  return rc;
}

int handle_data_set_restore(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");
  ZDS zds = {0};
  string response;
  unsigned int code = 0;

  // perform dynalloc
  vector<string> dds;
  dds.push_back("alloc da('" + dsn + "') shr");
  dds.push_back("free da('" + dsn + "')");

  rc = loop_dynalloc(dds);
  if (0 != rc)
  {
    return RTNCD_FAILURE;
  }

  cout << "Data set '" << dsn << "' restored" << endl;

  return rc;
}

int handle_data_set_compress(const ParseResult &result)
{
  int rc = 0;
  string dsn = result.find_pos_arg_string("dsn");

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
    cerr << "Error: data set'" << dsn << "' is not a PDS'" << endl;
    return RTNCD_FAILURE;
  }

  // perform dynalloc
  vector<string> dds;
  dds.push_back("alloc dd(a) da('" + dsn + "') shr");
  dds.push_back("alloc dd(b) da('" + dsn + "') shr");
  dds.push_back("alloc dd(sysprint) lrecl(80) recfm(f,b) blksize(80)");
  dds.push_back("alloc dd(sysin) lrecl(80) recfm(f,b) blksize(80)");

  rc = loop_dynalloc(dds);
  if (0 != rc)
  {
    return RTNCD_FAILURE;
  }

  // write control statements
  ZDS zds = {0};
  zds_write_to_dd(&zds, "sysin", "        COPY OUTDD=B,INDD=A");
  if (0 != rc)
  {
    cerr << "Error: could not write to dd: '" << "sysin" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  // perform compress
  rc = zut_run("IEBCOPY");
  if (RTNCD_SUCCESS != rc)
  {
    cerr << "Error: could error invoking IEBCOPY rc: '" << rc << "'" << endl;
  }

  // read output from iebcopy
  string output;
  rc = zds_read_from_dd(&zds, "sysprint", output);
  if (0 != rc)
  {
    cerr << "Error: could not read from dd: '" << "sysprint" << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zds.diag.e_msg << endl;
    cerr << output << endl;
    return RTNCD_FAILURE;
  }
  cout << "Data set '" << dsn << "' compressed" << endl;

  // free dynalloc dds
  free_dynalloc_dds(dds);

  return RTNCD_SUCCESS;
}
