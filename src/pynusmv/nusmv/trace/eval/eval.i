%module(package="pynusmv.nusmv.trace.eval") eval

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/trace/eval/BaseEvaluator.h" 
%}

%feature("autodoc", 1);

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/trace/eval/BaseEvaluator.h