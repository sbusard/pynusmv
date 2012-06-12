%module(package="pynusmv.nusmv.enc.utils") utils

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/enc/utils/AddArray.h" 
#include "../../../../nusmv/src/enc/utils/OrdGroups.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/enc/utils/AddArray.h
%include ../../../../nusmv/src/enc/utils/OrdGroups.h