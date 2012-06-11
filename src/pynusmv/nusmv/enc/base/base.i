%module(package="pynusmv.nusmv.enc.base") base

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/base/BaseEnc.h" 
#include "../../../../nusmv/src/enc/base/BoolEncClient.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/base/BaseEnc.h
%include ../../../../nusmv/src/enc/base/BoolEncClient.h