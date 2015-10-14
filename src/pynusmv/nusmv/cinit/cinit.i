%module(package="pynusmv.nusmv.cinit") cinit

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/cinit/cinit.h" 
#include "../../../cudd-2.4.1.1/include/cudd.h"
%}

// Removing possible memory leak warning
// The use of FILE* pointers can lead to memory leak.
// Have to be used cautiously.
#pragma SWIG nowarn=454

// Ignoring unimplemented functions
%ignore NuSMVCore_set_init_fun;
%ignore NuSMVCore_set_quit_fun;
%ignore NuSMVCore_set_reset_init_fun;
%ignore NuSMVCore_set_reset_quit_fun;

%feature("autodoc", 1);

%include ../typedefs.tpl

%inline %{
EXTERN DdManager* dd_manager;
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/cinit/cinit.h