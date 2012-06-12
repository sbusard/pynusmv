%module(package="pynusmv.nusmv.ltl") ltl

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/ltl/ltl.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/ltl/ltl.h