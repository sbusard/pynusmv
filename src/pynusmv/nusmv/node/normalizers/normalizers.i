%module(package="pynusmv.nusmv.node.normalizers") normalizers

%{
#include "../../../../nusmv/nusmv-config.h"
#include "../../../../nusmv/src/utils/defs.h"
#include "../../../../nusmv/src/utils/object.h"
#include "../../../../nusmv/src/node/normalizers/MasterNormalizer.h" 
#include "../../../../nusmv/src/node/normalizers/NormalizerBase.h" 
#include "../../../../nusmv/src/node/normalizers/NormalizerCore.h" 
#include "../../../../nusmv/src/node/normalizers/NormalizerPsl.h" 
%}

%include ../../../../nusmv/src/utils/defs.h
%include ../../../../nusmv/src/utils/object.h
%include ../../../../nusmv/src/node/normalizers/MasterNormalizer.h
%include ../../../../nusmv/src/node/normalizers/NormalizerBase.h
%include ../../../../nusmv/src/node/normalizers/NormalizerCore.h
%include ../../../../nusmv/src/node/normalizers/NormalizerPsl.h