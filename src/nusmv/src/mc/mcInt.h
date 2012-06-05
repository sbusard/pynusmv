/**CHeaderFile*****************************************************************

  FileName    [mcInt.h]

  PackageName [mc]

  Synopsis    [Internal header file of the mc package.]

  Description [Internal header file of the mc package.]

  SeeAlso     [ mcMc.c mcExplain.c]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``mc'' package of NuSMV version 2.
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

  Revision    [$Id: mcInt.h,v 1.3.6.13.4.4 2006-04-11 12:37:17 nusmv Exp $]

******************************************************************************/

#ifndef __MC_INT_H__
#define __MC_INT_H__

#include "utils/utils.h"
#include "dd/dd.h"
#include "node/node.h"
#include "opt/opt.h"
#include "compile/compile.h"
#include "fsm/bdd/BddFsm.h"
#include "fsm/FsmBuilder.h"
#include "trace/TraceManager.h"
#include "trace/Trace.h"

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;
EXTERN DdManager* dd_manager;
EXTERN int yylineno;
EXTERN cmp_struct_ptr cmps;

EXTERN node_ptr one_number;
EXTERN node_ptr zero_number;

EXTERN node_ptr false_const;
EXTERN node_ptr true_const;

EXTERN FsmBuilder_ptr global_fsm_builder;
EXTERN TraceManager_ptr global_trace_manager;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN node_ptr ex_explain ARGS((BddFsm_ptr, BddEnc_ptr, node_ptr, bdd_ptr));
EXTERN node_ptr eu_explain ARGS((BddFsm_ptr, BddEnc_ptr,
                                 node_ptr, bdd_ptr, bdd_ptr));

EXTERN node_ptr eu_si_explain ARGS((BddFsm_ptr fsm, BddEnc_ptr enc,
                                    node_ptr path, bdd_ptr f, bdd_ptr g_si,
                                    bdd_ptr hulk));

EXTERN BddStatesInputs ex_si ARGS((BddFsm_ptr fsm, bdd_ptr si));
EXTERN BddStatesInputs eu_si ARGS((BddFsm_ptr fsm, bdd_ptr f, bdd_ptr g));
EXTERN BddStatesInputs eg_si ARGS((BddFsm_ptr fsm, bdd_ptr g_si));

EXTERN node_ptr
ebu_explain ARGS((BddFsm_ptr, BddEnc_ptr, node_ptr, bdd_ptr,
                  bdd_ptr, int, int));

EXTERN node_ptr eg_explain  ARGS((BddFsm_ptr, BddEnc_ptr, node_ptr, bdd_ptr));
EXTERN node_ptr ebg_explain ARGS((BddFsm_ptr, BddEnc_ptr, node_ptr,
                                  bdd_ptr, int, int));

#endif /* __MC_INT_H__ */
