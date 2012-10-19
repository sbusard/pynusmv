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

ifndef PYTHON3
PYTHON3=python3
endif

ifndef SWIG
ifneq ($(wildcard ../dep/swig-2.0.6/swig),)
SWIG=../dep/swig-2.0.6/swig
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
		if [! -f Makefile ]; then \
			./configure; \
		fi; \
		make; \
	fi


pynusmv: nusmv
ifndef SWIG
	./build_libnusmv.sh
	$(PYTHON3) setup.py build_ext --inplace
else
	./build_libnusmv.sh
	$(PYTHON3) setup.py build_ext --inplace --swig=$(SWIG)
endif