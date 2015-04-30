%module(package="pynusmv.nusmv.sat") sat

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/utils/object.h"
#include "../../../nusmv/src/sat/sat.h" 
#include "../../../nusmv/src/sat/SatIncSolver.h" 
#include "../../../nusmv/src/sat/SatSolver.h" 
%}

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/utils/object.h
%include ../../../nusmv/src/sat/sat.h
%include ../../../nusmv/src/sat/SatIncSolver.h
%include ../../../nusmv/src/sat/SatSolver.h