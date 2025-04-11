/*
MIT License

Copyright (c) 2025 Trae Yelovich

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

namespace pparser
{

  class Command;
  class ArgumentParser;
  class ParseResult;
  struct ArgValueImpl;

  struct ArgValueImpl
  {
    enum ArgValueKind
    {
      AVK_NONE,
      AVK_BOOL,
      AVK_INT,
      AVK_DOUBLE,
      AVK_STRING,
      AVK_STRING_VECTOR
    };

    ArgValueKind kind;
    union
    {
      bool b;
      long long i; // Assuming long long is available as extension, else use long
      double d;
      std::string *s;               // Pointer for backwards compatibility for unions in
                                    // pre-C++11 environments
      std::vector<std::string> *sv; // Pointer for the same reason
    } value;

    // Default constructor
    ArgValueImpl() : kind(AVK_NONE)
    {
      // Initialize one member to avoid undefined state, though not strictly
      // necessary for NONE
      value.i = 0;
    }

    explicit ArgValueImpl(bool val) : kind(AVK_BOOL) { value.b = val; }
    explicit ArgValueImpl(long long val) : kind(AVK_INT) { value.i = val; }
    explicit ArgValueImpl(double val) : kind(AVK_DOUBLE) { value.d = val; }
    // note: takes ownership of new string
    explicit ArgValueImpl(const std::string &val) : kind(AVK_STRING)
    {
      value.s = new std::string(val);
    }
    // note: takes ownership of new vector
    explicit ArgValueImpl(const std::vector<std::string> &val)
        : kind(AVK_STRING_VECTOR)
    {
      value.sv = new std::vector<std::string>(val);
    }

    // Cleanup pointer members
    ~ArgValueImpl() { clear(); }

    ArgValueImpl(const ArgValueImpl &other) : kind(AVK_NONE) { copyFrom(other); }
    ArgValueImpl &operator=(const ArgValueImpl &other)
    {
      if (this != &other)
      {
        clear();
        copyFrom(other);
      }
      return *this;
    }

    // Helper to clear existing data (delete pointers)
    void clear()
    {
      if (kind == AVK_STRING)
      {
        delete value.s;
        value.s = nullptr;
      }
      else if (kind == AVK_STRING_VECTOR)
      {
        delete value.sv;
        value.sv = nullptr;
      }
      kind = AVK_NONE;
      value.i = 0;
    }

    // Helper for copy construction/assignment
    void copyFrom(const ArgValueImpl &other)
    {
      kind = other.kind;
      switch (kind)
      {
      case AVK_NONE:
        value.i = 0;
        break;
      case AVK_BOOL:
        value.b = other.value.b;
        break;
      case AVK_INT:
        value.i = other.value.i;
        break;
      case AVK_DOUBLE:
        value.d = other.value.d;
        break;
      case AVK_STRING:
        value.s = other.value.s ? new std::string(*other.value.s) : nullptr;
        break;
      case AVK_STRING_VECTOR:
        value.sv = other.value.sv ? new std::vector<std::string>(*other.value.sv)
                                  : nullptr;
        break;
      }
    }

    // Type Checkers
    bool isNone() const { return kind == AVK_NONE; }
    bool isBool() const { return kind == AVK_BOOL; }
    bool isInt() const { return kind == AVK_INT; }
    bool isDouble() const { return kind == AVK_DOUBLE; }
    bool isString() const { return kind == AVK_STRING; }
    bool isStringVector() const { return kind == AVK_STRING_VECTOR; }

    // Getters (unsafe, check type first or use safe getters below)
    bool getBoolUnsafe() const { return value.b; }
    long long getIntUnsafe() const { return value.i; }
    double getDoubleUnsafe() const { return value.d; }
    const std::string &getStringUnsafe() const { return *value.s; }
    const std::vector<std::string> &getStringVectorUnsafe() const
    {
      return *value.sv;
    }
    std::string *getStringPtrUnsafe() const { return value.s; }
    std::vector<std::string> *getStringVectorPtrUnsafe() const
    {
      return value.sv;
    }

    // Safe getters (returns a pointer; nullptr if wrong type/unset)
    const bool *getBool() const { return kind == AVK_BOOL ? &value.b : nullptr; }
    const long long *getInt() const
    {
      return kind == AVK_INT ? &value.i : nullptr;
    }
    const double *getDouble() const
    {
      return kind == AVK_DOUBLE ? &value.d : nullptr;
    }
    const std::string *getString() const
    {
      return kind == AVK_STRING ? value.s : nullptr;
    }
    const std::vector<std::string> *getStringVector() const
    {
      return kind == AVK_STRING_VECTOR ? value.sv : nullptr;
    }
    std::string getStringValue(const std::string &defaultVal = "") const
    {
      const std::string *ptr = getString();
      return ptr ? *ptr : defaultVal;
    }

    void print(std::ostream &os) const
    {
      switch (kind)
      {
      case AVK_NONE:
        os << "<none>";
        break;
      case AVK_BOOL:
        os << (value.b ? "true" : "false");
        break;
      case AVK_INT:
        os << value.i;
        break;
      case AVK_DOUBLE:
        os << value.d;
        break;
      case AVK_STRING:
        os << (value.s ? *value.s : "<invalid_string>");
        break;
      case AVK_STRING_VECTOR:
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

  typedef ArgValueImpl ArgValue;

  enum ArgType
  {
    ARGTYPE_FLAG,      // Boolean switch (e.g., --verbose)
    ARGTYPE_SINGLE,    // Expects a single value (e.g., --output file.txt)
    ARGTYPE_MULTIPLE,  // Expects one or more values (e.g., --input a.txt b.txt)
    ARGTYPE_POSITIONAL // Argument determined by position
  };

  struct ArgumentDef
  {
    std::string name;      // Internal name to access the parsed value
    std::string shortName; // Short flag (e.g., "-f") - Optional
    std::string longName;  // Long flag (e.g., "--file") - Optional
    std::string help;      // Help text description
    ArgType type;          // Type of argument (flag, single, etc.)
    bool required;         // Is this argument mandatory?
    ArgValue defaultValue; // Default value if argument is not provided
    bool isHelpFlag;       // Internal flag to identify the help argument

    ArgumentDef(std::string n, std::string sn, std::string ln, std::string h,
                ArgType t = ARGTYPE_FLAG, bool req = false,
                ArgValue defVal = ArgValue(), bool helpFlag = false)
        : name(n), shortName(sn), longName(ln), help(h), type(t), required(req),
          defaultValue(defVal), isHelpFlag(helpFlag) {}

    ArgumentDef() : type(ARGTYPE_FLAG), required(false), isHelpFlag(false) {}

    // Get display name (e.g., "-f, --file")
    std::string getDisplayName() const
    {
      std::string display;
      if (!shortName.empty())
      {
        display += shortName;
      }
      if (!longName.empty())
      {
        if (!display.empty())
          display += ", ";
        display += longName;
      }
      if (type != ARGTYPE_FLAG && type != ARGTYPE_POSITIONAL)
      {
        display += " <value>";
        if (type == ARGTYPE_MULTIPLE)
        {
          display += "...";
        }
      }
      return display;
    }
  };

  // Stores the outcome of the parsing process
  class ParseResult
  {
  public:
    enum PParserStatus
    {
      PPARSER_STATUS_SUCCESS,
      PPARSER_STATUS_HELP_REQUESTED,
      PPARSER_STATUS_PARSE_ERROR
    };

    PParserStatus status;     // No default initializer
    int exitCode;             // Exit code returned by handler or set by parser
    std::string errorMessage; // Error message if status is PARSE_ERROR
    std::string
        commandPath; // Full path of the executed command (e.g., "git remote add")

    // Map storing parsed keyword argument values (name -> value)
    std::map<std::string, ArgValue> keywordValues;
    // Vector storing parsed positional argument values in order
    std::vector<ArgValue> positionalValues;

    ParseResult() : status(PPARSER_STATUS_SUCCESS), exitCode(0) {}

    // Check if a keyword arg was provided (doesn't check type)
    bool hasKeywordArg(const std::string &name) const
    {
      return keywordValues.find(name) != keywordValues.end();
    }

    // Getters returning pointers, nullptr if missing or wrong type
    const bool *getKeywordArgBool(const std::string &name) const
    {
      std::map<std::string, ArgValue>::const_iterator it =
          keywordValues.find(name);
      if (it != keywordValues.end())
      {
        return it->second.getBool();
      }
      return nullptr;
    }

    const long long *getKeywordArgInt(const std::string &name) const
    {
      std::map<std::string, ArgValue>::const_iterator it =
          keywordValues.find(name);
      if (it != keywordValues.end())
      {
        return it->second.getInt();
      }
      return nullptr;
    }

    const double *getKeywordArgDouble(const std::string &name) const
    {
      std::map<std::string, ArgValue>::const_iterator it =
          keywordValues.find(name);
      if (it != keywordValues.end())
      {
        return it->second.getDouble();
      }
      return nullptr;
    }

    const std::string *getKeywordArgString(const std::string &name) const
    {
      std::map<std::string, ArgValue>::const_iterator it =
          keywordValues.find(name);
      if (it != keywordValues.end())
      {
        return it->second.getString();
      }
      return nullptr;
    }

    const std::vector<std::string> *
    getKeywordArgStringVector(const std::string &name) const
    {
      std::map<std::string, ArgValue>::const_iterator it =
          keywordValues.find(name);
      if (it != keywordValues.end())
      {
        return it->second.getStringVector();
      }
      return nullptr;
    }

    // Getters returning value or a default if missing/wrong type
    bool getKeywordArgBoolOrDefault(const std::string &name,
                                    bool defaultValue = false) const
    {
      const bool *ptr = getKeywordArgBool(name);
      return ptr ? *ptr : defaultValue;
    }

    long long getKeywordArgIntOrDefault(const std::string &name,
                                        long long defaultValue = 0) const
    {
      const long long *ptr = getKeywordArgInt(name);
      return ptr ? *ptr : defaultValue;
    }

    double getKeywordArgDoubleOrDefault(const std::string &name,
                                        double defaultValue = 0.0) const
    {
      const double *ptr = getKeywordArgDouble(name);
      return ptr ? *ptr : defaultValue;
    }

    std::string
    getKeywordArgStringOrDefault(const std::string &name,
                                 const std::string &defaultValue = "") const
    {
      std::map<std::string, ArgValue>::const_iterator it =
          keywordValues.find(name);
      if (it != keywordValues.end())
      {
        return it->second.getStringValue(defaultValue);
      }
      return defaultValue;
    }

    // Returns a copy of the vector or an empty vector as default
    std::vector<std::string>
    getKeywordArgStringVectorOrDefault(const std::string &name) const
    {
      const std::vector<std::string> *ptr = getKeywordArgStringVector(name);
      return ptr ? *ptr : std::vector<std::string>();
    }

    // Getters for positional args by index
    const bool *getPositionalArgBool(size_t index) const
    {
      if (index < positionalValues.size())
      {
        return positionalValues[index].getBool();
      }
      return nullptr;
    }
    const long long *getPositionalArgInt(size_t index) const
    {
      if (index < positionalValues.size())
      {
        return positionalValues[index].getInt();
      }
      return nullptr;
    }
    const double *getPositionalArgDouble(size_t index) const
    {
      if (index < positionalValues.size())
      {
        return positionalValues[index].getDouble();
      }
      return nullptr;
    }
    const std::string *getPositionalArgString(size_t index) const
    {
      if (index < positionalValues.size())
      {
        return positionalValues[index].getString();
      }
      return nullptr;
    }
    const std::vector<std::string> *
    getPositionalArgStringVector(size_t index) const
    {
      if (index < positionalValues.size())
      {
        return positionalValues[index].getStringVector();
      }
      return nullptr;
    }
  };

  // Represents a command or subcommand with its arguments
  class Command : public std::enable_shared_from_this<Command>
  {
  public:
    typedef int (*CommandHandler)(const ParseResult &result);

    Command(std::string name, std::string help)
        : name_(name), help_(help), handler_(nullptr)
    {
      ensureHelpArgument();
    }

    // Destructor to clean up subcommand pointers
    ~Command() { subcommands_.clear(); }

    // Add a keyword/option argument (e.g., --file, -f)
    Command &addKeywordArg(std::string name, std::string shortName,
                           std::string longName, std::string help,
                           ArgType type = ARGTYPE_FLAG, bool required = false,
                           ArgValue defaultValue = ArgValue())
    {
      // Prevent adding another argument named "help" if it conflicts
      if (name == "help")
      {
        throw std::invalid_argument(
            "Argument name 'help' is reserved for the automatic help flag.");
      }

      // If it's a flag and the default value is still the initial AVK_NONE,
      // set the actual default to false.
      ArgValue finalDefaultValue = defaultValue;
      if (type == ARGTYPE_FLAG && defaultValue.isNone())
      {
        finalDefaultValue = ArgValue(false);
      }

      // Ensure the new keyword argument is unique
      auto it = std::find_if(
          keywordArgs_.begin(), keywordArgs_.end(),
          [&name](const ArgumentDef &arg)
          { return arg.name == name; });
      if (it != keywordArgs_.end())
      {
        throw std::invalid_argument("Argument '" + name + "' already exists.");
      }

      keywordArgs_.push_back(ArgumentDef(name, shortName, longName, help, type,
                                         required, finalDefaultValue));
      return *this;
    }

    // Add a positional argument (defined by order)
    Command &addPositionalArg(std::string name, std::string help,
                              ArgType type = ARGTYPE_SINGLE, bool required = true,
                              ArgValue defaultValue = ArgValue())
    {
      if (type == ARGTYPE_FLAG)
      {
        throw std::invalid_argument("Positional arguments cannot be flags.");
      }

      // Ensure the new positional argument is unique
      auto it = std::find_if(
          positionalArgs_.begin(), positionalArgs_.end(),
          [&name](const ArgumentDef &arg)
          { return arg.name == name; });
      if (it != positionalArgs_.end())
      {
        throw std::invalid_argument("Argument '" + name + "' already exists.");
      }

      positionalArgs_.push_back(ArgumentDef(
          name, "", "", help, ARGTYPE_POSITIONAL, required, defaultValue));
      positionalArgs_.back().type = type;
      return *this;
    }

    // Add a subcommand (takes ownership of the pointer)
    Command &addSubcommand(std::shared_ptr<Command> sub)
    {
      if (!sub)
        return *this;

      sub->ensureHelpArgument();

      std::string subName = sub->getName();
      // Check for conflicts with existing names and aliases
      if (subcommands_.count(subName))
      {
        throw std::invalid_argument("Subcommand name '" + subName +
                                    "' already exists.");
      }
      for (const auto &pair : subcommands_)
      {
        if (pair.second->hasAlias(subName))
        {
          throw std::invalid_argument("Subcommand name '" + subName +
                                      "' conflicts with an existing alias.");
        }
        for (const auto &alias : sub->getAliases())
        {
          if (pair.first == alias || pair.second->hasAlias(alias))
          {
            throw std::invalid_argument(
                "Subcommand alias '" + alias +
                "' conflicts with an existing name or alias.");
          }
        }
      }
      // Also check the new subcommand's aliases against its own name
      for (const auto &alias : sub->getAliases())
      {
        if (alias == subName)
        {
          throw std::invalid_argument("Subcommand alias '" + alias +
                                      "' cannot be the same as its name '" +
                                      subName + "'.");
        }
      }

      subcommands_[subName] = sub;
      return *this;
    }

    // Add an alias to this command
    Command &addAlias(const std::string &alias)
    {
      if (alias == name_)
      {
        throw std::invalid_argument(
            "Alias cannot be the same as the command name '" + name_ + "'.");
      }
      // Could add checks here to ensure alias doesn't conflict with other sibling
      // commands/aliases if added to a parent
      aliases_.push_back(alias);
      return *this;
    }

    // Set the function pointer handler
    Command &setHandler(CommandHandler handler)
    {
      handler_ = handler;
      return *this;
    }

    const std::string &getName() const { return name_; }
    const std::string &getHelp() const { return help_; }
    const std::map<std::string, std::shared_ptr<Command>> &
    getSubcommands() const
    {
      return subcommands_;
    }
    const std::vector<ArgumentDef> &getKeywordArgs() const
    {
      return keywordArgs_;
    }
    const std::vector<ArgumentDef> &getPositionalArgs() const
    {
      return positionalArgs_;
    }
    const std::vector<std::string> &getAliases() const { return aliases_; }
    CommandHandler getHandler() const { return handler_; }

    ParseResult parse(const std::vector<lexer::Token> &tokens,
                      size_t &currentTokenIndex,
                      const std::string &commandPathPrefix) const
    {
      ParseResult result;
      result.commandPath = commandPathPrefix + name_;
      // Initialize exit code for potential errors or help requests
      result.exitCode = 0; // Default to 0 for success before handler runs

      std::map<std::string, bool> keywordArgsSeen;
      size_t currentPositionalArgIndex = 0;

      for (std::vector<ArgumentDef>::const_iterator it = keywordArgs_.begin();
           it != keywordArgs_.end(); ++it)
      {
        const ArgumentDef &arg = *it;
        if (arg.isHelpFlag)
        {
          continue;
        }
        result.keywordValues[arg.name] = arg.defaultValue;
      }

      while (currentTokenIndex < tokens.size())
      {
        const lexer::Token &token = tokens[currentTokenIndex];
        lexer::TokenKind kind = token.getKind();

        // Check for Keyword Arguments/Flags (TOK_FLAG_SHORT, TOK_FLAG_LONG)
        if (kind == lexer::TOK_FLAG_SHORT || kind == lexer::TOK_FLAG_LONG)
        {
          std::string flagNameStr = token.getIdValue(); // Get name without dashes
          bool is_short_flag_kind = (kind == lexer::TOK_FLAG_SHORT);

          if (is_short_flag_kind && flagNameStr.length() > 1)
          {
            currentTokenIndex++;

            for (size_t i = 0; i < flagNameStr.length(); ++i)
            {
              std::string singleFlagChar(1, flagNameStr[i]);
              const ArgumentDef *matchedArg =
                  findKeywordArg(singleFlagChar, true);

              if (!matchedArg)
              {
                result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
                result.errorMessage = "Unknown option in combined flags: -";
                result.errorMessage += singleFlagChar;
                std::cerr << "Error: " << result.errorMessage << "\n\n";
                generateHelp(std::cerr, commandPathPrefix);
                result.exitCode = 1;
                return result;
              }

              if (matchedArg->isHelpFlag)
              {
                generateHelp(std::cout, commandPathPrefix);
                result.status = ParseResult::PPARSER_STATUS_HELP_REQUESTED;
                result.exitCode = 0; // Help request is a successful exit
                return result;
              }

              // Ensure combined flags are actually boolean flags
              if (matchedArg->type != ARGTYPE_FLAG)
              {
                result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
                result.errorMessage = "Option -" + singleFlagChar +
                                      " requires a value and cannot be combined.";
                std::cerr << "Error: " << result.errorMessage << "\n\n";
                generateHelp(std::cerr, commandPathPrefix);
                result.exitCode = 1;
                return result;
              }

              // Mark as seen and set value to true
              keywordArgsSeen[matchedArg->name] = true;
              result.keywordValues[matchedArg->name] = ArgValue(true);
            }
            continue;
          }

          const ArgumentDef *matchedArg =
              findKeywordArg(flagNameStr, is_short_flag_kind);

          if (!matchedArg)
          {
            result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
            result.errorMessage = "Unknown option: ";
            result.errorMessage += (is_short_flag_kind ? "-" : "--");
            result.errorMessage += flagNameStr;
            std::cerr << "Error: " << result.errorMessage << "\n\n";
            generateHelp(std::cerr, commandPathPrefix);
            result.exitCode = 1;
            return result;
          }

          // If this is the specifically marked help flag, handle it immediately.
          if (matchedArg->isHelpFlag)
          {
            generateHelp(std::cout, commandPathPrefix);
            result.status = ParseResult::PPARSER_STATUS_HELP_REQUESTED;
            result.exitCode = 0; // Help request is a successful exit
            return result;
          }

          currentTokenIndex++; // Consume the flag token itself
          keywordArgsSeen[matchedArg->name] = true;

          // Handle argument value based on type
          if (matchedArg->type == ARGTYPE_FLAG)
          {
            result.keywordValues[matchedArg->name] = ArgValue(true);
          }
          else
          {
            // Check if next token exists and is not another flag
            if (currentTokenIndex >= tokens.size() ||
                tokens[currentTokenIndex].getKind() == lexer::TOK_FLAG_SHORT ||
                tokens[currentTokenIndex].getKind() == lexer::TOK_FLAG_LONG)
            {
              result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
              result.errorMessage =
                  "Option " + matchedArg->getDisplayName() + " requires a value.";
              std::cerr << "Error: " << result.errorMessage << "\n\n";
              generateHelp(std::cerr, commandPathPrefix);
              result.exitCode = 1;
              return result;
            }

            const lexer::Token &valueToken = tokens[currentTokenIndex];
            ArgValue parsedValue = parseTokenValue(valueToken, matchedArg->type);
            if (parsedValue.isNone())
            {
              result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
              result.errorMessage =
                  "Invalid value for option " + matchedArg->getDisplayName();
              std::cerr << "Error: " << result.errorMessage << "\n\n";
              generateHelp(std::cerr, commandPathPrefix);
              result.exitCode = 1;
              return result;
            }

            if (matchedArg->type == ARGTYPE_SINGLE)
            {
              result.keywordValues[matchedArg->name] = parsedValue;
              currentTokenIndex++;
            }
            else if (matchedArg->type == ARGTYPE_MULTIPLE)
            {
              // Ensure value type is string for MULTIPLE
              if (!parsedValue.isString())
              {
                result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
                result.errorMessage =
                    "Internal error: Expected string value for multiple option " +
                    matchedArg->name;
                std::cerr << "Error: " << result.errorMessage << "\n\n";
                generateHelp(std::cerr, commandPathPrefix);
                result.exitCode = 1;
                return result;
              }

              std::map<std::string, ArgValue>::iterator mapIt =
                  result.keywordValues.find(matchedArg->name);
              if (mapIt == result.keywordValues.end() ||
                  !mapIt->second.isStringVector() || mapIt->second.isNone())
              {
                result.keywordValues[matchedArg->name] =
                    ArgValue(std::vector<std::string>());
                mapIt = result.keywordValues.find(matchedArg->name);
              }

              std::vector<std::string> *vec =
                  mapIt->second.getStringVectorPtrUnsafe();
              if (vec)
              {
                const std::string *firstValStrPtr = parsedValue.getString();
                if (firstValStrPtr)
                {
                  vec->push_back(*firstValStrPtr);
                }

                currentTokenIndex++;

                // Keep consuming values until next flag or end
                while (currentTokenIndex < tokens.size() &&
                       tokens[currentTokenIndex].getKind() !=
                           lexer::TOK_FLAG_SHORT &&
                       tokens[currentTokenIndex].getKind() !=
                           lexer::TOK_FLAG_LONG)
                {
                  const lexer::Token &nextValueToken = tokens[currentTokenIndex];
                  ArgValue nextParsedValue =
                      parseTokenValue(nextValueToken, ARGTYPE_SINGLE);
                  const std::string *nextValStrPtr = nextParsedValue.getString();
                  if (nextValStrPtr)
                  {
                    vec->push_back(*nextValStrPtr);
                    currentTokenIndex++;
                  }
                  else
                  {
                    break;
                  }
                }
              }
              else
              {
                result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
                result.errorMessage =
                    "Internal error accessing vector for multiple values for " +
                    matchedArg->name;
                std::cerr << "Error: " << result.errorMessage << "\n\n";
                generateHelp(std::cerr, commandPathPrefix);
                result.exitCode = 1;
                return result;
              }
            }
          }
          continue;
        }

        // Check for subcommand
        if (kind == lexer::TOK_ID)
        {
          std::string potentialSubcommandOrAlias = token.getIdValue();
          std::shared_ptr<Command> matchedSubcommand = nullptr;

          // First, check if it's a direct name match
          auto subIt = subcommands_.find(potentialSubcommandOrAlias);
          if (subIt != subcommands_.end())
          {
            matchedSubcommand = subIt->second;
          }
          else
          {
            // If not a direct name match, check aliases
            for (const auto &pair : subcommands_)
            {
              if (pair.second->hasAlias(potentialSubcommandOrAlias))
              {
                // Prevent aliasing to multiple commands
                if (matchedSubcommand)
                {
                  result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
                  result.errorMessage = "Ambiguous alias '" +
                                        potentialSubcommandOrAlias +
                                        "' matches multiple subcommands.";
                  std::cerr << "Error: " << result.errorMessage << "\n\n";
                  generateHelp(std::cerr, commandPathPrefix);
                  result.exitCode = 1;
                  return result;
                }
                matchedSubcommand = pair.second;
              }
            }
          }

          // If a subcommand (by name or alias) was found
          if (matchedSubcommand)
          {
            currentTokenIndex++; // Consume the subcommand/alias token
            ParseResult subResult = matchedSubcommand->parse(
                tokens, currentTokenIndex, result.commandPath + " ");

            // Propagate the result (success, error, or help request) from the
            // subcommand.
            return subResult;
          }
        }

        // If not a flag/option or subcommand, treat as positional argument
        if (currentPositionalArgIndex < positionalArgs_.size())
        {
          const ArgumentDef &posArgDef =
              positionalArgs_[currentPositionalArgIndex];
          ArgValue parsedValue = parseTokenValue(token, posArgDef.type);

          if (parsedValue.isNone())
          {
            result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
            result.errorMessage =
                "Invalid value for positional argument '" + posArgDef.name + "'";
            std::cerr << "Error: " << result.errorMessage << "\n\n";
            generateHelp(std::cerr, commandPathPrefix);
            result.exitCode = 1;
            return result;
          }

          if (posArgDef.type == ARGTYPE_SINGLE)
          {
            result.positionalValues.push_back(parsedValue);
            currentTokenIndex++;
            currentPositionalArgIndex++;
          }
          else if (posArgDef.type == ARGTYPE_MULTIPLE)
          {
            // Ensure value type is string for MULTIPLE positional
            if (!parsedValue.isString())
            {
              result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
              result.errorMessage = "Internal error: Expected string value for "
                                    "multiple positional argument " +
                                    posArgDef.name;
              std::cerr << "Error: " << result.errorMessage << "\n\n";
              generateHelp(std::cerr, commandPathPrefix);
              result.exitCode = 1;
              return result;
            }

            std::vector<std::string> values;
            // Add the first value (must be string)
            const std::string *firstValStrPtr = parsedValue.getString();
            if (firstValStrPtr)
            { // Should always be true
              values.push_back(*firstValStrPtr);
            }

            currentTokenIndex++;
            while (currentTokenIndex < tokens.size() &&
                   !isFlagToken(tokens,
                                currentTokenIndex) /* && !isSubcommand(...) */)
            {
              // TODO: Add isSubcommand check more robustly?
              // Currently relies on subcommand check earlier in loop
              const lexer::Token &nextValueToken = tokens[currentTokenIndex];
              // Parse subsequent as strings
              ArgValue nextParsedValue =
                  parseTokenValue(nextValueToken, ARGTYPE_SINGLE);
              const std::string *nextValStrPtr = nextParsedValue.getString();
              if (nextValStrPtr)
              {
                values.push_back(*nextValStrPtr);
                currentTokenIndex++;
              }
              else
              {
                break;
              }
            }
            // Add the vector to positional args using ArgValue constructor
            result.positionalValues.push_back(ArgValue(values));
            currentPositionalArgIndex++;
          }
          // TODO: Add validation for number of positional args
        }
        else
        {
          // Unexpected token (neither flag, subcommand, nor expected positional)
          result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
          std::stringstream ss;
          ss << "Unexpected argument: ";
          token.print(ss);
          result.errorMessage = ss.str();
          // Print error and help for this command to stderr
          std::cerr << "Error: " << result.errorMessage << "\n\n";
          generateHelp(std::cerr, commandPathPrefix);
          result.exitCode = 1;
          return result;
        }
      }

      // Check for required keyword arguments
      for (std::vector<ArgumentDef>::const_iterator it = keywordArgs_.begin();
           it != keywordArgs_.end(); ++it)
      {
        const ArgumentDef &arg = *it;
        // Skip check for the automatic help flag
        if (arg.isHelpFlag)
          continue;
        if (arg.required &&
            keywordArgsSeen.find(arg.name) == keywordArgsSeen.end())
        {
          result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
          result.errorMessage =
              "Missing required option: " + arg.getDisplayName();
          std::cerr << "Error: " << result.errorMessage << "\n\n";
          generateHelp(std::cerr, commandPathPrefix);
          result.exitCode = 1;
          return result;
        }
      }

      // Check for required positional arguments
      if (positionalArgs_.size() > currentPositionalArgIndex)
      {
        for (size_t i = currentPositionalArgIndex; i < positionalArgs_.size();
             ++i)
        {
          const ArgumentDef &posArgDef = positionalArgs_[i];
          if (posArgDef.required)
          {
            result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
            result.errorMessage =
                "Missing required positional argument: " + posArgDef.name;
            std::cerr << "Error: " << result.errorMessage << "\n\n";
            generateHelp(std::cerr, commandPathPrefix);
            result.exitCode = 1;
            return result;
          }
          else
          {
            // Add default value for optional missing positional args
            result.positionalValues.push_back(posArgDef.defaultValue);
          }
        }
      }

      // Only execute the handler if parsing was successful and no subcommand took
      // over.
      if (handler_ && result.status == ParseResult::PPARSER_STATUS_SUCCESS)
      {
        result.exitCode = handler_(result);
      }

      return result;
    }

    // Generate help text for this command and its subcommands
    void generateHelp(std::ostream &os,
                      const std::string &commandPathPrefix) const
    {
      std::string fullCommandPath =
          commandPathPrefix.empty() ? name_ : commandPathPrefix + name_;
      if (!commandPathPrefix.empty() &&
          commandPathPrefix[commandPathPrefix.length() - 1] != ' ')
      {
        fullCommandPath = commandPathPrefix + " " + name_;
      }
      else
      {
        fullCommandPath = commandPathPrefix + name_;
      }

      os << "Usage: " << fullCommandPath;
      // Collect display names for positional args
      std::string positionalUsage;
      for (std::vector<ArgumentDef>::const_iterator it = positionalArgs_.begin();
           it != positionalArgs_.end(); ++it)
      {
        const ArgumentDef &posArg = *it;
        positionalUsage += " ";
        positionalUsage += (posArg.required ? "<" : "[");
        positionalUsage += posArg.name;
        positionalUsage += (posArg.required ? ">" : "]");
        if (posArg.type == ARGTYPE_MULTIPLE)
          positionalUsage += "...";
      }

      if (!keywordArgs_.empty())
        os << " [options]";
      if (!subcommands_.empty())
        os << " <command>";

      os << positionalUsage;

      os << "\n\n";

      if (!help_.empty())
      {
        os << help_ << "\n\n";
      }

      // Info about positional arguments
      if (!positionalArgs_.empty())
      {
        os << "Arguments:\n";
        for (std::vector<ArgumentDef>::const_iterator it =
                 positionalArgs_.begin();
             it != positionalArgs_.end(); ++it)
        {
          const ArgumentDef &arg = *it;
          os << "  " << arg.name << "\t" << arg.help;
          // Check default value is not NONE
          if (!arg.defaultValue.isNone())
          {
            os << " (default: ";
            arg.defaultValue.print(os);
            os << ")";
          }
          if (!arg.required)
            os << " [optional]";
          os << "\n";
        }
        os << "\n";
      }

      // Info about keyword arguments
      if (!keywordArgs_.empty())
      {
        os << "Options:\n";
        for (std::vector<ArgumentDef>::const_iterator it = keywordArgs_.begin();
             it != keywordArgs_.end(); ++it)
        {
          const ArgumentDef &arg = *it;
          os << "  " << arg.getDisplayName() << "\t" << arg.help;
          if (!arg.defaultValue.isNone())
          {
            // Only show non-default bools (true) or non-bool defaults
            const bool *bVal = arg.defaultValue.getBool();
            if (!bVal ||
                *bVal == true)
            { // Show default if it's true, or if not a bool
              os << " (default: ";
              arg.defaultValue.print(os);
              os << ")";
            }
          }
          if (arg.required)
            os << " [required]";
          os << "\n";
        }
        os << "\n";
      }

      // Available subcommands
      if (!subcommands_.empty())
      {
        os << "Commands:\n";
        for (std::map<std::string, std::shared_ptr<Command>>::const_iterator it =
                 subcommands_.begin();
             it != subcommands_.end(); ++it)
        {
          os << "  " << it->first; // Print the main command name
          // Print aliases if any exist
          const auto &aliases = it->second->getAliases();
          if (!aliases.empty())
          {
            os << " (" << aliases[0];
            for (size_t i = 1; i < aliases.size(); ++i)
            {
              os << ", " << aliases[i];
            }
            os << ")";
          }
          os << "\t" << it->second->getHelp() << "\n";
        }
        os << "\nUse '" << fullCommandPath
           << " <command> --help' for more information on a command.\n";
      }
    }

  private:
    // Private copy constructor and assignment operator to prevent copying
    Command(const Command &);
    Command &operator=(const Command &);

    std::string name_;
    std::string help_;
    std::vector<ArgumentDef> keywordArgs_;
    std::vector<ArgumentDef> positionalArgs_;
    std::map<std::string, std::shared_ptr<Command>> subcommands_;
    std::vector<std::string> aliases_;

  public:
    CommandHandler handler_;

  private:
    // Helper to check if the token at the given index is a flag/option token
    bool isFlagToken(const std::vector<lexer::Token> &tokens,
                     size_t index) const
    {
      if (index >= tokens.size())
        return false;
      lexer::TokenKind kind = tokens[index].getKind();
      return kind == lexer::TOK_FLAG_SHORT || kind == lexer::TOK_FLAG_LONG;
    }

    // Find keyword argument definition by flag name (use iterator loop)
    const ArgumentDef *findKeywordArg(
        const std::string &flag_name_value, // Name part only (e.g., "f", "force")
        bool is_short_flag_kind) const
    {
      for (std::vector<ArgumentDef>::const_iterator it = keywordArgs_.begin();
           it != keywordArgs_.end(); ++it)
      {
        const ArgumentDef &arg = *it;
        if (is_short_flag_kind)
        {
          // Compare flag_name_value against the name part of shortName (e.g.,
          // compare "f" with "-f")
          if (!arg.shortName.empty() && arg.shortName.length() > 1 &&
              arg.shortName.substr(1) == flag_name_value)
          {
            return &arg; // Return pointer to the element
          }
        }
        else
        {
          // Compare flag_name_value against the name part of longName (e.g.,
          // compare "force" with "--force")
          if (!arg.longName.empty() && arg.longName.length() > 2 &&
              arg.longName.substr(2) == flag_name_value)
          {
            return &arg;
          }
        }
      }
      return nullptr;
    }

    // Helper to parse a single token into an ArgValue based on expected type
    // Returns ArgValue (which manages memory for string/vector)
    ArgValue parseTokenValue(const lexer::Token &token,
                             ArgType expectedType) const
    {
      lexer::TokenKind kind = token.getKind();

      // Allow broader range of tokens to be interpreted as strings if expected
      bool expectString =
          (expectedType == ARGTYPE_SINGLE || expectedType == ARGTYPE_MULTIPLE ||
           expectedType == ARGTYPE_POSITIONAL);

      switch (kind)
      {
      case lexer::TOK_INT_LIT:
        if (expectedType == ARGTYPE_SINGLE || expectedType == ARGTYPE_POSITIONAL)
          return ArgValue(token.getIntValue());
        else if (expectString) // Allow int literal as string if needed
          return ArgValue(token.getStrLitValue());
        break;
      case lexer::TOK_FLOAT_LIT:
        if (expectedType == ARGTYPE_SINGLE || expectedType == ARGTYPE_POSITIONAL)
          return ArgValue(token.getFloatValue());
        else if (expectString) // Allow float literal as string if needed
          return ArgValue(token.getStrLitValue());
        break;
      case lexer::TOK_TRUE:
        if (expectedType == ARGTYPE_SINGLE || expectedType == ARGTYPE_POSITIONAL)
          return ArgValue(true);
        else if (expectString)
          return ArgValue("true");
        break;
      case lexer::TOK_FALSE:
        if (expectedType == ARGTYPE_SINGLE || expectedType == ARGTYPE_POSITIONAL)
          return ArgValue(false);
        else if (expectString)
          return ArgValue("false");
        break;
      case lexer::TOK_STR_LIT:
        if (expectString)
          return ArgValue(token.getStrLitValue());
        break;
      case lexer::TOK_ID:
        if (expectString)
          return ArgValue(token.getIdValue());
        break;
      default:
        break;
      }

      return ArgValue();
    }

    // Helper to add the standard help argument
    void ensureHelpArgument()
    {
      bool helpExists = false;
      for (std::vector<ArgumentDef>::const_iterator it = keywordArgs_.begin();
           it != keywordArgs_.end(); ++it)
      {
        if (it->name == "help")
        {
          helpExists = true;
          break;
        }
      }
      if (!helpExists)
      {
        keywordArgs_.push_back(
            ArgumentDef("help", "-h", "--help", "Show this help message and exit",
                        ARGTYPE_FLAG, false, ArgValue(false), true));
      }
    }

    // Check if the command has a specific alias
    bool hasAlias(const std::string &alias) const
    {
      return std::find(aliases_.begin(), aliases_.end(), alias) != aliases_.end();
    }
  };

  class ArgumentParser
  {
  public:
    ArgumentParser(std::string progName, std::string description = "")
        : programName_(progName), programDescription_(description),
          rootCommand_(std::make_shared<Command>(progName, description)) {}

    ~ArgumentParser() = default;

    // Get a reference to the root command to add arguments or subcommands
    Command &getRootCommand() { return *rootCommand_; }

    // Parse command line arguments from main(argc, argv)
    ParseResult parse(int argc, char *argv[])
    {
      if (argc < 1)
      {
        ParseResult errorResult;
        errorResult.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
        errorResult.errorMessage = "Invalid arguments provided (argc < 1)";
        errorResult.exitCode = 1;
        return errorResult;
      }

      // Combine args into a single string for the lexer
      std::stringstream ss;
      for (int i = 1; i < argc; ++i)
      {
        ss << argv[i] << " ";
      }
      return parse(ss.str());
    }

    // Parse command line arguments from a single string
    ParseResult parse(const std::string &commandLine)
    {
      if (!rootCommand_)
      {
        ParseResult errorResult;
        errorResult.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
        errorResult.errorMessage = "ArgumentParser is not initialized correctly.";
        errorResult.exitCode = 1;
        return errorResult;
      }

      try
      {
        lexer::Source source = lexer::Source::fromString(commandLine, "<cli>");
        std::vector<lexer::Token> tokens = lexer::Lexer::tokenize(source);

        // Filter out TOK_EOF at the end
        if (!tokens.empty() && tokens.back().getKind() == lexer::TOK_EOF)
        {
          tokens.pop_back();
        }

        size_t tokenIndex = 0;
        ParseResult result = rootCommand_->parse(tokens, tokenIndex, "");

        // Return early in the event of an error or help request from the parse
        // call
        if (result.status == ParseResult::PPARSER_STATUS_PARSE_ERROR)
        {
          return result;
        }
        if (result.status == ParseResult::PPARSER_STATUS_HELP_REQUESTED)
        {
          return result;
        }

        // Parsing succeeded, but not all tokens were consumed (and no subcommand
        // handled them)
        if (tokenIndex < tokens.size())
        {
          result.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
          std::stringstream ss;
          ss << "Unexpected arguments starting from: ";
          tokens[tokenIndex].print(ss);
          result.errorMessage = ss.str();
          std::cerr << "Error: " << result.errorMessage << "\n\n";
          // Show root help on unexpected trailing arguments error (to stderr)
          rootCommand_->generateHelp(std::cerr, "");
          result.exitCode = 1;
          return result;
        }

        return result;
      }
      catch (const lexer::LexError &e)
      {
        std::cerr << "Lexer Error: " << e.what() << std::endl;
        ParseResult errorResult;
        errorResult.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
        errorResult.errorMessage = e.what();
        errorResult.exitCode = 1;
        // Show root help on lexer error (to stderr)
        if (rootCommand_)
          rootCommand_->generateHelp(std::cerr, "");
        return errorResult;
      }
      catch (const std::exception &e)
      {
        // Catch standard exceptions that might arise
        std::cerr << "Parser Error: " << e.what() << std::endl;
        ParseResult errorResult;
        errorResult.status = ParseResult::PPARSER_STATUS_PARSE_ERROR;
        errorResult.errorMessage = e.what();
        errorResult.exitCode = 1;
        // Show root help on general parser error (to stderr)
        if (rootCommand_)
          rootCommand_->generateHelp(std::cerr, "");
        return errorResult;
      }
    }

  private:
    // Prevent copying
    ArgumentParser(const ArgumentParser &);
    ArgumentParser &operator=(const ArgumentParser &);

    std::string programName_;
    std::string programDescription_;
    std::shared_ptr<Command> rootCommand_;
  }; // class ArgumentParser

} // namespace pparser

#endif // PPARSER_HPP