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

#include <string>
#include <vector>

#include "parser.test.hpp"
#include "ztest.hpp"
#include "../parser.hpp"

using namespace parser;
using namespace ztst;

namespace
{

std::vector<char *> to_argv(std::vector<std::string> &storage)
{
  std::vector<char *> argv;
  argv.reserve(storage.size());
  for (std::string &arg : storage)
  {
    argv.push_back(const_cast<char *>(arg.c_str()));
  }
  return argv;
}

bool g_handler_called = false;
std::string g_handler_arg;

int sample_handler(plugin::InvocationContext &context)
{
  g_handler_called = true;
  g_handler_arg = context.get<std::string>("name");
  return 7;
}

} // namespace

void parser_tests()
{
  describe("parser tests", []() -> void
           { describe("dynamic keyword arguments", []() -> void
                      {
             it("captures single dynamic keyword values", []() {
               ArgumentParser arg_parser("prog", "dynamic sample");
               Command &root = arg_parser.get_root_command();
               root.enable_dynamic_keywords(ArgType_Single, "example", "placeholder description");
               root.add_keyword_arg("name", make_aliases("-n"), "name to record",
                                   ArgType_Single);

               std::vector<std::string> raw =
                   {"prog", "--foo", "bar", "--name", "cli"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               Expect(result.has_dynamic("foo")).ToBe(true);

               const ArgValue *foo_value = result.get_dynamic("foo");
               Expect(foo_value != nullptr).ToBe(true);
               const std::string foo_string = foo_value ? *foo_value->get_string() : nullptr;
               Expect(foo_string == "bar").ToBe(true);

               const std::string *name_value = result.get<std::string>("name");
               Expect(name_value != nullptr).ToBe(true);
               if (name_value)
               {
                 Expect(*name_value == "cli").ToBe(true);
               }
             });

             it("collects multiple dynamic keyword values", []() {
               ArgumentParser arg_parser("prog", "dynamic sample");
               Command &root = arg_parser.get_root_command();
               root.enable_dynamic_keywords(ArgType_Multiple, "mult-example", "multiple placeholder example");

               std::vector<std::string> raw =
                   {"prog", "--tags", "one", "two", "three"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               Expect(result.has_dynamic("tags")).ToBe(true);

               const ArgValue *tags_value = result.get_dynamic("tags");
               Expect(tags_value != nullptr).ToBe(true);
               const std::vector<std::string> *tags_list =
                   tags_value ? tags_value->get_string_vector() : nullptr;
               Expect(tags_list != nullptr).ToBe(true);
               if (tags_list)
               {
                 Expect(tags_list->size()).ToBe(static_cast<size_t>(3));
                 Expect((*tags_list)[0] == "one").ToBe(true);
                 Expect((*tags_list)[1] == "two").ToBe(true);
                 Expect((*tags_list)[2] == "three").ToBe(true);
               }
             });

             it("requires values for dynamic single keywords", []() {
               ArgumentParser arg_parser("prog", "dynamic sample");
               Command &root = arg_parser.get_root_command();
               root.enable_dynamic_keywords(ArgType_Single, "val", "description");

               std::vector<std::string> raw = {"prog", "--missing"};
               std::vector<char *> argv = to_argv(raw);
               
               std::stringstream err_output;
               std::streambuf* original_cerr_buf = std::cerr.rdbuf();
               std::cerr.rdbuf(err_output.rdbuf());

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               std::cerr.rdbuf(original_cerr_buf);

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
               Expect(result.error_message.find("option --missing requires a value.")).Not().ToBe(std::string::npos);
             });

             it("enforces values for dynamic multi-value keywords", []() {
               ArgumentParser arg_parser("prog", "dynamic sample");
               Command &root = arg_parser.get_root_command();
               root.enable_dynamic_keywords(ArgType_Multiple, "items", "description");

               std::vector<std::string> raw = {"prog", "--tags"};
               std::vector<char *> argv = to_argv(raw);
               
               std::stringstream err_output;
               std::streambuf* original_cerr_buf = std::cerr.rdbuf();
               std::cerr.rdbuf(err_output.rdbuf());

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               std::cerr.rdbuf(original_cerr_buf);

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
               Expect(result.error_message.find("option --tags requires at least one value.")).Not().ToBe(std::string::npos);
             });

             it("errors on unknown keyword when dynamic support disabled", []() {
               ArgumentParser arg_parser("prog", "dynamic sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("name", make_aliases("-n"), "known option",
                                   ArgType_Single);

               std::vector<std::string> raw = {"prog", "--foo", "bar"};
               std::vector<char *> argv = to_argv(raw);

               std::stringstream err_output;
               std::streambuf* original_cerr_buf = std::cerr.rdbuf();
               std::cerr.rdbuf(err_output.rdbuf());

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               std::cerr.rdbuf(original_cerr_buf);

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
               Expect(result.has_dynamic("foo")).ToBe(false);
             }); });


             describe("keyword arguments", []() -> void
                      {
             it("captures values for named keywords", []() {
               ArgumentParser arg_parser("prog", "keyword sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("dataset", make_aliases("-d"), "dataset to process",
                                   ArgType_Single);

               std::vector<std::string> raw = {"prog", "--dataset", "HLQ.PDS"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               Expect(result.get_value<std::string>("dataset", ""))
                   .ToBe("HLQ.PDS");
             });

             it("accepts short aliases for keyword arguments", []() {
               ArgumentParser arg_parser("prog", "keyword sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("name", make_aliases("-n"), "name to capture", ArgType_Single);

               std::vector<std::string> raw = {"prog", "-n", "cli"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               Expect(result.get_value<std::string>("name", ""))
                   .ToBe("cli");
             });

             it("collects multiple values for keyword arguments", []() {
               ArgumentParser arg_parser("prog", "keyword sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("members", make_aliases("-m"), "member names",
                                   ArgType_Multiple);

               std::vector<std::string> raw =
                   {"prog", "--members", "alpha", "beta", "gamma"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               const std::vector<std::string> *members =
                   result.get<std::vector<std::string>>("members");
               Expect(members != nullptr).ToBe(true);
               if (members)
               {
                 Expect(members->size()).ToBe(static_cast<size_t>(3));
                 Expect((*members)[0] == "alpha").ToBe(true);
                 Expect((*members)[1] == "beta").ToBe(true);
                 Expect((*members)[2] == "gamma").ToBe(true);
               }
             });

             it("allows explicit boolean values for keyword flags", []() {
               ArgumentParser arg_parser("prog", "keyword sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("force", make_aliases("-f"), "force execution",
                                   ArgType_Flag);

               std::vector<std::string> raw = {"prog", "--force", "false"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               Expect(result.get_value<bool>("force", true)).ToBe(false);
             });

             it("reports missing values for keyword arguments", []() {
               ArgumentParser arg_parser("prog", "keyword sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("target", make_aliases("-t"), "target data set",
                                   ArgType_Single);

               std::vector<std::string> raw = {"prog", "--target"};
               std::vector<char *> argv = to_argv(raw);
               
               std::stringstream err_output;
               std::streambuf* original_cerr_buf = std::cerr.rdbuf();
               std::cerr.rdbuf(err_output.rdbuf());

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               std::cerr.rdbuf(original_cerr_buf);

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
             });
             });


             describe("flag parsing", []() -> void
                      {
             it("supports combined short boolean flags", []() {
               ArgumentParser arg_parser("prog", "flag sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("verbose", make_aliases("-v"), "enable verbose output", ArgType_Flag);
               root.add_keyword_arg("force", make_aliases("-f"), "force execution", ArgType_Flag);

               std::vector<std::string> raw = {"prog", "-vf"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               Expect(result.get_value<bool>("verbose")).ToBe(true);
               Expect(result.get_value<bool>("force")).ToBe(true);
             });

             it("rejects combined short flags when a value is required", []() {
               ArgumentParser arg_parser("prog", "flag sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("verbose", make_aliases("-v"), "enable verbose output", ArgType_Flag);
               root.add_keyword_arg("output", make_aliases("-o"), "output file", ArgType_Single);

               std::vector<std::string> raw = {"prog", "-vo", "file.txt"};
               std::vector<char *> argv = to_argv(raw);

               std::stringstream err_output;
               std::streambuf* original_cerr_buf = std::cerr.rdbuf();
               std::cerr.rdbuf(err_output.rdbuf());

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               std::cerr.rdbuf(original_cerr_buf);

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
               Expect(result.error_message.find("option -o requires a value and cannot be combined")).Not().ToBe(std::string::npos);
             });

             it("honors automatically generated no- boolean flags", []() {
               ArgumentParser arg_parser("prog", "flag sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("debug", make_aliases("-d"), "enable debug output", ArgType_Flag, false,
                                   ArgValue(true));

               std::vector<std::string> raw = {"prog", "--no-debug"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               Expect(result.get_value<bool>("debug")).ToBe(false);
             });
             });

             describe("positional arguments", []() -> void
                      {
             it("reports missing required positional arguments", []() {
               ArgumentParser arg_parser("prog", "positional sample");
               Command &root = arg_parser.get_root_command();
               root.add_positional_arg("dataset", "dataset to process", ArgType_Single, true);

               std::vector<std::string> raw = {"prog"};
               std::vector<char *> argv = to_argv(raw);
               
               std::stringstream err_output;
               std::streambuf* original_cerr_buf = std::cerr.rdbuf();
               std::cerr.rdbuf(err_output.rdbuf());

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               std::cerr.rdbuf(original_cerr_buf);

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
               Expect(result.error_message.find("missing required positional argument: dataset")).Not().ToBe(std::string::npos);
             });

             it("applies defaults when optional positional arguments are omitted", []() {
               ArgumentParser arg_parser("prog", "positional sample");
               Command &root = arg_parser.get_root_command();
               root.add_positional_arg("dataset", "dataset to process", ArgType_Single, true);
               root.add_positional_arg("target", "optional target", ArgType_Single, false, ArgValue("default"));

               std::vector<std::string> raw = {"prog", "hlq.dataset"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               const std::string *dataset = result.get<std::string>("dataset");
               Expect(dataset != nullptr).ToBe(true);
               if (dataset)
               {
                 Expect(*dataset == "hlq.dataset").ToBe(true);
               }
               Expect(result.get_value<std::string>("target", "fallback")).ToBe("default");
             });

             it("collects multiple positional values", []() {
               ArgumentParser arg_parser("prog", "positional sample");
               Command &root = arg_parser.get_root_command();
               root.add_positional_arg("dataset", "dataset to process", ArgType_Single, true);
               root.add_positional_arg("members", "one or more members", ArgType_Multiple, false);

               std::vector<std::string> raw = {"prog", "hlq.dataset", "mem1", "mem2", "mem3"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               const std::vector<std::string> *members = result.get<std::vector<std::string>>("members");
               Expect(members != nullptr).ToBe(true);
               if (members)
               {
                 Expect(members->size()).ToBe(static_cast<size_t>(3));
                 Expect((*members)[0] == "mem1").ToBe(true);
                 Expect((*members)[1] == "mem2").ToBe(true);
                 Expect((*members)[2] == "mem3").ToBe(true);
               }
             });

             it("rejects unexpected trailing positional arguments", []() {
               ArgumentParser arg_parser("zowex", "command sample");
               Command &root = arg_parser.get_root_command();

               command_ptr job_cmd(new Command("job", "job operations"));
               command_ptr list_cmd(new Command("list", "list jobs"));
               job_cmd->add_command(list_cmd);
               root.add_command(job_cmd);

               std::vector<std::string> raw = {"zowex", "job", "list", "extra"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
               Expect(result.error_message.find("unexpected argument") != std::string::npos).ToBe(true);
             });
             });

             describe("conflict handling", []() -> void
                      {
             it("flags conflicting keyword arguments", []() {
               ArgumentParser arg_parser("prog", "conflict sample");
               Command &root = arg_parser.get_root_command();
               std::stringstream err_output;
               std::streambuf* original_cerr_buf = std::cerr.rdbuf();
               std::cerr.rdbuf(err_output.rdbuf());

               root.add_argument("primary")
                   .aliases(make_aliases("-p"))
                   .help("primary action")
                   .type(ArgType_Flag)
                   .conflicts_with("secondary");
               root.add_argument("secondary")
                   .aliases(make_aliases("-s"))
                   .help("secondary action")
                   .type(ArgType_Flag)
                   .conflicts_with("primary");

               std::vector<std::string> raw = {"prog", "--primary", "--secondary"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());
               std::cerr.rdbuf(original_cerr_buf);

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
               Expect(result.error_message.find("conflicting options provided: --primary conflicts with --secondary")).Not().ToBe(std::string::npos);
             });
             });

             describe("required options", []() -> void
                      {
             it("demands presence of required keyword arguments", []() {
               ArgumentParser arg_parser("prog", "required sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("source", make_aliases("-s"), "source file", ArgType_Single, true);
               
               std::stringstream err_output;
               std::streambuf* original_cerr_buf = std::cerr.rdbuf();
               std::cerr.rdbuf(err_output.rdbuf());

               std::vector<std::string> raw = {"prog"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               std::cerr.rdbuf(original_cerr_buf);

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
               Expect(result.error_message.find("missing required option: -s, --source <value>")).Not().ToBe(std::string::npos);
             });
             });

             describe("command handlers", []() -> void
                      {
             it("invokes registered handlers on successful parse", []() {
               g_handler_called = false;
               g_handler_arg.clear();

               ArgumentParser arg_parser("prog", "handler sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("name", make_aliases("-n"), "name to capture", ArgType_Single, true);
               root.set_handler(&sample_handler);

               std::vector<std::string> raw = {"prog", "--name", "cli"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_Success);
               Expect(result.exit_code).ToBe(7);
               Expect(g_handler_called).ToBe(true);
               Expect(g_handler_arg == "cli").ToBe(true);
               Expect(result.get_value<std::string>("name", ""))
                   .ToBe("cli");
             });
             }); });
}
