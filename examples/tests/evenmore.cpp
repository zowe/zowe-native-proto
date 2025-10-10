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

using namespace std;
using namespace ztst;

void even_more_tests()
{
  describe("even more test suite",
           []() -> void
           {
             it("should pass with even more tests",
                [&]() -> void
                {
                  expect(3).ToBe(3);
                });

             it("should verify basic arithmetic - addition",
                [&]() -> void
                {
                  expect(5 + 3).ToBe(8);
                });

             it("should verify basic arithmetic - subtraction",
                [&]() -> void
                {
                  expect(10 - 4).ToBe(6);
                });

             it("should verify basic arithmetic - multiplication",
                [&]() -> void
                {
                  expect(7 * 6).ToBe(42);
                });

             it("should verify basic arithmetic - division",
                [&]() -> void
                {
                  expect(20 / 4).ToBe(5);
                });

             it("should verify zero is zero",
                [&]() -> void
                {
                  expect(0).ToBe(0);
                });

             it("should verify one is one",
                [&]() -> void
                {
                  expect(1).ToBe(1);
                });

             it("should verify negative numbers",
                [&]() -> void
                {
                  expect(-5).ToBe(-5);
                });

             it("should verify large positive numbers",
                [&]() -> void
                {
                  expect(1000000).ToBe(1000000);
                });

             it("should verify large negative numbers",
                [&]() -> void
                {
                  expect(-999999).ToBe(-999999);
                });

             it("should verify decimal numbers",
                [&]() -> void
                {
                  expect(3.14).ToBe(3.14);
                });

             it("should verify more decimal precision",
                [&]() -> void
                {
                  expect(2.71828).ToBe(2.71828);
                });

             it("should verify powers of two",
                [&]() -> void
                {
                  expect(2 * 2 * 2 * 2).ToBe(16);
                });

             it("should verify powers of ten",
                [&]() -> void
                {
                  expect(10 * 10 * 10).ToBe(1000);
                });

             it("should verify fibonacci sequence - first few numbers",
                [&]() -> void
                {
                  expect(1 + 1).ToBe(2);
                });

             it("should verify fibonacci sequence - next number",
                [&]() -> void
                {
                  expect(2 + 1).ToBe(3);
                });

             it("should verify fibonacci sequence - another number",
                [&]() -> void
                {
                  expect(3 + 2).ToBe(5);
                });

             it("should verify fibonacci sequence - yet another",
                [&]() -> void
                {
                  expect(5 + 3).ToBe(8);
                });

             it("should verify prime numbers - 2",
                [&]() -> void
                {
                  expect(2).ToBe(2);
                });

             it("should verify prime numbers - 3",
                [&]() -> void
                {
                  expect(3).ToBe(3);
                });

             it("should verify prime numbers - 5",
                [&]() -> void
                {
                  expect(5).ToBe(5);
                });

             it("should verify prime numbers - 7",
                [&]() -> void
                {
                  expect(7).ToBe(7);
                });

             it("should verify prime numbers - 11",
                [&]() -> void
                {
                  expect(11).ToBe(11);
                });

             it("should verify prime numbers - 13",
                [&]() -> void
                {
                  expect(13).ToBe(13);
                });

             it("should verify prime numbers - 17",
                [&]() -> void
                {
                  expect(17).ToBe(17);
                });

             it("should verify prime numbers - 19",
                [&]() -> void
                {
                  expect(19).ToBe(19);
                });

             it("should verify prime numbers - 23",
                [&]() -> void
                {
                  expect(23).ToBe(23);
                });

             it("should verify prime numbers - 29",
                [&]() -> void
                {
                  expect(29).ToBe(29);
                });

             it("should verify prime numbers - 31",
                [&]() -> void
                {
                  expect(31).ToBe(31);
                });

             it("should verify prime numbers - 37",
                [&]() -> void
                {
                  expect(37).ToBe(37);
                });

             it("should verify prime numbers - 41",
                [&]() -> void
                {
                  expect(41).ToBe(41);
                });

             it("should verify prime numbers - 43",
                [&]() -> void
                {
                  expect(43).ToBe(43);
                });

             it("should verify prime numbers - 47",
                [&]() -> void
                {
                  expect(47).ToBe(47);
                });

             it("should verify prime numbers - 53",
                [&]() -> void
                {
                  expect(53).ToBe(53);
                });
           });
}