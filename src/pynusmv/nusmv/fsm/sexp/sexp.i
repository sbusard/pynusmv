%module(package="pynusmv.nusmv.fsm.sexp") sexp

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/fsm/sexp/BoolSexpFsm.h" 
#include "../../../../nusmv/src/fsm/sexp/Expr.h" 
#include "../../../../nusmv/src/fsm/sexp/sexp.h" 
#include "../../../../nusmv/src/fsm/sexp/SexpFsm.h" 
%}

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/fsm/sexp/BoolSexpFsm.h
%include ../../../../nusmv/src/fsm/sexp/Expr.h
%include ../../../../nusmv/src/fsm/sexp/sexp.h
%include ../../../../nusmv/src/fsm/sexp/SexpFsm.h