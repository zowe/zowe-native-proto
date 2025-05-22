#include <iostream>
#include "zstorage.test.hpp"
#include "zut.test.hpp"
#include "zjb.test.hpp"
#include "ztest.hpp"

using namespace std;
using namespace ztst;

int main()
{

  tests(
      []() -> void
      {
        zstorage_tests();
        zut_tests();
        zjb_tests();
      });

  return 0;
}
