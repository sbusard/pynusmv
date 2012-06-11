%module(package="pynusmv.nusmv.addons_core") addons_core

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/addons_core/addonsCore.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/addons_core/addonsCore.h