%module(package="pynusmv.nusmv.wff") wff

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/wff/wff.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/wff/wff.h