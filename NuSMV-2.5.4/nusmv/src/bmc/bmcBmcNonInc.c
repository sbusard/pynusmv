/**CFile***********************************************************************

   FileName [bmcBmcNonInc.c]

   PackageName [bmc]

   Synopsis [High level functionalities layer for non incremental sat
   solving]

   Description []

   SeeAlso  []

   Author   [Roberto Cavada]

   Copyright [ This file is part of the ``bmc'' package of NuSMV
   version 2.  Copyright (C) 2004 by FBK-irst.

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

   For more information on NuSMV see <http://nusmv.fbk.eu> or
   email to <nusmv-users@fbk.eu>.  Please report bugs to
   <nusmv-users@fbk.eu>.

   To contact the NuSMV development board, email to
   <nusmv@fbk.eu>. ]

******************************************************************************/

#include "bmcBmc.h"
#include "bmcInt.h"
#include "bmcGen.h"
#include "bmcDump.h"
#include "bmcModel.h"
#include "bmcConv.h"
#include "bmcUtils.h"

#include "wff/wff.h"
#include "wff/w2w/w2w.h"

#include "enc/be/BeEnc.h"
#include "enc/enc.h"
#include "be/be.h"

#include "parser/symbols.h"

#include "mc/mc.h" /* for print_spec */
#include "sat/sat.h" /* for solver and result */
#include "sat/SatSolver.h"
#include "sat/SatIncSolver.h"
#include "prop/propPkg.h"

#include "trace/Trace.h"
#include "trace/TraceManager.h"

#include "dag/dag.h"
#include "node/node.h"
#include "utils/error.h"


#ifdef BENCHMARKING
#include <time.h>
clock_t start_time;
#endif


static char rcsid[] UTIL_UNUSED = "$Id: bmcBmcNonInc.c,v 1.1.2.13.2.8.4.18 2010-02-12 17:14:49 nusmv Exp $";

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

#define BMC_REWRITE_INVARSPEC_LAYER_NAME "bmc_invarspec_rewrite_layer"

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis           [Given a LTL property generates and solve the problems
   for all Ki (k_min<=i<=k_max). If bIncreaseK is 0 then k_min==k_max==k and
   only one problem is generated. If bIncreaseK is 1 then k_min == 0 and
   k_max == k.
   Each problem Ki takes into account of all possible loops from k_min to Ki
   if loopback is '*' (BMC_ALL_LOOPS). <BR>
   Also see the Bmc_GenSolve_Action possible values. Returns 1 if solver could
   not be created, 0 if everything went smooth]

   Description [Returns 1 if solver could not be created, 0 if
   everything went smooth]

   SideEffects        []

   SeeAlso            [Bmc_GenSolve_Action]

******************************************************************************/
int Bmc_GenSolveLtl(Prop_ptr ltlprop,
                    const int k, const int relative_loop,
                    const boolean must_inc_length,
                    const boolean must_solve,
                    const Bmc_DumpType dump_type,
                    const char* dump_fname_template)
{
  node_ptr bltlspec;  /* Its booleanization */
  BeFsm_ptr be_fsm; /* The corresponding be fsm */
  BeEnc_ptr be_enc;
  Be_Manager_ptr be_mgr;

  /* ---------------------------------------------------------------------- */
  /* Here a property was selected                                           */
  /* ---------------------------------------------------------------------- */
  int k_max = k;
  int k_min = 0;
  int increasingK;
  boolean found_solution;

  /* checks that a property was selected: */
  nusmv_assert(ltlprop != PROP(NULL));

  /* checks if it has already been checked: */
  if (Prop_get_status(ltlprop) != Prop_Unchecked) {
    /* aborts this check */
    return 0;
  }

  found_solution = false;
  if (!must_inc_length) k_min = k_max;

  /* booleanized, negated and NNFed formula: */
  bltlspec
    = Wff2Nnf(Wff_make_not(Compile_detexpr2bexpr(Enc_get_bdd_encoding(),
                                                  Prop_get_expr_core(ltlprop))));

  if (opt_cone_of_influence(OptsHandler_get_instance()) == true) {
    Prop_apply_coi_for_bmc(ltlprop, global_fsm_builder);
  }

  be_fsm = Prop_get_be_fsm(ltlprop);
  if (be_fsm == (BeFsm_ptr) NULL) {
    PropDb_set_fsm_to_master(PropPkg_get_prop_database(), ltlprop);
    be_fsm = Prop_get_be_fsm(ltlprop);
    nusmv_assert(be_fsm != (BeFsm_ptr) NULL);
  }

  be_enc = BeFsm_get_be_encoding(be_fsm);
  be_mgr = BeEnc_get_be_manager(be_enc);

  /* Start problems generations: */
  for (increasingK = k_min; (increasingK <= k_max) && ! found_solution;
       ++increasingK) {
    int l;
    char szLoop[16]; /* to keep loopback string */
    be_ptr prob; /* The problem in BE format */
    Be_Cnf_ptr cnf; /* The CNFed be problem */

    /* the loopback value could be depending on the length
       if it were relative: */
    l = Bmc_Utils_RelLoop2AbsLoop(relative_loop, increasingK);

    /* this is for verbose messages */
    Bmc_Utils_ConvertLoopFromInteger(relative_loop, szLoop, sizeof(szLoop));

    /* prints a verbose message: */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      if (Bmc_Utils_IsNoLoopback(l)) {
        fprintf(nusmv_stderr,
                "\nGenerating problem with bound %d, no loopback...\n",
                increasingK);
      }
      else if (Bmc_Utils_IsAllLoopbacks(l)) {
        fprintf(nusmv_stderr,
                "\nGenerating problem with bound %d, all possible loopbacks...\n",
                increasingK);
      }
      else {
        /* l can be negative iff loopback from the user pov is < -length */
        if ((l < increasingK) && (l >= 0)) {
          fprintf(nusmv_stderr,
                  "\nGenerating problem with bound %d, loopback %s...\n",
                  increasingK, szLoop);
        }
      }
    } /* verbose message */

    /* checks for loopback vs k compatibility */
    if (Bmc_Utils_IsSingleLoopback(l) && ((l >= increasingK) || (l < 0))) {
      fprintf(nusmv_stderr,
              "\nWarning: problem with bound %d and loopback %s is not allowed: skipped\n",
              increasingK, szLoop);
      continue;
    }

    /* generates the problem: */
#ifdef BENCHMARKING
    fprintf(nusmv_stdout,":START:benchmarking Generation\n");
    start_time = clock();
#endif

    prob = Bmc_Gen_LtlProblem(be_fsm, bltlspec, increasingK, l);
    prob = Bmc_Utils_apply_inlining(be_mgr, prob); /* inline if needed */

#ifdef BENCHMARKING
    fprintf(nusmv_stdout,":UTIME = %.4f secs.\n",
            ((double)(clock()-start_time))/CLOCKS_PER_SEC);
    fprintf(nusmv_stdout,":STOP:benchmarking Generation\n");
#endif

    /* Problem is cnf-ed */
    cnf = (Be_Cnf_ptr) NULL;

    /* Problem dumping: */
    if (dump_type != BMC_DUMP_NONE) {
      cnf = Be_ConvertToCnf(be_mgr, prob, 1);
      Bmc_Dump_WriteProblem(be_enc, cnf, ltlprop, increasingK, l,
                            dump_type, dump_fname_template);
    }

    /* SAT problem solving */
    if (must_solve) {
      SatSolver_ptr solver;
      SatSolverResult sat_res;

      /* Sat construction */
      solver = Sat_CreateNonIncSolver(get_sat_solver(OptsHandler_get_instance()));
      if (solver == SAT_SOLVER(NULL)) {
        fprintf(nusmv_stderr,
                "Non-incremental sat solver '%s' is not available.\n",
                get_sat_solver(OptsHandler_get_instance()));

        if (cnf != (Be_Cnf_ptr) NULL) Be_Cnf_Delete(cnf);
        return 1;
      }

      /* Cnf construction (if needed): */
      if (cnf == (Be_Cnf_ptr) NULL) {
        cnf = Be_ConvertToCnf(be_mgr, prob, 1);
      }

#ifdef BENCHMARKING
      fprintf(nusmv_stdout, ":START:benchmarking Solving\n");
      start_time = clock();
#endif

      /* SAT invokation */
      SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
      SatSolver_set_polarity(solver, cnf, 1,
                             SatSolver_get_permanent_group(solver));
      sat_res = SatSolver_solve_all_groups(solver);

#ifdef BENCHMARKING
      fprintf(nusmv_stdout, ":UTIME = %.4f secs.\n",
              ((double)(clock()-start_time))/CLOCKS_PER_SEC);
      fprintf(nusmv_stdout, ":STOP:benchmarking Solving\n");
#endif

      /* Processes the result: */
      switch (sat_res) {

      case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
        {
          char szLoopMsg[16]; /* for loopback part of message */
          memset(szLoopMsg, 0, sizeof(szLoopMsg));

          if (Bmc_Utils_IsAllLoopbacks(l)) {
            strncpy(szLoopMsg, "", sizeof(szLoopMsg)-1);
          }
          else if (Bmc_Utils_IsNoLoopback(l)) {
            strncpy(szLoopMsg, " and no loop", sizeof(szLoopMsg)-1);
          }
          else {
            /* loop is Natural: */
            strncpy(szLoopMsg, " and loop at ", sizeof(szLoopMsg)-1);
            strncat(szLoopMsg, szLoop, sizeof(szLoopMsg)-1-strlen(szLoopMsg));
          }

          fprintf(nusmv_stdout,
                  "-- no counterexample found with bound %d%s",
                  increasingK, szLoopMsg);
          if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
            fprintf(nusmv_stdout, " for ");
            print_spec(nusmv_stdout, ltlprop);
          }
          fprintf(nusmv_stdout, "\n");

          break;
        }

      case SAT_SOLVER_SATISFIABLE_PROBLEM:
        fprintf(nusmv_stdout, "-- ");
        print_spec(nusmv_stdout, ltlprop);
        fprintf(nusmv_stdout, "  is false\n");
        Prop_set_status(ltlprop, Prop_False);

        found_solution = true;

        if (opt_counter_examples(OptsHandler_get_instance())) {
          BoolSexpFsm_ptr bsexp_fsm; /* needed for trace language */

          bsexp_fsm = Prop_get_bool_sexp_fsm(ltlprop);
          if (BOOL_SEXP_FSM(NULL) == bsexp_fsm) {
            bsexp_fsm = \
              PropDb_master_get_bool_sexp_fsm(PropPkg_get_prop_database());
            BOOL_SEXP_FSM_CHECK_INSTANCE(bsexp_fsm);
          }

          Trace_ptr trace = \
            Bmc_Utils_generate_and_print_cntexample(be_enc,
                                                    solver,
                                                    prob, increasingK,
                                                    "BMC Counterexample",
                             SexpFsm_get_symbols_list(SEXP_FSM(bsexp_fsm)));

          Prop_set_trace(ltlprop, Trace_get_id(trace));
        }

        break;

      case SAT_SOLVER_INTERNAL_ERROR:
        internal_error("Sorry, solver answered with a fatal Internal "
                       "Failure during problem solving.\n");

      case SAT_SOLVER_TIMEOUT:
      case SAT_SOLVER_MEMOUT:
        internal_error("Sorry, solver ran out of resources and aborted "
                       "the execution.\n");

      default:
        internal_error("Bmc_GenSolveLtl: Unexpected value in sat result");

      } /* switch */

      SatSolver_destroy(solver);
    } /* must solve */

    if (cnf != (Be_Cnf_ptr) NULL) {
      Be_Cnf_Delete(cnf);
      cnf = (Be_Cnf_ptr) NULL;
    }

  } /* for all problems length */

  return 0;
}



/**Function********************************************************************

   Synopsis           [Generates DIMACS version and/or solve and INVARSPEC
   problems]

   Description [Returns 1 if solver could not be created, 0 if
   everything went smooth]

   SideEffects        []

   SeeAlso            [Bmc_GenSolvePbs]

******************************************************************************/
int Bmc_GenSolveInvar(Prop_ptr invarprop,
                      const boolean must_solve,
                      const Bmc_DumpType dump_type,
                      const char* dump_fname_template)
{
  node_ptr binvarspec;  /* Its booleanization */
  BeFsm_ptr be_fsm; /* The corresponding be fsm */
  BeEnc_ptr be_enc;
  Be_Manager_ptr be_mgr;
  Trace_ptr trace;
  Bmc_result result;

  /* Used in rewriting */
  Prop_ptr newprop = PROP(NULL);
  Prop_ptr oldprop;
  boolean was_rewritten;
  SymbLayer_ptr layer = SYMB_LAYER(NULL);
  SymbTable_ptr st;

  /* checks that a property was selected: */
  nusmv_assert(invarprop != PROP(NULL));

  /* checks if it has already been checked: */
  if (Prop_get_status(invarprop) != Prop_Unchecked) {
    /* aborts this check */
    return 0;
  }

  if (opt_cone_of_influence(OptsHandler_get_instance()) == true) {
    Prop_apply_coi_for_bmc(invarprop, global_fsm_builder);
  }

  be_fsm = Prop_get_be_fsm(invarprop);
  if (be_fsm == (BeFsm_ptr) NULL) {
    PropDb_set_fsm_to_master(PropPkg_get_prop_database(), invarprop);
    be_fsm = Prop_get_be_fsm(invarprop);
    nusmv_assert(be_fsm != (BeFsm_ptr) NULL);
  }

  /* save the original property in case of rewrite */
  oldprop = invarprop;

  st = BaseEnc_get_symb_table(BASE_ENC(BeFsm_get_be_encoding(be_fsm)));

  was_rewritten = false;
  if (Prop_needs_rewriting(invarprop)) {
    /* Create a new temporary layer */
    layer = SymbTable_get_layer(st, BMC_REWRITE_INVARSPEC_LAYER_NAME);

    if (SYMB_LAYER(NULL) == layer) {
      layer = SymbTable_create_layer(st,
                                     BMC_REWRITE_INVARSPEC_LAYER_NAME,
                                     SYMB_LAYER_POS_BOTTOM);

      /* Set the layer in artifacts calss to avoid new symbols to appear
         in traces */
      SymbTable_layer_add_to_class(st, SymbLayer_get_name(layer),
                                   ARTIFACTS_LAYERS_CLASS);
    }

    /* Rewrite the property */
    newprop = Bmc_rewrite_invar(invarprop,
                                Enc_get_bdd_encoding(),
                                layer);

    /* Assign the rewritten property to invarprop */
    invarprop = newprop;

    be_fsm = Prop_get_be_fsm(invarprop);

    /* Remember that we are performing a rewrite */
    was_rewritten = true;
  }

  /* booleanized, negated and NNFed formula: */
  binvarspec =
    Wff2Nnf(Compile_detexpr2bexpr(Enc_get_bdd_encoding(),
                                        Prop_get_expr_core(invarprop)));

  be_enc = BeFsm_get_be_encoding(be_fsm);
  be_mgr = BeEnc_get_be_manager(be_enc);

  /* generates the problem: */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "\nGenerating invariant problem\n");
  }

  /* Problem dumping: */
  if (dump_type != BMC_DUMP_NONE) {
    be_ptr prob;
    Be_Cnf_ptr cnf;

        prob = Bmc_Gen_InvarProblem(be_fsm, binvarspec);
    prob = Bmc_Utils_apply_inlining(be_mgr, prob);

    cnf = Be_ConvertToCnf(be_mgr, prob, 0);
    Bmc_Dump_WriteProblem(be_enc, cnf, invarprop,
                          1, Bmc_Utils_GetNoLoopback(),
                          dump_type, dump_fname_template);

    Be_Cnf_Delete(cnf);
  }

  if (must_solve) {
    {
      BoolSexpFsm_ptr bsexp_fsm; /* needed for trace language */

      bsexp_fsm = Prop_get_bool_sexp_fsm(invarprop);
      if (BOOL_SEXP_FSM(NULL) == bsexp_fsm) {
        bsexp_fsm = \
          PropDb_master_get_bool_sexp_fsm(PropPkg_get_prop_database());
        BOOL_SEXP_FSM_CHECK_INSTANCE(bsexp_fsm);
      }

      result = \
        Bmc_induction_algorithm(be_fsm, binvarspec, &trace,
                                SexpFsm_get_symbols_list(SEXP_FSM(bsexp_fsm)));
    }

    if (result == BMC_TRUE) {
      fprintf(nusmv_stdout, "-- ");
      print_invar(nusmv_stdout, oldprop);
      fprintf(nusmv_stdout, "  is true\n");
      Prop_set_status(invarprop, Prop_True);
    }
    else if(result == BMC_UNKNOWN) {
      fprintf(nusmv_stdout, "-- cannot prove the ");
      print_invar(nusmv_stdout, oldprop);
      fprintf(nusmv_stdout, " is true or false : the induction fails\n");

      if (opt_counter_examples(OptsHandler_get_instance())) {

        /* Print the trace using default plugin */
        fprintf(nusmv_stdout,
                "-- as demonstrated by the following execution sequence\n");

        TraceManager_register_trace(global_trace_manager, trace);
        TraceManager_execute_plugin(global_trace_manager, TRACE_OPT(NULL),
                                    TRACE_MANAGER_DEFAULT_PLUGIN,
                                    TRACE_MANAGER_LAST_TRACE);

        Prop_set_trace(invarprop, Trace_get_id(trace));

      }
    }
    else {
      /* no other handled cases */
      error_unreachable_code();
    }
  } /* must solve */

  if (was_rewritten) {
    /* Save the results in the original property */
    Prop_set_trace(oldprop, Prop_get_trace(invarprop));
    Prop_set_status(oldprop, Prop_get_status(invarprop));

    /* Perform cleanup */
    Bmc_rewrite_cleanup(newprop, Enc_get_bdd_encoding(), layer);
  }

  return 0;
}


/**Function********************************************************************

   Synopsis           [Apply Induction algorithm on th given FSM to
                       check the given NNFd invarspec]

   Description        [Returns BMC_TRUE if the property is true, BMC_UNKNOWN
                       if the induction failed, if the induction fails and the
                       counter example option is activated, then a trace is
                       registered in the global trace manager and its index is
                       stored in trace_index parameter.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
Bmc_result Bmc_induction_algorithm(BeFsm_ptr be_fsm,
                                   node_ptr binvarspec,
                                   Trace_ptr* trace,
                                   NodeList_ptr symbols)
{
  /* SAT problem solving */
  SatSolver_ptr solver;
  SatSolverResult sat_res;
  Be_Cnf_ptr cnf;
  be_ptr prob;
  int result;
  Be_Manager_ptr be_mgr;
  BeEnc_ptr be_enc;

  be_enc = BeFsm_get_be_encoding(be_fsm);
  be_mgr = BeEnc_get_be_manager(be_enc);

  prob = Bmc_Gen_InvarProblem(be_fsm, binvarspec);
  prob = Bmc_Utils_apply_inlining(be_mgr, prob);

  cnf = (Be_Cnf_ptr) NULL;

  /* Sat construction */
  solver = Sat_CreateNonIncSolver(get_sat_solver(OptsHandler_get_instance()));
  if (solver == SAT_SOLVER(NULL)) {
    fprintf(nusmv_stderr,
            "Non-incremental sat solver '%s' is not available.\n",
            get_sat_solver(OptsHandler_get_instance()));

    if (cnf != (Be_Cnf_ptr) NULL) Be_Cnf_Delete(cnf);
    return 1;
  }

  /* Cnf construction (if needed): */
  if (cnf == (Be_Cnf_ptr) NULL) {
    cnf = Be_ConvertToCnf(be_mgr, prob, 1);
  }

  /* SAT invokation */
  SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
  SatSolver_set_polarity(solver, cnf, 1,
                         SatSolver_get_permanent_group(solver));
  sat_res = SatSolver_solve_all_groups(solver);

  result = BMC_ERROR;

  /* Processes the result: */
  switch (sat_res) {

  case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
    result = BMC_TRUE;
    break;

  case SAT_SOLVER_SATISFIABLE_PROBLEM:

    if (opt_counter_examples(OptsHandler_get_instance())) {
      be_enc = BeFsm_get_be_encoding(be_fsm);
      *trace = Bmc_Utils_generate_cntexample(be_enc,
                                             solver,
                                             prob, 1,
                                             "BMC Failed Induction",
                                             symbols);
    }
    result = BMC_UNKNOWN;
    break;

  case SAT_SOLVER_INTERNAL_ERROR:
    internal_error("Sorry, solver answered with a fatal Internal "
                   "Failure during problem solving.\n");

  case SAT_SOLVER_TIMEOUT:
  case SAT_SOLVER_MEMOUT:
    internal_error("Sorry, solver ran out of resources and aborted "
                   "the execution.\n");

  default:
    internal_error("Bmc_GenSolveLtl: Unexpected value in sat result");

  } /* switch */

  SatSolver_destroy(solver);
  if (cnf != (Be_Cnf_ptr) NULL) Be_Cnf_Delete(cnf);

  return result;
}


/**Function********************************************************************

   Synopsis           [Solve and INVARSPEC problems by using
   Een/Sorensson method non-incrementally]

   Description        [Returns 1 if solver could not be created, 0 if
   everything went smooth]

   SideEffects        []

   SeeAlso            [Bmc_GenSolvePbs]

******************************************************************************/
int Bmc_GenSolveInvar_EenSorensson(Prop_ptr invarprop,
                                   const int max_k,
                                   const Bmc_DumpType dump_type,
                                   const char* dump_fname_template,
                                   boolean use_extra_step)
{
  node_ptr binvarspec;  /* Its booleanization */
  BeFsm_ptr be_fsm; /* The corresponding be fsm */

  /* Used in rewriting */
  Prop_ptr newprop = PROP(NULL);
  Prop_ptr oldprop;
  boolean was_rewritten;
  SymbLayer_ptr layer = SYMB_LAYER(NULL);
  SymbTable_ptr st;

  Bmc_result result;
  Trace_ptr trace;

  /* checks that a property was selected: */
  nusmv_assert(invarprop != PROP(NULL));

  /* checks if it has already been checked: */
  if (Prop_get_status(invarprop) != Prop_Unchecked) {
    /* aborts this check */
    return 0;
  }

  if (opt_cone_of_influence(OptsHandler_get_instance()) == true) {
    Prop_apply_coi_for_bmc(invarprop, global_fsm_builder);
  }

  be_fsm = Prop_get_be_fsm(invarprop);
  if (be_fsm == (BeFsm_ptr) NULL) {
    PropDb_set_fsm_to_master(PropPkg_get_prop_database(), invarprop);
    be_fsm = Prop_get_be_fsm(invarprop);
    nusmv_assert(be_fsm != (BeFsm_ptr) NULL);
  }

  st = BaseEnc_get_symb_table(BASE_ENC(BeFsm_get_be_encoding(be_fsm)));

  /* save the original property in case of rewrite */
  oldprop = invarprop;

  was_rewritten = false;
  if (Prop_needs_rewriting(invarprop)) {
    /* Create a new temporary layer */
    layer = SymbTable_get_layer(st, BMC_REWRITE_INVARSPEC_LAYER_NAME);

    if (SYMB_LAYER(NULL) == layer) {
      layer = SymbTable_create_layer(st,
                                     BMC_REWRITE_INVARSPEC_LAYER_NAME,
                                     SYMB_LAYER_POS_BOTTOM);

      /* Set the layer in artifacts calss to avoid new symbols to appear
         in traces */
      SymbTable_layer_add_to_class(st, SymbLayer_get_name(layer),
                                   ARTIFACTS_LAYERS_CLASS);
    }

    /* Rewrite the property */
    newprop = Bmc_rewrite_invar(invarprop,
                                Enc_get_bdd_encoding(),
                                layer);

    /* Assign the rewritten property to invarprop */
    invarprop = newprop;

    be_fsm = Prop_get_be_fsm(invarprop);

    /* Remember that we are performing a rewrite */
    was_rewritten = true;
  }

  /* booleanized, negated and NNFed formula: */
  binvarspec = Wff2Nnf(Compile_detexpr2bexpr(Enc_get_bdd_encoding(),
                                                   Prop_get_expr_core(invarprop)));

  result = Bmc_een_sorensson_algorithm(be_fsm,
                                       Prop_get_bool_sexp_fsm(invarprop),
                                       binvarspec,
                                       max_k,
                                       dump_type,
                                       dump_fname_template,
                                       invarprop,
                                       true,
                                       use_extra_step,
                                       &trace);

  switch (result) {
  case BMC_FALSE:
    fprintf(nusmv_stdout, "-- ");
    print_invar(nusmv_stdout, oldprop);
    fprintf(nusmv_stdout, "  is false\n");
    Prop_set_status(invarprop, Prop_False);

    if (opt_counter_examples(OptsHandler_get_instance())) {
      /* Print the trace using default plugin */
      fprintf(nusmv_stdout,
              "-- as demonstrated by the following execution sequence\n");

      TraceManager_register_trace(global_trace_manager, trace);
      TraceManager_execute_plugin(global_trace_manager, TRACE_OPT(NULL),
                                  TRACE_MANAGER_DEFAULT_PLUGIN,
                                  TRACE_MANAGER_LAST_TRACE);

      Prop_set_trace(invarprop, Trace_get_id(trace));
    }
  break;

  case BMC_TRUE:
    fprintf(nusmv_stdout, "-- ");
    print_invar(nusmv_stdout, oldprop);
    fprintf(nusmv_stdout, "  is true\n");
    Prop_set_status(invarprop, Prop_True);
    break;

  case BMC_UNKNOWN:
    fprintf(nusmv_stdout, "-- cannot prove the ");
    print_invar(nusmv_stdout, oldprop);
    fprintf(nusmv_stdout, " is true or false.\n");
    break;

  default:
    error_unreachable_code();
  }

  if (was_rewritten) {
    /* Save the results in the original property */
    Prop_set_trace(oldprop, Prop_get_trace(invarprop));
    Prop_set_status(oldprop, Prop_get_status(invarprop));

    /* Perform cleanup */
    Bmc_rewrite_cleanup(newprop, Enc_get_bdd_encoding(), layer);
  }

  return 0;
}


/**Function********************************************************************

   Synopsis           [Solve and INVARSPEC problems by using
   Een/Sorensson method non-incrementally and without dumping the problem]

   Description        [Returns a Bmc_result according to the result of the
                       checking]

   SideEffects        []

   SeeAlso            [Bmc_GenSolvePbs]

******************************************************************************/
Bmc_result
Bmc_een_sorensson_algorithm_without_dump(BeFsm_ptr be_fsm,
                                         BoolSexpFsm_ptr bool_fsm,
                                         node_ptr binvarspec,
                                         int max_k,
                                         boolean use_extra_step,
                                         Trace_ptr* trace) {
  return Bmc_een_sorensson_algorithm(be_fsm,
                                     bool_fsm,
                                     binvarspec,
                                     max_k,
                                     BMC_DUMP_NONE,
                                     NULL,
                                     PROP(NULL),
                                     false,
                                     use_extra_step,
                                     trace);
}

/**Function********************************************************************

   Synopsis           [Solve and INVARSPEC problems by using
   Een/Sorensson method non-incrementally]

   Description        [Returns a Bmc_result according to the result of the
                       checking]

   SideEffects        []

   SeeAlso            [Bmc_GenSolvePbs]

******************************************************************************/
Bmc_result Bmc_een_sorensson_algorithm(BeFsm_ptr be_fsm,
                                       BoolSexpFsm_ptr bool_fsm,
                                       node_ptr binvarspec,
                                       int max_k,
                                       const Bmc_DumpType dump_type,
                                       const char* dump_fname_template,
                                       Prop_ptr pp,
                                       boolean print_steps,
                                       boolean use_extra_step,
                                       Trace_ptr* trace)
{
  BeEnc_ptr be_enc;
  Be_Manager_ptr be_mgr;
  Bmc_result result = BMC_ERROR;
  be_ptr be_invarspec;
  be_ptr be_init;
  boolean solved;

  be_enc = BeFsm_get_be_encoding(be_fsm);
  be_mgr = BeEnc_get_be_manager(be_enc);

  char template_name[BMC_DUMP_FILENAME_MAXLEN];
  int k;
  lsList crnt_state_be_vars;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "\nGenerating invariant problem (Een/Sorensson)\n");
  }

  be_invarspec = Bmc_Conv_Bexp2Be(be_enc, binvarspec);
  be_init = Bmc_Model_GetInit0(be_fsm);

  k = 0;
  solved = false;

  /* retrieves the list of bool variables needed to calculate the
     state uniqueness, taking into account of coi if enabled. */
  crnt_state_be_vars =
    Bmc_Utils_get_vars_list_for_uniqueness_fsm(be_enc, (SexpFsm_ptr) bool_fsm);

  while (!solved && (k <= max_k)) {
    be_ptr be_base;
    Be_Cnf_ptr cnf;
    int i;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "\nBuilding the base for k=%d\n", k);
    }

    /* Get the unrolling (s_0,...,s_k)*/
    be_base = Bmc_Model_GetUnrolling(be_fsm, 0, k);
    /* Set the initial condition to hold in s_0 */
    be_base = Be_And(be_mgr, be_base, be_init);
    /* The invariant property should be true in all s_0...s_{k-1}*/
    for (i = 0; i < k; i++) {
      be_base = Be_And(be_mgr, be_base,
                       BeEnc_untimed_expr_to_timed(be_enc, be_invarspec, i));
    }

    /* The invariant property should be violated in s_k */
    be_base = Be_And(be_mgr, be_base,
                     Be_Not(be_mgr,
                            BeEnc_untimed_expr_to_timed(be_enc,
                                                        be_invarspec, k)));
    be_base = Bmc_Utils_apply_inlining(be_mgr, be_base);

    /* Problem is cnf-ed */
    cnf = (Be_Cnf_ptr) NULL;

    /* Problem dumping: */
    if (dump_type != BMC_DUMP_NONE) {
      cnf = Be_ConvertToCnf(be_mgr, be_base, 1);

      strncpy(template_name, dump_fname_template, sizeof(template_name)-2);
      template_name[sizeof(template_name)-1] = '\0'; /* terms the string */
      strncat(template_name, "_base",
              sizeof(template_name) - strlen(template_name) - 1);
      template_name[sizeof(template_name)-1] = '\0'; /* terms the string */

      Bmc_Dump_WriteProblem(be_enc, cnf, pp,
                            1, Bmc_Utils_GetNoLoopback(),
                            dump_type, template_name);
    }

    /* SAT problem solving */
    {
      SatSolver_ptr solver;
      SatSolverResult sat_res;

      /* Sat construction */
      solver = Sat_CreateNonIncSolver(get_sat_solver(OptsHandler_get_instance()));
      if (solver == SAT_SOLVER(NULL)) {
        fprintf(nusmv_stderr,
                "Non-incremental sat solver '%s' is not available.\n",
                get_sat_solver(OptsHandler_get_instance()));

        if (cnf != (Be_Cnf_ptr) NULL) Be_Cnf_Delete(cnf);
        return BMC_ERROR;
      }

      /* Cnf construction (if needed): */
      if (cnf == (Be_Cnf_ptr) NULL) {
        cnf = Be_ConvertToCnf(be_mgr, be_base, 1);
      }

      /* SAT invokation */
      SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
      SatSolver_set_polarity(solver, cnf, 1,
                             SatSolver_get_permanent_group(solver));
      sat_res = SatSolver_solve_all_groups(solver);

      /* Processes the result: */
      switch (sat_res) {

      case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
        /* continue the loop */
        break;

      case SAT_SOLVER_SATISFIABLE_PROBLEM:
        result = BMC_FALSE;
        solved = true;

        if (opt_counter_examples(OptsHandler_get_instance())) {

      /* [MP] bool sexp fsm from the caller. */
          *trace = \
            Bmc_Utils_generate_cntexample(be_enc, solver,
                                          be_base, k, "BMC Counterexample",
                                  SexpFsm_get_symbols_list(SEXP_FSM(bool_fsm)));
        }
        break;

      case SAT_SOLVER_INTERNAL_ERROR:
        internal_error("Sorry, solver answered with a fatal Internal "
                       "Failure during problem solving.\n");

      case SAT_SOLVER_TIMEOUT:
      case SAT_SOLVER_MEMOUT:
        internal_error("Sorry, solver ran out of resources and aborted "
                       "the execution.\n");

      default:
        internal_error("Bmc_GenSolveLtl: Unexpected value in sat result");

      } /* switch */

      SatSolver_destroy(solver);
    } /* solving */

    /* base cnf no longer useful here */
    if (cnf != (Be_Cnf_ptr) NULL) {
      Be_Cnf_Delete(cnf);
      cnf = (Be_Cnf_ptr) NULL;
    }

    /* induction step */
    if (!solved) {
      /*
         steps[0]: P(S0) ^ ... ^ P(Si-1) ^ !P(Si) ^ path(0..i) ^
         loopFree(0..i)

         steps[1]: Init(S0) ^ !Init(S1) ^ ... ^ !Init(Si) ^ path(0..i)
         ^ loopFree(0..i)
      */
      be_ptr be_steps[3] = {(be_ptr)NULL, (be_ptr)NULL, (be_ptr)NULL};
      be_ptr be_unique;
      int j;

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
        fprintf(nusmv_stderr, "\nBuilding the step for k=%d\n", k);
      }

      /* Get the unrolling (s_0,...,s_k)*/
      be_steps[0] = Bmc_Model_GetUnrolling(be_fsm, 0, k);

      if (use_extra_step) {
        /* The same for the extra step, plus the initial states @0 */
        be_steps[1] = Be_And(be_mgr, be_steps[0], be_init);
      }

      /* The invariant property should be true in all s_0...s_{k-1}*/
      for (i = 0; i < k; i++) {
        be_steps[0] = Be_And(be_mgr,
                             be_steps[0],
                             BeEnc_untimed_expr_to_timed(be_enc,
                                                         be_invarspec, i));

        if (use_extra_step) {
          be_steps[1] = Be_And(be_mgr, be_steps[1],
                               Be_Not(be_mgr,
                                      Bmc_Model_GetInitI(be_fsm, i + 1)));
        }
      }

      /* The invariant property should be violated in s_k */
      be_steps[0] = Be_And(be_mgr, be_steps[0],
                           Be_Not(be_mgr,
                                  BeEnc_untimed_expr_to_timed(be_enc,
                                                              be_invarspec,
                                                              k)));

      /* All states s_0,...,s_{k-1} should be different.
       * Insert and force to true s_j != s_i for each 0 <= j < i <= k-1
       * in frame 0 */
      be_unique = Be_Truth(be_mgr);
      for (i = 0; i < k ; i++) {
        for (j = 0; j < i; j++) {
          be_ptr not_equal = Be_Falsity(be_mgr);
          be_ptr be_var;
          lsGen gen;

          lsForEachItem(crnt_state_be_vars, gen, be_var) {
            be_ptr be_xor = Be_Xor(be_mgr,
                                   BeEnc_untimed_expr_to_timed(be_enc, be_var, i),
                                   BeEnc_untimed_expr_to_timed(be_enc, be_var, j));
            not_equal = Be_Or(be_mgr, not_equal, be_xor);
          }

          be_unique = Be_And(be_mgr, be_unique, not_equal);
        }
      } /* for i */
      be_steps[0] = Be_And(be_mgr, be_steps[0], be_unique);
      be_steps[0] = Bmc_Utils_apply_inlining(be_mgr, be_steps[0]);

      if (use_extra_step) {
        be_steps[1] = Be_And(be_mgr, be_steps[1], be_unique);
        be_steps[1] = Bmc_Utils_apply_inlining(be_mgr, be_steps[1]);
      }

      nusmv_assert(use_extra_step || (be_ptr)NULL == be_steps[1]);

      /* SAT problem solving */
      for (i = 0; ((be_ptr)NULL != be_steps[i]) && !solved; ++i) {
        SatSolver_ptr solver;
        SatSolverResult sat_res;

        /* Problem dumping: */
        if (dump_type != BMC_DUMP_NONE) {
          nusmv_assert((Be_Cnf_ptr)NULL == cnf);

          cnf = Be_ConvertToCnf(be_mgr, be_steps[i], 1);

          strncpy(template_name, dump_fname_template, sizeof(template_name)-2);
          template_name[sizeof(template_name)-1] = '\0'; /* terms the string */
          strncat(template_name, "_step",
                  sizeof(template_name) - strlen(template_name) - 1);
          template_name[sizeof(template_name)-1] = '\0'; /* terms the string */

          Bmc_Dump_WriteProblem(be_enc, cnf, pp,
                                1, Bmc_Utils_GetNoLoopback(),
                                dump_type, template_name);
        }


        /* Sat construction */
        solver = Sat_CreateNonIncSolver(get_sat_solver(OptsHandler_get_instance()));
        if (solver == SAT_SOLVER(NULL)) {
          fprintf(nusmv_stderr,
                  "Non-incremental sat solver '%s' is not available.\n",
                  get_sat_solver(OptsHandler_get_instance()));

          if (cnf != (Be_Cnf_ptr) NULL) Be_Cnf_Delete(cnf);
          return BMC_ERROR;
        }

        /* Cnf construction (if needed): */
        if (cnf == (Be_Cnf_ptr) NULL) {
          cnf = Be_ConvertToCnf(be_mgr, be_steps[i], 1);
        }

        /* SAT invokation */
        SatSolver_add(solver, cnf, SatSolver_get_permanent_group(solver));
        SatSolver_set_polarity(solver, cnf, 1,
                               SatSolver_get_permanent_group(solver));
        sat_res = SatSolver_solve_all_groups(solver);

        /* Processes the result: */
        switch (sat_res) {

        case SAT_SOLVER_UNSATISFIABLE_PROBLEM:
          result = BMC_TRUE;
          solved = true;
          break;

        case SAT_SOLVER_SATISFIABLE_PROBLEM:
          if (print_steps) {
            /* Prints out the current state of solving, and continues
               the loop */
            fprintf(nusmv_stdout,
                    "-- no proof or counterexample found with bound %d", k);
            if ((PROP(NULL) != pp) && opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
              fprintf(nusmv_stdout, " for ");
              print_invar(nusmv_stdout, pp);
            }
            fprintf(nusmv_stdout, "\n");
          }
          break;

        case SAT_SOLVER_INTERNAL_ERROR:
          internal_error("Sorry, solver answered with a fatal Internal "
                         "Failure during problem solving.\n");

        case SAT_SOLVER_TIMEOUT:
        case SAT_SOLVER_MEMOUT:
          internal_error("Sorry, solver ran out of resources and aborted "
                         "the execution.\n");

        default:
          internal_error("Bmc_GenSolveLtl: Unexpected value in sat result");

        } /* switch */

        if (cnf != (Be_Cnf_ptr) NULL) {
          Be_Cnf_Delete(cnf);
          cnf = (Be_Cnf_ptr) NULL;
        }

        SatSolver_destroy(solver);
      } /* solving */


        /* base cnf no longer useful here */
      nusmv_assert((Be_Cnf_ptr)NULL == cnf);

    } /* induction step */

    k = k + 1;
  } /* while !solved */

  lsDestroy(crnt_state_be_vars, NULL);

  if (!solved) {
    return BMC_UNKNOWN;
  }
  else {
    return result;
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/
