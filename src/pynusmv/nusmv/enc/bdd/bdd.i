%module(package="pynusmv.nusmv.enc.bdd") bdd

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/base/BaseEnc.h"
#include "../../../../nusmv/src/enc/bdd/bdd.h" 
#include "../../../../nusmv/src/enc/bdd/BddEnc.h" 
#include "../../../../nusmv/src/enc/bdd/BddEncCache.h" 

#include "../../../../nusmv/src/utils/error.h"

#include "../../../../nusmv/src/dd/dd.h"

#include "../../../../nusmv/src/enc/bdd/BddEnc_private.h"
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
bdd_ptr pick_one_state_rand(const BddEnc_ptr self, bdd_ptr states) {
    bdd_ptr result;
    CATCH {
        result = BddEnc_pick_one_state_rand(self, states);
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

// Result is NULL if an error occured, not NULL otherwise
bdd_ptr pick_one_input_rand(const BddEnc_ptr self, bdd_ptr inputs) {
    bdd_ptr result;
    CATCH {
        result = BddEnc_pick_one_input_rand(self, inputs);
    }
    FAIL {
        result = NULL;
    }
    return result;
}
%}



%inline %{
bdd_ptr bdd_dup(bdd_ptr dd_node);
%}

%include "carrays.i"
%array_functions(bdd_ptr, bddArray);

%pythoncode %{

def pick_all_terms_states(bddenc, bdd):
    # count states
    count = int(BddEnc_count_states_of_bdd(bddenc, bdd))
    
    if count <= 0:
        return (0, tuple())
    
    # init array
    array = new_bddArray(count)
    for i in range(count):
        bddArray_setitem(array, i, None)
    
    # call function
    err = _pick_all_terms_states(bddenc, bdd, array, count)
    
    if err:
        delete_bddArray(array)
        return (err, tuple())
    
    else:
        # create tuple from array
        l = list()
        for i in range(count):
            if bddArray_getitem(array, i) is not None:
                l.append(bddArray_getitem(array, i))
        t = tuple(l)
    
        # delete array
        delete_bddArray(array)
    
        return (err, t)
    
    
def pick_all_terms_inputs(bddenc, bdd):
    # count states
    count = int(BddEnc_count_inputs_of_bdd(bddenc, bdd))
    
    if count <= 0:
        return (0, tuple())
    
    # init array
    array = new_bddArray(count)
    for i in range(count):
        bddArray_setitem(array, i, None)
    
    # call function
    err = _pick_all_terms_inputs(bddenc, bdd, array, count)
    
    if err:
        delete_bddArray(array)
        return (err, tuple())
    
    else:
        # create tuple from array
        l = list()
        for i in range(count):
            if bddArray_getitem(array, i) is not None:
                l.append(bddArray_getitem(array, i))
        t = tuple(l)
    
        # delete array
        delete_bddArray(array)
    
        return (err, t)
        
def pick_all_terms_states_inputs(bddenc, bdd):
    # count states
    count = int(BddEnc_count_states_inputs_of_bdd(bddenc, bdd))
    
    if count <= 0:
        return (0, tuple())
    
    # init array
    array = new_bddArray(count)
    for i in range(count):
        bddArray_setitem(array, i, None)
    
    # call function
    err = _pick_all_terms_states_inputs(bddenc, bdd, array, count)
    
    if err:
        delete_bddArray(array)
        return (err, tuple())
    
    else:
        # create tuple from array
        l = list()
        for i in range(count):
            if bddArray_getitem(array, i) is not None:
                l.append(bddArray_getitem(array, i))
        t = tuple(l)
    
        # delete array
        delete_bddArray(array)
    
        return (err, t)
%}

%inline %{

boolean _pick_all_terms_states(const BddEnc_ptr self, bdd_ptr bdd,
                                     bdd_ptr* result_array,
                                     const int array_len) {
                                     
    boolean error;
    CATCH {
        error = BddEnc_pick_all_terms_states(self, bdd, result_array,
                                             array_len);
    }
    FAIL {
        error = true;
    }
    return error;
}

boolean _pick_all_terms_inputs(const BddEnc_ptr self, bdd_ptr bdd,
                                     bdd_ptr* result_array,
                                     const int array_len) {
                                     
    boolean error;
    CATCH {
        error = BddEnc_pick_all_terms_inputs(self, bdd, result_array,
                                             array_len);
    }
    FAIL {
        error = true;
    }
    return error;
}

boolean _pick_all_terms_states_inputs(const BddEnc_ptr self, bdd_ptr bdd,
                                      bdd_ptr* result_array,
                                      const int array_len) {
                                     
    boolean error;
    CATCH {
        error = BddEnc_pick_all_terms_states_inputs(self, bdd, result_array,
                                                    array_len);
    }
    FAIL {
        error = true;
    }
    return error;
}

%}

%inline %{
bdd_ptr pick_one_state_input(const BddEnc_ptr self, bdd_ptr si)
{
    bdd_ptr result;
    CATCH {
        BDD_ENC_CHECK_INSTANCE(self);
        result = bdd_pick_one_minterm(self->dd, si,
                                      array_fetch_p(bdd_ptr,
                                      self->minterm_state_frozen_input_vars,
                                      0),
                                    self->minterm_state_frozen_input_vars_dim);
    }
    FAIL {
        result = NULL;
    }
    return result;
}

bdd_ptr pick_one_state_input_rand(const BddEnc_ptr self, bdd_ptr si)
{
    bdd_ptr result;
    CATCH {
        BDD_ENC_CHECK_INSTANCE(self);
        result = bdd_pick_one_minterm_rand(self->dd, si,
                                      array_fetch_p(bdd_ptr,
                                      self->minterm_state_frozen_input_vars,
                                      0),
                                    self->minterm_state_frozen_input_vars_dim);
    }
    FAIL {
        result = NULL;
    }
    return result;
}


int BddEnc_force_order_from_filename(const BddEnc_ptr self,
                                     const char * filename) {
    FILE* orderfile;
    orderfile = fopen(filename, "r");
    if (orderfile == (FILE*) NULL) { 
      return 1;
    }
    BddEnc_force_order_from_file(self, orderfile);
    fclose(orderfile);
    return 0;
}

%}



%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/bdd/bdd.h
%include ../../../../nusmv/src/enc/bdd/BddEnc.h
%include ../../../../nusmv/src/enc/bdd/BddEncCache.h