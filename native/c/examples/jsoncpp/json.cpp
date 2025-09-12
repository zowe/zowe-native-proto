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

#include <iostream>
#include <string>
#include <vector>
#include "zjsonm.h"
#include "zjsontype.h"
#include "zjson2.hpp"

struct Pet
{
  std::string name;
  std::string species;
  int age;
  bool is_vaccinated;
};

struct Address
{
  std::string street;
  std::string city;
  std::string state;
  int zip_code;
};

struct Person
{
  std::string name;
  int age;
  bool is_active;
  Address address;
  std::vector<Pet> pets;
  std::string email;
};

// Register the types for serialization/deserialization (in dependency order)
ZSERDE_DERIVE(Pet, name, species, age, is_vaccinated);
ZSERDE_DERIVE(Address, street, city, state, zip_code);
ZSERDE_DERIVE(Person, name, age, is_active, address, pets, email);

void basic_example()
{
  std::cout << "=== ZSerde Advanced Example - Nested Objects and Arrays ===" << std::endl;

  // Create nested data structures similar to serde_json examples

  // Create pets
  Pet dog{"Buddy", "Golden Retriever", 5, true};
  Pet cat{"Whiskers", "Siamese", 3, true};
  Pet bird{"Tweety", "Canary", 2, false};

  // Create address
  Address address{"123 Main Street", "Boston", "MA", 2101};

  // Create person with nested objects and array
  Person person{
      "Alice Johnson",            // name
      28,                         // age
      true,                       // is_active
      address,                    // nested address object
      {dog, cat, bird},           // array of pets
      "alice.johnson@example.com" // email
  };

  std::cout << "\n--- Test 1: Serialization of Complex Nested Structure ---" << std::endl;
  auto json_result = zserde::to_string(person);
  if (json_result.has_value())
  {
    std::cout << "✅ Serialization SUCCESS!" << std::endl;
    std::cout << "Compact JSON: " << json_result.value() << std::endl;
  }
  else
  {
    std::cout << "❌ Serialization FAILED" << std::endl;
    return;
  }

  std::cout << "\n--- Test 2: Pretty-Printed JSON ---" << std::endl;
  auto pretty_result = zserde::to_string_pretty(person);
  if (pretty_result.has_value())
  {
    std::cout << "✅ Pretty print SUCCESS!" << std::endl;
    std::cout << "Formatted JSON:\n"
              << pretty_result.value() << std::endl;
  }
  else
  {
    std::cout << "❌ Pretty print FAILED" << std::endl;
  }

  std::cout << "\n--- Test 3: Deserialization of Complex JSON ---" << std::endl;
  // Complex JSON with nested objects and arrays (similar to serde_json examples)
  std::string complex_json = R"({
    "name": "Bob Smith",
    "age": 35,
    "is_active": true,
    "address": {
      "street": "456 Oak Avenue",
      "city": "Seattle", 
      "state": "WA",
      "zip_code": 98101
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

  auto person_result = zserde::from_str<Person>(complex_json);
  if (person_result.has_value())
  {
    Person p = person_result.value();
    std::cout << "✅ Deserialization SUCCESS!" << std::endl;
    std::cout << "Person: " << p.name << " (age " << p.age << ")" << std::endl;
    std::cout << "Address: " << p.address.street << ", " << p.address.city << ", " << p.address.state << std::endl;
    std::cout << "Pets (" << p.pets.size() << " total):" << std::endl;

    for (size_t i = 0; i < p.pets.size(); ++i)
    {
      const Pet &pet = p.pets[i];
      std::cout << "  - " << pet.name << " (" << pet.species << ", age " << pet.age
                << ", vaccinated: " << (pet.is_vaccinated ? "yes" : "no") << ")" << std::endl;
    }
    std::cout << "Email: " << p.email << std::endl;
  }
  else
  {
    std::cout << "❌ Deserialization FAILED: " << person_result.error().what() << std::endl;
  }

  std::cout << "\n--- Test 4: Individual Structure Serialization ---" << std::endl;

  // Test individual Pet serialization
  auto pet_json = zserde::to_string(dog);
  if (pet_json.has_value())
  {
    std::cout << "✅ Pet serialization: " << pet_json.value() << std::endl;
  }

  // Test individual Address serialization
  auto address_json = zserde::to_string(address);
  if (address_json.has_value())
  {
    std::cout << "✅ Address serialization: " << address_json.value() << std::endl;
  }

  std::cout << "\n=== All tests completed! ===" << std::endl;
}

int main()
{
  try
  {
    std::cout << "Starting ZSerde Advanced Nested Example..." << std::endl;
    basic_example();
    std::cout << "\nAdvanced example completed successfully!" << std::endl;
    return 0;
  }
  catch (const std::exception &e)
  {
    std::cout << "Exception: " << e.what() << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cout << "Unknown exception!" << std::endl;
    return 2;
  }
}
