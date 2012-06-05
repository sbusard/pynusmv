/**CFile***********************************************************************

  FileName    [BddFsmCache.c]

  PackageName [fsm.bdd]

  Synopsis    [Private module, that defines a structure privately used
  into each instance of the class BddFsm, to cache some information]

  Description [This defines the interface of class BddFsmCache only.
  Since the cache is privately accessed also by the class BddFsm, the
  cache is declared by file bddFsmInt.h]

  Author      [Roberto Cavada]

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

#include "bdd.h"
#include "bddInt.h"

static char rcsid[] UTIL_UNUSED = "$Id: BddFsmCache.c,v 1.1.2.8.4.3.4.9 2010-02-26 14:26:35 nusmv Exp $";


/* ----------------------------------------------------------------------
   When you add a new data member:
   1) initialize it to a default value in BddFsmCache_create
   2) if required: destroy it in BddFsmCache_destroy
   3) copy it in BddFsmCache_hard_copy

   FOR BDDs: - initialize them to NULL
             - use macros CACHE_UNREF_BDD and CACHE_COPY_BDD
   ---------------------------------------------------------------------- */


/* some utilities to help addition of bdd members: */
#define CACHE_UNREF_BDD(type, member)                  \
  if ( self->member != (type) NULL ) {                 \
     bdd_free(self->dd, (bdd_ptr) (self->member) );    \
     self->member = (type) NULL;                       \
  }

#define CACHE_COPY_BDD(type, member)                       \
  if ( self->member != (type) NULL ) {                     \
    copy->member = (type) bdd_dup((bdd_ptr) self->member); \
  }


static void bdd_fsm_cache_init ARGS((BddFsmCache_ptr self, DdManager* dd));
static void bdd_fsm_cache_deinit ARGS((BddFsmCache_ptr self));
static void bdd_fsm_cache_deinit_reachables ARGS((BddFsmCache_ptr self));


/* ---------------------------------------------------------------------- */
/*                          private methods                               */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis     [Class contructor]

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/
BddFsmCache_ptr BddFsmCache_create(DdManager* dd)
{
  BddFsmCache_ptr self = ALLOC(BddFsmCache, 1);

  BDD_FSM_CACHE_CHECK_INSTANCE(self);

  bdd_fsm_cache_init(self, dd);

  return self;
}


/**Function********************************************************************

  Synopsis     [Class destructor]

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/
void BddFsmCache_destroy(BddFsmCache_ptr self)
{
  BDD_FSM_CACHE_CHECK_INSTANCE(self);

  (*self->family_counter)--;
  if (*(self->family_counter) == 0) {
    bdd_fsm_cache_deinit(self);
    FREE(self);
  }
}


/**Function********************************************************************

  Synopsis     [Class copy constructor]

  Description  [Hardly copy the instance, by creating a new, separate family]

  SideEffects  []

  SeeAlso      [BddFsmCache_soft_copy]

******************************************************************************/
BddFsmCache_ptr BddFsmCache_hard_copy(const BddFsmCache_ptr self)
{
  BddFsmCache_ptr copy;

  BDD_FSM_CACHE_CHECK_INSTANCE(self);

  copy = BddFsmCache_create(self->dd);

  CACHE_COPY_BDD(BddStates, fair_states);
  CACHE_COPY_BDD(BddStatesInputs, fair_states_inputs);
  CACHE_COPY_BDD(BddStates, revfair_states);
  CACHE_COPY_BDD(BddStatesInputs, revfair_states_inputs);
  CACHE_COPY_BDD(BddStates, not_successor_states);
  CACHE_COPY_BDD(BddStates, deadlock_states);
  CACHE_COPY_BDD(BddStatesInputs, legal_state_input);
  CACHE_COPY_BDD(BddStatesInputs, monolithic_trans);

  BddFsmCache_copy_reachables(copy, self);

  return copy;
}


/**Function********************************************************************

  Synopsis     [Resets any field in the cache that must be recalculated]

  Description [This is called when syncronous product is carried out.
  In particular LTL BDD-based model checking, after having applied the
  product between the original fsm and the fsm coming from the tableau
  contruction, needs to disable the cache sharing and to recalculate
  fair states and other fields, since the fsm changed.
  All fsm's fields that need to be recalculated after the syncronous
  product must be reset by this method.]

  SideEffects  []

  SeeAlso      [BddFsm_apply_synchronous_product]

******************************************************************************/
void BddFsmCache_reset_not_reusable_fields_after_product(BddFsmCache_ptr self)
{
  BDD_FSM_CACHE_CHECK_INSTANCE(self);

  CACHE_UNREF_BDD(BddStates, fair_states);
  CACHE_UNREF_BDD(BddStatesInputs, fair_states_inputs);
  CACHE_UNREF_BDD(BddStates, revfair_states);
  CACHE_UNREF_BDD(BddStatesInputs, revfair_states_inputs);
  CACHE_UNREF_BDD(BddStates, not_successor_states);
  CACHE_UNREF_BDD(BddStates, deadlock_states);
  CACHE_UNREF_BDD(BddStatesInputs, legal_state_input);
  CACHE_UNREF_BDD(BddStatesInputs, monolithic_trans);
}


/**Function********************************************************************

  Synopsis     [Family soft copier]

  Description  [Returns the same instance, but the family counter is increased,
  in order to handle sharing of the instance. The destructor will actually
  destroy this instance only when the last family member will be destroyed]

  SideEffects  []

  SeeAlso      [BddFsmCache_hard_copy]

******************************************************************************/
BddFsmCache_ptr BddFsmCache_soft_copy(const BddFsmCache_ptr self)
{
  BDD_FSM_CACHE_CHECK_INSTANCE(self);

  (*self->family_counter)++;
  return self;
}


/**Function********************************************************************

  Synopsis     [Copies reachable states information within other into self]

  Description  [This method is used when copying reachable states
  information between to FSMs]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
void BddFsmCache_copy_reachables(BddFsmCache_ptr self,
                                 const BddFsmCache_ptr other)
{
  int count;

  BDD_FSM_CACHE_CHECK_INSTANCE(self);

  if (self->family_counter == other->family_counter) return; /*same family*/

  /* shuts down the current reachables */
  bdd_fsm_cache_deinit_reachables(self);

  self->reachable.computed = other->reachable.computed;
  self->reachable.diameter = other->reachable.diameter;

  /* This can be done under the assumption that the set of reachable
     states of the copy will never be an under-approximation of the
     "other" one */
  if ((bdd_ptr)NULL != other->reachable.reachable_states) {
    self->reachable.reachable_states =
      bdd_dup(other->reachable.reachable_states);
  }

  count = other->reachable.diameter;
  if (count > 0) {
    self->reachable.layers = ALLOC(BddStates, count);
    while (count > 0) {
      bdd_ptr tmp;
      count -= 1;
      tmp = (bdd_ptr) other->reachable.layers[count];
      self->reachable.layers[count] = BDD_STATES(bdd_dup(tmp));
    }
  }
}

/**Function********************************************************************

  Synopsis     [Fills cache structure with reachable states information]

  Description  [Fills cache structure with reachable states
                information.  The given BDD is supposed to represent
                the whole set of reachable states of the Bdd FSM. It
                should NOT contain other informations (such as onion
                rings ecc)]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
void BddFsmCache_set_reachable_states(BddFsmCache_ptr self,
                                      BddStates reachable)
{
  BDD_FSM_CACHE_CHECK_INSTANCE(self);

  if ((bdd_ptr)NULL != self->reachable.reachable_states) {
    bdd_free(self->dd, self->reachable.reachable_states);
  }

  self->reachable.reachable_states = bdd_dup(reachable);
}

/**Function********************************************************************

  Synopsis     [Fills cache structure with reachable states information]

  Description [Given list layers_list must be reversed, from last
  layer to the layer corresponding to initial state.  Given list
  layers_list will be destroyed.]

  SideEffects [given list layers_list will be destroyed, cache
  changes]

  SeeAlso      []

******************************************************************************/
void BddFsmCache_set_reachables(BddFsmCache_ptr self,
                                node_ptr  layers_list,
                                const int diameter,
                                boolean completed)
{
  int i;

  BDD_FSM_CACHE_CHECK_INSTANCE(self);

  /* If we have previously saved states free them */
  if (self->reachable.diameter > 0) {
        for (i = 0; i < self->reachable.diameter; i++) {
                bdd_free(self->dd, self->reachable.layers[i]);
        }
        FREE(self->reachable.layers);
  }

  /* Update the cache */
  self->reachable.computed = completed;
  self->reachable.diameter = diameter;

  self->reachable.layers = ALLOC(BddStates, diameter);
  nusmv_assert(self->reachable.layers != (BddStates*) NULL);

  for (i = diameter-1; i >= 0; --i) {
    node_ptr tmp;
    bdd_ptr  layer;

    layer = BDD_STATES( car(layers_list) );
    self->reachable.layers[i] = bdd_dup(layer);
    tmp = layers_list;
    layers_list = cdr(layers_list);
    bdd_free(self->dd, layer);
    free_node(tmp);
  }
}


/**Function********************************************************************

  Synopsis     [private initializer]

  Description  [private initializer]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
static void bdd_fsm_cache_init(BddFsmCache_ptr self, DdManager* dd)
{
  self->family_counter = ALLOC(unsigned int, 1);
  nusmv_assert(self->family_counter != (unsigned int*) NULL);

  *(self->family_counter) = 1;

  self->dd = dd;

  self->fair_states = BDD_STATES(NULL);
  self->fair_states_inputs = BDD_STATES_INPUTS(NULL);
  self->revfair_states = BDD_STATES(NULL);
  self->revfair_states_inputs = BDD_STATES_INPUTS(NULL);

  self->reachable.computed = false;
  self->reachable.layers   = (BddStates*) NULL;
  self->reachable.diameter = -1;
  self->reachable.reachable_states = (BddStates)NULL;

  self->not_successor_states  = BDD_STATES(NULL);
  self->deadlock_states       = BDD_STATES(NULL);
  self->legal_state_input   = BDD_STATES_INPUTS(NULL);
  self->monolithic_trans      = BDD_STATES_INPUTS(NULL);
}


/**Function********************************************************************

  Synopsis     [private deinitializer]

  Description  [private deinitializer. Call only if family_counter is 0]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
static void bdd_fsm_cache_deinit(BddFsmCache_ptr self)
{
  nusmv_assert(*(self->family_counter) == 0);

  CACHE_UNREF_BDD(BddStates, fair_states);
  CACHE_UNREF_BDD(BddStatesInputs, fair_states_inputs);
  CACHE_UNREF_BDD(BddStates, revfair_states);
  CACHE_UNREF_BDD(BddStatesInputs, revfair_states_inputs);

  bdd_fsm_cache_deinit_reachables(self);

  CACHE_UNREF_BDD(BddStates, not_successor_states);
  CACHE_UNREF_BDD(BddStates, deadlock_states);
  CACHE_UNREF_BDD(BddStatesInputs, legal_state_input);
  CACHE_UNREF_BDD(BddStatesInputs, monolithic_trans);

  FREE(self->family_counter);
}


/**Function********************************************************************

  Synopsis     [private deinitializer for reachables states]

  Description  [Call only if family_counter is 0]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
static void bdd_fsm_cache_deinit_reachables(BddFsmCache_ptr self)
{
  nusmv_assert(*(self->family_counter) <= 1); /* not shared */

  if (self->reachable.computed) {

    while (self->reachable.diameter > 0) {
      bdd_ptr tmp;
      self->reachable.diameter -= 1;
      tmp = (bdd_ptr) self->reachable.layers[self->reachable.diameter];
      bdd_free(self->dd, tmp);
    }

    FREE(self->reachable.layers);
    self->reachable.computed = false;
  }

  if ((bdd_ptr)NULL != self->reachable.reachable_states) {
    bdd_free(self->dd, self->reachable.reachable_states);
  }
}
