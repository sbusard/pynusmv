/**CFile***********************************************************************

  FileName    [traceExec.c]

  PackageName [trace.exec]

  Synopsis    [This module contains the functions needed to support trace
               re-execution]

  Description [This module contains the functions needed to support trace
               re-execution]

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.exec'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK.

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
#include "pkg_trace.h"
#include "pkg_traceInt.h"

#include "prop/propPkg.h"

#include "compile/compile.h"
#include "bmc/bmc.h"
#include "Trace.h"
#include "traceExec.h"

static char rcsid[] UTIL_UNUSED = "$Id: traceExec.c,v 1.1.2.28 2010-02-12 16:25:47 nusmv Exp $";

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Complete trace re-execution]

  Description [Complete trace re-execution.  In order to be run, the
  trace must be complete w.r.t. master fsm language. Returns 0 if a
  trace is executed successfully, and 1 otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int Trace_execute_trace(const Trace_ptr trace,
                        const CompleteTraceExecutor_ptr executor)
{
  boolean success = true;

  TRACE_CHECK_INSTANCE(trace);
  COMPLETE_TRACE_EXECUTOR_CHECK_INSTANCE(executor);

  SexpFsm_ptr sexp_fsm =  \
      PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());

  SEXP_FSM_CHECK_INSTANCE(sexp_fsm);

  if (!Trace_is_complete(trace, SexpFsm_get_vars_list(sexp_fsm), true)) {
        fprintf(nusmv_stderr, "Error: cannot execute incomplete trace.\n");
        success = false;
  }

  else {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr,
              "Executing trace of length %d\n", Trace_get_length(trace));
    }

    /* execute the trace using given executor. */
    success = CompleteTraceExecutor_execute(executor, trace, NIL(int));
  }

  return success ? 0 : 1;
}


/**Function********************************************************************

  Synopsis           [Partial trace re-execution and fill-in]

  Description        [Partial trace re-execution and fill-in.

                      Tries to complete the given trace using the
                      given incomplete trace executor.  If successful,
                      a complete trace is registered into the Trace
                      Manager.

                      0 is returned if trace could be succesfully completed.
                      1 is returned otherwise]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int Trace_execute_partial_trace(const Trace_ptr trace,
                                const PartialTraceExecutor_ptr executor,
                                const NodeList_ptr language)
{
  boolean success = true;

  TRACE_CHECK_INSTANCE(trace);
  PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(executor);
  NODE_LIST_CHECK_INSTANCE(language);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr,
            "Executing trace of length %d\n", Trace_get_length(trace));
  }

  { /* execute a partial trace using given executor, register complete
       trace upon succesful completion */
    Trace_ptr complete_trace = \
      PartialTraceExecutor_execute(executor, trace, language, NIL(int));

    if (TRACE(NULL) != complete_trace) {
      int trace_id = \
        TraceManager_register_trace(TracePkg_get_global_trace_manager(),
                                                 complete_trace);
      fprintf(nusmv_stdout,
              "-- New complete trace is stored at %d index.\n",
              1 + trace_id);
    }
    else { success = false; }
  }

  return success ? 0 : 1;
}


