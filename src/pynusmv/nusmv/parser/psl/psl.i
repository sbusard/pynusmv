%module(package="pynusmv.nusmv.parser.psl") psl

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/parser/psl/psl_grammar.h" 
#include "../../../../nusmv/src/parser/psl/psl_symbols.h" 
#include "../../../../nusmv/src/parser/psl/pslExpr.h" 
#include "../../../../nusmv/src/parser/psl/pslNode.h" 
%}

# Removing duplicate macros definition (token macros).
#pragma SWIG nowarn=302

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/parser/psl/psl_grammar.h
%include ../../../../nusmv/src/parser/psl/psl_symbols.h
%include ../../../../nusmv/src/parser/psl/pslExpr.h
%include ../../../../nusmv/src/parser/psl/pslNode.h