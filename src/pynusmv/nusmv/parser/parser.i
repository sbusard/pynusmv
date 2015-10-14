%module(package="pynusmv.nusmv.parser") parser

%include ../global.i

%{
#include "../../../nusmv/nusmv-config.h"
#include "../../../nusmv/src/node/node.h"
#include "../../../nusmv/src/utils/defs.h"
#include "../../../nusmv/src/parser/grammar.h" 
#include "../../../nusmv/src/parser/parser.h" 
#include "../../../nusmv/src/parser/symbols.h" 

#include "../../../nusmv/src/utils/error.h"
EXTERN int yylineno;
%}

// Removing duplicate macros definition (TOK_X macros).
#pragma SWIG nowarn=302

%feature("autodoc", 1);
%include "typemaps.i"
%include "cstring.i"

%include ../typedefs.tpl


// Typemap to be sure that even the node_ptr is Nil, it is returned as None
%typemap(in, numinputs=0) int* error (int temp) {
    $1 = &temp;
}

%typemap(argout) int* error {
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

// These helpers reverse the returned int and argument node pointer
%inline %{
// error is set to 0 if everything is fine
//                 1 if a syntax error occured
//                 2 if a long jump was made (CATCH/FAIL) meaning a lexer error
node_ptr ReadSimpExprFromString(const char* str_expr, int* error) {
    node_ptr ptr;
    
    // yylineno seems not to be set at 1 when parsing a simple expression.
    yylineno = 1;
    
    CATCH {
        *error = Parser_ReadSimpExprFromString(str_expr, &ptr);
    }
    FAIL {
        // As there is an exception occuring, error is not set. Set it so
        *error = 2;
    }
    
    return ptr;
}


// error is set to 0 if everything is fine
//                 1 if a syntax error occured
//                 2 if a long jump was made (CATCH/FAIL) meaning a lexer error
node_ptr ReadNextExprFromString(const char* str_expr, int* error) {
    node_ptr ptr;
    
    // yylineno seems not to be set at 1 when parsing a simple expression.
    yylineno = 1;
    
    CATCH {
        *error = Parser_ReadNextExprFromString(str_expr, &ptr);
    }
    FAIL {
        // As there is an exception occuring, error is not set. Set it so
        *error = 2;
    }
    
    return ptr;
}


// error is set to 0 if everything is fine
//                 1 if a syntax error occured
//                 2 if a long jump occured (CATCH/FAIL) meaning a lexer error
node_ptr ReadIdentifierExprFromString(const char* str_expr, int* error) {
    node_ptr ptr;
    
    // yylineno seems not to be set at 1 when parsing a simple expression.
    yylineno = 1;
    
    CATCH {
        *error = Parser_ReadIdentifierExprFromString(str_expr, &ptr);
    }
    FAIL {
        // As there is an exception occuring, error is not set. Set it so
        *error = 2;
    }
    
    return ptr;
}


// error is set to 0 if everything is fine
//                 1 if a syntax error occured
//                 2 if a long jump occured (CATCH/FAIL) meaning a lexer error
node_ptr ReadCmdFromString(const char* str_expr, int* error) {
    node_ptr ptr;
    const int argc = 2;
    const char* argv[2];
    
    argv[0] = (char*) NULL;
    argv[1] = (char*) str_expr;
    
    // yylineno seems not to be set at 1 when parsing a simple expression.
    yylineno = 1;
    
    CATCH {
        *error = Parser_ReadCmdFromString(argc, argv, "", "", &ptr);
    }
    FAIL {
        // As there is an exception occuring, error is not set. Set it so
        *error = 2;
    }
    
    return ptr;
}


// returned value is 0 if everything is fine
//                   1 if Parser_ReadSMVFromFile(filename) return 1
//                   2 if a long jump occured
int ReadSMVFromFile(const char *filename) {
    int err;
    
    CATCH {
        err = Parser_ReadSMVFromFile(filename);
    }
    FAIL {
        err = 2;
    }
    return err;
}


// TODO Remove it?
EXTERN node_ptr parsed_tree;

%}
%clear int* error;


%include ../../../nusmv/src/utils/defs.h
%include ../../../nusmv/src/parser/grammar.h


// Apply typemaps to get as output all pointer arguments of
// Parser_get_syntax_error

%apply int *OUTPUT {int* out_lineno};

%cstring_output_allocate(const char **out_filename, );
%cstring_output_allocate(const char **out_token, );
%cstring_output_allocate(const char **out_message, );

%include ../../../nusmv/src/parser/parser.h

%clear int* out_lineno;


%include ../../../nusmv/src/parser/symbols.h