%module(package="pynusmv.nusmv.addons_core.compass") compass

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/addons_core/compass/compass.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/addons_core/compass/compass.h