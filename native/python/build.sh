set -e
# PREREQUISITE: SWIG libraries must be installed at /u/users/UTILS/swig_lib
# Contact your mainframe administrator if this path doesn't exist
export _BPXK_AUTOCVT=ON
export _CEE_RUNOPTS="FILETAG(AUTOCVT,AUTOTAG) POSIX(ON)"
export SWIG_LIB=/u/users/UTILS/swig_lib
export _CC_CCMODE=1
export _CXX_CCMODE=1
export _C89_CCMODE=1
export _CC_EXTRA_ARGS=1
export _CXX_EXTRA_ARGS=1
export _C89_EXTRA_ARGS=1

swig -python -c++ zusf_py.i
python setup.py build_ext --inplace
