%module(package="pynusmv.nusmv.mc") mc

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/mc/mc.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/mc/mc.h