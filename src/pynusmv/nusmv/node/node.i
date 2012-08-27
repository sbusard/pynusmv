%module(package="pynusmv.nusmv.node") node

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/node/MasterNodeWalker.h" 
#include "../../../nusmv/src/node/node.h" 
#include "../../../nusmv/src/node/NodeWalker.h"
#include "../../../nusmv/src/dd/dd.h"
%}

# Renaming new_node to create_node to avoid clash with default construtor
%rename (create_node) new_node;

# Ignoring unimplemented functions
%ignore normalize_nonboolean_case;

%feature("autodoc", 1);

%include ../typedefs.tpl

%inline %{
bdd_ptr node2bdd(node_ptr ptr) {
    return (bdd_ptr) ptr;
}

node_ptr bdd2node(bdd_ptr ptr) {
    return (node_ptr) ptr;
}

node_ptr int2node(int val) {
    return (node_ptr) val;
}
%}


%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/node/MasterNodeWalker.h
%include ../../../nusmv/src/node/node.h
%include ../../../nusmv/src/node/NodeWalker.h