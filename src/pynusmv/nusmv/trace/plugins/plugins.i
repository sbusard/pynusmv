%module(package="pynusmv.nusmv.trace.plugins") plugins

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/trace/plugins/TraceCompact.h" 
#include "../../../../nusmv/src/trace/plugins/TraceExplainer.h" 
#include "../../../../nusmv/src/trace/plugins/TracePlugin.h" 
#include "../../../../nusmv/src/trace/plugins/TraceTable.h" 
#include "../../../../nusmv/src/trace/plugins/TraceXmlDumper.h" 
%}

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/trace/plugins/TraceCompact.h
%include ../../../../nusmv/src/trace/plugins/TraceExplainer.h
%include ../../../../nusmv/src/trace/plugins/TracePlugin.h
%include ../../../../nusmv/src/trace/plugins/TraceTable.h
%include ../../../../nusmv/src/trace/plugins/TraceXmlDumper.h