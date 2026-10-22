"""Standalone read/transform and native-boundary measurements for IBM Python."""

# SPDX-License-Identifier: EPL-2.0
# Copyright Contributors to the Zowe Project.

import argparse
import ctypes
import glob
import hashlib
import importlib
import json
import os
from pathlib import Path
import platform
import resource
import statistics
import subprocess
import sys
import tempfile
import time
import tracemalloc


VARIANTS = ("buffered", "batched", "whole", "binding")
MASK = (1 << 64) - 1
TOD_PER_SECOND = 4096 * 1_000_000
COUNTER_TOLERANCE = 4096  # One microsecond: fields can have sub-microsecond skew.


class CpuClock:
    def __init__(self, library):
        self.library = ctypes.CDLL(str(Path(library).resolve())) if library else None
        if self.library:
            self.library.znp_cpu_time.argtypes = [ctypes.POINTER(ctypes.c_ulonglong)]
            self.library.znp_cpu_time.restype = ctypes.c_int

    def read(self):
        if not self.library:
            return None
        values = (ctypes.c_ulonglong * 4)()
        rc = self.library.znp_cpu_time(values)
        if rc:
            raise RuntimeError(f"TIMEUSED failed: rc={rc}")
        return list(values)


def cpu_delta(before, after):
    names = ("total_normalized_seconds", "gp_seconds", "offload_raw_seconds", "eligible_on_gp_seconds")
    if before is None:
        return dict.fromkeys((*names, "offload_normalized_percent"))
    ticks = [end - start for start, end in zip(before, after)]
    if (any(value < 0 for value in ticks)
            or ticks[1] > ticks[0] + COUNTER_TOLERANCE
            or ticks[3] > ticks[1] + COUNTER_TOLERANCE):
        raise RuntimeError(f"Inconsistent TIMEUSED counters: {ticks}")
    result = dict(zip(names, (value / TOD_PER_SECOND for value in ticks)))
    result["raw_delta_ticks"] = ticks
    result["offload_normalized_percent"] = 100 * max(0, ticks[0] - ticks[1]) / ticks[0] if ticks[0] else None
    return result


def compute(iterations):
    value = 1
    for _ in range(iterations):
        value = (value * 1664525 + 1013904223) & 0xFFFFFFFF
    return value


def consume(lines):
    count = checksum = 0
    for line in lines:
        identifier, group, amount, label = line.rstrip("\n").split(",")
        value = int(identifier) * 31 + int(group) * 17 + int(amount) * 13 + len(label)
        checksum = ((checksum * 1000003) ^ value) & MASK
        count += 1
    return {"records": count, "checksum": checksum}


def input_lines(path, variant, chunk_chars):
    if variant == "binding":
        binding = importlib.import_module("zusf_py")
        yield from binding.read_uss_file(str(path), "UTF-8").splitlines()
        return
    with open(path, encoding="utf-8", newline="") as stream:
        if variant == "whole":
            yield from stream.read().splitlines()
        elif variant == "batched":
            while batch := stream.readlines(chunk_chars):
                yield from batch
        elif variant == "buffered":
            yield from stream
        else:
            raise ValueError(f"Unknown variant: {variant}")


def create_input(path, rows):
    checksum = 0
    with open(path, "x", encoding="utf-8", newline="\n") as stream:
        for index in range(rows):
            group, amount = index % 97, index % 100003
            stream.write(f"{index:08d},{group:02d},{amount:06d},caf\u00e9\n")
            value = index * 31 + group * 17 + amount * 13 + 4
            checksum = ((checksum * 1000003) ^ value) & MASK
    if sys.platform == "zos":
        subprocess.run(["chtag", "-t", "-c", "1208", str(path)], check=True, capture_output=True)
    return {"records": rows, "checksum": checksum}


def measure(operation):
    started = time.perf_counter()
    result = operation()
    elapsed = time.perf_counter() - started
    return {"elapsed_seconds": elapsed, "result": result}


def metadata():
    libraries = []
    for path in glob.glob(str(Path(sys.base_prefix) / "lib/python*/lib-dynload/libpyzutil*.so")):
        attributes = subprocess.run(["extattr", path], capture_output=True, text=True)
        libraries.append({"path": str(Path(path).resolve()), "returncode": attributes.returncode,
                          "attributes": attributes.stdout.strip(), "error": attributes.stderr.strip()})
    return {"version": sys.version, "executable": str(Path(sys.executable).resolve()),
            "platform": platform.uname()._asdict(), "pid": os.getpid(), "libpyzutil": libraries,
            "binding_modules": {name: str(Path(module.__file__).resolve())
                                for name, module in sys.modules.items()
                                if name in ("zusf_py", "_zusf_py")},
            "cpu_scope": "current task since creation through one final sample; includes startup and imports; eligible-on-GP is a subset of CP",
            "memory_scope": "separate tracemalloc pass: Python allocations only, excludes native buffers"}


def worker(args):
    if args.bindings:
        sys.path.insert(0, str(Path(args.bindings).resolve()))
    if args.variant in ("binding", "import"):
        importlib.import_module("zusf_py")
    clock = CpuClock(args.timer)
    if args.memory:
        tracemalloc.start()
        result = consume(input_lines(args.input, args.variant, args.chunk_chars))
        _, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        return {"result": result, "python_peak_bytes": peak}
    if args.variant in ("compute", "import", "boundary"):
        phases = [measure(lambda: compute(args.iterations))]
        if args.variant == "boundary":
            phases.append(measure(lambda: consume(input_lines(args.input, "binding", args.chunk_chars))))
        phases.append(measure(lambda: compute(args.iterations)))
        if phases[0]["result"] != phases[-1]["result"]:
            raise RuntimeError("Compute control changed result")
        result = {"phases": phases}
    else:
        result = measure(lambda: consume(input_lines(args.input, args.variant, args.chunk_chars)))
    # A ctypes timer call suppressed subsequent offload in the control probes.
    # Sample once, after all measured work, and report cumulative task CPU.
    counters = clock.read()
    result.update(cpu_delta([0] * 4 if counters is not None else None, counters))
    usage = resource.getrusage(resource.RUSAGE_SELF)
    result["ru_maxrss_reported"] = usage.ru_maxrss or None
    return {**result, "metadata": metadata()}


def invoke_worker(args, variant, path, memory=False):
    command = [sys.executable, str(Path(__file__).resolve()), "worker", "--variant", variant,
               "--input", str(path), "--iterations", str(args.iterations),
               "--chunk-chars", str(args.chunk_chars)]
    for flag, value in (("--timer", args.timer), ("--bindings", args.bindings)):
        if value:
            command.extend((flag, str(Path(value).resolve())))
    if memory:
        command.append("--memory")
    completed = subprocess.run(command, check=True, capture_output=True, text=True, timeout=300)
    return json.loads(completed.stdout)


def summarize(samples):
    result = {}
    for name in ("elapsed_seconds", "gp_seconds", "total_normalized_seconds",
                 "offload_raw_seconds", "eligible_on_gp_seconds", "offload_normalized_percent"):
        values = [sample[name] for sample in samples if sample[name] is not None]
        result[name] = {"median": statistics.median(values), "min": min(values), "max": max(values)} if values else None
    return result


def run_suite(args):
    # Exclusive creation protects earlier evidence; save partial results on failure.
    with open(args.output, "x", encoding="utf-8") as output:
        report = {"schema": 1, "started_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                  "source_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                  "settings": vars(args), "controls": [], "workloads": []}
        try:
            with tempfile.TemporaryDirectory(prefix="znp-ziip-data-", dir=args.work_dir) as directory:
                control_path = Path(directory) / "control.txt"
                control_expected = create_input(control_path, 128)
                for variant in ("compute", "import", "boundary"):
                    invoke_worker(args, variant, control_path)
                    for repeat in range(args.repeats):
                        sample = invoke_worker(args, variant, control_path)
                        if variant == "boundary" and sample["phases"][1]["result"] != control_expected:
                            raise RuntimeError("Native boundary result mismatch")
                        report["controls"].append({"variant": variant, "repeat": repeat, **sample})
                    print(f"Completed {variant} control", file=sys.stderr, flush=True)
                for rows in args.rows:
                    path = Path(directory) / f"records-{rows}.txt"
                    expected = create_input(path, rows)
                    workload = {"rows": rows, "input_bytes": path.stat().st_size, "expected": expected,
                                "samples": [], "memory": {}, "summary": {}}
                    report["workloads"].append(workload)
                    for variant in VARIANTS:
                        warmup = invoke_worker(args, variant, path)
                        if warmup["result"] != expected:
                            raise RuntimeError(f"Warmup mismatch: {variant}")
                    for repeat in range(args.repeats):
                        order = VARIANTS[repeat % len(VARIANTS):] + VARIANTS[:repeat % len(VARIANTS)]
                        for variant in order:
                            sample = invoke_worker(args, variant, path)
                            if sample["result"] != expected:
                                raise RuntimeError(f"Result mismatch: {variant}")
                            workload["samples"].append({"variant": variant, "repeat": repeat, **sample})
                        print(f"Completed rows={rows} repetition={repeat + 1}", file=sys.stderr, flush=True)
                    for variant in VARIANTS:
                        workload["summary"][variant] = summarize([s for s in workload["samples"] if s["variant"] == variant])
                        memory = invoke_worker(args, variant, path, memory=True)
                        if memory["result"] != expected:
                            raise RuntimeError(f"Memory pass mismatch: {variant}")
                        workload["memory"][variant] = memory
            report["complete"] = True
        except Exception as error:
            report["complete"] = False
            report["error"] = str(error)
            raise
        finally:
            json.dump(report, output, indent=2)
            output.write("\n")
    return {"output": args.output, "complete": True}


def positive_integer(value):
    number = int(value)
    if number < 1:
        raise argparse.ArgumentTypeError("must be positive")
    return number


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("suite", "worker"))
    parser.add_argument("--timer", help="Path to the TIMEUSED helper DLL; omitted means no offload accounting")
    parser.add_argument("--bindings", help="Path containing the matching zbind modules")
    parser.add_argument("--iterations", type=positive_integer, default=10_000_000)
    parser.add_argument("--chunk-chars", type=positive_integer, default=65536)
    parser.add_argument("--variant", choices=(*VARIANTS, "compute", "import", "boundary"), default="compute")
    parser.add_argument("--input")
    parser.add_argument("--memory", action="store_true")
    parser.add_argument("--rows", type=positive_integer, nargs="*", default=[32768, 262144])
    parser.add_argument("--repeats", type=positive_integer, default=5)
    parser.add_argument("--output")
    parser.add_argument("--work-dir")
    args = parser.parse_args()
    if args.command == "suite" and not args.output:
        parser.error("suite requires --output")
    if args.command == "worker" and args.variant not in ("compute", "import") and not args.input:
        parser.error("this worker requires --input")
    if args.memory and args.variant not in VARIANTS:
        parser.error("--memory requires a file variant")
    print(json.dumps(run_suite(args) if args.command == "suite" else worker(args)))


if __name__ == "__main__":
    main()
