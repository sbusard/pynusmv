/**CHeaderFile*****************************************************************

  FileName    [simulate.h]

  PackageName [simulate]

  Synopsis    [External Header File for MC Simulator]

  Description [External Header File for simulation package: simulation
  package provides a set of utilities for traces generation (a trace is a
  possible execution of the model). It performs initial state picking,
  trace inspection, simulation according to different policies (deterministic,
  random, interactive) and with the possibility to specify constraints.]

  Author      [Andrea Morichetti]

  Copyright   [
  This file is part of the ``simulate'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and FBK-irst. 

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

  Revision    [$Id: simulate.h,v 1.2.6.6.4.2.6.3 2010-01-29 12:50:45 nusmv Exp $]

******************************************************************************/

#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "utils/utils.h"
#include "dd/dd.h"
#include "fsm/bdd/BddFsm.h"
#include "node/node.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef enum {Deterministic, Random, Interactive} Simulation_Mode;

/*--------------------------------------------------------------------------*/
/* Function prototypes                                                      */
/*--------------------------------------------------------------------------*/

EXTERN void Simulate_Init ARGS((void));
EXTERN void Simulate_End ARGS((void));

EXTERN bdd_ptr 
Simulate_ChooseOneState ARGS((BddFsm_ptr, bdd_ptr, Simulation_Mode, int));

EXTERN void 
Simulate_ChooseOneStateInput ARGS((BddFsm_ptr,
                                   bdd_ptr, bdd_ptr, 
                                   Simulation_Mode, int, 
                                   bdd_ptr*,  bdd_ptr*));

EXTERN node_ptr 
Simulate_MultipleSteps ARGS((BddFsm_ptr, bdd_ptr, boolean,
                             Simulation_Mode, int, int));

EXTERN void store_and_print_trace ARGS((node_ptr, boolean, int));

#endif /* __SIMULATE_H__ */
