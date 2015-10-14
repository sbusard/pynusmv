%module(package="pynusmv.nusmv.mc") mc

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/mc/mc.h"
#include "../../../nusmv/src/mc/mcInt.h"
%}

// Ignoring unimplemented functions
%ignore check_invariant_forward;

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/mc/mc.h
%include ../../../nusmv/src/mc/mcInt.h