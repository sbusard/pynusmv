/**Cfile***********************************************************************

FileName    [pslNode.c]

PackageName [parser.psl]

Synopsis    [Implementation of the PslNode structure]

Description []

SeeAlso     [psl_node.h]

Author      [Roberto Cavada, Marco Roveri]

Copyright   [
This file is part of the ``parser.psl'' package of NuSMV version 2.
Copyright (C) 2005 by FBK-irst.

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

#include "pslNode.h"
#include "pslExpr.h"
#include "psl_symbols.h"
#include "pslInt.h"

#include "parser/symbols.h"
#include "compile/compile.h"
#include "utils/ustring.h"

static char rcsid[] UTIL_UNUSED = "$Id: pslNode.c,v 1.1.4.18.4.15 2010-01-04 23:01:16 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE* nusmv_stderr;
extern FILE* nusmv_stdout;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static boolean psl_node_is_equal ARGS((PslNode_ptr e, PslNode_ptr f));
static boolean psl_node_is_star_free ARGS((PslNode_ptr expr));
static boolean psl_node_is_unbound_star_free ARGS((PslNode_ptr expr));
static boolean psl_node_is_emptystar_free ARGS((PslNode_ptr expr));
static boolean psl_node_is_handled_next ARGS((PslNode_ptr expr));
static boolean psl_node_is_handled_fl_op ARGS((PslOp op));
static boolean psl_node_is_fl_op ARGS((PslOp op));
static boolean psl_node_is_obe_op ARGS((PslOp op));

static boolean
psl_node_is_handled_sere ARGS((PslNode_ptr e, boolean toplevel));

static boolean
psl_node_is_propositional ARGS((const PslNode_ptr expr,
                                boolean accept_next));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [Casts a PslNode_ptr to a node_ptr]

Description        [The returned structure will still contain operators
in the SMV parser's domain]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr PslNode_convert_from_node_ptr(node_ptr expr)
{
  if (expr == Nil) return PSL_NULL;
  return (PslNode_ptr) expr;
}


/**Function********************************************************************

Synopsis           [Casts a node_ptr to a PslNode_ptr]

Description        [The returned structure will still contain operators
in the PSL parser's domain]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
node_ptr PslNode_convert_to_node_ptr(PslNode_ptr expr)
{
  if (expr == PSL_NULL) return Nil;
  return (node_ptr) expr;
}


/**Function********************************************************************

Synopsis           [Creates a psl node that represents a contestualized
node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr PslNode_new_context(PslNode_ptr ctx, PslNode_ptr node)
{
  return psl_new_node(CONTEXT, ctx, node);
}


/**Function********************************************************************

Synopsis           [Returns true iff given expression can be translated
into LTL.]

Description        []

SideEffects        [None]

SeeAlso            [optional]

******************************************************************************/
boolean PslNode_is_handled_psl(PslNode_ptr e)
{
  PslOp op;

  if (e == PSL_NULL) return true;

  op = psl_node_get_op(e);

  /* no sere */
  if (PslNode_is_propositional(e)) return true;

  /* no obe expressions */
  if (psl_node_is_obe_op(op)) return false;

  /* weak suffix operator */
  if (psl_node_is_suffix_implication_weak(e)) {
    fprintf(nusmv_stderr, "In PSL expression '");
    print_node(nusmv_stderr, e);
    fprintf(nusmv_stderr, "'\nWeak suffix implications are not currently supported.\n");
    return false;
  }

  /* operators we do not deal with */
  if (!psl_node_is_handled_fl_op(op)) {
    fprintf(nusmv_stderr, "In PSL expression '");
    print_node(nusmv_stderr, e);
    fprintf(nusmv_stderr, "'\nPSL operator not currently supported.\n");
    return false;
  }

  if (psl_node_is_extended_next(e) && !psl_node_is_handled_next(e)) {
    fprintf(nusmv_stderr, "In PSL expression '");
    print_node(nusmv_stderr, e);
    fprintf(nusmv_stderr, "'\nPSL next expression contains "\
            "an unsupported feature.\n");
    return false;
  }

  if (psl_node_is_suffix_implication(e)) {
    PslNode_ptr pre = psl_node_suffix_implication_get_premise(e);
    PslNode_ptr con = psl_node_suffix_implication_get_consequence(e);

    if (!psl_node_is_unbound_star_free(pre)) {
      fprintf(nusmv_stderr, "In PSL expression '");
      print_node(nusmv_stderr, e);
      fprintf(nusmv_stderr, "'\nPremise in suffix implication "\
              "contains an unsupported feature.\n");
      return false;
    }

    if (!PslNode_is_handled_psl(pre) || !PslNode_is_handled_psl(con)) {
      return false;
    }

    return true;
  }

  if (op==PSL_SERE || op==PSL_SERECONCAT || op==PSL_SEREFUSION ||
      op==PSL_SERECOMPOUND || op==PSL_SEREREPEATED) {
    if (psl_node_is_handled_sere(e, true)) return true;

    fprintf(nusmv_stderr, "In PSL expression '");
    print_node(nusmv_stderr, e);
    fprintf(nusmv_stderr, "'\nPSL SERE expression contains an "\
            "unsupported feature.\n");
    return false;
  }

  /* applies to binary and unary FL operators */
  return PslNode_is_handled_psl(psl_node_get_left(e)) &&
    PslNode_is_handled_psl(psl_node_get_right(e));
}



/**Function********************************************************************

Synopsis           [Checks for a formula being a propositional formula]

Description        [Checks for a formula being a propositional formula]

SideEffects        [None]

SeeAlso            [PslNode_is_trans_propositional]

******************************************************************************/
boolean PslNode_is_propositional(const PslNode_ptr expr)
{
  return psl_node_is_propositional(expr, false);
}

/**Function********************************************************************

Synopsis           [Checks for a formula being a propositional formula]

Description        [Checks for a formula being a propositional formula,
                    next operator here leaves the formula propositional]

SideEffects        [None]

SeeAlso            [PslNode_is_propositional]

******************************************************************************/
boolean PslNode_is_trans_propositional(const PslNode_ptr expr)
{
  return psl_node_is_propositional(expr, true);
}

/**Function********************************************************************

Synopsis           [Checks if a propositional formula contains a next]

Description        [Checks for a formula being a propositional formula]

SideEffects        [None]

SeeAlso            [optional]

******************************************************************************/
boolean PslNode_propositional_contains_next(const PslNode_ptr expr)
{
  boolean result = false;

  if (expr == PSL_NULL) return false;

  if (psl_node_is_leaf(expr) || psl_node_is_id(expr)) return false;

  switch (psl_node_get_op(expr)) {

  case NEXT:
    result = true;
    break;

  case CONTEXT:
    result = PslNode_propositional_contains_next(psl_node_get_right(expr));
    break;

  case MINUS:
  case PLUS:
  case NOT:
  case PSL_TILDE:
    result = PslNode_propositional_contains_next(psl_node_get_left(expr));
    break;

  case UNION:
  case SETIN:
  case TIMES:
  case DIVIDE:
  case MOD:
  case PSL_EQEQ:
  case EQUAL:
  case NOTEQUAL:
  case LT:
  case LE:
  case GT:
  case GE:
  case AND:
  case OR:
  case PSL_OR:
  case XOR:
  case PSL_CARET:
  case IFF:
  case IMPLIES:
    result = PslNode_propositional_contains_next(psl_node_get_left(expr)) ||
      PslNode_propositional_contains_next(psl_node_get_right(expr));
    break;

  case CASE:
    result = PslNode_propositional_contains_next(psl_node_get_case_cond(expr)) ||
      PslNode_propositional_contains_next(psl_node_get_case_then(expr)) ||
      PslNode_propositional_contains_next(psl_node_get_case_next(expr));
    break;

  case PSL_ITE:
    result = PslNode_propositional_contains_next(psl_node_get_ite_cond(expr)) ||
      PslNode_propositional_contains_next(psl_node_get_ite_then(expr)) ||
      PslNode_propositional_contains_next(psl_node_get_ite_else(expr));
    break;

  case PSL_REPLPROP:
    result = PslNode_propositional_contains_next(psl_node_repl_prop_get_property(expr));
    break;

  default:
    result = false;
  }

  return result;
}


/**Function********************************************************************

Synopsis           [Checks for a formula being an CTL formula]

Description        [Checks for a formula being an CTL formula]

SideEffects        [None]

SeeAlso            [optional]

******************************************************************************/
boolean PslNode_is_obe(const PslNode_ptr expr)
{
  boolean result = false;

  if (expr == PSL_NULL) return true;

  /* no FL expressions */
  if (psl_node_is_fl_op(psl_node_get_op(expr))) return false;

  switch (psl_node_get_op(expr)) {
  case CONTEXT:
    result = PslNode_is_obe(psl_node_get_right(expr));
    break;

    /* id */
  case DOT:
  case ATOM:
  case ARRAY:

    /* boolean */
  case TRUEEXP:
  case FALSEEXP:

  case FAILURE:

    /* numbers */
  case NUMBER:
    /* primary */
  case MINUS:
  case PLUS:
    /* binary operators */
  case UNION:
  case SETIN:
  case TIMES:
  case DIVIDE:
  case MOD:
  case PSL_EQEQ:
  case EQUAL:
  case NOTEQUAL:
  case LT:
  case LE:
  case GT:
  case GE:
    result = true;
    break;
    /* boolean binary operators */
  case NOT:
  case PSL_TILDE:
    result = PslNode_is_obe(psl_node_get_left(expr));
    break;
  case AND:
  case OR:
  case PSL_OR:
  case PSL_CARET:
  case XOR:
  case IFF:
  case IMPLIES:
    result = PslNode_is_obe(psl_node_get_left(expr)) &&
      PslNode_is_obe(psl_node_get_right(expr));
    break;
    /* next operators */
  case AX:
  case EX:
    /* globally operators */
  case AG:
  case EG:
    /* eventually operators */
  case AF:
  case EF:
    /* Until operators */
    result = PslNode_is_obe(psl_node_get_left(expr));
    break;
    /* until* operators */
  case EU:
  case AU:
    result = PslNode_is_obe(psl_node_get_left(expr)) &&
      PslNode_is_obe(psl_node_get_right(expr));
    break;

    /* case: */
  case CASE:
    result = PslNode_is_obe(psl_node_get_case_cond(expr)) &&
      PslNode_is_obe(psl_node_get_case_then(expr)) &&
      PslNode_is_obe(psl_node_get_case_next(expr));
    break;

  case PSL_ITE:
    result = PslNode_is_obe(psl_node_get_ite_cond(expr)) &&
      PslNode_is_obe(psl_node_get_ite_then(expr)) &&
      PslNode_is_obe(psl_node_get_ite_else(expr));
    break;

  case PSL_REPLPROP:
    result = PslNode_is_obe(psl_node_repl_prop_get_property(expr));
    break;

  default:
    result = false;
  }

  return result;
}


/**Function********************************************************************

Synopsis           [Checks for a formula being an LTL formula]

Description        [Checks for a formula being an LTL formula]

SideEffects        [None]

SeeAlso            [optional]

******************************************************************************/
boolean PslNode_is_ltl(const PslNode_ptr expr)
{
  boolean result = false;

  if (expr == PSL_NULL) return true;

  /* no obe expressions */
  if (psl_node_is_obe_op(psl_node_get_op(expr))) return false;

  switch (psl_node_get_op(expr)) {

  case CONTEXT:
    result = PslNode_is_ltl(psl_node_get_right(expr));
    break;

    /* id */
  case DOT:
  case ATOM:
  case ARRAY:

    /* boolean */
  case TRUEEXP:
  case FALSEEXP:

  case FAILURE:

    /* numbers */
  case NUMBER:
    /* primary */
  case MINUS:
  case PLUS:
    /* binary operators */
  case UNION:
  case SETIN:
  case TIMES:
  case DIVIDE:
  case MOD:
  case PSL_EQEQ:
  case EQUAL:
  case NOTEQUAL:
  case LT:
  case LE:
  case GT:
  case GE:
    result = true;
    break;

  case NOT:
  case PSL_TILDE:
    result = PslNode_is_ltl(psl_node_get_left(expr));
    break;
    /* boolean binary operators */
  case AND:
  case OR:
  case PSL_OR:
  case PSL_CARET:
  case XOR:
  case IFF:
  case IMPLIES:
    result = PslNode_is_ltl(psl_node_get_left(expr)) &&
      PslNode_is_ltl(psl_node_get_right(expr));
    break;

    /* case: */
  case CASE:
    result = PslNode_is_ltl(psl_node_get_case_cond(expr)) &&
      PslNode_is_ltl(psl_node_get_case_then(expr)) &&
      PslNode_is_ltl(psl_node_get_case_next(expr));
    break;

  case PSL_ITE:
    result = PslNode_is_ltl(psl_node_get_ite_cond(expr)) &&
      PslNode_is_ltl(psl_node_get_ite_then(expr)) &&
      PslNode_is_ltl(psl_node_get_ite_else(expr));
    break;

    /* next* operators */
  case OP_NEXT:
  case PSL_X:
  case PSL_XBANG:
  case PSL_NEXT:
  case PSL_NEXTBANG:
  case PSL_NEXT_E:
  case PSL_NEXT_EBANG:
  case PSL_NEXT_A:
  case PSL_NEXT_ABANG:
  case PSL_NEXT_EVENT:
  case PSL_NEXT_EVENTBANG:
  case PSL_NEXT_EVENT_E:
  case PSL_NEXT_EVENT_EBANG:
  case PSL_NEXT_EVENT_A:
  case PSL_NEXT_EVENT_ABANG:
    {
      PslNode_ptr l = psl_node_get_left(expr);
      PslNode_ptr r = psl_node_get_right(expr);

      result = PslNode_is_ltl(l);

      if (result && (r != PSL_NULL)) {

        nusmv_assert(psl_node_get_op(r) == COLON);

        /* checks the boolean expression if there is any */
        if (psl_node_get_right(r) != PSL_NULL) {
          result = PslNode_is_ltl(psl_node_get_right(r));
        }

        /* Here there is either an expression or a range */
        if (result && (psl_node_get_left(r) != PSL_NULL)) {
          switch (psl_node_get_op(psl_node_get_left(r))) {
          case PSL_RANGE:
            break;
          default:
            result = result && PslNode_is_ltl(psl_node_get_left(r));
            break;
          }
        }
      }
    }
    break; /* end of next operators case */

    /* eventually operators */
  case OP_FUTURE:
    /* never */
  case PSL_NEVER:
    /* globally */
  case OP_GLOBAL:
  case PSL_ALWAYS:
    /* eventually */
  case PSL_EVENTUALLYBANG:
    result = PslNode_is_ltl(psl_node_get_left(expr));
    break;
    /* until* operators */
  case UNTIL:
  case PSL_W:
  case PSL_UNTILBANG:
  case PSL_UNTIL:
  case PSL_UNTIL_:
  case PSL_UNTILBANG_:
  case PSL_BEFORE:
  case PSL_BEFOREBANG:
  case PSL_BEFORE_:
  case PSL_BEFOREBANG_:
    result = PslNode_is_ltl(psl_node_get_left(expr)) &&
      PslNode_is_ltl(psl_node_get_right(expr));
    break;
  case PSL_REPLPROP:
    result = PslNode_is_ltl(psl_node_repl_prop_get_property(expr));
    break;

  default:
    result = false;
  }

  return result;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [Creates a new PSL node, re-using already an existing
node if there is one]

Description [WARNING: If this function is being called to build a
 branch of the parse-tree from a branch coming from the parsing phase
 (i.e. it is a token, and not a symbol), the token *must* be converted
 to a PSL node by calling psl_conv_op(TOK2PSL, op) ]

SideEffects        [None]

SeeAlso            [optional]

******************************************************************************/
PslNode_ptr psl_new_node(PslOp op, PslNode_ptr left, PslNode_ptr right)
{
  return (PslNode_ptr) find_node((int) op,
                                 (node_ptr) left,
                                 (node_ptr) right);
}


/**Function********************************************************************

Synopsis           [Returns the given expression's top level operator]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslOp psl_node_get_op(PslNode_ptr n)
{ return (PslOp) node_get_type(((node_ptr) n)); }


/**Function********************************************************************

Synopsis           [Returns the given expression's left branch]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_left(PslNode_ptr n)
{ return (PslNode_ptr) car((node_ptr) n); }


/**Function********************************************************************

Synopsis           [Returns the given expression's right branch]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_right(PslNode_ptr n)
{ return (PslNode_ptr) cdr((node_ptr) n); }


/**Function********************************************************************

Synopsis           [Sets the given expression's left branch]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
void psl_node_set_left(PslNode_ptr n, PslNode_ptr l)
{ setcar((node_ptr) n, (node_ptr) l); }


/**Function********************************************************************

Synopsis           [Sets the given expression's right branch]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
void psl_node_set_right (PslNode_ptr n, PslNode_ptr r)
{ setcdr((node_ptr) n, (node_ptr) r); }


/**Function********************************************************************

Synopsis           [Creates a new TRUE node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_make_true()
{ return psl_new_node(TRUEEXP, PSL_NULL, PSL_NULL); }


/**Function********************************************************************

Synopsis           [Checks if a node is a TRUE node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_true(PslNode_ptr self)
{ return psl_node_get_op(self)==TRUEEXP; }


/**Function********************************************************************

Synopsis           [Creates a new FALSE node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_make_false()
{ return psl_new_node(FALSEEXP, PSL_NULL, PSL_NULL); }


/**Function********************************************************************

Synopsis           [Checks if a node is a FALSE node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_false(PslNode_ptr self)
{ return psl_node_get_op(self)==FALSEEXP; }


/**Function********************************************************************

Synopsis           [Returns true if the given expression is a SERE]

Description        [A SERE can be in the form {a}, {a};{b}, {a}:{b},
{a}\[*\], {a}\[+\], {a}|{b}, {a}&{a}, {a}&&{b} ]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_sere(PslNode_ptr expr)
{
  PslOp op;
  if (expr == PSL_NULL) return false;

  op = psl_node_get_op(expr);

  return (op == PSL_SERE || op == PSL_SERECONCAT ||
          op == PSL_SEREFUSION || op == PSL_SEREREPEATED ||
          op == PSL_SERECOMPOUND);
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression is a SERE in the form {a}]

Description        []

SideEffects        [None]

SeeAlso            [psl_node_is_sere]

******************************************************************************/
boolean psl_node_is_serebrackets(PslNode_ptr expr)
{
  PslOp op;
  if (expr == PSL_NULL) return false;

  op = psl_node_get_op(expr);

  return (op == PSL_SERE);
}


/**Function********************************************************************

Synopsis           [Returns the count of a starred sere.]

Description        [Returned value can be either a positive integer value, or
the constant PSL_EMPTYSTAR to represent an empty starred sere.]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_star_get_count(const PslNode_ptr e)
{
  nusmv_assert(psl_node_sere_is_star(e));

  if (psl_node_get_right(e) == PSL_NULL) return PSL_EMPTYSTAR;
  else {
    PslNode_ptr nc = psl_node_get_right(e);
    return nc;
  }
}


/**Function********************************************************************

Synopsis [Returns true if the given starred sere can be handled by the
system. ]

Description        [precond: expr must be a repeated sere]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_handled_star(PslNode_ptr expr, boolean toplevel)
{
  PslNode_ptr repeated_expr;

  /* precond: expr must be a repeated sere */
  nusmv_assert(psl_node_sere_is_repeated(expr));

  /* [* count] are handled if count is a number */
  if (psl_node_sere_is_star_count(expr)) {
    PslNode_ptr count = psl_node_sere_star_get_count(expr);
    if (psl_node_is_number(count)) {
      /* star count might be applied to unbound stars */
      return (psl_node_number_get_value(count) > 0) &&
        psl_node_is_handled_sere(psl_node_sere_repeated_get_expr(expr),
                                 toplevel);
    }
    fprintf(nusmv_stderr, "In expression ");
    print_node(nusmv_stderr, expr);
    error_expected_number();
  }

  /* r[= count] and r[->] are not handled */
  if (psl_node_sere_is_stareq(expr) || psl_node_sere_is_starminusgt(expr)) {
    return false;
  }

  /* toplevel stars are not handled, only plus are handled at toplevel */
  if (toplevel && psl_node_sere_is_star(expr)) return false;

  /* here we are sure expr is a repeated sere, and not a toplevel *,
     a [*Range], a [= count], [->] */

  repeated_expr = psl_node_sere_repeated_get_expr(expr);

  /* from here on we are sure we are not toplevel and we handle only
     [*] and [+] that are stand alone or applied to propositionals */
  return ((repeated_expr == PSL_NULL) ||
          PslNode_is_propositional(repeated_expr));
}


/**Function********************************************************************

Synopsis [Returns true if the given sere contains a single
propositional expression]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_propositional(PslNode_ptr e)
{
  if (e==NULL) return false;
  if (psl_node_get_op(e)!=PSL_SERE) return false;
  if (PslNode_is_propositional(psl_node_get_left(e))) return true;
  return psl_node_sere_is_propositional(psl_node_get_left(e));
}


/**Function********************************************************************

Synopsis           [Returns true if the given expr is a repeated sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_repeated(PslNode_ptr e)
{
  return (e != PSL_NULL) && (psl_node_get_op(e)==PSL_SEREREPEATED);
}


/**Function********************************************************************

Synopsis [Returns true if the given expr is a starred repeated sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_star(PslNode_ptr e)
{
  return psl_node_sere_is_repeated(e) &&
    (psl_node_get_op(psl_node_get_left(e))==PSL_LBSPLAT);
}

/**Function********************************************************************

Synopsis [Returns true if the given expr is a starred-eq repeated sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_stareq(PslNode_ptr e)
{
  return psl_node_sere_is_repeated(e) &&
    psl_node_get_op(psl_node_get_left(e))==PSL_LBEQ;
}


/**Function********************************************************************

Synopsis [Returns true if the given expr is a starred-minusgt repeated sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_starminusgt(PslNode_ptr e)
{
  return psl_node_sere_is_repeated(e) &&
    psl_node_get_op(psl_node_get_left(e))==PSL_LBMINUSGT;
}


/**Function********************************************************************

Synopsis [Returns true if the given expr is in the form <empty>\[*\],
with or without a counter.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_standalone_star(PslNode_ptr e)
{
  return psl_node_sere_is_star(e) &&
    psl_node_sere_repeated_get_expr(e) == PSL_NULL;
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression a plus repeated sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_plus(PslNode_ptr e)
{
  return psl_node_sere_is_repeated(e) &&
    (psl_node_get_op(psl_node_get_left(e))==PSL_LBPLUSRB);
}


/**Function********************************************************************

Synopsis           [Returns true if the given repeated sere is in the form
<empty>\[+\]]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_standalone_plus(PslNode_ptr e)
{
  return psl_node_sere_is_plus(e) &&
    (psl_node_sere_repeated_get_expr(e) == PSL_NULL);
}


/**Function********************************************************************

Synopsis           [Returns true if the given starred repeated sere as also
a counter]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_star_count(PslNode_ptr e)
{
  if (psl_node_sere_is_star(e)) {
    PslNode_ptr count = psl_node_sere_star_get_count(e);
    return (count != PSL_EMPTYSTAR);
  }
  return false;
}

/**Function********************************************************************

Synopsis           [Returns true if the given expr is a star sere with
count zero]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_star_count_zero(PslNode_ptr e)
{
  return (e != PSL_NULL) && (psl_node_sere_is_star_count(e)) &&
    (psl_node_is_number(psl_node_sere_star_get_count(e))) &&
    (0 == psl_node_number_get_value(psl_node_sere_star_get_count(e)));
}

/**Function********************************************************************

Synopsis           [Returns the expression in a propositional sere.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_propositional_get_expr(PslNode_ptr e)
{
  PslNode_ptr expr;

  nusmv_assert(psl_node_sere_is_propositional(e));

  expr = psl_node_get_left(e);

  if (expr == PSL_NULL) return PSL_NULL;

  /* getting rid of { } */
  while (psl_node_get_op(expr)==PSL_SERE) expr = psl_node_get_left(expr);

  /* here expr it is not a SERE */
  return expr;
}

/**Function********************************************************************

Synopsis           [Returns the left operand of a compound sere.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_compound_get_left(PslNode_ptr e)
{
  PslNode_ptr left;

  nusmv_assert(psl_node_is_sere_compound_binary(e));
  nusmv_assert(psl_node_get_left(e) != PSL_NULL);

  left = psl_node_get_left(psl_node_get_left(e));

  return left;
}

/**Function********************************************************************

Synopsis           [Returns the right operand of a compound sere.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_compound_get_right(PslNode_ptr e)
{
  PslNode_ptr right;

  nusmv_assert(psl_node_is_sere_compound_binary(e));
  nusmv_assert(psl_node_get_left(e) != PSL_NULL);

  right = psl_node_get_right(psl_node_get_left(e));

  return right;
}



/**Function********************************************************************

Synopsis [Returns the repeated expression associated to the repeated
sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_repeated_get_expr(PslNode_ptr e)
{
  PslNode_ptr expr;

  nusmv_assert(psl_node_sere_is_repeated(e));

  expr = psl_node_get_left(psl_node_get_left(e));

  if (expr == PSL_NULL) return PSL_NULL;

  /* getting rid of { } */
  while (psl_node_get_op(expr)==PSL_SERE) expr = psl_node_get_left(expr);

  /* here expr it is not a SERE */
  return expr;
}

/**Function********************************************************************

Synopsis [Returns the count associated to the repeated sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_repeated_get_count(PslNode_ptr e)
{
  PslNode_ptr cnt = PSL_EMPTYSTAR;

  nusmv_assert(psl_node_sere_is_repeated(e));

  if (PSL_NULL != psl_node_get_right(e)) {
    cnt = psl_node_get_right(e);
  }

  return cnt;
}

/**Function********************************************************************

Synopsis [Returns the count associated to the repeated sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslOp psl_node_sere_repeated_get_op(PslNode_ptr e)
{
  nusmv_assert(psl_node_sere_is_repeated(e));

  return psl_node_get_op(psl_node_get_left(e));
}



/**Function********************************************************************

Synopsis           [Maker for a propositional sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr
psl_node_make_sere_propositional(PslNode_ptr expr)
{
  return psl_new_node(PSL_SERE, expr, PSL_NULL);
}

/**Function********************************************************************

Synopsis           [Maker for a concatenation sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr
psl_node_make_sere_concat(PslNode_ptr seq1, PslNode_ptr seq2)
{
  return psl_new_node(PSL_SERECONCAT, seq1, seq2) ;
}

/**Function********************************************************************

Synopsis           [Maker for a star sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr
psl_node_make_sere_star(PslNode_ptr seq)
{
  return psl_new_node(PSL_SEREREPEATED,
                      psl_new_node(PSL_LBSPLAT, seq, PSL_NULL), PSL_NULL);
}

/**Function********************************************************************

Synopsis           [Getter for a star sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_star_get_starred(PslNode_ptr e)
{
  nusmv_assert(psl_node_sere_is_star(e));
  return psl_node_get_left(psl_node_get_left(e));
}

/**Function********************************************************************

Synopsis           [Maker for a && sere]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr
psl_node_make_sere_2ampersand(PslNode_ptr seq1, PslNode_ptr seq2)
{
  return psl_node_make_sere_compound(seq1, PSL_AMPERSANDAMPERSAND, seq2);
}

/**Function********************************************************************

Synopsis           [Maker for the sere compound]

Description        [Warning: the operator must be a symbol, not a token.
This means that psl_conv_op must be called to convert tokens before.]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr
psl_node_make_sere_compound(PslNode_ptr seq1, PslOp op, PslNode_ptr seq2)
{
  return psl_new_node(PSL_SERECOMPOUND,
                      psl_new_node(op, seq1, seq2),
                      PSL_NULL);
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression is a sere compound]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_sere_compound_binary(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;
  return (psl_node_get_op(e) == PSL_SERECOMPOUND);
}


/**Function********************************************************************

Synopsis [Returns true if the given expression is a suffix
implication]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_suffix_implication(PslNode_ptr expr)
{
  PslOp op;

  if (expr == PSL_NULL) return false;

  op = psl_node_get_op(expr);
  return (op == PSL_PIPEMINUSGT || op == PSL_PIPEEQGT);
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression is a weak suffix
implication]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_suffix_implication_weak(PslNode_ptr expr)
{
  return psl_node_is_suffix_implication(expr) &&
    (psl_node_get_right(expr) == PSL_NULL);
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression is a strong suffix
implication]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_suffix_implication_strong(PslNode_ptr expr)
{
  return psl_node_is_suffix_implication(expr) &&
    (psl_node_get_right(expr) != PSL_NULL) &&
    (psl_node_get_op(psl_node_get_right(expr)) == NOT);
}


/**Function********************************************************************

Synopsis           [Returns the premise of the given suffix implication]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr
psl_node_suffix_implication_get_premise(PslNode_ptr e)
{
  nusmv_assert(psl_node_is_suffix_implication(e));
  return psl_node_get_left(psl_node_get_left(e));
}


/**Function********************************************************************

Synopsis           [Returns the consequence of the given suffix implication]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr
psl_node_suffix_implication_get_consequence(PslNode_ptr e)
{
  nusmv_assert(psl_node_is_suffix_implication(e));
  return psl_node_get_right(psl_node_get_left(e));
}


/**Function********************************************************************

Synopsis [Returns true if there are no holes in the given concat sere
to be filled in.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_concat_holes_free(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;

  if (psl_node_get_op(e)==PSL_SERE && !psl_node_is_sere(psl_node_get_left(e))) {
    return (psl_node_get_op(psl_node_get_left(e)) != PSL_UNTILBANG &&
            psl_node_get_op(psl_node_get_left(e)) != PSL_XBANG &&
            psl_node_get_op(psl_node_get_left(e)) != PSL_EVENTUALLYBANG);
  }

  if (psl_node_get_op(e)==PSL_SERE) {
    return psl_node_sere_is_concat_fusion_holes_free(psl_node_get_left(e));
  }

  if (psl_node_get_op(e)==PSL_SERECONCAT) {
    return psl_node_sere_is_concat_holes_free(psl_node_get_left(e)) &&
      psl_node_sere_is_concat_holes_free(psl_node_get_right(e));
  }

  return false;
}


/**Function********************************************************************

Synopsis [Returns true if the given expression is a concat or fusion
sere.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_concat_fusion(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;

  if (psl_node_get_op(e)==PSL_SERE &&
      !psl_node_is_sere(psl_node_get_left(e))) {
    return true;
  }

  if (psl_node_get_op(e)==PSL_SERE ) {
    return psl_node_sere_is_concat_fusion(psl_node_get_left(e));
  }

  if (psl_node_get_op(e)==PSL_SERECONCAT ||
      psl_node_get_op(e)==PSL_SEREFUSION) {
    return psl_node_sere_is_concat_fusion(psl_node_get_left(e)) &&
      psl_node_sere_is_concat_fusion(psl_node_get_right(e));
  }

  return false;
}

/**Function********************************************************************

Synopsis           [Returns true if the given expression is a concat.]

Description        [Returns true if the top level operator is a concat.]

SideEffects        [None]

SeeAlso            [psl_node_sere_is_concat_fusion,
                    psl_node_sere_is_fusion]

******************************************************************************/
boolean psl_node_sere_is_concat(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;

  if (psl_node_get_op(e)==PSL_SERE &&
      psl_node_get_left(e)!=NULL &&
      psl_node_get_op(psl_node_get_left(e))==PSL_SERECONCAT) {
    return true;
  }

  if (psl_node_get_op(e)==PSL_SERECONCAT) {
    return true;
  }

  return false;
}

/**Function********************************************************************

Synopsis           [Returns the left operand of a concat.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_concat_get_left(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;

  if (psl_node_get_op(e)==PSL_SERE &&
      psl_node_get_left(e)!=NULL &&
      psl_node_get_op(psl_node_get_left(e))==PSL_SERECONCAT) {
    return psl_node_get_left(psl_node_get_left(e));
  } else
    return psl_node_get_left(e);
}

/**Function********************************************************************

Synopsis           [Returns the right operand of a concat.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_concat_get_right(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;

  if (psl_node_get_op(e)==PSL_SERE &&
      psl_node_get_left(e)!=NULL &&
      psl_node_get_op(psl_node_get_left(e))==PSL_SERECONCAT) {
    return psl_node_get_right(psl_node_get_left(e));
  } else
    return psl_node_get_right(e);
}


/**Function********************************************************************

Synopsis           [Returns the leftmost element of a concat sere]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_concat_get_leftmost(PslNode_ptr e)
{
  if (psl_node_sere_is_concat(e))
    return psl_node_sere_concat_get_leftmost(psl_node_sere_concat_get_left(e));
  else
    return e;
}

/**Function********************************************************************

Synopsis           [Returns the rightmost element of a concat sere]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_concat_get_rightmost(PslNode_ptr e)
{
  if (psl_node_sere_is_concat(e))
    return
      psl_node_sere_concat_get_rightmost(psl_node_sere_concat_get_right(e));
  else
    return e;
}

/**Function********************************************************************

Synopsis           [Cuts the leftmost element of a concat sere]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_concat_cut_leftmost(PslNode_ptr e)
{
  PslNode_ptr left;
  PslNode_ptr right;
  nusmv_assert(psl_node_sere_is_concat(e));
  left = psl_node_sere_concat_get_left(e);
  right = psl_node_sere_concat_get_right(e);
  if (psl_node_sere_is_concat(left))
    return psl_node_make_sere_concat(psl_node_sere_concat_cut_leftmost(left),
                                     right);
  else
    return right;
}

/**Function********************************************************************

Synopsis           [Returns the left operand of a fusion.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_fusion_get_left(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;

  if (psl_node_get_op(e)==PSL_SERE &&
      psl_node_get_left(e)!=NULL &&
      psl_node_get_op(psl_node_get_left(e))==PSL_SEREFUSION) {
    return psl_node_get_left(psl_node_get_left(e));
  } else
    return psl_node_get_left(e);
}

/**Function********************************************************************

Synopsis           [Returns the right operand of a fusion.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_sere_fusion_get_right(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;

  if (psl_node_get_op(e)==PSL_SERE &&
      psl_node_get_left(e)!=NULL &&
      psl_node_get_op(psl_node_get_left(e))==PSL_SEREFUSION) {
    return psl_node_get_right(psl_node_get_left(e));
  } else
    return psl_node_get_right(e);
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression is a fusion.]

Description        [Returns true if the top level operator is a fusion.]

SideEffects        [None]

SeeAlso            [psl_node_sere_is_concat_fusion,
                    psl_node_sere_is_concat]

******************************************************************************/
boolean psl_node_sere_is_fusion(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;

  if (psl_node_get_op(e)==PSL_SERE &&
      psl_node_get_left(e)!=NULL &&
      psl_node_get_op(psl_node_get_left(e))==PSL_SEREFUSION) {
    return true;
  }

  if (psl_node_get_op(e)==PSL_SEREFUSION) {
    return true;
  }

  return false;
}

/**Function********************************************************************

Synopsis           [Returns true if the given expression is an or.]

Description        [Duplicate of psl_node_sere_is_disj.]

SideEffects        [None]

SeeAlso            [psl_node_sere_is_disj]

******************************************************************************/
boolean psl_node_sere_is_or(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;
  if (psl_node_get_left(e) == PSL_NULL) return false;
  if (psl_node_get_op(e) == PSL_SERE) {
    return psl_node_sere_is_or(psl_node_get_left(e));
  }
  return ((psl_node_get_op(e) == PSL_SERECOMPOUND) &&
          (psl_node_get_op(psl_node_get_left(e)) == OR));
}

/**Function********************************************************************

Synopsis [[Returns true if there are no holes in the given
fusion/concat sere to be filled in.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_concat_fusion_holes_free(PslNode_ptr e)
{
  if (e == PSL_NULL) return false;

  if (psl_node_get_op(e)==PSL_SERE &&
      !psl_node_is_sere(psl_node_get_left(e))) {
    return (psl_node_get_op(psl_node_get_left(e)) != PSL_UNTILBANG &&
            psl_node_get_op(psl_node_get_left(e)) != PSL_XBANG &&
            psl_node_get_op(psl_node_get_left(e)) != PSL_EVENTUALLYBANG);
  }

  if (psl_node_get_op(e)==PSL_SERE ) {
    return psl_node_sere_is_concat_fusion_holes_free(psl_node_get_left(e));
  }

  if (psl_node_get_op(e)==PSL_SERECONCAT ||
      psl_node_get_op(e)==PSL_SEREFUSION) {
    return psl_node_sere_is_concat_fusion_holes_free(psl_node_get_left(e)) &&
      psl_node_sere_is_concat_fusion_holes_free(psl_node_get_right(e));
  }

  return false;
}


/**Function********************************************************************

Synopsis           [Prunes aways the given branch from the given tree]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_prune(PslNode_ptr tree, PslNode_ptr branch)
{
  PslOp op;

  if (tree == PSL_NULL) return PSL_NULL;

  op = psl_node_get_op(tree);

  /* base case */
  if (psl_node_is_equal(tree, branch)) return PSL_NULL;

  /* atomic tree which is not equal to the branch to be pruned */
  if (psl_node_sere_is_propositional(tree)) return tree;

  if (psl_node_is_equal(psl_node_get_left(tree), branch)) {
    return psl_node_get_right(tree);
  }

  if (psl_node_is_equal(psl_node_get_right(tree), branch)) {
    return psl_node_get_left(tree);
  }

  return psl_new_node(op,
                      psl_node_prune(psl_node_get_left(tree), branch),
                      psl_node_prune(psl_node_get_right(tree), branch));
}


/**Function********************************************************************

Synopsis [Returns true if the given expression is a propositional
starred sere.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_propstar(PslNode_ptr e)
{
  PslNode_ptr expr;

  while (psl_node_get_op(e) == PSL_SERE) e = psl_node_get_left(e);

  if (!psl_node_sere_is_star(e)) return false;

  expr = psl_node_sere_repeated_get_expr(e);

  return (expr != PSL_NULL) &&
    PslNode_is_propositional(expr);
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression is a sere in the form
{ s2 && s1 } ]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_sere_is_2ampersand(PslNode_ptr e)
{
  return psl_node_is_sere_compound_binary(e) &&
    psl_node_get_op(psl_node_get_left(e)) == PSL_AMPERSANDAMPERSAND;
}


/**Function********************************************************************

Synopsis           [Maker for a list]

Description        [This gets the element to insert at top level, and
the list for next]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_make_cons(PslNode_ptr elem, PslNode_ptr next)
{
  return psl_new_node(CONS, elem, next);
}


/**Function********************************************************************

Synopsis           [Maker for a list, does not use find_node]

Description        [This gets the element to insert at top level, and
the list for next]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_make_cons_new(PslNode_ptr elem, PslNode_ptr next)
{
  return (PslNode_ptr) new_node(CONS, (node_ptr) elem, (node_ptr) next);
}

/**Function********************************************************************

Synopsis           [Returns true if the given node is a list]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_cons(PslNode_ptr e)
{
  nusmv_assert(e!= PSL_NULL);
  if (psl_node_get_op(e) == CONS) return true;
  return false;
}


/**Function********************************************************************

Synopsis           [Returns the currently pointed element of a list]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_cons_get_element(PslNode_ptr e)
{
  nusmv_assert(psl_node_is_cons(e));
  return psl_node_get_left(e);
}


/**Function********************************************************************

Synopsis           [Returns the next element of a list]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_cons_get_next(PslNode_ptr e)
{
  nusmv_assert(psl_node_is_cons(e));
  return psl_node_get_right(e);
}


/**Function********************************************************************

  Synopsis           [Reverse a list.]

  Description        [Returns a new sequence containing the same
  elements as 'e' but in reverse order]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_cons_reverse(PslNode_ptr e)
{
  PslNode_ptr y;

  if (e == PSL_NULL) return PSL_NULL;
  nusmv_assert(psl_node_is_cons(e));

  y = PSL_NULL;
  while (e != PSL_NULL) {
    PslNode_ptr z = psl_node_get_right(e);
    psl_node_set_right(e, y);
    y = e;
    e = z;
  }

  return y;
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression is If Then Else]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_ite(PslNode_ptr _ite)
{
  if (_ite == PSL_NULL) return false;
  return (psl_node_get_op(_ite) == PSL_ITE) &&
    (psl_node_get_op(psl_node_get_left(_ite)) == COLON);
}


/**Function********************************************************************

Synopsis           [Returns the condition of the given ITE node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_ite_cond(PslNode_ptr _ite)
{
  nusmv_assert(psl_node_is_ite(_ite));
  return psl_node_get_left(psl_node_get_left(_ite));
}


/**Function********************************************************************

Synopsis           [Returns the 'then' branch of the given ITE node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_ite_then(PslNode_ptr _ite)
{
  nusmv_assert(psl_node_is_ite(_ite));
  return psl_node_get_right(psl_node_get_left(_ite));
}


/**Function********************************************************************

Synopsis           [Returns the 'else' branch of the given ITE node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_ite_else(PslNode_ptr _ite)
{
  nusmv_assert(psl_node_is_ite(_ite));
  return psl_node_get_right(_ite);
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression is a case expression]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_case(PslNode_ptr _case)
{
  if (_case == PSL_NULL) return false;
  if (psl_node_get_op(_case) != CASE) return false;
  if (psl_node_get_op(psl_node_get_left(_case)) == COLON) return true;
  else return false;
}


/**Function********************************************************************

Synopsis           [Returns the condition of the given case node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_case_cond(PslNode_ptr _case)
{
  nusmv_assert(psl_node_is_case(_case));
  return psl_node_get_left(psl_node_get_left(_case));
}


/**Function********************************************************************

Synopsis           [Returns the 'then' branch of the given case node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_case_then(PslNode_ptr _case)
{
  nusmv_assert(psl_node_is_case(_case));
  return psl_node_get_right(psl_node_get_left(_case));
}


/**Function********************************************************************

Synopsis           [Returns the next case node of the given case.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_case_next(PslNode_ptr _case)
{
  nusmv_assert(psl_node_is_case(_case));
  return psl_node_get_right(_case);
}


/**Function********************************************************************

Synopsis           [Returns true if the given node is a range]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_range(PslNode_ptr expr)
{
  nusmv_assert(expr != PSL_NULL);
  if (psl_node_get_op(expr) == PSL_RANGE) return true;
  return false;
}


/**Function********************************************************************

Synopsis           [Returns the low bound of the given range]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_range_get_low(PslNode_ptr expr)
{
  nusmv_assert(psl_node_is_range(expr) == true);
  return psl_node_get_left(expr);
}


/**Function********************************************************************

Synopsis           [Returns the high bound of the given range]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_range_get_high(PslNode_ptr expr)
{
  nusmv_assert(psl_node_is_range(expr) == true);
  return psl_node_get_right(expr);
}


/**Function********************************************************************

Synopsis           [Maker for a NUMBER node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_make_number(int value)
{
  return psl_new_node(NUMBER, PSLNODE_FROM_INT(value), PSL_NULL);
}


/**Function********************************************************************

Synopsis           [Maker for a CASE node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_make_case(PslNode_ptr _cond,
                               PslNode_ptr _then, PslNode_ptr _next)
{
  return psl_new_node(CASE,
                      psl_new_node(COLON, _cond, _then), _next) ;
}


/**Function********************************************************************

Synopsis           [Maker for a FAILURE node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_make_failure(const char* msg, FailureKind kind)
{
  return psl_new_node(FAILURE,
                      psl_new_node(COLON,
                                   (PslNode_ptr) find_string((char*) msg),
                                   (PslNode_ptr) kind),
                      (PslNode_ptr) 0 /* fake line number */);
}


/**Function********************************************************************

Synopsis           [Returns true if the given expression is an integer number]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_number(PslNode_ptr e)
{
  nusmv_assert(e != PSL_NULL);
  return (psl_node_get_op(e) == NUMBER);
}

/**Function********************************************************************

Synopsis           [Returns true if the given expression is a word number]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_word_number(PslNode_ptr e)
{
  nusmv_assert(e != PSL_NULL);
  return (psl_node_get_op(e) == NUMBER_SIGNED_WORD ||
          psl_node_get_op(e) == NUMBER_UNSIGNED_WORD);
}


/**Function********************************************************************

Synopsis [Returns the integer value associated with the given number
node. ]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int psl_node_number_get_value(PslNode_ptr e)
{
  PslNode_ptr n;
  nusmv_assert(psl_node_is_number(e));

  n = PslNode_convert_from_node_ptr(
            CompileFlatten_resolve_number(Compile_get_global_symb_table(),
                          PslNode_convert_to_node_ptr(e), Nil));

  nusmv_assert(psl_node_get_op(n) == NUMBER);
  return PSLNODE_TO_INT(psl_node_get_left(e));
}


/**Function********************************************************************

Synopsis           [Maker for a NEXT* family node]

Description        [Warning: the operator must be a symbol, not a token.
This means that psl_conv_op must be called to convert tokens before.]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_make_extended_next(PslOp op, PslNode_ptr expr,
                                        PslNode_ptr when,
                                        PslNode_ptr condition)
{
  return psl_new_node(op, expr, psl_new_node(COLON, when, condition));
}


/**Function********************************************************************

Synopsis           [Given a psl node returns true iff the expression belongs to
the next operators family.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_extended_next(PslNode_ptr e)
{
  PslOp op = psl_node_get_op(e);

  return (op == PSL_X ||
          op == PSL_XBANG ||
          op == PSL_NEXT ||
          op == PSL_NEXTBANG ||
          op == PSL_NEXT_E ||
          op == PSL_NEXT_EBANG ||
          op == PSL_NEXT_A ||
          op == PSL_NEXT_ABANG ||
          op == PSL_NEXT_EVENT ||
          op == PSL_NEXT_EVENTBANG ||
          op == PSL_NEXT_EVENT_E ||
          op == PSL_NEXT_EVENT_EBANG ||
          op == PSL_NEXT_EVENT_A ||
          op == PSL_NEXT_EVENT_ABANG);
}


/**Function********************************************************************

Synopsis           [Returns the FL expression of a next expression node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_extended_next_get_expr(PslNode_ptr next)
{
  nusmv_assert(psl_node_is_extended_next(next));

  return psl_node_get_left(next);
}


/**Function********************************************************************

Synopsis           [Returns the when component of a next expression node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_extended_next_get_when(PslNode_ptr next)
{
  PslNode_ptr r;

  nusmv_assert(psl_node_is_extended_next(next));

  r = psl_node_get_right(next);
  if (r == PSL_NULL) return PSL_NULL;

  nusmv_assert(psl_node_get_op(r) == COLON);
  return psl_node_get_left(r);
}


/**Function********************************************************************

Synopsis           [Returns the boolean condition of a next expression node]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_extended_next_get_condition(PslNode_ptr next)
{
  PslNode_ptr r;

  nusmv_assert(psl_node_is_extended_next(next));

  r = psl_node_get_right(next);
  if (r == PSL_NULL) return PSL_NULL;

  nusmv_assert(psl_node_get_op(r) == COLON);
  return psl_node_get_right(r);
}


/**Function********************************************************************

Synopsis [Returns true if the given node is the PSL syntactic type
'boolean']

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_boolean_type(PslNode_ptr expr)
{
  return (expr != PSL_NULL) && (psl_node_get_op(expr) == BOOLEAN);
}


/**Function********************************************************************

Synopsis           [Returns true if the given node is the PSL syntactic value
'inf']

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_infinite(PslNode_ptr expr)
{
  return (expr != PSL_NULL) && (psl_node_get_op(expr) == PSL_INF);
}


/**Function********************************************************************

Synopsis           [Returns true if the given node is an identifier]

Description [The top level operator of an ID can be DOT,
ATOM or ARRAY]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_id(PslNode_ptr expr)
{
  nusmv_assert(expr != PSL_NULL);

  return (psl_node_get_op(expr) == DOT ||
          psl_node_get_op(expr) == ATOM ||
          psl_node_get_op(expr) == ARRAY);
}


/**Function********************************************************************

Synopsis           [Returns true if two ids are equal]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_id_equal(PslNode_ptr _id1, PslNode_ptr _id2)
{
  boolean res = false;
  nusmv_assert(psl_node_is_id(_id1) && psl_node_is_id(_id2));

  if (psl_node_get_op(_id1) != psl_node_get_op(_id2)) return false;
  switch (psl_node_get_op(_id1)) {
  case NUMBER:
    res = psl_node_is_num_equal(_id1, _id2);
    break;

  case ATOM:
    res = (psl_node_get_left(_id1) == psl_node_get_left(_id2));
    break;

  case ARRAY:
    res = psl_node_is_id_equal(psl_node_get_left(_id1), psl_node_get_left(_id2))
      &&
      psl_node_is_id_equal(psl_node_get_right(_id1), psl_node_get_right(_id2));
    break;

  case DOT:
    res = psl_node_is_id_equal(psl_node_get_left(_id1),
                               psl_node_get_left(_id2)) &&
      ((psl_node_get_right(_id1) == PSL_NULL &&
        psl_node_get_right(_id2) == PSL_NULL) ||
       psl_node_is_id_equal(psl_node_get_right(_id1), psl_node_get_right(_id2)));
    break;

  default:
    fprintf(nusmv_stderr,
            "psl_node_is_id_equal: operator type not supported \"%d\"\n",
            psl_node_get_op(_id1));
    error_unreachable_code();
  }

  return res;
}


/**Function********************************************************************

Synopsis           [Returns true if the given numbers are equal]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_num_equal(PslNode_ptr _id1, PslNode_ptr _id2)
{
  nusmv_assert(psl_node_get_op(_id1) == NUMBER &&
               psl_node_get_op(_id2) == NUMBER);
  return psl_node_get_left(_id1) == psl_node_get_left(_id2);
}


/**Function********************************************************************

Synopsis [Returns true if the given node is a leaf, i.e. PSL_NULL, a
   number, a boolean constant, or an atom.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_leaf(PslNode_ptr expr)
{
  PslOp op;
  if (expr == PSL_NULL) return true;
  op = psl_node_get_op(expr);

  return (NUMBER == op ||
          NUMBER_FRAC == op || NUMBER_REAL == op || NUMBER_EXP == op ||
          ATOM == op ||
          TRUEEXP == op || FALSEEXP == op ||
          NUMBER_SIGNED_WORD == op || NUMBER_UNSIGNED_WORD == op ||
          FAILURE == op);
}


/**Function********************************************************************

Synopsis [Returns true if the given expression is a replicated
property]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_repl_prop(PslNode_ptr _prop)
{
  return (psl_node_get_op(_prop) == PSL_REPLPROP);
}


/**Function********************************************************************

Synopsis [Given a replicated property, returns the node that contains
the replicator.]

Description        []

SideEffects        [None]

SeeAlso            [psl_node_repl_prop_get_property]

******************************************************************************/
PslNode_ptr psl_node_repl_prop_get_replicator(PslNode_ptr _prop)
{
  nusmv_assert(psl_node_is_repl_prop(_prop));
  return psl_node_get_left(_prop);
}


/**Function********************************************************************

Synopsis           [Given a replicated property, returns the node that contains
the property.]

Description        []

SideEffects        [None]

SeeAlso            [psl_node_repl_prop_get_replicator]

******************************************************************************/
PslNode_ptr psl_node_repl_prop_get_property(PslNode_ptr _prop)
{
  nusmv_assert(psl_node_is_repl_prop(_prop));
  return psl_node_get_right(_prop);
}


/**Function********************************************************************

Synopsis [Returns true if the given expression represents a
replicator.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
boolean psl_node_is_replicator(PslNode_ptr _repl)
{
  if (_repl == PSL_NULL) return false;
  if (psl_node_get_op(_repl) != PSL_FORALL &&
      psl_node_get_op(_repl) != PSL_FORANY) return false;
  if (psl_node_get_op(psl_node_get_right(_repl)) != SETIN) return false;
  return true;
}


/**Function********************************************************************

Synopsis           [Given a replicator, returns the its ID]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_replicator_id(PslNode_ptr _repl)
{
  nusmv_assert(psl_node_is_replicator(_repl));
  return psl_node_get_left(_repl);
}


/**Function********************************************************************

Synopsis           [Given a replicator, returns its range]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_replicator_range(PslNode_ptr _repl)
{
  nusmv_assert(psl_node_is_replicator(_repl));
  return psl_node_get_left(psl_node_get_right(_repl));
}


/**Function********************************************************************

Synopsis           [Given a replicator, returns the its values set.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_get_replicator_value_set(PslNode_ptr _repl)
{
  nusmv_assert(psl_node_is_replicator(_repl));
  return psl_node_get_right(psl_node_get_right(_repl));
}


/**Function********************************************************************

Synopsis           [Given a replicator, returns the operator joining each
replicated expression]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslOp psl_node_get_replicator_join_op(PslNode_ptr _repl)
{
  nusmv_assert(psl_node_is_replicator(_repl));
  switch (psl_node_get_op(_repl)) {
  case PSL_FORALL: return AND;
  case PSL_FORANY: return OR;
  default: error_unreachable_code(); /* no other possible cases */
  }
  return -1;
}


/**Function********************************************************************

Synopsis           [Given a replicator, returns its values set as a list
of the enumerated values]

Description        []

SideEffects        [required]

SeeAlso            [optional]

******************************************************************************/
PslNode_ptr psl_node_get_replicator_normalized_value_set(PslNode_ptr rep)
{

  PslNode_ptr range, vset;
  PslNode_ptr result;

  nusmv_assert(psl_node_is_replicator(rep));

  range = psl_node_get_replicator_range(rep);
  vset = psl_node_get_replicator_value_set(rep);

  if (range != PSL_NULL) {
    fprintf(nusmv_stderr, "psl_node_get_replicator_normalized_value_set: " \
            "Replicator range not yet handled.\n");
    error_psl_not_supported_feature();
  }

  result = PSL_NULL;

  if (psl_node_is_boolean_type(vset) == true) {
    result = psl_node_make_cons_new(psl_node_make_false(),
                                psl_node_make_cons_new(psl_node_make_true(),
                                                   result));
  }
  else {
    nusmv_assert(psl_node_is_cons(vset));

    for (; vset != PSL_NULL; vset = psl_node_cons_get_next(vset)) {
      PslNode_ptr v = psl_node_cons_get_element(vset);

      switch (psl_node_get_op(v)) {
      case PSL_RANGE:
        {
          int i, inf, sup;

          if (!psl_node_is_number(psl_node_range_get_low(v))) {
            error_psl_not_supported_feature_next_number();
          }
          inf = psl_node_number_get_value(psl_node_range_get_low(v));

          if (!psl_node_is_number(psl_node_range_get_high(v))) {
            error_psl_not_supported_feature_next_number();
          }
          sup = psl_node_number_get_value(psl_node_range_get_high(v));

          if (inf > sup) {
            fprintf(nusmv_stderr, "Error in: ");
            print_node(nusmv_stderr, v);
            fprintf(nusmv_stderr, "\n");

            error_invalid_numeric_value(sup,
                        "Range with high bound greater than low bound.");
          }

          for (i = inf; i <= sup; i++) {
            result = psl_node_make_cons_new(psl_node_make_number(i), result);
          }
          break;
        }
      case BOOLEAN:
        result = psl_node_make_cons_new(psl_node_make_false(),
                                    psl_node_make_cons_new(psl_node_make_true(),
                                                       result));
        break;
      case NUMBER:
        result = psl_node_make_cons_new(v, result);
        break;

      case ATOM:
      case DOT:
        result = psl_node_make_cons_new(v, result);
        break;

      default:
        fprintf(nusmv_stderr,
                "psl_node_get_replicator_normalized_value_set: expression not " \
                "supported \"%d\"\n",
                psl_node_get_op(v));
        error_unreachable_code();
        break;
      }
    }
        result = psl_node_cons_reverse(result);  /* We must reverse the list */
  }

  return result;
}


/**Function********************************************************************

Synopsis           [Contestualizes a context node into the 'main' context ]

Description        [This function is used to build the internal structure of
   the context (e.g. module instance name) from the parse tree. The
   function is needed since with the grammar it is not possible/simple
   to build directly the desired structure.]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
PslNode_ptr psl_node_context_to_main_context(PslNode_ptr context)
{
  /* assumes psl_new_node invoke find_node */

  if (context == PSL_NULL) return PSL_NULL;

  switch (psl_node_get_op(context)) {
  case ATOM:
    return psl_new_node(DOT, PSL_NULL, context);

  case DOT:
    return psl_new_node(DOT,
             psl_node_context_to_main_context(psl_node_get_left(context)),
             psl_node_get_right(context));

  case ARRAY:
    return psl_new_node(ARRAY,
            psl_node_context_to_main_context(psl_node_get_left(context)),
            psl_node_get_right(context));

  default:
    fprintf(nusmv_stderr,
            "psl_node_context_to_mainc_ontext: undefined token \"%d\"\n",
            psl_node_get_op(context));
    error_unreachable_code();
  }
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           []

Description [To compare structures like {{a}} and {{{{a}}}} and check
   whether the innermost {a}'s are actually the same node pointer]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
static boolean psl_node_is_equal(PslNode_ptr e, PslNode_ptr f)
{
  if (e==f) return true;

  if (e==PSL_NULL || f==PSL_NULL) return false;

  if (PslNode_is_propositional(e) && PslNode_is_propositional(f))
    return false;

  if ((psl_node_get_op(e)==PSL_SERE) &&
      !PslNode_is_propositional(psl_node_get_left(e))) {
    return psl_node_is_equal(psl_node_get_left(e), f);
  }

  if ((psl_node_get_op(f)==PSL_SERE) &&
      !PslNode_is_propositional(psl_node_get_left(f))) {
    return psl_node_is_equal(e, psl_node_get_left(f));
  }

  return false;
}


/**Function********************************************************************

Synopsis           [Returns true if the given sere is star-free]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
static boolean psl_node_is_star_free(PslNode_ptr expr)
{
  PslOp op;

  if (expr == PSL_NULL) return true;

  op = psl_node_get_op(expr);

  /* leaf? */
  if (psl_node_is_leaf(expr) || psl_node_is_id(expr)) {
    return true;
  }
  if (psl_node_sere_is_repeated(expr)) return false;

  return psl_node_is_star_free(psl_node_get_left(expr)) &&
    psl_node_is_star_free(psl_node_get_right(expr));
}


/**Function********************************************************************

Synopsis           [Returns true if the given sere doesn't contain any unbound
                    star]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
static boolean psl_node_is_unbound_star_free(PslNode_ptr expr)
{
  PslOp op;

  if (expr == PSL_NULL) return true;

  op = psl_node_get_op(expr);

  /* leaf? */
  if (psl_node_is_leaf(expr) || psl_node_is_id(expr)) return true;

  /* repeated sere? */
  if (psl_node_sere_is_repeated(expr)) {
    if (psl_node_sere_is_star_count(expr)) {
      /* only star-count with a number as counter are accepted */
      PslNode_ptr count = psl_node_sere_star_get_count(expr);
      if (!psl_node_is_number(count)) {
        fprintf(nusmv_stderr, "In expression ");
        print_node(nusmv_stderr, expr);
        error_expected_number();
      }
    }
    else return false;
  }

  return psl_node_is_unbound_star_free(psl_node_get_left(expr)) &&
    psl_node_is_unbound_star_free(psl_node_get_right(expr));
}

/**Function********************************************************************

Synopsis           [Returns true if the given expression is empty star-free]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
static boolean psl_node_is_emptystar_free(PslNode_ptr expr)
{
  PslOp op;

  if (expr == PSL_NULL) return true;

  op = psl_node_get_op(expr);

  /* leaf? */
  if (psl_node_is_leaf(expr) || psl_node_is_id(expr)) return true;

  if (psl_node_sere_is_plus(expr)) return true;

  if (psl_node_sere_is_star_count(expr)) return true;

  /* here expr is not a star count */
  if (psl_node_sere_is_star(expr)) return false;

  if (op==PSL_SERECONCAT) {
    return psl_node_is_emptystar_free(psl_node_get_left(expr)) ||
      psl_node_is_emptystar_free(psl_node_get_right(expr));
  };

  return psl_node_is_emptystar_free(psl_node_get_left(expr)) &&
    psl_node_is_emptystar_free(psl_node_get_right(expr));
}


/**Function********************************************************************

Synopsis           [Private service of PslNode_is_handled_psl]

Description        []

SideEffects        [None]

SeeAlso            [PslNode_is_handled_psl]

******************************************************************************/
static boolean psl_node_is_handled_sere(PslNode_ptr e, boolean toplevel)
{
  PslOp op;

  if (e == PSL_NULL) return false;
  if (PslNode_is_propositional(e)) return true;

  op = psl_node_get_op(e);

  if (op==PSL_SERE) {
    return psl_node_is_handled_sere(psl_node_get_left(e), toplevel);
  }

  if (op==PSL_SERECONCAT) {

    if (toplevel) {
      return (psl_node_is_emptystar_free(psl_node_get_left(e)) ||
              psl_node_is_emptystar_free(psl_node_get_right(e))) &&
        psl_node_is_handled_sere(psl_node_get_left(e), false) &&
        psl_node_is_handled_sere(psl_node_get_right(e), false);
    }

    return psl_node_is_handled_sere(psl_node_get_left(e), false) &&
      psl_node_is_handled_sere(psl_node_get_right(e), false);
  }

  if (op==PSL_SEREFUSION) {
    return psl_node_is_handled_sere(psl_node_get_left(e), false) &&
      psl_node_is_handled_sere(psl_node_get_right(e), false) &&
      psl_node_is_emptystar_free(psl_node_get_left(e)) &&
      psl_node_is_emptystar_free(psl_node_get_right(e));
  }

  if (op==PSL_SERECOMPOUND) {
    op = psl_node_get_op(psl_node_get_left(e));

    if (op==AND) {
      /* we rule out starts in arguments of & because: {a;b[*]} &
        {c;d;e} would be translated into {a AND c};{bU(b AND e) AND d}
        forcing the sequence of bs to end with an e which belongs to
        {c;d;e} */
      return psl_node_is_star_free(psl_node_get_left(psl_node_get_left(e))) &&
        psl_node_is_star_free(psl_node_get_right(psl_node_get_left(e))) &&
        psl_node_is_handled_sere(psl_node_get_left(psl_node_get_left(e)), toplevel) &&
        psl_node_is_handled_sere(psl_node_get_right(psl_node_get_left(e)), toplevel);
    }

    if (op==PSL_AMPERSANDAMPERSAND) {
      return psl_node_is_star_free(psl_node_get_left(psl_node_get_left(e))) &&
        psl_node_is_star_free(psl_node_get_right(psl_node_get_left(e))) &&
        psl_node_is_handled_sere(psl_node_get_left(psl_node_get_left(e)), toplevel) &&
        psl_node_is_handled_sere(psl_node_get_right(psl_node_get_left(e)), toplevel);
    }

    if (op==OR) {

      if (toplevel) {
        return psl_node_is_emptystar_free(psl_node_get_left(psl_node_get_left(e))) &&
          psl_node_is_emptystar_free(psl_node_get_right(psl_node_get_left(e))) &&
          psl_node_is_handled_sere(psl_node_get_left(psl_node_get_left(e)), toplevel) &&
          psl_node_is_handled_sere(psl_node_get_right(psl_node_get_left(e)), toplevel);
      }

      return psl_node_is_handled_sere(psl_node_get_left(psl_node_get_left(e)), toplevel) &&
        psl_node_is_handled_sere(psl_node_get_right(psl_node_get_left(e)), toplevel);

    }

    return psl_node_is_emptystar_free(psl_node_get_left(psl_node_get_left(e))) &&
      psl_node_is_emptystar_free(psl_node_get_right(psl_node_get_left(e))) &&
      psl_node_is_handled_sere(psl_node_get_left(psl_node_get_left(e)), toplevel) &&
      psl_node_is_handled_sere(psl_node_get_right(psl_node_get_left(e)), toplevel);
  }

  if (op==PSL_SEREREPEATED) return psl_node_is_handled_star(e, toplevel);

  return false;
}


/**Function********************************************************************

Synopsis           [Private service of PslNode_is_handled_psl]

Description        []

SideEffects        [None]

SeeAlso            [PslNode_is_handled_psl]

******************************************************************************/
static boolean psl_node_is_handled_next(PslNode_ptr next)
{
  PslNode_ptr expr;
  PslNode_ptr condition;
  PslNode_ptr when;

  nusmv_assert(psl_node_is_extended_next(next));

  /* gets the components of the next-expression */
  expr = psl_node_extended_next_get_expr(next);
  condition = psl_node_extended_next_get_condition(next);
  when = psl_node_extended_next_get_when(next);

  /* check that the expression is handled psl
     expr cannot be NULL */
  if ((expr == PSL_NULL) || !PslNode_is_handled_psl(expr)) {
    return false;
  }

  /* check that the boolean condition is handled psl
     condition may be NULL */
  if ((condition != PSL_NULL) && !PslNode_is_propositional(condition)) {
    return false;
  }

  /* check that the "when" part of the expression is numeric when may
     be NULL */

  /* Developers' note: this section is delayed until convertion is
     performed, because later forall might resolve ranges and counters
     in 'when' to be pure numbers. */
  if (when != PSL_NULL) {
    if (!psl_node_is_number(when) &&
        (psl_node_is_range(when) &&
         (!psl_node_is_number(psl_node_range_get_low(when)) ||
          !psl_node_is_number(psl_node_range_get_high(when))))) {
      fprintf(nusmv_stderr, "In expression ");
      print_node(nusmv_stderr, when);
      error_expected_number();
    }
  }

  /* here we have: "expr" != NULL and handled, and "when" and
     "condition" NULL or handled */
  return true;
}


/**Function********************************************************************

Synopsis           [Private service of PslNode_is_handled_psl]

Description        []

SideEffects        [None]

SeeAlso            [PslNode_is_handled_psl]

******************************************************************************/
static boolean psl_node_is_handled_fl_op(PslOp op)
{
  return (op != PSL_WHILENOTBANG && op != PSL_WHILENOT &&
          op != PSL_WHILENOTBANG_ && op != PSL_WHILENOT_ &&

          op != PSL_WITHINBANG && op != PSL_WITHIN &&
          op != PSL_WITHINBANG_ && op != PSL_WITHIN_ &&

          op != PSL_ABORT);
}


/**Function********************************************************************

Synopsis           [Private service of PslNode_is_handled_psl]

Description        []

SideEffects        [None]

SeeAlso            [PslNode_is_handled_psl]

******************************************************************************/
static boolean psl_node_is_fl_op(PslOp op)
{
  return (op == OP_NEXT ||
          op == PSL_X ||
          op == PSL_XBANG ||
          op == PSL_NEXT ||
          op == PSL_NEXTBANG ||
          op == PSL_NEXT_E ||
          op == PSL_NEXT_EBANG ||
          op == PSL_NEXT_A ||
          op == PSL_NEXT_ABANG ||
          op == PSL_NEXT_EVENT ||
          op == PSL_NEXT_EVENTBANG ||
          op == PSL_NEXT_EVENT_E ||
          op == PSL_NEXT_EVENT_EBANG ||
          op == PSL_NEXT_EVENT_A ||
          op == PSL_NEXT_EVENT_ABANG ||
          op == OP_FUTURE ||
          op == PSL_NEVER ||
          op == OP_GLOBAL ||
          op == PSL_ALWAYS ||
          op == PSL_EVENTUALLYBANG ||
          op == PSL_W ||
          op == PSL_UNTIL ||
          op == UNTIL ||
          op == PSL_UNTILBANG ||
          op == PSL_UNTIL_ ||
          op == PSL_UNTILBANG_ ||
          op == PSL_BEFORE ||
          op == PSL_BEFOREBANG ||
          op == PSL_BEFORE_ ||
          op == PSL_BEFOREBANG_ ||
          op == PSL_WHILENOTBANG ||
          op == PSL_WHILENOT ||
          op == PSL_WHILENOTBANG_ ||
          op == PSL_WHILENOT_ ||
          op == PSL_ABORT);
}


/**Function********************************************************************

Synopsis           [Private service of PslNode_is_handled_psl]

Description        []

SideEffects        [None]

SeeAlso            [PslNode_is_handled_psl]

******************************************************************************/
static boolean psl_node_is_obe_op(PslOp op)
{
  return (op == AX ||
          op == EX ||
          op == AG ||
          op == EG ||
          op == AF ||
          op == EF ||
          op == EU ||
          op == AU);
}

/**Function********************************************************************

Synopsis           [Checks for a formula being a propositional formula]

Description        [Checks for a formula being a propositional formula]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
static boolean psl_node_is_propositional(const PslNode_ptr expr,
                                         boolean accept_next)
{
  boolean result = false;

  if (expr == PSL_NULL) return true;

  if (psl_node_is_leaf(expr) || psl_node_is_id(expr)) return true;

  switch (psl_node_get_op(expr)) {

  case CONTEXT:
    result = PslNode_is_propositional(psl_node_get_right(expr));
    break;

    /* primary */
  case MINUS:
  case PLUS:

    /* binary operators */
  case UNION:
  case SETIN:
  case TIMES:
  case DIVIDE:
  case MOD:
  case PSL_EQEQ:
  case EQUAL:
  case NOTEQUAL:
  case LT:
  case LE:
  case GT:
  case GE:
    result = true;
    break;

  case NOT:
  case PSL_TILDE:
    result = PslNode_is_propositional(psl_node_get_left(expr));
    break;

    /* boolean binary operators */
  case AND:
  case OR:
  case PSL_OR:
  case XOR:
  case PSL_CARET:
  case IFF:
  case IMPLIES:
    result = PslNode_is_propositional(psl_node_get_left(expr)) &&
      PslNode_is_propositional(psl_node_get_right(expr));
    break;

  case CASE:
    result = PslNode_is_propositional(psl_node_get_case_cond(expr)) &&
      PslNode_is_propositional(psl_node_get_case_then(expr)) &&
      PslNode_is_propositional(psl_node_get_case_next(expr));
    break;

  case PSL_ITE:
    result = PslNode_is_propositional(psl_node_get_ite_cond(expr)) &&
      PslNode_is_propositional(psl_node_get_ite_then(expr)) &&
      PslNode_is_propositional(psl_node_get_ite_else(expr));
    break;

  case PSL_REPLPROP:
    result = PslNode_is_propositional(psl_node_repl_prop_get_property(expr));
    break;

  case NEXT:
    result = accept_next;
    break;

  default:
    result = false;
  }

  return result;
}
