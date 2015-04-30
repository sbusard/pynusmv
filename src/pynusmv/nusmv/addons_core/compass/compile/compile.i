%module(package="pynusmv.nusmv.addons_core.compass.compile") compile

%include ../../../global.i

%{
#include "../../../../../nusmv/nusmv-config.h"
#include "../../../../../nusmv/src/utils/defs.h"
#include "../../../../../nusmv/src/addons_core/compass/compile/ProbAssign.h" 
%}

%feature("autodoc", 1);

%include ../../../typedefs.tpl

%include ../../../../../nusmv/src/utils/defs.h
%include ../../../../../nusmv/src/addons_core/compass/compile/ProbAssign.h