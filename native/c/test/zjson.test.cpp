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

#include <string>
#include <cstring>

#include "ztest.hpp"
#include "../zjson2.hpp"

using namespace std;
using namespace ztst;

void zjson_tests()
{
  describe("zjson basic construction tests",
           []() -> void
           {
             it("should create null json",
                []() -> void
                {
                  zjson::basic_json<> j;
                  Expect(j.is_null()).ToBe(true);
                  Expect(j.type()).ToBe(zjson::value_t::null);
                  Expect(j.empty()).ToBe(true);
                  Expect(j.size()).ToBe(0);
                });

             it("should create boolean json",
                []() -> void
                {
                  zjson::basic_json<> j_true(true);
                  zjson::basic_json<> j_false(false);

                  Expect(j_true.is_boolean()).ToBe(true);
                  Expect(j_true.get<bool>()).ToBe(true);
                  Expect(j_true.type()).ToBe(zjson::value_t::boolean);

                  Expect(j_false.is_boolean()).ToBe(true);
                  Expect(j_false.get<bool>()).ToBe(false);
                  Expect(j_false.type()).ToBe(zjson::value_t::boolean);
                });

             it("should create integer json",
                []() -> void
                {
                  zjson::basic_json<> j(42);
                  Expect(j.is_number()).ToBe(true);
                  Expect(j.is_number_integer()).ToBe(true);
                  Expect(j.get<int>()).ToBe(42);
                  Expect(j.type()).ToBe(zjson::value_t::number_integer);
                });

             it("should create float json",
                []() -> void
                {
                  zjson::basic_json<> j(3.14);
                  Expect(j.is_number()).ToBe(true);
                  Expect(j.is_number_float()).ToBe(true);
                  Expect(j.get<double>()).ToBe(3.14);
                  Expect(j.type()).ToBe(zjson::value_t::number_float);
                });

             it("should create string json",
                []() -> void
                {
                  zjson::basic_json<> j("hello");
                  Expect(j.is_string()).ToBe(true);
                  Expect(j.get<std::string>()).ToBe("hello");
                  Expect(j.type()).ToBe(zjson::value_t::string);

                  std::string str = "world";
                  zjson::basic_json<> j2(str);
                  Expect(j2.get<std::string>()).ToBe("world");
                });

             it("should create nullptr json",
                []() -> void
                {
                  zjson::basic_json<> j(nullptr);
                  Expect(j.is_null()).ToBe(true);
                  Expect(j.type()).ToBe(zjson::value_t::null);
                });
           });

  describe("zjson copy and move semantics",
           []() -> void
           {
             it("should copy construct properly",
                []() -> void
                {
                  zjson::basic_json<> original("test");
                  zjson::basic_json<> copy(original);

                  Expect(copy.is_string()).ToBe(true);
                  Expect(copy.get<std::string>()).ToBe("test");
                  Expect(original.get<std::string>()).ToBe("test");
                });

             it("should move construct properly",
                []() -> void
                {
                  zjson::basic_json<> original("test");
                  zjson::basic_json<> moved(std::move(original));

                  Expect(moved.is_string()).ToBe(true);
                  Expect(moved.get<std::string>()).ToBe("test");
                  Expect(original.is_null()).ToBe(true);
                });

             it("should copy assign properly",
                []() -> void
                {
                  zjson::basic_json<> original(42);
                  zjson::basic_json<> assigned;
                  assigned = original;

                  Expect(assigned.is_number_integer()).ToBe(true);
                  Expect(assigned.get<int>()).ToBe(42);
                  Expect(original.get<int>()).ToBe(42);
                });

             it("should move assign properly",
                []() -> void
                {
                  zjson::basic_json<> original(42);
                  zjson::basic_json<> assigned;
                  assigned = std::move(original);

                  Expect(assigned.is_number_integer()).ToBe(true);
                  Expect(assigned.get<int>()).ToBe(42);
                  Expect(original.is_null()).ToBe(true);
                });
           });

  describe("zjson type checking",
           []() -> void
           {
             it("should identify primitive types correctly",
                []() -> void
                {
                  zjson::basic_json<> j_null;
                  zjson::basic_json<> j_bool(true);
                  zjson::basic_json<> j_int(42);
                  zjson::basic_json<> j_float(3.14);
                  zjson::basic_json<> j_string("test");

                  Expect(j_null.is_primitive()).ToBe(true);
                  Expect(j_bool.is_primitive()).ToBe(true);
                  Expect(j_int.is_primitive()).ToBe(true);
                  Expect(j_float.is_primitive()).ToBe(true);
                  Expect(j_string.is_primitive()).ToBe(true);

                  Expect(j_null.is_structured()).ToBe(false);
                  Expect(j_bool.is_structured()).ToBe(false);
                  Expect(j_int.is_structured()).ToBe(false);
                  Expect(j_float.is_structured()).ToBe(false);
                  Expect(j_string.is_structured()).ToBe(false);
                });

             it("should identify structured types correctly",
                []() -> void
                {
                  zjson::basic_json<> j;
                  j["key"] = "value"; // Creates object

                  zjson::basic_json<> j_array;
                  j_array.push_back(1); // Creates array

                  Expect(j.is_object()).ToBe(true);
                  Expect(j.is_structured()).ToBe(true);
                  Expect(j.is_primitive()).ToBe(false);

                  Expect(j_array.is_array()).ToBe(true);
                  Expect(j_array.is_structured()).ToBe(true);
                  Expect(j_array.is_primitive()).ToBe(false);
                });
           });

  describe("zjson array operations",
           []() -> void
           {
             it("should create and manipulate arrays",
                []() -> void
                {
                  zjson::basic_json<> arr;

                  // Array should be created on first push_back
                  arr.push_back(1);
                  arr.push_back(2);
                  arr.push_back("three");

                  Expect(arr.is_array()).ToBe(true);
                  Expect(arr.size()).ToBe(3);
                  Expect(arr.empty()).ToBe(false);

                  Expect(arr[static_cast<size_t>(0)].get<int>()).ToBe(1);
                  Expect(arr[static_cast<size_t>(1)].get<int>()).ToBe(2);
                  Expect(arr[static_cast<size_t>(2)].get<std::string>()).ToBe("three");
                });

             it("should handle array access with bounds checking",
                []() -> void
                {
                  zjson::basic_json<> arr;
                  arr.push_back(1);
                  arr.push_back(2);

                  // Valid access
                  Expect(arr.at(static_cast<size_t>(0)).get<int>()).ToBe(1);
                  Expect(arr.at(static_cast<size_t>(1)).get<int>()).ToBe(2);

                  // Out of bounds access should throw
                  try
                  {
                    arr.at(5);
                    Expect(false).ToBe(true); // Should not reach here
                  }
                  catch (const zjson::out_of_range &)
                  {
                    Expect(true).ToBe(true); // Expected
                  }
                });

             it("should handle array index operator with auto-resize",
                []() -> void
                {
                  zjson::basic_json<> arr;

                  // Accessing index 0 should create array
                  arr[static_cast<size_t>(0)] = 10;
                  Expect(arr.is_array()).ToBe(true);
                  Expect(arr.size()).ToBe(1);

                  // Accessing index 5 should resize array
                  arr[static_cast<size_t>(5)] = 50;
                  Expect(arr.size()).ToBe(6);
                  Expect(arr[static_cast<size_t>(0)].get<int>()).ToBe(10);
                  Expect(arr[static_cast<size_t>(5)].get<int>()).ToBe(50);

                  // Intermediate elements should be null
                  Expect(arr[static_cast<size_t>(1)].is_null()).ToBe(true);
                  Expect(arr[static_cast<size_t>(4)].is_null()).ToBe(true);
                });

             it("should clear arrays",
                []() -> void
                {
                  zjson::basic_json<> arr;
                  arr.push_back(1);
                  arr.push_back(2);
                  arr.push_back(3);

                  Expect(arr.size()).ToBe(3);

                  arr.clear();
                  Expect(arr.size()).ToBe(0);
                  Expect(arr.empty()).ToBe(true);
                  Expect(arr.is_array()).ToBe(true); // Should still be array type
                });
           });

  describe("zjson object operations",
           []() -> void
           {
             it("should create and manipulate objects",
                []() -> void
                {
                  zjson::basic_json<> obj;

                  // Object should be created on first key assignment
                  obj["name"] = "John";
                  obj["age"] = 30;
                  obj["active"] = true;

                  Expect(obj.is_object()).ToBe(true);
                  Expect(obj.size()).ToBe(3);
                  Expect(obj.empty()).ToBe(false);

                  Expect(obj["name"].get<std::string>()).ToBe("John");
                  Expect(obj["age"].get<int>()).ToBe(30);
                  Expect(obj["active"].get<bool>()).ToBe(true);
                });

             it("should handle object key access with bounds checking",
                []() -> void
                {
                  zjson::basic_json<> obj;
                  obj["key1"] = "value1";
                  obj["key2"] = "value2";

                  // Valid access
                  Expect(obj.at("key1").get<std::string>()).ToBe("value1");
                  Expect(obj.at("key2").get<std::string>()).ToBe("value2");

                  // Non-existent key should throw
                  try
                  {
                    obj.at("nonexistent");
                    Expect(false).ToBe(true); // Should not reach here
                  }
                  catch (const zjson::out_of_range &)
                  {
                    Expect(true).ToBe(true); // Expected
                  }
                });

             it("should handle contains and count operations",
                []() -> void
                {
                  zjson::basic_json<> obj;
                  obj["key1"] = "value1";
                  obj["key2"] = 42;

                  Expect(obj.contains("key1")).ToBe(true);
                  Expect(obj.contains("key2")).ToBe(true);
                  Expect(obj.contains("nonexistent")).ToBe(false);

                  Expect(obj.count("key1")).ToBe(1);
                  Expect(obj.count("key2")).ToBe(1);
                  Expect(obj.count("nonexistent")).ToBe(0);
                });

             it("should handle value with default",
                []() -> void
                {
                  zjson::basic_json<> obj;
                  obj["name"] = "John";
                  obj["age"] = 30;

                  // Existing keys
                  Expect(obj.value("name", std::string("Unknown"))).ToBe("John");
                  Expect(obj.value("age", 0)).ToBe(30);

                  // Non-existent keys
                  Expect(obj.value("height", 0)).ToBe(0);
                  Expect(obj.value("city", std::string("Unknown"))).ToBe("Unknown");
                });

             it("should erase object keys",
                []() -> void
                {
                  zjson::basic_json<> obj;
                  obj["key1"] = "value1";
                  obj["key2"] = "value2";
                  obj["key3"] = "value3";

                  Expect(static_cast<int>(obj.size())).ToBe(3);

                  size_t erased = obj.erase("key2");
                  Expect(static_cast<int>(erased)).ToBe(1);
                  Expect(static_cast<int>(obj.size())).ToBe(2);
                  Expect(obj.contains("key2")).ToBe(false);

                  // Erase non-existent key
                  erased = obj.erase("nonexistent");
                  Expect(static_cast<int>(erased)).ToBe(0);
                  Expect(static_cast<int>(obj.size())).ToBe(2);
                });
           });

  describe("zjson basic operations",
           []() -> void
           {
             it("should handle mixed operations",
                []() -> void
                {
                  zjson::basic_json<> root;

                  // Create a simple structure
                  root["count"] = 3;
                  root["items"].push_back("first");
                  root["items"].push_back("second");
                  root["items"].push_back("third");

                  Expect(root.is_object()).ToBe(true);
                  Expect(root["count"].get<int>()).ToBe(3);
                  Expect(root["items"].is_array()).ToBe(true);
                  Expect(static_cast<int>(root["items"].size())).ToBe(3);

                  Expect(root["items"][static_cast<size_t>(0)].get<std::string>()).ToBe("first");
                  Expect(root["items"][static_cast<size_t>(2)].get<std::string>()).ToBe("third");
                });
           });

  describe("zjson value access and conversion",
           []() -> void
           {
             it("should handle implicit conversions",
                []() -> void
                {
                  zjson::basic_json<> j_bool(true);
                  zjson::basic_json<> j_int(42);
                  zjson::basic_json<> j_float(3.14);
                  zjson::basic_json<> j_string("test");

                  bool b = j_bool;
                  int i = j_int;
                  double d = j_float;
                  std::string s = j_string;

                  Expect(b).ToBe(true);
                  Expect(i).ToBe(42);
                  Expect(d).ToBe(3.14);
                  Expect(s).ToBe("test");
                });

             it("should handle template get with type conversion",
                []() -> void
                {
                  zjson::basic_json<> j_int(42);
                  zjson::basic_json<> j_float(3.14);

                  // Number conversions
                  Expect(j_int.get<int>()).ToBe(42);
                  Expect(j_int.get<double>()).ToBe(42.0);
                  Expect(j_int.get<std::int64_t>()).ToBe(42);

                  Expect(j_float.get<double>()).ToBe(3.14);
                  Expect(j_float.get<int>()).ToBe(3);
                });

             it("should throw on invalid type conversions",
                []() -> void
                {
                  zjson::basic_json<> j_string("not_a_number");

                  try
                  {
                    j_string.get<int>();
                    Expect(false).ToBe(true); // Should not reach here
                  }
                  catch (const zjson::type_error &)
                  {
                    Expect(true).ToBe(true); // Expected
                  }

                  try
                  {
                    j_string.get<bool>();
                    Expect(false).ToBe(true); // Should not reach here
                  }
                  catch (const zjson::type_error &)
                  {
                    Expect(true).ToBe(true); // Expected
                  }
                });
           });

  describe("zjson serialization (dump)",
           []() -> void
           {
             it("should serialize primitive values",
                []() -> void
                {
                  zjson::basic_json<> j_null;
                  zjson::basic_json<> j_bool(true);
                  zjson::basic_json<> j_int(42);
                  zjson::basic_json<> j_float(3.14);
                  zjson::basic_json<> j_string("hello");

                  Expect(j_null.dump()).ToBe("null");
                  Expect(j_bool.dump()).ToBe("true");
                  Expect(j_int.dump()).ToBe("42");
                  // Float comparison - just check it contains the number
                  std::string float_str = j_float.dump();
                  Expect(float_str.find("3.14") != std::string::npos).ToBe(true);
                  Expect(j_string.dump()).ToBe("\"hello\"");
                });

             it("should serialize arrays",
                []() -> void
                {
                  zjson::basic_json<> arr;
                  arr.push_back(1);
                  arr.push_back(2);
                  arr.push_back("three");

                  std::string compact = arr.dump();
                  // Should be compact format
                  Expect(compact.find("\n") == std::string::npos).ToBe(true);
                  Expect(compact.find("[") != std::string::npos).ToBe(true);
                  Expect(compact.find("]") != std::string::npos).ToBe(true);

                  std::string pretty = arr.dump(2);
                  // Should be pretty format with newlines
                  Expect(pretty.find("\n") != std::string::npos).ToBe(true);
                });

             it("should serialize objects",
                []() -> void
                {
                  zjson::basic_json<> obj;
                  obj["name"] = "John";
                  obj["age"] = 30;
                  obj["active"] = true;

                  std::string compact = obj.dump();
                  Expect(compact.find("{") != std::string::npos).ToBe(true);
                  Expect(compact.find("}") != std::string::npos).ToBe(true);
                  Expect(compact.find("\"name\"") != std::string::npos).ToBe(true);

                  std::string pretty = obj.dump(2);
                  Expect(pretty.find("\n") != std::string::npos).ToBe(true);
                });

             it("should handle nested structures",
                []() -> void
                {
                  zjson::basic_json<> root;
                  root["user"]["name"] = "John";
                  root["user"]["age"] = 30;
                  root["user"]["hobbies"].push_back("reading");
                  root["user"]["hobbies"].push_back("coding");

                  std::string serialized = root.dump();
                  Expect(serialized.find("\"user\"") != std::string::npos).ToBe(true);
                  Expect(serialized.find("\"name\"") != std::string::npos).ToBe(true);
                  Expect(serialized.find("\"hobbies\"") != std::string::npos).ToBe(true);
                });

             it("should escape special characters in strings",
                []() -> void
                {
                  zjson::basic_json<> j;
                  j["quote"] = "say \"hello\"";
                  j["newline"] = "line1\nline2";
                  j["tab"] = "before\tafter";

                  std::string serialized = j.dump();
                  Expect(serialized.find("\\\"") != std::string::npos).ToBe(true);
                  Expect(serialized.find("\\n") != std::string::npos).ToBe(true);
                  Expect(serialized.find("\\t") != std::string::npos).ToBe(true);
                });
           });

  describe("zjson error handling",
           []() -> void
           {
             it("should throw type_error for wrong operations",
                []() -> void
                {
                  zjson::basic_json<> j_string("test");

                  try
                  {
                    j_string.push_back(1); // Can't push to string
                    Expect(false).ToBe(true);
                  }
                  catch (const zjson::type_error &)
                  {
                    Expect(true).ToBe(true);
                  }

                  try
                  {
                    j_string["key"] = "value"; // Can't index string
                    Expect(false).ToBe(true);
                  }
                  catch (const zjson::type_error &)
                  {
                    Expect(true).ToBe(true);
                  }
                });

             it("should throw out_of_range for invalid access",
                []() -> void
                {
                  zjson::basic_json<> arr;
                  arr.push_back(1);

                  try
                  {
                    arr.at(10); // Out of bounds
                    Expect(false).ToBe(true);
                  }
                  catch (const zjson::out_of_range &)
                  {
                    Expect(true).ToBe(true);
                  }

                  zjson::basic_json<> obj;
                  obj["key"] = "value";

                  try
                  {
                    obj.at("nonexistent"); // Key doesn't exist
                    Expect(false).ToBe(true);
                  }
                  catch (const zjson::out_of_range &)
                  {
                    Expect(true).ToBe(true);
                  }
                });

             it("should handle invalid operations gracefully",
                []() -> void
                {
                  zjson::basic_json<> j_string("test");

                  // Test that string operations work correctly
                  Expect(j_string.is_string()).ToBe(true);
                  Expect(j_string.get<std::string>()).ToBe("test");

                  // Test that you can't treat string as array/object
                  try
                  {
                    j_string.push_back(1); // Can't push to string
                    Expect(false).ToBe(true);
                  }
                  catch (const zjson::type_error &)
                  {
                    Expect(true).ToBe(true);
                  }
                });
           });

  describe("zjson complex scenarios",
           []() -> void
           {
             it("should handle deeply nested structures",
                []() -> void
                {
                  zjson::basic_json<> root;

                  // Create nested structure
                  root["level1"]["level2"]["level3"]["value"] = 42;
                  root["level1"]["level2"]["array"].push_back("item1");
                  root["level1"]["level2"]["array"].push_back("item2");
                  root["level1"]["another"]["field"] = true;

                  Expect(root["level1"]["level2"]["level3"]["value"].get<int>()).ToBe(42);
                  Expect(root["level1"]["level2"]["array"][static_cast<size_t>(0)].get<std::string>()).ToBe("item1");
                  Expect(root["level1"]["another"]["field"].get<bool>()).ToBe(true);

                  // Verify structure types
                  Expect(root.is_object()).ToBe(true);
                  Expect(root["level1"].is_object()).ToBe(true);
                  Expect(root["level1"]["level2"]["array"].is_array()).ToBe(true);
                });

             it("should handle mixed array types",
                []() -> void
                {
                  zjson::basic_json<> arr;
                  arr.push_back(42);
                  arr.push_back(3.14);
                  arr.push_back("string");
                  arr.push_back(true);
                  arr.push_back(nullptr);

                  zjson::basic_json<> nested_obj;
                  nested_obj["key"] = "value";
                  arr.push_back(nested_obj);

                  zjson::basic_json<> nested_arr;
                  nested_arr.push_back(1);
                  nested_arr.push_back(2);
                  arr.push_back(nested_arr);

                  Expect(arr.size()).ToBe(7);
                  Expect(arr[static_cast<size_t>(0)].get<int>()).ToBe(42);
                  Expect(arr[static_cast<size_t>(1)].get<double>()).ToBe(3.14);
                  Expect(arr[static_cast<size_t>(2)].get<std::string>()).ToBe("string");
                  Expect(arr[static_cast<size_t>(3)].get<bool>()).ToBe(true);
                  Expect(arr[static_cast<size_t>(4)].is_null()).ToBe(true);
                  Expect(arr[static_cast<size_t>(5)].is_object()).ToBe(true);
                  Expect(arr[static_cast<size_t>(6)].is_array()).ToBe(true);

                  Expect(arr[static_cast<size_t>(5)]["key"].get<std::string>()).ToBe("value");
                  Expect(arr[static_cast<size_t>(6)][static_cast<size_t>(0)].get<int>()).ToBe(1);
                });

             it("should handle assignment type changes",
                []() -> void
                {
                  zjson::basic_json<> j;

                  // Start as null
                  Expect(j.is_null()).ToBe(true);

                  // Assign different types
                  j = 42;
                  Expect(j.is_number_integer()).ToBe(true);
                  Expect(j.get<int>()).ToBe(42);

                  j = "string";
                  Expect(j.is_string()).ToBe(true);
                  Expect(j.get<std::string>()).ToBe("string");

                  j = true;
                  Expect(j.is_boolean()).ToBe(true);
                  Expect(j.get<bool>()).ToBe(true);

                  // Assign to create object
                  j["key"] = "value";
                  Expect(j.is_object()).ToBe(true);
                  Expect(j["key"].get<std::string>()).ToBe("value");

                  // Clear and create array
                  j = zjson::basic_json<>(); // Reset to null
                  j.push_back(1);
                  Expect(j.is_array()).ToBe(true);
                  Expect(j[static_cast<size_t>(0)].get<int>()).ToBe(1);
                });
           });

  describe("zjson type compatibility",
           []() -> void
           {
             it("should handle different construction methods",
                []() -> void
                {
                  zjson::basic_json<> j1;
                  j1["name"] = "test";
                  j1["value"] = 42;

                  // Copy construction
                  zjson::basic_json<> j2 = j1;

                  Expect(j1.is_object()).ToBe(true);
                  Expect(j2.is_object()).ToBe(true);
                  Expect(j1["name"].get<std::string>()).ToBe("test");
                  Expect(j2["name"].get<std::string>()).ToBe("test");
                  Expect(j1["value"].get<int>()).ToBe(42);
                  Expect(j2["value"].get<int>()).ToBe(42);
                });
           });

  describe("zjson static cleanup",
           []() -> void
           {
             it("should handle cleanup without errors",
                []() -> void
                {
                  zjson::basic_json<> j;
                  j["test"] = "value";

                  // Cleanup should not throw
                  try
                  {
                    zjson::basic_json<>::cleanup();
                    Expect(true).ToBe(true);
                  }
                  catch (...)
                  {
                    Expect(false).ToBe(true);
                  }
                });
           });
}
