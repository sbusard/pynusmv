%module(package="pynusmv.nusmv.trace.eval") eval

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/trace/eval/BaseEvaluator.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/trace/eval/BaseEvaluator.h