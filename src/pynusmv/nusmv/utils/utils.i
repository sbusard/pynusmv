%module(package="pynusmv.nusmv.utils") utils

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/utils/utils.h"
#include "../../../nusmv/src/utils/array.h" 
#include "../../../nusmv/src/utils/assoc.h" 
#include "../../../nusmv/src/utils/avl.h"  
#include "../../../nusmv/src/utils/error.h" 
#include "../../../nusmv/src/utils/heap.h" 
#include "../../../nusmv/src/utils/list.h" 
#include "../../../nusmv/src/utils/NodeGraph.h" 
#include "../../../nusmv/src/utils/NodeList.h" 
#include "../../../nusmv/src/utils/object.h" 
#include "../../../nusmv/src/utils/Olist.h" 
#include "../../../nusmv/src/utils/Pair.h" 
#include "../../../nusmv/src/utils/portability.h" 
#include "../../../nusmv/src/utils/range.h" 
#include "../../../nusmv/src/utils/Slist.h" 
#include "../../../nusmv/src/utils/Sset.h" 
#include "../../../nusmv/src/utils/Stack.h" 
#include "../../../nusmv/src/utils/TimerBench.h" 
#include "../../../nusmv/src/utils/Triple.h" 
#include "../../../nusmv/src/utils/ucmd.h" 
#include "../../../nusmv/src/utils/ustring.h" 
#include "../../../nusmv/src/utils/utils_io.h" 
#include "../../../nusmv/src/utils/WordNumber.h" 

/* sbusard 11/06/12 - Ignoring lsort.h due to errors in file parsing. */
/*#include "../../../nusmv/src/utils/lsort.h"*/
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/utils/utils.h
%include ../../../nusmv/src/utils/array.h
%include ../../../nusmv/src/utils/assoc.h
%include ../../../nusmv/src/utils/avl.h
%include ../../../nusmv/src/utils/error.h
%include ../../../nusmv/src/utils/heap.h
%include ../../../nusmv/src/utils/list.h
%include ../../../nusmv/src/utils/NodeGraph.h
%include ../../../nusmv/src/utils/NodeList.h
%include ../../../nusmv/src/utils/object.h
%include ../../../nusmv/src/utils/Olist.h
%include ../../../nusmv/src/utils/Pair.h
%include ../../../nusmv/src/utils/portability.h
%include ../../../nusmv/src/utils/range.h
%include ../../../nusmv/src/utils/Slist.h
%include ../../../nusmv/src/utils/Sset.h
%include ../../../nusmv/src/utils/Stack.h
%include ../../../nusmv/src/utils/TimerBench.h
%include ../../../nusmv/src/utils/Triple.h
%include ../../../nusmv/src/utils/ucmd.h
%include ../../../nusmv/src/utils/ustring.h
%include ../../../nusmv/src/utils/utils_io.h
%include ../../../nusmv/src/utils/WordNumber.h

#sbusard 11/06/12 - Ignoring lsort.h due to errors in file parsing.
#%include ../../../nusmv/src/utils/lsort.h