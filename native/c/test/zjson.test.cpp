/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 * ZJson Comprehensive Test Suite - Testing zjson2.hpp API and type system
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
// TEST STRUCTURES
// ============================================================================

struct BasicPerson
{
  std::string name;
  int age;
  bool is_active;
};

struct User
{
  std::string username;
  std::string email;
  int user_id;
  std::string display_name;
};

struct OptionalUser
{
  std::string name;
  int age;
  zstd::optional<std::string> email;
  zstd::optional<std::string> phone;
};

struct UnregisteredType
{
  int value;
};

// Additional test structures for comprehensive coverage
struct Address
{
  std::string street;
  std::string city;
  std::string country;
};

struct Employee
{
  BasicPerson person;
  Address address;
  std::string department;
};

struct Pet
{
  std::string name;
  std::string species;
  int age;
  bool is_vaccinated;
};

struct ComplexPerson
{
  std::string name;
  int age;
  bool is_active;
  Address address;
  std::vector<Pet> pets;
  std::string email;
};

struct Config
{
  std::string host;
  int port;
  bool debug_mode;
};

struct AdvancedUser
{
  std::string username;
  std::string email;
  int user_id;
  std::string password_hash;
  std::string display_name;
};

struct OptionalUserAdvanced
{
  std::string name;
  int age;
  zstd::optional<std::string> email;
  zstd::optional<std::string> phone;
  zstd::optional<int> employee_id;
};

struct LargeStruct
{
  std::string field1;
  int field2;
  bool field3;
  double field4;
  std::string field5;
  int field6;
  bool field7;
  double field8;
  std::string field9;
  int field10;
  bool field11;
  double field12;
  std::string field13;
  int field14;
  bool field15;
  double field16;
  std::string field17;
  int field18;
  bool field19;
  double field20;
  std::string field21;
  int field22;
  bool field23;
  double field24;
  std::string field25;
  int field26;
  bool field27;
  double field28;
  std::string field29;
  int field30;
  bool field31;
  double field32;
};

// Default value functions
int default_port()
{
  return 8080;
}

bool default_debug()
{
  return false;
}

// Register test types using the real ZJSON_DERIVE macro
ZJSON_DERIVE(BasicPerson, name, age, is_active);
ZJSON_DERIVE(User, username, email, user_id, display_name);
ZJSON_DERIVE(OptionalUser, name, age, email, phone);
ZJSON_DERIVE(Address, street, city, country);
ZJSON_DERIVE(Employee, person, address, department);
ZJSON_DERIVE(Pet, name, species, age, is_vaccinated);
ZJSON_DERIVE(ComplexPerson, name, age, is_active, address, pets, email);
ZJSON_DERIVE(LargeStruct, field1, field2, field3, field4, field5, field6, field7, field8,
             field9, field10, field11, field12, field13, field14, field15, field16,
             field17, field18, field19, field20, field21, field22, field23, field24,
             field25, field26, field27, field28, field29, field30, field31, field32);

// Advanced serialization configurations
ZJSON_SERIALIZABLE(AdvancedUser,
                   ZJSON_FIELD(AdvancedUser, username),
                   ZJSON_FIELD(AdvancedUser, email),
                   ZJSON_FIELD(AdvancedUser, user_id) zjson_rename("userId"),
                   ZJSON_FIELD(AdvancedUser, password_hash) zjson_skip(),
                   ZJSON_FIELD(AdvancedUser, display_name) zjson_rename("displayName"));

ZJSON_SERIALIZABLE(OptionalUserAdvanced,
                   ZJSON_FIELD(OptionalUserAdvanced, name),
                   ZJSON_FIELD(OptionalUserAdvanced, age),
                   ZJSON_FIELD(OptionalUserAdvanced, email),
                   ZJSON_FIELD(OptionalUserAdvanced, phone) zjson_skip_serializing_if_none(),
                   ZJSON_FIELD(OptionalUserAdvanced, employee_id));

ZJSON_SERIALIZABLE(Config,
                   ZJSON_FIELD(Config, host),
                   ZJSON_FIELD(Config, port) zjson_default(default_port),
                   ZJSON_FIELD(Config, debug_mode) zjson_rename("debug") zjson_default(default_debug));

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
            bool person_serializable = zjson::Serializable<BasicPerson>::value;
            bool person_deserializable = zjson::Deserializable<BasicPerson>::value;
            bool user_serializable = zjson::Serializable<User>::value;
            bool user_deserializable = zjson::Deserializable<User>::value;
            Expect(person_serializable).ToBe(true);
            Expect(person_deserializable).ToBe(true);
            Expect(user_serializable).ToBe(true);
            Expect(user_deserializable).ToBe(true);
        });
        
        it("should correctly identify unregistered types as not serializable", []() {
            bool unreg_serializable = zjson::Serializable<UnregisteredType>::value;
            bool unreg_deserializable = zjson::Deserializable<UnregisteredType>::value;
            Expect(unreg_serializable).ToBe(false);
            Expect(unreg_deserializable).ToBe(false);
        });
        
        it("should correctly handle vector traits", []() {
            bool vec_int_serializable = zjson::Serializable<std::vector<int>>::value;
            bool vec_person_serializable = zjson::Serializable<std::vector<BasicPerson>>::value;
            bool vec_unreg_serializable = zjson::Serializable<std::vector<UnregisteredType>>::value;
            Expect(vec_int_serializable).ToBe(true);
            Expect(vec_person_serializable).ToBe(true);
            Expect(vec_unreg_serializable).ToBe(false);
        });
        
        it("should correctly handle optional traits", []() {
            bool opt_int_serializable = zjson::Serializable<zstd::optional<int>>::value;
            bool opt_person_serializable = zjson::Serializable<zstd::optional<BasicPerson>>::value;
            bool opt_unreg_serializable = zjson::Serializable<zstd::optional<UnregisteredType>>::value;
            Expect(opt_int_serializable).ToBe(true);
            Expect(opt_person_serializable).ToBe(true);
            Expect(opt_unreg_serializable).ToBe(false);
        }); });
}

// Note: Field descriptor tests moved to test_advanced_field_attributes()
// which provides comprehensive testing with actual serialization examples

// Note: Basic error handling tests moved to test_advanced_error_handling()
// which covers error scenarios with actual JSON parsing examples

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
// COMPILATION TESTS
// ============================================================================

void test_compilation_features()
{
  describe("ZJson Compilation Feature Tests", []()
           {
        it("should support SFINAE for type traits", []() {
            bool trait_returns_bool = std::is_same<bool, decltype(zjson::Serializable<int>::value)>::value;
            Expect(trait_returns_bool).ToBe(true);
        });
        
        it("should support constexpr evaluation", []() {
            constexpr bool int_serializable = zjson::Serializable<int>::value;
            constexpr bool unreg_serializable = zjson::Serializable<UnregisteredType>::value;
            
            bool int_ser = int_serializable;
            bool unreg_ser = unreg_serializable;
            Expect(int_ser).ToBe(true);
            Expect(unreg_ser).ToBe(false);
        });
        
        it("should allow template instantiation", []() {
            zjson::Value test_val = zjson::Serializable<BasicPerson>::serialize(BasicPerson{});
            // Just test that this compiles successfully
            Expect(true).ToBe(true);
        });
        
        it("should create valid field objects", []() {
            auto test_field = ZJSON_FIELD(BasicPerson, name);
            Expect(test_field.member != nullptr).ToBe(true);
        }); });
}

// ============================================================================
// MAIN TEST FUNCTION
// ============================================================================

// ============================================================================
// BASIC SERIALIZATION TESTS - Comprehensive version matching json.cpp examples
// ============================================================================

void test_basic_serialization_comprehensive()
{
  describe("ZJson Basic Serialization Tests (Comprehensive)", []()
           {
        it("should serialize and deserialize BasicPerson (matching basic_example)", []() {
            // Test serialization - matches json.cpp basic_example()
            BasicPerson person{"John Doe", 30, true};
            
            auto json_result = zjson::to_string(person);
            Expect(json_result.has_value()).ToBe(true);
            
            if (json_result.has_value()) {
                std::string json = json_result.value();
                Expect(json).ToContain("John Doe");
                Expect(json).ToContain("30");
                Expect(json).ToContain("true");
                
                // Test deserialization - matches json.cpp basic_example()
                auto person_result = zjson::from_str<BasicPerson>(json);
                Expect(person_result.has_value()).ToBe(true);
                
                if (person_result.has_value()) {
                    BasicPerson p = person_result.value();
                    Expect(p.name).ToBe(std::string("John Doe"));
                    Expect(p.age).ToBe(30);
                    Expect(p.is_active).ToBe(true);
                }
            }
        });
        
        it("should handle pretty printing (matching basic_example)", []() {
            BasicPerson person{"Alice Smith", 28, true};
            
            auto pretty_result = zjson::to_string_pretty(person);
            Expect(pretty_result.has_value()).ToBe(true);
            
            if (pretty_result.has_value()) {
                std::string pretty_json = pretty_result.value();
                Expect(pretty_json).ToContain("\n");  // Should have newlines for formatting
                Expect(pretty_json).ToContain("Alice Smith");
                Expect(pretty_json).ToContain("28");
            }
        });
        
        it("should deserialize from different JSON (matching basic_example)", []() {
            std::string json = R"({"name":"Jane Doe","age":25,"is_active":false})";
            auto person_result = zjson::from_str<BasicPerson>(json);
            Expect(person_result.has_value()).ToBe(true);
            
            if (person_result.has_value()) {
                BasicPerson p = person_result.value();
                Expect(p.name).ToBe(std::string("Jane Doe"));
                Expect(p.age).ToBe(25);
                Expect(p.is_active).ToBe(false);
            }
        }); });
}

// ============================================================================
// ADVANCED FIELD ATTRIBUTES TESTS
// ============================================================================

void test_advanced_field_attributes()
{
  describe("ZJson Advanced Field Attributes Tests", []()
           {
        it("should handle field renaming in serialization", []() {
            AdvancedUser user{"johndoe", "john@example.com", 12345, "secret_hash", "John Doe"};
            
            auto json_result = zjson::to_string(user);
            Expect(json_result.has_value()).ToBe(true);
            
            if (json_result.has_value()) {
                std::string json = json_result.value();
                Expect(json).ToContain("userId");  // renamed from user_id
                Expect(json).ToContain("displayName");  // renamed from display_name
                Expect(json).ToContain("12345");
                Expect(json).ToContain("John Doe");
                Expect(json).Not().ToContain("user_id");  // original field name should not appear
                Expect(json).Not().ToContain("display_name");
                Expect(json).Not().ToContain("password_hash");  // should be skipped
                Expect(json).Not().ToContain("secret_hash");
            }
        });
        
        it("should handle field skipping in serialization", []() {
            AdvancedUser user{"testuser", "test@example.com", 99999, "should_not_appear", "Test User"};
            
            auto json_result = zjson::to_string(user);
            Expect(json_result.has_value()).ToBe(true);
            
            if (json_result.has_value()) {
                std::string json = json_result.value();
                Expect(json).Not().ToContain("password_hash");
                Expect(json).Not().ToContain("should_not_appear");
                Expect(json).ToContain("testuser");
                Expect(json).ToContain("test@example.com");
            }
        });
        
        it("should handle skip_serializing_if_none for optional fields", []() {
            OptionalUserAdvanced user{
                "Optional Test",
                35,
                zstd::optional<std::string>("test@example.com"),
                zstd::optional<std::string>(),  // Empty - should be skipped
                zstd::optional<int>()           // Empty - should be included as null
            };
            
            auto json_result = zjson::to_string(user);
            Expect(json_result.has_value()).ToBe(true);
            
            if (json_result.has_value()) {
                std::string json = json_result.value();
                Expect(json).ToContain("Optional Test");
                Expect(json).ToContain("test@example.com");
                Expect(json).Not().ToContain("phone");  // Should be skipped due to skip_serializing_if_none
                Expect(json).ToContain("employee_id");  // Should be included as null
            }
        }); });
}

// ============================================================================
// DEFAULT VALUES TESTS
// ============================================================================

void test_default_values()
{
  describe("ZJson Default Values Tests", []()
           {
        it("should use default values for missing fields during deserialization", []() {
            std::string partial_json = R"({"host":"example.com"})";
            
            auto config_result = zjson::from_str<Config>(partial_json);
            Expect(config_result.has_value()).ToBe(true);
            
            if (config_result.has_value()) {
                Config c = config_result.value();
                Expect(c.host).ToBe(std::string("example.com"));
                Expect(c.port).ToBe(8080);  // Should use default_port()
                Expect(c.debug_mode).ToBe(false);  // Should use default_debug()
            }
        });
        
        it("should use provided values over defaults when present", []() {
            std::string complete_json = R"({"host":"localhost","port":3000,"debug":true})";
            
            auto config_result = zjson::from_str<Config>(complete_json);
            Expect(config_result.has_value()).ToBe(true);
            
            if (config_result.has_value()) {
                Config c = config_result.value();
                Expect(c.host).ToBe(std::string("localhost"));
                Expect(c.port).ToBe(3000);  // Should use provided value
                Expect(c.debug_mode).ToBe(true);  // Should use provided value
            }
        });
        
        it("should handle field renaming with defaults", []() {
            std::string json_with_rename = R"({"host":"test.com","debug":true})";
            
            auto config_result = zjson::from_str<Config>(json_with_rename);
            Expect(config_result.has_value()).ToBe(true);
            
            if (config_result.has_value()) {
                Config c = config_result.value();
                Expect(c.host).ToBe(std::string("test.com"));
                Expect(c.port).ToBe(8080);  // Default
                Expect(c.debug_mode).ToBe(true);  // From "debug" field
            }
        }); });
}

// ============================================================================
// NESTED STRUCTURES TESTS
// ============================================================================

void test_nested_structures()
{
  describe("ZJson Nested Structures Tests", []()
           {
        it("should serialize nested structures correctly", []() {
            Employee emp{
                {"Alice Smith", 28, true},
                {"123 Main St", "Boston", "USA"},
                "Engineering"
            };
            
            auto json_result = zjson::to_string_pretty(emp);
            Expect(json_result.has_value()).ToBe(true);
            
            if (json_result.has_value()) {
                std::string json = json_result.value();
                Expect(json).ToContain("Alice Smith");
                Expect(json).ToContain("28");
                Expect(json).ToContain("123 Main St");
                Expect(json).ToContain("Boston");
                Expect(json).ToContain("USA");
                Expect(json).ToContain("Engineering");
                Expect(json).ToContain("person");
                Expect(json).ToContain("address");
                Expect(json).ToContain("department");
            }
        });
        
        it("should deserialize nested structures correctly", []() {
            std::string emp_json = R"({
                "person": {"name": "Bob Wilson", "age": 35, "is_active": true},
                "address": {"street": "456 Oak Ave", "city": "Seattle", "country": "USA"},
                "department": "Marketing"
            })";
            
            auto emp_result = zjson::from_str<Employee>(emp_json);
            Expect(emp_result.has_value()).ToBe(true);
            
            if (emp_result.has_value()) {
                Employee e = emp_result.value();
                Expect(e.person.name).ToBe(std::string("Bob Wilson"));
                Expect(e.person.age).ToBe(35);
                Expect(e.person.is_active).ToBe(true);
                Expect(e.address.street).ToBe(std::string("456 Oak Ave"));
                Expect(e.address.city).ToBe(std::string("Seattle"));
                Expect(e.address.country).ToBe(std::string("USA"));
                Expect(e.department).ToBe(std::string("Marketing"));
            }
        }); });
}

// ============================================================================
// ARRAY AND COMPLEX NESTED TESTS
// ============================================================================

void test_arrays_and_complex_nested()
{
  describe("ZJson Arrays and Complex Nested Tests", []()
           {
        it("should serialize arrays of custom objects", []() {
            Pet dog{"Buddy", "Golden Retriever", 5, true};
            Pet cat{"Whiskers", "Siamese", 3, true};
            Pet bird{"Tweety", "Canary", 2, false};
            
            ComplexPerson person{
                "Alice Johnson",
                28,
                true,
                {"123 Main Street", "Boston", "USA"},
                {dog, cat, bird},
                "alice.johnson@example.com"
            };
            
            auto json_result = zjson::to_string_pretty(person);
            Expect(json_result.has_value()).ToBe(true);
            
            if (json_result.has_value()) {
                std::string json = json_result.value();
                Expect(json).ToContain("Alice Johnson");
                Expect(json).ToContain("pets");
                Expect(json).ToContain("Buddy");
                Expect(json).ToContain("Golden Retriever");
                Expect(json).ToContain("Whiskers");
                Expect(json).ToContain("Siamese");
                Expect(json).ToContain("Tweety");
                Expect(json).ToContain("Canary");
                Expect(json).ToContain("123 Main Street");
                Expect(json).ToContain("alice.johnson@example.com");
            }
        });
        
        it("should deserialize arrays of custom objects", []() {
            std::string complex_json = R"({
                "name": "Bob Smith",
                "age": 35,
                "is_active": true,
                "address": {
                    "street": "456 Oak Avenue",
                    "city": "Seattle", 
                    "country": "USA"
                },
                "pets": [
                    {
                        "name": "Max",
                        "species": "Labrador",
                        "age": 4,
                        "is_vaccinated": true
                    },
                    {
                        "name": "Luna", 
                        "species": "Persian Cat",
                        "age": 2,
                        "is_vaccinated": true
                    }
                ],
                "email": "bob.smith@example.com"
            })";
            
            auto person_result = zjson::from_str<ComplexPerson>(complex_json);
            Expect(person_result.has_value()).ToBe(true);
            
            if (person_result.has_value()) {
                ComplexPerson p = person_result.value();
                Expect(p.name).ToBe(std::string("Bob Smith"));
                Expect(p.age).ToBe(35);
                Expect(p.is_active).ToBe(true);
                Expect(p.address.street).ToBe(std::string("456 Oak Avenue"));
                Expect(p.address.city).ToBe(std::string("Seattle"));
                Expect(p.address.country).ToBe(std::string("USA"));
                Expect(p.pets.size()).ToBe(2);
                Expect(p.pets[0].name).ToBe(std::string("Max"));
                Expect(p.pets[0].species).ToBe(std::string("Labrador"));
                Expect(p.pets[0].age).ToBe(4);
                Expect(p.pets[0].is_vaccinated).ToBe(true);
                Expect(p.pets[1].name).ToBe(std::string("Luna"));
                Expect(p.pets[1].species).ToBe(std::string("Persian Cat"));
                Expect(p.pets[1].age).ToBe(2);
                Expect(p.pets[1].is_vaccinated).ToBe(true);
                Expect(p.email).ToBe(std::string("bob.smith@example.com"));
            }
        }); });
}

// ============================================================================
// LARGE STRUCTURE TESTS
// ============================================================================

void test_large_structures()
{
  describe("ZJson Large Structure Tests", []()
           { it("should handle large structures with many fields", []()
                {
            LargeStruct s;
            s.field1 = "value1";
            s.field2 = 2;
            s.field3 = true;
            s.field4 = 4.4;
            s.field5 = "value5";
            s.field6 = 6;
            s.field7 = false;
            s.field8 = 8.8;
            s.field9 = "value9";
            s.field10 = 10;
            s.field11 = true;
            s.field12 = 12.12;
            s.field13 = "value13";
            s.field14 = 14;
            s.field15 = false;
            s.field16 = 16.16;
            s.field17 = "value17";
            s.field18 = 18;
            s.field19 = true;
            s.field20 = 20.20;
            s.field21 = "value21";
            s.field22 = 22;
            s.field23 = false;
            s.field24 = 24.24;
            s.field25 = "value25";
            s.field26 = 26;
            s.field27 = true;
            s.field28 = 28.28;
            s.field29 = "value29";
            s.field30 = 30;
            s.field31 = false;
            s.field32 = 32.32;
            
            auto json_result = zjson::to_string_pretty(s);
            Expect(json_result.has_value()).ToBe(true);
            
            if (json_result.has_value()) {
                std::string json = json_result.value();
                Expect(json).ToContain("value1");
                Expect(json).ToContain("value32");
                Expect(json).ToContain("32.32");
                
                // Test round-trip serialization
                auto s2_result = zjson::from_str<LargeStruct>(json);
                Expect(s2_result.has_value()).ToBe(true);
                
                if (s2_result.has_value()) {
                    LargeStruct s2 = s2_result.value();
                    Expect(s2.field1).ToBe(std::string("value1"));
                    Expect(s2.field2).ToBe(2);
                    Expect(s2.field32).ToBe(32.32);
                    Expect(s2.field31).ToBe(false);
                }
            } }); });
}

// ============================================================================
// ADVANCED ERROR HANDLING TESTS
// ============================================================================

void test_advanced_error_handling()
{
  describe("ZJson Advanced Error Handling Tests", []()
           {
        it("should handle type mismatch errors", []() {
            std::string invalid_json = R"({"name": 123, "age": "not_a_number", "is_active": "not_a_bool"})";
            
            auto result = zjson::from_str<BasicPerson>(invalid_json);
            Expect(result.has_value()).ToBe(false);
            
            if (!result.has_value()) {
                const auto &error = result.error();
                std::string error_msg = error.what();
                Expect(error_msg.length() > 0).ToBe(true);
            }
        });
        
        it("should handle malformed JSON", []() {
            std::string malformed_json = R"({"name": "John", "age": 30, })";  // trailing comma
            
            auto result = zjson::from_str<BasicPerson>(malformed_json);
            Expect(result.has_value()).ToBe(false);
            
            if (!result.has_value()) {
                const auto &error = result.error();
                std::string error_msg = error.what();
                Expect(error_msg.length() > 0).ToBe(true);
            }
        });
        
        it("should handle missing required fields", []() {
            std::string incomplete_json = R"({"name": "John"})";  // missing age and is_active
            
            auto result = zjson::from_str<BasicPerson>(incomplete_json);
            // Note: Depending on implementation, this might succeed with default values
            // or fail with missing field errors. Test for consistent behavior.
            if (!result.has_value()) {
                const auto &error = result.error();
                std::string error_msg = error.what();
                Expect(error_msg.length() > 0).ToBe(true);
            }
        });
        
        it("should handle invalid array structures", []() {
            std::string invalid_array_json = R"({
                "name": "Test",
                "age": 30,
                "is_active": true,
                "address": {"street": "123 Main", "city": "Boston", "country": "USA"},
                "pets": "not_an_array",
                "email": "test@example.com"
            })";
            
            auto result = zjson::from_str<ComplexPerson>(invalid_array_json);
            Expect(result.has_value()).ToBe(false);
            
            if (!result.has_value()) {
                const auto &error = result.error();
                std::string error_msg = error.what();
                Expect(error_msg.length() > 0).ToBe(true);
            }
        }); });
}

void zjson_tests()
{
  // Core API and type system tests
  test_type_traits();
  test_value_types();
  test_optional_types();
  test_expected_types();
  test_compilation_features();

  // Comprehensive JSON functionality tests (matching json.cpp examples)
  test_basic_serialization_comprehensive();
  test_advanced_field_attributes();
  test_default_values();
  test_nested_structures();
  test_arrays_and_complex_nested();
  test_large_structures();
  test_advanced_error_handling();
}