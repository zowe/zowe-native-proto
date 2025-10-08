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

#ifndef ZJSON_HPP
#define ZJSON_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <limits>
#include "zjsonm.h"
#include "zjsontype.h"
#include "zstd.hpp"
#include "zlogger.hpp"
#include <hwtjic.h> // ensure to include /usr/include

#if defined(__clang__)
#define ZJSON_CONSTEXPR constexpr
#define ZJSON_NOEXCEPT noexcept
#else
#define ZJSON_CONSTEXPR const
#define ZJSON_NOEXCEPT throw()
namespace std
{
template <typename T, T v>
struct integral_constant
{
  static ZJSON_CONSTEXPR T value = v;
  typedef T value_type;
  typedef integral_constant type;
};
typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;
} // namespace std
#endif

/*
 * ZJson - C++ JSON library with automatic struct serialization
 *
 * ==================== COMPILER REQUIREMENTS ====================
 *
 * This library supports two compilation modes:
 *
 *   - Minimal mode: Compatible with z/OS XLC (C++98/03)
 *     Only dynamic JSON with zjson::Value is supported, making this a minimal
 *     header-only library with faster compilation times.
 *
 *   - Full mode: Requires xlclang (C++11/14)
 *     Enables automatic struct serialization/deserialization with ZJSON_DERIVE
 *     and ZJSON_SERIALIZABLE macros.
 *
 * ==================== QUICK REFERENCE ====================
 *
 * Parsing (from struct requires xlclang):
 *   zjson::Value data = zjson::from_str(json_string);
 *   auto result = zjson::from_str<MyStruct>(json_string);
 *
 * Serialization:
 *   std::string json = zjson::to_string(obj);
 *   std::string pretty = zjson::to_string_pretty(obj);
 *
 * Struct Registration (requires xlclang):
 *   ZJSON_DERIVE(StructName, field1, field2, ...)
 *   ZJSON_SERIALIZABLE(StructName, ZJSON_FIELD(StructName, field).rename("name"))
 *
 * Dynamic JSON (always available):
 *   zjson::Value obj = zjson::Value::create_object();
 *   zjson::Value arr = zjson::Value::create_array();
 *   obj["name"] = "John"; arr[0] = "item";
 *
 * Type Conversion (requires xlclang):
 *   zjson::Value val = zjson::to_value(obj);
 *   auto obj = zjson::from_value<MyStruct>(val);
 *
 * Value Access:
 *   value["key"]              // Object field access
 *   value[0]                  // Array element access
 *   value.as_string()         // Type conversion
 *
 * Error Handling:
 *   auto result = zjson::from_str<MyStruct>(json);
 *   if (result.has_value()) { auto obj = result.value(); }
 *
 * Field Attributes (requires xlclang):
 *   .rename("newName")        // JSON field name
 *   .skip()                   // Skip serialization/deserialization
 *   .skip_serializing_if_none()  // Skip if optional field is empty
 *   .with_default(func)       // Default value for missing fields
 *
 * Optional Fields:
 *   zstd::optional<T> field; // Nullable fields (null when empty)
 */

// Forward declarations
namespace zjson
{
class Value;
class Error;
template <typename T>
struct Serializable;
template <typename T>
struct Deserializable;

// Attributes namespace with RenameAll implementation
namespace attributes
{
struct RenameAll
{
  enum CaseStyle
  {
    none, // No transformation (default)
    lowercase,
    UPPERCASE,
    PascalCase,
    camelCase,
    snake_case,
    SCREAMING_SNAKE_CASE,
    kebab_case,
    SCREAMING_KEBAB_CASE
  };

  static std::string transform_name(const std::string &name, CaseStyle style)
  {
    switch (style)
    {
    case none:
      return name; // No transformation
    case lowercase:
    {
      std::string result;
      for (std::string::const_iterator it = name.begin(); it != name.end(); ++it)
      {
        result += std::tolower(*it);
      }
      return result;
    }
    case UPPERCASE:
    {
      std::string result;
      for (std::string::const_iterator it = name.begin(); it != name.end(); ++it)
      {
        result += std::toupper(*it);
      }
      return result;
    }
    case snake_case:
      return name; // Already in snake_case format (C++ convention)
    case camelCase:
    {
      std::string result;
      bool capitalize_next = false;
      for (std::string::const_iterator it = name.begin(); it != name.end(); ++it)
      {
        if (*it == '_')
        {
          capitalize_next = true;
        }
        else if (capitalize_next)
        {
          result += std::toupper(*it);
          capitalize_next = false;
        }
        else
        {
          result += std::tolower(*it);
        }
      }
      return result;
    }
    case PascalCase:
    {
      std::string result = transform_name(name, camelCase);
      if (!result.empty())
      {
        result[0] = std::toupper(result[0]);
      }
      return result;
    }
    case SCREAMING_SNAKE_CASE:
    {
      std::string result;
      for (std::string::const_iterator it = name.begin(); it != name.end(); ++it)
      {
        result += std::toupper(*it);
      }
      return result;
    }
    case kebab_case:
    {
      std::string result;
      for (std::string::const_iterator it = name.begin(); it != name.end(); ++it)
      {
        if (*it == '_')
        {
          result += '-';
        }
        else
        {
          result += std::tolower(*it);
        }
      }
      return result;
    }
    case SCREAMING_KEBAB_CASE:
    {
      std::string result;
      for (std::string::const_iterator it = name.begin(); it != name.end(); ++it)
      {
        if (*it == '_')
        {
          result += '-';
        }
        else
        {
          result += std::toupper(*it);
        }
      }
      return result;
    }
    default:
      return name;
    }
  }
};
} // namespace attributes

// Forward declare detail namespace and StructConfig
namespace detail
{
template <typename T>
struct StructConfig
{
  static attributes::RenameAll::CaseStyle rename_all_case;
  static bool deny_unknown_fields;
};

template <typename T>
attributes::RenameAll::CaseStyle StructConfig<T>::rename_all_case = attributes::RenameAll::none;

template <typename T>
bool StructConfig<T>::deny_unknown_fields = false;
} // namespace detail

/**
 * Error handling class for JSON operations
 */
class Error : public std::runtime_error
{
public:
  enum ErrorKind
  {
    InvalidValue,
    InvalidType,
    InvalidLength,
    InvalidData,
    MissingField,
    UnknownField,
    Custom
  };

private:
  ErrorKind kind_;
  size_t line_;
  size_t column_;

public:
  Error(ErrorKind kind, const std::string &message)
      : std::runtime_error(message), kind_(kind), line_(0), column_(0)
  {
  }

  Error(ErrorKind kind, const std::string &message, size_t line, size_t column)
      : std::runtime_error(message), kind_(kind), line_(line), column_(column)
  {
  }

  ErrorKind kind() const
  {
    return kind_;
  }
  size_t line() const
  {
    return line_;
  }
  size_t column() const
  {
    return column_;
  }

  static inline Error invalid_value(const std::string &msg)
  {
    return Error(InvalidValue, "Invalid value: " + msg);
  }

  static inline Error invalid_type(const std::string &expected, const std::string &found)
  {
    return Error(InvalidType, "Invalid type. Expected " + expected + ", found " + found);
  }

  static inline Error missing_field(const std::string &field)
  {
    return Error(MissingField, "Missing field: " + field);
  }

  static inline Error unknown_field(const std::string &field)
  {
    return Error(UnknownField, "Unknown field: " + field);
  }

  static inline Error invalid_data(const std::string &msg)
  {
    return Error(InvalidData, "Invalid data: " + msg);
  }
};

/**
 * Value type for JSON operations
 */
class Value
{
  // Forward declare friend functions and classes
  friend std::string value_to_json_string(const Value &value);
  friend Value parse_json_string(const std::string &json_str);
  friend Value json_handle_to_value(JSON_INSTANCE *instance, KEY_HANDLE *key_handle);

  // Friend template specializations for vector serialization
  template <typename T>
  friend struct Serializable;

public:
  // Public methods for creating Values to avoid private member access
  static Value create_object()
  {
    Value result;
    result.data_ = std::map<std::string, Value>();
    return result;
  }

  static Value create_array()
  {
    Value result;
    result.data_ = std::vector<Value>();
    return result;
  }

  void add_to_object(const std::string &key, const Value &value)
  {
    if (!is_object())
    {
      throw Error::invalid_type("object", "other");
    }
    get_object()[key] = value;
  }

  void add_to_array(const Value &value)
  {
    if (!is_array())
    {
      throw Error::invalid_type("array", "other");
    }
    get_array().push_back(value);
  }

  void reserve_array(size_t capacity)
  {
    if (!is_array())
    {
      throw Error::invalid_type("array", "other");
    }
    get_array().reserve(capacity);
  }
  enum Type
  {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
  };

private:
  // Variant type for JSON values
  typedef zstd::variant<
      zstd::monostate,             // Null
      bool,                        // Bool
      long long,                   // Integer
      double,                      // Float
      std::string,                 // String
      std::vector<Value>,          // Array
      std::map<std::string, Value> // Object
      >
      ValueVariant;

  ValueVariant data_;

  // Helper methods to avoid .template syntax for XLC compatibility
  inline bool &get_bool()
  {
    return data_.get<bool>();
  }
  inline long long &get_long_long()
  {
    return data_.get<long long>();
  }
  inline double &get_double()
  {
    return data_.get<double>();
  }
  inline std::string &get_string()
  {
    return data_.get<std::string>();
  }
  inline std::vector<Value> &get_array()
  {
    return data_.get<std::vector<Value>>();
  }
  inline std::map<std::string, Value> &get_object()
  {
    return data_.get<std::map<std::string, Value>>();
  }

  inline const bool &get_bool() const
  {
    return data_.get<bool>();
  }
  inline const long long &get_long_long() const
  {
    return data_.get<long long>();
  }
  inline const double &get_double() const
  {
    return data_.get<double>();
  }
  inline const std::string &get_string() const
  {
    return data_.get<std::string>();
  }
  inline const std::vector<Value> &get_array() const
  {
    return data_.get<std::vector<Value>>();
  }
  inline const std::map<std::string, Value> &get_object() const
  {
    return data_.get<std::map<std::string, Value>>();
  }

public:
  Value() : data_(zstd::monostate())
  {
  }
  Value(bool b) : data_(b)
  {
  }
  Value(int i) : data_(static_cast<long long>(i))
  {
  }
  Value(long long ll) : data_(ll)
  {
  }
  Value(unsigned long long ull)
  {
    if (ull <= static_cast<unsigned long long>(std::numeric_limits<long long>::max()))
    {
      data_ = static_cast<long long>(ull);
    }
    else
    {
      data_ = static_cast<double>(ull);
    }
  }
  Value(double d) : data_(d)
  {
  }
  Value(const std::string &s) : data_(s)
  {
  }
  Value(const char *s) : data_(std::string(s))
  {
  }

  // Copy constructor and assignment
  Value(const Value &other) : data_(other.data_)
  {
  }

  Value &operator=(const Value &other)
  {
    if (this != &other)
    {
      data_ = other.data_;
    }
    return *this;
  }

  ~Value()
  {
  }

  inline Type get_type() const
  {
    switch (data_.index())
    {
    case 0:
      return Null;
    case 1:
      return Bool;
    case 2:
    case 3:
      return Number;
    case 4:
      return String;
    case 5:
      return Array;
    case 6:
      return Object;
    default:
      return Null;
    }
  }

  inline bool as_bool() const
  {
    if (!is_bool())
      throw Error::invalid_type("bool", type_name());
    return get_bool();
  }

  inline long long as_int64() const
  {
    if (is_integer())
    {
      return get_long_long();
    }

    if (is_double())
    {
      double value = get_double();

      if (value != std::floor(value))
      {
        std::stringstream ss;
        ss << value;
        throw Error::invalid_value("Cannot convert floating-point number " + ss.str() + " to int64");
      }
      else if (std::isnan(value) || std::isinf(value))
      {
        std::stringstream ss;
        ss << value;
        throw Error::invalid_value("Number is not finite: " + ss.str());
      }
      else if (value > static_cast<double>(std::numeric_limits<long long>::max()) || value < static_cast<double>(std::numeric_limits<long long>::min()))
      {
        std::stringstream ss;
        ss << value;
        throw Error::invalid_value("Number " + ss.str() + " overflows long long range");
      }

      return static_cast<long long>(value);
    }

    throw Error::invalid_type("number", type_name());
  }

  inline unsigned long long as_uint64() const
  {
    if (is_integer())
    {
      long long value = get_long_long();
      if (value < 0)
      {
        std::stringstream ss;
        ss << value;
        throw Error::invalid_value("Cannot convert negative number " + ss.str() + " to uint64");
      }
      return static_cast<unsigned long long>(value);
    }

    if (is_double())
    {
      double value = get_double();

      if (value < 0)
      {
        std::stringstream ss;
        ss << value;
        throw Error::invalid_value("Cannot convert negative number " + ss.str() + " to uint64");
      }
      else if (value != std::floor(value))
      {
        std::stringstream ss;
        ss << value;
        throw Error::invalid_value("Cannot convert floating-point number " + ss.str() + " to uint64");
      }
      else if (std::isnan(value) || std::isinf(value))
      {
        std::stringstream ss;
        ss << value;
        throw Error::invalid_value("Number is not finite: " + ss.str());
      }
      else if (value > static_cast<double>(std::numeric_limits<unsigned long long>::max()))
      {
        std::stringstream ss;
        ss << value;
        throw Error::invalid_value("Number " + ss.str() + " overflows unsigned long long range");
      }

      return static_cast<unsigned long long>(value);
    }

    throw Error::invalid_type("number", type_name());
  }

  inline double as_double() const
  {
    if (is_double())
    {
      double value = get_double();
      if (std::isnan(value) || std::isinf(value))
      {
        std::stringstream ss;
        ss << value;
        throw Error::invalid_value("Number is not finite: " + ss.str());
      }
      return value;
    }

    if (is_integer())
    {
      return static_cast<double>(get_long_long());
    }

    throw Error::invalid_type("number", type_name());
  }

  inline std::string as_string() const
  {
    if (!is_string())
      throw Error::invalid_type("string", type_name());
    return get_string();
  }

  const std::vector<Value> &as_array() const
  {
    if (!is_array())
      throw Error::invalid_type("array", type_name());
    return get_array();
  }

  const std::map<std::string, Value> &as_object() const
  {
    if (!is_object())
      throw Error::invalid_type("object", type_name());
    return get_object();
  }

  inline bool is_null() const
  {
    return zstd::holds_alternative<zstd::monostate>(data_);
  }
  inline bool is_bool() const
  {
    return zstd::holds_alternative<bool>(data_);
  }
  inline bool is_integer() const
  {
    return zstd::holds_alternative<long long>(data_);
  }
  inline bool is_double() const
  {
    return zstd::holds_alternative<double>(data_);
  }
  inline bool is_string() const
  {
    return zstd::holds_alternative<std::string>(data_);
  }
  inline bool is_array() const
  {
    return zstd::holds_alternative<std::vector<Value>>(data_);
  }
  inline bool is_object() const
  {
    return zstd::holds_alternative<std::map<std::string, Value>>(data_);
  }

  // Object access by key
  Value &operator[](const std::string &key)
  {
    if (is_null())
    {
      // Convert null to empty object on first access
      data_ = std::map<std::string, Value>();
    }

    if (!is_object())
    {
      throw Error::invalid_type("object", type_name());
    }

    return get_object()[key]; // Creates entry if doesn't exist
  }

  const Value &operator[](const std::string &key) const
  {
    if (!is_object())
    {
      throw Error::invalid_type("object", type_name());
    }

    const std::map<std::string, Value> &obj = get_object();
    std::map<std::string, Value>::const_iterator it = obj.find(key);
    if (it == obj.end())
    {
      static const Value null_value; // Return reference to static null
      return null_value;
    }

    return it->second;
  }

  // Array access by index
  Value &operator[](size_t index)
  {
    if (is_null())
    {
      // Convert null to empty array on first access
      data_ = std::vector<Value>();
    }

    if (!is_array())
    {
      throw Error::invalid_type("array", type_name());
    }

    std::vector<Value> &arr = get_array();

    // Expand array if needed
    if (index >= arr.size())
    {
      arr.resize(index + 1);
    }

    return arr[index];
  }

  const Value &operator[](size_t index) const
  {
    if (!is_array())
    {
      throw Error::invalid_type("array", type_name());
    }

    const std::vector<Value> &arr = get_array();
    if (index >= arr.size())
    {
      static const Value null_value; // Return reference to static null
      return null_value;
    }

    return arr[index];
  }

private:
  inline std::string type_name() const
  {
    switch (get_type())
    {
    case Null:
      return "null";
    case Bool:
      return "bool";
    case Number:
      return "number";
    case String:
      return "string";
    case Array:
      return "array";
    case Object:
      return "object";
    default:
      return "unknown";
    }
  }
};

/**
 * Serialization trait for JSON operations
 */
template <typename T>
struct Serializable
{
  static constexpr bool value = false;

  static Value serialize(const T &obj)
  {
    static_assert(Serializable<T>::value, "Type must implement Serializable trait");
    return Value();
  }
};

/**
 * Deserialization trait for JSON operations
 */
template <typename T>
struct Deserializable
{
  static constexpr bool value = false;

  static zstd::expected<T, Error> deserialize(const Value &value)
  {
    static_assert(Deserializable<T>::value, "Type must implement Deserializable trait");
    return zstd::make_unexpected(Error::invalid_type("deserializable", "unknown"));
  }
};

// Specializations for basic types
template <>
struct Serializable<bool>
{
  static ZJSON_CONSTEXPR bool value = true;
  static Value serialize(const bool &obj) ZJSON_NOEXCEPT
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<bool>
{
  static ZJSON_CONSTEXPR bool value = true;
  static zstd::expected<bool, Error> deserialize(const Value &value)
  {
    try
    {
      return value.as_bool();
    }
    catch (const Error &e)
    {
      return zstd::make_unexpected(e);
    }
  }
};

template <>
struct Serializable<int>
{
  static ZJSON_CONSTEXPR bool value = true;
  static Value serialize(const int &obj) ZJSON_NOEXCEPT
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<int>
{
  static ZJSON_CONSTEXPR bool value = true;
  static zstd::expected<int, Error> deserialize(const Value &value)
  {
    try
    {
      // Get as int64 and downcast to int with range check
      long long val = value.as_int64();
      if (val > static_cast<long long>(std::numeric_limits<int>::max()) ||
          val < static_cast<long long>(std::numeric_limits<int>::min()))
      {
        std::stringstream ss;
        ss << val;
        throw Error::invalid_value("Number " + ss.str() + " overflows int range");
      }
      return static_cast<int>(val);
    }
    catch (const Error &e)
    {
      return zstd::make_unexpected(e);
    }
  }
};

template <>
struct Serializable<unsigned int>
{
  static ZJSON_CONSTEXPR bool value = true;
  static Value serialize(const unsigned int &obj) ZJSON_NOEXCEPT
  {
    return Value(static_cast<unsigned long long>(obj));
  }
};

template <>
struct Deserializable<unsigned int>
{
  static ZJSON_CONSTEXPR bool value = true;
  static zstd::expected<unsigned int, Error> deserialize(const Value &value)
  {
    try
    {
      // Get as uint64 and downcast to unsigned int with range check
      unsigned long long val = value.as_uint64();
      if (val > static_cast<unsigned long long>(std::numeric_limits<unsigned int>::max()))
      {
        std::stringstream ss;
        ss << val;
        throw Error::invalid_value("Number " + ss.str() + " overflows unsigned int range");
      }
      return static_cast<unsigned int>(val);
    }
    catch (const Error &e)
    {
      return zstd::make_unexpected(e);
    }
  }
};

template <>
struct Serializable<long long>
{
  static ZJSON_CONSTEXPR bool value = true;
  static Value serialize(const long long &obj) ZJSON_NOEXCEPT
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<long long>
{
  static ZJSON_CONSTEXPR bool value = true;
  static zstd::expected<long long, Error> deserialize(const Value &value)
  {
    try
    {
      return value.as_int64();
    }
    catch (const Error &e)
    {
      return zstd::make_unexpected(e);
    }
  }
};

template <>
struct Serializable<unsigned long long>
{
  static ZJSON_CONSTEXPR bool value = true;
  static Value serialize(const unsigned long long &obj) ZJSON_NOEXCEPT
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<unsigned long long>
{
  static ZJSON_CONSTEXPR bool value = true;
  static zstd::expected<unsigned long long, Error> deserialize(const Value &value)
  {
    try
    {
      return value.as_uint64();
    }
    catch (const Error &e)
    {
      return zstd::make_unexpected(e);
    }
  }
};

// Support for size_t (typically an alias for unsigned long long or unsigned int)
template <>
struct Serializable<size_t>
{
  static ZJSON_CONSTEXPR bool value = true;
  static Value serialize(const size_t &obj) ZJSON_NOEXCEPT
  {
    return Value(static_cast<unsigned long long>(obj));
  }
};

template <>
struct Deserializable<size_t>
{
  static ZJSON_CONSTEXPR bool value = true;
  static zstd::expected<size_t, Error> deserialize(const Value &value)
  {
    try
    {
      unsigned long long val = value.as_uint64();
      // size_t range check (in case size_t is smaller than unsigned long long)
      if (val > static_cast<unsigned long long>(std::numeric_limits<size_t>::max()))
      {
        std::stringstream ss;
        ss << val;
        throw Error::invalid_value("Number " + ss.str() + " overflows size_t range");
      }
      return static_cast<size_t>(val);
    }
    catch (const Error &e)
    {
      return zstd::make_unexpected(e);
    }
  }
};

template <>
struct Serializable<double>
{
  static ZJSON_CONSTEXPR bool value = true;
  static Value serialize(const double &obj) ZJSON_NOEXCEPT
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<double>
{
  static ZJSON_CONSTEXPR bool value = true;
  static zstd::expected<double, Error> deserialize(const Value &value)
  {
    try
    {
      return value.as_double();
    }
    catch (const Error &e)
    {
      return zstd::make_unexpected(e);
    }
  }
};

template <>
struct Serializable<std::string>
{
  static ZJSON_CONSTEXPR bool value = true;
  static Value serialize(const std::string &obj) ZJSON_NOEXCEPT
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<std::string>
{
  static ZJSON_CONSTEXPR bool value = true;
  static zstd::expected<std::string, Error> deserialize(const Value &value)
  {
    try
    {
      return value.as_string();
    }
    catch (const Error &e)
    {
      return zstd::make_unexpected(e);
    }
  }
};

// Specializations for Value itself - enables both to_value<Value> and from_str<Value>
template <>
struct Serializable<Value>
{
  static ZJSON_CONSTEXPR bool value = true;
  static Value serialize(const Value &obj)
  {
    return obj; // Value serializes to itself (identity)
  }
};

template <>
struct Deserializable<Value>
{
  static ZJSON_CONSTEXPR bool value = true;
  static zstd::expected<Value, Error> deserialize(const Value &value)
  {
    // Value to Value is just a copy - already parsed from JSON
    return value;
  }
};

// Specializations for std::vector<T>
template <typename T>
struct Serializable<std::vector<T>>
{
  static ZJSON_CONSTEXPR bool value = Serializable<T>::value;
  static Value serialize(const std::vector<T> &vec)
  {
    Value result = Value::create_array();
    result.reserve_array(vec.size());

    for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
      result.add_to_array(Serializable<T>::serialize(*it));
    }
    return result;
  }
};

template <typename T>
struct Deserializable<std::vector<T>>
{
  static ZJSON_CONSTEXPR bool value = Deserializable<T>::value;
  static zstd::expected<std::vector<T>, Error> deserialize(const Value &value)
  {
    if (!value.is_array())
    {
      return zstd::make_unexpected(Error::invalid_type("array", "other"));
    }

    std::vector<T> result;
    const std::vector<Value> &array = value.as_array();
    result.reserve(array.size());

    for (std::vector<Value>::const_iterator it = array.begin(); it != array.end(); ++it)
    {
      zstd::expected<T, Error> item_result = Deserializable<T>::deserialize(*it);
      if (!item_result.has_value())
      {
        return zstd::make_unexpected(item_result.error());
      }
      result.push_back(item_result.value());
    }

    return result;
  }
};

// Specializations for zstd::optional<T>
template <typename T>
struct Serializable<zstd::optional<T>>
{
  static ZJSON_CONSTEXPR bool value = Serializable<T>::value;
  static Value serialize(const zstd::optional<T> &opt)
  {
    if (opt.has_value())
    {
      return Serializable<T>::serialize(opt.value());
    }
    else
    {
      return Value(); // Create null value
    }
  }
};

template <typename T>
struct Deserializable<zstd::optional<T>>
{
  static ZJSON_CONSTEXPR bool value = Deserializable<T>::value;
  static zstd::expected<zstd::optional<T>, Error> deserialize(const Value &value)
  {
    if (value.is_null())
    {
      return zstd::optional<T>(); // Empty optional
    }
    else
    {
      zstd::expected<T, Error> result = Deserializable<T>::deserialize(value);
      if (!result.has_value())
      {
        return zstd::make_unexpected(result.error());
      }
      return zstd::optional<T>(result.value());
    }
  }
};

#if defined(__clang__)
/**
 * Field descriptor for reflection-like behavior
 */
template <typename T, typename FieldType>
struct Field
{
  std::string name;
  FieldType T::*member;
  bool skip_serializing;
  bool skip_deserializing;
  bool skip_if_none;  // For optional fields - skip if no value
  bool flatten_field; // For struct flattening - inline fields from nested struct
  std::string rename_to;
  std::function<FieldType()> default_value;

  Field(const std::string &n, FieldType T::*m)
      : name(n), member(m), skip_serializing(false), skip_deserializing(false), skip_if_none(false), flatten_field(false)
  {
  }

  inline Field &rename(const std::string &new_name)
  {
    rename_to = new_name;
    return *this;
  }

  inline Field &skip()
  {
    skip_serializing = true;
    skip_deserializing = true;
    return *this;
  }

  inline Field &skip_serializing_field()
  {
    skip_serializing = true;
    return *this;
  }

  inline Field &skip_deserializing_field()
  {
    skip_deserializing = true;
    return *this;
  }

  inline Field &with_default(std::function<FieldType()> default_fn)
  {
    default_value = default_fn;
    return *this;
  }

  inline Field &skip_serializing_if_none()
  {
    skip_if_none = true;
    return *this;
  }

  inline Field &flatten()
  {
    flatten_field = true;
    return *this;
  }

  inline std::string get_serialized_name() const
  {
    if (!rename_to.empty())
    {
      return rename_to;
    }
    // Apply struct-level rename_all transformation if no explicit rename
    return attributes::RenameAll::transform_name(name, detail::StructConfig<T>::rename_all_case);
  }
};

/**
 * Serialization registry for custom types
 */
template <typename T>
class SerializationRegistry
{
public:
  using SerializeFunc = std::function<Value(const T &)>;
  using DeserializeFunc = std::function<zstd::expected<T, Error>(const Value &)>;

  static inline void register_serializer(SerializeFunc func)
  {
    get_serializer() = func;
  }

  static inline void register_deserializer(DeserializeFunc func)
  {
    get_deserializer() = func;
  }

  static SerializeFunc &get_serializer()
  {
    static SerializeFunc serializer;
    return serializer;
  }

  static DeserializeFunc &get_deserializer()
  {
    static DeserializeFunc deserializer;
    return deserializer;
  }

  static inline bool has_serializer()
  {
    return static_cast<bool>(get_serializer());
  }

  static inline bool has_deserializer()
  {
    return static_cast<bool>(get_deserializer());
  }
};
#endif

/**
 * Main serialization and deserialization functions
 */

// Helper implementation functions for C++14 compatibility
template <typename T>
zstd::expected<std::string, Error> to_string_impl(const T &value, std::true_type)
{
  try
  {
    Value serialized = Serializable<T>::serialize(value);
    std::string json_str = value_to_json_string(serialized);
    return json_str;
  }
  catch (const Error &e)
  {
    return zstd::make_unexpected(e);
  }
  catch (const std::exception &e)
  {
    return zstd::make_unexpected(Error(Error::Custom, e.what()));
  }
}

template <typename T>
zstd::expected<std::string, Error> to_string_impl(const T &value, std::false_type)
{
#if defined(__clang__)
  try
  {
    if (SerializationRegistry<T>::has_serializer())
    {
      Value serialized = SerializationRegistry<T>::get_serializer()(value);
      std::string json_str = value_to_json_string(serialized);
      return json_str;
    }
  }
  catch (const Error &e)
  {
    return zstd::make_unexpected(e);
  }
  catch (const std::exception &e)
  {
    return zstd::make_unexpected(Error(Error::Custom, e.what()));
  }
#endif
  return zstd::make_unexpected(Error::invalid_type("serializable", "unknown"));
}

template <typename T>
zstd::expected<T, Error> from_str_impl(const Value &parsed, std::true_type)
{
  return Deserializable<T>::deserialize(parsed);
}

template <typename T>
zstd::expected<T, Error> from_str_impl(const Value &parsed, std::false_type)
{
#if defined(__clang__)
  if (SerializationRegistry<T>::has_deserializer())
  {
    return SerializationRegistry<T>::get_deserializer()(parsed);
  }
#endif
  return zstd::make_unexpected(Error::invalid_type("deserializable", "unknown"));
}

// to_string function for JSON serialization
template <typename T>
zstd::expected<std::string, Error> to_string(const T &value)
{
  try
  {
    return to_string_impl<T>(value, typename std::integral_constant<bool, Serializable<T>::value>::type());
  }
  catch (const Error &e)
  {
    return zstd::make_unexpected(e);
  }
  catch (const std::exception &e)
  {
    return zstd::make_unexpected(Error(Error::Custom, e.what()));
  }
}

// Helper function for JSON string escaping
inline std::string escape_json_string(const std::string &input)
{
  std::string output;
  output.reserve(input.length());

  for (std::string::const_iterator it = input.begin(); it != input.end(); ++it)
  {
    char c = *it;
    switch (c)
    {
    case '"':
      output += "\\\"";
      break;
    case '\\':
      output += "\\\\";
      break;
    case '\b':
      output += "\\b";
      break;
    case '\f':
      output += "\\f";
      break;
    case '\n':
      output += "\\n";
      break;
    case '\r':
      output += "\\r";
      break;
    case '\t':
      output += "\\t";
      break;
    default:
      if (iscntrl(c))
      {
        char buf[7];
        snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
        output += buf;
      }
      else
      {
        output += c;
      }
      break;
    }
  }
  return output;
}

// Helper function for JSON string unescaping
inline std::string unescape_json_string(const std::string &s)
{
  std::string res;
  res.reserve(s.length());
  for (std::string::size_type i = 0; i < s.length(); ++i)
  {
    if (s[i] == '\\' && i + 1 < s.length())
    {
      switch (s[++i])
      {
      case '"':
        res += '"';
        break;
      case '\\':
        res += '\\';
        break;
      case '/':
        res += '/';
        break;
      case 'b':
        res += '\b';
        break;
      case 'f':
        res += '\f';
        break;
      case 'n':
        res += '\n';
        break;
      case 'r':
        res += '\r';
        break;
      case 't':
        res += '\t';
        break;
      case 'u':
        if (i + 4 < s.length())
        {
          try
          {
            std::string hex = s.substr(i + 1, 4);
            char *endptr;
            unsigned long val = std::strtoul(hex.c_str(), &endptr, 16);
            if (endptr != hex.c_str() && val < 256)
            {
              res += static_cast<char>(val);
            }
            else
            {
              // For values outside of single byte range, preserve original escape
              res += "\\u" + hex;
            }
            i += 4;
          }
          catch (...)
          {
            res += "\\u"; // Malformed, just append what we can
          }
        }
        else
        {
          res += '\\';
          res += 'u';
        }
        break;
      default:
        // Unrecognized escape sequence, just append as is
        res += '\\';
        res += s[i];
        break;
      }
    }
    else
    {
      res += s[i];
    }
  }
  return res;
}

inline Value json_handle_to_value(JSON_INSTANCE *instance, KEY_HANDLE *key_handle)
{
  try
  {
    int type = 0;
    int rc = ZJSNGJST(instance, key_handle, &type);
    if (rc != 0)
    {
      return Value(); // Return null on error
    }

    switch (type)
    {
    case HWTJ_NULL_TYPE:
      return Value();

    case HWTJ_BOOLEAN_TYPE:
    {
      char bool_val = 0;
      rc = ZJSMGBOV(instance, key_handle, &bool_val);
      if (rc != 0)
      {
        return Value();
      }
      return Value(bool_val == HWTJ_TRUE);
    }

    case HWTJ_NUMBER_TYPE:
    {
      char *value_ptr = 0;
      int value_length = 0;
      rc = ZJSMGVAL(instance, key_handle, &value_ptr, &value_length);
      if (rc != 0)
      {
        return Value();
      }
      try
      {
        std::string str_val(value_ptr, value_length);
        if (str_val.find('.') != std::string::npos || str_val.find('e') != std::string::npos || str_val.find('E') != std::string::npos)
        {
          char *endptr;
          double num_val = std::strtod(str_val.c_str(), &endptr);
          return Value(num_val);
        }
        else
        {
          char *endptr;
          long long int_val = std::strtoll(str_val.c_str(), &endptr, 10);
          return Value(int_val);
        }
      }
      catch (...)
      {
        return Value();
      }
    }

    case HWTJ_STRING_TYPE:
    {
      char *value_ptr = 0;
      int value_length = 0;
      rc = ZJSMGVAL(instance, key_handle, &value_ptr, &value_length);
      if (rc != 0)
      {
        return Value();
      }
      std::string raw_str(value_ptr, value_length);
      return Value(unescape_json_string(raw_str));
    }

    case HWTJ_ARRAY_TYPE:
    {
      Value result = Value::create_array();

      int num_entries = 0;
      rc = ZJSMGNUE(instance, key_handle, &num_entries);
      if (rc != 0)
      {
        return result; // Return empty array
      }

      for (int i = 0; i < num_entries; i++)
      {
        KEY_HANDLE element_handle = {0};
        rc = ZJSMGAEN(instance, key_handle, &i, &element_handle);
        if (rc == 0)
        {
          try
          {
            Value element = json_handle_to_value(instance, &element_handle);
            result.add_to_array(element);
          }
          catch (...)
          {
            // Skip problematic array elements
            ZLOG_WARN("Failed to parse JSON array element at index %d to Value", i);
            continue;
          }
        }
      }
      return result;
    }

    case HWTJ_OBJECT_TYPE:
    {
      Value result = Value::create_object();

      int num_entries = 0;
      rc = ZJSMGNUE(instance, key_handle, &num_entries);
      if (rc != 0)
      {
        return result; // Return empty object
      }

      for (int i = 0; i < num_entries; i++)
      {
        char key_buffer[256] = {0};
        char *key_buffer_ptr = key_buffer;
        int key_buffer_length = sizeof(key_buffer);
        KEY_HANDLE value_handle = {0};
        int actual_length = 0;

        rc = ZJSMGOEN(instance, key_handle, &i, &key_buffer_ptr, &key_buffer_length, &value_handle, &actual_length);
        if (rc == 0)
        {
          try
          {
            std::string key_name(key_buffer_ptr, actual_length);
            Value value = json_handle_to_value(instance, &value_handle);
            result.add_to_object(key_name, value);
          }
          catch (...)
          {
            // Skip problematic object fields
            ZLOG_WARN("Failed to parse JSON object field at index %d to Value", i);
            continue;
          }
        }
        else if (rc == HWTJ_BUFFER_TOO_SMALL)
        {
          // Allocate larger buffer for long keys
          std::vector<char> dynamic_buffer(actual_length);
          key_buffer_ptr = &dynamic_buffer[0];
          key_buffer_length = actual_length;

          rc = ZJSMGOEN(instance, key_handle, &i, &key_buffer_ptr, &key_buffer_length, &value_handle, &actual_length);
          if (rc == 0)
          {
            try
            {
              std::string key_name(key_buffer_ptr, actual_length);
              Value value = json_handle_to_value(instance, &value_handle);
              result.add_to_object(key_name, value);
            }
            catch (...)
            {
              // Skip problematic object fields
              ZLOG_WARN("Failed to parse JSON object field at index %d to Value", i);
              continue;
            }
          }
        }
      }
      return result;
    }

    default:
      return Value();
    }
  }
  catch (const std::exception &e)
  {
    return Value();
  }
  catch (...)
  {
    return Value();
  }
}

// from_value function - convert Value to any deserializable type
template <typename T>
zstd::expected<T, Error> from_value(const Value &value)
{
  try
  {
    return from_str_impl<T>(value, typename std::integral_constant<bool, Deserializable<T>::value>::type());
  }
  catch (const Error &e)
  {
    return zstd::make_unexpected(e);
  }
  catch (const std::exception &e)
  {
    return zstd::make_unexpected(Error(Error::Custom, e.what()));
  }
}

// Helper implementation functions for to_value for C++14 compatibility
template <typename T>
zstd::expected<Value, Error> to_value_impl(const T &obj, std::true_type)
{
  return Serializable<T>::serialize(obj);
}

template <typename T>
zstd::expected<Value, Error> to_value_impl(const T &obj, std::false_type)
{
#if defined(__clang__)
  if (SerializationRegistry<T>::has_serializer())
  {
    return SerializationRegistry<T>::get_serializer()(obj);
  }
#endif
  return zstd::make_unexpected(Error::invalid_type("serializable", "unknown"));
}

// to_value function - convert any serializable type to Value
template <typename T>
zstd::expected<Value, Error> to_value(const T &obj)
{
  try
  {
    return to_value_impl<T>(obj, typename std::integral_constant<bool, Serializable<T>::value>::type());
  }
  catch (const Error &e)
  {
    return zstd::make_unexpected(e);
  }
  catch (const std::exception &e)
  {
    return zstd::make_unexpected(Error(Error::Custom, e.what()));
  }
}

// from_str function for JSON deserialization
template <typename T>
zstd::expected<T, Error> from_str(const std::string &json_str)
{
  JSON_INSTANCE instance = {};
  int rc = ZJSMINIT(&instance);
  if (rc != 0)
  {
    return zstd::make_unexpected(Error(Error::Custom, "Failed to initialize JSON parser"));
  }

  try
  {
    rc = ZJSMPARS(&instance, json_str.c_str());
    if (rc != 0)
    {
      ZJSMTERM(&instance);
      return zstd::make_unexpected(Error(Error::Custom, "Failed to parse JSON string"));
    }

    // Get root handle and convert to Value
    KEY_HANDLE root_handle = {0};
    Value parsed = json_handle_to_value(&instance, &root_handle);

    // Clean up
    ZJSMTERM(&instance);

    return from_str_impl<T>(parsed, typename std::integral_constant<bool, Deserializable<T>::value>::type());
  }
  catch (const Error &e)
  {
    ZJSMTERM(&instance);
    return zstd::make_unexpected(e);
  }
  catch (const std::exception &e)
  {
    ZJSMTERM(&instance);
    return zstd::make_unexpected(Error(Error::Custom, e.what()));
  }
}

// Convenience overload: from_str without template parameter defaults to Value
inline zstd::expected<Value, Error> from_str(const std::string &json_str)
{
  return from_str<Value>(json_str);
}

// Helper function to add indentation to JSON string
inline std::string add_json_indentation(const std::string &json_str, int spaces)
{
  std::string result;
  int indent_level = 0;
  bool in_string = false;
  bool escape_next = false;

  for (std::string::const_iterator it = json_str.begin(); it != json_str.end(); ++it)
  {
    char ch = *it;
    if (escape_next)
    {
      result += ch;
      escape_next = false;
      continue;
    }

    if (ch == '\\' && in_string)
    {
      result += ch;
      escape_next = true;
      continue;
    }

    if (ch == '\"')
    {
      result += ch;
      in_string = !in_string;
    }
    else if (!in_string)
    {
      switch (ch)
      {
      case '{':
      case '[':
        result += ch;
        result += '\n';
        indent_level++;
        result.append(indent_level * spaces, ' ');
        break;
      case '}':
      case ']':
        result += '\n';
        indent_level--;
        result.append(indent_level * spaces, ' ');
        result += ch;
        break;
      case ',':
        result += ch;
        result += '\n';
        result.append(indent_level * spaces, ' ');
        break;
      case ':':
        result += ch;
        result += ' ';
        break;
      default:
        result += ch;
        break;
      }
    }
    else
    {
      result += ch;
    }
  }
  return result;
}

// to_string_pretty function for pretty-printed JSON serialization
template <typename T>
zstd::expected<std::string, Error> to_string_pretty(const T &value)
{
  zstd::expected<std::string, Error> result = to_string(value);
  if (!result.has_value())
  {
    return result;
  }

  try
  {
    // Use our simple indentation function instead of ZJson
    return add_json_indentation(result.value(), 2);
  }
  catch (const std::exception &e)
  {
    return zstd::make_unexpected(Error(Error::Custom, e.what()));
  }
}

// Helper function to add Value to JSON instance using ZJSM API
inline int value_to_json_instance(JSON_INSTANCE *instance, KEY_HANDLE *parent_handle, const std::string &entry_name, const Value &value)
{
  int rc = 0;
  int entry_type = 0;
  KEY_HANDLE new_entry_handle = {0};
  const char *entry_name_ptr = entry_name.empty() ? nullptr : entry_name.c_str();

  switch (value.get_type())
  {
  case Value::Null:
    entry_type = HWTJ_NULLVALUETYPE;
    rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, nullptr, &entry_type, &new_entry_handle);
    break;

  case Value::Bool:
    entry_type = value.as_bool() ? HWTJ_TRUEVALUETYPE : HWTJ_FALSEVALUETYPE;
    rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, nullptr, &entry_type, &new_entry_handle);
    break;

  case Value::Number:
  {
    entry_type = HWTJ_NUMVALUETYPE;
    std::stringstream ss;

    if (value.is_integer())
    {
      ss << value.as_int64();
    }
    else
    {
      // Use full double precision (17 significant digits)
      ss << std::setprecision(17) << value.as_double();
    }

    std::string num_str = ss.str();
    rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, num_str.c_str(), &entry_type, &new_entry_handle);
    break;
  }

  case Value::String:
  {
    if (!value.as_string().empty())
    {
      entry_type = HWTJ_STRINGVALUETYPE;
      std::string escaped_str = escape_json_string(value.as_string());
      rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, escaped_str.c_str(), &entry_type, &new_entry_handle);
    }
    else
    {
      // Empty strings must use HWTJ_JSONTEXTVALUETYPE because HWTJ_STRINGVALUETYPE doesn't support them
      entry_type = HWTJ_JSONTEXTVALUETYPE;
      rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, "\"\"", &entry_type, &new_entry_handle);
    }
  }
  break;

  case Value::Array:
  {
    entry_type = HWTJ_JSONTEXTVALUETYPE;
    rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, "[]", &entry_type, &new_entry_handle);
    if (rc != 0)
      break;

    // Add all array elements
    const std::vector<Value> &arr = value.as_array();
    for (std::vector<Value>::const_iterator it = arr.begin(); it != arr.end(); ++it)
    {
      rc = value_to_json_instance(instance, &new_entry_handle, "", *it);
      if (rc != 0)
        break;
    }
    break;
  }

  case Value::Object:
  {
    entry_type = HWTJ_JSONTEXTVALUETYPE;
    rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, "{}", &entry_type, &new_entry_handle);
    if (rc != 0)
      break;

    // Add all object properties
    const std::map<std::string, Value> &obj = value.as_object();
    for (std::map<std::string, Value>::const_iterator it = obj.begin(); it != obj.end(); ++it)
    {
      rc = value_to_json_instance(instance, &new_entry_handle, it->first, it->second);
      if (rc != 0)
        break;
    }
    break;
  }

  default:
    entry_type = HWTJ_NULLVALUETYPE;
    rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, nullptr, &entry_type, &new_entry_handle);
    break;
  }

  return rc;
}

// Convert Value to JSON string using ZJSM API
inline std::string value_to_json_string(const Value &value)
{
  JSON_INSTANCE instance = {};
  int rc = ZJSMINIT(&instance);
  if (rc != 0)
  {
    std::stringstream ss;
    ss << std::hex << rc;
    throw Error(Error::Custom, "Failed to initialize JSON parser. RC: x'" + ss.str() + "'");
  }

  // Start with an empty root object/array/value
  KEY_HANDLE root_handle = {0};

  try
  {
    // For root level values, we need to handle them specially
    if (value.is_object() || value.is_array())
    {
      // Parse a minimal JSON to get a root container
      const char *init_json = value.is_object() ? "{}" : "[]";
      rc = ZJSMPARS(&instance, init_json);
      if (rc != 0)
      {
        ZJSMTERM(&instance);
        std::stringstream ss;
        ss << std::hex << rc;
        throw Error(Error::Custom, "Failed to parse initial JSON container. RC: x'" + ss.str() + "'");
      }

      // Clear the initial content and rebuild
      if (value.is_object())
      {
        const std::map<std::string, Value> &obj = value.as_object();
        for (std::map<std::string, Value>::const_iterator it = obj.begin(); it != obj.end(); ++it)
        {
          rc = value_to_json_instance(&instance, &root_handle, it->first, it->second);
          if (rc != 0)
          {
            ZJSMTERM(&instance);
            std::stringstream ss;
            ss << std::hex << rc;
            throw Error(Error::Custom, "Failed to serialize object property '" + it->first + "'. RC: x'" + ss.str() + "'");
          }
        }
      }
      else // Array
      {
        const std::vector<Value> &arr = value.as_array();
        for (size_t i = 0; i < arr.size(); ++i)
        {
          rc = value_to_json_instance(&instance, &root_handle, "", arr[i]);
          if (rc != 0)
          {
            ZJSMTERM(&instance);
            std::stringstream ss;
            ss << std::hex << rc;
            std::stringstream idx_ss;
            idx_ss << i;
            throw Error(Error::Custom, "Failed to serialize array element at index " + idx_ss.str() + ". RC: x'" + ss.str() + "'");
          }
        }
      }
    }
    else
    {
      throw Error(Error::Custom, "Serializing primitive values is not supported");
    }

    // Serialize the result using the same pattern as ZJson
    char buffer[1] = {0};
    int buffer_length = (int)sizeof(buffer);
    int actual_length = 0;

    rc = ZJSMSERI(&instance, buffer, &buffer_length, &actual_length);

    std::string result;
    if (rc == HWTJ_BUFFER_TOO_SMALL)
    {
      // Allocate proper buffer and retry
      std::vector<char> dynamic_buffer(actual_length);
      int dynamic_buffer_length = actual_length;
      rc = ZJSMSERI(&instance, &dynamic_buffer[0], &dynamic_buffer_length, &actual_length);
      if (rc != 0)
      {
        ZJSMTERM(&instance);
        std::stringstream ss;
        ss << std::hex << rc;
        throw Error(Error::Custom, "Failed to serialize JSON with dynamic buffer. RC: x'" + ss.str() + "'");
      }
      result = std::string(&dynamic_buffer[0], actual_length);
    }
    else if (rc == 0)
    {
      result = std::string(buffer, actual_length);
    }
    else
    {
      ZJSMTERM(&instance);
      std::stringstream ss;
      ss << std::hex << rc;
      throw Error(Error::Custom, "Failed to serialize JSON. RC: x'" + ss.str() + "'");
    }

    ZJSMTERM(&instance);

    if (result.empty())
    {
      throw Error(Error::Custom, "Serialization resulted in empty string");
    }

    return result;
  }
  catch (const Error &e)
  {
    // Re-throw zjson::Error as-is
    throw;
  }
  catch (const std::exception &e)
  {
    ZJSMTERM(&instance);
    throw Error(Error::Custom, "Standard exception during serialization: " + std::string(e.what()));
  }
  catch (...)
  {
    ZJSMTERM(&instance);
    throw Error(Error::Custom, "Unknown exception during JSON serialization");
  }
}

// JSON parser using zjsonm C API
inline Value parse_json_string(const std::string &json_str)
{
  JSON_INSTANCE instance = {};
  int rc = ZJSMINIT(&instance);
  if (rc != 0)
  {
    std::stringstream ss;
    ss << std::hex << rc;
    throw Error(Error::Custom, "Failed to initialize JSON parser for parsing. RC: x'" + ss.str() + "'");
  }

  try
  {
    rc = ZJSMPARS(&instance, json_str.c_str());
    if (rc != 0)
    {
      ZJSMTERM(&instance);
      std::stringstream ss;
      ss << std::hex << rc;
      throw Error(Error::Custom, "Failed to parse JSON string. RC: x'" + ss.str() + "'");
    }

    // Get root handle and convert to Value
    KEY_HANDLE root_handle = {0};
    Value result = json_handle_to_value(&instance, &root_handle);

    // Clean up
    ZJSMTERM(&instance);

    return result;
  }
  catch (const Error &e)
  {
    // Re-throw zjson::Error as-is
    throw;
  }
  catch (const std::exception &e)
  {
    ZJSMTERM(&instance);
    throw Error(Error::Custom, "Standard exception during JSON parsing: " + std::string(e.what()));
  }
  catch (...)
  {
    ZJSMTERM(&instance);
    throw Error(Error::Custom, "Unknown exception during JSON parsing");
  }
}

#if defined(__clang__)
/**
 * Macro system for automatic serialization
 */

// Field creation macro
#define ZJSON_FIELD(StructType, field_name) \
  zjson::Field<StructType, decltype(StructType::field_name)>(#field_name, &StructType::field_name)

// Auto-generated field creation (with inline expansion)
#define ZJSON_FIELD_AUTO(StructType, field) \
  zjson::Field<StructType, decltype(StructType::field)>(#field, &StructType::field)

// Transform macros for field lists - these create Field objects from field names
#define ZJSON_TRANSFORM_FIELDS_1(StructType, f1) \
  ZJSON_FIELD_AUTO(StructType, f1)

#define ZJSON_TRANSFORM_FIELDS_2(StructType, f1, f2) \
  ZJSON_TRANSFORM_FIELDS_1(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2)

#define ZJSON_TRANSFORM_FIELDS_3(StructType, f1, f2, f3) \
  ZJSON_TRANSFORM_FIELDS_2(StructType, f1, f2), ZJSON_FIELD_AUTO(StructType, f3)

#define ZJSON_TRANSFORM_FIELDS_4(StructType, f1, f2, f3, f4) \
  ZJSON_TRANSFORM_FIELDS_3(StructType, f1, f2, f3), ZJSON_FIELD_AUTO(StructType, f4)

#define ZJSON_TRANSFORM_FIELDS_5(StructType, f1, f2, f3, f4, f5) \
  ZJSON_TRANSFORM_FIELDS_4(StructType, f1, f2, f3, f4), ZJSON_FIELD_AUTO(StructType, f5)

#define ZJSON_TRANSFORM_FIELDS_6(StructType, f1, f2, f3, f4, f5, f6) \
  ZJSON_TRANSFORM_FIELDS_5(StructType, f1, f2, f3, f4, f5), ZJSON_FIELD_AUTO(StructType, f6)

#define ZJSON_TRANSFORM_FIELDS_7(StructType, f1, f2, f3, f4, f5, f6, f7) \
  ZJSON_TRANSFORM_FIELDS_6(StructType, f1, f2, f3, f4, f5, f6), ZJSON_FIELD_AUTO(StructType, f7)

#define ZJSON_TRANSFORM_FIELDS_8(StructType, f1, f2, f3, f4, f5, f6, f7, f8) \
  ZJSON_TRANSFORM_FIELDS_7(StructType, f1, f2, f3, f4, f5, f6, f7), ZJSON_FIELD_AUTO(StructType, f8)

#define ZJSON_TRANSFORM_FIELDS_9(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9) \
  ZJSON_TRANSFORM_FIELDS_8(StructType, f1, f2, f3, f4, f5, f6, f7, f8), ZJSON_FIELD_AUTO(StructType, f9)

#define ZJSON_TRANSFORM_FIELDS_10(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10) \
  ZJSON_TRANSFORM_FIELDS_9(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9), ZJSON_FIELD_AUTO(StructType, f10)

#define ZJSON_TRANSFORM_FIELDS_11(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11) \
  ZJSON_TRANSFORM_FIELDS_10(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10), ZJSON_FIELD_AUTO(StructType, f11)

#define ZJSON_TRANSFORM_FIELDS_12(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12) \
  ZJSON_TRANSFORM_FIELDS_11(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11), ZJSON_FIELD_AUTO(StructType, f12)

#define ZJSON_TRANSFORM_FIELDS_13(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13) \
  ZJSON_TRANSFORM_FIELDS_12(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12), ZJSON_FIELD_AUTO(StructType, f13)

#define ZJSON_TRANSFORM_FIELDS_14(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14) \
  ZJSON_TRANSFORM_FIELDS_13(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13), ZJSON_FIELD_AUTO(StructType, f14)

#define ZJSON_TRANSFORM_FIELDS_15(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15) \
  ZJSON_TRANSFORM_FIELDS_14(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14), ZJSON_FIELD_AUTO(StructType, f15)

#define ZJSON_TRANSFORM_FIELDS_16(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16) \
  ZJSON_TRANSFORM_FIELDS_15(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15), ZJSON_FIELD_AUTO(StructType, f16)

#define ZJSON_TRANSFORM_FIELDS_17(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17) \
  ZJSON_TRANSFORM_FIELDS_16(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16), ZJSON_FIELD_AUTO(StructType, f17)

#define ZJSON_TRANSFORM_FIELDS_18(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18) \
  ZJSON_TRANSFORM_FIELDS_17(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17), ZJSON_FIELD_AUTO(StructType, f18)

#define ZJSON_TRANSFORM_FIELDS_19(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19) \
  ZJSON_TRANSFORM_FIELDS_18(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18), ZJSON_FIELD_AUTO(StructType, f19)

#define ZJSON_TRANSFORM_FIELDS_20(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20) \
  ZJSON_TRANSFORM_FIELDS_19(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19), ZJSON_FIELD_AUTO(StructType, f20)

#define ZJSON_TRANSFORM_FIELDS_21(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21) \
  ZJSON_TRANSFORM_FIELDS_20(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20), ZJSON_FIELD_AUTO(StructType, f21)

#define ZJSON_TRANSFORM_FIELDS_22(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22) \
  ZJSON_TRANSFORM_FIELDS_21(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21), ZJSON_FIELD_AUTO(StructType, f22)

#define ZJSON_TRANSFORM_FIELDS_23(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23) \
  ZJSON_TRANSFORM_FIELDS_22(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22), ZJSON_FIELD_AUTO(StructType, f23)

#define ZJSON_TRANSFORM_FIELDS_24(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24) \
  ZJSON_TRANSFORM_FIELDS_23(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23), ZJSON_FIELD_AUTO(StructType, f24)

#define ZJSON_TRANSFORM_FIELDS_25(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25) \
  ZJSON_TRANSFORM_FIELDS_24(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24), ZJSON_FIELD_AUTO(StructType, f25)

#define ZJSON_TRANSFORM_FIELDS_26(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26) \
  ZJSON_TRANSFORM_FIELDS_25(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25), ZJSON_FIELD_AUTO(StructType, f26)

#define ZJSON_TRANSFORM_FIELDS_27(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27) \
  ZJSON_TRANSFORM_FIELDS_26(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26), ZJSON_FIELD_AUTO(StructType, f27)

#define ZJSON_TRANSFORM_FIELDS_28(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28) \
  ZJSON_TRANSFORM_FIELDS_27(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27), ZJSON_FIELD_AUTO(StructType, f28)

#define ZJSON_TRANSFORM_FIELDS_29(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29) \
  ZJSON_TRANSFORM_FIELDS_28(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28), ZJSON_FIELD_AUTO(StructType, f29)

#define ZJSON_TRANSFORM_FIELDS_30(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30) \
  ZJSON_TRANSFORM_FIELDS_29(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29), ZJSON_FIELD_AUTO(StructType, f30)

#define ZJSON_TRANSFORM_FIELDS_31(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31) \
  ZJSON_TRANSFORM_FIELDS_30(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30), ZJSON_FIELD_AUTO(StructType, f31)

#define ZJSON_TRANSFORM_FIELDS_32(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31, f32) \
  ZJSON_TRANSFORM_FIELDS_31(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31), ZJSON_FIELD_AUTO(StructType, f32)

// Argument counting macros for up to 32 arguments
#define ZJSON_GET_ARG_COUNT(...) ZJSON_GET_ARG_COUNT_IMPL(__VA_ARGS__,                                                        \
                                                          32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, \
                                                          15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define ZJSON_GET_ARG_COUNT_IMPL(                                          \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, N, ...) N

#define ZJSON_CONCAT(a, b) ZJSON_CONCAT_IMPL(a, b)
#define ZJSON_CONCAT_IMPL(a, b) a##b

// Main derive macro for automatic serialization
#define ZJSON_DERIVE(StructName, ...) \
  ZJSON_SERIALIZABLE(StructName, ZJSON_CONCAT(ZJSON_TRANSFORM_FIELDS_, ZJSON_GET_ARG_COUNT(__VA_ARGS__))(StructName, __VA_ARGS__))

// Core serializable macro
#define ZJSON_SERIALIZABLE(StructType, ...)                                                                 \
  namespace zjson                                                                                           \
  {                                                                                                         \
  template <>                                                                                               \
  struct Serializable<StructType>                                                                           \
  {                                                                                                         \
    static constexpr bool value = true;                                                                     \
    static Value serialize(const StructType &obj)                                                           \
    {                                                                                                       \
      Value result = Value::create_object();                                                                \
      serialize_fields(obj, result, __VA_ARGS__);                                                           \
      return result;                                                                                        \
    }                                                                                                       \
  };                                                                                                        \
  template <>                                                                                               \
  struct Deserializable<StructType>                                                                         \
  {                                                                                                         \
    static constexpr bool value = true;                                                                     \
    static zstd::expected<StructType, Error> deserialize(const Value &value)                                \
    {                                                                                                       \
      if (!value.is_object())                                                                               \
      {                                                                                                     \
        return zstd::make_unexpected(Error::invalid_type("object", "other"));                               \
      }                                                                                                     \
      StructType result{};                                                                                  \
      const auto &obj = value.as_object();                                                                  \
      bool deserialize_result = deserialize_fields(result, obj, __VA_ARGS__);                               \
      if (!deserialize_result)                                                                              \
      {                                                                                                     \
        return zstd::make_unexpected(Error::invalid_data("Failed to deserialize fields"));                  \
      }                                                                                                     \
      if (zjson::detail::StructConfig<StructType>::deny_unknown_fields)                                     \
      {                                                                                                     \
        auto validation_result = detail::validate_no_unknown_fields<StructType>(obj, __VA_ARGS__);          \
        if (!validation_result.has_value())                                                                 \
        {                                                                                                   \
          return zstd::make_unexpected(validation_result.error());                                          \
        }                                                                                                   \
      }                                                                                                     \
      return result;                                                                                        \
    }                                                                                                       \
  };                                                                                                        \
  }                                                                                                         \
  namespace                                                                                                 \
  {                                                                                                         \
  struct StructType##_Registrar                                                                             \
  {                                                                                                         \
    StructType##_Registrar()                                                                                \
    {                                                                                                       \
      zjson::SerializationRegistry<StructType>::register_serializer(                                        \
          [](const StructType &obj) { return zjson::Serializable<StructType>::serialize(obj); });           \
      zjson::SerializationRegistry<StructType>::register_deserializer(                                      \
          [](const zjson::Value &value) { return zjson::Deserializable<StructType>::deserialize(value); }); \
    }                                                                                                       \
  };                                                                                                        \
  static StructType##_Registrar StructType##_registrar_instance;                                            \
  }

// Field serialization/deserialization helper functions
namespace detail
{

// Forward declare the _impl functions
template <typename T, typename FieldType>
void serialize_field_impl(const T &obj, Value &result, const Field<T, FieldType> &field, std::true_type);

template <typename T, typename FieldType>
void serialize_field_impl(const T &obj, Value &result, const Field<T, FieldType> &field, std::false_type);

template <typename T, typename FieldType>
bool deserialize_field_impl(T &obj, const Field<T, FieldType> &field, const Value &value, std::true_type);

template <typename T, typename FieldType>
bool deserialize_field_impl(T &obj, const Field<T, FieldType> &field, const Value &value, std::false_type);

// Implementation of serialize_field_impl functions
template <typename T, typename FieldType>
void serialize_field_impl(const T &obj, Value &result, const Field<T, FieldType> &field, std::true_type)
{
  const FieldType &field_value = obj.*(field.member);

  // Handle flattening for struct types
  if (field.flatten_field)
  {
    // Serialize the nested struct and merge its fields into the parent
    Value nested_value = Serializable<FieldType>::serialize(field_value);
    if (nested_value.is_object())
    {
      const auto &nested_obj = nested_value.as_object();
      for (const auto &nested_field : nested_obj)
      {
        // Check for name collisions before adding flattened fields
        if (result.is_object())
        {
          const auto &existing_obj = result.as_object();
          if (existing_obj.find(nested_field.first) != existing_obj.end())
          {
            // Name collision detected - this should be an error to match standard JSON library behavior
            throw Error::invalid_data("Field name collision during flattening: '" + nested_field.first + "' already exists in parent struct");
          }
        }
        result.add_to_object(nested_field.first, nested_field.second);
      }
      return;
    }
  }

  // Normal serialization
  result.add_to_object(field.get_serialized_name(), Serializable<FieldType>::serialize(field_value));
}

// Specialized implementation for optional types - by default include as null
// Use .skip_if_none() field attribute to skip empty optionals
template <typename T, typename OptionalType>
void serialize_field_impl(const T &obj, Value &result, const Field<T, zstd::optional<OptionalType>> &field, std::true_type)
{
  const zstd::optional<OptionalType> &field_value = obj.*(field.member);

  // Check if field has skip_if_none attribute
  if (!field_value.has_value() && field.skip_if_none)
  {
    return; // Skip field entirely
  }

  // Otherwise, serialize normally (null if empty, value if present)
  result.add_to_object(field.get_serialized_name(), Serializable<zstd::optional<OptionalType>>::serialize(field_value));
}

template <typename T, typename FieldType>
void serialize_field_impl(const T &obj, Value &result, const Field<T, FieldType> &field, std::false_type)
{
  // Type is not serializable - skip or handle error
}

// Implementation of deserialize_field_impl functions
template <typename T, typename FieldType>
bool deserialize_field_impl(T &obj, const Field<T, FieldType> &field, const Value &value, std::true_type)
{
  auto result = Deserializable<FieldType>::deserialize(value);
  if (!result.has_value())
  {
    return false;
  }
  obj.*(field.member) = result.value();
  return true;
}

// Specialized implementation for optional types - handle null values gracefully
template <typename T, typename OptionalType>
bool deserialize_field_impl(T &obj, const Field<T, zstd::optional<OptionalType>> &field, const Value &value, std::true_type)
{
  auto result = Deserializable<zstd::optional<OptionalType>>::deserialize(value);
  if (!result.has_value())
  {
    return false;
  }
  obj.*(field.member) = result.value();
  return true;
}

template <typename T, typename FieldType>
bool deserialize_field_impl(T &obj, const Field<T, FieldType> &field, const Value &value, std::false_type)
{
  // Type is not deserializable
  return false;
}

template <typename T, typename FieldType>
void serialize_field(const T &obj, Value &result, const Field<T, FieldType> &field)
{
  if (!field.skip_serializing)
  {
    serialize_field_impl(obj, result, field, typename std::integral_constant<bool, Serializable<FieldType>::value>{});
  }
}

template <typename T, typename FieldType>
bool deserialize_field(T &obj, const std::map<std::string, Value> &object, const Field<T, FieldType> &field)
{
  if (field.skip_deserializing)
  {
    return true; // Skip field successfully
  }

  // Handle flattening for struct types during deserialization
  if (field.flatten_field)
  {
    // Create a Value object from the current object map to deserialize the nested struct
    Value flattened_value = Value::create_object();
    for (const auto &pair : object)
    {
      flattened_value.add_to_object(pair.first, pair.second);
    }
    auto result = Deserializable<FieldType>::deserialize(flattened_value);
    if (result.has_value())
    {
      obj.*(field.member) = result.value();
      return true;
    }
    return false;
  }

  auto it = object.find(field.get_serialized_name());
  if (it == object.end())
  {
    if (field.default_value)
    {
      obj.*(field.member) = field.default_value();
      return true; // Used default value
    }
    return false; // Missing required field
  }

  return deserialize_field_impl(obj, field, it->second, typename std::integral_constant<bool, Deserializable<FieldType>::value>{});
}

// Specialized deserialize_field for optional types - missing fields are OK
template <typename T, typename OptionalType>
bool deserialize_field(T &obj, const std::map<std::string, Value> &object, const Field<T, zstd::optional<OptionalType>> &field)
{
  if (field.skip_deserializing)
  {
    return true; // Skip field successfully
  }

  auto it = object.find(field.get_serialized_name());
  if (it == object.end())
  {
    if (field.default_value)
    {
      obj.*(field.member) = field.default_value();
      return true; // Used default value
    }
    // For optional fields, missing is OK - leave as empty optional
    obj.*(field.member) = zstd::optional<OptionalType>();
    return true;
  }

  return deserialize_field_impl(obj, field, it->second, typename std::integral_constant<bool, Deserializable<zstd::optional<OptionalType>>::value>{});
}

// Variadic template helpers for field processing
template <typename T>
void serialize_fields_impl(const T &obj, Value &result)
{
  // Base case - do nothing
}

template <typename T, typename Field, typename... Fields>
void serialize_fields_impl(const T &obj, Value &result, Field field, Fields... fields)
{
  serialize_field(obj, result, field);
  serialize_fields_impl(obj, result, fields...);
}

template <typename T, typename... Fields>
void serialize_fields(const T &obj, Value &result, Fields... fields)
{
  serialize_fields_impl(obj, result, fields...);
}

template <typename T>
bool deserialize_fields_impl(T &obj, const std::map<std::string, Value> &object)
{
  return true;
}

template <typename T, typename Field, typename... Fields>
bool deserialize_fields_impl(T &obj, const std::map<std::string, Value> &object, Field field, Fields... fields)
{
  if (!deserialize_field(obj, object, field))
  {
    return false;
  }
  return deserialize_fields_impl(obj, object, fields...);
}

template <typename T, typename... Fields>
bool deserialize_fields(T &obj, const std::map<std::string, Value> &object, Fields... fields)
{
  return deserialize_fields_impl(obj, object, fields...);
}

// Helper to collect field names, including flattened struct fields
template <typename Field>
void collect_field_names_impl(std::set<std::string> &names, Field field)
{
  if (!field.flatten_field)
  {
    names.insert(field.get_serialized_name());
  }
}

template <typename Field, typename... Fields>
void collect_field_names_impl(std::set<std::string> &names, Field field, Fields... fields)
{
  if (!field.flatten_field)
  {
    names.insert(field.get_serialized_name());
  }
  collect_field_names_impl(names, fields...);
}

template <typename... Fields>
void collect_field_names(std::set<std::string> &names, Fields... fields)
{
  collect_field_names_impl(names, fields...);
}

template <typename Field>
bool has_flattened_field_impl(Field field)
{
  return field.flatten_field;
}

template <typename Field, typename... Fields>
bool has_flattened_field_impl(Field field, Fields... fields)
{
  return field.flatten_field || has_flattened_field_impl(fields...);
}

template <typename... Fields>
bool has_flattened_field(Fields... fields)
{
  return has_flattened_field_impl(fields...);
}

template <typename T, typename... Fields>
zstd::expected<bool, Error> validate_no_unknown_fields(const std::map<std::string, Value> &object, Fields... fields)
{
  // Check if any field is flattened - if so, we cannot reliably validate unknown fields
  if (has_flattened_field(fields...))
  {
    // When flattening is used, we cannot perform strict unknown field validation
    // because flattened fields effectively "consume" arbitrary field names
    return true;
  }

  // Collect all expected field names
  std::set<std::string> expected_fields;
  collect_field_names(expected_fields, fields...);

  // Check for unknown fields
  for (const auto &pair : object)
  {
    if (expected_fields.find(pair.first) == expected_fields.end())
    {
      return zstd::make_unexpected(Error::unknown_field(pair.first));
    }
  }

  return true;
}

} // namespace detail

// Expose helper functions
using detail::deserialize_fields;
using detail::serialize_fields;

/**
 * Container attributes for JSON serialization
 */

// Container attribute macros

#define ZJSON_RENAME_ALL(StructType, case_style)                                                                                \
  namespace zjson                                                                                                               \
  {                                                                                                                             \
  namespace detail                                                                                                              \
  {                                                                                                                             \
  template <>                                                                                                                   \
  zjson::attributes::RenameAll::CaseStyle StructConfig<StructType>::rename_all_case = zjson::attributes::RenameAll::case_style; \
  }                                                                                                                             \
  }

#define ZJSON_DENY_UNKNOWN_FIELDS(StructType)                \
  namespace zjson                                            \
  {                                                          \
  namespace detail                                           \
  {                                                          \
  template <>                                                \
  bool StructConfig<StructType>::deny_unknown_fields = true; \
  }                                                          \
  }

// Field attribute macros
#define zjson_rename(name) .rename(name)
#define zjson_skip() .skip()
#define zjson_skip_serializing() .skip_serializing_field()
#define zjson_skip_deserializing() .skip_deserializing_field()
#define zjson_skip_serializing_if_none() .skip_serializing_if_none()
#define zjson_default(func) .with_default(func)
#define zjson_flatten() .flatten()
#else
// XLC (minimal mode) - struct serialization macros not supported
#define ZJSON_DERIVE(StructType, ...) \
  typedef zjson::ERROR_ZJSON_DERIVE_requires_xlclang_compiler ZJSON_DERIVE_ERROR

#define ZJSON_SERIALIZABLE(StructType, ...) \
  typedef zjson::ERROR_ZJSON_SERIALIZABLE_requires_xlclang_compiler ZJSON_SERIALIZABLE_ERROR
#endif

} // namespace zjson

#endif // ZJSON_HPP
