# Python zIIP execution plan

## Objective and scope

Reduce general-purpose CPU consumed per completed Python workload while preserving
results, acceptable elapsed time, and bounded memory. Establish a reproducible
baseline before changing the shared C++/Metal C implementation or public APIs.

The first workload is a generated USS text file processed through the existing
Python bindings. It needs no production data, Db2 connection, or dataset changes.
Dataset, job, and certificate workflows follow only when measurements identify a
specific opportunity. Db2 access-method changes are a separate experiment.

## Eligibility boundaries

- IBM Python can offload up to 70% of designated eligible processing. This is not
  a whole-application target or a promise of 100% offload during computation.
- Custom SWIG/C++/Metal C code does not inherit Python eligibility. Native code
  improvements can reduce GP CPU, but removing I/O does not establish eligibility.
- Let the supported IBM runtime manage eligibility. Do not introduce custom SRB
  dispatch, use private eligibility controls, or mark our extensions APF-authorized.
- Buffered iteration is allowed. Prefer measured batches within a memory budget;
  load an entire input only when its expanded representation and copies fit.
- Preserve record boundaries, EBCDIC/UTF-8 behavior, errors, and resource cleanup.
- Db2's up-to-60% figure applies to specified server-side SQL processing through
  DDF/native REST. Measure client and server CPU separately when that work begins.

## 1. Runtime and measurement prerequisites

- [x] Review the plan against the current bindings and IBM eligibility guidance.
- [x] Replace absolute offload and whole-input rules with measured acceptance gates.
- [x] Record host, z/OS release, exact executable, Python build, and binding paths.
- [x] Inspect the selected runtime's `libpyzutil` APF attribute without changing it.
- [x] Record maintenance evidence, including later eligibility-restoration fixes.
      PH52983/OA63406 are the original enablement baseline; Python 3.11.7 predates
      PH65232 (included in 3.11.11). Use fixed executable paths for each runtime.
- [x] Establish CPU accounting that distinguishes GP, offload, and eligible-on-GP
      time. Record normalization and scope; elapsed time alone proves no offload.
- [x] Run a pure-Python computation control and check that actual offload is present.

Do not report prerequisite checks as proof that an installed APAR was applied.
Release information, APF attributes, maintenance records, and observed behavior
are distinct evidence. Unsupported or unavailable metrics must be reported as such.

## 2. Reproducible benchmark

- [x] Add a standalone benchmark outside the binding APIs, with machine-readable
      results, runtime metadata, correctness checks, and deterministic generated data.
- [x] Compare compute-only, import-only, and compute/read/compute controls.
      Record phase wall times and cumulative task CPU; individual-phase offload
      needs separate system accounting.
- [x] Compare buffered Python iteration, bounded Python batches, whole-file Python
      reads, and the existing whole-file binding under the same transform.
- [x] Run small and larger inputs in fresh processes, using a warm-up and at least
      five measured repetitions. Rotate variant order to reduce order bias.
- [x] Record elapsed time, GP CPU, normalized total CPU, actual offload time,
      eligible-on-GP time, and supported peak-memory measurements per result.
- [x] Verify equal record counts and deterministic output checksums across variants.
- [x] Preserve raw results, commands, source revision, and a concise analysis.

Start with modest synthetic inputs. Account for process-wide versus thread CPU,
filesystem caching, concurrent LPAR load, instrumentation overhead, and Python
version differences. A measurement helper must not alter eligibility during the
measured work. The validated approach samples once after work; CPU totals include
startup and imports, while wall time covers the operation itself.

## 3. Select and implement the supported optimization

- [ ] Identify the measured source of excess GP CPU: repeated calls, copying,
      encoding, transform placement, or I/O behavior.
- [ ] Select the smallest compatible change supported by the baseline.
- [ ] Preserve streaming interfaces and explicit memory limits.
- [ ] Keep system operations out of repeated compute loops where dependencies allow;
      preserve required ordering, transactions, and intermediate checkpoints.
- [ ] Re-run the same workload, compare medians and spread, and run appropriate
      functional tests for any binding or native-code changes.

Initial performance gate: at least 10% lower median GP CPU per record, no more than
5% elapsed-time regression beyond observed run variation, identical results, and
memory within the stated workload budget. These are experiment criteria, not a
performance promise. If no variant meets the gate, retain the existing behavior
and document that result rather than broadening the refactor without evidence.

## 4. Delivery

- [x] Document the candidate workload pattern, benchmarked runtimes, measurement
      limitations, and remaining production-workload validation.
- [ ] Add the relevant changelog entry if user-facing behavior changes.
- [x] Run the checks appropriate to the delivered files; repository lint/build are
      required if preparing a PR.
- [x] Record completed and outstanding steps here so work survives compaction.

## Execution checkpoint and benchmark findings

The benchmark stage is complete. Measurements ran on 2026-10-21 local time
(2026-10-22 UTC), on B025, OS/390 release 29.00, machine 3932. The user selected
benchmark findings before production-workload optimization. No binding API,
shared native implementation, runtime default, or APF attribute was changed.

### Runtime controls

Five measured repetitions per scenario used two ten-million-iteration Python
compute phases. These are medians for the compute-only scenario:

| Installed Python | `libpyzutil` APF | GP CPU (s) | Actual offload CPU, unnormalized (s) |
| --- | --- | ---: | ---: |
| 3.11.7, default | No | 3.9551 | 0 |
| 3.13.15 | Yes | 2.1660 | 2.1328 |
| 3.14.7 | Yes | 2.2392 | 2.4200 |

The default runtime recorded no actual offload in any control sample. Both newer
installed builds offloaded. Their version and APF differences are confounded;
these results do not isolate either cause or establish installed SMP/E maintenance.
Exact executable paths, build strings, module paths, and attributes are in the
raw reports. The benchmark used the existing multi-runtime `zbind` wheel.

Import-only medians were similar to compute-only medians on both newer runtimes.
Each had one high-GP outlier among five compute/read/compute samples: 4.1925 s
on 3.13 and 4.3031 s on 3.14, versus scenario medians of 2.1770 s and 2.2263 s.
The initial short probes do not establish a permanent loss of eligibility after
an import or native call. Individual-phase CPU attribution remains unverified.

### USS read/transform comparison

The initial Python 3.14 run used 32,768 and 262,144 deterministic UTF-8 records,
five repetitions, fresh processes, warm-ups, and rotating variant order. Wide
GP variation justified a second run at 262,144 records with twelve repetitions,
giving every variant three turns in every order position. Its ancillary controls
used one million iterations; file workloads were unchanged.

For the 262,144-record input (6.25 MiB):

| Variant | Initial median GP (s), n=5 | Confirmation median GP (s), n=12 | Confirmation median elapsed (s) | Traced Python peak (MiB) |
| --- | ---: | ---: | ---: | ---: |
| Buffered iteration | 0.3872 | 0.1535 | 0.4675 | 0.158 |
| 65,536-character batches | 0.3630 | 0.0885 | 0.4500 | 0.616 |
| Whole-file Python read | 0.0169 | 0.0507 | 0.4491 | 28.332 |
| Existing whole-file USS binding | 0.4262 | 0.4285 | 0.4412 | 28.206 |

Batches reduced median GP CPU versus the binding by 14.8% initially and 79.3%
in confirmation. Median elapsed time changed by -0.5% and +2.0%, respectively.
Both runs therefore pass the synthetic GP and elapsed gates. The Python allocation
peak stayed at 646,106 bytes across the eightfold input-size increase; buffered
iteration stayed at 166,025 bytes. Whole-file representations grew with input.
The batch hint is bounded here because generated record sizes are bounded; it is
not a hard memory cap for arbitrary records.

GP variation remains material: confirmation batches ranged from 0.0159 to
0.5191 s, versus 0.4171 to 0.5040 s for the binding. The median saving is not a
production guarantee. At the smaller 0.78125 MiB size, buffered iteration had the
lowest median GP CPU, 0.0168 s versus 0.0734 s for the binding; startup costs make
small-input comparisons especially sensitive.

### Measurement limits and next step

The [standalone harness](native/python/benchmarks/ziip/README.md) uses an
unprivileged TIMEUSED helper with a below-bar result buffer and F4SA. Calling
the helper before computation affected subsequent offload in the control probes;
loading it alone did not. The worker samples once after all measured work.
CPU totals include task startup and imports; elapsed time covers only the
operation. Normalized offload percentages are not raw processor-time shares or
the IBM eligibility ceiling.

All 88 timed file samples and all twelve separate memory passes matched expected
record counts and checksums. Six correctness/accounting tests passed locally and
on each installed z/OS runtime. The native helper built successfully and remained
non-APF-authorized. Repository-wide lint/build were not run; no PR was prepared.

Process-wide memory, native allocation peaks, service class, concurrent LPAR load,
and SMF/RMF corroboration were not captured. `ru_maxrss` returned zero and is
reported as unavailable. Production memory and performance acceptance remain open.

The next candidate is a bounded-batch Python USS read/transform pilot using a
fixed newer runtime and an explicit maximum record size. Validate it in the real
script, including its other native calls, and collect process memory plus system
CPU accounting before changing shared binding/native code. Step 3 remains pending.

Local evidence is preserved in `dist/ziip/manifest.json`, `analysis.json`,
`results-314.json`, `confirmation-314.json`, `controls-313.json`,
`controls-311.json`, and `measurement-probes-complete.json`. The manifest records
reproduction settings and source, report, helper, and wheel hashes. These artifacts
are ignored by Git; the harness and this plan are the reviewable source changes.

## References

- [IBM Python eligibility terms](https://public.dhe.ibm.com/systems/support/warranty/pdfs/aut/Authorized_Use_Table_06-2023_en_US.pdf)
- [IBM Python fix list](https://www.ibm.com/support/pages/fixlist-ibm-open-enterprise-sdk-python)
- [PH52983](https://www.ibm.com/support/pages/apar/PH52983) and [OA63406](https://www.ibm.com/support/pages/apar/OA63406)
- [TIMEUSED accounting enhancements and units](https://www.ibm.com/support/pages/apar/OA38409)
- [TIMEUSED calling requirements](https://www.ibm.com/docs/en/zos/2.5.0?topic=xct-timeused-obtain-accumulated-cpu-vector-time)
- [Db2 eligibility](https://www.ibm.com/docs/en/db2-for-zos/13.0.0?topic=db2-authorized-ziip-uses-processing)
