%module(package="pynusmv.nusmv.fsm.bdd") bdd

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/fsm/bdd/bdd.h" 
#include "../../../../nusmv/src/fsm/bdd/BddFsm.h" 
#include "../../../../nusmv/src/fsm/bdd/FairnessList.h" 
%}

# Ignoring unimplemented functions
%ignore BddFsm_get_fair_states_subset;

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/fsm/bdd/bdd.h
%include ../../../../nusmv/src/fsm/bdd/BddFsm.h
%include ../../../../nusmv/src/fsm/bdd/FairnessList.h