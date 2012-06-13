%module(package="pynusmv.nusmv.dd") dd

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/dd/dd.h" 
#include "../../../nusmv/src/dd/VarsHandler.h" 
%}

# Ignoring unimplemented functions
%ignore VarsHandler_promote_group;

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/dd/dd.h
%include ../../../nusmv/src/dd/VarsHandler.h