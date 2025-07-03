#!/usr/bin/env python3

"""
setup.py file for SWIG example
"""

from distutils.core import setup, Extension
import os

C_PATH = '../c'
chdsect = os.path.abspath(f'{C_PATH}/chdsect')
ztype = os.path.abspath(C_PATH)

zusf_py_module = Extension('_zusf_py',
	sources=['zusf_py_wrap.cxx', 'zusf_py.cpp', f"{C_PATH}/zusf.cpp", f"{C_PATH}/zut.cpp"],
	language='c++',
	include_dirs=[chdsect],
    define_macros=[('SWIG', '1')],
	libraries=['zut'],
	library_dirs=[f'{C_PATH}/build-out'],
	)

zds_py_module = Extension('_zds_py',
	sources=['zds_py_wrap.cxx', 'zds_py.cpp'],
	language='c++',
	define_macros=[('SWIG', '1')],
	extra_objects=[
		f'{C_PATH}/build-out/zdsm.o', 
		f"{C_PATH}/build-out/zutm.o", 
		f"{C_PATH}/build-out/zutm31.o", 
		f'{C_PATH}/build-out/xlclang/zds.o', 
		f'{C_PATH}/build-out/xlclang/zut.o', 
		f'{C_PATH}/build-out/zam.o'
	],
	include_dirs=[chdsect, ztype],
	)

zjb_py_module = Extension('_zjb_py',
	sources=['zjb_py_wrap.cxx', 'zjb_py.cpp'],
	language='c++',
    define_macros=[('SWIG', '1')],
	extra_objects=[
		f'{C_PATH}/build-out/zjbm.o', 
		f"{C_PATH}/build-out/zutm.o", 
		f"{C_PATH}/build-out/zutm31.o", 
		f'{C_PATH}/build-out/xlclang/zjb.o', 
		f'{C_PATH}/build-out/xlclang/zut.o', 
		f'{C_PATH}/build-out/zam.o',
		f'{C_PATH}/build-out/xlclang/zds.o', 
		f'{C_PATH}/build-out/zdsm.o', 
		f'{C_PATH}/build-out/zjbm31.o', 
		f'{C_PATH}/build-out/zssi31.o'
	],
	include_dirs=[chdsect, ztype],
	)

setup(name = 'zbind',
	description = """Simple swig example""",
	ext_modules = [zusf_py_module, zds_py_module, zjb_py_module],
	py_modules = ["zusf_py", "zds_py", "zjb_py"],
	)
