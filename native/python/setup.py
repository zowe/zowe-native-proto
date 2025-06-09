#!/usr/bin/env python3
from distutils.core import setup, Extension
import os
import os.path


lib_dir = os.path.abspath('../c/build-out')
include_dir = os.path.abspath('../c/chdsect')

zusf_py_module = Extension('_zusf_py',
                           sources=['zusf_py_wrap.cxx', 'zusf_py.cpp', "../c/zusf.cpp", "../c/zut.cpp"],
                           language='c++',
                           extra_compile_args=['-q64', '-qascii'],
                           include_dirs=[include_dir],
                           libraries=['zut'],
                           library_dirs=[lib_dir],
                           extra_link_args=[
                               '-q64', 
                               '-G', 
                               '-bexpall', 
                               '-brtl',
                           ],
                           )

setup(name = 'zusf_py',
       version = '0.1',
       author      = "SWIG Docs",
       description = """Simple swig example for uss list""",
       ext_modules = [zusf_py_module],
       py_modules = ["zusf_py"],
       )