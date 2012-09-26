%module(package="pynusmv.nusmv.enc.bdd") bdd

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/base/BaseEnc.h"
#include "../../../../nusmv/src/enc/bdd/bdd.h" 
#include "../../../../nusmv/src/enc/bdd/BddEnc.h" 
#include "../../../../nusmv/src/enc/bdd/BddEncCache.h" 

#include "../../../../nusmv/src/utils/error.h"
%}

%feature("autodoc", 1);

%inline %{
BaseEnc_ptr bddenc2baseenc(BddEnc_ptr bdd_enc) {
    return (BaseEnc_ptr) bdd_enc;
}

// Result is NULL if an error occured, not NULL otherwise
bdd_ptr pick_one_state(const BddEnc_ptr self, bdd_ptr states) {
    bdd_ptr result;
    CATCH {
        result = BddEnc_pick_one_state(self, states);
    }
    FAIL {
        result = NULL;
    }
    return result;
}


// Result is NULL if an error occured, not NULL otherwise
bdd_ptr pick_one_input(const BddEnc_ptr self, bdd_ptr inputs) {
    bdd_ptr result;
    CATCH {
        result = BddEnc_pick_one_input(self, inputs);
    }
    FAIL {
        result = NULL;
    }
    return result;
}
%}

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/bdd/bdd.h
%include ../../../../nusmv/src/enc/bdd/BddEnc.h
%include ../../../../nusmv/src/enc/bdd/BddEncCache.h