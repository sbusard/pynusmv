%module(package="pynusmv.nusmv.dd") dd

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/dd/dd.h" 
#include "../../../nusmv/src/dd/VarsHandler.h" 
%}

# Ignoring unimplemented functions
%ignore VarsHandler_promote_group;

%feature("autodoc", 1);

%include ../typedefs.tpl

%inline %{
int bdd_equal (bdd_ptr a, bdd_ptr b) {
    return a == b;
}
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/dd/dd.h
%include ../../../nusmv/src/dd/VarsHandler.h