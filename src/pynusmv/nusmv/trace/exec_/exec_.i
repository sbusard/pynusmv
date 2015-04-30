%module(package="pynusmv.nusmv.trace.exec_") exec_

%include ../../global.i

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

%inline %{

PartialTraceExecutor_ptr bddPartialTraceExecutor2partialTraceExecutor(
                                              BDDPartialTraceExecutor_ptr ptr) {
    return (PartialTraceExecutor_ptr) ptr;
}

CompleteTraceExecutor_ptr bddCompleteTraceExecutor2completeTraceExecutor(
                                             BDDCompleteTraceExecutor_ptr ptr) {
    return (CompleteTraceExecutor_ptr) ptr;
}

%}