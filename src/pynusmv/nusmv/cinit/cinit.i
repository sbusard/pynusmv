%module(package="pynusmv.nusmv.cinit") cinit

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/cinit/cinit.h" 
%}

# Removing possible memory leak warning
# The use of FILE* pointers can lead to memory leak. Have to be used cautiously.
#pragma SWIG nowarn=454

# Ignoring unimplemented functions
%ignore NuSMVCore_set_init_fun;
%ignore NuSMVCore_set_quit_fun;
%ignore NuSMVCore_set_reset_init_fun;
%ignore NuSMVCore_set_reset_quit_fun;

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/cinit/cinit.h