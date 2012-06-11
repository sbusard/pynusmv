%module(package="pynusmv.nusmv.dag") dag

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/dag/dag.h" 
%}

# Setting cpperraswarn to avoid problems with dag.h:91 (#error directive)
#pragma SWIG cpperraswarn=1
# And ignoring CPP #error warning
#pragma SWIG nowarn=205

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/dag/dag.h