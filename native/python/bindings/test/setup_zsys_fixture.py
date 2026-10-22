"""Build the real adapter/wrapper against EBCDIC service substitutes, outside the live bindings."""

from pathlib import Path
import os
import subprocess

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

HERE = Path(__file__).resolve().parent
BINDINGS = HERE.parent
NATIVE = BINDINGS.parent.parent / "c"


class FixtureBuild(build_ext):
    def build_extension(self, extension):
        compile_source = self.compiler._compile

        def mixed_compile(obj, src, ext, cc_args, extra_postargs, pp_opts):
            if Path(src).name == "zsys_fixture.cpp":
                extra_postargs = [*extra_postargs, "-fzos-le-char-mode=ebcdic"]
            return compile_source(obj, src, ext, cc_args, extra_postargs, pp_opts)

        self.compiler._compile = mixed_compile
        try:
            super().build_extension(extension)
        finally:
            self.compiler._compile = compile_source
        target = Path(self.build_lib) / "zsys_py.py"
        with target.open("wb") as output:
            subprocess.run(["iconv", "-f", "IBM-1047", "-t", "UTF-8", str(BINDINGS / "zsys_py.py")],
                           stdout=output, check=True)
        subprocess.run(["chtag", "-t", "-c", "UTF-8", str(target)], check=True)


os.environ.setdefault("CC", "ibm-clang64")
os.environ.setdefault("CXX", "ibm-clang++64")
setup(name="zsys-adapter-tests", version="0", cmdclass={"build_ext": FixtureBuild},
      ext_modules=[Extension("_zsys_py",
          sources=[str(BINDINGS / "zsys_py.cpp"), str(BINDINGS / "zsys_py_wrap.cxx"), str(HERE / "zsys_fixture.cpp")],
          include_dirs=[str(BINDINGS), str(NATIVE), str(NATIVE / "chdsect")],
          define_macros=[("SWIG", "1"), ("SWIG_TYPE_TABLE", "_zsys_py"), ("Py_LIMITED_API", "0x030B0000")],
          extra_compile_args=["-D_EXT", "-D_OPEN_SYS_FILE_EXT=1"], language="c++", py_limited_api=True)])
