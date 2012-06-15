%module(package="pynusmv.nusmv.enc.utils") utils

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/enc/utils/AddArray.h" 
#include "../../../../nusmv/src/enc/utils/OrdGroups.h" 
%}

%feature("autodoc", 1);

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/enc/utils/AddArray.h
%include ../../../../nusmv/src/enc/utils/OrdGroups.h