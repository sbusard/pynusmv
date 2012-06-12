%module(package="pynusmv.nusmv.compile") compile

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/compile/compile.h" 
#include "../../../nusmv/src/compile/FlatHierarchy.h" 
#include "../../../nusmv/src/compile/PredicateExtractor.h" 
#include "../../../nusmv/src/compile/PredicateNormaliser.h" 
%}

# Removing possible memory leak warning.
# Pointer to global flat hierarchy has to be cautiously used.
#pragma SWIG nowarn=454

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/compile/compile.h
%include ../../../nusmv/src/compile/FlatHierarchy.h
%include ../../../nusmv/src/compile/PredicateExtractor.h
%include ../../../nusmv/src/compile/PredicateNormaliser.h