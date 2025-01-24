/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#ifndef ZCLI_HPP
#define ZCLI_HPP

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <map>

using namespace std;

#define PROCESS_NAME_ARG 0
#define CLI_GROUP_ARG 1
#define CLI_INTERACTIVE_ARG 1
#define CLI_VERB_ARG 2
#define CLI_REMAIN_ARG_START 3

#define ZCLI_MENU_WIDTH 25
#define ZCLI_MENU_INDENT "  " // TODO(Kelosky)
#define ZCLI_FLAG_PREFIX "--"

// TODO(Kelosky): map instead of vectors for result

class ZCLIName
{
protected:
  string name;

public:
  ZCLIName(string n) : name(n) {}
  string get_name() { return name; }
};

class ZCLIRequired
{
protected:
  bool required;
  bool found;

public:
  ZCLIRequired() { required = false; }
  virtual void set_found(bool f)
  {
    found = f;
  }
  bool is_found() { return found; }

  void set_required(bool r) { required = r; }
  bool get_required() { return required; }
};

class ZCLIDescription
{
protected:
  string description;

public:
  void set_description(string d) { description = d; }
  string get_description() { return description; }
};

class ZCLIFlag : public ZCLIName
{
public:
  ZCLIFlag(string n) : ZCLIName(n) {}
  string get_flag_name() { return ZCLI_FLAG_PREFIX + name; };
};

class
    ZCLIOption : public ZCLIFlag,
                 public ZCLIRequired,
                 public ZCLIDescription
{
private:
  string value;
  vector<string> aliases;
  string default_value;
  bool default_set;

public:
  ZCLIOption(string n) : ZCLIFlag(n) { default_set = false; }
  void help_line()
  {
    string entry = get_flag_name();
    for (vector<string>::iterator it = get_aliases().begin(); it != get_aliases().end(); *it++)
    {
      entry += ("," + *it);
    }
    cerr << "  " << left << setw(ZCLI_MENU_WIDTH) << entry << "   " << get_description() << endl;
  }
  void set_value(string v) { value = v; }
  vector<string> &get_aliases() { return aliases; }
  string get_default() { return default_value; }
  void set_default(string v)
  {
    default_value = v;
    default_set = true;
  }
  bool default_provided() { return default_set; }
  string get_value() { return value; }
};

class ZCLIOptionProvider
{
protected:
  vector<ZCLIOption> options;

public:
  vector<ZCLIOption> &get_options() { return options; }
  ZCLIOption &get_option(string);
};

class ZCLIPositional : public ZCLIName,
                       public ZCLIRequired,
                       public ZCLIDescription
{
private:
  string value;

public:
  ZCLIPositional(string n) : ZCLIName(n) {}
  void help_line()
  {
    string syntax = get_required() ? "<" : "[";
    syntax += get_name();
    syntax += get_required() ? ">" : "]";
    cerr << "  " << left << setw(ZCLI_MENU_WIDTH) << syntax << "   " << get_description() << endl;
  }
  void set_value(string v) { value = v; }
  string get_value() { return value; }
};

class ZCLIPositionalProvider
{
protected:
  vector<ZCLIPositional> positionals;

public:
  vector<ZCLIPositional> &get_positionals() { return positionals; }
  ZCLIPositional &get_positional(string);
};

class ZCLIResult : public ZCLIOptionProvider, public ZCLIPositionalProvider
{
private:
public:
};

typedef ZCLIOption &(*zcli_get_option)(string); // callback
typedef int (*zcli_verb_handler)(ZCLIResult);   // handler

class ZCLIVerb : public ZCLIName, public ZCLIDescription, public ZCLIOptionProvider, public ZCLIPositionalProvider
{
private:
  zcli_verb_handler cb;

public:
  ZCLIVerb(string n) : ZCLIName(n) {}
  void set_zcli_verb_handler(zcli_verb_handler h) { cb = h; }
  zcli_verb_handler get_zcli_verb_handler() { return cb; }
  void help_line() { cerr << "  " << left << setw(ZCLI_MENU_WIDTH) << get_name() << " | " << get_description() << endl; }
  void help(string, string);
};

class ZCLIGroup : public ZCLIName, public ZCLIDescription, public ZCLIOptionProvider
{
private:
  vector<ZCLIVerb> verbs;

public:
  ZCLIGroup(string n) : ZCLIName(n) {};
  ZCLIVerb &get_verb(string);
  vector<ZCLIVerb> &get_verbs() { return verbs; }
  void help(string);
  void help_line() { cerr << "  " << left << setw(ZCLI_MENU_WIDTH) << get_name() << " | " << get_description() << endl; }
};

class ZCLI : public ZCLIName, public ZCLIOptionProvider
{
private:
  bool validate();
  void init();
  vector<ZCLIGroup> groups;
  bool interactive_mode;
  void parse_input(string, vector<string> &);
  bool should_quit(string input);
  int run(int, char *[]);

public:
  ZCLI(string n) : ZCLIName(n) { interactive_mode = false; }
  int parse(int, char *[]);
  vector<ZCLIGroup> &get_groups() { return groups; };
  ZCLIGroup &get_group(string);
  ZCLIVerb &get_verb(int, char *[]);
  void help();
  void set_interactive_mode(bool v) { interactive_mode = v; }
};

bool ZCLI::validate()
{
  // ensure at least one group
  if (0 == groups.size())
  {
    cerr << "ZCLI Error: must define at least one group" << endl;
    return false;
  }

  map<string, int> group_map;
  for (vector<ZCLIGroup>::iterator it = groups.begin(); it != groups.end(); it++)
  {
    // ensure no duplicate groups
    if (group_map.find(it->get_name()) != (group_map.end()))
    {
      cerr << "ZCLI Error: duplicate group found, '" << it->get_name() << "'" << endl;
      return false;
    }
    group_map.insert(map<string, int>::value_type(it->get_name(), 0));

    // ensure at least one verb
    if (0 == it->get_verbs().size())
    {
      cerr << "ZCLI Error: each group must contain at least one verb, " << it->get_name() << " does not" << endl;
      return false;
    }

    map<string, int> verb_map;
    for (vector<ZCLIVerb>::iterator iit = it->get_verbs().begin(); iit != it->get_verbs().end(); iit++)
    {
      // ensure no duplicate verbs
      if (verb_map.find(iit->get_name()) != (verb_map.end()))
      {
        cerr << "ZCLI Error: duplicate verb found, '" << iit->get_name() << "' on '" << it->get_name() << "'" << endl;
        return false;
      }
      verb_map.insert(map<string, int>::value_type(iit->get_name(), 0));

      // ensure handler provided
      if (NULL == iit->get_zcli_verb_handler())
      {
        cerr << "ZCLI Error: each verb must container a handler, " << iit->get_name() << " does not" << endl;
        return false;
      }

      map<string, int> option_map;

      for (vector<ZCLIOption>::iterator iiit = iit->get_options().begin(); iiit != iit->get_options().end(); iiit++)
      {

        map<string, int> alias_map;
        for (vector<string>::iterator iiiit = iiit->get_aliases().begin(); iiiit != iiit->get_aliases().end(); iiiit++)
        {
          if (alias_map.find(*iiiit) != (alias_map.end()))
          {
            cerr << "ZCLI Error: duplicate alias found, '" << *iiiit << "' on '" << it->get_name() << " " << iit->get_name() << "'" << endl;
            return false;
          }
          if (string::npos != iiiit->find(" "))
          {
            cerr << "ZCLI Error: alias cannot contain a space, '" << iiit->get_name() << "' does on '" << it->get_name() << " " << iit->get_name() << "'" << endl;
            return false;
          }
          alias_map.insert(map<string, int>::value_type(*iiiit, 0));
        }

        // ensure no duplicate options
        if (option_map.find(iiit->get_name()) != (option_map.end()))
        {
          cerr << "ZCLI Error: duplicate option found, '" << iiit->get_name() << "' on '" << it->get_name() << " " << iit->get_name() << "'" << endl;
          return false;
        }
        if (string::npos != iiit->get_name().find(" "))
        {
          cerr << "ZCLI Error: option cannot contain a space, '" << iiit->get_name() << "' does on '" << it->get_name() << " " << iit->get_name() << "'" << endl;
          return false;
        }
        option_map.insert(map<string, int>::value_type(iiit->get_name(), 0));
      }

      map<string, int> positional_map;
      for (vector<ZCLIPositional>::iterator iiit = iit->get_positionals().begin(); iiit != iit->get_positionals().end(); iiit++)
      {
        // ensure no duplicate positionals
        if (positional_map.find(iiit->get_name()) != (positional_map.end()))
        {
          cerr << "ZCLI Error: duplicate positional found, '" << iiit->get_name() << "' on '" << it->get_name() << " " << iit->get_name() << "'" << endl;
          return false;
        }
        positional_map.insert(map<string, int>::value_type(iiit->get_name(), 0));

        if (string::npos != iiit->get_name().find(" "))
        {
          cerr << "ZCLI Error: positional cannot contain a space, '" << iiit->get_name() << "' does on '" << it->get_name() << " " << iit->get_name() << "'" << endl;
          return false;
        }
      }
    }
  }
  return true;
}

void ZCLI::init()
{
  // add interactive mode if requested
  ZCLIOption interactive_mode_option("interactive");
  interactive_mode_option.set_description("interactive (REPL) mode");
  interactive_mode_option.get_aliases().push_back("--it");
  if (interactive_mode)
  {
    get_options().push_back(interactive_mode_option);
  }

  // add help to main CLI
  ZCLIOption main_help("help");
  main_help.set_description("CLI help");
  main_help.get_aliases().push_back("-h");
  get_options().push_back(main_help);

  for (vector<ZCLIGroup>::iterator it = groups.begin(); it != groups.end(); it++)
  {
    // add help to each group
    ZCLIOption group_help("help");
    group_help.set_description("group help");
    group_help.get_aliases().push_back("-h");
    it->get_options().push_back(group_help);

    for (vector<ZCLIVerb>::iterator iit = it->get_verbs().begin(); iit != it->get_verbs().end(); iit++)
    {
      // add help to each verb
      ZCLIOption verb_help("help");
      verb_help.set_description("verb help");
      verb_help.get_aliases().push_back("-h");
      iit->get_options().push_back(verb_help);
    }
  }
}

void ZCLIVerb::help(string cli_name, string group_name)
{
  cerr << "Usage is '" << cli_name << " " << group_name << " " << get_name() << "':" << endl;

  if (get_positionals().size() > 0)
  {
    cerr << "Positionals:" << endl;
    for (vector<ZCLIPositional>::iterator it = positionals.begin(); it != positionals.end(); it++)
    {
      it->help_line();
    }
  }

  if (get_options().size() > 0)
  {
    cerr << "Options:" << endl;
    for (vector<ZCLIOption>::iterator it = options.begin(); it != options.end(); it++)
    {
      it->help_line();
    }
  }
}

void ZCLIGroup::help(string cli_name)
{
  cerr << "Usage is '" << cli_name << " " << name << " <verb>' where <verb> is one of:" << endl;
  for (vector<ZCLIVerb>::iterator it = verbs.begin(); it != verbs.end(); it++)
  {
    it->help_line();
  }

  if (get_options().size() > 0)
  {
    cerr << "Options:" << endl;
    for (vector<ZCLIOption>::iterator it = options.begin(); it != options.end(); it++)
    {
      it->help_line();
    }
  }
}

void ZCLI::help()
{
  cerr << "Usage is '" << name << " <group>' where <group> is one of:" << endl;
  for (vector<ZCLIGroup>::iterator it = groups.begin(); it != groups.end(); it++)
  {
    it->help_line();
  }

  if (get_options().size() > 0)
  {
    cerr << "Options:" << endl;
    for (vector<ZCLIOption>::iterator it = options.begin(); it != options.end(); it++)
    {
      it->help_line();
    }
  }
}

ZCLIGroup &ZCLI::get_group(string group_name)
{
  for (vector<ZCLIGroup>::iterator it = groups.begin(); it != groups.end(); it++)
  {
    if (group_name == it->get_name())
      return *it;
  }
  ZCLIGroup *not_found = new ZCLIGroup("not found");
  return *not_found;
}

ZCLIVerb &ZCLIGroup::get_verb(string verb_name)
{
  for (vector<ZCLIVerb>::iterator it = verbs.begin(); it != verbs.end(); it++)
  {
    if (verb_name == it->get_name())
      return *it;
  }
  ZCLIVerb *not_found = new ZCLIVerb("not found");
  return *not_found;
}

ZCLIOption &ZCLIOptionProvider::get_option(string option_name)
{
  for (vector<ZCLIOption>::iterator it = options.begin(); it != options.end(); it++)
  {
    vector<string> &aliases = it->get_aliases();
    if (option_name == it->get_flag_name() || std::find(aliases.begin(), aliases.end(), option_name) != it->get_aliases().end())
      return *it;
  }
  ZCLIOption *not_found = new ZCLIOption("not found");
  return *not_found;
}

ZCLIPositional &ZCLIPositionalProvider::get_positional(string positional_name)
{
  for (vector<ZCLIPositional>::iterator it = positionals.begin(); it != positionals.end(); it++)
  {
    if (positional_name == it->get_name())
      return *it;
  }
  ZCLIPositional *not_found = new ZCLIPositional("not found");
  return *not_found;
}

bool ZCLI::should_quit(string arg)
{
  if ("quit" == arg)
    return true;
  if ("exit" == arg)
    return true;
  if ("QUIT" == arg)
    return true;
  if ("EXIT" == arg)
    return true;
  return false;
}

int ZCLI::run(int argc, char *argv[])
{
  if (argc <= CLI_GROUP_ARG || string(argv[CLI_GROUP_ARG]) == "--help" || string(argv[CLI_GROUP_ARG]) == "-h")
  {
    help();
    return 0;
  }

  // attempt to get a group
  ZCLIGroup &group = get_group(argv[CLI_GROUP_ARG]);

  // show main help if unknown group
  if (0 == group.get_verbs().size())
  {
    // delete command_group;
    cerr << "Unknown command group: " << argv[CLI_GROUP_ARG] << endl;
    help();
    return 0;
  }

  // show group level help if group only
  if (argc <= CLI_VERB_ARG || string(argv[CLI_VERB_ARG]) == "--help" || string(argv[CLI_VERB_ARG]) == "-h")
  {
    group.help(name);
    return 0;
  }

  // attempt to get a verb
  ZCLIVerb &verb = group.get_verb(argv[CLI_VERB_ARG]);

  // show group level help if unknwon verb
  if (NULL == verb.get_zcli_verb_handler())
  {
    // delete command_group;
    cerr << "Unknown command verb: " << argv[CLI_VERB_ARG] << endl;
    group.help(name);
    return 0;
  }

  // look for help
  for (int i = CLI_REMAIN_ARG_START; i < argc; i++)
  {
    if (string(argv[i]) == "--help" || string(argv[i]) == "-h")
    {
      verb.help(name, group.get_name());
      return 0;
    }
  }

  ZCLIResult results;

  int positional_index = 0;

  for (int i = CLI_REMAIN_ARG_START; i < argc; i++)
  {
    ZCLIOption &option = verb.get_option(argv[i]);

    // if not an option, check for positional
    if (string::npos != option.get_name().find(" "))
    {
      if (positional_index < verb.get_positionals().size())
      {
        // found positional
        verb.get_positionals()[positional_index].set_found(true);
        verb.get_positionals()[positional_index].set_value(argv[i]);
        results.get_positionals().push_back(verb.get_positionals()[positional_index]);
        positional_index++;
        continue;
      }
      else
      {
        cerr << "Unexpected positional present '" << argv[i] << "' on '" << group.get_name() << " " << verb.get_name() << "'" << endl;
        verb.help(name, group.get_name());
        return 1;
      }
    }

    if (i + 1 > argc - 1) // index vs count
    {
      cerr << "Missing required value for: " << argv[i] << endl;
      verb.help(name, group.get_name());
      return -1;
    }

    option.set_found(true);
    option.set_value(argv[i + 1]);
    results.get_options().push_back(option);

    i++; // advance to next parm
  }

  for (vector<ZCLIOption>::iterator it = verb.get_options().begin(); it != verb.get_options().end(); it++)
  {
    if (it->get_required() && !it->is_found())
    {
      cerr << "Required option missing: '" << it->get_flag_name() << "' on '" << group.get_name() << " " << verb.get_name() << "'" << endl;
      verb.help(name, group.get_name());
      return -1;
    }

    if (it->default_provided() && !it->is_found())
    {
      it->set_value(it->get_default());
      results.get_options().push_back(*it);
    }
  }

  for (vector<ZCLIPositional>::iterator it = verb.get_positionals().begin(); it != verb.get_positionals().end(); it++)
  {
    if (it->get_required() && !it->is_found())
    {
      cerr << "Required positional missing: '" << it->get_name() << "' on '" << group.get_name() << " " << verb.get_name() << "'" << endl;
      verb.help(name, group.get_name());
      return -1;
    }
  }

  // set values
  return verb.get_zcli_verb_handler()(results);
}

void ZCLI::parse_input(string input, vector<string> &values)
{
  istringstream iss(input);
  vector<string> args;

  string arg;
  string temp;
  bool open = false;

  while (getline(iss, arg, ' '))
  {
    args.push_back(arg);
  }

  for (vector<string>::iterator it = args.begin(); it != args.end(); it++)
  {
    size_t pos = it->find("\"");

    if (string::npos == pos)
    {
      if (open)
      {

        temp += (*it + " ");
      }
      else
      {
        values.push_back(*it);
      }
    }
    else
    {
      size_t end = it->find("\"", pos + 1);
      if (string::npos != end)
      {
        values.push_back(*it);
      }
      else
      {
        if (!open)
        {
          open = true;
          temp += (*it + " ");
        }
        else
        {
          open = false;
          temp += *it;
          values.push_back(temp);
          temp = "";
        }
      }
    }

    // if last entry and open quote, just append remaining string
    int index = distance(args.begin(), it);
    if (index + 1 == args.size())
    {
      if (open)
      {
        temp += *it;
        values.push_back(temp);
      }
    }
  }
}

int ZCLI::parse(int argc, char *argv[])
{
  init();
  bool valid = validate();

  if (!valid)
    return -1;

  if (interactive_mode && argc == CLI_INTERACTIVE_ARG + 1 && (string(argv[CLI_INTERACTIVE_ARG]) == "--interactive" || string(argv[CLI_INTERACTIVE_ARG]) == "--it"))
  {
    cout << "Started, enter command or 'quit' to quit..." << endl;

    string command;
    int rc = 0;
    do
    {
      getline(cin, command);

      if (should_quit(command))
        break;

      vector<string> entries;
      parse_input(command, entries);

#define ZCLI_MAX_PARMS 64

      char *args[ZCLI_MAX_PARMS] = {0};
      args[0] = argv[0];

      for (int i = 1; i < entries.size(); i++)
      {
        args[i] = (char *)entries[i].c_str();
      }

      rc = run(entries.size(), args);

    } while (!should_quit(command));

    cout << "...terminated" << endl;

    return rc;
  }
  else
  {
    return run(argc, argv);
  }
}

#endif
