/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 * ZJson Comprehensive Examples - Complete demonstration of ZJson C++ API
 * This file contains all usage examples from zjson2.hpp as executable code
 */

#include <iostream>
#include <string>
#include <cstdio>
#include "zjsonm.h"
#include "zjsontype.h"
#include "zjson2.hpp"

// ============================================================================
// BASIC USAGE - Similar to Rust #[derive(Serialize, Deserialize)]
// ============================================================================

struct BasicPerson
{
  std::string name;
  int age;
  bool is_active;
};

// Register the type for serialization/deserialization
ZJSON_DERIVE(BasicPerson, name, age, is_active);

void basic_example()
{
  std::cout << "\n=== BASIC USAGE EXAMPLE ===" << std::endl;

  BasicPerson person{"John Doe", 30, true};

  // Serialize to JSON string
  auto json_result = zjson::to_string(person);
  if (json_result.has_value())
  {
    std::cout << "✅ Serialization: " << json_result.value() << std::endl;
    // Output: {"name":"John Doe","age":30,"is_active":true}
  }
  else
  {
    std::cout << "❌ Serialization Error: " << json_result.error().what() << std::endl;
  }

  // Pretty print JSON
  auto pretty_result = zjson::to_string_pretty(person);
  if (pretty_result.has_value())
  {
    std::cout << "✅ Pretty JSON:\n"
              << pretty_result.value() << std::endl;
  }
  else
  {
    std::cout << "❌ Pretty JSON Error: " << pretty_result.error().what() << std::endl;
  }

  // Deserialize from JSON string
  std::string json = R"({"name":"Jane Doe","age":25,"is_active":false})";
  auto person_result = zjson::from_str<BasicPerson>(json);
  if (person_result.has_value())
  {
    BasicPerson p = person_result.value();
    std::cout << "✅ Deserialization: Name=" << p.name << ", Age=" << p.age << ", Active=" << (p.is_active ? "true" : "false") << std::endl;
  }
  else
  {
    std::cout << "❌ Error: " << person_result.error().what() << std::endl;
  }
}

// ============================================================================
// ADVANCED USAGE - Field Attributes for JSON serialization
// ============================================================================

struct User
{
  std::string username;
  std::string email;
  int user_id;
  std::string password_hash; // should be skipped
  std::string display_name;  // should be renamed
};

// Manual field configuration with attributes
ZJSON_SERIALIZABLE(User,
                   ZJSON_FIELD(User, username),
                   ZJSON_FIELD(User, email),
                   ZJSON_FIELD(User, user_id) zjson_rename("userId"),
                   ZJSON_FIELD(User, password_hash) zjson_skip(),
                   ZJSON_FIELD(User, display_name) zjson_rename("displayName"));

void advanced_attributes_example()
{
  std::cout << "\n=== ADVANCED FIELD ATTRIBUTES EXAMPLE ===" << std::endl;

  User user{"johndoe", "john@example.com", 12345, "secret_hash", "John Doe"};

  auto json_result = zjson::to_string_pretty(user);
  if (json_result.has_value())
  {
    std::cout << "✅ User with field attributes:\n"
              << json_result.value() << std::endl;
    std::cout << "Note: 'password_hash' skipped, 'user_id' → 'userId', 'display_name' → 'displayName'" << std::endl;
  }
}

// ============================================================================
// OPTIONAL FIELDS AND DEFAULTS for JSON serialization
// ============================================================================

struct OptionalUser
{
  std::string name;                  // Required field
  int age;                           // Required field
  zstd::optional<std::string> email; // Optional field - includes null by default
  zstd::optional<std::string> phone; // Optional field - includes null by default
  zstd::optional<int> employee_id;   // Optional field - includes null by default
};

ZJSON_SERIALIZABLE(OptionalUser,
                   ZJSON_FIELD(OptionalUser, name),
                   ZJSON_FIELD(OptionalUser, age),
                   ZJSON_FIELD(OptionalUser, email),                                  // Default: includes null if empty (serde-compatible)
                   ZJSON_FIELD(OptionalUser, phone) zjson_skip_serializing_if_none(), // Skip if no value (like serde's skip_serializing_if)
                   ZJSON_FIELD(OptionalUser, employee_id)                             // Default: includes null if empty (serde-compatible)
);

void optional_example()
{
  std::cout << "\n=== OPTIONAL FIELDS EXAMPLE ===" << std::endl;

  // Create user with some optional fields missing
  OptionalUser user{
      "John Doe",
      30,
      zstd::optional<std::string>("john@example.com"), // email present
      zstd::optional<std::string>(),                   // phone not set
      zstd::optional<int>()                            // employee_id not set
  };

  // Serialize - by default includes null for empty optionals (serde-compatible)
  auto json = zjson::to_string_pretty(user);
  if (json.has_value())
  {
    std::cout << "✅ Optional fields serialization:\n"
              << json.value() << std::endl;
    std::cout << "Note: 'phone' skipped due to zjson_skip_serializing_if_none(), 'employee_id' included as null" << std::endl;
  }

  // Deserialize - missing optional fields become empty optionals
  std::string partial_json = R"({"name":"Jane","age":25,"email":"jane@example.com"})";
  auto parsed_user = zjson::from_str<OptionalUser>(partial_json);
  if (parsed_user.has_value())
  {
    OptionalUser u = parsed_user.value();
    std::cout << "✅ Deserialization with missing optionals:" << std::endl;
    std::cout << "  Name: " << u.name << ", Age: " << u.age << std::endl;
    std::cout << "  Email: " << (u.email.has_value() ? u.email.value() : "NOT SET") << std::endl;
    std::cout << "  Phone: " << (u.phone.has_value() ? u.phone.value() : "NOT SET") << std::endl;
    std::cout << "  Employee ID: " << (u.employee_id.has_value() ? std::to_string(u.employee_id.value()) : "NOT SET") << std::endl;
  }
}

// ============================================================================
// DEFAULT VALUES example
// ============================================================================

struct Config
{
  std::string host;
  int port;
  bool debug_mode;
};

int default_port()
{
  return 8080;
}
bool default_debug()
{
  return false;
}

ZJSON_SERIALIZABLE(Config,
                   ZJSON_FIELD(Config, host),
                   ZJSON_FIELD(Config, port) zjson_default(default_port),
                   ZJSON_FIELD(Config, debug_mode) zjson_rename("debug") zjson_default(default_debug));

void defaults_example()
{
  std::cout << "\n=== DEFAULT VALUES EXAMPLE ===" << std::endl;

  // Test with complete JSON
  std::string complete_json = R"({"host":"localhost","port":3000,"debug":true})";
  auto config1 = zjson::from_str<Config>(complete_json);
  if (config1.has_value())
  {
    Config c = config1.value();
    std::cout << "✅ Complete config: host=" << c.host << ", port=" << c.port << ", debug=" << (c.debug_mode ? "true" : "false") << std::endl;
  }

  // Test with missing fields (should use defaults)
  std::string partial_json = R"({"host":"example.com"})";
  auto config2 = zjson::from_str<Config>(partial_json);
  if (config2.has_value())
  {
    Config c = config2.value();
    std::cout << "✅ Partial config (using defaults): host=" << c.host << ", port=" << c.port << ", debug=" << (c.debug_mode ? "true" : "false") << std::endl;
  }
}

// ============================================================================
// NESTED STRUCTURES (automatic serialization of registered types)
// ============================================================================

struct Address
{
  std::string street;
  std::string city;
  std::string country;
};

struct Employee
{
  BasicPerson person; // Previously registered type
  Address address;    // Will be registered below
  std::string department;
};

ZJSON_DERIVE(Address, street, city, country);
ZJSON_DERIVE(Employee, person, address, department);

void nested_example()
{
  std::cout << "\n=== NESTED STRUCTURES EXAMPLE ===" << std::endl;

  Employee emp{
      {"Alice Smith", 28, true},
      {"123 Main St", "Boston", "USA"},
      "Engineering"};

  auto json_result = zjson::to_string_pretty(emp);
  if (json_result.has_value())
  {
    std::cout << "✅ Nested structures serialization:\n"
              << json_result.value() << std::endl;
  }

  // Test deserialization
  std::string emp_json = R"({
         "person": {"name": "Bob Wilson", "age": 35, "is_active": true},
         "address": {"street": "456 Oak Ave", "city": "Seattle", "country": "USA"},
         "department": "Marketing"
     })";

  auto emp_result = zjson::from_str<Employee>(emp_json);
  if (emp_result.has_value())
  {
    Employee e = emp_result.value();
    std::cout << "✅ Nested deserialization: " << e.person.name << " from " << e.address.city << ", " << e.department << std::endl;
  }
}

// ============================================================================
// COMPLEX NESTED WITH ARRAYS
// ============================================================================

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

ZJSON_DERIVE(Pet, name, species, age, is_vaccinated);
ZJSON_DERIVE(ComplexPerson, name, age, is_active, address, pets, email);

void complex_nested_example()
{
  std::cout << "\n=== COMPLEX NESTED WITH ARRAYS EXAMPLE ===" << std::endl;

  // Create pets
  Pet dog{"Buddy", "Golden Retriever", 5, true};
  Pet cat{"Whiskers", "Siamese", 3, true};
  Pet bird{"Tweety", "Canary", 2, false};

  // Create address
  Address address{"123 Main Street", "Boston", "USA"};

  // Create person with nested objects and array
  ComplexPerson person{
      "Alice Johnson",            // name
      28,                         // age
      true,                       // is_active
      address,                    // nested address object
      {dog, cat, bird},           // array of pets
      "alice.johnson@example.com" // email
  };

  auto json_result = zjson::to_string_pretty(person);
  if (json_result.has_value())
  {
    std::cout << "✅ Complex nested with arrays:\n"
              << json_result.value() << std::endl;
  }

  // Test deserialization
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
  if (person_result.has_value())
  {
    ComplexPerson p = person_result.value();
    std::cout << "✅ Complex deserialization: " << p.name << " (age " << p.age << ")" << std::endl;
    std::cout << "  Address: " << p.address.street << ", " << p.address.city << ", " << p.address.country << std::endl;
    std::cout << "  Pets (" << p.pets.size() << " total):" << std::endl;

    for (size_t i = 0; i < p.pets.size(); ++i)
    {
      const Pet &pet = p.pets[i];
      std::cout << "    - " << pet.name << " (" << pet.species << ", age " << pet.age
                << ", vaccinated: " << (pet.is_vaccinated ? "yes" : "no") << ")" << std::endl;
    }
    std::cout << "  Email: " << p.email << std::endl;
  }
}

// ============================================================================
// ERROR HANDLING (using zstd::expected<T, E> pattern similar to Rust Result<T, E>)
// ============================================================================

void error_handling_example()
{
  std::cout << "\n=== ERROR HANDLING EXAMPLE ===" << std::endl;

  std::string invalid_json = R"({"name": 123, "age": "not_a_number"})";

  auto result = zjson::from_str<BasicPerson>(invalid_json);
  if (!result.has_value())
  {
    const auto &error = result.error();
    std::cout << "✅ Expected error caught: " << error.what() << std::endl;
    std::cout << "  Error kind: " << static_cast<int>(error.kind()) << std::endl;
  }
  else
  {
    std::cout << "❌ Expected error but got success!" << std::endl;
  }

  // Test malformed JSON
  std::string malformed_json = R"({"name": "John", "age": 30, })"; // trailing comma
  auto result2 = zjson::from_str<BasicPerson>(malformed_json);
  if (!result2.has_value())
  {
    std::cout << "✅ Malformed JSON error: " << result2.error().what() << std::endl;
  }
}

// ============================================================================
// COMPILE-TIME TYPE CHECKING (similar to Rust trait bounds)
// ============================================================================

// Helper functions for C++14 compatibility
template <typename T>
void serialize_if_possible_impl(const T &obj, const std::string &type_name, std::true_type)
{
  auto result = zjson::to_string(obj);
  if (result.has_value())
  {
    std::cout << "✅ " << type_name << " is serializable: " << result.value() << std::endl;
  }
}

template <typename T>
void serialize_if_possible_impl(const T &obj, const std::string &type_name, std::false_type)
{
  std::cout << "❌ " << type_name << " is not serializable" << std::endl;
}

template <typename T>
void serialize_if_possible(const T &obj, const std::string &type_name)
{
  serialize_if_possible_impl<T>(obj, type_name, typename std::integral_constant<bool, zjson::Serializable<T>::value>{});
}

struct UnregisteredType
{
  int value;
};

void compile_time_checking_example()
{
  std::cout << "\n=== COMPILE-TIME TYPE CHECKING EXAMPLE ===" << std::endl;

  BasicPerson person{"John", 30, true};
  UnregisteredType unregistered{42};

  serialize_if_possible(person, "BasicPerson");
  serialize_if_possible(unregistered, "UnregisteredType");
  serialize_if_possible(std::string("hello"), "std::string");
  serialize_if_possible(42, "int");
}

// ============================================================================
// LARGE STRUCT (32 fields)
// ============================================================================

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

ZJSON_DERIVE(LargeStruct,
             field1, field2, field3, field4, field5, field6, field7, field8,
             field9, field10, field11, field12, field13, field14, field15, field16,
             field17, field18, field19, field20, field21, field22, field23, field24,
             field25, field26, field27, field28, field29, field30, field31, field32);

void large_struct_example()
{
  std::cout << "\n=== LARGE STRUCT (32 FIELDS) EXAMPLE ===" << std::endl;

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
  if (json_result.has_value())
  {
    std::cout << "✅ Large struct serialization successful" << std::endl;

    auto s2_result = zjson::from_str<LargeStruct>(json_result.value());
    if (s2_result.has_value())
    {
      LargeStruct s2 = s2_result.value();
      std::cout << "✅ Large struct deserialization successful" << std::endl;
      if (s2.field1 == s.field1 && s2.field32 == s.field32)
      {
        std::cout << "✅ Deserialized values verified (spot check)" << std::endl;
      }
      else
      {
        std::cout << "❌ Deserialized values verification failed" << std::endl;
      }
    }
    else
    {
      std::cout << "❌ Large struct deserialization error: " << s2_result.error().what() << std::endl;
    }
  }
  else
  {
    std::cout << "❌ Large struct serialization error: " << json_result.error().what() << std::endl;
  }
}

// ============================================================================
// VALUE CONVERSION EXAMPLES (zjson::to_value and zjson::from_value)
// ============================================================================

void value_conversion_example()
{
  std::cout << "\n=== VALUE CONVERSION EXAMPLE ===" << std::endl;

  // Example 1: Convert typed objects to Value
  BasicPerson person{"Alice Johnson", 28, true};

  auto person_value_result = zjson::to_value(person);
  if (person_value_result.has_value())
  {
    zjson::Value person_value = person_value_result.value();
    std::cout << "✅ Converted BasicPerson to Value" << std::endl;

    // Can access the Value like any JSON object using dynamic indexing
    if (person_value.is_object())
    {
      // New dynamic access - much cleaner than manual iteration!
      std::string name = person_value["name"].as_string();
      int age = person_value["age"].as_int();
      bool active = person_value["is_active"].as_bool();

      std::cout << "  Name from Value (dynamic access): " << name << std::endl;
      std::cout << "  Age from Value (dynamic access): " << age << std::endl;
      std::cout << "  Active from Value (dynamic access): " << (active ? "true" : "false") << std::endl;
    }

    // Convert Value back to typed object
    auto person_back_result = zjson::from_value<BasicPerson>(person_value);
    if (person_back_result.has_value())
    {
      BasicPerson person_back = person_back_result.value();
      std::cout << "✅ Converted Value back to BasicPerson" << std::endl;
      std::cout << "  Round-trip: " << person_back.name << ", age " << person_back.age << std::endl;
    }
  }

  // Example 2: Convert primitive types
  auto int_value_result = zjson::to_value(42);
  if (int_value_result.has_value())
  {
    zjson::Value int_value = int_value_result.value();
    std::cout << "✅ Converted int to Value: " << int_value.as_int() << std::endl;

    auto int_back_result = zjson::from_value<int>(int_value);
    if (int_back_result.has_value())
    {
      int int_back = int_back_result.value();
      std::cout << "✅ Converted Value back to int: " << int_back << std::endl;
    }
  }

  // Example 3: Convert arrays
  std::vector<int> numbers = {1, 2, 3, 4, 5};
  auto array_value_result = zjson::to_value(numbers);
  if (array_value_result.has_value())
  {
    zjson::Value array_value = array_value_result.value();
    std::cout << "✅ Converted vector<int> to Value array" << std::endl;

    auto numbers_back_result = zjson::from_value<std::vector<int>>(array_value);
    if (numbers_back_result.has_value())
    {
      std::vector<int> numbers_back = numbers_back_result.value();
      std::cout << "✅ Converted Value back to vector<int>, size: " << numbers_back.size() << std::endl;
    }
  }
}

// ============================================================================
// DYNAMIC JSON PROCESSING EXAMPLE
// ============================================================================

void dynamic_json_processing_example()
{
  std::cout << "\n=== DYNAMIC JSON PROCESSING EXAMPLE ===" << std::endl;

  // Parse JSON into Value first for flexible processing
  std::string complex_json = R"({
    "users": [
      {"name": "Alice", "age": 30, "is_active": true},
      {"name": "Bob", "age": 25, "is_active": false}
    ],
    "metadata": {
      "total_count": 2,
      "last_updated": "2023-01-01",
      "api_version": "v2.1"
    },
    "settings": {
      "enable_notifications": true,
      "max_results": 100
    }
  })";

  auto root_result = zjson::from_str<zjson::Value>(complex_json);
  if (root_result.has_value())
  {
    zjson::Value root = root_result.value();
    std::cout << "✅ Parsed JSON into Value for dynamic processing" << std::endl;

    // NEW: Dynamic access with indexing operators (serde_json style!)
    std::cout << "\n--- Dynamic Indexing Examples ---" << std::endl;

    // Access nested data with chained indexing like serde_json
    std::string first_user_name = root["users"][0]["name"].as_string();
    int first_user_age = root["users"][0]["age"].as_int();
    bool first_user_active = root["users"][0]["is_active"].as_bool();

    std::cout << "✅ First user (chained access): " << first_user_name
              << " (age " << first_user_age << ", active: " << (first_user_active ? "yes" : "no") << ")" << std::endl;

    std::string second_user_name = root["users"][1]["name"].as_string();
    int second_user_age = root["users"][1]["age"].as_int();

    std::cout << "✅ Second user (chained access): " << second_user_name
              << " (age " << second_user_age << ")" << std::endl;

    // Access metadata with dynamic indexing
    int total_count = root["metadata"]["total_count"].as_int();
    std::string api_version = root["metadata"]["api_version"].as_string();
    std::string last_updated = root["metadata"]["last_updated"].as_string();

    std::cout << "✅ Metadata (dynamic access):" << std::endl;
    std::cout << "  Total count: " << total_count << std::endl;
    std::cout << "  API version: " << api_version << std::endl;
    std::cout << "  Last updated: " << last_updated << std::endl;

    // Access settings
    bool notifications = root["settings"]["enable_notifications"].as_bool();
    int max_results = root["settings"]["max_results"].as_int();

    std::cout << "✅ Settings (dynamic access):" << std::endl;
    std::cout << "  Notifications enabled: " << (notifications ? "yes" : "no") << std::endl;
    std::cout << "  Max results: " << max_results << std::endl;

    // OLD WAY: Manual iteration (still works for completeness)
    std::cout << "\n--- Traditional Manual Access (still supported) ---" << std::endl;
    if (root.is_object())
    {
      const auto &obj = root.as_object();

      // Extract and convert users array to typed vector
      auto users_it = obj.find("users");
      if (users_it != obj.end())
      {
        auto users_result = zjson::from_value<std::vector<BasicPerson>>(users_it->second);
        if (users_result.has_value())
        {
          std::vector<BasicPerson> users = users_result.value();
          std::cout << "✅ Extracted " << users.size() << " users as typed objects (manual way)" << std::endl;
        }
      }
    }
  }
}

// ============================================================================
// VALUE MANIPULATION EXAMPLE
// ============================================================================

void value_manipulation_example()
{
  std::cout << "\n=== VALUE MANIPULATION EXAMPLE ===" << std::endl;

  // Build JSON dynamically using Values and typed objects
  BasicPerson person1{"Alice", 30, true};
  BasicPerson person2{"Bob", 25, false};

  // Convert persons to Values
  auto person1_val_result = zjson::to_value(person1);
  auto person2_val_result = zjson::to_value(person2);

  if (person1_val_result.has_value() && person2_val_result.has_value())
  {
    std::cout << "\n--- Traditional Value Construction ---" << std::endl;
    // Build a complex structure (traditional way)
    zjson::Value root = zjson::Value::create_object();
    zjson::Value users_array = zjson::Value::create_array();

    users_array.add_to_array(person1_val_result.value());
    users_array.add_to_array(person2_val_result.value());

    root.add_to_object("users", users_array);
    root.add_to_object("total_count", zjson::Value(2));
    root.add_to_object("generated_at", zjson::Value("2023-12-01T10:30:00Z"));

    // Add a settings object
    zjson::Value settings = zjson::Value::create_object();
    settings.add_to_object("format_version", zjson::Value("1.0"));
    settings.add_to_object("include_metadata", zjson::Value(true));
    root.add_to_object("settings", settings);

    std::cout << "✅ Built complex JSON structure using traditional Value manipulation" << std::endl;

    std::cout << "\n--- NEW: Dynamic Construction with Indexing ---" << std::endl;
    // NEW: Build structure using dynamic indexing (much cleaner!)
    zjson::Value dynamic_root; // Starts as null, auto-converts to object

    // Auto-creates nested structure as we access it
    dynamic_root["users"][0] = person1_val_result.value();
    dynamic_root["users"][1] = person2_val_result.value();
    dynamic_root["total_count"] = zjson::Value(2);
    dynamic_root["generated_at"] = zjson::Value("2023-12-01T10:30:00Z");
    dynamic_root["settings"]["format_version"] = zjson::Value("1.0");
    dynamic_root["settings"]["include_metadata"] = zjson::Value(true);

    std::cout << "✅ Built identical structure using dynamic indexing (much cleaner!)" << std::endl;

    // Convert both results to JSON string to verify they're identical
    auto json_result = zjson::to_string_pretty(root);
    auto dynamic_json_result = zjson::to_string_pretty(dynamic_root);

    if (json_result.has_value() && dynamic_json_result.has_value())
    {
      std::cout << "✅ Traditional construction result:\n"
                << json_result.value() << std::endl;
      std::cout << "✅ Dynamic construction result:\n"
                << dynamic_json_result.value() << std::endl;

      // Verify they're the same
      if (json_result.value() == dynamic_json_result.value())
      {
        std::cout << "✅ Both construction methods produce identical JSON!" << std::endl;
      }
    }

    // Extract parts back to typed objects using dynamic access
    std::cout << "\n--- Extracting with Dynamic Access ---" << std::endl;
    auto users_result = zjson::from_value<std::vector<BasicPerson>>(dynamic_root["users"]);
    if (users_result.has_value())
    {
      std::vector<BasicPerson> users = users_result.value();
      std::cout << "✅ Extracted " << users.size() << " users using dynamic access: dynamic_root[\"users\"]" << std::endl;

      // Show individual access too
      std::string first_name = dynamic_root["users"][0]["name"].as_string();
      std::cout << "  First user name (chained access): " << first_name << std::endl;
    }
  }
}

// ============================================================================
// MAIN FUNCTION - Run all examples
// ============================================================================

int main()
{
  try
  {
    std::cout << "=== ZJson Comprehensive Examples ===" << std::endl;
    std::cout << "Demonstrating all features of the ZJson C++ API" << std::endl;

    basic_example();
    advanced_attributes_example();
    optional_example();
    defaults_example();
    nested_example();
    complex_nested_example();
    error_handling_example();
    compile_time_checking_example();
    large_struct_example();

    // New value conversion and utility function examples
    value_conversion_example();
    dynamic_json_processing_example();
    value_manipulation_example();

    std::cout << "\n=== All Examples Completed Successfully! ===" << std::endl;
    return 0;
  }
  catch (const std::exception &e)
  {
    std::cout << "❌ Exception: " << e.what() << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cout << "❌ Unknown exception!" << std::endl;
    return 2;
  }
}