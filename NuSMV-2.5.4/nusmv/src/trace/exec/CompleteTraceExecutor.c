/**CFile***********************************************************************

  FileName    [CompleteTraceExecutor.c]

  PackageName [trace.exec]

  Synopsis    [Implementation of class 'CompleteTraceExecutor']

  Description []

  SeeAlso     [CompleteTraceExecutor.h]

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

#include "CompleteTraceExecutor.h"
#include "CompleteTraceExecutor_private.h"

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
/* See 'CompleteTraceExecutor_private.h' for class 'CompleteTraceExecutor' definition. */

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

static void complete_trace_executor_finalize ARGS((Object_ptr object,
                                                   void* dummy));

static boolean
complete_trace_executor_execute ARGS((const CompleteTraceExecutor_ptr self,
                                      const Trace_ptr trace, int* n_steps));
/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Executes a complete trace]

  Description [Tries to execute a complete on FSM provided at
  construction time and returns true iff the trace is compatible with
  the fsm given at construction time. The number of performed steps is
  stored in *n_steps if a non-NULL pointer is given. This is -1 is if
  the Trace has no feasible initial state.]

  SideEffects [none]

  SeeAlso     [PartialTraceExecutor_execute]

******************************************************************************/
boolean CompleteTraceExecutor_execute (const CompleteTraceExecutor_ptr self,
                                       const Trace_ptr trace, int* n_steps)
{
  return (*self->execute)(self, trace, n_steps);
}


/**Function********************************************************************

  Synopsis           [The CompleteTraceExecutor class constructor]

  Description        [The CompleteTraceExecutor class constructor]

  SideEffects        []

  SeeAlso            [CompleteTraceExecutor_destroy]

******************************************************************************/
CompleteTraceExecutor_ptr CompleteTraceExecutor_create()
{
  CompleteTraceExecutor_ptr self = ALLOC(CompleteTraceExecutor, 1);
  COMPLETE_TRACE_EXECUTOR_CHECK_INSTANCE(self);

  complete_trace_executor_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [The CompleteTraceExecutor class destructor]

  Description        [The CompleteTraceExecutor class destructor]

  SideEffects        []

  SeeAlso            [CompleteTraceExecutor_create]

******************************************************************************/
void CompleteTraceExecutor_destroy(CompleteTraceExecutor_ptr self)
{
  COMPLETE_TRACE_EXECUTOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The CompleteTraceExecutor class private initializer]

  Description        [The CompleteTraceExecutor class private initializer]

  SideEffects        []

  SeeAlso            [CompleteTraceExecutor_create]

******************************************************************************/
void complete_trace_executor_init(CompleteTraceExecutor_ptr self)
{
  /* base class initialization */
  trace_executor_init(BASE_TRACE_EXECUTOR(self));

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = complete_trace_executor_finalize;

  /* virtual abstract complete execution method */
  OVERRIDE(CompleteTraceExecutor, execute) = complete_trace_executor_execute;
}


/**Function********************************************************************

  Synopsis           [The CompleteTraceExecutor class private deinitializer]

  Description        [The CompleteTraceExecutor class private deinitializer]

  SideEffects        []

  SeeAlso            [CompleteTraceExecutor_destroy]

******************************************************************************/
void complete_trace_executor_deinit(CompleteTraceExecutor_ptr self)
{
  /* members deinitialization */


  /* base class deinitialization */
  trace_executor_deinit(BASE_TRACE_EXECUTOR(self));
}


/**Function********************************************************************

  Synopsis    [Private service for loopback checking]

  Description [Returns true iff all loopback information for
  trace applies to the trace itself]

  SideEffects [none]

  SeeAlso     [partial_trace_executor_check_loopbacks]

******************************************************************************/
boolean
complete_trace_executor_check_loopbacks (const CompleteTraceExecutor_ptr self,
                                         const Trace_ptr trace)
{
  boolean res = true; /* no error */
  TraceIter step;

  int i = 1;

  /* local reference to superclass */
  BaseTraceExecutor_ptr executor = BASE_TRACE_EXECUTOR(self);

  COMPLETE_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  TRACE_CHECK_INSTANCE(trace);

  /* as a last check it is necessary to make sure loopback information
     is consistent (if trace is thawed this check is trivially
     true) */
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(BaseTraceExecutor_get_error_stream(executor),
            "now checking loopbacks...\n");
  }
  TRACE_FOREACH(trace, step) {
    if (trace_step_is_loopback(trace, step) &&
        !trace_step_test_loopback(trace, step)) {
      fprintf(BaseTraceExecutor_get_error_stream(executor), "*** Error ***\n"
              "Inconsistent loopback information found at step %d.\n", i);
      res = false;
      break; /*  continuing is pointless */
    }

    ++ i;
  } /* trace foreach */

  return res;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The CompleteTraceExecutor class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void complete_trace_executor_finalize(Object_ptr object, void* dummy)
{
  CompleteTraceExecutor_ptr self = COMPLETE_TRACE_EXECUTOR(object);

  complete_trace_executor_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Abstract execution method for complete trace executors]

  Description [This is a pure virtual functions. Every derived class
  must overwrite this function. Raises an assertion if invoked]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static boolean
complete_trace_executor_execute(const CompleteTraceExecutor_ptr self,
                                const Trace_ptr trace, int* n_steps)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
  return 0;
}


/**AutomaticEnd***************************************************************/

