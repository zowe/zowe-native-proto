/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
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

} // namespace

void parser_tests()
{
  describe("dynamic keyword arguments", []() -> void
           {
             it("captures single dynamic keyword values", []() {
               ArgumentParser arg_parser("prog", "dynamic sample");
               Command &root = arg_parser.get_root_command();
               root.enable_dynamic_keywords(ArgType_Single);
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
               const std::string *foo_string = foo_value ? foo_value->get_string() : nullptr;
               Expect(foo_string != nullptr).ToBe(true);
               if (foo_string)
               {
                 Expect(*foo_string).ToBe(std::string("bar"));
               }

               const std::string *name_value = result.get<std::string>("name");
               Expect(name_value != nullptr).ToBe(true);
               if (name_value)
               {
                 Expect(*name_value).ToBe(std::string("cli"));
               }
             });

             it("collects multiple dynamic keyword values", []() {
               ArgumentParser arg_parser("prog", "dynamic sample");
               Command &root = arg_parser.get_root_command();
               root.enable_dynamic_keywords(ArgType_Multiple);

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
                 Expect((*tags_list)[0]).ToBe(std::string("one"));
                 Expect((*tags_list)[1]).ToBe(std::string("two"));
                 Expect((*tags_list)[2]).ToBe(std::string("three"));
               }
             });

             it("errors on unknown keyword when dynamic support disabled", []() {
               ArgumentParser arg_parser("prog", "dynamic sample");
               Command &root = arg_parser.get_root_command();
               root.add_keyword_arg("name", make_aliases("-n"), "known option",
                                   ArgType_Single);

               std::vector<std::string> raw = {"prog", "--foo", "bar"};
               std::vector<char *> argv = to_argv(raw);

               ParseResult result =
                   arg_parser.parse(static_cast<int>(argv.size()), argv.data());

               Expect(result.status).ToBe(ParseResult::ParserStatus_ParseError);
               Expect(result.has_dynamic("foo")).ToBe(false);
             });
           });
}

