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

#ifndef ZTEST_HPP
#define ZTEST_HPP

#include <iostream>
#include <stdexcept>
#include <vector>

// TODO(Kelosky): handle test not run
// TODO(Kelosky): handle running individual test and/or suite

#define Expect(x) [&]() -> RESULT_CHECK { EXPECT_CONTEXT ctx = {__LINE__, __FILE__}; return expect(x, ctx); }()
#define ExpectWithContext(x, context) [&]() -> RESULT_CHECK { EXPECT_CONTEXT ctx = {__LINE__, __FILE__, std::string(context)}; return expect(x, ctx); }()

extern std::string matcher;

namespace ztst
{

  struct EXPECT_CONTEXT
  {
    int line_number;
    std::string file_name;
    std::string message;
    bool initialized;
  };

  class RESULT_CHECK
  {
    int int_result;
    std::string string_result;
    bool inverse;
    void *pointer_result;
    EXPECT_CONTEXT ctx;

  public:
    void ToBeGreaterThan(int);
    void ToBe(int);
    void ToBe(std::string);
    void ToBeNull();
    std::string append_error_details();
    RESULT_CHECK Not();
    RESULT_CHECK() {}
    ~RESULT_CHECK() {}

    void set_inverse(bool v)
    {
      inverse = v;
    }

    void set_context(EXPECT_CONTEXT &c)
    {
      ctx = c;
      ctx.initialized = true;
    }

    void set_result(int r)
    {
      int_result = r;
    }

    void set_result(std::string r)
    {
      string_result = r;
    }

    void set_result(void *r)
    {
      pointer_result = r;
    }
  };

  struct TEST_OPTIONS
  {
    bool remove_signal_handling;
  };

  typedef void (*cb)();

  void describe(std::string description, cb suite);

  void it(std::string description, cb test);
  void it(std::string description, cb test, TEST_OPTIONS &opts);

  RESULT_CHECK expect(int val);
  RESULT_CHECK expect(int val, EXPECT_CONTEXT &ctx);
  RESULT_CHECK expect(std::string val);
  RESULT_CHECK expect(std::string val, EXPECT_CONTEXT &ctx);
  RESULT_CHECK expect(void *val);
  RESULT_CHECK expect(void *val, EXPECT_CONTEXT &ctx);

  int report();

  int tests(cb tests);
}

#endif
