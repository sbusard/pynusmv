/**CFile***********************************************************************

  FileName    [SATPartialTraceExecutor.c]

  PackageName [trace.exec]

  Synopsis    [Implementation of class 'SATPartialTraceExecutor']

  Description []

  SeeAlso     [SATPartialTraceExecutor.h]

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

#include "SATPartialTraceExecutor.h"
#include "SATPartialTraceExecutor_private.h"

#include "utils/utils.h"
#include "opt/opt.h"

#include "bmc/bmc.h"

#include "sat/SatIncSolver.h"
#include "trace/pkg_trace.h"
#include "trace/pkg_traceInt.h"

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

static void sat_partial_trace_executor_finalize ARGS((Object_ptr object,
                                                      void* dummy));

static Trace_ptr
sat_partial_trace_executor_execute ARGS((const PartialTraceExecutor_ptr self,
                                         const Trace_ptr trace,
                                         const NodeList_ptr language,
                                         int* n_steps));

static Trace_ptr
sat_partial_trace_executor_execute_restart
ARGS((const SATPartialTraceExecutor_ptr self, const Trace_ptr trace,
      const NodeList_ptr language, int* n_steps));

static Trace_ptr
sat_partial_trace_executor_execute_no_restart
ARGS((const SATPartialTraceExecutor_ptr self, const Trace_ptr trace,
      const NodeList_ptr language, int* n_steps));

static inline be_ptr
sat_partial_trace_executor_get_initial_state ARGS((BeFsm_ptr be_fsm));

static inline be_ptr
sat_partial_trace_executor_get_transition_relation ARGS((BeFsm_ptr be_fsm));

static inline void
bmc_add_be_into_solver_positively ARGS((SatSolver_ptr solver,
                                        SatSolverGroup group, be_ptr prob,
                                        BeEnc_ptr be_enc));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The SATPartialTraceExecutor class constructor]

  Description        [The SATPartialTraceExecutor class constructor]

  SideEffects        []

  SeeAlso            [SATPartialTraceExecutor_destroy]

******************************************************************************/
SATPartialTraceExecutor_ptr
SATPartialTraceExecutor_create(const BeFsm_ptr fsm,
                               const BeEnc_ptr enc,
                               const BddEnc_ptr bdd_enc,
                               boolean use_restart)
{
  SATPartialTraceExecutor_ptr self = ALLOC(SATPartialTraceExecutor, 1);
  SAT_PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self);

  sat_partial_trace_executor_init(self, fsm, enc, bdd_enc, use_restart);
  return self;
}


/**Function********************************************************************

  Synopsis           [The SATPartialTraceExecutor class destructor]

  Description        [The SATPartialTraceExecutor class destructor]

  SideEffects        []

  SeeAlso            [SATPartialTraceExecutor_create]

******************************************************************************/
void SATPartialTraceExecutor_destroy(SATPartialTraceExecutor_ptr self)
{
  SAT_PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The SATPartialTraceExecutor class private initializer]

  Description        [The SATPartialTraceExecutor class private initializer]

  SideEffects        []

  SeeAlso            [SATPartialTraceExecutor_create]

******************************************************************************/
void sat_partial_trace_executor_init(SATPartialTraceExecutor_ptr self,
                                     const BeFsm_ptr fsm, const BeEnc_ptr enc,
                                     const BddEnc_ptr bdd_enc,
                                     boolean use_restart)
{
  /* base class initialization */
  partial_trace_executor_init(PARTIAL_TRACE_EXECUTOR(self));

  /* members initialization */
  self->fsm = fsm;
  self->enc = enc;
  self->bdd_enc = bdd_enc;
  self->use_restart = use_restart;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = sat_partial_trace_executor_finalize;

  OVERRIDE(PartialTraceExecutor, execute) = sat_partial_trace_executor_execute;
}


/**Function********************************************************************

  Synopsis           [The SATPartialTraceExecutor class private deinitializer]

  Description        [The SATPartialTraceExecutor class private deinitializer]

  SideEffects        []

  SeeAlso            [SATPartialTraceExecutor_destroy]

******************************************************************************/
void sat_partial_trace_executor_deinit(SATPartialTraceExecutor_ptr self)
{
  /* members deinitialization */

  /* base class deinitialization */
  partial_trace_executor_deinit(PARTIAL_TRACE_EXECUTOR(self));
}



/**Function********************************************************************

  Synopsis    [Partial trace execution method]

  Description [Performs partial trace re-execution. Algorithm is
  selected depending on the value of restart flag given at
  construction time]

  SideEffects [a complete trace is built and returned in case of success]

  SeeAlso     [sat_partial_trace_executor_execute_restart,
  sat_partial_trace_executor_execute_no_restart]

******************************************************************************/
static Trace_ptr
sat_partial_trace_executor_execute(const PartialTraceExecutor_ptr partial_executor,
                                   const Trace_ptr trace,
                                   const NodeList_ptr language,
                                   int* n_steps)
{
  const SATPartialTraceExecutor_ptr self = \
    SAT_PARTIAL_TRACE_EXECUTOR(partial_executor);

  /* pick execution algorithm according to internal "use_restart"
   flag, set at construction time */
  return self->use_restart
    ? sat_partial_trace_executor_execute_restart(self, trace, language, n_steps)
    : sat_partial_trace_executor_execute_no_restart(self, trace,
                                                    language, n_steps);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The SATPartialTraceExecutor class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void sat_partial_trace_executor_finalize(Object_ptr object, void* dummy)
{
  SATPartialTraceExecutor_ptr self = SAT_PARTIAL_TRACE_EXECUTOR(object);

  sat_partial_trace_executor_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Executes a trace on the given fsm using SAT solver]

  Description [Executes a trace on the fsm given at construction time
  using SAT solver, that is a proof that the fsm is compatible with
  the trace is built (if such proof exists). Incomplete traces are
  filled-in with compatible values for state and input variables.

  Given trace can be either complete or incomplete.

  The number of performed steps (transitions) is returned in *n_steps,
  if it is a non-NULL pointer. If the initial state is not compatible
  -1 is written.]

  SideEffects [a complete trace is built and returned is case of success]

  SeeAlso     [sat_partial_trace_executor_restart]

******************************************************************************/
static Trace_ptr sat_partial_trace_executor_execute_no_restart
(const SATPartialTraceExecutor_ptr self, const Trace_ptr trace,
 const NodeList_ptr language, int* n_steps)
{
  /* local references to self */
  const BaseTraceExecutor_ptr executor = \
    BASE_TRACE_EXECUTOR(self);

  const PartialTraceExecutor_ptr partial_executor = \
    PARTIAL_TRACE_EXECUTOR(self);

  Trace_ptr res = TRACE(NULL);
  int count = -1; /* failure */

  SatIncSolver_ptr solver;
  SatSolverResult satResult;

  boolean success = true;

  be_ptr be_current;
  be_ptr be_trans;
  be_ptr be_problem;

  TraceIter step;

  /* 0- Check prerequisites */
  SAT_PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  TRACE_CHECK_INSTANCE(trace);

  BE_FSM_CHECK_INSTANCE(self->fsm);
  BE_ENC_CHECK_INSTANCE(self->enc);
  BDD_ENC_CHECK_INSTANCE(self->bdd_enc);

  solver  = Sat_CreateIncSolver(get_sat_solver(OptsHandler_get_instance()));
  SAT_INC_SOLVER_CHECK_INSTANCE(solver);

  step = Trace_first_iter(trace);
  nusmv_assert(TRACE_END_ITER != step);

  { /* 1- Check initial state */
    Be_Manager_ptr be_mgr = BeEnc_get_be_manager(self->enc);

    be_current = \
      TraceUtils_fetch_as_be(trace, step, TRACE_ITER_SF_SYMBOLS,
                             self->enc, self->bdd_enc);
    be_problem = Be_And(be_mgr,
                        BeEnc_untimed_expr_to_timed(self->enc, be_current, 0),
                        Bmc_Model_GetInit0(self->fsm));

    /* push the problem into the SAT solver (permanent push) */
    bmc_add_be_into_solver_positively(SAT_SOLVER(solver),
        SatSolver_get_permanent_group(SAT_SOLVER(solver)), be_problem,
                                      self->enc);

    satResult = SatSolver_solve_all_groups(SAT_SOLVER(solver));
  }

  if (SAT_SOLVER_UNSATISFIABLE_PROBLEM == satResult) {
    fprintf(nusmv_stderr,
            "Error: starting state is not initial state\n");
    success = false;
  }
  else {
    boolean terminate = false;
    nusmv_assert(SAT_SOLVER_SATISFIABLE_PROBLEM == satResult);

    ++ count;

    be_trans = sat_partial_trace_executor_get_transition_relation(self->fsm);

    /* 2- Check Consecutive States are related by transition relation */
    do {
      Be_Manager_ptr be_mgr = BeEnc_get_be_manager(self->enc);

      be_ptr be_input;
      be_ptr be_comb;
      be_ptr be_next;

      step = TraceIter_get_next(step);
      if (TRACE_END_ITER != step) {

        if (0 < BaseTraceExecutor_get_verbosity(executor)) {
          fprintf(BaseTraceExecutor_get_output_stream(executor),
                  "-- executing step %d ... ", 1+count);
          fflush(BaseTraceExecutor_get_output_stream(executor));
        }

        be_input = \
          TraceUtils_fetch_as_be(trace, step, TRACE_ITER_I_SYMBOLS,
                                 self->enc, self->bdd_enc);
        be_comb = \
          TraceUtils_fetch_as_be(trace, step, TRACE_ITER_COMBINATORIAL,
                                 self->enc, self->bdd_enc);
        be_next = \
          TraceUtils_fetch_as_be(trace, step, TRACE_ITER_SF_SYMBOLS,
                                 self->enc, self->bdd_enc);

        /* create problem, time it, and push into the SAT solver */
        be_problem = \
          BeEnc_untimed_expr_to_timed(self->enc,
                                      Be_And(be_mgr, be_trans,
                                             Be_And(be_mgr,
                                                    Be_And(be_mgr,
                                                           be_input,
                                                           be_comb),
                        BeEnc_shift_curr_to_next(self->enc, be_next))), count);

        /* permanent push */
        bmc_add_be_into_solver_positively(SAT_SOLVER(solver),
                        SatSolver_get_permanent_group(SAT_SOLVER(solver)),
                                          be_problem, self->enc);

        satResult = SatSolver_solve_all_groups(SAT_SOLVER(solver));

        if (SAT_SOLVER_UNSATISFIABLE_PROBLEM == satResult) {
          if (0 < BaseTraceExecutor_get_verbosity(executor)) {
            fprintf(BaseTraceExecutor_get_output_stream(executor), "failed!\n");
          }
          success = false;
          terminate = true;
        }
        else {
          if (0 < BaseTraceExecutor_get_verbosity(executor)) {
            fprintf(BaseTraceExecutor_get_output_stream(executor), "ok\n");
          }
          ++ count;

          nusmv_assert(SAT_SOLVER_SATISFIABLE_PROBLEM == satResult);
        }
      }
      else {
        if (0 == count) {
          fprintf(nusmv_stderr, "Warning: trace has no transitions.\n");
        }
        terminate = true;
      }
    } while (!terminate); /* loop on input/state pairs */
  }

  /* generate a new trace if execution was successful */
  if (success) {
    const char* trace_description = "BMC Execution";

    res = \
      Bmc_Utils_generate_cntexample(self->enc, SAT_SOLVER(solver),
                                    be_problem, count, trace_description,
                                    language);

    /* can be overwritten  by caller */
    Trace_set_type(res, TRACE_TYPE_EXECUTION);
  }

  /* release the SAT solver instance */
  SatIncSolver_destroy(solver);

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

  Synopsis    [Executes a trace on the given fsm using SAT solver]

  Description [The trace is executed using SAT, that is a proof that
  the fsm is compatible with the trace is built. Uncomplete traces are
  filled-in with compatible values for state and input
  variables. Restart from complete states is performed.

  If trace is compatible, a new complete trace is registered in the
  TraceManager and its index is written into trace_index;

  The number of performed steps (transitions) is returned. If the
  initial state is not compatible -1 is returned.]

  SideEffects [None]

  SeeAlso     [sat_partial_trace_executor_no_restart]

******************************************************************************/
static Trace_ptr sat_partial_trace_executor_execute_restart
(const SATPartialTraceExecutor_ptr self, const Trace_ptr trace,
 const NodeList_ptr language, int* n_steps)
{
  /* local references to self */
  const BaseTraceExecutor_ptr executor = \
    BASE_TRACE_EXECUTOR(self);

  const PartialTraceExecutor_ptr partial_executor = \
    PARTIAL_TRACE_EXECUTOR(self);

  Trace_ptr res = TRACE(NULL);

  int count = -1, time = -1; /* failure */
  boolean success = true;

  TraceIter step;

  be_ptr be_current;
  be_ptr be_trans;
  be_ptr be_problem;

  SatIncSolver_ptr solver;
  SatSolverGroup satGroup;
  SatSolverResult satResult;

  const char* trace_description = "BMC Execution";

  /* 0- Check prerequisites */
  SAT_PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self);
  TRACE_CHECK_INSTANCE(trace);

  BE_FSM_CHECK_INSTANCE(self->fsm);
  BE_ENC_CHECK_INSTANCE(self->enc);
  BDD_ENC_CHECK_INSTANCE(self->bdd_enc);
  NODE_LIST_CHECK_INSTANCE(language);

  solver  = Sat_CreateIncSolver(get_sat_solver(OptsHandler_get_instance()));
  SAT_INC_SOLVER_CHECK_INSTANCE(solver);

  step = Trace_first_iter(trace);
  nusmv_assert(TRACE_END_ITER != step);

  { /* 1- Check initial State */
    Be_Manager_ptr be_mgr = BeEnc_get_be_manager(self->enc);

    /* pick the initial state from the trace */
    be_current = \
      TraceUtils_fetch_as_be(trace, step, TRACE_ITER_SF_SYMBOLS,
                             self->enc, self->bdd_enc);

    be_problem = Be_And(be_mgr,
                        BeEnc_untimed_expr_to_timed(self->enc, be_current, 0),
                        Bmc_Model_GetInit0(self->fsm));

    /* push the problem into the SAT solver */
    satGroup = SatIncSolver_create_group(solver);
    bmc_add_be_into_solver_positively(SAT_SOLVER(solver), satGroup,
                                      be_problem, self->enc);

    satResult = SatSolver_solve_all_groups(SAT_SOLVER(solver));
  }

  if (SAT_SOLVER_UNSATISFIABLE_PROBLEM == satResult) {
    fprintf(nusmv_stderr, "Error: starting state is not initial state\n");
    success = false;
  }
  else {
    boolean terminate = false;
    nusmv_assert(SAT_SOLVER_SATISFIABLE_PROBLEM == satResult);

    ++ count;
    ++ time;

    be_trans = sat_partial_trace_executor_get_transition_relation(self->fsm);

    /* 2- Check Consecutive States are related by transition relation */
    do {
      Be_Manager_ptr be_mgr = BeEnc_get_be_manager(self->enc);

      step = TraceIter_get_next(step);
      if (TRACE_END_ITER != step) {

        if (0 < BaseTraceExecutor_get_verbosity(executor)) {
          fprintf(BaseTraceExecutor_get_output_stream(executor),
                  "-- executing step %d ... ", 1+count);
          fflush(BaseTraceExecutor_get_output_stream(executor));
        }

        /* create problem, time it, and push into the SAT solver next
           input, next transitional, next state and the transition
           relation (previous state has already been pushed) */
         be_ptr be_input = \
          TraceUtils_fetch_as_be(trace, step, TRACE_ITER_I_SYMBOLS,
                                 self->enc, self->bdd_enc);
        be_ptr be_comb = \
          TraceUtils_fetch_as_be(trace, step, TRACE_ITER_COMBINATORIAL,
                                 self->enc, self->bdd_enc);

        be_ptr be_next = \
          TraceUtils_fetch_as_be(trace, step, TRACE_ITER_SF_SYMBOLS,
                                 self->enc, self->bdd_enc);

        /* SAT problem for incomplete trace re-execution (timed) */
        be_problem = \
          BeEnc_untimed_expr_to_timed(self->enc,
                                      Be_And(be_mgr, be_trans,
                                      Be_And(be_mgr,
                                             Be_And(be_mgr,
                                                    be_input,
                                                    be_comb),
            BeEnc_shift_curr_to_next(self->enc, be_next))), time);

        bmc_add_be_into_solver_positively(SAT_SOLVER(solver),
                                          satGroup, be_problem, self->enc);

        satResult = SatSolver_solve_all_groups(SAT_SOLVER(solver));

        if (SAT_SOLVER_UNSATISFIABLE_PROBLEM == satResult) {
          if (0 < BaseTraceExecutor_get_verbosity(executor)) {
            fprintf(BaseTraceExecutor_get_output_stream(executor), "failed!\n");
          }
          success = false;
          terminate = true;
        }

        else {
          if (0 < BaseTraceExecutor_get_verbosity(executor)) {
            fprintf(BaseTraceExecutor_get_output_stream(executor), "ok\n");
          }
          ++ count;
          ++ time;

          nusmv_assert(SAT_SOLVER_SATISFIABLE_PROBLEM == satResult);

          /* if last state was a complete one, perform restart */
          if (partial_trace_executor_is_complete_state(partial_executor,
                                                       trace, step)) {

            if (0 < BaseTraceExecutor_get_verbosity(executor)) {
              fprintf(BaseTraceExecutor_get_output_stream(executor),
                      "-- complete state found, performing restart.\n");
            }

            if (TRACE(NULL) == res) { /* no previous fragment exists */
              res = \
                Bmc_Utils_generate_cntexample(self->enc, SAT_SOLVER(solver),
                                              be_problem, time,
                                              trace_description, language);
            }
            else { /* append fragment to existing trace */
              Trace_ptr fragment = \
                Bmc_Utils_generate_cntexample(self->enc, SAT_SOLVER(solver),
                                              be_problem, time,
                                              NIL(char), language);

              Trace_concat(res, &fragment);
              nusmv_assert(TRACE(NULL) == fragment);
            }

            /* perform restart from last state */
            SatIncSolver_destroy_group(solver, satGroup);
            satGroup = SatIncSolver_create_group(solver);

            be_problem = \
              BeEnc_untimed_expr_to_timed(self->enc, be_next, 0);

            bmc_add_be_into_solver_positively(SAT_SOLVER(solver), satGroup,
                                              be_problem, self->enc);

            time = 0; /* restart */
          } /* is complete assignment */
        }
      }  /* TRACE_END_ITER != step */
      else {
        if (0 == count) {
          fprintf(nusmv_stderr, "Warning: trace has no transitions.\n");
        }
        terminate = true;
      }
    } while (!terminate);
  }

  /* register a new trace if execution was successful */
  if (success) {

    if (0 < time) { /* last trace fragment to be extracted exists */
      if (TRACE(NULL) == res) {
        res = \
          Bmc_Utils_generate_cntexample(self->enc, SAT_SOLVER(solver),
                                        be_problem, time,
                                        trace_description, language);
      }
      else { /* append this fragment to an existing trace */
        Trace_ptr fragment = \
          Bmc_Utils_generate_cntexample(self->enc, SAT_SOLVER(solver),
                                        be_problem, time,
                                        NIL(char), /* anonymous fragment */
                                        language);

        Trace_concat(res, &fragment);
        nusmv_assert(TRACE(NULL) == fragment);
      }
    }

  } /* if success */
  else { /* (!success) -> destroy trace fragment (if any) */
    if (TRACE(NULL) != res) {
      Trace_destroy(res);
      res = TRACE(NULL);
    }
  }

  /* release the SAT solver instance */
  SatIncSolver_destroy(solver);

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

  Synopsis           [Builds the initial state formula]

  Description        [Private service of sat_complete_trace_executor_execute]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static inline be_ptr
sat_partial_trace_executor_get_initial_state(BeFsm_ptr be_fsm)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  be_ptr init = Be_And(BeEnc_get_be_manager(be_enc),
                       BeFsm_get_init(be_fsm),
                       BeFsm_get_invar(be_fsm));
  return init;
}

/**Function********************************************************************

  Synopsis           [Builds the transition relation formula]

  Description        [Private service of sat_partial_trace_executor_execute]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static inline be_ptr
sat_partial_trace_executor_get_transition_relation (BeFsm_ptr be_fsm)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr mgr = BeEnc_get_be_manager(be_enc);

  be_ptr invar = BeFsm_get_invar(be_fsm);
  be_ptr trans = BeFsm_get_trans(be_fsm);
  be_ptr n_invar = BeEnc_shift_curr_to_next(be_enc, invar);

  return Be_And(mgr, invar, Be_And(mgr, trans, n_invar));
}

/**Function********************************************************************

  Synopsis [Converts Be into CNF, and adds it into a group of a
  solver, sets polarity to 1, and then destroys the CNF.]

  Description [Private service of sat_partial_trace_executor_execute.]

  SideEffects [Outputs into nusmv_stdout the total time of conversion,
  adding, setting polarity and destroying BE.]

  SeeAlso       []

******************************************************************************/
static inline void
bmc_add_be_into_solver_positively(SatSolver_ptr solver, SatSolverGroup group,
                                  be_ptr prob, BeEnc_ptr be_enc)
{
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  Be_Cnf_ptr cnf;

  /* We force inclusion of the conjunct set to guarantee soundness */
  cnf = Be_ConvertToCnf(be_mgr, Bmc_Utils_apply_inlining4inc(be_mgr, prob), 1);
  SatSolver_add(solver, cnf, group);
  SatSolver_set_polarity(solver, cnf, 1, group);

  Be_Cnf_Delete(cnf);
}

/**AutomaticEnd***************************************************************/

