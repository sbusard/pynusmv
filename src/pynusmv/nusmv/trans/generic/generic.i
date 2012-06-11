%module(package="pynusmv.nusmv.trans.generic") generic

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/trans/generic/GenericTrans.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/trans/generic/GenericTrans.h