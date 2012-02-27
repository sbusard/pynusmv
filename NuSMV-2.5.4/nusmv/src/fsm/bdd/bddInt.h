/**CHeaderFile*****************************************************************

  FileName    [bddFsmInt.h]

  PackageName [bdd_fsm]

  Synopsis    [Private interface for package bdd_fsm]

  Description []
  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bdd_fsm'' package of NuSMV version 2. 
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


#ifndef __PKG_BDD_FSM_INT_H__
#define __PKG_BDD_FSM_INT_H__

#include "utils/utils.h" /* for EXTERN and ARGS */
#include "fsm/bdd/BddFsm.h"
#include "dd/dd.h"
#include "opt/opt.h"

EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;


/* members are public from within the bdd fsm */
typedef struct BddFsmCache_TAG 
{
  /* for sharing families (i.e. compatible instances): */
  unsigned int* family_counter;
  
  /* dd manager */
  DdManager* dd;
  
  /* cached values */
  BddStates  fair_states;
  BddStatesInputs fair_states_inputs;
  BddStates  revfair_states;
  BddStatesInputs revfair_states_inputs;

  /* interface to this structure is private */
  struct BddFsmReachable_TAG
  {
    boolean computed;
    BddStates* layers;   /* array of bdds */
    int diameter;
    BddStates reachable_states; /* Used to hold the bdd representing the
                                   whole set of reachable states of the
                                   BddFsm.  These may be computed for
                                   example by Guided Reachability */
  } reachable;

  BddStates successor_states;
  BddStates not_successor_states;
  BddStates deadlock_states;
  BddStatesInputs legal_state_input; 
  BddStatesInputs monolithic_trans;

} BddFsmCache;


typedef struct BddFsmCache_TAG* BddFsmCache_ptr;

#define BDD_FSM_CACHE(x) \
         ( (BddFsmCache_ptr) x )

#define BDD_FSM_CACHE_CHECK_INSTANCE(x) \
         ( nusmv_assert(BDD_FSM_CACHE(x) != BDD_FSM_CACHE(NULL)) ) 


#define CACHE_SET(member, value) \
         (self->cache->member = value)

#define CACHE_GET(member) \
         (self->cache->member)

#define CACHE_SET_BDD(member, value) \
         (self->cache->member = bdd_dup(value))

#define CACHE_GET_BDD(member) \
         (bdd_dup(self->cache->member))


#define CACHE_IS_EQUAL(member, value) \
         (self->cache->member == value)


EXTERN BddFsmCache_ptr BddFsmCache_create ARGS((DdManager* dd));

EXTERN void BddFsmCache_destroy ARGS((BddFsmCache_ptr self));

EXTERN BddFsmCache_ptr 
BddFsmCache_hard_copy ARGS((const BddFsmCache_ptr self));

EXTERN BddFsmCache_ptr 
BddFsmCache_soft_copy ARGS((const BddFsmCache_ptr self));

EXTERN void 
BddFsmCache_copy_reachables ARGS((BddFsmCache_ptr self, 
                                  const BddFsmCache_ptr other));

EXTERN void BddFsmCache_set_reachable_states ARGS((BddFsmCache_ptr self,
                                                   BddStates reachable));

EXTERN void BddFsmCache_set_reachables ARGS((BddFsmCache_ptr self, 
                                             node_ptr   layers_list, 
                                             const int  diameter,
                                             boolean completed));

EXTERN void 
BddFsmCache_reset_not_reusable_fields_after_product
ARGS((BddFsmCache_ptr self));

#endif /* __PKG_BDD_FSM_INT_H__ */
