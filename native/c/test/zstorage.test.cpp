#include <iostream>
#include <stdexcept>

#include "ztest.hpp"
#include "zstorage.metal.test.h"

using namespace std;
using namespace ztst;

void zstorage_tests()
{

  describe("zstorage tests",
           []() -> void
           {
             it("should obtain and free 31-bit storage",
                []() -> void
                {
                  int size = 128;
                  void *data = NULL;
                  expect(data).ToBeNull();
                  data = STBT31(size);
                  expect(data).Not().ToBeNull();
                  // expect(STFREE(data, size)).ToBe(0); TODO(Kelosky): this test fails!
                });

             it("should obtain and free 64-bit storage",
                []() -> void
                {
                  int size = 256;
                  void *data = NULL;
                  expect(data).ToBeNull();
                  // data = STGET64(size); TODO(Kelosky): this abends
                  expect(data).Not().ToBeNull();
                  // expect(STREL(data)).ToBe(0); TODO(Kelosky): this test fails!
                });
           });
}
