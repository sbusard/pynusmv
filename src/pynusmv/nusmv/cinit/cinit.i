%module(package="pynusmv.nusmv.cinit") cinit

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/cinit/cinit.h" 
%}

# Removing possible memory leak warning
# The use of FILE* pointers can lead to memory leak. Have to be used cautiously.
#pragma SWIG nowarn=454

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/cinit/cinit.h