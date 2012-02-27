/**CFile***********************************************************************

FileName    [TypeChecker.c]

PackageName [compile.type_checking]

Synopsis    [Implementation of class 'TypeChecker']

Description []

SeeAlso     [TypeChecker.h]

Author      [Andrei Tchaltsev, Roberto Cavada]

Copyright   [
This file is part of the ``compile.type_checking'' package of NuSMV
version 2. Copyright (C) 2005 by FBK-irst.

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

#include "TypeChecker.h"
#include "TypeChecker_private.h"
#include "checkers/CheckerBase.h"
#include "checkers/CheckerCore.h"
#include "checkers/CheckerStatement.h"
#include "checkers/CheckerPsl.h"

#include "type_checkingInt.h"

#include "utils/WordNumber.h"
#include "node/MasterNodeWalker_private.h"
#include "prop/Prop.h"
#include "compile/symb_table/symb_table.h"
#include "parser/symbols.h"
#include "utils/utils.h"
#include "utils/ustring.h"
#include "utils/assoc.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: TypeChecker.c,v 1.1.2.56.4.14 2010-01-11 15:07:54 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

   Synopsis    [TypeChecker class definition]

   Description []

   SeeAlso     []

******************************************************************************/
typedef struct TypeChecker_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(MasterNodeWalker);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  int memoizing_counter; /* if 0, memoizing is enabled */
  SymbTable_ptr symbolTable; /* symbol table to look for symbols */
  hash_ptr expressionTypeMapping; /* type of checked expression's */
  boolean freshly_cleared; /* Used to avoid calling more than once
                              clear_assoc, when invoked by the
                              trigger */
} TypeChecker;



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void type_checker_init  ARGS((TypeChecker_ptr self,
                                     SymbTable_ptr symbolTable));
static void type_checker_deinit ARGS((TypeChecker_ptr self));

static void type_checker_finalize ARGS((Object_ptr object, void* dummy));

static boolean
type_checker_viol_handler ARGS((TypeChecker_ptr self,
                                TypeSystemViolation violation,
                                node_ptr expression));

static boolean
type_checker_check_constrain_list ARGS((TypeChecker_ptr self,
                                        int kind, node_ptr expressions));

static void type_checker_memoizing_force_enabled ARGS((TypeChecker_ptr self));

static void type_checker_reset_memoizing ARGS((TypeChecker_ptr self));

static void type_checker_remove_symbol_trigger ARGS((const SymbTable_ptr st,
                                                     const node_ptr sym,
                                                     void* arg));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [TypeChecker class constructor]

   Description        [TypeChecker class constructor.
   The 'symbolTable' is a symbol table to look for the type of found
   identifiers]

   SideEffects        []

   SeeAlso            [TypeChecker_destroy]

******************************************************************************/
TypeChecker_ptr TypeChecker_create(SymbTable_ptr symbolTable)
{
  TypeChecker_ptr self = ALLOC(TypeChecker, 1);
  TYPE_CHECKER_CHECK_INSTANCE(self);

  type_checker_init(self, symbolTable);
  return self;
}


/**Function********************************************************************

   Synopsis           [TypeChecker class constructor, with registration of
   a set of default of checkers.]

   Description        [TypeChecker class constructor, with registration of
   a set of default of checkers.
   The 'symbolTable' is a symbol table to look for the type of found
   identifiers]

   SideEffects        []

   SeeAlso            [TypeChecker_destroy]

******************************************************************************/
TypeChecker_ptr
TypeChecker_create_with_default_checkers(SymbTable_ptr symbolTable)
{
  TypeChecker_ptr self = TypeChecker_create(symbolTable);
  NodeWalker_ptr checker;

  /* checkers creation and registration */
  checker = NODE_WALKER(CheckerCore_create()); /* core */
  MasterNodeWalker_register_walker(MASTER_NODE_WALKER(self), checker);

  checker = NODE_WALKER(CheckerStatement_create()); /* statements */
  MasterNodeWalker_register_walker(MASTER_NODE_WALKER(self), checker);

  checker = NODE_WALKER(CheckerPsl_create()); /* psl */
  MasterNodeWalker_register_walker(MASTER_NODE_WALKER(self), checker);

  return self;
}


/**Function********************************************************************

   Synopsis           [The TypeChecker class destructor]

   Description        [The TypeChecker class destructor]

   SideEffects        []

   SeeAlso            [TypeChecker_create]

******************************************************************************/
void TypeChecker_destroy(TypeChecker_ptr self)
{
  TYPE_CHECKER_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

   Synopsis           [Returns the symbol table this type checker is
   associated to.]

   Description        [During its lifetime every type checker can deal only
   with one symbol table instance (because type checker caches the
   checked expressions and their types). The symbol table is given to
   the type checker during construction, and this function returns
   this symbol table.]

   SideEffects        []

   SeeAlso            [TypeChecker_create]

******************************************************************************/
SymbTable_ptr TypeChecker_get_symb_table(TypeChecker_ptr self)
{
  TYPE_CHECKER_CHECK_INSTANCE(self);
  return self->symbolTable;
}


/**Function********************************************************************

   Synopsis           [Checks that the types of variable decalarations
   in the layer are correctly formed. Also defines are checked to have
   some well-formed type.]

   Description        [Constrain: the input layer should belong to the symbol
   table the type checker is associated with.

   The function iterates over all variables in the layer, and checks their type
   with function TypeChecker_is_type_wellformed.

   The function also type checks the expressions provided in
   defines (also the generated "running" defines) have some type.
   NB for developers: This is done to allow the type checker
   to remember the type of these defines (and associated constants and variable
   _process_selector_). Without this list the evaluation phase will not
   know the type of these defines (if there were not explicitly used in the
   input text), since they can be implicitly used in ASSIGN(TRANS) contrains.

   Returns true if all the types are correctly formed and the defines
   are correct, false otherwise. ]

   SideEffects        []

   SeeAlso            [TypeChecker_is_type_wellformed]

******************************************************************************/
boolean TypeChecker_check_layer(TypeChecker_ptr self,
                                SymbLayer_ptr layer)
{
  boolean isOK;
  SymbTable_ptr table;
  SymbLayerIter liter;

  TYPE_CHECKER_CHECK_INSTANCE(self);

  table = TypeChecker_get_symb_table(self);

  /* the type checker's symbol table should contain the given layer */
  nusmv_assert(SymbTable_get_layer(table, SymbLayer_get_name(layer)) == layer);

  isOK = true;

  SYMB_LAYER_FOREACH(layer, liter, STT_VAR) {
    node_ptr varName = SymbLayer_iter_get_symbol(layer, &liter);
    SymbType_ptr type = SymbTable_get_var_type(table, varName);
    isOK = TypeChecker_is_type_wellformed(self, type,  varName) && isOK;
  }

  if (!isOK) return false;


  SYMB_LAYER_FOREACH(layer, liter, STT_DEFINE | STT_ARRAY_DEFINE) {
    node_ptr defName = SymbLayer_iter_get_symbol(layer, &liter);

    /* wrap in DEFINE because it is required by the function invoked */
    isOK = TypeChecker_is_specification_wellformed(self,
                                                     find_node(DEFINE, defName, Nil))
      && isOK;
  }

  return isOK;
}


/**Function********************************************************************

   Synopsis           [Checks all the module contrains are correctly typed]

   Description        [
   The module contrains are declarations INIT, INVAR, TRANS, ASSIGN,
   JUSTICE, COMPASSION.

   The first parameter 'checker' is a type checker to perfrom checking.
   All the remaining parameters are the sets of expressions
   constituting the bodies of the corresponding high-level
   declarations. These expressions are created during compilation and
   then passed to this function unmodified, flattened or
   flattened+expanded.  So this function is relatively specialised to
   deal with concrete data-structures created during compilation. For
   example, the expressions in the given sets are expected to be
   separated by CONS and AND.

   NOTE: if an expression has been flattened, then
   info about line numbers mat not be accurate.

   The type checker remebers all the checked expressions and their
   types, thus TypeChecker_get_expression_type uses memoizing to
   return the type of already checked expressions.

   The parameter 'assign' is actually the 'procs' returned
   by Compile_FlattenHierarchy, which contains all the assignments.

   If some of expressions violates the type system, the type checker's
   violation handler is invoked. See checkers into the checkers
   sub-package for more info.

   Returns false if the module contrains violate the type system, and
   otherwise true is returned.
   ]

   SideEffects        []

******************************************************************************/
boolean TypeChecker_check_constrains(TypeChecker_ptr self,
                                     node_ptr init, node_ptr trans,
                                     node_ptr invar, node_ptr assign,
                                     node_ptr justice, node_ptr compassion)
{
  boolean isOK;

  TYPE_CHECKER_CHECK_INSTANCE(self);

  isOK = true;

  /* check INIT */
  if (!type_checker_check_constrain_list(self, INIT, init)) isOK = false;

  /* check TRANS */
  if (!type_checker_check_constrain_list(self, TRANS, trans)) isOK = false;

  /* check INVAR */
  if (!type_checker_check_constrain_list(self, INVAR, invar)) isOK = false;

  /* check ASSIGN */
  if (!type_checker_check_constrain_list(self, ASSIGN, assign)) isOK = false;

  /* check  JUSTICE */
  if (!type_checker_check_constrain_list(self, JUSTICE, justice)) isOK = false;

  /* check  COMPASSION */
  if (!type_checker_check_constrain_list(self, COMPASSION, compassion)) {
    isOK = false;
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    if (isOK) fprintf(nusmv_stderr,
                      "Successful type-checking of the module constrains\n");
  }

  return isOK;
}



/**Function********************************************************************

   Synopsis           [Checks that the expression constituting the
   property is correctly typed]

   Description        [
   If some of expressions violates the type system, the type checker's
   violation handler is invoked. See checkers into the checkers
   sub-package for more info.

   The type checker remebers all the checked expressions and their
   types, thus TypeChecker_get_expression_type uses memoizing to return
   the type of already checked expressions.

   If the property violates the type system, the false value is return,
   and true value otherwise.]

   SideEffects        []

******************************************************************************/
boolean TypeChecker_check_property(TypeChecker_ptr self,
                                   Prop_ptr property)
{
  int kind;
  boolean isOK;
  node_ptr exp;

  TYPE_CHECKER_CHECK_INSTANCE(self);

  switch (Prop_get_type(property)) {
  case Prop_NoType:  error_unreachable_code(); /* incorrect property */
  case Prop_Ctl:     kind = SPEC;      break;
  case Prop_Ltl:     kind = LTLSPEC;   break;
  case Prop_Psl:     kind = PSLSPEC;   break;
  case Prop_Invar:   kind = INVARSPEC; break;
  case Prop_Compute: kind = COMPUTE;   break;

  default:           error_unreachable_code();
  } /* switch */

  yylineno = node_get_lineno(Prop_get_expr(property));
  exp = find_node(kind, Prop_get_expr(property), Nil);

  isOK = TypeChecker_is_specification_wellformed(self, exp);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    if (isOK) {
      /* the property is not yet inserted to database => there is no index */
      fprintf(nusmv_stderr, "Successful type-checking of a property\n");
    }
  }

  return isOK;
}


/**Function********************************************************************

   Synopsis           [The method type checks an expression and returns true
   if an expression is wellformed with respect to the type system and
   false otherwise.]

   Description        [
   The main purpose of this function is to be invoked on temporarily
   created expressions before they are evaluated to ADD or BDD.
   This function may not be useful for expressions read from files (such as
   bodies of INVAR, SPEC, etc) since they go through flattening in the
   compilation package and type-checked there.

   NOTE: an expression may be unmodified (after parsing and compilation),
   flattened or flattened+expanded.
   The expressions and their types are remembered by the type checker and
   the expressions types can be obtained with TypeChecker_get_expression_type.

   NOTE: memoizing is enabled before checking the expression.

   See checkers into the checkers sub-package for more info.]

   SideEffects        []

   SeeAlso            [type_checker_check_expression]

******************************************************************************/
boolean TypeChecker_is_expression_wellformed(TypeChecker_ptr self,
                                             node_ptr expression,
                                             node_ptr context)
{
  SymbType_ptr type;

  /* enables memoizing */
  type_checker_memoizing_force_enabled(self);

  /* Nil expressions in NuSMV often correspond to TRUE value => no violations */
  if (Nil == expression) return true;

  type = type_checker_check_expression(self, expression, context);
  if (SymbType_is_error(type)) {
    /* an error occurred, resets the type memoizing hash */
    type_checker_reset_memoizing(self);
    return false;
  }
  return true;
}


/**Function********************************************************************

   Synopsis           [Performs type checking of a specification]

   Description        [
   A specification is a usual (i.e. able to have a type)
   expression wrapped into a node with a specification tag such as INIT,
   INVAR, SPEC, COMPASSION, etc.
   There are two special case:
   ASSIGN can contains a list of EQDEF statements, and
   COMPUTE can contains MIN and MAX statement.

   The returned value is true if no violations of the type system are detected,
   and false otherwise.

   NOTE: the expression may be unmodified (after compilation),
   flattened or flattened+expanded.
   The expressions and their types are remembered by the type checker and
   the expressions types can be obtained with TypeChecker_get_expression_type.

   NOTE: memizing is enbaled before checking the specification

   See checkers into the checkers sub-package for more info.]

   SideEffects        []

   SeeAlso            [type_checker_check_expression]

******************************************************************************/
boolean TypeChecker_is_specification_wellformed(TypeChecker_ptr self,
                                                node_ptr expression)
{
  SymbType_ptr type;

  TYPE_CHECKER_CHECK_INSTANCE(self);

  /* enables memoizing */
  type_checker_memoizing_force_enabled(self);

  type = tc_lookup_expr_type(self, expression);
  /* the _whole_ expression has been already checked */
  if (nullType != type) return (type != SymbTablePkg_error_type());

  type = type_checker_check_expression(self, expression, Nil);
  if (SymbType_is_error(type)) {
    /* an error occurred, resets the type memoizing hash */
    type_checker_reset_memoizing(self);
    return false;
  }

  return true;
}


/**Function********************************************************************

   Synopsis           [Checks that a type is well formed.]

   Description        [
   This function is used to check the well-formedness of a type
   from a symbol table. This type should have properly created body,
   in particular, bodies should have correct line info.

   The constrains on a type are:
   1. word type: the width should be a NUMBER and have positive value.
   The width should not be greater than implemenetation limit
   WordNumber_max_width() bit (since we do not use
   arbitrary-precision arithmetic and ADD structores will be
   too big otherwise)
   2. enum type: there should be no duplicate values.

   The third parameter is the variable name that type is to be checked.
   The variable name is used just to output proper message in the case
   of a type violation.

   In the case of a type violation the violation handler obtain an
   expression CONS with variable name as left child and the body of the type
   as the right child.
   ]

   SideEffects        []

   SeeAlso            [type_checker_check_expression]

******************************************************************************/
boolean TypeChecker_is_type_wellformed(TypeChecker_ptr self,
                                       SymbType_ptr type,
                                       node_ptr varName)
{
  TYPE_CHECKER_CHECK_INSTANCE(self);

  switch (SymbType_get_tag(type)) {
  case SYMB_TYPE_BOOLEAN: break;

  case SYMB_TYPE_ENUM: {
    /* check that no value is repeated */
    /* Here, we assume that 'constant_list' in parser/grammar.y makes
       the values in the list unique (with the help of find_atom or find_node)
       but the list itself is not unique and contains correct line info.
    */

    node_ptr values = SymbType_get_enum_type_values(type);
    NodeList_ptr list = NodeList_create_from_list(values);
    ListIter_ptr iter = NodeList_get_first_iter(list);
    while (!ListIter_is_end(iter)) {
      if ((NodeList_count_elem(list, NodeList_get_elem_at(list, iter)) > 1) &&
          (type_checker_viol_handler(self,
                                     TC_VIOLATION_DUPLICATE_CONSTANTS,
                                     new_lined_node(CONS, varName, values,
                                                    node_get_lineno(values))))) {
        return false;
      }

      iter = ListIter_get_next(iter);
    }

    NodeList_destroy(list);
    break;
  }

  case SYMB_TYPE_UNSIGNED_WORD:
  case SYMB_TYPE_SIGNED_WORD: {
    int width = SymbType_get_word_width(type);
    if (0 < width && width <= WordNumber_max_width()) {
      return true;
    }

    if ( type_checker_viol_handler(self,
                                   TC_VIOLATION_INCORRECT_WORD_WIDTH,
                                   new_lined_node(CONS, varName,
                                                  new_node(NUMBER, NODE_FROM_INT(width), Nil),
                                                  SymbType_get_word_line_number(type))) ) {
      return false;
    }
    break;
  }

  case SYMB_TYPE_WORDARRAY: {
    int awidth;
    int vwidth;

    awidth = SymbType_get_wordarray_awidth(type);
    vwidth = SymbType_get_wordarray_vwidth(type);

    if (!(0 < awidth && awidth <= WordNumber_max_width())) {
      if ( type_checker_viol_handler(self,
                                     TC_VIOLATION_INCORRECT_WORDARRAY_WIDTH,
                                     new_lined_node(CONS, varName,
                                                    new_node(NUMBER, NODE_FROM_INT(awidth),
                                                             Nil),
                                                    SymbType_get_word_line_number(type))) ) {
        return false;
      }
    }

    if (!(0 < vwidth && vwidth <= WordNumber_max_width())) {
      if ( type_checker_viol_handler(self,
                                     TC_VIOLATION_INCORRECT_WORDARRAY_WIDTH,
                                     new_lined_node(CONS, varName,
                                                    new_node(NUMBER, NODE_FROM_INT(vwidth),
                                                             Nil),
                                                    SymbType_get_word_line_number(type))) ) {
        return false;
      }
    }
    break;
  }

    /* Array & matrix type */
  case SYMB_TYPE_ARRAY: {
    break;
  }


  case SYMB_TYPE_INTEGER:  /* (infinite-precision) integer */
  case SYMB_TYPE_REAL: /* (infinite-precision) rational */
    break;

  case SYMB_TYPE_NONE: /* no-type */
  case SYMB_TYPE_STATEMENT: /* statement */
  case SYMB_TYPE_SET_BOOL:  /* a set of integer values */
  case SYMB_TYPE_SET_INT:  /* a set of integer values */
  case SYMB_TYPE_SET_SYMB: /* a set of symbolic values */
  case SYMB_TYPE_SET_INT_SYMB:/* a set of symbolic and integer values */
  case SYMB_TYPE_ERROR: /* indicates an error */
  default:
    error_unreachable_code(); /* a variable cannot have these types */
  }

  return true;
}


/**Function********************************************************************

   Synopsis           [Returns the type of an expression.]

   Description        [If the expression has been already type-checked
   by the same type-checker, then the type of an expression is returned.
   Otherwise, the expression is type checked and its type is returned.

   The parameter 'context' indicates the context where expression
   has been checked. It should be exactly the same as during type checking.
   For outside user this parameter is usually Nil.
   NOTE: The returned type may be error-type indicating that
   the expression violates the type system.
   NOTE: all returned types are the memory-sharing types
   (see SymbTablePkg_..._type). So you can compare pointers instead of
   the type's contents.

   ]

   SideEffects        [TypeChecker_is_expression_wellformed,
   TypeChecker_is_specification_wellformed]

   SeeAlso            []

******************************************************************************/
SymbType_ptr TypeChecker_get_expression_type(TypeChecker_ptr self,
                                             node_ptr expression,
                                             node_ptr context)
{
  SymbType_ptr res;
  node_ptr ctx_expr;

  /* wrap exp into context if context is not Nil */
  if (Nil != context) ctx_expr = find_node(CONTEXT, context, expression);
  else ctx_expr = expression;

  res = tc_lookup_expr_type(self, ctx_expr);

  if (res == nullType) {
    res = type_checker_check_expression(self, expression, context);
  }

  return res;
}


/**Function********************************************************************

   Synopsis           [Returns true iff a given expression has been type checked]

   Description        [If this function returns true then
   TypeChecker_get_expression_type will return the cached type without performing
   the actual type checking.

   The parameter 'context' indicates the context where expression
   has been checked. It should be exactly the same as during type checking.
   For outside user this parameter is usually Nil.
   ]

   SideEffects        []

   SeeAlso            [TypeChecker_get_expression_type]

******************************************************************************/
boolean TypeChecker_is_expression_type_checked(TypeChecker_ptr self,
                                               node_ptr expression,
                                               node_ptr context)
{
  SymbType_ptr res;
  node_ptr ctx_expr;

  /* wrap exp into context if context is not Nil */
  if (Nil != context) ctx_expr = find_node(CONTEXT, context, expression);
  else ctx_expr = expression;

  res = tc_lookup_expr_type(self, ctx_expr);

  return res != nullType;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [Creates the association between an expression and
   its type, if memoizing is enabled]

   Description        [There should be not existing associated type
   for a given expression.

   Note: if there is (not null) context then the expression
   must be wrapped into CONTEXT expression. For example,
   for an expression exp and not null context ctx, the
   expression provided for this function should be find_node(CONTEXT, ctx, exp).
   The expression should not be wrapped into Nil context.]

   SideEffects        []

   SeeAlso            [tc_lookup_expr_type]
******************************************************************************/
SymbType_ptr tc_set_expression_type(TypeChecker_ptr self,
                                    node_ptr expression, SymbType_ptr type)
{
  if (type_checker_is_memoizing_enabled(self)) {
    nusmv_assert(nullType == tc_lookup_expr_type(self, expression));
    insert_assoc(self->expressionTypeMapping, expression, (node_ptr) type);
    self->freshly_cleared = false;
  }
  return type;
}


/**Function********************************************************************

   Synopsis [Looks up the internal type memoizing for expression's type
   and returns the associated type, or nullType if the expression was not
   previously checked]

   Description [If memoizing is not enabled, nullType will always be
   returned]


   SideEffects        []

   SeeAlso            [tc_set_expression_type]
******************************************************************************/
SymbType_ptr tc_lookup_expr_type(TypeChecker_ptr self, node_ptr expression)
{
  if (type_checker_is_memoizing_enabled(self)) {
    return SYMB_TYPE(find_assoc(self->expressionTypeMapping, expression));
  }
  return nullType;
}


/**Function********************************************************************

   Synopsis           [Perform type checking of an expression, and returns its
   type]

   Description        [This function is the core of the expression type
   checking.

   The expression may by unmodified (after compilation), flattened or
   flattened+expanded.

   The return value is the type of the expression.  If an expression
   violates the type system, the violation handler function is invoked
   and "error" type is returned (not NULL).



   NB: As it is said in TypeChecker.h, the expression type checking package
   uses only memory-shared types (SymbTablePkg_..._type).

   NB: This function does not perform the type checking on already
   checked expressions (even if they were erroneous), but just returns
   their types (possibly error-type). This avoids multiple error and warning
   messages for the same subexpression.]

   SideEffects        []

   SeeAlso            [type_checking_violation_handler]

******************************************************************************/
SymbType_ptr type_checker_check_expression(TypeChecker_ptr self,
                                           node_ptr expression,
                                           node_ptr context)
{
  ListIter_ptr iter;
  iter = NodeList_get_first_iter(MASTER_NODE_WALKER(self)->walkers);
  while (!ListIter_is_end(iter)) {
    CheckerBase_ptr cb =
      CHECKER_BASE(NodeList_get_elem_at(MASTER_NODE_WALKER(self)->walkers,
                                        iter));

    if (NodeWalker_can_handle(NODE_WALKER(cb), expression)) {
      return CheckerBase_check_expr(cb, expression, context);
    }

    iter = ListIter_get_next(iter);
  }

  /* Fall back */
  fprintf(nusmv_stderr, "Warning: no compatible checker found for expression:\n");
  print_node(nusmv_stderr, expression);
  fprintf(nusmv_stderr, "\n");
  print_sexp(nusmv_stderr, expression);
  return SymbTablePkg_error_type();
}



/**Function********************************************************************

   Synopsis [Prints whther an error or a warning message, depending on
   the given parameter]

   Description [This private funciont is called by violation handler
   into checkers and self, to print a uniform message when type errors
   and warnings occur]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
void type_checker_print_error_message(TypeChecker_ptr self, node_ptr expr,
                                      boolean is_error)
{
  if (is_error) fprintf(nusmv_stderr, "\nTYPE ERROR ");
  else          fprintf(nusmv_stderr, "\nTYPE WARNING ");

  if (get_input_file(OptsHandler_get_instance())) {
    fprintf(nusmv_stderr, "file %s", get_input_file(OptsHandler_get_instance()));
  }
  else fprintf(nusmv_stderr, "file stdin");

  if (node_get_lineno(expr)) {
    fprintf(nusmv_stderr, ": line %d", node_get_lineno(expr));
  }
  fprintf(nusmv_stderr,  " : ");
}


/**Function********************************************************************

   Synopsis           [Enables or disables internal type memoizing]

   Description [This method is used by checkers to temporary disable
   internal type memoizing that associates each sub-expression to its
   type.  This can be used to force an already checked formula to be
   re-checked.  For example the PSL checker while checking 'forall'
   expressions requires the repeated formula to be checked as many times as
   the id has a range of possible values.

   Important: memoizing is enabled by default and re-enabled every time
   the user calls the top level checking method. However, it is good
   behaviour for checkers to re-enable memoizing after disabling it]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
void type_checker_enable_memoizing(TypeChecker_ptr self, boolean enabled)
{
  if (enabled && self->memoizing_counter > 0) self->memoizing_counter -= 1;
  else if (!enabled) self->memoizing_counter += 1;
}


/**Function********************************************************************

   Synopsis           [Returns true if memoizing is currently enabled, false
   otherwise]

   Description        []

   SideEffects        []

   SeeAlso            []
******************************************************************************/
boolean type_checker_is_memoizing_enabled(const TypeChecker_ptr self)
{
  return self->memoizing_counter == 0;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The TypeChecker class private initializer]

   Description        [The TypeChecker class private initializer]

   SideEffects        []

   SeeAlso            [TypeChecker_create]

******************************************************************************/
static void type_checker_init(TypeChecker_ptr self, SymbTable_ptr symbolTable)
{
  /* base class initialization */
  master_node_walker_init(MASTER_NODE_WALKER(self));

  /* members initialisation */
  self->memoizing_counter = 0; /* memoizing is enabled by default */
  self->expressionTypeMapping = new_assoc();
  self->symbolTable = symbolTable;
  self->freshly_cleared = false;

  /* When a symbol is redeclared, we need to clear the type checker
     cache. See bug #422*/
  SymbTable_add_trigger(symbolTable,
                        type_checker_remove_symbol_trigger,
                        ST_TRIGGER_SYMBOL_REDECLARE,
                        (void*)self);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = type_checker_finalize;
}


/**Function********************************************************************

   Synopsis           [The TypeChecker class private deinitializer]

   Description        [The TypeChecker class private deinitializer]

   SideEffects        []

   SeeAlso            [TypeChecker_destroy]

******************************************************************************/
static void type_checker_deinit(TypeChecker_ptr self)
{
  /* members deinitialization */
  free_assoc(self->expressionTypeMapping);

  /* base class deinitialization */
  master_node_walker_deinit(MASTER_NODE_WALKER(self));
}


/**Function********************************************************************

   Synopsis    [The PrinterBase class virtual finalizer]

   Description [Called by the class destructor]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static void type_checker_finalize(Object_ptr object, void* dummy)
{
  TypeChecker_ptr self = TYPE_CHECKER(object);

  type_checker_deinit(self);
  FREE(self);
}


/**Static Function************************************************************

   Synopsis           [Checks a list of expressions for being well-types.
   Expressions can be unmodified (after compilation), flattened or
   flattened+expanded.]

   Description        [The first parameter is the type checker performing
   the type checking. The second one is the kind of SMV constrain
   containing the input expressions (like INVAR, TRANS, etc). The third
   one is a set of expressions to be checked.
   In the set the expression are separated by AND or CONS. The set also
   can be Nil. This function is specialised to deal with the output of
   the compilation phase.

   NB for developers: the list of constrain expressions collected
   during flattening, i.e. "expressions" contains the actual
   expressions only. If the expression consist of one identifier only,
   then the indentifier is normalized (with resolve-name and find-atom)
   and info about the line number is lost. Then only the wrapping AND or CONS
   node will contain the correct line.

   The return value is true if there is no type violations, and false otherwise]

   SideEffects        []

******************************************************************************/
static boolean type_checker_check_constrain_list(TypeChecker_ptr self,
                                                 int kind,
                                                 node_ptr expressions)
{
  node_ptr exp;
  boolean isOK = true;

  if (Nil == expressions) return true;

  nusmv_assert(AND == node_get_type(expressions) ||
               CONS == node_get_type(expressions));


  /* process the left part of the list */
  exp = car(expressions);
  if (Nil != exp && AND != node_get_type(exp) && CONS != node_get_type(exp)) {
    /* this is an actual expression. */
    yylineno = node_get_lineno(expressions);
    isOK = TypeChecker_is_specification_wellformed(self,
                                                   find_node(kind, exp, Nil))
      && isOK;
  }
  else {
    /* this is a subset of expressions */
    isOK = type_checker_check_constrain_list(self, kind, exp) && isOK;
  }

  /* process the right part of the list */
  exp = cdr(expressions);
  if (Nil != exp && AND != node_get_type(exp) && CONS != node_get_type(exp)) {
    /* this is an actual expression  */
    yylineno = node_get_lineno(expressions);
    isOK = TypeChecker_is_specification_wellformed(self,
                                                   find_node(kind, exp, Nil))
      && isOK;
  }
  else {
    /* this is a subset of expressions */
    isOK = type_checker_check_constrain_list(self, kind, exp) && isOK;
  }

  return isOK;
}




/**Function********************************************************************

   Synopsis           [The type core violation handler.]

   Description        [The violation handler is implemented as
   a virtual method, which is invoked by the checker when an expression
   being checked violates the type system.
   See the violation handler TypeCheckingViolationHandler_ptr
   for more explanations.

   The below function is the default violation handler, and a
   user can potentially define its own violation handler, by deriving
   a new class from this class and by overriding this virtual method.

   This violation handler outputs an error and warning message to
   nusmv_stderr. A warning is output if the detected violation is
   TC_VIOLATION_DUPLICATE_CONSTANTS and
   the system variable "type_checking_backward_compatibility" is
   true. Only in this case the false value is returned, indicating that
   this is NOT an error. Otherwise the true value is returned,
   indicating that this is an error.

   Also, if the system variable "type_check_warning_on" is false,
   warning messages are not output.

   NB: if the expression is checked in some context (context is not null) then
   before providing the expression to this function the expression should be
   wrapped into context, i.e. with find_node(CONEXT, context, expr)]

   SideEffects       []

   SeeAlso           [TypeSystemViolation]

******************************************************************************/
static boolean
type_checker_viol_handler(TypeChecker_ptr self,
                          TypeSystemViolation violation, node_ptr expression)
{
  /* In the output message, the information about the expression
     location are output. So, make sure that the input file name and
     line number are correctly set!
  */

  boolean isError = true; /* is this error or warning */

  /* get rid of the context the expression may be wrapped in */
  node_ptr expr = expression;
  if (node_get_type(expression) == CONTEXT) {
    expr = cdr(expression);
  }

  /* checks the given violation */
  nusmv_assert(TypeSystemViolation_is_valid(violation));

  /* only violation TC_VIOLATION_DUPLICATE_CONSTANTS and the variable
     type_checking_backward_compatibility being true, may make a
     warning from an error.
  */
  if ((TC_VIOLATION_DUPLICATE_CONSTANTS == violation) &&
      opt_backward_comp(OptsHandler_get_instance())) {
    isError = false;
  }

  if (!isError && !opt_type_checking_warning_on(OptsHandler_get_instance())) {
    /* this is a warning and warning messages are not allowed.
       So, do nothing, just return false (this is not an error)
    */
    return false;
  }

  type_checker_print_error_message(self, expr, isError);

  switch (violation) {

  case TC_VIOLATION_INCORRECT_WORD_WIDTH:
    nusmv_assert(CONS == node_get_type(expr));
    fprintf(nusmv_stderr, "in the declaration of '");
    print_node(nusmv_stderr, car(expr));
    fprintf(nusmv_stderr,
            "' the Word width is not a positive number (from range [1..%d])\n",
            WordNumber_max_width());
    break;

  case TC_VIOLATION_INCORRECT_WORDARRAY_WIDTH:
    nusmv_assert(CONS == node_get_type(expr));
    fprintf(nusmv_stderr, "in the declaration of '");
    print_node(nusmv_stderr, car(expr));
    fprintf(nusmv_stderr,
            "' either the address or the value width are not in range [1..%d]\n",
            WordNumber_max_width());
    break;

  case TC_VIOLATION_DUPLICATE_CONSTANTS:
    nusmv_assert(CONS == node_get_type(expr));
    fprintf(nusmv_stderr, "duplicate constants in the enum type of variable '");
    print_node(nusmv_stderr, car(expr));
    fprintf(nusmv_stderr, "'\n");
    break;

  default:
    error_unreachable_code(); /* unknown kind of an error */
  } /* switch (errorKind) */

  return isError;
}


/**Function********************************************************************

   Synopsis           [Resets memoizing to be enabled]

   Description [This function is called by high level type checking
   functions to enable memoizing]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
static void type_checker_memoizing_force_enabled(TypeChecker_ptr self)
{ self->memoizing_counter = 0; }


/**Function********************************************************************

   Synopsis           [Resets memoizing, cleaning up all type information]

   Description [This function is called by high level type checking
   functions to reset memoizing, after an error occurs]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
static void type_checker_reset_memoizing(TypeChecker_ptr self)
{
  if (!(self->freshly_cleared)) {
    /* shuts down the memoizing */
    clear_assoc(self->expressionTypeMapping);
  }

  /* resets the memoizing enable counter */
  type_checker_memoizing_force_enabled(self);

  self->freshly_cleared = true;
}

/**Function********************************************************************

   Synopsis           [Trigger for symbols removal in the symbol table]

   Description        [Trigger for symbols removal in the symbol table.
                       When invoked, clears the memoization hash.]

   SideEffects        []

   SeeAlso            []
******************************************************************************/
static void type_checker_remove_symbol_trigger(const SymbTable_ptr st,
                                               const node_ptr sym,
                                               void* arg)
{
  TypeChecker_ptr self = TYPE_CHECKER(arg);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr, "TypeChecker: cache invalidator trigger invoked\n");
  }

  type_checker_reset_memoizing(self);
}


/**AutomaticEnd***************************************************************/
