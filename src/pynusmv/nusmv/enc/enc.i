%module(package="pynusmv.nusmv.enc") enc

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/enc/enc.h" 
#include "../../../nusmv/src/enc/operators.h" 
%}

%feature("autodoc", 1);

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/enc/enc.h
%include ../../../nusmv/src/enc/operators.h