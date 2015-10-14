%module(package="pynusmv.nusmv.bmc.sbmc") sbmc

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/bmc/sbmc/sbmcBmc.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcBmcInc.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcCmd.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcGen.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcHash.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcNodeStack.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcPkg.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcStructs.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcTableau.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcTableauInc.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcTableauIncLTLformula.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcTableauLTLformula.h" 
#include "../../../../nusmv/src/bmc/sbmc/sbmcUtils.h" 
%}

// Ignoring unimplemented function
%ignore sbmc_unroll_invariant_propositional;

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcBmc.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcBmcInc.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcCmd.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcGen.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcHash.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcNodeStack.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcPkg.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcStructs.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcTableau.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcTableauInc.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcTableauIncLTLformula.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcTableauLTLformula.h
%include ../../../../nusmv/src/bmc/sbmc/sbmcUtils.h