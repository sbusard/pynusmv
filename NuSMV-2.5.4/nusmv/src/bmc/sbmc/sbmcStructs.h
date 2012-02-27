/**CHeaderFile*****************************************************************

  FileName    [sbmcStructs.h]

  PackageName [bmc.sbmc]

  Synopsis    [Structures used within the SBMC package]

  Description [Structures used within the SBMC package]

  SeeAlso     [optional]

  Author      [Timo Latvala, Tommi Junttila, Marco Roveri]

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

  Revision    [$Id: sbmcStructs.h,v 1.1.2.2.4.2 2007-05-16 14:21:22 nusmv Exp $]

******************************************************************************/

#ifndef _SBCM_STRUCTS
#define _SBCM_STRUCTS


#include "node/node.h"

#include "utils/utils.h"
#include "utils/list.h"
#include "utils/assoc.h"
#include "utils/array.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct state_vars_struct_TAG  state_vars_struct;
typedef struct sbmc_node_info_struct sbmc_node_info;

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
EXTERN state_vars_struct* sbmc_state_vars_create ARGS((void));
EXTERN void sbmc_state_vars_destroy ARGS((state_vars_struct* svs));

EXTERN lsList sbmc_state_vars_get_trans_state_vars ARGS((const state_vars_struct * ss));
EXTERN node_ptr sbmc_state_vars_get_l_var ARGS((const state_vars_struct * ss));
EXTERN node_ptr sbmc_state_vars_get_LoopExists_var ARGS((const state_vars_struct * ss));
EXTERN node_ptr sbmc_state_vars_get_LastState_var ARGS((const state_vars_struct * ss));
EXTERN lsList sbmc_state_vars_get_translation_vars_pd0 ARGS((const state_vars_struct * ss));
EXTERN lsList sbmc_state_vars_get_translation_vars_pdx ARGS((const state_vars_struct * ss));
EXTERN lsList sbmc_state_vars_get_translation_vars_aux ARGS((const state_vars_struct * ss));
EXTERN lsList sbmc_state_vars_get_formula_state_vars ARGS((const state_vars_struct * ss));
EXTERN lsList sbmc_state_vars_get_formula_input_vars ARGS((const state_vars_struct * ss));
EXTERN lsList sbmc_state_vars_get_simple_path_system_vars ARGS((const state_vars_struct * ss));
EXTERN void sbmc_state_vars_set_trans_state_vars ARGS((state_vars_struct * ss, lsList f));
EXTERN void sbmc_state_vars_set_l_var ARGS((state_vars_struct * ss, node_ptr f));
EXTERN void sbmc_state_vars_set_LoopExists_var ARGS((state_vars_struct * ss, node_ptr f));
EXTERN void sbmc_state_vars_set_LastState_var ARGS((state_vars_struct * ss, node_ptr f));
EXTERN void sbmc_state_vars_set_translation_vars_pd0 ARGS((state_vars_struct * ss, lsList f));
EXTERN void sbmc_state_vars_set_translation_vars_pdx ARGS((state_vars_struct * ss, lsList f));
EXTERN void sbmc_state_vars_set_translation_vars_aux ARGS((state_vars_struct * ss, lsList f));
EXTERN void sbmc_state_vars_set_formula_state_vars ARGS((state_vars_struct * ss, lsList f));
EXTERN void sbmc_state_vars_set_formula_input_vars ARGS((state_vars_struct * ss, lsList f));
EXTERN void sbmc_state_vars_set_simple_path_system_vars ARGS((state_vars_struct * ss, lsList f));
EXTERN void sbmc_state_vars_print ARGS((state_vars_struct *svs, FILE* out));
EXTERN hash_ptr sbmc_set_create ARGS((void));
EXTERN void sbmc_set_destroy ARGS((hash_ptr hash));
EXTERN void sbmc_set_insert ARGS((hash_ptr hash, node_ptr bexp));
EXTERN int sbmc_set_is_in ARGS((hash_ptr hash, node_ptr bexp));
EXTERN sbmc_node_info * sbmc_alloc_node_info ARGS((void));
EXTERN void sbmc_node_info_free ARGS((sbmc_node_info * info));
EXTERN unsigned int sbmc_node_info_get_past_depth ARGS((sbmc_node_info * h)); 
EXTERN array_t * sbmc_node_info_get_trans_vars ARGS((sbmc_node_info * h)); 
EXTERN array_t * sbmc_node_info_get_trans_bes ARGS((sbmc_node_info * h)); 
EXTERN node_ptr sbmc_node_info_get_aux_F_node ARGS((sbmc_node_info * h)); 
EXTERN array_t * sbmc_node_info_get_aux_F_trans ARGS((sbmc_node_info * h)); 
EXTERN node_ptr sbmc_node_info_get_aux_G_node ARGS((sbmc_node_info * h)); 
EXTERN array_t * sbmc_node_info_get_aux_G_trans ARGS((sbmc_node_info * h)); 
EXTERN void sbmc_node_info_set_past_depth ARGS((sbmc_node_info * h, unsigned int s)); 
EXTERN void sbmc_node_info_set_past_trans_vars ARGS((sbmc_node_info * h, array_t * s)); 
EXTERN void sbmc_node_info_set_trans_bes ARGS((sbmc_node_info * h, array_t * s)); 
EXTERN void sbmc_node_info_set_aux_F_node ARGS((sbmc_node_info * h, node_ptr s)); 
EXTERN void sbmc_node_info_set_aux_F_trans ARGS((sbmc_node_info * h, array_t * s)); 
EXTERN void sbmc_node_info_set_aux_G_node ARGS((sbmc_node_info * h, node_ptr s)); 
EXTERN void sbmc_node_info_set_aux_G_trans ARGS((sbmc_node_info * h, array_t * s)); 
EXTERN sbmc_node_info * sbmc_node_info_assoc_find ARGS((hash_ptr a, node_ptr n));
EXTERN void sbmc_node_info_assoc_insert ARGS((hash_ptr a, node_ptr n, sbmc_node_info * i));
EXTERN void sbmc_node_info_assoc_free ARGS((hash_ptr * a));
EXTERN hash_ptr sbmc_node_info_assoc_create ARGS((void));

/**AutomaticEnd***************************************************************/

#endif /* _SBCM_UTIL */
