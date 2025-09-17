#ifndef EXTEND_IO_HPP
#define EXTEND_IO_HPP

#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>

typedef union
{
  bool b;
  long long i;
  double d;
  std::string *s;
  std::vector<std::string> *sv;
} ArgTypes;

struct Argument
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
  ArgTypes value;

  // default constructor
  Argument()
      : kind(ValueKind_None)
  {
    // initialize one member to avoid undefined state, though not strictly
    // necessary for none
    value.i = 0;
  }

  explicit Argument(bool val)
      : kind(ValueKind_Bool)
  {
    value.b = val;
  }
  explicit Argument(long long val)
      : kind(ValueKind_Int)
  {
    value.i = val;
  }
  explicit Argument(double val)
      : kind(ValueKind_Double)
  {
    value.d = val;
  }
  // note: takes ownership of new string
  explicit Argument(const std::string &val)
      : kind(ValueKind_String)
  {
    value.s = new std::string(val);
  }
  // note: takes ownership of new vector
  explicit Argument(const std::vector<std::string> &val)
      : kind(ValueKind_List)
  {
    value.sv = new std::vector<std::string>(val);
  }

  // cleanup pointer members
  ~Argument()
  {
    clear();
  }

  Argument(const Argument &other)
      : kind(ValueKind_None)
  {
    copy_from(other);
  }
  Argument &operator=(const Argument &other)
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
  void copy_from(const Argument &other)
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

  template <typename T>
  T get(const std::string &key);

  template <>
  bool get(const std::string &key)
  {
    return *get_bool();
  }

  template <>
  long long get(const std::string &key)
  {
    return *get_int();
  }

  template <>
  double get(const std::string &key)
  {
    return *get_double();
  }

  template <>
  std::string get(const std::string &key)
  {
    return *get_string();
  }

  template <>
  std::vector<std::string> get(const std::string &key)
  {
    return *get_string_vector();
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

template <typename>
struct dependent_false
{
  static const bool value = false;
};

// Helper struct for type-safe argument retrieval
template <typename T>
struct ArgGetter
{
  static const T *get(const Argument &)
  {
    static_assert(dependent_false<T>::value,
                  "ArgGetter is not specialized for this type");
    return nullptr;
  }
};
template <>
struct ArgGetter<bool>
{
  static const bool *get(const Argument &v)
  {
    return v.get_bool();
  }
};
template <>
struct ArgGetter<long long>
{
  static const long long *get(const Argument &v)
  {
    return v.get_int();
  }
};
template <>
struct ArgGetter<double>
{
  static const double *get(const Argument &v)
  {
    return v.get_double();
  }
};
template <>
struct ArgGetter<std::string>
{
  static const std::string *get(const Argument &v)
  {
    return v.get_string();
  }
};
template <>
struct ArgGetter<std::vector<std::string>>
{
  static const std::vector<std::string> *get(const Argument &v)
  {
    return v.get_string_vector();
  }
};

#if defined(__IBMTR1_CPP__) && !defined(__CLANG__)
typedef std::tr1::unordered_map<std::string, ArgValue> arg_map;
#else
typedef std::unordered_map<std::string, Argument> arg_map;
#endif

class Io
{
  class Impl;
  Impl *m_impl;

  arg_map m_args;
  arg_map m_output;

  std::ostream *m_output_stream;
  std::ostream *m_error_stream;

public:
  Io(arg_map &args, std::ostream *out = nullptr, std::ostream *err = nullptr);

  template <typename T>
  const T get_as(const std::string &key)
  {
    const auto ptr = ArgGetter<T>::get(m_args.at(key));
    if (ptr)
    {
      return *ptr;
    }
  }

  template <typename T>
  const T *has(const std::string &key)
  {
    if (m_args.find(key) != m_args.end())
    {
      return m_args[key];
    }

    return nullptr;
  }

  void print(const char *s)
  {
    if (m_output_stream && s != nullptr)
    {
      m_output_stream->write(s, strlen(s));
    }
  }

  void err(const char *e)
  {
    if (m_error_stream && e != nullptr)
    {
      m_error_stream->write(e, strlen(e));
    }
  }

  void println(const char *s)
  {
    print(s);
    if (m_output_stream)
    {
      m_output_stream->put('\n');
    }
  }

  void errln(const char *e)
  {
    err(e);
    if (m_error_stream)
    {
      m_error_stream->put('\n');
    }
  }

  template <typename T>
  void set_output(const std::string &key, const T value)
  {
    m_output.insert({key, Argument(value)});
  }

  void set_output(const arg_map &output)
  {
    m_output = output;
  }
};

#endif