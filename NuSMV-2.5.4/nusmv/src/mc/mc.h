/**CHeaderFile*****************************************************************

  FileName    [mc.h]

  PackageName [mc]

  Synopsis    [Fair CTL model checking algorithms. External header file.]

  Description [Fair CTL model checking algorithms. External header file.]

  Author      [Marco Roveri, Roberto Cavada]

  Revision    [$Id: mc.h,v 1.5.4.16.4.5.6.8 2009-12-11 15:08:04 nusmv Exp $]

  Copyright   [
  This file is part of the ``mc'' package of NuSMV version 2.
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

******************************************************************************/

#ifndef __MC_H__
#define __MC_H__

#include "utils/utils.h"
#include "dd/dd.h"
#include "prop/Prop.h"
#include "fsm/bdd/BddFsm.h"
#include "trace/Trace.h"
#include "opt/opt.h"

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void Mc_Init ARGS((void));
EXTERN void Mc_End ARGS((void));

EXTERN void Mc_CheckCTLSpec ARGS((Prop_ptr prop));
EXTERN void Mc_CheckAGOnlySpec ARGS((Prop_ptr prop));
EXTERN void Mc_CheckInvar ARGS((Prop_ptr prop));

EXTERN void Mc_CheckInvarSilently ARGS((Prop_ptr prop,
                                        Trace_ptr* trace));

EXTERN void
Mc_CheckInvar_With_Strategy ARGS((Prop_ptr prop,
                                  Check_Strategy strategy,
                                  Trace_ptr* trace,
                                  boolean silent));
EXTERN void
Mc_CheckInvar_With_Strategy_And_Symbols ARGS((Prop_ptr prop,
                                              Check_Strategy strategy,
                                              Trace_ptr* trace,
                                              boolean silent,
                                              NodeList_ptr symbols));

EXTERN void Mc_CheckCompute ARGS((Prop_ptr prop));
EXTERN int Mc_check_psl_property ARGS((Prop_ptr prop));

EXTERN void Mc_CheckLanguageEmptiness
ARGS((const BddFsm_ptr fsm, boolean allinit, boolean verbose));

EXTERN SexpFsm_ptr
Mc_rewrite_invar_get_sexp_fsm ARGS((const Prop_ptr prop,
                                    SymbLayer_ptr layer,
                                    node_ptr* created_var));

/* mcTrace.c */
EXTERN Trace_ptr
Mc_create_trace_from_bdd_state_input_list ARGS((const BddEnc_ptr bdd_enc,
                                                const NodeList_ptr symbols,
                                                const char* desc,
                                                const TraceType type,
                                                node_ptr path));

EXTERN Trace_ptr
Mc_fill_trace_from_bdd_state_input_list ARGS((const BddEnc_ptr bdd_enc,
                                              Trace_ptr trace,
                                              node_ptr path));

EXTERN void
Mc_trace_step_put_state_from_bdd ARGS((Trace_ptr trace, TraceIter step,
                                       BddEnc_ptr bdd_enc, bdd_ptr bdd));

EXTERN void
Mc_trace_step_put_input_from_bdd ARGS((Trace_ptr trace, TraceIter step,
                                       BddEnc_ptr bdd_enc, bdd_ptr bdd));


EXTERN void print_spec  ARGS((FILE *file, Prop_ptr prop));
EXTERN void print_invar ARGS((FILE *file, Prop_ptr n));
EXTERN void print_compute ARGS((FILE *, Prop_ptr));

EXTERN BddStates ex      ARGS((BddFsm_ptr, BddStates));
EXTERN BddStates ef      ARGS((BddFsm_ptr, BddStates));
EXTERN BddStates eg      ARGS((BddFsm_ptr, BddStates));
EXTERN BddStates eu      ARGS((BddFsm_ptr, BddStates, BddStates));
EXTERN BddStates au      ARGS((BddFsm_ptr, BddStates, BddStates));

EXTERN BddStates ebu     ARGS((BddFsm_ptr, BddStates, BddStates, int, int));
EXTERN BddStates ebf     ARGS((BddFsm_ptr, BddStates, int, int));
EXTERN BddStates ebg     ARGS((BddFsm_ptr, BddStates, int, int));
EXTERN BddStates abu     ARGS((BddFsm_ptr, BddStates, BddStates, int, int));

EXTERN int       minu    ARGS((BddFsm_ptr, bdd_ptr, bdd_ptr));
EXTERN int       maxu    ARGS((BddFsm_ptr, bdd_ptr, bdd_ptr));

EXTERN node_ptr explain  ARGS((BddFsm_ptr, BddEnc_ptr, node_ptr,
                               node_ptr, node_ptr));

EXTERN bdd_ptr
eval_ctl_spec ARGS((BddFsm_ptr, BddEnc_ptr enc, node_ptr, node_ptr));

EXTERN node_ptr
eval_formula_list ARGS((BddFsm_ptr, BddEnc_ptr enc, node_ptr, node_ptr));

EXTERN int
eval_compute ARGS((BddFsm_ptr, BddEnc_ptr enc, node_ptr, node_ptr));

EXTERN int      check_invariant_forward     ARGS((BddFsm_ptr, Prop_ptr));

EXTERN void     free_formula_list ARGS((DdManager *, node_ptr));

#endif /* __MC_H__ */
