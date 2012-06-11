%module(package="pynusmv.nusmv.fsm.bdd") bdd

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/fsm/bdd/bdd.h" 
#include "../../../../nusmv/src/fsm/bdd/BddFsm.h" 
#include "../../../../nusmv/src/fsm/bdd/FairnessList.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/fsm/bdd/bdd.h
%include ../../../../nusmv/src/fsm/bdd/BddFsm.h
%include ../../../../nusmv/src/fsm/bdd/FairnessList.h