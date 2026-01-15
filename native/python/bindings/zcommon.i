%begin %{
#define Py_LIMITED_API 0x030A0000
%}

%include "exception.i"

%exception {
    try {
        $action
    } catch (const std::runtime_error& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (...) {
        SWIG_exception(SWIG_RuntimeError, "Unknown exception");
    }
}

%include "std_string.i"
%include "std_vector.i"