/**CFile***********************************************************************

  FileName    [Expr.c]

  PackageName [fsm.sexp]

  Synopsis    [Abstraction for expression type implemented as node_ptr]

  Description [
  -----------------------------------WARNING!----------------------------------
  This packages assumes all the expressions being passed to be already
  flattened! Make sure to know what you are doing, if you want to use it with
  unflattened ones.
  -----------------------------------WARNING!----------------------------------
              ]

  SeeAlso     []

  Author      [Roberto Cavada, Michele Dorigatti, Alessandro Mariotti]

  Copyright   [
  This file is part of the ``fsm.sexp'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.
  Copyright (C) 2011 by FBK.

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
#include "Expr.h"

#include "parser/symbols.h"
#include "enc/operators.h"
#include "compile/compile.h"
#include "compile/symb_table/ResolveSymbol.h"
#include "utils/WordNumber.h"
#include "utils/error.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variables                                                                 */
/*---------------------------------------------------------------------------*/
/**Variable********************************************************************

  Synopsis           [A static symbol table now disabled]

  Description [
  Now all the functions in Expr.c needing a symbol table take it as
  parameter. If you have to use any of these functions you can pass them a NULL
  symbol table for having only sintactic semplification (this is the old
  behaviour), or a symbol table to achieve symbolic simplification.

  I passed the symbol table to the functions in every file I could understand
  what the right one is.
  I launched all the tests, both of NuSMV and of esmc, but it'possible a symbol
  table is now not proper used, leading to bugs. If you will get trouble, you
  can define the SYMB_TABLE_STATIC macro that restores the old (debugged)
  behaviour, in order understand if the bug is due to this change.

  Passed some times without problem, we can get rid of the old code.]

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
static SymbTable_ptr expr_st = SYMB_TABLE(NULL);

void fsm_sexp_expr_init() { expr_st = SYMB_TABLE(NULL); }
void fsm_sexp_expr_quit() { expr_st = SYMB_TABLE(NULL); }
#endif

/* Don't change these values (used to handle NEXT untimed case and FROZEN) */
const int UNTIMED_CURRENT = -2;
const int UNTIMED_NEXT = -1;
const int UNTIMED_DONTCARE = -3;
const int _TIME_OFS = 10;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static Expr_ptr expr_simplify_aux ARGS((SymbTable_ptr st, Expr_ptr expr,
                                        hash_ptr hash));

static Expr_ptr expr_bool_to_word1 ARGS((const Expr_ptr a));

static int expr_get_curr_time ARGS((SymbTable_ptr st,
                                    node_ptr expr,
                                    hash_ptr cache));

static Expr_ptr expr_timed_to_untimed ARGS((SymbTable_ptr st, Expr_ptr expr,
                                            int curr_time, boolean in_next,
                                            hash_ptr cache));

static boolean expr_is_timed_aux ARGS((Expr_ptr expr, hash_ptr cache));

static boolean expr_is_bool ARGS((const Expr_ptr a));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Returns the true expression value]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_true()
{
  return EXPR( find_node(TRUEEXP, Nil, Nil) );
}


/**Function********************************************************************

  Synopsis           [Returns the false expression value]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_false()
{
  return EXPR( find_node(FALSEEXP, Nil, Nil) );
}


/**Function********************************************************************

  Synopsis           [Checkes whether given value is the true value]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean Expr_is_true(const Expr_ptr expr)
{
  return TRUEEXP == node_get_type(NODE_PTR(expr));
}


/**Function********************************************************************

  Synopsis           [Checkes whether given value is the false value]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
boolean Expr_is_false(const Expr_ptr expr)
{
  return FALSEEXP == node_get_type(NODE_PTR(expr));
}


/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise AND of given operators]

  Description        [Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_and(const Expr_ptr a, const Expr_ptr b)
{
  /* boolean */
  if (a == EXPR(NULL) && b == EXPR(NULL)) return Expr_true();
  if (a == EXPR(NULL) || Expr_is_true(a))  return b;
  if (b == EXPR(NULL) || Expr_is_true(b))  return a;
  if (Expr_is_false(a)) return a;
  if (Expr_is_false(b)) return b;
  if (a == b)           return a;
  {
    int ta = node_get_type(NODE_PTR(a)); int tb = node_get_type(NODE_PTR(b));
    if ((ta == NOT && EXPR(car(NODE_PTR(a))) == b) ||
        (tb == NOT && EXPR(car(NODE_PTR(b))) == a)) return Expr_false();

    /* bitwise */
    if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
        (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {

#ifndef DISABLE_EXPR_POINTERS_ORDERING
      /* Take in count pointers to increment node sharing */
      if (car(NODE_PTR(a)) > car(NODE_PTR(b))) {
        return EXPR(find_node(ta,
                              NODE_PTR(WordNumber_and(WORD_NUMBER(car(b)),
                                                      WORD_NUMBER(car(a)))),
                              Nil));
      }
#endif

      return EXPR(find_node(ta,
                            NODE_PTR(WordNumber_and(WORD_NUMBER(car(a)),
                                                    WORD_NUMBER(car(b)))),
                            Nil));
    }
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible, but take in count pointers for
     better node sharing */
  if (a > b) {
    return EXPR(find_node(AND, NODE_PTR(b), NODE_PTR(a)));
  }
#endif

  return EXPR(find_node(AND, NODE_PTR(a), NODE_PTR(b)));
}


/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise AND of given operators,
  considering Nil as the true value]

  Description        [Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_and_nil(const Expr_ptr a, const Expr_ptr b)
{
  Expr_ptr result;
  Expr_ptr atmp, btmp;

  atmp = (EXPR(NULL) != a) ? a : Expr_true();
  btmp = (EXPR(NULL) != b) ? b : Expr_true();
  result = Expr_and(atmp, btmp);

  return result;
}


/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise AND of all elements in the
  list]

  Description        [Performs local syntactic simplification.
  Nil value is considered as true value]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_and_from_list(node_ptr list, SymbTable_ptr symb_table)
#else
Expr_ptr Expr_and_from_list(node_ptr list, const SymbTable_ptr symb_table)
#endif
{
#ifdef SYMB_TABLE_STATIC
  symb_table = expr_st;
#endif

  int type;
  if (list == Nil) return Expr_true();



  type = node_get_type(list);
  if (CONS != type && AND != type) {
    return Expr_resolve(symb_table,
                        type, EXPR(car(list)), EXPR(cdr(list)));
  }

  /* recursive step */
  return Expr_and_nil(EXPR(car(list)),
                      Expr_and_from_list(cdr(list), symb_table));
}


/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise NOT of given operator]

  Description        [Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_not(const Expr_ptr expr)
{
  /* boolean */
  if (Expr_is_true(expr)) return Expr_false();
  if (Expr_is_false(expr)) return Expr_true();

  {
    int ta = node_get_type(NODE_PTR(expr));
    if (NOT == ta) return EXPR(car(NODE_PTR(expr)));

    /* bitwise */
    if (ta == NUMBER_UNSIGNED_WORD || ta == NUMBER_SIGNED_WORD) {
      return find_node(ta,
                       NODE_PTR(WordNumber_not(WORD_NUMBER(car(expr)))),
                       Nil);
    }
  }

  /* no simplification is possible */
  return EXPR( find_node(NOT, NODE_PTR(expr), Nil) );
}


/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise OR of given operators]

  Description        [Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_or(const Expr_ptr a, const Expr_ptr b)
{
  /* boolean */
  if (Expr_is_true(a)) return a;
  if (Expr_is_true(b)) return b;
  if (Expr_is_false(a)) return b;
  if (Expr_is_false(b)) return a;
  if (a==b) return a;
  {
    int ta = node_get_type(NODE_PTR(a)); int tb = node_get_type(NODE_PTR(b));

    if ((ta == NOT && EXPR(car(NODE_PTR(a))) == b) ||
        (tb == NOT && EXPR(car(NODE_PTR(b))) == a)) return Expr_true();

    if ((ta == AND) && (tb == AND)) {
      /* ((A & B) || (A & !B)) ---> A */
      /* ((A & !B) || (A & B)) ---> A */
      if ((car(NODE_PTR(a)) == car(NODE_PTR(b))) &&
          (((node_get_type(cdr(NODE_PTR(b))) == NOT) &&
            (car(cdr(NODE_PTR(b))) == cdr(a))) ||
           ((node_get_type(cdr(NODE_PTR(a))) == NOT) &&
            (car(cdr(NODE_PTR(a))) == cdr(NODE_PTR(b)))))) {
        return car(a);
      }

      /* ((A & B) || (!A & B)) ---> B */
      /* ((!A & B) || ( A & B)) ---> B */
      if ((cdr(a) == cdr(b)) &&
          (((node_get_type(car(b)) == NOT) &&
            (car(car(b)) == car(a))) ||
           ((node_get_type(car(a)) == NOT) &&
            (car(car(a)) == car(b))))) {
        return cdr(a);
      }

      /* (( A & B) || (B & !A)) ---> B */
      /* ((!A & B) || (B & A)) ---> B */
      if ((cdr(a) == car(b)) &&
          (((node_get_type(cdr(b)) == NOT) &&
            (car(cdr(b)) == car(a))) ||
           ((node_get_type(car(a)) == NOT) &&
            (car(car(a)) == cdr(b))))) {
        return cdr(a);
      }

      /* ((A & B) || (!B & A)) ---> A */
      /* ((A & !B) || ( B & A)) ---> A */
      if ((car(a) == cdr(b)) &&
          (((node_get_type(car(b)) == NOT) &&
            (car(car(b)) == cdr(a))) ||
           (((node_get_type(cdr(a)) == NOT) &&
             (car(cdr(a)) == car(b)))))) {
        return car(a);
      }
    }

    /* bitwise */
    if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
        (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {

#ifndef DISABLE_EXPR_POINTERS_ORDERING
      /* Swap if needed, for better sharing */
      if (car(a) > car(b)) {
        return find_node(ta,
                         (node_ptr) WordNumber_or(WORD_NUMBER(car(b)),
                                                  WORD_NUMBER(car(a))),
                         Nil);
      }
#endif
      return find_node(ta,
                       (node_ptr) WordNumber_or(WORD_NUMBER(car(a)),
                                                WORD_NUMBER(car(b))),
                       Nil);
    }
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible, but improve node sharing by
     ordering the children */
  if (a > b) {
    return EXPR( find_node(OR, NODE_PTR(b), NODE_PTR(a)) );
  }
#endif

  return EXPR( find_node(OR, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise XOR of given operators]

  Description        [Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_xor(const Expr_ptr a, const Expr_ptr b)
{
  /* boolean */
  if (Expr_is_true(a)) return Expr_not(b);
  if (Expr_is_true(b)) return Expr_not(a);
  if (Expr_is_false(a)) return b;
  if (Expr_is_false(b)) return a;

  {
    int ta = node_get_type(a); int tb = node_get_type(b);
    if ((ta == NOT && car(a) == b) ||
        (tb == NOT && car(b) == a)) return Expr_true();

    /* bitwise */
    if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
        (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {

#ifndef DISABLE_EXPR_POINTERS_ORDERING
      if (car(a) > car(b)) {
        return find_node(ta,
                         (node_ptr) WordNumber_xor(WORD_NUMBER(car(b)),
                                                   WORD_NUMBER(car(a))),
                         Nil);
      }
#endif
      return find_node(ta,
                       (node_ptr) WordNumber_xor(WORD_NUMBER(car(a)),
                                                 WORD_NUMBER(car(b))),
                       Nil);
    }
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible, order children by pointer for
     better node sharing */
  if (a > b) {
    return EXPR( find_node(XOR, NODE_PTR(b), NODE_PTR(a)) );
  }
#endif

  return EXPR( find_node(XOR, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise XNOR of given operators]

  Description        [Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_xnor(const Expr_ptr a, const Expr_ptr b)
{
  /* boolean */
  if (Expr_is_true(a)) return b;
  if (Expr_is_true(b)) return a;
  if (Expr_is_false(a)) return Expr_not(b);
  if (Expr_is_false(b)) return Expr_not(a);

  {
    int ta = node_get_type(a); int tb = node_get_type(b);

    if ((ta == NOT && car(a) == b) ||
        (tb == NOT && car(b) == a)) return Expr_false();

    /* bitwise */
    if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
        (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {

#ifndef DISABLE_EXPR_POINTERS_ORDERING
      if (car(a) > car(b)) {
        return find_node(ta,
                         (node_ptr) WordNumber_xnor(WORD_NUMBER(car(b)),
                                                    WORD_NUMBER(car(a))),
                         Nil);
      }
#endif

      return find_node(ta,
                       (node_ptr) WordNumber_xnor(WORD_NUMBER(car(a)),
                                                  WORD_NUMBER(car(b))),
                       Nil);
    }
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible, remember pointer ordering  */
  if (a > b) {
    return EXPR( find_node(XNOR, NODE_PTR(b), NODE_PTR(a)) );
  }
#endif

  return EXPR( find_node(XNOR, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise IFF of given operators]

  Description        [Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_iff(const Expr_ptr a, const Expr_ptr b)
{
  /* boolean */
  if (Expr_is_true(a)) return b;
  if (Expr_is_true(b)) return a;
  if (Expr_is_false(a)) return Expr_not(b);
  if (Expr_is_false(b)) return Expr_not(a);

  {
    int ta = node_get_type(a); int tb = node_get_type(b);
    if ((ta == NOT && car(a) == b) ||
        (tb == NOT && car(b) == a)) return Expr_false();

    /* bitwise */
    if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
        (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {

#ifndef DISABLE_EXPR_POINTERS_ORDERING
      if (car(a) > car(b)) {
        return find_node(ta,
                         (node_ptr) WordNumber_iff(WORD_NUMBER(car(b)),
                                                   WORD_NUMBER(car(a))),
                         Nil);
      }
#endif

      return find_node(ta,
                       (node_ptr) WordNumber_iff(WORD_NUMBER(car(a)),
                                                 WORD_NUMBER(car(b))),
                       Nil);
    }
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible, remember pointer ordering */
  if (a > b) {
    return EXPR( find_node(IFF, NODE_PTR(b), NODE_PTR(a)) );
  }
#endif

  return EXPR( find_node(IFF, NODE_PTR(a), NODE_PTR(b)) );
}

/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise IFF of given operators]

  Description        [Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_simplify_iff(const SymbTable_ptr st,
                           const Expr_ptr a, const Expr_ptr b)
{
  /* boolean */
  if (Expr_is_true(a)) return b;
  if (Expr_is_true(b)) return a;
  if (Expr_is_false(a)) return Expr_not(b);
  if (Expr_is_false(b)) return Expr_not(a);

  {
    int ta = node_get_type(a); int tb = node_get_type(b);
    if ((ta == NOT && car(a) == b) ||
        (tb == NOT && car(b) == a)) return Expr_false();

    /* bitwise */
    if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
        (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {

#ifndef DISABLE_EXPR_POINTERS_ORDERING
      if (car(a) > car(b)) {
        return find_node(ta,
                         (node_ptr) WordNumber_iff(WORD_NUMBER(car(b)),
                                                   WORD_NUMBER(car(a))),
                         Nil);
      }
#endif

      return find_node(ta,
                       (node_ptr) WordNumber_iff(WORD_NUMBER(car(a)),
                                                 WORD_NUMBER(car(b))),
                       Nil);
    }
  }

  if (SYMB_TABLE(NULL) != st) {
    SymbType_ptr at, bt;
    TypeChecker_ptr tc = SymbTable_get_type_checker(st);

    at = TypeChecker_get_expression_type(tc, NODE_PTR(a), Nil);
    bt = TypeChecker_get_expression_type(tc, NODE_PTR(b), Nil);

    if (!SymbType_is_word(at) || !SymbType_is_word(bt)) {
      /* For non word expressions A <-> A is true */
      if (a == b) {
        return Expr_true();
      }
    }
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible, but order pointers */
  if (a > b) {
    return EXPR( find_node(IFF, NODE_PTR(b), NODE_PTR(a)) );
  }
#endif

  return EXPR( find_node(IFF, NODE_PTR(a), NODE_PTR(b)) );
}

/**Function********************************************************************

  Synopsis           [Builds the logical/bitwise IMPLIES of given operators]

  Description        [Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_implies(const Expr_ptr a, const Expr_ptr b)
{
  /* boolean */
  if (Expr_is_true(a))  return b;
  if (Expr_is_false(a)) return Expr_true();
  if (Expr_is_true(b))  return Expr_true();
  if (Expr_is_false(b)) return Expr_not(a);

  {
    int ta = node_get_type(a); int tb = node_get_type(b);

    if ((ta == NOT && car(a) == b) ||
        (tb == NOT && car(b) == a)) return b;

    /* bitwise */
    if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
        (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {
      return find_node(ta,
                       (node_ptr) WordNumber_implies(WORD_NUMBER(car(a)),
                                                     WORD_NUMBER(car(b))),
                       Nil);
    }
  }

  /* no simplification is possible */
  return Expr_or(Expr_not(a), b);
}


/**Function********************************************************************

  Synopsis           [Builds the If-Then-Else node with given operators]

  Description [Performs local syntactic simplification. 'cond' is the
  case/ite condition, 't' is the THEN expression, 'e' is the ELSE
  expression]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_ite(const Expr_ptr cond,
                  const Expr_ptr t,
                  const Expr_ptr e,
                  SymbTable_ptr symb_table)
#else
Expr_ptr Expr_ite(const Expr_ptr cond,
                  const Expr_ptr t,
                  const Expr_ptr e,
                  const SymbTable_ptr symb_table)
#endif
{
  node_ptr tmp;

#ifdef SYMB_TABLE_STATIC
  symb_table = expr_st;
#endif


  if (Expr_is_true(cond)) return t;
  if (Expr_is_false(cond)) return e;

  if (t == e) return t;

  /* ITE(cond, TRUE, FALSE) -> cond */
  if (Expr_is_true(t) && Expr_is_false(e)) return cond;

  /* ITE(cond, FALSE, TRUE) -> NOT cond */
  if (Expr_is_false(t) && Expr_is_true(e)) return Expr_not(cond);

  /* We can apply simplifications only if the return type is not a
     set, because only CASE expressions allow sets. */
  if (Expr_is_false(t)) {
    if (FAILURE == node_get_type(e)) {
      warning_failure_node(e);
      return Expr_not(cond);
    }
    else if (SYMB_TABLE(NULL) != symb_table) {
      TypeChecker_ptr tc = SymbTable_get_type_checker(symb_table);
      SymbType_ptr et = TypeChecker_get_expression_type(tc, e, Nil);
      if (!SymbType_is_set(et)) { return Expr_and(Expr_not(cond), e); }
    }
  }
  if (Expr_is_true(t)) {
    if (FAILURE == node_get_type(e)) {
      warning_failure_node(e);
      return cond;
    }
    else if (SYMB_TABLE(NULL) != symb_table) {
      TypeChecker_ptr tc = SymbTable_get_type_checker(symb_table);
      SymbType_ptr et = TypeChecker_get_expression_type(tc, e, Nil);
      if (!SymbType_is_set(et)) { return Expr_or(cond, e); }
    }
  }

  if (Expr_is_false(e) && (SYMB_TABLE(NULL) != symb_table)) {
      TypeChecker_ptr tc = SymbTable_get_type_checker(symb_table);
      SymbType_ptr tt = TypeChecker_get_expression_type(tc, t, Nil);
      if (!SymbType_is_set(tt)) { return Expr_and(cond, t); }
  }
  if (Expr_is_true(e) && (SYMB_TABLE(NULL) != symb_table)) {
    TypeChecker_ptr tc = SymbTable_get_type_checker(symb_table);
    SymbType_ptr tt = TypeChecker_get_expression_type(tc, t, Nil);
    if (!SymbType_is_set(tt)) { return Expr_or(Expr_not(cond), t); }
  }

  /*
     case                case
        C1 : E1;            C1 | C2 : E1;
        C2 : E1;  --->      C3 : E2;
        C3 : E2;         esac
     esac
  */
  {
    if ((CASE == node_get_type(e)) ||
        (IFTHENELSE == node_get_type(e))) {
      node_ptr _c, _t, _e;

      nusmv_assert(COLON == node_get_type(car(e)));

      _c = car(car(e));
      _t = cdr(car(e));
      _e = cdr(e);

      if (_t == t) {
        return Expr_ite(Expr_or(cond, _c), t, _e, symb_table);
      }
    }
  }

  /*
     case
         cond1 : case
                   cond1 : expr1;
                   ...

                 esac;
         ...
     esac;

     simplifies into

     case
         cond1 : expr1;
         ...
     esac
  */
  if (((CASE == node_get_type(t)) ||
       (IFTHENELSE == node_get_type(t))) &&
      (cond == car(car(t)))) {
    tmp = find_node(COLON, NODE_PTR(cond), cdr(car(NODE_PTR(t))));
  }
  else {
    tmp = find_node(COLON, NODE_PTR(cond), NODE_PTR(t));
  }
  return EXPR( find_node(CASE, tmp, NODE_PTR(e)) );
}


/**Function********************************************************************

  Synopsis           [Constructs a NEXT node of given expression]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_next(const Expr_ptr a, SymbTable_ptr symb_table)
#else
Expr_ptr Expr_next(const Expr_ptr a, const SymbTable_ptr symb_table)
#endif
{
  int ta;

#ifdef SYMB_TABLE_STATIC
  symb_table = expr_st;
#endif

  /* boolean constant */
  if (Expr_is_true(a) || Expr_is_false(a)) return a;

  /* scalar constants */
  ta = node_get_type(a);
  if (ta == NUMBER || ta == NUMBER_UNSIGNED_WORD || ta == NUMBER_SIGNED_WORD) {
    return a;
  }

  /* a range? */
  if (ta == TWODOTS &&
      NUMBER == node_get_type(car(a)) &&
      NUMBER == node_get_type(cdr(a))) {
    return a;
  }

  /* enumerative? */
  if (symb_table != SYMB_TABLE(NULL) &&
      SymbTable_is_symbol_constant(symb_table, a)) {
    return a;
  }

  /* set of constants ? */
  if (symb_table != SYMB_TABLE(NULL) && UNION == node_get_type(a)) {
    Set_t set = Set_MakeFromUnion(a);
    boolean is_const = true;
    Set_Iterator_t iter;
    SET_FOREACH(set, iter) {
      if (!SymbTable_is_symbol_constant(symb_table,
                                        (node_ptr) Set_GetMember(set, iter))) {
        is_const = false;
        break;
      }
    }

    Set_ReleaseSet(set);
    if (is_const) return a;
  }

  /* fall back */
  return EXPR( find_node(NEXT, NODE_PTR(a), Nil)  );
}

/**Function********************************************************************

  Synopsis           [Builds the logical EQUAL of given operators]

  Description [Works with boolean, scalar and words. Performs local
  syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_equal(const Expr_ptr a,
                    const Expr_ptr b,
                    SymbTable_ptr st)
#else
Expr_ptr Expr_equal(const Expr_ptr a,
                    const Expr_ptr b,
                    const SymbTable_ptr st)
#endif
{
#ifdef SYMB_TABLE_STATIC
  st = expr_st;
#endif

  if (a == b) return Expr_true();
  if (Expr_is_true(a) && Expr_is_true(b)) return Expr_true();
  if (Expr_is_true(a) && Expr_is_false(b)) return Expr_false();
  if (Expr_is_false(a) && Expr_is_false(b)) return Expr_true();
  if (Expr_is_false(a) && Expr_is_true(b)) return Expr_false();

  {
    int ta, tb;
    ta = node_get_type(a); tb = node_get_type(b);

    if ((ta == NOT && car(a) == b) ||
        (tb == NOT && car(b) == a)) return Expr_false();

    /* scalar constants */
    if (NUMBER == ta && NUMBER == tb) {
      int va = node_get_int(a);
      int vb = node_get_int(b);
      return (va == vb) ? Expr_true() : Expr_false();
    }
    /* words */
    else if (NUMBER_UNSIGNED_WORD == ta || NUMBER_UNSIGNED_WORD == tb ||
             NUMBER_SIGNED_WORD == ta || NUMBER_SIGNED_WORD == tb) {
      WordNumber_ptr va =
        (NUMBER_UNSIGNED_WORD == ta || NUMBER_SIGNED_WORD == ta)
        ? WORD_NUMBER(car(a)) : WORD_NUMBER(NULL);
      WordNumber_ptr vb =
        (NUMBER_UNSIGNED_WORD == tb || NUMBER_SIGNED_WORD == tb)
        ? WORD_NUMBER(car(b)) : WORD_NUMBER(NULL);

      if (va != WORD_NUMBER(NULL) && vb != (WORD_NUMBER(NULL)))
        return WordNumber_equal(va, vb)
          ? Expr_true() : Expr_false();
    }
  }

  /* additional simplifications */
  if (SYMB_TABLE(NULL) != st) {
    SymbType_ptr ta, tb;
    TypeChecker_ptr tc = SymbTable_get_type_checker(st);

    /* enumerative? */
    if (SymbTable_is_symbol_constant(st, a) &&
        SymbTable_is_symbol_constant(st, b)) {
      return (a == b) ? Expr_true() : Expr_false();
    }

    /* TRUE = B --------> B */
    if (Expr_is_true(a)) {
      tb = TypeChecker_get_expression_type(tc, b, Nil);
      if (SymbType_is_boolean(tb)) {
        return b;
      }
    }
    /* A = TRUE --------> A */
    else if (Expr_is_true(b)) {
      ta = TypeChecker_get_expression_type(tc, a, Nil);
      if (SymbType_is_boolean(ta)) {
        return a;
      }
    }
    /* FALSE = B --------> !B */
    else if (Expr_is_false(a)) {
      tb = TypeChecker_get_expression_type(tc, b, Nil);
      if (SymbType_is_boolean(tb)) {
        return Expr_not(b);
      }
    }
    /* A = FALSE --------> !A */
    else if (Expr_is_false(b)) {
      ta = TypeChecker_get_expression_type(tc, a, Nil);
      if (SymbType_is_boolean(ta)) {
        return Expr_not(a);
      }
    }
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible, remember ordering */
  if (a > b) {
    return EXPR( find_node(EQUAL, NODE_PTR(b), NODE_PTR(a)) );
  }
#endif

  return EXPR( find_node(EQUAL, NODE_PTR(a), NODE_PTR(b)) );
}

/**Function********************************************************************

  Synopsis           [Builds the logical NOTEQUAL of given operators]

  Description        [Works with boolean, scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_notequal(const Expr_ptr a,
                       const Expr_ptr b,
                       SymbTable_ptr st)
#else
Expr_ptr Expr_notequal(const Expr_ptr a,
                       const Expr_ptr b,
                       const SymbTable_ptr st)
#endif
{

#ifdef SYMB_TABLE_STATIC
  st = expr_st;
#endif

  if (a == b) return Expr_false();
  if (Expr_is_true(a) && Expr_is_true(b)) return Expr_false();
  if (Expr_is_true(a) && Expr_is_false(b)) return Expr_true();
  if (Expr_is_false(a) && Expr_is_false(b)) return Expr_false();
  if (Expr_is_false(a) && Expr_is_true(b)) return Expr_true();

  {
    int ta, tb;
    ta = node_get_type(a); tb = node_get_type(b);

    if ((ta == NOT && car(a) == b) ||
        (tb == NOT && car(b) == a)) return Expr_true();

    /* scalar constants */
    if (NUMBER == ta && NUMBER == tb) {
      int va = node_get_int(a);
      int vb = node_get_int(b);
      return (va != vb) ? Expr_true() : Expr_false();
    }
    else if (NUMBER_UNSIGNED_WORD == ta || NUMBER_UNSIGNED_WORD == tb ||
             NUMBER_SIGNED_WORD == ta || NUMBER_SIGNED_WORD == tb) {
      WordNumber_ptr va =
        (NUMBER_UNSIGNED_WORD == ta || NUMBER_SIGNED_WORD == ta)
        ? WORD_NUMBER(car(a)) : WORD_NUMBER(NULL);
      WordNumber_ptr vb =
        (NUMBER_UNSIGNED_WORD == tb || NUMBER_SIGNED_WORD == tb)
        ? WORD_NUMBER(car(b)) : WORD_NUMBER(NULL);

      if (va != WORD_NUMBER(NULL) && vb != (WORD_NUMBER(NULL)))
        return WordNumber_not_equal(va, vb)
          ? Expr_true() : Expr_false();
    }
  }

  if (SYMB_TABLE(NULL) != st) {
    SymbType_ptr ta, tb;
    TypeChecker_ptr tc = SymbTable_get_type_checker(st);

    /* enumerative? */
    if (SymbTable_is_symbol_constant(st, a) &&
        SymbTable_is_symbol_constant(st, b)) {
      return (a == b) ? Expr_false() : Expr_true();
    }

    /* TRUE != B --------> !B */
    if (Expr_is_true(a)) {
      tb = TypeChecker_get_expression_type(tc, b, Nil);
      if (SymbType_is_boolean(tb)) {
        return Expr_not(b);
      }
    }
    /* A != TRUE --------> !A */
    else if (Expr_is_true(b)) {
      ta = TypeChecker_get_expression_type(tc, a, Nil);
      if (SymbType_is_boolean(ta)) {
        return Expr_not(a);
      }
    }
    /* FALSE != B --------> B */
    else if (Expr_is_false(a)) {
      tb = TypeChecker_get_expression_type(tc, b, Nil);
      if (SymbType_is_boolean(tb)) {
        return b;
      }
    }
    /* A != FALSE --------> A */
    else if (Expr_is_false(b)) {
      ta = TypeChecker_get_expression_type(tc, a, Nil);
      if (SymbType_is_boolean(ta)) {
        return a;
      }
    }
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible */
  if (a > b) {
    return EXPR( find_node(NOTEQUAL, NODE_PTR(b), NODE_PTR(a)) );
  }
#endif

  return EXPR( find_node(NOTEQUAL, NODE_PTR(a), NODE_PTR(b)) );
}

/**Function********************************************************************

  Synopsis           [Builds the predicate LT (less-then) of given operators]

  Description        [Works with scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_lt(const Expr_ptr a, const Expr_ptr b)
{
  if (a == b) return Expr_false();

  /* Booleans are not valid for this operator */
  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  {
    int ta, tb;
    ta = node_get_type(a); tb = node_get_type(b);

    /* scalar constants */
    if (NUMBER == ta && NUMBER == tb) {
      int va = node_get_int(a);
      int vb = node_get_int(b);
      return (va < vb) ? Expr_true() : Expr_false();
    }
    else if (NUMBER_UNSIGNED_WORD == ta || NUMBER_UNSIGNED_WORD == tb ||
             NUMBER_SIGNED_WORD == ta || NUMBER_SIGNED_WORD == tb) {

      WordNumber_ptr va =
        (NUMBER_UNSIGNED_WORD == ta || NUMBER_SIGNED_WORD == ta)
        ? WORD_NUMBER(car(a)) : WORD_NUMBER(NULL);
      WordNumber_ptr vb =
        (NUMBER_UNSIGNED_WORD == tb || NUMBER_SIGNED_WORD == tb)
        ? WORD_NUMBER(car(b)) : WORD_NUMBER(NULL);

      /* if both are constants => evaluate */
      if (va != NULL && vb != NULL) {
        nusmv_assert(ta == tb); /* signess has to be the same by type rules */

        return (NUMBER_UNSIGNED_WORD == ta
                ? WordNumber_unsigned_less(va, vb)
                : WordNumber_signed_less(va, vb))
          ? Expr_true() : Expr_false();
      }
      /* expr < uwconst(<size>,0)  =========> FALSE
         uwconst(<size>,max_value) < expr =========> FALSE
         swconst(<size>,max_value) < expr =========> FALSE */
      else if ((tb == NUMBER_UNSIGNED_WORD &&
                WordNumber_is_zero(vb))
               ||
               (ta == NUMBER_UNSIGNED_WORD &&
                WordNumber_get_unsigned_value(va) ==
                WordNumber_max_unsigned_value(WordNumber_get_width(va)))
               ||
               (ta == NUMBER_SIGNED_WORD &&
                WordNumber_get_signed_value(va) ==
                WordNumber_max_signed_value(WordNumber_get_width(va)))) {
        return Expr_false();
      }
      /* go to no-simplification return */
    }
  }

  /* no simplification is possible */
  return EXPR( find_node(LT, NODE_PTR(a), NODE_PTR(b)) );
}

/**Function********************************************************************

  Synopsis           [Builds the predicate LT (less-then) of given operators]

  Description        [Works with scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_simplify_lt(const SymbTable_ptr st,
                          const Expr_ptr a, const Expr_ptr b)
{
  Expr_ptr res = Expr_lt(a, b);
  if (Expr_is_true(res) || Expr_is_false(res)) return res;

  /* no simplification is possible */
  return res;
}

/**Function********************************************************************

  Synopsis           [Builds the predicate LE (less-then-equal)
  of given operators]

  Description        [Works with scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_le(const Expr_ptr a,
                 const Expr_ptr b,
                 SymbTable_ptr symb_table)
#else
Expr_ptr Expr_le(const Expr_ptr a,
                 const Expr_ptr b,
                 const SymbTable_ptr symb_table)
#endif
{
#ifdef SYMB_TABLE_STATIC
  symb_table = expr_st;
#endif

  if (a == b) return Expr_true();
  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  {
    int ta, tb;
    ta = node_get_type(a); tb = node_get_type(b);

    /* scalar constants */
    if (NUMBER == ta && NUMBER == tb) {
      int va = node_get_int(a);
      int vb = node_get_int(b);
      return (va <= vb) ? Expr_true() : Expr_false();
    }
    /* words */
    else if (NUMBER_UNSIGNED_WORD == ta || NUMBER_UNSIGNED_WORD == tb ||
             NUMBER_SIGNED_WORD == ta || NUMBER_SIGNED_WORD == tb) {
      WordNumber_ptr va =
        (NUMBER_UNSIGNED_WORD == ta || NUMBER_SIGNED_WORD == ta)
        ? WORD_NUMBER(car(a)) : WORD_NUMBER(NULL);
      WordNumber_ptr vb =
        (NUMBER_UNSIGNED_WORD == tb || NUMBER_SIGNED_WORD == tb)
        ? WORD_NUMBER(car(b)) : WORD_NUMBER(NULL);

      /* if both are constants => evaluate */
      if (va != NULL && vb != NULL) {
        nusmv_assert(ta == tb); /* signess has to be the same by type rules */

        return (NUMBER_UNSIGNED_WORD == ta
                ? WordNumber_unsigned_less_or_equal(va, vb)
                : WordNumber_signed_less_or_equal(va, vb))
          ? Expr_true() : Expr_false();
      }
      /* expr <= uwconst(<size>,0) =========> expr = uwconst(<size>,0) */
      else if (tb == NUMBER_UNSIGNED_WORD &&
               WordNumber_is_zero(vb)) {
        return Expr_equal(a,b, symb_table);
      }
      /* uwconst(<size>,0) <= expr =========> TRUE
         expr <= uwconst(<size>,max_value) =========> TRUE
         expr <= swconst(<size>,max_value) =========> TRUE */
      else if ((ta == NUMBER_UNSIGNED_WORD &&
                WordNumber_is_zero(va))
               ||
               (tb == NUMBER_UNSIGNED_WORD &&
                WordNumber_get_unsigned_value(vb) ==
                WordNumber_max_unsigned_value(WordNumber_get_width(vb)))
               ||
               (tb == NUMBER_SIGNED_WORD &&
                WordNumber_get_signed_value(vb) ==
                WordNumber_max_signed_value(WordNumber_get_width(vb)))) {
        return Expr_true();
      }
      /* go to no-simplification return */
    }
  }

  /* no simplification is possible */
  return EXPR( find_node(LE, NODE_PTR(a), NODE_PTR(b)) );
}

/**Function********************************************************************

  Synopsis           [Builds the predicate GT (greater-then)
  of given operators]

  Description        [Works with scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_gt(const Expr_ptr a, const Expr_ptr b)
{
  if (a == b) return Expr_false();

  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  {
    int ta, tb;
    ta = node_get_type(a); tb = node_get_type(b);

    /* scalar constants */
    if (NUMBER == ta && NUMBER == tb) {
      int va = node_get_int(a);
      int vb = node_get_int(b);
      return (va >= vb) ? Expr_true() : Expr_false();
    }
    /* words */
    else if (NUMBER_UNSIGNED_WORD == ta || NUMBER_UNSIGNED_WORD == tb ||
             NUMBER_SIGNED_WORD == ta || NUMBER_SIGNED_WORD == tb) {
      WordNumber_ptr va =
        (NUMBER_UNSIGNED_WORD == ta || NUMBER_SIGNED_WORD == ta)
        ? WORD_NUMBER(car(a)) : WORD_NUMBER(NULL);
      WordNumber_ptr vb =
        (NUMBER_UNSIGNED_WORD == tb || NUMBER_SIGNED_WORD == tb)
        ? WORD_NUMBER(car(b)) : WORD_NUMBER(NULL);

      /* if both are constants => evaluate */
      if (va != NULL && vb != NULL) {
        nusmv_assert(ta == tb); /* signess has to be the same by type rules */

        return (NUMBER_UNSIGNED_WORD == ta
                ? WordNumber_unsigned_greater(va, vb)
                : WordNumber_signed_greater(va, vb))
          ? Expr_true() : Expr_false();
      }
      /* uwconst(<size>,0) > expr =========> FALSE
         expr > uwconst(<size>,max_value) =========> FALSE
         expr > swconst(<size>,max_value) =========> FALSE */
      else if ((ta == NUMBER_UNSIGNED_WORD &&
                WordNumber_is_zero(va))
               ||
               (tb == NUMBER_UNSIGNED_WORD &&
                WordNumber_get_unsigned_value(vb) ==
                WordNumber_max_unsigned_value(WordNumber_get_width(vb)))
               ||
               (tb == NUMBER_SIGNED_WORD &&
                WordNumber_get_signed_value(vb) ==
                WordNumber_max_signed_value(WordNumber_get_width(vb)))) {
        return Expr_false();
      }
    }
  }

  /* no simplification is possible */
  return EXPR( find_node(GT, NODE_PTR(a), NODE_PTR(b)) );
}

/**Function********************************************************************

  Synopsis           [Builds the predicate GT (greater-then)
  of given operators]

  Description        [Works with boolean, scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_simplify_gt(const SymbTable_ptr st,
                          const Expr_ptr a, const Expr_ptr b)
{
  Expr_ptr res = Expr_gt(a, b);

  return res;
}

/**Function********************************************************************

  Synopsis           [Builds the predicate GE (greater-then-equal)
  of given operators]

  Description        [Works with boolean, scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_ge(const Expr_ptr a,
                 const Expr_ptr b,
                 SymbTable_ptr symb_table)
#else
Expr_ptr Expr_ge(const Expr_ptr a,
                 const Expr_ptr b,
                 const SymbTable_ptr symb_table)
#endif
{
#ifdef SYMB_TABLE_STATIC
  symb_table = expr_st;
#endif
  if (a == b) return Expr_true();

  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  {
    int ta, tb;
    ta = node_get_type(a); tb = node_get_type(b);

    /* scalar constants */
    if (NUMBER == ta && NUMBER == tb) {
      int va = node_get_int(a);
      int vb = node_get_int(b);
      return (va >= vb) ? Expr_true() : Expr_false();
    }
    /* words */
    else if (NUMBER_UNSIGNED_WORD == ta || NUMBER_UNSIGNED_WORD == tb ||
             NUMBER_SIGNED_WORD == ta || NUMBER_SIGNED_WORD == tb) {
      WordNumber_ptr va =
        (NUMBER_UNSIGNED_WORD == ta || NUMBER_SIGNED_WORD == ta)
        ? WORD_NUMBER(car(a)) : WORD_NUMBER(NULL);
      WordNumber_ptr vb =
        (NUMBER_UNSIGNED_WORD == tb || NUMBER_SIGNED_WORD == tb)
        ? WORD_NUMBER(car(b)) : WORD_NUMBER(NULL);

      /* if both are constants => evaluate */
      if (va != NULL && vb != NULL) {
        nusmv_assert(ta == tb); /* signess has to be the same by type rules */

        return (NUMBER_UNSIGNED_WORD == ta
                ? WordNumber_unsigned_greater_or_equal(va, vb)
                : WordNumber_signed_greater_or_equal(va, vb))
          ? Expr_true() : Expr_false();
      }
      /*  uwconst(<size>,0) >= expr =========> uwconst(<size>,0) = expr*/
      else if (ta == NUMBER_UNSIGNED_WORD &&
               WordNumber_is_zero(va)) {
        return Expr_equal(a,b, symb_table);
      }
      /* expr >= uwconst(<size>,0) =========> TRUE
         uwconst(<size>,max_value) >= expr =========> TRUE
         swconst(<size>,max_value) >= expr=========> TRUE */
      else if ((tb == NUMBER_UNSIGNED_WORD &&
                WordNumber_is_zero(vb))
               ||
               (ta == NUMBER_UNSIGNED_WORD &&
                WordNumber_get_unsigned_value(va) ==
                WordNumber_max_unsigned_value(WordNumber_get_width(va)))
               ||
               (ta == NUMBER_SIGNED_WORD &&
                WordNumber_get_signed_value(va) ==
                WordNumber_max_signed_value(WordNumber_get_width(va)))) {
        return Expr_true();
      }
      /* go to no-simplification return */
    }
  }

  /* no simplification is possible */
  return EXPR( find_node(GE, NODE_PTR(a), NODE_PTR(b)) );
}

/**Function********************************************************************

  Synopsis           [Builds the scalar node for PLUS of given operators]

  Description        [Works with boolean, scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_plus(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);
  int tb = node_get_type(b);

  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  if (ta == NUMBER && tb == NUMBER) {
    return find_node(NUMBER,
                     NODE_FROM_INT((node_get_int(a) +
                                   node_get_int(b))),
                     Nil);
  }

  if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
      (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {

#ifndef DISABLE_EXPR_POINTERS_ORDERING
    if (car(a) > car(b)) {
      return find_node(ta,
                       (node_ptr) WordNumber_plus(WORD_NUMBER(car(b)),
                                                  WORD_NUMBER(car(a))),
                       Nil);
    }
#endif

    return find_node(ta,
                     (node_ptr) WordNumber_plus(WORD_NUMBER(car(a)),
                                                WORD_NUMBER(car(b))),
                     Nil);
  }

  /* 0 + A = A */
  if (((ta == NUMBER) && (0 == node_get_int(a))) ||
      (((ta == NUMBER_SIGNED_WORD) || (ta == NUMBER_UNSIGNED_WORD)) &&
       WordNumber_is_zero(WORD_NUMBER(car(a))))) {
    return b;
  }
  /* A + 0 = A */
  if (((tb == NUMBER) && (0 == node_get_int(b))) ||
      (((tb == NUMBER_SIGNED_WORD) || (tb == NUMBER_UNSIGNED_WORD)) &&
       WordNumber_is_zero(WORD_NUMBER(car(b))))) {
    return a;
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible, remember pointer ordering */
  if (a > b) {
    return EXPR( find_node(PLUS, NODE_PTR(b), NODE_PTR(a)) );
  }
#endif

  return EXPR( find_node(PLUS, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the scalar node for MINUS of given operators]

  Description        [Works with boolean, scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_minus(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);
  int tb = node_get_type(b);

  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  if (ta == NUMBER && tb == NUMBER) {
    return find_node(NUMBER,
                     NODE_FROM_INT((node_get_int(a) -
                                   node_get_int(b))),
                     Nil);
  }

  if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
      (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {
    return find_node(ta,
                     (node_ptr) WordNumber_minus(WORD_NUMBER(car(a)),
                                                 WORD_NUMBER(car(b))),
                     Nil);
  }

  /* 0 - A = -A */
  if (((ta == NUMBER) && (0 == node_get_int(a))) ||
      (((ta == NUMBER_SIGNED_WORD) || (ta == NUMBER_UNSIGNED_WORD)) &&
       WordNumber_is_zero(WORD_NUMBER(car(a))))) {
    return Expr_unary_minus(b);
  }
  /* A - 0 = A */
  if (((tb == NUMBER) && (0 == node_get_int(b))) ||
      (((tb == NUMBER_SIGNED_WORD) || (tb == NUMBER_UNSIGNED_WORD))  &&
       WordNumber_is_zero(WORD_NUMBER(car(b))))) {
    return a;
  }

  /* no simplification is possible */
  return EXPR( find_node(MINUS, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the scalar node for TIMES of given operators]

  Description        [Works with boolean, scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_times(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);
  int tb = node_get_type(b);

  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  if (ta == NUMBER && tb == NUMBER) {
    return find_node(NUMBER,
                     NODE_FROM_INT((node_get_int(a) *
                                   node_get_int(b))),
                     Nil);
  }
  if ((ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) ||
      (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD)) {

#ifndef DISABLE_EXPR_POINTERS_ORDERING
    if (car(a) > car(b)) {
      return find_node(ta,
                       (node_ptr) WordNumber_times(WORD_NUMBER(car(b)),
                                                   WORD_NUMBER(car(a))),
                       Nil);
    }
#endif

    return find_node(ta,
                     (node_ptr) WordNumber_times(WORD_NUMBER(car(a)),
                                                 WORD_NUMBER(car(b))),
                     Nil);
  }

  /* 0 * A = 0 */
  if (((ta == NUMBER) && (0 == node_get_int(a))) ||
      ((tb == NUMBER) && (0 == node_get_int(b)))) {
    return find_node(NUMBER, NODE_FROM_INT(0), Nil);
  }
  /* A * 0 = 0 */
  if ((((ta == NUMBER_SIGNED_WORD) || (ta == NUMBER_UNSIGNED_WORD)) &&
       WordNumber_is_zero(WORD_NUMBER(car(a)))) ||
      (((tb == NUMBER_SIGNED_WORD) || (tb == NUMBER_UNSIGNED_WORD)) &&
       WordNumber_is_zero(WORD_NUMBER(car(b))))) {
    return ((ta == NUMBER_SIGNED_WORD) ||
            (ta == NUMBER_UNSIGNED_WORD)) ? a : b;
  }

#ifndef DISABLE_EXPR_POINTERS_ORDERING
  /* no simplification is possible, remember pointer ordering */
  if (a > b) {
    return EXPR( find_node(TIMES, NODE_PTR(b), NODE_PTR(a)) );
  }
#endif

  return EXPR( find_node(TIMES, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the scalar node for DIVIDE of given operators]

  Description        [Works with boolean, scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_divide(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);
  int tb = node_get_type(b);

  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  if (ta == NUMBER && tb == NUMBER) {
    int vb = node_get_int(b);
    if (vb == 0) error_div_by_zero(b);
    return find_node(NUMBER,
                     NODE_FROM_INT((node_get_int(a) / vb)),
                     Nil);
  }
  if (ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) {
    if (WordNumber_is_zero(WORD_NUMBER(car(b)))) error_div_by_zero(b);
    return find_node(ta,
            (node_ptr) WordNumber_unsigned_divide(WORD_NUMBER(car(a)),
                                                  WORD_NUMBER(car(b))),
                     Nil);
  }
  if (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD) {
    if (WordNumber_is_zero(WORD_NUMBER(car(b)))) error_div_by_zero(b);
    return find_node(ta,
            (node_ptr) WordNumber_signed_divide(WORD_NUMBER(car(a)),
                                                WORD_NUMBER(car(b))),
                     Nil);
  }

  /* no simplification is possible */
  return EXPR( find_node(DIVIDE, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the scalar node for MODule of given operators]

  Description        [Works with boolean, scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_mod(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);
  int tb = node_get_type(b);

  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  if (ta == NUMBER && tb == NUMBER) {
    int vb = node_get_int(b);
    if (vb == 0) error_div_by_zero(b);

    return find_node(NUMBER,
                     NODE_FROM_INT((node_get_int(a) % vb)),
                     Nil);
  }
  if (ta == NUMBER_UNSIGNED_WORD && tb == NUMBER_UNSIGNED_WORD) {
    if (WordNumber_is_zero(WORD_NUMBER(car(b)))) error_div_by_zero(b);
    return find_node(ta,
            (node_ptr) WordNumber_unsigned_mod(WORD_NUMBER(car(a)),
                                               WORD_NUMBER(car(b))),
                     Nil);
  }
  if (ta == NUMBER_SIGNED_WORD && tb == NUMBER_SIGNED_WORD) {
    if (WordNumber_is_zero(WORD_NUMBER(car(b)))) error_div_by_zero(b);
    return find_node(ta,
            (node_ptr) WordNumber_signed_mod(WORD_NUMBER(car(a)),
                                             WORD_NUMBER(car(b))),
                     Nil);
  }

  /* no simplification is possible */
  return EXPR( find_node(MOD, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis [Builds the scalar node for UMINUS (unary minus) of given
  operators]

  Description        [Works with scalar and words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_unary_minus(const Expr_ptr a)
{
  nusmv_assert(! expr_is_bool(a));
  switch (node_get_type(a)) {
  case NUMBER: return find_node(NUMBER,
                                NODE_FROM_INT(-node_get_int(a)), Nil);
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
    return find_node(node_get_type(a),
                     (node_ptr) WordNumber_unary_minus(WORD_NUMBER(car(a))),
                     Nil);
  }

  /* no simplification is possible */
  return EXPR( find_node(UMINUS, NODE_PTR(a), Nil) );
}


/**Function********************************************************************

  Synopsis           [Builds the node left shifting of words.

  Description        [Works with words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_word_left_shift(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);

  if (ta == NUMBER_UNSIGNED_WORD || ta == NUMBER_SIGNED_WORD) {
    nusmv_assert(!expr_is_bool(b));

    int bits;
    switch (node_get_type(b)) {
    case NUMBER: bits = node_get_int(b); break;
    case NUMBER_UNSIGNED_WORD:
      bits = WordNumber_get_unsigned_value(WORD_NUMBER(car(b))); break;
    case NUMBER_SIGNED_WORD:
      bits = WordNumber_get_signed_value(WORD_NUMBER(car(b))); break;
    default: bits = -1;
    }

    if (bits == 0) return a;
    if (bits > 0) {
      if (bits > WordNumber_get_width(WORD_NUMBER(car(a)))) {
        error_wrong_word_operand("Right operand of shift is out of range", b);
      }
      return find_node(ta,
                       NODE_PTR(WordNumber_left_shift(WORD_NUMBER(car(a)),
                                                      bits)),
                       Nil);
    }
    /* b here is not a constant */
  }

  /* no simplification is possible */
  return EXPR( find_node(LSHIFT, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the node right shifting of words.

  Description        [Works with words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_word_right_shift(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);

  if (ta == NUMBER_UNSIGNED_WORD || ta == NUMBER_SIGNED_WORD) {
    nusmv_assert(!expr_is_bool(b));
    int bits;
    switch (node_get_type(b)) {
    case NUMBER: bits = node_get_int(b); break;
    case NUMBER_UNSIGNED_WORD:
      bits = WordNumber_get_unsigned_value(WORD_NUMBER(car(b))); break;
    case NUMBER_SIGNED_WORD:
      bits = WordNumber_get_signed_value(WORD_NUMBER(car(b))); break;
    default: bits = -1;
    }

    if (bits == 0) return a;
    if (bits > 0) {
      WordNumber_ptr rs;

      if (bits > WordNumber_get_width(WORD_NUMBER(car(a)))) {
        error_wrong_word_operand("Right operand of shift is out of range", b);
      }
      if (ta == NUMBER_UNSIGNED_WORD) {
        rs = WordNumber_unsigned_right_shift(WORD_NUMBER(car(a)), bits);
      }
      else {
        rs = WordNumber_signed_right_shift(WORD_NUMBER(car(a)), bits);
      }

      return find_node(ta, (node_ptr) rs, Nil);
    }
    /* b here is not a constant */
  }

  /* no simplification is possible */
  return EXPR( find_node(RSHIFT, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the node left rotation of words.

  Description        [Works with words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_word_left_rotate(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);

  if (ta == NUMBER_UNSIGNED_WORD || ta == NUMBER_SIGNED_WORD) {
    int bits;

    nusmv_assert(!expr_is_bool(b));

    switch (node_get_type(b)) {
    case NUMBER: bits = node_get_int(b); break;
    case NUMBER_UNSIGNED_WORD:
      bits = WordNumber_get_unsigned_value(WORD_NUMBER(car(b))); break;
    case NUMBER_SIGNED_WORD:
      bits = WordNumber_get_signed_value(WORD_NUMBER(car(b))); break;
    default: bits = -1;
    }

    if (bits == 0) return a;
    if (bits > 0) {
      if (bits > WordNumber_get_width(WORD_NUMBER(car(a)))) {
        error_wrong_word_operand("Right operand of rotate is out of range", b);
      }
      return find_node(ta,
                       NODE_PTR(WordNumber_left_rotate(WORD_NUMBER(car(a)),
                                                       bits)),
                       Nil);
    }
    /* b here is not a constant */
  }

  /* no simplification is possible */
  return EXPR( find_node(LROTATE, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the node right rotation of words.

  Description        [Works with words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_word_right_rotate(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);

  if (ta == NUMBER_UNSIGNED_WORD || ta == NUMBER_SIGNED_WORD) {
    int bits;
    nusmv_assert(!expr_is_bool(b));
    switch (node_get_type(b)) {
    case NUMBER: bits = node_get_int(b); break;
    case NUMBER_UNSIGNED_WORD:
      bits = WordNumber_get_unsigned_value(WORD_NUMBER(car(b))); break;
    case NUMBER_SIGNED_WORD:
      bits = WordNumber_get_signed_value(WORD_NUMBER(car(b))); break;
    default: bits = -1;
    }

    if (bits == 0) return a;
    if (bits > 0) {
      if (bits > WordNumber_get_width(WORD_NUMBER(car(a)))) {
        error_wrong_word_operand("Right operand of rotate is out of range", b);
      }
      return find_node(ta,
                       NODE_PTR(WordNumber_right_rotate(WORD_NUMBER(car(a)),
                                                        bits)),
                       Nil);
    }
    /* b here is not a constant */
  }

  /* no simplification is possible */
  return EXPR( find_node(RROTATE, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the node for bit selection of words.

  Description        [Works with words. Performs local syntactic
                      simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_word_bit_select(const Expr_ptr w, const Expr_ptr r)
{
  /* Expr_ptr _r = expr_bool_const_to_number(r); */
  if (/* Simplification can be done iff the range is constant. If the
         range is a constant expression, we simply return the
         BIT_SELECTION node */
      (NUMBER == node_get_type(car(r)) &&
       NUMBER == node_get_type(cdr(r))) &&

      (((node_get_type(w) == UNSIGNED_WORD ||
         node_get_type(w) == SIGNED_WORD) &&
        (node_word_get_width(w) > 0))
       || (node_get_type(w) == NUMBER_UNSIGNED_WORD)
       || (node_get_type(w) == NUMBER_SIGNED_WORD))) {
    return EXPR(node_word_selection(w, r));
  }

  return EXPR(find_node(BIT_SELECTION, w, r));
}


/**Function********************************************************************

  Synopsis           [Builds the node for bit selection of words.

  Description        [Works with words. Performs local semantic and syntactic
                      simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_simplify_word_bit_select(const SymbTable_ptr st,
                                       const Expr_ptr w, const Expr_ptr r)
{
  if (SYMB_TABLE(NULL) != st) {
    nusmv_assert(SYMB_TABLE(NULL) != st);
    TypeChecker_ptr tc = SymbTable_get_type_checker(st);
    SymbType_ptr wt = TypeChecker_get_expression_type(tc, w, Nil);
    int argt_width = SymbType_get_word_width(wt);
    node_ptr msb, lsb;

    /* Simplify constant expressions */
    msb = CompileFlatten_resolve_number(st, car(r), Nil);
    lsb = CompileFlatten_resolve_number(st, cdr(r), Nil);

    nusmv_assert(COLON == node_get_type(r));
    nusmv_assert(Nil != msb && Nil != lsb &&
                 NUMBER == node_get_type(msb) &&
                 NUMBER == node_get_type(lsb));

    int sel_msb = node_get_int(msb);
    int sel_lsb = node_get_int(lsb);

    /* these simplification apply to unsigned words only */
    if (SymbType_is_unsigned_word(wt)) {

      /* Discard useless bit selection operations */
      if (0 == sel_lsb && (argt_width -1) == sel_msb) return w;

      if (EXTEND == node_get_type(w)) {
        Expr_ptr _w = car(w);
        SymbType_ptr _wt = TypeChecker_get_expression_type(tc, _w, Nil);

        int orig_width = SymbType_get_word_width(_wt);
        nusmv_assert(0 < orig_width && argt_width >= orig_width);

        {
          Expr_ptr res = Nil;
          int pivot = orig_width; /* starting bit position for '0' padding */


          /* if the selection is from the extension only rewrite as as
             0 word constant of appropriate width */
          if (sel_lsb >= pivot) {
            res = \
              find_node(NUMBER_UNSIGNED_WORD,
                        NODE_PTR(WordNumber_from_integer(
                                                 0LL,
                                                 sel_msb - sel_lsb +1)),
                        Nil);
          }
          /* if the selection is from the original word only, discard the
             EXTEND operation */
          else if (sel_msb < pivot) {
            res = Expr_simplify_word_bit_select(st, _w, r);
          }
          /* if the selection is from both the extension and the original
             word, rewrite it as the extension to appropriate size of the
             selection of the relevant part of the word. */
          else {
            nusmv_assert(sel_msb >= pivot && pivot > sel_lsb);
            res = Expr_simplify_word_extend(st,
                    Expr_simplify_word_bit_select(st, _w,
                         find_node(COLON,
                                   find_node(NUMBER,
                                             NODE_FROM_INT(pivot-1), Nil),
                                   find_node(NUMBER,
                                             NODE_FROM_INT(sel_lsb), Nil))),
                    find_node(NUMBER,
                              NODE_FROM_INT(sel_msb - pivot +1), Nil));
          }

          return res;
        }
      }
    }
  }
  /* fallback */
  return Expr_word_bit_select(w, r);
}


/**Function********************************************************************

  Synopsis           [Builds the node for word concatenation.

  Description        [Works with words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_word_concatenate(const Expr_ptr a, const Expr_ptr b)
{
  int ta = node_get_type(a);
  int tb = node_get_type(b);

  nusmv_assert(! (expr_is_bool(a) || expr_is_bool(b)));

  if ((ta == NUMBER_UNSIGNED_WORD || ta == NUMBER_SIGNED_WORD) &&
      (tb == NUMBER_UNSIGNED_WORD || tb == NUMBER_SIGNED_WORD)) {
    return find_node(NUMBER_UNSIGNED_WORD,
                     (node_ptr) WordNumber_concatenate(WORD_NUMBER(car(a)),
                                                       WORD_NUMBER(car(b))),
                     Nil);
  }

  /* no simplification is possible */
  return EXPR( find_node(CONCATENATION, NODE_PTR(a), NODE_PTR(b)) );
}


/**Function********************************************************************

  Synopsis           [Builds the node for casting word1 to boolean.

  Description        [Works with words with width 1.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_word1_to_bool(Expr_ptr w)
{
  int tw = node_get_type(w);
  if (tw == NUMBER_UNSIGNED_WORD || tw == NUMBER_SIGNED_WORD) {
    WordNumber_ptr wn = WORD_NUMBER(car(w));
    return (WordNumber_get_unsigned_value(wn) != 0)
      ? Expr_true()
      : Expr_false();
  }

  /* no simplification is possible */
  return EXPR( find_node(CAST_BOOL, NODE_PTR(w), Nil) );
}


/**Function********************************************************************

  Synopsis           [Builds the node for casting boolean to word1.

  Description        [Works with booleans.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_bool_to_word1(Expr_ptr a)
{
  Expr_ptr _a = expr_bool_to_word1(a);
  if (_a != a) return _a;

   /* no simplification is possible */
  return EXPR( find_node(CAST_WORD1, NODE_PTR(a), Nil) );
}


/**Function********************************************************************

  Synopsis [Builds the node for casting signed words to unsigned
  words.

  Description        [Works with words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_signed_word_to_unsigned(Expr_ptr w)
{
  if (node_get_type(w) == NUMBER_SIGNED_WORD) {
    return find_node(NUMBER_UNSIGNED_WORD, car(w), cdr(w));
  }

   /* no simplification is possible */
  return EXPR( find_node(CAST_UNSIGNED, NODE_PTR(w), Nil) );
}


/**Function********************************************************************

  Synopsis [Builds the node for casting unsigned words to signed words.

  Description        [Works with words.
  Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_unsigned_word_to_signed(Expr_ptr w)
{
  if (node_get_type(w) == NUMBER_UNSIGNED_WORD) {
    return find_node(NUMBER_SIGNED_WORD, car(w), cdr(w));
  }

   /* no simplification is possible */
  return EXPR( find_node(CAST_SIGNED, NODE_PTR(w), Nil) );
}



/**Function********************************************************************

  Synopsis    [Builds the node for resizing a word.]

  Description [Works with words. Performs local syntactic simplification]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
Expr_ptr Expr_simplify_word_resize(const SymbTable_ptr st,
                                   Expr_ptr w,
                                   Expr_ptr i)
{
  Expr_ptr _i;
  int w_type = node_get_type(w);

  /* if (SYMB_TABLE(NULL) != st) { */
    _i = CompileFlatten_resolve_number(st, i, Nil);
  /* } */
  /* else _i = i; */

  if (Nil != _i &&
      NUMBER == node_get_type(_i) &&
      (NUMBER_UNSIGNED_WORD == w_type || NUMBER_SIGNED_WORD == w_type)) {

    int m = WordNumber_get_width(WORD_NUMBER(car(w)));

    int n = node_get_int(i); /* shouldn't be (_i)? */

    nusmv_assert(0 < n);

    if (m == n) { return w; }
    else if (m < n) {
      return Expr_simplify_word_extend(st, w,
                                find_node(NUMBER, NODE_FROM_INT(n - m), Nil));
    }
    else { /* n < m */
      if (NUMBER_UNSIGNED_WORD == node_get_type(w)) { /* unsigned */

        return Expr_word_bit_select(w,
                                    find_node(COLON,
                                              find_node(NUMBER,
                                                        NODE_FROM_INT(n - 1),
                                                        Nil),
                                              find_node(NUMBER,
                                                        NODE_FROM_INT(0),
                                                        Nil)));
      }
      else { /* signed */
        nusmv_assert(NUMBER_SIGNED_WORD ==  node_get_type(w));

        node_ptr msb_sel = \
          find_node(COLON,
                    find_node(NUMBER, NODE_FROM_INT(m-1), Nil),
                    find_node(NUMBER, NODE_FROM_INT(m-1), Nil));

        node_ptr rightmost_sel = \
          find_node(COLON,
                    find_node(NUMBER, NODE_FROM_INT(n-2), Nil),
                    find_node(NUMBER, NODE_FROM_INT(0), Nil));

        node_ptr nexpr = \
          Expr_word_concatenate(Expr_word_bit_select(w, msb_sel),
                                Expr_word_bit_select(w, rightmost_sel));

        return Expr_unsigned_word_to_signed(nexpr);
      }
    }
  }

  /* no simplification possible */
  return find_node(WRESIZE, w, _i);
}


/**Function********************************************************************

  Synopsis           [Builds the node for extending a word.]

  Description        [Works with words.
                      Performs local syntactic simplification]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_word_extend(Expr_ptr w, Expr_ptr i, SymbTable_ptr symb_table)
#else
Expr_ptr Expr_word_extend(Expr_ptr w, Expr_ptr i, const SymbTable_ptr symb_table)
#endif
{
#ifdef SYMB_TABLE_STATIC
  symb_table = expr_st;
#endif
  int tw = node_get_type(w);
  node_ptr _i;

  nusmv_assert(! expr_is_bool(i));

  _i = CompileFlatten_resolve_number(symb_table, i, Nil);
  nusmv_assert(Nil != _i && node_get_type(_i) == NUMBER);

  if (tw == NUMBER_UNSIGNED_WORD) {
    return find_node(NUMBER_UNSIGNED_WORD,
                     (node_ptr) WordNumber_unsigned_extend(WORD_NUMBER(car(w)),
                                                           node_get_int(_i)),
                     Nil);
  }
  if (tw == NUMBER_SIGNED_WORD) {
    return find_node(NUMBER_SIGNED_WORD,
                     (node_ptr) WordNumber_signed_extend(WORD_NUMBER(car(w)),
                                                         node_get_int(_i)),
                     Nil);
  }

   /* no simplification is possible */
  return EXPR( find_node(EXTEND, NODE_PTR(w), NODE_PTR(_i)) );
}


/**Function********************************************************************

  Synopsis    [Builds the node for extending a word.]

  Description [Works with words. Performs local syntactic
               simplification]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
Expr_ptr Expr_simplify_word_extend(const SymbTable_ptr st,
                                   Expr_ptr w, Expr_ptr i)
{
  Expr_ptr _i;
  int tw = node_get_type(w);

  _i = CompileFlatten_resolve_number(st, i, Nil);
  nusmv_assert(Nil != _i && node_get_type(_i) == NUMBER);

  if (tw == NUMBER_UNSIGNED_WORD) {
    return find_node(NUMBER_UNSIGNED_WORD,
                     (node_ptr) WordNumber_unsigned_extend(WORD_NUMBER(car(w)),
                                                           node_get_int(_i)),
                     Nil);
  }
  if (tw == NUMBER_SIGNED_WORD) {
    return find_node(NUMBER_SIGNED_WORD,
                     (node_ptr) WordNumber_signed_extend(WORD_NUMBER(car(w)),
                                                         node_get_int(_i)),
                     Nil);
  }

   /* no simplification is possible */
  return EXPR( find_node(EXTEND, NODE_PTR(w), NODE_PTR(_i)) );
}


/**Function********************************************************************

  Synopsis           [Creates a ATTIME node]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_attime(Expr_ptr e, int time, SymbTable_ptr symb_table)
#else
Expr_ptr Expr_attime(Expr_ptr e, int time, const SymbTable_ptr symb_table)
#endif

{
  int te;

#ifdef SYMB_TABLE_STATIC
  symb_table = expr_st;
#endif

  /* boolean constant */
  if (Expr_is_true(e) || Expr_is_false(e)) return e;

  /* scalar constants */
  te = node_get_type(e);
  if (te == NUMBER || te == NUMBER_UNSIGNED_WORD || te == NUMBER_SIGNED_WORD) {
    return e;
  }

  /* a range? */
  if (te == TWODOTS &&
      NUMBER == node_get_type(car(e)) && NUMBER == node_get_type(cdr(e))) {
    return e;
  }

  /* enumerative? */
  if (symb_table != SYMB_TABLE(NULL) &&
      SymbTable_is_symbol_constant(symb_table, e)) {
    return e;
  }

  /* set of constants ? */
  if (symb_table != SYMB_TABLE(NULL) && UNION == node_get_type(e)) {
    Set_t set = Set_MakeFromUnion(e);
    boolean is_const = true;
    Set_Iterator_t iter;
    SET_FOREACH(set, iter) {
      if (!SymbTable_is_symbol_constant(symb_table,
                                        (node_ptr) Set_GetMember(set, iter))) {
        is_const = false;
        break;
      }
    }

    Set_ReleaseSet(set);
    if (is_const) return e;
  }

  /* fallback */
  return find_node(ATTIME, e,
                   find_node(NUMBER, NODE_FROM_INT(time), Nil));
}


/**Function********************************************************************

  Synopsis           [Retrieves the time out of an ATTIME node]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int Expr_attime_get_time(Expr_ptr e)
{
  nusmv_assert(ATTIME == node_get_type(e));
  return node_get_int(cdr(e));
}


/**Function********************************************************************

  Synopsis           [Retrieves the untimed node out of an ATTIME node]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_attime_get_untimed(Expr_ptr e)
{
  nusmv_assert(ATTIME == node_get_type(e));
  return car(e);
}


/**Function********************************************************************

  Synopsis           [Makes a union node]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_union(const Expr_ptr a, const Expr_ptr b)
{
  Expr_ptr res;

  if (Nil == a) return b;
  if (Nil == b) return a;
  if (a == b) return a;

  res = find_node(UNION, a, b);


  /* If expression is recursively constructed of many UNION
     the below Set_MakeFromUnion will be applied many times on the
     same elements.
     All the uses of this function have to be checked.

     A better implementation is to construct UNION from Set_t of elements.

     Note that addition of new elements to Set_t has to be done
     with Set_MakeFromUnion as it normalizes elements.
     It is also better to make Set_MakeFromUnion have a parameter
     Set_t where to add new elements instead of creating new Set_t
     every time and then unite.
  */

  { /* checks if cardinality is 1 */
    Set_t set = Set_MakeFromUnion(res);
    if (Set_GiveCardinality(set) == 1) {
      res = (Expr_ptr) Set_GetMember(set, Set_GetFirstIter(set));
    }
    Set_ReleaseSet(set);
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Makes a TWODOTS node, representing an integer range]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_range(const Expr_ptr a, const Expr_ptr b)
{
  if (Nil == a) return b;
  if (Nil == b) return a;
  if (a == b) return a;

  if (NUMBER == node_get_type(a) && NUMBER == node_get_type(b) &&
      node_get_int(a) == node_get_int(b)) {
    return a;
  }

  return find_node(TWODOTS, a, b);
}


/**Function********************************************************************

  Synopsis           [Makes a setin node, with possible syntactic
                      simplification.]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_setin(const Expr_ptr a, const Expr_ptr b, SymbTable_ptr symb_table)
#else
Expr_ptr Expr_setin(const Expr_ptr a, const Expr_ptr b, const SymbTable_ptr symb_table)
#endif
{
#ifdef SYMB_TABLE_STATIC
  symb_table = expr_st;
#endif
  Expr_ptr res;
  Set_t seta = Set_MakeFromUnion(a);
  Set_t setb = Set_MakeFromUnion(b);

  /* checks if it can syntactically resolve it */
  if (Set_Contains(setb, seta)) res = Expr_true();
  else {
    if (symb_table != SYMB_TABLE(NULL)) {
      /* see if the sets are made of only constants */
      boolean a_b_const = true;
      Set_Iterator_t iter;
      SET_FOREACH(seta, iter) {
        a_b_const = SymbTable_is_symbol_constant(symb_table,
                              (node_ptr) Set_GetMember(seta, iter));
        if (!a_b_const) break;
      }
      if (a_b_const) {
        SET_FOREACH(setb, iter) {
          a_b_const = SymbTable_is_symbol_constant(symb_table,
                              (node_ptr) Set_GetMember(setb, iter));
          if (!a_b_const) break;
        }
      }

      if (a_b_const) {
        /* both sets contain only constants, so since seta is not
           contained into setb, seta is not containted in setb */
        res = Expr_false();
      }
      else res = find_node(SETIN, a, b); /* fallback */
    }
    else {
      /* symbol table is not available, so nothing can be said */
      res = find_node(SETIN, a, b);
    }
  }

  Set_ReleaseSet(setb);
  Set_ReleaseSet(seta);

  return res;
}


/**Function********************************************************************

  Synopsis           [Builds an Uninterpreted function]

  Description        [Builds an uninterpreted function named "name" with
                      "params" as parameters. "params" must be a cons
                      list of expressions (Created with find_node)]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Expr_ptr Expr_function(const Expr_ptr name, const Expr_ptr params)
{
  return EXPR(find_node(NFUNCTION, name, params));
}


/**Function********************************************************************

  Synopsis [This is the top-level function that simplifiers can use to
  simplify expressions. This evaluates constant values in operands
  left and right with respect to the operation required with parameter type.]

  Description [Given an expression node E (handled at
  simplifier-level) the simplifier call this function in post order
  after having simplified car(E) and cdr(E). It calls it by passing
  node_get_type(E) as type, and simplified sub expressions for left and right.
  The function Expr_resolve does not traverses further the structures, it simply
  combine given operation encoded in type with given already simplified
  operands left and right.

  For example, suppose E is AND(exp1, exp2). The simplifier:

  1. Simplifies recursively exp1 to exp1' and exp2 to exp2' (lazyness
  might be taken into account if exp1 is found to be a false
  constant).

  2. Calls in postorder Expr_resolve(AND, exp1', exp2')

  Expr_resolve will simplify sintactically the conjunction of (exp1', exp2')]

  SideEffects        [None]

  SeeAlso            [Expr_simplify]

******************************************************************************/
Expr_ptr Expr_resolve(SymbTable_ptr st,
                      int type,
                      Expr_ptr left,
                      Expr_ptr right)
{
  switch (type) {
    /* boolean leaves */
  case TRUEEXP: return Expr_true();
  case FALSEEXP: return Expr_false();

    /* other leaves */
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case BIT:
  case DOT:
  case ATOM:
  case ARRAY:
  case FAILURE:
    return find_node(type, left, right);

  case UWCONST:
  case SWCONST:
    return Expr_word_constant(st, type, left, right);

  case WSIZEOF:
    return Expr_wsizeof(left, right);

  case CAST_TOINT:
    return Expr_cast_toint(left, right);

  case WRESIZE: return Expr_simplify_word_resize(st, left, right);

    /* boolean */
  case AND: return Expr_and(left, right);
  case OR: return Expr_or(left, right);
  case NOT: return Expr_not(left);
  case IMPLIES: return Expr_implies(left, right);
  case IFF: return Expr_simplify_iff(st, left, right);
  case XOR: return Expr_xor(left, right);
  case XNOR: return Expr_xnor(left, right);

    /* predicates */
  case EQUAL: return Expr_equal(left, right, st);
  case NOTEQUAL: return Expr_notequal(left, right, st);
  case LT: return Expr_simplify_lt(st, left, right);
  case LE: return Expr_le(left, right, st);
  case GT: return Expr_simplify_gt(st, left, right);
  case GE: return Expr_ge(left, right, st);

    /* case */
  case IFTHENELSE:
  case CASE:
    nusmv_assert(node_get_type(left) == COLON);
    return Expr_ite(car(left), cdr(left), right, st);

  case NEXT: return Expr_next(left, st);

    /* scalar */
  case UMINUS: return Expr_unary_minus(left);
  case PLUS: return Expr_plus(left, right);
  case MINUS: return Expr_minus(left, right);
  case TIMES: return Expr_times(left, right);
  case DIVIDE: return Expr_divide(left, right);
  case MOD: return Expr_mod(left, right);

    /* word-specific */
  case CAST_WORD1: return Expr_bool_to_word1(left);
  case CAST_BOOL: return Expr_word1_to_bool(left);
  case CAST_SIGNED: return Expr_unsigned_word_to_signed(left);
  case CAST_UNSIGNED: return Expr_signed_word_to_unsigned(left);
  case EXTEND: return Expr_simplify_word_extend(st, left, right);
  case LSHIFT: return Expr_word_left_shift(left, right);
  case RSHIFT: return Expr_word_right_shift(left, right);
  case LROTATE: return Expr_word_left_rotate(left, right);
  case RROTATE: return Expr_word_right_rotate(left, right);
  case BIT_SELECTION: return Expr_simplify_word_bit_select(st, left, right);
  case CONCATENATION: return Expr_word_concatenate(left, right);

    /* wants number rsh */
  case ATTIME:
    nusmv_assert(node_get_type(right) == NUMBER);
    return Expr_attime(left, node_get_int(right), st);

    /* sets are simplified when involving constants only */
  case UNION: return Expr_union(left, right);

    /* sets are simplified when involving constants only */
  case SETIN: return Expr_setin(left, right, st);

    /* ranges are simplified when low and high coincide */
  case TWODOTS: return Expr_range(left, right);

    /* no simplification */
  case EQDEF:
  case CONS:
  case CONTEXT:
  case COLON:
  case SMALLINIT:

    /* no simplification in current implementation: */
  case EX:
  case AX:
  case EG:
  case AG:
  case EF:
  case AF:
  case OP_NEXT:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case OP_GLOBAL:
  case OP_HISTORICAL:
  case EBF:
  case ABF:
  case EBG:
  case ABG:
  case EBU:
  case ABU:
  case EU:
  case AU:
  case MINU:
  case MAXU:
  case UNTIL:
  case RELEASES:
  case SINCE:
  case TRIGGERED:
    return find_node(type, left, right);

  default:
    return find_node(type, left, right);
  }

  return EXPR(NULL);
}


/**Function********************************************************************

  Synopsis   [Top-level simplifier that evaluates constants and
  simplifies syntactically the given expression]

  Description [Top-level simplifier that evaluates constants and
  simplifies syntactically the given expression. Simplification is trivial,
  no lemma learning nor sintactic implication is carried out at the moment.

  WARNING:
  the results of simplifications are memoized in a hash stored
  in the symbol table provided. Be very careful not to free/modify the input
  expression or make sure that the input expressions are find_node-ed.
  Otherwise, it is very easy to introduce a bug which will be
  difficult to catch.
  The hash in the symbol table is reset when any layer is removed.

  NOTE FOR DEVELOPERS: if you think that memoization the simplification
  results may cause some bugs you always can try without global
  memoization. See the function body below for info.

  ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
Expr_ptr Expr_simplify(SymbTable_ptr st, Expr_ptr expr)
{
#ifdef SYMB_TABLE_STATIC
  SymbTable_ptr old_st = expr_st;
#endif

  Expr_ptr res;
  hash_ptr hash_memoize;

  /* NOTE FOR DEVELOPERS. In order to disable global memoization of
     simplification results make the below macro be equal to 0 instead of 1.
     This is purely debugging feature
  */
#define _ENABLE_GLOBAL_SIMPLIFICATION_MEMOIZATION_ 0

#if _ENABLE_GLOBAL_SIMPLIFICATION_MEMOIZATION_
  hash_memoize = SymbTable_get_simplification_hash(st);
#else
  hash_memoize = new_assoc();
#endif

  CATCH {

#ifdef SYMB_TABLE_STATIC
    expr_st = st;
#endif

    res = expr_simplify_aux(st, expr, hash_memoize);
#ifdef SYMB_TABLE_STATIC
    expr_st = old_st;
#endif
  }
  FAIL {
#ifdef SYMB_TABLE_STATIC
    expr_st = old_st;
#endif
    rpterr("An error occurred during Expr_simplify");
  }

#if ! _ENABLE_GLOBAL_SIMPLIFICATION_MEMOIZATION_
  free_assoc(hash_memoize);
#endif

  return res;
}


/**Function********************************************************************

   Synopsis           [Determines whether a formula has ATTIME nodes in it]

   Description        [Determines whether a formula has ATTIME nodes in it
                       If cache is not null whenever we encounter a formula in
                       the cache we simply return the previously computed value,
                       otherwise an internal and temporary map is used.

                       NOTE: the internal representation of cache is private so
                             the user should provide only caches generated by
                             this function!]

   SideEffects        [cache can be updated]

******************************************************************************/
boolean Expr_is_timed(Expr_ptr expr, hash_ptr cache)
{
  boolean res;
  if((hash_ptr)NULL == cache) {
    cache = new_assoc();
    res = expr_is_timed_aux(expr, cache);
    free_assoc(cache);
  }
  else {
    res = expr_is_timed_aux(expr, cache);
  }

  return res;
}


/**Function********************************************************************

   Synopsis           [Obtain the base time of an expression]

   Description        [Current time is recursively calculated as follows:

                       1. UNTIMED_CURRENT for Nil and leaves;
                       2. UNTIMED_FROZEN if all vars are frozen;
                       3. Time specified for an ATTIME node, assuming
                       that the inner expression is untimed.

                       Nesting of ATTIME nodes is _not_ allowed;
                       4. Minimum time for left and right children
                       assuming

                       UNTIMED_CURRENT <
                       UNTIMED_NEXT <
                       t, for any t >= 0.]

   SideEffects        [None]

******************************************************************************/
int Expr_get_time(SymbTable_ptr st, Expr_ptr expr)
{
  hash_ptr h;
  int res;

  h = new_assoc();
  res = expr_get_curr_time(st, expr, h);
  free_assoc(h);

  return res;
}

/**Function********************************************************************

   Synopsis           [Returns true if the time (obtained by Expr_get_time) is
                       dont't care]

   Description        []

   SideEffects        [Expr_get_time]

******************************************************************************/
boolean Expr_time_is_dont_care(int time)
{
  return time == UNTIMED_DONTCARE;
}


/**Function********************************************************************

   Synopsis           [Returns true if the time (obtained by Expr_get_time) is
                       current]

   Description        []

   SideEffects        [Expr_get_time]

******************************************************************************/
boolean Expr_time_is_current(int time)
{
  return time == UNTIMED_CURRENT;
}


/**Function********************************************************************

   Synopsis           [Returns true if the time (obtained by Expr_get_time) is
                       next]

   Description        []

   SideEffects        [Expr_get_time]

******************************************************************************/
boolean Expr_time_is_next(int time)
{
  return time == UNTIMED_NEXT;
}


/**Function********************************************************************

   Synopsis           [Returns the untimed version of an expression]

   Description        []

   SideEffects        [Expr_get_time]

******************************************************************************/
Expr_ptr Expr_untimed(SymbTable_ptr st, Expr_ptr expr)
{
  int time;

  time = Expr_get_time(st, expr);
  return Expr_untimed_explicit_time(st, expr, time);
}


/**Function********************************************************************

   Synopsis           [Returns the untimed version of an expression without
                       searching for the current time]

   Description        [Returns the untimed version of an expression using the
                       current time provided as an argument.]

   SideEffects        [Expr_get_time]

******************************************************************************/
Expr_ptr Expr_untimed_explicit_time(SymbTable_ptr st, Expr_ptr expr,
                                    int curr_time)
{
  hash_ptr h;
  Expr_ptr res;

  h = new_assoc();
  res = expr_timed_to_untimed(st, expr, curr_time, false, h);
  free_assoc(h);

  return res;
}

/**Function********************************************************************

  Synopsis    [Builds the node for UWCONST or SWCONST]

  Description [Works with words and scalars. Performs local syntactic
               simplification.]

  SideEffects [None]

  SeeAlso     [Expr_resolve]

******************************************************************************/
#ifdef SYMB_TABLE_STATIC
Expr_ptr Expr_word_constant(SymbTable_ptr symb_table,
                                     int type,
                                     Expr_ptr l,
                                     Expr_ptr r)
#else
Expr_ptr Expr_word_constant(const SymbTable_ptr symb_table,
                                     int type,
                                     Expr_ptr l,
                                     Expr_ptr r)
#endif
{
#ifdef SYMB_TABLE_STATIC
  symb_table = expr_st;
#endif
  node_ptr value;
  node_ptr size;

  int size_int;
  int value_int;

  int size_type;
  int value_type;

  WordNumber_ptr value_word;
  WordNumberValue tmp;

  nusmv_assert((type == UWCONST || type == SWCONST));

  if (SYMB_TABLE(NULL) != symb_table) {
  value = CompileFlatten_resolve_number(symb_table, NODE_PTR(l), Nil);
  size = CompileFlatten_resolve_number(symb_table, NODE_PTR(r), Nil);
  }

  if ((NUMBER == size_type ||
       NUMBER_UNSIGNED_WORD == size_type ||
       NUMBER_SIGNED_WORD == size_type) &&
      (NUMBER == value_type ||
       NUMBER_UNSIGNED_WORD == value_type ||
       NUMBER_SIGNED_WORD == value_type))
    {
    /*  process the size: it can be an integer or word number in range */
    /*  [0, max-allowed-size] */
    switch (node_get_type(size)) {
    case NUMBER:
      size_int = node_get_int(size);
      break;

    case NUMBER_UNSIGNED_WORD:
      tmp = WordNumber_get_unsigned_value(WORD_NUMBER(car(size)));
      size_int = tmp;
      if (tmp != size_int) {
        rpterr("size specifier of swconst/uwconst operator is "
               "not representable as int");
      }
      break;

    case NUMBER_SIGNED_WORD:
      tmp = WordNumber_get_signed_value(WORD_NUMBER(car(size)));
      size_int = tmp;
      if (tmp != size_int) {
        rpterr("size specifier of swconst/uwconst operator is "
               "not representable as int");
      }
      break;

    default: error_unreachable_code();
    }

    if (size_int <= 0 || size_int > WordNumber_max_width()) {
      rpterr("size specifier is out of range [0, %i]",
             WordNumber_max_width());
    }

    /*  process the value: it can be only integer and has to be */
    /*  representable with given size. */
    if (NUMBER != node_get_type(value)) {
      rpterr("value specifier of swconst/uwconst operator is not "
             "an integer constant");
    }

    value_int = node_get_int(value);

    /* two shifts are done because shift by the full width isn't allowed in
       C. If value is positive, an extra bit of width is needed to avoid
       overflow. */
    if ((value_int > 0 &&
         ((UWCONST == type && value_int >> (size_int-1) >> 1 != 0) ||
          (SWCONST == type && value_int >> (size_int-2) >> 1 != 0))) ||
        (value_int < 0 && value_int >> (size_int-1) != -1)) {
      rpterr("value specifier of swconst/uwconst operator is not "
             "representable with provided width");
    }

    if (value_int >= 0) {
      value_word = WordNumber_from_integer(value_int, size_int);
    }
    else {
      value_word = WordNumber_from_signed_integer(value_int, size_int);
    }

    nusmv_assert(WORD_NUMBER(NULL) != value_word);

    if (UWCONST == type) {
      return find_node(NUMBER_UNSIGNED_WORD, NODE_PTR(value_word), Nil);
    }
    else return find_node(NUMBER_SIGNED_WORD, NODE_PTR(value_word), Nil);
  }

  /* no simplification possible */
  return find_node(type, NODE_PTR(l), NODE_PTR(r));
}

/**Function********************************************************************

  Synopsis    [Builds the node for WSIZEOF]

  Description [Works with words. Performs local syntactic simplification.]

  SideEffects [None]

  SeeAlso     [Expr_resolve]

******************************************************************************/
Expr_ptr Expr_wsizeof(Expr_ptr l, Expr_ptr r)
{
  int width;
  int type;

  nusmv_assert(EXPR(NULL) == r);

  type = node_get_type(NODE_PTR(l));

  if (NUMBER_SIGNED_WORD == type || NUMBER_UNSIGNED_WORD == type) {
    width = WordNumber_get_width(WORD_NUMBER(car(NODE_PTR(l))));

    nusmv_assert(0 < width);

    return find_node(NUMBER, NODE_FROM_INT(width), Nil);
  }
  return find_node(WSIZEOF, NODE_PTR(l), Nil);
}

/**Function********************************************************************

  Synopsis    [Builds the node for CAST_TOINT]

  Description [Works with scalars. Performs local syntactic simplification.]

  SideEffects []

  SeeAlso     [Expr_resolve]

******************************************************************************/
Expr_ptr Expr_cast_toint(Expr_ptr l, Expr_ptr r)
{
  int type;

  nusmv_assert(EXPR(NULL) == r);

  type = node_get_type(NODE_PTR(l));

  if (NUMBER == type || INTEGER == type) return l;
  else return find_node(CAST_TOINT, NODE_PTR(l), Nil);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [Converts a timed node into an untimed node]

   Description        [Converts a timed node into an untimed node]

   SideEffects        [None]

******************************************************************************/
static Expr_ptr
expr_timed_to_untimed(SymbTable_ptr st, Expr_ptr expr, int curr_time,
                      boolean in_next, hash_ptr cache)
{
  if (expr == Nil) return Nil;
  const node_ptr key = in_next ? find_node(NEXT, expr, Nil) : expr;
  node_ptr res = find_assoc(cache, key);
  if (Nil != res) return res;

  switch (node_get_type(expr)) {
    /* leaves */
  case FAILURE:
  case ARRAY:
  case BIT:
  case DOT:
  case ATOM:
  case NUMBER_SIGNED_WORD:
  case NUMBER_UNSIGNED_WORD:
  case UWCONST:
  case SWCONST:
  case WORDARRAY:
  case NUMBER:
  case NUMBER_REAL:
  case NUMBER_FRAC:
  case NUMBER_EXP:
  case TRUEEXP:
  case FALSEEXP:
    res  = expr;
    break;

  case ATTIME:
    {
      /* a frozen var must be time compatible with any time */
      int time2 = SymbTable_is_symbol_frozen_var(st, car(expr))
        ? curr_time : node_get_int(cdr(expr));

      if (time2 == UNTIMED_CURRENT || time2 == curr_time) {
        res = expr_timed_to_untimed(st, car(expr), curr_time,
                                    in_next, cache);
      }
      else if (time2 == UNTIMED_NEXT || time2 == curr_time+1) {
        if (in_next) {
          internal_error("%s:%d:%s: Invalid nested NEXT (%s)",
                         __FILE__, __LINE__, __func__,
                         sprint_node(expr));
        }
        res = find_node(NEXT,
                        expr_timed_to_untimed(st, car(expr),
                                              curr_time,
                                              true, cache),
                        Nil);
      }
      else {
        internal_error("%s:%d:%s: Invalid ATTIME node (%s)",
                       __FILE__, __LINE__, __func__, sprint_node(expr));
      }

      break;
    }

  case NEXT:
    if (in_next) {
      internal_error("%s:%d:%s: Invalid nested NEXT (%s)",
                     __FILE__, __LINE__, __func__,
                     sprint_node(expr));
    }
    res = find_node(NEXT,
                    expr_timed_to_untimed(st, car(expr),
                                          curr_time,
                                          true, cache),
                    Nil);

    break;

  default:
    {
      node_ptr lt = expr_timed_to_untimed(st, car(expr),
                                          curr_time,
                                          in_next, cache);

      node_ptr rt = expr_timed_to_untimed(st, cdr(expr),
                                          curr_time,
                                          in_next, cache);

      res = find_node(node_get_type(expr), lt, rt);

      break;
    }

  } /* switch */

  insert_assoc(cache, key, res);
  nusmv_assert(Nil != res);
  return res;
}


/**Function********************************************************************

   Synopsis           [Calculates current time for an expression]

   Description        [Private service of Expr_get_time]

   SideEffects        [None]

******************************************************************************/
static int expr_get_curr_time(SymbTable_ptr st, node_ptr expr, hash_ptr cache)
{
  node_ptr tmp = find_assoc(cache, expr);
  if (Nil != tmp) return NODE_TO_INT(tmp) - _TIME_OFS;

  int res = 0;

  if (expr == Nil) {
    return UNTIMED_DONTCARE;
  }

  switch (node_get_type(expr)) {

    /* leaves */
  case DOT:
  case ATOM:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(st, expr, Nil);
      name = ResolveSymbol_get_resolved_name(rs);

      /* handle frozenvars as a special case with lookahead */
      if (SymbTable_is_symbol_frozen_var(st, name)) {
        return UNTIMED_DONTCARE;
      }
    }

  case FAILURE:
  case ARRAY:
  case BIT:
  case NUMBER_SIGNED_WORD:
  case NUMBER_UNSIGNED_WORD:
  case UWCONST:
  case SWCONST:
  case WORDARRAY:
  case NUMBER:
  case NUMBER_REAL:
  case NUMBER_FRAC:
  case NUMBER_EXP:
  case TRUEEXP:
  case FALSEEXP:
      return UNTIMED_CURRENT;

  case ATTIME: {
    int time1 = node_get_int(cdr(expr));
    int time2 = expr_get_curr_time(st, car(expr), cache);

    if (time2 == UNTIMED_DONTCARE) {
      res = UNTIMED_DONTCARE;
    }
    else if (time2 == UNTIMED_CURRENT) {
      res = time1; /* time1 is absolute */
    }
    else if (time2 == UNTIMED_NEXT) {
      internal_error("%s:%d:%s: Unexpected NEXT",
                     __FILE__, __LINE__, __func__);
    }
    else { /* time2 is absolute and this is wrong */
      nusmv_assert(0 <= time2);
      internal_error("%s:%d:%s: Invalid nested ATTIME",
                     __FILE__, __LINE__, __func__);
    }

    break;
  }

  default:
    {
      int time1 = expr_get_curr_time(st, car(expr), cache);
      int time2 = expr_get_curr_time(st, cdr(expr), cache);

      /* both are DON'T CARE? */
      if ((UNTIMED_DONTCARE == time1) && (UNTIMED_DONTCARE == time2)) {
        res = UNTIMED_DONTCARE;
      }

      /* one (but not both) is DON'T CARE? */
      else if (UNTIMED_DONTCARE == time1) {
        res = time2;
      }

      else if (UNTIMED_DONTCARE == time2) {
        res = time1;
      }

      /* both are CURRENT? */
      else if ((UNTIMED_CURRENT == time1) &&
          (UNTIMED_CURRENT == time2)) {
        res = UNTIMED_CURRENT;
      }

      /* one is CURRENT, the other is not */
      else if (UNTIMED_CURRENT == time1) {
        res = time2;
      }

      else if (UNTIMED_CURRENT == time2) {
        res = time1;
      }

      else {
        /* times can only be absolute beyond this point */
        nusmv_assert ((0 <= time1) && (0 <= time2));
        res = MIN(time1, time2);
      }

      break;
    }

  } /* switch */

  /* Cache */
  insert_assoc(cache, expr, NODE_FROM_INT(res + _TIME_OFS));
  return res;
}

/**Function********************************************************************

   Synopsis           [Recursive auxiliary function for Expr_simplify]

   Description        [Recursive auxiliary function for Expr_simplify]

   SideEffects        []

   SeeAlso            [Expr_simplify]

******************************************************************************/
static Expr_ptr expr_simplify_aux(SymbTable_ptr st, Expr_ptr expr,
                                  hash_ptr hash)
{
  node_ptr res = (node_ptr) NULL;
  int type;

  if (expr == Nil) return Nil;

  /* check memoization */
  res = find_assoc(hash, expr);
  if (res != (node_ptr) NULL) {
    return res;
  }

  type = node_get_type(expr);
  switch (type) {
    /* boolean leaves */
  case TRUEEXP: return Expr_true();
  case FALSEEXP: return Expr_false();

    /* name leaves */
  case ATOM:
  case BIT:
    return find_node(type, car(expr), cdr(expr));

    /* other leaves */
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case FAILURE:
    return find_node(type, car(expr), cdr(expr));

  case UWCONST:
  case SWCONST:
  case WRESIZE:
    {
      Expr_ptr left = expr_simplify_aux(st, car(expr), hash);
      Expr_ptr right = expr_simplify_aux(st, cdr(expr), hash);
      res = Expr_resolve(st, type, left, right);
      break;
    }

  case DOT:
  case ARRAY:
    return find_node(type,
                     expr_simplify_aux(st, car(expr), hash),
                     expr_simplify_aux(st, cdr(expr), hash));

    /* unary */
  case NOT:
  case NEXT:
  case UMINUS:
  case WSIZEOF:
  case CAST_TOINT:
  {
    Expr_ptr left = expr_simplify_aux(st, car(expr), hash);
    res = Expr_resolve(st, type, left, Nil);
    break;
  }

    /* binary with lazy eval */
  case AND:
  {
    Expr_ptr left = expr_simplify_aux(st, car(expr), hash);
    if (Expr_is_false(left)) res = left;
    else res = Expr_resolve(st, type, left, expr_simplify_aux(st, cdr(expr), hash));
    break;
  }

  case OR:
  {
    Expr_ptr left = expr_simplify_aux(st, car(expr), hash);
    if (Expr_is_true(left)) res = left;
    else res = Expr_resolve(st, type, left, expr_simplify_aux(st, cdr(expr), hash));
    break;
  }

  case IMPLIES:
  {
    Expr_ptr left = expr_simplify_aux(st, car(expr), hash);
    if (Expr_is_false(left)) res = Expr_true();
    else res = Expr_resolve(st, type, left, expr_simplify_aux(st, cdr(expr), hash));
    break;
  }

  /* binary, no lazyness */
  case IFF:
  case XOR:
  case XNOR:
  case EQUAL:
  case NOTEQUAL:
  case LT:
  case LE:
  case GT:
  case GE:
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
  case CAST_WORD1:
  case CAST_BOOL:
  case CAST_SIGNED:
  case CAST_UNSIGNED:
  case EXTEND:
  case LSHIFT:
  case RSHIFT:
  case LROTATE:
  case RROTATE:
  case BIT_SELECTION:
  case CONCATENATION:
    res = Expr_resolve(st, type,
                       expr_simplify_aux(st, car(expr), hash),
                       expr_simplify_aux(st, cdr(expr), hash));
    break;

    /* case with lazyness on condition */
  case IFTHENELSE:
  case CASE:
  {
    Expr_ptr cond = expr_simplify_aux(st, car(car(expr)), hash);
    Expr_ptr _then, _else;

    if (Expr_is_true(cond)) {
      _then = expr_simplify_aux(st, cdr(car(expr)), hash);
      _else = cdr(expr);
    }
    else if (Expr_is_false(cond)) {
      _then = cdr(car(expr));
      _else = expr_simplify_aux(st, cdr(expr), hash);
    }
    else {
      _then = expr_simplify_aux(st, cdr(car(expr)), hash);
      _else = expr_simplify_aux(st, cdr(expr), hash);
    }

    res = Expr_resolve(st, type, find_node(COLON, cond, _then), _else);
    break;
  }

  /* sets are simplified when possible */
  case SETIN:
  case UNION:
    res = Expr_resolve(st, type,
                       expr_simplify_aux(st, car(expr), hash),
                       expr_simplify_aux(st, cdr(expr), hash));
    break;

    /* ranges are simplified */
  case TWODOTS:
    res = Expr_resolve(st, type,
                       expr_simplify_aux(st, car(expr), hash),
                       expr_simplify_aux(st, cdr(expr), hash));
    break;

    /* no simplification */
  case EQDEF:
  case CONS:
  case CONTEXT:

    /* no simplification in current implementation: */
  case EX:
  case AX:
  case EG:
  case AG:
  case EF:
  case AF:
  case OP_NEXT:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case OP_GLOBAL:
  case OP_HISTORICAL:
  case EU:
  case AU:
  case MINU:
  case MAXU:
  case UNTIL:
  case RELEASES:
  case SINCE:
  case TRIGGERED:
    res = find_node(type,
                    expr_simplify_aux(st, car(expr), hash),
                    expr_simplify_aux(st, cdr(expr), hash));
    break;

    /* These nodes need special treatment when used with
       Expr_resolve, since recursively enter into their cdr may
       break the formula. (Ranges with min = max are resolved as
       number by Expr_resolve). See issue 2194. */
  case EBF:
  case ABF:
  case EBG:
  case ABG:
  case EBU:
  case ABU:
    nusmv_assert(Nil == cdr(expr) || TWODOTS == node_get_type(cdr(expr)));

    res = Expr_resolve(st, node_get_type(expr),
                       expr_simplify_aux(st, car(expr), hash),
                       cdr(expr));
    break;

  default:
    res = find_node(type,
                    expr_simplify_aux(st, car(expr), hash),
                    expr_simplify_aux(st, cdr(expr), hash));
    break;
  }

  /* memoize */
  insert_assoc(hash, expr, res);
  return EXPR(res);
}

/**Function********************************************************************

   Synopsis           [casts boolean constants to WORD[1]]

   Description        []

   SideEffects        []

******************************************************************************/
static Expr_ptr expr_bool_to_word1(const Expr_ptr a)
{
  if (Expr_is_true(a)) {
    return find_node(NUMBER_UNSIGNED_WORD,
                     (node_ptr) WordNumber_from_integer(1,1), Nil);
  }

  if (Expr_is_false(a)) {
    return find_node(NUMBER_UNSIGNED_WORD,
                     (node_ptr) WordNumber_from_integer(0,1), Nil);
  }

  return a;
}


/**Function********************************************************************

   Synopsis           [true if expression is timed]

   Description        [Private service of Expr_is_timed.
                       To represent 'true' in cache we use the constant 2 for
                       'false' we use 1 to avoid representation problems wrt Nil]

   SideEffects        [cache can be updated]

******************************************************************************/
static boolean expr_is_timed_aux(Expr_ptr expr, hash_ptr cache)
{
  Expr_ptr tmp;
  boolean result;

  nusmv_assert((hash_ptr)NULL != cache);

  if (expr == Nil) return false;

  tmp = find_assoc(cache, expr);
  if(Nil != tmp) {
    return (NODE_TO_INT(tmp) == 2);
  }

  switch (node_get_type(expr)) {
    /* leaves */
  case FAILURE:
  case ARRAY:
  case BIT:
  case DOT:
  case ATOM:
  case NUMBER_SIGNED_WORD:
  case NUMBER_UNSIGNED_WORD:
  case UWCONST:
  case SWCONST:
  case WORDARRAY:
  case NUMBER:
  case NUMBER_REAL:
  case NUMBER_FRAC:
  case NUMBER_EXP:
  case TRUEEXP:
  case FALSEEXP:
    return false;

  case ATTIME:
    result = true;
    break;
  case NEXT:
    result = false;
    break;

  default:
    {
      boolean ll;

      ll = expr_is_timed_aux(car(expr), cache);
      if(ll) {
        result = true;
      }
      else {
        result = expr_is_timed_aux(cdr(expr), cache);
      }
    }
  } /* switch */

  if(result) {
    insert_assoc(cache, expr, NODE_FROM_INT(2));
  }
  else {
    insert_assoc(cache, expr, NODE_FROM_INT(1));
  }

  return result;
}

/**Function********************************************************************

   Synopsis           [Check for an expr being boolean]

   Description        [Check for an expr being boolean]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean expr_is_bool(const Expr_ptr a)
{
  return (Expr_is_true(a) || Expr_is_false(a));
}

