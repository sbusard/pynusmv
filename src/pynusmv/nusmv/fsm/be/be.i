%module(package="pynusmv.nusmv.fsm.be") be

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/fsm/be/BeFsm.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/fsm/be/BeFsm.h