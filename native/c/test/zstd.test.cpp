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

#include "ztest.hpp"
#include "../zstd.hpp"
#include <stdint.h>
#include <utility>
#include <type_traits>
#include <string>

using namespace ztst;
using namespace zstd;

struct MyObject
{
  MyObject(int v)
      : value(v)
  {
  }
  int value;
};

struct AlignedObject
{
  char c;
  double d;
};

namespace
{
struct DestructorTester
{
  static int instance_count;

  DestructorTester()
  {
    ++instance_count;
  }

  DestructorTester(const DestructorTester &other)
  {
    ++instance_count;
  }

  DestructorTester &operator=(const DestructorTester &other)
  {
    return *this;
  }

  ~DestructorTester()
  {
    --instance_count;
  }
};

int DestructorTester::instance_count = 0;
} // namespace

struct TestClass
{
  template <typename T>
  static typename enable_if<std::is_integral<T>::value, int>::type
  test_function(T)
  {
    return 1;
  }

  template <typename T>
  static typename enable_if<!std::is_integral<T>::value, int>::type
  test_function(T)
  {
    return 2;
  }
};

void zstd_tests()
{
  describe("zstd::enable_if", []() -> void
           {
    it("should define type when condition is true", []() -> void {
      // Test that enable_if<true, int> has a type member
      typedef typename enable_if<true, int>::type result_type;
      bool is_same = std::is_same<result_type, int>::value;
      Expect(is_same).ToBe(true);
    });

    it("should define void type when condition is true and no type specified", []() -> void {
      // Test that enable_if<true> has a type member of void
      typedef typename enable_if<true>::type result_type;
      bool is_same = std::is_same<result_type, void>::value;
      Expect(is_same).ToBe(true);
    });

    // Note: We cannot directly test enable_if<false> because it would cause
    // compilation errors. In real usage, enable_if is used in SFINAE contexts
    // where the substitution failure is silently ignored.

    it("should work in SFINAE context for function overloading", []() -> void {
      int result1 = TestClass::test_function(42);     // integral type
      int result2 = TestClass::test_function(3.14);   // non-integral type
      
      Expect(result1).ToBe(1);
      Expect(result2).ToBe(2);
    });

    it("should work with custom boolean conditions", []() -> void {
      // Test with a compile-time constant expression
      const bool condition = (sizeof(int) == 4);
      typedef typename enable_if<condition, double>::type result_type;
      bool is_same = std::is_same<result_type, double>::value;
      Expect(is_same).ToBe(true);
    }); });

  describe("zstd::optional", []() -> void
           {
        it("should be empty on default construction", []() -> void {
            optional<int> opt;
            Expect(opt.has_value()).ToBe(false);
            Expect(static_cast<bool>(opt)).ToBe(false);
        });

        it("should hold a value when constructed with one", []() -> void {
            optional<int> opt(42);
            Expect(opt.has_value()).ToBe(true);
            Expect(*opt).ToBe(42);
            Expect(opt.value()).ToBe(42);
        });

        it("should handle copy construction", []() -> void {
            optional<int> opt1(42);
            optional<int> opt2(opt1);
            Expect(opt2.has_value()).ToBe(true);
            Expect(*opt2).ToBe(42);

            optional<int> opt3;
            optional<int> opt4(opt3);
            Expect(opt4.has_value()).ToBe(false);
        });

        it("should handle assignment", []() -> void {
            optional<int> opt1(42);
            optional<int> opt2;
            opt2 = opt1;
            Expect(opt2.has_value()).ToBe(true);
            Expect(*opt2).ToBe(42);

            optional<int> opt3;
            opt1 = opt3;
            Expect(opt1.has_value()).ToBe(false);

            optional<int> opt4(10);
            optional<int> opt5(20);
            opt4 = opt5;
            Expect(*opt4).ToBe(20);
        });

        it("should throw when accessing value of empty optional", []() -> void {
            optional<int> opt;
            bool thrown = false;
            try {
                opt.value();
            } catch (const std::logic_error&) {
                thrown = true;
            }
            Expect(thrown).ToBe(true);

            thrown = false;
            try {
                *opt;
            } catch (const std::logic_error&) {
                thrown = true;
            }
            Expect(thrown).ToBe(true);
        });
        
        it("should allow modification through operator*", []() -> void {
            optional<int> opt(10);
            *opt = 20;
            Expect(*opt).ToBe(20);
        });
        
        it("should allow modification through value()", []() -> void {
            optional<int> opt(10);
            opt.value() = 20;
            Expect(opt.value()).ToBe(20);
        });

        it("should allow access through operator->", []() -> void {
            optional<MyObject> opt(MyObject(55));
            Expect(opt->value).ToBe(55);
        });

        it("should reset to an empty state", []() -> void {
            optional<int> opt(42);
            opt.reset();
            Expect(opt.has_value()).ToBe(false);
        });

        it("should swap with another optional", []() -> void {
            optional<int> opt1(10);
            optional<int> opt2(20);
            opt1.swap(opt2);
            Expect(*opt1).ToBe(20);
            Expect(*opt2).ToBe(10);

            optional<int> opt3(30);
            optional<int> opt4;
            opt3.swap(opt4);
            Expect(opt3.has_value()).ToBe(false);
            Expect(opt4.has_value()).ToBe(true);
            Expect(*opt4).ToBe(30);

            optional<int> opt5;
            optional<int> opt6(40);
            opt5.swap(opt6);
            Expect(opt5.has_value()).ToBe(true);
            Expect(*opt5).ToBe(40);
            Expect(opt6.has_value()).ToBe(false);

            optional<int> opt7;
            optional<int> opt8;
            opt7.swap(opt8);
            Expect(opt7.has_value()).ToBe(false);
            Expect(opt8.has_value()).ToBe(false);
        });

        it("should ensure proper memory alignment", []() -> void {
            optional<AlignedObject> opt;
            opt = AlignedObject();

            const AlignedObject* ptr = &(*opt);
            uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
            size_t alignment = __alignof__(AlignedObject);

            Expect(address % alignment).ToBe(0);
        }); });

  describe("zstd::unique_ptr", []() -> void
           {
    it("should be empty on default construction", []() -> void {
      unique_ptr<int> ptr;
      Expect(ptr.get()).ToBe(nullptr);
      Expect(static_cast<bool>(ptr)).ToBe(false);
    });

    it("should hold a value when constructed with one", []() -> void {
      int *raw_ptr = new int(42);
      unique_ptr<int> ptr(raw_ptr);
      Expect(ptr.get()).ToBe(raw_ptr);
      Expect(*ptr).ToBe(42);
    });

    it("should delete the object on destruction", []() -> void {
      DestructorTester::instance_count = 0;
      {
        unique_ptr<DestructorTester> ptr(new DestructorTester());
        Expect(DestructorTester::instance_count).ToBe(1);
      }
      Expect(DestructorTester::instance_count).ToBe(0);
    });

    it("should release ownership", []() -> void {
      DestructorTester::instance_count = 0;
      DestructorTester *raw_ptr = new DestructorTester();
      unique_ptr<DestructorTester> ptr(raw_ptr);
      DestructorTester *released_ptr = ptr.release();
      Expect(ptr.get()).ToBe(nullptr);
      Expect(released_ptr).ToBe(raw_ptr);
      Expect(DestructorTester::instance_count).ToBe(1);
      delete released_ptr;
      Expect(DestructorTester::instance_count).ToBe(0);
    });

    it("should reset the pointer", []() -> void {
      DestructorTester::instance_count = 0;
      unique_ptr<DestructorTester> ptr(new DestructorTester());
      DestructorTester *raw_ptr2 = new DestructorTester();
      
      Expect(DestructorTester::instance_count).ToBe(2);
      ptr.reset(raw_ptr2);
      Expect(DestructorTester::instance_count).ToBe(1); // First object should be destroyed
      Expect(ptr.get()).ToBe(raw_ptr2);

      ptr.reset();
      Expect(DestructorTester::instance_count).ToBe(0); // Second object should be destroyed
      Expect(ptr.get()).ToBe(nullptr);
    });

    it("should handle move construction", []() -> void {
      int *raw_ptr = new int(123);
      unique_ptr<int> ptr1(raw_ptr);
      unique_ptr<int> ptr2(std::move(ptr1));

      Expect(ptr1.get()).ToBe(nullptr);
      Expect(ptr2.get()).ToBe(raw_ptr);
      Expect(*ptr2).ToBe(123);
    });

    it("should handle move assignment", []() -> void {
      DestructorTester::instance_count = 0;
      unique_ptr<DestructorTester> ptr1(new DestructorTester());

      int *raw_ptr2 = new int(20);
      unique_ptr<int> ptr2(raw_ptr2);

      // This is a bit of a complex scenario to test destruction on assignment.
      // We create a new scope to control the lifetime of ptr1's replacement.
      {
        int *raw_ptr1 = new int(10);
        unique_ptr<int> ptr_new(raw_ptr1);
        ptr2 = std::move(ptr_new);
      }

      Expect(ptr2.get()).Not().ToBe(raw_ptr2); // Should have been replaced
      Expect(*ptr2).ToBe(10);
    });

    it("should swap with another unique_ptr", []() -> void {
      int *raw_ptr1 = new int(1);
      unique_ptr<int> ptr1(raw_ptr1);

      int *raw_ptr2 = new int(2);
      unique_ptr<int> ptr2(raw_ptr2);

      ptr1.swap(ptr2);

      Expect(ptr1.get()).ToBe(raw_ptr2);
      Expect(ptr2.get()).ToBe(raw_ptr1);
    });

    it("should allow access through operator->", []() -> void {
      struct TestObject
      {
        int value;
      };
      unique_ptr<TestObject> ptr(new TestObject{55});
      Expect(ptr->value).ToBe(55);
    }); });

  describe("zstd::make_unique", []() -> void
           {
    it("should create a unique_ptr with a default-constructed object", []() -> void {
      struct Simple
      {
        Simple() : value(123)
        {
        }
        int value;
      };
      unique_ptr<Simple> ptr = make_unique<Simple>();
      Expect(ptr->value).ToBe(123);
    });

    it("should create a unique_ptr with an object constructed with arguments", []() -> void {
      struct Complex
      {
        Complex(int a, double b) : val1(a), val2(b)
        {
        }
        int val1;
        double val2;
      };
      unique_ptr<Complex> ptr = make_unique<Complex>(10, 20.5);
      Expect(ptr->val1).ToBe(10);
      Expect(ptr->val2).ToBe(20.5);
    });

    it("should create a unique_ptr that correctly manages lifetime", []() -> void {
      DestructorTester::instance_count = 0;
      {
        unique_ptr<DestructorTester> ptr = make_unique<DestructorTester>();
        Expect(DestructorTester::instance_count).ToBe(1);
      }
      Expect(DestructorTester::instance_count).ToBe(0);
    }); });

  describe("zstd::unexpected", []() -> void
           {
    it("should store an error value", []() -> void {
      unexpected<std::string> unexp("test error");
      Expect(unexp.error()).ToBe(std::string("test error"));
    });

    it("should handle move construction", []() -> void {
      std::string error_msg = "moved error";
      unexpected<std::string> unexp(std::move(error_msg));
      Expect(unexp.error()).ToBe(std::string("moved error"));
    }); });

  describe("zstd::make_unexpected", []() -> void
           {
    it("should create unexpected from value", []() -> void {
      auto unexp = make_unexpected(42);
      Expect(unexp.error()).ToBe(42);
    });

    it("should create unexpected from moved value", []() -> void {
      std::string error_msg = "test error";
      auto unexp = make_unexpected(std::move(error_msg));
      Expect(unexp.error()).ToBe(std::string("test error"));
    }); });

  describe("zstd::expected", []() -> void
           {
    it("should be successful on default construction", []() -> void {
      expected<int, std::string> exp;
      Expect(exp.has_value()).ToBe(true);
      Expect(static_cast<bool>(exp)).ToBe(true);
      Expect(exp.value()).ToBe(0); // default constructed int
    });

    it("should hold a value when constructed with one", []() -> void {
      expected<int, std::string> exp(42);
      Expect(exp.has_value()).ToBe(true);
      Expect(exp.value()).ToBe(42);
      Expect(*exp).ToBe(42);
    });

    it("should hold an error when constructed with unexpected", []() -> void {
      expected<int, std::string> exp = make_unexpected(std::string("error"));
      Expect(exp.has_value()).ToBe(false);
      Expect(static_cast<bool>(exp)).ToBe(false);
      Expect(exp.error()).ToBe(std::string("error"));
    });

    it("should handle copy construction of successful expected", []() -> void {
      expected<int, std::string> exp1(42);
      expected<int, std::string> exp2(exp1);
      Expect(exp2.has_value()).ToBe(true);
      Expect(exp2.value()).ToBe(42);
    });

    it("should handle copy construction of error expected", []() -> void {
      expected<int, std::string> exp1 = make_unexpected(std::string("error"));
      expected<int, std::string> exp2(exp1);
      Expect(exp2.has_value()).ToBe(false);
      Expect(exp2.error()).ToBe(std::string("error"));
    });

    it("should handle move construction of successful expected", []() -> void {
      expected<int, std::string> exp1(42);
      expected<int, std::string> exp2(std::move(exp1));
      Expect(exp2.has_value()).ToBe(true);
      Expect(exp2.value()).ToBe(42);
    });

    it("should handle move construction of error expected", []() -> void {
      expected<int, std::string> exp1 = make_unexpected(std::string("error"));
      expected<int, std::string> exp2(std::move(exp1));
      Expect(exp2.has_value()).ToBe(false);
      Expect(exp2.error()).ToBe(std::string("error"));
    });

    it("should handle copy assignment from value to value", []() -> void {
      expected<int, std::string> exp1(10);
      expected<int, std::string> exp2(20);
      exp1 = exp2;
      Expect(exp1.has_value()).ToBe(true);
      Expect(exp1.value()).ToBe(20);
    });

    it("should handle copy assignment from error to error", []() -> void {
      expected<int, std::string> exp1 = make_unexpected(std::string("error1"));
      expected<int, std::string> exp2 = make_unexpected(std::string("error2"));
      exp1 = exp2;
      Expect(exp1.has_value()).ToBe(false);
      Expect(exp1.error()).ToBe(std::string("error2"));
    });

    it("should handle copy assignment from value to error", []() -> void {
      expected<int, std::string> exp1(42);
      expected<int, std::string> exp2 = make_unexpected(std::string("error"));
      exp1 = exp2;
      Expect(exp1.has_value()).ToBe(false);
      Expect(exp1.error()).ToBe(std::string("error"));
    });

    it("should handle copy assignment from error to value", []() -> void {
      expected<int, std::string> exp1 = make_unexpected(std::string("error"));
      expected<int, std::string> exp2(42);
      exp1 = exp2;
      Expect(exp1.has_value()).ToBe(true);
      Expect(exp1.value()).ToBe(42);
    });

    it("should handle value assignment", []() -> void {
      expected<int, std::string> exp = make_unexpected(std::string("error"));
      exp = 42;
      Expect(exp.has_value()).ToBe(true);
      Expect(exp.value()).ToBe(42);
    });

    it("should handle unexpected assignment", []() -> void {
      expected<int, std::string> exp(42);
      exp = make_unexpected(std::string("error"));
      Expect(exp.has_value()).ToBe(false);
      Expect(exp.error()).ToBe(std::string("error"));
    });

    it("should throw when accessing value of error expected", []() -> void {
      expected<int, std::string> exp = make_unexpected(std::string("error"));
      bool thrown = false;
      try {
        exp.value();
      } catch (const std::logic_error&) {
        thrown = true;
      }
      Expect(thrown).ToBe(true);

      thrown = false;
      try {
        *exp;
      } catch (const std::logic_error&) {
        thrown = true;
      }
      Expect(thrown).ToBe(true);
    });

    it("should throw when accessing error of successful expected", []() -> void {
      expected<int, std::string> exp(42);
      bool thrown = false;
      try {
        exp.error();
      } catch (const std::logic_error&) {
        thrown = true;
      }
      Expect(thrown).ToBe(true);
    });

    it("should allow modification through operator*", []() -> void {
      expected<int, std::string> exp(10);
      *exp = 20;
      Expect(*exp).ToBe(20);
    });

    it("should allow modification through value()", []() -> void {
      expected<int, std::string> exp(10);
      exp.value() = 20;
      Expect(exp.value()).ToBe(20);
    });

    it("should allow access through operator->", []() -> void {
      expected<MyObject, std::string> exp(MyObject(55));
      Expect(exp->value).ToBe(55);
    });

    it("should provide value_or functionality", []() -> void {
      expected<int, std::string> success(42);
      expected<int, std::string> failure = make_unexpected(std::string("error"));
      
      Expect(success.value_or(0)).ToBe(42);
      Expect(failure.value_or(99)).ToBe(99);
    });

    it("should swap successful expectations", []() -> void {
      expected<int, std::string> exp1(10);
      expected<int, std::string> exp2(20);
      exp1.swap(exp2);
      Expect(exp1.value()).ToBe(20);
      Expect(exp2.value()).ToBe(10);
    });

    it("should swap error expectations", []() -> void {
      expected<int, std::string> exp1 = make_unexpected(std::string("error1"));
      expected<int, std::string> exp2 = make_unexpected(std::string("error2"));
      exp1.swap(exp2);
      Expect(exp1.error()).ToBe(std::string("error2"));
      Expect(exp2.error()).ToBe(std::string("error1"));
    });

    it("should swap success with error", []() -> void {
      expected<int, std::string> exp1(42);
      expected<int, std::string> exp2 = make_unexpected(std::string("error"));
      exp1.swap(exp2);
      Expect(exp1.has_value()).ToBe(false);
      Expect(exp1.error()).ToBe(std::string("error"));
      Expect(exp2.has_value()).ToBe(true);
      Expect(exp2.value()).ToBe(42);
    });

    it("should ensure proper memory alignment", []() -> void {
      expected<AlignedObject, std::string> exp;
      exp = AlignedObject();

      const AlignedObject* ptr = &(*exp);
      uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
      size_t alignment = __alignof__(AlignedObject);

      Expect(address % alignment).ToBe(0);
    });

    it("should properly destruct contained objects", []() -> void {
      DestructorTester::instance_count = 0; // Reset counter
      
      {
        expected<DestructorTester, std::string> exp{DestructorTester()};
        Expect(DestructorTester::instance_count).ToBe(1); // One instance should remain in expected
      }
      Expect(DestructorTester::instance_count).ToBe(0); // Should be destroyed when expected goes out of scope

      DestructorTester::instance_count = 0; // Reset counter
      {
        expected<int, DestructorTester> exp = make_unexpected(DestructorTester());
        Expect(DestructorTester::instance_count).ToBe(1); // One instance should remain in expected
      }
      Expect(DestructorTester::instance_count).ToBe(0); // Should be destroyed when expected goes out of scope
    }); });

  describe("zstd::expected comparisons", []() -> void
           {
    it("should compare two successful expectations", []() -> void {
      expected<int, std::string> exp1(42);
      expected<int, std::string> exp2(42);
      expected<int, std::string> exp3(43);
      
      Expect(exp1 == exp2).ToBe(true);
      Expect(exp1 != exp3).ToBe(true);
    });

    it("should compare two error expectations", []() -> void {
      expected<int, std::string> exp1 = make_unexpected(std::string("error"));
      expected<int, std::string> exp2 = make_unexpected(std::string("error"));
      expected<int, std::string> exp3 = make_unexpected(std::string("different"));
      
      Expect(exp1 == exp2).ToBe(true);
      Expect(exp1 != exp3).ToBe(true);
    });

    it("should compare success and error expectations", []() -> void {
      expected<int, std::string> success(42);
      expected<int, std::string> error = make_unexpected(std::string("error"));
      
      Expect(success != error).ToBe(true);
      Expect(error != success).ToBe(true);
    });

    it("should compare expected with value", []() -> void {
      expected<int, std::string> exp(42);
      expected<int, std::string> error = make_unexpected(std::string("error"));
      
      Expect(exp == 42).ToBe(true);
      Expect(42 == exp).ToBe(true);
      Expect(exp != 43).ToBe(true);
      Expect(43 != exp).ToBe(true);
      Expect(error != 42).ToBe(true);
      Expect(42 != error).ToBe(true);
    });

    it("should compare expected with unexpected", []() -> void {
      expected<int, std::string> exp(42);
      expected<int, std::string> error = make_unexpected(std::string("error"));
      auto unexp = make_unexpected(std::string("error"));
      auto different_unexp = make_unexpected(std::string("different"));
      
      Expect(error == unexp).ToBe(true);
      Expect(unexp == error).ToBe(true);
      Expect(error != different_unexp).ToBe(true);
      Expect(different_unexp != error).ToBe(true);
      Expect(exp != unexp).ToBe(true);
      Expect(unexp != exp).ToBe(true);
    }); });

  describe("zstd::variant", []() -> void
           {
    it("should default construct to first type", []() -> void {
      variant<int, std::string> var;
      Expect(var.index()).ToBe(0);
      Expect(var.get<int>()).ToBe(0);
    });

    it("should hold a value when constructed with one", []() -> void {
      variant<int, std::string> var(42);
      Expect(var.index()).ToBe(0);
      Expect(var.get<int>()).ToBe(42);
    });

    it("should hold a value when constructed with another", []() -> void {
      variant<int, std::string> var(std::string("hello"));
      Expect(var.index()).ToBe(1);
      Expect(var.get<std::string>()).ToBe(std::string("hello"));
    });

    it("should handle copy construction", []() -> void {
      variant<int, std::string> var1(42);
      variant<int, std::string> var2(var1);
      Expect(var2.index()).ToBe(0);
      Expect(var2.get<int>()).ToBe(42);

      variant<int, std::string> var3(std::string("world"));
      variant<int, std::string> var4(var3);
      Expect(var4.index()).ToBe(1);
      Expect(var4.get<std::string>()).ToBe(std::string("world"));
    });

    it("should handle assignment", []() -> void {
      variant<int, std::string> var1(42);
      variant<int, std::string> var2;
      var2 = var1;
      Expect(var2.index()).ToBe(0);
      Expect(var2.get<int>()).ToBe(42);

      variant<int, std::string> var3(std::string("test"));
      var1 = var3;
      Expect(var1.index()).ToBe(1);
      Expect(var1.get<std::string>()).ToBe(std::string("test"));
    });

    it("should throw when accessing with incorrect type", []() -> void {
      variant<int, std::string> var(42);
      bool thrown = false;
      try {
        var.get<std::string>();
      } catch (const zstd::bad_variant_access&) {
        thrown = true;
      }
      Expect(thrown).ToBe(true);
    });
    
    it("should throw when accessing with incorrect index", []() -> void {
      variant<int, std::string> var(42);
      bool thrown = false;
      try {
        var.get<1>();
      } catch (const zstd::bad_variant_access&) {
        thrown = true;
      }
      Expect(thrown).ToBe(true);
    });

    it("should allow modification through get()", []() -> void {
      variant<int, std::string> var(10);
      var.get<int>() = 20;
      Expect(var.get<int>()).ToBe(20);
    });

    it("should allow member access through get()", []() -> void {
      variant<MyObject, std::string> var(MyObject(55));
      Expect(var.get<MyObject>().value).ToBe(55);
    });

    it("should ensure proper memory alignment", []() -> void {
      variant<AlignedObject, std::string> var;
      var = AlignedObject();

      const AlignedObject* ptr = &var.get<AlignedObject>();
      uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
      size_t alignment = __alignof__(AlignedObject);

      Expect(address % alignment).ToBe(0);
    });

    it("should properly destruct contained objects", []() -> void {
      DestructorTester::instance_count = 0; // Reset counter
      
      {
        variant<DestructorTester, std::string> var((DestructorTester()));
        Expect(DestructorTester::instance_count).ToBe(1); // One instance should remain in variant
      }
      Expect(DestructorTester::instance_count).ToBe(0); // Should be destroyed when variant goes out of scope

      DestructorTester::instance_count = 0; // Reset counter
      {
        variant<int, DestructorTester> var((DestructorTester()));
        Expect(DestructorTester::instance_count).ToBe(1); // One instance should remain in variant
        var = 42; // Assigning a new type should destruct the old one
        Expect(DestructorTester::instance_count).ToBe(0);
      }
      Expect(DestructorTester::instance_count).ToBe(0);
    }); });
}
