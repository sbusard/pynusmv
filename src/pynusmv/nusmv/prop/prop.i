%module(package="pynusmv.nusmv.prop") prop

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/prop/Prop.h" 
#include "../../../nusmv/src/prop/PropDb.h" 
#include "../../../nusmv/src/prop/propPkg.h" 
%}

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/prop/Prop.h
%include ../../../nusmv/src/prop/PropDb.h
%include ../../../nusmv/src/prop/propPkg.h