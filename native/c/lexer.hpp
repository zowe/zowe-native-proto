#ifndef LEXER_HPP
#define LEXER_HPP

// Compatibility for shared_ptr and enable_shared_from_this for compilers w/
// separate TR1 folder (C++11 Tech Report 1 standard)
#if defined(__has_include)
#if __has_include(<memory>)
#include <memory>
#elif __has_include(<tr1/memory>)
#include <tr1/memory>
namespace std {
typedef tr1::shared_ptr shared_ptr;
typedef tr1::enable_shared_from_this enable_shared_from_this;
} // namespace std
#endif
#else
// Fallback for specialized compilers w/ standard headers but separation via tr1
// namespace
#if defined(__IBMCPP_TR1__)
#include <memory>
#else
#include <tr1/memory>
namespace std {
using tr1::enable_shared_from_this;
using tr1::shared_ptr;
} // namespace std
#endif
#endif

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
 * represents a location within a piece of input or source.
 * includes the filename, line, and column.
 */
class Location {
public:
  Location() : m_filename(""), m_line(1), m_col(1) {}
  Location(const std::string &filename, size_t line, size_t col)
      : m_filename(filename), m_line(line), m_col(col) {}

  const std::string &get_filename() const { return m_filename; }
  size_t get_line() const { return m_line; }
  size_t get_col() const { return m_col; }

  void print(std::ostream &os) const {
    os << (m_filename.empty() ? "<string>" : m_filename) << " (" << m_line
       << ":" << m_col << ")";
  }

  std::string m_filename;
  size_t m_line;
  size_t m_col;
};

inline std::ostream &operator<<(std::ostream &os, const Location &loc) {
  loc.print(os);
  return os;
}

/**
 * iterator over an input's characters.
 * keeps track of line and column and can produce a location.
 */
class InputIter {
public:
  InputIter(const std::string &filename, const std::vector<char> &code)
      : m_input(code), m_filename(filename), m_line(1), m_col(1), m_pos(0) {}

  char current() const {
    if (m_pos < m_input.size()) {
      return m_input[m_pos];
    }
    return '\0';
  }

  char peek() const {
    if (m_pos + 1 < m_input.size()) {
      return m_input[m_pos + 1];
    }
    return '\0';
  }

  char peek2() const {
    if (m_pos + 2 < m_input.size()) {
      return m_input[m_pos + 2];
    }
    return '\0';
  }

  void next() {
    if (m_pos < m_input.size()) {
      if (m_input[m_pos] == '\n') {
        m_line++;
        m_col = 1;
      } else if (m_input[m_pos] == '\t') {
        m_col += 4;
      } else {
        m_col++;
      }
      m_pos++;
    }
  }

  void next2() {
    next();
    next();
  }

  bool has_more() const { return m_pos < m_input.size(); }
  size_t position() const { return m_pos; }
  Location get_location() const { return Location(m_filename, m_line, m_col); }
  size_t get_line() const { return m_line; }
  size_t get_col() const { return m_col; }
  const std::string &get_filename() const { return m_filename; }

private:
  const std::vector<char> &m_input;
  std::string m_filename;
  size_t m_line;
  size_t m_col;
  size_t m_pos;
};

/**
 * represents a piece of input or source code.
 * includes the filename (if applicable) and the raw data.
 */
class Src {
public:
  Src() {}

  static Src from_file(const std::string &filename) {
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error("could not open file: " + filename);
    }

    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    if (size < 0) {
      throw std::runtime_error("error determining size of file: " + filename);
    }
    std::vector<char> buffer(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);

    if (size > 0) {
      file.read(&buffer[0], size);
      if (!file && !file.eof()) {
        throw std::runtime_error("error reading file: " + filename);
      }
    }

    return Src(filename, buffer);
  }

  static Src from_string(const std::string &code_str,
                         const std::string &filename = "<string>") {
    std::vector<char> chars(code_str.begin(), code_str.end());
    return Src(filename, chars);
  }

  const std::string &get_filename() const { return m_filename; }
  const std::vector<char> &get_code() const { return m_input; }

  InputIter get_iterator() const { return InputIter(m_filename, m_input); }

  const char *get_code_ptr() const {
    return m_input.empty() ? nullptr : &m_input[0];
  }

private:
  Src(const std::string &filename, const std::vector<char> &code)
      : m_filename(filename), m_input(code) {}

  std::string m_filename;
  std::vector<char> m_input;
};

enum LexErrorKind {
  InvalidChar,
  UnclosedString,
  UnknownEscape,
  IntOutOfRange,
  IncompleteInt,
  FloatOutOfRange,
  InvalidFloat
};

class LexError : public std::exception {
public:
  LexError(const Location &loc, LexErrorKind kind) : m_loc(loc), m_kind(kind) {}

  static LexError invalid_char(const Location &loc) {
    return LexError(loc, LexErrorKind::InvalidChar);
  }
  static LexError unclosed_string(const Location &loc) {
    return LexError(loc, LexErrorKind::UnclosedString);
  }
  static LexError unknown_escape(const Location &loc) {
    return LexError(loc, LexErrorKind::UnknownEscape);
  }
  static LexError int_out_of_range(const Location &loc) {
    return LexError(loc, LexErrorKind::IntOutOfRange);
  }
  static LexError incomplete_int(const Location &loc) {
    return LexError(loc, LexErrorKind::IncompleteInt);
  }
  static LexError float_out_of_range(const Location &loc) {
    return LexError(loc, LexErrorKind::FloatOutOfRange);
  }
  static LexError invalid_float(const Location &loc) {
    return LexError(loc, LexErrorKind::InvalidFloat);
  }

  const Location &get_location() const { return m_loc; }
  LexErrorKind get_kind() const { return m_kind; }

  virtual const char *what() const throw() {
    if (m_msg.empty()) {
      m_msg = to_string();
    }
    return m_msg.c_str();
  }

  std::string to_string() const {
    std::ostringstream ss;
    ss << m_loc << ": ";

    switch (m_kind) {
    case LexErrorKind::InvalidChar:
      ss << "invalid character";
      break;
    case LexErrorKind::UnclosedString:
      ss << "unclosed string literal";
      break;
    case LexErrorKind::UnknownEscape:
      ss << "unknown escape character";
      break;
    case LexErrorKind::IntOutOfRange:
      ss << "integer literal out of 64-bit range";
      break;
    case LexErrorKind::IncompleteInt:
      ss << "incomplete integer literal";
      break;
    case LexErrorKind::FloatOutOfRange:
      ss << "floating-point literal out of range";
      break;
    case LexErrorKind::InvalidFloat:
      ss << "invalid floating-point literal";
      break;
    default:
      ss << "unknown lexer error";
      break;
    }

    return ss.str();
  }

private:
  Location m_loc;
  LexErrorKind m_kind;
  mutable std::string m_msg;
};

enum Radix { Dec = 10, Bin = 2, Hex = 16 };

enum TokenKind {
  TokEof,

  TokIf,
  TokElse,
  TokFor,
  TokIn,
  TokWhile,
  TokBreak,
  TokReturn,
  TokInt,
  TokBool,
  TokString,
  TokAnd,
  TokOr,
  TokNot,
  TokTrue,
  TokFalse,

  TokAssign,
  TokPlus,
  TokMinus,
  TokDoubleMinus,
  TokTimes,
  TokDivide,
  TokModulo,
  TokShl,
  TokShr,
  TokLess,
  TokGreater,
  TokLessEq,
  TokGreaterEq,
  TokEq,
  TokNotEq,

  TokLParen,
  TokRParen,
  TokLBrace,
  TokRBrace,
  TokLBracket,
  TokRBracket,
  TokSemi,
  TokColon,
  TokComma,
  TokDot,

  TokId,
  TokIntLit,
  TokFloatLit,
  TokStrLit,
  TokFlagShort, // e.g., -f
  TokFlagLong   // e.g., --force
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

  std::string to_string() const {
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
  struct IntLit {
    long long value;
    Radix base;
  } int_lit;

  struct FloatLit {
    double value;
    bool has_exponent;
  } float_lit;

  StringRef string_ref;

  TokenData() : string_ref() {}
  ~TokenData() {}
};

// no-alloc class to represent lexical tokens
class Token {
public:
  Token() : m_kind(TokEof), m_span() {}

  // constructor for simple tokens
  Token(TokenKind kind, const Span &span) : m_kind(kind), m_span(span) {
    m_data.int_lit.value = 0;
    m_data.int_lit.base = Dec;
  }

  Token(const Token &other) : m_kind(other.m_kind), m_span(other.m_span) {
    copy_data(other);
  }

  Token &operator=(const Token &other) {
    if (this != &other) {
      m_kind = other.m_kind;
      m_span = other.m_span;
      copy_data(other);
    }
    return *this;
  }

  // todo: may be unneeded
  ~Token() {}

  // static factory methods for complex tokens below

  static Token make_id(const char *start_ptr, size_t len, const Span &span) {
    Token token(TokId, span);
    token.m_data.string_ref.start = start_ptr;
    token.m_data.string_ref.length = len;
    return token;
  }

  static Token make_short_flag(const char *name_ptr, size_t len,
                               const Span &span) {
    Token token(TokFlagShort, span);
    token.m_data.string_ref.start = name_ptr;
    token.m_data.string_ref.length = len;
    return token;
  }

  static Token make_long_flag(const char *name_ptr, size_t len,
                              const Span &span) {
    Token token(TokFlagLong, span);
    token.m_data.string_ref.start = name_ptr;
    token.m_data.string_ref.length = len;
    return token;
  }

  static Token make_int_lit(long long value, Radix base, const Span &span) {
    Token token(TokIntLit, span);
    token.m_data.int_lit.value = value;
    token.m_data.int_lit.base = base;
    return token;
  }

  static Token make_float_lit(double value, bool has_exponent,
                              const Span &span) {
    Token token(TokFloatLit, span);
    token.m_data.float_lit.value = value;
    token.m_data.float_lit.has_exponent = has_exponent;
    return token;
  }

  // stores raw string content (pointer between quotes, length).
  // escape sequences are processed on demand by get_str_lit_value().
  static Token make_str_lit(const char *content_start_ptr, size_t content_len,
                            const Span &span) {
    Token token(TokStrLit, span);
    token.m_data.string_ref.start = content_start_ptr;
    token.m_data.string_ref.length = content_len;
    return token;
  }

  TokenKind get_kind() const { return m_kind; }
  const Span &get_span() const { return m_span; }

  // getter methods for complex token data below

  // identifier/flag content (raw pointer and length)
  const char *get_string_ref_start() const {
    // check for kinds that use StringRef
    if (m_kind == TokId || m_kind == TokStrLit || m_kind == TokFlagShort ||
        m_kind == TokFlagLong) {
      return m_data.string_ref.start;
    }
    return nullptr;
  }

  size_t get_string_ref_length() const {
    // check for kinds that use StringRef
    if (m_kind == TokId || m_kind == TokStrLit || m_kind == TokFlagShort ||
        m_kind == TokFlagLong) {
      return m_data.string_ref.length;
    }
    return 0u;
  }

  // helper to get identifier/flag value as std::string (allocates)
  std::string get_id_value() const {
    // note: this returns the name of the flag, not including dashes.
    if (m_kind == TokId || m_kind == TokFlagShort || m_kind == TokFlagLong) {
      if (m_data.string_ref.start) {
        return std::string(m_data.string_ref.start, m_data.string_ref.length);
      } else {
        return "";
      }
    }
    // todo: consider throwing or returning a sentinel
    throw std::runtime_error("token is not an identifier or flag");
  }

  long long get_int_value() const {
    if (m_kind == TokIntLit) {
      return m_data.int_lit.value;
    }
    // todo: consider throwing or returning a sentinel
    throw std::runtime_error("token is not an integer literal");
  }

  Radix get_int_base() const {
    if (m_kind == TokIntLit) {
      return m_data.int_lit.base;
    }
    // todo: consider throwing or returning a sentinel
    throw std::runtime_error("token is not an integer literal");
  }

  double get_float_value() const {
    if (m_kind == TokFloatLit) {
      return m_data.float_lit.value;
    }
    // todo: consider throwing or returning a sentinel
    throw std::runtime_error("token is not a float literal");
  }

  bool has_float_exponent() const {
    if (m_kind == TokFloatLit) {
      return m_data.float_lit.has_exponent;
    }
    // todo: consider throwing or returning a sentinel
    throw std::runtime_error("token is not a float literal");
  }

  // get a string literal value (processes escapes, allocates a new std::string)
  std::string get_str_lit_value() const {
    if (m_kind != TokStrLit) {
      throw std::runtime_error("token is not a string literal");
    }
    if (!m_data.string_ref.start)
      return ""; // should not happen if created correctly

    std::string processed_value;
    // estimate capacity, might overestimate if many escapes
    processed_value.reserve(m_data.string_ref.length);
    const char *ptr = m_data.string_ref.start;
    const char *end = ptr + m_data.string_ref.length;

    while (ptr < end) {
      if (*ptr == '\\') {
        ptr++; // consume backslash
        if (ptr >= end) {
          // this indicates an error caught by lexer or malformed raw slice
          // for robustness, stop processing here and add a literal backslash
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
        // add other escapes if supported by the language
        default:
          // unknown escape was already checked by lexer.
          // if it got here, maybe append the literal char after backslash?
          processed_value += *ptr;
          break;
        }
      } else {
        // should not encounter unescaped double quotes here as they delimit the
        // raw slice. if found, it's an internal error. add it literally for
        // now.
        processed_value += *ptr;
      }
      ptr++;
    }
    return processed_value;
  }

  // print token to stream
  void print(std::ostream &os) const {
    // cached map for keyword/symbol lookup
    static std::map<TokenKind, std::string> simple_tokens;
    if (simple_tokens.empty()) { // initialize on first call
      simple_tokens[TokEof] = "<eof>";
      simple_tokens[TokIf] = "if";
      simple_tokens[TokElse] = "else";
      simple_tokens[TokFor] = "for";
      simple_tokens[TokIn] = "in";
      simple_tokens[TokWhile] = "while";
      simple_tokens[TokBreak] = "break";
      simple_tokens[TokReturn] = "return";
      simple_tokens[TokInt] = "int";
      simple_tokens[TokBool] = "bool";
      simple_tokens[TokString] = "string";
      simple_tokens[TokAnd] = "and";
      simple_tokens[TokOr] = "or";
      simple_tokens[TokNot] = "not";
      simple_tokens[TokTrue] = "true";
      simple_tokens[TokFalse] = "false";
      simple_tokens[TokAssign] = "=";
      simple_tokens[TokPlus] = "+";
      simple_tokens[TokMinus] = "-";
      simple_tokens[TokDoubleMinus] = "--";
      simple_tokens[TokTimes] = "*";
      simple_tokens[TokDivide] = "/";
      simple_tokens[TokModulo] = "%";
      simple_tokens[TokShl] = "<<";
      simple_tokens[TokShr] = ">>";
      simple_tokens[TokLess] = "<";
      simple_tokens[TokGreater] = ">";
      simple_tokens[TokLessEq] = "<=";
      simple_tokens[TokGreaterEq] = ">=";
      simple_tokens[TokEq] = "==";
      simple_tokens[TokNotEq] = "!=";
      simple_tokens[TokLParen] = "(";
      simple_tokens[TokRParen] = ")";
      simple_tokens[TokLBrace] = "{";
      simple_tokens[TokRBrace] = "}";
      simple_tokens[TokLBracket] = "[";
      simple_tokens[TokRBracket] = "]";
      simple_tokens[TokSemi] = ";";
      simple_tokens[TokColon] = ":";
      simple_tokens[TokComma] = ",";
      simple_tokens[TokDot] = ".";
    }

    // handle flag tokens
    if (m_kind == TokFlagShort) {
      os << "-";
      if (m_data.string_ref.start)
        os.write(m_data.string_ref.start, m_data.string_ref.length);
      else
        os << "<null_short_flag>";
      return;
    }
    if (m_kind == TokFlagLong) {
      os << "--";
      if (m_data.string_ref.start)
        os.write(m_data.string_ref.start, m_data.string_ref.length);
      else
        os << "<null_long_flag>";
      return;
    }

    // handle simple tokens first using the map
    auto it = simple_tokens.find(m_kind);
    if (it != simple_tokens.end()) {
      os << it->second;
      return;
    }

    // handle complex tokens
    switch (m_kind) {
    case TokId: // includes flags if using TokId for them
      if (m_data.string_ref.start) {
        os.write(m_data.string_ref.start, m_data.string_ref.length);
      } else {
        os << "<null_id>"; // should not happen
      }
      break;
    case TokStrLit:
      os << "\"";
      // print raw content, processing escapes for display purposes
      if (m_data.string_ref.start) {
        const char *ptr = m_data.string_ref.start;
        const char *end = ptr + m_data.string_ref.length;
        while (ptr < end) {
          char c = *ptr;
          if (c == '\\') {
            ptr++; // look at the escaped char
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
                break; // show the escaped quote
              case '0':
                os << "\\0";
                break;
              default: // print unknown escapes literally for representation
                // check if printable, otherwise use hex?
                if (std::isprint(static_cast<unsigned char>(*ptr))) {
                  os << '\\' << *ptr;
                } else {
                  // simple fallback: print original backslash only
                  os << '\\';
                  // decrement ptr so the non-printable char is handled below
                  ptr--;
                }
                break;
              }
            } else {
              os << '\\'; // dangling backslash at end of content
            }
          } else if (c == '"') { // should not happen in raw content, but escape
                                 // if found
            os << "\\\"";
          } else if (c == '\n') { // represent newline visually
            os << "\\n";
          } else if (c == '\r') {
            os << "\\r";
          } else if (c == '\t') {
            os << "\\t";
          } else if (std::isprint(static_cast<unsigned char>(c))) {
            os << c; // print printable chars directly
          } else {
            // non-printable chars, print unicode replacement character
            os << "\ufffd";
          }
          ptr++;
        }
      }
      os << "\"";
      break;
    case TokIntLit:
      // ... (integer printing logic remains the same) ...
      if (m_data.int_lit.base == Hex) {
        std::ios_base::fmtflags original_flags = os.flags();
        os << "0x" << std::hex << m_data.int_lit.value;
        os.flags(original_flags);
      } else if (m_data.int_lit.base == Bin) {
        os << "0b";
        if (m_data.int_lit.value == 0) {
          os << "0";
        } else {
          // simple binary printing for positive numbers up to 63 bits
          bool leading_zeros = true;
          for (int i = 63; i >= 0; --i) {
            if ((m_data.int_lit.value >> i) & 1) {
              leading_zeros = false;
              os << '1';
            } else if (!leading_zeros) {
              os << '0';
            }
          }
          if (leading_zeros)
            os << '0'; // should only happen for value 0, handled above
        }
      } else // dec
      {
        os << m_data.int_lit.value;
      }
      break;
    case TokFloatLit: {
      std::ios_base::fmtflags original_flags = os.flags();
      std::streamsize original_precision = os.precision();

      // preserve default float format unless scientific notation was used
      if (m_data.float_lit.has_exponent) {
        os.setf(std::ios::scientific, std::ios::floatfield);
      }
      // else: use default formatting (often avoids trailing zeros vs fixed)

      os << m_data.float_lit.value;

      os.flags(original_flags);
      os.precision(original_precision);
    } break;
    default: // note: should not happen if all kinds are covered
      os << "<unknown token_kind: " << static_cast<int>(m_kind) << ">";
      break;
    }
  }

private:
  // helper to copy data based on kind (used by copy ctor and assignment)
  // this should be safe now, as it relies on the (trivial) copy/assignment
  // of the member structs (IntLit, FloatLit, StringRef).
  void copy_data(const Token &other) {
    switch (other.m_kind) {
    case TokId:
    case TokStrLit:
    case TokFlagShort:
    case TokFlagLong:
      // assign StringRef (member-wise copy)
      m_data.string_ref = other.m_data.string_ref;
      break;
    case TokIntLit:
      // assign IntLit struct (member-wise copy)
      m_data.int_lit = other.m_data.int_lit;
      break;
    case TokFloatLit:
      // assign FloatLit struct (member-wise copy)
      m_data.float_lit = other.m_data.float_lit;
      break;
    default:
      // for simple tokens or TokEof, the data doesn't strictly matter,
      // but copying something ensures defined state
      m_data.int_lit = other.m_data.int_lit;
      break;
    }
  }

  TokenKind m_kind;
  Span m_span;
  TokenData m_data; // union holding complex data (value types or StringRef)
};

// stream operator for token
inline std::ostream &operator<<(std::ostream &os, const Token &token) {
  token.print(os);
  return os;
}

// facility for tokenizing source code or input strings
class Lexer {
public:
  /**
   * primary static function to tokenize source (input, code, etc.).
   * takes a source object and returns a vector of tokens.
   * note: source object must remain valid while tokens are used.
   */
  static std::vector<Token> tokenize(const Src &source) {
    Lexer lexer(source);
    lexer.lex_all();
    return lexer.m_tokens;
  }

private:
  // private constructor - only used internally by the static tokenize method
  Lexer(const Src &source)
      : m_iter(source.get_iterator()), m_input_ptr(source.get_code_ptr()) {
    size_t estimated_tokens = source.get_code().size() / 5;
    if (estimated_tokens > 10) { // avoid tiny allocations
      m_tokens.reserve(estimated_tokens);
    }
  }

  // helper functions for character classification
  // todo: using direct checks can be slightly faster if locale is not a
  // concern..
  static bool is_ascii_dec_digit(char c) { return c >= '0' && c <= '9'; }
  static bool is_ascii_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'a' && c <= 'f');
  }
  static bool is_ascii_bin_digit(char c) { return c == '0' || c == '1'; }
  // using std::isalpha for broader identifier start definition (factors in
  // locale) sticking with std::isalpha unless pure ascii is
  // guaranteed/required.
  static bool is_ident_start(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '$' ||
           c == '_' || c == '/';
  }
  static bool is_ident_cont(char c) {
    return is_ident_start(c) || is_ascii_dec_digit(c) || c == '.' || c == '/';
  }

  // combined digit check for lex_number
  static bool is_digit_or_underscore(char c, Radix base) {
    if (c == '_')
      return true;
    switch (base) {
    case Dec:
      return is_ascii_dec_digit(c);
    case Hex:
      return is_ascii_hex_digit(c);
    case Bin:
      return is_ascii_bin_digit(c);
    default:
      return false;
    }
  }

  // core lexing driver function
  void lex_all() {
    m_tokens.clear(); // clear if lexer object was reused
    while (true) {
      eat_whitespace_and_comments();
      Token token = next_token();
      m_tokens.push_back(token);
      if (token.get_kind() == TokEof) {
        break;
      }
    }
  }

  // skips over whitespace and single-line comments
  void eat_whitespace_and_comments() {
    while (true) {
      char c = current(); // cache current char
      switch (c) {
      // whitespace
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        next();
        break;

      // comments (single line //)
      case '/':
        if (peek() == '/') {
          next2(); // consume //
          while (true) {
            char comm_c = current();
            if (comm_c == '\n' || comm_c == '\0')
              break;
            next();
          }
        } else {
          // not a comment, maybe division
          return;
        }
        break;

      // end of whitespace/comment section
      default:
        return;
      }
    }
  }

  // lexes the next token from the input stream
  Token next_token() {
    size_t start_pos = m_iter.position();
    Location start_loc = m_iter.get_location(); // get location at start

    char c = current();
    switch (c) {
    // eof
    case '\0':
      return Token(TokEof, Span(start_pos, start_pos));

    // single-character tokens that are unambiguous
    case '+':
      next();
      return Token(TokPlus, Span(start_pos, m_iter.position()));
    case '*':
      next();
      return Token(TokTimes, Span(start_pos, m_iter.position()));
    case '%':
      next();
      return Token(TokModulo, Span(start_pos, m_iter.position()));
    case '(':
      next();
      return Token(TokLParen, Span(start_pos, m_iter.position()));
    case ')':
      next();
      return Token(TokRParen, Span(start_pos, m_iter.position()));
    case '{':
      next();
      return Token(TokLBrace, Span(start_pos, m_iter.position()));
    case '}':
      next();
      return Token(TokRBrace, Span(start_pos, m_iter.position()));
    case '[':
      next();
      return Token(TokLBracket, Span(start_pos, m_iter.position()));
    case ']':
      next();
      return Token(TokRBracket, Span(start_pos, m_iter.position()));
    case ';':
      next();
      return Token(TokSemi, Span(start_pos, m_iter.position()));
    case ':':
      next();
      return Token(TokColon, Span(start_pos, m_iter.position()));
    case ',':
      next();
      return Token(TokComma, Span(start_pos, m_iter.position()));

    // potentially multi-character tokens
    case '-':
      next();               // consume '-'
      if (current() == '-') // check for second '-'
      {
        next(); // consume second '-'
                // check if it's followed by identifier start (long flag)
        if (is_ident_start(current())) {
          // current position is *after* '--'. start pos was before first '-'.
          return lex_long_flag(start_pos); // pass original start pos
        }
        // check if just '--' (often end of options)
        else if (!is_ident_cont(current()) && !is_ascii_dec_digit(current())) {
          return Token(TokDoubleMinus, Span(start_pos, m_iter.position()));
        } else {
          // case like "--1" or "--_" ? treat as error? or allow as part of long
          // flag? current lex_long_flag expects ident start. let's treat as
          // error here.
          throw LexError::invalid_char(
              Location(start_loc.m_filename, start_loc.m_line,
                       start_loc.m_col + 2)); // error after '--'
        }
      }
      // check if it's followed by identifier or digit (short flag)
      else if (is_ident_start(current()) || is_ascii_dec_digit(current())) {
        // current position is after '-'. start pos was before '-'.
        return lex_short_flag(start_pos); // pass original start pos
      }
      // check for negative numbers (handled by lex_number starting with digit)
      // or just the minus operator
      else {
        return Token(TokMinus, Span(start_pos, m_iter.position()));
      }

    case '/': // division, start of comment, or start of path-like identifier
      // comments are handled by eat_whitespace_and_comments
      // check if it starts a path-like identifier
      if (is_ident_cont(peek())) { // if '/' is followed by another identifier
                                   // character...
        return lex_identifier_or_keyword(
            start_pos); // ...treat it as the start of an identifier
      } else {
        // otherwise, it's the division operator
        next();
        return Token(TokDivide, Span(start_pos, m_iter.position()));
      }

    case '<':
      next();
      if (current() == '<') {
        next();
        return Token(TokShl, Span(start_pos, m_iter.position()));
      }
      if (current() == '=') {
        next();
        return Token(TokLessEq, Span(start_pos, m_iter.position()));
      }
      return Token(TokLess, Span(start_pos, m_iter.position()));

    case '>':
      next();
      if (current() == '>') {
        next();
        return Token(TokShr, Span(start_pos, m_iter.position()));
      }
      if (current() == '=') {
        next();
        return Token(TokGreaterEq, Span(start_pos, m_iter.position()));
      }
      return Token(TokGreater, Span(start_pos, m_iter.position()));

    case '=':
      next();
      if (current() == '=') {
        next();
        return Token(TokEq, Span(start_pos, m_iter.position()));
      }
      return Token(TokAssign, Span(start_pos, m_iter.position()));

    case '!':
      next();
      if (current() == '=') {
        next();
        return Token(TokNotEq, Span(start_pos, m_iter.position()));
      }
      // assume '!' is TokNot if not followed by '='
      return Token(TokNot, Span(start_pos, m_iter.position()));

    // string literals
    case '"':
      return lex_string(start_pos); // pass start position

    // numbers (integers or floats)
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
      return lex_number(start_pos); // pass start position

    // identifiers or keywords
    default:
      if (is_ident_start(c)) {
        return lex_identifier_or_keyword(start_pos); // pass start position
      }
      // unknown character
      throw LexError::invalid_char(start_loc); // use location at start of char
    }
  }

  // lexes an identifier or a keyword
  Token lex_identifier_or_keyword(size_t start_pos) {
    // start_pos is the position of the first character
    // current() is the first character
    next(); // consume the start character

    while (is_ident_cont(current())) {
      next();
    }
    size_t end_pos = m_iter.position();
    size_t length = end_pos - start_pos;
    const char *id_start_ptr = m_input_ptr + start_pos;

    // check if the identifier slice matches a keyword
    // keyword lookup: using std::map requires temporary string.
    // optimization: use a structure that can check char*/len directly.
    // simple linear scan for small keyword sets (pre-c++11 friendly):
    struct keyword {
      const char *name;
      size_t len;
      TokenKind kind;
    };
    static const keyword keywords[] = {
        {"if", 2, TokIf},         {"else", 4, TokElse},
        {"for", 3, TokFor},       {"in", 2, TokIn},
        {"while", 5, TokWhile},   {"break", 5, TokBreak},
        {"return", 6, TokReturn}, {"int", 3, TokInt},
        {"bool", 4, TokBool},     {"string", 6, TokString},
        {"and", 3, TokAnd},       {"or", 2, TokOr},
        {"not", 3, TokNot},       {"true", 4, TokTrue},
        {"false", 5, TokFalse},   {nullptr, 0, TokEof} // sentinel
    };

    for (int i = 0; keywords[i].name != nullptr; ++i) {
      if (keywords[i].len == length &&
          memcmp(id_start_ptr, keywords[i].name, length) == 0) {
        return Token(keywords[i].kind, Span(start_pos, end_pos));
      }
    }

    // not a keyword, it's an identifier
    return Token::make_id(id_start_ptr, length, Span(start_pos, end_pos));
  }

  // lexes a string literal enclosed in double quotes
  // returns token with raw content slice (pointer/length between quotes)
  Token lex_string(size_t span_start) {
    // span_start is position of opening quote "
    Location start_loc = m_iter.get_location();   // location of "
    next();                                       // consume the opening quote "
    size_t content_start_pos = m_iter.position(); // position after "

    while (true) {
      Location char_loc = m_iter.get_location(); // location of current char
      char c = current();
      if (c == '\0') {
        // error location: ideally point to the opening quote or where eof
        // encountered
        throw LexError::unclosed_string(start_loc);
      }
      if (c == '\n') {
        // strings cannot contain raw newlines (adjust if language allows)
        throw LexError::unclosed_string(char_loc); // error at the newline
      }
      if (c == '"') {
        break; // end of string content
      }

      if (c == '\\') {                               // escape sequence
        Location escape_loc = m_iter.get_location(); // location of backslash
        next();                                      // consume backslash
        char escaped_char = current();
        if (escaped_char == '\0' ||
            escaped_char == '\n') { // invalid state after backslash
          throw LexError::unclosed_string(escape_loc); // error at the backslash
        }
        // validate escape sequence based on language rules
        switch (escaped_char) {
        case 'n':
        case 'r':
        case 't':
        case '\\':
        case '"':
        case '0':
          // valid escapes in this language
          break;
        // add other valid escapes (e.g., \x_hh, \u_hhhh) if needed
        default:
          // unknown escape sequence
          throw LexError::unknown_escape(
              m_iter.get_location()); // error at char after backslash
        }
        next(); // consume the character after backslash
      } else {
        next(); // consume regular character
      }
    }

    size_t content_end_pos = m_iter.position(); // position of closing quote "
    next();                                     // consume closing quote "
    size_t span_end = m_iter.position();

    const char *content_start_ptr = m_input_ptr + content_start_pos;
    size_t content_length = content_end_pos - content_start_pos;

    return Token::make_str_lit(content_start_ptr, content_length,
                               Span(span_start, span_end));
  }

  // lexes a number (integer or float)
  Token lex_number(size_t start_pos) {
    // start_pos is position of first digit
    Location start_loc = m_iter.get_location(); // location of first digit
    Radix base = Dec;
    bool is_float = false;
    bool has_exponent = false;

    // use a temporary buffer for digits to pass to strtoll/strtod
    // avoids std::string allocation overhead in the common case.
    // max length: 64-bit int ~20 digits, double ~30 chars? add
    // prefixes/signs/etc.
    char num_buffer[64]; // should be sufficient for valid numbers
    int buffer_idx = 0;
    const int buffer_max_idx =
        sizeof(num_buffer) - 1; // leave space for null terminator

    // check for base prefixes (0x, 0b)
    if (current() == '0') {
      if (buffer_idx < buffer_max_idx) {
        num_buffer[buffer_idx++] = '0';
      }
      next();             // consume '0'
      char p = current(); // use current() after consuming '0'
      if (p == 'x' || p == 'x') {
        base = Hex;
        if (buffer_idx < buffer_max_idx) {
          num_buffer[buffer_idx++] = p;
        }
        next();                               // consume x/x
        if (!is_ascii_hex_digit(current())) { // check must follow prefix
          throw LexError::incomplete_int(m_iter.get_location());
        }
      } else if (p == 'b' || p == 'b') {
        base = Bin;
        if (buffer_idx < buffer_max_idx) {
          num_buffer[buffer_idx++] = p;
        }
        next();                               // consume b/b
        if (!is_ascii_bin_digit(current())) { // check must follow prefix
          throw LexError::incomplete_int(m_iter.get_location());
        }
      }
      // if just '0' followed by non-prefix, non-digit/non-dot/non-exp -> treat
      // as single 0
      else if (!is_ascii_dec_digit(p) && p != '.' && p != 'e' && p != 'e') {
        num_buffer[buffer_idx] = '\0'; // null-terminate
        return Token::make_int_lit(0, Dec, Span(start_pos, m_iter.position()));
      }
      // allow '0' followed by octal digits if octal is supported (not
      // currently) else if (p >= '0' && p <= '7') { /* handle octal */ }

      // otherwise, continue parsing (might be 0.1, 0e1, or just 0123 in dec)
    }

    // parse the main part of the number (integer part for floats)
    // handles first digit if not '0' prefix case
    while (is_digit_or_underscore(current(), base)) {
      // need to add the first digit if it wasn't '0'
      if (buffer_idx == 0 &&
          start_pos ==
              m_iter.position() - 1) { // add first digit if not already added
        if (buffer_idx < buffer_max_idx) {
          num_buffer[buffer_idx++] = m_input_ptr[start_pos];
        }
      }

      if (current() != '_') {
        if (buffer_idx < buffer_max_idx) {
          num_buffer[buffer_idx++] = current();
        }
      }
      next();
    }
    // ensure first digit was added if loop didn't run (e.g., single digit
    // number)
    if (buffer_idx == 0 && start_pos == m_iter.position() - 1) {
      if (buffer_idx < buffer_max_idx) {
        num_buffer[buffer_idx++] = m_input_ptr[start_pos];
      }
    }

    // check for float components (only if base is dec)
    if (base == Dec) {
      // check for decimal point '.' followed by a digit
      Location dot_loc = m_iter.get_location();
      if (current() == '.' &&
          is_ascii_dec_digit(peek())) // must have digit after '.'
      {
        is_float = true;
        if (buffer_idx < buffer_max_idx) {
          num_buffer[buffer_idx++] = '.';
        } // add the decimal point
        next(); // consume .

        // parse fractional part
        while (is_digit_or_underscore(current(), Dec)) {
          if (current() != '_') {
            if (buffer_idx < buffer_max_idx) {
              num_buffer[buffer_idx++] = current();
            }
          }
          next();
        }
      }
      // if only ".", treat number as integer and '.' as separate token (handled
      // by next_token)
      else if (current() == '.') {
        // stop number parsing here. let '.' be handled by next_token.
      }

      // check for exponent e/e
      Location exp_loc = m_iter.get_location();
      if (current() == 'e' || current() == 'e') {
        char exp_peek = peek();
        char exp_peek2 = peek2(); // need peek2 from iterator

        // valid exponent part requires digit, or sign followed by digit
        if (is_ascii_dec_digit(exp_peek) ||
            ((exp_peek == '+' || exp_peek == '-') &&
             is_ascii_dec_digit(exp_peek2))) {
          is_float = true; // a number with exponent is always float
          has_exponent = true;
          if (buffer_idx < buffer_max_idx) {
            num_buffer[buffer_idx++] = current();
          } // add 'e' or 'e'
          next(); // consume e/e

          // add exponent sign if present
          if (current() == '+' || current() == '-') {
            if (buffer_idx < buffer_max_idx) {
              num_buffer[buffer_idx++] = current();
            }
            next();
          }

          // must have at least one digit in exponent (already checked by outer
          // condition) parse exponent digits
          while (is_digit_or_underscore(current(), Dec)) {
            if (current() != '_') {
              if (buffer_idx < buffer_max_idx) {
                num_buffer[buffer_idx++] = current();
              }
            }
            next();
          }
        }
        // else: 'e'/'e' not followed by valid exponent chars, stop number
        // parsing here. let 'e' be handled as part of an identifier later if
        // applicable.
      }
    }
    // cannot have float parts ('.', 'e', 'e') for hex/bin
    else if (current() == '.' || current() == 'e' || current() == 'e') {
      throw LexError::invalid_char(
          m_iter.get_location()); // e.g., 0x_ff.0 is invalid
    }

    // null-terminate the buffer
    num_buffer[buffer_idx] = '\0';

    // handle cases where only prefix was consumed (e.g., "0x" then eof)
    // need to check if any digits were actually added after prefix
    const char *digits_start = num_buffer;
    if (base == Hex && buffer_idx <= 2) { // only "0x"
      throw LexError::incomplete_int(m_iter.get_location());
    }
    if (base == Bin && buffer_idx <= 2) { // only "0b"
      throw LexError::incomplete_int(m_iter.get_location());
    }
    // point digits_start after prefix for conversion functions
    if (base == Hex || base == Bin) {
      digits_start += 2; // skip "0x" or "0b"
    }

    // convert the accumulated digits string
    size_t end_pos = m_iter.position();
    Span number_span(start_pos, end_pos);
    Location end_loc =
        m_iter.get_location(); // use location at end for range errors

    if (is_float) {
      errno = 0;
      char *endptr;
      // use the full buffer content for strtod
      double value = strtod(num_buffer, &endptr);

      if (errno == ERANGE) {
        throw LexError::float_out_of_range(end_loc);
      }
      // check if conversion consumed the part we expected
      if (endptr != num_buffer + buffer_idx) {
        // this might happen if invalid chars snuck in or buffer overflowed.
        // or if strtod stopped early unexpectedly.
        throw LexError::invalid_float(
            start_loc); // error likely relates to start/format
      }

      return Token::make_float_lit(value, has_exponent, number_span);
    } else // it's an integer
    {
      errno = 0;
      char *endptr;
      int strtoll_base = (base == Hex) ? 16 : (base == Bin) ? 2 : 10;

      // handle empty digits_start case (e.g. "0x" where check above failed
      // somehow)
      if (*digits_start == '\0') {
        throw LexError::incomplete_int(start_loc);
      }

      // use digits_start (skips 0x/0b if present)
      long long value = strtoll(digits_start, &endptr, strtoll_base);

      if (errno == ERANGE) {
        throw LexError::int_out_of_range(end_loc);
      }

      // check if conversion consumed the entire digits part we expected
      // calculate expected end pointer within the original buffer
      const char *expected_end_in_buffer = num_buffer + buffer_idx;
      if (endptr != expected_end_in_buffer) {
        // incomplete conversion likely means invalid chars (e.g., "0x_fg")
        // or buffer overflow truncated valid digits?
        // use location where parsing stopped if possible, else start_loc.
        // finding exact failure point post-strtoll is hard. use start_loc.
        throw LexError::incomplete_int(start_loc);
      }

      return Token::make_int_lit(value, base, number_span);
    }
  }

  // lexes a short flag (e.g., -v, -f)
  // assumes called when current() is the char *after* '-'
  Token lex_short_flag(size_t start_pos) // start_pos is the position of '-'
  {
    Location flag_char_loc = m_iter.get_location(); // loc of char after '-'
    size_t content_start_pos = m_iter.position(); // position of char after '-'

    // read the flag content (alphanumeric sequence)
    // short flags are typically single char, but allow sequence for flexibility
    // adjust is_ident_cont / is_ascii_dec_digit if flags have different rules
    while (is_ident_cont(current()) || is_ascii_dec_digit(current())) {
      next();
    }
    size_t end_pos = m_iter.position();
    size_t length = end_pos - content_start_pos;

    if (length == 0) {
      // this means '-' was followed by whitespace or eof or symbol
      // treat as just TokMinus, need to backtrack or handle in next_token
      // revisit next_token logic: it should return TokMinus if '-' not followed
      // by flag char for now, assume if we reach here, length > 0. if this
      // error occurs, the logic in next_token needs adjustment.
      throw std::runtime_error(
          "internal lexer error: lex_short_flag called incorrectly"); // should
                                                                      // not
                                                                      // happen
    }

    const char *flag_start_ptr = m_input_ptr + content_start_pos;
    // use make_flag (which currently creates TokId)
    // the span should include the leading '-'
    return Token::make_short_flag(flag_start_ptr, length,
                                  Span(start_pos, end_pos));
  }

  // lexes a long flag (e.g., --version, --file)
  // assumes called when current() is the char *after* '--'
  Token
  lex_long_flag(size_t start_pos) // start_pos is the position of first '-'
  {
    Location name_start_loc = m_iter.get_location(); // loc of char after '--'
    size_t name_start_pos = m_iter.position(); // position of char after '--'

    // read the identifier part of the flag name
    // allow '-' within long flag names? e.g. --my-flag
    // current is_ident_cont doesn't include '-', adjust if needed.
    while (is_ident_cont(current())) { // add || current() == '-' if desired
      next();
    }
    size_t name_end_pos = m_iter.position();
    size_t name_length = name_end_pos - name_start_pos;

    if (name_length == 0) {
      // should not happen if called correctly from next_token (checked for
      // is_ident_start)
      throw LexError::invalid_char(name_start_loc); // error started after '--'
    }

    const char *name_start_ptr = m_input_ptr + name_start_pos;

    // check for =value attached?
    // current design: lexer returns only the flag name token.
    // parser handles the '=' and subsequent value token if present.
    // so, just return the flag token based on the name.
    // the span should include the leading '--'
    return Token::make_long_flag(name_start_ptr, name_length,
                                 Span(start_pos, name_end_pos));
  }

  // character navigation helpers (inline wrappers around iterator)
  // could be made inline if compiler doesn't already do it.
  char current() const { return m_iter.current(); }
  char peek() const { return m_iter.peek(); }
  char peek2() const { return m_iter.peek2(); } // use iterator's peek2

  void next() { m_iter.next(); }
  void next2() { m_iter.next2(); }

  // state
  InputIter m_iter;            // iterator over the input
  const char *m_input_ptr;     // pointer to start of the input
  std::vector<Token> m_tokens; // vector to store the generated tokens
};

} // namespace lexer

#endif // LEXER_HPP
