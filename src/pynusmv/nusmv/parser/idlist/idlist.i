%module(package="pynusmv.nusmv.parser.idlist") idlist

%{
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/parser/idlist/idlist_grammar.h" 
#include "../../../../nusmv/src/parser/idlist/ParserIdList.h" 
%}

# Removing duplicate macros definition (token macros).
#pragma SWIG nowarn=302

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/parser/idlist/idlist_grammar.h
%include ../../../../nusmv/src/parser/idlist/ParserIdList.h