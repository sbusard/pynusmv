%module(package="pynusmv.nusmv.cmd") cmd

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/cmd/cmd.h" 
%}

%feature("autodoc", 1);

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/cmd/cmd.h