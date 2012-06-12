%module(package="pynusmv.nusmv.enc") enc

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/enc/enc.h" 
#include "../../../nusmv/src/enc/operators.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/enc/enc.h
%include ../../../nusmv/src/enc/operators.h