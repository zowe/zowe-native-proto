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

#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <string>
#include <vector>
#include <set>
#include <map>
#include <functional>
#include <sstream>
#include "../c/zjson.hpp"

/**
 * Lightweight schema validation for JSON-RPC request parameters.
 *
 * This validates JSON structure without deserializing to C++ structs,
 * making it 2-5x faster than full deserialization.
 *
 * Usage:
 *
 * 1. Define a schema for your request parameters:
 *
 *    struct ListDatasetRequest {};
 *
 *    ZJSON_SCHEMA(ListDatasetRequest,
 *        FIELD_REQUIRED(pattern, STRING),
 *        FIELD_REQUIRED(attributes, BOOL)
 *    )
 *
 * 2. Register the validator:
 *
 *    CommandBuilder(handler).validate_schema<ListDatasetRequest>()
 *
 * 3. Validation happens automatically before the command handler is invoked.
 *    If validation fails, an INVALID_PARAMS error is returned to the client.
 *
 * Available field types:
 *   TYPE_BOOL     - boolean (true/false)
 *   TYPE_INTEGER  - integer numbers only
 *   TYPE_NUMBER   - any number (integer or double)
 *   TYPE_STRING   - string values
 *   TYPE_ARRAY    - array (use FIELD_*_ARRAY for typed arrays)
 *   TYPE_OBJECT   - object (no deep validation)
 *   TYPE_ANY      - any type
 */

namespace validator
{

/**
 * Field types for schema validation
 */
enum FieldType
{
  TYPE_BOOL,
  TYPE_INTEGER,
  TYPE_NUMBER,
  TYPE_STRING,
  TYPE_ARRAY,
  TYPE_OBJECT,
  TYPE_ANY
};

/**
 * Field requirement level
 */
enum FieldRequirement
{
  REQUIRED,
  OPTIONAL
};

/**
 * Schema field descriptor
 */
struct FieldDescriptor
{
  std::string name;
  FieldType type;
  FieldRequirement requirement;
  FieldType array_element_type;

  FieldDescriptor(const std::string &n, FieldType t, FieldRequirement r)
      : name(n), type(t), requirement(r), array_element_type(TYPE_ANY)
  {
  }

  FieldDescriptor(const std::string &n, FieldType t, FieldRequirement r, FieldType array_elem_type)
      : name(n), type(t), requirement(r), array_element_type(array_elem_type)
  {
  }
};

/**
 * Validation result type
 */
struct ValidationResult
{
  bool is_valid;
  std::string error_message;

  static ValidationResult success()
  {
    return ValidationResult{true, ""};
  }

  static ValidationResult error(const std::string &message)
  {
    return ValidationResult{false, message};
  }
};

/**
 * Check if a zjson::Value matches the expected type
 */
inline bool check_type(const zjson::Value &value, FieldType expected_type)
{
  switch (expected_type)
  {
  case TYPE_BOOL:
    return value.is_bool();
  case TYPE_INTEGER:
    return value.is_integer();
  case TYPE_NUMBER:
    return value.is_integer() || value.is_double();
  case TYPE_STRING:
    return value.is_string();
  case TYPE_ARRAY:
    return value.is_array();
  case TYPE_OBJECT:
    return value.is_object();
  case TYPE_ANY:
    return true;
  default:
    return false;
  }
}

/**
 * Get human-readable type name
 */
inline std::string type_name(FieldType type)
{
  switch (type)
  {
  case TYPE_BOOL:
    return "boolean";
  case TYPE_INTEGER:
    return "integer";
  case TYPE_NUMBER:
    return "number";
  case TYPE_STRING:
    return "string";
  case TYPE_ARRAY:
    return "array";
  case TYPE_OBJECT:
    return "object";
  case TYPE_ANY:
    return "any";
  default:
    return "unknown";
  }
}

/**
 * Get actual type name from zjson::Value
 */
inline std::string actual_type_name(const zjson::Value &value)
{
  if (value.is_null())
    return "null";
  if (value.is_bool())
    return "boolean";
  if (value.is_integer())
    return "integer";
  if (value.is_double())
    return "number";
  if (value.is_string())
    return "string";
  if (value.is_array())
    return "array";
  if (value.is_object())
    return "object";
  return "unknown";
}

/**
 * Validate a JSON value against a schema
 */
inline ValidationResult validate_schema(const zjson::Value &params,
                                        const std::vector<FieldDescriptor> &schema,
                                        bool allow_unknown_fields = true)
{
  if (!params.is_object())
  {
    return ValidationResult::error("Parameters must be an object");
  }

  const std::map<std::string, zjson::Value> &obj = params.as_object();
  std::set<std::string> seen_fields;

  // Check each field in the schema
  for (std::vector<FieldDescriptor>::const_iterator it = schema.begin(); it != schema.end(); ++it)
  {
    const FieldDescriptor &field = *it;
    std::map<std::string, zjson::Value>::const_iterator field_it = obj.find(field.name);

    if (field_it == obj.end())
    {
      if (field.requirement == REQUIRED)
      {
        return ValidationResult::error("Missing required field: " + field.name);
      }
      continue;
    }

    seen_fields.insert(field.name);
    const zjson::Value &value = field_it->second;

    // Allow null for optional fields
    if (value.is_null() && field.requirement == OPTIONAL)
    {
      continue;
    }

    // Check type
    if (!check_type(value, field.type))
    {
      return ValidationResult::error("Field '" + field.name + "' has wrong type. Expected " +
                                     type_name(field.type) + ", got " + actual_type_name(value));
    }

    // For arrays, validate element types
    if (field.type == TYPE_ARRAY && field.array_element_type != TYPE_ANY)
    {
      const std::vector<zjson::Value> &arr = value.as_array();
      for (size_t i = 0; i < arr.size(); ++i)
      {
        if (!check_type(arr[i], field.array_element_type))
        {
          std::stringstream ss;
          ss << "Field '" << field.name << "' array element at index " << i
             << " has wrong type. Expected " << type_name(field.array_element_type)
             << ", got " << actual_type_name(arr[i]);
          return ValidationResult::error(ss.str());
        }
      }
    }
  }

  // Check for unknown fields if requested
  if (!allow_unknown_fields)
  {
    for (std::map<std::string, zjson::Value>::const_iterator it = obj.begin(); it != obj.end(); ++it)
    {
      if (seen_fields.find(it->first) == seen_fields.end())
      {
        return ValidationResult::error("Unknown field: " + it->first);
      }
    }
  }

  return ValidationResult::success();
}

/**
 * Schema registry - maps struct types to their schema definitions
 */
template <typename T>
struct SchemaRegistry
{
  static std::vector<FieldDescriptor> fields;
};

template <typename T>
std::vector<FieldDescriptor> SchemaRegistry<T>::fields;

/**
 * Base validator interface
 */
class ParamsValidator
{
public:
  virtual ~ParamsValidator()
  {
  }

  virtual ValidationResult validate(const zjson::Value &params) const = 0;
};

/**
 * Schema validator - validates using ZJSON_SCHEMA definitions
 */
template <typename T>
class SchemaValidator : public ParamsValidator
{
public:
  explicit SchemaValidator(bool allow_unknown_fields = true)
      : allow_unknown_fields_(allow_unknown_fields)
  {
  }

  ValidationResult validate(const zjson::Value &params) const
  {
    return validate_schema(params, SchemaRegistry<T>::fields, allow_unknown_fields_);
  }

private:
  bool allow_unknown_fields_;
};

/**
 * Custom function validator
 */
class FunctionValidator : public ParamsValidator
{
public:
  typedef std::function<ValidationResult(const zjson::Value &)> ValidatorFunc;

  explicit FunctionValidator(ValidatorFunc func) : func_(func)
  {
  }

  ValidationResult validate(const zjson::Value &params) const
  {
    return func_(params);
  }

private:
  ValidatorFunc func_;
};

} // namespace validator

// Macros for defining schemas

#define FIELD_REQUIRED(name, type) \
  validator::FieldDescriptor(#name, validator::TYPE_##type, validator::REQUIRED)

#define FIELD_OPTIONAL(name, type) \
  validator::FieldDescriptor(#name, validator::TYPE_##type, validator::OPTIONAL)

#define FIELD_REQUIRED_ARRAY(name, element_type) \
  validator::FieldDescriptor(#name, validator::TYPE_ARRAY, validator::REQUIRED, validator::TYPE_##element_type)

#define FIELD_OPTIONAL_ARRAY(name, element_type) \
  validator::FieldDescriptor(#name, validator::TYPE_ARRAY, validator::OPTIONAL, validator::TYPE_##element_type)

#define ZJSON_SCHEMA(StructType, ...)                                 \
  namespace validator                                                 \
  {                                                                   \
  template <>                                                         \
  std::vector<FieldDescriptor> SchemaRegistry<StructType>::fields = { \
      __VA_ARGS__};                                                   \
  }

#endif // VALIDATOR_HPP
