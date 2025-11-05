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
#include <functional>
#include "../c/zstd.hpp"

/**
 * Schema-based validation for JSON-RPC messages.
 *
 * Define schemas using ZJSON_SCHEMA macro, then register with CommandBuilder:
 *
 *   struct ListDatasetRequest {};
 *   ZJSON_SCHEMA(ListDatasetRequest,
 *       FIELD_REQUIRED(pattern, STRING),
 *       FIELD_OPTIONAL(attributes, BOOL)
 *   )
 *
 *   CommandBuilder(handler).validate<ListDatasetRequest, ListDatasetResponse>()
 *
 * Available field types: BOOL, NUMBER, STRING, ARRAY, OBJECT, ANY
 */

// Forward declaration for types that depend on zjson
namespace zjson
{
class Value;
}

namespace validator
{

/**
 * Field types for schema validation
 */
enum class FieldType
{
  TYPE_BOOL,
  TYPE_NUMBER,
  TYPE_STRING,
  TYPE_ARRAY,
  TYPE_OBJECT,
  TYPE_ANY
};

/**
 * Schema field descriptor
 */
struct FieldDescriptor
{
  std::string name;
  FieldType type;
  bool required;
  FieldType array_element_type;
  const std::vector<FieldDescriptor> *nested_schema;

  FieldDescriptor(const std::string &n, FieldType t, bool req)
      : name(n), type(t), required(req), array_element_type(FieldType::TYPE_ANY), nested_schema(nullptr)
  {
  }

  FieldDescriptor(const std::string &n, FieldType t, bool req, FieldType array_elem_type)
      : name(n), type(t), required(req), array_element_type(array_elem_type), nested_schema(nullptr)
  {
  }

  FieldDescriptor(const std::string &n, FieldType t, bool req, const std::vector<FieldDescriptor> *nested)
      : name(n), type(t), required(req), array_element_type(FieldType::TYPE_ANY), nested_schema(nested)
  {
  }

  FieldDescriptor(const std::string &n, FieldType t, bool req, FieldType array_elem_type, const std::vector<FieldDescriptor> *nested)
      : name(n), type(t), required(req), array_element_type(array_elem_type), nested_schema(nested)
  {
  }
};

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
 * Validation result type - either success (nullopt) or error (string message)
 * Similar to Rust Result<(), E> pattern
 * - nullopt = validation success
 * - some(message) = validation error with message
 */
using ValidationResult = zstd::optional<std::string>;

/**
 * Validator function type - validates JSON parameters
 * Can be null to indicate no validation is required
 */
using ValidatorFn = std::function<ValidationResult(const zjson::Value &)>;

/**
 * Validate a JSON value against a schema (implementation in validator.cpp)
 */
ValidationResult validate_schema(const zjson::Value &params,
                                 const std::vector<FieldDescriptor> &schema,
                                 bool allow_unknown_fields = false);

} // namespace validator

// Macros for defining schemas

#define FIELD_REQUIRED(name, type) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_##type, true)

#define FIELD_OPTIONAL(name, type) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_##type, false)

#define FIELD_REQUIRED_ARRAY(name, element_type) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_ARRAY, true, validator::FieldType::TYPE_##element_type)

#define FIELD_OPTIONAL_ARRAY(name, element_type) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_ARRAY, false, validator::FieldType::TYPE_##element_type)

#define FIELD_REQUIRED_OBJECT(name, StructType) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_OBJECT, true, &validator::SchemaRegistry<StructType>::fields)

#define FIELD_OPTIONAL_OBJECT(name, StructType) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_OBJECT, false, &validator::SchemaRegistry<StructType>::fields)

#define FIELD_REQUIRED_OBJECT_ARRAY(name, StructType) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_ARRAY, true, validator::FieldType::TYPE_OBJECT, &validator::SchemaRegistry<StructType>::fields)

#define FIELD_OPTIONAL_OBJECT_ARRAY(name, StructType) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_ARRAY, false, validator::FieldType::TYPE_OBJECT, &validator::SchemaRegistry<StructType>::fields)

#define ZJSON_SCHEMA(StructType, ...)                                 \
  namespace validator                                                 \
  {                                                                   \
  template <>                                                         \
  std::vector<FieldDescriptor> SchemaRegistry<StructType>::fields = { \
      __VA_ARGS__};                                                   \
  }

#endif // VALIDATOR_HPP
