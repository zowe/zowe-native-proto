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
#include <hwtjic.h> // ensure to include /usr/include
#include "zjsonm.h"

class ZJson
{

private:
  JSON_INSTANCE instance;
  char *dynamic_buffer;
  int dynamic_buffer_length;

public:
  class JsonValueProxy
  {
  private:
    ZJson &parent;
    std::string key;
    KEY_HANDLE key_handle = {0};

  public:
    JsonValueProxy(ZJson &p, const std::string &k)
        : parent(p), key(k)
    {
      int type = HWTJ_SEARCHTYPE_SHALLOW;
      KEY_HANDLE object_handle = {0};
      KEY_HANDLE starting_handle = {0};

      int rc = ZJSMSRCH(&parent.instance, &type, key.c_str(), &object_handle, &starting_handle, &key_handle);
      if (0 != rc)
      {
        throw format_error("Error searching for key '" + key + "'", rc);
      }
    }

  private:
    JsonValueProxy(ZJson &p, const std::string &k, KEY_HANDLE kh)
        : parent(p), key(k), key_handle(kh)
    {
    }

    JsonValueProxy(ZJson &p, const std::string &k, KEY_HANDLE parent_kh, const std::string &search_key)
        : parent(p), key(k)
    {
      int type = HWTJ_SEARCHTYPE_SHALLOW;
      KEY_HANDLE starting_handle = {0};

      int rc = ZJSMSRCH(&parent.instance, &type, search_key.c_str(), &parent_kh, &starting_handle, &key_handle);
      if (0 != rc)
      {
        throw format_error("Error searching for nested key '" + search_key + "' in '" + key + "'", rc);
      }
    }

  public:
    explicit operator std::string() const
    {
      int rc = 0;
      char *value_ptr = nullptr;
      int value_length = 0;

      rc = ZJSMGVAL(&parent.instance, get_mutable_key_handle(), &value_ptr, &value_length);
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

      rc = ZJSMGVAL(&parent.instance, get_mutable_key_handle(), &value_ptr, &value_length);
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

      rc = ZJSMGBOV(&parent.instance, get_mutable_key_handle(), &b_val);
      if (0 != rc)
      {
        throw format_error("Error getting boolean value for key '" + key + "'", rc);
      }
      return (b_val == 1);
    }

    int getType() const
    {
      int rc = 0;
      int type = 0;
      rc = ZJSNGJST(&parent.instance, get_mutable_key_handle(), &type);
      if (0 != rc)
      {
        throw format_error("Error getting type for key '" + key + "'", rc);
      }
      return type;
    }

    std::string getKey() const
    {
      return key;
    }

    JsonValueProxy operator[](int index) const
    {
      int type = this->getType();
      if (type != 2) // not an array
      {
        throw std::runtime_error("Cannot apply operator[] to a non-array type for key '" + key + "'. Type was " + std::to_string(type));
      }

      int rc = 0;
      KEY_HANDLE element_handle = {0};
      int mutable_index = index;

      rc = ZJSMGAEN(&parent.instance, get_mutable_key_handle(), &mutable_index, &element_handle);
      if (0 != rc)
      {
        throw format_error("Error getting array element at index " + std::to_string(index) + " for key '" + key + "'.", rc);
      }

      std::string new_key = key + "[" + std::to_string(index) + "]";
      return JsonValueProxy(parent, new_key, element_handle);
    }

    JsonValueProxy operator[](const std::string &index_str) const
    {
      // Check if this is an array access (numeric string) or object access
      int type = this->getType();
      if (type == 2) // Array type
      {
        return (*this)[std::stoi(index_str)];
      }
      else if (type == 1) // Object type
      {
        // Create a nested search proxy
        std::string nested_key = key + "." + index_str;
        return JsonValueProxy(parent, nested_key, key_handle, index_str);
      }
      else
      {
        throw std::runtime_error("Cannot apply string operator[] to type " + std::to_string(type) + " for key '" + key + "'");
      }
    }

  private:
    std::runtime_error format_error(const std::string &msg, int rc) const
    {
      std::stringstream ss;
      ss << std::hex << rc;
      return std::runtime_error(msg + " rc was x'" + ss.str() + "'");
    }

    KEY_HANDLE *get_mutable_key_handle() const
    {
      return const_cast<KEY_HANDLE *>(&key_handle);
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

  void parse(const std::string &json)
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

  JsonValueProxy operator[](const std::string &key)
  {
    return JsonValueProxy(*this, key);
  }
};

inline std::ostream &operator<<(std::ostream &os, const ZJson::JsonValueProxy &proxy)
{
  int type = proxy.getType();
  switch (type)
  {
  case 2:                   // Array
    return os << "Array[]"; // TODO(Kelosky): print array type
  case 3:                   // String
    return os << static_cast<std::string>(proxy);
  case 4: // Number
    return os << static_cast<int>(proxy);
  case 5: // Boolean
    return os << std::boolalpha << static_cast<bool>(proxy);
  default:
  {
    std::stringstream ss;
    ss << std::hex << type;
    throw std::runtime_error("Unsupported JSON type for key '" + proxy.getKey() + "' type was x'" + ss.str() + "'");
  }
  }
}

#endif
