%module zsys_py

%{
#include "zsys_py.hpp"
%}

%feature("compactdefaultargs");
%feature("kwargs");
%include "exception.i"
%include "std_string.i"
%include "std_vector.i"

%ignore ZsysError;
%rename(_SyslogQuery) ZsysQuery;
%rename(_view_syslog) view_syslog;

%typemap(default) const std::string &filter (std::string default_filter) {
  default_filter = "*";
  $1 = &default_filter;
}

%typemap(in) const std::string & (std::string value) {
  if (!PyUnicode_Check($input)) {
    SWIG_exception_fail(SWIG_TypeError, "expected str for argument $argnum");
  }
  Py_ssize_t length = 0;
  const char *data = PyUnicode_AsUTF8AndSize($input, &length);
  if (!data) SWIG_fail;
  value.assign(data, static_cast<size_t>(length));
  $1 = &value;
}
%typemap(freearg) const std::string & ""

%{
static PyObject *g_zsys_error = nullptr;

static bool zsys_set_attr(PyObject *exception, const char *name, PyObject *value)
{
  if (!value)
    return false;
  const int rc = PyObject_SetAttrString(exception, name, value);
  Py_DECREF(value);
  return rc == 0;
}
%}

%init %{
  g_zsys_error = PyErr_NewException("zsys_py.ZsysError", PyExc_RuntimeError, nullptr);
  if (!g_zsys_error) return nullptr;
  if (PyModule_AddObject(m, "ZsysError", g_zsys_error) != 0) {
    Py_DECREF(g_zsys_error);
    return nullptr;
  }
  Py_INCREF(g_zsys_error);
%}

%exception {
  try {
    $action
  } catch (const ZsysError &error) {
    PyObject *exception = PyObject_CallFunction(g_zsys_error, "s", error.what());
    if (exception) {
      if (zsys_set_attr(exception, "operation", PyUnicode_FromString(error.operation.c_str())) &&
          zsys_set_attr(exception, "rc", PyLong_FromLong(error.rc)) &&
          zsys_set_attr(exception, "service_name", PyUnicode_FromString(error.service_name.c_str())) &&
          zsys_set_attr(exception, "detail_rc", PyLong_FromLong(error.detail_rc)) &&
          zsys_set_attr(exception, "service_rc", PyLong_FromLong(error.service_rc)) &&
          zsys_set_attr(exception, "service_rsn", PyLong_FromLong(error.service_rsn)) &&
          zsys_set_attr(exception, "service_rsn_secondary", PyLong_FromLong(error.service_rsn_secondary))) {
        PyErr_SetObject(g_zsys_error, exception);
      }
      Py_DECREF(exception);
    }
    SWIG_fail;
  } catch (const std::bad_alloc &) {
    PyErr_NoMemory();
    SWIG_fail;
  } catch (const std::invalid_argument &error) {
    SWIG_exception(SWIG_ValueError, error.what());
  } catch (const std::exception &error) {
    SWIG_exception(SWIG_RuntimeError, error.what());
  } catch (...) {
    SWIG_exception(SWIG_RuntimeError, "Unknown system binding exception");
  }
}

%feature("docstring") display_symbol "Resolve a bare system symbol name, such as SYSNAME. Returns UTF-8 text.";
%feature("docstring") list_parmlib "Return PARMLIB dataset names in native search order.";
%feature("docstring") list_proclib "Return JES PROCLIB dataset names in native order.";
%feature("docstring") list_subsystems "Return subsystem names matching the native filter (1-4 ASCII characters).";
%feature("docstring") list_apf "Return APF entries with dsname and volume fields.";
%feature("docstring") list_linklist "Return active LNKLST entries with dsname, volume, and apf fields.";

%include "zsys_py.hpp"
%template(StringVector) std::vector<std::string>;
%template(ApfEntryVector) std::vector<ApfEntry>;
%template(LinklistEntryVector) std::vector<LinklistEntry>;

%pythoncode %{
ZsysError = _zsys_py.ZsysError

def view_syslog(*, date=None, time=None, seconds_ago=None, max_lines=None):
    """Read bounded SYSLOG records using local z/OS time.

    Defaults to the last 30 seconds and 10,000 lines. Absolute date/time
    queries default to 100 lines. has_more means the limit was reached;
    end timestamps have second precision and are not lossless cursors.
    """
    import datetime
    import re

    for name, value in (("date", date), ("time", time)):
        if value is not None and not isinstance(value, str):
            raise TypeError(f"{name} must be str or None")
    for name, value, maximum in (("seconds_ago", seconds_ago, 86400), ("max_lines", max_lines, 10000)):
        if value is not None:
            if isinstance(value, bool) or not isinstance(value, int):
                raise TypeError(f"{name} must be int or None")
            if not 1 <= value <= maximum:
                raise ValueError(f"{name} must be between 1 and {maximum}")
    if seconds_ago is not None and (date is not None or time is not None):
        raise ValueError("seconds_ago is mutually exclusive with date and time")
    if date is not None:
        if re.fullmatch(r"[0-9]{4}-[0-9]{2}-[0-9]{2}", date) is None:
            raise ValueError("date must use YYYY-MM-DD")
        parsed = datetime.date.fromisoformat(date)
        if not 1900 <= parsed.year <= 2185:
            raise ValueError("date year must be between 1900 and 2185")
    if time is not None:
        if re.fullmatch(r"[0-9]{2}:[0-9]{2}:[0-9]{2}", time) is None:
            raise ValueError("time must use HH:MM:SS")
        datetime.time.fromisoformat(time)
    query = _SyslogQuery()
    if date is not None:
        query.date = date
    if time is not None:
        query.time = time
    if seconds_ago is not None:
        query.seconds_ago = seconds_ago
    if max_lines is not None:
        query.max_lines = max_lines
        query.has_max_lines = True
    return _view_syslog(query)

# Indexing returns a borrowed C++ element; keep the vector alive with it.
def _own_indexed_entry(getter):
    def get(self, index):
        entry = getter(self, index)
        if isinstance(entry, (ApfEntry, LinklistEntry)):
            entry._zsys_owner = self
        return entry
    return get

def _iterate_entries(self):
    for index in range(len(self)):
        yield self[index]

for _vector in (ApfEntryVector, LinklistEntryVector):
    _vector.__getitem__ = _own_indexed_entry(_vector.__getitem__)
    _vector.__iter__ = _iterate_entries
del _vector
%}
