#!/usr/bin/env python3
from distutils.core import setup, Extension
import os
import os.path

lib_dir = os.path.abspath('../c/build-out')
chdsect = os.path.abspath('../c/chdsect')
ztype = os.path.abspath('../c')

zds_py_module = Extension('_zds_py',
                           sources=['zds_py_wrap.cxx', 'zds_py.cpp'],
                           language='c++',
                           extra_objects=['../c/build-out/zdsm.o', "../c/build-out/zutm.o", "../c/build-out/zutm31.o", '../c/build-out/xlclang/zds.o', '../c/build-out/xlclang/zut.o', '../c/build-out/zam.o'],
                           include_dirs=[chdsect, ztype],
                           )

setup(name = 'zusf_py',
       version = '0.1',
       author      = "SWIG Docs",
       description = """Simple swig example for zds list""",
       ext_modules = [zds_py_module],
       py_modules = ["zusf_py"],
       )