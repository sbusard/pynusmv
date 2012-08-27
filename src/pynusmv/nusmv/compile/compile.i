%module(package="pynusmv.nusmv.compile") compile

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/compile/compile.h" 
#include "../../../nusmv/src/compile/FlatHierarchy.h" 
#include "../../../nusmv/src/compile/PredicateExtractor.h" 
#include "../../../nusmv/src/compile/PredicateNormaliser.h" 
%}

# Removing possible memory leak warning.
# Pointer to global flat hierarchy has to be cautiously used.
#pragma SWIG nowarn=454

# Ignoring unimplemented functions
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

EXTERN FlatHierarchy_ptr mainFlatHierarchy;

%}