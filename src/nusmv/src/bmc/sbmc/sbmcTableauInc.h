/**CHeaderFile*****************************************************************

  FileName    [sbmcTableauInc.h]

  PackageName [bmc.sbmc]

  Synopsis    [Tableau function for SBMC package]

  Description [Tableau function for SBMC package]

  SeeAlso     []

  Author      [Tommi Junttila, Marco Roveri]

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2. 
  Copyright (C) 2006 by Tommi Junttila.

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

  Revision    [$Id: sbmcTableauInc.h,v 1.1.2.2.4.2 2007-05-14 16:05:42 nusmv Exp $]

******************************************************************************/

#ifndef _SBMC_TABLEAUINC
#define _SBMC_TABLEAUINC

#include "enc/enc.h"
#include "enc/be/BeEnc.h"
#include "trace/Trace.h"
#include "sbmcStructs.h"
#include "utils/utils.h"

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
#define sbmc_SNH_text "%s:%d: Should not happen"
#define sbmc_SNYI_text "%s:%d: Something not yet implemented\n"


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN be_ptr sbmc_equal_vectors_formula ARGS((const BeEnc_ptr be_enc,
                                               lsList vars,
                                               const unsigned int i,
                                               const unsigned int j));

EXTERN hash_ptr sbmc_init_LTL_info ARGS((SymbLayer_ptr layer, node_ptr ltlspec,
                                             lsList state_vars_formula_pd0,
                                             lsList state_vars_formula_pdx,
                                             lsList state_vars_formula_aux,
                                             const int opt_force_state_vars,
                                             const int opt_do_virtual_unrolling));

EXTERN void sbmc_init_state_vector ARGS((const BeEnc_ptr be_enc,
                                         const node_ptr ltlspec,
                                         const hash_ptr info_map,
                                         const unsigned int i_real,
                                         const node_ptr LastState_var,
                                         const be_ptr be_LoopExists));

EXTERN be_ptr sbmc_build_InLoop_i ARGS((const BeEnc_ptr be_enc,
                                            const state_vars_struct * state_vars,
                                            array_t *InLoop_array,
                                            const unsigned int i_model));

EXTERN lsList sbmc_SimplePaths ARGS((const BeEnc_ptr be_enc,
                                         const state_vars_struct *state_vars,
                            array_t *InLoop_array,
                                         const unsigned int k));

/**AutomaticEnd***************************************************************/

#endif /* _SBMC_TABLEAUINC */
