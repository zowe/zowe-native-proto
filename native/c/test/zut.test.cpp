#include <iostream>
#include <stdexcept>

#include "ztest.hpp"
#include "zut.hpp"
#include "zstorage.metal.test.h"

using namespace std;
using namespace ztst;

void zut_tests()
{

  describe("zut tests",
           []() -> void
           {
             it("should upper case and truncate a long string",
                []() -> void
                {
                  char buffer[9] = {0};
                  string data = "lowercaselongstring";
                  zut_uppercase_pad_truncate(buffer, data, sizeof(buffer) - 1);
                  expect(string(buffer)).ToBe("LOWERCAS");
                });

             it("should upper case and pad a short string",
                []() -> void
                {
                  char buffer[9] = {0};
                  string data = "abc";
                  zut_uppercase_pad_truncate(buffer, data, sizeof(buffer) - 1);
                  expect(string(buffer)).ToBe("ABC     ");
                });
           });
}
