%module(package="pynusmv.nusmv.parser.idlist") idlist

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/node/node.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/parser/idlist/idlist_grammar.h" 
#include "../../../../nusmv/src/parser/idlist/ParserIdList.h" 
%}

// Removing duplicate macros definition (token macros).
#pragma SWIG nowarn=302

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/parser/idlist/idlist_grammar.h
%include ../../../../nusmv/src/parser/idlist/ParserIdList.h