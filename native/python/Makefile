# Makefile for building Python bindings for zowe-native-proto

# Default target
all: swig-extenders build

# Environment variables for z/OS compatibility
_SWIG_ENV= \
	export _BPXK_AUTOCVT=ON; \
	export _CEE_RUNOPTS="FILETAG(AUTOCVT,AUTOTAG) POSIX(ON)" ; \
	export _CC_CCMODE=1; \
	export _CXX_CCMODE=1; \
	export _C89_CCMODE=1; \
	export _CC_EXTRA_ARGS=1; \
	export _CXX_EXTRA_ARGS=1; \
	export _C89_EXTRA_ARGS=1;

# Determine SWIG path and directory
_SWIG_LIB=export SWIG_LIB=$${SWIG_LIB:-$$(dirname $$(type swig | awk '{print $$NF}'))/swig_lib};
SWIG=$(_SWIG_ENV) $(_SWIG_LIB) swig

# Generate SWIG wrappers
swig-wrappers:
	@echo '\nGenerating SWIG wrappers'
	$(SWIG) -python -c++ zusf_py.i
	$(SWIG) -python -c++ zjb_py.i
	$(SWIG) -c++ -python zds_py.i

# Build Python extensions
build: swig-wrappers
	@echo '\nBuilding Python extensions'
	$(_SWIG_ENV) python setup.py build_ext

# Run SWIG extenders in the C directory
swig-extenders:
	@echo '\nBuilding SWIG extenders'
	$(MAKE) -c ../c swig-extenders

# Clean generated files
clean:
	@echo '\nCleaning generated files'
	rm -f *.cxx
	rm -f *.x
	rm -f *.o
	rm -f *.so
	rm -f *.py[co]
	rm -rf build/
	rm -rf ../c/build-out/swig

.PHONY: all swig-wrappers swig-extenders build clean
