%module(package="pynusmv.nusmv.trace.loaders") loaders

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/trace/loaders/TraceLoader.h" 
#include "../../../../nusmv/src/trace/loaders/TraceXmlLoader.h" 
%}

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/trace/loaders/TraceLoader.h
%include ../../../../nusmv/src/trace/loaders/TraceXmlLoader.h