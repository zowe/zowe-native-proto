#!/usr/bin/env python3

"""
setup.py file for SWIG example
"""

from distutils.core import setup, Extension
import os

C_PATH = "../c"
chdsect = os.path.abspath(f"{C_PATH}/chdsect")
ztype = os.path.abspath(C_PATH)
build_out_path = f"{C_PATH}/build-out"
swig_build_path = f"{build_out_path}/swig"

zusf_py_module = Extension("_zusf_py",
                           sources=["zusf_py_wrap.cxx", "zusf_py.cpp", f"{C_PATH}/zusf.cpp", f"{C_PATH}/zut.cpp"],
                           language="c++",
                           include_dirs=[chdsect],
                           libraries=["zut"],
                           library_dirs=[build_out_path],
                           )

zds_py_module = Extension("_zds_py",
                           sources=["zds_py_wrap.cxx", "zds_py.cpp"],
                           language="c++",
                           extra_objects=[
                               f"{build_out_path}/zdsm.o", 
                               f"{build_out_path}/zutm.o", 
                               f"{build_out_path}/zam.o",
                               f"{build_out_path}/zutm31.o", 
                               f"{swig_build_path}/zds.o", 
                               f"{swig_build_path}/zut.o", 
                           ],
                           include_dirs=[chdsect, ztype],
                           )

zjb_py_module = Extension("_zjb_py",
                           sources=["zjb_py_wrap.cxx", "zjb_py.cpp"],
                           language="c++",
                           extra_objects=[
                               f"{build_out_path}/zjbm.o", 
                               f"{build_out_path}/zutm.o", 
                               f"{build_out_path}/zutm31.o", 
                               f"{build_out_path}/zam.o",
                               f"{build_out_path}/zdsm.o", 
                               f"{build_out_path}/zjbm31.o", 
                               f"{build_out_path}/zssi31.o",
                               f"{swig_build_path}/zjb.o", 
                               f"{swig_build_path}/zut.o", 
                               f"{swig_build_path}/zds.o", 
                           ],
                           include_dirs=[chdsect, ztype],
                           )

setup(name = "zbind",
       description = """Simple swig example""",
       ext_modules = [zusf_py_module, zds_py_module, zjb_py_module],
       py_modules = ["zusf_py", "zds_py", "zjb_py"],
       )