/**CFile***********************************************************************

  FileName    [compileBEval.c]

  PackageName [compile]

  Synopsis    []

  Description [Conversion from scalar expressions to boolean expressions.]

  SeeAlso     []

  Author      [Alessandro Cimatti and Marco Roveri]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst.

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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "compile/compile.h"
#include "compileInt.h"
#include "enc/enc.h"
#include "enc/operators.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "utils/utils_io.h" /* for indent_node */
#include "utils/range.h"
#include "utils/assoc.h"
#include "utils/WordNumber.h"
#include "compile/symb_table/ResolveSymbol.h"

static char rcsid[] UTIL_UNUSED = "$Id: compileBEval.c,v 1.13.4.12.2.1.2.18.4.52 2010-02-24 18:34:52 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

static hash_ptr expr2bexpr_hash = (hash_ptr) NULL;

void init_expr2bexp_hash()
{
  nusmv_assert(expr2bexpr_hash == (hash_ptr) NULL);
  expr2bexpr_hash = new_assoc();
}

void clear_expr2bexp_hash()
{
  if (expr2bexpr_hash != (hash_ptr) NULL) {
    free_assoc(expr2bexpr_hash);
    expr2bexpr_hash = (hash_ptr) NULL;
  }
}

static node_ptr make_key(node_ptr expr, boolean a, boolean b)
{
  int j = 0;

  j += a? 1 : 0;
  j += b? 2 : 0;

  return find_node(CONTEXT, expr, NODE_FROM_INT(j));
}

static void
expr2bexpr_hash_insert_entry(node_ptr expr, node_ptr ctx,
                             node_ptr bexpr, boolean a, boolean b)
{
  node_ptr cexp;
  nusmv_assert(expr2bexpr_hash != (hash_ptr) NULL);

  if (ctx == Nil) cexp = expr;
  else cexp = find_node(CONTEXT, ctx, expr);
  insert_assoc(expr2bexpr_hash, make_key(cexp, a, b), bexpr);
}

static node_ptr
expr2bexpr_hash_lookup_entry(node_ptr expr, node_ptr ctx,
                             boolean a, boolean b)
{
  node_ptr cexp;
  nusmv_assert(expr2bexpr_hash != (hash_ptr) NULL);

  if (ctx == Nil) cexp = expr;
  else cexp = find_node(CONTEXT, ctx, expr);

  return find_assoc(expr2bexpr_hash, make_key(cexp, a, b));
}


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static node_ptr
expr2bexpr_recur ARGS((BddEnc_ptr enc, SymbLayer_ptr,
                       node_ptr, node_ptr, boolean));
static node_ptr
scalar_atom2bexpr ARGS((BddEnc_ptr, SymbLayer_ptr,
                        node_ptr, node_ptr, boolean));

static node_ptr
expr2bexpr_ite ARGS((BddEnc_ptr enc, SymbLayer_ptr det_layer,
                     node_ptr expr, node_ptr context, boolean in_next));

static node_ptr
expr2bexpr_word_ite_aux ARGS((BddEnc_ptr enc, SymbLayer_ptr det_layer,
                              node_ptr expr, node_ptr context,
                              boolean in_next));

static boolean compile_is_booleanizable_aux ARGS((const SymbTable_ptr st,
                                                  node_ptr expr,
                                                  node_ptr context,
                                                  boolean word_unbooleanizable,
                                                  hash_ptr cache));

/* static node_ptr  */
/* expr2bexpr_resolve_name_fully ARGS((SymbTable_ptr symb_table, */
/*                                     node_ptr expr, */
/*                                     node_ptr context)); */


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Converts a scalar expression into a boolean expression.]

  Description        [Takes an scalar expression intended to evaluate
  to boolean, maps through booleans down to the atomic scalar
  propositions, builds the corresponding boolean function, and returns
  the resulting boolean expression.

  The conversion of atomic scalar proposition is currently performed
  by generating the corresponding ADD, and then printing it in terms
  of binary variables.

  If one or more determinization variable must be created
  (i.e. non-determinism must be allowed) then det_layer is the
  SymbLayer instance to be filled with the newly created
  determinization variables. If non-determinism is not allowed, specify
  NULL as det_layer value. In this case you can use detexpr2bexpr as well.

  The input expression will be processed with Nil context (for
  flattened expr this does not matter).]

  SideEffects        [None]

  SeeAlso            [Compile_detexpr2bexpr, expr2bexpr_recur]

******************************************************************************/
Expr_ptr Compile_expr2bexpr(BddEnc_ptr enc,
                            SymbLayer_ptr det_layer,
                            Expr_ptr expr)
{
  node_ptr res;
  int temp = yylineno;

  if (expr == EXPR(NULL)) return expr;

  yylineno = node_get_lineno( NODE_PTR(expr) );
  res = expr2bexpr_recur(enc, det_layer, expr, Nil, false);
  yylineno = temp;
  return EXPR(res);
}


/**Function********************************************************************

  Synopsis           [Converts a scalar expression into a boolean expression.]

  Description        [Takes an scalar expression intended to evaluate
  to boolean, maps through booleans down to the atomic scalar
  propositions, builds the corresponding boolean function, and returns
  the resulting boolean expression.

  The conversion of atomic scalar proposition is currently performed
  by generating the corresponding ADD, and then printing it in terms
  of binary variables.

  An error is returned if determinization variables are introduced in
  the booleanization process.

  The input expression will be processed with Nil context (for
  flattened expr this does not matter).]

  SideEffects        [None]

  SeeAlso            [Compile_expr2bexpr, expr2bexpr_recur,
  Compile_detexpr2bexpr_list]

******************************************************************************/
Expr_ptr Compile_detexpr2bexpr(BddEnc_ptr enc, Expr_ptr expr)
{
  node_ptr res;
  int temp = yylineno;

  if (expr == EXPR(NULL)) return expr;

  yylineno = node_get_lineno(expr);

  res = expr2bexpr_recur(enc, SYMB_LAYER(NULL), expr, Nil, false);

  yylineno = temp;
  return EXPR(res);
}


/**Function********************************************************************

  Synopsis           [Converts a scalar expression into a boolean expression.]

  Description        [This function is exactly like Compile_detexpr2bexpr
  except that the input expressions is expected to be a list of expressions.
  The only purpose of this function wrt Compile_detexpr2bexpr is efficiency.
  For big model list of expressions may be huge and stack overflow may happen
  in Compile_detexpr2bexpr because the expressions are processed recursively
  whereas here top-level expressions are processed in loop.

  expr has to be a RIGHT-connected list of elements (i.e. car is head
  and cdr is tail). The connecting nodes have to be of type AND or
  CONS with the semantics of AND.  The returned expression is a list
  of the same order but with the booleanized expressions and AND used
  as connector.

  NOTE: some simplifications are done, e.g. if FALSE is met among
  elements then FALSE is returned.

  NOTE: when the function see on the right a node of a type other than
  AND and CONS then right child is considered as the last element in the
  list.

  NOTE: special case: if NEXT is met at the top then its sub-expression
  is processed as a list.

  TODO: if in future is will be necessary to process lists of
  different connector kind, e.g. OR, it will be necessary to provided
  the kind as parameter. Still AND and CONS have to dealt the same way
  because in traces it is unspecified if AND or CONS is used in
  var=value lists.]

  SideEffects        [None]

  SeeAlso            [Compile_detexpr2bexpr, Compile_expr2bexpr,
  expr2bexpr_recur]

******************************************************************************/
Expr_ptr Compile_detexpr2bexpr_list(BddEnc_ptr enc, Expr_ptr expr)
{
  node_ptr list, res, iter;
  boolean is_next = false;


  /* get rid of top level next */
  if (expr != Nil && NEXT == node_get_type(expr)) {
    is_next = true;
    expr = car(expr);
  }

  /* NOTE: to keep the order, at first, the original list is reversed
     and then it is processed. During booleanization the order is reversed
     again, i.e. in the returned expressions the original order is restored.
  */
  list = Nil;
  while (expr != Nil &&
         (node_get_type(expr) == AND || node_get_type(expr) == CONS)) {
    list = cons(car(expr), list);
    expr = cdr(expr);
  }

  /* process the last element */
  res = expr2bexpr_recur(enc, SYMB_LAYER(NULL), expr, Nil, is_next);

  /* process the rest of the list in the loop */
  for (iter = list; Nil != iter; iter = cdr(iter)) {
    node_ptr elem;

    /* lazy evaluation : avoid computation of other elements if possible */
    if (res != Nil && Expr_is_false(res)) {
      break; /* fully avoid computation of the rest */
    }

    elem = expr2bexpr_recur(enc, SYMB_LAYER(NULL), car(iter), Nil, is_next);
    res = Expr_and(elem, res); /* merge new element with existing results */
  }

  /* free the reversed list */
  free_list(list);


  return EXPR(res);
}

/**Function********************************************************************

   Synopsis           [Check if an expr is of a finite range type]

   Description        [Check if an expr is of a finite range type.

                       REMARK: Words are considered finite only if
                       word_unbooleanizable is set to false

                       If cache is not null whenever we encounter a formula in
                       the cache we simply return the previously computed value,
                       otherwise an internal and temporary map is used.

                       NOTE: the internal representation of cache is private so
                             the user should provide only caches generated by
                             this function!]

   SideEffects        [none]

******************************************************************************/
boolean Compile_is_expr_booleanizable(const SymbTable_ptr st,
                                      node_ptr expr,
                                      boolean word_unbooleanizable,
                                      hash_ptr cache)
{
  hash_ptr to_use;
  boolean res;

  if ((hash_ptr)NULL == cache) {
    to_use = new_assoc();
  }
  else {
    to_use = cache;
  }

  res = compile_is_booleanizable_aux(st, expr, Nil,
                                     word_unbooleanizable, to_use);

  if ((hash_ptr)NULL == cache) {
    free_assoc(to_use);
  }

  return res;
}


/**Function********************************************************************

  Synopsis [Cleans those hashed entries that are about a symbol that
  is being removed]

  Description        [Called by BoolEnc class when removing a layer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Compile_cleanup_booleanizer_cache_about(SymbTable_ptr st,
                                             NodeList_ptr symbs)
{
  assoc_iter iter;
  node_ptr expr;

  ASSOC_FOREACH(expr2bexpr_hash, iter, &expr, NULL) {
    Set_t deps = Formula_GetDependencies(st, expr, Nil);
    ListIter_ptr sym_iter;

    NODE_LIST_FOREACH(symbs, sym_iter) {
      node_ptr name = NodeList_get_elem_at(symbs, sym_iter);
      if (Set_IsMember(deps, name)) {
        /* removes the corresponding entry */
        insert_assoc(expr2bexpr_hash, expr, Nil);
      }
    }

    Set_ReleaseSet(deps);
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis [Creates the encoding of the left-shifting circuit for
  words]

  Description        [ numWidth is the width of b or -1 if b is not a word
  (it can be a number)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr expr2bexpr_shift_left(BddEnc_ptr enc,
                                      SymbLayer_ptr det_layer,
                                      node_ptr context,
                                      boolean in_next,
                                      node_ptr a,
                                      node_ptr b,
                                      node_ptr def_case,
                                      int numWidth)
{
  /* creates shift circuit */
  array_t* va;
  node_ptr res;
  unsigned long long maxShift;
  int width, i;
  SymbTable_ptr symb_table = BaseEnc_get_symb_table(BASE_ENC(enc));

  width = node_get_int(cdr(a));
  va = node_word_to_array(a);
  if (numWidth > 0) maxShift = (2ULL << (numWidth-1)) - 1;
  else maxShift = ~0U; /* value for sure bigger than any possible width */

  res = Nil;
  for (i=0; i<width; ++i) {
    node_ptr bit = def_case;
    int k;

    for (k=MIN(i,maxShift); k >= 0; --k) {
      node_ptr beqk;

      if (numWidth > 0) {/* b is a word */
        node_ptr wk = node_word_create_from_integer(k, numWidth);
        beqk = node_word_equal(b, wk);
      }
      else {/* b is not a word */
        beqk = expr2bexpr_recur(enc,
                                det_layer,
                                find_node(EQUAL,
                                          b,
                                          find_node(NUMBER,
                                                    NODE_FROM_INT(k),
                                                    Nil)),
                                context, in_next);
      }

      bit = Expr_ite(beqk, array_fetch(node_ptr, va, i-k), bit, symb_table);
    }

    res = cons(bit, res); /* msb at the top */
  }
  array_free(va);

  return node_word_create_from_list(res, width);
}


/**Function********************************************************************

  Synopsis [Creates the encoding of the unsigned right-shifting circuit for
  words]

  Description        []

  SideEffects        [numWidth is the width of b or -1 if b is not a word
  (it can be a number)]

  SeeAlso            []

******************************************************************************/
static node_ptr expr2bexpr_shift_right(BddEnc_ptr enc,
                                       SymbLayer_ptr det_layer,
                                       node_ptr context,
                                       boolean in_next,
                                       node_ptr a,
                                       node_ptr b,
                                       node_ptr def_case,
                                       int numWidth)
{
  /* creates shift circuit */
  array_t* va;
  node_ptr res;
  unsigned long long maxShift;
  int width, i;
  SymbTable_ptr symb_table = BaseEnc_get_symb_table(BASE_ENC(enc));

  width = node_get_int(cdr(a));
  va = node_word_to_array(a);
  if (numWidth > 0) maxShift = (2ULL << (numWidth-1)) - 1;
  else maxShift = ~0U; /* value for sure bigger than any possible width */

  res = Nil;
  for (i = 0; i < width; ++i) {
    node_ptr bit = def_case;
    int k;

    for (k = MIN(maxShift, width - i - 1); k >= 0; --k) {
      node_ptr beqk;

      if (numWidth > 0) { /* b is a word */
        node_ptr wk = node_word_create_from_integer(k, numWidth);
        beqk = node_word_equal(b, wk);
      }
      else { /* b is not a word */
        beqk = expr2bexpr_recur(enc,
                                det_layer,
                                find_node(EQUAL,
                                          b,
                                          find_node(NUMBER,
                                                    NODE_FROM_INT(k),
                                                    Nil)),
                                context,
                                in_next);
      }

      bit = Expr_ite(beqk, array_fetch(node_ptr, va, i+k), bit, symb_table);
    } /* for k */

    res = cons(bit, res); /* msb at the top */
  } /* for i */

  array_free(va);

  return node_word_create_from_list(res, width);
}


/**Function********************************************************************

  Synopsis    [Private service for shifting operations]

  Description [creates a default error case.
  numWidth is the width of b or -1 if b is not a word.
  defaultBit is a default value of a bit. Typically it is 0 and
  the highest bit of a for right signed shift.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr
expr2bexpr_get_shift_def_value(BddEnc_ptr enc, SymbLayer_ptr det_layer,
                               node_ptr context, boolean in_next,
                               node_ptr a, node_ptr b, int numWidth,
                               node_ptr defaultBit)
{
  node_ptr error, res;
  int width;
  SymbTable_ptr symb_table = BaseEnc_get_symb_table(BASE_ENC(enc));

  width = node_get_int(cdr(a));

  error = failure_make("Right operand of shift operator is out of range",
                       FAILURE_UNSPECIFIED, node_get_lineno(get_the_node()));

  if (numWidth > 0) { /* b is a word */
    /* right operand can be only *unsigned* word => do not compare with 0 */
    if ((width >= (2ULL<<(numWidth-1)) - 1)) { /* width >= (2^numWidth - 1). */
      /* 'number' cannot represent values > width. Thus
         a check that number<=width is not required. */
      res = defaultBit;
    }
    else {
      node_ptr cond = node_word_unsigned_less_equal(b,
                                  node_word_create_from_integer(width,
                                                                numWidth));
      res = Expr_ite(cond, defaultBit, error, symb_table);
    }
  }
  else { /* b is not a word */
    node_ptr bge0, blew, cond;
    bge0 = expr2bexpr_recur(enc, det_layer,
                            find_node(GE, b,
                                      find_node(NUMBER, NODE_FROM_INT(0), Nil)),
                            context, in_next);
    blew = expr2bexpr_recur(enc, det_layer,
                            find_node(LE, b,
                                      find_node(NUMBER, NODE_FROM_INT(width), Nil)),
                            context, in_next);
    cond = Expr_and(bge0, blew);
    res = Expr_ite(cond, defaultBit, error, BaseEnc_get_symb_table(BASE_ENC(enc)));
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [High-level function for shifting operations]

  Description        [This function is called directly by the booleanizer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr expr2bexpr_shift(BddEnc_ptr enc, SymbLayer_ptr det_layer,
                                 node_ptr expr, node_ptr context,
                                 boolean in_next)
{
  TypeChecker_ptr tc;
  SymbType_ptr ta, tb;
  node_ptr a, b;
  int width, numWidth;
  int etype = node_get_type(expr);

  nusmv_assert(etype == LSHIFT || etype == RSHIFT);

  tc = BaseEnc_get_type_checker(BASE_ENC(enc));
  ta = TypeChecker_get_expression_type(tc, car(expr), context);

  if (ta == SYMB_TYPE(NULL) || SymbType_is_error(ta)) {
    internal_error("expr2bexpr_shift: operand has invalid type");
  }

  nusmv_assert(SymbType_is_word(ta));
  a = expr2bexpr_recur(enc, det_layer, car(expr), context, in_next);
  _CHECK_WORD(a);
  if (node_get_type(a) == NUMBER_UNSIGNED_WORD ||
      node_get_type(a) == NUMBER_SIGNED_WORD) {
    width = WordNumber_get_width(WORD_NUMBER(car(a)));
  }
  else width = node_get_int(cdr(a));

  tb = TypeChecker_get_expression_type(tc, cdr(expr), context);
  if (SymbType_is_unsigned_word(tb)) {
    b = expr2bexpr_recur(enc, det_layer, cdr(expr), context, in_next);
    _CHECK_WORD(b);
    if (node_get_type(b) == NUMBER_UNSIGNED_WORD) {
      /* b is a constant word: converts it to integer */
      b = find_node(NUMBER,
                    NODE_FROM_INT(
          (int) WordNumber_get_unsigned_value(WORD_NUMBER(car(b)))), Nil);
      numWidth = -1;
    }
    else numWidth = node_get_int(cdr(b));
  }
  else {
    /* only unsigned word or int allowed */
    nusmv_assert(SymbType_get_tag(tb) == SYMB_TYPE_INTEGER
                 || SymbType_is_boolean(tb));
    b = cdr(expr);
    numWidth = -1;
  }

  /* checks if constant word and constant shift value */
  if (node_get_type(a) == NUMBER_UNSIGNED_WORD ||
      node_get_type(a) == NUMBER_SIGNED_WORD) {
    int bits;
    switch (node_get_type(b)) {
    case NUMBER: bits = node_get_int(b); break;
    case NUMBER_UNSIGNED_WORD:
      bits = WordNumber_get_unsigned_value(WORD_NUMBER(car(b))); break;
    case NUMBER_SIGNED_WORD:
      bits = WordNumber_get_signed_value(WORD_NUMBER(car(b))); break;
    case TRUEEXP: bits = 1; break;
    case FALSEEXP: bits = 0; break;
    default: bits = -1;
    }

    if (bits == 0) return a;
    if (bits > 0) {
      WordNumber_ptr ws;

      /* checks width */
      if (bits > WordNumber_get_width(WORD_NUMBER(car(a)))) {
        error_wrong_word_operand("Right operand of shift is out of range",
                                 expr);
      }

      if (etype == LSHIFT) ws = WordNumber_left_shift(WORD_NUMBER(car(a)), bits);
      else {
        ws = (node_get_type(a) == NUMBER_UNSIGNED_WORD) ?
          WordNumber_unsigned_right_shift(WORD_NUMBER(car(a)), bits)
          : WordNumber_signed_right_shift(WORD_NUMBER(car(a)), bits);
      }
      return find_node(node_get_type(a), (node_ptr) ws, Nil);
    }

    /* fallback to default code */
    a = node_word_create_from_wordnumber(WORD_NUMBER(car(a)));
  }  /* else bits=-1 and fallback to default code */

  set_the_node(expr); /* set node for error reporting */

  { /* construct the default value.  For right shift of signed
       words the default value is the highest bit of a.  For all
       other case default value is 0.  */
    node_ptr def_case;
    node_ptr res;

    def_case = car(car(a)); /* highest bit of a */
    def_case = expr2bexpr_get_shift_def_value(enc, det_layer, context, in_next,
                                              a, b, numWidth,
                                etype == RSHIFT && SymbType_is_signed_word(ta)
                                ? def_case : find_node(FALSEEXP, Nil, Nil));

    if (etype == LSHIFT) {
      res = expr2bexpr_shift_left(enc, det_layer, context, in_next,
                                  a, b, def_case, numWidth);
    }
    else {
      res = expr2bexpr_shift_right(enc, det_layer, context, in_next,
                                   a, b, def_case, numWidth);
    }
    return res;
  }
}


/**Function********************************************************************

  Synopsis           [High-level function for rotating words]

  Description        [This function is called directly by the booleanizer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr expr2bexpr_rotate(BddEnc_ptr enc, SymbLayer_ptr det_layer,
                                  node_ptr expr, node_ptr context,
                                  boolean in_next)
{
  TypeChecker_ptr tc;
  SymbType_ptr ta, tb;
  node_ptr a, b;
  int width, numWidth;
  int etype = node_get_type(expr);
  SymbTable_ptr symb_table = BaseEnc_get_symb_table(BASE_ENC(enc));

  nusmv_assert(etype == LROTATE || etype == RROTATE);

  tc = BaseEnc_get_type_checker(BASE_ENC(enc));
  ta = TypeChecker_get_expression_type(tc, car(expr), context);

  if (ta == SYMB_TYPE(NULL) || SymbType_is_error(ta)) {
    internal_error("expr2bexpr_rotate: operand has invalid type");
  }

  nusmv_assert(SymbType_is_word(ta));
  a = expr2bexpr_recur(enc, det_layer, car(expr), context, in_next);
  _CHECK_WORD(a);

  if (node_get_type(a) == NUMBER_UNSIGNED_WORD ||
      node_get_type(a) == NUMBER_SIGNED_WORD) {
    width = WordNumber_get_width(WORD_NUMBER(car(a)));
  }
  else width = node_get_int(cdr(a));

  tb = TypeChecker_get_expression_type(tc, cdr(expr), context);
  if (SymbType_is_word(tb)) {
    b = expr2bexpr_recur(enc, det_layer, cdr(expr), context, in_next);
    _CHECK_WORD(b);

    if (node_get_type(b) == NUMBER_UNSIGNED_WORD) {
      /* b is a constant word: converts it to integer */
      b = find_node(NUMBER,
                    NODE_FROM_INT(
            (int) WordNumber_get_unsigned_value(WORD_NUMBER(car(b)))), Nil);
      numWidth = -1;
    }
    else if (node_get_type(b) == NUMBER_SIGNED_WORD) {
      /* b is a constant word: converts it to integer */
      b = find_node(NUMBER,
                    NODE_FROM_INT(
            (int) WordNumber_get_signed_value(WORD_NUMBER(car(b)))), Nil);
      numWidth = -1;
    }
    else numWidth = node_get_int(cdr(b));
  }
  else { /* an integer value */
    b = cdr(expr);
    numWidth = -1;
  }

  /* checks if constant word and constant shift value */
  if (node_get_type(a) == NUMBER_UNSIGNED_WORD ||
      node_get_type(a) == NUMBER_SIGNED_WORD) {
    int bits;
    switch (node_get_type(b)) {
    case NUMBER: bits = node_get_int(b); break;
    case NUMBER_UNSIGNED_WORD:
      bits = WordNumber_get_unsigned_value(WORD_NUMBER(car(b))); break;
    case NUMBER_SIGNED_WORD:
      bits = WordNumber_get_signed_value(WORD_NUMBER(car(b))); break;
    case TRUEEXP: bits = 1; break;
    case FALSEEXP: bits = 0; break;
    default: bits = -1;
    }

    if (bits == 0) return a;
    if (bits > 0) {
      WordNumber_ptr ws;
      if (etype == LROTATE) {
        ws = WordNumber_left_rotate(WORD_NUMBER(car(a)), bits);
      }
      else ws =WordNumber_right_rotate(WORD_NUMBER(car(a)), bits);
      return find_node(node_get_type(a), (node_ptr) ws, Nil);
    }

    /* fallback to default code */
    a = node_word_create_from_wordnumber(WORD_NUMBER(car(a)));
  }  /* else bits=-1 and fallback to default code */

  {
    array_t* va = node_word_to_array(a);
    node_ptr err_case;
    node_ptr res;
    unsigned long long maxRotate;
    int i;

    err_case = failure_make("Right operand of rotate operation is out of range",
                            FAILURE_UNSPECIFIED, (int) node_get_lineno(expr));

    if (numWidth > 0) maxRotate = MIN(((2ULL << (numWidth-1)) - 1), width);
    else maxRotate = width;

    res = Nil;
    /* performs the rotation */
    for (i = 0; i < width; ++i) {
      node_ptr bit = err_case;
      int k;

      for (k = maxRotate; k >= 0; --k) {
        node_ptr beqk;
        int p;

        if (numWidth > 0) {
          node_ptr wk = node_word_create_from_integer(k, numWidth);
          beqk = node_word_equal(b, wk);
        }
        else { /* b is not a word */
          beqk = expr2bexpr_recur(enc, det_layer,
                                  find_node(EQUAL, b,
                                            find_node(NUMBER,
                                                      NODE_FROM_INT(k), Nil)),
                                  context, in_next);
        }

        if (etype == LROTATE) {
          p = (i >= k) ? i-k : i-k+width;
        }
        else { /* RROTATE */
          p = ((i+k) % width);
        }
        bit = Expr_ite(beqk, array_fetch(node_ptr, va, p), bit, symb_table);
      } /* for k */

      res = cons(bit, res); /* msb at the top */
    } /* for i */

    array_free(va);

    return node_word_create_from_list(res, width);
  }
}


/**Function********************************************************************

  Synopsis    [This function booleanize an unary expression in a standard way:
  at first process the argument. Then for words apply a corresponding unary
  word function, for all other type just create exp of the same kind with
 find_node.]

  Description []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static node_ptr expr2bexpr_recur_unary(BddEnc_ptr enc, SymbLayer_ptr det_layer,
                                       node_ptr expr, node_ptr context,
                                       boolean in_next)
{
  TypeChecker_ptr tc = BaseEnc_get_type_checker(BASE_ENC(enc));
  SymbType_ptr ta;

  ta = TypeChecker_get_expression_type(tc, car(expr), context);

  if (ta == SYMB_TYPE(NULL) || SymbType_is_error(ta)) {
    internal_error("expr2bexpr_recur_unary: operand has invalid type");
  }

  /* if word, applies the given operation to its encoding */
  if (SymbType_is_word(ta)) {
    node_ptr a = expr2bexpr_recur(enc, det_layer, car(expr), context, in_next);
    NPFN op;
    _CHECK_WORD(a);

    if (node_get_type(a) == NUMBER_UNSIGNED_WORD ||
        node_get_type(a) == NUMBER_SIGNED_WORD) {
      /* constant */
      WordNumber_ptr wn = WORD_NUMBER(car(a));
      switch (node_get_type(expr)) {
      case CAST_BOOL:
        nusmv_assert(WordNumber_get_width(wn) == 1);
        if (WordNumber_get_unsigned_value(wn) != 0) return find_node(TRUEEXP, Nil, Nil);
        else return find_node(FALSEEXP, Nil, Nil);

      case NOT:
        wn = WordNumber_not(wn);
        return find_node(node_get_type(a), (node_ptr) wn, Nil);

      case UMINUS:
        wn = WordNumber_unary_minus(wn);
        return find_node(node_get_type(a), (node_ptr) wn, Nil);

      default: internal_error("expr2bexpr_recur_unary: illegal expression");
      }
    }

    /* not a constant */
    switch (node_get_type(expr)) {
    case CAST_BOOL: op = node_word_cast_bool; break;
    case NOT:       op = node_word_not; break;
    case UMINUS:    op = node_word_uminus; break;
    default: internal_error("expr2bexpr_recur_unary: illegal expression");
    }

    set_the_node(expr);
    return op(a);
  }

  {  /* not a leaf, applies the standard encoding */
    node_ptr v = expr2bexpr_recur(enc, det_layer, car(expr), context, in_next);
    if (node_get_type(expr) == NOT) return Expr_not(v);
    return find_node(node_get_type(expr), v, Nil);
  }
}


/**Function********************************************************************

  Synopsis     [This function booleanize a binary expression in a standard way.]

  Description   [ For words: at first convert to boolean the arguments
  and then apply a corresponding word function.
    For all other types if the kind of an expression is arithmetic or
  relational converte the exp down to an ADD, and then back to a
  node_ptr to booleanize it.
    Otherwise process the arguments and create a new expression of the
  same kind with find_node.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static node_ptr
expr2bexpr_recur_binary(BddEnc_ptr enc, SymbLayer_ptr det_layer,
                        node_ptr expr, node_ptr context,
                        boolean in_next)
{
  TypeChecker_ptr tc = BaseEnc_get_type_checker(BASE_ENC(enc));
  SymbType_ptr t_left, t_right;
  node_ptr left = car(expr);
  node_ptr right = cdr(expr);
  node_ptr res = NODE_PTR(-1); /* for sure incorrect value to catch bugs */

  t_left = TypeChecker_get_expression_type(tc, left, context);
  t_right = TypeChecker_get_expression_type(tc, right, context);

  if (t_left == SYMB_TYPE(NULL) || t_right == SYMB_TYPE(NULL) ||
      SymbType_is_error(t_left) || SymbType_is_error(t_right)) {
    internal_error("expr2bexpr_recur_binary: operands have invalid types");
  }

  /* ------------------------------------------------------------- */
  /* if words, apply the given operation to their encodings. */
  if (SymbType_is_word(t_left) || SymbType_is_word(t_right)) {
    node_ptr a;
    node_ptr b;
    /* if one operand is word and other is boolean => convert bool to word
       (it is required by below word functions) */
    if (SymbType_is_boolean(t_right)) {
      right = find_node(CAST_WORD1, right, Nil);
      t_right = t_left;
    }
    else if (SymbType_is_boolean(t_left)) {
      left = find_node(CAST_WORD1, left, Nil);
      t_left = t_right;
    }

    a = expr2bexpr_recur(enc, det_layer, left, context, in_next);
    b = expr2bexpr_recur(enc, det_layer, right, context, in_next);

    _CHECK_WORD(a);
    _CHECK_WORD(b);

    if ((node_get_type(a) == NUMBER_UNSIGNED_WORD ||
         node_get_type(a) == NUMBER_SIGNED_WORD) &&
        (node_get_type(b) == NUMBER_UNSIGNED_WORD ||
         node_get_type(b) == NUMBER_SIGNED_WORD)) {
      /* constant */
      boolean (*bop)(WordNumber_ptr, WordNumber_ptr) = NULL;
      WordNumber_ptr (*wop)(WordNumber_ptr, WordNumber_ptr) = NULL;
      int wtype = node_get_type(a);

      switch (node_get_type(expr)) {
        /* boolean operations */
      case EQDEF: bop = WordNumber_equal; break;
      case EQUAL: bop = WordNumber_equal; break;
      case NOTEQUAL: bop = WordNumber_not_equal; break;
      case LT: bop = SymbType_is_unsigned_word(t_left)
          ? WordNumber_unsigned_less : WordNumber_signed_less;
        break;
      case LE: bop = SymbType_is_unsigned_word(t_left) ?
          WordNumber_unsigned_less_or_equal
          : WordNumber_signed_less_or_equal;
        break;
      case GT: bop = SymbType_is_unsigned_word(t_left)
          ? WordNumber_unsigned_greater : WordNumber_signed_greater;
        break;
      case GE: bop = SymbType_is_unsigned_word(t_left) ?
          WordNumber_unsigned_greater_or_equal
          : WordNumber_signed_greater_or_equal;
        break;

        /* logical and arithmetical */
      case CONCATENATION:
        wop = WordNumber_concatenate; wtype = NUMBER_UNSIGNED_WORD;
        break;

      case AND: wop = WordNumber_and; break;
      case OR: wop = WordNumber_or; break;
      case XOR: wop = WordNumber_xor; break;
      case XNOR: wop = WordNumber_xnor; break;
      case IMPLIES: wop = WordNumber_implies; break;
      case IFF: wop = WordNumber_iff; break;
      case PLUS: wop = WordNumber_plus; break;
      case MINUS: wop = WordNumber_minus; break;
      case TIMES: wop = WordNumber_times; break;
      case DIVIDE: wop = SymbType_is_unsigned_word(t_left)
          ? WordNumber_unsigned_divide : WordNumber_signed_divide;
        break;
      case MOD: wop = SymbType_is_unsigned_word(t_left)
          ? WordNumber_unsigned_mod : WordNumber_signed_mod;
        break;

      default: internal_error("expr2bexpr_recur_binary: illegal word expression");
      }

      /* not both wop and bop */
      if (bop != NULL) {
        nusmv_assert(wop == NULL);
        return (bop(WORD_NUMBER(car(a)), WORD_NUMBER(car(b)))) ?
          find_node(TRUEEXP, Nil, Nil) : find_node(FALSEEXP, Nil, Nil);
      }
      nusmv_assert(wop != NULL);
      /* according to the grammar, the result keeps the sign of
         the left operand */
      return find_node(wtype, (node_ptr) wop(WORD_NUMBER(car(a)),
                                             WORD_NUMBER(car(b))),
                       Nil);
    }
    else {  /* not both constant word numbers */
      NPFNN op;

      /* encodes the possible constant word (only one is admitted here) */
      if (node_get_type(a) == NUMBER_UNSIGNED_WORD ||
          node_get_type(a) == NUMBER_SIGNED_WORD) {
        a = node_word_create_from_wordnumber(WORD_NUMBER(car(a)));
      }
      else if (node_get_type(b) == NUMBER_UNSIGNED_WORD ||
               node_get_type(b) == NUMBER_SIGNED_WORD) {
        b = node_word_create_from_wordnumber(WORD_NUMBER(car(b)));
      }

      switch (node_get_type(expr)) {
      case EQDEF: op = node_word_equal; break;
      case CONCATENATION: op = node_word_concat; break;
      case AND: op = node_word_and; break;
      case OR: op = node_word_or; break;
      case XOR: op = node_word_xor; break;
      case XNOR: op = node_word_xnor; break;
      case IMPLIES: op = node_word_implies; break;
      case IFF: op = node_word_iff; break;
      case EQUAL: op = node_word_equal; break;
      case NOTEQUAL: op = node_word_notequal; break;
      case PLUS: op = node_word_plus; break;
      case MINUS: op = node_word_minus; break;
      case LT: op = SymbType_is_unsigned_word(t_left)
          ? node_word_unsigned_less : node_word_signed_less;
        break;
      case LE: op = SymbType_is_unsigned_word(t_left)
          ? node_word_unsigned_less_equal : node_word_signed_less_equal;
        break;
      case GT: op = SymbType_is_unsigned_word(t_left)
          ? node_word_unsigned_greater : node_word_signed_greater;
        break;
      case GE: op = SymbType_is_unsigned_word(t_left)
          ? node_word_unsigned_greater_equal : node_word_signed_greater_equal;
        break;

      case TIMES: op = node_word_times; break;
      case DIVIDE: op =  SymbType_is_unsigned_word(t_left)
          ? node_word_unsigned_divide : node_word_signed_divide;
        break;
      case MOD: op =  SymbType_is_unsigned_word(t_left)
          ? node_word_unsigned_mod : node_word_signed_mod; break;
      default: internal_error("expr2bexpr_recur_binary: illegal expression");
      }

      set_the_node(expr);
      return op(a, b);
    }
  }

  /* ------------------------------------------------------------- */
  /* if both operands are boolean, process them more efficiently */
  if (SymbType_is_boolean(t_left) &&
      SymbType_is_boolean(t_right)) {

    switch (node_get_type(expr)) {
      /* Arithmetic operations which can be booleanized without BDD */
    case LT:
      res = Expr_and(Expr_not(expr2bexpr_recur(enc, det_layer,
                                               left, context, in_next)),
                     expr2bexpr_recur(enc, det_layer,
                                      right, context, in_next));
      break;

    case GT:
      res = Expr_and(expr2bexpr_recur(enc, det_layer,
                                      left, context, in_next),
                     Expr_not(expr2bexpr_recur(enc, det_layer,
                                               right, context, in_next)));
      break;

    case LE:
      res = Expr_or(Expr_not(expr2bexpr_recur(enc, det_layer,
                                              left, context, in_next)),
                    expr2bexpr_recur(enc, det_layer,
                                     right, context, in_next));
      break;

    case GE:
      res = Expr_or(expr2bexpr_recur(enc, det_layer,
                                      left, context, in_next),
                     Expr_not(expr2bexpr_recur(enc, det_layer,
                                               right, context, in_next)));
      break;

      /* Equality/disequality */
    case EQDEF:
    case SETIN:
    case EQUAL:
      res = Expr_iff(expr2bexpr_recur(enc, det_layer,
                                      left, context, in_next),
                     expr2bexpr_recur(enc, det_layer,
                                      right, context, in_next));
      break;

    case NOTEQUAL: /* !(a=b) is smaller than (a!=b) */
      res = Expr_not(Expr_iff(expr2bexpr_recur(enc, det_layer,
                                               left, context, in_next),
                              expr2bexpr_recur(enc, det_layer,
                                               right, context, in_next)));
      break;

      /* pure local operations (impossible for scalars) */
    case AND: {
      node_ptr a = expr2bexpr_recur(enc, det_layer, left, context, in_next);
      if (Expr_is_false(a)) return a; /* fully avoid computation of the right */
      res = Expr_and(a, expr2bexpr_recur(enc, det_layer, right, context,
                                         in_next));
      break;
    }

    case OR: {
      node_ptr a = expr2bexpr_recur(enc, det_layer, left, context, in_next);
      if (Expr_is_true(a)) return a;
      res = Expr_or(a, expr2bexpr_recur(enc, det_layer, right, context,
                                        in_next));
      break;
    }

    case XOR:
      res = Expr_xor(expr2bexpr_recur(enc, det_layer, left, context, in_next),
                     expr2bexpr_recur(enc, det_layer, right, context, in_next));
      break;

    case XNOR:
      res = Expr_xnor(expr2bexpr_recur(enc, det_layer, left, context, in_next),
                      expr2bexpr_recur(enc, det_layer, right, context, in_next));
      break;

    case IMPLIES: {
      node_ptr a = expr2bexpr_recur(enc, det_layer, left, context, in_next);
      if (Expr_is_false(a)) return Expr_true();
      res = Expr_implies(a, expr2bexpr_recur(enc, det_layer, right, context,
                                             in_next));
    break;
    }

    case IFF:
      res = Expr_iff(expr2bexpr_recur(enc, det_layer, left, context, in_next),
                     expr2bexpr_recur(enc, det_layer, right, context, in_next));
      break;

      /* word operations applied to booleans */
    case CONCATENATION: {
      /* concatenation of two booleans. Tries to fix this by casting to
         word1 and retries */
      node_ptr retry = find_node(node_get_type(expr),
                                 find_node(CAST_WORD1, left, Nil),
                                 find_node(CAST_WORD1, right, Nil));

      res = expr2bexpr_recur(enc, det_layer, retry, context, in_next);
      break;
    }

      /* boolean expressions which do not require special dealing or
         simplification */
    case CONS:
      res = find_node(node_get_type(expr),
                      expr2bexpr_recur(enc, det_layer, left, context, in_next),
                      expr2bexpr_recur(enc, det_layer, right, context, in_next));
      break;

     /* arithmetic operations which cannot be easily booleanized =>
         use BDD then */
    case PLUS:
    case MINUS:
    case DIVIDE:
    case MOD:
    case TIMES:
     res = scalar_atom2bexpr(enc, det_layer, expr, context, in_next);
    break;

    case UNION:
    default:
    /* all other expressions are impossible. they cannot have boolean
       operands or it has to be processed as part of the outer
       (parent) expression */
      internal_error("impossible code location");
    }
    return res;
  }

  /* not words, not pure boolean => they are scalar, i.e.
     pass through BDD evaluation is required. */

  switch (node_get_type(expr)) {

  case NOTEQUAL: /* !(a=b) is smaller than (a!=b) */
    res = expr2bexpr_recur(enc, det_layer,
                           find_node(NOT, find_node(EQUAL, left, right), Nil),
                           context, in_next);
    break;

  case EQDEF:
  case EQUAL:

    /* Optimization: if any of the operands is a case- or
       if-then-else-expression then equality/comparison is pushed down
       to subexpressions . Such a way bigger parts of the expression will not
       go through BDD (which is especially good if conditions contains
       words).
    */
    /* A proper solution is to implement a specialized predicate
       normalizer over scalars only and invoke it before the
       booleanization. Existing normalizer probably can be
       reused. Note: it is necessary to memoize the results of
       normalization but it is better to keep them locally and
       free after every top-level normalization call */

    /* before performing any check it is necessary to get rid of any
       chains of define definitions to actually get the expression */
    {
      SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));
      left = CompileFlatten_resolve_define_chains(st, left, context);
      right = CompileFlatten_resolve_define_chains(st, right, context);
    }

    if (CASE == node_get_type(right) || CASE == node_get_type(left) ||
        IFTHENELSE == node_get_type(right) || IFTHENELSE == node_get_type(left)) {
      int kind = node_get_type(expr);
      node_ptr iter;
      node_ptr list = Nil;
      node_ptr new_exp;

      if (CASE == node_get_type(right) || IFTHENELSE == node_get_type(right)) {
        iter = right;
        while (node_get_type(iter) == CASE || node_get_type(iter) == IFTHENELSE) {
          nusmv_assert(COLON == node_get_type(car(iter)));
          list = cons(find_node(COLON, car(car(iter)),
                                find_node(kind, left, cdr(car(iter)))),
                      list);
          iter = cdr(iter);
        }
        if (FAILURE != node_get_type(iter)) iter = find_node(kind, left, iter);
      }
      else {
        iter = left;
        while (node_get_type(iter) == CASE || node_get_type(iter) == IFTHENELSE) {
          nusmv_assert(COLON == node_get_type(car(iter)));
          list = cons(find_node(COLON, car(car(iter)),
                                find_node(kind, cdr(car(iter)), right)),
                      list);
          iter = cdr(iter);
        }
        if (FAILURE != node_get_type(iter)) iter = find_node(kind, iter, right);
      }
      /* revert the list, find_node it and add CASE */
      for (new_exp = iter, iter = list; iter != Nil; iter = cdr(iter)) {
        new_exp = find_node(CASE, car(iter), new_exp);
      }
      free_list(list);

      /* process the expression again */
      res = expr2bexpr_recur(enc, det_layer, new_exp, context, in_next);
    }
    else {
      Utils_set_mode_for_range_check(false); /* issue just a warning */
      CATCH {
        res = scalar_atom2bexpr(enc, det_layer, expr, context, in_next);
      }
      FAIL {
        Utils_set_mode_for_range_check(true); /* revert to default behavior */
        rpterr(NULL);
      }
      Utils_set_mode_for_range_check(true); /* revert to default behavior */
    }
    break;

    /* Arithmetic and relational exprs are booleanized by convert to ADD
       and back to node_ptr */
  case LT:
  case GT:
  case LE:
  case GE:
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
  case UNION:
  case SETIN:
    res = scalar_atom2bexpr(enc, det_layer, expr, context, in_next);
    break;

  case AND:
  case OR:
  case XOR:
  case XNOR:
  case IMPLIES:
  case IFF:
  case CONCATENATION:
    internal_error("Only booleans can have above operators,but here are scalars");

  default:
    internal_error("Unexpected operators");
/*     /\* All other expressoin are for sure boolean and can be */
/*        process normal way *\/ */
/*     res = find_node(node_get_type(expr), */
/*                     expr2bexpr_recur(enc, det_layer, left, context, in_next), */
/*                     expr2bexpr_recur(enc, det_layer, right, context, in_next)); */
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Creates an encoding for CASE node. If CASE evaluates to
  a word, a WORD encoding is created.]

  Description        [Private sesrvice called by expr2bexpr_recur]

  SideEffects        [None]

  SeeAlso            [expr2bexpr_word_ite_aux]

******************************************************************************/
static node_ptr expr2bexpr_ite(BddEnc_ptr enc, SymbLayer_ptr det_layer,
                               node_ptr expr, node_ptr context,
                               boolean in_next)
{
  node_ptr res;
  node_ptr c;
  SymbTable_ptr symb_table = BaseEnc_get_symb_table(BASE_ENC(enc));

  nusmv_assert(node_get_type(expr) == CASE ||
               node_get_type(expr) == IFTHENELSE);

  if (SymbType_is_word(TypeChecker_get_expression_type(
                  BaseEnc_get_type_checker(BASE_ENC(enc)), expr, context))) {
    res = expr2bexpr_word_ite_aux(enc, det_layer, expr, context, in_next);
  }
  else {
    c = expr2bexpr_recur(enc, det_layer, car(car(expr)),
                         context, in_next);
    if (Expr_is_true(c)) {
      res = expr2bexpr_recur(enc, det_layer, cdr(car(expr)), context, in_next);
    }
    else if (Expr_is_false(c)) {
      res = expr2bexpr_recur(enc, det_layer, cdr(expr), context, in_next);
    }
    else { /* not a word, fallback case */
      res = Expr_ite(c, expr2bexpr_recur(enc, det_layer, cdr(car(expr)),
                                         context, in_next),
                        expr2bexpr_recur(enc, det_layer, cdr(expr),
                                         context, in_next), symb_table);
    }
  }
  return res;
}

/**Function********************************************************************

 Synopsis [Service of expr2bexpr_word_ite, that creates the word encoding]

 Description        [Creates the resulting WORD encoding as:
 <textarea>
                     WORD
                  /        \
                 /          NUMBER(size)
                /
             _____ CONS ________________
            /                           \
       _   ITE                         CONS
      |  /  |  \                      /    \
      |c1 t1.0  ITE                 ITE     ...
  bit0|        /  |  \            _ ...
  |          c2  t2.0 ITE       |
  .           ...      |        |
  .                    .    bit1|
  |_                   .
  |_
</textarea>
Encoding complexity is N*C (N=word width, C=num of cases)]

SideEffects        []

SeeAlso            []

******************************************************************************/
static node_ptr
expr2bexpr_word_ite_aux(BddEnc_ptr enc, SymbLayer_ptr det_layer,
                        node_ptr expr, node_ptr context, boolean in_next)
{
  node_ptr res;
  node_ptr bcase, iter, failure;
  int size = -1;
  SymbTable_ptr symb_table = BaseEnc_get_symb_table(BASE_ENC(enc));

  /* extracts and booleanizes the case */
  bcase = Nil; /* a list of pairs <bcond, bthen as array> */
  failure = Nil; /* to hold possible FAILURE node */
  iter = expr;
  while (true) {
    node_ptr bcond = expr2bexpr_recur(enc, det_layer, car(car(iter)),
                                      context, in_next);

    /* optimization: if the condition is FALSE then skip the current expression */
    if (!Expr_is_false(bcond)) {
      node_ptr bthen = expr2bexpr_recur(enc, det_layer, cdr(car(iter)),
                                        context, in_next);
      _CHECK_WORD(bthen);

      if ((node_get_type(bthen) == NUMBER_UNSIGNED_WORD ||
           node_get_type(bthen) == NUMBER_SIGNED_WORD)) {
        /* encodes the constant word as a normal word */
        bthen = node_word_create_from_wordnumber(WORD_NUMBER(car(bthen)));
      }

      /* all then nodes have the same width: */
      if (size == -1) size = node_word_get_width(bthen);
      else { nusmv_assert(node_word_get_width(bthen) == size); }

      /* optimization : if condition is TRUE do not process anything else */
      if (Expr_is_true(bcond)) { failure = bthen; break; }

      bcase = cons(find_node(COLON, bcond, (node_ptr) node_word_to_array(bthen)),
                   bcase);
    }

    iter = cdr(iter);

    /* terminating conditions */
    if (node_get_type(iter) == FAILURE) { failure = iter; break; }
    else if (node_get_type(iter) != CASE && node_get_type(iter) != IFTHENELSE) {
      /* non-standard last case, i.e. a normal word expression */
      failure = expr2bexpr_recur(enc, det_layer, iter, context, in_next);
      _CHECK_WORD(failure); /* must be a proper word expr */

      if ((node_get_type(failure) == NUMBER_UNSIGNED_WORD ||
           node_get_type(failure) == NUMBER_SIGNED_WORD)) {
        /* encodes the constant word as a normal word */
        failure = node_word_create_from_wordnumber(WORD_NUMBER(car(failure)));
      }
      /* all conditions were FALSE => only now the size is known */
      if (-1 == size) size = node_word_get_width(failure);
      nusmv_assert(node_word_get_width(failure) == size); /* of proper size */
      break;
    }
  }

  /* all conditions were simplified to FALSE, there must be default exp */
  if (Nil == bcase && FAILURE == node_get_type(failure)) {
    rpterr_node(expr, "Error: in conditional expression all conditions "
                "are false and there is no default case :  ");
  }

  /* get a reversed list of word bits for default expression.
     We do not free previous value of failure because it may or may not
     be find_noded and may potentially be already cached somewhere (in type
     checker or evaluator) */
  if (FAILURE != node_get_type(failure)) failure = reverse_ns(car(failure));

  { /* creates the encoding */

    int i;
    node_ptr wbits = Nil;
    node_ptr failure_iter = failure; /* used if failure is a list of bits */

    for (i=0; i < size; ++i) {
      node_ptr cases;
      /* create a failure value:
         'failure' may be a FAILURE node or a list of bits
      */
      if (FAILURE == node_get_type(failure)) cases = failure;
      else {
        nusmv_assert(CONS == node_get_type(failure_iter)); /* a list of bits */
        cases = car(failure_iter);
        failure_iter = cdr(failure_iter);/* a bit for next iteration */
      }

      for (iter=bcase; iter!=Nil; iter=cdr(iter)) {
        nusmv_assert(node_get_type(car(iter)) == COLON);
        node_ptr bcond = car(car(iter));
        node_ptr biti = array_fetch(node_ptr, (array_t*) cdr(car(iter)), i);

        cases = Expr_ite(bcond, biti, cases, symb_table);
      }
      wbits = cons(cases, wbits);
    }

    res = node_word_create_from_list(wbits, size);
  }

  /* frees the bcase structure and the failure list of bits */
  for (iter=bcase; iter!=Nil; iter=cdr(iter)) {
    array_free((array_t*) cdr(car(iter)));
  }
  /* these lists were not cached anywhere (for example, during evaluation) */
  free_list(bcase);
  if (CONS == node_get_type(failure)) free_list(failure);

  return res;
}


/**Function********************************************************************

 Synopsis           [Converts a generic expression into a boolean expression.]

 Description        [Takes an expression intended to evaluate to boolean,
 maps through booleans down to the atomic scalar propositions,
 builds the corresponding boolean function,
 and returns the resulting boolean expression.

 The conversion of atomic scalar proposition is currently performed
 by generating the corresponding ADD, and then printing it in terms
 of binary variables.

 The introduction of determinization variables is allowed only if
 parameter "det_layer" is not NULL; Those variables will be declared
 within the given layer

 The input expression may be normal (not flattened), flattened or
 expanded. Parameter 'context' is used if the expression is not flattened.]

 SideEffects        [None]

 SeeAlso            [Compile_expr2bexpr, detexpr2bexpr]

******************************************************************************/
static node_ptr expr2bexpr_recur(BddEnc_ptr enc,
                                 SymbLayer_ptr det_layer,
                                 node_ptr expr,
                                 node_ptr context,
                                 boolean in_next)
{
/* For example, in above function cons(..) is returned.
The behavior has to be consistent. */

  SymbTable_ptr symb_table;
  TypeChecker_ptr tc;
  node_ptr res;

  if (expr == Nil) return Nil;

  res = expr2bexpr_hash_lookup_entry(expr, context, in_next,
                                     det_layer != SYMB_LAYER(NULL));
  if (res != (node_ptr) NULL) return res;

  symb_table = BaseEnc_get_symb_table(BASE_ENC(enc));
  tc = BaseEnc_get_type_checker(BASE_ENC(enc));

  switch (node_get_type(expr)) {

  case FAILURE:
  case TRUEEXP:
  case FALSEEXP:
    res = expr;
    break;

  case NUMBER:
    rpterr("Number can not be casted to boolean");
    break;

  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
    /* delayed encoding */
    res = expr;
    /* previous code was : */
    /* res = node_word_create_from_wordnumber(WORD_NUMBER(car(expr))); */
    break;

  case UWCONST:
  case SWCONST:
    /* here we rely on the flattener, which is in charge of getting
       rid of SWCONST and UWCONST */
    res = Compile_FlattenSexp(symb_table, expr, context);
    break;

  case WSIZEOF:
    rpterr("Number can not be casted to boolean");

    /* here we rely on the flattener, which is in charge of getting
       rid of WSIZEOF */
    /* res = Compile_FlattenSexp(symb_table, expr, context); */

    /* The result here should be a NUMBER */
    /* nusmv_assert(NUMBER == node_get_type(res)); */
    break;

  case CAST_TOINT:
    rpterr("Number can not be casted to boolean");

    /* here we rely on the flattener, which is in charge of getting
       rid of CAST_TOINT */
    /* res = Compile_FlattenSexp(symb_table, expr, context); */

    /* The result here should be a NUMBER */
    /* nusmv_assert(NUMBER == node_get_type(res)); */
    break;

  case CAST_BOOL:
    {
    /* here we rely on the flattener, which is in charge of getting
       rid of CAST_BOOL */
      node_ptr tmp = Compile_FlattenSexp(symb_table, expr, context);
      res = expr2bexpr_recur(enc, det_layer, tmp, Nil, in_next);
      break;
    }

  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    rpterr("Real constant cannot be casted to boolean");

  case BIT:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, expr, context);
      name = ResolveSymbol_get_resolved_name(rs);

      nusmv_assert(ResolveSymbol_is_var(rs));

      if (in_next) res = find_node(NEXT, name, Nil);
      else res = name;
      break;
    }

  case ARRAY:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, expr, context);
      name = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_undefined(rs)) {
        /* ARRAY may be identifier-with-brackets and may be an
           expression.  Here array expression is detected => flatten
           it and then process.  See description of
           compileFlattenSexpRecur for details */
        node_ptr tmp = Compile_FlattenSexp(symb_table, expr, context);
        nusmv_assert(tmp != expr); /* loop in recursion is impossible */
        res = expr2bexpr_recur(enc, det_layer, tmp, Nil, in_next);
        break;
      }
      else {
        /* array is actually identifier => process it with ATOM and DOT */
      }
      /* NO BREAK HERE */
    }

  case DOT:
  case ATOM:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, expr, context);
      name = ResolveSymbol_get_resolved_name(rs);

      /* no check for ambiguity is required, since type checker did it */

      /* parameter */
      if (ResolveSymbol_is_parameter(rs)) {
        /* process the value of the parameter */
        node_ptr param = SymbTable_get_flatten_actual_parameter(symb_table, name);
        res = expr2bexpr_recur(enc, det_layer, param, context, in_next);
        break;
      }

      /* define */
      if (ResolveSymbol_is_define(rs)) {
        /* define (rhs must be boolean, recur to check) */
        node_ptr body = SymbTable_get_define_flatten_body(symb_table, name);
        if (body == (node_ptr) NULL) error_undefined(name);

        res = expr2bexpr_recur(enc, det_layer, body, context, in_next);
      }
      /* variable */
      else if (ResolveSymbol_is_var(rs)) {
        SymbType_ptr vtype = SymbTable_get_var_type(symb_table, name);
        /* variable, must be boolean or word */
        if (SymbType_is_boolean(vtype)) {
          if (in_next) res = find_node(NEXT, name, Nil);
          else res = name;
        }
        else if (SymbType_is_word(vtype)) {
          BoolEnc_ptr benc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(enc));

          res = BoolEnc_get_var_encoding(benc, name);
          _CHECK_WORD(res);
          if (in_next) res = node_word_apply_unary(res, NEXT);
        }
        else {
          rpterr("Unexpected non boolean variable");
        }
      }

      else {
        /* unknow identifier. This code should be impossible since the
           type checker already checked the expr for being correct
        */
        print_sexp(nusmv_stderr, expr);
        rpterr("Unexpected data structure");
      }
    }
    break;

  case CAST_WORD1:
    {
      node_ptr a = car(expr);

      nusmv_assert(SymbType_is_boolean(
                           TypeChecker_get_expression_type(tc, a, context)));

      a = expr2bexpr_recur(enc, det_layer, a, context, in_next);

      if (node_get_type(a) == TRUEEXP) {
        res = find_node(NUMBER_UNSIGNED_WORD,
                        (node_ptr) WordNumber_from_integer(1,1), Nil);
      }
      else if (node_get_type(a) == FALSEEXP) {
        res = find_node(NUMBER_UNSIGNED_WORD,
                        (node_ptr) WordNumber_from_integer(0,1), Nil);
      }
      else res = node_word_create(a, 1);
      break;
    }

  case CAST_SIGNED:
    res = expr2bexpr_recur(enc, det_layer, car(expr), context, false);
    _CHECK_WORD(res);
    if (node_get_type(res) == NUMBER_UNSIGNED_WORD) {
      res = find_node(NUMBER_SIGNED_WORD, car(res), cdr(res));
    }
    break;

  case CAST_UNSIGNED:
    res = expr2bexpr_recur(enc, det_layer, car(expr), context, false);
    _CHECK_WORD(res);
    if (node_get_type(res) == NUMBER_SIGNED_WORD) {
      res = find_node(NUMBER_UNSIGNED_WORD, car(res), cdr(res));
    }
    break;

  case WRESIZE: {
    /* see note 0003136, issue #0001787 for full definition of the
       semantics of the word resize operator */
    int m;
    int n;

    SymbType_ptr orig_type = TypeChecker_get_expression_type(tc, car(expr),
                                                             context);
    node_ptr _size = CompileFlatten_resolve_number(symb_table, cdr(expr),
                                                   context);
    nusmv_assert(NULL != _size  && NUMBER == node_get_type(_size));
    m = SymbType_get_word_width(orig_type);
    n = node_get_int(_size); nusmv_assert(0 < n);

    if (n == m) {
      res = expr2bexpr_recur(enc, det_layer, car(expr), context, in_next);
    }
    else if (n < m) {
      if (SymbType_is_signed_word(orig_type)) {
        node_ptr msb_sel = \
          find_node(COLON,
                    find_node(NUMBER, NODE_FROM_INT(m-1), Nil),
                    find_node(NUMBER, NODE_FROM_INT(m-1), Nil));

        node_ptr rightmost_sel = \
          find_node(COLON,
                    find_node(NUMBER, NODE_FROM_INT(n-2), Nil),
                    find_node(NUMBER, NODE_FROM_INT(0), Nil));

        node_ptr nexpr =  \
          find_node(CONCATENATION,
                    find_node(BIT_SELECTION, car(expr), msb_sel),
                    find_node(BIT_SELECTION, car(expr), rightmost_sel));

        node_ptr cnexpr = \
          find_node(CAST_SIGNED, nexpr, Nil);

        res = expr2bexpr_recur(enc, det_layer, cnexpr, context, in_next);
      }/* signed words */
      else {
        node_ptr rightmost_sel = \
          find_node(COLON,
                    find_node(NUMBER, NODE_FROM_INT(n-1), Nil),
                    find_node(NUMBER, NODE_FROM_INT(0), Nil));

        node_ptr nexpr = find_node(BIT_SELECTION, car(expr), rightmost_sel);

        res = expr2bexpr_recur(enc, det_layer, nexpr, context, in_next);
      }/* unsigned */
    }
    else {
      /* word extension (handles both signed and unsigned) */
      node_ptr nexpr = find_node(EXTEND, car(expr),
                                 find_node(NUMBER, NODE_FROM_INT(n-m), Nil));

      res = expr2bexpr_recur(enc, det_layer, nexpr, context, in_next);
    }
    break;
  }

  case EXTEND:
    {
      SymbType_ptr ta;
      node_ptr a;

      node_ptr delta = CompileFlatten_resolve_number(symb_table,
                                                     cdr(expr),
                                                     context);
      nusmv_assert(Nil != delta && NUMBER == node_get_type(delta));

      ta = TypeChecker_get_expression_type(tc, car(expr), context);
      nusmv_assert(SymbType_is_word(ta));

      a = expr2bexpr_recur(enc, det_layer, car(expr), context, in_next);
      if (node_get_type(a) == NUMBER_UNSIGNED_WORD) {
        WordNumber_ptr wn = WORD_NUMBER(car(a));
        int times = node_get_int(delta);
        res = find_node(NUMBER_UNSIGNED_WORD,
                        (node_ptr) WordNumber_unsigned_extend(wn, times), Nil);
      }
      else if (node_get_type(a) == NUMBER_SIGNED_WORD) {
        WordNumber_ptr wn = WORD_NUMBER(car(a));
        int times = node_get_int(delta);
        res = find_node(NUMBER_SIGNED_WORD,
                        (node_ptr) WordNumber_signed_extend(wn, times), Nil);
      }
      else {
        /* not a constant */
        res = node_word_extend(a, delta, SymbType_is_signed_word(ta));
      }
      break;
    }

  case NEXT:
    /* Operator NEXT is not applied to the obtained exp because
       NEXT is passed as "true" in the last parameter in the fun below).*/
    nusmv_assert(!in_next);
    res = expr2bexpr_recur(enc, det_layer, car(expr), context, true);
    break;

  case LSHIFT:
  case RSHIFT:
    res = expr2bexpr_shift(enc, det_layer, expr, context, in_next);
    break;

  case LROTATE:
  case RROTATE:
    res = expr2bexpr_rotate(enc, det_layer, expr, context, in_next);
    break;

  case BIT_SELECTION:
    {
      SymbType_ptr ta;
      node_ptr a;
      node_ptr hbit;
      node_ptr lbit;

      ta = TypeChecker_get_expression_type(tc, car(expr), context);
      nusmv_assert(SymbType_is_word(ta));

      a = expr2bexpr_recur(enc, det_layer, car(expr), context, in_next);

      /* Resolve constant expressions. node_word_selection will assert
         that the range is numeric. */
      hbit = CompileFlatten_resolve_number(symb_table, car(cdr(expr)), context);
      lbit = CompileFlatten_resolve_number(symb_table, cdr(cdr(expr)), context);

      nusmv_assert(Nil != hbit && Nil != lbit &&
                   NUMBER == node_get_type(hbit) &&
                   NUMBER == node_get_type(lbit));

      /* this handles both constant and non-constant words */
      res = node_word_selection(a, find_node(COLON, hbit, lbit));
      break;
    }

  case EQDEF:
    {
      node_ptr lhs = car(expr);
      node_ptr rhs = cdr(expr);

      SymbType_ptr lhs_type = TypeChecker_get_expression_type(tc, lhs, context);
      SymbType_ptr rhs_type = TypeChecker_get_expression_type(tc, rhs, context);

      node_ptr var_name;

      switch (node_get_type(lhs)) {
      case SMALLINIT:
        var_name = car(lhs);
        break;

      case NEXT:
        var_name = car(lhs);
        break;

      default:
        var_name = lhs;
      }

      { /* Resolve the symbol */
        ResolveSymbol_ptr rs;
        rs = SymbTable_resolve_symbol(symb_table, var_name, context);
        var_name = ResolveSymbol_get_resolved_name(rs);
      }

      nusmv_assert(SymbTable_is_symbol_declared(symb_table, var_name));

      if (!SymbTable_is_symbol_var(symb_table, var_name)) {
        rpterr("Unexpected data structure, variable was expected");
      }

      if (SymbType_is_boolean(lhs_type) &&
          (SymbType_is_boolean(rhs_type) ||
           SYMB_TYPE_SET_INT == SymbType_get_tag(rhs_type))) {
        /* assignment is fully boolean (it is not bool := int) */

        if (SYMB_LAYER(NULL) == det_layer) {
          if (!SymbType_is_boolean(rhs_type)) {
            fprintf(nusmv_stderr, "Attempting to booleanize the formula: '");
            print_node(nusmv_stderr, expr);
            fprintf(nusmv_stderr, "'");
            internal_error("Unexpected non-deterministic formula:"      \
                           " determinization layer not specified");
          }
        }

        /* boolean variable, rhs will be determinized */
        if (node_get_type(lhs) == NEXT) {
          var_name = find_node(NEXT, var_name, Nil);
        }

        res = Expr_iff(var_name, expr2bexpr_recur(enc, det_layer, rhs, context,
                                                  in_next));
      }
      else { /* scalar variable */
        node_ptr fixed_expr;
        nusmv_assert(!in_next);

        /* take the original unflatten LHS and get rid of SMALLINIT */
        if (SMALLINIT == node_get_type(lhs)) lhs = car(lhs);
        fixed_expr = find_node(EQDEF, lhs, rhs);

        res = expr2bexpr_recur_binary(enc, det_layer, fixed_expr, context,
                                      in_next);
      }
      break;
    } /* end of case EQDEF */

  case CASE:
  case IFTHENELSE:
    res = expr2bexpr_ite(enc, det_layer, expr, context, in_next);
    break;

  case UNION:
  case SETIN:
    res = scalar_atom2bexpr(enc, det_layer, expr, context, in_next);
    break;

    /* -- unary operations which can be processed the standard way -- */
  case NOT:
  case UMINUS:
    res = expr2bexpr_recur_unary(enc, det_layer, expr, context, in_next);
    break;

    /* -- binary operations which can be processed the standard way -- */
  case CONCATENATION:
  case AND:
  case OR:
  case XOR:
  case XNOR:
  case IMPLIES:
  case IFF:
  case EQUAL:
  case NOTEQUAL:/* Predicates over possibly scalar terms (guaranteed to return boolean) */
  case PLUS:
  case MINUS:
  case LT:/* Predicates over scalar terms (guaranteed to return boolean) */
  case LE:
  case GT:
  case GE:
    /* Function symbols over scalar terms Might return boolean, but
       check is needed Assumption: if boolean is returned, then the
       function is determinized by introducing a variable on the {0,1}
       leaves. */
  case TIMES:
  case DIVIDE:
  case MOD:
    res = expr2bexpr_recur_binary(enc, det_layer, expr, context, in_next);
    break;

    /* -- end of standard processing -- */

    /* -- unary boolean expressions which do not require any special care */

  case EX:  /* UNARY CTL/LTL OPERATORS */
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

  case EBF: /* BOUNDED TEMPORAL OPERATORS (cdr is range, no recursion needed) */
  case ABF:
  case EBG:
  case ABG:
  case EBU:
  case ABU:
    res = find_node(node_get_type(expr),
                    expr2bexpr_recur(enc, det_layer, car(expr), context, in_next),
                    cdr(expr)); /* second operand (if any) is passed as it is */
    break;

    /* -- binary boolean expressions which do not require any special care */
  case CONS:
  case EU: /* BINARY CTL/LTL OPERATORS */
  case AU:
  case MINU:
  case MAXU:
  case UNTIL:
  case RELEASES:
  case SINCE:
  case TRIGGERED:
    res = find_node(node_get_type(expr),
                    expr2bexpr_recur(enc, det_layer, car(expr), context, in_next),
                    expr2bexpr_recur(enc, det_layer, cdr(expr), context, in_next));
    break;

  case SPEC:
  case LTLSPEC:
  case PSLSPEC:
  case INVARSPEC:
  case COMPUTE:
    res =
      expr2bexpr_recur(enc, det_layer, cdr(car(expr)), car(car(expr)), in_next);
    break;

  case CONTEXT:
    res = expr2bexpr_recur(enc, det_layer, cdr(expr), car(expr), in_next);
    break;

  case TWODOTS:
    rpterr("Unexpected TWODOTS node");
    res = (node_ptr) NULL;
    break;

  default:
    internal_error("expr2bexpr_recur: type = %d\n", node_get_type(expr));
    res = (node_ptr) NULL;
  }

  /* updates the results hash  */
  if (res != (node_ptr) NULL) {
    expr2bexpr_hash_insert_entry(expr, context, res, in_next,
                                 det_layer != SYMB_LAYER(NULL));
  }

  return res;
}


/**Function********************************************************************

  Synopsis [Converts an atomic expression into the corresponding
  (boolean) expression.]

  Description        [Takes an atomic expression and converts it into
  a corresponding boolean expression.

  The introduction of determinization variables is allowed only if the layer
  <tt>det_layer</tt> is not NULL]

  SideEffects        [A new boolean variable might be introduced.]

  SeeAlso            []

******************************************************************************/
static node_ptr scalar_atom2bexpr(BddEnc_ptr enc, SymbLayer_ptr det_layer,
                                  node_ptr expr, node_ptr context,
                                  boolean in_next)
{
  node_ptr res;
  int temp = yylineno;
  add_ptr bool_add = BddEnc_expr_to_add(enc, (in_next) ?
                                        find_node(NEXT, expr, Nil) : expr,
                                        context);

  yylineno = node_get_lineno(expr);
  res = BddEnc_add_to_expr(enc, bool_add, det_layer);
  add_free(BddEnc_get_dd_manager(enc), bool_add);
  yylineno = temp;

  return res;
}


/**Function********************************************************************

   Synopsis           [true if expression is booleanizable]

   Description        [Private service of compile_is_booleanizable.
                       To represent 'true' in cache we use the constant 2 for
                       'false' we use 1 to avoid representation problems wrt Nil]

   SideEffects        [cache can be updated]

******************************************************************************/
static boolean compile_is_booleanizable_aux(const SymbTable_ptr st,
                                            node_ptr expr,
                                            node_ptr context,
                                            boolean word_unbooleanizable,
                                            hash_ptr cache)
{
  node_ptr tmp;
  boolean res;
  if (Nil == expr) {
    return true;
  }

  tmp = find_assoc(cache, expr);
  if (Nil != tmp) {
    /* 2 means true */
    return (NODE_TO_INT(tmp) == 2);
  }

  if (CONTEXT == node_get_type(expr)) {
    return compile_is_booleanizable_aux(st, car(expr), cdr(expr),
                                        word_unbooleanizable, cache);
  }

  res = true;
  switch (node_get_type(expr)) {
  case NUMBER:
  case NUMBER_SIGNED_WORD:
  case NUMBER_UNSIGNED_WORD:
  case TRUEEXP:
  case FALSEEXP:
  case FAILURE:
    res = true;
    break;

  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    res = false;
    break;

  case NFUNCTION:
    res = false;
    break;

  case BIT:
    res = true;
    break;

  case ATOM:
  case DOT:
  case ARRAY:
    {
      ResolveSymbol_ptr rs = SymbTable_resolve_symbol(st, expr, context);
      node_ptr resolved = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_var(rs)) {
        SymbType_ptr t = SymbTable_get_var_type(st, resolved);

        /* Real, integers, strings.. */
        res = ! (SymbType_is_infinite_precision(t) ||
                 SymbType_is_string(t));

        if (res && SymbType_is_word(t)) {
          res = !word_unbooleanizable;
        }
      }
      else if (ResolveSymbol_is_define(rs)) {
        res =
          compile_is_booleanizable_aux(st,
                                       SymbTable_get_define_body(st,
                                                                 resolved),
                                       SymbTable_get_define_context(st,
                                                                    resolved),
                                       word_unbooleanizable, cache);
      }
      else if (ResolveSymbol_is_parameter(rs)) {
        res =
          compile_is_booleanizable_aux(st,
                   SymbTable_get_actual_parameter(st,
                                                  resolved),
                   SymbTable_get_actual_parameter_context(st,
                                                          resolved),
                                         word_unbooleanizable, cache);
      }
      else if (ResolveSymbol_is_function(rs)) {
        res = false;
      }
      else if (ResolveSymbol_is_constant(rs)) {
        TypeChecker_ptr tc = SymbTable_get_type_checker(st);
        SymbType_ptr ty = TypeChecker_get_expression_type(tc, resolved, Nil);
        /* Only real constants are unbooleanizable */
        res = !SymbType_is_real(ty);
      }
      else {
        rpterr("Unexpected symbol in Compile_is_expr_booleanizable.");
        error_unreachable_code();
      }
      break;
    }
  default:
    {
      /* Lazy evaluation */
      res = compile_is_booleanizable_aux(st,
                                         car(expr), context,
                                         word_unbooleanizable,
                                         cache);
      if (res) {
        /* Check the cdr iff car is booleanizable */
        res = compile_is_booleanizable_aux(st,
                                           cdr(expr), context,
                                           word_unbooleanizable,
                                           cache);
      }
    }
  }

  if (res) {
    /* 2 means true */
    insert_assoc(cache, expr, NODE_FROM_INT(2));
  }
  else {
    /* 1 means false */
    insert_assoc(cache, expr, NODE_FROM_INT(1));
  }

  return res;
}
