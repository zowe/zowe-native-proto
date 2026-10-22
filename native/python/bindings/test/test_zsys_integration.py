"""Read-only system service checks; authorization failures remain visible."""

import datetime
import gc
import json
import os
from pathlib import Path
import subprocess
import sys

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import zsys_py as system


def test_symbol_case_and_native_unresolved_result():
    value = system.display_symbol("SYSNAME")
    assert isinstance(value, str) and value
    assert system.display_symbol(symbol="sysname") == value
    try:
        unknown = system.display_symbol("ZSYSUNKNOWN123")
        assert isinstance(unknown, str)
    except system.ZsysError as error:
        assert error.operation == "display_symbol"
        assert error.rc != 0
        assert str(error)


@pytest.mark.parametrize("query", [system.list_parmlib, system.list_proclib, system.list_subsystems])
def test_name_lists(query):
    values = query()
    assert len(values) == len(list(values))
    for name in values:
        assert isinstance(name, str) and name
        assert name == name.rstrip(" ")
        assert "\x00" not in name
    del values
    gc.collect()
    assert list(query()) == list(query())


def test_subsystem_default_and_keywords():
    assert list(system.list_subsystems()) == list(system.list_subsystems(filter="*"))
    assert list(system.list_subsystems("jes*")) == list(system.list_subsystems(filter="JES*"))


@pytest.mark.parametrize("query,fields", [
    (system.list_apf, ("dsname", "volume")),
    (system.list_linklist, ("dsname", "volume")),
])
def test_structured_lists(query, fields):
    entries = list(query())
    gc.collect()
    for entry in entries:
        for field in fields:
            value = getattr(entry, field)
            assert isinstance(value, str)
            assert value == value.rstrip(" ")
        if hasattr(entry, "apf"):
            assert isinstance(entry.apf, bool)


@pytest.mark.parametrize("options", [
    {}, {"seconds_ago": 300}, {"date": datetime.date.today().isoformat()},
    {"time": "00:00:00"}, {"date": datetime.date.today().isoformat(), "time": "00:00:00"},
])
def test_syslog_modes(options):
    result = system.view_syslog(max_lines=2, **options)
    datetime.date.fromisoformat(result.start_date)
    datetime.time.fromisoformat(result.start_time)
    assert 0 <= result.returned_lines <= 2
    assert isinstance(result.has_more, bool)
    assert isinstance(result.data, str)
    if result.returned_lines:
        assert result.data.endswith("\n")
        datetime.date.fromisoformat(result.end_date)
        datetime.time.fromisoformat(result.end_time)
    else:
        assert result.data == result.end_date == result.end_time == ""


def _native_range_failure():
    try:
        system.view_syslog(date="2185-12-31", time="23:59:59", max_lines=1)
    except system.ZsysError as error:
        return error
    pytest.skip("This system supports the extreme CONVTOD date; no native failure to inspect")


def test_native_error_diagnostics():
    error = _native_range_failure()
    assert error.operation == "view_syslog"
    assert error.rc != 0
    assert isinstance(error.service_name, str)
    assert str(error)
    for field in ("rc", "detail_rc", "service_rc", "service_rsn", "service_rsn_secondary"):
        assert isinstance(getattr(error, field), int)
    references = sys.getrefcount(error.operation)
    assert references == 2


def test_native_error_attribute_failure(monkeypatch):
    _native_range_failure()

    def reject_attribute(self, name, value):
        raise MemoryError("diagnostic assignment failed")

    monkeypatch.setattr(system.ZsysError, "__setattr__", reject_attribute)
    with pytest.raises(MemoryError, match="diagnostic assignment failed"):
        system.view_syslog(date="2185-12-31", time="23:59:59", max_lines=1)


def test_repeated_syslog_reads_release_resources():
    for _ in range(20):
        result = system.view_syslog(seconds_ago=30, max_lines=1)
        assert result.returned_lines in (0, 1)


def test_syslog_defaults_are_independent_of_process_timezone():
    code = "import json,zsys_py; r=zsys_py.view_syslog(seconds_ago=30,max_lines=1); print(json.dumps([r.start_date,r.start_time]))"
    starts = []
    for timezone in ("UTC0", "UTC12"):
        env = dict(os.environ, TZ=timezone)
        raw = subprocess.check_output([sys.executable, "-c", code], env=env, cwd=Path(system.__file__).parent)
        date, time = json.loads(raw.decode("utf-8" if raw.startswith(b"[") else "cp1047"))
        starts.append(datetime.datetime.fromisoformat(date + "T" + time))
    assert abs((starts[1] - starts[0]).total_seconds()) < 30


def test_cli_parity():
    executable = os.environ.get("ZSYS_TEST_ZO")
    if not executable:
        pytest.skip("Set ZSYS_TEST_ZO to compare with the freshly built CLI")
    for command, query in (("list-parmlib", system.list_parmlib), ("list-proclib", system.list_proclib),
                           ("list-subsystems", system.list_subsystems)):
        result = subprocess.run([executable, "system", command], capture_output=True, check=True)
        try:
            text = result.stdout.decode("utf-8")
        except UnicodeDecodeError:
            text = result.stdout.decode("cp1047")
        assert [line.rstrip(" ") for line in text.splitlines() if line] == list(query())
