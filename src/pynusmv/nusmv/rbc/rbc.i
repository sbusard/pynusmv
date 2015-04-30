%module(package="pynusmv.nusmv.rbc") rbc

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/rbc/ConjSet.h" 
#include "../../../nusmv/src/rbc/InlineResult.h" 
#include "../../../nusmv/src/rbc/rbc.h" 
%}

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/rbc/ConjSet.h
%include ../../../nusmv/src/rbc/InlineResult.h
%include ../../../nusmv/src/rbc/rbc.h