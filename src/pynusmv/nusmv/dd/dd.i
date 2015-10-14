%module(package="pynusmv.nusmv.dd") dd

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/dd/dd.h"
#include "../../../nusmv/src/dd/VarsHandler.h"
#include "../../../cudd-2.4.1.1/include/cudd.h"
%}

// Ignoring unimplemented functions
%ignore VarsHandler_promote_group;

%feature("autodoc", 1);
%include "typemaps.i"

%include ../typedefs.tpl

typedef enum {
    CUDD_REORDER_SAME,
    CUDD_REORDER_NONE,
    CUDD_REORDER_RANDOM,
    CUDD_REORDER_RANDOM_PIVOT,
    CUDD_REORDER_SIFT,
    CUDD_REORDER_SIFT_CONVERGE,
    CUDD_REORDER_SYMM_SIFT,
    CUDD_REORDER_SYMM_SIFT_CONV,
    CUDD_REORDER_WINDOW2,
    CUDD_REORDER_WINDOW3,
    CUDD_REORDER_WINDOW4,
    CUDD_REORDER_WINDOW2_CONV,
    CUDD_REORDER_WINDOW3_CONV,
    CUDD_REORDER_WINDOW4_CONV,
    CUDD_REORDER_GROUP_SIFT,
    CUDD_REORDER_GROUP_SIFT_CONV,
    CUDD_REORDER_ANNEALING,
    CUDD_REORDER_GENETIC,
    CUDD_REORDER_LINEAR,
    CUDD_REORDER_LINEAR_CONVERGE,
    CUDD_REORDER_LAZY_SIFT,
    CUDD_REORDER_EXACT
} Cudd_ReorderingType;

// Typemap to be sure that even if the node_ptr is Nil, it is returned as None
%typemap(in, numinputs=0) dd_reorderingtype * method (dd_reorderingtype temp) {
    $1 = &temp;
}

%typemap(argout) dd_reorderingtype * method {
    PyObject *o, *o2, *o3;
    o = PyInt_FromLong(*$1);
    if (!$result) {
        $result = o;
    } else {
        if (!PyTuple_Check($result)) {
            PyObject *o2 = $result;
            $result = PyTuple_New(1);
            PyTuple_SetItem($result,0,o2);
        }
        o3 = PyTuple_New(1);
        PyTuple_SetItem(o3,0,o);
        o2 = $result;
        $result = PySequence_Concat(o2,o3);
        Py_DECREF(o2);
        Py_DECREF(o3);
    }
}

%inline %{
int reordering_status(DdManager *dd, dd_reorderingtype * method)
{
  return dd_reordering_status(dd, method);
}
%}

%inline %{
int bdd_equal (bdd_ptr a, bdd_ptr b) {
    return a == b;
}
%}

%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/dd/dd.h
%include ../../../nusmv/src/dd/VarsHandler.h