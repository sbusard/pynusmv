.PHONY: dep src

all: dep src

dep:
	export PYTHON3
	export SWIG
	export PYPARSING
	$(MAKE) -C dep

src:
	export PYTHON3
	export SWIG
	export PYPARSING
	$(MAKE) -C src