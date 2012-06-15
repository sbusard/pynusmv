%module(package="pynusmv.nusmv.trans.bdd") bdd

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/trans/bdd/bdd.h" 
#include "../../../../nusmv/src/trans/bdd/BddTrans.h" 
#include "../../../../nusmv/src/trans/bdd/Cluster.h" 
#include "../../../../nusmv/src/trans/bdd/ClusterList.h" 
#include "../../../../nusmv/src/trans/bdd/ClusterOptions.h" 
%}

%feature("autodoc", 1);

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/trans/bdd/bdd.h
%include ../../../../nusmv/src/trans/bdd/BddTrans.h
%include ../../../../nusmv/src/trans/bdd/Cluster.h
%include ../../../../nusmv/src/trans/bdd/ClusterList.h
%include ../../../../nusmv/src/trans/bdd/ClusterOptions.h