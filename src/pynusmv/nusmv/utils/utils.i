%module(package="pynusmv.nusmv.utils") utils

%include ../global.i

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

// Ignoring unimplemented functions
%ignore Siter_set_element;
%ignore Utils_get_temp_filename;
%ignore Utils_strtoint;
%ignore error_id_appears_twice_in_idlist_file;
%ignore error_not_word_sizeof;
%ignore util_str2int_inc;

%feature("autodoc", 1);

%include ../typedefs.tpl

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

// sbusard 11/06/12 - Ignoring lsort.h due to errors in file parsing.
#%include ../../../nusmv/src/utils/lsort.h

%inline %{

array_t* array_alloc_strings(int number) {
    return array_alloc(const char*, number);
}

#include <stdio.h>
void array_insert_strings(array_t* array, int i, const char* datum) {
    datum = util_strsav(datum);
    array_insert(const char*, array, i, datum);
}

const char* array_fetch_strings(array_t* array, int i) {
    return (const char*) array_fetch(const char*, array, i);
}
%}