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
#include <functional>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <cstring>
#include <cstdio>
#include "zjsonm.h"
#include "zjsontype.h"
#include "zstd.hpp"
#include <hwtjic.h> // ensure to include /usr/include

/*
 * USAGE EXAMPLES:
 *
 * For comprehensive usage examples demonstrating all ZJson features, see:
 * native/c/examples/jsoncpp/json.cpp
 *
 * Examples include:
 * - Basic serialization/deserialization
 * - Field attributes (rename, skip, skip_if_none, defaults)
 * - Optional fields with serde-compatible behavior
 * - Nested structures and arrays
 * - Error handling with zstd::expected
 * - Compile-time type checking
 * - Complex real-world scenarios
 *
 * Quick Reference:
 *
 * // Dynamic JSON parsing (no struct registration needed)
 * zjson::Value data = zjson::from_str(json_string);        // Default to Value
 * auto result = zjson::from_str<MyStruct>(json_string);    // Parse to specific type
 *
 * ZJSON_DERIVE(StructName, field1, field2, ...)  // Auto-generate serialization
 * ZJSON_SERIALIZABLE(StructName, ...)            // Manual field configuration
 * ZJSON_FIELD(StructName, field)                 // Field descriptor
 *
 * Field Attributes:
 * .rename("newName")         // Rename field in JSON
 * .skip()                    // Skip field entirely
 * .skip_serializing_if_none() // Skip optional field if empty (like serde's skip_serializing_if)
 * .with_default(func)       // Default value for missing fields
 *
 * Functions:
 * zjson::to_string(obj)         // Serialize to compact JSON
 * zjson::to_string_pretty(obj)  // Serialize to formatted JSON
 * zjson::from_str<T>(json)      // Deserialize from JSON string
 * zjson::to_value<T>(obj)       // Convert any type to Value
 * zjson::from_value<T>(value)   // Convert Value to any type
 *
 * Dynamic Value Access (serde_json compatible):
 * value["key"]                  // Object access by key
 * value[0]                      // Array access by index
 * value["users"][0]["name"]     // Chained access like serde_json
 * value.as_string()             // Use existing C++ methods for type conversion
 * value.as_int(), .as_number(), .as_bool(), etc.
 *
 * Optional Fields (serde-compatible):
 * zstd::optional<T> field;      // Include null by default when empty
 * ZJSON_FIELD(...) zjson_skip_serializing_if_none()  // Skip when empty
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
} // namespace zjson

namespace zjson
{

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

  inline Type get_type() const
  {
    return type_;
  }

  inline bool as_bool() const
  {
    if (type_ == Bool)
      return bool_value_;
    return false;
  }

  inline double as_number() const
  {
    if (type_ == Number)
      return number_value_;
    return 0.0;
  }

  inline int as_int() const
  {
    if (type_ == Number)
      return static_cast<int>(number_value_);
    return 0;
  }

  inline std::string as_string() const
  {
    if (type_ == String)
      return *string_value_;
    return "";
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

  inline bool is_null() const
  {
    return type_ == Null;
  }
  inline bool is_bool() const
  {
    return type_ == Bool;
  }
  inline bool is_number() const
  {
    return type_ == Number;
  }
  inline bool is_string() const
  {
    return type_ == String;
  }
  inline bool is_array() const
  {
    return type_ == Array;
  }
  inline bool is_object() const
  {
    return type_ == Object;
  }

  // Object access by key
  Value &operator[](const std::string &key)
  {
    if (type_ == Null)
    {
      // Convert null to empty object on first access
      clear();
      type_ = Object;
      object_value_ = new std::map<std::string, Value>();
    }

    if (type_ != Object)
    {
      throw Error::invalid_type("object", type_name());
    }

    return (*object_value_)[key]; // Creates entry if doesn't exist
  }

  const Value &operator[](const std::string &key) const
  {
    if (type_ != Object)
    {
      throw Error::invalid_type("object", type_name());
    }

    auto it = object_value_->find(key);
    if (it == object_value_->end())
    {
      static const Value null_value; // Return reference to static null
      return null_value;
    }

    return it->second;
  }

  // Array access by index
  Value &operator[](size_t index)
  {
    if (type_ == Null)
    {
      // Convert null to empty array on first access
      clear();
      type_ = Array;
      array_value_ = new std::vector<Value>();
    }

    if (type_ != Array)
    {
      throw Error::invalid_type("array", type_name());
    }

    // Expand array if needed
    if (index >= array_value_->size())
    {
      array_value_->resize(index + 1);
    }

    return (*array_value_)[index];
  }

  const Value &operator[](size_t index) const
  {
    if (type_ != Array)
    {
      throw Error::invalid_type("array", type_name());
    }

    if (index >= array_value_->size())
    {
      static const Value null_value; // Return reference to static null
      return null_value;
    }

    return (*array_value_)[index];
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

  inline std::string type_name() const
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
  static zstd::expected<bool, Error> deserialize(const Value &value)
  {
    if (!value.is_bool())
    {
      return zstd::make_unexpected(Error::invalid_type("bool", "other"));
    }
    return value.as_bool();
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
  static zstd::expected<int, Error> deserialize(const Value &value)
  {
    if (!value.is_number())
    {
      return zstd::make_unexpected(Error::invalid_type("number", "other"));
    }
    return value.as_int();
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
  static zstd::expected<double, Error> deserialize(const Value &value)
  {
    if (!value.is_number())
    {
      return zstd::make_unexpected(Error::invalid_type("number", "other"));
    }
    return value.as_number();
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
  static zstd::expected<std::string, Error> deserialize(const Value &value)
  {
    if (!value.is_string())
    {
      return zstd::make_unexpected(Error::invalid_type("string", "other"));
    }
    return value.as_string();
  }
};

// Specializations for Value itself - enables both to_value<Value> and from_str<Value>
template <>
struct Serializable<Value>
{
  static constexpr bool value = true;
  static Value serialize(const Value &obj)
  {
    return obj; // Value serializes to itself (identity)
  }
};

template <>
struct Deserializable<Value>
{
  static constexpr bool value = true;
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
  static zstd::expected<std::vector<T>, Error> deserialize(const Value &value)
  {
    if (!value.is_array())
    {
      return zstd::make_unexpected(Error::invalid_type("array", "other"));
    }

    std::vector<T> result;
    const auto &array = value.as_array();
    result.reserve(array.size());

    for (const auto &item : array)
    {
      auto item_result = Deserializable<T>::deserialize(item);
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
  static constexpr bool value = Serializable<T>::value;
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
  static constexpr bool value = Deserializable<T>::value;
  static zstd::expected<zstd::optional<T>, Error> deserialize(const Value &value)
  {
    if (value.is_null())
    {
      return zstd::optional<T>(); // Empty optional
    }
    else
    {
      auto result = Deserializable<T>::deserialize(value);
      if (!result.has_value())
      {
        return zstd::make_unexpected(result.error());
      }
      return zstd::optional<T>(result.value());
    }
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
  bool skip_if_none; // For optional fields - skip if no value (serde-compatible)
  std::string rename_to;
  std::function<FieldType()> default_value;

  Field(const std::string &n, FieldType T::*m)
      : name(n), member(m), skip_serializing(false), skip_deserializing(false), skip_if_none(false)
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

  inline std::string get_serialized_name() const
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
  try
  {
    if (SerializationRegistry<T>::has_serializer())
    {
      Value serialized = SerializationRegistry<T>::get_serializer()(value);
      std::string json_str = value_to_json_string(serialized);
      return json_str;
    }
    else
    {
      return zstd::make_unexpected(Error::invalid_type("serializable", "unknown"));
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
}

template <typename T>
zstd::expected<T, Error> from_str_impl(const Value &parsed, std::true_type)
{
  return Deserializable<T>::deserialize(parsed);
}

template <typename T>
zstd::expected<T, Error> from_str_impl(const Value &parsed, std::false_type)
{
  if (SerializationRegistry<T>::has_deserializer())
  {
    return SerializationRegistry<T>::get_deserializer()(parsed);
  }
  else
  {
    return zstd::make_unexpected(Error::invalid_type("deserializable", "unknown"));
  }
}

// to_string function for JSON serialization
template <typename T>
zstd::expected<std::string, Error> to_string(const T &value)
{
  try
  {
    return to_string_impl<T>(value, typename std::integral_constant<bool, Serializable<T>::value>{});
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

// from_value function - convert Value to any deserializable type
template <typename T>
zstd::expected<T, Error> from_value(const Value &value)
{
  try
  {
    return from_str_impl<T>(value, typename std::integral_constant<bool, Deserializable<T>::value>{});
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
  if (SerializationRegistry<T>::has_serializer())
  {
    return SerializationRegistry<T>::get_serializer()(obj);
  }
  else
  {
    return zstd::make_unexpected(Error::invalid_type("serializable", "unknown"));
  }
}

// to_value function - convert any serializable type to Value
template <typename T>
zstd::expected<Value, Error> to_value(const T &obj)
{
  try
  {
    return to_value_impl<T>(obj, typename std::integral_constant<bool, Serializable<T>::value>{});
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
  JSON_INSTANCE instance = {0};
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

    return from_str_impl<T>(parsed, typename std::integral_constant<bool, Deserializable<T>::value>{});
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

// to_string_pretty function for pretty-printed JSON serialization
template <typename T>
zstd::expected<std::string, Error> to_string_pretty(const T &value)
{
  auto result = to_string(value);
  if (!result.has_value())
  {
    return result;
  }

  try
  {
    // Use our simple indentation function instead of ZJson
    std::string pretty_json = add_json_indentation(result.value(), 2);
    return pretty_json;
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
    ss << value.as_number();
    std::string num_str = ss.str();
    rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, num_str.c_str(), &entry_type, &new_entry_handle);
    break;
  }

  case Value::String:
    entry_type = HWTJ_STRINGVALUETYPE;
    // ZJSMCREN handles all string escaping automatically
    rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, value.as_string().c_str(), &entry_type, &new_entry_handle);
    break;

  case Value::Array:
  {
    entry_type = HWTJ_JSONTEXTVALUETYPE;
    rc = ZJSMCREN(instance, parent_handle, entry_name_ptr, "[]", &entry_type, &new_entry_handle);
    if (rc != 0)
      break;

    // Add all array elements
    const auto &arr = value.as_array();
    for (const auto &item : arr)
    {
      rc = value_to_json_instance(instance, &new_entry_handle, "", item);
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
    const auto &obj = value.as_object();
    for (const auto &pair : obj)
    {
      rc = value_to_json_instance(instance, &new_entry_handle, pair.first, pair.second);
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
  JSON_INSTANCE instance = {0};
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
    if (value.get_type() == Value::Object || value.get_type() == Value::Array)
    {
      // Parse a minimal JSON to get a root container
      const char *init_json = (value.get_type() == Value::Object) ? "{}" : "[]";
      rc = ZJSMPARS(&instance, init_json);
      if (rc != 0)
      {
        ZJSMTERM(&instance);
        std::stringstream ss;
        ss << std::hex << rc;
        throw Error(Error::Custom, "Failed to parse initial JSON container. RC: x'" + ss.str() + "'");
      }

      // Clear the initial content and rebuild
      if (value.get_type() == Value::Object)
      {
        const auto &obj = value.as_object();
        for (const auto &pair : obj)
        {
          rc = value_to_json_instance(&instance, &root_handle, pair.first, pair.second);
          if (rc != 0)
          {
            ZJSMTERM(&instance);
            std::stringstream ss;
            ss << std::hex << rc;
            throw Error(Error::Custom, "Failed to serialize object property '" + pair.first + "'. RC: x'" + ss.str() + "'");
          }
        }
      }
      else // Array
      {
        const auto &arr = value.as_array();
        for (size_t i = 0; i < arr.size(); ++i)
        {
          rc = value_to_json_instance(&instance, &root_handle, "", arr[i]);
          if (rc != 0)
          {
            ZJSMTERM(&instance);
            std::stringstream ss;
            ss << std::hex << rc;
            throw Error(Error::Custom, "Failed to serialize array element at index " + std::to_string(i) + ". RC: x'" + ss.str() + "'");
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
      result = std::string(dynamic_buffer.data(), actual_length);
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
  JSON_INSTANCE instance = {0};
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

// Function moved before from_str to fix declaration order

} // namespace zjson

/**
 * Macro system for automatic serialization similar to #[derive(Serialize, Deserialize)]
 */

// Field creation macro
#define ZJSON_FIELD(StructType, field_name) \
  zjson::Field<StructType, decltype(StructType::field_name)>(#field_name, &StructType::field_name)

// Auto-generated field creation (simplified version)
#define ZJSON_FIELD_AUTO(StructType, field) ZJSON_FIELD(StructType, field)

// Transform macros for field lists - these create Field objects from field names
#define ZJSON_TRANSFORM_FIELDS_1(StructType, f1) \
  ZJSON_FIELD_AUTO(StructType, f1)

#define ZJSON_TRANSFORM_FIELDS_2(StructType, f1, f2) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2)

#define ZJSON_TRANSFORM_FIELDS_3(StructType, f1, f2, f3) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3)

#define ZJSON_TRANSFORM_FIELDS_4(StructType, f1, f2, f3, f4) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4)

#define ZJSON_TRANSFORM_FIELDS_5(StructType, f1, f2, f3, f4, f5) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5)

#define ZJSON_TRANSFORM_FIELDS_6(StructType, f1, f2, f3, f4, f5, f6) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6)

#define ZJSON_TRANSFORM_FIELDS_7(StructType, f1, f2, f3, f4, f5, f6, f7) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7)

#define ZJSON_TRANSFORM_FIELDS_8(StructType, f1, f2, f3, f4, f5, f6, f7, f8) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8)

#define ZJSON_TRANSFORM_FIELDS_9(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9)

#define ZJSON_TRANSFORM_FIELDS_10(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10)

#define ZJSON_TRANSFORM_FIELDS_11(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11)

#define ZJSON_TRANSFORM_FIELDS_12(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12)

#define ZJSON_TRANSFORM_FIELDS_13(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13)

#define ZJSON_TRANSFORM_FIELDS_14(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14)

#define ZJSON_TRANSFORM_FIELDS_15(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15)

#define ZJSON_TRANSFORM_FIELDS_16(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16)

#define ZJSON_TRANSFORM_FIELDS_17(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17)

#define ZJSON_TRANSFORM_FIELDS_18(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18)

#define ZJSON_TRANSFORM_FIELDS_19(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19)

#define ZJSON_TRANSFORM_FIELDS_20(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20)

#define ZJSON_TRANSFORM_FIELDS_21(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21)

#define ZJSON_TRANSFORM_FIELDS_22(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22)

#define ZJSON_TRANSFORM_FIELDS_23(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23)

#define ZJSON_TRANSFORM_FIELDS_24(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23), ZJSON_FIELD_AUTO(StructType, f24)

#define ZJSON_TRANSFORM_FIELDS_25(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23), ZJSON_FIELD_AUTO(StructType, f24), ZJSON_FIELD_AUTO(StructType, f25)

#define ZJSON_TRANSFORM_FIELDS_26(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23), ZJSON_FIELD_AUTO(StructType, f24), ZJSON_FIELD_AUTO(StructType, f25), ZJSON_FIELD_AUTO(StructType, f26)

#define ZJSON_TRANSFORM_FIELDS_27(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23), ZJSON_FIELD_AUTO(StructType, f24), ZJSON_FIELD_AUTO(StructType, f25), ZJSON_FIELD_AUTO(StructType, f26), ZJSON_FIELD_AUTO(StructType, f27)

#define ZJSON_TRANSFORM_FIELDS_28(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23), ZJSON_FIELD_AUTO(StructType, f24), ZJSON_FIELD_AUTO(StructType, f25), ZJSON_FIELD_AUTO(StructType, f26), ZJSON_FIELD_AUTO(StructType, f27), ZJSON_FIELD_AUTO(StructType, f28)

#define ZJSON_TRANSFORM_FIELDS_29(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23), ZJSON_FIELD_AUTO(StructType, f24), ZJSON_FIELD_AUTO(StructType, f25), ZJSON_FIELD_AUTO(StructType, f26), ZJSON_FIELD_AUTO(StructType, f27), ZJSON_FIELD_AUTO(StructType, f28), ZJSON_FIELD_AUTO(StructType, f29)

#define ZJSON_TRANSFORM_FIELDS_30(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23), ZJSON_FIELD_AUTO(StructType, f24), ZJSON_FIELD_AUTO(StructType, f25), ZJSON_FIELD_AUTO(StructType, f26), ZJSON_FIELD_AUTO(StructType, f27), ZJSON_FIELD_AUTO(StructType, f28), ZJSON_FIELD_AUTO(StructType, f29), ZJSON_FIELD_AUTO(StructType, f30)

#define ZJSON_TRANSFORM_FIELDS_31(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23), ZJSON_FIELD_AUTO(StructType, f24), ZJSON_FIELD_AUTO(StructType, f25), ZJSON_FIELD_AUTO(StructType, f26), ZJSON_FIELD_AUTO(StructType, f27), ZJSON_FIELD_AUTO(StructType, f28), ZJSON_FIELD_AUTO(StructType, f29), ZJSON_FIELD_AUTO(StructType, f30), ZJSON_FIELD_AUTO(StructType, f31)

#define ZJSON_TRANSFORM_FIELDS_32(StructType, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31, f32) \
  ZJSON_FIELD_AUTO(StructType, f1), ZJSON_FIELD_AUTO(StructType, f2), ZJSON_FIELD_AUTO(StructType, f3), ZJSON_FIELD_AUTO(StructType, f4), ZJSON_FIELD_AUTO(StructType, f5), ZJSON_FIELD_AUTO(StructType, f6), ZJSON_FIELD_AUTO(StructType, f7), ZJSON_FIELD_AUTO(StructType, f8), ZJSON_FIELD_AUTO(StructType, f9), ZJSON_FIELD_AUTO(StructType, f10), ZJSON_FIELD_AUTO(StructType, f11), ZJSON_FIELD_AUTO(StructType, f12), ZJSON_FIELD_AUTO(StructType, f13), ZJSON_FIELD_AUTO(StructType, f14), ZJSON_FIELD_AUTO(StructType, f15), ZJSON_FIELD_AUTO(StructType, f16), ZJSON_FIELD_AUTO(StructType, f17), ZJSON_FIELD_AUTO(StructType, f18), ZJSON_FIELD_AUTO(StructType, f19), ZJSON_FIELD_AUTO(StructType, f20), ZJSON_FIELD_AUTO(StructType, f21), ZJSON_FIELD_AUTO(StructType, f22), ZJSON_FIELD_AUTO(StructType, f23), ZJSON_FIELD_AUTO(StructType, f24), ZJSON_FIELD_AUTO(StructType, f25), ZJSON_FIELD_AUTO(StructType, f26), ZJSON_FIELD_AUTO(StructType, f27), ZJSON_FIELD_AUTO(StructType, f28), ZJSON_FIELD_AUTO(StructType, f29), ZJSON_FIELD_AUTO(StructType, f30), ZJSON_FIELD_AUTO(StructType, f31), ZJSON_FIELD_AUTO(StructType, f32)

// Argument counting macros for up to 32 arguments
#define ZJSON_GET_ARG_COUNT(...) ZJSON_GET_ARG_COUNT_IMPL(__VA_ARGS__,                                                        \
                                                          32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, \
                                                          15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define ZJSON_GET_ARG_COUNT_IMPL(                                          \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, N, ...) N

#define ZJSON_CONCAT(a, b) ZJSON_CONCAT_IMPL(a, b)
#define ZJSON_CONCAT_IMPL(a, b) a##b

// Main derive macro - equivalent to #[derive(Serialize, Deserialize)]
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
      bool deserialize_result = deserialize_fields(result, value.as_object(), __VA_ARGS__);                 \
      if (!deserialize_result)                                                                              \
      {                                                                                                     \
        return zstd::make_unexpected(Error::invalid_data("Failed to deserialize fields"));                  \
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

// Field serialization/deserialization helper functions need to be implemented
namespace zjson
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

// Specialized implementation for optional types - by default include as null (serde-compatible)
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

} // namespace zjson

/**
 * Usage examples and convenience macros
 */

// Container attributes for JSON serialization
namespace zjson
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
} // namespace zjson

// Container attribute macros
#define ZJSON_RENAME_ALL(StructType, case_style) \
  /* This would modify the field name transformation during serialization */

#define ZJSON_DENY_UNKNOWN_FIELDS(StructType) \
  /* This would add validation during deserialization to reject unknown fields */

// Field attribute macros
#define zjson_rename(name) .rename(name)
#define zjson_skip() .skip()
#define zjson_skip_serializing() .skip_serializing_field()
#define zjson_skip_deserializing() .skip_deserializing_field()
#define zjson_skip_serializing_if_none() .skip_serializing_if_none()
#define zjson_default(func) .with_default(func)

#endif // ZJSON_HPP
