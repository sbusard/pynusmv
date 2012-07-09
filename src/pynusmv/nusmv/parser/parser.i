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

%feature("autodoc", 1);

%include ../typedefs.tpl

%include "typemaps.i"
%apply int *OUTPUT {int* error};
%inline %{
node_ptr ReadSimpExprFromString(const char* str_expr, int* error)
{
    node_ptr ptr;
    *error = Parser_ReadSimpExprFromString(str_expr, &ptr);
    return ptr;
}
%}
%clear int* error;

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/parser/grammar.h
%include ../../../nusmv/src/parser/parser.h
%include ../../../nusmv/src/parser/symbols.h