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

struct ForwardTestHelper
{
  static bool is_lvalue_ref;
  static bool is_rvalue_ref;

  static void reset()
  {
    is_lvalue_ref = false;
    is_rvalue_ref = false;
  }

  static void test_func(std::string &)
  {
    is_lvalue_ref = true;
  }

  static void test_func(std::string &&)
  {
    is_rvalue_ref = true;
  }
};

bool ForwardTestHelper::is_lvalue_ref = false;
bool ForwardTestHelper::is_rvalue_ref = false;
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
  describe("zstd::remove_reference", []() -> void
           {
    it("should remove reference from regular type", []() -> void {
      bool is_same = std::is_same<typename remove_reference<int>::type, int>::value;
      Expect(is_same).ToBe(true);
    });

    it("should remove l-value reference", []() -> void {
      bool is_same = std::is_same<typename remove_reference<int &>::type, int>::value;
      Expect(is_same).ToBe(true);
    });

    it("should remove r-value reference", []() -> void {
      bool is_same = std::is_same<typename remove_reference<int &&>::type, int>::value;
      Expect(is_same).ToBe(true);
    });

    it("should remove reference from const type", []() -> void {
      bool is_same = std::is_same<typename remove_reference<const int &>::type, const int>::value;
      Expect(is_same).ToBe(true);
    });

    it("should remove reference from pointer type", []() -> void {
      bool is_same = std::is_same<typename remove_reference<int *&>::type, int *>::value;
      Expect(is_same).ToBe(true);
    });

    it("should handle complex types", []() -> void {
      bool is_same = std::is_same<typename remove_reference<std::string &&>::type, std::string>::value;
      Expect(is_same).ToBe(true);
    }); });

  describe("zstd::is_same", []() -> void
           {
    it("should return true for identical types", []() -> void {
      bool result = is_same<int, int>::value;
      Expect(result).ToBe(true);
    });

    it("should return false for different types", []() -> void {
      bool result = is_same<int, double>::value;
      Expect(result).ToBe(false);
    });

    it("should distinguish between reference and non-reference types", []() -> void {
      bool result = is_same<int &, int>::value;
      Expect(result).ToBe(false);
    });

    it("should distinguish between const and non-const types", []() -> void {
      bool result = is_same<const int, int>::value;
      Expect(result).ToBe(false);
    });

    it("should return true for identical const types", []() -> void {
      bool result = is_same<const int, const int>::value;
      Expect(result).ToBe(true);
    });

    it("should return true for identical pointer types", []() -> void {
      bool result = is_same<int *, int *>::value;
      Expect(result).ToBe(true);
    });

    it("should inherit from std::true_type when types are same", []() -> void {
      bool result = std::is_base_of<std::true_type, is_same<int, int>>::value;
      Expect(result).ToBe(true);
    });

    it("should inherit from std::false_type when types are different", []() -> void {
      bool result = std::is_base_of<std::false_type, is_same<int, double>>::value;
      Expect(result).ToBe(true);
    }); });

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

  describe("zstd::forward", []() -> void
           {
    it("should forward an l-value as an l-value reference", []() -> void {
      ForwardTestHelper::reset();
      std::string s = "test";
      ForwardTestHelper::test_func(zstd::forward<std::string &>(s));
      Expect(ForwardTestHelper::is_lvalue_ref).ToBe(true);
      Expect(ForwardTestHelper::is_rvalue_ref).ToBe(false);
    });

    it("should forward an l-value as an r-value reference", []() -> void {
      ForwardTestHelper::reset();
      std::string s = "test";
      ForwardTestHelper::test_func(zstd::forward<std::string>(s));
      Expect(ForwardTestHelper::is_lvalue_ref).ToBe(false);
      Expect(ForwardTestHelper::is_rvalue_ref).ToBe(true);
    });

    it("should forward an r-value as an r-value reference", []() -> void {
      ForwardTestHelper::reset();
      ForwardTestHelper::test_func(
          zstd::forward<std::string>(std::string("test")));
      Expect(ForwardTestHelper::is_lvalue_ref).ToBe(false);
      Expect(ForwardTestHelper::is_rvalue_ref).ToBe(true);
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

  describe("zstd::string_view", []() -> void
           {
    it("should default construct to empty string_view", []() -> void {
      string_view sv;
      Expect(sv.data()).ToBe(nullptr);
      Expect(sv.size()).ToBe(0);
      Expect(sv.empty()).ToBe(true);
    });

    it("should construct from C-string", []() -> void {
      const char* str = "hello";
      string_view sv(str);
      Expect(sv.data()).ToBe(str);
      Expect(sv.size()).ToBe(5);
      Expect(sv.empty()).ToBe(false);
    });

    it("should construct from data and size", []() -> void {
      const char* data = "hello world";
      string_view sv(data, 5);
      Expect(sv.data()).ToBe(data);
      Expect(sv.size()).ToBe(5);
      Expect(sv.length()).ToBe(5);
    });

    it("should construct from std::string", []() -> void {
      std::string str = "test string";
      string_view sv(str);
      Expect(sv.data()).ToBe(str.c_str());
      Expect(sv.size()).ToBe(11);
    });

    it("should handle copy construction", []() -> void {
      string_view sv1("original");
      string_view sv2(sv1);
      Expect(sv2.data()).ToBe(sv1.data());
      Expect(sv2.size()).ToBe(sv1.size());
    });

    it("should handle assignment", []() -> void {
      string_view sv1("hello");
      string_view sv2;
      sv2 = sv1;
      Expect(sv2.data()).ToBe(sv1.data());
      Expect(sv2.size()).ToBe(sv1.size());
    });

    it("should provide element access via operator[]", []() -> void {
      string_view sv("hello");
      Expect(sv[0] == 'h').ToBe(true);
      Expect(sv[4] == 'o').ToBe(true);
    });

    it("should provide bounds-checked access via at()", []() -> void {
      string_view sv("test");
      Expect(sv.at(0) == 't').ToBe(true);
      Expect(sv.at(3) == 't').ToBe(true);
      
      bool thrown = false;
      try {
        sv.at(4);
      } catch (const std::out_of_range&) {
        thrown = true;
      }
      Expect(thrown).ToBe(true);
    });

    it("should provide front() and back() access", []() -> void {
      string_view sv("hello");
      Expect(sv.front() == 'h').ToBe(true);
      Expect(sv.back() == 'o').ToBe(true);
    });

    it("should provide iterators", []() -> void {
      string_view sv("abc");
      auto it = sv.begin();
      Expect(*it == 'a').ToBe(true);
      ++it;
      Expect(*it == 'b').ToBe(true);
      ++it;
      Expect(*it == 'c').ToBe(true);
      ++it;
      Expect(it == sv.end()).ToBe(true);
    });

    it("should provide const iterators", []() -> void {
      string_view sv("test");
      auto cit = sv.cbegin();
      Expect(*cit == 't').ToBe(true);
      Expect(sv.cend() - sv.cbegin()).ToBe(4);
    });

    it("should report max_size", []() -> void {
      string_view sv;
      Expect(sv.max_size()).ToBe(static_cast<string_view::size_type>(-1));
    });

    it("should remove prefix correctly", []() -> void {
      string_view sv("hello world");
      sv.remove_prefix(6);
      Expect(sv.size()).ToBe(5);
      Expect(sv[0] == 'w').ToBe(true);
      Expect(std::string(sv.data(), sv.size())).ToBe(std::string("world"));
    });

    it("should remove suffix correctly", []() -> void {
      string_view sv("hello world");
      sv.remove_suffix(6);
      Expect(sv.size()).ToBe(5);
      Expect(std::string(sv.data(), sv.size())).ToBe(std::string("hello"));
    });

    it("should swap with another string_view", []() -> void {
      string_view sv1("first");
      string_view sv2("second");
      const char* data1 = sv1.data();
      const char* data2 = sv2.data();
      sv1.swap(sv2);
      Expect(sv1.data()).ToBe(data2);
      Expect(sv2.data()).ToBe(data1);
      Expect(sv1.size()).ToBe(6);
      Expect(sv2.size()).ToBe(5);
    });

    it("should copy data to buffer", []() -> void {
      string_view sv("hello");
      char buffer[10];
      string_view::size_type copied = sv.copy(buffer, 3, 1);
      Expect(copied).ToBe(3);
      Expect(std::string(buffer, copied)).ToBe(std::string("ell"));

      bool thrown = false;
      try {
        sv.copy(buffer, 3, 10);
      } catch (const std::out_of_range&) {
        thrown = true;
      }
      Expect(thrown).ToBe(true);
    });

    it("should create substring", []() -> void {
      string_view sv("hello world");
      string_view sub = sv.substr(6, 5);
      Expect(sub.size()).ToBe(5);
      Expect(std::string(sub.data(), sub.size())).ToBe(std::string("world"));

      string_view sub2 = sv.substr(6);
      Expect(sub2.size()).ToBe(5);

      bool thrown = false;
      try {
        sv.substr(20);
      } catch (const std::out_of_range&) {
        thrown = true;
      }
      Expect(thrown).ToBe(true);
    });

    it("should compare with other string_view", []() -> void {
      string_view sv1("abc");
      string_view sv2("abc");
      string_view sv3("def");
      string_view sv4("ab");

      Expect(sv1.compare(sv2)).ToBe(0);
      Expect(sv1.compare(sv3) < 0).ToBe(true);
      Expect(sv3.compare(sv1) > 0).ToBe(true);
      Expect(sv1.compare(sv4) > 0).ToBe(true);
    });

    it("should compare with substring", []() -> void {
      string_view sv("hello world");
      string_view other("world");
      Expect(sv.compare(6, 5, other)).ToBe(0);
      Expect(sv.compare(0, 5, other) < 0).ToBe(true);
    });

    it("should compare with C-string", []() -> void {
      string_view sv("hello");
      Expect(sv.compare("hello")).ToBe(0);
      Expect(sv.compare("world") < 0).ToBe(true);
      Expect(sv.compare("abc") > 0).ToBe(true);
    });

    it("should test starts_with", []() -> void {
      string_view sv("hello world");
      Expect(sv.starts_with("hello")).ToBe(true);
      Expect(sv.starts_with("world")).ToBe(false);
      Expect(sv.starts_with('h')).ToBe(true);
      Expect(sv.starts_with('w')).ToBe(false);
      Expect(sv.starts_with(string_view("hello"))).ToBe(true);
    });

    it("should test ends_with", []() -> void {
      string_view sv("hello world");
      Expect(sv.ends_with("world")).ToBe(true);
      Expect(sv.ends_with("hello")).ToBe(false);
      Expect(sv.ends_with('d')).ToBe(true);
      Expect(sv.ends_with('h')).ToBe(false);
      Expect(sv.ends_with(string_view("world"))).ToBe(true);
    });

    it("should find substring", []() -> void {
      string_view sv("hello world hello");
      Expect(sv.find("hello")).ToBe(0);
      Expect(sv.find("world")).ToBe(6);
      Expect(sv.find("hello", 1)).ToBe(12);
      Expect(sv.find("xyz")).ToBe(string_view::npos);
      Expect(sv.find("")).ToBe(0);
    });

    it("should find character", []() -> void {
      string_view sv("hello");
      Expect(sv.find('e')).ToBe(1);
      Expect(sv.find('l')).ToBe(2);
      Expect(sv.find('x')).ToBe(string_view::npos);
      Expect(sv.find('l', 3)).ToBe(3);
    });

    it("should reverse find substring", []() -> void {
      string_view sv("hello world hello");
      Expect(sv.rfind("hello")).ToBe(12);
      Expect(sv.rfind("world")).ToBe(6);
      Expect(sv.rfind("xyz")).ToBe(string_view::npos);
      Expect(sv.rfind("hello", 10)).ToBe(0);
    });

    it("should reverse find character", []() -> void {
      string_view sv("hello");
      Expect(sv.rfind('l')).ToBe(3);
      Expect(sv.rfind('e')).ToBe(1);
      Expect(sv.rfind('x')).ToBe(string_view::npos);
      Expect(sv.rfind('l', 2)).ToBe(2);
    });

    it("should find first of characters", []() -> void {
      string_view sv("hello world");
      Expect(sv.find_first_of("aeiou")).ToBe(1); // 'e'
      Expect(sv.find_first_of('o')).ToBe(4);
      Expect(sv.find_first_of("xyz")).ToBe(string_view::npos);
      Expect(sv.find_first_of("aeiou", 2)).ToBe(4); // 'o'
    });

    it("should find last of characters", []() -> void {
      string_view sv("hello world");
      Expect(sv.find_last_of("aeiou")).ToBe(7); // 'o' in world
      Expect(sv.find_last_of('l')).ToBe(9);
      Expect(sv.find_last_of("xyz")).ToBe(string_view::npos);
      Expect(sv.find_last_of("aeiou", 5)).ToBe(4); // 'o' in hello
    });

    it("should find first not of characters", []() -> void {
      string_view sv("hello world");
      Expect(sv.find_first_not_of("hel")).ToBe(4); // 'o'
      Expect(sv.find_first_not_of('h')).ToBe(1);
      Expect(sv.find_first_not_of("helo wrd")).ToBe(string_view::npos);
      Expect(sv.find_first_not_of("he", 2)).ToBe(2); // 'l'
    });

    it("should find last not of characters", []() -> void {
      string_view sv("hello world");
      Expect(sv.find_last_not_of("drl")).ToBe(7); // 'o' in world
      Expect(sv.find_last_not_of('d')).ToBe(9);
      Expect(sv.find_last_not_of("helo wrd")).ToBe(string_view::npos);
      Expect(sv.find_last_not_of("ld", 7)).ToBe(7); // 'o'
    });

    it("should handle empty string operations", []() -> void {
      string_view empty;
      Expect(empty.starts_with("")).ToBe(true);
      Expect(empty.ends_with("")).ToBe(true);
      Expect(empty.find("")).ToBe(0);
      Expect(empty.find("x")).ToBe(string_view::npos);
      Expect(empty.substr().size()).ToBe(0);
    }); });

  describe("zstd::string_view comparisons", []() -> void
           {
    it("should compare equal string_views", []() -> void {
      string_view sv1("hello");
      string_view sv2("hello");
      string_view sv3("world");
      
      Expect(sv1 == sv2).ToBe(true);
      Expect(sv1 != sv3).ToBe(true);
      Expect(sv1 == sv3).ToBe(false);
    });

    it("should compare string_views with ordering", []() -> void {
      string_view sv1("abc");
      string_view sv2("def");
      string_view sv3("ab");
      
      Expect(sv1 < sv2).ToBe(true);
      Expect(sv1 <= sv2).ToBe(true);
      Expect(sv2 > sv1).ToBe(true);
      Expect(sv2 >= sv1).ToBe(true);
      Expect(sv1 > sv3).ToBe(true);
      Expect(sv1 >= sv3).ToBe(true);
    }); });

  describe("zstd::monostate", []() -> void
           {
    it("should default construct", []() -> void {
      monostate m;
      (void)m; // Suppress unused variable warning
    });

    it("should copy construct", []() -> void {
      monostate m1;
      monostate m2(m1);
      (void)m2; // Suppress unused variable warning
    });

    it("should handle assignment", []() -> void {
      monostate m1;
      monostate m2;
      m2 = m1;
      (void)m1;
      (void)m2; // Suppress unused variable warnings
    });

    it("should compare equal with any other monostate", []() -> void {
      monostate m1;
      monostate m2;
      Expect(m1 == m2).ToBe(true);
      Expect(m1 != m2).ToBe(false);
    });

    it("should have consistent ordering relationships", []() -> void {
      monostate m1;
      monostate m2;
      Expect(m1 < m2).ToBe(false);
      Expect(m1 <= m2).ToBe(true);
      Expect(m1 > m2).ToBe(false);
      Expect(m1 >= m2).ToBe(true);
    });

    it("should work as variant alternative", []() -> void {
      variant<monostate, int, std::string> var;
      Expect(var.index()).ToBe(0);
      // Should construct monostate by default
      var.get<monostate>(); // Should not throw
    });

    it("should allow switching between monostate and other types", []() -> void {
      variant<monostate, int> var;
      Expect(var.index()).ToBe(0);

      // Switch to int
      var = 42;
      Expect(var.index()).ToBe(1);
      Expect(var.get<int>()).ToBe(42);

      // Switch back to monostate
      var = monostate();
      Expect(var.index()).ToBe(0);
      var.get<monostate>(); // Should not throw
    });

    it("should have minimal size", []() -> void {
      // monostate should have size of at least 1 (empty class requirement)
      Expect(sizeof(monostate) >= 1).ToBe(true);
    }); });

  describe("zstd::bad_variant_access", []() -> void
           {
    it("should be a logic_error", []() -> void {
      bad_variant_access ex;
      bool is_logic_error = std::is_base_of<std::logic_error, bad_variant_access>::value;
      Expect(is_logic_error).ToBe(true);
    });

    it("should have appropriate error message", []() -> void {
      bad_variant_access ex;
      std::string msg = ex.what();
      Expect(msg).ToBe(std::string("bad_variant_access"));
    });

    it("should be thrown by variant get with wrong type", []() -> void {
      variant<int, std::string> var(42);
      bool thrown = false;
      try {
        var.get<std::string>();
      } catch (const bad_variant_access&) {
        thrown = true;
      }
      Expect(thrown).ToBe(true);
    });

    it("should be catchable as logic_error", []() -> void {
      variant<int, std::string> var(42);
      bool caught_as_logic_error = false;
      try {
        var.get<std::string>();
      } catch (const std::logic_error&) {
        caught_as_logic_error = true;
      }
      Expect(caught_as_logic_error).ToBe(true);
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
