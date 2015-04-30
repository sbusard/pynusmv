%module(package="pynusmv.nusmv.enc.utils") utils

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/enc/utils/AddArray.h" 
#include "../../../../nusmv/src/enc/utils/OrdGroups.h" 
%}

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/enc/utils/AddArray.h
%include ../../../../nusmv/src/enc/utils/OrdGroups.h