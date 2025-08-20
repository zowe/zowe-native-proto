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
#include "zusf.hpp"
#include "zut.hpp"
#include "zcli.hpp"

using namespace parser;
using namespace std;

int handle_uss_create_file(const ParseResult &result)
{
  int rc = 0;
  string file_path = result.find_pos_arg_string("file-path");

  int mode = result.find_kw_arg_int("mode");
  if (result.find_kw_arg_string("mode").empty())
  {
    mode = 644;
  }
  else if (mode == 0 && result.find_kw_arg_string("mode") != "0")
  {
    cerr << "Error: invalid mode provided.\nExamples of valid modes: 777, 0644" << endl;
    return RTNCD_FAILURE;
  }

  // Convert mode from decimal to octal
  mode_t cf_mode = 0;
  int temp_mode = mode;
  int multiplier = 1;

  // Convert decimal representation of octal to actual octal value
  // e.g. user inputs 777 -> converted to correct value for chmod
  while (temp_mode > 0)
  {
    cf_mode += (temp_mode % 10) * multiplier;
    temp_mode /= 10;
    multiplier *= 8;
  }

  ZUSF zusf = {0};
  rc = zusf_create_uss_file_or_dir(&zusf, file_path, cf_mode, false);
  if (0 != rc)
  {
    cerr << "Error: could not create USS file: '" << file_path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS file '" << file_path << "' created" << endl;

  return rc;
}

int handle_uss_create_dir(const ParseResult &result)
{
  int rc = 0;
  string file_path = result.find_pos_arg_string("file-path");

  int mode = result.find_kw_arg_int("mode");
  if (result.find_kw_arg_string("mode").empty())
  {
    mode = 755;
  }
  else if (mode == 0 && result.find_kw_arg_string("mode") != "0")
  {
    cerr << "Error: invalid mode provided.\nExamples of valid modes: 777, 0644" << endl;
    return RTNCD_FAILURE;
  }

  // Convert mode from decimal to octal
  mode_t cf_mode = 0;
  int temp_mode = mode;
  int multiplier = 1;

  // Convert decimal representation of octal to actual octal value
  // e.g. user inputs 777 -> converted to correct value for chmod
  while (temp_mode > 0)
  {
    cf_mode += (temp_mode % 10) * multiplier;
    temp_mode /= 10;
    multiplier *= 8;
  }

  ZUSF zusf = {0};
  rc = zusf_create_uss_file_or_dir(&zusf, file_path, cf_mode, true);
  if (0 != rc)
  {
    cerr << "Error: could not create USS directory: '" << file_path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS directory '" << file_path << "' created" << endl;

  return rc;
}

int handle_uss_list(const ParseResult &result)
{
  int rc = 0;
  string uss_file = result.find_pos_arg_string("file-path");

  ListOptions list_options = {0};
  list_options.all_files = result.find_kw_arg_bool("all");
  list_options.long_format = result.find_kw_arg_bool("long");

  const auto use_csv_format = result.find_kw_arg_bool("response-format-csv");

  ZUSF zusf = {0};
  string response;
  rc = zusf_list_uss_file_path(&zusf, uss_file, response, list_options, use_csv_format);
  if (0 != rc)
  {
    cerr << "Error: could not list USS files: '" << uss_file << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl
         << response << endl;
    return RTNCD_FAILURE;
  }

  cout << response;

  return rc;
}

int handle_uss_view(const ParseResult &result)
{
  int rc = 0;
  string uss_file = result.find_pos_arg_string("file-path");

  ZUSF zusf = {0};
  if (result.has_kw_arg("encoding"))
  {
    zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zusf.encoding_opts);
  }

  struct stat file_stats;
  if (stat(uss_file.c_str(), &file_stats) == -1)
  {
    cerr << "Error: Path " << uss_file << " does not exist" << endl;
    return RTNCD_FAILURE;
  }

  bool has_pipe_path = result.has_kw_arg("pipe-path");
  string pipe_path = result.find_kw_arg_string("pipe-path");

  if (has_pipe_path && !pipe_path.empty())
  {
    size_t content_len = 0;
    rc = zusf_read_from_uss_file_streamed(&zusf, uss_file, pipe_path, &content_len);

    if (result.find_kw_arg_bool("return-etag"))
    {
      cout << "etag: " << zut_build_etag(file_stats.st_mtime, file_stats.st_size) << endl;
    }
    cout << "size: " << content_len << endl;
  }
  else
  {
    string response;
    rc = zusf_read_from_uss_file(&zusf, uss_file, response);
    if (0 != rc)
    {
      cerr << "Error: could not view USS file: '" << uss_file << "' rc: '" << rc << "'" << endl;
      cerr << "  Details:\n"
           << zusf.diag.e_msg << endl
           << response << endl;
      return RTNCD_FAILURE;
    }

    if (result.find_kw_arg_bool("return-etag"))
    {
      cout << "etag: " << zut_build_etag(file_stats.st_mtime, file_stats.st_size) << endl;
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

int handle_uss_write(const ParseResult &result)
{
  int rc = 0;
  string file = result.find_pos_arg_string("file-path");
  ZUSF zusf = {0};

  if (result.has_kw_arg("encoding"))
  {
    zut_prepare_encoding(result.find_kw_arg_string("encoding"), &zusf.encoding_opts);
  }

  if (result.has_kw_arg("etag"))
  {
    string etag_value = result.find_kw_arg_string("etag");
    if (!etag_value.empty())
    {
      strcpy(zusf.etag, etag_value.c_str());
    }
  }

  bool has_pipe_path = result.has_kw_arg("pipe-path");
  string pipe_path = result.find_kw_arg_string("pipe-path");
  size_t content_len = 0;

  if (has_pipe_path && !pipe_path.empty())
  {
    rc = zusf_write_to_uss_file_streamed(&zusf, file, pipe_path, &content_len);
  }
  else
  {
    string data = "";
    string line = "";

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
    rc = zusf_write_to_uss_file(&zusf, file, data);
  }

  if (0 != rc)
  {
    cerr << "Error: could not write to USS file: '" << file << "' rc: '" << rc << "'" << endl;
    cerr << "  Details: " << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  if (result.find_kw_arg_bool("etag-only"))
  {
    cout << "etag: " << zusf.etag << endl
         << "created: " << (zusf.created ? "true" : "false") << endl;
    if (content_len > 0)
      cout << "size: " << content_len << endl;
  }
  else
  {
    cout << "Wrote data to '" << file << "'" << (zusf.created ? " (created new file)" : " (overwrote existing)") << endl;
  }

  return rc;
}

int handle_uss_delete(const ParseResult &result)
{
  string file_path = result.find_pos_arg_string("file-path");
  bool recursive = result.find_kw_arg_bool("recursive");

  ZUSF zusf = {0};
  const auto rc = zusf_delete_uss_item(&zusf, file_path, recursive);

  if (0 != rc)
  {
    cerr << "Failed to delete USS item " << file_path << ":\n " << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS item '" << file_path << "' deleted" << endl;

  return rc;
}

int handle_uss_chmod(const ParseResult &result)
{
  int rc = 0;
  int mode = result.find_pos_arg_int("mode");
  if (mode == 0 && !result.find_pos_arg_string("mode").empty())
  {
    cerr << "Error: invalid mode provided.\nExamples of valid modes: 777, 0644" << endl;
    return RTNCD_FAILURE;
  }

  string file_path = result.find_pos_arg_string("file-path");
  bool recursive = result.find_kw_arg_bool("recursive");

  // Convert mode from decimal to octal
  mode_t chmod_mode = 0;
  int temp_mode = mode;
  int multiplier = 1;

  // Convert decimal representation of octal to actual octal value
  // e.g. user inputs 777 -> converted to correct value for chmod
  while (temp_mode > 0)
  {
    chmod_mode += (temp_mode % 10) * multiplier;
    temp_mode /= 10;
    multiplier *= 8;
  }

  ZUSF zusf = {0};
  rc = zusf_chmod_uss_file_or_dir(&zusf, file_path, chmod_mode, recursive);
  if (0 != rc)
  {
    cerr << "Error: could not chmod USS path: '" << file_path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS path '" << file_path << "' modified: '" << mode << "'" << endl;

  return rc;
}

int handle_uss_chown(const ParseResult &result)
{
  string path = result.find_pos_arg_string("file-path");
  string owner = result.find_pos_arg_string("owner");
  bool recursive = result.find_kw_arg_bool("recursive");

  ZUSF zusf = {0};

  const auto rc = zusf_chown_uss_file_or_dir(&zusf, path, owner, recursive);
  if (0 != rc)
  {
    cerr << "Error: could not chown USS path: '" << path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS path '" << path << "' owner changed to '" << owner << "'" << endl;

  return rc;
}

int handle_uss_chtag(const ParseResult &result)
{
  string path = result.find_pos_arg_string("file-path");
  string tag = result.find_pos_arg_string("tag");
  if (tag.empty())
  {
    tag = zut_int_to_string(result.find_pos_arg_int("tag"));
  }

  if (tag.empty())
  {
    cerr << "Error: no tag provided" << endl;
    return RTNCD_FAILURE;
  }

  bool recursive = result.find_kw_arg_bool("recursive");

  ZUSF zusf = {0};
  const auto rc = zusf_chtag_uss_file_or_dir(&zusf, path, tag, recursive);

  if (0 != rc)
  {
    cerr << "Error: could not chtag USS path: '" << path << "' rc: '" << rc << "'" << endl;
    cerr << "  Details:\n"
         << zusf.diag.e_msg << endl;
    return RTNCD_FAILURE;
  }

  cout << "USS path '" << path << "' tag changed to '" << tag << "'" << endl;

  return rc;
}
