%module(package="pynusmv.nusmv.sat.solvers") solvers

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/sat/solvers/SatMinisat.h" 
#include "../../../../nusmv/src/sat/solvers/satMiniSatIfc.h" 
#include "../../../../nusmv/src/sat/solvers/SatZchaff.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/sat/solvers/SatMinisat.h
%include ../../../../nusmv/src/sat/solvers/satMiniSatIfc.h
%include ../../../../nusmv/src/sat/solvers/SatZchaff.h