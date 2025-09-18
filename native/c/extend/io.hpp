#ifndef EXTEND_IO_HPP
#define EXTEND_IO_HPP

#include <cstring>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#if defined(__IBMTR1_CPP__) && !defined(__CLANG__)
#  include <tr1/unordered_map>
#else
#  include <unordered_map>
#endif

namespace plugin
{

class Argument
{
public:
  enum ValueKind
  {
    ValueKind_None,
    ValueKind_Bool,
    ValueKind_Int,
    ValueKind_Double,
    ValueKind_String,
    ValueKind_List
  };

  Argument()
      : m_kind(ValueKind_None)
  {
    m_value.i = 0;
  }

  explicit Argument(bool val)
      : m_kind(ValueKind_Bool)
  {
    m_value.b = val;
  }

  explicit Argument(long long val)
      : m_kind(ValueKind_Int)
  {
    m_value.i = val;
  }

  explicit Argument(double val)
      : m_kind(ValueKind_Double)
  {
    m_value.d = val;
  }

  explicit Argument(const std::string &val)
      : m_kind(ValueKind_String)
  {
    m_value.s = new std::string(val);
  }

  explicit Argument(const char *val)
      : m_kind(ValueKind_String)
  {
    m_value.s = val ? new std::string(val) : new std::string();
  }

  explicit Argument(const std::vector<std::string> &val)
      : m_kind(ValueKind_List)
  {
    m_value.sv = new std::vector<std::string>(val);
  }

  Argument(const Argument &other)
      : m_kind(ValueKind_None)
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

  Argument(Argument &&other) noexcept
      : m_kind(ValueKind_None)
  {
    move_from(other);
  }

  Argument &operator=(Argument &&other) noexcept
  {
    if (this != &other)
    {
      clear();
      move_from(other);
    }
    return *this;
  }

  ~Argument()
  {
    clear();
  }

  ValueKind get_kind() const
  {
    return m_kind;
  }

  bool is_none() const
  {
    return m_kind == ValueKind_None;
  }

  bool is_bool() const
  {
    return m_kind == ValueKind_Bool;
  }

  bool is_int() const
  {
    return m_kind == ValueKind_Int;
  }

  bool is_double() const
  {
    return m_kind == ValueKind_Double;
  }

  bool is_string() const
  {
    return m_kind == ValueKind_String;
  }

  bool is_string_vector() const
  {
    return m_kind == ValueKind_List;
  }

  const bool *get_bool() const
  {
    return m_kind == ValueKind_Bool ? &m_value.b : nullptr;
  }

  const long long *get_int() const
  {
    return m_kind == ValueKind_Int ? &m_value.i : nullptr;
  }

  const double *get_double() const
  {
    return m_kind == ValueKind_Double ? &m_value.d : nullptr;
  }

  const std::string *get_string() const
  {
    return m_kind == ValueKind_String ? m_value.s : nullptr;
  }

  const std::vector<std::string> *get_string_vector() const
  {
    return m_kind == ValueKind_List ? m_value.sv : nullptr;
  }

  std::string *get_string_mutable()
  {
    return m_kind == ValueKind_String ? m_value.s : nullptr;
  }

  std::vector<std::string> *get_string_vector_mutable()
  {
    return m_kind == ValueKind_List ? m_value.sv : nullptr;
  }

  std::string get_string_value(const std::string &default_val = "") const
  {
    const std::string *ptr = get_string();
    return ptr ? *ptr : default_val;
  }

  void print(std::ostream &os) const
  {
    switch (m_kind)
    {
    case ValueKind_None:
      os << "<none>";
      break;
    case ValueKind_Bool:
      os << (m_value.b ? "true" : "false");
      break;
    case ValueKind_Int:
      os << m_value.i;
      break;
    case ValueKind_Double:
      os << m_value.d;
      break;
    case ValueKind_String:
      os << (m_value.s ? *m_value.s : "<invalid_string>");
      break;
    case ValueKind_List:
    {
      os << "[";
      if (m_value.sv)
      {
        for (size_t j = 0; j < m_value.sv->size(); ++j)
        {
          os << (*m_value.sv)[j];
          if (j + 1 < m_value.sv->size())
            os << ", ";
        }
      }
      os << "]";
      break;
    }
    }
  }

private:
  void clear()
  {
    if (m_kind == ValueKind_String)
    {
      delete m_value.s;
      m_value.s = nullptr;
    }
    else if (m_kind == ValueKind_List)
    {
      delete m_value.sv;
      m_value.sv = nullptr;
    }
    m_kind = ValueKind_None;
    m_value.i = 0;
  }

  void copy_from(const Argument &other)
  {
    m_kind = other.m_kind;
    switch (other.m_kind)
    {
    case ValueKind_None:
      m_value.i = 0;
      break;
    case ValueKind_Bool:
      m_value.b = other.m_value.b;
      break;
    case ValueKind_Int:
      m_value.i = other.m_value.i;
      break;
    case ValueKind_Double:
      m_value.d = other.m_value.d;
      break;
    case ValueKind_String:
      m_value.s = other.m_value.s ? new std::string(*other.m_value.s) : nullptr;
      break;
    case ValueKind_List:
      m_value.sv = other.m_value.sv
                       ? new std::vector<std::string>(*other.m_value.sv)
                       : nullptr;
      break;
    }
  }

  void move_from(Argument &other)
  {
    m_kind = other.m_kind;
    m_value = other.m_value;
    other.m_kind = ValueKind_None;
    other.m_value.i = 0;
  }

  ValueKind m_kind;
  union
  {
    bool b;
    long long i;
    double d;
    std::string *s;
    std::vector<std::string> *sv;
  } m_value;
};

// Helper struct for type-safe argument retrieval
template <typename T>
struct ArgGetter
{
  static const T *get(const Argument &)
  {
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
typedef std::tr1::unordered_map<std::string, Argument> ArgumentMap;
#else
typedef std::unordered_map<std::string, Argument> ArgumentMap;
#endif

class Io
{
public:
  Io() = default;

  Io(const ArgumentMap &args,
     std::ostream *out_stream = nullptr,
     std::ostream *err_stream = nullptr)
      : m_args(args), m_output_stream(out_stream), m_error_stream(err_stream)
  {
  }

  bool has(const std::string &key) const
  {
    return m_args.find(key) != m_args.end();
  }

  const Argument *find(const std::string &key) const
  {
    ArgumentMap::const_iterator it = m_args.find(key);
    return it != m_args.end() ? &it->second : nullptr;
  }

  template <typename T>
  const T *get_if(const std::string &key) const
  {
    ArgumentMap::const_iterator it = m_args.find(key);
    if (it == m_args.end())
      return nullptr;
    return ArgGetter<T>::get(it->second);
  }

  template <typename T>
  T get(const std::string &key) const
  {
    const T *ptr = get_if<T>(key);
    if (!ptr)
    {
      throw std::runtime_error("argument '" + key + "' missing or wrong type");
    }
    return *ptr;
  }

  template <typename T>
  T get_or(const std::string &key, const T &default_value) const
  {
    const T *ptr = get_if<T>(key);
    return ptr ? *ptr : default_value;
  }

  void print(const char *s) const
  {
    if (m_output_stream && s)
    {
      m_output_stream->write(s, std::strlen(s));
    }
  }

  void println(const char *s) const
  {
    print(s);
    if (m_output_stream)
    {
      m_output_stream->put('\n');
    }
  }

  void err(const char *e) const
  {
    if (m_error_stream && e)
    {
      m_error_stream->write(e, std::strlen(e));
    }
  }

  void errln(const char *e) const
  {
    err(e);
    if (m_error_stream)
    {
      m_error_stream->put('\n');
    }
  }

  template <typename T>
  void set_output(const std::string &key, const T &value)
  {
    m_output[key] = Argument(value);
  }

  void set_output(const ArgumentMap &output)
  {
    m_output = output;
  }

  const ArgumentMap &arguments() const
  {
    return m_args;
  }

  const ArgumentMap &output() const
  {
    return m_output;
  }

  std::ostream *output_stream() const
  {
    return m_output_stream;
  }

  std::ostream *error_stream() const
  {
    return m_error_stream;
  }

private:
  ArgumentMap m_args;
  ArgumentMap m_output;
  std::ostream *m_output_stream = nullptr;
  std::ostream *m_error_stream = nullptr;
};

class InvocationContext
{
public:
  InvocationContext(std::string command_path,
                    const ArgumentMap &args,
                    std::ostream *out_stream = nullptr,
                    std::ostream *err_stream = nullptr)
      : m_command_path(std::move(command_path)), m_io(args, out_stream, err_stream)
  {
  }

  Io &io()
  {
    return m_io;
  }

  const Io &io() const
  {
    return m_io;
  }

  const std::string &command_path() const
  {
    return m_command_path;
  }

private:
  std::string m_command_path;
  Io m_io;
};

} // namespace plugin

#endif
