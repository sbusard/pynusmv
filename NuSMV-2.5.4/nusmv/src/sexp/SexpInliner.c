/**CFile*****************************************************************

  FileName    [SexpInliner.c]

  PackageName [fsm.sexp]

  Synopsis    [The SexpInliner implementation]

  Description []

  SeeAlso     [SexpInliner.h]

  Author      [Roberto Cavada, Marco Roveri]

  Copyright   [
  This file is part of the ``fsm.sexp'' package of NuSMV version 2.
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

#include "SexpInliner.h"

#include "compile/compile.h"
#include "compile/PredicateNormaliser.h"
#include "compile/symb_table/ResolveSymbol.h"

#include "parser/symbols.h"
#include "opt/opt.h"
#include "utils/error.h"
#include "utils/assoc.h"



static char rcsid[] UTIL_UNUSED = "$Id: SexpInliner.c,v 1.1.2.41 2010-01-25 19:43:34 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constants declarations                                                    */
/*---------------------------------------------------------------------------*/
static const int _UNTIMED_CURRENT = -2;
static const int _UNTIMED_NEXT = -1;

/* Uncomment the following line to enable debugging of the
   substitutions */
/* #derfine _DEBUG_SUBST */

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/**Type************************************************************************

  Synopsis    [Sexp Inliner]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct SexpInliner_TAG {

  SymbTable_ptr st;
  hash_ptr var2expr;     /* substitution hash var -> expr (forced) */
  hash_ptr var2invar;    /* substitution hash var -> invar (forced) */
  Set_t invars;          /* the set of forced invariants */
  size_t fixpoint_limit; /* limit for fixpoint computation in subst */
  Set_t blacklist;       /* name of symbols not to be learn nor substituted */

  hash_ptr hash_timed2leaves; /* hash used in memoization when
                                 bringing timed nodes to leaves */
  hash_ptr hash_subst; /* hash used in memoization of substitution */

  /* [AT] Maybe 2 above hashes are enough to memoization but I do know
     the exact semantics and as result introduced 2 new hash to two
     functions */
  hash_ptr hash_extract_equals_invars; /* hash to memoize the results of
                                          sexp_inliner_extract_equals_invars */
  hash_ptr hash_is_expr_deterministic; /* hash to memoize the results of
                     sexp_inliner_is_expr_deterministic */
} SexpInliner;


/**Type************************************************************************

  Synopsis    [Result structure for inliner]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct InlineRes_TAG {
  Expr_ptr orig;
  Expr_ptr inlined;
  Set_t equivs;
  Set_t invars;

} InlineRes;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define _PRINT(txt)                             \
  fprintf(nusmv_stderr, "%s", txt);             \
  fflush(nusmv_stderr)


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE* nusmv_stderr;
extern FILE* nusmv_stdout;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void sexp_inliner_init ARGS((SexpInliner_ptr self,
                                    SymbTable_ptr st,
                                    const size_t fixpoint_limit));

static void sexp_inliner_deinit ARGS((SexpInliner_ptr self));

static void sexp_inliner_copy ARGS((const SexpInliner_ptr self,
                                    SexpInliner_ptr copy));

static node_ptr
sexp_inliner_move_time_to_leaves ARGS((const SexpInliner_ptr self,
                                       node_ptr expr, int time));

static node_ptr
sexp_inliner_extract_equals_invars ARGS((const SexpInliner_ptr self,
                                         node_ptr expr,
                                         boolean is_neg,
                                         Set_t* equals, Set_t* invars,
                                         Set_t* vars,
                                         boolean* changed));

static node_ptr
sexp_try_acquiring_equality ARGS((const SexpInliner_ptr self,
                                  node_ptr equal, boolean is_neg,
                                  Set_t* set, Set_t* vars, boolean* changed));

static array_t*
sexp_inliner_extract_candidate_equals ARGS((const SexpInliner_ptr self,
                                            const Set_t equals,
                                            const Set_t imp_vars,
                                            const hash_ptr var2invar,
                                            Set_t* rem_equals));

static hash_ptr
sexp_inliner_remove_loops ARGS((const SexpInliner_ptr self,
                                array_t* good_equals,
                                hash_ptr hash_invars,
                                Set_t* good, Set_t* rem));

static node_ptr
sexp_inliner_substitute ARGS((SexpInliner_ptr self, node_ptr expr,
                              hash_ptr var2expr, hash_ptr var2invar,
                              boolean* changed));

static boolean
sexp_inliner_force_equivalence ARGS((SexpInliner_ptr self,
                                     node_ptr var, Expr_ptr expr));

static boolean
sexp_inliner_force_invariant ARGS((SexpInliner_ptr self,
                                   node_ptr var, Expr_ptr expr));

static boolean
sexp_inliner_expr_is_var ARGS((const SexpInliner_ptr self, node_ptr expr));

static InlineRes_ptr inline_res_create ARGS((Expr_ptr orig));
static void inline_res_deinit ARGS((InlineRes_ptr self));

static int sexp_inliner_expr_ptr_compare
ARGS((const void * c1, const void * c2));

static void sexp_inliner_free_equalities_array ARGS((array_t* arr));

static boolean
sexp_inliner_is_expr_deterministic ARGS((const SexpInliner_ptr self,
                                         node_ptr expr));

static boolean sexp_inliner_is_expr_timed ARGS((node_ptr expr));

static Expr_ptr sexp_inliner_assign_to_setin ARGS((SexpInliner_ptr self,
                                                   Expr_ptr assign));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Sexp Inliner constructor]

  Description        [fixpoint_limit is a integer bound controlling the
                      maximum number of iteration to be carried out
                      when inlining an expression. Use 0 (zero) for
                      no limit.]

  SideEffects        []

******************************************************************************/
SexpInliner_ptr SexpInliner_create(SymbTable_ptr st,
                                   const size_t fixpoint_limit)
{
  SexpInliner_ptr self;

  /* allocation: */
  self = ALLOC(SexpInliner, 1);
  SEXP_INLINER_CHECK_INSTANCE(self);

  /* initialization: */
  sexp_inliner_init(self, st, fixpoint_limit);
  return self;
}


/**Function********************************************************************

  Synopsis           [Copy costructor]

  Description        []

  SideEffects        []

******************************************************************************/
SexpInliner_ptr SexpInliner_copy(const SexpInliner_ptr self)
{
  SexpInliner_ptr copy;

  SEXP_INLINER_CHECK_INSTANCE(self);

  copy = ALLOC(SexpInliner, 1);
  SEXP_INLINER_CHECK_INSTANCE(copy);

  sexp_inliner_copy(self, copy);
  return copy;
}


/**Function********************************************************************

  Synopsis           [Destructor]

  Description        []

  SideEffects        []

******************************************************************************/
void SexpInliner_destroy(SexpInliner_ptr self)
{
  SEXP_INLINER_CHECK_INSTANCE(self);

  sexp_inliner_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Clears the internal cache of forced equivalences]

  Description        []

  SideEffects        []

******************************************************************************/
void SexpInliner_clear_equivalences(SexpInliner_ptr self)
{
  SEXP_INLINER_CHECK_INSTANCE(self);
  clear_assoc(self->var2expr);
  clear_assoc(self->hash_subst);
}


/**Function********************************************************************

  Synopsis           [Clears the internal cache of forced invariants]

  Description        []

  SideEffects        []

******************************************************************************/
void SexpInliner_clear_invariants(SexpInliner_ptr self)
{
  SEXP_INLINER_CHECK_INSTANCE(self);
  clear_assoc(self->var2invar);

  Set_ReleaseSet(self->invars);
  self->invars = Set_MakeEmpty();
  clear_assoc(self->hash_subst);
}


/**Function********************************************************************

  Synopsis           [Clears the internal set of blacklisted names.]

  Description        []

  SideEffects        []

******************************************************************************/
void SexpInliner_clear_blacklist(SexpInliner_ptr self)
{
  SEXP_INLINER_CHECK_INSTANCE(self);

  Set_ReleaseSet(self->blacklist);
  self->blacklist = Set_MakeEmpty();
  clear_assoc(self->hash_subst);
}


/**Function********************************************************************

  Synopsis           [Returns the symbol table that is connected to the
                      BoolEnc instance connected to self]

  Description        []

  SideEffects        []

******************************************************************************/
SymbTable_ptr SexpInliner_get_symb_table(const SexpInliner_ptr self)
{
  SEXP_INLINER_CHECK_INSTANCE(self);
  return self->st;
}

/**Function********************************************************************

  Synopsis           [Get the internal var2expr hash]

  Description [Get the internal var2expr hash. Do not perform any
  side-effects on this hash]

  SideEffects        []

******************************************************************************/
hash_ptr SexpInliner_get_var2expr_hash(const SexpInliner_ptr self)
{
  SEXP_INLINER_CHECK_INSTANCE(self);
  return self->var2expr;
}

/**Function********************************************************************

  Synopsis           [Get the internal var2invar hash]

  Description [Get the internal var2invar hash. Do not perform any
  side-effects on this hash]

  SideEffects        []

******************************************************************************/
hash_ptr SexpInliner_get_var2invar_hash(const SexpInliner_ptr self)
{
  SEXP_INLINER_CHECK_INSTANCE(self);
  return self->var2invar;
}


/**Function********************************************************************

  Synopsis           [Forces to learn that var (can be timed) and expr are
                      equivalent. The expression is assumed to be
                      already flattened, and defines expanded.]

  Description        [The equivalence is learnt even if given name is
                      blacklisted. The equivalence substitutes any
                      previously forced equivalence about the same
                      variable. Returns true if the equivalence was
                      accepted, or false otherwise.]

  SideEffects        []

******************************************************************************/
boolean SexpInliner_force_equivalence(SexpInliner_ptr self,
                                      node_ptr var, Expr_ptr expr)
{
  node_ptr expr2;
  boolean res;

  SEXP_INLINER_CHECK_INSTANCE(self);

  expr2 = sexp_inliner_move_time_to_leaves(self, expr, _UNTIMED_CURRENT);
  res = sexp_inliner_force_equivalence(self, var, expr2);
  if (res) clear_assoc(self->hash_subst);
  return res;
}


/**Function********************************************************************

  Synopsis           [Forces to learn all equivalences in given set]

  Description        [There is an implicit assumption about the format of
                      each element in the set. It must be either a
                      EQUAL, a EQDEF or a IFF node where left
                      operand is a variable. This method may be
                      useful to force equivalences previously
                      returned by a InlineRes instance.

                      Returns true if any equivalence was accepted,
                      false if all were rejected]

  SideEffects        []

******************************************************************************/
boolean SexpInliner_force_equivalences(SexpInliner_ptr self, Set_t equivs)
{
  boolean res = false;
  Set_Iterator_t iter;

  SEXP_INLINER_CHECK_INSTANCE(self);

  SET_FOREACH(equivs, iter) {
    node_ptr eq = Set_GetMember(equivs, iter);
    nusmv_assert(EQUAL == node_get_type(eq) || IFF == node_get_type(eq) ||
                 EQDEF == node_get_type(eq));

    res |= SexpInliner_force_equivalence(self, car(eq), cdr(eq));
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Forces to learn that var and expr are
                      invariantly equivalent]

  Description        [var must be a flat variable name (not nexted not
                      timed). The expression is assumed to be
                      already flattened.
                      The invariant is learnt even if given name is
                      blacklisted. The invariant substitutes any
                      previously forced invariant about the same
                      variable.

                      Returns true if the invariant was successfully
                      forced, or false otherwise.]

  SideEffects        []

******************************************************************************/
boolean SexpInliner_force_invariant(SexpInliner_ptr self,
                                    node_ptr var, Expr_ptr expr)
{
  boolean res;
  node_ptr expr2;
  SEXP_INLINER_CHECK_INSTANCE(self);

  nusmv_assert(SymbTable_is_symbol_var(self->st, var));

  expr2 = sexp_inliner_move_time_to_leaves(self, expr, _UNTIMED_CURRENT);
  res = sexp_inliner_force_invariant(self, var, expr2);
  if (res) clear_assoc(self->hash_subst);
  return res;
}


/**Function********************************************************************

  Synopsis           [Forces to learn all invariants in given set]

  Description        [There is an implicit assumption about the format of
                      each element in the set. It must be either a
                      EQUAL, a EQDEF, or a IFF node where left
                      operand is a variable. This method may be
                      useful to force invariants previously
                      returned by a InlineRes instance.

                      Returns true if any invariant was accepted,
                      false if all were rejected]

  SideEffects        []

******************************************************************************/
boolean SexpInliner_force_invariants(SexpInliner_ptr self, Set_t invars)
{
  boolean res = false;
  Set_Iterator_t iter;

  SEXP_INLINER_CHECK_INSTANCE(self);

  SET_FOREACH(invars, iter) {
    node_ptr invar = Set_GetMember(invars, iter);
    nusmv_assert( EQUAL == node_get_type(invar) ||
                  IFF == node_get_type(invar) ||
                  /* an invariant assignment */
                  (EQDEF == node_get_type(invar) &&
                   SMALLINIT != node_get_type(car(invar)) &&
                   NEXT != node_get_type(car(invar))) );

    res |= SexpInliner_force_invariant(self, car(invar), cdr(invar));
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Adds to the blacklist the given name]

  Description        [Any name occurring in the blacklist will be not
                      substituted. Use to avoid inlining a set of variables.]

  SideEffects        []

******************************************************************************/
void SexpInliner_blacklist_name(SexpInliner_ptr self, node_ptr var)
{
  SEXP_INLINER_CHECK_INSTANCE(self);
  nusmv_assert(SymbTable_is_symbol_var(self->st, var));

  self->blacklist = Set_AddMember(self->blacklist, (Set_Element_t) var);
  clear_assoc(self->hash_subst);
}


/**Function********************************************************************

  Synopsis      [Performs inlining of given expression]

  Description   [Applies inlining to the given expression, with fixpoint.

                 Returned InlineRes object contains the
                 result. Returned instance must be destroyed by the
                 caller. If given variable changed is not NULL, it
                 will be set to true if any inlining has been
                 applied, or will be set to false if no inlining
                 has been applied.

                 Before carrying out the actual inlining, this
                 method learn automatically equivalences out of the
                 given formula.

                 WARNING: The expression is assumed to be already
                 flattened, and normalized (all nodes created with
                 find_node)]

  SideEffects   []

******************************************************************************/
InlineRes_ptr SexpInliner_inline(SexpInliner_ptr self, Expr_ptr expr,
                                 boolean* changed)
{
  size_t counter;
  node_ptr psi;
  Set_t kept_equals;
  boolean _changed; /* local changed */
  Set_t extracted_invars = (Set_t) NULL;

  SEXP_INLINER_CHECK_INSTANCE(self);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
    _PRINT("SexpInliner: Applying inlining...\n");
  }

  /* 0. resets change variable */
  if ((boolean*) NULL != changed) *changed = false;

  /* 1. moves timed nodes to the leaves */
  psi = sexp_inliner_move_time_to_leaves(self, expr, _UNTIMED_CURRENT);

  /* these two are accumulated out of psi */
  kept_equals = Set_MakeEmpty();
  extracted_invars = Set_MakeEmpty();

  /* enters the fixpoint loop */
  counter = 0;
  do {
    array_t* good_equals;
    hash_ptr hash_equals, hash_invars;
    Set_t rem_equals, equals, vars;

    /* 10. extracts equals and set of involved vars incrementally */
    equals = Set_MakeEmpty();
    vars = Set_MakeEmpty();
    psi = sexp_inliner_extract_equals_invars(self, psi, false,
                                             &equals, &extracted_invars,
                                             &vars, changed);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 7)) {
      fprintf(nusmv_stderr, "\nSexpInliner: before inlining (before re-introduction):\n");
      print_node(nusmv_stderr, psi);
    }

    /* this is the set of equivalences to be re-introduced in psi later */
    rem_equals = Set_MakeEmpty();

    { /* 12. extract hash out of auto-extracted invars set */
      Set_Iterator_t invars_iter;
      hash_invars = new_assoc();
      SET_FOREACH(extracted_invars, invars_iter) {
        node_ptr invar = (node_ptr) Set_GetMember(extracted_invars, invars_iter);
        /* extracted invars are always assign */
        nusmv_assert(EQDEF == node_get_type(invar));

        /* multiple invariants for the same var */
        if ((node_ptr) NULL == find_assoc(hash_invars, car(invar))) {
          /* there is no existing extracted invariant about this
             variable (this is always true with well-formed
             expressions coming from the compiler, but not true in
             general with hand-made formulae) */
          insert_assoc(hash_invars, car(invar), cdr(invar));
        }
        else {
          /* a multiple invariant assignment for the same
             variable. This is not a well-formed expression, but it
             may happen in hand-made expressions. In this case the
             expression is kept in psi, transformed to get inlined
             later. */

          rem_equals = Set_AddMember(rem_equals,
                                     sexp_inliner_assign_to_setin(self, invar));
        }
      }
    }

    /* 14. extracts unique equals sorted array and remaining set */
    good_equals = sexp_inliner_extract_candidate_equals(self, equals, vars,
                                                        hash_invars,
                                                        &rem_equals);

    /* 16. removes loops from good_equals, obtaining kep_equals and
       possibly growing rem_equals with equals causing loops */
    hash_equals = sexp_inliner_remove_loops(self, good_equals, hash_invars,
                                            &kept_equals, &rem_equals);

    /* prints out the remaining set */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 6)) {
      Set_Iterator_t iter;
      fprintf(nusmv_stderr, "\nSexpInliner: re-introduced equals are:\n");
      SET_FOREACH(rem_equals, iter) {
        print_node(nusmv_stderr, (node_ptr) Set_GetMember(rem_equals, iter));
        fprintf(nusmv_stderr, "\n");
      }
    }

    { /* accumulates the remaining equalitites and conjuncts it to
         psi, as they may reveal new equalities. For example if
         rem_equals contained v1 <-> (v2 -> v3 = t2) and for some
         reason this got simplified to v3 = t2 */
      Set_Iterator_t iter;
      SET_FOREACH(rem_equals, iter) {
        psi = Expr_and(psi, (node_ptr) Set_GetMember(rem_equals, iter));
      }
    }

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 7)) {
      fprintf(nusmv_stderr, "\nSexpInliner: before inlining (after re-introduction):\n");
      print_node(nusmv_stderr, psi);
    }

    /* 20. substitutes the assignments into the expression */
    _changed=false;
    psi = sexp_inliner_substitute(self, psi, hash_equals, hash_invars,
                                  &_changed);

    /* marks if changed */
    if (_changed && (boolean*) NULL != changed) *changed = _changed;

    /* 22. gets rid of all no-longer useful things */
    Set_ReleaseSet(rem_equals);
    free_assoc(hash_invars);
    free_assoc(hash_equals);
    sexp_inliner_free_equalities_array(good_equals);
    array_free(good_equals);
    Set_ReleaseSet(vars);
    Set_ReleaseSet(equals);

    /* 24. increments the counter if needed */
    if (self->fixpoint_limit > 0) counter += 1;

  /* Handling of the end of fixpoint loop. */
  } while (_changed && counter <= self->fixpoint_limit);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 7)) {
    Set_Iterator_t iter;

    fprintf(nusmv_stderr, "\nSexpInliner: Inlined expression was:\n");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "\nSexpInliner: After inlining is:\n");
    print_node(nusmv_stderr, psi);
    fprintf(nusmv_stderr, "\nSexpInliner: Kept equalities are:\n");
    SET_FOREACH(kept_equals, iter) {
      print_node(nusmv_stderr, (node_ptr) Set_GetMember(kept_equals, iter));
      fprintf(nusmv_stderr, "\n");
    }
    fprintf(nusmv_stderr, "\nSexpInliner: Extracted invariants are:\n");
    SET_FOREACH(extracted_invars, iter) {
      print_node(nusmv_stderr,
                 (node_ptr) Set_GetMember(extracted_invars, iter));
      fprintf(nusmv_stderr, "\n");
    }
  }

  { /* constructs the result */
    InlineRes_ptr res = inline_res_create(expr);

    /* the inlined expression */
    res->inlined = psi;

    /* retrieves the equivalences */
    res->equivs = Set_Union(res->equivs, kept_equals);

    /* retrieves the invariants */
    res->invars = Set_Union(Set_Union(res->invars, extracted_invars),
                            self->invars);

    Set_ReleaseSet(extracted_invars);
    Set_ReleaseSet(kept_equals);
    return res;
  }
}


/**Function********************************************************************

  Synopsis      [Performs inlining of given expression]

  Description [Applies inlining to the given expression, with
  fixpoint, and returns the result expression.


  If given variable changed is not NULL, it will be set to true if
  any inlining has been applied, or will be set to false if no
  inlining has been applied.

  Before carrying out the actual inlining, this method learn
  automatically equivalences out of the given formula.

  WARNING: The expression is assumed to be already flattened, and
  normalized (all nodes created with find_node)]

  SideEffects   [SexpInliner_inline]

******************************************************************************/
Expr_ptr SexpInliner_inline_no_learning(SexpInliner_ptr self, Expr_ptr expr,
                                        boolean* changed)
{
  size_t counter;
  Expr_ptr psi;
  hash_ptr hash_equals;
  boolean _changed;

  SEXP_INLINER_CHECK_INSTANCE(self);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
    _PRINT("\nSexpInliner: Applying inlining (no learning)...\n");
  }

  /* 0. resets change variable */
  if ((boolean*) NULL != changed) *changed = false;

  /* 1. moves timed nodes to the leaves */
  psi = sexp_inliner_move_time_to_leaves(self, expr, _UNTIMED_CURRENT);

  /* this is a dummy, will be not used by the inliner, as only
     forced equivalences and invariants will be used */
  hash_equals = new_assoc();

  /* 3. enters the fixpoint loop */
  counter = 0;
  do {
    /* this controls the exit from the loop */
    _changed = false;

    /* performs the smooth inlining */
    psi = sexp_inliner_substitute(self, psi,
                                  hash_equals,
                                  (hash_ptr) NULL /* no extracted invars */,
                                  &_changed);

    /* marks if changed */
    if (_changed && (boolean*) NULL != changed) *changed = _changed;

    /* increments the counter if needed */
    if (self->fixpoint_limit > 0) counter += 1;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 6)) {
      _PRINT("SexpInliner: Done inlining iteration...\n");
    }

  /* Handling of the end of fixpoint loop. */
  } while (_changed && counter <= self->fixpoint_limit);

  free_assoc(hash_equals);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 7)) {
    fprintf(nusmv_stderr, "\nSexpInliner: Inlined expression was:\n");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "\nSexpInliner: After inlining is:\n");
    print_node(nusmv_stderr, psi);
    fprintf(nusmv_stderr, "\n");
  }

  return psi;
}



/**Function********************************************************************

  Synopsis      [Class destroyer]

  Description   []

  SideEffects   []

******************************************************************************/
void InlineRes_destroy(InlineRes_ptr self)
{
  INLINE_RES_CHECK_INSTANCE(self);
  inline_res_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis      [Composes the whole result, making the conjuction of the
                 inlined expression, equivalences and invariants]

  Description   []

  SideEffects   []

******************************************************************************/
Expr_ptr InlineRes_get_result(const InlineRes_ptr self)
{
  INLINE_RES_CHECK_INSTANCE(self);
  return Expr_and(InlineRes_get_inlined_expr(self),
                  Expr_and(InlineRes_get_equivalences_expr(self),
                           InlineRes_get_invariant_expr(self)));
}


/**Function********************************************************************

  Synopsis      [Composes the whole result, making the conjuction of the
                 inlined expression, equivalences and invariants]

  Description   [The equivalences and the invariants are sorted before being
  conjuncted to the inlined expression, to return a unique expression.]

  SideEffects   []

******************************************************************************/
Expr_ptr InlineRes_get_result_unique(const InlineRes_ptr self)
{
  array_t* arr;
  node_ptr iter;
  Expr_ptr sorted_expr;

  INLINE_RES_CHECK_INSTANCE(self);

  arr = array_alloc(Expr_ptr, 8);

  /* accumulates equivalences */
  for (iter=InlineRes_get_equivalences_expr(self);
       (node_ptr) NULL != iter; iter=cdr(iter)) {
    if (AND != node_get_type(iter)) {
      array_insert_last(Expr_ptr, arr, iter);
      break;
    }
    else array_insert_last(Expr_ptr, arr, car(iter));
  }

  /* accumulates invariants */
  for (iter=InlineRes_get_invariant_expr(self);
       (node_ptr) NULL != iter; iter=cdr(iter)) {
    if (AND != node_get_type(iter)) {
      array_insert_last(Expr_ptr, arr, iter);
      break;
    }
    else array_insert_last(Expr_ptr, arr, car(iter));
  }

  sorted_expr = Expr_true();

  { /* sorts and produces a unique expression */
    Expr_ptr conj;
    int j;
    array_sort(arr, sexp_inliner_expr_ptr_compare);
    arrayForEachItem(Expr_ptr, arr, j, conj) {
      sorted_expr = Expr_and_nil(sorted_expr, conj);
    }
  }
  array_free(arr);


  return Expr_and(InlineRes_get_inlined_expr(self), sorted_expr);
}


/**Function********************************************************************

  Synopsis      [Returns the original expression which has been inlined]

  Description   []

  SideEffects   []

******************************************************************************/
Expr_ptr InlineRes_get_original_expr(const InlineRes_ptr self)
{
  INLINE_RES_CHECK_INSTANCE(self);
  return ((node_ptr) NULL != self->orig) ? self->orig : Expr_true();
}


/**Function********************************************************************

  Synopsis      [Returns the inlined expression, without equivalences and
                 invariants]

  Description   []

  SideEffects   []

******************************************************************************/
Expr_ptr
InlineRes_get_inlined_expr(const InlineRes_ptr self)
{
  INLINE_RES_CHECK_INSTANCE(self);
  return ((node_ptr) NULL != self->inlined) ? self->inlined : Expr_true();
}


/**Function********************************************************************

  Synopsis      [Returns the extracted and forced equivalences as an
                 expression]

  Description   []

  SideEffects   []

******************************************************************************/
Expr_ptr InlineRes_get_equivalences_expr(const InlineRes_ptr self)
{
  Expr_ptr res;
  Set_Iterator_t iter;

  INLINE_RES_CHECK_INSTANCE(self);

  res = Expr_true();
  SET_FOREACH(self->equivs, iter) {
    res = Expr_and_nil(res, (Expr_ptr) Set_GetMember(self->equivs, iter));
  }
  return res;
}


/**Function********************************************************************

  Synopsis      [Returns the extracted and forced equivalences as a set]

  Description   [Returned set belongs to self, do not free it]

  SideEffects   []

******************************************************************************/
Set_t InlineRes_get_equivalences(const InlineRes_ptr self)
{
  INLINE_RES_CHECK_INSTANCE(self);
  return self->equivs;
}


/**Function********************************************************************

  Synopsis      [Returns the conjuction of all forced invariants]

  Description   []

  SideEffects   []

******************************************************************************/
Expr_ptr InlineRes_get_invariant_expr(const InlineRes_ptr self)
{
  Expr_ptr res;
  Set_Iterator_t iter;

  INLINE_RES_CHECK_INSTANCE(self);

  res = Expr_true();
  SET_FOREACH(self->invars, iter) {
    res = Expr_and_nil(res, (Expr_ptr) Set_GetMember(self->invars, iter));
  }
  return res;
}


/**Function********************************************************************

  Synopsis      [Returns the extracted and forced invariants as a set]

  Description   [Returned set belongs to self, do not free it]

  SideEffects   []

******************************************************************************/
Set_t InlineRes_get_invariants(const InlineRes_ptr self)
{
  INLINE_RES_CHECK_INSTANCE(self);
  return self->invars;
}


/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis      [Initializes either the boolean or scalar sexp fsm]

  Description [hierarchy is copied into an independent FlatHierarchy
  instance. If the new sexp must be based only on a set of variables, the
  hierarchy must be empty]

  SideEffects        []

******************************************************************************/
static void sexp_inliner_init(SexpInliner_ptr self,
                              SymbTable_ptr st, const size_t fixpoint_limit)
{
  /* inits some private members */
  self->st = st;
  self->var2expr = new_assoc();
  self->var2invar = new_assoc();
  self->invars = Set_MakeEmpty();
  self->fixpoint_limit = fixpoint_limit;
  self->blacklist = Set_MakeEmpty();
  self->hash_timed2leaves = new_assoc();
  self->hash_subst = new_assoc();
  self->hash_extract_equals_invars = new_assoc();
  self->hash_is_expr_deterministic = new_assoc();
}


/**Function********************************************************************

  Synopsis      [Class deinitializer]

  Description   []

  SideEffects   []

******************************************************************************/
static void sexp_inliner_deinit(SexpInliner_ptr self)
{

  free_assoc(self->hash_is_expr_deterministic);
  self->hash_is_expr_deterministic = (hash_ptr) NULL;

  free_assoc(self->hash_extract_equals_invars);
  self->hash_extract_equals_invars = (hash_ptr) NULL;

  free_assoc(self->hash_subst);
  self->hash_subst = (hash_ptr) NULL;

  free_assoc(self->hash_timed2leaves);
  self->hash_timed2leaves = (hash_ptr) NULL;

  if ((hash_ptr) NULL != self->var2expr) {
    free_assoc(self->var2expr);
    self->var2expr = (hash_ptr) NULL;
  }

  if ((hash_ptr) NULL != self->var2invar) {
    free_assoc(self->var2invar);
    self->var2invar = (hash_ptr) NULL;
  }

  Set_ReleaseSet(self->invars);
  self->invars = (Set_t) NULL;

  Set_ReleaseSet(self->blacklist);
  self->blacklist = (Set_t) NULL;
}


/**Function********************************************************************

  Synopsis      [Copies members from self to copy]

  Description   []

  SideEffects   []

******************************************************************************/
static void sexp_inliner_copy(const SexpInliner_ptr self, SexpInliner_ptr copy)
{
  /* inits some private members */
  copy->fixpoint_limit = self->fixpoint_limit;
  copy->st = self->st;

  if (self->var2expr != (hash_ptr) NULL) {
    copy->var2expr = copy_assoc(self->var2expr);
  } else copy->var2expr = (hash_ptr) NULL;

  if (self->var2invar != (hash_ptr) NULL) {
    copy->var2invar = copy_assoc(self->var2invar);
  } else copy->var2invar = (hash_ptr) NULL;

  copy->invars = Set_Copy(self->invars);
  copy->blacklist = Set_Copy(self->blacklist);
}


/**Function********************************************************************

  Synopsis      [Brings all time-related nodes (ATTIME and NEXT) down to the
                 leaves, while expanding defines and formal parameters]

  Description   [internal self->hash_timed2leaves is used for memoization]

  SideEffects   [self->hash_timed2leaves is changed]

******************************************************************************/
static node_ptr sexp_inliner_move_time_to_leaves(const SexpInliner_ptr self,
                                                 node_ptr expr, int time)
{
  node_ptr key;
  node_ptr res;
  SymbTable_ptr symb_table = SexpInliner_get_symb_table(self);

  if ((node_ptr) NULL == expr) return expr;

  /* checks memoization */
  key = find_node(COLON, expr, NODE_FROM_INT(time));
  res = find_assoc(self->hash_timed2leaves, key);
  if (res != (node_ptr) NULL) return res;

  switch (node_get_type(expr)) {
    /* leaves */
  case FAILURE:
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    res = expr;
    break;

  case CONTEXT:
    internal_error("SexpInliner::mode_time_to_leaves: CONTEXT in expression assumed "\
                   "to be already flattened");

  case BIT:
  case DOT:
  case ATOM:
  case ARRAY: {
    ResolveSymbol_ptr rs;
    node_ptr symb;

    rs = SymbTable_resolve_symbol(self->st, expr, Nil);
    symb = ResolveSymbol_get_resolved_name(rs);

    /* defines are expanded */

    /*   [MR??]: We may think in more deep if we want the DEFINES to be expanded
         [MR??]: or if we can think of expanding later code to handle also expressions
         [MR??]: containing DEFINEs. In this way the expression may grow alot. */
    if (ResolveSymbol_is_define(rs)) {
      /* define (rhs must be boolean, recur to check) */
      node_ptr body = SymbTable_get_define_flatten_body(self->st, symb);
      res = sexp_inliner_move_time_to_leaves(self, body, time);
    }

    /* is it a formal param? */
    else if (ResolveSymbol_is_parameter(rs)) {
      node_ptr actual = SymbTable_get_flatten_actual_parameter(self->st, symb);
      res = sexp_inliner_move_time_to_leaves(self, actual, time);
    }

    /* is it a constant? */
    else if (ResolveSymbol_is_constant(rs)) res = symb;

    /* otherwise keep as it is and attach time information to it */
    else if (time == _UNTIMED_CURRENT) res = symb;

    else if (time == _UNTIMED_NEXT) res = Expr_next(symb, symb_table);

    else {
      /* here it is timed */
      nusmv_assert(time >= 0);
      res = Expr_attime(symb, time, symb_table);
    }

    break;
  }

  case NEXT:
    nusmv_assert(time != _UNTIMED_NEXT); /* no nested next */
    if (time == _UNTIMED_CURRENT) {
      res = sexp_inliner_move_time_to_leaves(self, car(expr), _UNTIMED_NEXT);
    }
    else {
      /* here it is timed */
      nusmv_assert(time >= 0);
      res = sexp_inliner_move_time_to_leaves(self, car(expr), time+1);
    }
    break;

  case ATTIME:
    nusmv_assert(_UNTIMED_NEXT != time); /* not in NEXT */

    res = sexp_inliner_move_time_to_leaves(self,
                                           Expr_attime_get_untimed(expr),
                                           Expr_attime_get_time(expr));
    break;

    /* These are special cases: The cdr part of the subformula is a
       range. If the range is passed to Expr_resolve, and min == max
       (e.g. 18..18), then it is simplified to "18", which then leads
       to a non well-formed formula. */
  case EBF:
  case ABF:
  case EBG:
  case ABG:
  case EBU:
  case ABU:
    nusmv_assert(Nil == cdr(expr) || TWODOTS == node_get_type(cdr(expr)));

    res = Expr_resolve(self->st, node_get_type(expr),
                       sexp_inliner_move_time_to_leaves(self, car(expr), time),
                       cdr(expr));
    break;

  default: /* all other uninteresting cases */
    res = Expr_resolve(self->st, node_get_type(expr),
                       sexp_inliner_move_time_to_leaves(self, car(expr), time),
                       sexp_inliner_move_time_to_leaves(self, cdr(expr), time));
  }

  insert_assoc(self->hash_timed2leaves, key, res); /* memoizes the result */
  return res;
}


/**Function********************************************************************

  Synopsis      [Extract all equals (EQUAL and IFF) in the form v = t.]

  Description   [Unicity of v is not guaranteed. Returns new expr where all
  extracted equals have been substituted.

  This function collect all equalities which HAVE TO be satisfied to
  make the top-expression hold. For example, for (a=b ^ c=d) both
  equalities are returned but for (a=b | c=d) the returned set is
  empty.


  SideEffects   [The results of the function call is memoized in self]

******************************************************************************/
static node_ptr
sexp_inliner_extract_equals_invars(const SexpInliner_ptr self, node_ptr expr,
                                   boolean is_neg,
                                   Set_t* equals, Set_t* invars,
                                   Set_t* vars, boolean* changed)
{
  node_ptr key, res;

  if (Nil == expr) return (node_ptr) NULL;

  /* checks memoization */
  key = expr;
  if (is_neg) key = find_node(NOT, key, Nil);
  res = find_assoc(self->hash_extract_equals_invars, key);
  if (res != Nil) return res;

  switch (node_get_type(expr)) {
  case CONTEXT:
    internal_error("SexpInliner::auto_learn_equals: CONTEXT in expression "\
                   " assumed to be already flattened");

  case NOT:
    res = Expr_not(sexp_inliner_extract_equals_invars(self, car(expr),
                                                      !is_neg,
                                                      equals, invars,
                                                      vars, changed));
    break;

  case AND:
    if (!is_neg) { /* negated AND becomes OR which is useless for us */
      res = Expr_and(sexp_inliner_extract_equals_invars(self, car(expr),
                                                        is_neg,
                                                        equals, invars,
                                                        vars, changed),
                     sexp_inliner_extract_equals_invars(self, cdr(expr),
                                                        is_neg,
                                                        equals, invars,
                                                        vars, changed));
    }
    /* otherwise just resolve */
    else res = Expr_resolve(self->st, node_get_type(expr), car(expr), cdr(expr));
    break;

  case OR:
    if (is_neg) { /* negated OR becomes AND */
      res = Expr_or(sexp_inliner_extract_equals_invars(self, car(expr),
                                                       is_neg,
                                                       equals, invars,
                                                       vars, changed),
                    sexp_inliner_extract_equals_invars(self, cdr(expr),
                                                       is_neg,
                                                       equals, invars,
                                                       vars, changed));
    }
    /* otherwise just resolve */
    else res = Expr_resolve(self->st, node_get_type(expr), car(expr), cdr(expr));
    break;


    /*   [MR??]: a iff b = !a xor b, or alternatively a xor !b. Are there other operators that
         [MR??]: it would be worth to consider? */
  case IMPLIES:
    if (is_neg) {
      res = Expr_implies(sexp_inliner_extract_equals_invars(self, car(expr),
                                                            false,
                                                            equals, invars,
                                                            vars, changed),
                         sexp_inliner_extract_equals_invars(self, cdr(expr),
                                                            is_neg,
                                                            equals, invars,
                                                            vars, changed));
    }
    /* otherwise just resolve */
    else res = Expr_resolve(self->st, node_get_type(expr), car(expr), cdr(expr));
    break;

  case NOTEQUAL:
    if (is_neg) {
      res = sexp_try_acquiring_equality(self, expr, is_neg,
                                        equals, vars, changed);
    }
    /* otherwise just resolve */
    else res = Expr_resolve(self->st, node_get_type(expr), car(expr), cdr(expr));
    break;

  case IFF:
  case EQUAL:
    if (!is_neg) { /* process equalities only with positive polarity */
      res = sexp_try_acquiring_equality(self, expr, is_neg,
                                        equals, vars, changed);
    }
    /* otherwise just resolve */
    else res = Expr_resolve(self->st, node_get_type(expr), car(expr), cdr(expr));
    break;

  case EQDEF:
    if (!is_neg) {
      if (NEXT != node_get_type(car(expr)) &&
          SMALLINIT != node_get_type(car(expr))) {
        /* this is an invariant */
        node_ptr hit;


        /* [MR??]: assignment to next variables and to to initial values
           [MR??]: of variables. Is there any reason why for instance we
           [MR??]: do not want to inline initial expression or next?
           [MR??]: init(x) := y + 1; init(y) := z + 2 the y can be inlined in
           [MR??]: the first expression (similarly for next(x)) thus possibly
           [MR??]: simplifying a lot expressions, and possibly reduce the cone
           [MR??]: of variables. */
        /* checks if the variable has a forced invariant already: in
           this case keep the formula */
        hit = find_assoc(self->var2invar, car(expr));
        if ((node_ptr) NULL != hit) {
          res = sexp_inliner_assign_to_setin(self, expr);
          break;
        }

        /* checks if variable has a forced equivalence already: in
           this case keep the formula */
        hit = find_assoc(self->var2expr, car(expr));
        if ((node_ptr) NULL != hit) {
          res = sexp_inliner_assign_to_setin(self, expr);
          break;
        }

        if (!sexp_inliner_is_expr_deterministic(self, cdr(expr))) {
          res = sexp_inliner_assign_to_setin(self, expr);
          break;
        }

        /* acquire the invariant */
        if (opt_verbose_level_gt(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "SexpInliner: acquiring invar '");
          print_node(nusmv_stderr, expr);
          fprintf(nusmv_stderr, "'\n");
        }

        *invars = Set_AddMember(*invars, (Set_Element_t) expr);
        res = Expr_true();
        break;
      }
      /* not an invariant, handles it as a normal equality */
      res = sexp_try_acquiring_equality(self, expr, is_neg,
                                        equals, vars, changed);
    }
    /* otherwise just resolve */
    else res = Expr_resolve(self->st, node_get_type(expr), car(expr), cdr(expr));
    break;



  case NEXT:
    nusmv_assert((DOT == node_get_type(car(expr))) ||
                 (ATOM == node_get_type(car(expr))) ||
                 (BIT == node_get_type(car(expr))) ||
                 (ARRAY == node_get_type(car(expr))));
    /* a variable here must be boolean, and we can acquire
       next(v) <-> neg ? F:T */
    if (SymbTable_is_symbol_var(self->st, car(expr))) {
      nusmv_assert(SymbTable_is_symbol_bool_var(self->st, car(expr)));
      res = sexp_try_acquiring_equality(self,
                                        is_neg ?

      /* [MR??]: simplify the expressions into expr or !expr thus breaking the
         [MR??]: expected input. */
                                        find_node(IFF, expr, Expr_false()) :
                                        find_node(IFF, expr, Expr_true()),
                                        is_neg, /*false,*/ equals, vars,
                                        changed);
    }
    /* otherwise just resolve */
    else res = Expr_resolve(self->st, node_get_type(expr), car(expr), cdr(expr));
    break;

  case DOT:
  case ATOM:
  case BIT:
  case ARRAY:
    /* a variable here must be boolean, and we can acquire v <-> neg ? F:T */
    if (SymbTable_is_symbol_var(self->st, expr)) {
      nusmv_assert(SymbTable_is_symbol_bool_var(self->st, expr));
      res = sexp_try_acquiring_equality(self,
                                        is_neg ?

      /* [MR??]: simplify the expressions into expr or !expr thus breaking the
         [MR??]: expected input. */
                                        find_node(IFF, expr, Expr_false()) :
                                        find_node(IFF, expr, Expr_true()),
                                        is_neg, /*false,*/ equals, vars, changed);
    }
    /* otherwise just resolve */
    else res = Expr_resolve(self->st, node_get_type(expr), car(expr), cdr(expr));
    break;

  default:
    /* for all other expressions just resolve */
    res = Expr_resolve(self->st, node_get_type(expr), car(expr), cdr(expr));
  }

  nusmv_assert(Nil != res); /* the exp has to be processed already */

  /* memoizes the result */
  insert_assoc(self->hash_extract_equals_invars, key, res);

  return res;
}


/**Function********************************************************************

  Synopsis      [privately used by sexp_inliner_extract_candidate_equals]

  Description   [Callback to fill in the good_equals array]

  SideEffects   []

******************************************************************************/
static enum st_retval
sexp_inliner_fill_good_equals(char* key, char* data, char* arg)
{
  nusmv_assert(COLON == node_get_type((node_ptr) data));
  array_insert_last(node_ptr, (array_t*) arg, (node_ptr) data);
  return ST_CONTINUE;
}


/**Function********************************************************************

  Synopsis      [privately used by sexp_inliner_extract_candidate_equals]

  Description   [Callback to sort the good_equals array]

  SideEffects   []

******************************************************************************/
static int sexp_inliner_sort_good_equals(const void* obj1, const void* obj2)
{
  register node_ptr n1 = (*(node_ptr*) obj1);
  register node_ptr n2 = (*(node_ptr*) obj2);

  nusmv_assert(COLON == node_get_type(n1) && COLON == node_get_type(n2));

  return (PTR_TO_INT(cdr(n1)) - PTR_TO_INT(cdr(n2)));
}


/**Function********************************************************************

  Synopsis      [used privately for free the content of the array
                 returned by sexp_inliner_extract_candidate_equals]

  Description   []

  SideEffects   []

******************************************************************************/
static void sexp_inliner_free_equalities_array(array_t* arr)
{
  int k;
  node_ptr col;

  arrayForEachItem(node_ptr, arr, k, col) {
    if ((node_ptr) NULL != col) {
      nusmv_assert(COLON == node_get_type(col));
      free_node(col);
    }
  }
}


/**Function********************************************************************

  Synopsis      [used for debugging purposes]

  Description   []

  SideEffects   []

******************************************************************************/
static void sexp_inliner_print_equality_array(array_t* arr, FILE* _file)
{

  int k;
  node_ptr col;

  fprintf(_file, "The ordered equalities array is:\n");
  fprintf(_file, "pos:deps:equality\n");

  arrayForEachItem(node_ptr, arr, k, col) {
    if ((node_ptr) NULL != col) {
      int deps;
      nusmv_assert(COLON == node_get_type(col));
      deps = PTR_TO_INT(cdr(col));
      fprintf(_file, "%d:%d:", k, deps);
      print_node(_file, car(col));
    }
    else fprintf(_file, "%d: : REMOVED", k);

    fprintf(_file, "\n");
  }
  fprintf(_file, "\n");
}


/**Function********************************************************************

  Synopsis      [Removes duplicates]

  Description [splits equals into good_equals and rem_equals
  sets. Returned array must be freed by the caller]

  SideEffects   []

******************************************************************************/
static array_t*
sexp_inliner_extract_candidate_equals(const SexpInliner_ptr self,
                                      const Set_t equals, const Set_t imp_vars,
                                      const hash_ptr var2invar,
                                      Set_t* rem_equals)
{
  array_t* good_equals;

  /* this hash associates 'var' -> COLON('var op expr', deps_num),
     where op can be IFF or EQUAL. It used to keep the best var */
  hash_ptr hash = new_assoc();
  Set_Iterator_t iter;

  SET_FOREACH(equals, iter) {
    node_ptr equal = Set_GetMember(equals, iter);
    node_ptr var, expr, prev_expr;
    Set_t deps;
    Set_t ideps;

    nusmv_assert(EQUAL == node_get_type(equal) ||
                 IFF == node_get_type(equal) ||
                 EQDEF == node_get_type(equal));

    var = car(equal);
    expr = cdr(equal);

    /* skip init(var) := ... */
    if (EQDEF == node_get_type(equal) &&
        SMALLINIT == node_get_type(var)) continue;

    /* check if there is already an extracted invariant about the
       variable, and re-introduce the equivalence in rem_equivs */
    if ((node_ptr) NULL != find_assoc(var2invar, var)) {
      *rem_equals = Set_AddMember(*rem_equals, (Set_Element_t) equal);
      continue;
    }

    /* Dependencies wrt imp_vars only */
    deps = Formula_GetDependenciesByType(self->st, expr, Nil,
                                         VFT_ALL, true /* preserve time */);
    ideps = Set_Copy(deps);
    ideps = Set_Intersection(ideps, imp_vars);

    if (Set_IsMember(ideps, var)) {
      /* here there is a self-loop. Inserts into remaining */
      *rem_equals = Set_AddMember(*rem_equals, (Set_Element_t) equal);
    }
    else { /* no self-loop, compares with the best candidate found so far */
      int expr_deps = Set_GiveCardinality(deps);

      prev_expr = find_assoc(hash, var);
      if ((node_ptr) NULL != prev_expr) {
        int prev_deps;
        nusmv_assert(COLON == node_get_type(prev_expr));
        prev_deps = PTR_TO_INT(cdr(prev_expr));

        if (prev_deps > expr_deps) {
          /* prev_expr is substituted by expr */
          insert_assoc(hash, var,
                       new_node(COLON, equal, PTR_FROM_INT(node_ptr,
                                                           expr_deps)));

      /* [MR??]: However, no free_node is applied on the previous
                 stored data, thus that node is lost */
          *rem_equals = Set_AddMember(*rem_equals,
                                      (Set_Element_t) car(prev_expr));
        }
        else { /* keeps prev_deps */
          *rem_equals = Set_AddMember(*rem_equals, (Set_Element_t) equal);
        }
      }
      else { /* never found var before */
        insert_assoc(hash, var,
                     new_node(COLON, equal, PTR_FROM_INT(node_ptr, expr_deps)));
      }
    }
    /* This needs to be released, Formula_GetDependenciesByType
       returns a copy of the set, and it is the responsibility of
       the caller to free it. */
    Set_ReleaseSet(deps);
    Set_ReleaseSet(ideps);
  } /* foreach equals loop */

  /* constructs, fills in and sort good_equals array. Sorting is
     performed looking at the cardinality associated with each
     equal within the hash */
  good_equals = array_alloc(node_ptr,
       Set_GiveCardinality(equals) - Set_GiveCardinality(*rem_equals));
  st_foreach(hash, sexp_inliner_fill_good_equals, (char*) good_equals);
  array_sort(good_equals, sexp_inliner_sort_good_equals);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 6)) {
    fprintf(nusmv_stderr, "In sexp_inliner_extract_candidate_equals:\n");
    sexp_inliner_print_equality_array(good_equals, nusmv_stderr);
  }

  /* nodes within the hash are not freed (even if they were built
     with new_node), as they have been copied inside the returned
     array, and they will be freed later by
     sexp_inliner_free_equalities_array */
  free_assoc(hash);
  return good_equals;
}


#if 1
/* warning "Inefficient version of loop removal" /*
/**Function********************************************************************

  Synopsis      [Removes loops from good_equals]

  Description   []

  SideEffects   []

******************************************************************************/
static hash_ptr
sexp_inliner_remove_loops(const SexpInliner_ptr self,
                          array_t* good_equals, hash_ptr hash_invars,
                          Set_t* good, Set_t* rem)
{
  hash_ptr hash_equals = new_assoc();
  int i;

  for (i=0; i < array_n(good_equals); ++i) {
    node_ptr coli = array_fetch(node_ptr, good_equals, i);
    node_ptr vi, ti;
    int j;

    if ((node_ptr) NULL != coli) {
      nusmv_assert(COLON == node_get_type(coli));
      vi = car(car(coli));
      ti = cdr(car(coli));

      insert_assoc(hash_equals, vi, ti);

      for (j=i+1; j < array_n(good_equals); ++j) {
        /* this loop tries to inline the following equal, or drops
           the following equals if loops are detected */

        node_ptr colj = array_fetch(node_ptr, good_equals, j);
        if ((node_ptr) NULL != colj) {
          node_ptr vj, tj, tj_inlined;
          Set_t deps;

          nusmv_assert(COLON == node_get_type(colj));
          vj = car(car(colj));
          tj = cdr(car(colj));

          /* here we inline even if vi is not in tj, as there are
             also user-specified invariants and equalities which
             may be introducing loops */
          tj_inlined = sexp_inliner_substitute(self, tj,
                                               hash_equals, hash_invars,
                                               (boolean*) NULL);

          /* now checks if a loop has been introduced */
          deps = Formula_GetDependenciesByType(self->st, tj_inlined, Nil,
                                               VFT_ALL, true/*preserve time*/);
          if (Set_IsMember(deps, vj)) {
            /* detected a loop: move to remaining set, clears the
               corresponding entry within the array */
            *rem = Set_AddMember(*rem, (Set_Element_t) car(colj));
            free_node(colj);
            array_insert(node_ptr, good_equals, j, (node_ptr) NULL);
          }
          else { /* inlines tj */
            node_ptr new_colj =
              new_node(COLON,
                       find_node(node_get_type(car(colj)), vj, tj_inlined),
                       cdr(colj));

            /* removes the previous node, which will be substituted
               with one containing the inlined expression */
            free_node(colj);

            /* substitutes colj with new_colj within the ordered array */
            array_insert(node_ptr, good_equals, j, new_colj);
          }

          /* dependencies are no longer needed */
          Set_ReleaseSet(deps);
        }
      } /* end of inner loop */
    }
  } /* end of outer loop */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 7)) {
    fprintf(nusmv_stderr, "Removing loops: after first traversal:\n");
    sexp_inliner_print_equality_array(good_equals, nusmv_stderr);
  }

  /* here hash_equals does not contain loops, but expression may be
     furtherly simplified, so we drops it and rebuild it by
     traversing the array bottom-up */
  clear_assoc(hash_equals);
  /* traverses the array from bottom-up to re-construct the hash */
  for (i=array_n(good_equals)-1; i>=0; --i) {
    node_ptr col = array_fetch(node_ptr, good_equals, i);

    /* if the entry was not removed */
    if ((node_ptr) NULL != col) {
      node_ptr v, t, e;
      nusmv_assert(COLON == node_get_type(col));

      e = car(col);
      v = car(e); t = cdr(e);

      /* last entry is surely already simplified */
      if (i < array_n(good_equals)-1) {
        boolean tchanged;
        t = sexp_inliner_substitute(self, t, hash_equals, hash_invars,
                                    &tchanged);
        if (tchanged) e = find_node(node_get_type(e), v, t);
      }

      /* adds the equality to the good set, and updates hash_equals */
      *good = Set_AddMember(*good, (Set_Element_t) e);
      insert_assoc(hash_equals, v, t);
    }
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 7)) {
    fprintf(nusmv_stderr, "Removing loops, after re-traversing bottom-up:\n");
    sexp_inliner_print_equality_array(good_equals, nusmv_stderr);
  }

  /* here hash_equals is fully simplified */
  return hash_equals;
}
#else


/**Function********************************************************************

  Synopsis      [Removes loops from good_equals]

  Description   []

  SideEffects   []

******************************************************************************/
static hash_ptr
sexp_inliner_remove_loops(const SexpInliner_ptr self,
                          array_t* good_equals, hash_ptr invar_hash,
                          Set_t* good, Set_t* rem)
{
  hash_ptr hash_equals = new_assoc();
  int i;

  for (i=0; i < array_n(good_equals); ++i) {
    node_ptr coli = array_fetch(node_ptr, good_equals, i);
    node_ptr vi, ti;
    int j;

    if ((node_ptr) NULL != coli) {
      nusmv_assert(COLON == node_get_type(coli));
      vi = car(car(coli));
      ti = cdr(car(coli));

      insert_assoc(hash_equals, vi, ti);

      for (j=i+1; j < array_n(good_equals); ++j) {
        /* this loop tries to inline the following equal, or drops
           the following equals if loops are detected */

        node_ptr colj = array_fetch(node_ptr, good_equals, j);
        if ((node_ptr) NULL != colj) {
          node_ptr vj, tj, tj_inlined;
          Set_t deps;
          boolean change = false;

          nusmv_assert(COLON == node_get_type(colj));
          vj = car(car(colj));
          tj = cdr(car(colj));

          /* here we inline even if vi is not in tj, as there are
             also user-specified invariants and equalities which
             may be introducing loops */
          
          tj_inlined = sexp_inliner_substitute(self, tj,
                                               hash_equals, invar_hash,
                                               (boolean*) NULL);

          /* now checks if a loop has been introduced */
          deps = Formula_GetDependenciesByType(self->st, tj_inlined, Nil,
                                               VFT_ALL, true /* preserve time */);
          if (Set_IsMember(deps, vj)) {
            /* detected a loop: move to remaining set, clears the
               corresponding entry within the array */
            *rem = Set_AddMember(*rem, (Set_Element_t) car(colj));
            free_node(colj);
            array_insert(node_ptr, good_equals, j, (node_ptr) NULL);
          }
          else { /* inlines tj */
            node_ptr new_colj =
              new_node(COLON,
                       find_node(node_get_type(car(colj)), vj, tj_inlined),
                       cdr(colj));

            /* removes the previous node, which will be substituted
               with one containing the inlined expression */
            free_node(colj);

            /* substitutes colj with new_colj within the ordered array */
            array_insert(node_ptr, good_equals, j, new_colj);
          }

          /* dependencies are no longer needed */
          Set_ReleaseSet(deps);
        }
      } /* end of inner loop */
    }
  } /* end of outer loop */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 7)) {
    fprintf(nusmv_stderr, "Removing loops: after first traversal:\n");
    sexp_inliner_print_equality_array(good_equals, nusmv_stderr);
  }

  /* here hash_equals does not contain loops, but expression may be
     furtherly simplified, so we drops it and rebuild it by
     traversing the array bottom-up */
  clear_assoc(hash_equals);
  /* traverses the array from bottom-up to re-construct hash_equals */
  for (i=array_n(good_equals)-1; i>=0; --i) {
    node_ptr col = array_fetch(node_ptr, good_equals, i);

    /* if the entry was not removed */
    if ((node_ptr) NULL != col) {
      node_ptr v, t;
      nusmv_assert(COLON == node_get_type(col));

      v = car(car(col));
      t = cdr(car(col));

      /* last entry is surely already simplified */
      if (i < array_n(good_equals)-1) {
        t = sexp_inliner_substitute(self, t, hash_equals, hash_invar,
                                    (boolean*) NULL);
      }

      /* adds the equality to the good set, and updates hash_equals */
      *good = Set_AddMember(*good, (Set_Element_t) car(col));
      insert_assoc(hash_equals, v, t);
    }
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 7)) {
    fprintf(nusmv_stderr, "Removing loops, after re-traversing bottom-up:\n");
    sexp_inliner_print_equality_array(good_equals, nusmv_stderr);
  }

  /* here hash_equals is fully simplified */
  return hash_equals;
}
#endif


/**Function********************************************************************

  Synopsis           [If given equal concerns a variable (like in
                      'expr = var') the equality is added to the given set
                      (after possible syntactic manipolation) and
                      the True expression is returned. Otherwise
                      the given equal expression is returned.]

  Description        [This is called during the traversal of the
                      expression when auto-extracting equalities.
                      WARNING! The equal expression is assumed to
                      be already flattened.]

  SideEffects        []

*****************************************************************************/
static node_ptr sexp_try_acquiring_equality(const SexpInliner_ptr self,
                                            node_ptr equal, boolean is_neg,
                                            Set_t* set, Set_t* vars,
                                            boolean* changed)
{
  node_ptr left, right;
  node_ptr var = Nil;
  node_ptr expr = Nil;
  node_ptr res;
  int type;

  /* normalizes type wrt to the essential value */
  if (is_neg && NOTEQUAL == node_get_type(equal)) type = EQUAL;
  else type = node_get_type(equal);

  nusmv_assert(EQUAL == type || IFF == type || EQDEF == type);

#if 0
  /* skip init(var) := expr */
  
  if (EQDEF == type && SMALLINIT == node_get_type(car(equal))) return equal;
#endif

  left = sexp_inliner_expr_is_var(self, car(equal)) ? car(equal) : Nil;
  right = sexp_inliner_expr_is_var(self, cdr(equal)) ? cdr(equal) : Nil;

  if (Nil != left && Nil == right) {
    var = left;
    expr = cdr(equal);
  }
  else if (Nil == left && Nil != right) {
    var = right;
    expr = car(equal);
  }
  else if (Nil != left && Nil != right && left != right) {
    /* take one randomly (this can be improved) */
    var = left;
    expr = right;
  }

  /*     [MR??]: an equality among constants and thus we do not need to even proceed and
         [MR??]: we can simplify the expression if possible w.r.t. the values
         [MR??]: either to TRUE, FALSE or leave as it is if we cannot perfrm the test. */
  if ((Nil != var) && (SMALLINIT == node_get_type(var))) {
    var = car(var);

  }

  /* the equality is acquired only if not in black list, and if the
     expression is found syntactically deterministic */
  if (Nil != var && Nil != expr &&
      !Set_IsMember(self->blacklist, (Set_Element_t) var) &&
      sexp_inliner_is_expr_deterministic(self, expr)) {
    node_ptr hit;

    /* checks if the variable has a forced invariant already: in
       this case keep the formula */
    hit = find_assoc(self->var2invar, var);
    if ((node_ptr) NULL != hit) return equal;

    /* checks if variable has a forced equivalence already: in
       this case keep the formula */
    hit = find_assoc(self->var2expr, var);
    if ((node_ptr) NULL != hit) return equal;

    /* here the equivalence can be acquired */
    *vars = Set_AddMember(*vars, (Set_Element_t) var);

    *set = Set_AddMember(*set, (Set_Element_t) find_node(type, var, expr));

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 6)) {
      fprintf(nusmv_stderr, "SexpInliner: acquiring equality '");
      print_node(nusmv_stderr, find_node(type, var, expr));
      fprintf(nusmv_stderr, "'\n");
    }

    res = is_neg ? Expr_false() : Expr_true();
  }
  else res = equal; /* not a variable equality */

  return res;
}


/**Function********************************************************************

  Synopsis         [Returns true if given flattened expression is a
                    variable (potentially timed).]

  Description      [expr is assumed to have next and timed nodes moved
                    to the leaves.]

  SideEffects      []

******************************************************************************/
static boolean
sexp_inliner_expr_is_var(const SexpInliner_ptr self, node_ptr expr)
{
  node_ptr symb;

  if (NEXT == node_get_type(expr) || SMALLINIT == node_get_type(expr) ||
      ATTIME == node_get_type(expr)) symb = car(expr);
  else symb = expr;

  if (BIT == node_get_type(symb)    ||
      DOT == node_get_type(symb)    ||
      ATOM == node_get_type(symb)   ||
      ARRAY == node_get_type(symb)) {
    return SymbTable_is_symbol_var(self->st, symb);
  }
  return false;
}


/**Function********************************************************************

  Synopsis [Traverses the structure of the expression, substituting
  (in top-level conjuctions) all found equivalences. Invariants are
  substituted within the whole expression. changed is set to true
  when applying the inlining, otherwise it keeps its values.]

  Description [The inlined expression is returned. var2invar (can be NULL) and
  is used to resolve invariants extracted from ASSIGNs.
  internal , self->hash_subst is used for memoization]

  SideEffects   [changed and self->hash_subst are modified]

******************************************************************************/
static node_ptr
sexp_inliner_substitute(SexpInliner_ptr self, node_ptr expr,
                        hash_ptr var2expr, hash_ptr var2invar,
                        boolean* changed)
{
  node_ptr res;
  SymbTable_ptr symb_table = SexpInliner_get_symb_table(self);

  if ((node_ptr) NULL == expr) return (node_ptr) NULL;

  /* memoization */
  res = find_assoc(self->hash_subst, expr);
  if (res != (node_ptr) NULL) {
    node_ptr res_in = car(res);

    nusmv_assert(COLON == node_get_type(res));
    if ((boolean*) NULL != changed) *changed = false;

    if (res_in != expr) {
      res_in = sexp_inliner_substitute(self, res_in, var2expr, var2invar,
                                       changed);
    }

    if ((boolean*) NULL == changed || *changed) {
      insert_assoc(self->hash_subst, expr,



                   find_node(COLON, res_in,
                             ((boolean*) NULL != changed) ?
                             PTR_FROM_INT(node_ptr, (int) *changed) :
                             PTR_FROM_INT(node_ptr, false)));
    }

    return res_in;
  }

  switch (node_get_type(expr)) {
    /* leaves */
  case FAILURE:
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
    return expr;

  case BIT:
  case ATOM:
  case DOT:
  case ARRAY: {
    node_ptr hit;

    /* tries with automatic equivalences */
    hit = find_assoc(var2expr, expr);
    if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
      fprintf(nusmv_stdout, "Substituting v2e \""); print_node(nusmv_stdout, expr);
      fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
      if ((boolean*) NULL != changed && hit != expr) *changed = true;
      return hit;
    }

    /* tries with user-provided equivalences */
    hit = find_assoc(self->var2expr, expr);
    if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
      fprintf(nusmv_stdout, "Substituting iv2e \""); print_node(nusmv_stdout, expr);
      fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
      if ((boolean*) NULL != changed && hit != expr) *changed = true;
      return hit;
    }

    /* tries with invariants (forced and auto-extracted) */
    if ((hash_ptr) NULL != var2invar) {
      hit = find_assoc(var2invar, expr);
      if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
        fprintf(nusmv_stdout, "Substituting v2i \""); print_node(nusmv_stdout, expr);
        fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
        if ((boolean*) NULL != changed && hit != expr) *changed = true;
        return hit;
      }
    }
    hit = find_assoc(self->var2invar, expr);
    if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
      fprintf(nusmv_stdout, "Substituting iv2i \""); print_node(nusmv_stdout, expr);
      fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
      if ((boolean*) NULL != changed && hit != expr) *changed = true;
      return hit;
    }
    return expr;
  }

  case CONTEXT:
    internal_error("SexpInliner::substitute: CONTEXT in expression assumed"\
                   " to be already flattened");

  case NEXT: { /* next was moved to leaves */
    node_ptr name = car(expr);
    node_ptr nname = Expr_next(name, symb_table);
    node_ptr hit;

    nusmv_assert((DOT == node_get_type(name)) ||
                 (ATOM == node_get_type(name)) ||
                 (BIT == node_get_type(name)) ||
                 (ARRAY == node_get_type(name)));

    /* tries with automatic equivalences */
    hit = find_assoc(var2expr, nname);
    if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
      fprintf(nusmv_stdout, "Substituting v2e \""); print_node(nusmv_stdout, nname);
      fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
      if ((boolean*) NULL != changed && hit != nname) *changed = true;
      return hit;
    }

    /* tries with user-provided equivalences */
    hit = find_assoc(self->var2expr, nname);
    if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
      fprintf(nusmv_stdout, "Substituting iv2e \""); print_node(nusmv_stdout, nname);
      fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
      if ((boolean*) NULL != changed && hit != nname) *changed = true;
      return hit;
    }

    /* sees if there is an invariant about this variable (first
       auto-extracted, then user-provided) */
    if ((hash_ptr) NULL != var2invar) {
      hit = find_assoc(var2invar, name);
      if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
        fprintf(nusmv_stdout, "Substituting v2i \""); print_node(nusmv_stdout, nname);
        fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
        if ((boolean*) NULL != changed && hit != name) *changed = true;
        hit = Expr_next(hit, symb_table);
        hit = sexp_inliner_move_time_to_leaves(self, hit, _UNTIMED_CURRENT);
        return hit;
      }
    }
    hit = find_assoc(self->var2invar, name);
    if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
      fprintf(nusmv_stdout, "Substituting iv2i \""); print_node(nusmv_stdout, nname);
      fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
      if ((boolean*) NULL != changed && hit != name) *changed = true;
      /* We move next to leaves */
      hit = Expr_next(hit, symb_table);
      hit = sexp_inliner_move_time_to_leaves(self, hit, _UNTIMED_CURRENT);
      return hit;
    }
    /* no hit in the invariants */
    return nname;
  }

  case ATTIME: { /* attime was moved to leaves */
    node_ptr name = Expr_attime_get_untimed(expr);
    node_ptr hit;

    nusmv_assert((DOT == node_get_type(name)) ||
                 (ATOM == node_get_type(name)) ||
                 (BIT == node_get_type(name)) ||
                 (ARRAY == node_get_type(name)));

    /* tries with automatic equivalences */
    hit = find_assoc(var2expr, expr);
    if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
      fprintf(nusmv_stdout, "Substituting v2e \""); print_node(nusmv_stdout, expr);
      fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
      if ((boolean*) NULL != changed && hit != expr) *changed = true;
      return hit;
    }

    /* tries with user-provided equivalences */
    hit = find_assoc(self->var2expr, expr);
    if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
      fprintf(nusmv_stdout, "Substituting iv2e \""); print_node(nusmv_stdout, expr);
      fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
      if ((boolean*) NULL != changed && hit != expr) *changed = true;
      return hit;
    }

    /* sees if there is an invariant about this variable (first
       auto-extracted, then user-provided) */
    if ((hash_ptr) NULL != var2invar) {
      hit = find_assoc(var2invar, name);
      if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
        fprintf(nusmv_stdout, "Substituting v2i \""); print_node(nusmv_stdout, expr);
        fprintf(nusmv_stdout, "\" with \""); print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
        if ((boolean*) NULL != changed && hit != name) *changed = true;
        error_unreachable_code();

      /* [MRRC]: should be returned assuring at-time is moved to leaves */
        return hit;
      }
    }
    hit = find_assoc(self->var2invar, name);
    if ((node_ptr) NULL != hit) {
#ifdef _DEBUG_SUBST
      fprintf(nusmv_stdout, "Substituting iv2i \"");
      print_node(nusmv_stdout, expr);
      fprintf(nusmv_stdout, "\" with \"");
      print_node(nusmv_stdout, hit); fprintf(nusmv_stdout, "\n");
#endif
      if ((boolean*) NULL != changed && hit != name) *changed = true;
      error_unreachable_code();

      /* [MRRC]: should be returned assuring at-time is moved to leaves */
      return Expr_attime(hit, Expr_attime_get_time(expr), symb_table);
    }
    /* no hit in the invariants */
    return expr;
  }

  case AND:
    res = Expr_and(sexp_inliner_substitute(self, car(expr), var2expr, var2invar,
                                           changed),
                   sexp_inliner_substitute(self, cdr(expr), var2expr, var2invar,
                                           changed));
    break;

  case EQDEF:
    /* substitutes only left side */
    res = Expr_resolve(self->st, node_get_type(expr), car(expr),
                       sexp_inliner_substitute(self, cdr(expr), var2expr,
                                               var2invar, changed));
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

    res = Expr_resolve(self->st, node_get_type(expr),
                       sexp_inliner_substitute(self, car(expr), var2expr,
                                               var2invar, changed),
                       cdr(expr));
    break;

  default:
    res = Expr_resolve(self->st, node_get_type(expr),
                       sexp_inliner_substitute(self, car(expr), var2expr,
                                               var2invar, changed),
                       sexp_inliner_substitute(self, cdr(expr), var2expr,
                                               var2invar, changed));
  }

  /* memoizes the result */
  insert_assoc(self->hash_subst, expr,
               find_node(COLON, res,
                         ((boolean*) NULL != changed) ?
                         PTR_FROM_INT(node_ptr, (int) *changed) :
                          PTR_FROM_INT(node_ptr, false)));
  return res;
}


/**Function********************************************************************

  Synopsis      [Here expr has next and attime moved to the leaves]

  Description   [If the expression's cone contains the variable, the
                 equivalence is not created. If the given
                 expression is syntactically non-deterministic (see
                 sexp_inliner_is_expr_deterministic about this
                 over-approximation) the equivalence is not
                 created. The equivalence is learnt independently
                 on the blacklist. This is called only when the
                 user forces an equivalence to exist, and not when
                 extracting equivalences automatically.

                 Returns true if the equivalences was successfully
                 created, or false otherwise.]

  SideEffects   []

******************************************************************************/
static boolean
sexp_inliner_force_equivalence(SexpInliner_ptr self,
                               node_ptr var, Expr_ptr expr)
{
  boolean res = false;

  if (sexp_inliner_is_expr_deterministic(self, expr)) {
    Set_t deps = Formula_GetDependenciesByType(self->st, expr, Nil,
                                               VFT_ALL,
                                               true /* preserve time */);
    if (!Set_IsMember(deps, var)) {
      insert_assoc(self->var2expr, var, expr);
      res = true;

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 6)) {
        _PRINT("SexpInliner: accepted user-provided equivalence '");
        print_node(nusmv_stderr, var);
        _PRINT(" ==> ");
        print_node(nusmv_stderr, expr);
        _PRINT("'\n");
      }
    }
    Set_ReleaseSet(deps);
  }

  return res;
}


/**Function********************************************************************

  Synopsis      [Here expr has next and attime moved to the leaves]

  Description   [If the expression's cone contains the variable,
                 the equivalence is not created.
                 If the given
                 expression is syntactically non-deterministic (see
                 sexp_inliner_is_expr_deterministic about this
                 over-approximation) the equivalence is not
                 created.

                 Returns true if the equivalences was successfully
                 created, or false otherwise.]

  SideEffects   []

******************************************************************************/
static boolean
sexp_inliner_force_invariant(SexpInliner_ptr self, node_ptr var, Expr_ptr expr)
{
  boolean res = false;
  SymbTable_ptr symb_table = SexpInliner_get_symb_table(self);

  if (sexp_inliner_is_expr_deterministic(self, expr)) {
    Set_t deps = Formula_GetDependenciesByType(self->st, expr, Nil,
                                               VFT_ALL, true /*preserve time*/);
    if (!Set_IsMember(deps, var)) {
      Expr_ptr old_invar = find_assoc(self->var2invar, var);

      if (expr != old_invar) {
        expr = Expr_and_nil(old_invar, expr);
        insert_assoc(self->var2invar, var, expr);
        self->invars = Set_AddMember(self->invars,
                                     (Set_Element_t)Expr_equal(var,
                                                               expr,
                                                               symb_table));
        res = true;

        if (opt_verbose_level_gt(OptsHandler_get_instance(), 6)) {
          _PRINT("SexpInliner: accepted invariant '");
          print_node(nusmv_stderr, var);
          _PRINT(" ==> ");
          print_node(nusmv_stderr, expr);
          _PRINT("'\n");
        }
      }
    }
    Set_ReleaseSet(deps);
  }

  return res;
}



/**Function********************************************************************

  Synopsis      [Private constructor]

  Description   []

  SideEffects   []

******************************************************************************/
static InlineRes_ptr inline_res_create(Expr_ptr orig)
{
  InlineRes_ptr self = ALLOC(InlineRes, 1);
  INLINE_RES_CHECK_INSTANCE(self);

  self->orig = orig;
  self->inlined = Expr_true();
  self->equivs = Set_MakeEmpty();
  self->invars = Set_MakeEmpty();

  return self;
}


/**Function********************************************************************

  Synopsis      [Class deinitializer]

  Description   []

  SideEffects   []

******************************************************************************/
static void inline_res_deinit(InlineRes_ptr self)
{
  Set_ReleaseSet(self->equivs);
  self->equivs = (Set_t) NULL;

  Set_ReleaseSet(self->invars);
  self->invars = (Set_t) NULL;
}


/**Function********************************************************************

  Synopsis      [Private comparison]

  Description   []

  SideEffects   []

******************************************************************************/
static int sexp_inliner_expr_ptr_compare(const void * c1, const void * c2)
{
  Expr_ptr a = *((Expr_ptr *) c1); Expr_ptr b = *((Expr_ptr *) c2);
  if (a < b) return -1;
  if (a == b) return 0;
  return 1;
}


/**Function********************************************************************

  Synopsis      [Detects if the given expression is deterministic.]

  Description   [If TWODOTS or UNION node is found in the expression,
                 then the expression (with an over-approximation)
                 is considered as non-deterministic.  WARNING:
                 defines are not expanded, and cardinality is not
                 considered for sets (even singletons are
                 considered as sets, so the expression should be
                 simplified before calling this function).]

  SideEffects   [The results of the function call is memoized in self]

******************************************************************************/
static boolean
sexp_inliner_is_expr_deterministic(const SexpInliner_ptr self, node_ptr expr)
{
  node_ptr res;
  boolean is_deterministic;

  if (Nil == expr) return true;

  /* checks memoization */
  res = find_assoc(self->hash_is_expr_deterministic, expr);
  if (res != Nil) { /* this expression has been already checked */
    /* here two values are possible : 1 - exp is deterministic, 2 --
       exp is not deterministic */
    switch (PTR_TO_INT(res)) {
    case 1: return true;
    case 2: return false;
    default: internal_error("impossible code");
    }
  }

  if (expr == Nil) return true;

  switch (node_get_type(expr)) {
    /* leaves */
  case FAILURE:
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case BIT:
  case ATOM:
  case DOT:
  case ARRAY:
    return true; /* do not remember constants */

  case TWODOTS:
  case UNION:
    return false; /* do not remember constants */

  case CASE:
  case IFTHENELSE:
    /* ignores conditions, focuses only on values */
    is_deterministic
      = (sexp_inliner_is_expr_deterministic(self, cdr(car(expr))) /*then*/
         &&
         sexp_inliner_is_expr_deterministic(self, cdr(expr)) /*else*/);

  default: /* all other cases */
    is_deterministic
      = (sexp_inliner_is_expr_deterministic(self, car(expr)) /*left*/
         &&
         sexp_inliner_is_expr_deterministic(self, cdr(expr)) /*right*/);
  } /* switch */

  /* remember the result : see at the beginning of fun why numbers are 1 & 2. */
  if (is_deterministic) res = NODE_PTR(1);
  else res = NODE_PTR(2);
  insert_assoc(self->hash_is_expr_deterministic, expr, res);

  return is_deterministic;
}


/**Function********************************************************************

  Synopsis      [Detects if the given expression contains ATTIME nodes.]

  Description   [Expression is assumed to have DEFINEs expanded]

  SideEffects   []

******************************************************************************/
static boolean sexp_inliner_is_expr_timed(node_ptr expr)
{
  if (expr == Nil) return false;

  switch (node_get_type(expr)) {
    /* leaves */
  case FAILURE:
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case BIT:
  case ATOM:
  case DOT:
  case ARRAY:
    return false;

  case ATTIME: return true;

  default: /* all other cases */
    return (sexp_inliner_is_expr_timed(car(expr)) ||
            sexp_inliner_is_expr_timed(cdr(expr)));
  } /* switch */
}


/**Function********************************************************************

  Synopsis      [Converts the given assign to an equivalent SETIN expression]

  Description   [WARNING: init(x) := e is converted to "x in e"]

  SideEffects   []

******************************************************************************/
static Expr_ptr sexp_inliner_assign_to_setin(const SexpInliner_ptr self,
                                             Expr_ptr assign)
{
  SymbTable_ptr symb_table = SexpInliner_get_symb_table(self);

  nusmv_assert(EQDEF == node_get_type(assign));

  if (SMALLINIT == node_get_type(car(assign))) {
    /* gets rid of init */
    return Expr_setin(caar(assign), cdr(assign), symb_table);
  }

  return Expr_setin(car(assign), cdr(assign), symb_table);
}

