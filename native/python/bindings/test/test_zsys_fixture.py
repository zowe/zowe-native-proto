"""Run only against the isolated service-substitute build, never the live extension."""

from pathlib import Path
import sys

import pytest
import zsys_py as system

if "fixture-lib" not in str(Path(system.__file__).resolve()):
    pytest.skip("Requires make test-zsys-fixture", allow_module_level=True)


def test_utf8_symbol_and_embedded_nul():
    assert system.display_symbol("SYSNAME") == "Caf\u00e9"
    assert system.display_symbol("NUL") == "A\x00B\n"


def test_symbol_failure_preserves_rc_and_fallback():
    with pytest.raises(system.ZsysError) as captured:
        system.display_symbol("FAIL")
    error = captured.value
    assert error.operation == "display_symbol"
    assert error.rc == 42
    assert str(error) == "display_symbol failed (rc=42)"
    assert error.service_name == ""
    assert error.service_rc == 0


def test_bounded_diagnostic_buffers_and_fields():
    with pytest.raises(system.ZsysError) as captured:
        system.list_proclib()
    error = captured.value
    assert str(error) == "X" * 256
    assert error.service_name == "S" * 24
    assert (error.detail_rc, error.service_rc, error.service_rsn, error.service_rsn_secondary) == (11, 12, 13, 14)
    references = sys.getrefcount(error.service_name)
    assert references == 2


def test_diagnostic_assignment_failure(monkeypatch):
    def reject(self, name, value):
        raise MemoryError("attribute assignment failed")
    monkeypatch.setattr(system.ZsysError, "__setattr__", reject)
    with pytest.raises(MemoryError, match="attribute assignment failed"):
        system.list_proclib()


def test_order_duplicates_padding_and_default_filter():
    assert list(system.list_parmlib()) == ["SYS1.PARMLIB", "SYS1.PARMLIB"]
    assert list(system.list_subsystems()) == ["*"]
    assert list(system.list_subsystems(filter="jes*")) == ["JES*"]
    assert [(entry.dsname, entry.volume) for entry in system.list_apf()] == [("SYS1.LINKLIB", "VOL001"), ("SYS1.OTHER", "")]
    assert [entry.apf for entry in system.list_linklist()] == [True, False]


def test_empty_syslog_result():
    result = system.view_syslog(max_lines=1)
    assert result.data == result.end_date == result.end_time == ""
    assert result.returned_lines == 0
    assert result.has_more is False
    assert result.start_date == "2024-02-29"
    assert result.start_time == "12:00:00"


def test_partial_syslog_failure_is_not_returned_as_success():
    with pytest.raises(system.ZsysError, match="view_syslog failed") as captured:
        system.view_syslog(max_lines=2)
    assert captured.value.rc == 8


def test_syslog_text_and_metadata():
    result = system.view_syslog(date="2024-02-29", time="01:02:03", max_lines=3)
    assert result.data == "Caf\u00e9\x00 tail\n"
    assert result.returned_lines == 1
    assert result.has_more is True
    assert result.end_date == result.start_date == "2024-02-29"
    assert result.end_time == result.start_time == "01:02:03"
