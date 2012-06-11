%module(package="pynusmv.nusmv.enc.be") be

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/be/BeEnc.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/be/BeEnc.h