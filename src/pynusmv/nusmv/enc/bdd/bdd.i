%module(package="pynusmv.nusmv.enc.bdd") bdd

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/enc/base/BaseEnc.h"
#include "../../../../nusmv/src/enc/bdd/bdd.h" 
#include "../../../../nusmv/src/enc/bdd/BddEnc.h" 
#include "../../../../nusmv/src/enc/bdd/BddEncCache.h" 
%}

%feature("autodoc", 1);

%inline %{
BaseEnc_ptr bddenc2baseenc(BddEnc_ptr bdd_enc) {
    return (BaseEnc_ptr) bdd_enc;
}
%}

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/enc/bdd/bdd.h
%include ../../../../nusmv/src/enc/bdd/BddEnc.h
%include ../../../../nusmv/src/enc/bdd/BddEncCache.h