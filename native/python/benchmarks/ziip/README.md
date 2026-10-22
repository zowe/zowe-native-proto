# Python zIIP benchmark

This standalone experiment measures the existing bindings. It does not change
their APIs, processor eligibility, APF attributes, or the default interpreter.
It uses generated USS files and needs no production datasets or credentials.

## Build and run on z/OS

Use a fixed IBM Python executable and a matching built or installed `zbind`.
For the multi-runtime wheel, an isolated venv installed from that wheel is also
suitable. All worker processes use the same executable as the parent.

Build the optional, unprivileged timing DLL with IBM Open XL C and HLASM:

```sh
make
```

The helper uses the supported `TIMEUSED ECT=YES` interface with a work area and
F4SA register-save area below 2 GB. It checks the hardware and OS support bits
and returns an error when the requested accounting is unavailable. The DLL
does not need APF authorization.

Python sources must be UTF-8. If a text transfer converted them to IBM-1047,
preserve the transferred sources and stage a UTF-8 copy before running:

```sh
mkdir -p utf8
iconv -f IBM-1047 -t UTF-8 benchmark.py > utf8/benchmark.py
iconv -f IBM-1047 -t UTF-8 test_benchmark.py > utf8/test_benchmark.py
chtag -t -c 1208 utf8/benchmark.py utf8/test_benchmark.py
/path/to/python -m unittest discover -s utf8 -v
/path/to/python utf8/benchmark.py suite \
  --timer ./cpu_time.so --bindings /path/to/matching/bindings \
  --output results.json
```

For sources already in UTF-8, invoke `benchmark.py` directly. Never convert an
already converted upload twice. `--bindings` is unnecessary if the modules are
installed in the selected interpreter's environment. Each result filename must
be new; an existing report is never overwritten.

The defaults are five repetitions, 32,768 and 262,144 records, 65,536-character
batch hints, and ten million iterations per compute phase. Use `--rows`,
`--repeats`, `--chunk-chars`, and `--iterations` to change these. `--rows` with
no values runs only the compute, import, and native-boundary controls.

The suite runs serially. Each measurement has a fresh process; file variants
are warmed up and their order rotates between repetitions. Wall time covers only
the operation. CPU counters cover the current task from creation through one
sample after the operation, including interpreter startup, imports, and timer
loading. Fixture creation happens in the parent; inventory and serialization
follow the CPU sample. Temporary fixtures are removed on normal exit
or an exception. Partial results are saved if a worker fails.

## Workloads

- `compute`: two identical pure-Python arithmetic phases.
- `import`: the same phases after importing the existing USS binding, without
  calling its read operation.
- `boundary`: the same phases separated by one existing USS binding read and
  transform of a small file, including its first import; compare with both other
  controls. Phase wall times are separate; CPU accounting covers the whole task.
- `buffered`: normal Python text-file iteration.
- `batched`: Python `readlines(sizehint)` followed by processing each batch.
- `whole`: Python whole-file read followed by `splitlines()`.
- `binding`: existing `zusf_py.read_uss_file()` followed by `splitlines()`.

Every file variant runs the same transform and validates its count and rolling
checksum against values calculated from the generated records. Inputs contain
UTF-8 text and are tagged 1208 on z/OS. This is a read/transform microbenchmark;
it does not model output I/O, Db2, mixed record formats, or a production workload.

The batch hint is not a strict arbitrary-input memory limit: `readlines()` can
exceed it by one record, and Python objects have overhead. The generated records
are bounded. Production code must also constrain the maximum record size.

## Interpreting results

| Field | Meaning |
| --- | --- |
| `elapsed_seconds` | Wall time of the operation |
| `total_normalized_seconds` | Cumulative current-task CPU, normalized to CP speed |
| `gp_seconds` | Current-task time on general processors |
| `offload_raw_seconds` | Actual time on offload processors, not normalized |
| `eligible_on_gp_seconds` | Eligible work executed on CP; already included in GP time |
| `offload_normalized_percent` | `100 * (total_normalized - GP) / total_normalized` |
| `python_peak_bytes` | Peak traced Python allocations in a separate run |
| `ru_maxrss_reported` | Raw platform report; null when the platform returns zero |

Never add raw offload time to normalized CP time to compute a percentage.
Reports preserve the raw counter deltas. Consistency checks allow one microsecond
of field skew, observed in the GP-only control; a negative offload percentage
within that tolerance is reported as zero. Larger inconsistencies fail the run.
The helper measures the calling task, not all threads or address-space services.
On the supported zIIP-only machines the offload counter represents zIIP time.
An initial native timer call suppressed subsequent offload in the control probes,
so the worker samples only after all measured work. Never use this helper before
the operation to subtract startup CPU: that would change the behavior measured.
CPU per record therefore includes startup cost, which matters most for small
inputs. Compare identically configured fresh processes and corroborate surprising
restoration results with system accounting before attributing them to a binding.

Memory tracing runs separately and excludes native buffers. An unavailable
`ru_maxrss` is not evidence of zero memory consumption. This suite cannot by
itself certify a process-wide peak-memory budget. Collect platform memory and
SMF/RMF data for production acceptance, including service-class and LPAR load.

Inspect the active runtime's `libpyzutil` extended attributes and maintenance
level before interpreting an offload result. Functional correctness and an APF
attribute alone do not prove that work ran on a zIIP. Reports record the actual
interpreter, module paths, and attributes without changing them.

CPU accounting: [IBM TIMEUSED enhancement and field definitions](https://www.ibm.com/support/pages/apar/OA38409).
Calling requirements: [IBM TIMEUSED reference](https://www.ibm.com/docs/en/zos/2.5.0?topic=xct-timeused-obtain-accumulated-cpu-vector-time).
Runtime maintenance: [IBM Python fix list](https://www.ibm.com/support/pages/fixlist-ibm-open-enterprise-sdk-python).
