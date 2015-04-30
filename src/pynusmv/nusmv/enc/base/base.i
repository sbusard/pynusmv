%module(package="pynusmv.nusmv.enc.base") base

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/base/BaseEnc.h" 
#include "../../../../nusmv/src/enc/base/BoolEncClient.h" 
%}

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/base/BaseEnc.h
%include ../../../../nusmv/src/enc/base/BoolEncClient.h