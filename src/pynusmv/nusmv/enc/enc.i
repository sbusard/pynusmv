%module(package="pynusmv.nusmv.enc") enc

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/enc/enc.h" 
#include "../../../nusmv/src/enc/operators.h" 
%}

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/enc/enc.h
%include ../../../nusmv/src/enc/operators.h