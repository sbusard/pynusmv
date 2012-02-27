/**CFile***********************************************************************

  FileName    [ResolveSymbol.c]

  PackageName [compile]

  Synopsis    [Implementation of class 'ResolveSymbol']

  Description [Basic routines for resolving a symbol]

  SeeAlso     [ResolveSymbol.h]

  Author      [Marco Roveri Alessandro Mariotti]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK-irst.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#include "ResolveSymbol.h"
#include "utils/utils.h"
#include "compile/compile.h"
#include "parser/symbols.h"
#include "utils/error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* Return value in case an error occurs */
#define TYPE_ERROR ((node_ptr) -1)

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [ResolveSymbol class definition]

  Description []

  SeeAlso     []

******************************************************************************/

typedef struct ResolveSymbol_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  boolean initialized;

  boolean isVar;
  boolean isDefine;
  boolean isArrayDef;
  boolean isArray;
  boolean isParameter;
  boolean isConstantSimple;
  boolean isConstantComplex;
  boolean isFunction;

  node_ptr resolvedName;
  node_ptr name;
  node_ptr context;
} ResolveSymbol;

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern int yylineno;
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define CHECK_INITIALIZED(self)                 \
  nusmv_assert(true == self->initialized)

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void resolve_symbol_init ARGS((ResolveSymbol_ptr self));
static void resolve_symbol_deinit ARGS((ResolveSymbol_ptr self));


static node_ptr
resolve_symbol_resolve_name ARGS((const SymbTable_ptr symb_table,
                                  node_ptr n, node_ptr context));

static node_ptr
resolve_symbol_resolve_name_recur ARGS((const SymbTable_ptr symb_table,
                                        node_ptr n, node_ptr context));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The ResolveSymbol class constructor]

  Description        [The ResolveSymbol class constructor]

  SideEffects        []

  SeeAlso            [ResolveSymbol_destroy]

******************************************************************************/
ResolveSymbol_ptr ResolveSymbol_create()
{
  ResolveSymbol_ptr self = ALLOC(ResolveSymbol, 1);
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);

  resolve_symbol_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [The ResolveSymbol class destructor]

  Description        [The ResolveSymbol class destructor]

  SideEffects        []

  SeeAlso            [ResolveSymbol_create]

******************************************************************************/
void ResolveSymbol_destroy(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);

  resolve_symbol_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Checks if the resolved symbol is undefined or not.]

  Description        [Checks if the resolved symbol is undefined or not.
                      Returns true if the symbol is undefined. A
                      symbol is undefined if it is not declared within
                      the symbol table (i.e it is not a var, a define,
                      an array define, an array, a parameter or a
                      constant)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_undefined(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return ((self->isVar       + self->isDefine         +
           self->isArrayDef  + self->isArray          +
           self->isParameter + self->isConstantSimple +
           self->isConstantComplex + self->isFunction) == 0) ? true : false;
}


/**Function********************************************************************

  Synopsis           [Checks if the resolved symbol is defined or not.]

  Description        [Checks if the resolved symbol is defined or not.
                      Returns true if the symbol is defined. A
                      symbol is defined if it is declared within
                      the symbol table (i.e it is a var or a define,
                      an array define, an array, a parameter or a
                      constant)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_defined(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return ((self->isVar       + self->isDefine         +
           self->isArrayDef  + self->isArray          +
           self->isParameter + self->isConstantSimple +
           self->isConstantComplex + self->isFunction) == 0) ? false : true;
}


/**Function********************************************************************

  Synopsis           [Checks if the symbol is ambiguos or not.]

  Description        [Checks if the symbol is ambiguos or not. A symbol
                      is ambiguos if it is declared more than once
                      (e.g. as a variable and as a constant)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_ambiguous(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return ((self->isVar       + self->isDefine         +
           self->isArrayDef  + self->isArray          +
           self->isParameter + self->isConstantSimple +
           self->isConstantComplex + self->isFunction) > 1) ? true : false;
}


/**Function********************************************************************

  Synopsis           [Checks if the symbol is a variable]

  Description        [Checks if the symbol is a variable]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_var(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return self->isVar;
}


/**Function********************************************************************

  Synopsis           [Checks if the symbol is a define]

  Description        [Checks if the symbol is a define]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_define(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return self->isDefine;
}


/**Function********************************************************************

  Synopsis           [Checks if the symbol is a function]

  Description        [Checks if the symbol is a function]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_function(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return self->isFunction;
}


/**Function********************************************************************

  Synopsis           [Checks if the symbol is a constant]

  Description        [Checks if the symbol is a constant]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_constant(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return (1 == (self->isConstantSimple +
                self->isConstantComplex)) ? true : false;
}


/**Function********************************************************************

  Synopsis           [Checks if the symbol is a parameter (formal)]

  Description        [Checks if the symbol is a parameter (formal)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_parameter(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return self->isParameter;
}


/**Function********************************************************************

  Synopsis           [Checks if the symbol is an array]

  Description        [Checks if the symbol is an array]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_array(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return self->isArray;
}


/**Function********************************************************************

  Synopsis           [Checks if the symbol is an array define]

  Description        [Checks if the symbol is an array define]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_array_def(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return self->isArrayDef;
}


/**Function********************************************************************

  Synopsis           [Returns the resolved name of the symbol]

  Description        [Returns the resolved name of the symbol]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr ResolveSymbol_get_resolved_name(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  return self->resolvedName;
}


/**Function********************************************************************

  Synopsis           [Check if there has been some error in the
                      resolution of the symbol]

  Description        [Check if there has been some error in the
                      resolution of the symbol]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ResolveSymbol_is_error(ResolveSymbol_ptr self)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  return ResolveSymbol_is_ambiguous(self) ||
    ResolveSymbol_is_undefined(self);
}


/**Function********************************************************************

  Synopsis           [Get the error message, if any error occurred.]

  Description        [Get the error message, if any error occurred.
                      The returned message has to be freed by the caller]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char* ResolveSymbol_get_error_message(ResolveSymbol_ptr self)
{
  char* message = (char*)NULL;

  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);
  nusmv_assert(ResolveSymbol_is_error(self));

  if (ResolveSymbol_is_undefined(self)) {
    char* undef = sprint_node(self->resolvedName);

    message = ALLOC(char, strlen(undef) + 23);

    sprintf(message, "\"%s\" undefined", undef);
  }
  else if (ResolveSymbol_is_ambiguous(self)) {
    char* s1 = sprint_node(self->name);
    char* s2 = sprint_node(self->context);

    message = ALLOC(char, strlen(s1) + strlen(s2) + 29);

    sprintf(message, "Symbol \"%s\" is ambiguous in \"%s\"", s1, s2);
  }

  return message;
}


/**Function********************************************************************

  Synopsis           [Prints the error message, if any error occurred.]

  Description        [Prints the error message on the given stream, 
                      if any error occurred.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ResolveSymbol_print_error_message(ResolveSymbol_ptr self, FILE* stream)
{
  char* err;
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);

  err = ResolveSymbol_get_error_message(self);

  fprintf(stream, "%s\n", err);

  FREE(err);
}


/**Function********************************************************************

  Synopsis           [Throws an internal error if an error occurred.]

  Description        [Throws an internal error if an error occurred.
                      The error MUST exist, so the function should be
                      used in couple with ResolveSymbol_is_error. The
                      printed message is taken using
                      ResolveSymbol_get_error_message, and rpterr is
                      used for throwing the error.]

  SideEffects        []

  SeeAlso            [ResolveSymbol_get_error_message ResolveSymbol_is_error]

******************************************************************************/
void ResolveSymbol_throw_error(ResolveSymbol_ptr self)
{
  char *err;
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  CHECK_INITIALIZED(self);

  err = ResolveSymbol_get_error_message(self);

  rpterr("%s", err);
  FREE(err);
}


/**Function********************************************************************

  Synopsis           [Resolves the given symbol in the given context, and 
                      returns it.]

  Description        [Resolves the given symbol in the given context, and
                      returns it.

                      This function has to be called before any other
                      function, since it initializes the internal
                      structure. The internal structure is reset
                      before doing anything else.

                      It is possible to get the resolved name later by calling 
                      get_resolved_name.
                      ]

  SideEffects        []

  SeeAlso            [ResolveSymbol_get_resolved_name]

******************************************************************************/
node_ptr ResolveSymbol_resolve(ResolveSymbol_ptr self, SymbTable_ptr st,
                               node_ptr name, node_ptr context)
{
  RESOLVE_SYMBOL_CHECK_INSTANCE(self);
  node_ptr simpleName, complexName;

  nusmv_assert(Nil != name);

  /* Being ResolveSymbol_resolve the main function of the class, we
     can reset all fields, in order to work on a clean structure. */
  resolve_symbol_init(self);

  /* Build uniq internal representation for name */
  simpleName = find_atom(name);
  /* Build the complex name: resolve_symbol_resolve_name cannot be
     applied to a parameter, and parameters can be only simple ATOM
     prefixed with the context name */
  complexName = (ATOM == node_get_type(name)) ?
    find_node(DOT, context, simpleName) :
    resolve_symbol_resolve_name(st, simpleName, context);

  self->name = simpleName;
  self->context = context;

  /* only ATOMs can be parameters */
  self->isVar = SymbTable_is_symbol_var(st, complexName);
  self->isDefine = SymbTable_is_symbol_define(st, complexName);
  self->isArray = SymbTable_is_symbol_variable_array(st, complexName);
  self->isArrayDef = SymbTable_is_symbol_array_define(st, complexName);
  self->isParameter = SymbTable_is_symbol_parameter(st, complexName);
  self->isFunction = SymbTable_is_symbol_function(st, complexName);

  /* Pick the symbol suffix, and then check if there exists a constant
     with that name. If so, this symbol is ambiguous. Remember that
     constants have global scope and should not clash with any other
     symbol. */
  if (Nil != complexName) {
    node_ptr curr = complexName;

    while (DOT == node_get_type(curr)) { curr = cdr(curr); }

    if (ATOM == node_get_type(curr)) {
      self->isConstantSimple = SymbTable_is_symbol_constant(st, curr);
    }
  }

  /* We may have that simpleName and complexName are the same: If a
     constant is declared as (DOT Nil (ATOM name)), for example. But
     this is not ambiguous, so we just have to set only one of the two
     constants fields */
  if (simpleName != complexName) {
    /* Check if simpleName is a constant */
    self->isConstantSimple |= SymbTable_is_symbol_constant(st, simpleName);
  }

  /* Check if complexName is a constant */
  self->isConstantComplex = SymbTable_is_symbol_constant(st, complexName);

  self->resolvedName = NODE_FROM_INT(-1);
  if (self->isConstantSimple) {
    self->resolvedName = simpleName;
  }
  else {
    self->resolvedName = complexName;
  }
  self->initialized = true;

  return self->resolvedName;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The ResolveSymbol class private initializer]

  Description        [The ResolveSymbol class private initializer]

  SideEffects        []

  SeeAlso            [ResolveSymbol_create]

******************************************************************************/
static void resolve_symbol_init(ResolveSymbol_ptr self)
{
  /* members initialization */
  self->isVar = self->isDefine = self->isArray = false;
  self->isArrayDef = self->isParameter = self->isFunction = false;
  self->isConstantSimple = self->isConstantComplex = false;
  self->resolvedName = NODE_FROM_INT(-1);
  self->name = NODE_FROM_INT(-1);
  self->context = NODE_FROM_INT(-1);
  self->initialized = false;
}


/**Function********************************************************************

  Synopsis           [The ResolveSymbol class private deinitializer]

  Description        [The ResolveSymbol class private deinitializer]

  SideEffects        []

  SeeAlso            [ResolveSymbol_destroy]

******************************************************************************/
static void resolve_symbol_deinit(ResolveSymbol_ptr self)
{
  /* members deinitialization */

}


/**AutomaticEnd***************************************************************/


/**Function********************************************************************

   Synopsis    [Takes an expression representing an identifier
   and recursively normalizes it.]

   Description [The result of this function is a properly formed
   identifier, find_atom-ed, and suitable to access hash tables such
   as symbol table, etc.

   An input expression may be a simple or complex (with DOT)
   identifier, a bit or an array element.

   Note: identifiers are not expanded, i.e. defines and formal parameters
   are not substituted by the corresponding expressions.

   Nil is returned if the given expression is not syntactically an
   identifier.

   Currently, arrays are additionally flattened to maintain old code.
   See the description of compileFlattenSexpRecur for info about ARRAY.
   ]

   SideEffects        []

   SeeAlso            [resolve_symbol_resolve_name_recur]

******************************************************************************/
static node_ptr resolve_symbol_resolve_name(const SymbTable_ptr symb_table,
                                            node_ptr name, node_ptr context)
{
  node_ptr res;
  int temp;

  if (name == Nil) return name;

  temp = yylineno;
  yylineno = node_get_lineno(name);
  res = resolve_symbol_resolve_name_recur(symb_table, name, context);
  yylineno = temp;

  if (res == TYPE_ERROR) return (node_ptr) NULL;

#if 0
  /* In the past, ARRAY could be only in identifiers. Now it may be expression.
     To support old code we additionally flat ARRAY (e.g. conversion to
     if-then-else may be done, etc) to obtain expressions
     having only declared identifiers.
     This behavior may change in future. In which case all existing
     code dealing with ARRAY has to know that ARRAY may be identifier
     and may be expression.
     See the description of compileFlattenSexpRecur for info about ARRAY.

     Only high level ARRAY are of importance as ARRAY before DOT cannot
     be expressions because arrays of modules are not supported.
  */
  if (ARRAY == node_get_type(res) &&
      !SymbTable_is_symbol_declared(symb_table, res)) {
    return compileFlattenSexpRecur(symb_table, res, Nil);
  }
#endif


  return res;
}


/**Function********************************************************************

   Synopsis           [Performs the name "normalization", i.e.
   applies find_node and merges context with the identifier.]

   Description        [
   If name is complex and first ATOM is a parameter then the parameter is
   substituted by its value (in order to pass modules in parameters and
   access their members).
   Returns TYPE_ERROR if not resolvable name is provided]

   SideEffects        []

   SeeAlso            [resolve_symbol_resolve_name]

******************************************************************************/
static node_ptr
resolve_symbol_resolve_name_recur(const SymbTable_ptr symb_table,
                                   node_ptr n, node_ptr context)
{
  node_ptr temp, name;

  /* Warning : yylineno should NOT be set here.
     The upper level function has to decide which line number to use */

  switch (node_get_type(n)) {

  case CONTEXT:
    return resolve_symbol_resolve_name_recur(symb_table, cdr(n), car(n));

  case ATOM:
    name = find_node(DOT, context, find_atom(n));
    return name;

  case NUMBER: return find_atom(n);

  case BIT:
    temp = resolve_symbol_resolve_name_recur(symb_table, car(n), context);
    if (temp == TYPE_ERROR) rpterr("error in name resolution, operator bit");
    return find_node(BIT, temp, cdr(n)); /* cdr(n) is a int */

  case DOT:
    temp = (node_ptr) NULL;
    if (car(n) != (node_ptr) NULL) {
      temp = resolve_symbol_resolve_name_recur(symb_table, car(n), context);

      /* Check if temp is a module parameter*/
      while (SymbTable_is_symbol_parameter(symb_table, temp)) {
        int line_tmp = yylineno;
        /* unflattened version is accessed here !! */
        yylineno = node_get_lineno(SymbTable_get_actual_parameter(symb_table,
                                                                  temp));
        /* Recursively solve the parameter (required for modules passed
           as parameters). note: here flattened version is accessed */
        temp = resolve_symbol_resolve_name_recur(symb_table,
                   SymbTable_get_flatten_actual_parameter(symb_table, temp),
                   Nil);

        yylineno = line_tmp;
      }

      if (temp == TYPE_ERROR) rpterr("error in name resolution, operator = .");
    }
    /* on the right of DOT can be only ATOM */
    return find_node(DOT, temp, find_atom(cdr(n)));

  case ARRAY: {
    node_ptr index;
    /* ARRAY may be an expression and may be an identifier-with-brackets.
       Here we care only if array is an identifier. In this case
       index has to be a NUMBER. */
    temp = resolve_symbol_resolve_name_recur(symb_table, car(n), context);
    if (temp == TYPE_ERROR) return temp; /* array is expression */

    /* Check if temp is a module parameter*/
    while (SymbTable_is_symbol_parameter(symb_table, temp)) {
      int line_tmp = yylineno;
      /* unflattened version is accessed here !! */
      yylineno = node_get_lineno(SymbTable_get_actual_parameter(symb_table,
                                                                temp));
      /* Recursively solve the parameter (required for modules passed
         as parameters). note: here flattened version is accessed */
      temp = resolve_symbol_resolve_name_recur(symb_table,
                   SymbTable_get_flatten_actual_parameter(symb_table, temp),
                   Nil);

      yylineno = line_tmp;
    }

    /* on the right of [] we care only about NUMBER or MINUS NUMBER */
    index = cdr(n);
    if (node_get_type(index) == NUMBER) {
      index = find_atom(index);
    }
    else if (node_get_type(index) == UMINUS &&
        node_get_type(car(index)) == NUMBER) {
      index = find_node(NUMBER,
                        PTR_FROM_INT(node_ptr, -node_get_int(car(index))),
                        Nil);
    }
    else {
      /* keep the index-expression as it is since this exp is not identifier
         and we do not care about its normalization */
    }
    return find_node(ARRAY, temp, index);
  }

  case BIT_SELECTION:
    {
      node_ptr name;
      node_ptr t1, t2;
      name = resolve_symbol_resolve_name_recur(symb_table, car(n), context);
      if (name == TYPE_ERROR) return TYPE_ERROR;

      nusmv_assert(node_get_type(cdr(n)) == COLON);
      t1 = resolve_symbol_resolve_name_recur(symb_table, car(cdr(n)), context);
      if (t1 == TYPE_ERROR) return TYPE_ERROR;

      t2 = resolve_symbol_resolve_name_recur(symb_table, cdr(cdr(n)), context);
      if (t2 == TYPE_ERROR) return TYPE_ERROR;

      return find_node(BIT_SELECTION, name, find_node(COLON, t1, t2));
    }

  case SELF: return context;

  default:
    return TYPE_ERROR;
  }
}
