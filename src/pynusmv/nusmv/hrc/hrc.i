%module(package="pynusmv.nusmv.hrc") hrc

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/hrc/hrc.h" 
#include "../../../nusmv/src/hrc/hrcCmd.h" 
#include "../../../nusmv/src/hrc/HrcFlattener.h" 
#include "../../../nusmv/src/hrc/HrcNode.h" 
#include "../../../nusmv/src/hrc/hrcPrefixUtils.h" 
#include "../../../nusmv/src/hrc/HrcVarDependencies.h" 
%}

// Removing possible memory leak warning.
// Global variables must be cautiously used.
#pragma SWIG nowarn=454

// Ignoring unimplemented functions
%ignore HrcPopulateSymbTable;

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/hrc/hrc.h
%include ../../../nusmv/src/hrc/hrcCmd.h
%include ../../../nusmv/src/hrc/HrcFlattener.h
%include ../../../nusmv/src/hrc/HrcNode.h
%include ../../../nusmv/src/hrc/hrcPrefixUtils.h
%include ../../../nusmv/src/hrc/HrcVarDependencies.h

%inline %{

HrcNode_ptr mainHrcNode;

%}