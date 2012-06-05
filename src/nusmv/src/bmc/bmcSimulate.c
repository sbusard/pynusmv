/**CFile***********************************************************************

  FileName    [bmcSimulate.c]

  PackageName [bmc]

  Synopsis    [Incremental SAT Based simulation]

  Description [Incremental SAT Based simulation]

  SeeAlso     [optional]

  Author      [Marco Roveri]

  Copyright [

  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2010 FBK-irst.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  as published by the Free Software Foundation; either version 2 of
  the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]
******************************************************************************/

#include "bmcModel.h"

#include "util.h"
#include "utils/utils.h"
#include "bmcInt.h"
#include "bmcUtils.h"
#include "bmcConv.h"
#include "trace/pkg_trace.h"
#include "trace/Trace.h"
#include "simulate/simulate.h"

#include "compile/compile.h"
#include "sat/sat.h"
#include "parser/parser.h"
#include "parser/symbols.h"
#include "parser/parser.h"
#include "bmcSimulate.h"
#include "utils/Olist.h"

#include "prop/propPkg.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcSimulate.c,v 1.1.2.12 2010-02-19 15:05:21 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define BMC_PICK_TRACE_DESCRIPTION "BMC Simulation"

#define READ_BUF_SIZE 8

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/* this is the trace used during the simulation */
static Trace_ptr bmc_sim_trace;
static int bmc_sim_trace_idx;

extern FILE* nusmv_stdin;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void
bmc_simulate_add_be_into_inc_solver_positively ARGS((SatIncSolver_ptr solver,
                                                     SatSolverGroup group,
                                                     be_ptr prob,
                                                     BeEnc_ptr be_enc));
static void
bmc_simulate_add_be_into_non_inc_solver_positively ARGS((SatSolver_ptr solver,
                                                         be_ptr prob,
                                                         BeEnc_ptr be_enc));

static void bmc_simulate_enable_random_mode ARGS((SatSolver_ptr solver));

static void bmc_simulate_print_state ARGS((Trace_ptr trace, TraceIter step,
                                           int state_num, boolean show_inputs,
                                           hash_ptr shown_assignments));

static void bmc_simulate_trace_step_print ARGS((const Trace_ptr trace,
                                                const TraceIter step,
                                                TraceIteratorType it_type,
                                                hash_ptr shown_assignments));

static Trace_ptr bmc_simulate_interactive_step ARGS((SatSolver_ptr solver,
                                                     BeEnc_ptr be_enc,
                                                     BddEnc_ptr bdd_enc,
                                                     NodeList_ptr symbols,
                                                     boolean in_simulation,
                                                     boolean display_all));

static int bmc_simulate_ask_for_state ARGS((int max_choice));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis           [Performs simulation]

   Description [Generate a problem with no property, and search for a
   solution, appending it to the current simulation trace.
   Returns 1 if solver could not be created, 0 if everything went smooth]

   SideEffects        [None]

   SeeAlso            []

******************************************************************************/
int Bmc_Simulate(const BeFsm_ptr be_fsm, BddEnc_ptr bdd_enc,
                 be_ptr be_constraints, boolean time_shift,
                 const int k, const boolean print_trace,
                 const boolean changes_only,
                 const Simulation_Mode mode)
{
  be_ptr init, prob; /* The problem in BE format */
  SatSolver_ptr solver;
  SatSolverResult sat_res;
  BeEnc_ptr be_enc;
  Be_Manager_ptr be_mgr;
  Be_Cnf_ptr cnf;

  Trace_ptr trace = bmc_simulate_get_curr_sim_trace();
  int tr_num = bmc_simulate_get_curr_sim_trace_index();

  TRACE_CHECK_INSTANCE(trace); /* curr trace was picked */

  be_enc = BeFsm_get_be_encoding(be_fsm);
  be_mgr = BeEnc_get_be_manager(be_enc);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Simulation of length %d (no loopback)\n", k);
  }

  solver = Sat_CreateNonIncSolver(get_sat_solver(OptsHandler_get_instance()));
  if (solver == SAT_SOLVER(NULL)) {
    fprintf(nusmv_stderr,
            "Non-incremental sat solver '%s' is not available.\n",
            get_sat_solver(OptsHandler_get_instance()));
    return 1;
  }

  switch (mode) {
  case Random:
    bmc_simulate_enable_random_mode(SAT_SOLVER(solver));
    break;
  case Interactive:
    internal_error("%s: Interactive mode not supported yet", __func__);
    break;
  case Deterministic:
    /* Do nothing */
    break;
  default:
    internal_error("%s: Invalid mode given", __func__);
  }

  /* starting state taken from the last node of the current sim trace */
  init = BeEnc_untimed_expr_to_timed(be_enc,
               TraceUtils_fetch_as_be(trace, Trace_last_iter(trace),
                                      TRACE_ITER_SF_VARS, be_enc, bdd_enc), 0);

  prob = Be_And(be_mgr, Bmc_Model_GetPathNoInit(be_fsm, k), init);

  /* Use constraints only if actually necessary */
  if (!Be_IsTrue(be_mgr, be_constraints)) {
    int i;
    for (i = 0; i <= k; ++i) {
      be_ptr be_timed = be_constraints;

      if (time_shift) {
        be_timed = BeEnc_shift_curr_to_next(be_enc, be_timed);
      }

      be_timed = BeEnc_untimed_expr_to_timed(be_enc, be_timed, i);

      prob = Be_And(be_mgr, prob, be_timed);
    }
  }

  prob = Bmc_Utils_apply_inlining(be_mgr, prob);
  cnf = Be_ConvertToCnf(be_mgr, prob, 1);

  SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
  SatSolver_set_polarity(solver, cnf, 1,
                         SatSolver_get_permanent_group(solver));
  sat_res = SatSolver_solve_all_groups(solver);

  /* Processes the result: */
  switch (sat_res) {

  case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
    fprintf(nusmv_stdout,
            "The model deadlocks before requested length %d!\n", k);
    break;

  case SAT_SOLVER_SATISFIABLE_PROBLEM:
    {
      BoolSexpFsm_ptr bsexp_fsm; /* needed for trace language */
      bsexp_fsm =  \
        PropDb_master_get_bool_sexp_fsm(PropPkg_get_prop_database());
      BOOL_SEXP_FSM_CHECK_INSTANCE(bsexp_fsm);

      Trace_ptr extension = \
        Bmc_create_trace_from_cnf_model(be_enc,
             SexpFsm_get_symbols_list(SEXP_FSM(bsexp_fsm)), NIL(char),
                                      TRACE_TYPE_UNSPECIFIED,
                                      SatSolver_get_model(solver), k);

      trace = Trace_concat(trace, &extension);
      nusmv_assert(TRACE(NULL) == extension);

      if (print_trace) {
        TraceManager_execute_plugin(global_trace_manager, TRACE_OPT(NULL),
                                    (changes_only) ? 0 : 1, tr_num);
      }

      break;
    } /* SAT */

  case SAT_SOLVER_INTERNAL_ERROR:
    internal_error("Sorry, solver answered with a fatal Internal "
                   "Failure during problem solving.\n");

  case SAT_SOLVER_TIMEOUT:
  case SAT_SOLVER_MEMOUT:
    internal_error("Sorry, solver ran out of resources and aborted "
                   "the execution.\n");

  default:
    internal_error(" Bmc_Simulate: Unexpected value in sat result");
  } /* switch */

  /* cleanup */
  SatSolver_destroy(solver);
  Be_Cnf_Delete(cnf);

  return 0;
}

/**Function********************************************************************

  Synopsis           [SAT Based Incremental simulation]

  Description        [This function performs incremental sat based
  simulation up to <tt>target_steps</tt>.

  Simulation starts from an initial state internally selected.

  It accepts a constraint to direct the simulation to paths satisfying
  such constraints. The constraints is assumed to be over state, input
  and next state variables. Thus, please carefully consider this
  information while providing constraints to this routine.

  The simulation stops if either the <tt>target_steps</tt> steps of
  simulation have been performed, or the simulation bumped in a
  deadlock (that might be due to the constraints that are too strong).

  Parameters:

  'print_trace'  : shows the generated trace
  'changes_only' : shows only variables that actually change value
  between one step and it's next one]

  SideEffects        [The possibly partial generated simulaiton trace
  is added to the trace manager for possible reuse.]

  SeeAlso            [optional]

******************************************************************************/
int Bmc_StepWiseSimulation(BeFsm_ptr be_fsm,
                           BddEnc_ptr bdd_enc,
                           TraceManager_ptr trace_manager,
                           int target_steps,
                           be_ptr constraints,
                           boolean time_shift,
                           boolean print_trace,
                           boolean changes_only,
                           Simulation_Mode mode,
                           boolean display_all)
{
#if NUSMV_HAVE_INCREMENTAL_SAT
  int steps;
  boolean no_deadlock;
  BeEnc_ptr be_enc = BE_ENC(NULL);
  Be_Manager_ptr be_mgr = (Be_Manager_ptr)NULL;
  SatIncSolver_ptr solver = SAT_INC_SOLVER(NULL);
  SatSolverGroup satGrp = (SatSolverGroup)-1;
  SatSolverResult satResult = SAT_SOLVER_UNAVAILABLE;
  Trace_ptr trace = bmc_simulate_get_curr_sim_trace();
  int tr_num = bmc_simulate_get_curr_sim_trace_index();
  be_ptr be_trans = (be_ptr)NULL;
  long time_statistics = util_cpu_time();

  TRACE_CHECK_INSTANCE(trace); /* a trace was picked */

  if (target_steps <= 0) return -1;

  /* Create the SAT solver instance */
  solver  = Sat_CreateIncSolver(get_sat_solver(OptsHandler_get_instance()));
  if (SAT_INC_SOLVER(NULL) == solver) {
    fprintf(nusmv_stderr,
            "Incremental sat solver '%s' is not available.\n",
            get_sat_solver(OptsHandler_get_instance()));
    return -1;
  }

  switch (mode) {
  case Random:
    bmc_simulate_enable_random_mode(SAT_SOLVER(solver));
    break;
  case Interactive:
    /* Do nothing */
    break;
  case Deterministic:
    /* Do nothing */
    break;
  default:
    internal_error("%s: Invalid mode given", __func__);
  }

  no_deadlock = true;

  be_enc = BeFsm_get_be_encoding(be_fsm);
  be_mgr = BeEnc_get_be_manager(be_enc);

  /* 1. Add the transition relation into the solver permanently */
  be_trans = BeFsm_get_invar(be_fsm);
  be_trans = Be_And(be_mgr,
                    be_trans,
                    BeEnc_shift_curr_to_next(be_enc, be_trans));
  be_trans = Be_And(be_mgr, BeFsm_get_trans(be_fsm), be_trans);

  /* We force the constraints that can be over starting states, or
     over next states, or over the input. If the constraint is over
     the current state variables, then it might be the case the chosen
     next statre does not satisfy the constraint. */
  if (time_shift) {
    constraints = BeEnc_shift_curr_to_next(be_enc, constraints);
  }

  be_trans = Be_And(be_mgr, be_trans, constraints);

  /* Necessary to re-use the routines for extracting the model */
  be_trans = BeEnc_untimed_expr_to_timed(be_enc, be_trans, 0);
  bmc_simulate_add_be_into_inc_solver_positively(SAT_INC_SOLVER(solver),
                      SatSolver_get_permanent_group(SAT_SOLVER(solver)),
                      be_trans, be_enc);

  /* 2. Iteration adding last computed state as src state and
        extracting the pair (input,next_state), adding the pair to the
        computed trace, and then iterating from the so far computed
        next_state */
  {
    for (steps = 0; ((steps < target_steps) && no_deadlock) ; steps++) {
      be_ptr be_src_state = (be_ptr)NULL;

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
        fprintf(nusmv_stderr, "Performing simulation step %d ...", steps+1);
      }

      /* 2.0. Extraction from the so far compute trace the last
         state. */
      be_src_state = BeEnc_untimed_expr_to_timed(be_enc,
                         TraceUtils_fetch_as_be(trace, Trace_last_iter(trace),
                                        TRACE_ITER_SF_VARS, be_enc, bdd_enc), 0);

      /* 2.2 Create the group to store the last state and, add the
           last state to the solver */
      satGrp = SatIncSolver_create_group(solver);
      bmc_simulate_add_be_into_inc_solver_positively(SAT_INC_SOLVER(solver),
                                                     satGrp,
                                                     be_src_state, be_enc);

      /* 2.3. Call the solver on the instantiated problem */
      satResult = SatSolver_solve_all_groups(SAT_SOLVER(solver));

      switch (satResult) {
      case SAT_SOLVER_SATISFIABLE_PROBLEM:
        if (Interactive == mode) {
          Trace_ptr iTrace =
            bmc_simulate_interactive_step(SAT_SOLVER(solver), be_enc,
                                          bdd_enc, Trace_get_symbols(trace),
                                          true, display_all);
          Trace_concat(trace, &iTrace);
        }
        else {
          /* Append current computed state to the trace */
          bmc_trace_utils_append_input_state(trace, be_enc,
                                   SatSolver_get_model(SAT_SOLVER(solver)));
        }

        break;

      case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
        fprintf(nusmv_stderr,
            "The model reached a deadlock state: iteration %d.\n", steps);
        if (!Be_IsTrue(be_mgr, constraints)) {
          fprintf(nusmv_stderr,
            "This might be due to the constraints that are too strong.\n");
        }
        no_deadlock = false;
        break;

      default:
        fprintf(nusmv_stderr,
            "At iteration %d, the solver returned an unexpected value: %d\n",
                steps, satResult);
        no_deadlock = false;
        break;
      }

      /* 2.4. Remove and destroy the group containing the last
              computed state */
      SatIncSolver_destroy_group(solver, satGrp);

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
        fprintf(nusmv_stderr, "... done\n");
      }

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
        fprintf(nusmv_stdout,
                " -- simulation of step %d has finished in %2.1f seconds\n",
                steps, (util_cpu_time() - time_statistics) / 1000.0);
        time_statistics = util_cpu_time();
      }
    } /* (steps < target_steps) && no_deadlock) */

  } /* iteration block */

  /* 3. We clean the memory and we return */
  SatIncSolver_destroy(solver);

  if (no_deadlock && print_trace) {
    TraceManager_execute_plugin(trace_manager, TRACE_OPT(NULL),
                                (changes_only) ? 0 : 1, tr_num);
  }

  return steps;

#else
  fprintf(nusmv_stderr,
          "%s: Relies on Incremental solving. "
          "No incremental SAT solver is available now\n", __func__);
  return -1;
#endif
}


/**Function********************************************************************

  Synopsis           [Checks the truth value of a list of constraints on the
                      current state, transitions and next states,
                      from given starting state. This can be used
                      in guided interactive simulation to propose
                      the set of transitions which are allowed to
                      occur in the interactive simulation.]

  Description        [Given a list of constraints (next-expressions as be_ptr),
                      checks which (flattened) constraints are
                      satisfiable from a given state. Iff
                      from_state is NULL (and not TRUE), the
                      initial state of the fsm is
                      considered. Returned list contains values in
                      {0,1}, and has to be freed.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Olist_ptr
Bmc_simulate_check_feasible_constraints(BeFsm_ptr be_fsm,
                                        BddEnc_ptr bdd_enc,
                                        Olist_ptr constraints,
                                        be_ptr from_state)
{
  Olist_ptr res = Olist_create();
#if NUSMV_HAVE_INCREMENTAL_SAT
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);

  SatIncSolver_ptr solver;
  be_ptr be_init, be_trans, be_prob;

  /* Create the SAT solver instance */
  solver = Sat_CreateIncSolver(get_sat_solver(OptsHandler_get_instance()));
  if (SAT_INC_SOLVER(NULL) == solver) {
    internal_error("Incremental sat solver '%s' is not available.\n",
                   get_sat_solver(OptsHandler_get_instance()));
  }

  be_init = (from_state == (be_ptr) NULL) ? BeFsm_get_init(be_fsm) : from_state;
  be_init = Be_And(be_mgr, be_init, BeFsm_get_invar(be_fsm));

  be_trans = Be_And(be_mgr,
                    BeFsm_get_trans(be_fsm),
                    BeEnc_shift_curr_to_next(be_enc, BeFsm_get_invar(be_fsm)));

  be_prob = Be_And(be_mgr, be_init, be_trans);
  be_prob = BeEnc_untimed_expr_to_timed(be_enc, be_prob, 0);

  /* pushes the transition relation */
  bmc_simulate_add_be_into_inc_solver_positively(SAT_INC_SOLVER(solver),
                            SatSolver_get_permanent_group(SAT_SOLVER(solver)),
                            be_prob, be_enc);

  { /* loops over the constraints list */
    Oiter iter;
    OLIST_FOREACH(constraints, iter) {
      be_ptr be_constr = (be_ptr) Oiter_element(iter);
      boolean constr_truth;
      SatSolverGroup satGrp;
      SatSolverResult sat_res;

      be_constr = BeEnc_untimed_expr_to_timed(be_enc, be_constr, 0);

      /* pushes the constraint */
      satGrp = SatIncSolver_create_group(solver);
      bmc_simulate_add_be_into_inc_solver_positively(SAT_INC_SOLVER(solver),
                                                     satGrp, be_constr,
                                                     be_enc);

      /* solves */
      sat_res = SatSolver_solve_all_groups(SAT_SOLVER(solver));
      switch (sat_res) {
      case SAT_SOLVER_SATISFIABLE_PROBLEM:
        constr_truth = true; break;

      case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
        constr_truth = false; break;

      default:
        error_unreachable_code(); /* no other options handled! */
      }

      Olist_append(res, (void*) constr_truth);
      SatIncSolver_destroy_group(solver, satGrp);
    } /* constr loop */
  }

  /* clean up */
  SatIncSolver_destroy(solver);

  /* there is a result for each input */
  nusmv_assert(Olist_get_size(constraints) == Olist_get_size(res));

#else
  fprintf(nusmv_stderr, "%s: Relies on Incremental solving. No incremental SAT solver is available now\n",
          __func__);
#endif

  return res;
}

/**Function********************************************************************

  Synopsis          [Picks a state from the initial state, creates a trace
                     from it.]

  Description       [The trace is added into the trace manager.
                     Returns the index of the added trace, or -1 if
                     no trace was created.]

  SideEffects       [A new trace possibly created into the trace manager]

  SeeAlso           []

******************************************************************************/
int Bmc_pick_state_from_constr(BeFsm_ptr fsm, BddEnc_ptr bdd_enc,
                               be_ptr constr, Simulation_Mode mode,
                               boolean display_all)
{
  TraceManager_ptr tm = TracePkg_get_global_trace_manager();
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  SatSolver_ptr solver = \
    Sat_CreateNonIncSolver(get_sat_solver(OptsHandler_get_instance()));
  SatSolverResult sat_res;
  int tr_num = -1;

  be_ptr init, invar, prob;

  if (solver == SAT_SOLVER(NULL)) {
    internal_error("Non-incremental sat solver '%s' is not available.",
                   get_sat_solver(OptsHandler_get_instance()));
  }

  switch (mode) {
  case Random:
    bmc_simulate_enable_random_mode(SAT_SOLVER(solver));
    break;
  case Interactive:
    /* Do nothing */
    break;
  case Deterministic:
    /* Do nothing */
    break;
  default:
    internal_error("%s: Invalid mode given", __func__);
  }

  init = BeFsm_get_init(fsm);
  invar = BeFsm_get_invar(fsm);
  prob = Be_And(be_mgr, Be_And(be_mgr, init, Be_And(be_mgr, invar, constr)),
                constr);
  prob = BeEnc_untimed_expr_to_timed(be_enc, prob, 0);

  bmc_simulate_add_be_into_non_inc_solver_positively(SAT_SOLVER(solver),
                                                     prob, be_enc);
  sat_res = SatSolver_solve_all_groups(solver);

  /* Processes the result: */
  switch (sat_res) {
  case SAT_SOLVER_SATISFIABLE_PROBLEM: {

    /* extracts the trace */
    Trace_ptr trace;

    /* this was set to false in previous implementation */
    boolean has_constraints = false;

    { /* Build language for the FSM */
      BoolSexpFsm_ptr bsexp_fsm; /* needed for trace language */
      NodeList_ptr symbols;
      bsexp_fsm =  \
          PropDb_master_get_bool_sexp_fsm(PropPkg_get_prop_database());
      BOOL_SEXP_FSM_CHECK_INSTANCE(bsexp_fsm);
      symbols = SexpFsm_get_symbols_list(SEXP_FSM(bsexp_fsm));

      if (Interactive == mode) {
        trace = bmc_simulate_interactive_step(solver, be_enc, bdd_enc,
                                              symbols, false, display_all);
      }
      else {
        trace =                                 \
          Bmc_create_trace_from_cnf_model(be_enc,
                                          symbols,
                                          BMC_PICK_TRACE_DESCRIPTION,
                                          TRACE_TYPE_SIMULATION,
                                          SatSolver_get_model(solver),
                                          has_constraints ? 1 : 0);
      }
    }

    /* Registering the trace in the trace manager */
    tr_num = TraceManager_register_trace(tm, trace);
    TraceManager_set_current_trace_number(tm, tr_num);
    bmc_simulate_set_curr_sim_trace(trace, tr_num);
    break;
  }

  case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
    /* deadlock in the initial state */
    bmc_simulate_set_curr_sim_trace(NULL, -1);
    tr_num = -1;
    break;

  default:
    internal_error("%s: Unexpected value in sat result", __func__);
  }

  return tr_num;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis      [Internal function used during the simulation to set the
  current simulation trace]

  Description   []

  SideEffects   []

  SeeAlso       []

******************************************************************************/
void bmc_simulate_set_curr_sim_trace(Trace_ptr trace, int idx)
{
  bmc_sim_trace = trace;
  bmc_sim_trace_idx = idx;
}

Trace_ptr bmc_simulate_get_curr_sim_trace()
{ return bmc_sim_trace; }

int bmc_simulate_get_curr_sim_trace_index()
{ return bmc_sim_trace_idx; }


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis      [Converts Be into CNF, and adds it into a group of a
                 incremental solver, sets polarity to 1, and then destroys
                 the CNF.]

  Description   [Outputs into nusmv_stdout the total time of conversion,
                 adding, setting polarity and destroying BE. ]

  SideEffects   []

  SeeAlso       []

******************************************************************************/
static void
bmc_simulate_add_be_into_inc_solver_positively(SatIncSolver_ptr solver,
                                               SatSolverGroup group,
                                               be_ptr prob,
                                               BeEnc_ptr be_enc)
{
  Be_Cnf_ptr cnf;
  Be_Manager_ptr be_mgr;
  be_ptr inprob;
  int polarity = 1;

  /* get the be manager for applying inlining */
  be_mgr = BeEnc_get_be_manager(be_enc);

  /* We force inclusion of the conjunct set to guarantee soundness */
  inprob = Bmc_Utils_apply_inlining4inc(be_mgr, prob);

  /* Convert the problem into CNF */
  cnf = Be_ConvertToCnf(be_mgr, inprob, polarity);

  /* Add the problem into the solver */
  SatSolver_add(SAT_SOLVER(solver), cnf, group);

  /* Force the added group to be considered positively */
  SatSolver_set_polarity(SAT_SOLVER(solver), cnf, polarity, group);

  /* The CNF is no longer needed */
  Be_Cnf_Delete(cnf);
}


/**Function********************************************************************

  Synopsis      [Converts Be into CNF, and adds it into a group of a
                 non-incremental solver, sets polarity to 1, and
                 then destroys the CNF.]

  Description   [Outputs into nusmv_stdout the total time of conversion,
                 adding, setting polarity and destroying BE. ]

  SideEffects   []

  SeeAlso       []

******************************************************************************/
static void
bmc_simulate_add_be_into_non_inc_solver_positively(SatSolver_ptr solver,
                                                   be_ptr prob,
                                                   BeEnc_ptr be_enc)
{
  Be_Cnf_ptr cnf;
  Be_Manager_ptr be_mgr;
  be_ptr inprob;
  int polarity = 1;

  /* get the be manager for applying inlining */
  be_mgr = BeEnc_get_be_manager(be_enc);

  /* We force inclusion of the conjunct set to guarantee soundness */
  inprob = Bmc_Utils_apply_inlining(be_mgr, prob);

  /* Convert the problem into CNF */
  cnf = Be_ConvertToCnf(be_mgr, inprob, polarity);

  /* Add the problem into the solver */
  SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));

  /* Force the added group to be considered positively */
  SatSolver_set_polarity(solver, cnf, polarity,
                         SatSolver_get_permanent_group(solver));

  /* The CNF is no longer needed */
  Be_Cnf_Delete(cnf);
}

/**Function********************************************************************

  Synopsis      [Enables random mode in the given sat solver]

  Description   [Enables random mode in the given sat solver.
                 Seed used in random]

  SideEffects   []

  SeeAlso       []

******************************************************************************/
static void bmc_simulate_enable_random_mode(SatSolver_ptr solver)
{
  utils_random_set_seed();
  SatSolver_set_random_mode(solver, utils_random());
}

/**Function********************************************************************

   Synopsis           [Performs a step of interactive simulation]

   Description        [Performs a step of interactive simulation.

                       Finds all alternative assignments of the
                       current model in the given sat solver. For this
                       reason, the function must be called only after
                       a call to SatSolver_solve_all_groups that
                       returned SAT. "in_simulation" determines
                       whether the interactive step should be done for
                       extending a trace (i.e. simulate) or when
                       creating a new one (i.e. pick_state). Returns a
                       trace that represents the new choosen state,
                       which can be used as starting point for
                       pick_state and can be concatenated to the
                       previously existing one when simulating]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static Trace_ptr bmc_simulate_interactive_step(SatSolver_ptr solver,
                                               BeEnc_ptr be_enc,
                                               BddEnc_ptr bdd_enc,
                                               NodeList_ptr symbols,
                                               boolean in_simulation,
                                               boolean display_all)
{
  const Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  const int max_states = opt_shown_states_level(OptsHandler_get_instance());
  const SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(be_enc));

  Trace_ptr result = TRACE(NULL);

  Slist_ptr curr_model = SatSolver_get_model(solver);
  Trace_ptr curr_trace = Trace_create(st, BMC_PICK_TRACE_DESCRIPTION,
                                      TRACE_TYPE_SIMULATION, symbols, true);
  Trace_ptr traces[max_states];
  Slist_ptr groups = Slist_create();
  Siter siter;
  int i = 1, j = 0, choice = 0;
  boolean found_all_states = false;

  /* Fill the first temporary trace */
  Bmc_fill_trace_from_cnf_model(be_enc, curr_model,
                                (in_simulation ? 1 : 0), curr_trace);
  /* Append the current set of assignments */
  traces[0] = curr_trace;

  /* Now lookup for other states */
  while ((i < max_states) && (!found_all_states)) {
    SatSolverGroup iSatGrp = (SatSolverGroup)-1;
    SatSolverResult iSatResult = SAT_SOLVER_UNAVAILABLE;
    be_ptr be_tr;
    be_ptr be_ass;

    /* Retrieve the variables assignments from the very latest
       trace: Those will be negated and added as a group to
       the sat solver */
    be_tr = Be_Not(be_mgr,
                   TraceUtils_fetch_as_be(curr_trace,
                                          (in_simulation ?
                                           Trace_last_iter(curr_trace) :
                                           Trace_first_iter(curr_trace)),
                                          TRACE_ITER_SF_VARS, be_enc, bdd_enc));
    be_ass = BeEnc_untimed_expr_to_timed(be_enc, be_tr, (in_simulation ? 1 : 0));

    if (in_simulation) {
      iSatGrp = SatIncSolver_create_group(SAT_INC_SOLVER(solver));
      bmc_simulate_add_be_into_inc_solver_positively(SAT_INC_SOLVER(solver),
                                                     iSatGrp, be_ass, be_enc);
      Slist_push(groups, (void*)iSatGrp);
    }
    else {
      bmc_simulate_add_be_into_non_inc_solver_positively(solver, be_ass, be_enc);
    }

    iSatResult = SatSolver_solve_all_groups(solver);

    /* Processes the result: */
    switch (iSatResult) {

    case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
      /* no more possible states.. Quit the loop  */
      found_all_states = true;
      break;

    case SAT_SOLVER_SATISFIABLE_PROBLEM:
      {
        curr_model = SatSolver_get_model(solver);
        curr_trace = Trace_create(st, BMC_PICK_TRACE_DESCRIPTION,
                                  TRACE_TYPE_SIMULATION, symbols, true);
        /* Build the trace upon what we just found */
        Bmc_fill_trace_from_cnf_model(be_enc, curr_model,
                                      (in_simulation ? 1 : 0), curr_trace);

        /* Append the current set of assignments */
        traces[i] = curr_trace;

        ++i; /* Found one state, count it */
        break;
      } /* SAT */

    case SAT_SOLVER_INTERNAL_ERROR:
      internal_error("Sorry, solver answered with a fatal Internal "
                     "Failure during problem solving.\n");

    case SAT_SOLVER_TIMEOUT:
    case SAT_SOLVER_MEMOUT:
      internal_error("Sorry, solver ran out of resources and aborted "
                     "the execution.\n");

    default:
      internal_error(" Bmc_Simulate: Unexpected value in sat result");
    } /* switch */
  }

  { /* Start printing found states */
    hash_ptr shown_assignments = (hash_ptr)NULL;

    if (!display_all) {
      shown_assignments = new_assoc();
    }

    /* Show found states */
    fprintf(nusmv_stdout,
            "\n***************  AVAILABLE STATES  *************\n");
    for (j = 0; j < i; ++j) {
      if (in_simulation) {
        bmc_simulate_print_state(traces[j], Trace_last_iter(traces[j]),
                                 j, in_simulation, shown_assignments);
      }
      else {
        bmc_simulate_print_state(traces[j], Trace_first_iter(traces[j]),
                                 j, in_simulation, shown_assignments);
      }
    }

    if (!display_all) {
      free_assoc(shown_assignments);
    }
  } /* End printing found states */

  /* Let the user choose */
  choice = bmc_simulate_ask_for_state(j - 1);

  /* Pick the "trace" (i.e. the state) choosed by the user */
  result = traces[choice];

  /* Free Memory */
  for (j = 0; j < i; ++j) {
    /* The one left will be returned, shall be destroyed by the
       caller. */
    if (j != choice) { Trace_destroy(traces[j]); }
  }

  SLIST_FOREACH(groups, siter) {
    SatSolverGroup g = (SatSolverGroup)Siter_element(siter);

    /* Just because we expect an incremental solver only if simulating
       (bmc_inc_simulate). Pick state uses a non incremental sat
       solver. */
    nusmv_assert(in_simulation);
    SatIncSolver_destroy_group(SAT_INC_SOLVER(solver), g);
  }
  Slist_destroy(groups);

  nusmv_assert(TRACE(NULL) != result);
  return result;
}

/**Function********************************************************************

   Synopsis           [Aux function for interactive simulation.
                       Prints the given set of assignments]

   Description        [Prints all variable assignments of trace "trace", in
                       step "step"]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bmc_simulate_print_state(Trace_ptr trace, TraceIter step,
                                     int state_num, boolean show_inputs,
                                     hash_ptr shown_assignments)
{
  NodeList_ptr inputs = Trace_get_i_vars(trace);
  boolean has_inputs = NodeList_get_length(inputs) > 0;

  fprintf(nusmv_stdout, "\n================= State =================\n");
  fprintf(nusmv_stdout, "%d) -------------------------\n", state_num);

  bmc_simulate_trace_step_print(trace, step, TRACE_ITER_SF_VARS,
                                shown_assignments);

  if (has_inputs && show_inputs) {
    fprintf(nusmv_stdout, "\nThis state is reachable through:\n");
    bmc_simulate_trace_step_print(trace, step, TRACE_ITER_I_VARS,
                                  shown_assignments);
  }

  fprintf(nusmv_stdout, "\n");
}

/**Function********************************************************************

   Synopsis           [Aux function for interactive simulation.
                       Prints the given set of assignments]

   Description        [Prints all variable assignments of trace "trace", in
                       step "step" and of type "it_type"]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bmc_simulate_trace_step_print(const Trace_ptr trace,
                                          const TraceIter step,
                                          TraceIteratorType it_type,
                                          hash_ptr shown_assignments)
{
  TraceStepIter iter;
  node_ptr symb, val;

  TRACE_STEP_FOREACH(trace, step, it_type, iter, symb, val) {
    if ((hash_ptr)NULL != shown_assignments) {
      if (val == find_assoc(shown_assignments, symb)) continue;

      insert_assoc(shown_assignments, symb, val);
    }

    fprintf(nusmv_stdout, "   ");
    print_node(nusmv_stdout, symb);
    fprintf(nusmv_stdout, " = ");
    print_node(nusmv_stdout, val);
    fprintf(nusmv_stdout, "\n");
  }
}

/**Function********************************************************************

   Synopsis           [Aux function for interactive simulation.
                       Asks the user for a number from 0 to max_choice.]

   Description        [Asks the user for a number from 0 to max_choice. Input
                       is taken from nusmv_stdin. Returns the selected number]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int bmc_simulate_ask_for_state(int max_choice)
{
  int choice = 0;
  int i;

  if (max_choice > 0) {
    char line[READ_BUF_SIZE];

    for (i = 0; i < READ_BUF_SIZE; i++) line[i] = '\0';

    fprintf(nusmv_stdout,
            "\nChoose a state from the above (0-%d): ", max_choice);

    while (NIL(char) != (fgets(line, READ_BUF_SIZE, nusmv_stdin)) ||
           (line[0] != '\n') ) {
      if ((sscanf(line, "%d", &choice) != 1) || (choice < 0) ||
          (choice > max_choice)) {
        fprintf(nusmv_stdout,
                "Choose a state from the above (0-%d): ", max_choice);
        continue;
      }
      else break;
    }
  }
  else {
    /* there is only one possible choice here: */
    fprintf(nusmv_stdout,
            "\nThere's only one available state. Press Return to Proceed.");
    while ((choice = getc(nusmv_stdin)) != '\n') ; /* consumes chars */
    choice = 0;
  }

  fprintf(nusmv_stdout, "\nChosen state is: %d\n", choice);

  return choice;
}
