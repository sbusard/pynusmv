/**CHeaderFile*****************************************************************

  FileName    [sbmcUtils.h]

  PackageName [bmc.sbmc]

  Synopsis    [Utilities function for SBMC package]

  Description [Utilities function for SBMC package]

  SeeAlso     []

  Author      [Tommi Junttila, Timo Latvala, Marco Roveri]

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2. 
  Copyright (C) 2006 by Tommi Junttila, Timo Latvala.

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

  Revision    [$Id: sbmcUtils.h,v 1.1.2.6.4.6 2010-02-19 15:05:22 nusmv Exp $]

******************************************************************************/

#ifndef _SBMC_UTILS
#define _SBMC_UTILS

#include "prop/propPkg.h"
#include "sbmcStructs.h"
#include "enc/enc.h"
#include "enc/be/BeEnc.h"
#include "trace/Trace.h"
#include "sat/sat.h" /* for solver and result */
#include "utils/utils.h"
#include "utils/assoc.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct sbmc_MetaSolver_TAG sbmc_MetaSolver;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define sbmc_SNH_text "%s:%d: Should not happen"
#define sbmc_SNYI_text "%s:%d: Something not yet implemented\n"


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int sbmc_get_unique_id ARGS((void));
EXTERN void sbmc_reset_unique_id ARGS((void));
EXTERN void sbmc_increment_unique_id ARGS((void));

EXTERN void sbmc_print_node ARGS((FILE * out, const char * prefix, node_ptr node,
                                  const char * postfix));
EXTERN void sbmc_print_node_list ARGS((FILE *out, lsList l));
EXTERN node_ptr sbmc_add_new_state_variable ARGS((SymbLayer_ptr layer, char *name));
EXTERN lsList sbmc_find_formula_vars ARGS((node_ptr ltlspec));
EXTERN void sbmc_print_varmap ARGS((FILE *out, node_ptr node, sbmc_node_info *info));
EXTERN void sbmc_print_Gvarmap ARGS((FILE *out, node_ptr var, node_ptr formula));
EXTERN void sbmc_print_Fvarmap ARGS((FILE *out, node_ptr var, node_ptr formula));
EXTERN node_ptr sbmc_1_fresh_state_var ARGS((SymbLayer_ptr layer, unsigned int *index));
EXTERN array_t * sbmc_n_fresh_state_vars ARGS((SymbLayer_ptr layer, const unsigned int n,
                                  unsigned int *index));
EXTERN void sbmc_allocate_trans_vars ARGS((sbmc_node_info *info, SymbLayer_ptr layer,
                                           lsList state_vars_formula_pd0,
                                           lsList state_vars_formula_pdx,
                                           unsigned int *new_var_index));
EXTERN node_ptr sbmc_make_boolean_formula ARGS((Prop_ptr ltlprop));
EXTERN void sbmc_find_relevant_vars ARGS((state_vars_struct *svs,
                                              BeFsm_ptr be_fsm, node_ptr bltlspec));

EXTERN Trace_ptr
Sbmc_Utils_generate_cntexample ARGS((BeEnc_ptr be_enc, sbmc_MetaSolver * solver,
                                     node_ptr l_var, const int k,
                                     const char * trace_name,
                                     NodeList_ptr symbols));

EXTERN Trace_ptr
Sbmc_Utils_generate_and_print_cntexample ARGS((BeEnc_ptr be_enc,
                                               sbmc_MetaSolver * solver,
                                               node_ptr l_var,
                                               const int k,
                                               const char * trace_name,
                                               NodeList_ptr symbols));

EXTERN Trace_ptr
Sbmc_Utils_fill_cntexample ARGS((BeEnc_ptr be_enc, sbmc_MetaSolver * solver,
                                 node_ptr l_var, const int k, Trace_ptr trace));

EXTERN int sbmc_L_state ARGS((void));
EXTERN int sbmc_E_state ARGS((void));
EXTERN int sbmc_real_k ARGS((int k));
EXTERN unsigned int sbmc_model_k ARGS((int k));
EXTERN char* sbmc_real_k_string ARGS((const unsigned int k_real));


EXTERN sbmc_MetaSolver * sbmc_MS_create ARGS((BeEnc_ptr be_enc));
EXTERN void sbmc_MS_destroy ARGS((sbmc_MetaSolver *ms));
EXTERN void sbmc_MS_create_volatile_group ARGS((sbmc_MetaSolver *ms));
EXTERN void sbmc_MS_destroy_volatile_group ARGS((sbmc_MetaSolver *ms));
EXTERN void sbmc_MS_switch_to_permanent_group ARGS((sbmc_MetaSolver *ms));
EXTERN void sbmc_MS_switch_to_volatile_group ARGS((sbmc_MetaSolver *ms));
EXTERN void sbmc_MS_goto_permanent_group ARGS((sbmc_MetaSolver *ms));
EXTERN void sbmc_MS_goto_volatile_group ARGS((sbmc_MetaSolver *ms));
EXTERN void sbmc_MS_force_true ARGS((sbmc_MetaSolver *ms, be_ptr be_constraint));
EXTERN void sbmc_MS_force_constraint_list ARGS((sbmc_MetaSolver *ms, lsList constraints));
EXTERN SatSolverResult sbmc_MS_solve ARGS((sbmc_MetaSolver *ms));
EXTERN SatSolverResult sbmc_MS_solve_assume ARGS((sbmc_MetaSolver *ms, Slist_ptr assumptions));
EXTERN SatSolver_ptr sbmc_MS_get_solver ARGS((sbmc_MetaSolver *ms));
EXTERN Slist_ptr sbmc_MS_get_conflicts ARGS((sbmc_MetaSolver *ms));
EXTERN Slist_ptr sbmc_MS_get_model ARGS((sbmc_MetaSolver *ms));
EXTERN void sbmc_add_loop_variable ARGS((BeFsm_ptr fsm));
EXTERN void sbmc_remove_loop_variable ARGS((BeFsm_ptr fsm));
EXTERN void sbmc_loop_var_name_set ARGS((node_ptr n));
EXTERN node_ptr sbmc_loop_var_name_get ARGS((void));

/**AutomaticEnd***************************************************************/

#endif /* _SBMC_UTILS */
