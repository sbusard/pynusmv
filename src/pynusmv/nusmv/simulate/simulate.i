%module(package="pynusmv.nusmv.simulate") simulate

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/simulate/simulate.h" 
#include "../../../nusmv/src/simulate/simulateTransSet.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/simulate/simulate.h
%include ../../../nusmv/src/simulate/simulateTransSet.h