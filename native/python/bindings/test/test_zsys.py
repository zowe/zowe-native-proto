"""System binding contracts that do not require system service authority."""

import gc
import inspect
from pathlib import Path
import sys

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import zsys_py as system


@pytest.mark.parametrize("function", [system.display_symbol, system.list_subsystems])
@pytest.mark.parametrize("value", [None, 1, True, b"SYS", [], object()])
def test_name_types(function, value):
    with pytest.raises(TypeError):
        function(value)


@pytest.mark.parametrize("symbol", ["", "&SYSNAME", "SYS.NAME", "SYS\x00NAME", "a" * 127, "caf\u00e9", "a b"])
def test_invalid_symbols(symbol):
    with pytest.raises(ValueError):
        system.display_symbol(symbol)


@pytest.mark.parametrize("pattern", ["", "JES**", "A\x00B", "a b", "caf\u00e9", "[A]"])
def test_invalid_subsystem_filters(pattern):
    with pytest.raises(ValueError):
        system.list_subsystems(pattern)


@pytest.mark.parametrize("field", ["date", "time"])
@pytest.mark.parametrize("value", [1, True, b"2024-01-01", [], object()])
def test_syslog_text_types(field, value):
    with pytest.raises(TypeError):
        system.view_syslog(**{field: value})


@pytest.mark.parametrize("field", ["seconds_ago", "max_lines"])
@pytest.mark.parametrize("value", [True, False, 1.0, "1", b"1", [], object()])
def test_syslog_integer_types(field, value):
    with pytest.raises(TypeError):
        system.view_syslog(**{field: value})


@pytest.mark.parametrize("field,value", [
    ("seconds_ago", 0), ("seconds_ago", -1), ("seconds_ago", 86401),
    ("max_lines", 0), ("max_lines", -1), ("max_lines", 10001),
    ("seconds_ago", 2**100), ("max_lines", -(2**100)),
])
def test_syslog_integer_bounds(field, value):
    with pytest.raises(ValueError):
        system.view_syslog(**{field: value})


@pytest.mark.parametrize("date", [
    "", "2024-1-01", "2024-01-1", "2024-01-01suffix", "2024-01-01\x00",
    "2023-02-29", "1900-02-29", "2100-02-29", "2024-04-31", "2024-00-01",
    "1899-12-31", "2186-01-01", "0000-01-01", "2024-13-01", "2024-01-00",
])
def test_invalid_syslog_dates(date):
    with pytest.raises(ValueError):
        system.view_syslog(date=date)


@pytest.mark.parametrize("time", [
    "", "1:02:03", "01:2:03", "01:02:3", "24:00:00", "12:60:00", "12:00:60",
    "12:00:00.01", "12:00:00suffix", "12:00:00\x00", "12:00", "12:00:00:00",
])
def test_invalid_syslog_times(time):
    with pytest.raises(ValueError):
        system.view_syslog(time=time)


@pytest.mark.parametrize("absolute", [{"date": "2024-01-01"}, {"time": "12:00:00"}, {"date": ""}])
def test_relative_absolute_exclusion(absolute):
    with pytest.raises(ValueError, match="mutually exclusive"):
        system.view_syslog(seconds_ago=1, **absolute)


@pytest.mark.parametrize("options", [
    {}, {"date": None, "time": None, "seconds_ago": None, "max_lines": None},
    {"date": "2000-02-29"}, {"date": "2024-02-29", "time": "23:59:59"},
    {"seconds_ago": 1, "max_lines": 1}, {"seconds_ago": 86400, "max_lines": 10000},
])
def test_public_options_preserve_presence(monkeypatch, options):
    sentinel = object()
    captured = []

    def capture(query):
        captured.append((query.date, query.time, query.seconds_ago, query.max_lines, query.has_max_lines))
        return sentinel

    monkeypatch.setattr(system, "_view_syslog", capture)
    assert system.view_syslog(**options) is sentinel
    assert captured == [(options.get("date") or "", options.get("time") or "",
                         options.get("seconds_ago") if options.get("seconds_ago") is not None else -1,
                         options.get("max_lines") or 0, options.get("max_lines") is not None)]


def test_syslog_public_signature():
    parameters = inspect.signature(system.view_syslog).parameters
    assert list(parameters) == ["date", "time", "seconds_ago", "max_lines"]
    assert all(p.kind == inspect.Parameter.KEYWORD_ONLY and p.default is None for p in parameters.values())
    with pytest.raises(TypeError):
        system.view_syslog("2024-01-01")
    with pytest.raises(TypeError):
        system.view_syslog(unexpected=1)


@pytest.mark.parametrize("vector_type,item_type", [
    (system.ApfEntryVector, system.ApfEntry),
    (system.LinklistEntryVector, system.LinklistEntry),
])
@pytest.mark.parametrize("access", ["index", "iteration", "slice"])
def test_entry_keeps_result_storage(vector_type, item_type, access):
    def create():
        vector = vector_type()
        item = item_type()
        item.dsname = "SYS1.LINKLIB"
        item.volume = "VOL001"
        vector.push_back(item)
        return vector

    if access == "index":
        entry = create()[0]
    elif access == "iteration":
        entry = next(iter(create()))
    else:
        entry = create()[:][0]
    gc.collect()
    assert entry.dsname == "SYS1.LINKLIB"
    assert entry.volume == "VOL001"


def test_result_and_exception_types():
    result = system.SyslogResult()
    assert result.data == result.start_date == result.start_time == result.end_date == result.end_time == ""
    assert result.returned_lines == 0
    assert result.has_more is False
    assert issubclass(system.ZsysError, RuntimeError)
    with pytest.raises(RuntimeError):
        raise system.ZsysError("sample")
