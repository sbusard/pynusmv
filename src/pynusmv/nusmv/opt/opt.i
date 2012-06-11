%module(package="pynusmv.nusmv.opt") opt

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/opt/opt.h" 
#include "../../../nusmv/src/opt/OptsHandler.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/opt/opt.h
%include ../../../nusmv/src/opt/OptsHandler.h