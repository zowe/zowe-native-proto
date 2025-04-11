/**
MIT License

Copyright (c) 2025 Trae Yelovich <trae@trae.is>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef PPARSER_HPP
#define PPARSER_HPP

#include "lexer.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace pparser {

class Command;
class ArgumentParser;
class ParseResult;
struct ArgValue;

struct ArgValue {
  enum arg_value_kind {
    avk_none,
    avk_bool,
    avk_int,
    avk_double,
    avk_string,
    avk_string_vector
  };

  arg_value_kind kind;
  union {
    bool b;
    long long i; // assuming long long is available as extension, else use long
    double d;
    std::string *s; // pointer for backwards compatibility for unions in
                    // pre-c++11 environments
    std::vector<std::string> *sv; // pointer for the same reason
  } value;

  // default constructor
  ArgValue() : kind(avk_none) {
    // initialize one member to avoid undefined state, though not strictly
    // necessary for none
    value.i = 0;
  }

  explicit ArgValue(bool val) : kind(avk_bool) { value.b = val; }
  explicit ArgValue(long long val) : kind(avk_int) { value.i = val; }
  explicit ArgValue(double val) : kind(avk_double) { value.d = val; }
  // note: takes ownership of new string
  explicit ArgValue(const std::string &val) : kind(avk_string) {
    value.s = new std::string(val);
  }
  // note: takes ownership of new vector
  explicit ArgValue(const std::vector<std::string> &val)
      : kind(avk_string_vector) {
    value.sv = new std::vector<std::string>(val);
  }

  // cleanup pointer members
  ~ArgValue() { clear(); }

  ArgValue(const ArgValue &other) : kind(avk_none) { copy_from(other); }
  ArgValue &operator=(const ArgValue &other) {
    if (this != &other) {
      clear();
      copy_from(other);
    }
    return *this;
  }

  // helper to clear existing data (delete pointers)
  void clear() {
    if (kind == avk_string) {
      delete value.s;
      value.s = nullptr;
    } else if (kind == avk_string_vector) {
      delete value.sv;
      value.sv = nullptr;
    }
    kind = avk_none;
    value.i = 0;
  }

  // helper for copy construction/assignment
  void copy_from(const ArgValue &other) {
    kind = other.kind;
    switch (kind) {
    case avk_none:
      value.i = 0;
      break;
    case avk_bool:
      value.b = other.value.b;
      break;
    case avk_int:
      value.i = other.value.i;
      break;
    case avk_double:
      value.d = other.value.d;
      break;
    case avk_string:
      value.s = other.value.s ? new std::string(*other.value.s) : nullptr;
      break;
    case avk_string_vector:
      value.sv = other.value.sv ? new std::vector<std::string>(*other.value.sv)
                                : nullptr;
      break;
    }
  }

  // type checkers
  bool is_none() const { return kind == avk_none; }
  bool is_bool() const { return kind == avk_bool; }
  bool is_int() const { return kind == avk_int; }
  bool is_double() const { return kind == avk_double; }
  bool is_string() const { return kind == avk_string; }
  bool is_string_vector() const { return kind == avk_string_vector; }

  // getters (unsafe, check type first or use safe getters below)
  bool get_bool_unsafe() const { return value.b; }
  long long get_int_unsafe() const { return value.i; }
  double get_double_unsafe() const { return value.d; }
  const std::string &get_string_unsafe() const { return *value.s; }
  const std::vector<std::string> &get_string_vector_unsafe() const {
    return *value.sv;
  }
  std::string *get_string_ptr_unsafe() const { return value.s; }
  std::vector<std::string> *get_string_vector_ptr_unsafe() const {
    return value.sv;
  }

  // safe getters (returns a pointer; nullptr if wrong type/unset)
  const bool *get_bool() const { return kind == avk_bool ? &value.b : nullptr; }
  const long long *get_int() const {
    return kind == avk_int ? &value.i : nullptr;
  }
  const double *get_double() const {
    return kind == avk_double ? &value.d : nullptr;
  }
  const std::string *get_string() const {
    return kind == avk_string ? value.s : nullptr;
  }
  const std::vector<std::string> *get_string_vector() const {
    return kind == avk_string_vector ? value.sv : nullptr;
  }
  std::string get_string_value(const std::string &default_val = "") const {
    const std::string *ptr = get_string();
    return ptr ? *ptr : default_val;
  }

  void print(std::ostream &os) const {
    switch (kind) {
    case avk_none:
      os << "<none>";
      break;
    case avk_bool:
      os << (value.b ? "true" : "false");
      break;
    case avk_int:
      os << value.i;
      break;
    case avk_double:
      os << value.d;
      break;
    case avk_string:
      os << (value.s ? *value.s : "<invalid_string>");
      break;
    case avk_string_vector: {
      os << "[";
      if (value.sv) {
        for (size_t j = 0; j < value.sv->size(); ++j) {
          os << (*value.sv)[j] << (j == value.sv->size() - 1 ? "" : ", ");
        }
      }
      os << "]";
      break;
    }
    }
  }
};

enum ArgType {
  argtype_flag,      // boolean switch (e.g., --verbose)
  argtype_single,    // expects a single value (e.g., --output file.txt)
  argtype_multiple,  // expects one or more values (e.g., --input a.txt b.txt)
  argtype_positional // argument determined by position
};

struct ArgumentDef {
  std::string name;      // internal name to access the parsed value
  std::string short_name; // short flag (e.g., "-f") - optional
  std::string long_name;  // long flag (e.g., "--file") - optional
  std::string help;      // help text description
  ArgType type;          // type of argument (flag, single, etc.)
  bool required;         // is this argument mandatory?
  ArgValue default_value; // default value if argument is not provided
  bool is_help_flag;       // internal flag to identify the help argument

  ArgumentDef(std::string n, std::string sn, std::string ln, std::string h,
              ArgType t = argtype_flag, bool req = false,
              ArgValue def_val = ArgValue(), bool help_flag = false)
      : name(n), short_name(sn), long_name(ln), help(h), type(t), required(req),
        default_value(def_val), is_help_flag(help_flag) {}

  ArgumentDef()
      : type(argtype_flag), required(false), is_help_flag(false) {}
  // get display name (e.g., "-f, --file")
  std::string get_display_name() const {
    std::string display;
    if (!short_name.empty()) {
      display += short_name;
    }
    if (!long_name.empty()) {
      if (!display.empty())
        display += ", ";
      display += long_name;
    }
    if (type != argtype_flag && type != argtype_positional) {
      display += " <value>";
      if (type == argtype_multiple) {
        display += "...";
      }
    }
    return display;
  }
};

// stores the outcome of the parsing process
class ParseResult {
public:
  enum ParserStatus {
    ParserStatus_Success,
    ParserStatus_HelpRequested,
    ParserStatus_ParseError
  };

  ParserStatus status;     // no default initializer
  int exit_code;             // exit code returned by handler or set by parser
  std::string error_message; // error message if status is parse_error
  std::string
      command_path; // full path of the executed command (e.g., "git remote add")

  // map storing parsed keyword argument values (name -> value)
  std::map<std::string, ArgValue> keyword_values;
  // vector storing parsed positional argument values in order
  std::vector<ArgValue> positional_values;

  ParseResult() : status(ParserStatus_Success), exit_code(0) {}

  // check if a keyword arg was provided (doesn't check type)
  bool has_kw_arg(const std::string &name) const {
    return keyword_values.find(name) != keyword_values.end();
  }

  // getters returning pointers, nullptr if missing or wrong type
  const bool *get_kw_arg_bool(const std::string &name) const {
    std::map<std::string, ArgValue>::const_iterator it =
        keyword_values.find(name);
    if (it != keyword_values.end()) {
      return it->second.get_bool();
    }
    return nullptr;
  }

  const long long *get_kw_arg_int(const std::string &name) const {
    std::map<std::string, ArgValue>::const_iterator it =
        keyword_values.find(name);
    if (it != keyword_values.end()) {
      return it->second.get_int();
    }
    return nullptr;
  }

  const double *get_kw_arg_double(const std::string &name) const {
    std::map<std::string, ArgValue>::const_iterator it =
        keyword_values.find(name);
    if (it != keyword_values.end()) {
      return it->second.get_double();
    }
    return nullptr;
  }

  const std::string *get_kw_arg_string(const std::string &name) const {
    std::map<std::string, ArgValue>::const_iterator it =
        keyword_values.find(name);
    if (it != keyword_values.end()) {
      return it->second.get_string();
    }
    return nullptr;
  }

  const std::vector<std::string> *
  get_kw_arg_list(const std::string &name) const {
    std::map<std::string, ArgValue>::const_iterator it =
        keyword_values.find(name);
    if (it != keyword_values.end()) {
      return it->second.get_string_vector();
    }
    return nullptr;
  }

  // getters returning value or a default if missing/wrong type
  bool find_kw_arg_bool(const std::string &name,
                                  bool default_value = false) const {
    const bool *ptr = get_kw_arg_bool(name);
    return ptr ? *ptr : default_value;
  }

  long long find_kw_arg_int(const std::string &name,
                                      long long default_value = 0) const {
    const long long *ptr = get_kw_arg_int(name);
    return ptr ? *ptr : default_value;
  }

  double find_kw_arg_double(const std::string &name,
                                      double default_value = 0.0) const {
    const double *ptr = get_kw_arg_double(name);
    return ptr ? *ptr : default_value;
  }

  std::string
  find_kw_arg_string(const std::string &name,
                               const std::string &default_value = "") const {
    std::map<std::string, ArgValue>::const_iterator it =
        keyword_values.find(name);
    if (it != keyword_values.end()) {
      return it->second.get_string_value(default_value);
    }
    return default_value;
  }

  // returns a copy of the vector or an empty vector as default
  std::vector<std::string>
  find_kw_arg_list(const std::string &name) const {
    const std::vector<std::string> *ptr = get_kw_arg_list(name);
    return ptr ? *ptr : std::vector<std::string>();
  }

  // getters for positional args by index
  const bool *get_pos_arg_bool(size_t index) const {
    if (index < positional_values.size()) {
      return positional_values[index].get_bool();
    }
    return nullptr;
  }
  const long long *get_pos_arg_int(size_t index) const {
    if (index < positional_values.size()) {
      return positional_values[index].get_int();
    }
    return nullptr;
  }
  const double *get_pos_arg_double(size_t index) const {
    if (index < positional_values.size()) {
      return positional_values[index].get_double();
    }
    return nullptr;
  }
  const std::string *get_pos_arg_string(size_t index) const {
    if (index < positional_values.size()) {
      return positional_values[index].get_string();
    }
    return nullptr;
  }
  const std::vector<std::string> *
  get_pos_arg_list(size_t index) const {
    if (index < positional_values.size()) {
      return positional_values[index].get_string_vector();
    }
    return nullptr;
  }
};

// represents a command or subcommand with its arguments
class Command : public std::enable_shared_from_this<Command> {
public:
  typedef int (*CommandHandler)(const ParseResult &result);

  Command(std::string name, std::string help)
      : m_name(name), m_help(help), m_handler(nullptr) {
    ensure_help_argument();
  }

  // destructor to clean up subcommand pointers
  ~Command() { m_commands.clear(); }

  // add a keyword/option argument (e.g., --file, -f)
  Command &add_keyword_arg(std::string name, std::string short_name,
                         std::string long_name, std::string help,
                         ArgType type = argtype_flag, bool required = false,
                         ArgValue default_value = ArgValue()) {
    // prevent adding another argument named "help" or starting with "no_"
    if (name == "help") {
      throw std::invalid_argument(
          "argument name 'help' is reserved for the automatic help flag.");
    }
    if (name.rfind("no_", 0) == 0) {
       throw std::invalid_argument(
          "argument name cannot start with 'no_'. this prefix is reserved for automatic negation flags.");
    }
    // ensure long name doesn't start with --no- if provided manually
    if (!long_name.empty() && long_name.rfind("--no-", 0) == 0) {
         throw std::invalid_argument(
          "long name cannot start with '--no-'. this prefix is reserved for automatic negation flags.");
    }


    // if it's a flag and the default value is still the initial avk_none,
    // set the actual default to false. otherwise, use the provided default.
    ArgValue final_default_value = default_value;
    if (type == argtype_flag && default_value.is_none()) {
      final_default_value = ArgValue(false);
    }

    // ensure the new keyword argument name is unique
    auto it = std::find_if(
        m_kw_args.begin(), m_kw_args.end(),
        [&name](const ArgumentDef &arg) { return arg.name == name; });
    if (it != m_kw_args.end()) {
      throw std::invalid_argument("argument name '" + name + "' already exists.");
    }
    // ensure short/long names are unique across all args (including potential auto-generated ones)
    for(const auto& existing_arg : m_kw_args) {
        if (!short_name.empty() && existing_arg.short_name == short_name) {
             throw std::invalid_argument("short name '" + short_name + "' already exists.");
        }
        if (!long_name.empty() && existing_arg.long_name == long_name) {
             throw std::invalid_argument("long name '" + long_name + "' already exists.");
        }
    }


    // add the primary argument definition
    m_kw_args.push_back(ArgumentDef(name, short_name, long_name, help, type,
                                       required, final_default_value));

    // check if we need to add an automatic --no-<flag>
    const bool *default_bool = final_default_value.get_bool();
    if (type == argtype_flag && default_bool && *default_bool == true && !long_name.empty() && long_name.rfind("--", 0) == 0) {
        std::string no_flag_name = "no_" + name;
        std::string no_flag_long_name = "--no-" + long_name.substr(2); // remove "--" prefix
        std::string no_flag_help = "disable the " + long_name + " flag.";

        // ensure the generated --no- name/long_name doesn't conflict
        auto no_it = std::find_if(
            m_kw_args.begin(), m_kw_args.end(),
            [&no_flag_name](const ArgumentDef &arg) { return arg.name == no_flag_name; });
        if (no_it != m_kw_args.end()) {
          throw std::invalid_argument("automatic negation flag name '" + no_flag_name + "' conflicts with an existing argument.");
        }
         for(const auto& existing_arg : m_kw_args) {
            if (existing_arg.long_name == no_flag_long_name) {
                throw std::invalid_argument("automatic negation flag long name '" + no_flag_long_name + "' conflicts with an existing argument.");
            }
        }

        // add the negation argument definition
        m_kw_args.push_back(ArgumentDef(no_flag_name, "", no_flag_long_name, no_flag_help,
                                           argtype_flag, false, ArgValue(false), false));
    }

    return *this;
  }

  // add a positional argument (defined by order)
  Command &add_positional_arg(std::string name, std::string help,
                            ArgType type = argtype_single, bool required = true,
                            ArgValue default_value = ArgValue()) {
    if (type == argtype_flag) {
      throw std::invalid_argument("positional arguments cannot be flags.");
    }

    // ensure the new positional argument is unique
    auto it = std::find_if(
        m_pos_args.begin(), m_pos_args.end(),
        [&name](const ArgumentDef &arg) { return arg.name == name; });
    if (it != m_pos_args.end()) {
      throw std::invalid_argument("argument '" + name + "' already exists.");
    }

    m_pos_args.push_back(ArgumentDef(
        name, "", "", help, argtype_positional, required, default_value));
    m_pos_args.back().type = type;
    return *this;
  }

  // add a command under this command (making this command act as a group)
  Command &add_command(std::shared_ptr<Command> sub) {
    if (!sub)
      return *this;

    sub->ensure_help_argument();

    std::string sub_name = sub->get_name();
    // check for conflicts with existing names and aliases
    if (m_commands.count(sub_name)) {
      throw std::invalid_argument("subcommand name '" + sub_name +
                                  "' already exists.");
    }
    for (const auto &pair : m_commands) {
      if (pair.second->has_alias(sub_name)) {
        throw std::invalid_argument("subcommand name '" + sub_name +
                                    "' conflicts with an existing alias.");
      }
      for (const auto &alias : sub->get_aliases()) {
        if (pair.first == alias || pair.second->has_alias(alias)) {
          throw std::invalid_argument(
              "subcommand alias '" + alias +
              "' conflicts with an existing name or alias.");
        }
      }
    }
    // also check the new subcommand's aliases against its own name
    for (const auto &alias : sub->get_aliases()) {
      if (alias == sub_name) {
        throw std::invalid_argument("subcommand alias '" + alias +
                                    "' cannot be the same as its name '" +
                                    sub_name + "'.");
      }
    }

    m_commands[sub_name] = sub;
    return *this;
  }

  // add an alias to this command
  Command &add_alias(const std::string &alias) {
    if (alias == m_name) {
      throw std::invalid_argument(
          "alias cannot be the same as the command name '" + m_name + "'.");
    }
    // could add checks here to ensure alias doesn't conflict with other sibling
    // commands/aliases if added to a parent
    m_aliases.push_back(alias);
    return *this;
  }

  // set the function pointer handler
  Command &set_handler(CommandHandler handler) {
    m_handler = handler;
    return *this;
  }

  const std::string &get_name() const { return m_name; }
  const std::string &get_help() const { return m_help; }
  const std::map<std::string, std::shared_ptr<Command>> &
  get_subcommands() const {
    return m_commands;
  }
  const std::vector<ArgumentDef> &get_keyword_args() const {
    return m_kw_args;
  }
  const std::vector<ArgumentDef> &get_positional_args() const {
    return m_pos_args;
  }
  const std::vector<std::string> &get_aliases() const { return m_aliases; }
  CommandHandler get_handler() const { return m_handler; }

  ParseResult parse(const std::vector<lexer::Token> &tokens,
                    size_t &current_token_index,
                    const std::string &command_path_prefix) const {
    ParseResult result;
    result.command_path = command_path_prefix + m_name;
    // initialize exit code for potential errors or help requests
    result.exit_code = 0; // default to 0 for success before handler runs

    std::map<std::string, bool> keyword_args_seen;
    size_t current_positional_arg_index = 0;

    for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
         it != m_kw_args.end(); ++it) {
      const ArgumentDef &arg = *it;
      if (arg.is_help_flag) {
        continue;
      }
      result.keyword_values[arg.name] = arg.default_value;
    }

    while (current_token_index < tokens.size()) {
      const lexer::Token &token = tokens[current_token_index];
      lexer::TokenKind kind = token.get_kind();

      // check for keyword arguments/flags (TokFlagShort, TokFlagLong)
      if (kind == lexer::TokFlagShort || kind == lexer::TokFlagLong) {
        std::string flag_name_str = token.get_id_value(); // get name without dashes
        bool is_short_flag_kind = (kind == lexer::TokFlagShort);

        if (is_short_flag_kind && flag_name_str.length() > 1) {
          current_token_index++;

          for (size_t i = 0; i < flag_name_str.length(); ++i) {
            std::string single_flag_char(1, flag_name_str[i]);
            const ArgumentDef *matched_arg =
                find_keyword_arg(single_flag_char, true);

            if (!matched_arg) {
              result.status = ParseResult::ParserStatus_ParseError;
              result.error_message = "unknown option in combined flags: -";
              result.error_message += single_flag_char;
              std::cerr << "error: " << result.error_message << "\n\n";
              generate_help(std::cerr, command_path_prefix);
              result.exit_code = 1;
              return result;
            }

            if (matched_arg->is_help_flag) {
              generate_help(std::cout, command_path_prefix);
              result.status = ParseResult::ParserStatus_HelpRequested;
              result.exit_code = 0; // help request is a successful exit
              return result;
            }

            // ensure combined flags are actually boolean flags
            if (matched_arg->type != argtype_flag) {
              result.status = ParseResult::ParserStatus_ParseError;
              result.error_message = "option -" + single_flag_char +
                                    " requires a value and cannot be combined.";
              std::cerr << "error: " << result.error_message << "\n\n";
              generate_help(std::cerr, command_path_prefix);
              result.exit_code = 1;
              return result;
            }

            // mark as seen and set value to true
            keyword_args_seen[matched_arg->name] = true;
            result.keyword_values[matched_arg->name] = ArgValue(true);
          }
          continue;
        }

        const ArgumentDef *matched_arg =
            find_keyword_arg(flag_name_str, is_short_flag_kind);

        if (!matched_arg) {
          result.status = ParseResult::ParserStatus_ParseError;
          result.error_message = "unknown option: ";
          result.error_message += (is_short_flag_kind ? "-" : "--");
          result.error_message += flag_name_str;
          std::cerr << "error: " << result.error_message << "\n\n";
          generate_help(std::cerr, command_path_prefix);
          result.exit_code = 1;
          return result;
        }

        // if this is the specifically marked help flag, handle it immediately.
        if (matched_arg->is_help_flag) {
          generate_help(std::cout, command_path_prefix);
          result.status = ParseResult::ParserStatus_HelpRequested;
          result.exit_code = 0; // help request is a successful exit
          return result;
        }

        current_token_index++; // consume the flag token itself
        keyword_args_seen[matched_arg->name] = true;

        // handle argument value based on type
        if (matched_arg->type == argtype_flag) {
          result.keyword_values[matched_arg->name] = ArgValue(true);
        } else {
          // check if next token exists and is not another flag
          if (current_token_index >= tokens.size() ||
              tokens[current_token_index].get_kind() == lexer::TokFlagShort ||
              tokens[current_token_index].get_kind() == lexer::TokFlagLong) {
            result.status = ParseResult::ParserStatus_ParseError;
            result.error_message =
                "option " + matched_arg->get_display_name() + " requires a value.";
            std::cerr << "error: " << result.error_message << "\n\n";
            generate_help(std::cerr, command_path_prefix);
            result.exit_code = 1;
            return result;
          }

          const lexer::Token &value_token = tokens[current_token_index];
          ArgValue parsed_value = parse_token_value(value_token, matched_arg->type);
          if (parsed_value.is_none()) {
            result.status = ParseResult::ParserStatus_ParseError;
            result.error_message =
                "invalid value for option " + matched_arg->get_display_name();
            std::cerr << "error: " << result.error_message << "\n\n";
            generate_help(std::cerr, command_path_prefix);
            result.exit_code = 1;
            return result;
          }

          if (matched_arg->type == argtype_single) {
            result.keyword_values[matched_arg->name] = parsed_value;
            current_token_index++;
          } else if (matched_arg->type == argtype_multiple) {
            // ensure value type is string for multiple
            if (!parsed_value.is_string()) {
              result.status = ParseResult::ParserStatus_ParseError;
              result.error_message =
                  "internal error: expected string value for multiple option " +
                  matched_arg->name;
              std::cerr << "error: " << result.error_message << "\n\n";
              generate_help(std::cerr, command_path_prefix);
              result.exit_code = 1;
              return result;
            }

            std::map<std::string, ArgValue>::iterator map_it =
                result.keyword_values.find(matched_arg->name);
            if (map_it == result.keyword_values.end() ||
                !map_it->second.is_string_vector() || map_it->second.is_none()) {
              result.keyword_values[matched_arg->name] =
                  ArgValue(std::vector<std::string>());
              map_it = result.keyword_values.find(matched_arg->name);
            }

            std::vector<std::string> *vec =
                map_it->second.get_string_vector_ptr_unsafe();
            if (vec) {
              const std::string *first_val_str_ptr = parsed_value.get_string();
              if (first_val_str_ptr) {
                vec->push_back(*first_val_str_ptr);
              }

              current_token_index++;

              // keep consuming values until next flag or end
              while (current_token_index < tokens.size() &&
                     tokens[current_token_index].get_kind() !=
                         lexer::TokFlagShort &&
                     tokens[current_token_index].get_kind() !=
                         lexer::TokFlagLong) {
                const lexer::Token &next_value_token = tokens[current_token_index];
                ArgValue next_parsed_value =
                    parse_token_value(next_value_token, argtype_single);
                const std::string *next_val_str_ptr = next_parsed_value.get_string();
                if (next_val_str_ptr) {
                  vec->push_back(*next_val_str_ptr);
                  current_token_index++;
                } else {
                  break;
                }
              }
            } else {
              result.status = ParseResult::ParserStatus_ParseError;
              result.error_message =
                  "internal error accessing vector for multiple values for " +
                  matched_arg->name;
              std::cerr << "error: " << result.error_message << "\n\n";
              generate_help(std::cerr, command_path_prefix);
              result.exit_code = 1;
              return result;
            }
          }
        }
        continue;
      }

      // check for subcommand
      if (kind == lexer::TokId) {
        std::string potential_subcommand_or_alias = token.get_id_value();
        std::shared_ptr<Command> matched_subcommand = nullptr;

        // first, check if it's a direct name match
        auto sub_it = m_commands.find(potential_subcommand_or_alias);
        if (sub_it != m_commands.end()) {
          matched_subcommand = sub_it->second;
        } else {
          // if not a direct name match, check aliases
          for (const auto &pair : m_commands) {
            if (pair.second->has_alias(potential_subcommand_or_alias)) {
              // prevent aliasing to multiple commands
              if (matched_subcommand) {
                result.status = ParseResult::ParserStatus_ParseError;
                result.error_message = "ambiguous alias '" +
                                      potential_subcommand_or_alias +
                                      "' matches multiple subcommands.";
                std::cerr << "error: " << result.error_message << "\n\n";
                generate_help(std::cerr, command_path_prefix);
                result.exit_code = 1;
                return result;
              }
              matched_subcommand = pair.second;
            }
          }
        }

        // if a subcommand (by name or alias) was found
        if (matched_subcommand) {
          current_token_index++; // consume the subcommand/alias token
          ParseResult sub_result = matched_subcommand->parse(
              tokens, current_token_index, result.command_path + " ");

          // propagate the result (success, error, or help request) from the
          // subcommand.
          return sub_result;
        }
      }

      // if not a flag/option or subcommand, treat as positional argument
      if (current_positional_arg_index < m_pos_args.size()) {
        const ArgumentDef &pos_arg_def =
            m_pos_args[current_positional_arg_index];
        ArgValue parsed_value = parse_token_value(token, pos_arg_def.type);

        if (parsed_value.is_none()) {
          result.status = ParseResult::ParserStatus_ParseError;
          result.error_message =
              "invalid value for positional argument '" + pos_arg_def.name + "'";
          std::cerr << "error: " << result.error_message << "\n\n";
          generate_help(std::cerr, command_path_prefix);
          result.exit_code = 1;
          return result;
        }

        if (pos_arg_def.type == argtype_single) {
          result.positional_values.push_back(parsed_value);
          current_token_index++;
          current_positional_arg_index++;
        } else if (pos_arg_def.type == argtype_multiple) {
          // ensure value type is string for multiple positional
          if (!parsed_value.is_string()) {
            result.status = ParseResult::ParserStatus_ParseError;
            result.error_message = "internal error: expected string value for "
                                  "multiple positional argument " +
                                  pos_arg_def.name;
            std::cerr << "error: " << result.error_message << "\n\n";
            generate_help(std::cerr, command_path_prefix);
            result.exit_code = 1;
            return result;
          }

          std::vector<std::string> values;
          // add the first value (must be string)
          const std::string *first_val_str_ptr = parsed_value.get_string();
          if (first_val_str_ptr) { // should always be true
            values.push_back(*first_val_str_ptr);
          }

          current_token_index++;
          while (current_token_index < tokens.size() &&
                 !is_flag_token(tokens,
                              current_token_index) /* && !is_subcommand(...) */) {
            // todo: add is_subcommand check more robustly?
            // currently relies on subcommand check earlier in loop
            const lexer::Token &next_value_token = tokens[current_token_index];
            // parse subsequent as strings
            ArgValue next_parsed_value =
                parse_token_value(next_value_token, argtype_single);
            const std::string *next_val_str_ptr = next_parsed_value.get_string();
            if (next_val_str_ptr) {
              values.push_back(*next_val_str_ptr);
              current_token_index++;
            } else {
              break;
            }
          }
          // add the vector to positional args using ArgValue constructor
          result.positional_values.push_back(ArgValue(values));
          current_positional_arg_index++;
        }
        // todo: add validation for number of positional args
      } else {
        // unexpected token (neither flag, subcommand, nor expected positional)
        result.status = ParseResult::ParserStatus_ParseError;
        std::stringstream ss;
        ss << "unexpected argument: ";
        token.print(ss);
        result.error_message = ss.str();
        // print error and help for this command to stderr
        std::cerr << "error: " << result.error_message << "\n\n";
        generate_help(std::cerr, command_path_prefix);
        result.exit_code = 1;
        return result;
      }
    }

    // check for required keyword arguments
    for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
         it != m_kw_args.end(); ++it) {
      const ArgumentDef &arg = *it;
      // skip check for the automatic help flag
      if (arg.is_help_flag)
        continue;
      if (arg.required &&
          keyword_args_seen.find(arg.name) == keyword_args_seen.end()) {
        result.status = ParseResult::ParserStatus_ParseError;
        result.error_message =
            "missing required option: " + arg.get_display_name();
        std::cerr << "error: " << result.error_message << "\n\n";
        generate_help(std::cerr, command_path_prefix);
        result.exit_code = 1;
        return result;
      }
    }

    // check for required positional arguments
    if (m_pos_args.size() > current_positional_arg_index) {
      for (size_t i = current_positional_arg_index; i < m_pos_args.size();
           ++i) {
        const ArgumentDef &pos_arg_def = m_pos_args[i];
        if (pos_arg_def.required) {
          result.status = ParseResult::ParserStatus_ParseError;
          result.error_message =
              "missing required positional argument: " + pos_arg_def.name;
          std::cerr << "error: " << result.error_message << "\n\n";
          generate_help(std::cerr, command_path_prefix);
          result.exit_code = 1;
          return result;
        } else {
          // add default value for optional missing positional args
          result.positional_values.push_back(pos_arg_def.default_value);
        }
      }
    }

    // only execute the handler if parsing was successful and no subcommand took
    // over.
    if (m_handler && result.status == ParseResult::ParserStatus_Success) {
      result.exit_code = m_handler(result);
    }

    return result;
  }

  // generate help text for this command and its subcommands
  void generate_help(std::ostream &os,
                    const std::string &command_path_prefix) const {
    std::string full_command_path =
        command_path_prefix.empty() ? m_name : command_path_prefix + m_name;
    if (!command_path_prefix.empty() &&
        command_path_prefix[command_path_prefix.length() - 1] != ' ') {
      full_command_path = command_path_prefix + " " + m_name;
    } else {
      full_command_path = command_path_prefix + m_name;
    }

    os << "usage: " << full_command_path;
    // collect display names for positional args
    std::string positional_usage;
    for (std::vector<ArgumentDef>::const_iterator it = m_pos_args.begin();
         it != m_pos_args.end(); ++it) {
      const ArgumentDef &pos_arg = *it;
      positional_usage += " ";
      positional_usage += (pos_arg.required ? "<" : "[");
      positional_usage += pos_arg.name;
      positional_usage += (pos_arg.required ? ">" : "]");
      if (pos_arg.type == argtype_multiple)
        positional_usage += "...";
    }

    if (!m_kw_args.empty())
      os << " [options]";
    if (!m_commands.empty())
      os << " <command>";

    os << positional_usage;

    os << "\n\n";

    if (!m_help.empty()) {
      os << m_help << "\n\n";
    }

    // info about positional arguments
    if (!m_pos_args.empty()) {
      os << "arguments:\n";
      for (std::vector<ArgumentDef>::const_iterator it =
               m_pos_args.begin();
           it != m_pos_args.end(); ++it) {
        const ArgumentDef &arg = *it;
        os << "  " << arg.name << "\t" << arg.help;
        // check default value is not none
        if (!arg.default_value.is_none()) {
          os << " (default: ";
          arg.default_value.print(os);
          os << ")";
        }
        if (!arg.required)
          os << " [optional]";
        os << "\n";
      }
      os << "\n";
    }

    // info about keyword arguments
    if (!m_kw_args.empty()) {
      os << "options:\n";
      for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
           it != m_kw_args.end(); ++it) {
        const ArgumentDef &arg = *it;
        os << "  " << arg.get_display_name() << "\t" << arg.help;
        if (!arg.default_value.is_none()) {
          // only show non-default bools (true) or non-bool defaults
          const bool *b_val = arg.default_value.get_bool();
          if (!b_val ||
              *b_val == true) { // show default if it's true, or if not a bool
            os << " (default: ";
            arg.default_value.print(os);
            os << ")";
          }
        }
        if (arg.required)
          os << " [required]";
        os << "\n";
      }
      os << "\n";
    }

    // available subcommands
    if (!m_commands.empty()) {
      os << "commands:\n";
      for (std::map<std::string, std::shared_ptr<Command>>::const_iterator it =
               m_commands.begin();
           it != m_commands.end(); ++it) {
        os << "  " << it->first; // print the main command name
        // print aliases if any exist
        const auto &aliases = it->second->get_aliases();
        if (!aliases.empty()) {
          os << " (" << aliases[0];
          for (size_t i = 1; i < aliases.size(); ++i) {
            os << ", " << aliases[i];
          }
          os << ")";
        }
        os << "\t" << it->second->get_help() << "\n";
      }
      os << "\n_use '" << full_command_path
         << " <command> --help' for more information on a command.\n";
    }
  }

private:
  // private copy constructor and assignment operator to prevent copying
  Command(const Command &);
  Command &operator=(const Command &);

  std::string m_name;
  std::string m_help;
  std::vector<ArgumentDef> m_kw_args;
  std::vector<ArgumentDef> m_pos_args;
  std::map<std::string, std::shared_ptr<Command>> m_commands;
  std::vector<std::string> m_aliases;

public:
  CommandHandler m_handler;

private:
  // helper to check if the token at the given index is a flag/option token
  bool is_flag_token(const std::vector<lexer::Token> &tokens,
                   size_t index) const {
    if (index >= tokens.size())
      return false;
    lexer::TokenKind kind = tokens[index].get_kind();
    return kind == lexer::TokFlagShort || kind == lexer::TokFlagLong;
  }

  // find keyword argument definition by flag name (use iterator loop)
  const ArgumentDef *find_keyword_arg(
      const std::string &flag_name_value, // name part only (e.g., "f", "force")
      bool is_short_flag_kind) const {
    for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
         it != m_kw_args.end(); ++it) {
      const ArgumentDef &arg = *it;
      if (is_short_flag_kind) {
        // compare flag_name_value against the name part of short_name (e.g.,
        // compare "f" with "-f")
        if (!arg.short_name.empty() && arg.short_name.length() > 1 &&
            arg.short_name.substr(1) == flag_name_value) {
          return &arg; // return pointer to the element
        }
      } else {
        // compare flag_name_value against the name part of long_name (e.g.,
        // compare "force" with "--force")
        if (!arg.long_name.empty() && arg.long_name.length() > 2 &&
            arg.long_name.substr(2) == flag_name_value) {
          return &arg;
        }
      }
    }
    return nullptr;
  }

  // helper to parse a single token into an ArgValue based on expected type
  // returns ArgValue (which manages memory for string/vector)
  ArgValue parse_token_value(const lexer::Token &token,
                           ArgType expected_type) const {
    lexer::TokenKind kind = token.get_kind();

    // allow broader range of tokens to be interpreted as strings if expected
    bool expect_string =
        (expected_type == argtype_single || expected_type == argtype_multiple ||
         expected_type == argtype_positional);

    switch (kind) {
    case lexer::TokIntLit:
      if (expected_type == argtype_single || expected_type == argtype_positional)
        return ArgValue(token.get_int_value());
      else if (expect_string) // allow int literal as string if needed
        return ArgValue(token.get_str_lit_value());
      break;
    case lexer::TokFloatLit:
      if (expected_type == argtype_single || expected_type == argtype_positional)
        return ArgValue(token.get_float_value());
      else if (expect_string) // allow float literal as string if needed
        return ArgValue(token.get_str_lit_value());
      break;
    case lexer::TokTrue:
      if (expected_type == argtype_single || expected_type == argtype_positional)
        return ArgValue(true);
      else if (expect_string)
        return ArgValue("true");
      break;
    case lexer::TokFalse:
      if (expected_type == argtype_single || expected_type == argtype_positional)
        return ArgValue(false);
      else if (expect_string)
        return ArgValue("false");
      break;
    case lexer::TokStrLit:
      if (expect_string)
        return ArgValue(token.get_str_lit_value());
      break;
    case lexer::TokId:
      if (expect_string)
        return ArgValue(token.get_id_value());
      break;
    default:
      break;
    }

    return ArgValue();
  }

  // helper to add the standard help argument
  void ensure_help_argument() {
    bool help_exists = false;
    for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
         it != m_kw_args.end(); ++it) {
      if (it->name == "help") {
        help_exists = true;
        break;
      }
    }
    if (!help_exists) {
      m_kw_args.push_back(
          ArgumentDef("help", "-h", "--help", "show this help message and exit",
                      argtype_flag, false, ArgValue(false), true));
    }
  }

  // check if the command has a specific alias
  bool has_alias(const std::string &alias) const {
    return std::find(m_aliases.begin(), m_aliases.end(), alias) != m_aliases.end();
  }
};

class ArgumentParser {
public:
  ArgumentParser(std::string prog_name, std::string description = "")
      : m_program_name(prog_name), m_program_desc(description),
        m_root_cmd(std::make_shared<Command>(prog_name, description)) {}

  ~ArgumentParser() = default;

  // get a reference to the root command to add arguments or subcommands
  Command &get_root_command() { return *m_root_cmd; }

  // parse command line arguments from main(argc, argv)
  ParseResult parse(int argc, char *argv[]) {
    if (argc < 1) {
      ParseResult error_result;
      error_result.status = ParseResult::ParserStatus_ParseError;
      error_result.error_message = "invalid arguments provided (argc < 1)";
      error_result.exit_code = 1;
      return error_result;
    }

    // combine args into a single string for the lexer
    std::stringstream ss;
    for (int i = 1; i < argc; ++i) {
      ss << argv[i] << " ";
    }
    return parse(ss.str());
  }

  // parse command line arguments from a single string
  ParseResult parse(const std::string &command_line) {
    if (!m_root_cmd) {
      ParseResult error_result;
      error_result.status = ParseResult::ParserStatus_ParseError;
      error_result.error_message = "ArgumentParser is not initialized correctly.";
      error_result.exit_code = 1;
      return error_result;
    }

    try {
      lexer::Src source = lexer::Src::from_string(command_line, "<cli>");
      std::vector<lexer::Token> tokens = lexer::Lexer::tokenize(source);

      // filter out TokEof at the end
      if (!tokens.empty() && tokens.back().get_kind() == lexer::TokEof) {
        tokens.pop_back();
      }

      size_t token_index = 0;
      ParseResult result = m_root_cmd->parse(tokens, token_index, "");

      // return early in the event of an error or help request from the parse
      // call
      if (result.status == ParseResult::ParserStatus_ParseError) {
        return result;
      }
      if (result.status == ParseResult::ParserStatus_HelpRequested) {
        return result;
      }

      // parsing succeeded, but not all tokens were consumed (and no subcommand
      // handled them)
      if (token_index < tokens.size()) {
        result.status = ParseResult::ParserStatus_ParseError;
        std::stringstream ss;
        ss << "unexpected arguments starting from: ";
        tokens[token_index].print(ss);
        result.error_message = ss.str();
        std::cerr << "error: " << result.error_message << "\n\n";
        // show root help on unexpected trailing arguments error (to stderr)
        m_root_cmd->generate_help(std::cerr, "");
        result.exit_code = 1;
        return result;
      }

      return result;
    } catch (const lexer::LexError &e) {
      std::cerr << "lexer error: " << e.what() << std::endl;
      ParseResult error_result;
      error_result.status = ParseResult::ParserStatus_ParseError;
      error_result.error_message = e.what();
      error_result.exit_code = 1;
      // show root help on lexer error (to stderr)
      if (m_root_cmd)
        m_root_cmd->generate_help(std::cerr, "");
      return error_result;
    } catch (const std::exception &e) {
      // catch standard exceptions that might arise
      std::cerr << "parser error: " << e.what() << std::endl;
      ParseResult error_result;
      error_result.status = ParseResult::ParserStatus_ParseError;
      error_result.error_message = e.what();
      error_result.exit_code = 1;
      // show root help on general parser error (to stderr)
      if (m_root_cmd)
        m_root_cmd->generate_help(std::cerr, "");
      return error_result;
    }
  }

private:
  // prevent copying
  ArgumentParser(const ArgumentParser &);
  ArgumentParser &operator=(const ArgumentParser &);

  std::string m_program_name;
  std::string m_program_desc;
  std::shared_ptr<Command> m_root_cmd;
}; // class ArgumentParser

} // namespace pparser

#endif // PPARSER_HPP