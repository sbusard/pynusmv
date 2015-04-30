%module(package="pynusmv.nusmv.compile.type_checking.checkers") checkers

%include ../../../global.i

%{
#include "../../../../../nusmv/nusmv-config.h"
#include "../../../../../nusmv/src/utils/defs.h"
#include "../../../../../nusmv/src/utils/object.h"
#include "../../../../../nusmv/src/compile/type_checking/checkers/CheckerBase.h" 
#include "../../../../../nusmv/src/compile/type_checking/checkers/CheckerCore.h" 
#include "../../../../../nusmv/src/compile/type_checking/checkers/CheckerPsl.h" 
#include "../../../../../nusmv/src/compile/type_checking/checkers/CheckerStatement.h" 
%}

%feature("autodoc", 1);

%include ../../../typedefs.tpl

%include ../../../../../nusmv/src/utils/defs.h
%include ../../../../../nusmv/src/utils/object.h
%include ../../../../../nusmv/src/compile/type_checking/checkers/CheckerBase.h
%include ../../../../../nusmv/src/compile/type_checking/checkers/CheckerCore.h
%include ../../../../../nusmv/src/compile/type_checking/checkers/CheckerPsl.h
%include ../../../../../nusmv/src/compile/type_checking/checkers/CheckerStatement.h