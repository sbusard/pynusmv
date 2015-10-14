%module(package="pynusmv.nusmv.hrc.dumpers") dumpers

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/hrc/dumpers/HrcDumper.h" 
#include "../../../../nusmv/src/hrc/dumpers/HrcDumperDebug.h" 
#include "../../../../nusmv/src/hrc/dumpers/HrcDumperSmv.h" 
#include "../../../../nusmv/src/hrc/dumpers/HrcDumperXml.h" 
%}

// Removing possible memory leak warning.
// Global variables have to be cautiously used.
#pragma SWIG nowarn=451

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/hrc/dumpers/HrcDumper.h
%include ../../../../nusmv/src/hrc/dumpers/HrcDumperDebug.h
%include ../../../../nusmv/src/hrc/dumpers/HrcDumperSmv.h
%include ../../../../nusmv/src/hrc/dumpers/HrcDumperXml.h