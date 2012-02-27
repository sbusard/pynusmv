/**CHeaderFile*****************************************************************

  FileName    [bmcModel.h]

  PackageName [bmc]

  Synopsis    [Public interface for the model-related functionalities]

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

  Revision    [$Id: bmcModel.h,v 1.2.4.1.2.1.2.3.4.1 2010-01-29 15:22:32 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_MODEL__H
#define _BMC_MODEL__H


#include "utils/utils.h"
#include "be/be.h"
#include "node/node.h"

#include "fsm/be/BeFsm.h"


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
EXTERN be_ptr Bmc_Model_GetInit0 ARGS((const BeFsm_ptr be_fsm));
EXTERN be_ptr Bmc_Model_GetInitI ARGS((const BeFsm_ptr be_fsm, const int i));

EXTERN be_ptr Bmc_Model_GetInvarAtTime ARGS((const BeFsm_ptr be_fsm,
                                             const int time));

EXTERN be_ptr Bmc_Model_GetTransAtTime ARGS((const BeFsm_ptr be_fsm,
                                             const int time));

EXTERN be_ptr
Bmc_Model_GetUnrolling ARGS((const BeFsm_ptr be_fsm,
                             const int j, const int k));

EXTERN be_ptr
Bmc_Model_GetPathNoInit ARGS((const BeFsm_ptr be_fsm, const int k));

EXTERN be_ptr
Bmc_Model_GetPathWithInit ARGS((const BeFsm_ptr be_fsm, const int k));

EXTERN be_ptr
Bmc_Model_GetFairness ARGS((const BeFsm_ptr be_fsm,
                            const int k, const int l));

EXTERN be_ptr 
Bmc_Model_Invar_Dual_forward_unrolling ARGS((const BeFsm_ptr be_fsm,
                                             const be_ptr invarspec, 
                                             int i));
/**AutomaticEnd***************************************************************/

#endif /* _BMC_MODEL__H */
