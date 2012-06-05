/**CHeaderFile*****************************************************************

  FileName    [bmcInt.h]

  PackageName [bmc]

  Synopsis    [The private interfaces for the <tt>bmc</tt> package]

  Description []

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

  Revision    [$Id: bmcInt.h,v 1.31.2.7.2.5.2.4.6.10 2010-02-12 17:14:49 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_INT_H
#define _BMC_INT_H


#include <time.h>
#include <limits.h>
#include <stdio.h>


#include "enc/be/BeEnc.h"

#include "fsm/FsmBuilder.h"
#include "compile/compile.h"

#include "be/be.h"

#include "trace/Trace.h"
#include "trace/TraceManager.h"

#include "dd/dd.h"
#include "opt/opt.h"
#include "node/node.h"
#include "utils/utils.h"

/* Uncomment the following line to print out benchmarking info */
/* #define BENCHMARKING */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define BMC_DUMP_FILENAME_MAXLEN 4096

#define BMC_NO_PROPERTY_INDEX -1

#define BMC_BEXP_OUTPUT_SMV 0
#define BMC_BEXP_OUTPUT_LB 1

/* BMC Options  default values */
#define DEFAULT_DIMACS_FILENAME        "@f_k@k_l@l_n@n"
#define DEFAULT_INVAR_DIMACS_FILENAME  "@f_invar_n@n"
#define DEFAULT_BMC_PB_LENGTH     10
#define DEFAULT_BMC_PB_LOOP         Bmc_Utils_GetAllLoopbacksString()
#define DEFAULT_BMC_INVAR_ALG       "classic"
#define DEFAULT_BMC_INC_INVAR_ALG   "dual"
#define DEFAULT_BMC_OPTIMIZED_TABLEAU 1
#define DEFAULT_BMC_FORCE_PLTL_TABLEAU 0


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;
EXTERN cmp_struct_ptr cmps;

EXTERN DdManager* dd_manager;
EXTERN FsmBuilder_ptr global_fsm_builder;
EXTERN TraceManager_ptr global_trace_manager;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/


/**Enum************************************************************************

  Synopsis    [BMC invariant checking algorithms]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
typedef enum {
  ALG_UNDEFINED,
  ALG_CLASSIC,
  ALG_EEN_SORENSSON,
  ALG_FALSIFICATION,
  ALG_DUAL,
  ALG_ZIGZAG,

  ALG_INTERP_SEQ,
  ALG_INTERPOLANTS,
} bmc_invar_algorithm;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void bmc_init_wff2nnf_hash ARGS((void));

EXTERN void bmc_quit_wff2nnf_hash ARGS((void));

EXTERN void bmc_quit_tableau_memoization ARGS((void));

EXTERN node_ptr
bmc_tableau_memoization_get_key ARGS((node_ptr wff, int time, int k, int l));

EXTERN void bmc_tableau_memoization_insert ARGS((node_ptr key, be_ptr be));
EXTERN be_ptr bmc_tableau_memoization_lookup ARGS((node_ptr key));

EXTERN int
Bmc_TestTableau ARGS((int argc, char ** argv));

EXTERN void Bmc_TestReset    ARGS((void));


EXTERN void
Bmc_PrintStats ARGS((Be_Manager_ptr beManager, int clustSize, FILE* outFile));


EXTERN boolean
isPureFuture ARGS((const node_ptr pltl_wff));

EXTERN be_ptr
Bmc_GetTestTableau ARGS((const BeEnc_ptr be_enc,
                         const node_ptr ltl_wff,
                         const int k, const int l));

EXTERN be_ptr
BmcInt_Tableau_GetAtTime ARGS((const BeEnc_ptr be_enc,
                               const node_ptr ltl_wff,
                               const int time, const int k, const int l));

/* ================================================== */
/* Tableaux for an LTL formula:                       */
EXTERN be_ptr
bmc_tableauGetNextAtTime ARGS((const BeEnc_ptr be_enc,
                               const node_ptr ltl_wff,
                               const int time, const int k, const int l));

EXTERN be_ptr
bmc_tableauGetEventuallyAtTime ARGS((const BeEnc_ptr be_enc,
                                     const node_ptr ltl_wff,
                                     const int intime, const int k,
                                     const int l));

EXTERN be_ptr
bmc_tableauGetGloballyAtTime ARGS((const BeEnc_ptr be_enc,
                                   const node_ptr ltl_wff,
                                   const int intime, const int k,
                                   const int l));

EXTERN be_ptr
bmc_tableauGetUntilAtTime ARGS((const BeEnc_ptr be_enc,
                                const node_ptr p, const node_ptr q,
                                const int time, const int k, const int l));

EXTERN be_ptr
bmc_tableauGetReleasesAtTime ARGS((const BeEnc_ptr be_enc,
                                   const node_ptr p, const node_ptr q,
                                   const int time, const int k, const int l));
/* ================================================== */


/* ================================================== */
/* Tableaux for a PLTL formula:                       */
EXTERN be_ptr
Bmc_TableauPLTL_GetTableau ARGS((const BeEnc_ptr be_enc,
                                 const node_ptr pltl_wff,
                                 const int k, const int l));

EXTERN be_ptr
Bmc_TableauPLTL_GetAllTimeTableau ARGS((const BeEnc_ptr be_enc,
                                        const node_ptr pltl_wff,
                                        const int k));
/* ================================================== */


/* ================================================== */
/* Conv module:                                       */
EXTERN void Bmc_Conv_init_cache ARGS((void));
EXTERN void Bmc_Conv_quit_cache ARGS((void));

/* ================================================== */


/* ================================================== */
/* Utils module:                                      */

EXTERN lsList
Bmc_Utils_get_vars_list_for_uniqueness ARGS((BeEnc_ptr be_enc,
                                             Prop_ptr invarprop));

EXTERN lsList
Bmc_Utils_get_vars_list_for_uniqueness_fsm ARGS((BeEnc_ptr be_enc,
                                                 SexpFsm_ptr bool_fsm));

/* ================================================== */


/* Rewrite INVARSPECs */

EXTERN Prop_ptr Bmc_rewrite_invar ARGS((const Prop_ptr prop,
                                        const BddEnc_ptr bdd_enc,
                                        SymbLayer_ptr layer));

EXTERN void Bmc_rewrite_cleanup ARGS((Prop_ptr rewritten_prop,
                                      const BddEnc_ptr bdd_enc,
                                      SymbLayer_ptr layer));

/* ================================================== */
/* Simulation                                         */
void bmc_simulate_set_curr_sim_trace ARGS((Trace_ptr trace, int idx));
Trace_ptr bmc_simulate_get_curr_sim_trace ARGS((void));
int bmc_simulate_get_curr_sim_trace_index ARGS((void));
/* ================================================== */

EXTERN Trace_ptr
Bmc_create_trace_from_cnf_model ARGS((const BeEnc_ptr be_enc,
                                      const NodeList_ptr symbols,
                                      const char* desc,
                                      const TraceType type,
                                      const Slist_ptr cnf_model,
                                      int k));

EXTERN Trace_ptr
Bmc_fill_trace_from_cnf_model ARGS((const BeEnc_ptr be_enc,
                                    const Slist_ptr cnf_model,
                                    int k, Trace_ptr trace));

/* internal bmc/trace utils */
EXTERN void
bmc_trace_utils_complete_trace ARGS((Trace_ptr trace,
                                     const BoolEnc_ptr bool_enc));

EXTERN void
bmc_trace_utils_append_input_state ARGS((Trace_ptr trace, BeEnc_ptr be_enc,
                                         const Slist_ptr cnf_model));

/**AutomaticEnd***************************************************************/

#endif /* _BMC_INT_H */

























