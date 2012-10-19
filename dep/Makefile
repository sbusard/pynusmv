.PHONY: swig pyparsing

all: swig pyparsing

PYTHON3=python3
SWIG=$(shell which swig)
PYPARSING=$(shell $(PYTHON3) -c "import pyparsing" &> /dev/null; echo $$?)

swig:
ifeq ($(SWIG),) # SWIG must be built
	@if [ ! -f swig-2.0.6/swig ]; then \
		tar xvf swig-2.0.6.tar.gz; cd swig-2.0.6; ./configure; make; \
	fi
SWIG=$(shell echo `pwd`/swig-2.0.6/swig)
endif

pyparsing:
ifneq ($(PYPARSING), 0) # PyParsing is not installed
	@if [ ! -f ../src/pyparsing.py ]; then \
		unzip pyparsing-1.5.6.zip; cp pyparsing-1.5.6/pyparsing_py3.py \
			../src/pyparsing.py; \
	fi
PYPARSING=0
endif