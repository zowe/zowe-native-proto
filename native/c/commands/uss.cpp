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

#include "uss.hpp"
#include "../parser.hpp"
#include "../zusf.hpp"
#include "../zut.hpp"
#include <unistd.h>

using namespace parser;
using namespace std;

namespace uss
{

int handle_uss_create_file(const ParseResult &result)
{
  int rc = 0;
  string file_path = result.get_value<std::string>("file-path", "");

  long long mode = result.get_value<long long>("mode", 0);
  if (result.get_value<std::string>("mode", "").empty())
  {
    mode = 644;
  }
  else if (mode == 0 && result.get_value<std::string>("mode", "") != "0")
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
  string file_path = result.get_value<std::string>("file-path", "");

  long long mode = result.get_value<long long>("mode", 0);
  if (result.get_value<std::string>("mode", "").empty())
  {
    mode = 755;
  }
  else if (mode == 0 && result.get_value<std::string>("mode", "") != "0")
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
  string uss_file = result.get_value<std::string>("file-path", "");

  ListOptions list_options = {0};
  list_options.all_files = result.get_value<bool>("all", false);
  list_options.long_format = result.get_value<bool>("long", false);

  const auto use_csv_format = result.get_value<bool>("response-format-csv", false);

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
  string uss_file = result.get_value<std::string>("file-path", "");

  ZUSF zusf = {0};
  if (result.has("encoding"))
  {
    zut_prepare_encoding(result.get_value<std::string>("encoding", ""), &zusf.encoding_opts);
  }
  if (result.has("local-encoding"))
  {
    const auto source_encoding = result.get_value<std::string>("local-encoding", "");
    if (!source_encoding.empty() && source_encoding.size() < sizeof(zusf.encoding_opts.source_codepage))
    {
      memcpy(zusf.encoding_opts.source_codepage, source_encoding.data(), source_encoding.length() + 1);
    }
  }

  struct stat file_stats;
  if (stat(uss_file.c_str(), &file_stats) == -1)
  {
    cerr << "Error: Path " << uss_file << " does not exist" << endl;
    return RTNCD_FAILURE;
  }

  bool has_pipe_path = result.has("pipe-path");
  string pipe_path = result.get_value<std::string>("pipe-path", "");

  if (has_pipe_path && !pipe_path.empty())
  {
    size_t content_len = 0;
    rc = zusf_read_from_uss_file_streamed(&zusf, uss_file, pipe_path, &content_len);

    if (result.get_value<bool>("return-etag", false))
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

    if (result.get_value<bool>("return-etag", false))
    {
      cout << "etag: " << zut_build_etag(file_stats.st_mtime, file_stats.st_size) << endl;
      cout << "data: ";
    }

    bool has_encoding = result.has("encoding");
    bool response_format_bytes = result.get_value<bool>("response-format-bytes", false);

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
  string file = result.get_value<std::string>("file-path", "");
  ZUSF zusf = {0};

  if (result.has("encoding"))
  {
    zut_prepare_encoding(result.get_value<std::string>("encoding", ""), &zusf.encoding_opts);
  }
  if (result.has("local-encoding"))
  {
    const auto source_encoding = result.get_value<std::string>("local-encoding", "");
    if (!source_encoding.empty() && source_encoding.size() < sizeof(zusf.encoding_opts.source_codepage))
    {
      memcpy(zusf.encoding_opts.source_codepage, source_encoding.data(), source_encoding.length() + 1);
    }
  }

  if (result.has("etag"))
  {
    string etag_value = result.get_value<std::string>("etag", "");
    if (!etag_value.empty())
    {
      strcpy(zusf.etag, etag_value.c_str());
    }
  }

  bool has_pipe_path = result.has("pipe-path");
  string pipe_path = result.get_value<std::string>("pipe-path", "");
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

  if (result.get_value<bool>("etag-only", false))
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
  string file_path = result.get_value<std::string>("file-path", "");
  bool recursive = result.get_value<bool>("recursive", false);

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
  long long mode = result.get_value<long long>("mode", 0);
  if (mode == 0 && !result.get_value<std::string>("mode", "").empty())
  {
    cerr << "Error: invalid mode provided.\nExamples of valid modes: 777, 0644" << endl;
    return RTNCD_FAILURE;
  }

  string file_path = result.get_value<std::string>("file-path", "");
  bool recursive = result.get_value<bool>("recursive", false);

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
  string path = result.get_value<std::string>("file-path", "");
  string owner = result.get_value<std::string>("owner", "");
  bool recursive = result.get_value<bool>("recursive", false);

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
  string path = result.get_value<std::string>("file-path", "");
  string tag = result.get_value<std::string>("tag", "");
  if (tag.empty())
  {
    tag = zut_int_to_string(result.get_value<long long>("tag", 0));
  }

  if (tag.empty())
  {
    cerr << "Error: no tag provided" << endl;
    return RTNCD_FAILURE;
  }

  bool recursive = result.get_value<bool>("recursive", false);

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

void register_commands(parser::Command &root_command)
{
  auto response_format_csv_option = make_aliases("--response-format-csv", "--rfc");

  // USS command group
  auto uss_group = command_ptr(new Command("uss", "z/OS USS operations"));

  // Common encoding/etag/pipe-path option helpers (reuse from data-set group)
  auto uss_encoding_option = make_aliases("--encoding", "--ec");
  auto uss_source_encoding_option = make_aliases("--local-encoding", "--lec");
  auto uss_etag_option = make_aliases("--etag");
  auto uss_etag_only_option = make_aliases("--etag-only");
  auto uss_return_etag_option = make_aliases("--return-etag");
  auto uss_pipe_path_option = make_aliases("--pipe-path");
  auto uss_response_format_bytes_option = make_aliases("--response-format-bytes", "--rfb");

  // Create-file subcommand
  auto uss_create_file_cmd = command_ptr(new Command("create-file", "create a USS file"));
  uss_create_file_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_create_file_cmd->add_keyword_arg("mode", make_aliases("--mode"), "permissions", ArgType_Single, false);
  uss_create_file_cmd->set_handler(handle_uss_create_file);
  uss_group->add_command(uss_create_file_cmd);

  // Create-dir subcommand
  auto uss_create_dir_cmd = command_ptr(new Command("create-dir", "create a USS directory"));
  uss_create_dir_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_create_dir_cmd->add_keyword_arg("mode", make_aliases("--mode"), "permissions", ArgType_Single, false);
  uss_create_dir_cmd->set_handler(handle_uss_create_dir);
  uss_group->add_command(uss_create_dir_cmd);

  // List subcommand
  auto uss_list_cmd = command_ptr(new Command("list", "list USS files and directories"));
  uss_list_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_list_cmd->add_keyword_arg("all", make_aliases("--all", "-a"), "list all files and directories", ArgType_Flag, false, ArgValue(false));
  uss_list_cmd->add_keyword_arg("long", make_aliases("--long", "-l"), "list long format", ArgType_Flag, false, ArgValue(false));
  uss_list_cmd->add_keyword_arg("response-format-csv", response_format_csv_option, "returns the response in CSV format", ArgType_Flag, false, ArgValue(false));
  uss_list_cmd->set_handler(handle_uss_list);
  uss_group->add_command(uss_list_cmd);

  // View subcommand
  auto uss_view_cmd = command_ptr(new Command("view", "view a USS file"));
  uss_view_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_view_cmd->add_keyword_arg("encoding", uss_encoding_option, "return contents in given encoding", ArgType_Single, false);
  uss_view_cmd->add_keyword_arg("local-encoding", uss_source_encoding_option, "source encoding of the USS file content (defaults to UTF-8)", ArgType_Single, false);
  uss_view_cmd->add_keyword_arg("response-format-bytes", uss_response_format_bytes_option, "returns the response as raw bytes", ArgType_Flag, false, ArgValue(false));
  uss_view_cmd->add_keyword_arg("return-etag", uss_return_etag_option, "Display the e-tag for a read response in addition to data", ArgType_Flag, false, ArgValue(false));
  uss_view_cmd->add_keyword_arg("pipe-path", uss_pipe_path_option, "Specify a FIFO pipe path for transferring binary data", ArgType_Single, false);
  uss_view_cmd->set_handler(handle_uss_view);
  uss_group->add_command(uss_view_cmd);

  // Write subcommand
  auto uss_write_cmd = command_ptr(new Command("write", "write to a USS file"));
  uss_write_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_write_cmd->add_keyword_arg("encoding", uss_encoding_option, "encoding for input data", ArgType_Single, false);
  uss_write_cmd->add_keyword_arg("local-encoding", uss_source_encoding_option, "source encoding of the input data (defaults to UTF-8)", ArgType_Single, false);
  uss_write_cmd->add_keyword_arg("etag", uss_etag_option, "Provide the e-tag for a write response to detect conflicts before save", ArgType_Single, false);
  uss_write_cmd->add_keyword_arg("etag-only", uss_etag_only_option, "Only print the e-tag for a write response (when successful)", ArgType_Flag, false, ArgValue(false));
  uss_write_cmd->add_keyword_arg("pipe-path", uss_pipe_path_option, "Specify a FIFO pipe path for transferring binary data", ArgType_Single, false);
  uss_write_cmd->set_handler(handle_uss_write);
  uss_group->add_command(uss_write_cmd);

  // Delete subcommand
  auto uss_delete_cmd = command_ptr(new Command("delete", "delete a USS item"));
  uss_delete_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_delete_cmd->add_keyword_arg("recursive", make_aliases("--recursive", "-r"), "Applies the operation recursively (e.g. for folders w/ inner files)", ArgType_Flag, false, ArgValue(false));
  uss_delete_cmd->set_handler(handle_uss_delete);
  uss_group->add_command(uss_delete_cmd);

  // Chmod subcommand
  auto uss_chmod_cmd = command_ptr(new Command("chmod", "change permissions on a USS file or directory"));
  uss_chmod_cmd->add_positional_arg("mode", "new permissions for the file or directory", ArgType_Single, true);
  uss_chmod_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_chmod_cmd->add_keyword_arg("recursive", make_aliases("--recursive", "-r"), "Applies the operation recursively (e.g. for folders w/ inner files)", ArgType_Flag, false, ArgValue(false));
  uss_chmod_cmd->set_handler(handle_uss_chmod);
  uss_group->add_command(uss_chmod_cmd);

  // Chown subcommand
  auto uss_chown_cmd = command_ptr(new Command("chown", "change owner on a USS file or directory"));
  uss_chown_cmd->add_positional_arg("owner", "New owner (or owner:group) for the file or directory", ArgType_Single, true);
  uss_chown_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_chown_cmd->add_keyword_arg("recursive", make_aliases("--recursive", "-r"), "Applies the operation recursively (e.g. for folders w/ inner files)", ArgType_Flag, false, ArgValue(false));
  uss_chown_cmd->set_handler(handle_uss_chown);
  uss_group->add_command(uss_chown_cmd);

  // Chtag subcommand
  auto uss_chtag_cmd = command_ptr(new Command("chtag", "change tags on a USS file"));
  uss_chtag_cmd->add_positional_arg("file-path", "file path", ArgType_Single, true);
  uss_chtag_cmd->add_positional_arg("tag", "new tag for the file", ArgType_Single, true);
  uss_chtag_cmd->add_keyword_arg("recursive", make_aliases("--recursive", "-r"), "Applies the operation recursively (e.g. for folders w/ inner files)", ArgType_Flag, false, ArgValue(false));
  uss_chtag_cmd->set_handler(handle_uss_chtag);
  uss_group->add_command(uss_chtag_cmd);

  root_command.add_command(uss_group);
}
} // namespace uss