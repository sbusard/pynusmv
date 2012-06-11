%module(package="pynusmv.nusmv.sat") sat

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/utils/object.h"
#include "../../../nusmv/src/sat/sat.h" 
#include "../../../nusmv/src/sat/SatIncSolver.h" 
#include "../../../nusmv/src/sat/SatSolver.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/utils/object.h
%include ../../../nusmv/src/sat/sat.h
%include ../../../nusmv/src/sat/SatIncSolver.h
%include ../../../nusmv/src/sat/SatSolver.h