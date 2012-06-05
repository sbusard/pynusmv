/**CHeaderFile*****************************************************************

   FileName    [bmcBmc.h]

   PackageName [bmc]

   Synopsis    [High-level functionalities interface file]

   Description [High level functionalities allow to perform Bounded Model
   Checking for LTL properties and invariants, as well as simulations.]

   SeeAlso     []

   Author      [Roberto Cavada]

   Copyright   [
   This file is part of the ``bmc'' package of NuSMV version 2.
   Copyright (C) 2000-2001 by FBK-irst and University of Trento.

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

   Revision    [$Id: bmcBmc.h,v 1.2.4.3.2.4.2.3.6.7 2010-02-12 17:14:49 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_BMC_H
#define _BMC_BMC_H

#include "bmcDump.h"
#include "bmc/bmc.h"

#include "fsm/be/BeFsm.h"
#include "fsm/sexp/SexpFsm.h"
#include "trace/Trace.h"

#include "utils/utils.h"
#include "prop/Prop.h"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef enum {BMC_TRUE, BMC_FALSE, BMC_UNKNOWN, BMC_ERROR} Bmc_result;

/**Enum************************************************************************

   Synopsis    [BMC invariant checking closure strategies]

   Description [optional]

   SeeAlso     [optional]

******************************************************************************/
typedef enum {
  BMC_INVAR_BACKWARD_CLOSURE,
  BMC_INVAR_FORWARD_CLOSURE
} bmc_invar_closure_strategy;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN int Bmc_GenSolveLtl ARGS((Prop_ptr ltlprop,
                                 const int k, const int relative_loop,
                                 const boolean must_inc_length,
                                 const boolean must_solve,
                                 const Bmc_DumpType dump_type,
                                 const char* dump_fname_template));


EXTERN int Bmc_GenSolveInvar ARGS((Prop_ptr invarprop,
                                   const boolean must_solve,
                                   const Bmc_DumpType dump_type,
                                   const char* dump_fname_template));

EXTERN Bmc_result Bmc_induction_algorithm ARGS((BeFsm_ptr be_fsm,
                                                node_ptr binvarspec,
                                                Trace_ptr* trace_index,
                                                NodeList_ptr symbols));

EXTERN Bmc_result
Bmc_een_sorensson_algorithm ARGS((BeFsm_ptr be_fsm,
                                  BoolSexpFsm_ptr bool_fsm,
                                  node_ptr binvarspec,
                                  int max_k,
                                  const Bmc_DumpType dump_type,
                                  const char* dump_fname_template,
                                  Prop_ptr pp,
                                  boolean print_steps,
                                  boolean use_extra_step,
                                  Trace_ptr* trace));

EXTERN Bmc_result
Bmc_een_sorensson_algorithm_without_dump ARGS((BeFsm_ptr be_fsm,
                                               BoolSexpFsm_ptr bool_fsm,
                                               node_ptr binvarspec,
                                               int max_k,
                                               boolean use_extra_step,
                                               Trace_ptr* trace));

EXTERN int
Bmc_GenSolveInvar_EenSorensson ARGS((Prop_ptr invarprop,
                                     const int max_k,
                                     const Bmc_DumpType dump_type,
                                     const char* dump_fname_template,
                                     boolean use_extra_step));

/* incremental algorithms */

EXTERN int Bmc_GenSolveLtlInc ARGS((Prop_ptr ltlprop,
                                    const int k, const int relative_loop,
                                    const boolean must_inc_length));

EXTERN int Bmc_GenSolveInvarZigzag ARGS((Prop_ptr invarprop, const int max_k));

EXTERN int Bmc_GenSolveInvarDual ARGS((Prop_ptr invarprop, const int max_k,
                                       bmc_invar_closure_strategy strategy));

EXTERN int Bmc_GenSolveInvarFalsification ARGS((Prop_ptr invarprop,
                                                const int max_k));

EXTERN int Bmc_check_psl_property ARGS((Prop_ptr prop,
                                        boolean dump_prob,
                                        boolean inc_sat,
                                        boolean single_prob,
                                        int k, int rel_loop));

#endif /* _BMC_BMC_H */

