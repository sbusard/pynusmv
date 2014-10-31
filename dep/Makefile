.PHONY: swig pyparsing

all: swig pyparsing

PYTHON:=python3
SWIG:=$(shell which swig)
PYPARSING:=$(shell $(PYTHON) -c "import pyparsing" 1>&2 2> /dev/null; echo $$?)

swig:
ifeq ($(SWIG),) # SWIG must be built
	@if [ ! -f swig-2.0.6/swig ]; then \
		tar xvf swig-2.0.6.tar.gz; cd swig-2.0.6; ./configure; make; \
		SWIG_LIB=`pwd`/Lib; export SWIG_LIB; \
	fi
SWIG=$(shell echo `pwd`/swig-2.0.6/swig)
endif

pyparsing:
ifneq ($(PYPARSING),0)
	@if [ ! -f ../src/pyparsing.py ]; then \
		unzip -q -n pyparsing-2.0.2.zip; cp pyparsing-2.0.2/pyparsing.py \
			../src/pyparsing.py; \
	fi
endif
