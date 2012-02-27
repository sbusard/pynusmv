/**CFile***********************************************************************

  FileName    [BDDPartialTraceExecutor.c]

  PackageName [trace.exec]

  Synopsis    [Implementation of class 'BDDPartialTraceExecutor']

  Description []

  SeeAlso     [BDDPartialTraceExecutor.h]

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
#include "PartialTraceExecutor.h"

#include "BDDPartialTraceExecutor.h"
#include "BDDPartialTraceExecutor_private.h"

#include "utils/utils.h"
#include "opt/opt.h"

#include "trace/Trace.h"
#include "trace/Trace_private.h"
#include "trace/pkg_trace.h"

#include "mc/mc.h"

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
/* See 'BDDPartialTraceExecutor_private.h' for class 'BDDPartialTraceExecutor' definition. */

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

static void bdd_partial_trace_executor_finalize ARGS((Object_ptr object,
                                                      void* dummy));

static Trace_ptr
bdd_partial_trace_executor_execute ARGS((const PartialTraceExecutor_ptr self,
                                         const Trace_ptr trace,
                                         const NodeList_ptr language,
                                         int* n_steps));

static Trace_ptr
bdd_partial_trace_executor_generate ARGS((const BDDPartialTraceExecutor_ptr self,
                                          const BddStates goal_states,
                                          node_ptr fwd_image, int length,
                                          const NodeList_ptr language,
                                          const char* trace_name));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BDDPartialTraceExecutor class constructor]

  Description        [The BDDPartialTraceExecutor class constructor]

  SideEffects        []

  SeeAlso            [BDDPartialTraceExecutor_destroy]

******************************************************************************/
BDDPartialTraceExecutor_ptr BDDPartialTraceExecutor_create(const BddFsm_ptr fsm,
                                                           const BddEnc_ptr enc)
{
  BDDPartialTraceExecutor_ptr self = ALLOC(BDDPartialTraceExecutor, 1);
  BDD_PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self);

  bdd_partial_trace_executor_init(self, fsm, enc);
  return self;
}


/**Function********************************************************************

  Synopsis           [The BDDPartialTraceExecutor class destructor]

  Description        [The BDDPartialTraceExecutor class destructor]

  SideEffects        []

  SeeAlso            [BDDPartialTraceExecutor_create]

******************************************************************************/
void BDDPartialTraceExecutor_destroy(BDDPartialTraceExecutor_ptr self)
{
  BDD_PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BDDPartialTraceExecutor class private initializer]

  Description        [The BDDPartialTraceExecutor class private initializer]

  SideEffects        []

  SeeAlso            [BDDPartialTraceExecutor_create]

******************************************************************************/
void bdd_partial_trace_executor_init(BDDPartialTraceExecutor_ptr self,
                                     const BddFsm_ptr fsm,
                                     const BddEnc_ptr enc)
{
  /* base class initialization */
  partial_trace_executor_init(PARTIAL_TRACE_EXECUTOR(self));

  /* members initialization */
  self->fsm = fsm;
  self->enc = enc;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = bdd_partial_trace_executor_finalize;

  /* partial traces virtual execution method */
  OVERRIDE(PartialTraceExecutor, execute) = bdd_partial_trace_executor_execute;
}


/**Function********************************************************************

  Synopsis           [The BDDPartialTraceExecutor class private deinitializer]

  Description        [The BDDPartialTraceExecutor class private deinitializer]

  SideEffects        []

  SeeAlso            [BDDPartialTraceExecutor_destroy]

******************************************************************************/
void bdd_partial_trace_executor_deinit(BDDPartialTraceExecutor_ptr self)
{
  /* members deinitialization */


  /* base class deinitialization */
  partial_trace_executor_deinit(PARTIAL_TRACE_EXECUTOR(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The BDDPartialTraceExecutor class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void bdd_partial_trace_executor_finalize(Object_ptr object, void* dummy)
{
  BDDPartialTraceExecutor_ptr self = BDD_PARTIAL_TRACE_EXECUTOR(object);

  bdd_partial_trace_executor_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis [Executes a trace on the fsm given at construction time
  using BDDs]

  Description [The trace is executed using BDDs, that is a proof that
  the fsm is compatible with the trace is built (if such proof
  exists). Incomplete traces are filled-in with compatible values for
  state and input variables.

  Given trace can be either complete or incomplete.

  The number of performed steps (transitions) is returned in *n_steps,
  if a non-NULL pointer is given. If the initial state is not
  compatible -1 is written.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static Trace_ptr
bdd_partial_trace_executor_execute
(const PartialTraceExecutor_ptr partial_executor, const Trace_ptr trace,
 const NodeList_ptr language, int* n_steps)
{
  /* local references to self */
  const BDDPartialTraceExecutor_ptr self = \
    BDD_PARTIAL_TRACE_EXECUTOR(partial_executor);

  const BaseTraceExecutor_ptr executor = \
    BASE_TRACE_EXECUTOR(partial_executor);

  Trace_ptr res = TRACE(NULL); /* failure */

  int count = -1;
  boolean success = true;

  DdManager* dd;
  BddStates trace_states;
  TraceIter step = TRACE_END_ITER;

  BddStates fwd_image = (BddStates) NULL;
  node_ptr path = Nil; /* forward constrained images will be used
                          later to compute the complete trace */

  const char* trace_description = "BDD Execution";

  /* 0- Check prerequisites */
  BDD_PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  TRACE_CHECK_INSTANCE(trace);
  BDD_FSM_CHECK_INSTANCE(self->fsm);
  BDD_ENC_CHECK_INSTANCE(self->enc);

  dd = BddEnc_get_dd_manager(self->enc);

  step = trace_first_iter(trace);
  nusmv_assert(TRACE_END_ITER != step);

  trace_states = TraceUtils_fetch_as_bdd(trace, step,
                                         TRACE_ITER_SF_SYMBOLS, self->enc);

  /* 1- Check Start State */
  {
    bdd_ptr init_bdd = BddFsm_get_init(self->fsm);
    bdd_ptr invar_bdd = BddFsm_get_state_constraints(self->fsm);
    BddStates source_states; /* last known states */

    source_states = bdd_and(dd, init_bdd, invar_bdd);
    bdd_and_accumulate(dd, &source_states, trace_states);

    bdd_free(dd, invar_bdd);
    bdd_free(dd, init_bdd);
    bdd_free(dd, trace_states);

    if (!bdd_is_false(dd, source_states)) {

      boolean terminate = false;
      path = cons(NODE_PTR(bdd_dup(source_states)), Nil);

      ++ count;

      /* 2- Check Consecutive States are related by transition relation */
      do {
        BddStates last_state; /* (unshifted) next state */
        BddStates next_state; /* next state constraints */

        BddInputs next_input; /* next input constraints */
        BddStatesInputsNexts next_combo; /* state-input-next constraints */

        BddStatesInputsNexts constraints;

        step = TraceIter_get_next(step);
        if (TRACE_END_ITER != step) {

          next_input = \
            TraceUtils_fetch_as_bdd(trace, step, TRACE_ITER_I_SYMBOLS,
                                    self->enc);
          next_combo = \
            TraceUtils_fetch_as_bdd(trace, step, TRACE_ITER_COMBINATORIAL,
                                    self->enc);
          last_state = \
            TraceUtils_fetch_as_bdd(trace, step, TRACE_ITER_SF_SYMBOLS,
                                    self->enc);

          next_state = BddEnc_state_var_to_next_state_var(self->enc, last_state);

          if (0 < BaseTraceExecutor_get_verbosity(executor)) {
            fprintf(BaseTraceExecutor_get_output_stream(executor),
                    "-- executing step %d ... ", 1 + count);
            fflush(BaseTraceExecutor_get_output_stream(executor));
          }

          /* building constrained fwd image */
          constraints = bdd_and(dd, next_input, next_combo);
          bdd_and_accumulate(dd, &constraints, next_state);

          fwd_image =
            BddFsm_get_sins_constrained_forward_image(self->fsm, source_states,
                                                      constraints);

           /* test whether the constrained fwd image is not empty */
          if (!bdd_is_false(dd, fwd_image)) {
            if (0 < BaseTraceExecutor_get_verbosity(executor)) {
              fprintf(BaseTraceExecutor_get_output_stream(executor), "done\n");
            }
            path = cons(NODE_PTR(fwd_image), path);
            ++ count;
          }
          else {
            if (0 < BaseTraceExecutor_get_verbosity(executor)) {
              fprintf(BaseTraceExecutor_get_output_stream(executor),
                      "failed!\n");
            }
            terminate = true;
            success = false;
          }

          /* no longer used bdd refs */
          bdd_free(dd, next_input);
          bdd_free(dd, next_combo);
          bdd_free(dd, last_state);
          bdd_free(dd, next_state);

          bdd_free(dd, source_states);
          source_states = bdd_dup(fwd_image);
        }

        else {
          if (0 == count) {
            fprintf(BaseTraceExecutor_get_error_stream(executor),
                    "Warning: trace has no transitions.\n");
          }
          terminate = true;
        }
      } while (!terminate); /* loop on state/input pairs */
    } /* if has initial state */

    else {
      fprintf(BaseTraceExecutor_get_error_stream(executor),
              "Error: starting state is not initial state.\n");
      success = false;
    }

    /* 3- If last state could be reached a complete trace exists */
    if (success) {
      if (0 < count) {
        res = \
          bdd_partial_trace_executor_generate(self, fwd_image, path,
                                              count, language,
                                              trace_description);
      }

      else { /* generates a complete state of trace of length 0 */
        res = \
          bdd_partial_trace_executor_generate(self, source_states,
                                              Nil, 0, language,
                                              trace_description);
      }

      nusmv_assert(TRACE(NULL) != res);

      /* cleanup */
      walk_dd(dd, bdd_free, path);
      free_list(path);
    }

    bdd_free(dd, source_states);
  }

  /* as a final stage, verify loopbacks consistency using internal
     service. The incomplete trace is compatible (that is, a valid
     completion exists) iff exactly len(Trace) steps have been
     performed *and* loopback data for the incomplete trace applies to
     the complete one as well. */
  if (TRACE(NULL) != res) {
    if (Trace_get_length(trace) == count &&
        partial_trace_executor_check_loopbacks(partial_executor, trace, res)) {

      fprintf(BaseTraceExecutor_get_output_stream(executor),
              "-- Trace was successfully completed.\n");
    }

    else {
      Trace_destroy(res);
      res = TRACE(NULL);
    }
  }

  if (TRACE(NULL) == res) {
    fprintf(BaseTraceExecutor_get_output_stream(executor),
            "-- Trace could not be completed.\n");
  }

  if (NIL(int) != n_steps) { *n_steps = count; }
  return res;
}


/**Function********************************************************************


  Synopsis    [Generates a complete trace]

  Description [This function is a private service of
  bdd_partial_trace_executor_execute]

  SideEffects [None]

******************************************************************************/
static Trace_ptr
bdd_partial_trace_executor_generate(const BDDPartialTraceExecutor_ptr self,
                                    const BddStates goal_states,
                                    node_ptr reachable, int length,
                                    const NodeList_ptr language,
                                    const char* trace_name)
{
  Trace_ptr res;
  DdManager* dd;
  node_ptr path;
  bdd_ptr target;

  NODE_LIST_CHECK_INSTANCE(language);

  dd = BddEnc_get_dd_manager(self->enc);

  target = BddEnc_pick_one_state_rand(self->enc, goal_states);
  path = cons((node_ptr) target, Nil);

  if (Nil != reachable) {
    reachable = cdr(reachable);
    while (0 != length) {
      bdd_ptr source;
      bdd_ptr input;
      bdd_ptr inputs;
      bdd_ptr bwd_image;
      bdd_ptr intersect;

      /* pick source state */
      bwd_image = BddFsm_get_backward_image(self->fsm, target);
      intersect = bdd_and(dd, bwd_image, (bdd_ptr) car(reachable));
      nusmv_assert(!bdd_is_false(dd, intersect));
      source = BddEnc_pick_one_state(self->enc, intersect);
      bdd_free(dd, intersect);
      bdd_free(dd, bwd_image);

      /* pick one input s.t. source -> (input) -> target */
      inputs = BddFsm_states_to_states_get_inputs(self->fsm, source, target);
      input = BddEnc_pick_one_input(self->enc, inputs);
      nusmv_assert(!bdd_is_false(dd, input));
      bdd_free(dd, inputs);

      /* prepend input and source state */
      path = cons((node_ptr) input, path);
      path = cons((node_ptr) source, path);

      -- length;
      target = source;
      reachable = cdr(reachable);
    }
  }
  /* make sure the trace length is correct */
  nusmv_assert(0 == length &&  Nil == reachable);

  res = \
    Mc_create_trace_from_bdd_state_input_list(self->enc, language, trace_name,
                                              TRACE_TYPE_EXECUTION, path);
  /* cleanup */
  walk_dd(dd, bdd_free, path);
  free_list(path);

  return res;
}

/**AutomaticEnd***************************************************************/

