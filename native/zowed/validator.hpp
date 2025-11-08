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
#include <functional>

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

// Forward declaration for nested schema support
struct FieldDescriptor;

/**
 * Schema field descriptor with nested schema support
 */
struct NestedSchemaInfo
{
  const FieldDescriptor *fields;
  size_t count;

  NestedSchemaInfo() : fields(nullptr), count(0)
  {
  }
  NestedSchemaInfo(const FieldDescriptor *f, size_t c) : fields(f), count(c)
  {
  }
};

/**
 * Schema field descriptor
 * Note: const char* used for field names (lightweight, no std::string allocation)
 */
struct FieldDescriptor
{
  const char *name; // Changed from std::string to const char* for constexpr compatibility
  FieldType type;
  bool required;
  FieldType array_element_type;
  NestedSchemaInfo nested_schema; // Nested schema with count

  FieldDescriptor(const char *n, FieldType t, bool req)
      : name(n), type(t), required(req), array_element_type(FieldType::TYPE_ANY), nested_schema()
  {
  }

  FieldDescriptor(const char *n, FieldType t, bool req, FieldType array_elem_type)
      : name(n), type(t), required(req), array_element_type(array_elem_type), nested_schema()
  {
  }

  FieldDescriptor(const char *n, FieldType t, bool req, const FieldDescriptor *nested, size_t nested_count)
      : name(n), type(t), required(req), array_element_type(FieldType::TYPE_ANY), nested_schema(nested, nested_count)
  {
  }

  FieldDescriptor(const char *n, FieldType t, bool req, FieldType array_elem_type, const FieldDescriptor *nested, size_t nested_count)
      : name(n), type(t), required(req), array_element_type(array_elem_type), nested_schema(nested, nested_count)
  {
  }
};

/**
 * Schema registry - maps struct types to their schema definitions
 */
template <typename T>
struct SchemaRegistry
{
  static const FieldDescriptor *fields;
  static size_t field_count;
};

// Default values for unspecialized types
template <typename T>
const FieldDescriptor *SchemaRegistry<T>::fields = nullptr;

template <typename T>
size_t SchemaRegistry<T>::field_count = 0;

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
 * Validator function type - validates JSON parameters
 * Can be null to indicate no validation is required
 */
using ValidatorFn = std::function<ValidationResult(const zjson::Value &)>;

/**
 * Validate a JSON value against a schema
 */
ValidationResult validate_schema(const zjson::Value &params,
                                 const FieldDescriptor *schema,
                                 size_t field_count,
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
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_OBJECT, true, validator::SchemaRegistry<StructType>::fields, validator::SchemaRegistry<StructType>::field_count)

#define FIELD_OPTIONAL_OBJECT(name, StructType) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_OBJECT, false, validator::SchemaRegistry<StructType>::fields, validator::SchemaRegistry<StructType>::field_count)

#define FIELD_REQUIRED_OBJECT_ARRAY(name, StructType) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_ARRAY, true, validator::FieldType::TYPE_OBJECT, validator::SchemaRegistry<StructType>::fields, validator::SchemaRegistry<StructType>::field_count)

#define FIELD_OPTIONAL_OBJECT_ARRAY(name, StructType) \
  validator::FieldDescriptor(#name, validator::FieldType::TYPE_ARRAY, false, validator::FieldType::TYPE_OBJECT, validator::SchemaRegistry<StructType>::fields, validator::SchemaRegistry<StructType>::field_count)

#define ZJSON_SCHEMA(StructType, ...)                              \
  namespace validator                                              \
  {                                                                \
  static const FieldDescriptor StructType##_schema_array[] = {     \
      __VA_ARGS__};                                                \
  template <>                                                      \
  const FieldDescriptor *SchemaRegistry<StructType>::fields =      \
      StructType##_schema_array;                                   \
  template <>                                                      \
  size_t SchemaRegistry<StructType>::field_count =                 \
      sizeof(StructType##_schema_array) / sizeof(FieldDescriptor); \
  }

#endif // VALIDATOR_HPP
