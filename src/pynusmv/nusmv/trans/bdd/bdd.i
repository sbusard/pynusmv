%module(package="pynusmv.nusmv.trans.bdd") bdd

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/trans/bdd/bdd.h" 
#include "../../../../nusmv/src/trans/bdd/BddTrans.h" 
#include "../../../../nusmv/src/trans/bdd/Cluster.h" 
#include "../../../../nusmv/src/trans/bdd/ClusterList.h" 
#include "../../../../nusmv/src/trans/bdd/ClusterOptions.h" 

#include "../../../../nusmv/src/utils/object.h"
%}

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/trans/bdd/bdd.h
%include ../../../../nusmv/src/trans/bdd/BddTrans.h
%include ../../../../nusmv/src/trans/bdd/Cluster.h
%include ../../../../nusmv/src/trans/bdd/ClusterList.h
%include ../../../../nusmv/src/trans/bdd/ClusterOptions.h


%inline %{

BddTrans_ptr BddTrans_copy(const BddTrans_ptr trans) {
    return BDD_TRANS(Object_copy(OBJECT(trans)));
}

void BddTrans_free(BddTrans_ptr trans) {
    Object_destroy(OBJECT(trans), NULL);
}

%}