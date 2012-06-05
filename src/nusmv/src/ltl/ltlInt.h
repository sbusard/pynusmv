/**CHeaderFile*****************************************************************

  FileName    [ltlInt.h]

  PackageName [ltl]

  Synopsis    [Internal header of the <tt>ltl</tt> package.]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``ltl'' package of NuSMV version 2.
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

  Revision    [$Id: ltlInt.h,v 1.3.4.12.4.7.6.1 2009-03-23 20:15:40 nusmv Exp $]

******************************************************************************/

#ifndef __LTL_INT_H__
#define __LTL_INT_H__

#include "utils/utils.h"
#include "node/node.h"
#include "dd/dd.h"
#include "opt/opt.h"

#include "compile/compile.h"
#include "compile/symb_table/SymbTable.h"
#include "compile/symb_table/SymbLayer.h"
#include "compile/symb_table/SymbType.h"

#include "fsm/FsmBuilder.h"
#include "fsm/bdd/BddFsm.h"
#include "trace/TraceManager.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef enum {
  LTL_REWRITE_STANDARD,
  LTL_REWRITE_DEADLOCK_FREE,
} LtlRewriteType;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern cmp_struct_ptr cmps;
extern bdd_ptr trans_bdd;
extern bdd_ptr fair_states_bdd;
extern node_ptr fairness_constraints_bdd;
extern FILE* nusmv_stdout;
extern FILE* nusmv_stderr;
extern DdManager* dd_manager;
extern bdd_ptr invar_bdd;
extern bdd_ptr init_bdd;

EXTERN FsmBuilder_ptr global_fsm_builder;
EXTERN TraceManager_ptr global_trace_manager;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr
witness ARGS((BddFsm_ptr fsm, BddEnc_ptr enc, bdd_ptr feasible));

EXTERN bdd_ptr feasible ARGS((BddFsm_ptr fsm, BddEnc_ptr enc));


EXTERN node_ptr
Ltl_RewriteInput ARGS((SymbTable_ptr symb_table, node_ptr expr,
                       SymbLayer_ptr layer,
                       node_ptr* init, node_ptr* invar, node_ptr* trans,
                       LtlRewriteType rewrite_type));

#endif /* __LTL_INT_H__ */
