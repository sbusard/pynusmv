%module(package="pynusmv.nusmv.enc.bool") "bool"

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/bool/BitValues.h" 
#include "../../../../nusmv/src/enc/bool/BoolEnc.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/bool/BitValues.h
%include ../../../../nusmv/src/enc/bool/BoolEnc.h