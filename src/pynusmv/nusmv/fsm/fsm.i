%module(package="pynusmv.nusmv.fsm") fsm

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/fsm/fsm.h" 
#include "../../../nusmv/src/fsm/FsmBuilder.h" 
%}

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/fsm/fsm.h
%include ../../../nusmv/src/fsm/FsmBuilder.h