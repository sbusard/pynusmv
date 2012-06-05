/**CFile***********************************************************************

  FileName    [operators.c]

  PackageName [enc]

  Synopsis    [These operators are used by dd package]

  Description [Functions like add_plus, add_equal, etc., call these operators]

  SeeAlso     [operators.h]

  Author      [Marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``enc'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.

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

#include "operators.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "utils/ustring.h"
#include "opt/opt.h"
#include "utils/WordNumber.h"

static char rcsid[] UTIL_UNUSED = "$Id: operators.c,v 1.1.2.5.4.17.4.20 2010-02-08 12:25:27 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef int (*INTPFII)(int, int);


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN int yylineno;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static node_ptr
node_word_full_adder ARGS((node_ptr ai, node_ptr bi, node_ptr carry_in,
                           node_ptr* carry_out));

static node_ptr node_word_unsigned_divide_reminder ARGS((node_ptr a, node_ptr b,
                                                node_ptr* reminder));
static node_ptr
node_word_signed_divide_reminder_simple ARGS((node_ptr a, node_ptr b,
                                              node_ptr* reminder));
static node_ptr
node_word_signed_divide_reminder_hardware ARGS((node_ptr a, node_ptr b,
                                                node_ptr* reminder));

static node_ptr node_word_signed_op ARGS((node_ptr a, node_ptr b, NPFNN op));

static boolean _is_bool ARGS((const node_ptr a));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [ performs logical AND on two nodes.]

  Description        [ Nodes can be integers with values 0 and 1 (logical AND).
  All other combinations are illegal.]

  SideEffects        []

******************************************************************************/
node_ptr node_and(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (_is_bool(n1) && _is_bool(n2)) {
    return Expr_and(n1, n2);
  }
  else error_not_proper_numbers("&", n1, n2);

  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [ performs logical OR on two nodes.]

  Description        [ Nodes can be integers with values 0 and 1 (logical OR).
  All other combinations are illegal.]

  SideEffects        []

******************************************************************************/
node_ptr node_or(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (_is_bool(n1) && _is_bool(n2)) {
    return Expr_or(n1, n2);
  }
  else error_not_proper_numbers("|", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [ performs logical NOT on a node.]

  Description        [ Node can be an integer with values 0 or 1 (logical NOR).
  All other combinations are illegal.

  NOTE: At the momement, CUDD does not have unary 'apply', so
  you have to write a unary operator in the form of a binary one which
  actually applies to the first operand only]

  SideEffects        []

******************************************************************************/
node_ptr node_not(node_ptr n, node_ptr this_node_is_not_used)
{
  if (node_get_type(n) == FAILURE) return n; /* error in previous expr */

  if (_is_bool(n)) {
    return Expr_not(n);
  }
  else error_not_proper_number("!", n);
  return (node_ptr) NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [ performs logical IFF on two nodes.]

  Description        [ Nodes can be integers with values 0 and 1 (logical IFF).
  All other combinations are illegal.]

  SideEffects        []

******************************************************************************/
node_ptr node_iff(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (_is_bool(n1) && _is_bool(n2)) {
    return Expr_iff(n1, n2);
  }
  else error_not_proper_numbers("<->", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [ performs logical XOR on two nodes.]

  Description        [ Nodes can be integers with values 0 and 1 (logical XOR).
  All other combinations are illegal.]

  SideEffects        []

******************************************************************************/
node_ptr node_xor(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (_is_bool(n1) && _is_bool(n2)) {
    return Expr_xor(n1, n2);
  }
  else error_not_proper_numbers("xor", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis        [ performs logical IMPLIES on two nodes.]

  Description     [Nodes can be integers with values 0 and 1 (logical IMPLIES).
  All other combinations are illegal.]

  SideEffects     []

******************************************************************************/
node_ptr node_implies(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (_is_bool(n1) && _is_bool(n2)) {
    return Expr_implies(n1, n2);
  }
  else error_not_proper_numbers("implies", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [returns NUMBER with value 1 (symbol Expr_true()) if
  the nodes are the same, and value 0 (symbol Expr_false()) otherwise]

  Description        [
  In NuSMV an constant is equal to another constant then this
  constants are actually the same and representable by the same node.
  ]

  SideEffects        []

  SeeAlso            [node_setin]

******************************************************************************/
node_ptr node_equal(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if(n1 == n2) return(Expr_true());

  return(Expr_false());
}

/**Function********************************************************************

  Synopsis           [returns NUMBER with value 1 (symbol Expr_true()) if
  the nodes are of different values, and value 0 (symbol Expr_false()) otherwise]

  Description        [
  In NuSMV an constant is equal to another constant then this
  constants are actually the same and representable by the same node.
  ]

  SideEffects        []

  SeeAlso            [node_setin]

******************************************************************************/
node_ptr node_not_equal(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if(n1 == n2) return(Expr_false());

  return(Expr_true());
}

/**Function********************************************************************

  Synopsis           [returns NUMBER with value 1 if
  the first node is less than the second one, and 0 - otherwise.]

  Description        [Nodes should be both NUMBER]

  SideEffects        []

******************************************************************************/
node_ptr node_lt(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    return (NODE_TO_INT(car(n1)) < NODE_TO_INT(car(n2))) ?
      Expr_true() : Expr_false();
  }
  else error_not_proper_numbers("<", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [returns NUMBER with value 1 if
  the first node is greater than the second one, and 0 - otherwise.]

  Description        [Nodes should be both NUMBER]

  SideEffects        []

******************************************************************************/
node_ptr node_gt(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    return NODE_TO_INT(car(n1)) > NODE_TO_INT(car(n2)) ?
      Expr_true() : Expr_false();
  }
  else error_not_proper_numbers(">", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [returns NUMBER with value 1 if
  the first node is less or equal than the second one, and 0 - otherwise.]

  Description        [Nodes should be both NUMBER]

  SideEffects        []

******************************************************************************/
node_ptr node_le(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    return (NODE_TO_INT(car(n1)) <= NODE_TO_INT(car(n2))) ?
      Expr_true() : Expr_false();
  }
  else error_not_proper_numbers("<=", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [returns NUMBER with value 1 if
  the first node is greater or equal than the second one, and 0 - otherwise.]

  Description        [Nodes should be both NUMBER]

  SideEffects        []

******************************************************************************/
node_ptr node_ge(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    return NODE_TO_INT(car(n1)) >= NODE_TO_INT(car(n2)) ?
      Expr_true() : Expr_false();
  }
  else error_not_proper_numbers(">=", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [Negates the operand (unary minus)]

  Description        [Left node can be NUMBER, and the right one is Nil.]

  SideEffects        []

******************************************************************************/
node_ptr node_unary_minus(node_ptr n, node_ptr this_node_is_not_used)
{
  if (node_get_type(n) == FAILURE) return n; /* error in previous expr */

  if (node_get_type(n) == NUMBER) {
    return find_node(NUMBER, NODE_FROM_INT(-NODE_TO_INT(car(n))), Nil);
  }
  else error_not_proper_number("- (unary)", n);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [Adds two nodes]

  Description        [Nodes can be both NUMBER.]

  SideEffects        []

******************************************************************************/
node_ptr node_plus(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    return find_node(NUMBER,
             NODE_FROM_INT(NODE_TO_INT(car(n1)) + NODE_TO_INT(car(n2))), Nil);
  }
  else error_not_proper_numbers("+", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [Subtract two nodes]

  Description        [Nodes can be both NUMBER.]

  SideEffects        []

******************************************************************************/
node_ptr node_minus(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    return  find_node(NUMBER,
              NODE_FROM_INT(NODE_TO_INT(car(n1)) - NODE_TO_INT(car(n2))), Nil);
  }
  else error_not_proper_numbers("-", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [Multiplies two nodes]

  Description        [Nodes can be both NUMBER.]

  SideEffects        []

******************************************************************************/
node_ptr node_times(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    return find_node(NUMBER,
             NODE_FROM_INT(NODE_TO_INT(car(n1)) * NODE_TO_INT(car(n2))), Nil);
  }
  else error_not_proper_numbers("*", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [Divides two nodes]

  Description        [Nodes can be both NUMBER.]

  SideEffects        []

******************************************************************************/
node_ptr node_divide(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  /* check the second operand for being zero */
  if (node_get_type(n2) == NUMBER && 0 == NODE_TO_INT(car(n2))) {
    return failure_make("Division by zero", FAILURE_DIV_BY_ZERO, yylineno);
  }

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    /* here is the check whether usual ANSI semantics of division is used */
    if (opt_use_ansi_c_div_op(OptsHandler_get_instance())) {
      return find_node(NUMBER,
              NODE_FROM_INT(NODE_TO_INT(car(n1)) / NODE_TO_INT(car(n2))), Nil);
    }
    /* the semantics of the division is from old version of NuSMV */
    else {
      int a = NODE_TO_INT(car(n1));
      int b = NODE_TO_INT(car(n2));
      int r = a % b;
      int result = a/b - (r < 0);/*IF r < 0 THEN a/b - 1 ELSE a/b */
      return find_node(NUMBER, NODE_FROM_INT(result), Nil);
    }
  }
  else error_not_proper_numbers("/", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [Computes the remainder of division of two nodes]

  Description        [Nodes can be both NUMBER.]

  SideEffects        []

******************************************************************************/
node_ptr node_mod(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  /* check the second operand for being zero */
  if (node_get_type(n2) == NUMBER && 0 == NODE_TO_INT(car(n2))) {
    return failure_make("Division by zero", FAILURE_DIV_BY_ZERO, yylineno);
  }

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    /* here is the check whether usual ANSI semantics of division is used */
    if (opt_use_ansi_c_div_op(OptsHandler_get_instance())) {
      return find_node(NUMBER,
         NODE_FROM_INT(NODE_TO_INT(car(n1)) % NODE_TO_INT(car(n2))), Nil);
    }
    /* the semantics of the division is from old version of NuSMV */
    else {
      int a = NODE_TO_INT(car(n1));
      int b = NODE_TO_INT(car(n2));
      int r = a % b;
      if (r < 0) r += b;
      return find_node(NUMBER, NODE_FROM_INT(r), Nil);
    }
  }
  else error_not_proper_numbers("mod", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}

/**Function********************************************************************

  Synopsis           [creates RANGE node from two NUMBER nodes.]

  Description        [this range is used in bit-selection only]

  SideEffects        [node_bit_selection]

******************************************************************************/
node_ptr node_bit_range(node_ptr n1, node_ptr n2)
{
  if (node_get_type(n1) == FAILURE) return n1; /* error in previous expr */
  if (node_get_type(n2) == FAILURE) return n2; /* error in previous expr */

  if (node_get_type(n1) == NUMBER && node_get_type(n2) == NUMBER) {
    return find_node(RANGE, n1, n2);
  }
  else error_not_proper_numbers("bit-selection-range", n1, n2);
  return (node_ptr)NULL;/* return something to suppress warnings */
}


/**Function********************************************************************

  Synopsis           [Computes the set union of two s_expr.]

  Description        [This function computes the sexp resulting from
  the union of s_expr "n1" and "n2".
  NB: if any of the operands is a FAILURE node, the FAILURE node is returned.]

  SideEffects        []

******************************************************************************/
node_ptr node_union(node_ptr n1, node_ptr n2)
{
  node_ptr tmp;

  if(n1 == Nil) return(n2);
  if(n2 == Nil) return(n1);

  if (node_get_type(n1) == FAILURE) return n1; /* error in operand */
  if (node_get_type(n2) == FAILURE) return n2; /* error in operand */

  /* convert singleton elements to lists */
  if(node_get_type(n1) != CONS) n1 = find_node(CONS, n1, Nil);
  if(node_get_type(n2) != CONS) n2 = find_node(CONS, n2, Nil);

  /* create a list merged from the given lists and
     with all elements ordered (less-comparison)
  */
  tmp = Nil;
  while (n1 != Nil && n2 != Nil) {
    if (car(n1) == car(n2)) {
      tmp = cons(car(n1), tmp);
      n1 = cdr(n1);
      n2 = cdr(n2);
    }
    else if (car(n1) < car(n2)) {/* < is used because the list will be reversed */
      tmp = cons(car(n1), tmp);
      n1 = cdr(n1);
    }
    else { /*car(n2) > car(n1) */
      tmp = cons(car(n2), tmp);
      n2 = cdr(n2);
    }
  }
  if (Nil == n1) n1 = n2; /* remaining elements (they were in n1 or n2) */

  /* reverse the obtained list and apply find_node. The result will be in n1 */
  while (Nil != tmp) {
    n1 = find_node(CONS, car(tmp), n1);

    n2 =  cdr(tmp);
    free_node(tmp);
    tmp = n2;
  }
  return n1;
}

/**Function********************************************************************

  Synopsis           [Set inclusion]

  Description        [Checks if s_expr "n1" is a subset of s_expr
  "n2", if it is the case them <code>Expr_true()</code> is returned,
  else <code>Expr_false()</code> is returned.

  If "n1" is a list of values then <code>Expr_true()</code> is returned only
  if all elements of "n1" is a subset of "n2".

  NB: if any of the operands is a FAILURE node, the FAILURE node is returned.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr node_setin(node_ptr n1, node_ptr n2)
{
  node_ptr iter1;
  node_ptr iter2;

  if (node_get_type(n1) == FAILURE) return n1; /* error in operand */
  if (node_get_type(n2) == FAILURE) return n2; /* error in operand */

  /* convert singleton elements to lists */
  if (CONS != node_get_type(n1)) n1 = find_node(CONS, n1, Nil);
  if (CONS != node_get_type(n2)) n2 = find_node(CONS, n2, Nil);

  /* check that every element of n1 is equal to some element of n2 */
  for (iter1 = n1; iter1 != Nil; iter1 = cdr(iter1)) {
    for (iter2 = n2; iter2 != Nil; iter2 = cdr(iter2)) {
      if (car(iter1) == car(iter2)) break; /* there is equality */
    }
    /* one of the elements of n1 is not equal to any elements of n2 */
    if (iter2 == Nil) return Expr_false();
  }
  return Expr_true();
}


/* ---------------------------------------------------------------------- */
/*   WORDS releated (encoded as node_ptr)                                 */
/* ---------------------------------------------------------------------- */


/**Function********************************************************************

  Synopsis           [Creates a node_ptr that represents the encoding of a
  WORD.]

  Description        [bitval is the initial value of all bits. w it the
  word width]

  SideEffects        []

******************************************************************************/
node_ptr node_word_create(node_ptr bitval, size_t w)
{
  node_ptr width = find_node(NUMBER, NODE_FROM_INT(w), Nil);
  node_ptr enc = Nil;
  for (;w>0; --w) { enc = find_node(CONS, bitval, enc); }

  return find_node(UNSIGNED_WORD, enc, width);
}


/**Function********************************************************************

  Synopsis           [Creates a node_ptr that represents the encoding of a
  WORD, taking the values of bits from the given list]

  Description        [The list (of CONS nodes) must have length equal to w]

  SideEffects        [node_word_create]

******************************************************************************/
node_ptr node_word_create_from_list(node_ptr l, size_t w)
{
  nusmv_assert(node_get_type(l) == CONS);
  nusmv_assert(llength(l) == w);

  return find_node(UNSIGNED_WORD, l, find_node(NUMBER, NODE_FROM_INT(w), Nil));
}


/**Function********************************************************************

  Synopsis           [Creates a node_ptr that represents the encoding of a
  WORD, taking the values of bits from the given WordNumber]

  Description        [Word width is taken from the given WordNumber]

  SideEffects        []

******************************************************************************/
node_ptr node_word_create_from_wordnumber(WordNumber_ptr wn)
{
  node_ptr bits;
  int w, i;

  w = WordNumber_get_width(wn);
  bits = Nil;
  for (i=0; i<w; ++i) {
    bits = find_node(CONS,
                     WordNumber_get_bit(wn, i)
                     ? Expr_true() : Expr_false(),
                     bits);
  }

  return node_word_create_from_list(bits, w);
}


/**Function********************************************************************

  Synopsis           [Creates a node_ptr that represents the encoding of a
  WORD, taking the values of bits from the given integer value]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_create_from_integer(unsigned long long value,
                                       size_t width)
{
  return node_word_create_from_wordnumber(
                          WordNumber_from_integer(value, width));
}


/**Function********************************************************************

  Synopsis           [Creates a node_ptr that represents the encoding of a
  WORD, taking the values of bits from the given array of nodes.]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_create_from_array(array_t* arr)
{
  node_ptr res = Nil;
  node_ptr bit;
  int i;

  arrayForEachItem(node_ptr, arr, i, bit) { res = find_node(CONS, bit, res); }
  return node_word_create_from_list(res, array_n(arr));
}


/**Function********************************************************************

  Synopsis           [Returns the width of the given word]

  Description        []

  SideEffects        []

******************************************************************************/
size_t node_word_get_width(node_ptr w)
{
  return node_get_int(cdr(w));
}


/**Function********************************************************************

  Synopsis           [Converts the given word to a dynamic array.]

  Description        [The array must be freed by the caller.
  Note that the order is reversed,i.e. bits found earlier in the WORD expression
  are but closer to the end in the array (they should be higher bits).]


  SideEffects        []

******************************************************************************/
array_t* node_word_to_array(node_ptr w)
{
  int wid, i;
  array_t* res;
  node_ptr iter;

  _CHECK_WORD(w);

  wid = node_get_int(cdr(w));
  res = array_alloc(node_ptr, wid);
  for (i=wid-1, iter=car(w); i>=0; --i, iter=cdr(iter)) {
    array_insert(node_ptr, res, i, car(iter));
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Private helpers for node_word_apply_unary]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static int _apply_op = 0;
static node_ptr _node_word_apply_unary_aux(node_ptr e)
{
  /* this optimizes the result when possible */
  switch (_apply_op) {
  case NOT: return Expr_not(e);
  default: return find_node(_apply_op, e, Nil);
  }
}

/**Function********************************************************************

  Synopsis           [Private helper for node_word_apply_attime]

  Description        [Helper for node_word_apply_attime]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static int _time_op = 0;
static node_ptr _node_word_apply_attime_aux(node_ptr e)
{
  return Expr_attime(e, _time_op, SYMB_TABLE(NULL));
}

static node_ptr _node_word_apply_binary_aux(node_ptr e1, node_ptr e2)
{
  /* this optimizes the result when possible */
  switch (_apply_op) {
  case AND: return Expr_and(e1, e2);
  case OR: return Expr_or(e1, e2);
  case XOR: return Expr_xor(e1, e2);
  case XNOR: return Expr_xnor(e1, e2);
  case IMPLIES: return Expr_implies(e1, e2);
  case IFF: return Expr_iff(e1, e2);
  default: return find_node(_apply_op, e1, e2);
  }
}


/**Function********************************************************************

  Synopsis           [Traverses the word bits, and foreach bit creates a node
  whose operator is given. The result is returned as a new word encoding]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_apply_unary(node_ptr wenc, int op)
{
  node_ptr res;

  _CHECK_WORD(wenc);

  _apply_op = op;
  res = map(_node_word_apply_unary_aux, car(wenc));
  return find_node(UNSIGNED_WORD, res, cdr(wenc));

}

/**Function********************************************************************

  Synopsis           [Traverses the word bits, and foreach bit creates a node
  whose operator is given. The result is returned as a new word encoding]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_apply_attime(node_ptr wenc, int time)
{
  node_ptr res;

  _CHECK_WORD(wenc);
  _time_op = time;
  res = map(_node_word_apply_attime_aux, car(wenc));
  return find_node(UNSIGNED_WORD, res, cdr(wenc));
}


/**Function********************************************************************

  Synopsis [Traverses two given words, and creates a new word
  applying to each pair of bits the given operator]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_apply_binary(node_ptr wenc1, node_ptr wenc2, int op)
{
  node_ptr res;

  _CHECK_WORDS(wenc1, wenc2);

  _apply_op = op;
  res = map2(_node_word_apply_binary_aux, car(wenc1), car(wenc2));
  return find_node(UNSIGNED_WORD, res, cdr(wenc1));
}


/**Function********************************************************************

  Synopsis           [Returns an AND node that is the conjuction of all
  bits of the given word]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_make_conjuction(node_ptr w)
{
  node_ptr res = Expr_true();
  for (w=car(w); w != Nil; w=cdr(w)) res = Expr_and(car(w), res);
  return res;
}


/**Function********************************************************************

  Synopsis           [Returns an OR node that is the disjuction of all
  bits of the given word]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_make_disjunction(node_ptr w)
{
  node_ptr res = Expr_false();
  for (w=car(w); w != Nil; w=cdr(w)) res = Expr_or(car(w), res);
  return res;
}

/**Function********************************************************************

  Synopsis           [Casts the given word to boolean]

  Description        [The word must have width 1]

  SideEffects        []

******************************************************************************/
node_ptr node_word_cast_bool(node_ptr w)
{
  _CHECK_WORD(w);
  nusmv_assert(node_get_int(cdr(w)) == 1);
  return car(car(w));
}

/**Function********************************************************************

  Synopsis [Returns a new word that is the negation of the given
  word]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_not(node_ptr w)
{
  return node_word_apply_unary(w, NOT);
}

/**Function********************************************************************

  Synopsis           [Returns a new word that is the conjuction of the given
  words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_and(node_ptr a, node_ptr b)
{
  return node_word_apply_binary(a, b, AND);
}

/**Function********************************************************************

  Synopsis           [Returns a new word that is the disjuction of the given
  words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_or(node_ptr a, node_ptr b)
{ return node_word_apply_binary(a, b, OR); }

/**Function********************************************************************

  Synopsis           [Returns a new word that is the xor of the given
  words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_xor(node_ptr a, node_ptr b)
{ return node_word_apply_binary(a, b, XOR); }

/**Function********************************************************************

  Synopsis           [Returns a new word that is the xnor of the given
  words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_xnor(node_ptr a, node_ptr b)
{ return node_word_apply_binary(a, b, XNOR); }

/**Function********************************************************************

  Synopsis [Returns a new word that is the logical implication of
  the given words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_implies(node_ptr a, node_ptr b)
{ return node_word_apply_binary(a, b, IMPLIES); }

/**Function********************************************************************

  Synopsis           [Returns a new word that is the <-> of the given
  words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_iff(node_ptr a, node_ptr b)
{ return node_word_apply_binary(a, b, IFF); }

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_equal(node_ptr a, node_ptr b)
{ return node_word_make_conjuction(node_word_iff(a, b)); }

/**Function********************************************************************

  Synopsis           [Returns a new word that is the xor of the given
  words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_notequal(node_ptr a, node_ptr b)
{ return node_word_make_disjunction(node_word_xor(a, b)); }

/**Function********************************************************************

  Synopsis           [Returns a new word that is the concatenationof the given
  words]

  Description [The first given word is the most significant word
  of the result]

  SideEffects        []

******************************************************************************/
node_ptr node_word_concat(node_ptr a, node_ptr b)
{
  size_t w;

  _CHECK_WORD(a);
  _CHECK_WORD(b);

  w = node_get_int(cdr(a)) + node_get_int(cdr(b));
  return node_word_create_from_list(append_ns(car(a), car(b)), w);
}

/**Function********************************************************************

  Synopsis [Performs bit selections of the given word, that can be
  constant and non-constant]

  Description [ Range must be compatible with the given word
  width, and must be a node in the form of COLON(NUMBER, NUMBER)]

  SideEffects        []

******************************************************************************/
node_ptr node_word_selection(node_ptr word, node_ptr range)
{
  int width, high, low, i;
  node_ptr res, iter, tmp;

  _CHECK_WORD(word);
  nusmv_assert(node_get_type(range) == COLON &&
               node_get_type(car(range)) == NUMBER &&
               node_get_type(cdr(range)) == NUMBER);

  high = node_get_int(car(range));
  low = node_get_int(cdr(range));

  /* constant? */
  if (node_get_type(word) == NUMBER_UNSIGNED_WORD ||
      node_get_type(word) == NUMBER_SIGNED_WORD) {
    WordNumber_ptr w = WORD_NUMBER(car(word));
    width = WordNumber_get_width(w);
    nusmv_assert(high >= low && low >= 0 && high < width);
    w = WordNumber_bit_select(w, high, low);
    return find_node(NUMBER_UNSIGNED_WORD, (node_ptr) w, Nil);
  }

  /* Non constant word, gets rid of higher bits */
  width = node_get_int(cdr(word));
  nusmv_assert(high >= low && low >= 0 && high < width);

  iter = car(word);
  for (i=width-1; i > high; --i) iter = cdr(iter);

  /* Takes only bits until low. */
  tmp = cons(car(iter), Nil); /* at least one bit exists */
  for (--i, iter = cdr(iter); i>=low; --i, iter = cdr(iter)) {
    tmp = cons(car(iter), tmp);
  }
  /* reverse the list and "find_node" it */
  for (res = Nil, iter = tmp; iter; iter = cdr(iter)) {
    res = find_node(CONS, car(iter), res);
  }
  free_list(tmp);

  return node_word_create_from_list(res, high-low+1);
}


/**Function********************************************************************

  Synopsis           [Concatenates bit 0 (if isSigned is false) or
  the highest bit of exp (if isSigned is true) 'times' number of times to exp]

  Description        [exp has to be a UNSIGNED_WORD and 'times' has to be
  a NUMBER]

  SideEffects        []

******************************************************************************/
node_ptr node_word_extend(node_ptr exp, node_ptr times, boolean isSigned) {
  _CHECK_WORD(exp);
  nusmv_assert(NUMBER == node_get_type(times));

  int width = node_get_int(cdr(exp));
  int delta = node_get_int(times);
  node_ptr list = car(exp);
  node_ptr bit = isSigned ? car(list) : Expr_false();

  nusmv_assert(delta >= 0);

  for (; delta > 0 ; --delta) {
    list = find_node(CONS, bit, list);
  }

  return node_word_create_from_list(list, width + node_get_int(times));
}


/**Function********************************************************************

  Synopsis [Bit-blasts the given words, creating a new word
  encoding that is an added circuit]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_adder(node_ptr a, node_ptr b, node_ptr carry_in,
                         node_ptr* carry_out)
{
  node_ptr res;
  node_ptr width;

  _CHECK_WORDS(a, b);

  width = cdr(a);
  res = Nil;
  for (a=reverse_ns(car(a)),b=reverse_ns(car(b));
       a!=Nil && b != Nil; a=cdr(a), b=cdr(b)) {
    node_ptr bit_carry;
    node_ptr bit = node_word_full_adder(car(a), car(b), carry_in, &bit_carry);
    res = find_node(CONS, bit, res);
    carry_in = bit_carry;
  }

  *carry_out = carry_in;
  return find_node(UNSIGNED_WORD, res, width);
}


/**Function********************************************************************

  Synopsis           [Creates a new word encoding that is the circuit that
  adds given words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_plus(node_ptr a, node_ptr b)
{
  node_ptr carry_out;
  return node_word_adder(a, b, Expr_false(), &carry_out);
}

/**Function********************************************************************

  Synopsis           [Creates a new word encoding that is the circuit that
  subtracts given words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_minus(node_ptr a, node_ptr b)
{
  /* a-b ==  a+(not b)+1 */
  node_ptr carry_out;
  return node_word_adder(a, node_word_not(b), Expr_true(), &carry_out);
}

/**Function********************************************************************

  Synopsis           [Creates a new word encoding that is the circuit that
  performs unsigned subtraction of given words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_uminus(node_ptr a)
{
  /* -a ==  (not a)+1 */
  node_ptr carry_out;
  node_ptr zero, zenc, wzero;
  int w;

  _CHECK_WORD(a);

  /* creates 0b0 */
  zero = Expr_false();
  zenc = Nil;
  for (w = node_get_int(cdr(a)); w > 0; --w) zenc = find_node(CONS, zero, zenc);
  wzero = find_node(UNSIGNED_WORD, zenc, cdr(a));

  return node_word_adder(node_word_not(a), wzero, Expr_true(), &carry_out);
}

/**Function********************************************************************

  Synopsis           [Creates a new word encoding that is the circuit that
  performs multiplication of given words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_times(node_ptr a, node_ptr b)
{
  /* A*B = ((B & A[0])<<0) +...+ ((B & A[i])<<i) +...+ ((B & A[N-1])<<N-1) */
  array_t *va, *vb, *vab;
  int w, i;

  _CHECK_WORDS(a,b);

  w = node_get_int(cdr(a));

  va = node_word_to_array(a);
  vb = node_word_to_array(b);

  vab = array_alloc(node_ptr, w);
  /* prepares (a[0] & b) */
  for (i=0; i<w; ++i) {
    node_ptr bit_a = array_fetch(node_ptr, va, 0);
    node_ptr bit_b = array_fetch(node_ptr, vb, i);
    array_insert(node_ptr, vab, i, Expr_and(bit_a, bit_b));
  }

  for (i=1; i<w; ++i) {
    node_ptr cin = Expr_false();
    int k;

    for (k=0; k < w-i; ++k) {
      node_ptr sum, ctmp;
      node_ptr bit_a = array_fetch(node_ptr, va, i);
      node_ptr bit_b = array_fetch(node_ptr, vb, k);
      sum = node_word_full_adder(array_fetch(node_ptr, vab, i+k),
                                 Expr_and(bit_a, bit_b), cin, &ctmp);
      array_insert(node_ptr, vab, i+k, sum);
      cin = ctmp;
    }
  }

  array_free(vb);
  array_free(va);

  { /* creates the WORD from the bit array */
    node_ptr res = node_word_create_from_array(vab);
    array_free(vab);
    return res;
  }
}


/**Function********************************************************************

  Synopsis           [Creates a new word encoding that is the circuit that
  divides given unsigned words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_unsigned_divide(node_ptr a, node_ptr b)
{
  node_ptr rem;
  return node_word_unsigned_divide_reminder(a, b, &rem);
}

/**Function********************************************************************

  Synopsis           [Creates a new word encoding that is the circuit that
  performs modulo of given unsigned words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_unsigned_mod(node_ptr a, node_ptr b)
{
  node_ptr rem;
  node_word_unsigned_divide_reminder(a, b, &rem);
  return rem;
}
/**Function********************************************************************

  Synopsis           [Creates a new word encoding that is the circuit that
  divides given signed words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_signed_divide(node_ptr a, node_ptr b)
{
  node_ptr rem;
#if 1
  return node_word_signed_divide_reminder_simple(a, b, &rem);
#else
  return node_word_signed_divide_reminder_hardware(a, b, &rem);
#endif

}

/**Function********************************************************************

  Synopsis           [Creates a new word encoding that is the circuit that
  performs modulo of given signed words]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_signed_mod(node_ptr a, node_ptr b)
{
  node_ptr rem;
#if 1
  node_word_signed_divide_reminder_simple(a, b, &rem);
#else
  node_word_signed_divide_reminder_hardware(a, b, &rem);
#endif
  return rem;
}

/**Function********************************************************************

  Synopsis           [Predicate for a < b]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_unsigned_less(node_ptr a, node_ptr b)
{
  /* "carry bit of" ((not A) + B) */
  node_ptr carry_out;
  node_word_adder(node_word_not(a), b, Expr_false(), &carry_out);
  return carry_out;
}

/**Function********************************************************************

  Synopsis           [Predicate for a <= b]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_unsigned_less_equal(node_ptr a, node_ptr b)
{
  /* "carry bit of" ((not A) + (B+1)) */
  node_ptr carry_out;
  node_word_adder(node_word_not(a), b, Expr_true(), &carry_out);
  return carry_out;
}

/**Function********************************************************************

  Synopsis           [Predicate for a > b]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_unsigned_greater(node_ptr a, node_ptr b)
{ return node_word_unsigned_less(b, a); }

/**Function********************************************************************

  Synopsis           [Predicate for a >= b]

  Description        []

  SideEffects        []

******************************************************************************/
node_ptr node_word_unsigned_greater_equal(node_ptr a, node_ptr b)
{ return node_word_unsigned_less_equal(b, a); }

/**Function********************************************************************

  Synopsis           [Predicate for a <s b]

  Description        [Signed operation is performed]

  SideEffects        []

******************************************************************************/
node_ptr node_word_signed_less(node_ptr a, node_ptr b)
{
  /* A <s B
     === (A[msb] & !B[msb]) | ((A[msb] = B[msb]) & (A <u B)) */
  return node_word_signed_op(a, b, node_word_unsigned_less);
}

/**Function********************************************************************

  Synopsis           [Predicate for a <=s b]

  Description        [Signed operation is performed]

  SideEffects        []

******************************************************************************/
node_ptr node_word_signed_less_equal(node_ptr a, node_ptr b)
{
  /* A <=s B
     === (A[msb] & !B[msb]) | ((A[msb] = B[msb]) & (A <=u B)) */
  return node_word_signed_op(a, b, node_word_unsigned_less_equal);
}

/**Function********************************************************************

  Synopsis           [Predicate for a >s b]

  Description        [Signed operation is performed]

  SideEffects        []

******************************************************************************/
node_ptr node_word_signed_greater(node_ptr a, node_ptr b)
{
  /* A >s B
     === B <s A */
  return node_word_signed_less(b, a);
}

/**Function********************************************************************

  Synopsis           [Predicate for a >=s b]

  Description        [Signed operation is performed]

  SideEffects        []

******************************************************************************/
node_ptr node_word_signed_greater_equal(node_ptr a, node_ptr b)
{
  /* A >=s B
     === B <=s A */
  return node_word_signed_less_equal(b, a);
}


/**Function********************************************************************

  Synopsis           [A private service for predicates]

  Description        []

  SideEffects        []

******************************************************************************/
static node_ptr node_word_signed_op(node_ptr a, node_ptr b, NPFNN op)
{
  _CHECK_WORDS(a, b);

  node_ptr msb_a = car(car(a));
  node_ptr msb_b = car(car(b));

  node_ptr opres = op(a, b);
  node_ptr tmp1, tmp2;

  /* tmp1 := ((A[msb] = B[msb]) & (A <u B)) */
  tmp1 = Expr_and(Expr_iff(msb_a, msb_b), opres);

  /* tmp2 := (A[msb] & !B[msb]) */
  tmp2 = Expr_and(msb_a, Expr_not(msb_b));

  /* result: tmp1 | tmp2 */
  return Expr_or(tmp1, tmp2);
}


/**Function********************************************************************

  Synopsis           [Implements a full adder circuit]

  Description        [implements a full adder circuit]

  SideEffects        []

******************************************************************************/
static node_ptr
node_word_full_adder(node_ptr ai, node_ptr bi, node_ptr carry_in,
                     node_ptr* carry_out)
{
  node_ptr tmp = Expr_xor(ai, bi);

  /* curry_out = (arg1 and arg2) OR ((arg1 XOR arg2) AND carry_in) */
  *carry_out = Expr_or(Expr_and(ai, bi),
                       Expr_and(tmp, carry_in));

  /* sum = arg1 XOR arg2 XOR carry_in */
  return Expr_xor(tmp, carry_in);
}


/**Function********************************************************************

  Synopsis           [Implements a (unsigned) divide-with-reminder circuit]

  Description        [Quotient is directly returned and remainder is return
  in 'reminder']

  SideEffects        []

******************************************************************************/
static node_ptr node_word_unsigned_divide_reminder(node_ptr a, node_ptr b,
                                                   node_ptr* reminder)
{
  int width, i;
  array_t* va, *vb, *vquot, *vrem;

  _CHECK_WORDS(a,b);

  width = node_get_int(cdr(a));


  /* creates quotient and remainder */
  vquot = array_alloc(node_ptr, width);
  vrem = node_word_to_array(node_word_create(Expr_false(), width));
  va = node_word_to_array(a);
  vb = node_word_to_array(b);

  /* calculates the division operation */
  for (i=width-1; i>=0; --i) {
    node_ptr bit, iter;
    int k;

    /* shifts remainder left by 1 bit and add the i-th bit from 'a' */
    for (k = width-1; k > 0; --k) {
      bit = array_fetch(node_ptr, vrem, k-1);
      array_insert(node_ptr, vrem, k, bit);
    }
    bit = array_fetch(node_ptr, va, i);
    array_insert(node_ptr, vrem, 0, bit);

    /* calculates (rem >= b), i.e. that subtraction can be performed,
     and the subtraction itself, i.e. (rem - b)
    */
    node_ptr rem = node_word_create_from_array(vrem);
    node_ptr is_dividable = node_word_unsigned_greater_equal(rem, b);
    node_ptr substruction = node_word_minus(rem, b);

    /* set the quotient's bit */
    array_insert(node_ptr, vquot, i, is_dividable);

    /* sets remainder to ITE(rem>=b, rem-b, rem) */
    for (k=width-1, iter = car(substruction);
         k>=0;
         --k, iter = cdr(iter)) {
      node_ptr bit = Expr_ite(is_dividable, car(iter),
                              array_fetch(node_ptr, vrem, k),
                              SYMB_TABLE(NULL));
      array_insert(node_ptr, vrem, k, bit);
    }
  } /* for i */

  array_free(vb);
  array_free(va);

  /* Now guards every bit of quotient and remainder by the condition
     that the divisor is not equal to zero */
  {
    /* prepares a check for 'b' not to be zero, to be inserted in the
       resulting encoding */
    node_ptr b_nz = node_word_make_disjunction(b);

    node_ptr dbz = failure_make("Division by zero",
                                FAILURE_DIV_BY_ZERO,
                                node_get_lineno(get_the_node()));

    for (i=0; i<width; ++i) {
      node_ptr ite;

      /* quotient */
      ite = Expr_ite(b_nz,
                     array_fetch(node_ptr, vquot, i),
                     dbz,
                     SYMB_TABLE(NULL));
      array_insert(node_ptr, vquot, i, ite);

      /* reminder */
      ite = Expr_ite(b_nz,
                     array_fetch(node_ptr, vrem, i),
                     dbz,
                     SYMB_TABLE(NULL));
      array_insert(node_ptr, vrem, i, ite);
    }
  }

  node_ptr rem = node_word_create_from_array(vrem);
  node_ptr quot = node_word_create_from_array(vquot);
  array_free(vrem);
  array_free(vquot);

  *reminder = rem;
  return quot;
}


/**Function********************************************************************

  Synopsis           [Implements a (signed) divide-with-reminder circuit]

  Description        [Quotient is directly returned and remainder is return
  in 'reminder'.
  This is simple impelementation of signed division.
  See also node_word_signed_divide_reminder_hardware.]

  SideEffects        [node_word_signed_divide_reminder_hardware]

******************************************************************************/
static node_ptr node_word_signed_divide_reminder_simple(node_ptr a, node_ptr b,
                                                        node_ptr* reminder)
{
  node_ptr quot, rem;
  node_ptr iter, iter_neg;
  node_ptr list;
  int width;

  _CHECK_WORDS(a,b);

  width = node_get_int(cdr(a));


  /* signs of a and b */
  node_ptr sign_a = car(car(a));
  node_ptr sign_b = car(car(b));

  /* now obtain for sure not-negative a and b */

  node_ptr positive_a = node_word_uminus(a);
  node_ptr positive_b = node_word_uminus(b);

  list = Nil;
  for (iter = car(a), iter_neg = car(positive_a);
       iter != Nil;
       iter = cdr(iter), iter_neg = cdr(iter_neg)) {
    node_ptr bit = Expr_ite(sign_a,
                            car(iter_neg),
                            car(iter),
                            SYMB_TABLE(NULL));
    list = cons(bit, list);
  }
  list = reverse(list);
  positive_a = new_node(UNSIGNED_WORD, list, cdr(a));


  list = Nil;
  for (iter = car(b), iter_neg = car(positive_b);
       iter != Nil;
       iter = cdr(iter), iter_neg = cdr(iter_neg)) {
    node_ptr bit = Expr_ite(sign_b,
                            car(iter_neg),
                            car(iter),
                            SYMB_TABLE(NULL));
    list = cons(bit, list);
  }
  list = reverse(list);
  positive_b = new_node(UNSIGNED_WORD, list, cdr(b));

  /* perform unsigned division */
  quot = node_word_unsigned_divide_reminder(positive_a, positive_b, &rem);

  free_list(car(positive_a));
  free_node(positive_a);
  free_list(car(positive_b));
  free_node(positive_b);

  /* negate the remainder if the dividend was negative */
  {
    node_ptr negated_rem = node_word_uminus(rem);

    array_t* arr_rem = node_word_to_array(rem);
    array_t* arr_neg_rem = node_word_to_array(negated_rem);
    int i;
    for (i=0; i<width; ++i) {
      node_ptr bit = array_fetch(node_ptr, arr_neg_rem, i);
      bit = Expr_ite(sign_a,
                     bit,
                     array_fetch(node_ptr, arr_rem, i),
                     SYMB_TABLE(NULL));
      array_insert(node_ptr, arr_rem, i, bit);
    }
    rem = node_word_create_from_array(arr_rem);
    array_free(arr_rem);
    array_free(arr_neg_rem);
  }

  {
    node_ptr negated_quot = node_word_uminus(quot);

    array_t* arr_quot = node_word_to_array(quot);
    array_t* arr_neg_quot = node_word_to_array(negated_quot);
    node_ptr diff_sign = Expr_xor(sign_a, sign_b);
    int i;
    for (i=0; i<width; ++i) {
      node_ptr bit = array_fetch(node_ptr, arr_neg_quot, i);
      bit = Expr_ite(diff_sign,
                     bit,
                     array_fetch(node_ptr, arr_quot, i),
                     SYMB_TABLE(NULL));
      array_insert(node_ptr, arr_quot, i, bit);
    }
    quot = node_word_create_from_array(arr_quot);
    array_free(arr_quot);
    array_free(arr_neg_quot);
  }

  *reminder = rem;
  return quot;
}


/**Function********************************************************************

  Synopsis           [Implements a (signed) divide-with-reminder circuit]

  Description        [Quotient is directly returned and remainder is return
  in 'reminder'.
  This is alternative to node_word_signed_divide_reminder_simple.
  This function should be impelemented similar to
  add_array_signed_division_remainder_hardware.
  Then node_word_signed_divide_reminder_simple and
  node_word_signed_divide_reminder_hardware should be compared which
  generated exps are better. Then the worse function should be removed.]

  SideEffects        [node_word_signed_divide_reminder_hardware]

******************************************************************************/
static node_ptr node_word_signed_divide_reminder_hardware(node_ptr a, node_ptr b,
                                                          node_ptr* reminder)
{
  rpterr("node_word_signed_divide_reminder_hardware is not impelemnted yet.");
  return Nil;
}

static boolean _is_bool(const node_ptr a)
{
  return (TRUEEXP == node_get_type(a) || FALSEEXP == node_get_type(a));
}

