%module(package="pynusmv.nusmv.trace") trace

%include ../global.i

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

// Ignoring unimplemented functions
%ignore TraceManager_create_evaluator;
%ignore TraceOpt_eval_defines;
%ignore TraceOpt_set_eval_defines;
%ignore TraceOpt_set_xml_reader_halts_on_undefined_symbols;
%ignore TraceOpt_set_xml_reader_halts_on_wrong_section;
%ignore TraceOpt_xml_reader_halts_on_undefined_symbols;
%ignore TraceOpt_xml_reader_halts_on_wrong_section;
%ignore TracePkg_execution_engine_from_string;
%ignore TraceUtils_complete_trace;
%ignore Trace_covers_language;
%ignore Trace_symbol_get_category;
%ignore Trace_symbol_is_assigned;

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/trace/pkg_trace.h
%include ../../../nusmv/src/trace/Trace.h
%include ../../../nusmv/src/trace/TraceLabel.h
%include ../../../nusmv/src/trace/TraceManager.h
%include ../../../nusmv/src/trace/TraceOpt.h
%include ../../../nusmv/src/trace/TraceXml.h