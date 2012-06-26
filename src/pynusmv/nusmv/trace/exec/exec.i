%module(package="pynusmv.nusmv.trace.exec") exec

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/trace/exec/BaseTraceExecutor.h" 
#include "../../../../nusmv/src/trace/exec/BDDCompleteTraceExecutor.h" 
#include "../../../../nusmv/src/trace/exec/BDDPartialTraceExecutor.h" 
#include "../../../../nusmv/src/trace/exec/CompleteTraceExecutor.h" 
#include "../../../../nusmv/src/trace/exec/PartialTraceExecutor.h" 
#include "../../../../nusmv/src/trace/exec/SATCompleteTraceExecutor.h" 
#include "../../../../nusmv/src/trace/exec/SATPartialTraceExecutor.h" 
#include "../../../../nusmv/src/trace/exec/traceExec.h" 
%}

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/trace/exec/BaseTraceExecutor.h
%include ../../../../nusmv/src/trace/exec/BDDCompleteTraceExecutor.h
%include ../../../../nusmv/src/trace/exec/BDDPartialTraceExecutor.h
%include ../../../../nusmv/src/trace/exec/CompleteTraceExecutor.h
%include ../../../../nusmv/src/trace/exec/PartialTraceExecutor.h
%include ../../../../nusmv/src/trace/exec/SATCompleteTraceExecutor.h
%include ../../../../nusmv/src/trace/exec/SATPartialTraceExecutor.h
%include ../../../../nusmv/src/trace/exec/traceExec.h