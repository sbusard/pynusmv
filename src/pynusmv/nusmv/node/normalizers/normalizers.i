%module(package="pynusmv.nusmv.node.normalizers") normalizers

%include ../../global.i

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/node/normalizers/MasterNormalizer.h" 
#include "../../../../nusmv/src/node/normalizers/NormalizerBase.h" 
#include "../../../../nusmv/src/node/normalizers/NormalizerCore.h" 
#include "../../../../nusmv/src/node/normalizers/NormalizerPsl.h" 
%}

// Ignoring unimplemented functions
%ignore MasterNormalizer_destroy;

%feature("autodoc", 1);

%include ../../typedefs.tpl

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/node/normalizers/MasterNormalizer.h
%include ../../../../nusmv/src/node/normalizers/NormalizerBase.h
%include ../../../../nusmv/src/node/normalizers/NormalizerCore.h
%include ../../../../nusmv/src/node/normalizers/NormalizerPsl.h