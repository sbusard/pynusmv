%module(package="pynusmv.nusmv.parser") parser

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/node/node.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/parser/grammar.h" 
#include "../../../nusmv/src/parser/parser.h" 
#include "../../../nusmv/src/parser/symbols.h" 
%}

# Removing duplicate macros definition (TOK_X macros).
#pragma SWIG nowarn=302

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/parser/grammar.h
%include ../../../nusmv/src/parser/parser.h
%include ../../../nusmv/src/parser/symbols.h