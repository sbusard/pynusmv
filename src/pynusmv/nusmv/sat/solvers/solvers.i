%module(package="pynusmv.nusmv.sat.solvers") solvers

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/sat/solvers/SatZchaff.h" 
%}

# Ignoring unimplemented functions
%ignore SatZchaff_create;
%ignore SatZchaff_destroy;

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/sat/solvers/SatZchaff.h