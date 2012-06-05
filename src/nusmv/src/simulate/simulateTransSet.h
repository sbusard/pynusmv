/**CHeaderFile*****************************************************************

  FileName    [simulateTransSet.h]

  PackageName [simulate]

  Synopsis    [Pubic interface for class SimulateTransSet]

  Description []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``simulate'' package of NuSMV version 2. 
  Copyright (C) 2004 by FBK-irst. 

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

  Revision    [$Id: simulateTransSet.h,v 1.1.2.1.4.3 2005-04-07 09:17:55 nusmv Exp $]

******************************************************************************/

#ifndef __SIMULATE_TRANS_SET_H__
#define __SIMULATE_TRANS_SET_H__

#include "utils/utils.h"
#include "fsm/bdd/BddFsm.h"
#include "enc/bdd/BddEnc.h"
#include "dd/dd.h"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct SimulateTransSet_TAG* SimulateTransSet_ptr;

#define SIMULATE_TRANS_SET(x) \
   ((SimulateTransSet_ptr) x)

#define SIMULATE_TRANS_SET_CHECK_INSTANCE(x) \
   (nusmv_assert(SIMULATE_TRANS_SET(x) != SIMULATE_TRANS_SET(NULL)))



/*--------------------------------------------------------------------------*/
/* Methods prototypes                                                       */
/*--------------------------------------------------------------------------*/

EXTERN SimulateTransSet_ptr 
SimulateTransSet_create ARGS((BddFsm_ptr fsm, BddEnc_ptr enc, 
			      bdd_ptr from_state, bdd_ptr next_states_set, 
			      double next_states_count));
EXTERN void 
SimulateTransSet_destroy ARGS((SimulateTransSet_ptr self)); 

EXTERN bdd_ptr 
SimulateTransSet_get_from_state ARGS((const SimulateTransSet_ptr self)); 

EXTERN int 
SimulateTransSet_get_next_state_num ARGS((const SimulateTransSet_ptr self)); 

EXTERN bdd_ptr 
SimulateTransSet_get_next_state ARGS((const SimulateTransSet_ptr self, 
				      int state_index));

EXTERN int SimulateTransSet_get_inputs_num_at_state 
ARGS((const SimulateTransSet_ptr self, int state_index));

EXTERN bdd_ptr 
SimulateTransSet_get_input_at_state ARGS((const SimulateTransSet_ptr self, 
					  int state_index, int input_index));

EXTERN int 
SimulateTransSet_print ARGS((const SimulateTransSet_ptr self, 
			     boolean show_changes_only, FILE* output));

EXTERN void 
SimulateTransSet_get_state_input_at ARGS((const SimulateTransSet_ptr self, 
					  int index, 
					  bdd_ptr* state, bdd_ptr* input));

EXTERN void 
SimulateTransSet_get_state_input_rand ARGS((const SimulateTransSet_ptr self, 
					    bdd_ptr* state, bdd_ptr* input));

EXTERN void 
SimulateTransSet_get_state_input_det ARGS((const SimulateTransSet_ptr self, 
					   bdd_ptr* state, bdd_ptr* input));


#endif /* __SIMULATE_TRANS_SET_H__ */
