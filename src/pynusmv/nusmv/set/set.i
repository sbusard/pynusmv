%module(package="pynusmv.nusmv.set") set

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/set/set.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/set/set.h