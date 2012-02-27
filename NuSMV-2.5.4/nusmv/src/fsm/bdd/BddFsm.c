/**CFile***********************************************************************

   FileName    [BddFsm.c]

   PackageName [fsm.bdd]

   Synopsis    [Defines the public interface for the class BddFsm]

   Description [A BddFsm is a Finite State Machine (FSM) whose building blocks
   (the set of initial state, the transition relation, the set of
   constraints on inputs and so on) are represented by means of
   BDD data structures, and whose capabilities are based on
   operations upon and between BDDs as well.

   Note: a state is an assignment to state and frozen variables.
   an input is an assigment to input variables.]

   Author      [Roberto Cavada, Marco Benedetti]

   Copyright   [
   This file is part of the ``fsm.bdd'' package of NuSMV version 2.
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

#include <math.h>

#include "bddInt.h"
#include "BddFsm.h"
#include "FairnessList.h"

#include "compile/compile.h"
#include "compile/symb_table/SymbTable.h"
#include "enc/enc.h"
#include "utils/utils_io.h"
#include "utils/error.h"


static char rcsid[] UTIL_UNUSED = "$Id: BddFsm.c,v 1.1.2.44.4.12.4.29 2010-03-02 08:45:22 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct BddFsm_TAG
{
  /* private members */
  DdManager*  dd;
  SymbTable_ptr symb_table;
  BddEnc_ptr  enc;

  BddStates      init;

  BddInvarStates invar_states;
  BddInvarInputs invar_inputs;

  BddTrans_ptr trans;

  JusticeList_ptr    justice;
  CompassionList_ptr compassion;

  BddFsmCache_ptr cache;
} BddFsm;

/* ---------------------------------------------------------------------- */
/*                     Static functions prototypes                        */
/* ---------------------------------------------------------------------- */

static void bdd_fsm_init ARGS((BddFsm_ptr self, BddEnc_ptr encoding,
                               BddStates init, BddInvarStates invar_states,
                               BddInvarInputs invar_inputs,
                               BddTrans_ptr trans,
                               JusticeList_ptr justice,
                               CompassionList_ptr compassion));

static void bdd_fsm_copy ARGS((const BddFsm_ptr self, BddFsm_ptr copy));

static void bdd_fsm_deinit ARGS((BddFsm_ptr self));

static void bdd_fsm_compute_reachable_states ARGS((BddFsm_ptr self));

static BddStatesInputs
bdd_fsm_get_legal_state_input ARGS((BddFsm_ptr self));

/* The new code for fairness */
static BddStatesInputs
bdd_fsm_get_fair_or_revfair_states_inputs ARGS((BddFsm_ptr self,
                                                BddFsm_dir dir));

static BddStatesInputs
bdd_fsm_get_fair_or_revfair_states_inputs_in_subspace
ARGS((const BddFsm_ptr self, BddStatesInputs subspace, BddFsm_dir dir));

static BddStatesInputs
bdd_fsm_compute_EL_SI_subset ARGS((const BddFsm_ptr self,
                                   BddStatesInputs subspace,
                                   BddFsm_dir dir));

static BddStatesInputs
bdd_fsm_compute_EL_SI_subset_aux ARGS((const BddFsm_ptr self,
                                       BddStatesInputs states,
                                       BddStatesInputs subspace,
                                       BddFsm_dir dir));


static void bdd_fsm_check_init_state_invar_emptiness
ARGS((const BddFsm_ptr self));
static void bdd_fsm_check_fairness_emptiness ARGS((const BddFsm_ptr self));



/* ---------------------------------------------------------------------- */
/*                          public methods                                */
/* ---------------------------------------------------------------------- */


/**Function********************************************************************

   Synopsis           [Constructor for BddFsm]

   Description        [All given bdd are referenced.
   self becomes the owner of given trans, justice and compassion objects,
   whereas the encoding is owned by the caller]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddFsm_ptr BddFsm_create(BddEnc_ptr encoding,
                         BddStates init,
                         BddInvarStates invar_states,
                         BddInvarInputs invar_inputs,
                         BddTrans_ptr trans,
                         JusticeList_ptr justice,
                         CompassionList_ptr compassion)
{
  BddFsm_ptr self = ALLOC( BddFsm, 1 );
  BDD_FSM_CHECK_INSTANCE(self);

  bdd_fsm_init(self, encoding, init, invar_states, invar_inputs,
               trans, justice, compassion);

  return self;
}



/**Function********************************************************************

   Synopsis           [Destructor of class BddFsm]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddFsm_destroy(BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);

  bdd_fsm_deinit(self);
  FREE(self);
}


/**Function********************************************************************

   Synopsis           [Copy constructor for BddFsm]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddFsm_ptr BddFsm_copy(const BddFsm_ptr self)
{
  BddFsm_ptr copy;

  BDD_FSM_CHECK_INSTANCE(self);

  copy = ALLOC( BddFsm, 1 );
  BDD_FSM_CHECK_INSTANCE(copy);

  bdd_fsm_copy(self, copy);

  return copy;
}



/**Function********************************************************************

   Synopsis           [Copies cached information of 'other' into self]

   Description [Copies cached information (reachable states, levels,
   fair states, etc.) possibly previoulsy calculated by 'other' into
   self.  Call this method when self is qualitatively identical to
   'other', but for some reason the trans is organized
   differently. Call to reuse still valid information calculated by
   'other' into self. If keep_family is true, the cache will be reused
   and not copied, meaning that self will belong to the same family as
   'other'. In this case a change in 'other' will have effects also on
   self (and viceversa). Notice that previoulsy calculated information
   into 'self' will be lost after the copy.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddFsm_copy_cache(BddFsm_ptr self, const BddFsm_ptr other,
                       boolean keep_family)
{
  BDD_FSM_CHECK_INSTANCE(self);

  BddFsmCache_destroy(self->cache);
  if (keep_family) self->cache = BddFsmCache_soft_copy(other->cache);
  else self->cache = BddFsmCache_hard_copy(other->cache);
}


/**Function********************************************************************

   Synopsis           [Getter for justice list]

   Description        [self keeps the ownership of the returned object]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
JusticeList_ptr BddFsm_get_justice(const BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);
  return self->justice;
}


/**Function********************************************************************

   Synopsis           [Getter for compassion list]

   Description        [self keeps the ownership of the returned object]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
CompassionList_ptr BddFsm_get_compassion(const BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);
  return self->compassion;
}

/**Function********************************************************************

   Synopsis           [Getter for init]

   Description        [Returned bdd is referenced]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddStates BddFsm_get_init(const BddFsm_ptr self)
{
  bdd_ptr res;

  BDD_FSM_CHECK_INSTANCE(self);

  res = bdd_dup((bdd_ptr) self->init);
  return BDD_STATES(res);
}


/**Function********************************************************************

   Synopsis           [Getter for state constraints]

   Description        [Returned bdd is referenced]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddInvarStates BddFsm_get_state_constraints(const BddFsm_ptr self)
{
  bdd_ptr res;

  BDD_FSM_CHECK_INSTANCE(self);

  res = bdd_dup( (bdd_ptr) self->invar_states );
  return BDD_INVAR_STATES(res);
}


/**Function********************************************************************

   Synopsis           [Getter for input constraints]

   Description        [Returned bdd is referenced]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddInvarInputs BddFsm_get_input_constraints(const BddFsm_ptr self)
{
  bdd_ptr res;

  BDD_FSM_CHECK_INSTANCE(self);

  res = bdd_dup( (bdd_ptr) self->invar_inputs );
  return BDD_INVAR_INPUTS(res);
}


/**Function********************************************************************

   Synopsis           [Getter for the trans]

   Description        [Returned Trans instance is not copied, do not destroy
   it, since self keeps the ownership.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddTrans_ptr BddFsm_get_trans(const BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);

  return self->trans;
}


/**Function********************************************************************

   Synopsis [Returns the be encoding associated with the given fsm
   instance]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddEnc_ptr BddFsm_get_bdd_encoding(const BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);
  return self->enc;
}


/**Function********************************************************************

   Synopsis [Returns the cached reachable states]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
boolean BddFsm_get_cached_reachable_states(const BddFsm_ptr self,
                                           BddStates** layers,
                                           int* size)
{
  BDD_FSM_CHECK_INSTANCE(self);

  *layers = CACHE_GET(reachable.layers);
  *size = CACHE_GET(reachable.diameter);

  return CACHE_GET(reachable.computed);
}

/**Function********************************************************************

   Synopsis [Sets the whole set of reachable states for this FSM, with
   no onion ring informations]

   Description [Sets the whole set of reachable states for this FSM, with
   no onion ring informations]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddFsm_set_reachable_states(const BddFsm_ptr self,
                                 BddStates reachable)
{
  BDD_FSM_CHECK_INSTANCE(self);
  BddFsmCache_set_reachable_states(self->cache, reachable);
}

/**Function********************************************************************

   Synopsis           [Checks if the set of reachable states exists in the FSM]

   Description        [Checks if the set of reachable states exists in the FSM]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
boolean BddFsm_has_cached_reachable_states(const BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);
  return !CACHE_IS_EQUAL(reachable.reachable_states, (bdd_ptr)NULL);
}


/**Function********************************************************************

   Synopsis [Updates the cached reachable states]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddFsm_update_cached_reachable_states(const BddFsm_ptr self,
                                           node_ptr layers_list,
                                           int size,
                                           boolean completed)
{
  BDD_FSM_CHECK_INSTANCE(self);

  BddFsmCache_set_reachables(self->cache, layers_list, size, completed);
}

/**Function********************************************************************

   Synopsis     [Returns true if the set of reachable states has already been
   computed]

   Description  [
   Note: a state is represented by state and frozen variables.]
   computed]

   SideEffects  []

   SeeAlso      []

******************************************************************************/
boolean BddFsm_reachable_states_computed(BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);

  if ( CACHE_IS_EQUAL(reachable.computed, false) ) return false;
  else return true;
}


/**Function********************************************************************

   Synopsis     [Gets the set of reachable states of this machine]

   Description  [Returned bdd is referenced.

   This method returns the set R of reachable states,
   i.e.  those states that can be actually reached
   starting from one of the initial state.

   R is the set of states such that "i TRC s" holds for
   some state i in the set of initial states, where TRC
   is the transitive closure of the conjunction of the
   transition relation of the machine with the set of
   invar states, the set of constraints on inputs and the
   set of state/input constraints.

   R is computed by this method in a forward manner by
   exploiting the "BddFsm_get_forward_image" method
   during a fixpoint calculation. In particular, R is
   computed by reaching the fixpoint on the functional
   that maps S onto the forward image
   BddFsm_get_forward_image(S) of S, where the
   computation is started from the set of initial states.
   Notice that the set of invar states, the set of
   constraints on inputs and the set of state/input
   constrains are implicitly taken into account by
   BddFsm_get_forward_image(S).

   Note: a state is represented by state and frozen variables.]

   SideEffects  [Internal cache could change]

   SeeAlso      []

******************************************************************************/
BddStates BddFsm_get_reachable_states(BddFsm_ptr self)
{
  BddStates res;

  BDD_FSM_CHECK_INSTANCE(self);

  /* If we already have the complete set of reachable states, return
     it. */
  if (BddFsm_has_cached_reachable_states(self)) {
    res = CACHE_GET(reachable.reachable_states);
  }
  /* Otherwise compute them, if necessary */
  else {

    if ( CACHE_IS_EQUAL(reachable.computed, false) ) {
      bdd_fsm_compute_reachable_states(self);
    }

    if (CACHE_GET(reachable.diameter) > 0) {
      res = CACHE_GET(reachable.layers[CACHE_GET(reachable.diameter) - 1]);
    }
    else {
      res = bdd_false(self->dd);
    }

  }

  bdd_ref((bdd_ptr) res);

  return res;
}


/**Function********************************************************************

   Synopsis     [Copies reachable states of 'other' into 'self']

   Description [This method can be called when reachable states among
   FSMs can be reused, for example when other's reachable states are an
   over-extimation of self's. Parameter force_calculation forces the
   calculation of the reachable states of 'other' if needed (i.e. not
   previoulsy calculated).

   The two FSMs are allowed to belong to the same family. If parameter
   keep_family is true, than the original FSM's family will not change,
   and all the family's members (all the FSMs that have a common
   relative) will have their reachable states changed
   accordingly. Otherwise, self will be detached by its own original
   family (originating a new one), and all relatives will be not
   changed.]

   SideEffects  [Internal cache could change of both self and other]

   SeeAlso      []

******************************************************************************/
void BddFsm_copy_reachable_states(BddFsm_ptr self, BddFsm_ptr other,
                                  boolean keep_family,
                                  boolean force_calculation)
{
  BDD_FSM_CHECK_INSTANCE(self);

  /* computes reachables in other if needed */
  if (!other->cache->reachable.computed && force_calculation) {
    bdd_fsm_compute_reachable_states(other);
  }

  if (!keep_family && *(self->cache->family_counter) > 1) {
    /* the cache is shared and must be copied to detach it */
    BddFsmCache_ptr new_cache = BddFsmCache_hard_copy(self->cache);
    BddFsmCache_destroy(self->cache);
    self->cache = new_cache;
  }

  BddFsmCache_copy_reachables(self->cache, other->cache);
}


/**Function********************************************************************

   Synopsis     [Returns the set of reachable states at a given distance]

   Description  [Computes the set of reachable states if not previously,
   cached. Returned bdd is referenced.

   If distance is greater than the diameter, an assertion
   is fired.

   This method returns the set R of states of this
   machine which can be reached in exactly "distance"
   steps by applying the "BddFsm_get_forward_image"
   method ("distance" times) starting from one of
   the initial states (and cannot be reached with less
   than "distance" steps).

   In the case that the distance is less than 0, the
   empty-set is returned.

   These states are computed as intermediate steps of the
   fixpoint characterization given in the
   "BddFsm_get_reachable_states" method.

   Note: a state is represented by state and frozen variables.]

   SideEffects  [Internal cache could change]

******************************************************************************/
BddStates BddFsm_get_reachable_states_at_distance(BddFsm_ptr self,
                                                  int distance)
{
  BddStates res;

  BDD_FSM_CHECK_INSTANCE(self);

  res = BDD_STATES(NULL);

  if (distance >= 0) {
    int diameter;

    if (CACHE_IS_EQUAL(reachable.computed, false)) {
      bdd_fsm_compute_reachable_states(self);
    }

    diameter = CACHE_GET(reachable.diameter);

    /* checks distance */
    nusmv_assert(distance < diameter);

    res = CACHE_GET_BDD(reachable.layers[distance]);
    /* Compute the distance-th frontier if we are not looking for the
       initial state */
    if (distance >= 1) {
      bdd_ptr x, neg;

      x = CACHE_GET_BDD(reachable.layers[distance - 1]);
      neg = bdd_not(self->dd, x);

      bdd_and_accumulate(self->dd, &res, neg);

      bdd_free(self->dd, x);
      bdd_free(self->dd, neg);
    }
  }

  /* checks if assigned: */
  if (res == BDD_STATES(NULL)) {
    res = BDD_STATES(bdd_false(self->dd));
  }

  return res;
}

/**Function********************************************************************

   Synopsis     [Returns a bdd that represents the monolithic
   transition relation]

   Description  [This method returns a monolithic representation of
   the transition relation, which is computed on the
   basis of the internal partitioned representation by
   composing all the element of the partition.

   Returned bdd is referenced.]

   SideEffects  [Internal cache could change]

   SeeAlso      []

******************************************************************************/
bdd_ptr BddFsm_get_monolithic_trans_bdd(BddFsm_ptr self)
{
  bdd_ptr res;

  BDD_FSM_CHECK_INSTANCE(self);

  if ( CACHE_IS_EQUAL(monolithic_trans, (bdd_ptr) NULL) ) {
    res = BddTrans_get_monolithic_bdd(self->trans);

    CACHE_SET_BDD(monolithic_trans, res);
    bdd_free(self->dd, res);
  }

  res = CACHE_GET_BDD(monolithic_trans);
  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the distance of a given set of states from initial
   states]

   Description  [Computes the set of reachable states if not previously cached.
   Returns -1 if given states set is not reachable.

   This method returns an integer which represents the
   distance of the farthest state in "states". The
   distance of one single state "s" is the number of
   applications of the "BddFsm_get_forward_image"
   method (starting from the initial set of states)
   which is necessary and sufficient to end up with a set
   of states containing "s". The distance of a *set* of
   states "set" is the maximal distance of states in
   "set", i.e. the number of applications of the
   "BddFsm_get_forward_image" method (starting from the
   initial set of states) which is necessary and
   sufficient to reach at least once (not necessarily
   during the last application, but somewhere along the
   way) each state in "set".

   So, the distance of a set of states is a max-min
   function.
   Could update the cache.]

   SideEffects  [Internal cache could change]

******************************************************************************/
int BddFsm_get_distance_of_states(BddFsm_ptr self, BddStates states)
{
  bdd_ptr constr_states;
  int i;
  int diameter;
  int result = -1;

  BDD_FSM_CHECK_INSTANCE(self);

  if (CACHE_IS_EQUAL(reachable.computed, false)) {
    bdd_fsm_compute_reachable_states(self);
  }

  /* applies state constraints: */
  constr_states = bdd_and(self->dd,
                          (bdd_ptr) states,
                          (bdd_ptr) self->invar_states);

  diameter = CACHE_GET(reachable.diameter);

  for (i=0; i<diameter; ++i) {
    bdd_ptr Ri = (bdd_ptr) BddFsm_get_reachable_states_at_distance(self, i);
    int entailed = bdd_entailed(self->dd, constr_states, Ri);

    if (entailed == 1) {
      bdd_free(self->dd, Ri);
      result = i;
      break;
    }

    bdd_free(self->dd, Ri);
  }

  bdd_free(self->dd, constr_states);
  return result;
}


/**Function********************************************************************

   Synopsis          [Returns the minimum distance of a given set of states
   from initial states]

   Description  [Computes the set of reachable states if not previously cached.
   Returns -1 if given states set is not reachable.

   This method returns an integer which represents the
   distance of the nearest state in "states". The
   distance of one single state "s" is the number of
   applications of the "BddFsm_get_forward_image"
   method (starting from the initial set of states)
   which is necessary and sufficient to end up with a set
   of states containing "s".
   Could update the cache.]

   SideEffects  [Internal cache could change]

******************************************************************************/
int BddFsm_get_minimum_distance_of_states(BddFsm_ptr self, BddStates states)
{
  bdd_ptr constr_states;
  int i;
  int diameter;
  int result = -1;

  BDD_FSM_CHECK_INSTANCE(self);

  if (CACHE_IS_EQUAL(reachable.computed, false)) {
    bdd_fsm_compute_reachable_states(self);
  }

  /* applies state constraints: */
  constr_states = bdd_and(self->dd, (bdd_ptr) states,
                          (bdd_ptr) self->invar_states);

  diameter = CACHE_GET(reachable.diameter);

  for (i=0; (-1 == result) && i<diameter; ++i) {
    bdd_ptr Ri  = (bdd_ptr) BddFsm_get_reachable_states_at_distance(self, i);

    bdd_and_accumulate(self->dd, &Ri, constr_states);
    if (bdd_isnot_false(self->dd, Ri)) { result = i; }

    bdd_free(self->dd, Ri);
  }

  bdd_free(self->dd, constr_states);
  return result;
}


/**Function********************************************************************

   Synopsis     [Returns the diameter of the machine from the inital state]

   Description  [This method returns an integer which represents the
   diameter of the machine with respect to the set of
   initial states, i.e.  the distance of the fatherst
   state in the machine (starting from the initial
   states), i.e. the maximal value among the lengths of
   shortest paths to each reachable state.  The initial
   diameter is computed as the number of iteration the
   fixpoint procedure described above (see
   "BddFsm_get_reachable_states") does before reaching
   the fixpoint.  It can also be seen as the maximal
   value the "BddFsm_get_distance_of_states" can return
   (which is returned when the argument "states" is set
   to "all the states").

   Could update the cache.]

   SideEffects  [Internal cache could change]

******************************************************************************/
int BddFsm_get_diameter(BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);

  if (CACHE_IS_EQUAL(reachable.computed, false)) {
    bdd_fsm_compute_reachable_states(self);
  }

  return CACHE_GET(reachable.diameter);
}


/**Function********************************************************************

   Synopsis     [Returns the set of states without subsequents]

   Description  [This method returns the set of states with no
   successor.  A state "ds" has no successor when all the
   following conditions hold:

   1) ds is a state satisfying stateConstr.
   2) no transition from ds exists which is consistent
   with input and state/input constraint and leads to
   a state satisfying stateConstr.

   Could update the cache.
   Note: a state is represented by state and frozen variables.]

   SideEffects  [Internal cache could change]

******************************************************************************/
BddStates BddFsm_get_not_successor_states(BddFsm_ptr self)
{
  BddStates res;

  BDD_FSM_CHECK_INSTANCE(self);

  if ( CACHE_IS_EQUAL(not_successor_states, BDD_STATES(NULL)) ) {
    bdd_ptr all_states = bdd_true(self->dd);
    bdd_ptr succ       = BddFsm_get_backward_image(self, all_states);
    bdd_ptr not_succ   = bdd_not(self->dd, succ);
    bdd_ptr no_succ_constr = bdd_and(self->dd, not_succ, self->invar_states);

    bdd_free(self->dd, not_succ);
    bdd_free(self->dd, succ);
    bdd_free(self->dd, all_states);

    CACHE_SET_BDD(not_successor_states, no_succ_constr);
    bdd_free(self->dd, no_succ_constr);
  }

  res = CACHE_GET_BDD(not_successor_states);
  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the set of deadlock states]

   Description  [This method returns the set of deadlock states.  A
   state ds is said to be a deadlock state when all the
   following conditions hold:

   1) ds is a state satisfying stateConstr;
   2) no transition from ds exists which is consistent
   with input and state/input constraint and leads to
   a state satisfying stateConstr;
   3) s is rechable.

   Could update the cache. May trigger the computation of
   reachable states and states without successors.
   Returned bdd is referenced.

   Note: a state is represented by state and frozen variables.]

   SideEffects  [Cache can change]

******************************************************************************/
BddStates BddFsm_get_deadlock_states(BddFsm_ptr self)
{
  BddStates res;

  BDD_FSM_CHECK_INSTANCE(self);

  if ( CACHE_IS_EQUAL(deadlock_states, BDD_STATES(NULL)) ) {
    BddStates no_succ = BddFsm_get_not_successor_states(self);
    BddStates reachable = BddFsm_get_reachable_states(self);

    bdd_ptr deadlock = bdd_and(self->dd, reachable, no_succ);
    bdd_free(self->dd, reachable);
    bdd_free(self->dd, no_succ);

    CACHE_SET_BDD(deadlock_states, BDD_STATES(deadlock));
    bdd_free(self->dd, deadlock);
  }

  res = CACHE_GET_BDD(deadlock_states);
  return res;
}


/**Function********************************************************************

   Synopsis     [Returns true if this machine is total]

   Description  [This method checks wether this machine is total, in
   the sense that each INVAR state has at least one INVAR
   successor state given the constraints on the inputs
   and the state/input.

   This is done by checking that the BddFsm_ImageBwd
   image of the set of all the states is the set of all
   the INVAR states.  This way, the INVAR constraints
   together with the set of constraints on both input and
   state/input are implicitly taken into account by
   BddFsm_get_forward_image.

   The answer "false" is produced when states exist that
   admit no INVAR successor, given the sets of input and
   state/input constraints. However, all these "dead"
   states may be non-reachable, so the machine can still
   be "deadlock free".  See the "BddFsm_is_deadlock_free"
   method.

   Could update the cache. May trigger the computation of
   states without successors.]

   SideEffects  [Cache can change]

******************************************************************************/
boolean BddFsm_is_total(BddFsm_ptr self)
{
  BddStates no_succ;
  boolean res;

  BDD_FSM_CHECK_INSTANCE(self);

  no_succ = BddFsm_get_not_successor_states(self);

  res = bdd_is_false(self->dd, (bdd_ptr) no_succ);
  bdd_free(self->dd, no_succ);

  return res;
}


/**Function********************************************************************

   Synopsis     [Returns true if this machine is deadlock free]

   Description  [This method checks wether this machine is deadlock
   free, i.e.  wether it is impossible to reach an INVAR
   state with no admittable INVAR successor moving from
   the initial condition.

   This happens when the machine is total. If it is not,
   each INVAR state from which no transition to another
   INVAR state can be made according to the input and
   state/input constraints is non-reachable.

   This method checks deadlock freeness by checking
   that the intersection between the set of reachable
   states and the set of INVAR states with no admittable
   INVAR successor is empty.

   Could update the cache. May trigger the computation of
   deadlock states.]

   SideEffects  [Cache can change]

******************************************************************************/
boolean BddFsm_is_deadlock_free(BddFsm_ptr self)
{
  BddStates deadlock;
  boolean res;

  BDD_FSM_CHECK_INSTANCE(self);

  deadlock = BddFsm_get_deadlock_states(self);

  res = bdd_is_false(self->dd, (bdd_ptr) deadlock);
  bdd_free(self->dd, deadlock);

  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the forward image of a set of states]

   Description  [This method computes the forward image of a set of
   states S, i.e. the set of INVAR states which are
   reachable from one of the INVAR states in S by means
   of one single machine transition among those
   consistent with both the input constraints and the
   state/input constraints.

   The forward image of S(X,F) is computed as follows.
   X - state variables, I - input variables, F - frozen variables.

   a. S1(X,F,I)   := S(X,F) and Invar(X,F) and InputConst(I)
   b. S2(X',F)    := { <x',f> | <x,f,i,x'> in Tr(X,F,I,X') for
   some <x,i> in S1(X,F,I) }
   c. S3(X,F)     := S2(X',F)[x/x']
   d. FwdImg(X,F) := S3(X,F) and Invar(X,F)

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced.]

   SideEffects  []

******************************************************************************/
BddStates BddFsm_get_forward_image(const BddFsm_ptr self, BddStates states)
{
  BddStatesInputs one = bdd_true(self->dd);
  BddStates res;

  BDD_FSM_CHECK_INSTANCE(self);

  res = BddFsm_get_constrained_forward_image(self, states, one);
  bdd_free(self->dd, one);

  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the constrained forward image of a set of states]

   Description  [This method computes the forward image of a set of
   states S, given a set C of contraints on STATE, FROZEN
   and INPUT vars which are meant to represent a
   restriction on allowed transitions and inputs.

   The constrained image is the set of INVAR states which
   are reachable from one of the INVAR states in S by
   means of one single machine transition among those
   consistent with both the constraints defined within
   the machine and the additional constraint C(X,F,I).

   The forward image of S(X,F) is computed as follows.
   X - state variables, I - input variables, F - frozen variables.

   a. S1(X,F,I) := S(X,F) and Invar(X,F) and InputConst(I) and C(X,F,I)
   b. S2(X',F)    := { <x',f> | <x,f,i,x'> in Tr(X,F,I,X') for
   some <x,f,i> in S1(X,F,I) }
   c. S3(X,F)     := S2(X',F)[x/x']
   d. FwdImg(X,F) := S3(X,F) and Invar(X,F)

   To apply no contraints, parameter constraints must be the
   true bdd.

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced]

   SideEffects  []

******************************************************************************/
BddStates
BddFsm_get_constrained_forward_image(const BddFsm_ptr self,
                                     BddStates states,
                                     BddStatesInputsNexts constraints)
{
  BddStates res;
  bdd_ptr constr_trans;
  bdd_ptr tmp;

  BDD_FSM_CHECK_INSTANCE(self);

  /* ------------------------------------------------------------ */
  /* Apply invariant contraints: */
  constr_trans = bdd_and(self->dd, states, self->invar_states);
  bdd_and_accumulate(self->dd, &constr_trans, self->invar_inputs);
  bdd_and_accumulate(self->dd, &constr_trans, constraints);
  /* ------------------------------------------------------------ */

  tmp = BddTrans_get_forward_image_state(self->trans, constr_trans);
  bdd_free(self->dd, constr_trans);

  res = BDD_STATES( BddEnc_next_state_var_to_state_var(self->enc, tmp) );
  bdd_free(self->dd, tmp);

  bdd_and_accumulate(self->dd, (bdd_ptr*) &res, self->invar_states);
  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the constrained forward image of a set of states]

   Description  [This method computes the forward image of a set of
   states S, given a set C of contraints on STATE, FROZEN
   and INPUT and NEXT vars which are meant to represent a
   restriction on allowed transitions and inputs.

   The constrained image is the set of INVAR states which
   are reachable from one of the INVAR states in S by
   means of one single machine transition among those
   consistent with both the constraints defined within
   the machine and the additional constraint C(X,F,I).

   The forward image of S(X,F) is computed as follows.
   X - state variables, I - input variables, F - frozen variables.

   a. S1(X,F,I) := S(X,F) and Invar(X,F) and InputConst(I) and C(X,F,I)
   b. S2(X',F)    := { <x',f> | <x,f,i,x'> in Tr(X,F,I,X') for
   some <x,f,i> in S1(X,F,I) }
   c. S3(X,F)     := S2(X',F)[x/x']
   d. FwdImg(X,F) := S3(X,F) and Invar(X,F)

   To apply no contraints, parameter constraints must be the
   true bdd.

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced]

   SideEffects  []

******************************************************************************/
BddStates
BddFsm_get_sins_constrained_forward_image(const BddFsm_ptr self,
                                          BddStates states,
                                          BddStatesInputsNexts constraints)
{
  BddStates res;
  bdd_ptr constr_trans;
  bdd_ptr tmp;

  BDD_FSM_CHECK_INSTANCE(self);

  /* ------------------------------------------------------------ */
  /* Apply invariant contraints: */
  constr_trans = bdd_and(self->dd, states, self->invar_states);
  bdd_and_accumulate(self->dd, &constr_trans, self->invar_inputs);
  bdd_and_accumulate(self->dd, &constr_trans, constraints);
  /* ------------------------------------------------------------ */

  tmp = BddTrans_get_forward_image_state(self->trans, constr_trans);
  bdd_free(self->dd, constr_trans);

  res = BDD_STATES( BddEnc_next_state_var_to_state_var(self->enc, tmp) );
  bdd_free(self->dd, tmp);

  bdd_and_accumulate(self->dd, (bdd_ptr*) &res, self->invar_states);
  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the forward image of a set of state-input pairs]

   Description  [This method computes the forward image of a set of
   state-input pairs SI. This is the set of state-input
   pairs that fulfills INVAR and INPUT constraints and
   can be reached via a legal transition from at least
   one member of si that itself must fulfill INVAR and
   INPUT.

   The forward image of SI(X,F,I) is computed as follows.
   X - state variables, F - frozen variables, I - input
   variables.

   a. S1(X,F,I)     := SI(X,F,I) and Invar(X,F) and Input(I)
   b. S2(X',F)      := { <x',f> | <x,f,i,x'> in Tr(X,F,I,X')
   for some <x,i> in S1(X,F,I) }
   c. S3(X,F)       := S2(X',F)[x/x']
   d. FwdImg(X,F,I) := S3(X,F) and Invar(X,F) and Input(X,F,I)

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced.]

   SideEffects  []

   SeeAlso      [BddFsm_get_constrained_forward_image_states_inputs,
   BddFsm_get_forward_image]

******************************************************************************/
BddStatesInputs
BddFsm_get_forward_image_states_inputs(const BddFsm_ptr self,
                                       BddStatesInputs si)
{
  BddStatesInputs one = bdd_true(self->dd);
  BddStatesInputs res;

  BDD_FSM_CHECK_INSTANCE(self);

  res = BddFsm_get_constrained_forward_image_states_inputs(self, si, one);
  bdd_free(self->dd, one);

  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the constrained forward image of a set of
   state-input pairs]

   Description  [This method computes the forward image of a set of
   state-input pairs SI constrained by constraints (from
   now on C). This is the set of state-input pairs that
   fulfills INVAR and INPUT constraints and can be
   reached via a legal transition from at least one
   member of SI that itself must fulfill INVAR, INPUT,
   and C.

   The forward image of SI(X,F,I) is computed as follows.
   X - state variables, F - frozen variables, I - input
   variables.

   a. S1(X,F,I)     := SI(X,F,I) and Invar(X,F) and Input(I)
   and C(X,F,I)
   b. S2(X',F)      := { <x',f> | <x,f,i,x'> in Tr(X,F,I,X')
   for some <x,i> in S1(X,F,I) }
   c. S3(X,F)       := S2(X',F)[x/x']
   d. FwdImg(X,F,I) := S3(X,F) and Invar(X,F) and Input(I)

   To apply no contraints, parameter constraints must be
   the true bdd.

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced.]

   SideEffects  []

   SeeAlso      [BddFsm_get_forward_image_states_inputs,
   BddFsm_get_constrained_forward_image]

******************************************************************************/
BddStatesInputs
BddFsm_get_constrained_forward_image_states_inputs(
                                    const BddFsm_ptr self,
                                    BddStatesInputs si,
                                    BddStatesInputsNexts constraints)
{
  BddStatesInputs res;
  bdd_ptr constr_trans;
  bdd_ptr tmp;

  BDD_FSM_CHECK_INSTANCE(self);

  /* ------------------------------------------------------------ */
  /* Apply invariant contraints: */
  constr_trans = bdd_and(self->dd, si, self->invar_states);
  bdd_and_accumulate(self->dd, &constr_trans, self->invar_inputs);
  bdd_and_accumulate(self->dd, &constr_trans, constraints);
  /* ------------------------------------------------------------ */

  tmp = BddTrans_get_forward_image_state(self->trans, constr_trans);
  bdd_free(self->dd, constr_trans);

  res = BDD_STATES_INPUTS( BddEnc_next_state_var_to_state_var(self->enc, tmp) );
  bdd_free(self->dd, tmp);

  bdd_and_accumulate(self->dd, (bdd_ptr*) &res, self->invar_states);
  bdd_and_accumulate(self->dd, (bdd_ptr*) &res, self->invar_inputs);
  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the backward image of a set of states]

   Description  [This method computes the backward image of a set S of
   states, i.e. the set of INVAR states from which some
   of the INVAR states in S is reachable by means of one
   single machine transition among those consistent with
   both the input constraints and the state/input
   constraints.

   The backward image of S(X,F) is computed as follows.
   X - state variables, I - input variables, F - frozen variables.

   a. S1(X,F)     := S(X,F) and Invar(X,F)
   b. S2(X',F)    := S1(X,F)[x'/x]
   c. S3(X,F,I)   := Invar(X,F) and InputConst(I)
   c. BwdImg(X,F) := { <x,f> | <x,f,i,x'> in Tr(X,F,I,X') for
   some <x,f,i> in S3(X,F,I) and some <x',f> in S2(X',F) }

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced.]

   SideEffects  []

******************************************************************************/
BddStates BddFsm_get_backward_image(const BddFsm_ptr self, BddStates states)
{
  BddStates res;
  BddStatesInputs one;

  BDD_FSM_CHECK_INSTANCE(self);

  one = bdd_true(self->dd);

  res = BddFsm_get_constrained_backward_image(self, states, one);
  bdd_free(self->dd, one);

  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the constrained backward image of a set of states]

   Description  [This method computes the backward image of a set of
   states S, given a set C(X,F,I) of contraints on STATE, FROZEN
   and INPUT vars which are meant to represent a
   restriction on allowed transitions and inputs.

   The constrained image is the set of INVAR states from
   which some of the INVAR states in S is reachable by
   means of one single machine transition among those
   consistent with both the machine constraints and the
   given additional constraint C(X,F,I).

   The backward image of S(X,F,I) is computed as follows.
   X - state variables, I - input variables, F - frozen variables.

   a. S1(X,F)     := S(X,F) and Invar(X,F)
   b. S2(X',F)    := S1(X,F)[x'/x]
   c. S3(X,F,I)   := Invar(X,F) and InputConst(I)
   and IC(I) and C(X,F,I)
   c. BwdImg(X,F) := { <x,f> | <x,f,i,x'> in Tr(X,F,I,X') for
   some <x,f,i> in S3(X,F,I) and some <x',f> in S2(X',F) }

   To apply no contraints, parameter constraints must be
   the true bdd.

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced.]

   SideEffects  []

******************************************************************************/
BddStates
BddFsm_get_constrained_backward_image(const BddFsm_ptr self,
                                      BddStates states,
                                      BddStatesInputsNexts constraints)
{
  bdd_ptr constr_trans;
  bdd_ptr tmp, result;

  BDD_FSM_CHECK_INSTANCE(self);

  tmp = bdd_and(self->dd, states, self->invar_states);
  constr_trans = BddEnc_state_var_to_next_state_var(self->enc, tmp);
  bdd_free(self->dd, tmp);

  bdd_and_accumulate(self->dd, &constr_trans, self->invar_inputs);
  bdd_and_accumulate(self->dd, &constr_trans, constraints);

  result = BddTrans_get_backward_image_state(self->trans, constr_trans);

  bdd_and_accumulate(self->dd, &result, self->invar_states);

  bdd_free(self->dd, constr_trans);

  return BDD_STATES(result);
}

/**Function********************************************************************

   Synopsis     [Returns the k-backward image of a set of states]

   Description  [This method computes the set of <state,frozen,input> tuples
   that lead into at least k distinct states of the set
   of states given as input. The returned couples
   and the states in the set given in input are restricted

   The k-backward image of S(X,F) is computed as follows.
   X - state variables, I - input variables, F - frozen variables.

   a. S1(X,F)   := S(X,F) and Invar(X,F)
   b. S2(X',F)    := S1(X,F)[X'/X]
   c. S3(X,F,I,k) := {<x,f,i> | exists x'[1..k] : S2(x'[m],f) and
   x'[m] != x'[n] if m != n and
   <x,f,i,x'[m]> in Tr }
   d. KBwdImg(X,F,I,k) := S3(X,F,I,k) and Invar(X,F) and
   InputConst(I)

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   The returned bdd is referenced.]

   SideEffects  []

******************************************************************************/
BddStatesInputs BddFsm_get_k_backward_image(const BddFsm_ptr self,
                                            BddStates states,
                                            int k)
{
  bdd_ptr tmp, tmp1, result;

  BDD_FSM_CHECK_INSTANCE(self);

  tmp = bdd_and(self->dd, states, self->invar_states);

  /* We need to apply the mask, otherwise the count is not correct! */
  tmp1 = BddEnc_apply_state_frozen_vars_mask_bdd(self->enc, tmp);
  bdd_free(self->dd, tmp);

  tmp = BddEnc_state_var_to_next_state_var(self->enc, tmp1);
  bdd_free(self->dd, tmp1);

  result = BddTrans_get_k_backward_image_state_input(self->trans, tmp, k);
  bdd_free(self->dd, tmp);

  bdd_and_accumulate(self->dd, &result, self->invar_inputs);
  bdd_and_accumulate(self->dd, &result, self->invar_states);

  return BDD_STATES(result);
}

/**Function********************************************************************

   Synopsis     [Returns the weak backward image of a set of states]

   Description  [This method computes the set of <state,frozen,input> tuples
   that leads into the set of states given as input.
   i.e. the set of <s,f,i> such that <s,f,i> is
   consistent with both the input constraints and the
   state/input constraints, s is INVAR, and a transition
   from s to s' labelled by i exists for some INVAR s' in
   S.

   The weak backward image of S(X,F) is computed as follows.
   X - state variables, I - input variables, F - frozen variables.

   a. S1(X,F)   := S(X,F) and Invar(X,F)
   b. S2(X',F   := S1(X,F)[x'/x]
   c. S3(X,F,I) := Invar(X,F) and InputConst(I)
   c. WeakBwdImg(X,F,I) := {<x,f,i> | <x,f,i,x'> in Tr(X,F,I,X')
   for some <x,f,i> in S3(X,I) and some <x,f>' in S2(X',F) }

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced.]

   SideEffects  []

******************************************************************************/
BddStatesInputs BddFsm_get_weak_backward_image(const BddFsm_ptr self,
                                               BddStates states)
{
  bdd_ptr constr_trans;
  bdd_ptr tmp, result;

  BDD_FSM_CHECK_INSTANCE(self);

  tmp = bdd_and(self->dd, states, self->invar_states);
  constr_trans = BddEnc_state_var_to_next_state_var(self->enc, tmp);
  bdd_free(self->dd, tmp);

  bdd_and_accumulate(self->dd, &constr_trans, self->invar_inputs);

  result = BddTrans_get_backward_image_state_input(self->trans, constr_trans);
  bdd_free(self->dd, constr_trans);

  bdd_and_accumulate(self->dd, &result, self->invar_states);

  return BDD_STATES(result);
}


/**Function********************************************************************

   Synopsis     [Returns the strong backward image of a set of states]

   Description  [This method computes the set of <state,frozem,input>
   transitions that have at least one successor and are
   such that all the successors lay inside the INVAR
   subset of the set of states given as input.

   The strong backward image of S(X, F, I) is computed as
   follows.
   X - state variables, I - input variables, F - frozen variables.

   a. S1(X,F,I) := WeakBwdImg(not S(X,F))
   b. S2(X,F,I) := (not S1(X,F,I)) and StateConstr(X,F) and
   InputConst(I)
   c. Tr(X,F,I) := {<x,d,i> | <x,d,i,x'> in Tr(X,F,I,X') for some x'}
   d. StrongBwdImg(X,F,I) := S2(X,F,I) and Tr(X,F,I)

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced.]

   SideEffects  []

******************************************************************************/
BddStatesInputs BddFsm_get_strong_backward_image(const BddFsm_ptr self,
                                                 BddStates states)
{
  bdd_ptr not_states;
  bdd_ptr tmp, result;

  BDD_FSM_CHECK_INSTANCE(self);

  not_states = bdd_not(self->dd, states);
  /* there is no need to add state or input invariants because
     there are added in BddFsm_get_weak_backward_image
  */
  tmp = BddFsm_get_weak_backward_image(self, not_states);
  bdd_free(self->dd, not_states);

  /* Here tmp is the set of state/input transitions that can be
     actually made and that lead outside the set defined by the
     input parameter 'states'
  */

  result = bdd_not(self->dd, tmp);
  bdd_free(self->dd, tmp);

  /* result is the set of state/input transitions that either
     originate from 'non-real states' or - in case there is at least
     one successor - are such that all the successors lay inside the
     set defined by the input parameter 'states' or or one of state,
     input or all successors belong to sets not satisfying invariants.
  */

  /* Obtain all the legal transitions, i.e. such state/input
     which have have at least one successor  and all state/input/successor
     satisty the invariants.
  */
  tmp = (bdd_ptr) bdd_fsm_get_legal_state_input(self);

  bdd_and_accumulate(self->dd, &result, tmp);
  bdd_free(self->dd, tmp);

  /* At this point, result is the set of state/input transitions that
     have at least one successor and are such that all
     the successors lay inside the set defined by the
     input parameter 'states' */

  return BDD_STATES_INPUTS(result);
}


/**Function********************************************************************

   Synopsis           [Prints some information about this BddFsm.]

   Description        [Prints some information about this BddFsm.]

   SideEffects        [None]

   SeeAlso            []

******************************************************************************/
void BddFsm_print_info(const BddFsm_ptr self, FILE* file)
{
  BddStates init_bdd;
  BddInvarStates state_bdd;
  BddInvarInputs input_bdd;

  BDD_FSM_CHECK_INSTANCE(self);

  init_bdd = BddFsm_get_init(self);
  state_bdd = BddFsm_get_state_constraints(self);
  input_bdd = BddFsm_get_input_constraints(self);

  fprintf(file, "Statistics on BDD FSM machine.\n");

  if ((init_bdd != NULL)) {
    fprintf(file, "BDD nodes representing init set of states: %d\n",
            bdd_size(self->dd, init_bdd));
    bdd_free(self->dd, init_bdd);
  }

  if ((state_bdd != NULL)) {
    fprintf(file, "BDD nodes representing state constraints: %d\n",
            bdd_size(self->dd, state_bdd));
    bdd_free(self->dd, state_bdd);
  }

  if ((input_bdd != NULL)) {
    fprintf(file, "BDD nodes representing input constraints: %d\n",
            bdd_size(self->dd, input_bdd));
    bdd_free(self->dd, input_bdd);
  }

  BddTrans_print_short_info(BddFsm_get_trans(self), file);
}


/**Function********************************************************************

   Synopsis           [Prints statistical information about reachable states.]

   Description        [Prints statistical information about reachable
   states, i.e. the real number of reachable states. It is computed
   taking care of the encoding and of the indifferent variables in the
   encoding.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddFsm_print_reachable_states_info(const BddFsm_ptr self,
                                        const boolean print_states,
                                        const boolean print_defines,
                                        const boolean print_formula,
                                        FILE* file)
{
  bdd_ptr reachable;
  bdd_ptr mask;
  double reached_cardinality;
  double search_space_cardinality;

  BDD_FSM_CHECK_INSTANCE(self);

  mask = BddEnc_get_state_frozen_vars_mask_bdd(self->enc);

  reachable = BddFsm_get_reachable_states(self);

  bdd_and_accumulate(self->dd, &reachable, mask);

  reached_cardinality = BddEnc_count_states_of_bdd(self->enc, reachable);
  search_space_cardinality = BddEnc_count_states_of_bdd(self->enc, mask);
  bdd_free(self->dd, mask);

  /* If we have diameter info, print it. Otherwise, we can only print
     the number of reachable states (ie. We do not have onion rings
     informations. For example, reachable states have been computed
     with Guided Reachability  */
  if (BddFsm_reachable_states_computed(self)) {
    fprintf(file, "system diameter: %d\n", BddFsm_get_diameter(self));
  }
  else {
    nusmv_assert(BddFsm_has_cached_reachable_states(self));
    fprintf(file, "system diameter: N/A\n");
  }

  fprintf(file, "reachable states: %g (2^%g) out of %g (2^%g)\n",
          reached_cardinality, log(reached_cardinality)/log(2.0),
          search_space_cardinality, log(search_space_cardinality)/log(2.0));

  /* one of these flags can be enabled, not both */
  nusmv_assert(!print_states || !print_formula);
  if (print_states) {
    BddEnc_print_set_of_states(self->enc, reachable, false, print_defines,
			       (VPFNNF) NULL, file);
  }
  else if (print_formula) {
    BoolEnc_ptr benc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self->enc));

    const array_t* layer_names =
      BaseEnc_get_committed_layer_names(BASE_ENC(self->enc));

    SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(self->enc));
    NodeList_ptr all_vars = SymbTable_get_layers_sf_vars(st, layer_names);
    NodeList_ptr scalar_vars = NodeList_create();
    ListIter_ptr iter;

    /* encoding variables are not allowed in the wff printer */
    NODE_LIST_FOREACH(all_vars, iter) {
      node_ptr v = NodeList_get_elem_at(all_vars, iter);
      if (BoolEnc_is_var_bit(benc, v)) continue;
      NodeList_append(scalar_vars, v);
    }
    NodeList_destroy(all_vars);

    fprintf(file, "\nFORMULA = \n");
    BddEnc_print_bdd_wff(self->enc, reachable, scalar_vars,
                         true, false, 0, file);

    NodeList_destroy(scalar_vars);
  }

  bdd_free(self->dd, reachable);
}


/**Function********************************************************************

   Synopsis     [Returns the set of fair state-input pairs of the machine.]

   Description  [A state-input pair is fair iff it can reach a cycle that
   visits all fairness constraints.

   Note: a state is represented by state and frozen variables.]

   SideEffects  [Internal cache could change]

   SeeAlso      []

******************************************************************************/
BddStatesInputs BddFsm_get_fair_states_inputs(BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);
  return bdd_fsm_get_fair_or_revfair_states_inputs(self, BDD_FSM_DIR_BWD);
}


/**Function********************************************************************

   Synopsis     [Returns the set of reverse fair state-input pairs of the
   machine.]

   Description  [A state-input pair is reverse fair iff it can be reached from
   a cycle that visits all fairness constraints.

   Note: a state is represented by state and frozen variables.]

   SideEffects  [Internal cache could change]

   SeeAlso      []

******************************************************************************/
BddStatesInputs BddFsm_get_revfair_states_inputs(BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);
  return bdd_fsm_get_fair_or_revfair_states_inputs(self, BDD_FSM_DIR_FWD);
}


/**Function********************************************************************

   Synopsis     [Returns the set of fair states of a fsm.]

   Description  [A state is fair iff it can reach a cycle that visits all
   fairness constraints.

   Note: a state is represented by state and frozen variables.]

   SideEffects  [Internal cache could change]

   SeeAlso      []

******************************************************************************/
BddStates BddFsm_get_fair_states(BddFsm_ptr self)
{
  BddStates res;

  BDD_FSM_CHECK_INSTANCE(self);

  if ( CACHE_IS_EQUAL(fair_states, BDD_STATES(NULL)) ) {
    BddStatesInputs si = BddFsm_get_fair_states_inputs(self);
    BddStates fs = BddFsm_states_inputs_to_states(self, si);

    CACHE_SET_BDD(fair_states, fs);

    bdd_free(self->dd, fs);
  }

  res = CACHE_GET_BDD(fair_states);
  return res;
}


/**Function********************************************************************

   Synopsis     [Returns the set of reverse fair states of a fsm.]

   Description  [A state is reverse fair iff it can be reached from a cycle
   that visits all fairness constraints.

   Note: a state is represented by state and frozen variables.]

   SideEffects  [Internal cache could change]

   SeeAlso      []

******************************************************************************/
BddStates BddFsm_get_revfair_states(BddFsm_ptr self)
{
  BddStates res;

  BDD_FSM_CHECK_INSTANCE(self);

  if ( CACHE_IS_EQUAL(revfair_states, BDD_STATES(NULL)) ) {
    BddStatesInputs si = BddFsm_get_revfair_states_inputs(self);
    BddStates fs = BddFsm_states_inputs_to_states(self, si);

    CACHE_SET_BDD(revfair_states, fs);

    bdd_free(self->dd, fs);
  }

  res = CACHE_GET_BDD(revfair_states);
  return res;
}


/**Function********************************************************************

   Synopsis [Given two sets of states, returns the set of inputs
   labeling any transition from a state in the first set to a state in
   the second set.]

   Description        [Note: a state is represented by state and frozen variables.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/

BddInputs BddFsm_states_to_states_get_inputs(const BddFsm_ptr self,
                                             BddStates cur_states,
                                             BddStates next_states)
{
  BddStates bwd_image_si;
  BddInputs inputs;

  BDD_FSM_CHECK_INSTANCE(self);

  bwd_image_si = BddFsm_get_weak_backward_image(self, next_states);
  bdd_and_accumulate(self->dd, &bwd_image_si, cur_states);

  inputs = BddFsm_states_inputs_to_inputs(self, bwd_image_si);

  bdd_free(self->dd, bwd_image_si);
  return inputs;
}

/**Function********************************************************************

   Synopsis           [Checks if a set of states is fair.]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/

boolean BddFsm_is_fair_states(const BddFsm_ptr self, BddStates states)
{
  BddStates fair_states;
  boolean res;

  BDD_FSM_CHECK_INSTANCE(self);

  fair_states = BddFsm_get_fair_states(self);

  res = (bdd_entailed(self->dd, states, fair_states) == 1);

  bdd_free(self->dd, fair_states);
  return res;
}


/**Function********************************************************************

   Synopsis           [Returns a state-input pair for which at least one
   legal successor (if dir  = BDD_FSM_DIR_BWD) or
   predecessor (otherwise) exists]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddStatesInputs BddFsm_get_states_inputs_constraints(const BddFsm_ptr self,
                                                     BddFsm_dir dir)
{
  BddStatesInputs result;
  BddStates all_states;

  BDD_FSM_CHECK_INSTANCE(self);

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    all_states = BddFsm_get_reachable_states(self);
  }
  else all_states = bdd_true(self->dd);

  /* it would be a better idea to cache it */
  if (dir == BDD_FSM_DIR_BWD) {
    result = BddTrans_get_backward_image_state_input(self->trans, all_states);
  }
  else {

    result = BddFsm_get_forward_image_states_inputs(self, all_states);
  }
  bdd_free(self->dd, all_states);
  return result;
}


/**Function********************************************************************

   Synopsis     [Returns the states occurring in a set of states-inputs pairs.]

   Description  [Quantifies away the input variables.
   Note: a state is represented by state and frozen variables.]

   SeeAlso      []

   SideEffects  []

******************************************************************************/
BddStates BddFsm_states_inputs_to_states(const BddFsm_ptr self,
                                         BddStatesInputs si)
{
  BddStates states;
  bdd_ptr input_vars_cube;

  BDD_FSM_CHECK_INSTANCE(self);

  input_vars_cube = BddEnc_get_input_vars_cube(self->enc);

  states = bdd_forsome(self->dd, si, input_vars_cube);

  bdd_free(self->dd, input_vars_cube);

  return states;
}


/**Function********************************************************************

   Synopsis [Makes k steps of expansion of the set of reachable states
   of this machine but limit the computation to terminate in the
   number of seconds specified (even if this limit can be exceeded for
   the termination of the last cycle)]

   Description [ If k<0 the set is expanded until fixpoint, if max_seconds<0 no
   time limit is considered ]

   SideEffects  [Changes the internal cache]

   SeeAlso      []

******************************************************************************/
boolean BddFsm_expand_cached_reachable_states(BddFsm_ptr self,
                                              int k,
                                              int max_seconds)
{
  bdd_ptr reachable_states_bdd;
  bdd_ptr from_lower_bound;   /* the frontier */
  bdd_ptr invars;
  node_ptr reachable_states_layers;

  int diameter;
  boolean completed;
  BddStates* layers;
  boolean result;

  long start_time;
  long limit_time;

  BDD_FSM_CHECK_INSTANCE(self);

  start_time = util_cpu_time();

  /* Transform max_seconds in milliseconds */
  limit_time = max_seconds * 1000;

  /* Initialize the layers list */
  reachable_states_layers = Nil;

  /* Get the cache */
  completed = BddFsm_get_cached_reachable_states(self, &layers, &diameter);

  /* Reload cache if any */
  if (diameter > 0) {
    int i;

    if (completed) {
      return true; /* already ready */
    }
    else {
      /* The cached analysis is not complete, so we have to resume the
         last state */

      /* Regen the list */
      for (i=0; i<diameter; i++) {
        reachable_states_layers =
          cons((node_ptr) bdd_dup(layers[i]), reachable_states_layers);
      }

      /* Last layer contains the last reachable set */
      reachable_states_bdd = bdd_dup(layers[diameter - 1]);

      /* Get the last frontier */
      if (diameter > 1) {
        /* Compute the last frontier*/
        bdd_ptr tmp;

        tmp = bdd_not(self->dd, layers[diameter - 2]);
        from_lower_bound = bdd_and(self->dd,
                                   reachable_states_bdd,
                                   tmp);
        bdd_free(self->dd, tmp);
      }
      else {
        /* We computed only the init state, so the frontier is the
           init itself */
        from_lower_bound = bdd_dup(layers[0]);
      }
    }
  }
  else {
    /* No cache, we hawe to start from scratch */

    /* Initial state = inits && invars */
    reachable_states_bdd = BddFsm_get_init(self);
    invars = BddFsm_get_state_constraints(self);
    bdd_and_accumulate(self->dd, &reachable_states_bdd, invars);
    bdd_free(self->dd, invars);

    /* The initial frontier is the initial reachables */
    from_lower_bound = bdd_dup(reachable_states_bdd);

    if (bdd_isnot_false(self->dd, reachable_states_bdd)) {
      reachable_states_layers =
        cons((node_ptr) bdd_dup(reachable_states_bdd), reachable_states_layers);

      diameter = 1;
    }
    else {
      /* If the initial region is empty then diameter is 0 */
      reachable_states_layers = Nil;
      diameter = 0;
    }
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "\ncomputing reachable state space\n");
  }

  /* Real analysis: the cycle terminates when fixpoint is reached so
     no new states can be visited */
  while ((bdd_isnot_false(self->dd, from_lower_bound)) &&
         (0 != k) &&
         ((-1 == max_seconds) ||
          (util_cpu_time()-start_time) < limit_time)) {
    bdd_ptr from_upper_bound, img, not_from_upper_bound;

    /* Decrease the remaining steps if k is not < 0*/
    if (k>0) k--;

    /* Save old reachables */
    from_upper_bound = bdd_dup(reachable_states_bdd);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr,
              "  iteration %d: BDD size = %d, frontier size = %d, states = %g\n",
              diameter, bdd_size(self->dd, reachable_states_bdd),
              bdd_size(self->dd, from_lower_bound),
              BddEnc_count_states_of_bdd(self->enc, reachable_states_bdd));
    }

    /* Get the forward image */
    img = BddFsm_get_forward_image(self, BDD_STATES(from_lower_bound));

    /* Now the reachable states are the old ones union the forward
       image */
    bdd_or_accumulate(self->dd, &reachable_states_bdd, img);
    bdd_free(self->dd, (bdd_ptr) img);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr, "  forward step done, size = %d\n",
              bdd_size(self->dd, reachable_states_bdd));
    }

    /* Now, update the frontier */

    /* Negate the old reachables ( = get the complementar set ) */
    not_from_upper_bound = bdd_not(self->dd, from_upper_bound);

    /* Free the old frontier */
    bdd_free(self->dd, from_lower_bound);


    /*   [AT] I agree. but some benchmarking is required (at least, 2-3 examples).
         [MR] It really depends where we are on the curve bdd size vs number of states.
         [MR] There might be cases where it is better one solution, others where
         [MR] it is better the other. As far I remember from the analysis we did
         [MR] in the past, this was the solution more widenly used. An heursitic
         [MR] to switch among the different option would be a good idea here. */

    /* New frontier is the differnece between old reachables and new
       ones so we do the intersection between the complementar set of
       old reachables with the new reachables */
    from_lower_bound = bdd_and(self->dd,
                                 reachable_states_bdd,
                                 not_from_upper_bound);

    /* Free old reachables negation */
    bdd_free(self->dd, not_from_upper_bound);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr, "  new frontier computed, size = %d\n",
              bdd_size(self->dd, from_lower_bound));
    }

    /* Free old reachables */
    bdd_free(self->dd, from_upper_bound);

    /* increment the diameter */
    ++diameter;

    /* Update the reachable list */
    reachable_states_layers =
      cons((node_ptr) bdd_dup(reachable_states_bdd), reachable_states_layers);

  } /* while loop */

  result = bdd_is_false(self->dd, from_lower_bound);

  if (result) {
    /*
       Cache the computed layers.
       BddFsm_update_cached_reachables is responsible of the free of the list
       reachable_states_layers
    */

    if (Nil != reachable_states_layers) {
      node_ptr tmp;

      tmp = reachable_states_layers;
      reachable_states_layers = cdr(reachable_states_layers);
      bdd_free(self->dd, (bdd_ptr) car(tmp));
      free_node(tmp);
      diameter --;
    }

    BddFsm_update_cached_reachable_states(self, reachable_states_layers,
                                          diameter, true);
  }
  else {
    /*
       Cache the partial computed layers.
       BddFsm_update_cached_reachables is responsible of the free of the list
       reachable_states_layers
    */
    BddFsm_update_cached_reachable_states(self, reachable_states_layers,
                                          diameter, false);
  }

  /* Free the last reachable states 'set' */
  bdd_free(self->dd, reachable_states_bdd);

  /* Free the last frontier */
  bdd_free(self->dd, from_lower_bound);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "done\n");
  }

  /* True if fixpoint, false otherwise */
  return result;
}


/**Function********************************************************************

   Synopsis     [Returns the inputs occurring in a set of states-inputs pairs.]

   Description  [Quantifies away the state variables (including frozen ones).
   A state is represented by state and frozen variables thus
   both state and frozen variables are abstracted away.]

   SeeAlso      []

   SideEffects  []

******************************************************************************/
BddStates BddFsm_states_inputs_to_inputs(const BddFsm_ptr self,
                                         BddStatesInputs si)
{
  BddStates input;

  BDD_FSM_CHECK_INSTANCE(self);

  bdd_ptr vars_cube = BddEnc_get_state_frozen_vars_cube(self->enc);
  input = bdd_forsome(self->dd, si, vars_cube);
  bdd_free(self->dd, vars_cube);

  return input;
}


/**Function********************************************************************

   Synopsis           [Prints statistical information about fair states.]

   Description [Prints the number of fair states, taking care of the
   encoding and of the indifferent variables in the
   encoding. In verbose mode also prints states.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddFsm_print_fair_states_info(const BddFsm_ptr self,
                                   const boolean print_states, FILE* file)
{
  bdd_ptr fair_states;
  bdd_ptr mask;
  double reached_cardinality;
  double search_space_cardinality;

  BDD_FSM_CHECK_INSTANCE(self);

  fair_states = BddFsm_get_fair_states(self);
  mask = BddEnc_get_state_frozen_vars_mask_bdd(self->enc);
  bdd_and_accumulate(self->dd, &fair_states, mask);

  reached_cardinality = BddEnc_count_states_of_bdd(self->enc, fair_states);
  search_space_cardinality = BddEnc_count_states_of_bdd(self->enc, mask);
  bdd_free(self->dd, mask);

  fprintf(file, "fair states: %g (2^%g) out of %g (2^%g)\n",
          reached_cardinality, log(reached_cardinality)/log(2.0),
          search_space_cardinality, log(search_space_cardinality)/log(2.0));

  if (print_states) {
    BddEnc_print_set_of_states(self->enc, fair_states, false, true,
			       (VPFNNF) NULL, file);
  }
  bdd_free(self->dd, fair_states);
}


/**Function********************************************************************

   Synopsis           [Prints statistical information about fair states and
   transitions.]

   Description        [Prints the number of fair states, taking care of
   the encoding and of the indifferent variables in the encoding.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddFsm_print_fair_transitions_info(const BddFsm_ptr self,
                                        const boolean print_transitions,
                                        FILE* file)
{
  bdd_ptr fair_trans;
  bdd_ptr mask;
  double fairstates_cardinality;
  double search_space_cardinality;

  BDD_FSM_CHECK_INSTANCE(self);

  fair_trans = BddFsm_get_fair_states_inputs(self);
  mask = BddEnc_get_state_frozen_input_vars_mask_bdd(self->enc);
  bdd_and_accumulate(self->dd, &fair_trans, mask);

  fairstates_cardinality = BddEnc_get_minterms_of_bdd(self->enc, fair_trans);
  search_space_cardinality = BddEnc_get_minterms_of_bdd(self->enc, mask);
  bdd_free(self->dd, mask);

  fprintf(file, "Fair state-input pairs: %g (2^%g) out of %g (2^%g)\n",
          fairstates_cardinality, log(fairstates_cardinality)/log(2.0),
          search_space_cardinality, log(search_space_cardinality)/log(2.0));

  if (print_transitions) {
    BddEnc_print_set_of_state_input_pairs(self->enc, fair_trans, false,
					  (VPFNNF) NULL, file);
  }

  bdd_free(self->dd, fair_trans);
}


/**Function********************************************************************

   Synopsis    [Check that the transition relation is total]

   Description [Check that the transition relation is total. If not the
   case than a deadlock state is printed out. May trigger the
   computation of reachable states and states without successors.]

   SideEffects []

   SeeAlso     []

******************************************************************************/
void BddFsm_check_machine(const BddFsm_ptr self)
{
  BDD_FSM_CHECK_INSTANCE(self);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Checking totality and deadlock states.\n");
  }

  bdd_fsm_check_init_state_invar_emptiness(self);
  bdd_fsm_check_fairness_emptiness(self);

  if (! BddFsm_is_total(self)) {
    bdd_ptr noSuccStates = BddFsm_get_not_successor_states(self);
    bdd_ptr ds = BddEnc_pick_one_state(self->enc, noSuccStates);
    NodeList_ptr vars;
    bdd_free(self->dd, noSuccStates);

    fprintf(nusmv_stdout, "\n##########################################################\n");
    fprintf(nusmv_stdout, "The transition relation is not total. A state without\n");
    fprintf(nusmv_stdout, "successors is:\n");

    vars = SymbTable_get_layers_sf_i_vars(self->symb_table,
                                          SymbTable_get_class_layer_names(self->symb_table, (const char*) NULL));
    BddEnc_print_bdd_begin(self->enc, vars, false);
    BddEnc_print_bdd(self->enc, ds, (VPFNNF) NULL, nusmv_stdout);

    if (CACHE_IS_EQUAL(reachable.computed, true) ||
        opt_use_reachable_states(OptsHandler_get_instance())) {
      /* here the reachable states calculation has been done or requested. */
      if (! BddFsm_is_deadlock_free(self)) {

        bdd_ptr deadlockStates = BddFsm_get_deadlock_states(self);
        bdd_ptr ds = BddEnc_pick_one_state(self->enc, deadlockStates);
        bdd_free(self->dd, deadlockStates);

        fprintf(nusmv_stdout, "The transition relation is not deadlock-free.\n");
        fprintf(nusmv_stdout, "A deadlock state is:\n");
        BddEnc_print_bdd(self->enc, ds, (VPFNNF) NULL, nusmv_stdout);
      }
      else {
        fprintf(nusmv_stdout, "However, all the states without successors are\n");
        fprintf(nusmv_stdout, "non-reachable, so the machine is deadlock-free.\n");
      }
    }
    else {
      /* reachables states should be calculated */
      fprintf(nusmv_stdout, "NOTE: No-successor states could be non-reachable, so\n");
      fprintf(nusmv_stdout, "      the machine could still be deadlock-free.\n");
      fprintf(nusmv_stdout, "      Reachable states have to be computed to check this.\n");
    }

    BddEnc_print_bdd_end(self->enc);
    NodeList_destroy(vars);

    fprintf(nusmv_stdout, "##########################################################\n");
    bdd_free(self->dd, ds);
  }
  else {
    fprintf(nusmv_stdout, "\n##########################################################\n");
    fprintf(nusmv_stdout, "The transition relation is total: No deadlock state exists\n");
    fprintf(nusmv_stdout, "##########################################################\n");
  }
}


/**Function********************************************************************

   Synopsis    [Performs the synchronous product of two fsm]

   Description [Original description for BddFsm_apply_synchronous_product:

                The result goes into self, no changes on other.  Both
                the two FSMs must be based on the same dd manager.
                The cache will change, since a new separated family
                will be created for the internal cache, and it will
                not be shared anymore with previous family.  From the
                old cache will be reused as much as possible.

                Modified part:

                Takes cubes of state, input, and next state variables
                as arguments (rather than obtaining the cubes of all
                these variables from the bdd encoding). This is
                supposed to avoid problems when only subsets of
                variables need to be considered (as is the case for
                games).

                ]

   SideEffects [self will change]

   SeeAlso     [BddFsm_apply_synchronous_product,
                BddFsmCache_reset_not_reusable_fields_after_product]

******************************************************************************/
void BddFsm_apply_synchronous_product_custom_varsets(BddFsm_ptr self,
                                                     const BddFsm_ptr other,
                                                     bdd_ptr state_vars_cube,
                                                     bdd_ptr input_vars_cube,
                                                     bdd_ptr next_vars_cube)
{
  BddFsmCache_ptr new_cache;

  BDD_FSM_CHECK_INSTANCE(self);

  /* check for the same dd manager */
  nusmv_assert(self->dd == other->dd);

  /* check for the same dd manager, in the future we will probably
     relax this constraint  */
  nusmv_assert(self->enc == other->enc);

  /* init */
  bdd_and_accumulate(self->dd, &(self->init), other->init);

  /* invars */
  bdd_and_accumulate(self->dd, &(self->invar_states), other->invar_states);

  /* trans */
  BddTrans_apply_synchronous_product(self->trans, other->trans);

  /* fairness constraints */
  JusticeList_apply_synchronous_product(self->justice, other->justice);
  CompassionList_apply_synchronous_product(self->compassion, other->compassion);

  /* cache substitution */
  new_cache = BddFsmCache_hard_copy(self->cache);
  BddFsmCache_reset_not_reusable_fields_after_product(new_cache);
  BddFsmCache_destroy(self->cache);
  self->cache = new_cache;
}


/**Function********************************************************************

   Synopsis    [Variant of
                BddFsm_apply_synchronous_product_custom_varsets that
                simply takes all variables in the encoding into
                account.]

   Description [The result goes into self, no changes on other. Both
                the two FSMs must be based on the same dd manager.
                The cache will change, since a new separated family
                will be created for the internal cache, and it will
                not be shared anymore with previous family.  From the
                old cache will be reused as much as possible]

   SideEffects [self will change]

   SeeAlso     [BddFsm_apply_synchronous_product_custom_varsets,
                BddFsmCache_reset_not_reusable_fields_after_product]

******************************************************************************/
void BddFsm_apply_synchronous_product(BddFsm_ptr self, const BddFsm_ptr other)
{
  BDD_FSM_CHECK_INSTANCE(self);

  bdd_ptr input_vars_cube = BddEnc_get_input_vars_cube(self->enc);
  bdd_ptr state_vars_cube = BddEnc_get_state_vars_cube(self->enc);
  bdd_ptr next_vars_cube = BddEnc_get_next_state_vars_cube(self->enc);

  BddFsm_apply_synchronous_product_custom_varsets(self,
                                                  other,
                                                  state_vars_cube,
                                                  input_vars_cube,
                                                  next_vars_cube);

  bdd_free(self->dd, (bdd_ptr) next_vars_cube);
  bdd_free(self->dd, (bdd_ptr) state_vars_cube);
  bdd_free(self->dd, (bdd_ptr) input_vars_cube);
}


/* ---------------------------------------------------------------------- */
/*                         Static functions                               */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

   Synopsis    [Private initializer]

   Description []

   SideEffects []

******************************************************************************/
static void bdd_fsm_init(BddFsm_ptr self,
                         BddEnc_ptr encoding,
                         BddStates init,
                         BddInvarStates invar_states,
                         BddInvarInputs invar_inputs,
                         BddTrans_ptr trans,
                         JusticeList_ptr justice,
                         CompassionList_ptr compassion)
{
  self->enc = encoding;
  self->dd = BddEnc_get_dd_manager(encoding);
  self->symb_table = BaseEnc_get_symb_table(BASE_ENC(encoding));

  /* Here no check for infinite precision variables is done, since the
     BddEnc should not be able to convert expressions containing such
     variables. With this assumption, it is impossible that BDD init,
     invar and trans are built, and consequently, this code should not
     be reached. */

  nusmv_assert(init != NULL);

  self->init = BDD_STATES( bdd_dup((bdd_ptr) init) );
  self->invar_states = BDD_INVAR_STATES( bdd_dup((bdd_ptr) invar_states) );
  self->invar_inputs = BDD_INVAR_INPUTS( bdd_dup((bdd_ptr) invar_inputs) );
  self->trans = trans;
  self->justice = justice;
  self->compassion = compassion;

  self->cache = BddFsmCache_create(self->dd);

  /* check inits and invars for emptiness */
  bdd_fsm_check_init_state_invar_emptiness(self);
}


/**Function********************************************************************

   Synopsis    [private copy constructor]

   Description []

   SideEffects []

******************************************************************************/
static void bdd_fsm_copy(const BddFsm_ptr self, BddFsm_ptr copy)
{
  copy->dd = self->dd;
  copy->enc = self->enc;
  copy->symb_table = self->symb_table;

  copy->init = BDD_STATES( bdd_dup((bdd_ptr) self->init ) );
  copy->invar_states =
    BDD_INVAR_STATES( bdd_dup((bdd_ptr) self->invar_states ) );
  copy->invar_inputs  =
    BDD_INVAR_INPUTS( bdd_dup((bdd_ptr) self->invar_inputs ) );

  copy->trans = BDD_TRANS( Object_copy(OBJECT(self->trans)) );
  copy->justice = JUSTICE_LIST( Object_copy(OBJECT(self->justice)));
  copy->compassion = COMPASSION_LIST( Object_copy(OBJECT(self->compassion)));

  copy->cache = BddFsmCache_soft_copy(self->cache);
}


/**Function********************************************************************

   Synopsis    [private member called by the destructor]

   Description []

   SideEffects []

******************************************************************************/
static void bdd_fsm_deinit(BddFsm_ptr self)
{
  bdd_free(self->dd, (bdd_ptr) self->init);
  bdd_free(self->dd, (bdd_ptr) self->invar_states);
  bdd_free(self->dd, (bdd_ptr) self->invar_inputs);

  Object_destroy(OBJECT(self->trans), NULL);
  Object_destroy(OBJECT(self->justice), NULL);
  Object_destroy(OBJECT(self->compassion), NULL);

  BddFsmCache_destroy(self->cache);
}


/**Function********************************************************************

   Synopsis     [Computes the set of reachable states of this machine]

   Description  []

   SideEffects  [Changes the internal cache]

   SeeAlso      []

******************************************************************************/
static void bdd_fsm_compute_reachable_states(BddFsm_ptr self)
{
  boolean res;

  /* Expand cacked reachable states until fixpoint without time limitations */
  res = BddFsm_expand_cached_reachable_states(self, -1, -1);

  /* Assert that we completed the reachability analysis */
  nusmv_assert(res);
}


/**Function********************************************************************

   Synopsis     [Returns the set of states and inputs,
   for which a legal transition can be made.]

   Description  [A legal transition is a transition which satisfy the
   transition relation, and the state, input and next-state satisfy the
   invariants.  So the image S(X, F, I) is computed as follows:
   S(X,F,I) = StateConstr(X,F) & InputConstr(i) & StateConstr(X',F) &
   Tr(X,F,I,X') for some X'
   X - state variables, I - input variables, F - frozen variables.

   Used for planning in strong backward image computation.
   Could update the cache.

   Note: a state is represented by state and frozen variables,
   but frozen variable are never abstracted away.

   Returned bdd is referenced.
   ]

   SideEffects  [Cache can change]

******************************************************************************/
static BddStatesInputs bdd_fsm_get_legal_state_input(BddFsm_ptr self)
{
  BddStatesInputs res;

  if ( CACHE_IS_EQUAL(legal_state_input, BDD_STATES_INPUTS(NULL)) ) {
    bdd_ptr one = bdd_true(self->dd);
    /* Here we use weak-backward-image because
       it automatically applies invariant contrains on state, input, next-state.
    */
    bdd_ptr tmp = BddFsm_get_weak_backward_image(self, one);

    CACHE_SET_BDD(legal_state_input, tmp);
    bdd_free(self->dd, one);
    bdd_free(self->dd, tmp);
  }

  res = CACHE_GET_BDD(legal_state_input);
  return res;
}

/**Function********************************************************************

   Synopsis     [Computes the preimage (if dir = BDD_FSM_DIR_BWD) or the
   postimage (otherwise) of a set of states-inputs pairs.]

   Description  [Preimage:

   Quantifies away the inputs, and computes the (states-inputs)
   preimage of the resulting set of states.

   Postimage:

   Computes the (states-inputs) postimage of si.
   ]

   SeeAlso      []

   SideEffects  []

******************************************************************************/
static BddStatesInputs bdd_fsm_EXorEY_SI(const BddFsm_ptr self,
                                         BddStatesInputs si,
                                         BddFsm_dir dir)
{
  BddStatesInputs si_image;

  if (dir == BDD_FSM_DIR_BWD) {
    BddStates states;

    states = BddFsm_states_inputs_to_states(self, si);
    si_image = BddFsm_get_weak_backward_image(self, states);

    bdd_free(self->dd, states);
  }
  else {
    si_image = BddFsm_get_forward_image_states_inputs(self, si);
  }

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(self);

    bdd_and_accumulate(self->dd, &si_image, reachable_states_bdd);
    bdd_free(self->dd, reachable_states_bdd);
  }

  return si_image;
}


/**Function********************************************************************

   Synopsis     [Computes the set of state-input pairs that satisfy E(f U g)
   (if dir = BDD_FSM_DIR_BWD) or E(f S g) (otherwise),
   with f and g sets of state-input pairs.]

   Description  []

   SeeAlso      []

   SideEffects  []

******************************************************************************/
static BddStatesInputs bdd_fsm_EUorES_SI(const BddFsm_ptr self,
                                         BddStatesInputs f, BddStatesInputs g,
                                         BddFsm_dir dir)
{
  int i;
  BddStatesInputs resY;
  BddStatesInputs newY;
  BddStatesInputs rg = bdd_dup(g);

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(self);

    bdd_and_accumulate(self->dd, &rg, reachable_states_bdd);
    bdd_free(self->dd, reachable_states_bdd);
  }

  resY = bdd_dup(rg);
  newY = bdd_dup(rg);
  bdd_free(self->dd, rg);
  i = 0;

  while (bdd_isnot_false(self->dd, newY)) {
    BddStatesInputs tmp_1, tmp_2;
    BddStatesInputs oldNotY;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr,
              "    size of Y%d = %g <states>x<inputs>, %d BDD nodes\n",
              i, BddEnc_count_states_inputs_of_bdd(self->enc, resY),
              bdd_size(self->dd, resY));
    }

    oldNotY = bdd_not(self->dd, resY);

    tmp_1 = bdd_fsm_EXorEY_SI(self, newY, dir);
    tmp_2 = bdd_and(self->dd, f, tmp_1);
    bdd_free(self->dd, tmp_1);

    bdd_or_accumulate(self->dd, &resY, tmp_2);
    bdd_free(self->dd, tmp_2);

    bdd_free(self->dd, newY);


    /*   [AT] I agree. But some benchmarking are required.
         [MR] similar considerations as for the reachability */
    newY = bdd_and(self->dd, resY, oldNotY);
    bdd_free(self->dd, oldNotY);

    i++;
  }

  bdd_free(self->dd, newY);

  /* We do not free resY since it is rersposibility of the caller to
     free it. Functions always return a referenced bdd. */
  return BDD_STATES_INPUTS( resY );
}


/**Function********************************************************************

   Synopsis     [Executes the Emerson-Lei algorithm]

   Description  [Executes the Emerson-Lei algorithm in the set of states
   given by subspace in the direction given by dir]

   SeeAlso      []

   SideEffects  []

******************************************************************************/
static BddStatesInputs bdd_fsm_compute_EL_SI_subset(const BddFsm_ptr self,
                                                    BddStatesInputs subspace,
                                                    BddFsm_dir dir)
{
  BddStatesInputs res;
  BddStatesInputs old;
  int i = 0;

  BDD_FSM_CHECK_INSTANCE(self);

  res = bdd_true(self->dd);
  old = bdd_false(self->dd);

  /* GFP computation */
  while (res != old) {
    BddStatesInputs new;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr, "  size of res%d = %g <states>x<input>, %d BDD nodes\n",
              i++, BddEnc_count_states_inputs_of_bdd(self->enc, res),
              bdd_size(self->dd, res));
    }

    bdd_free(self->dd, old);
    old = bdd_dup(res);


    /* [VS] My guess is that the conjunction with Z should be
            before the E[XY]_SI - that is both theory and implementation.
       [AT] What is "Q" in formula? What is MAP?
       [AT] it is better to move this formulas to the description of bdd_fsm_compute_EL_SI_subset_aux
       [VS] I am not sure - I prefer to see the documentation on the top-level, which is here
       [AT] from the formula (and my understanding)
            bdd_fsm_compute_EL_SI_subset_aux requires only one
            parameter -- Z. but some benchmarking has to be done. */

    /* MAP( ApplicableStatesInputs) over Fairness constraints */
    /* EL bwd:
       return GFP Z. (Q /\ EX_SI ( Z /\ AND_i EU_SI(Z, (Z/\ StatesInputFC_i))))
       EL fwd:
       return GFP Z. (Q /\ EY_SI ( Z /\ AND_i ES_SI(Z, (Z/\ StatesInputFC_i)))) */
    new = bdd_fsm_compute_EL_SI_subset_aux(self, BDD_STATES_INPUTS(res), subspace, dir);


    bdd_and_accumulate(self->dd, &res, (bdd_ptr) new);
    bdd_and_accumulate(self->dd, &res, (bdd_ptr) subspace);

    bdd_free(self->dd, (bdd_ptr) new);
  }
  bdd_free(self->dd, old);

  return BDD_STATES_INPUTS(res);
}



/**Function********************************************************************

   Synopsis     [Executes the inner fixed point of the Emerson-Lei algorithm]

   Description  [Executes the inner fixed point of the Emerson-Lei
   algorithm. Direction is given by dir, fair states are restricted to
   states, backward/forward exploration (other than the last, "strict"
   image) is restricted to subspace.]

   SeeAlso      []

   SideEffects  []

******************************************************************************/

static BddStatesInputs bdd_fsm_compute_EL_SI_subset_aux(const BddFsm_ptr self,
                                                        BddStatesInputs states,
                                                        BddStatesInputs subspace,
                                                        BddFsm_dir dir)
{
  BddStatesInputs res;
  FairnessListIterator_ptr iter;
  BddStatesInputs partial_result;
  int i;

  res = bdd_true(self->dd);
  partial_result = bdd_dup(states);
  i = 0;

  /* Accumulates justice constraints: */
  iter = FairnessList_begin( FAIRNESS_LIST(self->justice) );
  while ( ! FairnessListIterator_is_end(iter) ) {

    /*   [VS] It might be possible to use the result of fairness_i to constrain the computation for fairness_{i+1}.
         [AT] it looks like there must be only one BDD as input
              (not 2) and it should be updated between fairness
              checks. Benchmarking required! */
    BddStatesInputs p;
    BddStatesInputs constrained_state;
    BddStatesInputs temp;

    p = JusticeList_get_p(self->justice, iter);
    constrained_state = bdd_and(self->dd, states, p);
    temp = bdd_fsm_EUorES_SI(self, subspace, constrained_state, dir);

    bdd_free(self->dd, constrained_state);
    bdd_free(self->dd, p);

    bdd_and_accumulate(self->dd, &partial_result, temp);
    bdd_free(self->dd, temp);

    iter = FairnessListIterator_next(iter);
    i++;
  } /* outer while loop */

  res = bdd_fsm_EXorEY_SI(self, partial_result, dir);
  bdd_free(self->dd, partial_result);

  return res;
}


/**Function********************************************************************

   Synopsis     [Computes the set of (reverse) fair states in subspace]

   Description  [Computes the set of fair states (if dir =
   BDD_FSM_DIR_BWD) or reverse fair states (otherwise) by calling the
   Emerson-Lei algorithm.]

   SideEffects  []

   SeeAlso      []

******************************************************************************/
static BddStatesInputs
bdd_fsm_get_fair_or_revfair_states_inputs_in_subspace(const BddFsm_ptr self,
                                                      BddStatesInputs subspace,
                                                      BddFsm_dir dir)
{
  BddStatesInputs fair_or_revfair_states_inputs;

  fair_or_revfair_states_inputs =
    bdd_fsm_compute_EL_SI_subset(self, subspace, dir);

  return fair_or_revfair_states_inputs;
}

/**Function********************************************************************

   Synopsis     [Computes the set of (reverse) fair states]

   Description  [Computes the set of fair states (if dir =
   BDD_FSM_DIR_BWD) or reverse fair states (otherwise) by calling the
   Emerson-Lei algorithm.]

   SideEffects  [Cache might change]

   SeeAlso      [bdd_fsm_get_fair_or_revfair_states_inputs_in_subspace]

******************************************************************************/
static BddStatesInputs
bdd_fsm_get_fair_or_revfair_states_inputs(BddFsm_ptr self, BddFsm_dir dir)
{
  BddStatesInputs res;

  BDD_FSM_CHECK_INSTANCE(self);

  if ( (dir == BDD_FSM_DIR_BWD &&
        CACHE_IS_EQUAL(fair_states_inputs, BDD_STATES(NULL))) ||
       (dir == BDD_FSM_DIR_FWD &&
        CACHE_IS_EQUAL(revfair_states_inputs, BDD_STATES(NULL))) ) {
    BddStatesInputs si;
    BddStates fair_or_revfair_si;

    /* Computation is restricted to those states that have a legal
       successor (if dir = BDD_FSM_DIR_BWD) or predecessor
       (otherwise). This potentially reduces the number of states to
       be considered. */

    si = BddFsm_get_states_inputs_constraints(self, dir);

    if (opt_use_reachable_states(OptsHandler_get_instance())) {
      bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(self);

      bdd_and_accumulate(self->dd, &si, reachable_states_bdd);
      bdd_free(self->dd, reachable_states_bdd);
    }

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      if (dir == BDD_FSM_DIR_BWD) {
        fprintf(nusmv_stderr, "Computing the set of fair <state>x<input> pairs\n");
      }
      else {
        fprintf(nusmv_stderr, "Computing the set of reverse fair <state>x<input> pairs\n");
      }
    }

    fair_or_revfair_si =
      bdd_fsm_get_fair_or_revfair_states_inputs_in_subspace(self, si, dir);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr,"done\n");
    }

    if (dir == BDD_FSM_DIR_BWD) {
      CACHE_SET_BDD(fair_states_inputs, fair_or_revfair_si);
    }
    else {
      CACHE_SET_BDD(revfair_states_inputs, fair_or_revfair_si);
    }

    bdd_free(self->dd, fair_or_revfair_si);
    bdd_free(self->dd, si);

  }

  if (dir == BDD_FSM_DIR_BWD) {
    res = CACHE_GET_BDD(fair_states_inputs);
  }
  else {
    res = CACHE_GET_BDD(revfair_states_inputs);
  }

  return res;
}


/**Function********************************************************************

   Synopsis    [Check inits for emptiness, and prints a warning if needed]

   Description []

   SideEffects []

   SeeAlso     []

******************************************************************************/
static void bdd_fsm_check_init_state_invar_emptiness(const BddFsm_ptr self)
{
  /* checks for emptiness of inits: */
  if (bdd_is_false(self->dd, self->init)) {
    warning_fsm_init_empty();
  }
  if (bdd_is_false(self->dd, self->invar_states)) {
    warning_fsm_invar_empty();
  }
}


/**Function********************************************************************

   Synopsis    [Checks fair states for emptiness, as well as fot the
   intersaction of fair states and inits. Prints a warning if needed ]

   Description []

   SideEffects []

   SeeAlso     []

******************************************************************************/
static void bdd_fsm_check_fairness_emptiness(const BddFsm_ptr self)
{
  bdd_ptr fair;

  fair = BddFsm_get_fair_states_inputs(self);

  if (bdd_is_false(self->dd, fair)) {
    warning_fsm_fairness_empty();
  }
  else if (bdd_isnot_false(self->dd, self->init)) {
    bdd_ptr fair_init = bdd_and(self->dd, self->init, fair);

    if (bdd_is_false(self->dd, fair_init)) {
      warning_fsm_init_and_fairness_empty();
    }
    bdd_free(self->dd, fair_init);
  }

  bdd_free(self->dd, fair);
}
