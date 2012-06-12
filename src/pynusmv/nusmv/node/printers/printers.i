%module(package="pynusmv.nusmv.node.printers") printers

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/node/printers/MasterPrinter.h" 
#include "../../../../nusmv/src/node/printers/PrinterBase.h" 
#include "../../../../nusmv/src/node/printers/PrinterIWffCore.h" 
#include "../../../../nusmv/src/node/printers/PrinterPsl.h" 
#include "../../../../nusmv/src/node/printers/PrinterSexpCore.h" 
#include "../../../../nusmv/src/node/printers/PrinterWffCore.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/node/printers/MasterPrinter.h
%include ../../../../nusmv/src/node/printers/PrinterBase.h
%include ../../../../nusmv/src/node/printers/PrinterIWffCore.h
%include ../../../../nusmv/src/node/printers/PrinterPsl.h
%include ../../../../nusmv/src/node/printers/PrinterSexpCore.h
%include ../../../../nusmv/src/node/printers/PrinterWffCore.h