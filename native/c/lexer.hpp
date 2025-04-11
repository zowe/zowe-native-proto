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

#ifndef LEXER_HPP
#define LEXER_HPP

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace lexer {
/**
 * Represents a location within a piece of source code.
 * Includes the filename, line, and column.
 */
class Location {
public:
  Location() : filename(""), line(1), col(1) {}
  Location(const std::string &filename, size_t line, size_t col)
      : filename(filename), line(line), col(col) {}

  const std::string &getFilename() const { return filename; }
  size_t getLine() const { return line; }
  size_t getCol() const { return col; }

  void print(std::ostream &os) const {
    os << (filename.empty() ? "<string>" : filename) << " (" << line << ":"
       << col << ")";
  }

  std::string filename;
  size_t line;
  size_t col;
};

inline std::ostream &operator<<(std::ostream &os, const Location &loc) {
  loc.print(os);
  return os;
}

/**
 * Iterator over a Source object's characters.
 * Keeps track of line and column and can produce a Location.
 */
class SourceIterator {
public:
  SourceIterator(const std::string &filename, const std::vector<char> &code)
      : code(code), filename(filename), line(1), col(1), pos(0) {}

  char current() const {
    if (pos < code.size()) {
      return code[pos];
    }
    return '\0';
  }

  char peek() const {
    if (pos + 1 < code.size()) {
      return code[pos + 1];
    }
    return '\0';
  }

  char peek2() const {
    if (pos + 2 < code.size()) {
      return code[pos + 2];
    }
    return '\0';
  }

  void next() {
    if (pos < code.size()) {
      if (code[pos] == '\n') {
        line++;
        col = 1;
      } else if (code[pos] == '\t') {
        col += 4;
      } else {
        col++;
      }
      pos++;
    }
  }

  void next2() {
    next();
    next();
  }

  bool hasMore() const { return pos < code.size(); }
  size_t position() const { return pos; }
  Location getLocation() const { return Location(filename, line, col); }
  size_t getLine() const { return line; }
  size_t getCol() const { return col; }
  const std::string &getFilename() const { return filename; }

private:
  const std::vector<char> &code;
  std::string filename;
  size_t line;
  size_t col;
  size_t pos;
};

/**
 * Represents a piece of source code.
 * Includes the filename (if any) and the raw source code.
 */
class Source {
public:
  Source() {}

  static Source fromFile(const std::string &filename) {
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open file: " + filename);
    }

    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    if (size < 0) {
      throw std::runtime_error("Error determining size of file: " + filename);
    }
    std::vector<char> buffer(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);

    if (size > 0) {
      file.read(buffer.data(), size);
      if (!file && !file.eof()) {
        throw std::runtime_error("Error reading file: " + filename);
      }
    }

    return Source(filename, buffer);
  }

  static Source fromString(const std::string &codeStr,
                           const std::string &filename = "<string>") {
    std::vector<char> chars(codeStr.begin(), codeStr.end());
    return Source(filename, chars);
  }

  const std::string &getFilename() const { return filename; }
  const std::vector<char> &getCode() const { return code; }

  SourceIterator getIterator() const { return SourceIterator(filename, code); }

  const char *getCodePtr() const { return code.empty() ? NULL : &code[0]; }

private:
  Source(const std::string &filename, const std::vector<char> &code)
      : filename(filename), code(code) {}

  std::string filename;
  std::vector<char> code;
};

enum LexErrorKind {
  INVALID_CHAR,
  UNCLOSED_STRING,
  UNKNOWN_ESCAPE,
  INT_OUT_OF_RANGE,
  INCOMPLETE_INT,
  FLOAT_OUT_OF_RANGE,
  INVALID_FLOAT
};

class LexError : public std::exception {
public:
  LexError(const Location &loc, LexErrorKind kind) : loc(loc), kind(kind) {}

  static LexError invalidChar(const Location &loc) {
    return LexError(loc, INVALID_CHAR);
  }
  static LexError unclosedString(const Location &loc) {
    return LexError(loc, UNCLOSED_STRING);
  }
  static LexError unknownEscape(const Location &loc) {
    return LexError(loc, UNKNOWN_ESCAPE);
  }
  static LexError intOutOfRange(const Location &loc) {
    return LexError(loc, INT_OUT_OF_RANGE);
  }
  static LexError incompleteInt(const Location &loc) {
    return LexError(loc, INCOMPLETE_INT);
  }
  static LexError floatOutOfRange(const Location &loc) {
    return LexError(loc, FLOAT_OUT_OF_RANGE);
  }
  static LexError invalidFloat(const Location &loc) {
    return LexError(loc, INVALID_FLOAT);
  }

  const Location &getLocation() const { return loc; }
  LexErrorKind getKind() const { return kind; }

  virtual const char *what() const throw() {
    if (message.empty()) {
      message = toString();
    }
    return message.c_str();
  }

  std::string toString() const {
    std::ostringstream ss;
    ss << loc << ": ";

    switch (kind) {
    case INVALID_CHAR:
      ss << "invalid character";
      break;
    case UNCLOSED_STRING:
      ss << "unclosed string literal";
      break;
    case UNKNOWN_ESCAPE:
      ss << "unknown escape character";
      break;
    case INT_OUT_OF_RANGE:
      ss << "integer literal out of 64-bit range";
      break;
    case INCOMPLETE_INT:
      ss << "incomplete integer literal";
      break;
    case FLOAT_OUT_OF_RANGE:
      ss << "floating-point literal out of range";
      break;
    case INVALID_FLOAT:
      ss << "invalid floating-point literal";
      break;
    default:
      ss << "unknown lexer error";
      break;
    }

    return ss.str();
  }

private:
  Location loc;
  LexErrorKind kind;
  mutable std::string message;
};

enum Base { DEC = 10, BIN = 2, HEX = 16 };

enum TokenKind {
  TOK_EOF,

  TOK_IF,
  TOK_ELSE,
  TOK_FOR,
  TOK_IN,
  TOK_WHILE,
  TOK_BREAK,
  TOK_RETURN,
  TOK_INT,
  TOK_BOOL,
  TOK_STRING,
  TOK_AND,
  TOK_OR,
  TOK_NOT,
  TOK_TRUE,
  TOK_FALSE,

  TOK_ASSIGN,
  TOK_PLUS,
  TOK_MINUS,
  TOK_DOUBLE_MINUS,
  TOK_TIMES,
  TOK_DIVIDE,
  TOK_MODULO,
  TOK_SHL,
  TOK_SHR,
  TOK_LESS,
  TOK_GREATER,
  TOK_LESS_EQ,
  TOK_GREATER_EQ,
  TOK_EQ,
  TOK_NOT_EQ,

  TOK_LPAREN,
  TOK_RPAREN,
  TOK_LBRACE,
  TOK_RBRACE,
  TOK_LBRACKET,
  TOK_RBRACKET,
  TOK_SEMI,
  TOK_COLON,
  TOK_COMMA,
  TOK_DOT,

  TOK_ID,
  TOK_INT_LIT,
  TOK_FLOAT_LIT,
  TOK_STR_LIT,
  TOK_FLAG_SHORT, // e.g., -f
  TOK_FLAG_LONG   // e.g., --force
};

struct Span {
  size_t start;
  size_t end;

  Span() : start(0), end(0) {}
  Span(size_t s, size_t e) : start(s), end(e) {}
};

struct StringRef {
  const char *start;
  size_t length;

  StringRef() : start(NULL), length(0) {}
  StringRef(const char *s, size_t l) : start(s), length(l) {}

  std::string toString() const {
    if (!start)
      return "";
    return std::string(start, length);
  }

  void print(std::ostream &os) const {
    if (start) {
      os.write(start, length);
    }
  }
};

union TokenData {
  struct IntLitData {
    long long value;
    Base base;
  } intLit;

  struct FloatLitData {
    double value;
    bool hasExponent;
  } floatLit;

  StringRef stringRef;

  TokenData() : stringRef() {}
  ~TokenData() {}
};

// No-alloc class to represent lexical tokens
class Token {
public:
  Token() : kind(TOK_EOF), span() {}

  // Constructor for simple tokens
  Token(TokenKind kind, const Span &span) : kind(kind), span(span) {
    data.intLit.value = 0;
    data.intLit.base = DEC;
  }

  Token(const Token &other) : kind(other.kind), span(other.span) {
    copyData(other);
  }

  Token &operator=(const Token &other) {
    if (this != &other) {
      kind = other.kind;
      span = other.span;
      copyData(other);
    }
    return *this;
  }

  // TODO: May be unneeded
  ~Token() {}

  // Static factory methods for complex tokens below

  static Token makeId(const char *start_ptr, size_t len, const Span &span) {
    Token token(TOK_ID, span);
    token.data.stringRef.start = start_ptr;
    token.data.stringRef.length = len;
    return token;
  }

  static Token makeShortFlag(const char *name_ptr, size_t len,
                             const Span &span) {
    Token token(TOK_FLAG_SHORT, span);
    token.data.stringRef.start = name_ptr;
    token.data.stringRef.length = len;
    return token;
  }

  static Token makeLongFlag(const char *name_ptr, size_t len,
                            const Span &span) {
    Token token(TOK_FLAG_LONG, span);
    token.data.stringRef.start = name_ptr;
    token.data.stringRef.length = len;
    return token;
  }

  static Token makeIntLit(long long value, Base base, const Span &span) {
    Token token(TOK_INT_LIT, span);
    token.data.intLit.value = value;
    token.data.intLit.base = base;
    return token;
  }

  static Token makeFloatLit(double value, bool hasExponent, const Span &span) {
    Token token(TOK_FLOAT_LIT, span);
    token.data.floatLit.value = value;
    token.data.floatLit.hasExponent = hasExponent;
    return token;
  }

  // Stores raw string content (pointer between quotes, length).
  // Escape sequences are processed on demand by getStrLitValue().
  static Token makeStrLit(const char *content_start_ptr, size_t content_len,
                          const Span &span) {
    Token token(TOK_STR_LIT, span);
    token.data.stringRef.start = content_start_ptr;
    token.data.stringRef.length = content_len;
    return token;
  }

  TokenKind getKind() const { return kind; }
  const Span &getSpan() const { return span; }

  // Getter methods for complex token data below

  // Identifier/flag content (raw pointer and length)
  const char *getStringRefStart() const {
    // Check for kinds that use stringRef
    if (kind == TOK_ID || kind == TOK_STR_LIT || kind == TOK_FLAG_SHORT ||
        kind == TOK_FLAG_LONG) {
      return data.stringRef.start;
    }
    return nullptr;
  }

  size_t getStringRefLength() const {
    // Check for kinds that use stringRef
    if (kind == TOK_ID || kind == TOK_STR_LIT || kind == TOK_FLAG_SHORT ||
        kind == TOK_FLAG_LONG) {
      return data.stringRef.length;
    }
    return 0u;
  }

  // Helper to get Identifier/Flag value as std::string (allocates)
  std::string getIdValue() const {
    // NOTE: This returns the NAME of the flag, not including dashes.
    if (kind == TOK_ID || kind == TOK_FLAG_SHORT || kind == TOK_FLAG_LONG) {
      if (data.stringRef.start) {
        return std::string(data.stringRef.start, data.stringRef.length);
      } else {
        return "";
      }
    }
    // TODO: Consider throwing or returning a sentinel
    throw std::runtime_error("Token is not an identifier or flag");
  }

  long long getIntValue() const {
    if (kind == TOK_INT_LIT) {
      return data.intLit.value;
    }
    // TODO: Consider throwing or returning a sentinel
    throw std::runtime_error("Token is not an integer literal");
  }

  Base getIntBase() const {
    if (kind == TOK_INT_LIT) {
      return data.intLit.base;
    }
    // TODO: Consider throwing or returning a sentinel
    throw std::runtime_error("Token is not an integer literal");
  }

  double getFloatValue() const {
    if (kind == TOK_FLOAT_LIT) {
      return data.floatLit.value;
    }
    // TODO: Consider throwing or returning a sentinel
    throw std::runtime_error("Token is not a float literal");
  }

  bool hasFloatExponent() const {
    if (kind == TOK_FLOAT_LIT) {
      return data.floatLit.hasExponent;
    }
    // TODO: Consider throwing or returning a sentinel
    throw std::runtime_error("Token is not a float literal");
  }

  // Get a string literal value (processes escapes, allocates a new std::string)
  std::string getStrLitValue() const {
    if (kind != TOK_STR_LIT) {
      throw std::runtime_error("Token is not a string literal");
    }
    if (!data.stringRef.start)
      return ""; // Should not happen if created correctly

    std::string processed_value;
    // Estimate capacity, might overestimate if many escapes
    processed_value.reserve(data.stringRef.length);
    const char *ptr = data.stringRef.start;
    const char *end = ptr + data.stringRef.length;

    while (ptr < end) {
      if (*ptr == '\\') {
        ptr++; // Consume backslash
        if (ptr >= end) {
          // This indicates an error caught by lexer or malformed raw slice
          // For robustness, stop processing here and add a literal backslash
          processed_value += '\\';
          break;
        }
        switch (*ptr) {
        case 'n':
          processed_value += '\n';
          break;
        case 'r':
          processed_value += '\r';
          break;
        case 't':
          processed_value += '\t';
          break;
        case '\\':
          processed_value += '\\';
          break;
        case '"':
          processed_value += '"';
          break;
        case '0':
          processed_value += '\0';
          break;
        // Add other escapes if supported by the language
        default:
          // Unknown escape was already checked by lexer.
          // If it got here, maybe append the literal char after backslash?
          processed_value += *ptr;
          break;
        }
      } else {
        // Should not encounter unescaped double quotes here as they delimit the
        // raw slice. If found, it's an internal error. Add it literally for
        // now.
        processed_value += *ptr;
      }
      ptr++;
    }
    return processed_value;
  }

  // Print token to stream
  void print(std::ostream &os) const {
    // cached map for keyword/symbol lookup
    static std::map<TokenKind, std::string> simpleTokens;
    if (simpleTokens.empty()) { // Initialize on first call
      simpleTokens[TOK_EOF] = "<EOF>";
      simpleTokens[TOK_IF] = "if";
      simpleTokens[TOK_ELSE] = "else";
      simpleTokens[TOK_FOR] = "for";
      simpleTokens[TOK_IN] = "in";
      simpleTokens[TOK_WHILE] = "while";
      simpleTokens[TOK_BREAK] = "break";
      simpleTokens[TOK_RETURN] = "return";
      simpleTokens[TOK_INT] = "int";
      simpleTokens[TOK_BOOL] = "bool";
      simpleTokens[TOK_STRING] = "string";
      simpleTokens[TOK_AND] = "and";
      simpleTokens[TOK_OR] = "or";
      simpleTokens[TOK_NOT] = "not";
      simpleTokens[TOK_TRUE] = "true";
      simpleTokens[TOK_FALSE] = "false";
      simpleTokens[TOK_ASSIGN] = "=";
      simpleTokens[TOK_PLUS] = "+";
      simpleTokens[TOK_MINUS] = "-";
      simpleTokens[TOK_DOUBLE_MINUS] = "--";
      simpleTokens[TOK_TIMES] = "*";
      simpleTokens[TOK_DIVIDE] = "/";
      simpleTokens[TOK_MODULO] = "%";
      simpleTokens[TOK_SHL] = "<<";
      simpleTokens[TOK_SHR] = ">>";
      simpleTokens[TOK_LESS] = "<";
      simpleTokens[TOK_GREATER] = ">";
      simpleTokens[TOK_LESS_EQ] = "<=";
      simpleTokens[TOK_GREATER_EQ] = ">=";
      simpleTokens[TOK_EQ] = "==";
      simpleTokens[TOK_NOT_EQ] = "!=";
      simpleTokens[TOK_LPAREN] = "(";
      simpleTokens[TOK_RPAREN] = ")";
      simpleTokens[TOK_LBRACE] = "{";
      simpleTokens[TOK_RBRACE] = "}";
      simpleTokens[TOK_LBRACKET] = "[";
      simpleTokens[TOK_RBRACKET] = "]";
      simpleTokens[TOK_SEMI] = ";";
      simpleTokens[TOK_COLON] = ":";
      simpleTokens[TOK_COMMA] = ",";
      simpleTokens[TOK_DOT] = ".";
    }

    // Handle flag tokens
    if (kind == TOK_FLAG_SHORT) {
      os << "-";
      if (data.stringRef.start)
        os.write(data.stringRef.start, data.stringRef.length);
      else
        os << "<null_short_flag>";
      return;
    }
    if (kind == TOK_FLAG_LONG) {
      os << "--";
      if (data.stringRef.start)
        os.write(data.stringRef.start, data.stringRef.length);
      else
        os << "<null_long_flag>";
      return;
    }

    // Handle simple tokens first using the map
    auto it = simpleTokens.find(kind);
    if (it != simpleTokens.end()) {
      os << it->second;
      return;
    }

    // Handle complex tokens
    switch (kind) {
    case TOK_ID: // Includes flags if using TOK_ID for them
      if (data.stringRef.start) {
        os.write(data.stringRef.start, data.stringRef.length);
      } else {
        os << "<null_id>"; // Should not happen
      }
      break;
    case TOK_STR_LIT:
      os << "\"";
      // Print raw content, processing escapes for display purposes
      if (data.stringRef.start) {
        const char *ptr = data.stringRef.start;
        const char *end = ptr + data.stringRef.length;
        while (ptr < end) {
          char c = *ptr;
          if (c == '\\') {
            ptr++; // Look at the escaped char
            if (ptr < end) {
              switch (*ptr) {
              case 'n':
                os << "\\n";
                break;
              case 'r':
                os << "\\r";
                break;
              case 't':
                os << "\\t";
                break;
              case '\\':
                os << "\\\\";
                break;
              case '"':
                os << "\\\"";
                break; // Show the escaped quote
              case '0':
                os << "\\0";
                break;
              default: // Print unknown escapes literally for representation
                // Check if printable, otherwise use hex?
                if (std::isprint(static_cast<unsigned char>(*ptr))) {
                  os << '\\' << *ptr;
                } else {
                  // Simple fallback: print original backslash only
                  os << '\\';
                  // Decrement ptr so the non-printable char is handled below
                  ptr--;
                }
                break;
              }
            } else {
              os << '\\'; // Dangling backslash at end of content
            }
          } else if (c == '"') { // Should not happen in raw content, but escape
                                 // if found
            os << "\\\"";
          } else if (c == '\n') { // Represent newline visually
            os << "\\n";
          } else if (c == '\r') {
            os << "\\r";
          } else if (c == '\t') {
            os << "\\t";
          } else if (std::isprint(static_cast<unsigned char>(c))) {
            os << c; // Print printable chars directly
          } else {
            // non-printable chars, print Unicode replacement character
            os << u8"\uFFFD";
          }
          ptr++;
        }
      }
      os << "\"";
      break;
    case TOK_INT_LIT:
      // ... (integer printing logic remains the same) ...
      if (data.intLit.base == HEX) {
        std::ios_base::fmtflags original_flags = os.flags();
        os << "0x" << std::hex << data.intLit.value;
        os.flags(original_flags);
      } else if (data.intLit.base == BIN) {
        os << "0b";
        if (data.intLit.value == 0) {
          os << "0";
        } else {
          // Simple binary printing for positive numbers up to 63 bits
          bool leading_zeros = true;
          for (int i = 63; i >= 0; --i) {
            if ((data.intLit.value >> i) & 1) {
              leading_zeros = false;
              os << '1';
            } else if (!leading_zeros) {
              os << '0';
            }
          }
          if (leading_zeros)
            os << '0'; // Should only happen for value 0, handled above
        }
      } else // DEC
      {
        os << data.intLit.value;
      }
      break;
    case TOK_FLOAT_LIT: {
      std::ios_base::fmtflags original_flags = os.flags();
      std::streamsize original_precision = os.precision();

      // Preserve default float format unless scientific notation was used
      if (data.floatLit.hasExponent) {
        os.setf(std::ios::scientific, std::ios::floatfield);
      }
      // else: use default formatting (often avoids trailing zeros vs fixed)

      os << data.floatLit.value;

      os.flags(original_flags);
      os.precision(original_precision);
    } break;
    default: // note: should not happen if all kinds are covered
      os << "<Unknown TokenKind: " << static_cast<int>(kind) << ">";
      break;
    }
  }

private:
  // Helper to copy data based on kind (used by copy ctor and assignment)
  // This should be safe now, as it relies on the (trivial) copy/assignment
  // of the member structs (IntLitData, FloatLitData, StringRef).
  void copyData(const Token &other) {
    switch (other.kind) {
    case TOK_ID:
    case TOK_STR_LIT:
    case TOK_FLAG_SHORT:
    case TOK_FLAG_LONG:
      // Assign StringRef (member-wise copy)
      data.stringRef = other.data.stringRef;
      break;
    case TOK_INT_LIT:
      // Assign IntLitData struct (member-wise copy)
      data.intLit = other.data.intLit;
      break;
    case TOK_FLOAT_LIT:
      // Assign FloatLitData struct (member-wise copy)
      data.floatLit = other.data.floatLit;
      break;
    default:
      // For simple tokens or TOK_EOF, the data doesn't strictly matter,
      // but copying something ensures defined state
      data.intLit = other.data.intLit;
      break;
    }
  }

  TokenKind kind;
  Span span;
  TokenData data; // Union holding complex data (value types or StringRef)
};

// Stream operator for Token
inline std::ostream &operator<<(std::ostream &os, const Token &token) {
  token.print(os);
  return os;
}

// Facility for tokenizing source code or input strings
class Lexer {
public:
  /**
   * Primary static function to tokenize source code.
   * Takes a Source object and returns a vector of Tokens.
   * Note: Source object must remain valid while Tokens are used.
   */
  static std::vector<Token> tokenize(const Source &source) {
    Lexer lexer(source);
    lexer.lexAll();
    return lexer.tokens;
  }

private:
  // Private constructor - only used internally by the static tokenize method
  Lexer(const Source &source)
      : iter(source.getIterator()), source_code_ptr(source.getCodePtr()) {
    size_t estimated_tokens = source.getCode().size() / 5;
    if (estimated_tokens > 10) { // Avoid tiny allocations
      tokens.reserve(estimated_tokens);
    }
  }

  // Helper functions for character classification
  // TODO: Using direct checks can be slightly faster if locale is not a
  // concern..
  static bool isAsciiDecDigit(char c) { return c >= '0' && c <= '9'; }
  static bool isAsciiHexDigit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
  }
  static bool isAsciiBinDigit(char c) { return c == '0' || c == '1'; }
  // Using std::isalpha for broader identifier start definition (factors in
  // locale) Sticking with std::isalpha unless pure ASCII is
  // guaranteed/required.
  static bool isIdentStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '$' ||
           c == '_' || c == '/';
  }
  static bool isIdentCont(char c) {
    return isIdentStart(c) || isAsciiDecDigit(c) || c == '.' || c == '/';
  }

  // Combined digit check for lexNumber
  static bool isDigitOrUnderscore(char c, Base base) {
    if (c == '_')
      return true;
    switch (base) {
    case DEC:
      return isAsciiDecDigit(c);
    case HEX:
      return isAsciiHexDigit(c);
    case BIN:
      return isAsciiBinDigit(c);
    default:
      return false;
    }
  }

  // Core lexing driver function
  void lexAll() {
    tokens.clear(); // Clear if Lexer object was reused
    while (true) {
      eatWhitespaceAndComments();
      Token token = nextToken();
      tokens.push_back(token);
      if (token.getKind() == TOK_EOF) {
        break;
      }
    }
  }

  // Skips over whitespace and single-line comments
  void eatWhitespaceAndComments() {
    while (true) {
      char c = current(); // Cache current char
      switch (c) {
      // Whitespace
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        next();
        break;

      // Comments (single line //)
      case '/':
        if (peek() == '/') {
          next2(); // Consume //
          while (true) {
            char comm_c = current();
            if (comm_c == '\n' || comm_c == '\0')
              break;
            next();
          }
        } else {
          // Not a comment, maybe division
          return;
        }
        break;

      // End of whitespace/comment section
      default:
        return;
      }
    }
  }

  // Lexes the next token from the input stream
  Token nextToken() {
    size_t start_pos = iter.position();
    Location start_loc = iter.getLocation(); // Get location at start

    char c = current();
    switch (c) {
    // EOF
    case '\0':
      return Token(TOK_EOF, Span(start_pos, start_pos));

    // Single-character tokens that are unambiguous
    case '+':
      next();
      return Token(TOK_PLUS, Span(start_pos, iter.position()));
    case '*':
      next();
      return Token(TOK_TIMES, Span(start_pos, iter.position()));
    case '%':
      next();
      return Token(TOK_MODULO, Span(start_pos, iter.position()));
    case '(':
      next();
      return Token(TOK_LPAREN, Span(start_pos, iter.position()));
    case ')':
      next();
      return Token(TOK_RPAREN, Span(start_pos, iter.position()));
    case '{':
      next();
      return Token(TOK_LBRACE, Span(start_pos, iter.position()));
    case '}':
      next();
      return Token(TOK_RBRACE, Span(start_pos, iter.position()));
    case '[':
      next();
      return Token(TOK_LBRACKET, Span(start_pos, iter.position()));
    case ']':
      next();
      return Token(TOK_RBRACKET, Span(start_pos, iter.position()));
    case ';':
      next();
      return Token(TOK_SEMI, Span(start_pos, iter.position()));
    case ':':
      next();
      return Token(TOK_COLON, Span(start_pos, iter.position()));
    case ',':
      next();
      return Token(TOK_COMMA, Span(start_pos, iter.position()));

    // Potentially multi-character tokens
    case '-':
      next();               // Consume '-'
      if (current() == '-') // Check for second '-'
      {
        next(); // Consume second '-'
                // Check if it's followed by identifier start (long flag)
        if (isIdentStart(current())) {
          // Current position is *after* '--'. Start pos was before first '-'.
          return lexLongFlag(start_pos); // Pass original start pos
        }
        // Check if just '--' (often end of options)
        else if (!isIdentCont(current()) && !isAsciiDecDigit(current())) {
          return Token(TOK_DOUBLE_MINUS, Span(start_pos, iter.position()));
        } else {
          // Case like "--1" or "--_" ? Treat as error? Or allow as part of long
          // flag? Current lexLongFlag expects ident start. Let's treat as error
          // here.
          throw LexError::invalidChar(
              Location(start_loc.filename, start_loc.line,
                       start_loc.col + 2)); // Error after '--'
        }
      }
      // Check if it's followed by identifier or digit (short flag)
      else if (isIdentStart(current()) || isAsciiDecDigit(current())) {
        // Current position is after '-'. Start pos was before '-'.
        return lexShortFlag(start_pos); // Pass original start pos
      }
      // Check for negative numbers (handled by lexNumber starting with digit)
      // or just the minus operator
      else {
        return Token(TOK_MINUS, Span(start_pos, iter.position()));
      }

    case '/': // Division, start of comment, or start of path-like identifier
      // Comments are handled by eatWhitespaceAndComments
      // Check if it starts a path-like identifier
      if (isIdentCont(peek())) { // If '/' is followed by another identifier
                                 // character...
        return lexIdentifierOrKeyword(
            start_pos); // ...treat it as the start of an identifier
      } else {
        // Otherwise, it's the division operator
        next();
        return Token(TOK_DIVIDE, Span(start_pos, iter.position()));
      }

    case '<':
      next();
      if (current() == '<') {
        next();
        return Token(TOK_SHL, Span(start_pos, iter.position()));
      }
      if (current() == '=') {
        next();
        return Token(TOK_LESS_EQ, Span(start_pos, iter.position()));
      }
      return Token(TOK_LESS, Span(start_pos, iter.position()));

    case '>':
      next();
      if (current() == '>') {
        next();
        return Token(TOK_SHR, Span(start_pos, iter.position()));
      }
      if (current() == '=') {
        next();
        return Token(TOK_GREATER_EQ, Span(start_pos, iter.position()));
      }
      return Token(TOK_GREATER, Span(start_pos, iter.position()));

    case '=':
      next();
      if (current() == '=') {
        next();
        return Token(TOK_EQ, Span(start_pos, iter.position()));
      }
      return Token(TOK_ASSIGN, Span(start_pos, iter.position()));

    case '!':
      next();
      if (current() == '=') {
        next();
        return Token(TOK_NOT_EQ, Span(start_pos, iter.position()));
      }
      // Assume '!' is TOK_NOT if not followed by '='
      return Token(TOK_NOT, Span(start_pos, iter.position()));

    // String literals
    case '"':
      return lexString(start_pos); // Pass start position

    // Numbers (Integers or Floats)
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return lexNumber(start_pos); // Pass start position

    // Identifiers or Keywords
    default:
      if (isIdentStart(c)) {
        return lexIdentifierOrKeyword(start_pos); // Pass start position
      }
      // Unknown character
      throw LexError::invalidChar(start_loc); // Use location at start of char
    }
  }

  // Lexes an identifier or a keyword
  Token lexIdentifierOrKeyword(size_t start_pos) {
    // start_pos is the position of the first character
    // current() is the first character
    next(); // Consume the start character

    while (isIdentCont(current())) {
      next();
    }
    size_t end_pos = iter.position();
    size_t length = end_pos - start_pos;
    const char *id_start_ptr = source_code_ptr + start_pos;

    // Check if the identifier slice matches a keyword
    // Keyword lookup: Using std::map requires temporary string.
    // Optimization: Use a structure that can check char*/len directly.
    // Simple linear scan for small keyword sets (pre-C++11 friendly):
    struct Keyword {
      const char *name;
      size_t len;
      TokenKind kind;
    };
    static const Keyword keywords[] = {
        {"if", 2, TOK_IF},         {"else", 4, TOK_ELSE},
        {"for", 3, TOK_FOR},       {"in", 2, TOK_IN},
        {"while", 5, TOK_WHILE},   {"break", 5, TOK_BREAK},
        {"return", 6, TOK_RETURN}, {"int", 3, TOK_INT},
        {"bool", 4, TOK_BOOL},     {"string", 6, TOK_STRING},
        {"and", 3, TOK_AND},       {"or", 2, TOK_OR},
        {"not", 3, TOK_NOT},       {"true", 4, TOK_TRUE},
        {"false", 5, TOK_FALSE},   {NULL, 0, TOK_EOF} // Sentinel
    };

    for (int i = 0; keywords[i].name != NULL; ++i) {
      if (keywords[i].len == length &&
          memcmp(id_start_ptr, keywords[i].name, length) == 0) {
        return Token(keywords[i].kind, Span(start_pos, end_pos));
      }
    }

    // Not a keyword, it's an identifier
    return Token::makeId(id_start_ptr, length, Span(start_pos, end_pos));
  }

  // Lexes a string literal enclosed in double quotes
  // Returns token with raw content slice (pointer/length between quotes)
  Token lexString(size_t span_start) {
    // span_start is position of opening quote "
    Location start_loc = iter.getLocation();    // Location of "
    next();                                     // Consume the opening quote "
    size_t content_start_pos = iter.position(); // Position after "

    while (true) {
      Location char_loc = iter.getLocation(); // Location of current char
      char c = current();
      if (c == '\0') {
        // Error location: Ideally point to the opening quote or where EOF
        // encountered
        throw LexError::unclosedString(start_loc);
      }
      if (c == '\n') {
        // Strings cannot contain raw newlines (adjust if language allows)
        throw LexError::unclosedString(char_loc); // Error at the newline
      }
      if (c == '"') {
        break; // End of string content
      }

      if (c == '\\') {                            // Escape sequence
        Location escape_loc = iter.getLocation(); // Location of backslash
        next();                                   // Consume backslash
        char escaped_char = current();
        if (escaped_char == '\0' ||
            escaped_char == '\n') { // Invalid state after backslash
          throw LexError::unclosedString(escape_loc); // Error at the backslash
        }
        // Validate escape sequence based on language rules
        switch (escaped_char) {
        case 'n':
        case 'r':
        case 't':
        case '\\':
        case '"':
        case '0':
          // Valid escapes in this language
          break;
        // Add other valid escapes (e.g., \xHH, \uHHHH) if needed
        default:
          // Unknown escape sequence
          throw LexError::unknownEscape(
              iter.getLocation()); // Error at char after backslash
        }
        next(); // Consume the character after backslash
      } else {
        next(); // Consume regular character
      }
    }

    size_t content_end_pos = iter.position(); // Position of closing quote "
    next();                                   // Consume closing quote "
    size_t span_end = iter.position();

    const char *content_start_ptr = source_code_ptr + content_start_pos;
    size_t content_length = content_end_pos - content_start_pos;

    return Token::makeStrLit(content_start_ptr, content_length,
                             Span(span_start, span_end));
  }

  // Lexes a number (integer or float)
  Token lexNumber(size_t start_pos) {
    // start_pos is position of first digit
    Location start_loc = iter.getLocation(); // Location of first digit
    Base base = DEC;
    bool is_float = false;
    bool has_exponent = false;

    // Use a temporary buffer for digits to pass to strtoll/strtod
    // Avoids std::string allocation overhead in the common case.
    // Max length: 64-bit int ~20 digits, double ~30 chars? Add
    // prefixes/signs/etc.
    char num_buffer[64]; // Should be sufficient for valid numbers
    int buffer_idx = 0;
    const int buffer_max_idx =
        sizeof(num_buffer) - 1; // Leave space for null terminator

    auto addToBuffer = [&](char ch) {
      if (buffer_idx < buffer_max_idx) {
        num_buffer[buffer_idx++] = ch;
      } else {
        // Buffer overflow - indicates extremely long literal, likely out of
        // range anyway Throw specific error? Or let range check handle it? Let
        // range check handle it for now. Don't add more chars. Or, dynamically
        // allocate a larger buffer if truly huge numbers needed? Stick to fixed
        // buffer for pre-C++11 simplicity.
      }
    };

    // Check for base prefixes (0x, 0b)
    if (current() == '0') {
      addToBuffer('0');
      next();             // Consume '0'
      char p = current(); // Use current() after consuming '0'
      if (p == 'x' || p == 'X') {
        base = HEX;
        addToBuffer(p);
        next();                            // Consume x/X
        if (!isAsciiHexDigit(current())) { // Check must follow prefix
          throw LexError::incompleteInt(iter.getLocation());
        }
      } else if (p == 'b' || p == 'B') {
        base = BIN;
        addToBuffer(p);
        next();                            // Consume b/B
        if (!isAsciiBinDigit(current())) { // Check must follow prefix
          throw LexError::incompleteInt(iter.getLocation());
        }
      }
      // If just '0' followed by non-prefix, non-digit/non-dot/non-exp -> treat
      // as single 0
      else if (!isAsciiDecDigit(p) && p != '.' && p != 'e' && p != 'E') {
        num_buffer[buffer_idx] = '\0'; // Null-terminate
        return Token::makeIntLit(0, DEC, Span(start_pos, iter.position()));
      }
      // Allow '0' followed by octal digits if octal is supported (not
      // currently) else if (p >= '0' && p <= '7') { /* handle octal */ }

      // Otherwise, continue parsing (might be 0.1, 0e1, or just 0123 in DEC)
    }

    // Parse the main part of the number (integer part for floats)
    // Handles first digit if not '0' prefix case
    while (isDigitOrUnderscore(current(), base)) {
      // Need to add the first digit if it wasn't '0'
      if (buffer_idx == 0 &&
          start_pos ==
              iter.position() - 1) { // Add first digit if not already added
        addToBuffer(source_code_ptr[start_pos]);
      }

      if (current() != '_') {
        addToBuffer(current());
      }
      next();
    }
    // Ensure first digit was added if loop didn't run (e.g., single digit
    // number)
    if (buffer_idx == 0 && start_pos == iter.position() - 1) {
      addToBuffer(source_code_ptr[start_pos]);
    }

    // Check for float components (only if base is DEC)
    if (base == DEC) {
      // Check for decimal point '.' followed by a digit
      Location dot_loc = iter.getLocation();
      if (current() == '.' &&
          isAsciiDecDigit(peek())) // Must have digit after '.'
      {
        is_float = true;
        addToBuffer('.'); // Add the decimal point
        next();           // Consume .

        // Parse fractional part
        while (isDigitOrUnderscore(current(), DEC)) {
          if (current() != '_') {
            addToBuffer(current());
          }
          next();
        }
      }
      // If only ".", treat number as integer and '.' as separate token (handled
      // by nextToken)
      else if (current() == '.') {
        // Stop number parsing here. Let '.' be handled by nextToken.
      }

      // Check for exponent e/E
      Location exp_loc = iter.getLocation();
      if (current() == 'e' || current() == 'E') {
        char exp_peek = peek();
        char exp_peek2 = peek2(); // Need peek2 from iterator

        // Valid exponent part requires digit, or sign followed by digit
        if (isAsciiDecDigit(exp_peek) ||
            ((exp_peek == '+' || exp_peek == '-') &&
             isAsciiDecDigit(exp_peek2))) {
          is_float = true; // A number with exponent is always float
          has_exponent = true;
          addToBuffer(current()); // Add 'e' or 'E'
          next();                 // Consume e/E

          // Add exponent sign if present
          if (current() == '+' || current() == '-') {
            addToBuffer(current());
            next();
          }

          // Must have at least one digit in exponent (already checked by outer
          // condition) Parse exponent digits
          while (isDigitOrUnderscore(current(), DEC)) {
            if (current() != '_') {
              addToBuffer(current());
            }
            next();
          }
        }
        // else: 'e'/'E' not followed by valid exponent chars, stop number
        // parsing here. Let 'e' be handled as part of an identifier later if
        // applicable.
      }
    }
    // Cannot have float parts ('.', 'e', 'E') for hex/bin
    else if (current() == '.' || current() == 'e' || current() == 'E') {
      throw LexError::invalidChar(
          iter.getLocation()); // e.g., 0xFF.0 is invalid
    }

    // Null-terminate the buffer
    num_buffer[buffer_idx] = '\0';

    // Handle cases where only prefix was consumed (e.g., "0x" then EOF)
    // Need to check if any digits were actually added after prefix
    const char *digits_start = num_buffer;
    if (base == HEX && buffer_idx <= 2) { // Only "0x"
      throw LexError::incompleteInt(iter.getLocation());
    }
    if (base == BIN && buffer_idx <= 2) { // Only "0b"
      throw LexError::incompleteInt(iter.getLocation());
    }
    // Point digits_start after prefix for conversion functions
    if (base == HEX || base == BIN) {
      digits_start += 2; // Skip "0x" or "0b"
    }

    // Convert the accumulated digits string
    size_t end_pos = iter.position();
    Span number_span(start_pos, end_pos);
    Location end_loc =
        iter.getLocation(); // Use location at end for range errors

    if (is_float) {
      errno = 0;
      char *endptr;
      // Use the full buffer content for strtod
      double value = strtod(num_buffer, &endptr);

      if (errno == ERANGE) {
        throw LexError::floatOutOfRange(end_loc);
      }
      // Check if conversion consumed the part we expected
      if (endptr != num_buffer + buffer_idx) {
        // This might happen if invalid chars snuck in or buffer overflowed.
        // Or if strtod stopped early unexpectedly.
        throw LexError::invalidFloat(
            start_loc); // Error likely relates to start/format
      }

      return Token::makeFloatLit(value, has_exponent, number_span);
    } else // It's an integer
    {
      errno = 0;
      char *endptr;
      int strtoll_base = (base == HEX) ? 16 : (base == BIN) ? 2 : 10;

      // Handle empty digits_start case (e.g. "0x" where check above failed
      // somehow)
      if (*digits_start == '\0') {
        throw LexError::incompleteInt(start_loc);
      }

      // Use digits_start (skips 0x/0b if present)
      long long value = strtoll(digits_start, &endptr, strtoll_base);

      if (errno == ERANGE) {
        throw LexError::intOutOfRange(end_loc);
      }

      // Check if conversion consumed the entire digits part we expected
      // Calculate expected end pointer within the original buffer
      const char *expected_end_in_buffer = num_buffer + buffer_idx;
      if (endptr != expected_end_in_buffer) {
        // Incomplete conversion likely means invalid chars (e.g., "0xFG")
        // Or buffer overflow truncated valid digits?
        // Use location where parsing stopped if possible, else start_loc.
        // Finding exact failure point post-strtoll is hard. Use start_loc.
        throw LexError::incompleteInt(start_loc);
      }

      return Token::makeIntLit(value, base, number_span);
    }
  }

  // Lexes a short flag (e.g., -v, -f)
  // Assumes called when current() is the char *after* '-'
  Token lexShortFlag(size_t start_pos) // start_pos is the position of '-'
  {
    Location flag_char_loc = iter.getLocation(); // Loc of char after '-'
    size_t content_start_pos = iter.position();  // Position of char after '-'

    // Read the flag content (alphanumeric sequence)
    // Short flags are typically single char, but allow sequence for flexibility
    // Adjust isIdentCont / isAsciiDecDigit if flags have different rules
    while (isIdentCont(current()) || isAsciiDecDigit(current())) {
      next();
    }
    size_t end_pos = iter.position();
    size_t length = end_pos - content_start_pos;

    if (length == 0) {
      // This means '-' was followed by whitespace or EOF or symbol
      // Treat as just TOK_MINUS, need to backtrack or handle in nextToken
      // Revisit nextToken logic: it should return TOK_MINUS if '-' not followed
      // by flag char For now, assume if we reach here, length > 0. If this
      // error occurs, the logic in nextToken needs adjustment.
      throw std::runtime_error(
          "Internal lexer error: lexShortFlag called incorrectly"); // Should
                                                                    // not
                                                                    // happen
    }

    const char *flag_start_ptr = source_code_ptr + content_start_pos;
    // Use makeFlag (which currently creates TOK_ID)
    // The span should include the leading '-'
    return Token::makeShortFlag(flag_start_ptr, length,
                                Span(start_pos, end_pos));
  }

  // Lexes a long flag (e.g., --version, --file)
  // Assumes called when current() is the char *after* '--'
  Token lexLongFlag(size_t start_pos) // start_pos is the position of first '-'
  {
    Location name_start_loc = iter.getLocation(); // Loc of char after '--'
    size_t name_start_pos = iter.position();      // Position of char after '--'

    // Read the identifier part of the flag name
    // Allow '-' within long flag names? e.g. --my-flag
    // Current isIdentCont doesn't include '-', adjust if needed.
    while (isIdentCont(current())) { // Add || current() == '-' if desired
      next();
    }
    size_t name_end_pos = iter.position();
    size_t name_length = name_end_pos - name_start_pos;

    if (name_length == 0) {
      // Should not happen if called correctly from nextToken (checked for
      // isIdentStart)
      throw LexError::invalidChar(name_start_loc); // Error started after '--'
    }

    const char *name_start_ptr = source_code_ptr + name_start_pos;

    // Check for =value attached?
    // Current design: Lexer returns only the flag name token.
    // Parser handles the '=' and subsequent value token if present.
    // So, just return the flag token based on the name.
    // The span should include the leading '--'
    return Token::makeLongFlag(name_start_ptr, name_length,
                               Span(start_pos, name_end_pos));
  }

  // Character navigation helpers (inline wrappers around iterator)
  // Could be made inline if compiler doesn't already do it.
  char current() const { return iter.current(); }
  char peek() const { return iter.peek(); }
  char peek2() const { return iter.peek2(); } // Use iterator's peek2

  void next() { iter.next(); }
  void next2() { iter.next2(); }

  // State
  SourceIterator iter;         // Iterator over the source code
  const char *source_code_ptr; // Pointer to start of source data
  std::vector<Token> tokens;   // Vector to store the generated tokens
};

} // namespace lexer

#endif // LEXER_HPP