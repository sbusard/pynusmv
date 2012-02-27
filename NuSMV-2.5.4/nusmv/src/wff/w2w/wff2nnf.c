/**CFile***********************************************************************

   FileName    [wff2nnf.c]

   PackageName [wff.w2w]

   Synopsis    [Well Formed Formula to Negation Normal Form conversion]

   Description []

   SeeAlso     []

   Author      [Alessandro Cimatti, Lorenzo Delana, Alessandro Mariotti]

   Copyright   [
   This file is part of the ``wff.w2w'' package of NuSMV version 2.
   Copyright (C) 2000-2011 by FBK-irst and University of Trento.

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

#ifdef CONFIG_H
#include "nusmv-config.h"
#endif

#include "wff/wff.h"
#include "w2w.h"

#include "parser/symbols.h" /* for constants */
#include "utils/error.h"
#include "utils/assoc.h"

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

static hash_ptr wff2nnf_hash = (hash_ptr) NULL;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void w2w_wff2nnf_hash_insert_entry ARGS((node_ptr wff, boolean polarity,
                                                node_ptr nnf));

static node_ptr w2w_wff2nnf_hash_lookup_entry ARGS((node_ptr wff,
                                                    boolean polarity));

static node_ptr w2w_wff_expand_case_aux ARGS((node_ptr wff));

static node_ptr w2w_wff_expand_case ARGS((node_ptr wff));

static node_ptr w2w_wff_mk_nnf ARGS((node_ptr wff, boolean pol));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis           [Makes the <b>negative normal form</b> of given WFF]

   Description        [A positive (1) polarity will not negate entire formula]

   SideEffects        [node hash may change]

   SeeAlso            []

******************************************************************************/
node_ptr Wff2Nnf(node_ptr wff)
{
  return w2w_wff_mk_nnf(wff, true);
}


/**Function********************************************************************

   Synopsis           [Clears the memoization hash of the wff2nff
   conversion function]

   Description        [Clears the memoization hash of the wff2nff
   conversion function]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void w2w_clear_wff2nnf_memoization()
{
  nusmv_assert(wff2nnf_hash == (hash_ptr) NULL);
  clear_assoc(wff2nnf_hash);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [Initializes the wff2nff conversion system]

   Description        [Initializes the wff2nff conversion system]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void w2w_init_wff2nnf()
{
  nusmv_assert(wff2nnf_hash == (hash_ptr) NULL);
  wff2nnf_hash = new_assoc();
}

/**Function********************************************************************

   Synopsis           [Deinitializes the wff2nff conversion system]

   Description        [Deinitializes the wff2nff conversion system]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void w2w_quit_wff2nnf()
{
  if (wff2nnf_hash != (hash_ptr) NULL) {
    free_assoc(wff2nnf_hash);
    wff2nnf_hash = (hash_ptr) NULL;
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [Memoizes the given entry in the wff2nff memoization hash]

   Description        [Memoizes the given entry in the wff2nff memoization hash]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void w2w_wff2nnf_hash_insert_entry(node_ptr wff, boolean polarity,
                                          node_ptr nnf)
{
  nusmv_assert(wff2nnf_hash != (hash_ptr) NULL);
  insert_assoc(wff2nnf_hash, find_node(CONS, wff, (node_ptr) polarity), nnf);
}


/**Function********************************************************************

   Synopsis           [Looks up in the wff2nff memoization hash for
   the given entry]

   Description        [Looks up in the wff2nff memoization hash for
   the given entry]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr w2w_wff2nnf_hash_lookup_entry(node_ptr wff, boolean polarity)
{
  nusmv_assert(wff2nnf_hash != (hash_ptr) NULL);
  return find_assoc(wff2nnf_hash, find_node(CONS, wff, (node_ptr) polarity));
}


/**Function********************************************************************

   Synopsis           [Expands the given case expression]

   Description        [Expands the given case expression]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr w2w_wff_expand_case(node_ptr wff)
{
  node_ptr res;

  nusmv_assert(CASE == node_get_type(wff) || IFTHENELSE == node_get_type(wff));
  res = w2w_wff_expand_case_aux(wff);

  return res;
}


/**Function********************************************************************

   Synopsis           [Aux fun of w2w_wff_expand_case]

   Description        [Aux fun of w2w_wff_expand_case]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr w2w_wff_expand_case_aux(node_ptr wff)
{
  if (CASE == node_get_type(wff) || IFTHENELSE == node_get_type(wff)) {
    node_ptr cur_cond  = car(car(wff));
    node_ptr cur_rslt  = cdr(car(wff));
    node_ptr case_rest = cdr(wff);
    node_ptr res;

    nusmv_assert(node_get_type(car(wff)) == COLON);

    /* here lazy evaluation is required to get rid of FAILURE node
       in case-expressions (see this function below)
    */
    if (cur_cond == Wff_make_truth()) return cur_rslt;
    if (cur_cond == Wff_make_falsity()) return case_rest;

    res = Wff_make_or( Wff_make_and(cur_cond, cur_rslt),
                       Wff_make_and(Wff_make_not(cur_cond),
                                    w2w_wff_expand_case_aux(case_rest))
                       );
    return res;

  }
  else {
    if (FAILURE == node_get_type(wff)) {
      if (failure_get_kind(wff) == FAILURE_CASE_NOT_EXHAUSTIVE) {
        warning_case_not_exhaustive(wff);
        /* forces a default */
        return Wff_make_truth();
      }
      else if (failure_get_kind(wff) == FAILURE_DIV_BY_ZERO) {
        warning_possible_div_by_zero(wff);
        /* forces a default */
        return Wff_make_truth();
      }
      else if (failure_get_kind(wff) == FAILURE_ARRAY_OUT_OF_BOUNDS) {
        warning_possible_array_out_of_bounds(wff);
        /* forces a default */
        return Wff_make_truth();
      }
      else {
        report_failure_node(wff); /* some error in the input expr */
        error_unreachable_code();
      }
    }

    return wff;
  }
}


/**Function********************************************************************

   Synopsis           [The private function that does the actual
                       wff2nnf conversion]

   Description        [The private function that does the actual
                       wff2nnf conversion]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr w2w_wff_mk_nnf(node_ptr wff, boolean pol)
{
  node_ptr res;

  /* if reached a Nil branch then end recursion with a Nil node */
  if (wff == Nil) return Nil;

  /* Only temporal operator X (node type OP_NEXT) is legal in LTL wffs.
     The operator NEXT used in model definition is trapped. */
  nusmv_assert(node_get_type(wff) != NEXT);

  res = w2w_wff2nnf_hash_lookup_entry(wff, pol);
  if (res != (node_ptr) NULL) return res;

  switch (node_get_type(wff)) {
  case TRUEEXP:
    if (pol) res = Wff_make_truth();
    else res = Wff_make_falsity();  /* !1 <-> 0 */
    break;

  case FALSEEXP:
    if (pol) res = Wff_make_falsity();
    else res = Wff_make_truth();    /* !0 <-> 1 */
    break;

  case NOT:
    /* !(a) <-> (!a) */
    /* !(!a) <-> a */
    res = w2w_wff_mk_nnf(car(wff), !pol);
    break;

  case AND:
    if (pol) res = Wff_make_and(w2w_wff_mk_nnf(car(wff), true),
                                w2w_wff_mk_nnf(cdr(wff), true));
    else {
      /* !(a & b) <-> (!a | !b) */
      res = Wff_make_or(w2w_wff_mk_nnf(car(wff), false),
                        w2w_wff_mk_nnf(cdr(wff), false));
    }
    break;

  case OR:
    if (pol) res = Wff_make_or(w2w_wff_mk_nnf(car(wff), true),
                               w2w_wff_mk_nnf(cdr(wff), true));
    else {
      /* !(a | b) <-> (!a & !b) */
      res = Wff_make_and(w2w_wff_mk_nnf(car(wff), false),
                         w2w_wff_mk_nnf(cdr(wff), false));
    }
    break;

  case IMPLIES:
    if (pol) {
      /* (a -> b) <-> !(a & !b) <-> (!a | b) */
      res = Wff_make_or(w2w_wff_mk_nnf(car(wff), false),
                        w2w_wff_mk_nnf(cdr(wff), true));
    }
    else {
      /* !(a -> b) <-> (a & !b) */
      res = Wff_make_and(w2w_wff_mk_nnf(car(wff), true),
                         w2w_wff_mk_nnf(cdr(wff), false));
    }
    break;

  case IFF:
    if (pol) {
      /* (a <-> b) <->
         !(a & !b) & !(b & !a) <->
         (!a | b) & (!b | a) */
      res = Wff_make_and( Wff_make_or(w2w_wff_mk_nnf(car(wff), false),
                                      w2w_wff_mk_nnf(cdr(wff), true)),
                          Wff_make_or(w2w_wff_mk_nnf(car(wff), true),
                                      w2w_wff_mk_nnf(cdr(wff), false)) );
    }
    else {
      /* !(a <-> b) <->
         !(!(a & !b) & !(b & !a)) <->
         (a & !b) | (b & !a) */
      res = Wff_make_or( Wff_make_and(w2w_wff_mk_nnf(car(wff), true),
                                      w2w_wff_mk_nnf(cdr(wff), false)),
                         Wff_make_and(w2w_wff_mk_nnf(car(wff), false),
                                      w2w_wff_mk_nnf(cdr(wff), true)) );
    }
    break;

  case XOR:
    if (pol) {
      /* (a xor b) <-> (a & !b) | (!a & b) */
      res = Wff_make_or( Wff_make_and(w2w_wff_mk_nnf(car(wff), true),
                                      w2w_wff_mk_nnf(cdr(wff), false)),
                         Wff_make_and(w2w_wff_mk_nnf(car(wff), false),
                                      w2w_wff_mk_nnf(cdr(wff), true)) );
    }
    else {
      /* !(a xnor b) <-> (a | !b) & (!a | b) */
      res = Wff_make_and( Wff_make_or(w2w_wff_mk_nnf(car(wff), true),
                                      w2w_wff_mk_nnf(cdr(wff), false)),
                          Wff_make_or(w2w_wff_mk_nnf(car(wff), false),
                                      w2w_wff_mk_nnf(cdr(wff), true)) );
    }
    break;

  case XNOR:
    if (pol) {
      /* (a xnor b) <-> (!a | b) & (!b | a) */
      res = Wff_make_and( Wff_make_or(w2w_wff_mk_nnf(car(wff), false),
                                      w2w_wff_mk_nnf(cdr(wff), true)),
                          Wff_make_or(w2w_wff_mk_nnf(car(wff), true),
                                      w2w_wff_mk_nnf(cdr(wff), false)) );
    }
    else {
      /* !(a xnor b) <-> (a & !b) | (!a & b) */
      res = Wff_make_or( Wff_make_and(w2w_wff_mk_nnf(car(wff), true),
                                      w2w_wff_mk_nnf(cdr(wff), false)),
                         Wff_make_and(w2w_wff_mk_nnf(car(wff), false),
                                      w2w_wff_mk_nnf(cdr(wff), true)) );
    }
    break;

  case OP_NEXT:
    /* !X(a) <-> X(!a) */
    res = Wff_make_opnext(w2w_wff_mk_nnf(car(wff), pol));
    break;

  case OP_PREC:
    /* !Y(a) <-> Z(!a) */
    if (pol) res = Wff_make_opprec(w2w_wff_mk_nnf(car(wff), pol));
    else res = Wff_make_opnotprecnot(w2w_wff_mk_nnf(car(wff), pol));
    break;

  case OP_NOTPRECNOT:
    /* !Z(a) <-> Y(!a) */
    if (pol) res = Wff_make_opnotprecnot(w2w_wff_mk_nnf(car(wff), pol));
    else res = Wff_make_opprec(w2w_wff_mk_nnf(car(wff), pol));
    break;

  case OP_GLOBAL:
    if (pol) res = Wff_make_globally(w2w_wff_mk_nnf(car(wff), pol));
    else {
      /* !G(a) <-> F(!a) */
      res = Wff_make_eventually(w2w_wff_mk_nnf(car(wff), pol));
    }
    break;

  case OP_HISTORICAL:
    if (pol) res = Wff_make_historically(w2w_wff_mk_nnf(car(wff), pol));
    else {
      /* !H(a) <-> O(!a) */
      res = Wff_make_once(w2w_wff_mk_nnf(car(wff), pol));
    }
    break;

  case OP_FUTURE:
    if (pol) res = Wff_make_eventually(w2w_wff_mk_nnf(car(wff), pol));
    else {
      /* !F(a) <-> G(!a) */
      res = Wff_make_globally(w2w_wff_mk_nnf(car(wff), pol));
    }
    break;

  case OP_ONCE:
    if (pol) res = Wff_make_once(w2w_wff_mk_nnf(car(wff), pol));
    else {
      /* !O(a) <-> H(!a) */
      res = Wff_make_historically(w2w_wff_mk_nnf(car(wff), pol));
    }
    break;

  case UNTIL:
    if (pol) res = Wff_make_until(w2w_wff_mk_nnf(car(wff), pol),
                                  w2w_wff_mk_nnf(cdr(wff), pol));
    else {
      /* !(a U b) <-> (!a V !b) */
      res = Wff_make_releases(w2w_wff_mk_nnf(car(wff), pol),
                              w2w_wff_mk_nnf(cdr(wff), pol));
    }
    break;

  case SINCE:
    if (pol) res = Wff_make_since(w2w_wff_mk_nnf(car(wff), pol),
                                  w2w_wff_mk_nnf(cdr(wff), pol));
    else {
      /* !(a S b) <-> (!a T !b) */
      res = Wff_make_triggered(w2w_wff_mk_nnf(car(wff), pol),
                               w2w_wff_mk_nnf(cdr(wff), pol));
    }
    break;

  case RELEASES:
    if (pol) res = Wff_make_releases(w2w_wff_mk_nnf(car(wff), pol),
                                     w2w_wff_mk_nnf(cdr(wff), pol));
    else {
      /* !(a V b) <-> (!a U !b) */
      res = Wff_make_until(w2w_wff_mk_nnf(car(wff), pol),
                           w2w_wff_mk_nnf(cdr(wff), pol));
    }
    break;

  case TRIGGERED:
    if (pol) res = Wff_make_triggered(w2w_wff_mk_nnf(car(wff), pol),
                                      w2w_wff_mk_nnf(cdr(wff), pol));
    else {
      /* !(a T b) <-> (!a S !b) */
      res = Wff_make_since(w2w_wff_mk_nnf(car(wff), pol),
                           w2w_wff_mk_nnf(cdr(wff), pol));
    }
    break;

  case IFTHENELSE:
  case CASE:
    {
      node_ptr nocase_wff = w2w_wff_expand_case(wff);
      res = w2w_wff_mk_nnf(nocase_wff, pol);
      break;
    }

  case BIT:
  case DOT:
  case ARRAY:
    /* it is a bexp var */
    if (pol) res = wff;
    else res = Wff_make_not(wff);
    break;

  case ATOM:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    /* internal format for atoms that should have been previously
       hidden within DOT and ARRAY */
    internal_error("w2w_wff_mk_nnf: unexpected leaf %d\n",
                   node_get_type(wff));
    res = (node_ptr) NULL;
    break;

  case MOD:
    internal_error("w2w_wff_mk_nnf: unexpected mod operator\n");

    /* stop recursion when a predicate is found */
  case LE: case LT:
  case GE: case GT:
  case EQUAL: case NOTEQUAL:
  case SETIN:
    res = (pol) ? wff : Wff_make_not(wff);
      break;

  default:
    internal_error("w2w_wff_mk_nnf: unexpected TOKEN %d\n",
                   node_get_type(wff));

  }

  if (res != (node_ptr) NULL) {
    w2w_wff2nnf_hash_insert_entry(wff, pol, res);
  }

  return res;
}

