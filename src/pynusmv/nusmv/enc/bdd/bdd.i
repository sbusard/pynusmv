%module(package="pynusmv.nusmv.enc.bdd") bdd

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/bdd/bdd.h" 
#include "../../../../nusmv/src/enc/bdd/BddEnc.h" 
#include "../../../../nusmv/src/enc/bdd/BddEncCache.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/bdd/bdd.h
%include ../../../../nusmv/src/enc/bdd/BddEnc.h
%include ../../../../nusmv/src/enc/bdd/BddEncCache.h