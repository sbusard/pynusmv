%module(package="pynusmv.nusmv.trace.loaders") loaders

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/trace/loaders/TraceLoader.h" 
#include "../../../../nusmv/src/trace/loaders/TraceXmlLoader.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/trace/loaders/TraceLoader.h
%include ../../../../nusmv/src/trace/loaders/TraceXmlLoader.h