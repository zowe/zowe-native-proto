/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 * ZJson Basic Examples - Demonstrating core ZJson C++ API
 * Similar to Rust serde_json usage patterns
 */

#include <iostream>
#include <string>
#include <vector>
#include "zjsonm.h"
#include "zjsontype.h"
#include "zjson.hpp"

// ============================================================================
// PARSING JSON AS STRONGLY TYPED DATA STRUCTURES
// ============================================================================

struct Person
{
  std::string name;
  int age;
  std::vector<std::string> phones;
};

// Register the type for serialization/deserialization
ZJSON_DERIVE(Person, name, age, phones);

void typed_example()
{
  std::cout << "\n=== STRONGLY TYPED PARSING EXAMPLE ===" << std::endl;

  // Some JSON input data as a string. Maybe this comes from the user.
  std::string data = R"(
      {
          "name": "John Doe",
          "age": 43,
          "phones": [
              "+44 1234567",
              "+44 2345678"
          ]
      })";

  // Parse the string of data into a Person object. This is exactly the
  // same function as the one that produced serde_json::Value above, but
  // now we are asking it for a Person as output.
  auto person_result = zjson::from_str<Person>(data);
  if (person_result.has_value())
  {
    Person p = person_result.value();
    // Do things just like with any other C++ data structure.
    std::cout << "✅ Please call " << p.name << " at the number " << p.phones[0] << std::endl;
  }
  else
  {
    std::cout << "❌ Parse error: " << person_result.error().what() << std::endl;
  }
}

// ============================================================================
// CREATING JSON BY SERIALIZING DATA STRUCTURES
// ============================================================================

struct Address
{
  std::string street;
  std::string city;
};

ZJSON_DERIVE(Address, street, city);

void serialization_example()
{
  std::cout << "\n=== SERIALIZATION EXAMPLE ===" << std::endl;

  // Some data structure.
  Address address{"10 Downing Street", "London"};

  // Serialize it to a JSON string.
  auto json_result = zjson::to_string(address);
  if (json_result.has_value())
  {
    // Print, write to a file, or send to an HTTP server.
    std::cout << "✅ Address JSON: " << json_result.value() << std::endl;
  }
  else
  {
    std::cout << "❌ Serialization error: " << json_result.error().what() << std::endl;
  }

  // Pretty print version
  auto pretty_result = zjson::to_string_pretty(address);
  if (pretty_result.has_value())
  {
    std::cout << "✅ Pretty Address JSON:\n"
              << pretty_result.value() << std::endl;
  }
}

// ============================================================================
// ERROR HANDLING (using zstd::expected<T, E> pattern similar to Rust Result<T, E>)
// ============================================================================

void error_handling_example()
{
  std::cout << "\n=== ERROR HANDLING EXAMPLE ===" << std::endl;

  std::string invalid_json = R"({"name": 123, "age": "not_a_number"})";

  auto result = zjson::from_str<Person>(invalid_json);
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
  auto result2 = zjson::from_str<Person>(malformed_json);
  if (!result2.has_value())
  {
    std::cout << "✅ Malformed JSON error: " << result2.error().what() << std::endl;
  }
}

// ============================================================================
// MAIN FUNCTION - Run basic examples
// ============================================================================

int main()
{
  try
  {
    std::cout << "=== ZJson Basic Examples ===" << std::endl;
    std::cout << "Demonstrating core ZJson C++ API similar to serde_json" << std::endl;

    typed_example();
    serialization_example();
    error_handling_example();

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
