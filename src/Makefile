.PHONY: cudd nusmv pynusmv


SF=$(shell getconf LONG_BIT)
ifeq ($(SF), 64)
SF=_64bit
else
SF=
endif

SYS=$(shell gcc -dumpmachine)
ifneq (,$(findstring darwin,$(SYS)))
OS=_os_x
else ifneq (,$(findstring mingw,$(SYS)))
OS=_mingw
else
OS=
endif

ifndef PYTHON
PYTHON=python3
endif

ifndef SWIG
ifneq ($(wildcard ../dep/swig-2.0.6/swig),)
SWIG=../dep/swig-2.0.6/swig
endif
endif

ifndef SWIG_LIB
ifneq ($(wildcard ../dep/swig-2.0.6/swig),)
export SWIG_LIB=../dep/swig-2.0.6/Lib/
endif
endif



all: cudd nusmv pynusmv


cudd:
	$(get-sf)
	$(get-os)
	make -C cudd-2.4.1.1 -f Makefile$(OS)$(SF)


nusmv: cudd
	@if [ ! -f nusmv/NuSMV ]; then \
		cd nusmv; \
		if [ ! -f Makefile ]; then \
			./configure CFLAGS="-g -O2 -fPIC"; \
		fi; \
		make; \
	fi


pynusmv: nusmv
ifneq ($(SWIG),)
	@if [ ! -f lib/libnusmv.so ]; then ./build_libnusmv.sh; fi; \
	$(PYTHON) setup.py build_ext --inplace --swig=$(SWIG)
else
ifneq ($(wildcard ../dep/swig-2.0.6/swig),)
	@if [ ! -f lib/libnusmv.so ]; then ./build_libnusmv.sh; fi; \
	$(PYTHON) setup.py build_ext --inplace --swig=../dep/swig-2.0.6/swig
else
	@if [ ! -f lib/libnusmv.so ]; then ./build_libnusmv.sh; fi; \
	$(PYTHON) setup.py build_ext --inplace
endif
endif
