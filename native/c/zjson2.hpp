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

#ifndef ZSERDE_HPP
#define ZSERDE_HPP

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <cstring>
#include <cstdio>
#include "zjsonm.h"
#include "zjsontype.h"

// Forward declarations
namespace zserde
{
class Value;
class Error;
template <typename T>
struct Serializable;
template <typename T>
struct Deserializable;

// Result type similar to Rust's Result<T, E>
template <typename T, typename E = Error>
class Result;
} // namespace zserde

namespace zserde
{

/**
 * Error handling class similar to serde_json::Error
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

  static Error invalid_value(const std::string &msg)
  {
    return Error(InvalidValue, "Invalid value: " + msg);
  }

  static Error invalid_type(const std::string &expected, const std::string &found)
  {
    return Error(InvalidType, "Invalid type. Expected " + expected + ", found " + found);
  }

  static Error missing_field(const std::string &field)
  {
    return Error(MissingField, "Missing field: " + field);
  }

  static Error unknown_field(const std::string &field)
  {
    return Error(UnknownField, "Unknown field: " + field);
  }

  static Error invalid_data(const std::string &msg)
  {
    return Error(InvalidData, "Invalid data: " + msg);
  }
};

/**
 * Result type wrapper similar to Rust's Result<T, E>
 */
template <typename T, typename E>
class Result
{
private:
  bool is_ok_;
  union
  {
    T value_;
    E error_;
  };

public:
  Result(const T &value) : is_ok_(true), value_(value)
  {
  }
  Result(const E &error) : is_ok_(false), error_(error)
  {
  }

  Result(const Result &other) : is_ok_(other.is_ok_)
  {
    if (is_ok_)
    {
      new (&value_) T(other.value_);
    }
    else
    {
      new (&error_) E(other.error_);
    }
  }

  Result &operator=(const Result &other)
  {
    if (this != &other)
    {
      if (is_ok_)
      {
        value_.~T();
      }
      else
      {
        error_.~E();
      }

      is_ok_ = other.is_ok_;
      if (is_ok_)
      {
        new (&value_) T(other.value_);
      }
      else
      {
        new (&error_) E(other.error_);
      }
    }
    return *this;
  }

  ~Result()
  {
    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      error_.~E();
    }
  }

  bool is_ok() const
  {
    return is_ok_;
  }
  bool is_err() const
  {
    return !is_ok_;
  }

  const T &unwrap() const
  {
    if (!is_ok_)
    {
      throw std::runtime_error("Called unwrap on an error Result");
    }
    return value_;
  }

  T unwrap_or(const T &default_value) const
  {
    return is_ok_ ? value_ : default_value;
  }

  const E &unwrap_err() const
  {
    if (is_ok_)
    {
      throw std::runtime_error("Called unwrap_err on an ok Result");
    }
    return error_;
  }
};

/**
 * Value type similar to serde_json::Value
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
    result.clear();
    result.type_ = Object;
    try
    {
      result.object_value_ = new std::map<std::string, Value>();
    }
    catch (...)
    {
      result.type_ = Null;
      memset(&result.bool_value_, 0, sizeof(result.bool_value_));
      throw;
    }
    return result;
  }

  static Value create_array()
  {
    Value result;
    result.clear();
    result.type_ = Array;
    try
    {
      result.array_value_ = new std::vector<Value>();
    }
    catch (...)
    {
      result.type_ = Null;
      memset(&result.bool_value_, 0, sizeof(result.bool_value_));
      throw;
    }
    return result;
  }

  void add_to_object(const std::string &key, const Value &value)
  {
    if (type_ != Object)
    {
      throw Error::invalid_type("object", "other");
    }
    (*object_value_)[key] = value;
  }

  void add_to_array(const Value &value)
  {
    if (type_ != Array)
    {
      throw Error::invalid_type("array", "other");
    }
    array_value_->push_back(value);
  }

  void reserve_array(size_t capacity)
  {
    if (type_ != Array)
    {
      throw Error::invalid_type("array", "other");
    }
    array_value_->reserve(capacity);
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
  Type type_;
  union
  {
    bool bool_value_;
    double number_value_;
    std::string *string_value_;
    std::vector<Value> *array_value_;
    std::map<std::string, Value> *object_value_;
  };

public:
  Value() : type_(Null)
  {
    // Initialize union members safely
    memset(&bool_value_, 0, sizeof(bool_value_));
  }
  Value(bool b) : type_(Bool), bool_value_(b)
  {
  }
  Value(int i) : type_(Number), number_value_(static_cast<double>(i))
  {
  }
  Value(double d) : type_(Number), number_value_(d)
  {
  }
  Value(const std::string &s) : type_(String)
  {
    try
    {
      string_value_ = new std::string(s);
    }
    catch (...)
    {
      type_ = Null;
      memset(&bool_value_, 0, sizeof(bool_value_));
      throw;
    }
  }
  Value(const char *s) : type_(String)
  {
    try
    {
      string_value_ = new std::string(s);
    }
    catch (...)
    {
      type_ = Null;
      memset(&bool_value_, 0, sizeof(bool_value_));
      throw;
    }
  }

  Value(const Value &other) : type_(other.type_)
  {
    switch (type_)
    {
    case Bool:
      bool_value_ = other.bool_value_;
      break;
    case Number:
      number_value_ = other.number_value_;
      break;
    case String:
      string_value_ = new std::string(*other.string_value_);
      break;
    case Array:
      array_value_ = new std::vector<Value>(*other.array_value_);
      break;
    case Object:
      object_value_ = new std::map<std::string, Value>(*other.object_value_);
      break;
    default:
      break;
    }
  }

  Value &operator=(const Value &other)
  {
    if (this != &other)
    {
      clear();
      type_ = other.type_;
      switch (type_)
      {
      case Bool:
        bool_value_ = other.bool_value_;
        break;
      case Number:
        number_value_ = other.number_value_;
        break;
      case String:
        string_value_ = new std::string(*other.string_value_);
        break;
      case Array:
        array_value_ = new std::vector<Value>(*other.array_value_);
        break;
      case Object:
        object_value_ = new std::map<std::string, Value>(*other.object_value_);
        break;
      default:
        break;
      }
    }
    return *this;
  }

  ~Value()
  {
    clear();
  }

  Type get_type() const
  {
    return type_;
  }

  bool as_bool() const
  {
    if (type_ != Bool)
      throw Error::invalid_type("bool", type_name());
    return bool_value_;
  }

  double as_number() const
  {
    if (type_ != Number)
      throw Error::invalid_type("number", type_name());
    return number_value_;
  }

  int as_int() const
  {
    return static_cast<int>(as_number());
  }

  const std::string &as_string() const
  {
    if (type_ != String)
      throw Error::invalid_type("string", type_name());
    return *string_value_;
  }

  const std::vector<Value> &as_array() const
  {
    if (type_ != Array)
      throw Error::invalid_type("array", type_name());
    return *array_value_;
  }

  const std::map<std::string, Value> &as_object() const
  {
    if (type_ != Object)
      throw Error::invalid_type("object", type_name());
    return *object_value_;
  }

  bool is_null() const
  {
    return type_ == Null;
  }
  bool is_bool() const
  {
    return type_ == Bool;
  }
  bool is_number() const
  {
    return type_ == Number;
  }
  bool is_string() const
  {
    return type_ == String;
  }
  bool is_array() const
  {
    return type_ == Array;
  }
  bool is_object() const
  {
    return type_ == Object;
  }

private:
  void clear()
  {
    switch (type_)
    {
    case String:
      delete string_value_;
      break;
    case Array:
      delete array_value_;
      break;
    case Object:
      delete object_value_;
      break;
    default:
      break;
    }
  }

  std::string type_name() const
  {
    switch (type_)
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
 * Serialization trait similar to serde::Serialize
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
 * Deserialization trait similar to serde::Deserialize
 */
template <typename T>
struct Deserializable
{
  static constexpr bool value = false;

  static Result<T> deserialize(const Value &value)
  {
    static_assert(Deserializable<T>::value, "Type must implement Deserializable trait");
    return Result<T>(Error::invalid_type("deserializable", "unknown"));
  }
};

// Specializations for basic types
template <>
struct Serializable<bool>
{
  static constexpr bool value = true;
  static Value serialize(const bool &obj)
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<bool>
{
  static constexpr bool value = true;
  static Result<bool> deserialize(const Value &value)
  {
    if (!value.is_bool())
    {
      return Result<bool>(Error::invalid_type("bool", "other"));
    }
    return Result<bool>(value.as_bool());
  }
};

template <>
struct Serializable<int>
{
  static constexpr bool value = true;
  static Value serialize(const int &obj)
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<int>
{
  static constexpr bool value = true;
  static Result<int> deserialize(const Value &value)
  {
    if (!value.is_number())
    {
      return Result<int>(Error::invalid_type("number", "other"));
    }
    return Result<int>(value.as_int());
  }
};

template <>
struct Serializable<double>
{
  static constexpr bool value = true;
  static Value serialize(const double &obj)
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<double>
{
  static constexpr bool value = true;
  static Result<double> deserialize(const Value &value)
  {
    if (!value.is_number())
    {
      return Result<double>(Error::invalid_type("number", "other"));
    }
    return Result<double>(value.as_number());
  }
};

template <>
struct Serializable<std::string>
{
  static constexpr bool value = true;
  static Value serialize(const std::string &obj)
  {
    return Value(obj);
  }
};

template <>
struct Deserializable<std::string>
{
  static constexpr bool value = true;
  static Result<std::string> deserialize(const Value &value)
  {
    if (!value.is_string())
    {
      return Result<std::string>(Error::invalid_type("string", "other"));
    }
    return Result<std::string>(value.as_string());
  }
};

// Specializations for std::vector<T>
template <typename T>
struct Serializable<std::vector<T>>
{
  static constexpr bool value = Serializable<T>::value;
  static Value serialize(const std::vector<T> &vec)
  {
    Value result = Value::create_array();
    result.reserve_array(vec.size());

    for (const auto &item : vec)
    {
      result.add_to_array(Serializable<T>::serialize(item));
    }
    return result;
  }
};

template <typename T>
struct Deserializable<std::vector<T>>
{
  static constexpr bool value = Deserializable<T>::value;
  static Result<std::vector<T>> deserialize(const Value &value)
  {
    if (!value.is_array())
    {
      return Result<std::vector<T>>(Error::invalid_type("array", "other"));
    }

    std::vector<T> result;
    const auto &array = value.as_array();
    result.reserve(array.size());

    for (const auto &item : array)
    {
      auto item_result = Deserializable<T>::deserialize(item);
      if (item_result.is_err())
      {
        return Result<std::vector<T>>(item_result.unwrap_err());
      }
      result.push_back(item_result.unwrap());
    }

    return Result<std::vector<T>>(result);
  }
};

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
  std::string rename_to;
  std::function<FieldType()> default_value;

  Field(const std::string &n, FieldType T::*m)
      : name(n), member(m), skip_serializing(false), skip_deserializing(false)
  {
  }

  Field &rename(const std::string &new_name)
  {
    rename_to = new_name;
    return *this;
  }

  Field &skip()
  {
    skip_serializing = true;
    skip_deserializing = true;
    return *this;
  }

  Field &skip_serializing_field()
  {
    skip_serializing = true;
    return *this;
  }

  Field &skip_deserializing_field()
  {
    skip_deserializing = true;
    return *this;
  }

  Field &with_default(std::function<FieldType()> default_fn)
  {
    default_value = default_fn;
    return *this;
  }

  std::string get_serialized_name() const
  {
    return rename_to.empty() ? name : rename_to;
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
  using DeserializeFunc = std::function<Result<T>(const Value &)>;

  static void register_serializer(SerializeFunc func)
  {
    get_serializer() = func;
  }

  static void register_deserializer(DeserializeFunc func)
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

  static bool has_serializer()
  {
    return static_cast<bool>(get_serializer());
  }

  static bool has_deserializer()
  {
    return static_cast<bool>(get_deserializer());
  }
};

/**
 * Main serialization and deserialization functions
 */

// Helper implementation functions for C++14 compatibility
template <typename T>
Result<std::string> to_string_impl(const T &value, std::true_type)
{
  Value serialized = Serializable<T>::serialize(value);
  std::string json_str = value_to_json_string(serialized);
  return Result<std::string>(json_str);
}

template <typename T>
Result<std::string> to_string_impl(const T &value, std::false_type)
{
  if (SerializationRegistry<T>::has_serializer())
  {
    Value serialized = SerializationRegistry<T>::get_serializer()(value);
    std::string json_str = value_to_json_string(serialized);
    return Result<std::string>(json_str);
  }
  else
  {
    return Result<std::string>(Error::invalid_type("serializable", "unknown"));
  }
}

template <typename T>
Result<T> from_str_impl(const Value &parsed, std::true_type)
{
  return Deserializable<T>::deserialize(parsed);
}

template <typename T>
Result<T> from_str_impl(const Value &parsed, std::false_type)
{
  if (SerializationRegistry<T>::has_deserializer())
  {
    return SerializationRegistry<T>::get_deserializer()(parsed);
  }
  else
  {
    return Result<T>(Error::invalid_type("deserializable", "unknown"));
  }
}

// to_string function similar to serde_json::to_string
template <typename T>
Result<std::string> to_string(const T &value)
{
  try
  {
    return to_string_impl<T>(value, typename std::integral_constant<bool, Serializable<T>::value>{});
  }
  catch (const Error &e)
  {
    return Result<std::string>(e);
  }
  catch (const std::exception &e)
  {
    return Result<std::string>(Error(Error::Custom, e.what()));
  }
}

Value json_handle_to_value(JSON_INSTANCE *instance, KEY_HANDLE *key_handle)
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
      char *value_ptr = nullptr;
      int value_length = 0;
      rc = ZJSMGVAL(instance, key_handle, &value_ptr, &value_length);
      if (rc != 0)
      {
        return Value();
      }
      try
      {
        std::string str_val(value_ptr, value_length);
        double num_val = std::stod(str_val);
        return Value(num_val);
      }
      catch (...)
      {
        return Value();
      }
    }

    case HWTJ_STRING_TYPE:
    {
      char *value_ptr = nullptr;
      int value_length = 0;
      rc = ZJSMGVAL(instance, key_handle, &value_ptr, &value_length);
      if (rc != 0)
      {
        return Value();
      }
      return Value(std::string(value_ptr, value_length));
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

// from_str function similar to serde_json::from_str
template <typename T>
Result<T> from_str(const std::string &json_str)
{
  try
  {
    // Use zjsonm C API for JSON parsing
    JSON_INSTANCE instance = {0};
    int rc = ZJSMINIT(&instance);
    if (rc != 0)
    {
      return Result<T>(Error(Error::Custom, "Failed to initialize JSON parser"));
    }

    rc = ZJSMPARS(&instance, json_str.c_str());
    if (rc != 0)
    {
      ZJSMTERM(&instance);
      return Result<T>(Error(Error::Custom, "Failed to parse JSON string"));
    }

    // Get root handle and convert to Value
    KEY_HANDLE root_handle = {0};
    Value parsed = json_handle_to_value(&instance, &root_handle);

    // Clean up
    ZJSMTERM(&instance);

    return from_str_impl<T>(parsed, typename std::integral_constant<bool, Deserializable<T>::value>{});
  }
  catch (const Error &e)
  {
    return Result<T>(e);
  }
  catch (const std::exception &e)
  {
    return Result<T>(Error(Error::Custom, e.what()));
  }
}

// Helper function to add indentation to JSON string
std::string add_json_indentation(const std::string &json_str, int spaces)
{
  std::string result;
  int indent_level = 0;
  bool in_string = false;
  bool escape_next = false;

  for (char ch : json_str)
  {
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

// to_string_pretty function similar to serde_json::to_string_pretty
template <typename T>
Result<std::string> to_string_pretty(const T &value)
{
  auto result = to_string(value);
  if (result.is_err())
  {
    return result;
  }

  try
  {
    // Use our simple indentation function instead of ZJson
    std::string pretty_json = add_json_indentation(result.unwrap(), 2);
    return Result<std::string>(pretty_json);
  }
  catch (const std::exception &e)
  {
    return Result<std::string>(Error(Error::Custom, e.what()));
  }
}

// Convert Value to JSON string with proper escaping
std::string value_to_json_string(const Value &value)
{
  switch (value.get_type())
  {
  case Value::Null:
    return "null";
  case Value::Bool:
    return value.as_bool() ? "true" : "false";
  case Value::Number:
  {
    std::stringstream ss;
    ss << value.as_number();
    return ss.str();
  }
  case Value::String:
  {
    // Properly escape string according to JSON standard
    std::string escaped = "\"";
    for (char c : value.as_string())
    {
      switch (c)
      {
      case '"':
        escaped += "\\\"";
        break;
      case '\\':
        escaped += "\\\\";
        break;
      case '\b':
        escaped += "\\b";
        break;
      case '\f':
        escaped += "\\f";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      case '\0':
        escaped += "\\u0000";
        break;
      default:
        if (static_cast<unsigned char>(c) < 0x20)
        {
          // Control characters - escape as \uXXXX
          char buf[7];
          sprintf(buf, "\\u%04x", static_cast<unsigned char>(c));
          escaped += buf;
        }
        else
        {
          escaped += c;
        }
        break;
      }
    }
    escaped += "\"";
    return escaped;
  }
  case Value::Array:
  {
    std::string result = "[";
    const auto &arr = value.as_array();
    for (size_t i = 0; i < arr.size(); ++i)
    {
      if (i > 0)
        result += ",";
      result += value_to_json_string(arr[i]);
    }
    result += "]";
    return result;
  }
  case Value::Object:
  {
    std::string result = "{";
    const auto &obj = value.as_object();
    bool first = true;
    for (const auto &pair : obj)
    {
      if (!first)
        result += ",";
      first = false;

      // Escape the key name
      std::string escaped_key = "\"";
      for (char c : pair.first)
      {
        switch (c)
        {
        case '"':
          escaped_key += "\\\"";
          break;
        case '\\':
          escaped_key += "\\\\";
          break;
        case '\b':
          escaped_key += "\\b";
          break;
        case '\f':
          escaped_key += "\\f";
          break;
        case '\n':
          escaped_key += "\\n";
          break;
        case '\r':
          escaped_key += "\\r";
          break;
        case '\t':
          escaped_key += "\\t";
          break;
        default:
          if (static_cast<unsigned char>(c) < 0x20)
          {
            char buf[7];
            sprintf(buf, "\\u%04x", static_cast<unsigned char>(c));
            escaped_key += buf;
          }
          else
          {
            escaped_key += c;
          }
          break;
        }
      }
      escaped_key += "\"";

      result += escaped_key + ":" + value_to_json_string(pair.second);
    }
    result += "}";
    return result;
  }
  default:
    return "null";
  }
}

// JSON parser using zjsonm C API
Value parse_json_string(const std::string &json_str)
{
  try
  {
    JSON_INSTANCE instance = {0};
    int rc = ZJSMINIT(&instance);
    if (rc != 0)
    {
      return Value();
    }

    rc = ZJSMPARS(&instance, json_str.c_str());
    if (rc != 0)
    {
      ZJSMTERM(&instance);
      return Value();
    }

    // Get root handle and convert to Value
    KEY_HANDLE root_handle = {0};
    Value result = json_handle_to_value(&instance, &root_handle);

    // Clean up
    ZJSMTERM(&instance);

    return result;
  }
  catch (const std::exception &e)
  {
    // Return null value if parsing fails
    return Value();
  }
}

// Function moved before from_str to fix declaration order

} // namespace zserde

/**
 * Macro system for automatic serialization similar to #[derive(Serialize, Deserialize)]
 */

// Field creation macro
#define ZSERDE_FIELD(StructType, field_name) \
  zserde::Field<StructType, decltype(StructType::field_name)>(#field_name, &StructType::field_name)

// Auto-generated field creation (simplified version)
#define ZSERDE_FIELD_AUTO(StructType, field) ZSERDE_FIELD(StructType, field)

// Transform macros for field lists - these create Field objects from field names
#define ZSERDE_TRANSFORM_FIELDS_1(StructType, f1) \
  ZSERDE_FIELD_AUTO(StructType, f1)

#define ZSERDE_TRANSFORM_FIELDS_2(StructType, f1, f2) \
  ZSERDE_FIELD_AUTO(StructType, f1), ZSERDE_FIELD_AUTO(StructType, f2)

#define ZSERDE_TRANSFORM_FIELDS_3(StructType, f1, f2, f3) \
  ZSERDE_FIELD_AUTO(StructType, f1), ZSERDE_FIELD_AUTO(StructType, f2), ZSERDE_FIELD_AUTO(StructType, f3)

#define ZSERDE_TRANSFORM_FIELDS_4(StructType, f1, f2, f3, f4) \
  ZSERDE_FIELD_AUTO(StructType, f1), ZSERDE_FIELD_AUTO(StructType, f2), ZSERDE_FIELD_AUTO(StructType, f3), ZSERDE_FIELD_AUTO(StructType, f4)

#define ZSERDE_TRANSFORM_FIELDS_5(StructType, f1, f2, f3, f4, f5) \
  ZSERDE_FIELD_AUTO(StructType, f1), ZSERDE_FIELD_AUTO(StructType, f2), ZSERDE_FIELD_AUTO(StructType, f3), ZSERDE_FIELD_AUTO(StructType, f4), ZSERDE_FIELD_AUTO(StructType, f5)

#define ZSERDE_TRANSFORM_FIELDS_6(StructType, f1, f2, f3, f4, f5, f6) \
  ZSERDE_FIELD_AUTO(StructType, f1), ZSERDE_FIELD_AUTO(StructType, f2), ZSERDE_FIELD_AUTO(StructType, f3), ZSERDE_FIELD_AUTO(StructType, f4), ZSERDE_FIELD_AUTO(StructType, f5), ZSERDE_FIELD_AUTO(StructType, f6)

// Argument counting macros
#define ZSERDE_GET_ARG_COUNT(...) ZSERDE_GET_ARG_COUNT_IMPL(__VA_ARGS__, 6, 5, 4, 3, 2, 1)
#define ZSERDE_GET_ARG_COUNT_IMPL(_1, _2, _3, _4, _5, _6, N, ...) N

#define ZSERDE_CONCAT(a, b) ZSERDE_CONCAT_IMPL(a, b)
#define ZSERDE_CONCAT_IMPL(a, b) a##b

// Main derive macro - equivalent to #[derive(Serialize, Deserialize)]
#define ZSERDE_DERIVE(StructName, ...) \
  ZSERDE_SERIALIZABLE(StructName, ZSERDE_CONCAT(ZSERDE_TRANSFORM_FIELDS_, ZSERDE_GET_ARG_COUNT(__VA_ARGS__))(StructName, __VA_ARGS__))

// Core serializable macro
#define ZSERDE_SERIALIZABLE(StructType, ...)                                                                  \
  namespace zserde                                                                                            \
  {                                                                                                           \
  template <>                                                                                                 \
  struct Serializable<StructType>                                                                             \
  {                                                                                                           \
    static constexpr bool value = true;                                                                       \
    static Value serialize(const StructType &obj)                                                             \
    {                                                                                                         \
      Value result = Value::create_object();                                                                  \
      serialize_fields(obj, result, __VA_ARGS__);                                                             \
      return result;                                                                                          \
    }                                                                                                         \
  };                                                                                                          \
  template <>                                                                                                 \
  struct Deserializable<StructType>                                                                           \
  {                                                                                                           \
    static constexpr bool value = true;                                                                       \
    static Result<StructType> deserialize(const Value &value)                                                 \
    {                                                                                                         \
      if (!value.is_object())                                                                                 \
      {                                                                                                       \
        return Result<StructType>(Error::invalid_type("object", "other"));                                    \
      }                                                                                                       \
      StructType result{};                                                                                    \
      bool deserialize_result = deserialize_fields(result, value.as_object(), __VA_ARGS__);                   \
      if (!deserialize_result)                                                                                \
      {                                                                                                       \
        return Result<StructType>(Error::invalid_data("Failed to deserialize fields"));                       \
      }                                                                                                       \
      return Result<StructType>(result);                                                                      \
    }                                                                                                         \
  };                                                                                                          \
  }                                                                                                           \
  namespace                                                                                                   \
  {                                                                                                           \
  struct StructType##_Registrar                                                                               \
  {                                                                                                           \
    StructType##_Registrar()                                                                                  \
    {                                                                                                         \
      zserde::SerializationRegistry<StructType>::register_serializer(                                         \
          [](const StructType &obj) { return zserde::Serializable<StructType>::serialize(obj); });            \
      zserde::SerializationRegistry<StructType>::register_deserializer(                                       \
          [](const zserde::Value &value) { return zserde::Deserializable<StructType>::deserialize(value); }); \
    }                                                                                                         \
  };                                                                                                          \
  static StructType##_Registrar StructType##_registrar_instance;                                              \
  }

// Field serialization/deserialization helper functions need to be implemented
namespace zserde
{
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
  result.add_to_object(field.get_serialized_name(), Serializable<FieldType>::serialize(field_value));
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
  if (result.is_err())
  {
    return false;
  }
  obj.*(field.member) = result.unwrap();
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
    const FieldType &field_value = obj.*(field.member);
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

// Duplicate deserialize_field_impl functions removed - already defined above

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
  // Base case - success
  return true;
}

template <typename T, typename Field, typename... Fields>
bool deserialize_fields_impl(T &obj, const std::map<std::string, Value> &object, Field field, Fields... fields)
{
  bool result = deserialize_field(obj, object, field);
  if (!result)
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

} // namespace detail

// Expose helper functions
using detail::deserialize_fields;
using detail::serialize_fields;

} // namespace zserde

/**
 * Usage examples and convenience macros
 */

// Container attributes similar to serde container attributes
namespace zserde
{
namespace attributes
{

struct RenameAll
{
  enum CaseStyle
  {
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
    case camelCase:
    {
      std::string result;
      bool capitalize_next = false;
      for (char c : name)
      {
        if (c == '_')
        {
          capitalize_next = true;
        }
        else if (capitalize_next)
        {
          result += std::toupper(c);
          capitalize_next = false;
        }
        else
        {
          result += std::tolower(c);
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
      for (char c : name)
      {
        result += std::toupper(c);
      }
      return result;
    }
    case kebab_case:
    {
      std::string result;
      for (char c : name)
      {
        if (c == '_')
        {
          result += '-';
        }
        else
        {
          result += std::tolower(c);
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
} // namespace zserde

// Container attribute macros
#define ZSERDE_RENAME_ALL(StructType, case_style) \
  /* This would modify the field name transformation during serialization */

#define ZSERDE_DENY_UNKNOWN_FIELDS(StructType) \
  /* This would add validation during deserialization to reject unknown fields */

// Field attribute macros
#define zserde_rename(name) .rename(name)
#define zserde_skip() .skip()
#define zserde_skip_serializing() .skip_serializing_field()
#define zserde_skip_deserializing() .skip_deserializing_field()
#define zserde_default(func) .with_default(func)

#endif // ZSERDE_HPP

/*
 * USAGE EXAMPLES - ZSerde C++ API similar to Rust serde_json:
 *
 * ============================================================================
 * BASIC USAGE - Similar to Rust #[derive(Serialize, Deserialize)]
 * ============================================================================
 *
 * struct Person {
 *     std::string name;
 *     int age;
 *     bool is_active;
 * };
 *
 * // Register the type for serialization/deserialization
 * ZSERDE_DERIVE(Person, name, age, is_active);
 *
 * void basic_example() {
 *     Person person{"John Doe", 30, true};
 *
 *     // Serialize to JSON string (similar to serde_json::to_string)
 *     auto json_result = zserde::to_string(person);
 *     if (json_result.is_ok()) {
 *         std::cout << "JSON: " << json_result.unwrap() << std::endl;
 *         // Output: {"name":"John Doe","age":30,"is_active":true}
 *     }
 *
 *     // Pretty print (similar to serde_json::to_string_pretty)
 *     auto pretty_result = zserde::to_string_pretty(person);
 *     if (pretty_result.is_ok()) {
 *         std::cout << pretty_result.unwrap() << std::endl;
 *     }
 *
 *     // Deserialize from JSON string (similar to serde_json::from_str)
 *     std::string json = R"({"name":"Jane Doe","age":25,"is_active":false})";
 *     auto person_result = zserde::from_str<Person>(json);
 *     if (person_result.is_ok()) {
 *         Person p = person_result.unwrap();
 *         std::cout << "Name: " << p.name << ", Age: " << p.age << std::endl;
 *     } else {
 *         std::cout << "Error: " << person_result.unwrap_err().what() << std::endl;
 *     }
 * }
 *
 * ============================================================================
 * ADVANCED USAGE - Field Attributes (similar to #[serde(...)] attributes)
 * ============================================================================
 *
 * struct User {
 *     std::string username;
 *     std::string email;
 *     int user_id;
 *     std::string password_hash;  // should be skipped
 *     std::string display_name;   // should be renamed
 * };
 *
 * // Manual field configuration with attributes
 * ZSERDE_SERIALIZABLE(User,
 *     ZSERDE_FIELD(User, username),
 *     ZSERDE_FIELD(User, email),
 *     ZSERDE_FIELD(User, user_id).rename("userId"),
 *     ZSERDE_FIELD(User, password_hash).skip(),
 *     ZSERDE_FIELD(User, display_name).rename("displayName")
 * );
 *
 * ============================================================================
 * OPTIONAL FIELDS AND DEFAULTS (similar to #[serde(default)])
 * ============================================================================
 *
 * struct Config {
 *     std::string host;
 *     int port;
 *     bool debug_mode;
 * };
 *
 * int default_port() { return 8080; }
 * bool default_debug() { return false; }
 *
 * ZSERDE_SERIALIZABLE(Config,
 *     ZSERDE_FIELD(Config, host),
 *     ZSERDE_FIELD(Config, port).with_default(default_port),
 *     ZSERDE_FIELD(Config, debug_mode).rename("debug").with_default(default_debug)
 * );
 *
 * ============================================================================
 * NESTED STRUCTURES (automatic serialization of registered types)
 * ============================================================================
 *
 * struct Address {
 *     std::string street;
 *     std::string city;
 *     std::string country;
 * };
 *
 * struct Employee {
 *     Person person;    // Previously registered type
 *     Address address;  // Will be registered below
 *     std::string department;
 * };
 *
 * ZSERDE_DERIVE(Address, street, city, country);
 * ZSERDE_DERIVE(Employee, person, address, department);
 *
 * void nested_example() {
 *     Employee emp{
 *         {"Alice Smith", 28, true},
 *         {"123 Main St", "Boston", "USA"},
 *         "Engineering"
 *     };
 *
 *     auto json_result = zserde::to_string_pretty(emp);
 *     if (json_result.is_ok()) {
 *         std::cout << json_result.unwrap() << std::endl;
 *     }
 * }
 *
 * ============================================================================
 * ERROR HANDLING (similar to Rust Result<T, E> pattern)
 * ============================================================================
 *
 * void error_handling_example() {
 *     std::string invalid_json = R"({"name": 123, "age": "not_a_number"})";
 *
 *     auto result = zserde::from_str<Person>(invalid_json);
 *     if (result.is_err()) {
 *         const auto& error = result.unwrap_err();
 *         std::cout << "Deserialization failed: " << error.what() << std::endl;
 *         std::cout << "Error kind: " << error.kind() << std::endl;
 *     }
 *
 *     // Using unwrap_or for default values
 *     Person default_person{"Unknown", 0, false};
 *     Person person = result.unwrap_or(default_person);
 * }
 *
 * ============================================================================
 * COMPILE-TIME TYPE CHECKING (similar to Rust trait bounds)
 * ============================================================================
 *
 * template<typename T>
 * void serialize_if_possible(const T& obj) {
 *     if constexpr (zserde::Serializable<T>::value) {
 *         auto result = zserde::to_string(obj);
 *         if (result.is_ok()) {
 *             std::cout << "Serialized: " << result.unwrap() << std::endl;
 *         }
 *     } else {
 *         std::cout << "Type is not serializable" << std::endl;
 *     }
 * }
 *
 * ============================================================================
 * NATIVE z/OS JSON INTEGRATION (using z/OS Web Enablement Toolkit directly)
 * ============================================================================
 *
 * void native_json_integration_example() {
 *     // ZSerde uses z/OS HWTJIC services directly for optimal performance
 *     Person person{"John", 30, true};
 *
 *     // Convert to JSON using ZSerde (uses zjsonm C API internally)
 *     auto json_str = zserde::to_string(person).unwrap();
 *     std::cout << "JSON: " << json_str << std::endl;
 *
 *     // Parse JSON back to object (uses zjsonm C API internally)
 *     auto parsed_person = zserde::from_str<Person>(json_str);
 *     if (parsed_person.is_ok()) {
 *         Person p = parsed_person.unwrap();
 *         std::cout << "Parsed: " << p.name << ", age " << p.age << std::endl;
 *     }
 *
 *     // Pretty print JSON
 *     auto pretty_json = zserde::to_string_pretty(person).unwrap();
 *     std::cout << "Pretty JSON:\n" << pretty_json << std::endl;
 * }
 */
