%module(package="pynusmv.nusmv.opt") opt

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/opt/opt.h" 
#include "../../../nusmv/src/opt/OptsHandler.h" 
%}

// Ignoring unimplemented functions
%ignore opt_use_ltl_tableau_reachable_states;
%ignore set_use_ltl_tableau_reachable_states;
%ignore unset_use_ltl_tableau_reachable_states;

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/opt/opt.h
%include ../../../nusmv/src/opt/OptsHandler.h