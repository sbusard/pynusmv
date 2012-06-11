%module(package="pynusmv.nusmv.addons_core.compass.compile") compile

%{
#include "../../../../../nusmv/src/utils/defs.h"
#include "../../../../../nusmv/src/addons_core/compass/compile/ProbAssign.h" 
%}

%include ../../../../../nusmv/src/utils/defs.h
%include ../../../../../nusmv/src/addons_core/compass/compile/ProbAssign.h