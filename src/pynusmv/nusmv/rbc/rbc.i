%module(package="pynusmv.nusmv.rbc") rbc

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/rbc/ConjSet.h" 
#include "../../../nusmv/src/rbc/InlineResult.h" 
#include "../../../nusmv/src/rbc/rbc.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/rbc/ConjSet.h
%include ../../../nusmv/src/rbc/InlineResult.h
%include ../../../nusmv/src/rbc/rbc.h