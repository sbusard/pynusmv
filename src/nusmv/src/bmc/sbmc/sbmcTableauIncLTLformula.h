/**CHeaderFile*****************************************************************

  FileName    [sbmcTableauIncLTLformula.h]

  PackageName [bmc.sbmc]

  Synopsis    [Public interface for Incremental SBMC tableau-related functionalities]

  Description []

  SeeAlso     []

  Author      [Tommi Juntilla, Marco Roveri]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2006 by Tommi Juntilla

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

  Revision    [$Id: sbmcTableauIncLTLformula.h,v 1.1.2.2.4.2 2007-05-14 16:05:42 nusmv Exp $]

******************************************************************************/

#ifndef _SBMC_LTLTABLEAU__H
#define _SBMC_LTLTABLEAU__H

#include "fsm/be/BeFsm.h"

#include "be/be.h"
#include "node/node.h"
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


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN lsList sbmc_unroll_base ARGS((const BeEnc_ptr be_enc,
                                     const node_ptr ltlspec,
                                     const hash_ptr info_map,
                                     const be_ptr be_LoopExists,
                                     const int do_optimization));

EXTERN lsList sbmc_unroll_invariant_propositional ARGS((const BeEnc_ptr be_enc,
                                                        const node_ptr ltlspec,
                                                        const unsigned int i_model,
                                                        const hash_ptr info_map,
                                                        const be_ptr be_InLoop_i,
                                                        const be_ptr be_l_i,
                                                        const int do_optimization));

EXTERN lsList sbmc_unroll_invariant_f ARGS((const BeEnc_ptr be_enc,
                                                const node_ptr ltlspec,
                                                const unsigned int i_model,
                                                const hash_ptr info_map,
                                                const be_ptr be_InLoop_i,
                                                const be_ptr be_l_i,
                                                const int do_optimization));

EXTERN lsList sbmc_unroll_invariant_p ARGS((const BeEnc_ptr be_enc,
                                            const node_ptr ltlspec,
                                            const unsigned int i_model,
                                            const hash_ptr info_map,
                                            const be_ptr be_InLoop_i,
                                            const be_ptr be_l_i,
                                            const int do_optimization));

EXTERN lsList sbmc_formula_dependent ARGS((const BeEnc_ptr be_enc,
                                           const node_ptr ltlspec,
                                           const unsigned int k_model,
                                           const hash_ptr info_map));

EXTERN lsList sbmc_unroll_invariant ARGS((const BeEnc_ptr be_enc,
                                          const node_ptr bltlspec,
                                          const int previous_k,
                                          const int new_k,
                                          const state_vars_struct *state_vars,
                                          array_t * InLoop_array,
                                          const hash_ptr info_map,
                                          const be_ptr be_LoopExists,
                                          const int opt_do_optimization));

EXTERN lsList sbmc_dependent ARGS((const BeEnc_ptr be_enc,
                                   const node_ptr bltlspec,
                                   const int k,
                                   const state_vars_struct *state_vars,
                                   array_t *InLoop_array,
                                   const be_ptr be_LoopExists,
                                   const hash_ptr info_map));


/**AutomaticEnd***************************************************************/

#endif /* _BMC_TABLEAU__H */
