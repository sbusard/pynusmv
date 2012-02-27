/**CFile***********************************************************************

  FileName    [exprWff.c]

  PackageName [wff]

  Synopsis    [Well Formed Formula manipulation routines]

  Description []

  SeeAlso     []

  Author      [Alessandro Cimatti, Lorenzo Delana, Alessandro Mariotti]

  Copyright   [
  This file is part of the ``wff'' package of NuSMV version 2.
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

#include "wff.h"

#include "cudd.h" /* for FALSE */
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


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static node_ptr expr_wff_make_binary ARGS((int type, node_ptr arg1,
                                       node_ptr arg2));
static node_ptr expr_wff_make_unary ARGS((int type, node_ptr arg));
static node_ptr expr_wff_make_const ARGS((int type));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Makes a <i>truth</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_truth(void)
{
  return expr_wff_make_const(TRUEEXP);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>false</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_falsity(void)
{
  return expr_wff_make_const(FALSEEXP);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>not</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_not(node_ptr arg)
{
  return expr_wff_make_unary(NOT, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>and</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_and(node_ptr arg1, node_ptr arg2)
{
  node_ptr falsity;
  node_ptr truth;

  falsity = Wff_make_falsity();
  if ((arg1 == falsity) || (arg2 == falsity)) return falsity;

  truth = Wff_make_truth();
  if (arg1 == truth) return arg2;
  if (arg2 == truth) return arg1;

  return expr_wff_make_binary(AND, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>or</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_or(node_ptr arg1, node_ptr arg2)
{
  node_ptr falsity;
  node_ptr truth;

  truth = Wff_make_truth();
  if ((arg1 == truth) || (arg2 == truth)) return truth;

  falsity = Wff_make_falsity();
  if (arg1 == falsity) return arg2;
  if (arg2 == falsity) return arg1;

  return expr_wff_make_binary(OR, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>implies</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_implies(node_ptr arg1, node_ptr arg2)
{
  return expr_wff_make_binary(IMPLIES, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>iff</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_iff(node_ptr arg1, node_ptr arg2)
{
  return expr_wff_make_binary(IFF, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>next</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_next(node_ptr arg)
{
  return expr_wff_make_unary(NEXT, arg);
}


/**Function********************************************************************

  Synopsis           [Applies <i>op_next</i> x times]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_opnext_times(node_ptr arg, int x)
{
  nusmv_assert(x >= 0);

  if (x == 0)
    return arg;
  else
    return Wff_make_opnext(Wff_make_opnext_times(arg, x - 1));
}

/**Function********************************************************************

  Synopsis           [Makes an <i>op_next</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_opnext(node_ptr arg)
{
  return expr_wff_make_unary(OP_NEXT, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>op_next</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_opprec(node_ptr arg)
{
  return expr_wff_make_unary(OP_PREC, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>op_next</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_opnotprecnot(node_ptr arg)
{
  return expr_wff_make_unary(OP_NOTPRECNOT, arg);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>globally</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_globally(node_ptr arg)
{
  return expr_wff_make_unary(OP_GLOBAL, arg);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>historically</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_historically(node_ptr arg)
{
  return expr_wff_make_unary(OP_HISTORICAL, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>eventually</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_eventually(node_ptr arg)
{
  return expr_wff_make_unary(OP_FUTURE, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>once</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_once(node_ptr arg)
{
  return expr_wff_make_unary(OP_ONCE, arg);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>until</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_until(node_ptr arg1, node_ptr arg2)
{
  return expr_wff_make_binary(UNTIL, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes an <i>since</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_since(node_ptr arg1, node_ptr arg2)
{
  return expr_wff_make_binary(SINCE, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>releases</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_releases(node_ptr arg1, node_ptr arg2)
{
  return expr_wff_make_binary(RELEASES, arg1, arg2);
}

/**Function********************************************************************

  Synopsis           [Makes a <i>triggered</i> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Wff_make_triggered(node_ptr arg1, node_ptr arg2)
{
  return expr_wff_make_binary(TRIGGERED, arg1, arg2);
}


/**Function********************************************************************

  Synopsis           [Returns the modal depth of the given formula]

  Description        [Returns 0 for propositional formulae, 1 or more for
  temporal formulae]

  SideEffects        [none]

  SeeAlso            []

******************************************************************************/
int Wff_get_depth(node_ptr ltl_wff)
{
  int depth = -1;
  int d1,d2;

  switch (node_get_type(ltl_wff)) {
  case TRUEEXP:                           /* TRUEEXP   */
  case FALSEEXP:                          /* FALSEEXP  */
    depth = 0;
    break;

  case NOT:                             /* NOT       */
    depth = Wff_get_depth(car(ltl_wff));
    break;

  case AND:                             /* AND       */
  case OR:                              /* OR        */
  case IFF:                             /* IFF        */
    d1 = Wff_get_depth(car(ltl_wff));
    d2 = Wff_get_depth(cdr(ltl_wff));
    depth = max(d1,d2);
    break;

  case OP_NEXT:                         /* OP_NEXT   */
  case OP_PREC:                         /* OP_PREC   */
  case OP_NOTPRECNOT:                   /* OP_NOTPRECNOT */
  case OP_GLOBAL:                       /* OP_GLOBAL */
  case OP_HISTORICAL:                   /* OP_HISTORICAL */
  case OP_FUTURE:                       /* OP_FUTURE */
  case OP_ONCE:                         /* OP_ONCE */
    depth = 1 + Wff_get_depth(car(ltl_wff));
    break;

  case UNTIL:                           /* UNTIL     */
  case SINCE:                           /* SINCE     */
  case RELEASES:                        /* RELEASES  */
  case TRIGGERED:                       /* TRIGGERED */
    d1 = Wff_get_depth(car(ltl_wff));
    d2 = Wff_get_depth(cdr(ltl_wff));
    depth = 1 + max(d1,d2);
    break;

  case IMPLIES:
    internal_error("implies should have been nnf-ef away!\n");
    break;

  case BIT:
  case DOT:
  case ARRAY:
    depth = 0;
    break;

    /* in pure bool bmc these nodes these nodes are not present,
       whereas the mathsat add-on calls this function while building
       the tableau for LTL specifications. In the latter case the
       formula is no longer guaranteed to be pure boolean.*/
  case CAST_BOOL:
  case EQUAL:
  case NOTEQUAL:
  case LT:
  case GT:
  case LE:
  case GE:
    depth = 0;
    break;

    /* these nodes are expected to be unreachable as already handled
       by previous cases */
  case ATOM:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    internal_error("Unexpected leaf, node type %d\n",
                   node_get_type(ltl_wff));
    break;

  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
  case EBF:
  case EBG:
  case ABF:
  case ABG:
    internal_error("Unexpected CTL operator, node type %d\n",
                   node_get_type(ltl_wff));
    break;

  default:
    /* no other cases are currently allowed */
    internal_error("Unexpected node, node type %d\n",
                   node_get_type(ltl_wff));
  }

  return depth;
}




/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Makes a <b>binary</b> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
static node_ptr expr_wff_make_binary(int type, node_ptr arg1, node_ptr arg2)
{
  return find_node(type, arg1, arg2);
}


/**Function********************************************************************

  Synopsis           [Makes a <b>unary</b> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
static node_ptr expr_wff_make_unary(int type, node_ptr arg)
{
  return find_node(type, arg, Nil);
}

/**Function********************************************************************

  Synopsis           [Makes a <b>constant</b> WFF]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
static node_ptr expr_wff_make_const(int type)
{
  return find_node(type, Nil, Nil);
}

