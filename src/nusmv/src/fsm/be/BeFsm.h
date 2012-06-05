/**CHeaderFile*****************************************************************

  FileName    [BeFsm.h]

  PackageName [fsm.be]

  Synopsis [Public interface of the Finite State Machine class in BE
  format]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.be'' package of NuSMV version 2.
  Copyright (C) 2005 by FBK-irst. 

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

  Revision    [$Id: BeFsm.h,v 1.1.2.3.6.3 2009-09-04 09:22:47 nusmv Exp $]

******************************************************************************/

#ifndef __BE_FSM__H
#define __BE_FSM__H


#include "fsm/sexp/BoolSexpFsm.h"
#include "enc/be/BeEnc.h"

#include "node/node.h"
#include "be/be.h"

#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************

  Synopsis           [This is the BeFsm accessor type]
  Description        []
  SideEffects        []
  SeeAlso            []

******************************************************************************/
typedef struct BeFsm_TAG* BeFsm_ptr;


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class BeEnc]

  Description [These macros must be used respectively to cast and to check
  instances of class BeEnc]

******************************************************************************/
#define BE_FSM(self) \
         ((BeFsm_ptr) self)

#define BE_FSM_CHECK_INSTANCE(self) \
         (nusmv_assert(BE_FSM(self) != BE_FSM(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN BeFsm_ptr 
BeFsm_create ARGS((BeEnc_ptr be_enc, 
                    const be_ptr init, 
                    const be_ptr invar, 
                    const be_ptr trans, 
                    const node_ptr list_of_be_fairness)); 

EXTERN BeFsm_ptr 
BeFsm_create_from_sexp_fsm ARGS((BeEnc_ptr be_enc, 
                                 const BoolSexpFsm_ptr bfsm)); 


EXTERN void BeFsm_destroy ARGS((BeFsm_ptr self)); 

EXTERN BeFsm_ptr BeFsm_copy ARGS((BeFsm_ptr self)); 

EXTERN BeEnc_ptr BeFsm_get_be_encoding ARGS((const BeFsm_ptr self)); 

EXTERN be_ptr BeFsm_get_init ARGS((const BeFsm_ptr self)); 
EXTERN be_ptr BeFsm_get_invar ARGS((const BeFsm_ptr self)); 
EXTERN be_ptr BeFsm_get_trans ARGS((const BeFsm_ptr self)); 
EXTERN node_ptr BeFsm_get_fairness_list ARGS((const BeFsm_ptr self)); 

EXTERN void
BeFsm_apply_synchronous_product ARGS((BeFsm_ptr self, const BeFsm_ptr other));


/**AutomaticEnd***************************************************************/

#endif /* __BE_FSM__H */
