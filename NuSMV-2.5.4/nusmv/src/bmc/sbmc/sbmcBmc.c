/**CFile***********************************************************************

  FileName [sbmcBmc.c]

  PackageName [bmc.sbmc]

  Synopsis [High level functionalities for SBMC]

  Description [User-commands directly use function defined in this module. 
  This is the highest level in the SBMC API architecture. 

  For further information about this implementation see:
  T. Latvala, A. Biere, K. Heljanko, and T. Junttila. Simple is
  Better: Efficient Bounded Model Checking for Past LTL. In: R. Cousot
  (ed.), Verification, Model Checking, and Abstract Interpretation,
  6th International Conference VMCAI 2005, Paris, France, Volume 3385
  of LNCS, pp. 380-395, Springer, 2005.  Copyright ©
  Springer-Verlag. 
  ]

  SeeAlso  []

  Author   [Timo Latvala, Marco Roveri]

  Copyright [ This file is part of the ``bmc.sbmc'' package of NuSMV version 2.
  Copyright (C) 2004 Timo Latvala <timo.latvala@tkk.fi>

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

  For more information of NuSMV see <http://nusmv.fbk.eu> or email
  to <nusmv-users@fbk.eu>.  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#include "sbmcBmc.h"
#include "sbmcGen.h"
#include "sbmcUtils.h"

#include "bmc/bmcInt.h"

#include "bmc/bmcBmc.h"
#include "bmc/bmcDump.h"
#include "bmc/bmcModel.h"
#include "wff/wff.h"
#include "wff/w2w/w2w.h"
#include "bmc/bmcConv.h"
#include "bmc/bmcUtils.h"

#include "node/node.h"
#include "be/be.h"
#include "enc/enc.h"
#include "enc/be/BeEnc.h"

#include "sat/sat.h" /* for solver and result */
#include "sat/SatSolver.h"
#include "sat/SatIncSolver.h"

#include "mc/mc.h" /* for print_spec */

#include "prop/Prop.h"
#include "prop/propPkg.h"

#include "utils/error.h"


#ifdef BENCHMARKING
  #include <time.h>
  clock_t start_time;
#endif


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


static void 
bmc_expandFilename ARGS((const int k, const int l,
                         const int prop_idx,
                         const char* filename_to_be_expanded,
                         char* filename_expanded,
                         const size_t filename_expanded_maxlen));


/**AutomaticEnd***************************************************************/


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
  Also see the Bmc_GenSolve_Action possible values]

  Description        []

  SideEffects        []

  SeeAlso            [Bmc_GenSolve_Action]

******************************************************************************/
int Bmc_SBMCGenSolveLtl(Prop_ptr ltlprop, 
                        const int k, const int relative_loop,
                        const boolean must_inc_length,
                        const boolean must_solve, 
                        const Bmc_DumpType dump_type,
                        const char* dump_fname_template)
{
  node_ptr bltlspec;  /* Its booleanization */
  BeFsm_ptr be_fsm = BE_FSM(NULL); /* The corresponding be fsm  */
  BeEnc_ptr be_enc;
  Be_Manager_ptr be_mgr;

  /* ----------------------------------------------------------------------*/
  /* Here a property was selected                                          */
  /* ----------------------------------------------------------------------*/
  int k_max = k;
  int k_min = 0;
  int increasingK;
  int found_solution;

  /* checks that a property was selected: */
  nusmv_assert(ltlprop != (Prop_ptr)NULL);

  /* checks if it has been already checked: */
  if (Prop_get_status(ltlprop) != Prop_Unchecked) {
    /* aborts this check */
    return 0;
  }

  found_solution = false;

  if (must_inc_length == false) k_min = k_max;

  be_fsm = Prop_compute_ground_be_fsm(ltlprop, global_fsm_builder);
  BE_FSM_CHECK_INSTANCE(be_fsm);

  be_enc = BeFsm_get_be_encoding(be_fsm);
  be_mgr = BeEnc_get_be_manager(be_enc);

  sbmc_add_loop_variable(be_fsm);

  /* Start problems generations: */
  for (increasingK = k_min; (increasingK <= k_max) && !found_solution;
      ++increasingK) {
    int l;
    char szLoop[16]; /* to keep loopback string */
    be_ptr prob; /* The problem in BE format */
    Be_Cnf_ptr cnf; /* The CNFed be problem */

    /* the loopback value could be depending on the length
       if it were relative: */
    l = Bmc_Utils_RelLoop2AbsLoop(relative_loop, increasingK);

    /* prints a verbose message: */
    Bmc_Utils_ConvertLoopFromInteger(relative_loop, szLoop, sizeof(szLoop)); 

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
        if ((l < increasingK) && (l>=0)) {
          fprintf(nusmv_stderr,
                  "\nGenerating problem with bound %d, loopback %s...\n",
                  increasingK, szLoop);
        }
      }
    } /* verbose messages */

    /* checks for loopback vs k compatibility */
    if (Bmc_Utils_IsSingleLoopback(l) && ((l >= increasingK) || (l<0))) {
      fprintf(nusmv_stderr,
              "\nWarning: problem with bound %d and loopback %s is not allowed: skipped\n",
              increasingK, szLoop);
      continue;
    }

#ifdef BENCHMARKING
    fprintf(nusmv_stderr,":START:benchmarking bound\n");
    fprintf(nusmv_stderr,":bound %d\n", increasingK);
    fprintf(nusmv_stderr,":STOP:benchmarking bound\n");
    fprintf(nusmv_stdout,":START:benchmarking Generation\n");
    start_time = clock();
#endif

    /* generates the problem: */
    bltlspec = sbmc_make_boolean_formula(ltlprop);
    prob = Bmc_Gen_SBMCProblem(be_fsm, bltlspec, increasingK, l);
    prob = Bmc_Utils_apply_inlining(be_mgr, prob);

#ifdef BENCHMARKING
    fprintf(nusmv_stdout,":UTIME = %.4f secs.\n",((double)(clock()-start_time))/CLOCKS_PER_SEC);
    fprintf(nusmv_stdout,":STOP:benchmarking Generation\n");
#endif
    
    cnf = (Be_Cnf_ptr) NULL;

    /* Problem dumping: */
    if (dump_type != BMC_DUMP_NONE) {
      cnf = Be_ConvertToCnf(be_mgr, prob, 0);
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
        sbmc_remove_loop_variable(be_fsm);
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
        sbmc_remove_loop_variable(be_fsm);
        internal_error("Sorry, solver answered with a fatal Internal "
                       "Failure during problem solving.\n");

      case SAT_SOLVER_TIMEOUT:
      case SAT_SOLVER_MEMOUT:
        sbmc_remove_loop_variable(be_fsm);
        internal_error("Sorry, solver ran out of resources and aborted "
                       "the execution.\n");

      default:
        sbmc_remove_loop_variable(be_fsm);
        internal_error("Bmc_GenSolveLtl: Unexpected value in sat result");
        
      } /* switch */

      SatSolver_destroy(solver);
    } /* must solve */

    if (cnf != (Be_Cnf_ptr) NULL) {
      Be_Cnf_Delete(cnf); 
      cnf = (Be_Cnf_ptr) NULL;
    }

  } /* for all problems length */
  sbmc_remove_loop_variable(be_fsm);
  return 0;
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [This is only a useful wrapper for easily call
  Bmc_Utils_ExpandMacrosInFilename]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static void
bmc_expandFilename(const int k, const int l,
                   const int prop_idx,
                   const char* filename_to_be_expanded,
                   char* filename_expanded,
                   const size_t filename_expanded_maxlen)
{
  char szBuffer[1024];
  char szLoopback[16];

  /* Prepares the structure for macro-expansion: */
  SubstString aSubstTable[] =  { SYMBOL_CREATE(),
         SYMBOL_CREATE(),
         SYMBOL_CREATE(),
         SYMBOL_CREATE(),
         SYMBOL_CREATE(),
         SYMBOL_CREATE()
  };

  /* customizes the table with runtime values: */
  Utils_StripPathNoExtension(get_input_file(OptsHandler_get_instance()), szBuffer);
  Bmc_Utils_ConvertLoopFromInteger(l, szLoopback, sizeof(szLoopback));

  SYMBOL_ASSIGN(aSubstTable[0], "@F", string,  "%s", get_input_file(OptsHandler_get_instance()));
  SYMBOL_ASSIGN(aSubstTable[1], "@f", string,  "%s", szBuffer);
  SYMBOL_ASSIGN(aSubstTable[2], "@k", integer, "%d", k);
  SYMBOL_ASSIGN(aSubstTable[3], "@l", string, "%s", szLoopback);
  if (prop_idx != BMC_NO_PROPERTY_INDEX) {
    SYMBOL_ASSIGN(aSubstTable[4], "@n", integer, "%d", prop_idx);
  }
  else {
    SYMBOL_ASSIGN(aSubstTable[4], "@n", string, "%s", "undef");
  }
  SYMBOL_ASSIGN(aSubstTable[5], "@@", string,  "%s", "@");

  Bmc_Utils_ExpandMacrosInFilename(filename_to_be_expanded,
           aSubstTable,
           sizeof(aSubstTable)/sizeof(aSubstTable[0]),
           filename_expanded,
           filename_expanded_maxlen);
}

