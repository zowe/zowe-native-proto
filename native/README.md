# Miscellaneous Native Development Notes

## Metal C/C/C++ & HLASM

The following sections relate to the non-golang portions of the `native` codebase.

### Testing

Longer term, we may be able to make use of [Catch2](https://github.com/catchorg/Catch2). However, with our testing of Metal C code, we are exploring custom-written infrastructure with the ability to handle abends or other z-specific scenarios. So, in the near term, we will use custom testing infrastructure found in `native/c/test`:

- `ztest.hpp`
- `ztest.cpp`

This infrastructure requires `xlclang` compiler to enable "new" language features (lambdas) in order to provide a ~[Jest](https://jestjs.io/) testing syntax.

#### Format

Tests are written in jest-style:

```c

void my_tests()
{
  describe("my tests",
           []() -> void
           {
             it("should pass the test",
                []() -> void
                {
                  int rc = 0;
                  Expect(rc).ToBe(0);
                });
            });
}

```

#### Building

Tests are built via a `Makefile` in `c/test` and are also a default build target for the project root `Makefile`.

#### Executing

Tests may be run from the `c/test` folder via `make test` or from client project using `npm run z:test`.

#### Results

Test results are printed as they are executed, potentially interlaced with joblog output if using `_BPXK_JOBLOG=STDERR`, e.g.:

```txt
zstorage tests
  PASS  should obtain and free 31-bit storage
  PASS  should obtain and free 64-bit storage
zut tests
  PASS  should upper case and truncate a long string
  PASS  should upper case and pad a short string
zjb tests
  PASS  should be able to list a job
07.29.29 JOB01916  $HASP100 IEFBR14$ ON INTRDR                            FROM STC01874 DKELOSKY
07.29.29 JOB01916  IRR010I  USERID DKELOSKY IS ASSIGNED TO THIS JOB.
```

Summary results are printed upon completion, e.g.:

```txt
======== TESTS SUMMARY ========
Total Suites: 5 passed, 1 failed, 6 total
Tests:      : 13 passed, 1 failed, 14 total
```

#### Debugging

By default, LE or Metal C abends will signal for program termination. When this occurs, a message may appear:

```txt
    unexpected ABEND occured.  Add `TEST_OPTIONS.remove_signal_handling = false` to `it(...)` to capture abend dump
```

In this situation, no CEEDUMP is captured. To disable this behavior, disable signal handling by passing a `TEST_OPTIONS` object as a parameter to `it()`, e.g.:

```c
             TEST_OPTIONS opts = {0};
             opts.remove_signal_handling = true;

             it("should recover from an abend", []() -> void
                {
                  int rc = ZRCVYEN();
                Expect(rc).ToBe(0); }, opts);
           });
```

#### API

The testing infrastructure provides the following APIs

##### describe(name, fn)

`describe(name, fn)` creates a block that groups related tests. One top level `describe()` must exist and nesting is not yet supported.

##### `it(name, fn)

`it(name, fn)` is a test case contained within a `describe()` block. Individual tests and assertions are achieved with `expect()` calls and are contained within the `fn` provided by the second operand.

##### expect(value)

The `expect()` function is used to test a value (`int`, `string`, `pointer`, etc...) which returns a `RESULT_CHECK` object. To assert that a result matches an expected value,
use the `ToBe()` function on the `RESULT_CHECK` object.

Multiple `expect()` functions can exist within a `test()` to test multiple scenarios. However, if an `expect()` fails, the remaining `expect()` calls in a `test()` are skipped.

###### Expect(value) && ExpectWithContext(value, context)

`Expect(value)` and `ExpectWithContext(value, context)` are two C macros (`#define`s) which provide extra information whenever an `expect` fails. These provide the source filename and line number for which `expect` failed. `ExpectWithContext()` allows providing a second parameter to provide extra debugging information alongside a failed test.

For example:

```c
ExpectWithContext(5, "Failed to submit job").ToBe(3);
```

Produces:

```txt
  FAIL  should fail
    expected int '5' to be '3'
    at ./zjb.test.cpp:132   (Failed to submit job)
```

##### ToBe(value)

Use `.ToBe()` to compare values set in `RESULT_CHECK` through `expect`. If the expected value and checked value do not match, an exception is thrown and the test fails.

##### Not()

Use `.Not()` to inverse compare values behavior. `Not()` returns a `RESULT_CHECK` object which is then checked vai `ToBE()`.

### Creating a CHDR from a DSECT

To create a C header from an HLASM DSECT:

- create a `.s` file in `asmchdr` folder, e.g. `asasymbp.s`
- upload, e.g. `npm run z:deploy asmchdr/asasymbp.s`
- allocate output adata data set if none exists, e.g. `zowex data-set create-adata <hlq>.adata`
- allocate output chdr data set if none exists, e.g. `zowex data-set create-vb <hlq>.chdr`
- build `.s` file, e.g. `as -madata --gadata="//'<hlq>.USER(ASASYMBP)'" asasymbp.s`
- convert the file via `ccnedsct`, e.g. `zowex tool ccnedsct --ad "<hlq>.adata(asasymbp)" --cd "<hlq>.chdr(asasymbp)"`
- copy, download the `.h` file where needed, e.g. `zowe files download ds "<hlq>.chdr(asasymbp)" --file native/c/chdsect/asasymbp.h`

### Recovery

In order to use `ESTAEX`-type recovery, Language Environment (LE) must be enabled with `TRAP(ON,NOSPIE)`. Otherwise, `ESPIE` recovery will gain control in an
abend scenario and bypass any established `ESTAEX`.

- `_CEE_RUNOPTS` https://www.ibm.com/docs/en/zos/3.1.0?topic=options-how-specify-runtime
- `TRAP(ON,NOSPIE)` https://www.ibm.com/docs/en/zos/3.1.0?topic=ulero-trap

### Debugging Metal C

You can add temporary debugging messages within Metal C code which will issue `WTO` messages with a message prefix of `ZWEX0001I`. These messages may be used in development only
and must not appear in distributed versions of `zowex`.

To see these debugging messages within z/OS UNIX, set the environment variable `export _BPXK_JOBLOG=STDERR`. Then add debug messages via `zwto_debug(...)` found within `zwto.h` in the same format as C `printf` format strings, e.g. `zwto_debug("return code was %d", rc);`.

### Dumping Storage

Raw storage address contents can be printed to the console or a file to help with debugging scenarios. This can be achieved using the `zdbg.h` header file.

### LE C/C++

To dump storage in LE, use:

```c
#include "zdbg.h"

  int data = 3;

  zut_dump_storage(title, &data, sizeof(data), zut_debug_message);

```

By default, output is printed to `STDERR` when using `zut_debug_message()`; however, you may provide alternative callback functions via a function pointer in place of the default `zut_debug_message`.

### Metal C

You must ensure `zut_alloc_debug()` is called to allocate an output DD for log messages. Then in Metal C, use

```c
#include "zdbg.h"

  ZJB zjb = {0};

  zut_dump_storage("ZJB", zjb, sizeof(ZJB), ZUTDBGMG);

```

By default, output is printed to `/tmp/zowex_debug.txt` when using `ZUTDBGMG()`; however, you may provide a Metal C compatible alternative.
