%module(package="pynusmv.nusmv.node") node

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/node/MasterNodeWalker.h" 
#include "../../../nusmv/src/node/node.h" 
#include "../../../nusmv/src/node/NodeWalker.h" 
%}

# Renaming new_node to create_node to avoid clash with default construtor
%rename (create_node) new_node;

# Ignoring unimplemented functions
%ignore normalize_nonboolean_case;

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/node/MasterNodeWalker.h
%include ../../../nusmv/src/node/node.h
%include ../../../nusmv/src/node/NodeWalker.h