/**CFile***********************************************************************

  FileName    [BDDCompleteTraceExecutor.c]

  PackageName [trace.exec]

  Synopsis    [Implementation of class 'BDDCompleteTraceExecutor']

  Description []

  SeeAlso     [BDDCompleteTraceExecutor.h]

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

#include "BDDCompleteTraceExecutor.h"
#include "BDDCompleteTraceExecutor_private.h"

#include "trace/pkg_trace.h"
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
/* See 'BDDCompleteTraceExecutor_private.h' for class 'BDDCompleteTraceExecutor' definition. */

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

static void bdd_complete_trace_executor_finalize ARGS((Object_ptr object,
                                                       void* dummy));

static boolean
bdd_complete_trace_executor_execute ARGS((const CompleteTraceExecutor_ptr self,
                                          const Trace_ptr trace, int* n_steps));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BDDCompleteTraceExecutor class constructor]

  Description        [The BDDCompleteTraceExecutor class constructor]

  SideEffects        []

  SeeAlso            [BDDCompleteTraceExecutor_destroy]

******************************************************************************/
BDDCompleteTraceExecutor_ptr
BDDCompleteTraceExecutor_create(const BddFsm_ptr fsm, const BddEnc_ptr enc)
{
  BDDCompleteTraceExecutor_ptr self = ALLOC(BDDCompleteTraceExecutor, 1);
  BDD_COMPLETE_TRACE_EXECUTOR_CHECK_INSTANCE(self);

  bdd_complete_trace_executor_init(self, fsm, enc);
  return self;
}


/**Function********************************************************************

  Synopsis           [The BDDCompleteTraceExecutor class destructor]

  Description        [The BDDCompleteTraceExecutor class destructor]

  SideEffects        []

  SeeAlso            [BDDCompleteTraceExecutor_create]

******************************************************************************/
void BDDCompleteTraceExecutor_destroy(BDDCompleteTraceExecutor_ptr self)
{
  BDD_COMPLETE_TRACE_EXECUTOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BDDCompleteTraceExecutor class private initializer]

  Description        [The BDDCompleteTraceExecutor class private initializer]

  SideEffects        []

  SeeAlso            [BDDCompleteTraceExecutor_create]

******************************************************************************/
void bdd_complete_trace_executor_init(BDDCompleteTraceExecutor_ptr self,
                                      const BddFsm_ptr fsm, const BddEnc_ptr enc)
{
  /* base class initialization */
  complete_trace_executor_init(COMPLETE_TRACE_EXECUTOR(self));

  /* members initialization */
  self->fsm = fsm;
  self->enc = enc;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = bdd_complete_trace_executor_finalize;

  /* for example, to override a base class' virtual method: */
  OVERRIDE(CompleteTraceExecutor, execute) = bdd_complete_trace_executor_execute;
}


/**Function********************************************************************

  Synopsis           [The BDDCompleteTraceExecutor class private deinitializer]

  Description        [The BDDCompleteTraceExecutor class private deinitializer]

  SideEffects        []

  SeeAlso            [BDDCompleteTraceExecutor_destroy]

******************************************************************************/
void bdd_complete_trace_executor_deinit(BDDCompleteTraceExecutor_ptr self)
{
  /* members deinitialization */

  /* base class deinitialization */
  complete_trace_executor_deinit(COMPLETE_TRACE_EXECUTOR(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The BDDCompleteTraceExecutor class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void bdd_complete_trace_executor_finalize(Object_ptr object, void* dummy)
{
  BDDCompleteTraceExecutor_ptr self = BDD_COMPLETE_TRACE_EXECUTOR(object);

  bdd_complete_trace_executor_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Executes a trace on the given fsm using BDDs]

  Description [The trace is executed using BDDs. Every transition of
  the trace is tested for compatibility with the model. Trace is
  assumed to be complete and non-empty (i.e. at least one state
  exists). The number of successfullys executed transitions is written
  in *n_steps if a non-NULL pointer is given. If the initial state of
  the trace is not compatible -1 is written.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static boolean
bdd_complete_trace_executor_execute(const CompleteTraceExecutor_ptr complete_executor,
                                    const Trace_ptr trace, int* n_steps)
{
  /* local reference to self */
  const BaseTraceExecutor_ptr executor = \
    BASE_TRACE_EXECUTOR(complete_executor);

  const BDDCompleteTraceExecutor_ptr self = \
    BDD_COMPLETE_TRACE_EXECUTOR(executor);

  DdManager* dd;
  BddStates trace_state;
  TraceIter step;
  int count = -1; /* failure */

  /* 0- Check prerequisites */
  COMPLETE_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  TRACE_CHECK_INSTANCE(trace);

  BDD_FSM_CHECK_INSTANCE(self->fsm);
  BDD_ENC_CHECK_INSTANCE(self->enc);

  /* check for trace completeness */
  if (!Trace_is_complete(trace, Trace_get_sf_vars(trace), false) ||
      !Trace_is_complete(trace, Trace_get_i_vars(trace), false)) {
    internal_error("%s:%d:%s: This executor does not support partial traces.",
                   __FILE__, __LINE__, __func__);
  }

  dd = BddEnc_get_dd_manager(self->enc);

  step = Trace_first_iter(trace);
  nusmv_assert(TRACE_END_ITER != step);

  /* the set of initial states for the trace consists of just one
     state under the assumption that the trace is complete */
  trace_state = TraceUtils_fetch_as_bdd(trace, step,
                                        TRACE_ITER_SF_SYMBOLS, self->enc);

  /* 1- Check Start State */
  {
    bdd_ptr init_bdd = BddFsm_get_init(self->fsm);
    bdd_ptr invar_bdd = BddFsm_get_state_constraints(self->fsm);
    BddStates initial_states; /* the initial set of states for the model */
    BddStates from_state; /* last known state (just one, see above) */

    initial_states = bdd_and(dd, init_bdd, invar_bdd);
    bdd_free(dd, init_bdd);
    bdd_free(dd, invar_bdd);

    if (bdd_entailed(dd, trace_state, initial_states)) {
      boolean terminate = false;
      from_state = trace_state;
      ++ count;

      /* 2- Check Consecutive States are related by transition relation */
      do {
        BddStates forward_states;
        BddStates next_state;  /* (un-shifted) next state */

        BddInputs next_input; /* next input constraints */
        BddStatesInputsNexts next_combo; /* next combinatorials */

        step = TraceIter_get_next(step);
        if (TRACE_END_ITER != step) {

          /* fetch next bdds from trace */
          next_input = \
            TraceUtils_fetch_as_bdd(trace, step,
                                    TRACE_ITER_I_SYMBOLS, self->enc);

          next_combo =  \
            TraceUtils_fetch_as_bdd(trace, step, TRACE_ITER_COMBINATORIAL,
                                    self->enc);
          next_state = \
            TraceUtils_fetch_as_bdd(trace, step, TRACE_ITER_SF_SYMBOLS,
                                    self->enc);

          if (0 < BaseTraceExecutor_get_verbosity(executor)) {
            fprintf(BaseTraceExecutor_get_output_stream(executor),
                    "-- executing step %d ... ", 1 + count);
            fflush(BaseTraceExecutor_get_output_stream(executor));
          }

          {
            BddStatesInputsNexts constraints = bdd_dup(next_input);
            bdd_and_accumulate(dd, &constraints, next_combo);

            forward_states =
              BddFsm_get_sins_constrained_forward_image(self->fsm, from_state,
                                                        constraints);
            bdd_free(dd, constraints);
          }

          /* test whether the constrained image entails the next states */
          if (bdd_entailed(dd, next_state, forward_states)) {
            if (0 < BaseTraceExecutor_get_verbosity(executor)) {
              fprintf(BaseTraceExecutor_get_output_stream(executor), "done\n");
            }
            ++ count;
          }

          else {
            if (0 < BaseTraceExecutor_get_verbosity(executor)) {
              fprintf(BaseTraceExecutor_get_output_stream(executor),
                      "failed!\n");
            }
            terminate = true;
          }

          /* no longer used bdd refs */
          bdd_free(dd, forward_states);
          bdd_free(dd, next_combo);
          bdd_free(dd, next_input);

          bdd_free(dd, from_state);
          from_state = bdd_dup(next_state);
          bdd_free(dd, next_state);
        }

        else {
          if (0 == count) {
            fprintf(BaseTraceExecutor_get_error_stream(executor),
                    "Warning: trace has no transitions.\n");
          }
          terminate = true;
        }
      } while (!terminate); /* loop on state/input pairs */

      bdd_free(dd, from_state);
    }

    else {
      fprintf(BaseTraceExecutor_get_error_stream(executor),
              "Error: starting state is not initial state.\n");
    }

    bdd_free(dd, initial_states);
  }

  { /* as a last check, verify looback consistency using internal
       service, the trace is compatible iff exactly len(Trace) steps
       have been performed *and* loopback data is consistent. */
    boolean res = false;

    if (Trace_get_length(trace) == count) {
      if (complete_trace_executor_check_loopbacks(complete_executor, trace)) {
        fprintf(BaseTraceExecutor_get_output_stream(executor),
                "Trace %d execution completed successfully.\n"
                "%d steps performed.\n",
                Trace_get_id(trace),
                count);

        res = true;
      }
    }
    else {
      fprintf(BaseTraceExecutor_get_output_stream(executor),
              "Trace execution failed!\n"
              /* "(%d steps performed).\n", count */);
    }

    if (NIL(int) != n_steps) { *n_steps = count; }
    return res;
  }
}

/**AutomaticEnd***************************************************************/

