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

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <type_traits>
#include <functional>
#include <hwtjic.h> // ensure to include /usr/include
#include "zjsonm.h"
#include "zjsontype.h"
#include "zlogger.hpp"

// Simplified macro to reduce repetition - automatically applies struct name to all fields
// Uses a much simpler approach that just transforms the field list

#define ZJSON_FIELD_AUTO(StructName, field) ZJSON_FIELD(StructName, field)

// Transform macro that converts field list to ZJSON_FIELD calls
#define ZJSON_TRANSFORM_FIELDS_1(StructName, f1) \
  ZJSON_FIELD_AUTO(StructName, f1)

#define ZJSON_TRANSFORM_FIELDS_2(StructName, f1, f2) \
  ZJSON_FIELD_AUTO(StructName, f1), ZJSON_FIELD_AUTO(StructName, f2)

#define ZJSON_TRANSFORM_FIELDS_3(StructName, f1, f2, f3) \
  ZJSON_FIELD_AUTO(StructName, f1), ZJSON_FIELD_AUTO(StructName, f2), ZJSON_FIELD_AUTO(StructName, f3)

#define ZJSON_TRANSFORM_FIELDS_4(StructName, f1, f2, f3, f4) \
  ZJSON_FIELD_AUTO(StructName, f1), ZJSON_FIELD_AUTO(StructName, f2), ZJSON_FIELD_AUTO(StructName, f3), ZJSON_FIELD_AUTO(StructName, f4)

#define ZJSON_TRANSFORM_FIELDS_5(StructName, f1, f2, f3, f4, f5) \
  ZJSON_FIELD_AUTO(StructName, f1), ZJSON_FIELD_AUTO(StructName, f2), ZJSON_FIELD_AUTO(StructName, f3), ZJSON_FIELD_AUTO(StructName, f4), ZJSON_FIELD_AUTO(StructName, f5)

// Count arguments and dispatch to appropriate macro
#define ZJSON_GET_ARG_COUNT(...) ZJSON_GET_ARG_COUNT_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)
#define ZJSON_GET_ARG_COUNT_IMPL(_1, _2, _3, _4, _5, N, ...) N

#define ZJSON_CONCAT(a, b) ZJSON_CONCAT_IMPL(a, b)
#define ZJSON_CONCAT_IMPL(a, b) a##b

// Main macro - works with up to 5 fields (easily extendable)
#define ZJSON_AUTO_SERIALIZABLE(StructName, ...) \
  ZJSON_SERIALIZABLE(StructName, ZJSON_CONCAT(ZJSON_TRANSFORM_FIELDS_, ZJSON_GET_ARG_COUNT(__VA_ARGS__))(StructName, __VA_ARGS__))

/*
 * USAGE EXAMPLE:
 *
 * // Define your struct/class
 * struct Person {
 *     std::string name;
 *     int age;
 *     bool isActive;
 * };
 *
 * struct Address {
 *     std::string street;
 *     std::string city;
 *     int zipCode;
 * };
 *
 * struct Employee {
 *     Person person;
 *     Address address;
 *     std::string department;
 * };
 *
 * // Register your types for serialization
 * ZJSON_SERIALIZABLE(Person,
 *     ZJSON_FIELD(Person, name),
 *     ZJSON_FIELD(Person, age),
 *     ZJSON_FIELD(Person, isActive)
 * );
 *
 * ZJSON_SERIALIZABLE(Address,
 *     ZJSON_FIELD(Address, street),
 *     ZJSON_FIELD(Address, city),
 *     ZJSON_FIELD(Address, zipCode)
 * );
 *
 * ZJSON_SERIALIZABLE(Employee,
 *     ZJSON_FIELD(Employee, person),
 *     ZJSON_FIELD(Employee, address),
 *     ZJSON_FIELD(Employee, department)
 * );
 *
 * // Usage:
 * void example() {
 *     std::string json_str = R"({
 *         "name": "John Doe",
 *         "age": 30,
 *         "isActive": true
 *     })";
 *
 *     ZJson json;
 *     json.parse(json_str);
 *
 *     // Type-safe deserialization with compile-time checking
 *     Person person = json.serialize<Person>();
 *
 *     std::cout << "Name: " << person.name << std::endl;
 *     std::cout << "Age: " << person.age << std::endl;
 *     std::cout << "Active: " << std::boolalpha << person.isActive << std::endl;
 *
 *     // Check if type can be serialized at compile time
 *     static_assert(ZJson::canSerialize<Person>(), "Person should be serializable");
 * }
 */

// Forward declaration
class ZJson;

// Type traits for compile-time type checking
template <typename T>
struct is_serializable : std::false_type
{
};

template <typename T>
struct is_arithmetic_or_string : std::integral_constant<bool,
                                                        std::is_arithmetic<T>::value || std::is_same<T, std::string>::value>
{
};

// Field descriptor for reflection
template <typename T, typename FieldType>
struct FieldDescriptor
{
  std::string name;
  FieldType T::*member;

  FieldDescriptor(const std::string &n, FieldType T::*m)
      : name(n), member(m)
  {
  }
};

// Forward declaration for SerializationRegistry
template <typename T>
class SerializationRegistry;

// Macro definition moved after ZJson class - see below

// Forward declarations
class ZJson;

// Helper macro for creating field descriptors
#define ZJSON_FIELD(StructType, field_name) \
  FieldDescriptor<StructType, decltype(StructType::field_name)>(#field_name, &StructType::field_name)

// JSON type constants (from z/OS HWTJIC documentation)
#ifndef HWTJ_OBJECT_TYPE
#define HWTJ_OBJECT_TYPE 1
#endif
#ifndef HWTJ_ARRAY_TYPE
#define HWTJ_ARRAY_TYPE 2
#endif
#ifndef HWTJ_STRING_TYPE
#define HWTJ_STRING_TYPE 3
#endif
#ifndef HWTJ_NUMBER_TYPE
#define HWTJ_NUMBER_TYPE 4
#endif
#ifndef HWTJ_BOOLEAN_TYPE
#define HWTJ_BOOLEAN_TYPE 5
#endif
#ifndef HWTJ_NULL_TYPE
#define HWTJ_NULL_TYPE 6
#endif

// Additional constants
#ifndef HWTJ_BUFFER_TOO_SMALL
#define HWTJ_BUFFER_TOO_SMALL 0x406
#endif
#ifndef HWTJ_NOFORCE
#define HWTJ_NOFORCE 0
#endif

class ZJson
{

private:
  JSON_INSTANCE instance;
  char *dynamic_buffer;
  int dynamic_buffer_length;

public:
  class JsonValueProxy
  {
    friend class ZJson;

  private:
    ZJson &parser;
    std::string key;
    mutable KEY_HANDLE key_handle = {0};
    JsonValueProxy *parent = nullptr;

    // Constructor for JSON root level
    JsonValueProxy(ZJson &parser, KEY_HANDLE root_kh)
        : parser(parser), key(""), key_handle(root_kh)
    {
    }

    // Constructor for individual keys
    JsonValueProxy(ZJson &parser, const std::string &key, JsonValueProxy *parent, const std::string &search_key)
        : parser(parser), key(key), parent(parent)
    {
      int type = HWTJ_SEARCHTYPE_SHALLOW;
      KEY_HANDLE starting_handle = {0};

      ZJSMSRCH(&parser.instance, &type, search_key.c_str(), &parent->key_handle, &starting_handle, &key_handle);
    }

    // Constructor for array entries
    JsonValueProxy(ZJson &parser, const std::string &key, KEY_HANDLE parent_kh)
        : parser(parser), key(key), key_handle(parent_kh)
    {
    }

  public:
    explicit operator std::string() const
    {
      int rc = 0;
      char *value_ptr = nullptr;
      int value_length = 0;

      rc = ZJSMGVAL(&parser.instance, get_mutable_key_handle(), &value_ptr, &value_length);
      if (0 != rc)
      {
        throw format_error("Error getting string value for key '" + key + "'", rc);
      }
      return std::string(value_ptr, value_length);
    }

    explicit operator int() const
    {
      int rc = 0;
      char *value_ptr = nullptr;
      int value_length = 0;

      rc = ZJSMGVAL(&parser.instance, get_mutable_key_handle(), &value_ptr, &value_length);
      if (0 != rc)
      {
        throw format_error("Error getting integer value for key '" + key + "'", rc);
      }
      return std::strtol(value_ptr, nullptr, 10);
    }

    explicit operator bool() const
    {
      int rc = 0;
      char b_val = 0;

      rc = ZJSMGBOV(&parser.instance, get_mutable_key_handle(), &b_val);
      if (0 != rc)
      {
        throw format_error("Error getting boolean value for key '" + key + "'", rc);
      }
      return (b_val == HWTJ_TRUE);
    }

    int getType() const
    {
      int rc = 0;
      int type = 0;
      rc = ZJSNGJST(&parser.instance, get_mutable_key_handle(), &type);
      if (0 != rc)
      {
        throw format_error("Error getting type for key '" + key + "'", rc);
      }
      return type;
    }

    std::string getKeyPath() const
    {
      return key;
    }

    std::string getKey() const
    {
      size_t pos = key.find_last_of('.');
      if (pos != std::string::npos)
      {
        return key.substr(pos + 1);
      }
      return key;
    }

    std::vector<std::string> getKeys() const
    {
      std::vector<std::string> keys;
      int type = this->getType();

      if (type != HWTJ_OBJECT_TYPE) // not an object
      {
        throw std::runtime_error("Cannot get keys from non-object type for key '" + key + "'. Type was " + std::to_string(type));
      }

      int rc = 0;
      int number_entries = 0;

      rc = ZJSMGNUE(&parser.instance, get_mutable_key_handle(), &number_entries);
      if (0 != rc)
      {
        throw format_error("Error getting number of entries for key '" + key + "'", rc);
      }

      for (int i = 0; i < number_entries; i++)
      {
        char buffer[1] = {0};
        char *buffer_ptr = buffer;
        int buffer_length = (int)sizeof(buffer);
        KEY_HANDLE value_handle = {0};
        int actual_length = 0;
        char *dynamic_buffer = nullptr;

        rc = ZJSMGOEN(&parser.instance, get_mutable_key_handle(), &i, &buffer_ptr, &buffer_length, &value_handle, &actual_length);

        // if the buffer is too small, allocate a dynamic buffer
        if (HWTJ_BUFFER_TOO_SMALL == rc)
        {
          // dynamic_buffer = new char[actual_length];
          auto dynamic_buffer = std::vector<char>(actual_length);

          buffer_ptr = &dynamic_buffer[0];
          buffer_length = actual_length;
          rc = ZJSMGOEN(&parser.instance, get_mutable_key_handle(), &i, &buffer_ptr, &buffer_length, &value_handle, &actual_length);
          keys.push_back(std::string(dynamic_buffer.begin(), dynamic_buffer.end()));
        }

        else if (0 == rc)
        {
          keys.push_back(std::string(buffer_ptr, actual_length));
        }

        else
        {
          throw format_error("Error getting key at index " + std::to_string(i) + " for key '" + key + "'", rc);
        }
      }

      return keys;
    }

    JsonValueProxy operator[](int index) const
    {
      int type = this->getType();
      if (type != HWTJ_ARRAY_TYPE) // not an array
      {
        throw std::runtime_error("Cannot apply operator[] to a non-array type for key '" + key + "'. Type was " + std::to_string(type));
      }

      int rc = 0;
      KEY_HANDLE element_handle = {0};
      int mutable_index = index;

      rc = ZJSMGAEN(&parser.instance, get_mutable_key_handle(), &mutable_index, &element_handle);
      if (0 != rc)
      {
        throw format_error("Error getting array element at index " + std::to_string(index) + " for key '" + key + "'.", rc);
      }

      std::string new_key = key + "[" + std::to_string(index) + "]";
      return JsonValueProxy(parser, new_key, element_handle);
    }

    JsonValueProxy operator[](const std::string &index_str) const
    {
      // Check if this is an array access (numeric string) or object access
      int type = this->getType();
      if (type == HWTJ_ARRAY_TYPE) // Array type
      {
        return (*this)[std::stoi(index_str)];
      }
      else if (type == HWTJ_OBJECT_TYPE) // Object type
      {
        // Create a nested search proxy
        std::string nested_key = key + "." + index_str;
        return JsonValueProxy(parser, nested_key, const_cast<JsonValueProxy *>(this), index_str);
      }
      else
      {
        throw std::runtime_error("Cannot apply string operator[] to type " + std::to_string(type) + " for key '" + key + "'");
      }
    }

    // Assignment operators for different types
    JsonValueProxy &operator=(const std::string &value)
    {
      setValue(value, HWTJ_STRING_TYPE);
      return *this;
    }

    JsonValueProxy &operator=(const char *value)
    {
      setValue(std::string(value), HWTJ_STRING_TYPE);
      return *this;
    }

    JsonValueProxy &operator=(int value)
    {
      setValue(std::to_string(value), HWTJ_NUMBER_TYPE);
      return *this;
    }

    JsonValueProxy &operator=(double value)
    {
      setValue(std::to_string(value), HWTJ_NUMBER_TYPE);
      return *this;
    }

    JsonValueProxy &operator=(bool value)
    {
      setValue(std::to_string(value), HWTJ_BOOLEAN_TYPE);
      return *this;
    }

  private:
    void erase(const std::string &key_name)
    {
      int type = HWTJ_SEARCHTYPE_SHALLOW;
      KEY_HANDLE starting_handle = {0};
      KEY_HANDLE found_entry_handle = {0};

      int rc = ZJSMSRCH(&parser.instance, &type, key_name.c_str(), &key_handle, &starting_handle, &found_entry_handle);
      if (0 == rc)
      {
        ZJSMDEL(&parser.instance, &key_handle, &found_entry_handle);
      }
    }

    void setValue(const std::string &value, int entry_type)
    {
      // Delete existing key from parent
      auto key = getKey();
      parent->erase(key);

      // Create the new entry
      int rc = ZJSMCREN(&parser.instance, &parent->key_handle, key.c_str(), value.c_str(), &entry_type, &key_handle);
      if (0 != rc)
      {
        throw format_error("Error setting value for key '" + key + "'", rc);
      }
    }

    std::runtime_error format_error(const std::string &msg, int rc) const
    {
      std::stringstream ss;
      ss << std::hex << rc;
      return std::runtime_error(msg + " rc was x'" + ss.str() + "'");
    }

    KEY_HANDLE *get_mutable_key_handle() const
    {
      return &key_handle;
    }
  };

  ZJson()
  {
    int rc = 0;
    memset(&instance, 0, sizeof(JSON_INSTANCE));
    rc = ZJSMINIT(&instance);
    if (0 != rc)
    {
      std::stringstream ss;
      ss << std::hex << rc;
      throw std::runtime_error("Error initializing JSON rc was x'" + ss.str() + "'");
    }
  }
  ~ZJson()
  {
    int rc = 0;
    rc = ZJSMTERM(&instance);
    if (0 != rc)
    {
      std::stringstream ss;
      ss << std::hex << rc;
      throw std::runtime_error("Error terminating JSON rc was x'" + ss.str() + "'");
    }
  }

  JsonValueProxy parse(const std::string &json)
  {
    int rc = 0;
    if (instance.json != nullptr)
    {
      rc = ZJSMTERM(&instance);
      if (0 != rc)
      {
        std::stringstream ss;
        ss << std::hex << rc;
        throw std::runtime_error("Error terminating JSON rc was x'" + ss.str() + "'");
      }
      memset(&instance, 0, sizeof(JSON_INSTANCE));
      rc = ZJSMINIT(&instance);
      if (0 != rc)
      {
        std::stringstream ss;
        ss << std::hex << rc;
        throw std::runtime_error("Error initializing JSON rc was x'" + ss.str() + "'");
      }
    }
    rc = ZJSMPARS(&instance, json.c_str());
    if (0 != rc)
    {
      std::stringstream ss;
      ss << std::hex << rc;
      throw std::runtime_error("Error parsing JSON rc was x'" + ss.str() + "'");
    }

    // Return a proxy representing the root object
    KEY_HANDLE root_handle = {0};
    return JsonValueProxy(*this, root_handle);
  }

  std::string stringify()
  {
    int rc = 0;

    if (nullptr != dynamic_buffer)
    {
      delete[] dynamic_buffer;
      dynamic_buffer = nullptr;
      dynamic_buffer_length = 0;
    }

    char buffer[1] = {0};
    int buffer_length = (int)sizeof(buffer);
    int buffer_length_actual = 0;

    rc = ZJSMSERI(&instance, buffer, &buffer_length, &buffer_length_actual);

    // expect this failure on the first call to ZJSMSERI
    if (HWTJ_BUFFER_TOO_SMALL == rc)
    {
      dynamic_buffer_length = buffer_length_actual;
      dynamic_buffer = new char[dynamic_buffer_length];
      rc = ZJSMSERI(&instance, dynamic_buffer, &dynamic_buffer_length, &buffer_length_actual);
    }

    // if the rc is still not zero or not the previous expected failure, throw an error
    if (0 != rc)
    {
      std::stringstream ss;
      ss << std::hex << rc;
      throw std::runtime_error("Error serializing JSON rc was x'" + ss.str() + "'");
    }

    return std::string(dynamic_buffer, dynamic_buffer_length);
  }

  std::string stringify(int space)
  {
    std::string minified_json = this->stringify();
    std::string indented_json;
    int indent_level = 0;
    bool in_string = false;

    for (char ch : minified_json)
    {
      if (ch == '\"')
      {
        indented_json += ch;
        in_string = !in_string;
      }
      else if (!in_string)
      {
        switch (ch)
        {
        case '{':
        case '[':
          indented_json += ch;
          indented_json += '\n';
          indent_level++;
          indented_json.append(indent_level * space, ' ');
          break;
        case '}':
        case ']':
          indented_json += '\n';
          indent_level--;
          indented_json.append(indent_level * space, ' ');
          indented_json += ch;
          break;
        case ',':
          indented_json += ch;
          indented_json += '\n';
          indented_json.append(indent_level * space, ' ');
          break;
        case ':':
          indented_json += ch;
          indented_json += ' ';
          break;
        default:
          indented_json += ch;
          break;
        }
      }
      else
      {
        indented_json += ch;
      }
    }
    return indented_json;
  }

  // TODO(TAJ): Figure out how this overload is used and fix parameters
  JsonValueProxy operator[](const std::string &key)
  {
    return JsonValueProxy(*this, key);
  }

  // Type-safe deserialization method
  template <typename T>
  T serialize() const
  {
    static_assert(is_serializable<T>::value,
                  "Type must be registered with ZJSON_SERIALIZABLE macro");

    T result{};
    auto &deserializer = SerializationRegistry<T>::get_instance().getDeserializer();

    if (!deserializer)
    {
      throw std::runtime_error("No deserializer registered for type");
    }

    // Get the root object
    KEY_HANDLE root_handle = {0};
    JsonValueProxy root_proxy(*const_cast<ZJson *>(this), "root", root_handle);

    deserializer(result, root_proxy);
    return result;
  }

  // Overload for deserializing from a specific JSON path
  template <typename T>
  T serialize(const std::string &path) const
  {
    static_assert(is_serializable<T>::value,
                  "Type must be registered with ZJSON_SERIALIZABLE macro");

    T result{};
    auto &deserializer = SerializationRegistry<T>::get_instance().getDeserializer();

    if (!deserializer)
    {
      throw std::runtime_error("No deserializer registered for type");
    }

    // Get the specified path
    JsonValueProxy path_proxy = (*const_cast<ZJson *>(this))[path];

    deserializer(result, path_proxy);
    return result;
  }

  // Helper method to check if a type can be serialized
  template <typename T>
  static constexpr bool canSerialize()
  {
    return is_serializable<T>::value && SerializationRegistry<T>::get_instance().hasDeserializer();
  }
};

// Macro for easy struct registration (after ZJson class definition)
#define ZJSON_SERIALIZABLE(StructType, ...)                                                                        \
  template <>                                                                                                      \
  struct is_serializable<StructType> : std::true_type                                                              \
  {                                                                                                                \
  };                                                                                                               \
  namespace                                                                                                        \
  {                                                                                                                \
  struct StructType##_Registrar                                                                                    \
  {                                                                                                                \
    StructType##_Registrar()                                                                                       \
    {                                                                                                              \
      SerializationRegistry<StructType>::registerDeserializer(                                                     \
          [](StructType &obj, const ZJson::JsonValueProxy &json) { deserialize_fields(obj, json, __VA_ARGS__); }); \
    }                                                                                                              \
  };                                                                                                               \
  static StructType##_Registrar StructType##_registrar_instance;                                                   \
  }

// Serialization registry implementation (after ZJson class definition)
#include "singleton.hpp"
template <typename T>
class SerializationRegistry : public Singleton<SerializationRegistry<T>>
{
  friend class Singleton<SerializationRegistry<T>>;
  using DeserializerFunc = std::function<void(T &, const ZJson::JsonValueProxy &)>;
  DeserializerFunc m_deserializeFn;

public:
  void registerDeserializer(DeserializerFunc func)
  {
    SerializationRegistry<T>::get_instance().m_deserializeFn = func;
  }

  DeserializerFunc &getDeserializer()
  {
    return SerializationRegistry<T>::get_instance().m_deserializeFn;
  }

  bool hasDeserializer()
  {
    return SerializationRegistry<T>::get_instance().m_deserializeFn != nullptr;
  }
};

// Template implementations (after ZJson class definition)
template <typename T, typename FieldType>
typename std::enable_if<std::is_same<FieldType, std::string>::value>::type
deserialize_field_impl(T &obj, const ZJson::JsonValueProxy &json, const FieldDescriptor<T, FieldType> &field)
{
  obj.*(field.member) = static_cast<std::string>(json[field.name]);
}

template <typename T, typename FieldType>
typename std::enable_if<std::is_same<FieldType, int>::value>::type
deserialize_field_impl(T &obj, const ZJson::JsonValueProxy &json, const FieldDescriptor<T, FieldType> &field)
{
  obj.*(field.member) = static_cast<int>(json[field.name]);
}

template <typename T, typename FieldType>
typename std::enable_if<std::is_same<FieldType, bool>::value>::type
deserialize_field_impl(T &obj, const ZJson::JsonValueProxy &json, const FieldDescriptor<T, FieldType> &field)
{
  obj.*(field.member) = static_cast<bool>(json[field.name]);
}

template <typename T, typename FieldType>
typename std::enable_if<std::is_arithmetic<FieldType>::value &&
                        !std::is_same<FieldType, int>::value &&
                        !std::is_same<FieldType, bool>::value>::type
deserialize_field_impl(T &obj, const ZJson::JsonValueProxy &json, const FieldDescriptor<T, FieldType> &field)
{
  obj.*(field.member) = static_cast<FieldType>(static_cast<int>(json[field.name]));
}

template <typename T, typename FieldType>
typename std::enable_if<is_serializable<FieldType>::value>::type
deserialize_field_impl(T &obj, const ZJson::JsonValueProxy &json, const FieldDescriptor<T, FieldType> &field)
{
  FieldType nested_obj;
  auto &deserializer = SerializationRegistry<FieldType>::getDeserializer();
  if (deserializer)
  {
    deserializer(nested_obj, json[field.name]);
    obj.*(field.member) = nested_obj;
  }
}

template <typename T, typename FieldType>
void deserialize_field(T &obj, const ZJson::JsonValueProxy &json, const FieldDescriptor<T, FieldType> &field)
{
  try
  {
    deserialize_field_impl(obj, json, field);
  }
  catch (const std::exception &e)
  {
    // Field might be optional or have a different name
    ZLOG_TRACE("Unable to deserialize field %s: %s", field.name.c_str(), e.what());
  }
}

// Helper for unpacking variadic templates (C++11 compatible)
template <typename T>
void deserialize_fields_impl(T &obj, const ZJson::JsonValueProxy &json)
{
  // Base case - do nothing
}

template <typename T, typename Field, typename... Fields>
void deserialize_fields_impl(T &obj, const ZJson::JsonValueProxy &json, Field field, Fields... fields)
{
  deserialize_field(obj, json, field);
  deserialize_fields_impl(obj, json, fields...);
}

template <typename T, typename... Fields>
void deserialize_fields(T &obj, const ZJson::JsonValueProxy &json, Fields... fields)
{
  deserialize_fields_impl(obj, json, fields...);
}

inline std::ostream &operator<<(std::ostream &os, const ZJson::JsonValueProxy &proxy)
{
  int type = proxy.getType();
  switch (type)
  {
  case HWTJ_ARRAY_TYPE:     // Array
    return os << "Array[]"; // TODO(Kelosky): print array type
  case HWTJ_STRING_TYPE:    // String
    return os << static_cast<std::string>(proxy);
  case HWTJ_NUMBER_TYPE: // Number
    return os << static_cast<int>(proxy);
  case HWTJ_BOOLEAN_TYPE: // Boolean
    return os << std::boolalpha << static_cast<bool>(proxy);
  default:
  {
    std::stringstream ss;
    ss << std::hex << type;
    throw std::runtime_error("Unsupported JSON type for key '" + proxy.getKeyPath() + "' type was x'" + ss.str() + "'");
  }
  }
}

#endif
