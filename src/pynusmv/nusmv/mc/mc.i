%module(package="pynusmv.nusmv.mc") mc

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/mc/mc.h" 
%}

# Ignoring unimplemented functions
%ignore check_invariant_forward;

%feature("autodoc", 1);

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/mc/mc.h