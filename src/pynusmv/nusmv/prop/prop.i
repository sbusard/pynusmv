%module(package="pynusmv.nusmv.prop") prop

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/prop/Prop.h" 
#include "../../../nusmv/src/prop/PropDb.h" 
#include "../../../nusmv/src/prop/propPkg.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/prop/Prop.h
%include ../../../nusmv/src/prop/PropDb.h
%include ../../../nusmv/src/prop/propPkg.h