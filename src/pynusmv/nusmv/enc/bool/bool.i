# This package has been renamed due to reserved "bool" keyword.

%module(package="pynusmv.nusmv.enc.bool_package") bool_package

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/bool/BitValues.h" 
#include "../../../../nusmv/src/enc/bool/BoolEnc.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/bool/BitValues.h
%include ../../../../nusmv/src/enc/bool/BoolEnc.h