/**CHeaderFile*****************************************************************

  FileName    [bmcSimulate.h]

  PackageName [bmc]

  Synopsis    [SAT Based incremental simulation]

  Description [SAT Bases incremental simulation]

  SeeAlso     [optional]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2010 FBK-irst.

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

  Revision    [$Id: bmcSimulate.h,v 1.1.2.4 2010-02-12 17:14:49 nusmv Exp $]

******************************************************************************/

#ifndef _bmc_simulate
#define _bmc_simulate

#include "util.h"
#include "utils/utils.h"
#include "bmcInt.h"
#include "bmcUtils.h"
#include "bmcConv.h"
#include "trace/pkg_trace.h"
#include "trace/Trace.h"
#include "simulate/simulate.h"
#include "compile/compile.h"
#include "sat/sat.h"
#include "utils/Olist.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int
Bmc_Simulate ARGS((const BeFsm_ptr be_fsm,
                   BddEnc_ptr bdd_enc,
                   be_ptr constraints,
                   boolean time_shift,
                   const int k,
                   const boolean print_trace,
                   const boolean changes_only,
                   Simulation_Mode mode));

EXTERN int
Bmc_StepWiseSimulation ARGS((BeFsm_ptr be_fsm,
                             BddEnc_ptr bdd_enc,
                             TraceManager_ptr trace_manager,
                             int target_steps,
                             be_ptr constraints,
                             boolean time_shift,
                             boolean printtrace,
                             boolean changes_only,
                             Simulation_Mode mode,
                             boolean display_all));
EXTERN Olist_ptr
Bmc_simulate_check_feasible_constraints ARGS((BeFsm_ptr be_fsm,
                                              BddEnc_ptr bdd_enc,
                                              Olist_ptr constraints,
                                              be_ptr from_state));

EXTERN int
Bmc_pick_state_from_constr ARGS((BeFsm_ptr fsm, BddEnc_ptr bdd_enc,
                                 be_ptr constr, Simulation_Mode mode,
                                 boolean display_all));

/**AutomaticEnd***************************************************************/

#endif /* _bmc_simulate */
