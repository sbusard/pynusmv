%module(package="pynusmv.nusmv.node") node

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/node/MasterNodeWalker.h" 
#include "../../../nusmv/src/node/node.h" 
#include "../../../nusmv/src/node/NodeWalker.h"
#include "../../../nusmv/src/dd/dd.h"

#include "../../../nusmv/src/utils/ustring.h"
#include "../../../nusmv/src/utils/WordNumber.h"
%}

// Renaming new_node to create_node to avoid clash with default construtor
%rename (create_node) new_node;

// Ignoring unimplemented functions
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

int node2int(node_ptr ptr) {
    return (int) ptr;
}

node_ptr string2node(string_ptr string) {
    return (node_ptr) string;
}

string_ptr node2string(node_ptr node) {
    return (string_ptr) node;
}

WordNumber_ptr node2word(node_ptr node) {
    return (WordNumber_ptr) node;
}

node_ptr word2node(WordNumber_ptr word) {
    return (node_ptr) word;
}

boolean node_equal(node_ptr left, node_ptr right) {
    return left == right;
}
%}


%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/node/MasterNodeWalker.h
%include ../../../nusmv/src/node/node.h
%include ../../../nusmv/src/node/NodeWalker.h