/**CFile***********************************************************************

  FileName    [BaseTraceExecutor.c]

  PackageName [trace.exec]

  Synopsis    [Implementation of class 'BaseTraceExecutor']

  Description []

  SeeAlso     [BaseTraceExecutor.h]

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
#include <stdio.h>

#include "BaseTraceExecutor.h"
#include "BaseTraceExecutor_private.h"

#include "utils/utils.h"
#include "pkg_traceInt.h"
#include "trace/Trace.h"

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
/* See 'BaseTraceExecutor_private.h' for class 'BaseTraceExecutor' definition. */

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

static void trace_executor_finalize ARGS((Object_ptr object, void* dummy));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Getter for the verbosity field]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int
BaseTraceExecutor_get_verbosity (BaseTraceExecutor_ptr self)
{
  BASE_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  return self->verbosity;
}


/**Function********************************************************************

  Synopsis    [Setter for the verbosity field]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void
BaseTraceExecutor_set_verbosity (BaseTraceExecutor_ptr self, int verbosity)
{
  BASE_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  self->verbosity = verbosity;
}


/**Function********************************************************************

  Synopsis    [Getter for the output_stream field]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
FILE*
BaseTraceExecutor_get_output_stream (BaseTraceExecutor_ptr self)
{
  BASE_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  return (NIL(FILE) != self->output_stream ? self->output_stream : nusmv_stdout);
}


/**Function********************************************************************

  Synopsis    [Setter for the output_stream field]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void
BaseTraceExecutor_set_output_stream (BaseTraceExecutor_ptr self, FILE* output_stream)
{
  BASE_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  self->output_stream = output_stream;
}


/**Function********************************************************************

  Synopsis    [Getter for the error_stream field]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
FILE*
BaseTraceExecutor_get_error_stream (BaseTraceExecutor_ptr self)
{
  BASE_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  return (NIL(FILE) != self->error_stream ? self->error_stream : nusmv_stderr);
}


/**Function********************************************************************

  Synopsis    [Setter for the error_stream field]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void
BaseTraceExecutor_set_error_stream (BaseTraceExecutor_ptr self, FILE* error_stream)
{
  BASE_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  self->error_stream = error_stream;
}

/**Function********************************************************************

  Synopsis    [Virtual destructor for BaseTraceExecutor class]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
VIRTUAL void
BaseTraceExecutor_destroy (BaseTraceExecutor_ptr self)
{
  BASE_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  Object_ptr object =  OBJECT(self);

  object->finalize(object, NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Private service for defines checking]

  Description [Returns true iff values registered in the trace for
  defines actually match evaluated values. If either a value for a
  define is not present in the trace or could not be evaluated (due to
  missing dependencies) it is silently ignored.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
boolean trace_executor_check_defines(const BaseTraceExecutor_ptr self,
                                     const Trace_ptr trace)
{
  boolean res = true; /* no mismatch */

  /* final check, analyze defines (mismatches are warnings) */
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(BaseTraceExecutor_get_error_stream(self),
            "checking defines...\n");
  }

  { /* check defines */
    TraceIter step; int i = 1;

    TRACE_FOREACH(trace, step) {
      NodeList_ptr failures = NodeList_create();

      if (!trace_step_check_defines(trace, step, failures)) {
        ListIter_ptr iter;

        fprintf(BaseTraceExecutor_get_error_stream(self), "*** Warning ***\n"
                "Inconsistencies detected while analyzing the trace "
                "(step %d)\n\n", i);

        res = false;

        NODE_LIST_FOREACH(failures, iter) {
          node_ptr failure = NodeList_get_elem_at(failures, iter);
          fprintf(BaseTraceExecutor_get_error_stream(self), "%s",
                  failure_get_msg(failure));
          fprintf(BaseTraceExecutor_get_error_stream(self), "\n");
        } /* NODE_LIST_FOREACH */
      }

      NodeList_destroy(failures);

      ++ i;
    } /* TRACE_FOREACH */
  } /* check defines */

  return res;
}

/**Function********************************************************************

  Synopsis           [The BaseTraceExecutor class private initializer]

  Description        [The BaseTraceExecutor class private initializer]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void trace_executor_init(BaseTraceExecutor_ptr self)
{
  /* base class initialization */
  object_init(OBJECT(self));

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = trace_executor_finalize;

  self->verbosity = 0; /* default is quiet */

  self->output_stream = NIL(FILE); /* defaults to nusmv_stdout */
  self->error_stream = NIL(FILE); /* defaults to nusmv_stderr */
}


/**Function********************************************************************

  Synopsis           [The BaseTraceExecutor class private deinitializer]

  Description        [The BaseTraceExecutor class private deinitializer]

  SideEffects        []

  SeeAlso            [BaseTraceExecutor_destroy]

******************************************************************************/
void trace_executor_deinit(BaseTraceExecutor_ptr self)
{
  /* base class deinitialization */
  object_deinit(OBJECT(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The BaseTraceExecutor class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_executor_finalize(Object_ptr object, void* dummy)
{
  BaseTraceExecutor_ptr self = BASE_TRACE_EXECUTOR(object);

  trace_executor_deinit(self);
  FREE(self);
}

/**AutomaticEnd***************************************************************/

