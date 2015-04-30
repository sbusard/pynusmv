%module(package="pynusmv.nusmv.compile.symb_table") symb_table

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/compile/symb_table/NFunction.h" 
#include "../../../../nusmv/src/compile/symb_table/ResolveSymbol.h" 
#include "../../../../nusmv/src/compile/symb_table/symb_table.h"
#include "../../../../nusmv/src/compile/symb_table/SymbCache.h" 
#include "../../../../nusmv/src/compile/symb_table/SymbLayer.h" 
#include "../../../../nusmv/src/compile/symb_table/SymbTable.h" 
#include "../../../../nusmv/src/compile/symb_table/SymbType.h" 
%}

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/compile/symb_table/NFunction.h
%include ../../../../nusmv/src/compile/symb_table/ResolveSymbol.h
%include ../../../../nusmv/src/compile/symb_table/symb_table.h
%include ../../../../nusmv/src/compile/symb_table/SymbCache.h
%include ../../../../nusmv/src/compile/symb_table/SymbLayer.h
%include ../../../../nusmv/src/compile/symb_table/SymbTable.h
%include ../../../../nusmv/src/compile/symb_table/SymbType.h


%inline %{

SymbLayerIter gen_iter(const SymbLayer_ptr self, unsigned int mask) {
    SymbLayerIter iter;
    SymbLayer_gen_iter(self, &iter, mask);
    return iter;
}

SymbLayer_ptr node2layer(node_ptr node) {
    return (SymbLayer_ptr) node;
}

%}