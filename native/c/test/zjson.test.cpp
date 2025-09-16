/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 * ZJson Test Suite - Testing zjson2.hpp API functionality
 *
 * NOTE: This test suite uses the real zjson2.hpp implementation and tests:
 * - API structure, type traits, and macro system
 * - Real serialization traits and field descriptors
 * - Compile-time type checking and template instantiation
 *
 * This test runs in the z/OS test environment where all required
 * headers and APIs are available.
 */

#include "zjson.test.hpp"
#include "ztest.hpp"
#include <string>
#include <vector>
#include <cassert>

// Include the real zjson2.hpp implementation
#include "../zjson2.hpp"

using namespace std;
using namespace ztst;

// ============================================================================
// TEST STRUCTURES - Focused on API testing (examples covered in json.cpp)
// ============================================================================

struct SimpleStruct
{
  int id;
  std::string name;
};

struct UnregisteredType
{
  int value;
};

struct TestOptional
{
  zstd::optional<int> opt_int;
  zstd::optional<std::string> opt_string;
};

// Register minimal test types for API testing
ZJSON_DERIVE(SimpleStruct, id, name);
ZJSON_DERIVE(TestOptional, opt_int, opt_string);

// ============================================================================
// TYPE TRAIT TESTS
// ============================================================================

void test_type_traits()
{
  describe("ZJson Type Trait Tests", []()
           {
        it("should correctly identify basic types as serializable", []() {
            bool bool_serializable = zjson::Serializable<bool>::value;
            bool int_serializable = zjson::Serializable<int>::value;
            bool string_serializable = zjson::Serializable<std::string>::value;
            Expect(bool_serializable).ToBe(true);
            Expect(int_serializable).ToBe(true);
            Expect(string_serializable).ToBe(true);
        });

        it("should correctly identify basic types as deserializable", []() {
            bool bool_deserializable = zjson::Deserializable<bool>::value;
            bool int_deserializable = zjson::Deserializable<int>::value;
            bool string_deserializable = zjson::Deserializable<std::string>::value;
            Expect(bool_deserializable).ToBe(true);
            Expect(int_deserializable).ToBe(true);
            Expect(string_deserializable).ToBe(true);
        });

        it("should correctly identify registered struct traits", []() {
            bool simple_serializable = zjson::Serializable<SimpleStruct>::value;
            bool simple_deserializable = zjson::Deserializable<SimpleStruct>::value;
            Expect(simple_serializable).ToBe(true);
            Expect(simple_deserializable).ToBe(true);
        });

        it("should correctly identify unregistered types as not serializable", []() {
            bool unreg_serializable = zjson::Serializable<UnregisteredType>::value;
            bool unreg_deserializable = zjson::Deserializable<UnregisteredType>::value;
            Expect(unreg_serializable).ToBe(false);
            Expect(unreg_deserializable).ToBe(false);
        });

        it("should correctly handle vector traits", []() {
            bool vec_int_serializable = zjson::Serializable<std::vector<int>>::value;
            bool vec_simple_serializable = zjson::Serializable<std::vector<SimpleStruct>>::value;
            bool vec_unreg_serializable = zjson::Serializable<std::vector<UnregisteredType>>::value;
            Expect(vec_int_serializable).ToBe(true);
            Expect(vec_simple_serializable).ToBe(true);
            Expect(vec_unreg_serializable).ToBe(false);
        });

        it("should correctly handle optional traits", []() {
            bool opt_int_serializable = zjson::Serializable<zstd::optional<int>>::value;
            bool opt_simple_serializable = zjson::Serializable<zstd::optional<SimpleStruct>>::value;
            bool opt_unreg_serializable = zjson::Serializable<zstd::optional<UnregisteredType>>::value;
            Expect(opt_int_serializable).ToBe(true);
            Expect(opt_simple_serializable).ToBe(true);
            Expect(opt_unreg_serializable).ToBe(false);
        });

        it("should support constexpr trait evaluation", []() {
            constexpr bool int_serializable = zjson::Serializable<int>::value;
            constexpr bool unreg_serializable = zjson::Serializable<UnregisteredType>::value;

            // Convert constexpr values to runtime values for testing
            bool int_ser = int_serializable;
            bool unreg_ser = unreg_serializable;
            Expect(int_ser).ToBe(true);
            Expect(unreg_ser).ToBe(false);
        }); });
}

// ============================================================================
// VALUE TYPE TESTS
// ============================================================================

void test_value_types()
{
  describe("ZJson Value Type Tests", []()
           {
        it("should detect null values correctly", []() {
            zjson::Value null_val;
            Expect(null_val.is_null()).ToBe(true);
            Expect(null_val.get_type() == zjson::Value::Null).ToBe(true);
        });

        it("should detect boolean values correctly", []() {
            zjson::Value bool_val(true);
            Expect(bool_val.is_bool()).ToBe(true);
            Expect(bool_val.get_type() == zjson::Value::Bool).ToBe(true);
        });

        it("should detect numeric values correctly", []() {
            zjson::Value int_val(42);
            Expect(int_val.is_number()).ToBe(true);
            Expect(int_val.get_type() == zjson::Value::Number).ToBe(true);

            zjson::Value double_val(3.14);
            Expect(double_val.is_number()).ToBe(true);
        });

        it("should detect string values correctly", []() {
            zjson::Value string_val("hello");
            Expect(string_val.is_string()).ToBe(true);
            Expect(string_val.get_type() == zjson::Value::String).ToBe(true);
        });

        it("should have exclusive type detection", []() {
            zjson::Value bool_val(true);
            Expect(bool_val.is_null()).ToBe(false);
            Expect(bool_val.is_string()).ToBe(false);

            zjson::Value string_val("hello");
            Expect(string_val.is_number()).ToBe(false);
        });

        it("should support object indexing with string keys", []() {
            zjson::Value obj = zjson::Value::create_object();
            obj.add_to_object("name", zjson::Value("Alice"));
            obj.add_to_object("age", zjson::Value(30));

            // Test const access
            const zjson::Value& const_obj = obj;
            std::string name = const_obj["name"].as_string();
            int age = const_obj["age"].as_int();

            Expect(name == "Alice").ToBe(true);
            Expect(age).ToBe(30);

            // Test that missing keys return null
            const zjson::Value& missing = const_obj["nonexistent"];
            Expect(missing.is_null()).ToBe(true);
        });

        it("should support mutable object indexing", []() {
            zjson::Value obj = zjson::Value::create_object();

            // Test mutable access (creates entries)
            obj["name"] = zjson::Value("Bob");
            obj["age"] = zjson::Value(25);

            Expect(obj["name"].as_string() == "Bob").ToBe(true);
            Expect(obj["age"].as_int()).ToBe(25);
        });

        it("should support array indexing with size_t indices", []() {
            zjson::Value arr = zjson::Value::create_array();
            arr.add_to_array(zjson::Value("first"));
            arr.add_to_array(zjson::Value("second"));
            arr.add_to_array(zjson::Value("third"));

            // Test const access
            const zjson::Value& const_arr = arr;
            std::string first = const_arr[0].as_string();
            std::string second = const_arr[1].as_string();

            Expect(first == "first").ToBe(true);
            Expect(second == "second").ToBe(true);

            // Test that out-of-bounds returns null
            const zjson::Value& missing = const_arr[10];
            Expect(missing.is_null()).ToBe(true);
        });

        it("should support mutable array indexing with auto-expansion", []() {
            zjson::Value arr = zjson::Value::create_array();

            // Test auto-expansion
            arr[0] = zjson::Value("first");
            arr[2] = zjson::Value("third");  // Creates gap at index 1

            Expect(arr[0].as_string() == "first").ToBe(true);
            Expect(arr[1].is_null()).ToBe(true);  // Gap filled with null
            Expect(arr[2].as_string() == "third").ToBe(true);
        });

        it("should auto-convert null to object on string indexing", []() {
            zjson::Value null_val;  // Starts as null
            Expect(null_val.is_null()).ToBe(true);

            // Should auto-convert to object
            null_val["key"] = zjson::Value("value");

            Expect(null_val.is_object()).ToBe(true);
            Expect(null_val["key"].as_string() == "value").ToBe(true);
        });

        it("should auto-convert null to array on numeric indexing", []() {
            zjson::Value null_val;  // Starts as null
            Expect(null_val.is_null()).ToBe(true);

            // Should auto-convert to array
            null_val[0] = zjson::Value("item");

            Expect(null_val.is_array()).ToBe(true);
            Expect(null_val[0].as_string() == "item").ToBe(true);
        }); });
}

// ============================================================================
// OPTIONAL TYPE TESTS
// ============================================================================

void test_optional_types()
{
  describe("ZJson Optional Type Tests", []()
           {
        it("should handle empty optionals", []() {
            zstd::optional<int> empty_opt;
            Expect(empty_opt.has_value()).ToBe(false);
        });

        it("should handle optionals with values", []() {
            zstd::optional<int> valued_opt(42);
            Expect(valued_opt.has_value()).ToBe(true);
            Expect(valued_opt.value()).ToBe(42);
        });

        it("should handle string optionals", []() {
            zstd::optional<std::string> string_opt("hello");
            Expect(string_opt.has_value()).ToBe(true);
            Expect(string_opt.value()).ToBe(std::string("hello"));
        });

        it("should support copying", []() {
            zstd::optional<int> valued_opt(42);
            zstd::optional<int> copied_opt = valued_opt;
            Expect(copied_opt.has_value()).ToBe(true);
            Expect(copied_opt.value()).ToBe(42);
        }); });
}

// ============================================================================
// EXPECTED TYPE TESTS
// ============================================================================

void test_expected_types()
{
  describe("ZJson Expected Type Tests", []()
           {
        it("should handle successful results", []() {
            zstd::expected<int, zjson::Error> success_result(42);
            Expect(success_result.has_value()).ToBe(true);
            Expect(success_result.value()).ToBe(42);
        });

        it("should handle error results", []() {
            auto error = zjson::Error::invalid_value("test error");
            zstd::expected<int, zjson::Error> error_result = zstd::make_unexpected(error);
            Expect(error_result.has_value()).ToBe(false);
            Expect(error_result.error().kind() == zjson::Error::InvalidValue).ToBe(true);
        });

        it("should handle string results", []() {
            zstd::expected<std::string, zjson::Error> string_result("hello world");
            Expect(string_result.has_value()).ToBe(true);
            Expect(string_result.value()).ToBe(std::string("hello world"));
        }); });
}

// ============================================================================
// BASIC API TESTS - Focused tests complementing json.cpp examples
// ============================================================================

void test_basic_api()
{
  describe("ZJson Basic API Tests", []()
           {
        it("should handle basic serialization API", []() {
            // Test that the API compiles and basic functionality works
            // Actual serialization examples are covered in json.cpp
            SimpleStruct simple{42, "test"};

            // Test that the serialization traits are properly set up
            bool is_serializable = zjson::Serializable<SimpleStruct>::value;
            Expect(is_serializable).ToBe(true);

            // Test basic Value operations
            zjson::Value test_val(42);
            Expect(test_val.is_number()).ToBe(true);
        });

        it("should create valid field objects with ZJSON_FIELD macro", []() {
            auto test_field = ZJSON_FIELD(SimpleStruct, id);
            Expect(test_field.member != nullptr).ToBe(true);
        });

        it("should handle error cases in parsing", []() {
            std::string invalid_json = R"({"invalid": json})";
            auto result = zjson::from_str<zjson::Value>(invalid_json);
            Expect(result.has_value()).ToBe(false);

            const auto &error = result.error();
            std::string error_msg = error.what();
            Expect(error_msg.length() > 0).ToBe(true);
        });

        it("should support chained access patterns", []() {
            // Test parsing and chained access together
            std::string json = R"({
                "users": [
                    {"name": "Alice", "profile": {"age": 30}},
                    {"name": "Bob", "profile": {"age": 25}}
                ]
            })";

            auto result = zjson::from_str<zjson::Value>(json);
            Expect(result.has_value()).ToBe(true);

            zjson::Value root = result.value();

            // Test deep chained access
            std::string alice_name = root["users"][0]["name"].as_string();
            int alice_age = root["users"][0]["profile"]["age"].as_int();

            Expect(alice_name == "Alice").ToBe(true);
            Expect(alice_age).ToBe(30);

            // Test second element
            std::string bob_name = root["users"][1]["name"].as_string();
            int bob_age = root["users"][1]["profile"]["age"].as_int();

            Expect(bob_name == "Bob").ToBe(true);
            Expect(bob_age).ToBe(25);
        });

        it("should support dynamic construction with chaining", []() {
            zjson::Value root;  // Starts as null

            // Build nested structure using chained assignment
            root["company"]["name"] = zjson::Value("Tech Corp");
            root["company"]["employees"][0]["name"] = zjson::Value("Alice");
            root["company"]["employees"][0]["id"] = zjson::Value(1);
            root["company"]["employees"][1]["name"] = zjson::Value("Bob");
            root["company"]["employees"][1]["id"] = zjson::Value(2);
            root["metadata"]["count"] = zjson::Value(2);

            // Verify the structure was created correctly
            Expect(root.is_object()).ToBe(true);
            Expect(root["company"]["name"].as_string() == "Tech Corp").ToBe(true);
            Expect(root["company"]["employees"][0]["name"].as_string() == "Alice").ToBe(true);
            Expect(root["company"]["employees"][1]["id"].as_int()).ToBe(2);
            Expect(root["metadata"]["count"].as_int()).ToBe(2);
        }); });
}

// ============================================================================
// DYNAMIC ACCESS EDGE CASES
// ============================================================================

void test_dynamic_access_edge_cases()
{
  describe("ZJson Dynamic Access Edge Cases", []()
           {
        it("should handle type mismatch errors gracefully", []() {
            zjson::Value string_val("hello");

            try {
                // Should throw when trying to index string as object
                auto result = string_val["key"];
                Expect(false).ToBe(true); // Should not reach here
            } catch (const zjson::Error& e) {
                Expect(e.kind() == zjson::Error::InvalidType).ToBe(true);
            }

            try {
                // Should throw when trying to index string as array
                auto result = string_val[0];
                Expect(false).ToBe(true); // Should not reach here
            } catch (const zjson::Error& e) {
                Expect(e.kind() == zjson::Error::InvalidType).ToBe(true);
            }
        });

        it("should handle mixed access patterns", []() {
            zjson::Value root;

            // Mix traditional and dynamic construction
            root["data"] = zjson::Value::create_array();
            root["data"].add_to_array(zjson::Value("item1"));
            root["data"][1] = zjson::Value("item2");  // Dynamic indexing

            // Verify both methods work together
            Expect(root["data"][0].as_string() == "item1").ToBe(true);
            Expect(root["data"][1].as_string() == "item2").ToBe(true);
        });

        it("should handle deep nesting with mixed types", []() {
            zjson::Value complex;

            // Create deeply nested structure with arrays and objects
            complex["level1"]["level2"][0]["level3"] = zjson::Value("deep_value");
            complex["level1"]["array"][0] = zjson::Value(100);
            complex["level1"]["array"][1]["nested"] = zjson::Value(true);

            // Verify deep access works
            std::string deep_val = complex["level1"]["level2"][0]["level3"].as_string();
            int array_val = complex["level1"]["array"][0].as_int();
            bool nested_bool = complex["level1"]["array"][1]["nested"].as_bool();

            Expect(deep_val == "deep_value").ToBe(true);
            Expect(array_val).ToBe(100);
            Expect(nested_bool).ToBe(true);
        });

        it("should preserve structure consistency", []() {
            zjson::Value obj;

            // Build structure
            obj["users"][0]["name"] = zjson::Value("Alice");
            obj["users"][0]["settings"]["theme"] = zjson::Value("dark");
            obj["users"][1]["name"] = zjson::Value("Bob");

            // Verify structure integrity
            Expect(obj.is_object()).ToBe(true);
            Expect(obj["users"].is_array()).ToBe(true);
            Expect(obj["users"][0].is_object()).ToBe(true);
            Expect(obj["users"][0]["settings"].is_object()).ToBe(true);

            // Verify we can serialize it without errors
            auto serialization_result = zjson::to_value(obj);
            Expect(serialization_result.has_value()).ToBe(true);
            auto json_str_result = zjson::to_string(obj);
            Expect(json_str_result.has_value()).ToBe(true);
        });

        it("should handle safe access to non-existent paths", []() {
            zjson::Value root;
            root["existing"]["path"] = zjson::Value("value");

            // Access non-existent paths should return null, not throw
            const zjson::Value& missing1 = root["nonexistent"];
            const zjson::Value& missing2 = root["existing"]["nonexistent"];

            Expect(missing1.is_null()).ToBe(true);
            Expect(missing2.is_null()).ToBe(true);

            // Test that indexing wrong types throws appropriately
            try {
                // This should throw because we're indexing a string as object
                auto invalid = root["existing"]["path"]["invalid"];
                Expect(false).ToBe(true); // Should not reach here
            } catch (const zjson::Error& e) {
                Expect(e.kind() == zjson::Error::InvalidType).ToBe(true);
            }
        });

        it("should handle array bounds safely", []() {
            zjson::Value arr;
            arr[0] = zjson::Value("first");
            arr[1] = zjson::Value("second");

            // Out of bounds access on const should return null
            const zjson::Value& const_arr = arr;
            const zjson::Value& out_of_bounds = const_arr[10];
            Expect(out_of_bounds.is_null()).ToBe(true);

            // Mutable access should expand array
            arr[5] = zjson::Value("sixth");
            Expect(arr[5].as_string() == "sixth").ToBe(true);
            Expect(arr[3].is_null()).ToBe(true);  // Gap filled with nulls
        }); });
}

void zjson_tests()
{
  // Core API and type system tests - focused on API functionality
  test_type_traits();
  test_value_types();
  test_optional_types();
  test_expected_types();
  test_basic_api();
  test_dynamic_access_edge_cases();
}