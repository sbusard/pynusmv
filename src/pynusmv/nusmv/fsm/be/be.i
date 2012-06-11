%module(package="pynusmv.nusmv.fsm.be") be

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/fsm/be/BeFsm.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/fsm/be/BeFsm.h