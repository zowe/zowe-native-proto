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

#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace parser
{

// Levenshtein distance for suggestions
// Measures the similarity between strings a and b
inline size_t levenshtein_distance(const std::string &a, const std::string &b)
{
  const size_t len_a = a.size();
  const size_t len_b = b.size();
  std::vector<size_t> prev(len_b + 1, 0);
  std::vector<size_t> curr(len_b + 1, 0);

  for (size_t j = 0; j <= len_b; ++j)
    prev[j] = j;

  for (size_t i = 1; i <= len_a; ++i)
  {
    curr[0] = i;
    for (size_t j = 1; j <= len_b; ++j)
    {
      size_t cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
      size_t del = prev[j] + 1;
      size_t ins = curr[j - 1] + 1;
      size_t sub = prev[j - 1] + cost;
      size_t min_val = del < ins ? del : ins;
      min_val = min_val < sub ? min_val : sub;
      curr[j] = min_val;
    }
    prev = curr;
  }
  return prev[len_b];
}

// Helper for environments without initializer_list support
inline std::vector<std::string> make_aliases(const char *a1 = 0,
                                             const char *a2 = 0,
                                             const char *a3 = 0,
                                             const char *a4 = 0)
{
  std::vector<std::string> v;
  if (a1)
    v.push_back(a1);
  if (a2)
    v.push_back(a2);
  if (a3)
    v.push_back(a3);
  if (a4)
    v.push_back(a4);
  return v;
}

class Command;
#if defined(COMPAT_USE_TR1_SHARED_PTR)
typedef std::tr1::shared_ptr<Command> command_ptr;
typedef std::tr1::enable_shared_from_this<Command> enable_shared_command;
#else
typedef std::shared_ptr<Command> command_ptr;
typedef std::enable_shared_from_this<Command> enable_shared_command;
#endif

class ArgumentParser;
class ParseResult;
struct ArgValue;

struct ArgValue
{
  enum ValueKind
  {
    ValueKind_None,
    ValueKind_Bool,
    ValueKind_Int,
    ValueKind_Double,
    ValueKind_String,
    ValueKind_List
  };

  ValueKind kind;
  union
  {
    bool b;
    long long i; // assuming long long is available as extension, else use long
    double d;
    std::string *s;               // pointer for backwards compatibility for unions in
                                  // pre-c++11 environments
    std::vector<std::string> *sv; // pointer for the same reason
  } value;

  // default constructor
  ArgValue()
      : kind(ValueKind_None)
  {
    // initialize one member to avoid undefined state, though not strictly
    // necessary for none
    value.i = 0;
  }

  explicit ArgValue(bool val)
      : kind(ValueKind_Bool)
  {
    value.b = val;
  }
  explicit ArgValue(long long val)
      : kind(ValueKind_Int)
  {
    value.i = val;
  }
  explicit ArgValue(double val)
      : kind(ValueKind_Double)
  {
    value.d = val;
  }
  // note: takes ownership of new string
  explicit ArgValue(const std::string &val)
      : kind(ValueKind_String)
  {
    value.s = new std::string(val);
  }
  // note: takes ownership of new vector
  explicit ArgValue(const std::vector<std::string> &val)
      : kind(ValueKind_List)
  {
    value.sv = new std::vector<std::string>(val);
  }

  // cleanup pointer members
  ~ArgValue()
  {
    clear();
  }

  ArgValue(const ArgValue &other)
      : kind(ValueKind_None)
  {
    copy_from(other);
  }
  ArgValue &operator=(const ArgValue &other)
  {
    if (this != &other)
    {
      clear();
      copy_from(other);
    }
    return *this;
  }

  // helper to clear existing data (delete pointers)
  void clear()
  {
    if (kind == ValueKind_String)
    {
      delete value.s;
      value.s = nullptr;
    }
    else if (kind == ValueKind_List)
    {
      delete value.sv;
      value.sv = nullptr;
    }
    kind = ValueKind_None;
    value.i = 0;
  }

  // helper for copy construction/assignment
  void copy_from(const ArgValue &other)
  {
    kind = other.kind;
    switch (kind)
    {
    case ValueKind_None:
      value.i = 0;
      break;
    case ValueKind_Bool:
      value.b = other.value.b;
      break;
    case ValueKind_Int:
      value.i = other.value.i;
      break;
    case ValueKind_Double:
      value.d = other.value.d;
      break;
    case ValueKind_String:
      value.s = other.value.s ? new std::string(*other.value.s) : nullptr;
      break;
    case ValueKind_List:
      value.sv = other.value.sv ? new std::vector<std::string>(*other.value.sv)
                                : nullptr;
      break;
    }
  }

  // type checkers
  bool is_none() const
  {
    return kind == ValueKind_None;
  }
  bool is_bool() const
  {
    return kind == ValueKind_Bool;
  }
  bool is_int() const
  {
    return kind == ValueKind_Int;
  }
  bool is_double() const
  {
    return kind == ValueKind_Double;
  }
  bool is_string() const
  {
    return kind == ValueKind_String;
  }
  bool is_string_vector() const
  {
    return kind == ValueKind_List;
  }

  // getters (unsafe, check type first or use safe getters below)
  bool get_bool_unsafe() const
  {
    return value.b;
  }
  long long get_int_unsafe() const
  {
    return value.i;
  }
  double get_double_unsafe() const
  {
    return value.d;
  }
  const std::string &get_string_unsafe() const
  {
    return *value.s;
  }
  const std::vector<std::string> &get_string_vector_unsafe() const
  {
    return *value.sv;
  }
  std::string *get_string_ptr_unsafe() const
  {
    return value.s;
  }
  std::vector<std::string> *get_string_vector_ptr_unsafe() const
  {
    return value.sv;
  }

  // safe getters (returns a pointer; nullptr if wrong type/unset)
  const bool *get_bool() const
  {
    return kind == ValueKind_Bool ? &value.b : nullptr;
  }
  const long long *get_int() const
  {
    return kind == ValueKind_Int ? &value.i : nullptr;
  }
  const double *get_double() const
  {
    return kind == ValueKind_Double ? &value.d : nullptr;
  }
  const std::string *get_string() const
  {
    return kind == ValueKind_String ? value.s : nullptr;
  }
  const std::vector<std::string> *get_string_vector() const
  {
    return kind == ValueKind_List ? value.sv : nullptr;
  }
  std::string get_string_value(const std::string &default_val = "") const
  {
    const std::string *ptr = get_string();
    return ptr ? *ptr : default_val;
  }

  void print(std::ostream &os) const
  {
    switch (kind)
    {
    case ValueKind_None:
      os << "<none>";
      break;
    case ValueKind_Bool:
      os << (value.b ? "true" : "false");
      break;
    case ValueKind_Int:
      os << value.i;
      break;
    case ValueKind_Double:
      os << value.d;
      break;
    case ValueKind_String:
      os << (value.s ? *value.s : "<invalid_string>");
      break;
    case ValueKind_List:
    {
      os << "[";
      if (value.sv)
      {
        for (size_t j = 0; j < value.sv->size(); ++j)
        {
          os << (*value.sv)[j] << (j == value.sv->size() - 1 ? "" : ", ");
        }
      }
      os << "]";
      break;
    }
    }
  }
};

enum ArgType
{
  ArgType_Flag,      // boolean switch (e.g., --verbose)
  ArgType_Single,    // expects a single value (e.g., --output file.txt)
  ArgType_Multiple,  // expects one or more values (e.g., --input a.txt b.txt)
  ArgType_Positional // argument determined by position
};

struct ArgumentDef
{
  std::string name; // internal name to access the parsed value
  std::vector<std::string>
      aliases;            // all flag aliases (e.g., "-f", "--file", "--alias")
  std::string help;       // help text description
  ArgType type;           // type of argument (flag, single, etc.)
  bool required;          // is this argument mandatory?
  ArgValue default_value; // default value if argument is not provided
  bool is_help_flag;      // internal flag to identify the help argument

  // Main constructor: accepts a vector of aliases
  ArgumentDef(std::string n, const std::vector<std::string> &als, std::string h,
              ArgType t = ArgType_Flag, bool req = false,
              ArgValue def_val = ArgValue(), bool help_flag = false)
      : name(n), aliases(als), help(h), type(t), required(req),
        default_value(def_val), is_help_flag(help_flag)
  {
  }

  // Convenience constructor: single alias
  ArgumentDef(std::string n, std::string alias, std::string h,
              ArgType t = ArgType_Flag, bool req = false,
              ArgValue def_val = ArgValue(), bool help_flag = false)
      : name(n), help(h), type(t), required(req), default_value(def_val),
        is_help_flag(help_flag)
  {
    if (!alias.empty())
    {
      aliases.push_back(alias);
    }
  }

  // Default constructor
  ArgumentDef()
      : type(ArgType_Flag), required(false), is_help_flag(false)
  {
  }

  // get display name (e.g., "-f, --file")
  std::string get_display_name() const
  {
    std::string display;
    // Add all aliases
    for (size_t i = 0; i < aliases.size(); ++i)
    {
      if (!aliases[i].empty())
      {
        if (!display.empty())
          display += ", ";
        display += aliases[i];
      }
    }
    // Add canonical name as long flag if not already present
    if (!name.empty() && type != ArgType_Positional)
    {
      std::string long_flag = "--" + name;
      bool found = false;
      for (size_t i = 0; i < aliases.size(); ++i)
      {
        if (aliases[i] == long_flag)
        {
          found = true;
          break;
        }
      }
      if (!found)
      {
        if (!display.empty())
          display += ", ";
        display += long_flag;
      }
    }
    if (type != ArgType_Flag && type != ArgType_Positional)
    {
      display += " <value>";
      if (type == ArgType_Multiple)
      {
        display += "...";
      }
    }
    return display;
  }
};

// represents a command or subcommand with its arguments
class Command : public enable_shared_command
{
public:
  typedef int (*CommandHandler)(const ParseResult &result);

  Command(std::string name, std::string help)
      : m_name(name), m_help(help), m_handler(nullptr)
  {
    ensure_help_argument();
  }

  // destructor to clean up subcommand pointers
  ~Command()
  {
    m_commands.clear();
  }

  // add a keyword/option argument (e.g., --file, -f)
  // New add_keyword_arg: accepts a vector of aliases for maximum flexibility
  Command &add_keyword_arg(std::string name,
                           const std::vector<std::string> &aliases,
                           std::string help, ArgType type = ArgType_Flag,
                           bool required = false,
                           ArgValue default_value = ArgValue())
  {
    // prevent adding another argument named "help" or starting with "no-"
    if (name == "help")
    {
      throw std::invalid_argument(
          "argument name 'help' is reserved for the automatic help flag.");
    }
    if (name.rfind("no-", 0) == 0)
    {
      throw std::invalid_argument(
          "argument name cannot start with 'no-'. this prefix is reserved for "
          "automatic negation flags.");
    }
    for (size_t i = 0; i < aliases.size(); ++i)
    {
      if (!aliases[i].empty() && aliases[i].find("--no-") == 0)
      {
        throw std::invalid_argument(
            "alias cannot start with '--no-'. this prefix is reserved for "
            "automatic negation flags.");
      }
    }

    // if it's a flag and the default value is still the initial avk_none,
    // set the actual default to false. otherwise, use the provided default.
    ArgValue final_default_value = default_value;
    if (type == ArgType_Flag && default_value.is_none())
    {
      final_default_value = ArgValue(false);
    }

    // ensure the new keyword argument name is unique
    std::vector<ArgumentDef>::iterator it = m_kw_args.begin();
    for (; it != m_kw_args.end(); ++it)
    {
      if (it->name == name)
        break;
    }
    if (it != m_kw_args.end())
    {
      throw std::invalid_argument("argument name '" + name +
                                  "' already exists.");
    }
    // ensure aliases are unique across all args (including potential
    // auto-generated ones)
    for (std::vector<ArgumentDef>::const_iterator kwarg = m_kw_args.begin();
         kwarg != m_kw_args.end(); ++kwarg)
    {
      const ArgumentDef &existing_arg = *kwarg;
      for (size_t i = 0; i < aliases.size(); ++i)
      {
        for (size_t j = 0; j < existing_arg.aliases.size(); ++j)
        {
          if (!aliases[i].empty() && aliases[i] == existing_arg.aliases[j])
          {
            throw std::invalid_argument("alias '" + aliases[i] +
                                        "' already exists.");
          }
        }
      }
    }

    // add the primary argument definition
    m_kw_args.push_back(
        ArgumentDef(name, aliases, help, type, required, final_default_value));

    // check if we need to add an automatic --no-<flag>
    const bool *default_bool = final_default_value.get_bool();
    // Only add --no-<flag> for boolean flags with a true default
    if (type == ArgType_Flag && default_bool && *default_bool == true)
    {
      std::string no_flag_name = "no-" + name;
      std::string no_flag_help = "disable the --" + name + " flag.";

      // ensure the generated --no- name/alias doesn't conflict
      std::vector<ArgumentDef>::iterator no_it = m_kw_args.begin();
      for (; no_it != m_kw_args.end(); ++no_it)
      {
        if (no_it->name == no_flag_name)
          break;
      }
      if (no_it != m_kw_args.end())
      {
        throw std::invalid_argument("automatic negation flag name '" +
                                    no_flag_name +
                                    "' conflicts with an existing argument.");
      }

      // add the negation argument definition
      m_kw_args.push_back(ArgumentDef(no_flag_name, make_aliases(),
                                      no_flag_help, ArgType_Flag, false,
                                      ArgValue(false), false));
    }

    return *this;
  }

  // add a positional argument (defined by order)
  Command &add_positional_arg(std::string name, std::string help,
                              ArgType type = ArgType_Single,
                              bool required = true,
                              ArgValue default_value = ArgValue())
  {
    if (type == ArgType_Flag)
    {
      throw std::invalid_argument("positional arguments cannot be flags.");
    }

    // ensure the new positional argument is unique
    std::vector<ArgumentDef>::iterator it = m_pos_args.begin();
    for (; it != m_pos_args.end(); ++it)
    {
      if (it->name == name)
        break;
    }
    if (it != m_pos_args.end())
    {
      throw std::invalid_argument("argument '" + name + "' already exists.");
    }

    std::vector<std::string> empty_aliases;
    m_pos_args.push_back(ArgumentDef(name, empty_aliases, help,
                                     ArgType_Positional, required,
                                     default_value));
    m_pos_args.back().type = type;
    return *this;
  }

  // add a command under this command (making this command act as a group)
  Command &add_command(command_ptr sub)
  {
    if (!sub)
      return *this;

    sub->ensure_help_argument();

    std::string sub_name = sub->get_name();
    // check for conflicts with existing names and aliases
    if (m_commands.count(sub_name))
    {
      throw std::invalid_argument("subcommand name '" + sub_name +
                                  "' already exists.");
    }
    for (std::map<std::string, command_ptr>::const_iterator it =
             m_commands.begin();
         it != m_commands.end(); ++it)
    {
      const std::pair<const std::string, command_ptr> &pair = *it;
      if (pair.second->has_alias(sub_name))
      {
        throw std::invalid_argument("subcommand name '" + sub_name +
                                    "' conflicts with an existing alias.");
      }
      const std::vector<std::string> &aliases = sub->get_aliases();
      for (std::vector<std::string>::const_iterator alias_it = aliases.begin();
           alias_it != aliases.end(); ++alias_it)
      {
        const std::string &alias = *alias_it;
        if (pair.first == alias || pair.second->has_alias(alias))
        {
          throw std::invalid_argument(
              "subcommand alias '" + alias +
              "' conflicts with an existing name or alias.");
        }
      }
    }
    // also check the new subcommand's aliases against its own name
    auto &aliases = sub->get_aliases();
    for (auto alias = aliases.begin(); alias != aliases.end(); alias++)
    {
      if (*alias == sub_name)
      {
        throw std::invalid_argument("subcommand alias '" + *alias +
                                    "' cannot be the same as its name '" +
                                    sub_name + "'.");
      }
    }

    m_commands[sub_name] = sub;
    return *this;
  }

  // add an alias to this command
  Command &add_alias(const std::string &alias)
  {
    if (alias == m_name)
    {
      throw std::invalid_argument(
          "alias cannot be the same as the command name '" + m_name + "'.");
    }
    // could add checks here to ensure alias doesn't conflict with other sibling
    // commands/aliases if added to a parent
    m_aliases.push_back(alias);
    return *this;
  }

  // set the function pointer handler
  Command &set_handler(CommandHandler handler)
  {
    m_handler = handler;
    return *this;
  }

  const std::string &get_name() const
  {
    return m_name;
  }
  const std::string &get_help() const
  {
    return m_help;
  }
  const std::map<std::string, command_ptr> &get_commands() const
  {
    return m_commands;
  }
  const std::vector<ArgumentDef> &get_keyword_args() const
  {
    return m_kw_args;
  }
  const std::vector<ArgumentDef> &get_positional_args() const
  {
    return m_pos_args;
  }
  const std::vector<std::string> &get_aliases() const
  {
    return m_aliases;
  }
  CommandHandler get_handler() const
  {
    return m_handler;
  }

  ParseResult parse(const std::vector<lexer::Token> &tokens,
                    size_t &current_token_index,
                    const std::string &command_path_prefix) const;

  // generate help text for this command and its subcommands
  void generate_help(std::ostream &os,
                     const std::string &command_path_prefix) const
  {
    // calculate max widths for alignment
    size_t max_pos_arg_width = 0;
    for (std::vector<ArgumentDef>::const_iterator it = m_pos_args.begin();
         it != m_pos_args.end(); ++it)
    {
      const ArgumentDef &arg = *it;
      max_pos_arg_width = std::max(max_pos_arg_width, arg.name.length());
    }

    size_t max_kw_arg_width = 0;
    for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
         it != m_kw_args.end(); ++it)
    {
      const ArgumentDef &arg = *it;
      max_kw_arg_width =
          std::max(max_kw_arg_width, arg.get_display_name().length());
    }

    size_t max_cmd_width = 0;
    for (std::map<std::string, command_ptr>::const_iterator it =
             m_commands.begin();
         it != m_commands.end(); ++it)
    {
      size_t current_cmd_width = it->first.length();
      const std::vector<std::string> &aliases = it->second->get_aliases();
      if (!aliases.empty())
      {
        current_cmd_width += 3; // for " (" and ")"
        for (size_t i = 0; i < aliases.size(); ++i)
        {
          current_cmd_width += aliases[i].length();
          if (i < aliases.size() - 1)
          {
            current_cmd_width += 2; // for ", "
          }
        }
      }
      max_cmd_width = std::max(max_cmd_width, current_cmd_width);
    }

    // add a buffer for spacing
    const size_t width_buffer = 2;
    max_pos_arg_width += width_buffer;
    max_kw_arg_width += width_buffer;
    max_cmd_width += width_buffer;

    std::string full_command_path =
        command_path_prefix.empty() ? m_name : command_path_prefix + m_name;
    if (!command_path_prefix.empty() &&
        command_path_prefix[command_path_prefix.length() - 1] != ' ')
    {
      full_command_path = command_path_prefix + " " + m_name;
    }
    else
    {
      full_command_path = command_path_prefix + m_name;
    }

    os << "Usage: " << full_command_path;
    // collect display names for positional args
    std::string positional_usage;
    for (std::vector<ArgumentDef>::const_iterator it = m_pos_args.begin();
         it != m_pos_args.end(); ++it)
    {
      const ArgumentDef &pos_arg = *it;
      positional_usage += " ";
      positional_usage += (pos_arg.required ? "<" : "[");
      positional_usage += pos_arg.name;
      positional_usage += (pos_arg.required ? ">" : "]");
      if (pos_arg.type == ArgType_Multiple)
        positional_usage += "...";
    }

    if (!m_commands.empty())
      os << " <command>";

    os << positional_usage;

    if (!m_kw_args.empty())
      os << " [options]";

    os << "\n\n";

    if (!m_help.empty())
    {
      os << m_help << "\n\n";
    }

    // info about positional arguments
    if (!m_pos_args.empty())
    {
      os << "Arguments:\n";
      for (std::vector<ArgumentDef>::const_iterator it = m_pos_args.begin();
           it != m_pos_args.end(); ++it)
      {
        const ArgumentDef &arg = *it;
        os << "  " << std::left << std::setw(max_pos_arg_width) << arg.name
           << arg.help;
        // check default value is not none
        if (!arg.default_value.is_none())
        {
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
    if (!m_kw_args.empty())
    {
      os << "Options:\n";
      for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
           it != m_kw_args.end(); ++it)
      {
        const ArgumentDef &arg = *it;
        os << "  " << std::left << std::setw(max_kw_arg_width)
           << arg.get_display_name() << arg.help;
        if (!arg.default_value.is_none())
        {
          // only show non-default bools (true) or non-bool defaults
          const bool *b_val = arg.default_value.get_bool();
          if (!b_val ||
              *b_val == true)
          { // show default if it's true, or if not a bool
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
    if (!m_commands.empty())
    {
      os << "Commands:\n";
      for (std::map<std::string, command_ptr>::const_iterator it =
               m_commands.begin();
           it != m_commands.end(); ++it)
      {
        std::string cmd_display = it->first;
        const std::vector<std::string> &aliases = it->second->get_aliases();
        if (!aliases.empty())
        {
          cmd_display += " (";
          for (size_t i = 0; i < aliases.size(); ++i)
          {
            cmd_display += aliases[i];
            if (i < aliases.size() - 1)
            {
              cmd_display += ", ";
            }
          }
          cmd_display += ")";
        }
        os << "  " << std::left << std::setw(max_cmd_width) << cmd_display
           << it->second->get_help() << "\n";
      }
      os << "\nRun '" << full_command_path
         << " <command> --help' for more information on a command.\n";
    }
  }

  void set_name(const std::string &new_name)
  {
    m_name = new_name;
  }

private:
  // private copy constructor and assignment operator to prevent copying
  Command(const Command &);
  Command &operator=(const Command &);

  std::string m_name;
  std::string m_help;
  std::vector<ArgumentDef> m_kw_args;
  std::vector<ArgumentDef> m_pos_args;
  std::map<std::string, command_ptr> m_commands;
  std::vector<std::string> m_aliases;

public:
  CommandHandler m_handler;

private:
  // helper to check if the token at the given index is a flag/option token
  bool is_flag_token(const std::vector<lexer::Token> &tokens,
                     size_t index) const
  {
    if (index >= tokens.size())
      return false;
    lexer::TokenKind kind = tokens[index].get_kind();
    return kind == lexer::TokFlagShort || kind == lexer::TokFlagLong;
  }

  // find keyword argument definition by flag name (use iterator loop)
  const ArgumentDef *find_keyword_arg(
      const std::string &flag_name_value, // name part only (e.g., "f", "force")
      bool is_short_flag_kind) const
  {
    for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
         it != m_kw_args.end(); ++it)
    {
      const ArgumentDef &arg = *it;
      if (arg.name == flag_name_value)
      {
        return &arg;
      }
      for (size_t i = 0; i < arg.aliases.size(); ++i)
      {
        const std::string &alias = arg.aliases[i];
        if (is_short_flag_kind)
        {
          // match "-f" for short flag "f"
          if (alias.length() == 2 && alias[0] == '-' &&
              alias.substr(1) == flag_name_value)
          {
            return &arg;
          }
        }
        else
        {
          // match "--force" for long flag "force"
          if (alias.length() > 2 && alias.substr(0, 2) == "--" &&
              alias.substr(2) == flag_name_value)
          {
            return &arg;
          }
        }
      }
    }
    return nullptr;
  }

  // helper to parse a single token into an ArgValue based on expected type
  // returns ArgValue (which manages memory for string/vector)
  ArgValue parse_token_value(const lexer::Token &token,
                             ArgType expected_type) const
  {
    lexer::TokenKind kind = token.get_kind();

    // Coerce identifier/strlit values for flags to boolean values
    if (expected_type == ArgType_Flag && (kind == lexer::TokId || kind == lexer::TokStrLit))
    {
      std::string s_val = (kind == lexer::TokId) ? token.get_id_value()
                                                 : token.get_str_lit_value();
      return ArgValue(s_val == "true");
    }

    // allow broader range of tokens to be interpreted as strings if expected
    bool expect_string =
        (expected_type == ArgType_Single || expected_type == ArgType_Multiple ||
         expected_type == ArgType_Positional);

    switch (kind)
    {
    case lexer::TokIntLit:
      if (expected_type == ArgType_Single ||
          expected_type == ArgType_Positional)
        return ArgValue(token.get_int_value());
      else if (expect_string) // allow int literal as string if needed
        return ArgValue(token.get_str_lit_value());
      break;
    case lexer::TokFloatLit:
      if (expected_type == ArgType_Single ||
          expected_type == ArgType_Positional)
        return ArgValue(token.get_float_value());
      else if (expect_string) // allow float literal as string if needed
        return ArgValue(token.get_str_lit_value());
      break;
    case lexer::TokTrue:
      if (expect_string)
        return ArgValue("true");
      else
        return ArgValue(true);
      break;
    case lexer::TokFalse:
      if (expect_string)
        return ArgValue("false");
      else
        return ArgValue(false);
      break;
    case lexer::TokStrLit:
      if (expect_string)
        return ArgValue(token.get_str_lit_value());
      break;
    case lexer::TokId:
      if (expect_string)
        return ArgValue(token.get_id_value());
      break;
    case lexer::TokTimes:
      if (expect_string)
        return ArgValue("*");
      break;
    default:
      break;
    }

    return ArgValue();
  }

  // helper to add the standard help argument
  void ensure_help_argument()
  {
    bool help_exists = false;
    for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
         it != m_kw_args.end(); ++it)
    {
      if (it->name == "help")
      {
        help_exists = true;
        break;
      }
    }
    if (!help_exists)
    {
      std::vector<std::string> help_aliases;
      help_aliases.push_back("-h");
      help_aliases.push_back("--help");
      m_kw_args.push_back(
          ArgumentDef("help", help_aliases, "show this help message and exit",
                      ArgType_Flag, false, ArgValue(false), true));
    }
  }

  // check if the command has a specific alias
  bool has_alias(const std::string &alias) const
  {
    return std::find(m_aliases.begin(), m_aliases.end(), alias) !=
           m_aliases.end();
  }
};

// stores the outcome of the parsing process
class ParseResult
{
public:
  enum ParserStatus
  {
    ParserStatus_Success,
    ParserStatus_HelpRequested,
    ParserStatus_ParseError
  };

  ParserStatus status;       // no default initializer
  int exit_code;             // exit code returned by handler or set by parser
  std::string error_message; // error message if status is parse_error
  std::string command_path;  // full path of the executed command (e.g., "git
                             // remote add")

  // map storing parsed keyword argument values (name -> value)
  std::map<std::string, ArgValue> keyword_values;
  // map storing parsed positional argument values (name -> value)
  std::map<std::string, ArgValue> positional_values;

  // pointer to the command definition for default value lookup
  const Command *m_command;

  ParseResult()
      : status(ParserStatus_Success), exit_code(0), m_command(nullptr)
  {
  }

  // check if a keyword arg was provided (doesn't check type)
  bool has_kw_arg(const std::string &name) const
  {
    return keyword_values.find(name) != keyword_values.end();
  }

private:
  // Template helpers for getter methods
  template <typename T>
  const T *get_arg_value_ptr(const std::map<std::string, ArgValue> &values,
                             const std::string &name,
                             const T *(ArgValue::*getter)() const) const
  {
    std::map<std::string, ArgValue>::const_iterator it = values.find(name);
    return (it != values.end()) ? (it->second.*getter)() : NULL;
  }

public:
  // getters returning pointers, nullptr if missing or wrong type
  const bool *get_kw_arg_bool(const std::string &name) const
  {
    return get_arg_value_ptr(keyword_values, name, &ArgValue::get_bool);
  }

  const long long *get_kw_arg_int(const std::string &name) const
  {
    return get_arg_value_ptr(keyword_values, name, &ArgValue::get_int);
  }

  const double *get_kw_arg_double(const std::string &name) const
  {
    return get_arg_value_ptr(keyword_values, name, &ArgValue::get_double);
  }

  const std::string *get_kw_arg_string(const std::string &name) const
  {
    return get_arg_value_ptr(keyword_values, name, &ArgValue::get_string);
  }

  const std::vector<std::string> *get_kw_arg_list(const std::string &name) const
  {
    return get_arg_value_ptr(keyword_values, name, &ArgValue::get_string_vector);
  }

private:
  // Template helper for find methods with default values
  template <typename T>
  T find_arg_with_default(const std::string &name,
                          const T *(ParseResult::*getter)(const std::string &) const,
                          const T *(ArgValue::*def_getter)() const,
                          const T &fallback) const
  {
    const T *ptr = (this->*getter)(name);
    if (ptr)
      return *ptr;
    if (m_command)
    {
      const std::vector<ArgumentDef> &defs = m_command->get_keyword_args();
      for (size_t i = 0; i < defs.size(); ++i)
      {
        if (defs[i].name == name)
        {
          const T *def_ptr = (defs[i].default_value.*def_getter)();
          return def_ptr ? *def_ptr : fallback;
        }
      }
    }
    return fallback;
  }

public:
  // getters returning value or the argument's default if missing/wrong type
  bool find_kw_arg_bool(const std::string &name) const
  {
    return find_arg_with_default(name, &ParseResult::get_kw_arg_bool, &ArgValue::get_bool, false);
  }

  long long find_kw_arg_int(const std::string &name) const
  {
    return find_arg_with_default(name, &ParseResult::get_kw_arg_int, &ArgValue::get_int, (long long)0);
  }

  double find_kw_arg_double(const std::string &name) const
  {
    return find_arg_with_default(name, &ParseResult::get_kw_arg_double, &ArgValue::get_double, 0.0);
  }

  std::string find_kw_arg_string(const std::string &name) const
  {
    return find_arg_with_default(name, &ParseResult::get_kw_arg_string, &ArgValue::get_string, std::string(""));
  }

  std::vector<std::string> find_kw_arg_list(const std::string &name) const
  {
    return find_arg_with_default(name, &ParseResult::get_kw_arg_list, &ArgValue::get_string_vector, std::vector<std::string>());
  }

  // --- Positional Argument Getters (by Name) ---

  // check if a positional arg was provided (doesn't check type)
  bool has_pos_arg(const std::string &name) const
  {
    return positional_values.find(name) != positional_values.end();
  }

  // getters returning pointers, nullptr if missing or wrong type
  const bool *get_pos_arg_bool(const std::string &name) const
  {
    return get_arg_value_ptr(positional_values, name, &ArgValue::get_bool);
  }

  const long long *get_pos_arg_int(const std::string &name) const
  {
    return get_arg_value_ptr(positional_values, name, &ArgValue::get_int);
  }

  const double *get_pos_arg_double(const std::string &name) const
  {
    return get_arg_value_ptr(positional_values, name, &ArgValue::get_double);
  }

  const std::string *get_pos_arg_string(const std::string &name) const
  {
    return get_arg_value_ptr(positional_values, name, &ArgValue::get_string);
  }

  const std::vector<std::string> *get_pos_arg_list(const std::string &name) const
  {
    return get_arg_value_ptr(positional_values, name, &ArgValue::get_string_vector);
  }

private:
  // Template helper for positional find methods
  template <typename T>
  T find_pos_arg_with_default(const std::string &name,
                              const T *(ParseResult::*getter)(const std::string &) const,
                              const T *(ArgValue::*def_getter)() const,
                              const T &fallback) const
  {
    const T *ptr = (this->*getter)(name);
    if (ptr)
      return *ptr;
    if (m_command)
    {
      const std::vector<ArgumentDef> &defs = m_command->get_positional_args();
      for (size_t i = 0; i < defs.size(); ++i)
      {
        if (defs[i].name == name)
        {
          const T *def_ptr = (defs[i].default_value.*def_getter)();
          return def_ptr ? *def_ptr : fallback;
        }
      }
    }
    return fallback;
  }

public:
  // Simplified positional getters returning value or default
  bool find_pos_arg_bool(const std::string &name) const
  {
    return find_pos_arg_with_default(name, &ParseResult::get_pos_arg_bool, &ArgValue::get_bool, false);
  }

  long long find_pos_arg_int(const std::string &name) const
  {
    return find_pos_arg_with_default(name, &ParseResult::get_pos_arg_int, &ArgValue::get_int, (long long)0);
  }

  double find_pos_arg_double(const std::string &name) const
  {
    return find_pos_arg_with_default(name, &ParseResult::get_pos_arg_double, &ArgValue::get_double, 0.0);
  }

  std::string find_pos_arg_string(const std::string &name) const
  {
    return find_pos_arg_with_default(name, &ParseResult::get_pos_arg_string, &ArgValue::get_string, std::string(""));
  }

  std::vector<std::string> find_pos_arg_list(const std::string &name) const
  {
    return find_pos_arg_with_default(name, &ParseResult::get_pos_arg_list, &ArgValue::get_string_vector, std::vector<std::string>());
  }
};

inline ParseResult
Command::parse(const std::vector<lexer::Token> &tokens,
               size_t &current_token_index,
               const std::string &command_path_prefix) const
{
  ParseResult result;
  result.m_command = this;
  result.command_path = command_path_prefix + m_name;
  // initialize exit code for potential errors or help requests
  result.exit_code = 0; // default to 0 for success before handler runs

  std::map<std::string, bool> keyword_args_seen;
  size_t current_positional_arg_index = 0;

  for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
       it != m_kw_args.end(); ++it)
  {
    const ArgumentDef &arg = *it;
    if (arg.is_help_flag)
    {
      continue;
    }
    result.keyword_values[arg.name] = arg.default_value;
  }

  while (current_token_index < tokens.size())
  {
    const lexer::Token &token = tokens[current_token_index];
    lexer::TokenKind kind = token.get_kind();

    // check for keyword arguments/flags (TokFlagShort, TokFlagLong)
    if (kind == lexer::TokFlagShort || kind == lexer::TokFlagLong)
    {
      std::string flag_name_str =
          token.get_id_value(); // get name without dashes
      bool is_short_flag_kind = (kind == lexer::TokFlagShort);

      if (is_short_flag_kind && flag_name_str.length() > 1)
      {
        current_token_index++;

        for (size_t i = 0; i < flag_name_str.length(); ++i)
        {
          std::string single_flag_char(1, flag_name_str[i]);
          const ArgumentDef *matched_arg =
              find_keyword_arg(single_flag_char, true);

          if (!matched_arg)
          {
            result.status = ParseResult::ParserStatus_ParseError;
            result.error_message = "unknown option in combined flags: -";
            result.error_message += single_flag_char;
            std::cerr << "error: " << result.error_message << "\n\n";
            generate_help(std::cerr, command_path_prefix);
            result.exit_code = 1;
            return result;
          }

          if (matched_arg->is_help_flag)
          {
            generate_help(std::cout, command_path_prefix);
            result.status = ParseResult::ParserStatus_HelpRequested;
            result.exit_code = 0; // help request is a successful exit
            return result;
          }

          // ensure combined flags are actually boolean flags
          if (matched_arg->type != ArgType_Flag)
          {
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

      if (!matched_arg)
      {
        result.status = ParseResult::ParserStatus_ParseError;
        result.error_message = "unknown option: ";
        result.error_message += (is_short_flag_kind ? "-" : "--");
        result.error_message += flag_name_str;

        // Suggest similar option
        size_t best_dist = (size_t)-1;
        std::string best_match;
        for (size_t i = 0; i < m_kw_args.size(); ++i)
        {
          const ArgumentDef &arg = m_kw_args[i];
          // Check canonical name
          size_t dist = parser::levenshtein_distance(flag_name_str, arg.name);
          if (dist < best_dist)
          {
            best_dist = dist;
            best_match = "--" + arg.name;
          }
          // Check aliases
          for (size_t j = 0; j < arg.aliases.size(); ++j)
          {
            std::string alias = arg.aliases[j];
            // Remove leading dashes for comparison
            std::string alias_cmp = alias;
            while (!alias_cmp.empty() && alias_cmp[0] == '-')
              alias_cmp = alias_cmp.substr(1);
            dist = parser::levenshtein_distance(flag_name_str, alias_cmp);
            if (dist < best_dist)
            {
              best_dist = dist;
              best_match = alias;
            }
          }
        }
        std::cerr << "Error: " << result.error_message << "\n";

        if (best_dist != (size_t)-1 && best_dist <= 2)
        {
          std::cerr << "Did you mean '" << best_match << "'?\n\n";
        }
        else
        {
          std::cerr << "\n";
        }

        generate_help(std::cerr, command_path_prefix);
        result.exit_code = 1;
        return result;
      }

      // if this is the specifically marked help flag, handle it immediately.
      if (matched_arg->is_help_flag)
      {
        generate_help(std::cout, command_path_prefix);
        result.status = ParseResult::ParserStatus_HelpRequested;
        result.exit_code = 0; // help request is a successful exit
        return result;
      }

      current_token_index++; // consume the flag token itself
      keyword_args_seen[matched_arg->name] = true;

      // handle argument value based on type
      if (matched_arg->type == ArgType_Flag)
      {
        // default to true
        ArgValue flag_value(true);

        // check for an optional value (e.g., --flag false)
        if (current_token_index < tokens.size() &&
            !is_flag_token(tokens, current_token_index))
        {
          const lexer::Token &value_token = tokens[current_token_index];
          ArgValue parsed_value =
              parse_token_value(value_token, ArgType_Flag);

          if (!parsed_value.is_none())
          {
            flag_value = parsed_value;
            current_token_index++; // consume the value token if one exists
          }
        }
        result.keyword_values[matched_arg->name] = flag_value;
      }
      else
      {
        // check if next token exists and is not another flag
        if (current_token_index >= tokens.size() ||
            tokens[current_token_index].get_kind() == lexer::TokFlagShort ||
            tokens[current_token_index].get_kind() == lexer::TokFlagLong)
        {
          result.status = ParseResult::ParserStatus_ParseError;
          result.error_message = "option " + matched_arg->get_display_name() +
                                 " requires a value.";
          std::cerr << "error: " << result.error_message << "\n\n";
          generate_help(std::cerr, command_path_prefix);
          result.exit_code = 1;
          return result;
        }

        const lexer::Token &value_token = tokens[current_token_index];
        ArgValue parsed_value =
            parse_token_value(value_token, matched_arg->type);
        if (parsed_value.is_none())
        {
          result.status = ParseResult::ParserStatus_ParseError;
          result.error_message =
              "invalid value for option " + matched_arg->get_display_name();
          std::cerr << "error: " << result.error_message << "\n\n";
          generate_help(std::cerr, command_path_prefix);
          result.exit_code = 1;
          return result;
        }

        if (matched_arg->type == ArgType_Single)
        {
          result.keyword_values[matched_arg->name] = parsed_value;
          current_token_index++;
        }
        else if (matched_arg->type == ArgType_Multiple)
        {
          // ensure value type is string for multiple
          if (!parsed_value.is_string())
          {
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
              !map_it->second.is_string_vector() || map_it->second.is_none())
          {
            result.keyword_values[matched_arg->name] =
                ArgValue(std::vector<std::string>());
            map_it = result.keyword_values.find(matched_arg->name);
          }

          std::vector<std::string> *vec =
              map_it->second.get_string_vector_ptr_unsafe();
          if (vec)
          {
            const std::string *first_val_str_ptr = parsed_value.get_string();
            if (first_val_str_ptr)
            {
              vec->push_back(*first_val_str_ptr);
            }

            current_token_index++;

            // keep consuming values until next flag or end
            while (
                current_token_index < tokens.size() &&
                tokens[current_token_index].get_kind() != lexer::TokFlagShort &&
                tokens[current_token_index].get_kind() != lexer::TokFlagLong)
            {
              const lexer::Token &next_value_token =
                  tokens[current_token_index];
              ArgValue next_parsed_value =
                  parse_token_value(next_value_token, ArgType_Single);
              const std::string *next_val_str_ptr =
                  next_parsed_value.get_string();
              if (next_val_str_ptr)
              {
                vec->push_back(*next_val_str_ptr);
                current_token_index++;
              }
              else
              {
                break;
              }
            }
          }
          else
          {
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
    if (kind == lexer::TokId)
    {
      std::string potential_subcommand_or_alias = token.get_id_value();
      command_ptr matched_subcommand;

      // first, check if it's a direct name match
      auto sub_it = m_commands.find(potential_subcommand_or_alias);
      if (sub_it != m_commands.end())
      {
        matched_subcommand = sub_it->second;
      }
      else
      {
        // if not a direct name match, check aliases
        for (std::map<std::string, command_ptr>::const_iterator it2 =
                 m_commands.begin();
             it2 != m_commands.end(); ++it2)
        {
          const std::pair<const std::string, command_ptr> &pair = *it2;
          if (pair.second->has_alias(potential_subcommand_or_alias))
          {
            // prevent aliasing to multiple commands
            if (matched_subcommand)
            {
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
      if (matched_subcommand)
      {
        current_token_index++; // consume the subcommand/alias token
        ParseResult sub_result = matched_subcommand->parse(
            tokens, current_token_index, result.command_path + " ");

        // propagate the result (success, error, or help request) from the
        // subcommand.
        return sub_result;
      }
      else if (!m_commands.empty() && current_positional_arg_index == 0)
      {
        // Only treat as unknown subcommand if we haven't started processing positional args
        // Suggest similar subcommand/group
        size_t best_dist = (size_t)-1;
        std::string best_match;
        for (std::map<std::string, command_ptr>::const_iterator it2 = m_commands.begin();
             it2 != m_commands.end(); ++it2)
        {
          // Check subcommand name
          size_t dist = parser::levenshtein_distance(potential_subcommand_or_alias, it2->first);
          if (dist < best_dist)
          {
            best_dist = dist;
            best_match = it2->first;
          }
          // Check aliases
          const std::vector<std::string> &aliases = it2->second->get_aliases();
          for (size_t j = 0; j < aliases.size(); ++j)
          {
            dist = parser::levenshtein_distance(potential_subcommand_or_alias, aliases[j]);
            if (dist < best_dist)
            {
              best_dist = dist;
              best_match = aliases[j];
            }
          }
        }
        result.status = ParseResult::ParserStatus_ParseError;
        result.error_message = "unknown command or group: " + potential_subcommand_or_alias;
        std::cerr << "Error: " << result.error_message << "\n";

        if (best_dist != (size_t)-1 && best_dist <= 2)
        {
          std::cerr << "Did you mean '" << best_match << "'?\n\n";
        }
        else
        {
          std::cerr << "\n";
        }
        generate_help(std::cerr, command_path_prefix);
        result.exit_code = 1;
        return result;
      }
      // If we reach here and have subcommands but current_positional_arg_index > 0,
      // fall through to positional argument processing
    }

    // if not a flag/option or subcommand, treat as positional argument
    if (current_positional_arg_index < m_pos_args.size())
    {
      const ArgumentDef &pos_arg_def = m_pos_args[current_positional_arg_index];
      ArgValue parsed_value = parse_token_value(token, pos_arg_def.type);

      if (parsed_value.is_none())
      {
        result.status = ParseResult::ParserStatus_ParseError;
        result.error_message =
            "invalid value for positional argument '" + pos_arg_def.name + "'";
        std::cerr << "error: " << result.error_message << "\n\n";
        generate_help(std::cerr, command_path_prefix);
        result.exit_code = 1;
        return result;
      }

      if (pos_arg_def.type == ArgType_Single)
      {
        result.positional_values[pos_arg_def.name] = parsed_value;
        current_token_index++;
        current_positional_arg_index++;
      }
      else if (pos_arg_def.type == ArgType_Multiple)
      {
        // ensure value type is string for multiple positional
        if (!parsed_value.is_string())
        {
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
        if (first_val_str_ptr)
        { // should always be true
          values.push_back(*first_val_str_ptr);
        }

        current_token_index++;
        while (current_token_index < tokens.size() &&
               !is_flag_token(
                   tokens, current_token_index) /* && !is_subcommand(...) */)
        {
          // todo: add is_subcommand check more robustly?
          // currently relies on subcommand check earlier in loop
          const lexer::Token &next_value_token = tokens[current_token_index];
          // parse subsequent as strings
          ArgValue next_parsed_value =
              parse_token_value(next_value_token, ArgType_Single);
          const std::string *next_val_str_ptr = next_parsed_value.get_string();
          if (next_val_str_ptr)
          {
            values.push_back(*next_val_str_ptr);
            current_token_index++;
          }
          else
          {
            break;
          }
        }
        // add the vector to positional args map using ArgValue constructor
        result.positional_values[pos_arg_def.name] = ArgValue(values);
        current_positional_arg_index++;
      }
    }
    else
    {
      // Only treat as unexpected if not a string or identifier (quoted or
      // unquoted)
      if (token.get_kind() != lexer::TokId &&
          token.get_kind() != lexer::TokStrLit)
      {
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
      // Otherwise, forcibly advance the token and positional argument index to
      // avoid infinite loop
      current_token_index++;
      current_positional_arg_index++;
    }
  }

  // check for required keyword arguments
  for (std::vector<ArgumentDef>::const_iterator it = m_kw_args.begin();
       it != m_kw_args.end(); ++it)
  {
    const ArgumentDef &arg = *it;
    // skip check for the automatic help flag
    if (arg.is_help_flag)
      continue;
    if (arg.required &&
        keyword_args_seen.find(arg.name) == keyword_args_seen.end())
    {
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
  if (m_pos_args.size() > current_positional_arg_index)
  {
    for (size_t i = current_positional_arg_index; i < m_pos_args.size(); ++i)
    {
      const ArgumentDef &pos_arg_def = m_pos_args[i];
      if (pos_arg_def.required)
      {
        result.status = ParseResult::ParserStatus_ParseError;
        result.error_message =
            "missing required positional argument: " + pos_arg_def.name;
        std::cerr << "error: " << result.error_message << "\n\n";
        generate_help(std::cerr, command_path_prefix);
        result.exit_code = 1;
        return result;
      }
      else
      {
        // add default value for optional missing positional args
        result.positional_values[pos_arg_def.name] = pos_arg_def.default_value;
      }
    }
  }

  // only execute the handler if parsing was successful and no subcommand took
  // over.
  if (m_handler && result.status == ParseResult::ParserStatus_Success)
  {
    result.exit_code = m_handler(result);
  }

  // If this command is a group (has subcommands), has no handler, and parsing
  // succeeded, print help and set status to HelpRequested.
  if (!m_handler && !m_commands.empty() &&
      result.status == ParseResult::ParserStatus_Success)
  {
    generate_help(std::cout, command_path_prefix);
    result.status = ParseResult::ParserStatus_HelpRequested;
    result.exit_code = 0;
  }

  return result;
}

class ArgumentParser
{
public:
  explicit ArgumentParser(std::string prog_name, std::string description = "")
      : m_program_name(prog_name), m_program_desc(description),
        m_root_cmd(command_ptr(new Command(prog_name, description)))
  {
  }

  ~ArgumentParser() = default;

  void update_program_name(const std::string &new_program_name)
  {
    m_program_name = new_program_name;
    if (m_root_cmd)
    {
      m_root_cmd->set_name(new_program_name);
    }
  }

  // get a reference to the root command to add arguments or subcommands
  Command &get_root_command()
  {
    return *m_root_cmd;
  }

  // parse command line arguments from main(argc, argv)
  ParseResult parse(int argc, char *argv[])
  {
    if (argc < 1)
    {
      ParseResult error_result;
      error_result.status = ParseResult::ParserStatus_ParseError;
      error_result.error_message = "invalid arguments provided (argc < 1)";
      error_result.exit_code = 1;
      return error_result;
    }

    // Convert argv[1..argc-1] to vector<string>
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i)
    {
      args.push_back(std::string(argv[i]));
    }

    // Use lexer to parse each argument appropriately
    std::vector<lexer::Token> tokens;
    size_t pos = 0;
    for (size_t i = 0; i < args.size(); ++i)
    {
      const std::string &arg = args[i];
      lexer::Span span(pos, pos + arg.size());

      if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-')
      {
        // Long flag: --flag
        tokens.push_back(
            lexer::Token::make_long_flag(arg.c_str() + 2, arg.size() - 2, span));
      }
      else if (arg.size() > 1 && arg[0] == '-' && arg != "-")
      {
        // Short flag: -f or -abc
        tokens.push_back(
            lexer::Token::make_short_flag(arg.c_str() + 1, arg.size() - 1, span));
      }
      else if ((arg.size() >= 2 &&
                ((arg[0] == '"' && arg[arg.size() - 1] == '"') ||
                 (arg[0] == '\'' && arg[arg.size() - 1] == '\''))))
      {
        // Quoted string literal (remove quotes)
        tokens.push_back(
            lexer::Token::make_str_lit(arg.c_str() + 1, arg.size() - 2, span));
      }
      else
      {
        // Use lexer to determine if this is numeric or identifier
        try
        {
          lexer::Src arg_source = lexer::Src::from_string(arg, "<arg>");
          std::vector<lexer::Token> arg_tokens = lexer::Lexer::tokenize(arg_source);

          // Filter out EOF token and check if we got exactly one meaningful token
          if (arg_tokens.size() >= 1 && arg_tokens[0].get_kind() != lexer::TokEof)
          {
            lexer::Token parsed_token = arg_tokens[0];

            // If the lexer successfully parsed it as a single token that covers the whole argument,
            // and it's a numeric or boolean literal, use that. Otherwise treat as identifier.
            if (arg_tokens.size() == 2 && arg_tokens[1].get_kind() == lexer::TokEof &&
                (parsed_token.get_kind() == lexer::TokIntLit ||
                 parsed_token.get_kind() == lexer::TokFloatLit ||
                 parsed_token.get_kind() == lexer::TokTrue ||
                 parsed_token.get_kind() == lexer::TokFalse))
            {
              // Create a new token with the correct span for our context
              switch (parsed_token.get_kind())
              {
              case lexer::TokIntLit:
                tokens.push_back(lexer::Token::make_int_lit(
                    parsed_token.get_int_value(), parsed_token.get_int_base(), span));
                break;
              case lexer::TokFloatLit:
                tokens.push_back(lexer::Token::make_float_lit(
                    parsed_token.get_float_value(), parsed_token.has_float_exponent(), span));
                break;
              case lexer::TokTrue:
                tokens.push_back(lexer::Token(lexer::TokTrue, span));
                break;
              case lexer::TokFalse:
                tokens.push_back(lexer::Token(lexer::TokFalse, span));
                break;
              default:
                // Fallback to identifier
                tokens.push_back(lexer::Token::make_id(arg.c_str(), arg.size(), span));
                break;
              }
            }
            else
            {
              // Multiple tokens or not a literal - treat as identifier
              tokens.push_back(lexer::Token::make_id(arg.c_str(), arg.size(), span));
            }
          }
          else
          {
            // No meaningful tokens - treat as identifier
            tokens.push_back(lexer::Token::make_id(arg.c_str(), arg.size(), span));
          }
        }
        catch (const lexer::LexError &)
        {
          // Lexer failed - treat as identifier
          tokens.push_back(lexer::Token::make_id(arg.c_str(), arg.size(), span));
        }
      }
      pos += arg.size() + 1;
    }

    size_t token_index = 0;
    if (!m_root_cmd)
    {
      ParseResult error_result;
      error_result.status = ParseResult::ParserStatus_ParseError;
      error_result.error_message =
          "ArgumentParser is not initialized correctly.";
      error_result.exit_code = 1;
      return error_result;
    }
    ParseResult result = m_root_cmd->parse(tokens, token_index, "");
    if (result.status == ParseResult::ParserStatus_Success &&
        token_index < tokens.size())
    {
      result.status = ParseResult::ParserStatus_ParseError;
      std::stringstream ss;
      ss << "unexpected arguments starting from: ";
      tokens[token_index].print(ss);
      result.error_message = ss.str();
      std::cerr << "error: " << result.error_message << "\n\n";
      m_root_cmd->generate_help(std::cerr, "");
      result.exit_code = 1;
      return result;
    }
    return result;
  }

  // parse command line arguments from a single string
  ParseResult parse(const std::string &command_line)
  {
    if (!m_root_cmd)
    {
      ParseResult error_result;
      error_result.status = ParseResult::ParserStatus_ParseError;
      error_result.error_message =
          "ArgumentParser is not initialized correctly.";
      error_result.exit_code = 1;
      return error_result;
    }

    try
    {
      lexer::Src source = lexer::Src::from_string(command_line, "<cli>");
      std::vector<lexer::Token> tokens = lexer::Lexer::tokenize(source);

      // filter out TokEof at the end
      if (!tokens.empty() && tokens.back().get_kind() == lexer::TokEof)
      {
        tokens.pop_back();
      }

      size_t token_index = 0;
      ParseResult result = m_root_cmd->parse(tokens, token_index, "");

      // return early in the event of an error or help request from the parse
      // call
      if (result.status == ParseResult::ParserStatus_ParseError)
      {
        return result;
      }
      if (result.status == ParseResult::ParserStatus_HelpRequested)
      {
        return result;
      }

      // parsing succeeded, but not all tokens were consumed (and no subcommand
      // handled them)
      if (token_index < tokens.size())
      {
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
    }
    catch (const lexer::LexError &e)
    {
      std::cerr << "lexer error: " << e.what() << std::endl;
      ParseResult error_result;
      error_result.status = ParseResult::ParserStatus_ParseError;
      error_result.error_message = e.what();
      error_result.exit_code = 1;
      // show root help on lexer error (to stderr)
      if (m_root_cmd)
        m_root_cmd->generate_help(std::cerr, "");
      return error_result;
    }
    catch (const std::exception &e)
    {
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
  command_ptr m_root_cmd;
}; // class ArgumentParser

/**
 * Generate a bash completion script for the given CLI parser.
 * @param os Output stream to write the script to.
 * @param prog_name The name of the executable (as invoked).
 * @param root_cmd The root Command object.
 */
inline void generate_bash_completion(std::ostream &os, const std::string &prog_name, const Command &root_cmd)
{
  // Helper to recursively emit command tree as bash arrays
  struct BashEmit
  {
    static void emit_command_tree(const Command &cmd, const std::string &prefix, std::ostream &os)
    {
      std::string arr_name = "_parser_cmds";
      if (!prefix.empty())
        arr_name += "_" + prefix;
      os << arr_name << "=( ";
      // Subcommands
      const std::map<std::string, command_ptr> &subs = cmd.get_commands();
      for (std::map<std::string, command_ptr>::const_iterator it = subs.begin(); it != subs.end(); ++it)
      {
        os << "'" << it->first << "' ";
        // Aliases
        const std::vector<std::string> &aliases = it->second->get_aliases();
        for (size_t j = 0; j < aliases.size(); ++j)
        {
          os << "'" << aliases[j] << "' ";
        }
      }
      os << ")\n";
      // Options
      std::string opt_arr = "_parser_opts";
      if (!prefix.empty())
        opt_arr += "_" + prefix;
      os << opt_arr << "=( ";
      const std::vector<ArgumentDef> &kw = cmd.get_keyword_args();
      for (size_t i = 0; i < kw.size(); ++i)
      {
        for (size_t j = 0; j < kw[i].aliases.size(); ++j)
        {
          os << "'" << kw[i].aliases[j] << "' ";
        }
        // Also add --<name> if not already present
        if (!kw[i].name.empty() && kw[i].type != ArgType_Positional)
        {
          std::string long_flag = "--" + kw[i].name;
          bool found = false;
          for (size_t j = 0; j < kw[i].aliases.size(); ++j)
          {
            if (kw[i].aliases[j] == long_flag)
            {
              found = true;
              break;
            }
          }
          if (!found)
            os << "'" << long_flag << "' ";
        }
      }
      os << ")\n";
      // Recurse for subcommands
      for (std::map<std::string, command_ptr>::const_iterator it = subs.begin(); it != subs.end(); ++it)
      {
        std::string sub_prefix = prefix.empty() ? it->first : (prefix + "_" + it->first);
        emit_command_tree(*it->second, sub_prefix, os);
      }
    }
  };

  // Emit arrays for all commands/subcommands
  BashEmit::emit_command_tree(root_cmd, "", os);

  // Emit the completion function
  os << "_parser_complete_" << prog_name << "() {\n";
  os << "  local cur prev words cword\n";
  os << "  _get_comp_words_by_ref -n : cur prev words cword\n";
  os << "  local i cmd_path=\"\" arr_name=\"_parser_cmds\" opt_arr=\"_parser_opts\"\n";
  os << "  local level=0\n";
  os << "  for ((i=1; i < ${#words[@]}; ++i)); do\n";
  os << "    w=${words[i]}\n";
  os << "    eval arr=\"\\${!arr_name}\"\n";
  os << "    found=0\n";
  os << "    for entry in \"${arr[@]}\"; do\n";
  os << "      if [[ \"$w\" == \"$entry\" ]]; then\n";
  os << "        cmd_path+=\"_\"$w\n";
  os << "        arr_name=\"_parser_cmds$cmd_path\"\n";
  os << "        opt_arr=\"_parser_opts$cmd_path\"\n";
  os << "        found=1\n";
  os << "        break\n";
  os << "      fi\n";
  os << "    done\n";
  os << "    if [[ $found -eq 0 ]]; then break; fi\n";
  os << "  done\n";
  os << "  eval opts=\"\\${!opt_arr}\"\n";
  os << "  eval cmds=\"\\${!arr_name}\"\n";
  os << "  COMPREPLY=( $(compgen -W \"${opts[*]} ${cmds[*]}\" -- \"$cur\") )\n";
  os << "  return 0\n";
  os << "}\n";
  os << "complete -F _parser_complete_" << prog_name << " " << prog_name << "\n";
}
} // namespace parser

#endif // PARSER_HPP
