# System bindings implementation tasks

Working branch: `zf/pybi-system`. Source uploads are managed by the user's watcher; use SSH only for builds and tests.

Approved scope: [ZSYS_PLAN.md](ZSYS_PLAN.md). Existing native and Python APIs remain compatible.

- [x] Inventory system commands, existing bindings, native dependencies, and packaging paths.
- [x] Review and approve the design, including PROCLIB/SYSLOG migration and compatibility wrappers.
- [x] Extract shared spool allocation and system operations; preserve legacy entry points and struct identities.
- [x] Extract shared SYSLOG option resolution without changing CLI behavior.
- [x] Correct PROCLIB bounds/error preservation and VSAM control-block ownership.
- [x] Add native resolver, compatibility, and regression tests.
- [x] Implement `zsys_py` results, validation, text conversion, structured errors, and SWIG interface.
- [x] Integrate all five modules into selective builds, release wheels, source bundles, and precompiled packaging.
- [x] Add Python contract, integration, lifetime, and installed-wheel tests.
- [x] Update documentation, examples, changelog, and CI coverage.
- [x] Delegate source synchronization to the user's watcher as requested. No manual uploads.
- [x] Build native targets and Python extensions; resolve compiler/linkage issues on z/OS.
- [x] Run native, existing-binding, system-binding, and installed-wheel checks on available supported runtimes.
- [x] Run appropriate local checks and review the complete diff for compatibility and scope.
- [x] Download the validated wheel and verify its checksum, contents, and RECORD hashes locally.
- [x] Audit the API matrix and remaining integration references; correct system support flags, module/encoding documentation, bundle examples, and test help.

## Compatibility decisions

- Keep `zjb_list_proclib`, `zjb_read_syslog`, `ZJBSyslogOptions`, and `ZJBSyslogResponse` intact as public contracts.
- Keep certificate and key-ring operations in `zkr_py`; add `zsys_py` to the existing `zo` distribution.
- Resolve Python SYSLOG defaults with the native system offset, independent of process TZ; keep the CLI's existing process-time behavior.
- Preserve native result ordering, append behavior, diagnostic fields, allocation tokens, and cleanup precedence.
- Keep CLI parsing behavior; the new Python interface validates types, dates, and explicit option presence strictly.
- Preserve the wheel's existing runtime selection and isolated SWIG type tables.
- Report SYSLOG `has_more` as a limit indicator; provide no automatic pagination guarantee.

## Validation record

- Full native libraries, CLI, and test runner build successfully on z/OS.
- All five Python extensions pass a forced rebuild with Python 3.11.7 and SWIG 4.3.1.
- Native checks: 7 resolver/capacity/legacy API tests, 19 SYSLOG CLI tests, and 6 job regressions passed. The existing INPUT-job test remains skipped. CLI timestamp assertions used a process timezone matching the system offset.
- Python source checks: 118 system tests (including CLI list parity and process-timezone independence), 12 cross-module lifetime tests, and 8 isolated adapter tests passed.
- The isolated adapter uses the Limited API and EBCDIC service substitutes to cover UTF-8, embedded NULs, empty results, failed partial reads, bounded diagnostics, and Python reference ownership.
- Built `zo-1.0.2-py3-none-any.whl` with all five modules for IBM Python 3.11.7, 3.13.15, and 3.14.7.
- Fresh installed-wheel checks: 242 tests passed on Python 3.11 (system, dataset, job, USS, and certificate/key-ring suites); 137 tests passed on each of Python 3.13 and 3.14 (installation, system, and cross-module lifetimes). No skips or failures. Uninstall/reinstall and all five imports passed on each runtime.
- Confirmed that functional tests left no scratch datasets or key rings. Temporary wheel-test venvs were removed; diagnostic logs and JUnit reports were preserved.
- Fixed test-environment issues found during validation: honor `TMPDIR` for small `/tmp` filesystems, run the runtime matrix sequentially, and restrict pytest discovery so it cannot import source bindings alongside installed DLLs.
- Downloaded the wheel to `dist/zsys/zo-1.0.2-py3-none-any.whl`, with a matching `.sha256` file. Verified all 15 runtime libraries, Python source syntax, wheel tags, and RECORD hashes. SHA-256: `1927d3b0fcecd67a834b375853d1159ad2e7dfc2d4c33962b2818d75d2e275bf`.
- Local `npm run build`, `npm run lint`, Python syntax checks, generated source-bundle setup syntax, and `git diff --check` passed. Build-tools Biome check passed with two pre-existing informational suggestions. An unrelated SDK formatting change made by lint was removed.
- CI runs source tests by default. Provisioned hosts can enable the three-runtime installed-wheel gate with `ZRS_PYTHON_WHEEL_MATRIX=true`.
- Header changes were followed by source touches and a forced extension rebuild to avoid stale objects.
- Validation was completed before local commit creation. No PRs or publishing have been performed. The user manages source uploads. Changelog PR links can be added when a PR exists.

## Commit organization

The implementation is split into six local commits in dependency order, each with the user's attestation block:

1. Native storage, PROCLIB bounds, diagnostics, and SWIG utility linkage fixes.
2. Shared system/spool services, legacy compatibility, native tests, and existing job-binding link dependencies.
3. The `zsys_py` API and its integration into source builds, wheels, and source/precompiled bundles.
4. Python contracts, native adapter fixtures, integration, lifetime, and installed-wheel tests.
5. CI triggers and the optional installed-wheel runtime matrix.
6. API documentation, approved plan, completed task/validation record, and changelog.

A follow-up documentation commit corrects `doc/apis.md`, the architecture and
bindings guides, precompiled-bundle examples, test help, and the CI dependency comment.
The audit also checked all five module registrations, shared job/system link
dependencies, package manifests, and test/CI entry points. No additional missing
system-binding registrations or package entries were found.

The commits remain local until the user authorizes a push. The personal `test.py` examples retain their existing ignore rule.

## Native behavior characterized during testing

- Native SYSLOG positioning can return current records for a future start. Empty-read behavior is therefore tested with controlled native responses.
- CONVTOD rejects some extreme dates within the CLI's accepted calendar range; the binding preserves these native failures as structured errors.
- The Python process timezone can differ from the system offset used to read SYSLOG. Python default resolution now uses the system offset; CLI behavior remains compatible.
