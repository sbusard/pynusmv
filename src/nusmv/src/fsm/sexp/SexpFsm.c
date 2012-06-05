/**CFile*****************************************************************

  FileName    [SexpFsm.c]

  PackageName [fsm.sexp]

  Synopsis    [The SexpFsm implementation]

  Description [A SexpFsm instance represents a scalar FSM, but it
               is used also as base class for boolean FSMs which
               are instances of derived class BoolSexpFsm]

  SeeAlso     [SexpFsm.h SexpFsm_private.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.sexp'' package of NuSMV version 2.
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

#include "SexpFsm.h"
#include "SexpFsm_private.h"

#include "sexpInt.h"

/* there are still some variables to be accessed there: */
#include "compile/compile.h"
#include "compile/PredicateNormaliser.h"
#include "compile/symb_table/ResolveSymbol.h"
#include "parser/symbols.h"

#include "utils/error.h"
#include "utils/assoc.h"
#include "utils/utils.h"

#include "sexp/SexpInliner.h"

static char rcsid[] UTIL_UNUSED = "$Id: SexpFsm.c,v 1.1.2.8.4.10.4.45 2010-01-14 17:17:25 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constants declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

Synopsis    [Set to 1 (needs recompilation) to force auto-check of the
             SexpFsm]

Description [Use only in debugging mode, as self-checking can be expensive]

******************************************************************************/
#define SEXP_FSM__ENABLE_SELF_CHECK 0

#if SEXP_FSM__ENABLE_SELF_CHECK
# warning "SexpFsm self-check is enabled: this is *BAD* for performances"
#endif

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Type************************************************************************

  Synopsis [A fsm for a single variable. It is represented as a triple
  of Expr_ptr ]

  Description [Private structure, internally used]

  SeeAlso     []

******************************************************************************/
typedef node_ptr VarFsm_ptr;
#define VAR_FSM(x)  ((VarFsm_ptr) x)


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define _PRINT(txt)                             \
  fprintf(nusmv_stderr, "%s", txt);             \
  fflush(nusmv_stderr)


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void sexp_fsm_hash_var_fsm_init ARGS((SexpFsm_ptr self,
                                             hash_ptr simp_hash));

static void sexp_fsm_const_var_fsm_init ARGS((SexpFsm_ptr self,
                                              hash_ptr simp_hash));

static void sexp_fsm_finalize ARGS((Object_ptr object, void* dummy));

static Object_ptr sexp_fsm_copy ARGS((const Object_ptr object));

static Expr_ptr sexp_fsm_simplify_expr ARGS((SexpFsm_ptr self,
                                             hash_ptr hash, Expr_ptr expr,
                                             const int group));

static hash_ptr simplifier_hash_create ARGS((void));
static void simplifier_hash_destroy ARGS((hash_ptr hash));
static void simplifier_hash_add_expr ARGS((hash_ptr hash,
                                           Expr_ptr expr, const int group));
static boolean simplifier_hash_query_expr ARGS((hash_ptr hash, Expr_ptr expr,
                                                const int group));

static void sexp_fsm_hash_var_fsm_destroy ARGS((SexpFsm_ptr self));
static assoc_retval
sexp_fsm_callback_var_fsm_free ARGS((char *key, char *data, char * arg));
static VarFsm_ptr
sexp_fsm_hash_var_fsm_lookup_var ARGS((SexpFsm_ptr self, node_ptr var));
static void
sexp_fsm_hash_var_fsm_insert_var ARGS((SexpFsm_ptr self,
                                       node_ptr var, VarFsm_ptr varfsm));

static VarFsm_ptr var_fsm_create ARGS((Expr_ptr init, Expr_ptr invar,
                                       Expr_ptr next));

static void var_fsm_destroy ARGS((VarFsm_ptr self));
static Expr_ptr var_fsm_get_init ARGS((VarFsm_ptr self));
static Expr_ptr var_fsm_get_invar ARGS((VarFsm_ptr self));
static Expr_ptr var_fsm_get_next ARGS((VarFsm_ptr self));
static Expr_ptr var_fsm_get_input ARGS((VarFsm_ptr self));
static VarFsm_ptr var_fsm_synchronous_product ARGS((VarFsm_ptr fsm1,
                                                    VarFsm_ptr fsm2));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Costructor for a scalar sexp fsm]

  Description [Given hierarchy will be copied, so the caller is
  responsible for its destruction. Vars set is also copied, so the
  caller is responsible for its destruction (best if frozen)]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr SexpFsm_create(const FlatHierarchy_ptr hierarchy, const Set_t vars)
{
  SexpFsm_ptr self;

  /* allocation: */
  self = ALLOC(SexpFsm, 1);
  SEXP_FSM_CHECK_INSTANCE(self);

  /* initialization: */
  sexp_fsm_init(self, hierarchy, vars);

#if SEXP_FSM__ENABLE_SELF_CHECK
  SexpFsm_self_check(self);
#endif

  return self;
}


/**Function********************************************************************

  Synopsis           [Copy costructor]

  Description        []

  SideEffects        []

******************************************************************************/
VIRTUAL SexpFsm_ptr SexpFsm_copy(const SexpFsm_ptr self)
{
  return SEXP_FSM(Object_copy(OBJECT(self)));
}


/**Function********************************************************************

  Synopsis           [Copy the Sexp FSM and perform predicate-normalisation
  on all the expressions.]

  Description        [Predicate-normalisations means that an expression is
  modified in such a way that at the end the subexpressions of a
  not-boolean expression can be only not-boolean. This is performed by
  changing boolean expression "exp" (which is a subexpression of a
  not-boolean expression) to "ITE(exp, 1, 0)", and then pushing all
  ITE up to the root of not-boolean expressions.

  Constrain: the given Sexp FSM has to be NOT boolean. Otherwise,
  it is meaningless to apply normalisation functions, since all the exporessions
  are already boolean.
  ]

  SideEffects        [SexpFsm_copy]

******************************************************************************/
SexpFsm_ptr
SexpFsm_create_predicate_normalised_copy(const SexpFsm_ptr self,
                                         PredicateNormaliser_ptr normaliser)
{
  SexpFsm_ptr copy;

  SEXP_FSM_CHECK_INSTANCE(self);

  copy = SexpFsm_copy(self);

  FlatHierarchy_set_init(copy->hierarchy,
                         PredicateNormaliser_normalise_expr(normaliser,
                                 FlatHierarchy_get_init(copy->hierarchy)));

  FlatHierarchy_set_invar(copy->hierarchy,
                         PredicateNormaliser_normalise_expr(normaliser,
                                 FlatHierarchy_get_invar(copy->hierarchy)));

  FlatHierarchy_set_trans(copy->hierarchy,
                          PredicateNormaliser_normalise_expr(normaliser,
                                 FlatHierarchy_get_trans(copy->hierarchy)));

  FlatHierarchy_set_input(copy->hierarchy,
                          PredicateNormaliser_normalise_expr(normaliser,
                                 FlatHierarchy_get_input(copy->hierarchy)));

  FlatHierarchy_set_justice(copy->hierarchy,
                            PredicateNormaliser_normalise_expr(normaliser,
                                 FlatHierarchy_get_justice(copy->hierarchy)));

  FlatHierarchy_set_compassion(copy->hierarchy,
                    PredicateNormaliser_normalise_expr(normaliser,
                         FlatHierarchy_get_compassion(copy->hierarchy)));

#if SEXP_FSM__ENABLE_SELF_CHECK
  SexpFsm_self_check(copy);
#endif

  return copy;
}


/**Function********************************************************************

  Synopsis           [Destructor]

  Description        []

  SideEffects        []

******************************************************************************/
VIRTUAL void SexpFsm_destroy(SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}



/**Function********************************************************************

  Synopsis           [Use to check if this FSM is a scalar or boolean fsm]

  Description        [Since a BoolSexpFsm derives from SexpFsm, a SexpFsm
                      is not necessarily a scalar fsm. Use this
                      method to distinguish scalar from boolean fsm
                      when dealing with generic SexpFsm pointers. ]

  SideEffects        []

******************************************************************************/
boolean SexpFsm_is_boolean(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->is_boolean;
}


/**Function********************************************************************

  Synopsis           [Returns the symbol table that is connected to the
  BoolEnc instance connected to self]

  Description [This method can be called only when a valid BddEnc was
  passed to the class constructor (not NULL). Returned instance do not
  belongs to the caller and must _not_ be destroyed]

  SideEffects        []

******************************************************************************/
SymbTable_ptr SexpFsm_get_symb_table(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->st;
}


/**Function********************************************************************

  Synopsis           [Returns the internal complete hierarchy]

  Description [Returned hierarchy belongs to self and cannot be
  freely changed without indirectly modifying self as well. Copy
  the returned hierarchy before modifying it if you do not want to
  change self.  Also, notice that the SexpFsm constructor copies
  the passed hierarchy.]

  SideEffects        []

******************************************************************************/
FlatHierarchy_ptr SexpFsm_get_hierarchy(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->hierarchy;
}


/**Function********************************************************************

  Synopsis           [Returns an Expr that collects init states for all
  variables handled by self]

  Description        []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_init(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return FlatHierarchy_get_init(self->hierarchy);
}


/**Function********************************************************************

  Synopsis           [Returns an Expr that collects invar states for all
  variables handled by self]

  Description        []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_invar(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return FlatHierarchy_get_invar(self->hierarchy);
}


/**Function********************************************************************

  Synopsis           [Returns an Expr that collects all next states for all
  variables handled by self]

  Description        []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_trans(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return FlatHierarchy_get_trans(self->hierarchy);
}


/**Function********************************************************************

  Synopsis           [Returns an Expr that collects all input states for all
  variables handled by self]

  Description        []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_input(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  /* Currently no constraints over input are allowed, thus we return
     true to inidicate this. */
  return FlatHierarchy_get_input(self->hierarchy);
}


/**Function********************************************************************

  Synopsis      [ Gets the sexp expression defining the initial state for
                  the variable "v". ]

  Description   [ Gets the sexp expression defining the initial state for
                  the variable "v". ]

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_var_init(const SexpFsm_ptr self, node_ptr var_name)
{
  VarFsm_ptr var_fsm;
  SEXP_FSM_CHECK_INSTANCE(self);

  var_fsm = find_assoc(self->hash_var_fsm, var_name);
  return var_fsm_get_init(var_fsm);
}


/**Function********************************************************************

  Synopsis      [ Gets the sexp expression defining the state constraints
                  for the variable "v". ]

  Description   [ Gets the sexp expression defining the state constraints
                  for the variable "v". ]

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_var_invar(const SexpFsm_ptr self, node_ptr var_name)
{
  VarFsm_ptr var_fsm;

  SEXP_FSM_CHECK_INSTANCE(self);

  var_fsm = find_assoc(self->hash_var_fsm, var_name);
  return var_fsm_get_invar(var_fsm);
}


/**Function********************************************************************

  Synopsis      [ Gets the sexp expression defining the transition relation
                  for the variable "v". ]

  Description   [ Gets the sexp expression defining the transition relation
                  for the variable "v". ]

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_var_trans(const SexpFsm_ptr self, node_ptr var_name)
{
  VarFsm_ptr var_fsm;

  SEXP_FSM_CHECK_INSTANCE(self);

  var_fsm = find_assoc(self->hash_var_fsm, var_name);
  return var_fsm_get_next(var_fsm);
}


/**Function********************************************************************

  Synopsis      [ Gets the sexp expression defining the input relation
                  for the variable "v". ]

  Description   []

  SideEffects        []

******************************************************************************/
Expr_ptr SexpFsm_get_var_input(const SexpFsm_ptr self, node_ptr var_name)
{
  VarFsm_ptr var_fsm;

  SEXP_FSM_CHECK_INSTANCE(self);

  var_fsm = find_assoc(self->hash_var_fsm, var_name);
  return var_fsm_get_input(var_fsm);
}




/**Function********************************************************************

  Synopsis      [ Gets the list of sexp expressions defining the set of justice
                  constraints for this machine. ]

  Description   [ Gets the list of sexp expressions defining the set of justice
                  constraints for this machine. ]

  SideEffects        []

******************************************************************************/
node_ptr SexpFsm_get_justice(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return FlatHierarchy_get_justice(self->hierarchy);
}


/**Function********************************************************************

  Synopsis      [ Gets the list of sexp expressions defining the set of
                  compassion constraints for this machine. ]

  Description   [ Gets the list of sexp expressions defining the set of
                  compassion constraints for this machine. ]

  SideEffects        []

******************************************************************************/
node_ptr SexpFsm_get_compassion(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return FlatHierarchy_get_compassion(self->hierarchy);
}


/**Function********************************************************************

  Synopsis      [Returns the set of variables in the FSM]

  Description   [Returned instance belongs to self. Do not change not free it.]

  SideEffects   []

******************************************************************************/
NodeList_ptr SexpFsm_get_vars_list(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return Set_Set2List(self->vars_set);
}


/**Function********************************************************************

  Synopsis      [Returns the set of symbols in the FSM]

  Description   [Returned instance belongs to self. Do not change not free it.]

  SideEffects   []

******************************************************************************/
NodeList_ptr SexpFsm_get_symbols_list(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);

  if (NODE_LIST(NULL) == self->symbols) {
    SymbTableIter iter;

    self->symbols = NodeList_create();
    NodeList_concat(self->symbols, Set_Set2List(self->vars_set));

    SYMB_TABLE_FOREACH(self->st, iter, STT_DEFINE) {
      node_ptr def = SymbTable_iter_get_symbol(self->st, &iter);

      nusmv_assert(SymbTable_is_symbol_define(self->st, def));
      NodeList_append(self->symbols, def);
    }
  }

  return self->symbols;
}



/**Function********************************************************************

  Synopsis      [Returns the set of variables in the FSM]

  Description   [Returned instance belongs to self. Do not change not free it.]

  SideEffects   []

******************************************************************************/
Set_t SexpFsm_get_vars(const SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);
  return self->vars_set;
}

/**Function********************************************************************

  Synopsis      [Performs the synchronous product of two FSMs]

  Description   [The result goes into self, no changes to other.]

  SideEffects   [self will change]

******************************************************************************/
void SexpFsm_apply_synchronous_product(SexpFsm_ptr self, SexpFsm_ptr other)
{
  Set_Iterator_t iter;
  node_ptr var;
  VarFsm_ptr fsm_self;
  VarFsm_ptr fsm_other;
  VarFsm_ptr fsm_prod;

  SEXP_FSM_CHECK_INSTANCE(self);
  SEXP_FSM_CHECK_INSTANCE(other);
  nusmv_assert(*(self->family_counter) > 0);

  /* concatenate vars_sets */
  self->vars_set = Set_Union(self->vars_set, other->vars_set);

  /* destroy memoized symbols list */
  if (NODE_LIST(NULL) != self->symbols) {
    NodeList_destroy(self->symbols);
    self->symbols = NODE_LIST(NULL);
  }

  /* Merge const_var_fsm */
  fsm_self = self->const_var_fsm;
  self->const_var_fsm = var_fsm_synchronous_product(self->const_var_fsm,
                                                    other->const_var_fsm);
  var_fsm_destroy(fsm_self);

  /* merge hash_var_fsm
   *
   * [VS] note that the structure of var_fsms (first constrains, then
   * assigns) is not preserved; MR said that's ok */

  SET_FOREACH(self->vars_set, iter) {
    var = Set_GetMember(self->vars_set, iter);

    fsm_self = sexp_fsm_hash_var_fsm_lookup_var(self, var);
    fsm_other = sexp_fsm_hash_var_fsm_lookup_var(other, var);

    fsm_prod = var_fsm_synchronous_product(fsm_self, fsm_other);

    if ((Nil != fsm_self) && (*(self->family_counter) == 1)) {
      var_fsm_destroy(fsm_self);
    }
    sexp_fsm_hash_var_fsm_insert_var(self, var, fsm_prod);
  }

  /* merge hierarchy */
  FlatHierarchy_mergeinto(self->hierarchy, other->hierarchy);

  /* family_counter: we're not a copy anymore - if we're not the only
     instance, get a fresh family_counter and decrease old one */
  if (*(self->family_counter) > 1) {
    *(self->family_counter) -= 1;
    self->family_counter = ALLOC(int, 1);
    nusmv_assert(self->family_counter != (int*) NULL);
    *(self->family_counter) = 1;
  }
}

/**Function********************************************************************

   Synopsis           [Checks if the SexpFsm is syntactically universal]

   Description        [Checks if the SexpFsm is syntactically universal:
                       Checks INIT, INVAR, TRANS, INPUT, JUSTICE,
                       COMPASSION to be empty (ie: True Expr). In this
                       case returns true, false otherwise]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
boolean SexpFsm_is_syntactically_universal(SexpFsm_ptr self)
{
  SEXP_FSM_CHECK_INSTANCE(self);

  Expr_ptr init = SexpFsm_get_init(self);
  Expr_ptr invar = SexpFsm_get_invar(self);
  Expr_ptr trans = SexpFsm_get_trans(self);
  Expr_ptr input = SexpFsm_get_input(self);
  Expr_ptr justice = SexpFsm_get_justice(self);
  Expr_ptr compassion = SexpFsm_get_compassion(self);

  if (Nil != init && !Expr_is_true(init)) { return false; }
  if (Nil != invar && !Expr_is_true(invar)) { return false; }
  if (Nil != trans && !Expr_is_true(trans)) { return false; }
  if (Nil != input && !Expr_is_true(input)) { return false; }
  if (Nil != justice) { return false; }
  if (Nil != compassion) { return false; }

  return true;
}


/**Function********************************************************************

  Synopsis           [Self-check for the instance]

  Description        []

  SideEffects        []

******************************************************************************/
void SexpFsm_self_check(const SexpFsm_ptr self)
{
  FlatHierarchy_self_check(self->hierarchy);
  if (!Set_Contains(self->vars_set, FlatHierarchy_get_vars(self->hierarchy))) {
    internal_error("SexpFsm failed self-check.");
  }
}


/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis      [Initializes the sexp fsm]

  Description [hierarchy is copied into an independent FlatHierarchy
  instance. If the new sexp must be based only on a set of variables, the
  hierarchy must be empty]

  SideEffects        []

******************************************************************************/
void sexp_fsm_init(SexpFsm_ptr self,
                   const FlatHierarchy_ptr hierarchy, const Set_t vars_set)
{
  /* -------------------------------------------------------------------- */
  /* 0. Initialization                                                    */
  /* -------------------------------------------------------------------- */

  /* base class initialization */
  object_init(OBJECT(self));

  /* inits some private members */
  self->st = FlatHierarchy_get_symb_table(hierarchy);
  self->hierarchy = FlatHierarchy_copy(hierarchy);
  self->vars_set = Set_Copy(vars_set);
  self->symbols = NODE_LIST(NULL);

  self->inlining = opt_symb_inlining(OptsHandler_get_instance());
  self->is_boolean = false;

  self->hash_var_fsm = new_assoc();
  self->const_var_fsm = VAR_FSM(NULL);

  self->family_counter = ALLOC(int, 1);
  nusmv_assert(self->family_counter != (int*) NULL);
  *(self->family_counter) = 1; /* this is adam for this family */

  /* -------------------------------------------------------------------- */
  /* 1. Simplification                                                    */
  /* -------------------------------------------------------------------- */
  {
    hash_ptr hash = simplifier_hash_create();
    Set_Iterator_t iter;

    /* sets up and simplifies the single variable FSMs */
    sexp_fsm_hash_var_fsm_init(self, hash);

    /* Sets up and simplifies the constants FSM */
    sexp_fsm_const_var_fsm_init(self, hash);

    /* init */
    FlatHierarchy_set_init(self->hierarchy,
                           sexp_fsm_simplify_expr(self, hash,
                                                  FlatHierarchy_get_init(self->hierarchy), INIT));

    /* invar */
    FlatHierarchy_set_invar(self->hierarchy,
                            sexp_fsm_simplify_expr(self, hash,
                                                   FlatHierarchy_get_invar(self->hierarchy), INVAR));

    /* trans */
    FlatHierarchy_set_trans(self->hierarchy,
                            sexp_fsm_simplify_expr(self, hash,
                                                   FlatHierarchy_get_trans(self->hierarchy), TRANS));

    /* now integrates information coming from the variables FSMs
       (assign and constraints) and the constant FSM */



    /* inits */
    FlatHierarchy_set_init(self->hierarchy,
                           Expr_and_nil(FlatHierarchy_get_init(self->hierarchy),
                                        var_fsm_get_init(self->const_var_fsm)));
    /* invars */
    FlatHierarchy_set_invar(self->hierarchy,
                            Expr_and_nil(FlatHierarchy_get_invar(self->hierarchy),
                                         var_fsm_get_invar(self->const_var_fsm)));
    /* next */
    FlatHierarchy_set_trans(self->hierarchy,
                            Expr_and_nil(FlatHierarchy_get_trans(self->hierarchy),
                                         var_fsm_get_next(self->const_var_fsm)));

    SET_FOREACH(self->vars_set, iter) {
      node_ptr var = Set_GetMember(self->vars_set, iter);
      VarFsm_ptr varfsm = sexp_fsm_hash_var_fsm_lookup_var(self, var);

      if (varfsm != VAR_FSM(NULL)) {
        Expr_ptr tmp;

        /* inits */
        tmp = var_fsm_get_init(varfsm);
        FlatHierarchy_set_init(self->hierarchy,
                Expr_and_nil(FlatHierarchy_get_init(self->hierarchy),
                             tmp));
        /* invars */
        tmp = var_fsm_get_invar(varfsm);
        FlatHierarchy_set_invar(self->hierarchy,
                      Expr_and_nil(FlatHierarchy_get_invar(self->hierarchy),
                                   tmp));
        /* next */
        tmp = var_fsm_get_next(varfsm);
        FlatHierarchy_set_trans(self->hierarchy,
                      Expr_and_nil(FlatHierarchy_get_trans(self->hierarchy),
                                   tmp));
      }
    } /* loop over vars */

    simplifier_hash_destroy(hash);
  }

  /* -------------------------------------------------------------------- */
  /* 2. Inlining                                                          */
  /* -------------------------------------------------------------------- */
  if (self->inlining) {
    SexpInliner_ptr inliner;
    node_ptr invar, trans, init;
    InlineRes_ptr invar_res, init_res, trans_res;

    /* Create a fixpoint inliner */
    inliner = SexpInliner_create(self->st, 0);

    /* Get the non-inlined expressions */
    invar = FlatHierarchy_get_invar(self->hierarchy);
    init = FlatHierarchy_get_init(self->hierarchy);
    trans = FlatHierarchy_get_trans(self->hierarchy);

    /* Inline invariant */
    invar_res = SexpInliner_inline(inliner, invar, NULL);

    /* Inline init without clearing the inliner since invariants
       equivalences still hold */
    init_res = SexpInliner_inline(inliner, init, NULL);

    /* Reset inliner */
    SexpInliner_clear_equivalences(inliner);
    SexpInliner_clear_invariants(inliner);

    /* Re-learn invariant equivalences */
    SexpInliner_force_equivalences(inliner,
                                   InlineRes_get_equivalences(invar_res));
    SexpInliner_force_invariants(inliner, InlineRes_get_invariants(invar_res));

    /* Inline trans (with re-learned invariants) */
    trans_res = SexpInliner_inline(inliner, trans, NULL);

    /* Set inlined expressions */
    FlatHierarchy_set_invar(self->hierarchy, InlineRes_get_result(invar_res));
    FlatHierarchy_set_init(self->hierarchy, InlineRes_get_result(init_res));
    FlatHierarchy_set_trans(self->hierarchy, InlineRes_get_result(trans_res));

    /* Cleanup */
    InlineRes_destroy(invar_res);
    InlineRes_destroy(init_res);
    InlineRes_destroy(trans_res);
    SexpInliner_destroy(inliner);
  }

  /* -------------------------------------------------------------------- */
  /* 3. Other initializations                                             */
  /* -------------------------------------------------------------------- */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = sexp_fsm_finalize;
  OVERRIDE(Object, copy) = sexp_fsm_copy;
}


/**Function********************************************************************

  Synopsis      [Initializes the vars fsm hash]

  Description   []

  SideEffects        []

******************************************************************************/
void sexp_fsm_deinit(SexpFsm_ptr self)
{
  nusmv_assert(*(self->family_counter) > 0);
  *(self->family_counter) -= 1; /* self de-ref */

  if (VAR_FSM(NULL) != self->const_var_fsm) {
    var_fsm_destroy(self->const_var_fsm);
  }

  sexp_fsm_hash_var_fsm_destroy(self);
  FlatHierarchy_destroy(self->hierarchy);
  Set_ReleaseSet(self->vars_set);

  if (*(self->family_counter) == 0) {
    FREE(self->family_counter);
    self->family_counter = (int*) NULL;
  }
}


/**Function********************************************************************

  Synopsis      [private service for copying self to other]

  Description   []

  SideEffects   []

******************************************************************************/
void sexp_fsm_copy_aux(const SexpFsm_ptr self, SexpFsm_ptr copy)
{
  /* copies the base class: */
  object_copy_aux(OBJECT(self), OBJECT(copy));

  /* copies private members */
  copy->st = self->st;
  copy->vars_set   = Set_Copy(self->vars_set);
  copy->symbols    = NODE_LIST(NULL);
  copy->hierarchy = FlatHierarchy_copy(self->hierarchy);


  /* Since one varfsm entry is nothing else than a sequence of nodes
     created with cons, if a side-effect is performed on one of the
     two FSMs, the node changes for both. (And the same for node
     free!) */
  copy->hash_var_fsm = copy_assoc(self->hash_var_fsm);
  copy->const_var_fsm = var_fsm_create(var_fsm_get_init(self->const_var_fsm),
                                       var_fsm_get_invar(self->const_var_fsm),
                                       var_fsm_get_next(self->const_var_fsm));

  copy->inlining = self->inlining;
  copy->is_boolean = self->is_boolean;

  /* increments family members */
  copy->family_counter = self->family_counter;
  *(self->family_counter) += 1;

  /* copies local virtual methods */
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis      [This is called by the virtual copy constructor]

  Description   []

  SideEffects   []

******************************************************************************/
static Object_ptr sexp_fsm_copy(const Object_ptr object)
{
  SexpFsm_ptr self = SEXP_FSM(object);
  SexpFsm_ptr copy;

  SEXP_FSM_CHECK_INSTANCE(self);

  copy = ALLOC(SexpFsm, 1);
  SEXP_FSM_CHECK_INSTANCE(copy);

  sexp_fsm_copy_aux(self, copy);
  return OBJECT(copy);
}


/**Function********************************************************************

  Synopsis    [The SexpFsm class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void sexp_fsm_finalize(Object_ptr object, void* dummy)
{
  SexpFsm_ptr self = SEXP_FSM(object);

  sexp_fsm_deinit(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis      [Initializes the const_var_fsm field]

  Description   [Formulae are simplified through
                 sexp_fsm_simplify_expr. For this reason a
                 simplification hash is required as input]

  SideEffects   []

******************************************************************************/
static void sexp_fsm_const_var_fsm_init(SexpFsm_ptr self, hash_ptr simp_hash)
{
  node_ptr invar_const =
    FlatHierarchy_lookup_constant_constrains(self->hierarchy, INVAR);
  node_ptr trans_const =
    FlatHierarchy_lookup_constant_constrains(self->hierarchy, TRANS);
  node_ptr init_const =
    FlatHierarchy_lookup_constant_constrains(self->hierarchy, INIT);

  if (invar_const == Nil) { invar_const = Expr_true(); }
  if (trans_const == Nil) { trans_const = Expr_true(); }
  if (init_const == Nil) { init_const = Expr_true(); }

  invar_const = sexp_fsm_simplify_expr(self, simp_hash, invar_const, INVAR);
  init_const = sexp_fsm_simplify_expr(self, simp_hash, init_const, INIT);
  trans_const = sexp_fsm_simplify_expr(self, simp_hash, trans_const, TRANS);

  self->const_var_fsm = var_fsm_create(init_const, invar_const, trans_const);
}

/**Function********************************************************************

  Synopsis      [Initializes the vars fsm hash]

  Description   [Formulae are simplified through
                 sexp_fsm_simplify_expr. For this reason a
                 simplification hash is required as input]

  SideEffects   []

******************************************************************************/
static void sexp_fsm_hash_var_fsm_init(SexpFsm_ptr self, hash_ptr simp_hash)
{
  Set_Iterator_t iter;
  SymbTable_ptr symb_table = SexpFsm_get_symb_table(self);

  SET_FOREACH(self->vars_set, iter) {
    int saved_yylineno = yylineno;

    VarFsm_ptr var_fsm;

    node_ptr var_name = Set_GetMember(self->vars_set, iter);
    node_ptr var_name_i = find_node(SMALLINIT, var_name, Nil);
    node_ptr var_name_n = Expr_next(var_name, symb_table);

    node_ptr init_a =
      FlatHierarchy_lookup_assign(self->hierarchy, var_name_i);

    node_ptr invar_a =
      FlatHierarchy_lookup_assign(self->hierarchy, var_name);

    node_ptr next_a =
      FlatHierarchy_lookup_assign(self->hierarchy, var_name_n);

    node_ptr init_c =
      FlatHierarchy_lookup_constrains(self->hierarchy, var_name_i);

    node_ptr invar_c =
      FlatHierarchy_lookup_constrains(self->hierarchy, var_name);

    node_ptr next_c =
      FlatHierarchy_lookup_constrains(self->hierarchy, var_name_n);

    Expr_ptr init_sexp  = Expr_true();
    Expr_ptr invar_sexp = Expr_true();
    Expr_ptr trans_sexp = Expr_true();

    /* add all the constrains */
    if (Nil != init_c) init_sexp = Expr_and(init_sexp, EXPR(init_c));
    if (Nil != invar_c) invar_sexp = Expr_and(invar_sexp, EXPR(invar_c));
    if (Nil != next_c)  trans_sexp = Expr_and(trans_sexp, EXPR(next_c));

    /* add all assignments */
    if (Nil != init_a) {
      yylineno = init_a->lineno;
      init_sexp = Expr_and(init_sexp,
                           EXPR(find_node(EQDEF, var_name_i, init_a)));
    }

    if (Nil != invar_a) {
      yylineno = invar_a->lineno;
      invar_sexp = Expr_and(invar_sexp,
                            EXPR(new_node(EQDEF, var_name, invar_a)));
    }

    if (Nil != next_a) {
      yylineno = next_a->lineno;
      trans_sexp = Expr_and(trans_sexp,
                            EXPR(new_node(EQDEF, var_name_n, next_a)));
    }

    /* simplification */
    init_sexp = sexp_fsm_simplify_expr(self, simp_hash, init_sexp, INIT);
    invar_sexp = sexp_fsm_simplify_expr(self, simp_hash, invar_sexp, INVAR);
    trans_sexp = sexp_fsm_simplify_expr(self, simp_hash, trans_sexp, TRANS);

    /* inserts the var fsm inside the hash table */
    var_fsm = var_fsm_create(init_sexp, invar_sexp, trans_sexp);
    sexp_fsm_hash_var_fsm_insert_var(self, var_name, var_fsm);

    yylineno = saved_yylineno;
  } /* loop */
}


/**Function********************************************************************

  Synopsis           [removes duplicates from expression containing AND nodes]

  Description        [group identifies INVAR, TRANS or INIT group.]

  SideEffects        []

******************************************************************************/
static Expr_ptr
sexp_fsm_simplify_expr(SexpFsm_ptr self, hash_ptr hash, Expr_ptr expr,
                       const int group)
{
  Expr_ptr result;

  if ((expr == EXPR(NULL)) || simplifier_hash_query_expr(hash, expr, group)) {
    result = Expr_true();
  }
  else {
    switch (node_get_type(NODE_PTR(expr))) {
    case AND:
      {
        Expr_ptr left = sexp_fsm_simplify_expr(self, hash, car(NODE_PTR(expr)),
                                               group);
        Expr_ptr right = sexp_fsm_simplify_expr(self, hash, cdr(NODE_PTR(expr)),
                                                group);
        result = Expr_and(left, right);
        break;
      }

    default:
      result = expr;
    } /* switch */

    simplifier_hash_add_expr(hash, expr, group);
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [This is used when creating cluster list from vars list]

  Description        []

  SideEffects        []

******************************************************************************/
static hash_ptr simplifier_hash_create()
{
  hash_ptr result;

  result = st_init_table(st_ptrcmp, st_ptrhash);
  nusmv_assert(result != ((hash_ptr) NULL));

  return result;
}

/**Function********************************************************************

  Synopsis           [Call after sexp_fsm_cluster_hash_create]

  Description        []

  SideEffects        []

******************************************************************************/
static void simplifier_hash_destroy(hash_ptr hash)
{
  nusmv_assert(hash != (hash_ptr) NULL);
  st_free_table(hash);
}


/**Function********************************************************************

  Synopsis           [To insert a new node in the hash]

  Description        [group is INIT, INVAR or TRANS]

  SideEffects        [The hash can change]

******************************************************************************/
static void
simplifier_hash_add_expr(hash_ptr hash, Expr_ptr expr, const int group)
{
  int res;

  res = st_add_direct(hash, (char*) expr, PTR_FROM_INT(char*, group));
  nusmv_assert(res != ST_OUT_OF_MEM);
}


/**Function********************************************************************

  Synopsis           [Queries for an element in the hash, returns True if
  found]

  Description        []

  SideEffects        []

******************************************************************************/
static boolean
simplifier_hash_query_expr(hash_ptr hash, Expr_ptr expr,
                           const int group)
{
  nusmv_ptrint hashed_group;
  boolean result;

  result = st_lookup(hash, (char*) expr, (char**) &hashed_group);

  /* groups are checked consecutively, i.e. at first, *all* INIT expressions
     are checked, then *all* INVAR, and then *all* TRANS. So hash_group
     will not interfere with each other
  */
  return (result && ((int) hashed_group == group));
}


/**Function********************************************************************

  Synopsis           [Call to destroy the var fsm hash]

  Description        [Private method, used internally]

  SideEffects        []

******************************************************************************/
static void sexp_fsm_hash_var_fsm_destroy(SexpFsm_ptr self)
{
  nusmv_assert(self->hash_var_fsm != (hash_ptr) NULL);

  if (*(self->family_counter) == 0) {
    clear_assoc_and_free_entries(self->hash_var_fsm,
                                 sexp_fsm_callback_var_fsm_free);
  }
  free_assoc(self->hash_var_fsm);
}


/**Function********************************************************************

  Synopsis [Private callback that destroys a single variable fsm
  contained into the var fsm hash]

  Description        []

  SideEffects        []

******************************************************************************/
static assoc_retval sexp_fsm_callback_var_fsm_free(char *key,
                                                   char *data, char * arg)
{
  VarFsm_ptr varfsm = VAR_FSM(data);

  var_fsm_destroy(varfsm);
  return ASSOC_DELETE;
}


/**Function********************************************************************

  Synopsis [Given a variable name, returns the corresponding variable
  fsm, or NULL if not found]

  Description        []

  SideEffects        []

******************************************************************************/
static VarFsm_ptr
sexp_fsm_hash_var_fsm_lookup_var(SexpFsm_ptr self, node_ptr var)
{
  nusmv_assert(self->hash_var_fsm != (hash_ptr) NULL);

  return VAR_FSM( find_assoc(self->hash_var_fsm, var) );
}


/**Function********************************************************************

  Synopsis           [Adds a var fsm to the internal hash. Private.]

  Description        []

  SideEffects        []

******************************************************************************/
static void
sexp_fsm_hash_var_fsm_insert_var(SexpFsm_ptr self,
                                 node_ptr var, VarFsm_ptr varfsm)
{
  nusmv_assert(self->hash_var_fsm != (hash_ptr) NULL);

  insert_assoc(self->hash_var_fsm, var, varfsm);
}


/**Function********************************************************************

  Synopsis           [Creates a var fsm]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static VarFsm_ptr var_fsm_create(Expr_ptr init,
                                 Expr_ptr invar,
                                 Expr_ptr next)
{
  return VAR_FSM( cons(NODE_PTR(init),
                       cons(NODE_PTR(invar), NODE_PTR(next))) );
}


/**Function********************************************************************

  Synopsis           [It does not destroy the init, trans and invar nodes.
  It destroys only the support nodes]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void var_fsm_destroy(VarFsm_ptr self)
{
  node_ptr node = NODE_PTR(self);

  free_node(cdr(node));
  free_node(node);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Expr_ptr var_fsm_get_init(VarFsm_ptr self)
{
  return EXPR( car(NODE_PTR(self)) );
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Expr_ptr var_fsm_get_invar(VarFsm_ptr self)
{
  return EXPR( car(cdr(NODE_PTR(self))) );
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Expr_ptr var_fsm_get_next(VarFsm_ptr self)
{
  return EXPR( cdr(cdr(NODE_PTR(self))) );
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Expr_ptr var_fsm_get_input(VarFsm_ptr self)
{
  /* Currently no constraints over input are allowed, thus we return
     true to inidicate this. */
  return Expr_true();
}

/**Function********************************************************************

  Synopsis           [Returns new var fsm that is synchronous product of var
  fsms.]

  Description        [Any argument can be Nil. When both are Nil the product
  has all arguments true.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static VarFsm_ptr var_fsm_synchronous_product(VarFsm_ptr fsm1,
                                              VarFsm_ptr fsm2)
{
  VarFsm_ptr prod;
  node_ptr prod_init;
  node_ptr prod_invar;
  node_ptr prod_next;

  if (Nil == fsm1 && Nil == fsm2) {
    prod_init = Expr_true();
    prod_invar = Expr_true();
    prod_next = Expr_true();
  } else if (Nil == fsm1) {
    prod_init = var_fsm_get_init(fsm2);
    prod_invar = var_fsm_get_invar(fsm2);
    prod_next = var_fsm_get_next(fsm2);
  } else if (Nil == fsm2) {
    prod_init = var_fsm_get_init(fsm1);
    prod_invar = var_fsm_get_invar(fsm1);
    prod_next = var_fsm_get_next(fsm1);
  } else {
    prod_init = NODE_PTR(Expr_and_nil(EXPR(var_fsm_get_init(fsm1)),
                                      EXPR(var_fsm_get_init(fsm2))));
    prod_invar = NODE_PTR(Expr_and_nil(EXPR(var_fsm_get_invar(fsm1)),
                                       EXPR(var_fsm_get_invar(fsm2))));
    prod_next = NODE_PTR(Expr_and_nil(EXPR(var_fsm_get_next(fsm1)),
                                      EXPR(var_fsm_get_next(fsm2))));
  }

  prod = var_fsm_create(prod_init,
                        prod_invar,
                        prod_next);
  nusmv_assert(NULL != prod);

  return prod;
}
