#!/bin/bash
set -e

SWIG_PATH=$(type -p swig || true)
if [ -z "$SWIG_PATH" ]; then
  echo "ERROR: swig not found."
  echo "Please install swig and re-run this script."
  exit 1
fi
SWIG_DIR=$(dirname "$SWIG_PATH")
# Use SWIG_LIB if it is set, otherwise assume it is in SWIG_DIR
export SWIG_LIB=${SWIG_LIB:-$SWIG_DIR/swig_lib}

export _BPXK_AUTOCVT=ON
export _CEE_RUNOPTS="FILETAG(AUTOCVT,AUTOTAG) POSIX(ON)"
export _CC_CCMODE=1
export _CXX_CCMODE=1
export _C89_CCMODE=1
export _CC_EXTRA_ARGS=1
export _CXX_EXTRA_ARGS=1
export _C89_EXTRA_ARGS=1

swig -python -c++ zusf_py.i
swig -python -c++ zjb_py.i
swig -c++ -python zds_py.i
python setup.py build_ext --inplace
