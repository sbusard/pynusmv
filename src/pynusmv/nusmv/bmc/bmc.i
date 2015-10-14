%module(package="pynusmv.nusmv.bmc") bmc

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/bmc/bmc.h" 
#include "../../../nusmv/src/bmc/bmcBmc.h" 
#include "../../../nusmv/src/bmc/bmcCheck.h" 
#include "../../../nusmv/src/bmc/bmcCmd.h" 
#include "../../../nusmv/src/bmc/bmcConv.h" 
#include "../../../nusmv/src/bmc/bmcDump.h" 
#include "../../../nusmv/src/bmc/bmcGen.h" 
#include "../../../nusmv/src/bmc/bmcModel.h" 
#include "../../../nusmv/src/bmc/bmcPkg.h" 
#include "../../../nusmv/src/bmc/bmcSimulate.h" 
#include "../../../nusmv/src/bmc/bmcTableau.h" 
#include "../../../nusmv/src/bmc/bmcUtils.h" 
%}

// Ignoring unimplemented functions
%ignore Bmc_GenSolveLtlInc;
%ignore Bmc_GenSolveInvarZigzag;
%ignore Bmc_GenSolveInvarDual;
%ignore Bmc_GenSolveInvarFalsification;

%feature("autodoc", 1);

%include ../typedefs.tpl

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/bmc/bmc.h
%include ../../../nusmv/src/bmc/bmcBmc.h
%include ../../../nusmv/src/bmc/bmcCheck.h
%include ../../../nusmv/src/bmc/bmcCmd.h
%include ../../../nusmv/src/bmc/bmcConv.h
%include ../../../nusmv/src/bmc/bmcDump.h
%include ../../../nusmv/src/bmc/bmcGen.h
%include ../../../nusmv/src/bmc/bmcModel.h
%include ../../../nusmv/src/bmc/bmcPkg.h
%include ../../../nusmv/src/bmc/bmcSimulate.h
%include ../../../nusmv/src/bmc/bmcTableau.h
%include ../../../nusmv/src/bmc/bmcUtils.h