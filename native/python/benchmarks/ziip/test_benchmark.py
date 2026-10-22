"""Correctness and accounting checks independent of a z/OS installation."""

# SPDX-License-Identifier: EPL-2.0
# Copyright Contributors to the Zowe Project.

from pathlib import Path
import tempfile
import unittest

import benchmark


class BenchmarkTests(unittest.TestCase):
    def test_text_variants_preserve_records_and_unicode(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "records.txt"
            expected = benchmark.create_input(path, 1001)
            for variant in ("buffered", "batched", "whole"):
                for chunk_chars in (1, 7, 65536):
                    with self.subTest(variant=variant, chunk_chars=chunk_chars):
                        actual = benchmark.consume(benchmark.input_lines(path, variant, chunk_chars))
                        self.assertEqual(actual, expected)

    def test_last_record_without_newline(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "records.txt"
            path.write_text("00000000,00,000001,caf\u00e9", encoding="utf-8")
            for variant in ("buffered", "batched", "whole"):
                self.assertEqual(benchmark.consume(benchmark.input_lines(path, variant, 1)),
                                 {"records": 1, "checksum": 17})

    def test_percentage_uses_normalized_offload(self):
        unit = benchmark.TOD_PER_SECOND
        delta = benchmark.cpu_delta([unit] * 4, [11 * unit, 4 * unit, 3 * unit, 2 * unit])
        self.assertEqual(delta["total_normalized_seconds"], 10)
        self.assertEqual(delta["gp_seconds"], 3)
        self.assertEqual(delta["offload_raw_seconds"], 2)
        self.assertEqual(delta["eligible_on_gp_seconds"], 1)
        self.assertEqual(delta["offload_normalized_percent"], 70)

    def test_inconsistent_counters_are_rejected(self):
        for after in ([10000, 20000, 0, 0], [20000, 10000, 0, 20000], [-1, 0, 0, 0]):
            with self.assertRaises(RuntimeError):
                benchmark.cpu_delta([0] * 4, after)

    def test_sub_microsecond_field_skew_preserves_raw_counters(self):
        ticks = [752801946, 752802002, 0, 0]
        delta = benchmark.cpu_delta([0] * 4, ticks)
        self.assertEqual(delta["raw_delta_ticks"], ticks)
        self.assertEqual(delta["offload_normalized_percent"], 0)

    def test_unavailable_accounting_is_explicit(self):
        self.assertTrue(all(value is None for value in benchmark.cpu_delta(None, None).values()))


if __name__ == "__main__":
    unittest.main()
