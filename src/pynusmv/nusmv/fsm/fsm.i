%module(package="pynusmv.nusmv.fsm") fsm

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/fsm/fsm.h" 
#include "../../../nusmv/src/fsm/FsmBuilder.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/fsm/fsm.h
%include ../../../nusmv/src/fsm/FsmBuilder.h