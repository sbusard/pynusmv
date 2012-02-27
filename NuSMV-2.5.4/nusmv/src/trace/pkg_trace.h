/**CHeaderFile*****************************************************************

  FileName    [pkg_trace.h]

  PackageName [trace]

  Synopsis    [The header file for the trace package.]

  Description []

  SeeAlso     []

  Author      [Ashutosh Trivedi, Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.

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
#ifndef __PKG_TRACE__H
#define __PKG_TRACE__H

#include "trace/Trace.h"
#include "trace/TraceManager.h"
#include "trace/exec/traceExec.h"
/* #include "trace/exec/TraceExecInfo.h" */

#include "utils/utils.h" /* For EXTERN and ARGS */


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

/* package */
EXTERN void TracePkg_init ARGS((void));
EXTERN void TracePkg_quit ARGS((void));

/* commands */
EXTERN void traceCmd_init ARGS((void));

EXTERN int CommandShowTraces ARGS((int argc, char** argv));
EXTERN int CommandShowPlugins ARGS((int argc, char** argv));
EXTERN int CommandReadTrace ARGS((int argc, char** argv));
EXTERN int CommandExecuteTraces ARGS((int argc, char** argv));
EXTERN int CommandExecutePartialTraces ARGS((int argc, char** argv));

/* self-test */
#if defined TRACE_DEBUG
EXTERN int TracePkg_test_package ARGS((FILE* out, FILE* err));
#endif

/* Filtering services for external packags */
EXTERN NodeList_ptr
TracePkg_get_filtered_symbols ARGS((const NodeList_ptr symbols));

/* Trace Manager */
EXTERN TraceManager_ptr TracePkg_get_global_trace_manager ARGS((void));

EXTERN int TracePkg_get_default_trace_plugin ARGS((void));
EXTERN boolean TracePkg_set_default_trace_plugin ARGS((int dp));

EXTERN execEngine
TracePkg_execution_engine_from_string ARGS((const char* name));

/* Trace execution */
EXTERN int
Trace_execute_trace ARGS((Trace_ptr trace, CompleteTraceExecutor_ptr exec_info));

EXTERN int
Trace_execute_partial_trace ARGS((Trace_ptr trace,
                                  PartialTraceExecutor_ptr exec_info,
                                  NodeList_ptr language));

/* Custom value fetch functions */
EXTERN bdd_ptr
TraceUtils_fetch_as_bdd ARGS((Trace_ptr trace, TraceIter step,
                              TraceIteratorType iter_type,
                              BddEnc_ptr bdd_enc));
EXTERN be_ptr
TraceUtils_fetch_as_be ARGS((Trace_ptr trace, TraceIter step,
                             TraceIteratorType iter_type,
                             BeEnc_ptr be_enc, BddEnc_ptr bdd_enc));

EXTERN Expr_ptr
TraceUtils_fetch_as_sexp ARGS((Trace_ptr trace, TraceIter step,
                               TraceIteratorType iter_type));

EXTERN Expr_ptr
TraceUtils_fetch_as_big_and ARGS((Trace_ptr trace, TraceIter step,
                                  TraceIteratorType iter_type));

EXTERN void TraceUtils_complete_trace ARGS((Trace_ptr trace,
                                            const BoolEnc_ptr bool_enc));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE__H  */
