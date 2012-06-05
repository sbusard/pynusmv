/**CFile***********************************************************************

  FileName    [ltlRewrite.c]

  PackageName [ltl]

  Synopsis    [Rewrite formula to keep track of possible inputs]

  Description [The algorithm to check an LTL formula cannot deal with
  input variables. Thus it is necessary to rewrite LTL formula
  to get rid from input variables. This is done by introducing fresh
  state variables.

  The idea is the following: let's assume we have an LTL formula which
  contains a non-temporal boolean sub-formula Phi over state and input
  variables.

  LTL_REWRITE_STANDARD method:

    The LTL formula is rewritten by substituting a fresh boolean state
    variable sv for Phi and adding a new transition relation TRANS sv
    <-> Phi. For example, LTL formula

      G (s < i);

    becomes

      G sv;

    and the model is augmented by

      VAR sv : boolean;
      TRANS sv <-> (s < i);

    Note 1: new deadlocks are introduced after the rewriting (because
    new vars are assigned a value before the value of input vars are
    known).  For example, with "TRANS s <i" the original model does
    not have a deadlock but after rewriting it does.  For BDD LTL this
    is not a problem because all paths with deadlocks are ignored and
    all original paths are kept by the rewriting.


    Note 2: the validity of an old and a new LTL formulas is the same
    on *infinite* paths. On finite paths the semantics of formulas is
    different because of the deadlocks.  For above example, if there
    is additionally "TRANS s < i" then on infinite paths "G sv" and "G
    s < i" are both valid whereas there is finite path which violate
    "G sv" and there is NO such finite path for "G s<i".

    This thing happens with BMC (which looks for finite path violating
    a formula) vs BDD (which checks only infinite paths). See next
    rewrite method for a possible solution.

  LTL_REWRITE_DEADLOCK_FREE method:

    The LTL formula is rewriten by substituting Phi with "X sv", where
    sv is a fresh boolean state variable, and adding a new transition
    relation "TRANS next(sv) <-> Phi" and a new initial condition
    "INIT sv"; For example, LTL formula

      G (s < i)

    becomes

      G (X sv)

    and the model is augmented by

      VAR sv : boolean;
      INIT sv;
      TRANS next(sv) <-> (s < i);
  ]

  SeeAlso     []

  Author      [Marco Roveri, changed by Andrei Tchaltsev]

  Copyright   [   This file is part of the ``ltl'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK-irst.

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

#include "ltlInt.h"
#include "ltl/ltl.h"
#include "parser/symbols.h"
#include "ltlInt.h"

#include "utils/utils.h"
#include "utils/error.h"
#include "utils/assoc.h"

static char rcsid[] UTIL_UNUSED = "$Id: ltlRewrite.c,v 1.1.2.4.4.8.6.12 2010-02-02 09:47:49 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum***********************************************************************

  Synopsis    [Enumeration of possible sub-expressions kinds]

  Description []

******************************************************************************/
enum LtlInputKind_TAG {
  LTL_INPUT_KIND_STATE, /* non-temporal and without input vars */
  LTL_INPUT_KIND_INPUT, /* non-temporal but with input vars */
  LTL_INPUT_KIND_TEMP, /* temporal (it must be without input vars) */
};
typedef enum LtlInputKind_TAG LtlInputKind;


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

static int ltl_ltlspec_counter = -1;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/* prefix of the artificially created state variables */
#define LTL_VAR_PREFIX "LTL_INPUT_"

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static LtlInputKind ltl_rewrite_input ARGS((SymbTable_ptr symb_table,
                                            node_ptr* expr,
                                            NodeList_ptr new_var_exprs,
                                            const LtlRewriteType rewrite_type));

static node_ptr ltl_create_substitution ARGS((SymbTable_ptr symb_table,
                                              node_ptr expr,
                                              NodeList_ptr new_var_exprs,
                                              const LtlRewriteType rewrite_type));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Rewrites an LTL formula to get rid of input variables
  in it present]

  Description [The function takes an LTL formula and rewrite it such a
  way that it will not contain input variables any more. See the
  description of this file for more details.

  "layer" is the later where new state variables are defined (if it is
  required).

  "init", "invar", "trans" point to expressions corresponding to
  initial condition, invariant and transition relations of the
  hierarchy, respect. This expressions are added new expression if required.

  The returned expressions (the LTL formula and parts of hierarchy)
  are newly created node_ptr constructs and have to be freed by the
  invoker.
  NOTE ABOUT MEMORY: New expressions are created exactly the same way
  as it is done by Compile_FlattenSexpExpandDefine.

  Precondition: input expression has to be already flattened.

  rewrite_type determines the type of rewriting to be perfomed. For
  further details, please see the description of this file.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
node_ptr Ltl_RewriteInput(SymbTable_ptr symb_table, node_ptr expr,
                          SymbLayer_ptr layer,
                          node_ptr* init, node_ptr* invar, node_ptr* trans,
                          const LtlRewriteType rewrite_type)
{
  ListIter_ptr iter;

  NodeList_ptr new_var_exprs = NodeList_create();

  node_ptr result = Compile_FlattenSexpExpandDefine(symb_table, expr, Nil);

  LtlInputKind kind = ltl_rewrite_input(symb_table, &result, new_var_exprs,
                                        rewrite_type);

  /* get rid the input var from the top level expression when
   there are no temporal operators but there are input vars */
  if (LTL_INPUT_KIND_INPUT == kind) {
    result = ltl_create_substitution(symb_table, result, new_var_exprs,
                                     rewrite_type);
  }

  /* declare the new variables and create hierarchy expressions */
  NODE_LIST_FOREACH(new_var_exprs, iter) {
    node_ptr tmp = NodeList_get_elem_at(new_var_exprs, iter);
    node_ptr var = car(tmp);
    node_ptr expr = cdr(tmp);

    SymbLayer_declare_state_var(layer, var,
                                SymbType_create(SYMB_TYPE_BOOLEAN, Nil));

    switch(rewrite_type) {
    case LTL_REWRITE_STANDARD:
      /* currently, we add just TRANS "var = expr".
         See the description at the beginning of the file */
      *trans = new_node(AND, new_node(IFF, var, expr), *trans);
      break;

    case LTL_REWRITE_DEADLOCK_FREE:
      *init = new_node(AND, var, *init);
      *trans = new_node(AND,
                        new_node(IFF, new_node(NEXT, var, Nil), expr),
                        *trans);
      break;

    default: error_unreachable_code();
    }

    free_node(tmp);
  }

  NodeList_destroy(new_var_exprs);

  return result;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Recursively walk over the expressions and returns
  the kind of the expression, i.e. if it is temporal or with input vars.]

  Description [A copy of the provided expression is created and
  returned in the same pointer "expr". The copy may be exact or
  already rewritten (to remove inputs in temporal expressions).

  "new_var_exprs" is a list of pairs (CONS) of a new state var
  introduced during rewriting and an expression associated with that
  state variable.

  Precondition: the expression have to be correctly typed.

  NOTE FOR DEVELOPERS: This function creates new expression using the
  same approach as compileFlattenSexpRecur, i.e. consts and ids are
  find_atom-ed and operations are new_node-ed. Both functions should be
  changed synchronously.]

  SideEffects []

  SeeAlso     [LtlInputKind]

******************************************************************************/
static LtlInputKind ltl_rewrite_input(SymbTable_ptr symb_table,
                                      node_ptr* expr,
                                      NodeList_ptr new_var_exprs,
                                      const LtlRewriteType rw_type)
{
  LtlInputKind kind1, kind2;
  node_ptr expr1, expr2;
  if (Nil == *expr) return LTL_INPUT_KIND_STATE;

  switch (node_get_type(*expr)) {
    /* --- constants ---
       the expression is already find_atom => no need to create a copy */
  case FAILURE:  case TRUEEXP:  case FALSEEXP:
  case NUMBER:  case NUMBER_UNSIGNED_WORD:  case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC:  case NUMBER_REAL:  case NUMBER_EXP:
    /*  nusmv_assert(find_atom(*expr) == *expr); */
    return LTL_INPUT_KIND_STATE;

    /* --- identifier ---
       since the expression is already flattened there is not need
       to resolve the identifier, find_atom it or create a copy. */
  case ATOM:  case DOT:  case ARRAY:  case CONTEXT:
    if (SymbTable_is_symbol_input_var(symb_table, *expr)) return LTL_INPUT_KIND_INPUT;

    if (SymbTable_is_symbol_state_var(symb_table, *expr) ||
        SymbTable_is_symbol_constant(symb_table, *expr) ||
        SymbTable_is_symbol_frozen_var(symb_table, *expr)) return LTL_INPUT_KIND_STATE;

    internal_error("Unknown identifier is met during LTL INPUT REWRITE");

    /* --- unary non-temporal operator ---
       nothing has to be changed */
  case NOT:
  case UMINUS:
    expr1 = car(*expr);
    kind1 = ltl_rewrite_input(symb_table, &expr1, new_var_exprs, rw_type);
    nusmv_assert(Nil == cdr(*expr)); /* consistency check */

    *expr = new_node(node_get_type(*expr), expr1, Nil);
    return kind1;

    /* --- binary non-temporal operators ---
       if one operand has input and other has a temporal op =>
       rewrite operand with input var and return temporal kind.
       If kinds are different "input" and "temporal" kinds wins
       "state".*/

  case TWODOTS: /* This is dealt as a binary operator */
  case AND: case OR: case IMPLIES: case IFF: case XOR: case XNOR:
    expr1 = car(*expr);
    expr2 = cdr(*expr);
    kind1 = ltl_rewrite_input(symb_table, &expr1, new_var_exprs, rw_type);
    kind2 = ltl_rewrite_input(symb_table, &expr2, new_var_exprs, rw_type);

    if (LTL_INPUT_KIND_INPUT == kind1 && LTL_INPUT_KIND_TEMP == kind2) {
      expr1 = ltl_create_substitution(symb_table, expr1, new_var_exprs, rw_type);
      kind1 = LTL_INPUT_KIND_STATE;
    }
    else if (LTL_INPUT_KIND_INPUT == kind2 && LTL_INPUT_KIND_TEMP == kind1) {
      expr2 = ltl_create_substitution(symb_table, expr2, new_var_exprs, rw_type);
      kind2 = LTL_INPUT_KIND_STATE;
    }

    *expr = new_node(node_get_type(*expr), expr1, expr2);

    if (kind1 == kind2) return kind1;
    if (LTL_INPUT_KIND_TEMP == kind1 || LTL_INPUT_KIND_TEMP == kind2) {
      return LTL_INPUT_KIND_TEMP;
    }
    if (LTL_INPUT_KIND_INPUT == kind1 || LTL_INPUT_KIND_INPUT == kind2) {
      return LTL_INPUT_KIND_INPUT;
    }
    /* all the possible kinds have been checked already */
    internal_error("Impossible code");

  case WSIZEOF: case CAST_TOINT:
    /* In this case we do not need to extend the language */
    return LTL_INPUT_KIND_STATE;

  case WRESIZE:
    expr1 = car(*expr);
    expr2 = cdr(*expr);
    kind2 = ltl_rewrite_input(symb_table, &expr1, new_var_exprs, rw_type);
    nusmv_assert(LTL_INPUT_KIND_TEMP != kind2);

    *expr = new_node(node_get_type(*expr), expr1, expr2);

    return kind2;

    /* --- binary non-temporal operators ---
       it is exactly as previous case but the operands cannot have temporal
       operators. It is written as a special case only for debugging purposes.*/
  case CASE: case COLON:
  case EQUAL: case NOTEQUAL:
  case LT: case GT: case LE: case GE:
  case PLUS: case MINUS: case TIMES: case MOD: case DIVIDE:
  case UNION: case SETIN:
  case LSHIFT: case RSHIFT:
  case BIT: case CONCATENATION: case BIT_SELECTION:  case EXTEND:
  case CAST_BOOL:  case CAST_WORD1:  case CAST_SIGNED: case CAST_UNSIGNED:
  case IFTHENELSE:
    expr1 = car(*expr);
    expr2 = cdr(*expr);
    kind1 = ltl_rewrite_input(symb_table, &expr1, new_var_exprs, rw_type);
    kind2 = ltl_rewrite_input(symb_table, &expr2, new_var_exprs, rw_type);
    nusmv_assert(LTL_INPUT_KIND_TEMP != kind1 && LTL_INPUT_KIND_TEMP != kind2);

    *expr = new_node(node_get_type(*expr), expr1, expr2);

    if (LTL_INPUT_KIND_INPUT == kind1 || LTL_INPUT_KIND_INPUT == kind2) {
      return LTL_INPUT_KIND_INPUT;
    }
    return LTL_INPUT_KIND_STATE;

    /*  -- unary temporal operators ---
        if operand has inputs then rewrite it. */
  case OP_NEXT: case OP_PREC: case OP_NOTPRECNOT: case OP_FUTURE:
  case OP_ONCE: case OP_GLOBAL: case OP_HISTORICAL:
    expr1 = car(*expr);
    kind1 = ltl_rewrite_input(symb_table, &expr1, new_var_exprs, rw_type);
    nusmv_assert(Nil == cdr(*expr)); /* consistency check */

    if (LTL_INPUT_KIND_INPUT == kind1) {
      expr1 = ltl_create_substitution(symb_table, expr1, new_var_exprs, rw_type);
    }

    *expr = new_node(node_get_type(*expr), expr1, Nil);
    return LTL_INPUT_KIND_TEMP;

    /* --- binary temporal operators ---
     If any operand has inputs then rewrite it.*/
  case UNTIL: case SINCE: case RELEASES: case TRIGGERED:
    expr1 = car(*expr);
    expr2 = cdr(*expr);
    kind1 = ltl_rewrite_input(symb_table, &expr1, new_var_exprs, rw_type);
    kind2 = ltl_rewrite_input(symb_table, &expr2, new_var_exprs, rw_type);

    if (LTL_INPUT_KIND_INPUT == kind1) {
      expr1 = ltl_create_substitution(symb_table, expr1, new_var_exprs, rw_type);
    }
    if (LTL_INPUT_KIND_INPUT == kind2) {
      expr2 = ltl_create_substitution(symb_table, expr2, new_var_exprs, rw_type);
    }
    *expr = new_node(node_get_type(*expr), expr1, expr2);
    return LTL_INPUT_KIND_TEMP;

  default:
    internal_error("Unexpected expression is met during LTL INPUT REWRITING");
  } /* switch */

  internal_error("Impossible code");
  return -1; /* invalid value */
}


/**Function********************************************************************

  Synopsis    [Creates a new state variable and add a pair <var id, expr>
  to the list "new_var_exprs"]

  Description [The purpose of the function is to create a substitution
  for the given expression in an LTL formula.

  The function returns:
     if rewrite_type is
      LTL_REWRITE_STANDARD : new identifiers.
      LTL_REWRITE_DEADLOCK_FREE: "X new_identifier"
      ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static node_ptr ltl_create_substitution(SymbTable_ptr symb_table,
                                        node_ptr expr,
                                        NodeList_ptr new_var_exprs,
                                        const LtlRewriteType rewrite_type)
{
  node_ptr new_var;
  node_ptr result = Nil;

  /* declare a new variable */
  do {
    char buffer[50]; /* 50 is enough for LTL_VAR_PREFIX and any decimal number */
    int i = snprintf(buffer, 50, "%s%d", LTL_VAR_PREFIX, ++ltl_ltlspec_counter);
    SNPRINTF_CHECK(i, 50);  /* see buffer above */

    new_var = find_node(DOT, Nil, sym_intern(buffer));
  } while (SymbTable_is_symbol_declared(symb_table, new_var));

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) { /* debug info */
    fprintf(nusmv_stdout, "Associating ");
    print_node(nusmv_stdout, new_var);
    fprintf(nusmv_stdout, " to ");
    print_node(nusmv_stdout, expr);
    fprintf(nusmv_stdout, "\n");
  }

  NodeList_append(new_var_exprs, cons(new_var, expr));

  switch(rewrite_type) {
  case LTL_REWRITE_STANDARD:
    result = new_var;
    break;

  case LTL_REWRITE_DEADLOCK_FREE:
    result = new_node(OP_NEXT, new_var, Nil);
    break;

  default: error_unreachable_code();
  }

  return result;

}

