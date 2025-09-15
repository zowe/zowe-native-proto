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
#include <vector>
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

template <typename T>
void serialize_if_possible(const T &obj, const std::string &type_name)
{
  if constexpr (zjson::Serializable<T>::value)
  {
    auto result = zjson::to_string(obj);
    if (result.has_value())
    {
      std::cout << "✅ " << type_name << " is serializable: " << result.value() << std::endl;
    }
  }
  else
  {
    std::cout << "❌ " << type_name << " is not serializable" << std::endl;
  }
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