/**CHeaderFile*****************************************************************

  FileName    [FsmBuilder.h]

  PackageName [fsm]

  Synopsis [Public interface for a high level object that can contruct
  FSMs]

  Description [Declares the interface of an high-level object that
  lives at top-level, that is used to help contruction of FSMs.  It
  can control information that are not shared between lower levels, so
  it can handle with objects that have not the full knowledge of the
  whole system]
  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm'' package of NuSMV version 2. 
  Copyright (C) 2006 by FBK-irst. 

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



#ifndef __FSM_BUILDER_H__
#define __FSM_BUILDER_H__ 

#include "fsm/sexp/SexpFsm.h"
#include "fsm/sexp/BoolSexpFsm.h"
#include "fsm/bdd/BddFsm.h"

#include "compile/symb_table/SymbTable.h"
#include "dd/dd.h"
#include "trans/bdd/ClusterList.h"
#include "utils/utils.h"


typedef struct FsmBuilder_TAG* FsmBuilder_ptr;


#define FSM_BUILDER(x) \
         ((FsmBuilder_ptr) x)

#define FSM_BUILDER_CHECK_INSTANCE(x) \
         ( nusmv_assert(FSM_BUILDER(x) != FSM_BUILDER(NULL)) )


/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */
EXTERN FsmBuilder_ptr 
FsmBuilder_create ARGS((DdManager* dd));

EXTERN void  FsmBuilder_destroy ARGS((FsmBuilder_ptr self));

EXTERN SexpFsm_ptr 
FsmBuilder_create_scalar_sexp_fsm ARGS((const FsmBuilder_ptr self, 
                                        FlatHierarchy_ptr flat_hierarchy,
                                        const Set_t vars_list));
EXTERN BoolSexpFsm_ptr 
FsmBuilder_create_boolean_sexp_fsm ARGS((const FsmBuilder_ptr self, 
                                         FlatHierarchy_ptr flat_hierarchy,
                                         const Set_t vars,
                                         BddEnc_ptr bdd_enc, 
                                         SymbLayer_ptr det_layer));
EXTERN BddFsm_ptr 
FsmBuilder_create_bdd_fsm ARGS((const FsmBuilder_ptr self, 
                                BddEnc_ptr enc, 
                                const SexpFsm_ptr sexp_fsm, 
                                const TransType trans_type));

EXTERN BddFsm_ptr 
FsmBuilder_create_bdd_fsm_of_vars ARGS((const FsmBuilder_ptr self, 
                                        const SexpFsm_ptr sexp_fsm, 
                                        const TransType trans_type,
                                        BddEnc_ptr enc, 
                                        BddVarSet_ptr state_vars_cube,
                                        BddVarSet_ptr input_vars_cube,
                                        BddVarSet_ptr next_state_vars_cube));

EXTERN ClusterList_ptr 
FsmBuilder_clusterize_expr ARGS((FsmBuilder_ptr self, 
                                 BddEnc_ptr enc, Expr_ptr expr));

#endif /* __FSM_BUILDER_H__ */
