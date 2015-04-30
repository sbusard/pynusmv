%module(package="pynusmv.nusmv.enc.bool") "bool"

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/bool/BitValues.h" 
#include "../../../../nusmv/src/enc/bool/BoolEnc.h" 
%}

%feature("autodoc", 1);

%include ../../typedefs.tpl

%inline %{
BaseEnc_ptr boolenc2baseenc(BoolEnc_ptr bool_enc) {
    return (BaseEnc_ptr) bool_enc;
}
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/bool/BitValues.h
%include ../../../../nusmv/src/enc/bool/BoolEnc.h