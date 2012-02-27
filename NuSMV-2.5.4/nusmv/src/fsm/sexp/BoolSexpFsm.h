
/**CHeaderFile*****************************************************************

  FileName    [BoolSexpFsm.h]

  PackageName [fsm.sexp]

  Synopsis    [Public interface of class 'BoolSexpFsm']

  Description []

  SeeAlso     [BoolSexpFsm.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.sexp'' package of NuSMV version 2. 
  Copyright (C) 2009 by FBK-irst. 

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

  Revision    [$Id: BoolSexpFsm.h,v 1.1.2.3 2009-12-10 16:06:33 nusmv Exp $]

******************************************************************************/


#ifndef __BOOL_SEXP_FSM_H__
#define __BOOL_SEXP_FSM_H__


#include "SexpFsm.h" 
#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class BoolSexpFsm]

  Description []

******************************************************************************/
typedef struct BoolSexpFsm_TAG*  BoolSexpFsm_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class BoolSexpFsm]

  Description [These macros must be used respectively to cast and to check
  instances of class BoolSexpFsm]

******************************************************************************/
#define BOOL_SEXP_FSM(self) \
         ((BoolSexpFsm_ptr) self)

#define BOOL_SEXP_FSM_CHECK_INSTANCE(self) \
         (nusmv_assert(BOOL_SEXP_FSM(self) != BOOL_SEXP_FSM(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN BoolSexpFsm_ptr 
BoolSexpFsm_create ARGS((const FlatHierarchy_ptr hierarchy, 
                         const Set_t vars_set,
                         BddEnc_ptr benc, SymbLayer_ptr det_layer));

EXTERN BoolSexpFsm_ptr 
BoolSexpFsm_create_from_scalar_fsm ARGS((const SexpFsm_ptr scalar_fsm, 
                                         BddEnc_ptr benc, 
                                         SymbLayer_ptr det_layer));

EXTERN VIRTUAL void BoolSexpFsm_destroy ARGS((BoolSexpFsm_ptr self));

EXTERN BoolEnc_ptr BoolSexpFsm_get_bool_enc ARGS((const BoolSexpFsm_ptr self));

EXTERN BoolSexpFsm_ptr BoolSexpFsm_copy ARGS((BoolSexpFsm_ptr self));


/**AutomaticEnd***************************************************************/



#endif /* __BOOL_SEXP_FSM_H__ */
