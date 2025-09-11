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
  DestructorTester(bool *flag)
      : destroyed(flag)
  {
    *destroyed = false;
  }
  ~DestructorTester()
  {
    *destroyed = true;
  }
  bool *destroyed;
};
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
      bool destroyed = false;
      {
        unique_ptr<DestructorTester> ptr(new DestructorTester(&destroyed));
        Expect(destroyed).ToBe(false);
      }
      Expect(destroyed).ToBe(true);
    });

    it("should release ownership", []() -> void {
      bool destroyed = false;
      DestructorTester *raw_ptr = new DestructorTester(&destroyed);
      unique_ptr<DestructorTester> ptr(raw_ptr);
      DestructorTester *released_ptr = ptr.release();
      Expect(ptr.get()).ToBe(nullptr);
      Expect(released_ptr).ToBe(raw_ptr);
      Expect(destroyed).ToBe(false);
      delete released_ptr;
      Expect(destroyed).ToBe(true);
    });

    it("should reset the pointer", []() -> void {
      bool destroyed1 = false;
      bool destroyed2 = false;
      unique_ptr<DestructorTester> ptr(new DestructorTester(&destroyed1));
      DestructorTester *raw_ptr2 = new DestructorTester(&destroyed2);

      ptr.reset(raw_ptr2);
      Expect(destroyed1).ToBe(true);
      Expect(ptr.get()).ToBe(raw_ptr2);

      ptr.reset();
      Expect(destroyed2).ToBe(true);
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
      bool destroyed = false;
      unique_ptr<DestructorTester> ptr1(new DestructorTester(&destroyed));

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
      bool destroyed = false;
      {
        unique_ptr<DestructorTester> ptr = make_unique<DestructorTester>(&destroyed);
        Expect(destroyed).ToBe(false);
      }
      Expect(destroyed).ToBe(true);
    }); });
}
