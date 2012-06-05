/**CFile***********************************************************************

  FileName    [PartialTraceExecutor.c]

  PackageName [trace.exec]

  Synopsis    [Implementation of class 'PartialTraceExecutor']

  Description []

  SeeAlso     [PartialTraceExecutor.h]

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.exec'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK-irst.

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

  Revision    [$Id: $]

******************************************************************************/
#include "BaseTraceExecutor.h"
#include "PartialTraceExecutor.h"
#include "PartialTraceExecutor_private.h"

#include "trace/Trace.h"
#include "trace/Trace_private.h"

#include "utils/utils.h"
#include "opt/opt.h"

static char rcsid[] UTIL_UNUSED = "$Id: $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PartialTraceExecutor_private.h' for class 'PartialTraceExecutor' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void partial_trace_executor_finalize ARGS((Object_ptr object,
                                                  void* dummy));

static Trace_ptr
partial_trace_executor_execute ARGS((const PartialTraceExecutor_ptr self,
                                     const Trace_ptr trace,
                                     NodeList_ptr language,
                                     int* n_steps));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Executes a partial trace]

  Description [Tries to execute a partial trace on FSM provided at
  construction time. If execution is succesfully completed, a valid
  complete trace is built on language and returned. A NULL Trace is
  retured otherwise.

  The number of performed steps is stored in *n_steps, if n_steps is
  non-NULL. This is -1 if given trace has no feasible initial state.]

  SideEffects [A complete Trace on language is created upon successful
  completion]

  SeeAlso     [CompleteTraceExecutor_execute]

******************************************************************************/
Trace_ptr PartialTraceExecutor_execute (const PartialTraceExecutor_ptr self,
                                        const Trace_ptr trace,
                                        const NodeList_ptr language,
                                        int* n_steps)
{
  return (*self->execute)(self, trace, language, n_steps);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The PartialTraceExecutor class private initializer]

  Description        [The PartialTraceExecutor class private initializer]

  SideEffects        []

  SeeAlso            [PartialTraceExecutor_create]

******************************************************************************/
void partial_trace_executor_init(PartialTraceExecutor_ptr self)
{
  /* base class initialization */
  trace_executor_init(BASE_TRACE_EXECUTOR(self));

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = partial_trace_executor_finalize;

  /* virtual abstract partial execution method */
  OVERRIDE(PartialTraceExecutor, execute) = partial_trace_executor_execute;
}


/**Function********************************************************************

  Synopsis           [The PartialTraceExecutor class private deinitializer]

  Description        [The PartialTraceExecutor class private deinitializer]

  SideEffects        []

  SeeAlso            [PartialTraceExecutor_destroy]

******************************************************************************/
void partial_trace_executor_deinit(PartialTraceExecutor_ptr self)
{
  /* members deinitialization */


  /* base class deinitialization */
  trace_executor_deinit(BASE_TRACE_EXECUTOR(self));
}


/**Function********************************************************************

  Synopsis    [Private service for loopback checking]

  Description [Returns true iff all loopback information for
  partial_trace applies to the complete trace as well]

  SideEffects [none]

  SeeAlso     [complete_trace_executor_check_loopbacks]

******************************************************************************/
boolean
partial_trace_executor_check_loopbacks (const PartialTraceExecutor_ptr self,
                                        const Trace_ptr partial_trace,
                                        const Trace_ptr complete_trace)
{
  boolean res = true;

  /* local reference to superclass */
  const BaseTraceExecutor_ptr executor = BASE_TRACE_EXECUTOR(self);
  TraceIter partial_step;
  TraceIter complete_step;

  int i = 1;

  PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  TRACE_CHECK_INSTANCE(partial_trace);
  TRACE_CHECK_INSTANCE(complete_trace);

  partial_step = trace_first_iter(partial_trace);
  complete_step = trace_first_iter(complete_trace);
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(BaseTraceExecutor_get_error_stream(executor),
            "now checking loopbacks...\n");
  }
  while (TRACE_END_ITER != partial_step) {
    if ((trace_step_is_loopback(partial_trace, partial_step)) &&
        !trace_step_test_loopback(complete_trace, complete_step))  {

      fprintf(BaseTraceExecutor_get_error_stream(executor), "*** Error ***\n"
              "Inconsistent loopback information found at step %d.\n", i);

      res = false;
      break; /*  continuing is pointless */
    }

    ++ i;
    partial_step = trace_iter_get_next(partial_step);
    complete_step = trace_iter_get_next(complete_step);
  } /* trace foreach */

  return res;
}

/**Function********************************************************************

  Synopsis    [Returns true iff the given step is a complete assignment
  to state vars in the given trace]

  Description [Returns true iff the given step is a complete assignment
  to state vars in the given trace]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean
partial_trace_executor_is_complete_state (const PartialTraceExecutor_ptr self,
                                          const Trace_ptr trace,
                                          const TraceIter step)
{
  TraceSymbolsIter iter;
  node_ptr symb, val;

  PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  TRACE_CHECK_INSTANCE(trace);

  TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_SF_VARS, iter, symb) {
    val = Trace_step_get_value(trace, step, symb);
    if (Nil == val) return false;
  }

  return true;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The PartialTraceExecutor class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void partial_trace_executor_finalize(Object_ptr object, void* dummy)
{
  PartialTraceExecutor_ptr self = PARTIAL_TRACE_EXECUTOR(object);

  partial_trace_executor_deinit(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis    [Abstract execution method for partial trace executors]

  Description [This is a pure virtual functions. Every derived class
  must ovewrwrite this function. Raises an assertion if invoked]

  SideEffects []

  SeeAlso     []

******************************************************************************/
 Trace_ptr partial_trace_executor_execute(const PartialTraceExecutor_ptr self,
                                          const Trace_ptr trace,
                                          NodeList_ptr language, int* n_steps)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
  return 0;
}


/**AutomaticEnd***************************************************************/

