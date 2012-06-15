%module(package="pynusmv.nusmv.trans.generic") generic

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/trans/generic/GenericTrans.h" 
%}

%feature("autodoc", 1);

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/trans/generic/GenericTrans.h