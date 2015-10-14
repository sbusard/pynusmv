%module(package="pynusmv.nusmv.addons_core.compass.parser.prob") prob

%include ../../../../global.i

%{
#include "../../../../../../nusmv/nusmv-config.h"
#include "../../../../../../nusmv/src/utils/defs.h"
#include "../../../../../../nusmv/src/addons_core/compass/parser/prob/ParserProb.h" 
#include "../../../../../../nusmv/src/addons_core/compass/parser/prob/prob_grammar.h" 
%}

// Removing warnings for redefined macros (TOK_X defined twice in prob_grammar)
#pragma SWIG nowarn=302

%feature("autodoc", 1);

%include ../../../../typedefs.tpl

%include ../../../../../../nusmv/src/utils/defs.h
%include ../../../../../../nusmv/src/addons_core/compass/parser/prob/ParserProb.h
%include ../../../../../../nusmv/src/addons_core/compass/parser/prob/prob_grammar.h