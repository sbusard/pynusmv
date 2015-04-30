%module(package="pynusmv.nusmv.be") be

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/be/be.h" 
#include "../../../nusmv/src/be/bePkg.h" 
#include "../../../nusmv/src/be/beRbcManager.h" 
%}

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/be/be.h
%include ../../../nusmv/src/be/bePkg.h
%include ../../../nusmv/src/be/beRbcManager.h