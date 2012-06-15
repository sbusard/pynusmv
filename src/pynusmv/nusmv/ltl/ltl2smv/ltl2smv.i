%module(package="pynusmv.nusmv.ltl.ltl2smv") ltl2smv

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/ltl/ltl2smv/ltl2smv.h" 
%}

%feature("autodoc", 1);

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/ltl/ltl2smv/ltl2smv.h