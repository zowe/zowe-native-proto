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

#include "validator.hpp"
#include "../c/zjson.hpp"
#include <unordered_set>

using std::string;

namespace validator
{

/**
 * Check if a JSON value matches the expected type
 */
static bool check_type(const zjson::Value &value, FieldType expected_type)
{
  switch (expected_type)
  {
  case FieldType::TYPE_BOOL:
    return value.is_bool();
  case FieldType::TYPE_NUMBER:
    return value.is_integer() || value.is_double();
  case FieldType::TYPE_STRING:
    return value.is_string();
  case FieldType::TYPE_ARRAY:
    return value.is_array();
  case FieldType::TYPE_OBJECT:
    return value.is_object();
  case FieldType::TYPE_ANY:
    return true;
  default:
    return false;
  }
}

/**
 * Get human-readable name for a field type
 */
static std::string type_name(FieldType type)
{
  switch (type)
  {
  case FieldType::TYPE_BOOL:
    return "boolean";
  case FieldType::TYPE_NUMBER:
    return "number";
  case FieldType::TYPE_STRING:
    return "string";
  case FieldType::TYPE_ARRAY:
    return "array";
  case FieldType::TYPE_OBJECT:
    return "object";
  case FieldType::TYPE_ANY:
    return "any";
  default:
    return "unknown";
  }
}

/**
 * Get human-readable name for the actual type of a JSON value
 */
static std::string actual_type_name(const zjson::Value &value)
{
  if (value.is_null())
    return "null";
  if (value.is_bool())
    return "boolean";
  if (value.is_integer() || value.is_double())
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
ValidationResult validate_schema(const zjson::Value &params,
                                 const FieldDescriptor *schema,
                                 size_t field_count,
                                 bool allow_unknown_fields)
{
  if (!params.is_object())
  {
    return ValidationResult::error("Parameters must be an object");
  }

  const auto &obj = params.as_object();
  std::unordered_set<std::string> seen_fields;

  // Check each field in the schema
  for (size_t i = 0; i < field_count; i++)
  {
    const FieldDescriptor &field = schema[i];
    auto field_it = obj.find(field.name);

    if (field_it == obj.end())
    {
      if (field.required)
      {
        return ValidationResult::error(std::string("Missing required field: ") + field.name);
      }
      continue;
    }

    seen_fields.insert(field.name);
    const zjson::Value &value = field_it->second;

    // Allow null for optional fields
    if (value.is_null() && !field.required)
    {
      continue;
    }

    // Check type
    if (!check_type(value, field.type))
    {
      return ValidationResult::error(std::string("Field '") + field.name + "' has wrong type. Expected " +
                                     type_name(field.type) + ", got " + actual_type_name(value));
    }

    // Validate nested object schema (1 level deep)
    if (field.type == FieldType::TYPE_OBJECT && field.nested_schema.fields != nullptr)
    {
      ValidationResult nested_result = validate_schema(value, field.nested_schema.fields, field.nested_schema.count, allow_unknown_fields);
      if (!nested_result.is_valid)
      {
        return ValidationResult::error(std::string("Field '") + field.name + "': " + nested_result.error_message);
      }
    }

    // For arrays, validate only first element (spot check for performance)
    if (field.type == FieldType::TYPE_ARRAY && field.array_element_type != FieldType::TYPE_ANY)
    {
      const std::vector<zjson::Value> &arr = value.as_array();
      if (!arr.empty())
      {
        if (!check_type(arr[0], field.array_element_type))
        {
          return ValidationResult::error(std::string("Field '") + field.name + "' array element [0] has wrong type. Expected " +
                                         type_name(field.array_element_type) + ", got " + actual_type_name(arr[0]));
        }

        // If it's an object with a schema, validate the schema
        if (field.array_element_type == FieldType::TYPE_OBJECT && field.nested_schema.fields != nullptr)
        {
          ValidationResult nested_result = validate_schema(arr[0], field.nested_schema.fields, field.nested_schema.count, allow_unknown_fields);
          if (!nested_result.is_valid)
          {
            return ValidationResult::error(std::string("Field '") + field.name + "' array element [0]: " + nested_result.error_message);
          }
        }
      }
    }
  }

  // Check for unknown fields if requested
  if (!allow_unknown_fields)
  {
    for (const auto &pair : obj)
    {
      if (seen_fields.find(pair.first) == seen_fields.end())
      {
        return ValidationResult::error("Unknown field: " + pair.first);
      }
    }
  }

  return ValidationResult::success();
}

} // namespace validator
