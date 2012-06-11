%module(package="pynusmv.nusmv.node") node

%{
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/node/MasterNodeWalker.h" 
#include "../../../nusmv/src/node/node.h" 
#include "../../../nusmv/src/node/NodeWalker.h" 
%}

# Renaming necessary to deal with automatic construtor generation
# for struct node structure.
%rename(create_new_node) new_node;

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/node/MasterNodeWalker.h
%include ../../../nusmv/src/node/node.h
%include ../../../nusmv/src/node/NodeWalker.h