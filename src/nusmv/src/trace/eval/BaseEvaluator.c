/**CFile***********************************************************************

  FileName    [BaseEvaluator.c]

  PackageName [eval]

  Synopsis    [Implementation of class 'BaseEvaluator']

  Description []

  SeeAlso     [BaseEvaluator.h]

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``eval'' package of NuSMV version 2.
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

  Revision    [$Id: $]

******************************************************************************/

#include "BaseEvaluator.h"
#include "BaseEvaluator_private.h"

#include "compile/compile.h"
#include "parser/symbols.h"
#include "utils/utils.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'BaseEvaluator_private.h' for class 'BaseEvaluator' definition. */

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

static void base_evaluator_finalize ARGS((Object_ptr object, void* dummy));

static Expr_ptr base_evaluator_eval_recur ARGS((BaseEvaluator_ptr self,
                                                const Expr_ptr const_expr,
                                                boolean in_next));

static Expr_ptr base_evaluator_resolve_expr ARGS((BaseEvaluator_ptr self,
                                                  const Expr_ptr const_expr));

static node_ptr
base_evaluator_make_failure ARGS((const char* tmpl, node_ptr symbol));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BaseEvaluator class constructor]

  Description        [The BaseEvaluator class constructor]

  SideEffects        []

  SeeAlso            [BaseEvaluator_destroy]

******************************************************************************/
BaseEvaluator_ptr BaseEvaluator_create(void)
{
  BaseEvaluator_ptr self = ALLOC(BaseEvaluator, 1);
  BASE_EVALUATOR_CHECK_INSTANCE(self);

  base_evaluator_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [The BaseEvaluator class destructor]

  Description        [The BaseEvaluator class destructor]

  SideEffects        []

  SeeAlso            [BaseEvaluator_create]

******************************************************************************/
void BaseEvaluator_destroy(BaseEvaluator_ptr self)
{
  BASE_EVALUATOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}

/**Function********************************************************************

  Synopsis    [Initializes the evaluator with context information]

  Description [Initializes the evaluator with context
  information. This function must be called *before* invoking
  BaseEvaluator_evaluate in order to initialize the context of evaluation]

  SideEffects [The internal cache of the evaluator is cleared]

  SeeAlso     [BaseEvaluator_evaluate]

******************************************************************************/
void BaseEvaluator_set_context(BaseEvaluator_ptr self,
                               const SymbTable_ptr st, const hash_ptr env)
{
  BASE_EVALUATOR_CHECK_INSTANCE(self);

  if ((hash_ptr)(NULL) != self->cache) {
    free_assoc(self->cache);
  }
  self->cache = new_assoc();

  self->st = st;
  self->env = env;
}


/**Function********************************************************************

  Synopsis    [Evaluates given constant expression]

  Description [Evaluates a constant expression within context given
  using BaseEvaluator_set_context. Returns a constant which is the
  result of the evaluation of the expression. A FAILURE node is
  returned if result could not be computed (e.g. no assignment for an
  identifier could be found in the environment)]

  SideEffects []

  SeeAlso     [BaseEvaluator_set_context]

******************************************************************************/
Expr_ptr BaseEvaluator_evaluate(BaseEvaluator_ptr self,
                                const Expr_ptr const_expr)
{
  BASE_EVALUATOR_CHECK_INSTANCE(self);
  SYMB_TABLE_CHECK_INSTANCE(self->st);
  nusmv_assert((hash_ptr)(NULL) != self->env);

  return base_evaluator_eval_recur(self, const_expr, false);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BaseEvaluator class private initializer]

  Description        [The BaseEvaluator class private initializer]

  SideEffects        []

  SeeAlso            [BaseEvaluator_create]

******************************************************************************/
void base_evaluator_init(BaseEvaluator_ptr self)
{
  /* base class initialization */
  object_init(OBJECT(self));

  /* members initialization */
  self->cache = (hash_ptr)(NULL);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = base_evaluator_finalize;
  OVERRIDE(BaseEvaluator, resolve) = base_evaluator_resolve_expr;
}


/**Function********************************************************************

  Synopsis           [The BaseEvaluator class private deinitializer]

  Description        [The BaseEvaluator class private deinitializer]

  SideEffects        []

  SeeAlso            [BaseEvaluator_destroy]

******************************************************************************/
void base_evaluator_deinit(BaseEvaluator_ptr self)
{
  /* members deinitialization */
  if ((hash_ptr)(NULL) != self->cache) { free_assoc(self->cache); }

  /* base class deinitialization */
  object_deinit(OBJECT(self));
}


/**Function********************************************************************

  Synopsis           [Const expr virtual resolution method]

  Description        [This is intended to be overridden in order to support
  extra types]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Expr_ptr base_evaluator_resolve(const BaseEvaluator_ptr self,
                                const Expr_ptr const_expr)
{
  return (*self->resolve)(self, const_expr);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [This function is a private service of BaseEvaluator_eval]

  Description [This function is a private service of BaseEvaluator_eval]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static Expr_ptr base_evaluator_eval_recur(BaseEvaluator_ptr self,
                                          const Expr_ptr expr,
                                          boolean in_next)
{
  Expr_ptr res = Nil;

  const SymbTable_ptr st = \
    self->st;

  const hash_ptr env = \
    self->env;

  /* corner cases: they can all be handled together */
  if ((Nil == expr)  || FAILURE == node_get_type(expr) ||
      SymbTable_is_symbol_constant(st, expr) ||
      node_is_leaf(expr)) {

    return expr;
  }

  /* key for result memoization */
  const node_ptr key = in_next ? find_node(NEXT, expr, Nil) : expr;

  /* if a memoized result is available, return */
  res = find_assoc(self->cache, key);
  if (Nil != res) return res;

  switch(node_get_type(expr)) {

  case ARRAY:
  case ATOM:
  case DOT:
    {
      res = find_assoc(env, key);
      if (Nil == res) {
        const char* fail_msg = "Uknown symbol: '%s'";
        res = base_evaluator_make_failure(fail_msg, key);
      }
      break;
    } /* array, atom, dot */

    /** unary ops */
  case NOT:
  case UMINUS:
    {
      Expr_ptr tmp;
      node_ptr left_expr = car(expr);

      /* handle failure conditions */
      left_expr = base_evaluator_eval_recur(self, left_expr, in_next);
      if (FAILURE == node_get_type(left_expr)) return left_expr;

      tmp = find_node(node_get_type(expr), left_expr, Nil);
      res = find_assoc(self->cache, tmp);
      if (Nil == res) { res = base_evaluator_resolve(self, tmp); }
      break;
    }

  case NEXT:
    {
      nusmv_assert(!in_next); /* no nested NEXTs allowed */
      return base_evaluator_eval_recur(self, car(expr), true);
    }

    /** binary ops */
  case LT: case GT: case LE: case GE:
  case NOTEQUAL: case EQUAL: case EQDEF:
  case CONS: case AND: case OR: case XOR:
  case XNOR: case IFF: case IMPLIES: case PLUS:
  case MINUS: case TIMES: case DIVIDE: case MOD:
  case LSHIFT: case RSHIFT: case UNION: case SETIN:

    /** word bin ops */
  case COLON: /* probably not needed, but kept for safety */

  case CONCATENATION: case EXTEND: case LROTATE: case RROTATE:
    {
      Expr_ptr tmp;
      node_ptr left_expr = car(expr);
      node_ptr right_expr = cdr(expr);

      /* handle failure conditions */
      if (Nil != left_expr) {
        left_expr = base_evaluator_eval_recur(self, left_expr, in_next);
        if (FAILURE == node_get_type(left_expr)) return left_expr;
      }

      if (Nil != right_expr) {
        right_expr = base_evaluator_eval_recur(self, right_expr, in_next);
        if (FAILURE == node_get_type(right_expr)) return right_expr;
      }

      tmp = find_node(node_get_type(expr), left_expr, right_expr);
      res = find_assoc(self->cache, tmp);

      if (SETIN == node_get_type(expr)) {
        /* evaluate SETIN predicates semantically, by building set representation
           for left and right and verifying that left is contained into right */
        Set_t left = Set_MakeFromUnion(left_expr);
        Set_t right = Set_MakeFromUnion(right_expr);

        res = Set_Contains(right, left) ? Expr_true() : Expr_false();
        Set_ReleaseSet(left);
        Set_ReleaseSet(right);
      }

      if (Nil == res) { res = base_evaluator_resolve(self, tmp); }
      break;
     } /* binary ops */

  case BIT_SELECTION:
    {
      Expr_ptr m, M, tmp;
      Expr_ptr w = car(expr);
      Expr_ptr bits = cdr(expr);

      nusmv_assert(COLON == node_get_type(bits));

      m = car(bits);
      M = cdr(bits);

      /* To avoid recursing on the COLON of the BIT selection */
      if (Nil != m) {
        m = base_evaluator_eval_recur(self, car(bits), in_next);
        if (FAILURE == node_get_type(m)) return m;
      }
      if (Nil != M) {
        M = base_evaluator_eval_recur(self, cdr(bits), in_next);
        if (FAILURE == node_get_type(M)) return M;
      }
      if (Nil != w) {
        w = base_evaluator_eval_recur(self, w, in_next);
        if (FAILURE == node_get_type(w)) return w;
      }
      tmp = find_node(node_get_type(expr), w, find_node(COLON, m, M));

      res = find_assoc(self->cache, tmp);

      if (Nil == res) { res = base_evaluator_resolve(self, tmp); }
      break;
    }

  case WRESIZE:
    {
      Expr_ptr tmp;
      Expr_ptr wexpr = car(expr);
      Expr_ptr wsize = cdr(expr);

      /* handle failure conditions */
      if (Nil != wexpr) {
        wexpr = base_evaluator_eval_recur(self, wexpr, in_next);
        if (FAILURE == node_get_type(wexpr)) return wexpr;
      }
      if (Nil != wsize) {
        wsize = base_evaluator_eval_recur(self, wsize, in_next);
        if (FAILURE == node_get_type(wsize)) return wsize;
      }

      tmp = find_node(node_get_type(expr), wexpr, wsize);
      res = find_assoc(self->cache, tmp);

      if (Nil == res) { res = base_evaluator_resolve(self, tmp); }
      break;
    }

  case CAST_SIGNED:
  case CAST_UNSIGNED:
  case CAST_WORD1:
    {
      Expr_ptr tmp;
      Expr_ptr e = car(expr);

      /* handle failure conditions */
      if (Nil != e) {
        e = base_evaluator_eval_recur(self, e, in_next);
        if (FAILURE == node_get_type(e)) return e;
      }

      tmp = find_node(node_get_type(expr), e, Nil);
      res = find_assoc(self->cache, tmp);

      if (Nil == res) { res = base_evaluator_resolve(self, tmp); }
      break;
    }

  case CAST_BOOL:
    {
      Expr_ptr tmp;
      Expr_ptr w = car(expr);

      /* handle failure conditions */
      if (Nil != w) {
        w = base_evaluator_eval_recur(self, w, in_next);
        if (FAILURE == node_get_type(w)) return w;
      }

      tmp = find_node(node_get_type(expr), w, Nil);
      res = find_assoc(self->cache, tmp);

      if (Nil == res) { res = base_evaluator_resolve(self, tmp); }
      break;
    }

    /** ternary ops */
  case IFTHENELSE:
  case CASE:
    {
      Expr_ptr cond_expr = caar(expr);
      Expr_ptr cond_value = base_evaluator_eval_recur(self, cond_expr, in_next);
      if (FAILURE == node_get_type(cond_value)) {
        return cond_value;
      }

      /* condition is a predicate */
      nusmv_assert (Expr_is_true(cond_value) || Expr_is_false(cond_value));
      if (Expr_is_true(cond_value)) {
        res = base_evaluator_eval_recur(self, cdar(expr), in_next);
      }
      else {
        res = base_evaluator_eval_recur(self, cdr(expr), in_next);
      }
      break;
    }

  case FAILURE:
    return expr;

  default: internal_error("%s:%d:%s Unsupported node type (%d)",
                          __FILE__, __LINE__, __func__,
                          node_get_type(expr));
  } /* of switch */

  /* memoize results */
  insert_assoc(self->cache, key, res);
  return res;
}


/**Function********************************************************************

  Synopsis    [The BaseEvaluator class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void base_evaluator_finalize(Object_ptr object, void* dummy)
{
  BaseEvaluator_ptr self = BASE_EVALUATOR(object);

  base_evaluator_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static Expr_ptr base_evaluator_resolve_expr (BaseEvaluator_ptr self,
                                             const Expr_ptr const_expr)
{
  nusmv_assert(Nil != const_expr);
  return Expr_resolve(self->st, node_get_type(const_expr),
                      car(const_expr), cdr(const_expr));
}


/**Function********************************************************************

  Synopsis    [This function is a private service of BaseEvaluator_eval]

  Description [This function is a private service of BaseEvaluator_eval]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static node_ptr
base_evaluator_make_failure(const char* tmpl, node_ptr symbol)
{
  char *symb_str = sprint_node(symbol);
  char *buf = ALLOC(char, 1 + strlen(tmpl) + strlen(symb_str));
  node_ptr res;

  sprintf(buf, tmpl, symb_str);
  res = failure_make(buf, FAILURE_UNSPECIFIED, -1);

  FREE(buf);
  FREE(symb_str);

  return res;
}

/**AutomaticEnd***************************************************************/

