%module(package="pynusmv.nusmv.be") be

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/be/be.h" 
#include "../../../nusmv/src/be/bePkg.h" 
#include "../../../nusmv/src/be/beRbcManager.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/be/be.h
%include ../../../nusmv/src/be/bePkg.h
%include ../../../nusmv/src/be/beRbcManager.h