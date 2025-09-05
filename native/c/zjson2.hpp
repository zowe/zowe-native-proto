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
#include <sstream>
#include <type_traits>
#include <cstring>
#include <cstdint>
#include "zjsonm.h"

// z/OS XLC compatibility macros
#if defined(__IBMCPP__) || defined(__IBMC__)
#pragma filetag("IBM-1047") /* compile in EBCDIC */
#pragma longName
// Ensure C++11 compatibility on z/OS XLC
#if __cplusplus < 201103L
#define noexcept throw()
#define nullptr NULL
#endif
#endif

// JSON type constants from HWT JSON services
#define HWTJ_JTYPE_OBJECT 1
#define HWTJ_JTYPE_ARRAY 2
#define HWTJ_JTYPE_STRING 3
#define HWTJ_JTYPE_NUMBER 4
#define HWTJ_JTYPE_BOOLEAN 5
#define HWTJ_JTYPE_NULL 6

// Additional constants needed for Metal C functions
#define HWTJ_SEARCHTYPE_GLOBAL 1
#define HWTJ_NOFORCE 0

namespace zjson
{

// Forward declarations
template <typename = void, typename = void, typename = void, typename = void>
class basic_json;

// No extra typedef needed - basic_json is the main class

// JSON value types (compatible with nlohmann::json)
enum class value_t : std::uint8_t
{
  null,
  object,
  array,
  string,
  boolean,
  number_integer,
  number_unsigned,
  number_float,
  binary,
  discarded
};

// Exception classes compatible with nlohmann::json
class json_exception : public std::exception
{
public:
  explicit json_exception(const std::string &msg) : message(msg)
  {
  }
  const char *what() const noexcept override
  {
    return message.c_str();
  }

private:
  std::string message;
};

class parse_error : public json_exception
{
public:
  explicit parse_error(const std::string &msg) : json_exception("parse_error: " + msg)
  {
  }
};

class type_error : public json_exception
{
public:
  explicit type_error(const std::string &msg) : json_exception("type_error: " + msg)
  {
  }
};

class out_of_range : public json_exception
{
public:
  explicit out_of_range(const std::string &msg) : json_exception("out_of_range: " + msg)
  {
  }
};

class invalid_iterator : public json_exception
{
public:
  explicit invalid_iterator(const std::string &msg) : json_exception("invalid_iterator: " + msg)
  {
  }
};

class other_error : public json_exception
{
public:
  explicit other_error(const std::string &msg) : json_exception("other_error: " + msg)
  {
  }
};

// Main JSON class
template <typename ObjectType, typename ArrayType, typename StringType, typename BooleanType>
class basic_json
{
public:
  // Type definitions for compatibility
  using value_type = basic_json;
  using reference = value_type &;
  using const_reference = const value_type &;
  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;

  using object_t = std::map<std::string, basic_json>;
  using array_t = std::vector<basic_json>;
  using string_t = std::string;
  using boolean_t = bool;
  using number_integer_t = std::int64_t;
  using number_unsigned_t = std::uint64_t;
  using number_float_t = double;

private:
  // Internal value storage
  value_t m_type = value_t::null;

  union
  {
    object_t *m_object;
    array_t *m_array;
    string_t *m_string;
    boolean_t m_boolean;
    number_integer_t m_integer;
    number_unsigned_t m_unsigned;
    number_float_t m_float;
  } m_value = {nullptr};

  // Static parser state
  static PARSE_HANDLE s_parser_handle;
  static DIAG s_diag;
  static bool s_parser_initialized;
  static std::string s_last_json_string;

  // Helper methods
  void create_value(value_t type)
  {
    destroy_value();
    m_type = type;

    switch (type)
    {
    case value_t::object:
      m_value.m_object = new object_t();
      break;
    case value_t::array:
      m_value.m_array = new array_t();
      break;
    case value_t::string:
      m_value.m_string = new string_t();
      break;
    case value_t::boolean:
      m_value.m_boolean = false;
      break;
    case value_t::number_integer:
      m_value.m_integer = 0;
      break;
    case value_t::number_unsigned:
      m_value.m_unsigned = 0;
      break;
    case value_t::number_float:
      m_value.m_float = 0.0;
      break;
    default:
      break;
    }
  }

  void destroy_value()
  {
    switch (m_type)
    {
    case value_t::object:
      delete m_value.m_object;
      break;
    case value_t::array:
      delete m_value.m_array;
      break;
    case value_t::string:
      delete m_value.m_string;
      break;
    default:
      break;
    }
    m_value = {nullptr};
    m_type = value_t::null;
  }

  static bool ensure_parser_initialized()
  {
    if (!s_parser_initialized)
    {
      int rc = ZJSNMINIT(&s_parser_handle, &s_diag);
      if (rc != 0)
      {
        throw parse_error("Failed to initialize JSON parser: " + std::string(s_diag.msg));
      }
      s_parser_initialized = true;
    }
    return true;
  }

  static void cleanup_parser()
  {
    if (s_parser_initialized)
    {
      ZJSNMTERM(&s_parser_handle, &s_diag);
      s_parser_initialized = false;
    }
  }

  basic_json parse_from_handle(KEY_HANDLE *key_handle = nullptr)
  {
    basic_json result;

    // Get the type of the JSON value
    int json_type;
    int rc = ZJSNGJST(&s_parser_handle, key_handle, &json_type, &s_diag);
    if (rc != 0)
    {
      throw parse_error("Failed to get JSON type: " + std::string(s_diag.msg));
    }

    switch (json_type)
    {
    case HWTJ_JTYPE_OBJECT:
    {
      result.create_value(value_t::object);

      // Get number of entries in object
      int num_entries;
      rc = ZJSMGNUE(&s_parser_handle, key_handle, &num_entries, &s_diag);
      if (rc != 0)
        break;

      // Iterate through object entries
      for (int i = 0; i < num_entries; i++)
      {
        char *entry_name;
        int name_length;
        KEY_HANDLE entry_handle;
        int actual_length;

        rc = ZJSMGOEN(&s_parser_handle, key_handle, &i, &entry_name, &name_length, &entry_handle, &actual_length, &s_diag);
        if (rc == 0)
        {
          std::string key(entry_name, name_length);
          (*result.m_value.m_object)[key] = parse_from_handle(&entry_handle);
        }
      }
      break;
    }

    case HWTJ_JTYPE_ARRAY:
    {
      result.create_value(value_t::array);

      // Get number of entries in array
      int num_entries;
      rc = ZJSMGNUE(&s_parser_handle, key_handle, &num_entries, &s_diag);
      if (rc != 0)
        break;

      // Iterate through array entries
      for (int i = 0; i < num_entries; i++)
      {
        KEY_HANDLE entry_handle;
        rc = ZJSMGAEN(&s_parser_handle, key_handle, &i, &entry_handle, &s_diag);
        if (rc == 0)
        {
          result.m_value.m_array->push_back(parse_from_handle(&entry_handle));
        }
      }
      break;
    }

    case HWTJ_JTYPE_STRING:
    {
      result.create_value(value_t::string);
      char *value;
      int value_length;
      rc = ZJSMGVAL(&s_parser_handle, key_handle, &value, &value_length, &s_diag);
      if (rc == 0)
      {
        *result.m_value.m_string = std::string(value, value_length);
      }
      break;
    }

    case HWTJ_JTYPE_NUMBER:
    {
      char *value;
      int value_length;
      rc = ZJSMGVAL(&s_parser_handle, key_handle, &value, &value_length, &s_diag);
      if (rc == 0)
      {
        std::string num_str(value, value_length);
        if (num_str.find('.') != std::string::npos ||
            num_str.find('e') != std::string::npos ||
            num_str.find('E') != std::string::npos)
        {
          result.create_value(value_t::number_float);
          result.m_value.m_float = std::stod(num_str);
        }
        else
        {
          result.create_value(value_t::number_integer);
          result.m_value.m_integer = std::stoll(num_str);
        }
      }
      break;
    }

    case HWTJ_JTYPE_BOOLEAN:
    {
      result.create_value(value_t::boolean);
      char value[8];
      rc = ZJSMGBOV(&s_parser_handle, key_handle, value, &s_diag);
      if (rc == 0)
      {
        result.m_value.m_boolean = (strncmp(value, "true", 4) == 0);
      }
      break;
    }

    case HWTJ_JTYPE_NULL:
    default:
      result.create_value(value_t::null);
      break;
    }

    return result;
  }

public:
  // Constructors
  basic_json() : m_type(value_t::null)
  {
    m_value = {nullptr};
  }

  basic_json(std::nullptr_t) : basic_json()
  {
  }

  basic_json(bool value)
  {
    create_value(value_t::boolean);
    m_value.m_boolean = value;
  }

  basic_json(int value)
  {
    create_value(value_t::number_integer);
    m_value.m_integer = value;
  }

  basic_json(std::int64_t value)
  {
    create_value(value_t::number_integer);
    m_value.m_integer = value;
  }

  basic_json(std::uint64_t value)
  {
    create_value(value_t::number_unsigned);
    m_value.m_unsigned = value;
  }

  basic_json(double value)
  {
    create_value(value_t::number_float);
    m_value.m_float = value;
  }

  basic_json(const std::string &value)
  {
    create_value(value_t::string);
    *m_value.m_string = value;
  }

  basic_json(const char *value)
  {
    create_value(value_t::string);
    *m_value.m_string = std::string(value);
  }

  basic_json(const object_t &value)
  {
    create_value(value_t::object);
    *m_value.m_object = value;
  }

  basic_json(const array_t &value)
  {
    create_value(value_t::array);
    *m_value.m_array = value;
  }

  // Copy constructor
  basic_json(const basic_json &other)
  {
    create_value(other.m_type);

    switch (m_type)
    {
    case value_t::object:
      *m_value.m_object = *other.m_value.m_object;
      break;
    case value_t::array:
      *m_value.m_array = *other.m_value.m_array;
      break;
    case value_t::string:
      *m_value.m_string = *other.m_value.m_string;
      break;
    case value_t::boolean:
      m_value.m_boolean = other.m_value.m_boolean;
      break;
    case value_t::number_integer:
      m_value.m_integer = other.m_value.m_integer;
      break;
    case value_t::number_unsigned:
      m_value.m_unsigned = other.m_value.m_unsigned;
      break;
    case value_t::number_float:
      m_value.m_float = other.m_value.m_float;
      break;
    default:
      break;
    }
  }

  // Move constructor
  basic_json(basic_json &&other) noexcept
  {
    m_type = other.m_type;
    m_value = other.m_value;
    other.m_type = value_t::null;
    other.m_value = {nullptr};
  }

  // Destructor
  ~basic_json()
  {
    destroy_value();
  }

  // Assignment operators
  basic_json &operator=(const basic_json &other)
  {
    if (this != &other)
    {
      destroy_value();
      create_value(other.m_type);

      switch (m_type)
      {
      case value_t::object:
        *m_value.m_object = *other.m_value.m_object;
        break;
      case value_t::array:
        *m_value.m_array = *other.m_value.m_array;
        break;
      case value_t::string:
        *m_value.m_string = *other.m_value.m_string;
        break;
      case value_t::boolean:
        m_value.m_boolean = other.m_value.m_boolean;
        break;
      case value_t::number_integer:
        m_value.m_integer = other.m_value.m_integer;
        break;
      case value_t::number_unsigned:
        m_value.m_unsigned = other.m_value.m_unsigned;
        break;
      case value_t::number_float:
        m_value.m_float = other.m_value.m_float;
        break;
      default:
        break;
      }
    }
    return *this;
  }

  basic_json &operator=(basic_json &&other) noexcept
  {
    if (this != &other)
    {
      destroy_value();
      m_type = other.m_type;
      m_value = other.m_value;
      other.m_type = value_t::null;
      other.m_value = {nullptr};
    }
    return *this;
  }

  // Type checking methods
  value_t type() const noexcept
  {
    return m_type;
  }

  bool is_null() const noexcept
  {
    return m_type == value_t::null;
  }
  bool is_boolean() const noexcept
  {
    return m_type == value_t::boolean;
  }
  bool is_number() const noexcept
  {
    return m_type == value_t::number_integer ||
           m_type == value_t::number_unsigned ||
           m_type == value_t::number_float;
  }
  bool is_number_integer() const noexcept
  {
    return m_type == value_t::number_integer;
  }
  bool is_number_unsigned() const noexcept
  {
    return m_type == value_t::number_unsigned;
  }
  bool is_number_float() const noexcept
  {
    return m_type == value_t::number_float;
  }
  bool is_string() const noexcept
  {
    return m_type == value_t::string;
  }
  bool is_array() const noexcept
  {
    return m_type == value_t::array;
  }
  bool is_object() const noexcept
  {
    return m_type == value_t::object;
  }
  bool is_primitive() const noexcept
  {
    return is_null() || is_boolean() || is_number() || is_string();
  }
  bool is_structured() const noexcept
  {
    return is_array() || is_object();
  }

  // Value access methods
  template <typename T>
  typename std::enable_if<std::is_same<T, bool>::value, T>::type
  get() const
  {
    if (!is_boolean())
      throw type_error("cannot get boolean from " + type_name());
    return m_value.m_boolean;
  }

  template <typename T>
  typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, T>::type
  get() const
  {
    if (is_number_integer())
      return static_cast<T>(m_value.m_integer);
    if (is_number_unsigned())
      return static_cast<T>(m_value.m_unsigned);
    if (is_number_float())
      return static_cast<T>(m_value.m_float);
    throw type_error("cannot get integer from " + type_name());
  }

  template <typename T>
  typename std::enable_if<std::is_floating_point<T>::value, T>::type
  get() const
  {
    if (is_number_float())
      return static_cast<T>(m_value.m_float);
    if (is_number_integer())
      return static_cast<T>(m_value.m_integer);
    if (is_number_unsigned())
      return static_cast<T>(m_value.m_unsigned);
    throw type_error("cannot get float from " + type_name());
  }

  template <typename T>
  typename std::enable_if<std::is_same<T, std::string>::value, T>::type
  get() const
  {
    if (!is_string())
      throw type_error("cannot get string from " + type_name());
    return *m_value.m_string;
  }

  // Implicit conversions
  operator bool() const
  {
    return get<bool>();
  }
  operator int() const
  {
    return get<int>();
  }
  operator std::int64_t() const
  {
    return get<std::int64_t>();
  }
  operator std::uint64_t() const
  {
    return get<std::uint64_t>();
  }
  operator double() const
  {
    return get<double>();
  }
  operator std::string() const
  {
    return get<std::string>();
  }

  // Array access
  reference operator[](size_type idx)
  {
    if (is_null())
    {
      create_value(value_t::array);
    }
    if (!is_array())
    {
      throw type_error("cannot use operator[] with " + type_name());
    }

    if (idx >= m_value.m_array->size())
    {
      m_value.m_array->resize(idx + 1);
    }
    return (*m_value.m_array)[idx];
  }

  const_reference operator[](size_type idx) const
  {
    if (!is_array())
    {
      throw type_error("cannot use operator[] with " + type_name());
    }
    return (*m_value.m_array)[idx];
  }

  reference at(size_type idx)
  {
    if (!is_array())
    {
      throw type_error("cannot use at() with " + type_name());
    }
    if (idx >= m_value.m_array->size())
    {
      throw out_of_range("array index " + std::to_string(idx) + " is out of range");
    }
    return (*m_value.m_array)[idx];
  }

  const_reference at(size_type idx) const
  {
    if (!is_array())
    {
      throw type_error("cannot use at() with " + type_name());
    }
    if (idx >= m_value.m_array->size())
    {
      throw out_of_range("array index " + std::to_string(idx) + " is out of range");
    }
    return (*m_value.m_array)[idx];
  }

  // Object access
  reference operator[](const std::string &key)
  {
    if (is_null())
    {
      create_value(value_t::object);
    }
    if (!is_object())
    {
      throw type_error("cannot use operator[] with " + type_name());
    }
    return (*m_value.m_object)[key];
  }

  const_reference operator[](const std::string &key) const
  {
    if (!is_object())
    {
      throw type_error("cannot use operator[] with " + type_name());
    }
    auto it = m_value.m_object->find(key);
    if (it == m_value.m_object->end())
    {
      throw out_of_range("key '" + key + "' not found");
    }
    return it->second;
  }

  reference operator[](const char *key)
  {
    return operator[](std::string(key));
  }

  const_reference operator[](const char *key) const
  {
    return operator[](std::string(key));
  }

  reference at(const std::string &key)
  {
    if (!is_object())
    {
      throw type_error("cannot use at() with " + type_name());
    }
    auto it = m_value.m_object->find(key);
    if (it == m_value.m_object->end())
    {
      throw out_of_range("key '" + key + "' not found");
    }
    return it->second;
  }

  const_reference at(const std::string &key) const
  {
    if (!is_object())
    {
      throw type_error("cannot use at() with " + type_name());
    }
    auto it = m_value.m_object->find(key);
    if (it == m_value.m_object->end())
    {
      throw out_of_range("key '" + key + "' not found");
    }
    return it->second;
  }

  // Size and empty
  size_type size() const noexcept
  {
    switch (m_type)
    {
    case value_t::null:
      return 0;
    case value_t::array:
      return m_value.m_array->size();
    case value_t::object:
      return m_value.m_object->size();
    default:
      return 1;
    }
  }

  bool empty() const noexcept
  {
    switch (m_type)
    {
    case value_t::null:
      return true;
    case value_t::array:
      return m_value.m_array->empty();
    case value_t::object:
      return m_value.m_object->empty();
    case value_t::string:
      return m_value.m_string->empty();
    default:
      return false;
    }
  }

  // Array modification
  void push_back(const basic_json &val)
  {
    if (is_null())
    {
      create_value(value_t::array);
    }
    if (!is_array())
    {
      throw type_error("cannot use push_back() with " + type_name());
    }
    m_value.m_array->push_back(val);
  }

  void push_back(basic_json &&val)
  {
    if (is_null())
    {
      create_value(value_t::array);
    }
    if (!is_array())
    {
      throw type_error("cannot use push_back() with " + type_name());
    }
    m_value.m_array->push_back(std::move(val));
  }

  // Object modification
  bool contains(const std::string &key) const
  {
    if (!is_object())
    {
      return false;
    }
    return m_value.m_object->find(key) != m_value.m_object->end();
  }

  size_type erase(const std::string &key)
  {
    if (!is_object())
    {
      throw type_error("cannot use erase() with " + type_name());
    }
    return m_value.m_object->erase(key);
  }

  // Additional nlohmann::json compatibility methods
  size_type count(const std::string &key) const
  {
    if (!is_object())
    {
      return 0;
    }
    return m_value.m_object->count(key);
  }

  // Value access with default (key method for nlohmann compatibility)
  template <typename T>
  T value(const std::string &key, const T &default_value) const
  {
    if (!is_object())
    {
      return default_value;
    }
    auto it = m_value.m_object->find(key);
    if (it == m_value.m_object->end())
    {
      return default_value;
    }
    try
    {
      return it->second.template get<T>();
    }
    catch (...)
    {
      return default_value;
    }
  }

  // Basic iterator support for range-based for loops
  class iterator
  {
  private:
    basic_json *m_object;
    typename object_t::iterator m_obj_it;
    typename array_t::iterator m_arr_it;
    bool m_is_array;

  public:
    iterator(basic_json *obj, typename object_t::iterator it)
        : m_object(obj), m_obj_it(it), m_is_array(false)
    {
    }

    iterator(basic_json *obj, typename array_t::iterator it)
        : m_object(obj), m_arr_it(it), m_is_array(true)
    {
    }

    reference operator*()
    {
      return m_is_array ? *m_arr_it : m_obj_it->second;
    }

    iterator &operator++()
    {
      if (m_is_array)
        ++m_arr_it;
      else
        ++m_obj_it;
      return *this;
    }

    bool operator!=(const iterator &other) const
    {
      if (m_is_array != other.m_is_array)
        return true;
      return m_is_array ? (m_arr_it != other.m_arr_it) : (m_obj_it != other.m_obj_it);
    }

    std::string key() const
    {
      if (m_is_array)
      {
        throw type_error("cannot get key from array iterator");
      }
      return m_obj_it->first;
    }
  };

  class const_iterator
  {
  private:
    const basic_json *m_object;
    typename object_t::const_iterator m_obj_it;
    typename array_t::const_iterator m_arr_it;
    bool m_is_array;

  public:
    const_iterator(const basic_json *obj, typename object_t::const_iterator it)
        : m_object(obj), m_obj_it(it), m_is_array(false)
    {
    }

    const_iterator(const basic_json *obj, typename array_t::const_iterator it)
        : m_object(obj), m_arr_it(it), m_is_array(true)
    {
    }

    const_reference operator*() const
    {
      return m_is_array ? *m_arr_it : m_obj_it->second;
    }

    const_iterator &operator++()
    {
      if (m_is_array)
        ++m_arr_it;
      else
        ++m_obj_it;
      return *this;
    }

    bool operator!=(const const_iterator &other) const
    {
      if (m_is_array != other.m_is_array)
        return true;
      return m_is_array ? (m_arr_it != other.m_arr_it) : (m_obj_it != other.m_obj_it);
    }

    std::string key() const
    {
      if (m_is_array)
      {
        throw type_error("cannot get key from array iterator");
      }
      return m_obj_it->first;
    }
  };

public:
  // Iterator methods
  iterator begin()
  {
    if (is_object())
    {
      return iterator(this, m_value.m_object->begin());
    }
    else if (is_array())
    {
      return iterator(this, m_value.m_array->begin());
    }
    else
    {
      throw type_error("cannot get iterator for " + type_name());
    }
  }

  iterator end()
  {
    if (is_object())
    {
      return iterator(this, m_value.m_object->end());
    }
    else if (is_array())
    {
      return iterator(this, m_value.m_array->end());
    }
    else
    {
      throw type_error("cannot get iterator for " + type_name());
    }
  }

  const_iterator begin() const
  {
    if (is_object())
    {
      return const_iterator(this, m_value.m_object->begin());
    }
    else if (is_array())
    {
      return const_iterator(this, m_value.m_array->begin());
    }
    else
    {
      throw type_error("cannot get iterator for " + type_name());
    }
  }

  const_iterator end() const
  {
    if (is_object())
    {
      return const_iterator(this, m_value.m_object->end());
    }
    else if (is_array())
    {
      return const_iterator(this, m_value.m_array->end());
    }
    else
    {
      throw type_error("cannot get iterator for " + type_name());
    }
  }

  const_iterator cbegin() const
  {
    return begin();
  }
  const_iterator cend() const
  {
    return end();
  }

  // Find method for nlohmann compatibility
  iterator find(const std::string &key)
  {
    if (!is_object())
    {
      return end();
    }
    auto it = m_value.m_object->find(key);
    if (it == m_value.m_object->end())
    {
      return end();
    }
    return iterator(this, it);
  }

  const_iterator find(const std::string &key) const
  {
    if (!is_object())
    {
      return end();
    }
    auto it = m_value.m_object->find(key);
    if (it == m_value.m_object->end())
    {
      return end();
    }
    return const_iterator(this, it);
  }

  void clear() noexcept
  {
    switch (m_type)
    {
    case value_t::array:
      m_value.m_array->clear();
      break;
    case value_t::object:
      m_value.m_object->clear();
      break;
    case value_t::string:
      m_value.m_string->clear();
      break;
    default:
      break;
    }
  }

  // Serialization
  std::string dump(int indent = -1) const
  {
    std::ostringstream ss;
    dump_impl(ss, indent, 0);
    return ss.str();
  }

  // Static parsing method
  static basic_json parse(const std::string &json_str)
  {
    return parse(json_str.c_str());
  }

  static basic_json parse(const char *json_str)
  {
    ensure_parser_initialized();

    // Store the JSON string for the duration of parsing
    s_last_json_string = json_str;

    // Parse the JSON string
    int rc = ZJSMPARS(&s_parser_handle, const_cast<char *>(s_last_json_string.c_str()), &s_diag);
    if (rc != 0)
    {
      throw parse_error("Failed to parse JSON: " + std::string(s_diag.msg));
    }

    // Create the result from the root of the parsed JSON
    basic_json result;
    return result.parse_from_handle(nullptr);
  }

  // Cleanup method
  static void cleanup()
  {
    cleanup_parser();
  }

  // Static utility function to convert value_t to string
  // Matches nlohmann::json's type_name() behavior exactly
  static std::string value_type_to_string(value_t type)
  {
    switch (type)
    {
    case value_t::null:
      return "null";
    case value_t::object:
      return "object";
    case value_t::array:
      return "array";
    case value_t::string:
      return "string";
    case value_t::boolean:
      return "boolean";
    case value_t::number_integer:
    case value_t::number_unsigned:
    case value_t::number_float:
      return "number";
    case value_t::binary:
      return "binary";
    case value_t::discarded:
      return "discarded";
    default:
      return "unknown";
    }
  }

private:
  std::string type_name() const
  {
    return value_type_to_string(m_type);
  }

  void dump_impl(std::ostringstream &ss, int indent, int current_indent) const
  {
    const bool pretty = (indent > 0);
    const std::string indent_str(current_indent, ' ');
    const std::string next_indent_str(current_indent + indent, ' ');

    switch (m_type)
    {
    case value_t::null:
      ss << "null";
      break;

    case value_t::boolean:
      ss << (m_value.m_boolean ? "true" : "false");
      break;

    case value_t::number_integer:
      ss << m_value.m_integer;
      break;

    case value_t::number_unsigned:
      ss << m_value.m_unsigned;
      break;

    case value_t::number_float:
      ss << m_value.m_float;
      break;

    case value_t::string:
      ss << "\"";
      for (char c : *m_value.m_string)
      {
        switch (c)
        {
        case '"':
          ss << "\\\"";
          break;
        case '\\':
          ss << "\\\\";
          break;
        case '\b':
          ss << "\\b";
          break;
        case '\f':
          ss << "\\f";
          break;
        case '\n':
          ss << "\\n";
          break;
        case '\r':
          ss << "\\r";
          break;
        case '\t':
          ss << "\\t";
          break;
        default:
          ss << c;
          break;
        }
      }
      ss << "\"";
      break;

    case value_t::array:
      ss << "[";
      if (pretty && !m_value.m_array->empty())
      {
        ss << "\n";
      }
      for (size_t i = 0; i < m_value.m_array->size(); ++i)
      {
        if (pretty)
        {
          ss << next_indent_str;
        }
        (*m_value.m_array)[i].dump_impl(ss, indent, current_indent + indent);
        if (i < m_value.m_array->size() - 1)
        {
          ss << ",";
        }
        if (pretty)
        {
          ss << "\n";
        }
      }
      if (pretty && !m_value.m_array->empty())
      {
        ss << indent_str;
      }
      ss << "]";
      break;

    case value_t::object:
    {
      ss << "{";
      if (pretty && !m_value.m_object->empty())
      {
        ss << "\n";
      }
      auto it = m_value.m_object->begin();
      for (size_t i = 0; i < m_value.m_object->size(); ++i, ++it)
      {
        if (pretty)
        {
          ss << next_indent_str;
        }
        ss << "\"" << it->first << "\":";
        if (pretty)
        {
          ss << " ";
        }
        it->second.dump_impl(ss, indent, current_indent + indent);
        if (i < m_value.m_object->size() - 1)
        {
          ss << ",";
        }
        if (pretty)
        {
          ss << "\n";
        }
      }
      if (pretty && !m_value.m_object->empty())
      {
        ss << indent_str;
      }
      ss << "}";
      break;
    }

    case value_t::binary:
      // For binary data, output as base64 or hex representation
      // For now, just output a placeholder
      ss << "\"<binary data>\"";
      break;

    case value_t::discarded:
      // Discarded values shouldn't be serialized, but handle gracefully
      ss << "null";
      break;
    }
  }
};

// Static member definitions
template <typename ObjectType, typename ArrayType, typename StringType, typename BooleanType>
PARSE_HANDLE basic_json<ObjectType, ArrayType, StringType, BooleanType>::s_parser_handle;

template <typename ObjectType, typename ArrayType, typename StringType, typename BooleanType>
DIAG basic_json<ObjectType, ArrayType, StringType, BooleanType>::s_diag;

template <typename ObjectType, typename ArrayType, typename StringType, typename BooleanType>
bool basic_json<ObjectType, ArrayType, StringType, BooleanType>::s_parser_initialized = false;

template <typename ObjectType, typename ArrayType, typename StringType, typename BooleanType>
std::string basic_json<ObjectType, ArrayType, StringType, BooleanType>::s_last_json_string;

// Stream operator for value_t enum for debugging/testing
// Uses the same logic as nlohmann::json's type_name() method
inline std::ostream &operator<<(std::ostream &os, const value_t &type)
{
  return os << basic_json<>::value_type_to_string(type);
}

} // namespace zjson

// Global convenience typedef for compatibility
using json = zjson::basic_json<>;

#endif // ZJSON_HPP
