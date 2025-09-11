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
#include <string>
#include <stdint.h>

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

void zstd_tests()
{
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
}
