%module(package="pynusmv.nusmv.compile") compile

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/compile/compile.h" 
#include "../../../nusmv/src/compile/FlatHierarchy.h" 
#include "../../../nusmv/src/compile/PredicateExtractor.h" 
#include "../../../nusmv/src/compile/PredicateNormaliser.h" 

#include "../../../nusmv/src/utils/error.h"
%}

// Removing possible memory leak warning.
// Pointer to global flat hierarchy has to be cautiously used.
#pragma SWIG nowarn=454

// Ignoring unimplemented functions
%ignore Compile_BuildVarsBdd;
%ignore build_proc_selector;
%ignore Compile_CompileInit;
%ignore Compile_CompileModel;
%ignore lookup_param_hash;
%ignore print_conjunctive_partition_info;
%ignore Compile_BuildInitBdd;
%ignore Compile_BuildInvarBdd;
%ignore start_test;
%ignore end_test;

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/compile/compile.h
%include ../../../nusmv/src/compile/FlatHierarchy.h
%include ../../../nusmv/src/compile/PredicateExtractor.h
%include ../../../nusmv/src/compile/PredicateNormaliser.h

%inline %{

int compile_flatten_smv(boolean calc_vars_constrains);

// ret = 0 => Everything is ok
// ret = 1 => Errors while flattening
// ret = 2 => Exception (longjump) occured
int flatten_hierarchy() {
    int res;
    CATCH {
        res = compile_flatten_smv(0);
    } FAIL {
        res = 2;
    }
    return res;
}

EXTERN void Compile_ConstructHierarchy 
ARGS((SymbTable_ptr symb_table,
      SymbLayer_ptr, node_ptr, node_ptr,
      node_ptr, FlatHierarchy_ptr, HrcNode_ptr, hash_ptr));

EXTERN void Compile_ProcessHierarchy ARGS((SymbTable_ptr symb_table,
                                           SymbLayer_ptr layer,
                                           FlatHierarchy_ptr hierachy,
                                           node_ptr name,
                                           boolean create_process_variables, 
                                           boolean calc_vars_constr));


EXTERN FlatHierarchy_ptr mainFlatHierarchy;
EXTERN cmp_struct_ptr cmps;


struct cmp_struct {
  int      read_model;
  int      hrc_built;
  int      flatten_hierarchy;
  int      encode_variables;
  int      process_selector;
  int      build_frames;
  int      build_model;
  int      build_flat_model;
  int      build_bool_model;
  int      bmc_init;
  int      bmc_setup;
  int      fairness_constraints;
  int      coi;
};

void cmp_struct_reset(cmp_struct_ptr cmp) {
    cmp->read_model           = 0;
    cmp->hrc_built            = 0;
    cmp->flatten_hierarchy    = 0;
    cmp->encode_variables     = 0;
    cmp->process_selector     = 0;
    cmp->build_frames         = 0;
    cmp->build_model          = 0;
    cmp->build_flat_model     = 0;
    cmp->build_bool_model     = 0;
    cmp->bmc_init             = 0;
    cmp->bmc_setup            = 0;
    cmp->fairness_constraints = 0;
    cmp->coi                  = 0;
}

%}


// Typemap to be sure that even the node_ptr is Nil, it is returned as None
%typemap(in, numinputs=0) int* error (int temp) {
    $1 = &temp;
}

%typemap(argout) int* error {
    PyObject *o, *o2, *o3;
    o = PyInt_FromLong(*$1);
    if (!$result) {
        $result = o;
    } else {
        if (!PyTuple_Check($result)) {
            PyObject *o2 = $result;
            $result = PyTuple_New(1);
            PyTuple_SetItem($result,0,o2);
        }
        o3 = PyTuple_New(1);
        PyTuple_SetItem(o3,0,o);
        o2 = $result;
        $result = PySequence_Concat(o2,o3);
        Py_DECREF(o2);
        Py_DECREF(o3);
    }
}

%inline %{

// Helper for Compile_FlattenSexp with handling exceptions
// error = 0 => No exception
// error = 1 => Exception thrown                                         
node_ptr FlattenSexp(const SymbTable_ptr symb_table, node_ptr sexp,
                             node_ptr context, int* error)
{
  node_ptr result;
  
  CATCH {
    result = Compile_FlattenSexp(symb_table, sexp, context);
    *error = 0;
  }
  FAIL {
    result = Nil;
    *error = 1;
  }

  return result;
}

%}

%clear int* error;