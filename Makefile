.PHONY: dep src

all: dep src

dep:
	export PYTHON
	export SWIG
	export PYPARSING
	$(MAKE) -C dep

src:
	export PYTHON
	export SWIG
	export PYPARSING
	$(MAKE) -C src