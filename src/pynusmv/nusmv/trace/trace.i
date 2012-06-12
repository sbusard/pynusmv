%module(package="pynusmv.nusmv.trace") trace

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/trace/pkg_trace.h" 
#include "../../../nusmv/src/trace/Trace.h" 
#include "../../../nusmv/src/trace/TraceLabel.h" 
#include "../../../nusmv/src/trace/TraceManager.h" 
#include "../../../nusmv/src/trace/TraceOpt.h" 
#include "../../../nusmv/src/trace/TraceXml.h" 
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/trace/pkg_trace.h
%include ../../../nusmv/src/trace/Trace.h
%include ../../../nusmv/src/trace/TraceLabel.h
%include ../../../nusmv/src/trace/TraceManager.h
%include ../../../nusmv/src/trace/TraceOpt.h
%include ../../../nusmv/src/trace/TraceXml.h