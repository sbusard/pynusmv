/**CFile***********************************************************************

  FileName    [bmcCmd.c]

  PackageName [bmc]

  Synopsis    [Bmc.Cmd module]

  Description [This module contains all the bmc commands implementation.
  Options parsing and checking is performed here, than the high-level Bmc
  layer is called]

  SeeAlso     [bmcPkg.c, bmcBmc.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst and University of Trento.

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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "bmcCmd.h"

#include "bmc.h"
#include "bmcInt.h"
#include "bmcBmc.h"
#include "bmcPkg.h"
#include "bmcUtils.h"
#include "bmcSimulate.h"
#include "simulate/simulate.h"

#include "prop/propPkg.h"
#include "enc/enc.h"

#include "trace/exec/SATCompleteTraceExecutor.h"
#include "trace/exec/SATPartialTraceExecutor.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcCmd.c,v 1.85.4.5.2.16.2.6.6.20 2010-02-17 14:51:35 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
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

static int UsageBmcSetup    ARGS((void));
static int UsageBmcSimulate ARGS((void));
static int UsageBmcIncSimulate ARGS((void));

static int UsageBmcGenLtlSpec         ARGS((void));
static int UsageBmcGenLtlSpecOnePb    ARGS((void));
static int UsageBmcCheckLtlSpec       ARGS((void));
static int UsageBmcCheckLtlSpecOnePb  ARGS((void));
static int UsageBmcGenInvar           ARGS((void));
static int UsageBmcCheckInvar         ARGS((void));

#if NUSMV_HAVE_INCREMENTAL_SAT
static int UsageBmcCheckLtlSpecInc    ARGS((void));
static int UsageBmcCheckInvarInc      ARGS((void));
#endif

static int UsageBmcSimulateCheckFeasibleConstraints ARGS((void));
static int UsageBmcPickState ARGS((void));

static void bmc_build_master_be_fsm ARGS((void));

/**AutomaticEnd***************************************************************/



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Initializes the bmc sub-system, and builds the model in
  a Boolean Expression format]

  Description        []

  SideEffects        [Overall the bmc system]

  SeeAlso            []

  CommandName        [bmc_setup]

  CommandSynopsis    [Builds the model in a Boolean Epression format.]

  CommandArguments   [\[-h\] | \[-f\]]

  CommandDescription [You must call this command before use any other
  bmc-related command. Only one call per session is required.<BR>
  Command options:<p>
  <dl>
    <dt> <tt>-f </tt>
    <dd> Forces the BMC model to be built.
  </dl>]

******************************************************************************/
int Bmc_CommandBmcSetup(int argc, char** argv)
{
  /* processes the command options */
  int c;
  boolean forced = false;

  util_getopt_reset();
  while ((c = util_getopt((int)argc, (char**)argv, "hf")) != EOF) {
    switch (c) {
    case 'h': return UsageBmcSetup();
    case 'f': forced = true; break;
    default: return UsageBmcSetup();
    }
  }

  if (Compile_check_if_bool_model_was_built(nusmv_stderr, forced)) return 1;

  if (cmp_struct_get_bmc_setup(cmps) && !forced) {
    fprintf (nusmv_stderr, "A call to bmc_setup has already been done.\n");
    return 1;
  }

  /* Does the actual work */

  Bmc_Init(); /* encoding and layers */

  /* constructs the model only if coi is not enabled */
  if (opt_cone_of_influence(OptsHandler_get_instance()) && !forced) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr,
              "Construction of BE model is delayed due to use of COI\n");
    }
    return 0;
  }

  bmc_build_master_be_fsm();

  { /* register SAT based complete and partial executors */
    BeFsm_ptr fsm = PropDb_master_get_be_fsm(PropPkg_get_prop_database());
    BeEnc_ptr enc = BeFsm_get_be_encoding(fsm);
    BddEnc_ptr bdd_enc = Enc_get_bdd_encoding();

    TraceManager_register_complete_trace_executor(
                 TracePkg_get_global_trace_manager(),
                 "sat", "SAT complete trace execution",
                 COMPLETE_TRACE_EXECUTOR(
                     SATCompleteTraceExecutor_create(fsm, enc, bdd_enc)));

    TraceManager_register_partial_trace_executor(
                 TracePkg_get_global_trace_manager(),
                 "sat", "SAT partial trace execution (no restart)",
                 PARTIAL_TRACE_EXECUTOR(
                     SATPartialTraceExecutor_create(fsm, enc, bdd_enc, false)));

    TraceManager_register_partial_trace_executor(
                 TracePkg_get_global_trace_manager(),
                 "sat_r", "SAT partial trace execution (restart)",
                 PARTIAL_TRACE_EXECUTOR(
                     SATPartialTraceExecutor_create(fsm, enc, bdd_enc, true)));
  }

  cmp_struct_set_bmc_setup(cmps);
  return 0;
}


/**Function********************************************************************

  Synopsis           [Usage string for Bmc_CommandBmcSetup]

  Description        []

  SideEffects        []

  SeeAlso            [Bmc_CommandBmcSetup]
******************************************************************************/
static int UsageBmcSetup (void)
{
  fprintf (nusmv_stderr, "usage: bmc_setup [-h][-f]\n");
  fprintf (nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf (nusmv_stderr, "  -f \t\tForces the BMC model to be built.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Bmc_CommandBmcSimulate generates a trace of the problem
  represented from the simple path from 0 (zero) to k]

  Description        [Bmc_CommandBmcSimulate does not require a specification
  to build the problem, because only the model is used to build it.]

  SideEffects        [None]

  SeeAlso            []

  CommandName        [bmc_simulate]

  CommandSynopsis    [Generates a trace of the model from 0 (zero) to k]

  CommandArguments   [\[-h\] \[-p | -v\] \[-r\]
  [\[-c "constraints"\] | \[-t "constraints"\] ] \[-k steps\]
  ]

  CommandDescription [bmc_simulate does not require a specification
  to build the problem, because only the model is used to build it.
  The problem length is represented by the <i>-k</i> command parameter,
  or by its default value stored in the environment variable
  <i>bmc_length</i>.<BR>
  Command Options:<p>
  <dl>
    <dt> <tt>-p</tt>
       <dd> Prints current generated trace (only those variables whose value
       changed from the previous state).
    <dt> <tt>-v</tt>
       <dd> Verbosely prints current generated trace (changed and unchanged
       state variables).
    <dt> <tt>-r</tt>
       <dd> Picks a state from a set of possible future states in a random way.
    <dt> <tt>-c "constraints"</tt>
       <dd> Performs a simulation in which computation is restricted
       to states satisfying those <tt>constraints</tt>. The desired
       sequence of states could not exist if such constraints were too
       strong or it may happen that at some point of the simulation a
       future state satisfying those constraints doesn't exist: in
       that case a trace with a number of states less than
       <tt>steps</tt> trace is obtained. The expression cannot contain
       next operators, and is automatically shifted by one state in
       order to constraint only the next steps
    <dt> <tt>-t "constraints"</tt>
       <dd> Performs a simulation in which computation is restricted
       to states satisfying those <tt>constraints</tt>. The desired
       sequence of states could not exist if such constraints were too
       strong or it may happen that at some point of the simulation a
       future state satisfying those constraints doesn't exist: in
       that case a trace with a number of states less than
       <tt>steps</tt> trace is obtained.  The expression can contain
       next operators, and is NOT automatically shifted by one state
       as done with option -c
    <dt> <tt>-k steps</tt>
       <dd> Maximum length of the path according to the constraints.
       The length of a trace could contain less than <tt>steps</tt> states:
       this is the case in which simulation stops in an intermediate
       step because it may not exist any future state satisfying those
       constraints.
    </dl>
  ]

******************************************************************************/
int Bmc_CommandBmcSimulate(int argc, char** argv)
{
  BddEnc_ptr bdd_enc;
  BeEnc_ptr be_enc;
  Be_Manager_ptr be_mgr;
  be_ptr be_constraints = (be_ptr) NULL;
  boolean isconstraint = false;
  boolean printrace = false;
  int display_all = 0;
  int c = 0;
  boolean only_changes = 1;
  boolean time_shift = false;
  int steps = get_default_simulation_steps(OptsHandler_get_instance());
  Simulation_Mode mode = Deterministic;
  boolean k_specified = false;

  /* the string of constraint to parsificate */
  char* strConstr = NIL(char);

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"t:c:hpvrk:")) != EOF){
    switch(c){
    case 'h': return UsageBmcSimulate();
    case 'p':
      if (printrace == true) return UsageBmcSimulate();
      printrace = true;
      only_changes = true;
      break;
    case 'v':
      if (printrace == true) return UsageBmcSimulate();
      printrace = true;
      only_changes = false;
      break;
    case 'r':
      if (mode == Interactive) return UsageBmcSimulate();
      mode = Random;
      break;

      /* NOTE: Uncomment this if adding support for interactive
         mode. remember to add "ai" to the util_getopt format
         parameter */
    /* case 'i': */
    /*   if (mode == Random) return UsageBmcSimulate(); */
    /*   mode = Interactive; */
    /*   break; */
    /* case 'a': */
    /*   display_all = 1; */
    /*   break; */
    case 'c':
      if (NIL(char) != strConstr) return UsageBmcSimulate();
      strConstr = util_strsav(util_optarg);
      isconstraint = true;
      time_shift = true;
      break;
    case 't':
      if (NIL(char) != strConstr) return UsageBmcSimulate();
      strConstr = util_strsav(util_optarg);
      isconstraint = true;
      time_shift = false;
      break;

    case 'k':
      {
        char* strNumber;

        if (k_specified) {
          fprintf(nusmv_stderr,
                  "Option -k cannot be specified more than once.\n");
          return 1;
        }

        strNumber = util_strsav(util_optarg);

        if (util_str2int(strNumber, &steps) != 0) {
          error_invalid_number(strNumber);
          FREE(strNumber);
          return 1;
        }

        if (steps < 0) {
           error_invalid_number(strNumber);
           FREE(strNumber);
          return 1;
        }

        FREE(strNumber);
        k_specified = true;
        break;
      }

    default:
      return UsageBmcSimulate();
    }
  }

  if ((mode != Interactive) && (display_all == 1)) return UsageBmcSimulate();

  if (argc != util_optind) {
    return UsageBmcSimulate();
  }

  /* pre-conditions */
  if (Bmc_check_if_model_was_built(nusmv_stderr, true)) return 1;

  if (bmc_simulate_get_curr_sim_trace() == TRACE(NULL)) {
    fprintf(nusmv_stderr,
            "No current state set. Use the \"bmc_pick_state\" command.\n");
    return 1;
  }

  bdd_enc = Enc_get_bdd_encoding();
  be_enc = Enc_get_be_encoding();
  be_mgr = BeEnc_get_be_manager(be_enc);

  if (isconstraint) {
    if (time_shift) {
      be_constraints = Bmc_Utils_simple_costraint_from_string(be_enc,
                                                              bdd_enc,
                                                              strConstr,
                                                              (Expr_ptr*)NULL);
    }
    else {
      be_constraints = Bmc_Utils_next_costraint_from_string(be_enc,
                                                            bdd_enc,
                                                            strConstr,
                                                            (Expr_ptr*)NULL);
    }

    FREE(strConstr);
  }
  else {
    be_constraints = Be_Truth(be_mgr);
  }

  {
    /* This function does the actual work: */
    Bmc_Simulate(PropDb_master_get_be_fsm(PropPkg_get_prop_database()),
                 bdd_enc, be_constraints, time_shift,
                 steps, printrace, only_changes, mode == Random);
  }

  return 0;
}


/**Function********************************************************************

  Synopsis           [Usage string for UsageBmcSimulate]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandBmcSimulate]

******************************************************************************/
static int UsageBmcSimulate (void)
{
  /* fprintf(nusmv_stderr, */
  /*         "usage: bmc_simulate [-h] [-p | -v] [-r | -i [-a]] [[-c \"constr\"] " */
  /*         "| [-t \"constr\"]] [-k steps]\n"); */
  fprintf(nusmv_stderr,
          "usage: bmc_simulate [-h] [-p | -v] [-r] [[-c \"constr\"] "
          "| [-t \"constr\"]] [-k steps]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -p \t\tPrints current generated trace (only changed variables).\n");
  fprintf(nusmv_stderr, "  -v \t\tVerbosely prints current generated trace (all variables).\n");
  fprintf(nusmv_stderr, "  -r \t\tSets picking mode to random (default is deterministic).\n");
  /* fprintf(nusmv_stderr, "  -i \t\tEnters simulation's interactive mode.\n"); */
  /* fprintf(nusmv_stderr, "  -a \t\tDisplays all the state variables (changed and unchanged)\n"); */
  fprintf(nusmv_stderr, "     \t\tin every step of an interactive session.\n");
  fprintf(nusmv_stderr, "     \t\tIt works only together with -i option.\n");
  fprintf(nusmv_stderr, "  -c \"constr\"\tSets constraint (simple expression) for the next steps.\n");
  fprintf(nusmv_stderr, "  -t \"constr\"\tSets constraint (next expression) for the next steps.\n");
  fprintf(nusmv_stderr, "  -k <length> \tSpecifies the simulation length\n"
          "\t\tto be used when generating the simulated problem.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Bmc_CommandBmcIncSimulate does incremental
  simulation of the model starting from an initial state.]

  Description        [Bmc_CommandBmcIncSimulate does incremental
  simulation of the model starting from an initial state.]

  SideEffects        [None]

  SeeAlso            []

  CommandName        [bmc_inc_simulate]

  CommandSynopsis    [Incrementally generates a trace of the model
  performing a given number of steps.]

  CommandArguments   [\[-h\] \[-p | -v\] \[-r\]
  [\[-c "constraints"\] | \[-t "constraints"\] ] \[-k steps\]
  ]

  CommandDescription [bmc_inc_simulate performs incremental simulation
  of the model. If no length is specified with <i>-k</i> command
  parameter, then the number of steps of simulation to perform is
  taken from the value stored in the environment variable
  <i>bmc_length</i>.<BR>
  Command Options:<p>
  <dl>
    <dt> <tt>-p</tt>
       <dd> Prints current generated trace (only those variables whose value
       changed from the previous state).
    <dt> <tt>-v</tt>
       <dd> Verbosely prints current generated trace (changed and unchanged
       state variables).
    <dt> <tt>-r</tt>
       <dd> Picks a state from a set of possible future states in a random way.
    <dt> <tt>-i</tt>
       <dd> Enters simulation's interactive mode.
    <dt> <tt>-a</tt>
       <dd> Displays all the state variables (changed and unchanged)
            in the interactive session
    <dt> <tt>-c "constraints"</tt>
       <dd> Performs a simulation in which computation is restricted
       to states satisfying those <tt>constraints</tt>. The desired
       sequence of states could not exist if such constraints were too
       strong or it may happen that at some point of the simulation a
       future state satisfying those constraints doesn't exist: in
       that case a trace with a number of states less than
       <tt>steps</tt> trace is obtained. The expression cannot contain
       next operators, and is automatically shifted by one state in
       order to constraint only the next steps
    <dt> <tt>-t "constraints"</tt>
       <dd> Performs a simulation in which computation is restricted
       to states satisfying those <tt>constraints</tt>. The desired
       sequence of states could not exist if such constraints were too
       strong or it may happen that at some point of the simulation a
       future state satisfying those constraints doesn't exist: in
       that case a trace with a number of states less than
       <tt>steps</tt> trace is obtained.  The expression can contain
       next operators, and is NOT automatically shifted by one state
       as done with option -c
    <dt> <tt>-k steps</tt>
       <dd> Maximum length of the path according to the constraints.
       The length of a trace could contain less than <tt>steps</tt> states:
       this is the case in which simulation stops in an intermediate
       step because it may not exist any future state satisfying those
       constraints.
    </dl>
  ]

******************************************************************************/
int Bmc_CommandBmcIncSimulate(int argc, char** argv)
{
  BddEnc_ptr bdd_enc;
  BeEnc_ptr be_enc;
  Be_Manager_ptr be_mgr;
  be_ptr be_constraints = (be_ptr) NULL;
  boolean isconstraint = false;
  boolean printrace = false;
  int display_all = 0;
  int c = 0;
  boolean only_changes = 1;
  boolean time_shift = false;
  int steps = get_default_simulation_steps(OptsHandler_get_instance());
  Simulation_Mode mode = Deterministic;
  boolean k_specified = false;

  /* the string of constraint to parsificate */
  char* strConstr = NIL(char);

  util_getopt_reset();

  while((c = util_getopt(argc,argv,"t:c:hpvrk:ia")) != EOF){
    switch(c){
    case 'h': return UsageBmcIncSimulate();
    case 'p':
      if (printrace == true) return UsageBmcIncSimulate();
      printrace = true;
      only_changes = true;
      break;
    case 'v':
      if (printrace == true) return UsageBmcIncSimulate();
      printrace = true;
      only_changes = false;
      break;
    case 'r':
      if (mode == Interactive) return UsageBmcIncSimulate();
      mode = Random;
      break;

    case 'i':
      if (mode == Random) return UsageBmcIncSimulate();
      mode = Interactive;
      break;
    case 'a':
      display_all = 1;
      break;
    case 'c':
      if (NIL(char) != strConstr) return UsageBmcIncSimulate();
      strConstr = util_strsav(util_optarg);
      isconstraint = true;
      time_shift = true;
      break;
    case 't':
      if (NIL(char) != strConstr) return UsageBmcIncSimulate();
      strConstr = util_strsav(util_optarg);
      isconstraint = true;
      time_shift = false;
      break;

    case 'k':
      {
        char* strNumber;

        if (k_specified) {
          fprintf(nusmv_stderr,
                  "Option -k cannot be specified more than once.\n");
          return 1;
        }

        strNumber = util_strsav(util_optarg);

        if (util_str2int(strNumber, &steps) != 0) {
          error_invalid_number(strNumber);
          FREE(strNumber);
          return 1;
        }

        if (steps < 0) {
           error_invalid_number(strNumber);
           FREE(strNumber);
          return 1;
        }

        FREE(strNumber);
        k_specified = true;
        break;
      }

    default:
      return UsageBmcIncSimulate();
    }
  }

  if ((mode != Interactive) && (display_all == 1)) return UsageBmcIncSimulate();

  if (argc != util_optind) {
    return UsageBmcIncSimulate();
  }

  /* pre-conditions */
  if (Bmc_check_if_model_was_built(nusmv_stderr, true)) return 1;

  if (bmc_simulate_get_curr_sim_trace() == TRACE(NULL)) {
    fprintf(nusmv_stderr,
            "No current state set. Use the \"bmc_pick_state\" command.\n");
    return 1;
  }

  bdd_enc = Enc_get_bdd_encoding();
  be_enc = Enc_get_be_encoding();
  be_mgr = BeEnc_get_be_manager(be_enc);

  if (isconstraint) {
    if (time_shift) {
      be_constraints = Bmc_Utils_simple_costraint_from_string(be_enc,
                                                              bdd_enc,
                                                              strConstr,
                                                              (Expr_ptr*)NULL);
    }
    else {
      be_constraints = Bmc_Utils_next_costraint_from_string(be_enc,
                                                            bdd_enc,
                                                            strConstr,
                                                            (Expr_ptr*)NULL);
    }

    FREE(strConstr);
  }
  else {
    be_constraints = Be_Truth(be_mgr);
  }

  /* This function does the actual work: */
  {
    Bmc_StepWiseSimulation(PropDb_master_get_be_fsm(PropPkg_get_prop_database()),
                           bdd_enc, TracePkg_get_global_trace_manager(),
                           steps, be_constraints, time_shift,
                           printrace, only_changes, mode, display_all);
  }

  return 0;
}

/**Function********************************************************************

  Synopsis           [Usage string for UsageBmcIncSimulate]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandBmcIncSimulate]

******************************************************************************/
static int UsageBmcIncSimulate (void)
{
  fprintf(nusmv_stderr,
          "usage: bmc_inc_simulate [-h] [-p | -v] [-r | -i [-a]] [[-c \"constr\"] "
          "| [-t \"constr\"]] [-k steps]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -p \t\tPrints current generated trace (only changed variables).\n");
  fprintf(nusmv_stderr, "  -v \t\tVerbosely prints current generated trace (all variables).\n");
  fprintf(nusmv_stderr, "  -r \t\tSets picking mode to random (default is deterministic).\n");
  fprintf(nusmv_stderr, "  -i \t\tEnters simulation's interactive mode.\n");
  fprintf(nusmv_stderr, "  -a \t\tDisplays all the state variables (changed and unchanged)\n");
  fprintf(nusmv_stderr, "     \t\tin every step of an interactive session.\n");
  fprintf(nusmv_stderr, "     \t\tIt works only together with -i option.\n");
  fprintf(nusmv_stderr, "  -c \"constr\"\tSets constraint (simple expression) for the next steps.\n");
  fprintf(nusmv_stderr, "  -t \"constr\"\tSets constraint (next expression) for the next steps.\n");
  fprintf(nusmv_stderr, "  -k <length> \tSpecifies the simulation length\n"
          "\t\tto be used when generating the simulated problem.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Picks a state from the set of initial states]

  CommandName        [bmc_pick_state]

  CommandSynopsis    [Picks a state from the set of initial states]

  CommandArguments   [\[-h\] \[-v\] \] \ [-c "constraint" | -s trace.state\] \[-r\] \[-a \[-i\]\]]

  CommandDescription [

  Chooses an element from the set of initial states, and makes it the
  <tt>current state</tt> (replacing the old one). The chosen state is
  stored as the first state of a new trace ready to be lengthened by
  <tt>steps</tt> states by the <tt>bmc_simulate</tt> or
  <tt>bmc_inc_simulate</tt> commands. A constraint can be provided to
  restrict the set of candidate states. <p>

  Command Options:<p>
  <dl>
    <dt> <tt>-v</tt>
       <dd> Verbosely prints out chosen state (all state variables, otherwise
       it prints out only the label <tt>t.1</tt> of the state chosen, where
       <tt>t</tt> is the number of the new trace, that is the number of
       traces so far generated plus one).
    <dt> <tt>-r</tt>
       <dd> Randomly picks a state from the set of initial states.
    <dt> <tt>-i</tt>
       <dd> Enters simulation's interactive mode.
    <dt> <tt>-a</tt>
       <dd> Displays all the state variables (changed and unchanged)
            in the interactive session
    <dt> <tt>-c "constraint"</tt>
       <dd> Uses <tt>constraint</tt> to restrict the set of initial states
       in which the state has to be picked.
    <dt> <tt>-s trace.state</tt>
       <dd> Picks state from trace.state label. A new simulation trace will
       be created by copying prefix of the source trace up to specified state.
  </dl> ]

  SideEffects [The state chosen is stored in the traces_hash table as
  the first state of a new trace]

******************************************************************************/
int Bmc_CommandBmcPickState(int argc, char **argv)
{
  int retval = 0;
  int c = 0;
  char* str_constr = (char*) NULL;
  char* str_label = (char*) NULL;
  boolean verbose = false;
  int tr_number = -1;
  be_ptr be_constr = (be_ptr) NULL;
  Simulation_Mode mode = Deterministic;
  short int usedMode = 0;
  int display_all = 0;

  TraceManager_ptr gtm = TracePkg_get_global_trace_manager();

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hc:s:vrai")) != EOF) {
    switch (c) {
    case 'h':
      retval = UsageBmcPickState();
      goto CommandBmcPickState_exit;

    case 'c': str_constr = util_strsav(util_optarg); break;
    case 's': str_label = util_strsav(util_optarg); break;
    case 'v': verbose = true; break;
    case 'r':
      if (++usedMode > 1) {
        retval = UsageBmcPickState();
        goto CommandBmcPickState_exit;
      }

      mode = Random;
      break;

    case 'i':
      if (++usedMode > 1) {
        retval = UsageBmcPickState();
        goto CommandBmcPickState_exit;
      }

      mode = Interactive;
      break;
    case 'a':
      display_all = 1;
      break;

    default:
      retval = UsageBmcPickState();
      goto CommandBmcPickState_exit;
    }
  }

  if ((mode != Interactive) && (display_all == 1)) {
    retval = UsageBmcPickState();
    goto CommandBmcPickState_exit;
  }

  if (argc != util_optind) {
    retval = UsageBmcPickState();
    goto CommandBmcPickState_exit;
  }

  /* conditions */
  if (Bmc_check_if_model_was_built(nusmv_stderr, true)) {
    retval = 1;
    goto CommandBmcPickState_exit;
  }

  if (str_label != (char*) NULL) {
    TraceLabel label;

    if (str_constr != (char*) NULL) {
      fprintf(nusmv_stderr,
              "Options -c and -s cannot be used at the same time\n");
      retval = 1;
      goto CommandBmcPickState_exit;
    }

    label = TraceLabel_create_from_string(str_label);
    if (label == TRACE_LABEL_INVALID ||   \
        !TraceManager_is_label_valid(gtm, label)) {

      fprintf(nusmv_stderr, "Label \"%s\" is invalid\n", str_label);
      retval = 1;
      goto CommandBmcPickState_exit;
    }
    FREE(str_label); str_label = (char*) NULL;

    { /* constructs a new trace from given label */
      Trace_ptr from_trace = \
        TraceManager_get_trace_at_index(gtm, TraceLabel_get_trace(label));
      TraceIter iter = TraceManager_get_iterator_from_label(gtm, label);

      Trace_ptr new_trace  = TRACE(NULL);

      /* create new simulation trace from previous one */
      new_trace = Trace_copy(from_trace, iter, false);

      Trace_set_desc(new_trace, "BMC Simulation");
      Trace_set_type(new_trace, TRACE_TYPE_SIMULATION);

      tr_number = TraceManager_register_trace(gtm, new_trace);
      TraceManager_set_current_trace_number(gtm, tr_number);
      bmc_simulate_set_curr_sim_trace(new_trace, tr_number);
    }
  }
  else { /* creates the trace from given constraint */
    BeEnc_ptr be_enc = Enc_get_be_encoding();
    Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
    BddEnc_ptr bdd_enc = Enc_get_bdd_encoding();

    if (str_constr != (char*) NULL) {
      CATCH {
        be_constr = Bmc_Utils_simple_costraint_from_string(be_enc, bdd_enc,
                                                           str_constr,
                                                           (Expr_ptr*) NULL);
      }
      FAIL {
        fprintf(nusmv_stderr, "In constraint: %s\n", str_constr);
        fprintf(nusmv_stderr,
                "Parsing error: expected a simple expression.\n");

        retval = 1;
        goto CommandBmcPickState_exit;
      }

      FREE(str_constr); str_constr = (char*) NULL;
    }
    else {
      be_constr = Be_Truth(be_mgr);
    }

    tr_number = Bmc_pick_state_from_constr(PropDb_master_get_be_fsm(
                                                  PropPkg_get_prop_database()),
                                           bdd_enc, be_constr, mode, display_all);
  }

  /* results presentation */
  if (tr_number != -1) {
    if (verbose) {
      TraceManager_execute_plugin(gtm, TRACE_OPT(NULL),
                                  TRACE_MANAGER_DEFAULT_PLUGIN,
                                  TRACE_MANAGER_LAST_TRACE);
    }
  }
  else {
    if ((be_ptr) NULL == be_constr) {
      fprintf(nusmv_stderr, "No trace: initial state is inconsistent\n");
    }
    else {
      fprintf(nusmv_stderr,
              "No trace: constraint and initial state are inconsistent\n");
    }
    retval = 1;
  }

CommandBmcPickState_exit:
  if ((char*) NULL != str_constr) FREE(str_constr);
  if ((char*) NULL != str_label) FREE(str_label);
  return retval;
}


/**Function********************************************************************

  Synopsis           [Usage string for UsageBmcPickState]

  Description        []

  SideEffects        [None]

  SeeAlso            [CommandBmcPickState]

******************************************************************************/
static int UsageBmcPickState(void)
{
  fprintf(nusmv_stderr, "usage: bmc_pick_state [-h] [-v] [-r | -i [-a]] [-c \"constr\" | -s trace.state]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -v \t\tVerbosely prints picked state.\n");
  fprintf(nusmv_stderr, "  -r \t\tRandomly picks a state from the set of the initial states\n");
  fprintf(nusmv_stderr, "     \t\t(otherwise choice is deterministic).\n");
  fprintf(nusmv_stderr, "  -c \"constr\"   Sets constraints for the initial set of states.\n");
  fprintf(nusmv_stderr, "  -s state\tPicks state from trace.state label.\n");
  fprintf(nusmv_stderr, "  -i \t\tEnters simulation's interactive mode.\n");
  fprintf(nusmv_stderr, "  -a \t\tDisplays all the state variables (changed and unchanged)\n");
  fprintf(nusmv_stderr, "     \t\tin the interactive session.\n");
  fprintf(nusmv_stderr, "     \t\tIt works only together with -i option.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Checks feasibility of a list of constraints for the
  simulation]

  CommandName        [bmc_simulate_check_feasible_constraints]

  CommandSynopsis [Performs a feasibility check on the list of given
  constraints. Constraints that are found to be feasible can be safely
  assumed not to cause deadlocks if used in the following step of
  incremental simulation.]

  CommandArguments   [\[-h | -q\] \[-c "formula"\]* ]

  CommandDescription [This command generates feasibility problems for
  each constraint. Every constraint is checked against current state
  and FSM's transition relation, in order to exclude the possibility
  of deadlocks. Constraints found to be feasible can be safely assumed
  not to cause deadlocks if used in the following step of incremental
  simulation.<BR>
  <p>
    Command options:<p>
    <dl>
    <dt> <tt>-q</tt>
       <dd> Enables quiet mode. For each analyzed constraint "0" is
       printed if the constraint is found to be unfeasible, "1" is
       printed otherwise. <BR>
    <dt> <tt>-c "formula"</tt>
       <dd> Provide a constraint as a <tt>formula</tt> specified on
            the command-line. This option can be specified multiple
            times, in order to analyze a list of constraints.<BR>
  </dl>
  ]

  SideEffects        [None]

******************************************************************************/
int Bmc_CommandBmcSimulateCheckFeasibleConstraints(int argc, char **argv)
{
  Olist_ptr str_constraints = Olist_create();
  Olist_ptr expr_constraints = Olist_create();
  Olist_ptr be_constraints = Olist_create();

  boolean human_readable = true; /* -q not used */
  int c = 0;
  int retval = 0;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hc:q")) != EOF) {
    switch (c) {
    case 'h':
      retval = UsageBmcSimulateCheckFeasibleConstraints();
      goto BmcSimulateCheckFeasibleConstraints_exit;
      break;

    case 'c': {
      /* now stores the constraint, later they will be compiled and checked */
      char* str_constr = util_strsav(util_optarg);
      Olist_append(str_constraints, str_constr);
      break;
    }

    case 'q':
      human_readable = false;
      break;

    default:
      retval = UsageBmcSimulateCheckFeasibleConstraints();
      goto BmcSimulateCheckFeasibleConstraints_exit;
    }
  }

  /* pre-conditions */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    retval = 1;
    goto BmcSimulateCheckFeasibleConstraints_exit;
  }
  if (bmc_simulate_get_curr_sim_trace() == TRACE(NULL)) {
    fprintf(nusmv_stderr, "A starting state has to be chosen. " \
            "Use command 'bmc_pick_state'\n");
    retval = 1;
    goto BmcSimulateCheckFeasibleConstraints_exit;
  }

  {
    BeEnc_ptr be_enc = Enc_get_be_encoding();
    BddEnc_ptr bdd_enc = Enc_get_bdd_encoding();

    /* converts the string constraints to be constraints */
    while (!Olist_is_empty(str_constraints)) {
      char* str_constr = (char*) Olist_delete_first(str_constraints);

      CATCH {
        Expr_ptr constr;
        be_ptr be_constr =
          Bmc_Utils_next_costraint_from_string(be_enc, bdd_enc, str_constr,
                                               &constr);
        Olist_append(be_constraints, be_constr);
        Olist_append(expr_constraints, constr);
      }
      FAIL {
        fprintf(nusmv_stderr, "In constraint: %s\n", str_constr);
        fprintf(nusmv_stderr,
                "Parsing error: expected a next expression.\n");
        FREE(str_constr);
        retval = 1;
        goto BmcSimulateCheckFeasibleConstraints_exit;
      }
      FREE(str_constr);
    }

    { /* does the actual work, and presents the result */
      Trace_ptr curr_trace = bmc_simulate_get_curr_sim_trace();

      be_ptr curr_state = \
        TraceUtils_fetch_as_be(curr_trace, Trace_last_iter(curr_trace),
                               TRACE_ITER_SF_VARS, be_enc, bdd_enc);

      Olist_ptr res = Bmc_simulate_check_feasible_constraints(
                      PropDb_master_get_be_fsm(PropPkg_get_prop_database()),
                      bdd_enc, be_constraints,
                      curr_state);
      {
        Oiter iter_c;
        Oiter iter_r;
        int idx = 0;

        /* prints out the result */
        for (iter_c=Olist_first(expr_constraints), 
               iter_r=Olist_first(res), idx=0;
             !Oiter_is_end(iter_c) && !Oiter_is_end(iter_r);
             iter_c=Oiter_next(iter_c), iter_r=Oiter_next(iter_r), ++idx) {

          Expr_ptr constr = (Expr_ptr) Oiter_element(iter_c);
          boolean truth = (boolean) Oiter_element(iter_r);

          if (human_readable) {
            /* header */
            if (idx == 0) fprintf(nusmv_stdout, "#num\ttruth\tconstraint\n"); 
            fprintf(nusmv_stdout, "%d\t%d\t", idx, truth);
            print_node(nusmv_stdout, constr);
            fprintf(nusmv_stdout, "\n");
          }
          else {
            /* not human-readable output format */
            fprintf(nusmv_stdout, "%d", truth);
          }
        }

        fprintf(nusmv_stdout, "\n");
      }

      Olist_destroy(res);
    }

  BmcSimulateCheckFeasibleConstraints_exit:
    { /* frees any pending string in the constraints list */
      Oiter iter;
      OLIST_FOREACH(str_constraints, iter) {
        char* str_constr = (char*) Oiter_element(iter);
        FREE(str_constr);
      }
      Olist_destroy(str_constraints);
    }
    Olist_destroy(be_constraints);
    Olist_destroy(expr_constraints);
    return retval;
  }
}

/**Function********************************************************************

  Synopsis           [Usage string for
                      UsageBmcSimulateCheckFeasibleConstraints]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandSimulateCheckFeasibleConstraints]

******************************************************************************/
static int UsageBmcSimulateCheckFeasibleConstraints(void)
{
  fprintf(nusmv_stderr,
          "usage: bmc_simulate_check_feasible_constraints [-h] [-q] [-c \"next_expr\"] [-c ...]\n");
  fprintf(nusmv_stderr,
          "  -h \t\t Prints the command usage.\n");
  fprintf(nusmv_stderr,
          "  -q \t\t Prints the output in compact form.\n");
  fprintf(nusmv_stderr,
          "  -c next_expr\t Specify one constraint whose feasability has to be checked\n"\
          "\t\t (can be used multiple times, order is important to read the result)\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Generates length_max+1 problems iterating the problem
  bound from zero to length_max, and dumps each problem to a dimacs file]

  Description        [Each problem is dumped for the given LTL specification,
  or for all LTL specifications if no formula is given.
  Generation parameters are the maximum bound and the loopback values. <BR>
  After command line processing it calls the function Bmc_GenSolveLtl
  to generate and dump all problems from zero to k.]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_CommandGenLtlSpecBmcOnePb, Bmc_GenSolveLtl]

  CommandName        [gen_ltlspec_bmc]

  CommandSynopsis    [Dumps into one or more dimacs files the given LTL
  specification, or all LTL specifications if no formula is given.
  Generation and dumping parameters are the maximum bound and the loopback
  values]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\] | -P "name"\]
  \[-k max_length\] \[-l loopback\] \[-o filename\]]

  CommandDescription [  This command generates one or more problems, and
  dumps each problem into a dimacs file. Each problem is related to a specific
  problem bound, which increases from zero (0) to the given maximum problem
  bound. In this short description "<i>length</i>" is the bound of the
  problem that system is going to dump out. <BR>
  In this context the maximum problem bound is represented by the
  <i>max_length</i> parameter, or by its default value stored in the
  environment variable <i>bmc_length</i>.<BR>
  Each dumped problem also depends on the loopback you can explicitly
  specify by the <i>-l</i> option, or by its default value stored in the
  environment variable <i>bmc_loopback</i>. <BR>
  The property to be checked may be specified using the <i>-n idx</i>,
  the <i>-p "formula"</i> or the <i>-P "name"</i> options. <BR>
  You may specify dimacs file name by using the option <i>-o "filename"</i>,
  otherwise the default value stored in the environment variable
  <i>bmc_dimacs_filename</i> will be considered.<BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the LTLSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> <i>max_length</i> is the maximum problem bound used when
       increasing problem bound starting from zero. Only natural number are
       valid values for this option. If no value is given the environment
       variable <i>bmc_length</i> value is considered instead.
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can
       be also used as prefix of the number. Any invalid combination of bound
       and loopback will be skipped during the generation and
       dumping process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case
       <i>loopback</i> is considered a value relative to <i>max_length</i>.
       Any invalid combination of bound and loopback will be skipped during
       the generation process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to
       <i>length-1</i>"
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of dumped dimacs files, without
       extension. <BR>
       If this options is not specified, variable <i>bmc_dimacs_filename</i>
       will be considered. The file name string may contain special symbols
       which will be macro-expanded to form the real file name.
       Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties
       database <BR>
       - @@: the '@' character
  </dl>]


******************************************************************************/
int Bmc_CommandGenLtlSpecBmc(int argc, char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  Outcome opt_handling_res;
  int k = get_bmc_pb_length(OptsHandler_get_instance());
  char* fname = (char*) NULL;
  int relative_loop = Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(OptsHandler_get_instance()),
                                                      NULL);


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = Bmc_cmd_options_handling(argc, argv,
                                              Prop_Ltl, &ltlprop,
                                              &k, &relative_loop,
                                              NULL, NULL, &fname, NULL);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcGenLtlSpec();
  }

  if (opt_handling_res != OUTCOME_SUCCESS) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  if (fname == (char*) NULL) {
    fname = util_strsav(get_bmc_dimacs_filename(OptsHandler_get_instance()));
  }

  /* prepare the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Ltl);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Ltl);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtl(prop, k, relative_loop,
                          true, /* iterate on k */
                          false, /* do not solve */
                          BMC_DUMP_DIMACS, fname) != 0) {
        FREE(fname);
        return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve: */
    if (Bmc_GenSolveLtl(ltlprop, k, relative_loop,
                        true, /* iterate on k */
                        false, /* do not solve */
                        BMC_DUMP_DIMACS, fname) != 0) {
      FREE(fname);
      return 1;
    }
  }

  FREE(fname);
  return 0;
}


/**Function********************************************************************

  Synopsis           [Usage string for command gen_ltlspec_bmc]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandGenLtlSpecBmc]

******************************************************************************/
static int UsageBmcGenLtlSpec(void)
{
  fprintf(nusmv_stderr, "\nusage: gen_ltlspec_bmc [-h | -n idx | -p \"formula\" | -P \"name\"] [-k max_length] [-l loopback]\n\t\t\t [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -P \"name\"\tChecks the LTL property specified with <name>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n");
  fprintf(nusmv_stderr, "  -k max_length\tChecks the property using <max_length> value instead of using the\n\t\tvariable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n");
  fprintf(nusmv_stderr, "  -o filename\tUses <filename> as dimacs file instead of using the\n\t\t\"bmc_dimacs_filename\" variable. <filename> may contain patterns.\n\n");

  return 1;
}


/**Function********************************************************************

  Synopsis           [Generates only one problem with fixed bound and
  loopback, and dumps the problem to a dimacs file. The single problem
  is dumped for the given LTL specification, or for all LTL
  specifications if no formula is given]

  Description        [After command line processing it calls
  the function Bmc_GenSolveLtl to generate and dump the single problem.]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_CommandGenLtlSpecBmc, Bmc_GenSolveLtl]

  CommandName        [gen_ltlspec_bmc_onepb]

  CommandSynopsis    [Dumps into one dimacs file the problem generated for
  the given LTL specification, or for all LTL specifications if no formula
  is explicitly given.
  Generation and dumping parameters are the problem bound and the loopback
  values]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\] | -P "name"\] \[-k length\]
  \[-l loopback\] \[-o filename\]]

  CommandDescription [ As the <i>gen_ltlspec_bmc</i> command, but it generates
  and dumps only one problem given its bound and loopback. <BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the LTLSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>length</i></tt>
       <dd> <i>length</i> is the single problem bound used to generate and
       dump it. Only natural number are valid values for this option.
       If no value is given the environment variable <i>bmc_length</i>
       is considered instead.
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>length-1</i>). Positive sign ('+') can
       be also used as prefix of the number. Any invalid combination of length
       and loopback will be skipped during the generation and dumping
       process.<BR>
       - a negative number in (-1, -<i>length</i>).
       Any invalid combination of length and loopback will be skipped during
       the generation process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to
       <i>length-1</i>"
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file, without
       extension. <BR>
       If this
       options is not specified, variable <i>bmc_dimacs_filename</i> will be
       considered. The file name string may contain special symbols which
       will be macro-expanded to form the real file name.
       Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties
       database <BR>
       - @@: the '@' character
  </dl>]

******************************************************************************/
int Bmc_CommandGenLtlSpecBmcOnePb(int argc, char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  Outcome opt_handling_res;
  int k = get_bmc_pb_length(OptsHandler_get_instance());
  char* fname = (char*) NULL;
  int relative_loop =
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(OptsHandler_get_instance()), NULL);


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = Bmc_cmd_options_handling(argc, argv,
                                              Prop_Ltl, &ltlprop,
                                              &k, &relative_loop,
                                              NULL, NULL, &fname, NULL);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcGenLtlSpecOnePb();
  }

  if (opt_handling_res != OUTCOME_SUCCESS) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  if (fname == (char*) NULL) {
    fname = util_strsav(get_bmc_dimacs_filename(OptsHandler_get_instance()));
  }

  /* prepare the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Ltl);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Ltl);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtl(prop, k, relative_loop,
                          false, /* do not iterate k */
                          false, /* do not solve */
                          BMC_DUMP_DIMACS, fname) != 0) {
        FREE(fname);
        return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve: */
    if (Bmc_GenSolveLtl(ltlprop, k, relative_loop,
                        false, /* do not iterate k */
                        false, /* do not solve */
                        BMC_DUMP_DIMACS, fname) != 0) {
      FREE(fname);
      return 1;
    }
  }

  FREE(fname);
  return 0;
}


/**Function********************************************************************

  Synopsis           [Usage string for command gen_ltlspec_bmc_onepb]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandGenLtlSpecBmcOnePb]

******************************************************************************/
static int UsageBmcGenLtlSpecOnePb(void)
{
  fprintf(nusmv_stderr, "\nusage: gen_ltlspec_bmc_onepb [-h | -n idx | -p \"formula\" | -P \"name\"] [-k length] [-l loopback]\n\t\t\t [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -P \"name\"\tChecks the LTL property specified with <name>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n");
  fprintf(nusmv_stderr, "  -k length\tChecks the property using <length> value instead of using the\n\t\tvariable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n");
  fprintf(nusmv_stderr, "  -o filename\tUses <filename> as dimacs file instead of \"bmc_dimacs_filename\"\n\t\tvariable. <filename> may contain patterns.\n\n");

  return 1;
}


/**Function********************************************************************

  Synopsis           [Checks the given LTL specification, or all LTL
  specifications in the properties database if no formula is given]

  Description        [After command line processing this function calls
  the Bmc_GenSolveLtl to generate and solve all problems from 0 to k.
  Parameters are the maximum length and the loopback values.]

  SideEffects        [Properties database may change]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmcOnePb, Bmc_GenSolveLtl]

  CommandName        [check_ltlspec_bmc]

  CommandSynopsis    [Checks the given LTL specification, or all LTL
  specifications if no formula is given. Checking parameters are the maximum
  length and the loopback values]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\] | -P "name"\]
  \[-k max_length\] \[-l loopback\] \[-o filename\]]

  CommandDescription [
  This command generates one or more problems, and calls
  SAT solver for each one. Each problem is related to a specific problem
  bound, which increases from zero (0) to the given maximum problem
  length. Here "<i>length</i>" is the bound of the problem that system
  is going to generate and/or solve. <BR>
  In this context the maximum problem bound is represented by the
  <i>-k</i> command parameter, or by its default value stored in the
  environment variable <i>bmc_length</i>.<BR>
  The single generated problem also depends on the "<i>loopback</i>"
  parameter you can explicitly specify by the <i>-l</i> option, or by its
  default value stored in the environment variable <i>bmc_loopback</i>. <BR>
  The property to be checked may be specified using the <i>-n idx</i>,
  the <i>-p "formula"</i> or the <i>-P "name"</i> options. <BR>
  If you need to generate a dimacs dump file of all generated problems, you
  must use the option <i>-o "filename"</i>. <BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification
       formula actually located in the properties database. <BR>
    <dt> <tt>-p "formula" \[IN context\]</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the LTLSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> <i>max_length</i> is the maximum problem bound must be reached.
       Only natural number are valid values for this option. If no value
       is given the environment variable <i>bmc_length</i> is considered
       instead.
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can
       be also used as prefix of the number. Any invalid combination of length
       and loopback will be skipped during the generation/solving process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case
       <i>loopback</i> is considered a value relative to <i>max_length</i>.
       Any invalid combination of length and loopback will be skipped
       during the generation/solving process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to
       <i>length-1</i>"
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file, without
       extension. <BR>
       It may contain special symbols which will be macro-expanded to form
       the real file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties
       database <BR>
       - @@: the '@' character
  </dl>
  ]

******************************************************************************/
int Bmc_CommandCheckLtlSpecBmc(int argc, char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  Outcome opt_handling_res;
  int k = get_bmc_pb_length(OptsHandler_get_instance());
  char* fname = (char*) NULL;
  int relative_loop =
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(OptsHandler_get_instance()), NULL);


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = Bmc_cmd_options_handling(argc, argv,
                                              Prop_Ltl, &ltlprop,
                                              &k, &relative_loop,
                                              NULL, NULL, &fname, NULL);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcCheckLtlSpec();
  }

  if (opt_handling_res != OUTCOME_SUCCESS) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  /* prepares the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Ltl);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Ltl);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtl(prop, k, relative_loop, true,
                  true, /* solve */
                  (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                  fname) != 0) {
        if (fname != (char*) NULL) FREE(fname);
        return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve (a single property): */
    if (Bmc_GenSolveLtl(ltlprop, k, relative_loop, true,
                true, /* solve */
                (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                fname) != 0) {
      if (fname != (char*) NULL) FREE(fname);
      return 1;
    }
  }

  if (fname != (char*) NULL) FREE(fname);
  return 0;
}


/**Function********************************************************************

  Synopsis           [Usage string for command check_ltlspec_bmc]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmc]

******************************************************************************/
static int UsageBmcCheckLtlSpec(void)
{
  fprintf(nusmv_stderr, "\nusage: check_ltlspec_bmc [-h | -n idx | -p \"formula\" | -P \"name\"] [-k max_length] [-l loopback]\n\t\t\t [-o <filename>]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -P \"name\"\tChecks the LTL property specified with <name>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n");
  fprintf(nusmv_stderr, "  -k max_length\tChecks the property using <max_length> value instead of using the\n\t\tvariable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n");
  fprintf(nusmv_stderr, "  -o filename\tGenerates dimacs output file too. <filename> may contain patterns.\n\n");

  return 1;
}


/**Function********************************************************************

  Synopsis           [Checks the given LTL specification, or all LTL
  specifications if no formula is given. Checking parameters are the problem
  bound and the loopback values]

  Description        [After command line processing this function calls
  the Bmc_GenSolveLtl which generates and solve the singleton
  problem with bound k and loopback l. <BR>
  ]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmc, Bmc_GenSolveLtl]

  CommandName        [check_ltlspec_bmc_onepb]

  CommandSynopsis    [Checks the given LTL specification, or all LTL
  specifications if no formula is given. Checking parameters are the single
  problem bound and the loopback values]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\] | -P "name"\]
  \[-k length\] \[-l loopback\] \[-o filename\]]

  CommandDescription [As command check_ltlspec_bmc but it produces only one
  single problem with fixed bound and loopback values, with no iteration
  of the problem bound from zero to max_length. <BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the LTLSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>length</i></tt>
       <dd> <i>length</i> is the problem bound used when generating the
       single problem. Only natural number are valid values for this option.
       If no value is given the environment variable <i>bmc_length</i> is
       considered instead.
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can
       be also used as prefix of the number. Any invalid combination of length
       and loopback will be skipped during the generation/solving process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case
       <i>loopback</i> is considered a value relative to <i>length</i>.
       Any invalid combination of length and loopback will be skipped
       during the generation/solving process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to
       <i>length-1</i>"
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file, without
       extension.<BR>
       It may contain special symbols which will be macro-expanded to form
       the real file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @k: current problem bound <BR>
       - @l: current loopback value <BR>
       - @n: index of the currently processed formula in the properties
       database <BR>
       - @@: the '@' character
  </dl>]

******************************************************************************/
int Bmc_CommandCheckLtlSpecBmcOnePb(int argc, char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  Outcome opt_handling_res;
  int k = get_bmc_pb_length(OptsHandler_get_instance());
  char* fname = (char*) NULL;
  int relative_loop =
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(OptsHandler_get_instance()), NULL);


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = Bmc_cmd_options_handling(argc, argv,
                                              Prop_Ltl, &ltlprop,
                                              &k, &relative_loop,
                                              NULL, NULL, &fname, NULL);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcCheckLtlSpecOnePb();
  }

  if (opt_handling_res != OUTCOME_SUCCESS) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* ----------------------------------------------------------------------- */


  /* prepare the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Ltl);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Ltl);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtl(prop, k, relative_loop,
              false, /* do not iterate k */
              true, /* to always solve */
              (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
              fname) != 0) {
        if (fname != (char*) NULL) FREE(fname);
        return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve: */
    if (Bmc_GenSolveLtl(ltlprop, k, relative_loop,
                false, /* do not iterate k */
                true, /* to always solve */
                (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                fname) != 0) {
      if (fname != (char*) NULL) FREE(fname);
      return 1;
    }
  }

  if (fname != (char*) NULL) FREE(fname);
  return 0;
}


/**Function********************************************************************

  Synopsis           [Usage string for command check_ltlspec_bmc_onepb]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmcOnePb]

******************************************************************************/
static int UsageBmcCheckLtlSpecOnePb(void)
{
  fprintf(nusmv_stderr, "\nusage: check_ltlspec_bmc_onepb [-h | -n idx | -p \"formula\" | -P \"name\"] [-k length] [-l loopback]\n\t\t\t [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property.\n");
  fprintf(nusmv_stderr, "  -P \"name\"\tChecks the LTL property specified with <name>.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties.\n");
  fprintf(nusmv_stderr, "  -k length\tChecks the property using <length> value instead of using the\n\t\tvariable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n");
  fprintf(nusmv_stderr, "  -o filename\tGenerates dimacs output file too. <filename> may contain patterns.\n\n");

  return 1;
}

#if NUSMV_HAVE_INCREMENTAL_SAT
/**Function********************************************************************

  Synopsis          [Checks the given LTL specification, or all LTL
  specifications in the properties database if no formula is given,
  using incremental algorithms]

  Description        [Parameters are the maximum length and the loopback
  values. The function is compiled only if there is at least
  one incremental SAT solver]

  SideEffects        [Properties database may change]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmcOnePb, Bmc_CommandCheckLtlSpecBmc]

  CommandName        [check_ltlspec_bmc_inc]

  CommandSynopsis    [Checks the given LTL specification, or all LTL
  specifications if no formula is given, using incremental algorithms.
  Checking parameters are the maximum length and the loopback values]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\] | -P "name"\]
  \[-k max_length\] \[-l loopback\] ]

  CommandDescription [
  This command generates one or more problems, and calls (incremental)
  SAT solver for each one. Each problem is related to a specific problem
  bound, which increases from zero (0) to the given maximum problem
  length. Here "<i>length</i>" is the bound of the problem that system
  is going to generate and/or solve. <BR>
  In this context the maximum problem bound is represented by the
  <i>-k</i> command parameter, or by its default value stored in the
  environment variable <i>bmc_length</i>.<BR>
  The single generated problem also depends on the "<i>loopback</i>"
  parameter you can explicitly specify by the <i>-l</i> option, or by its
  default value stored in the environment variable <i>bmc_loopback</i>. <BR>
  The property to be checked may be specified using the <i>-n idx</i>,
  the <i>-p "formula"</i> or the <i>-P "name"</i> options. <BR>
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid LTL specification
       formula actually located in the properties database. <BR>
    <dt> <tt>-p "formula" \[IN context\]</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the LTLSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> <i>max_length</i> is the maximum problem bound must be reached.
       Only natural number are valid values for this option. If no value
       is given the environment variable <i>bmc_length</i> is considered
       instead.
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>max_length-1</i>). Positive sign ('+') can
       be also used as prefix of the number. Any invalid combination of length
       and loopback will be skipped during the generation/solving process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case
       <i>loopback</i> is considered a value relative to <i>max_length</i>.
       Any invalid combination of length and loopback will be skipped
       during the generation/solving process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to
       <i>length-1</i>"
  </dl>
  ]

******************************************************************************/
int Bmc_CommandCheckLtlSpecBmcInc(int argc, char** argv)
{
  Prop_ptr ltlprop = PROP(NULL);   /* The property being processed */
  Outcome opt_handling_res;
  int k = get_bmc_pb_length(OptsHandler_get_instance());
  int relative_loop =
    Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(OptsHandler_get_instance()), NULL);

  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = Bmc_cmd_options_handling(argc, argv,
                                              Prop_Ltl, &ltlprop,
                                              &k, &relative_loop,
                                              NULL, NULL, NULL, NULL);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    return UsageBmcCheckLtlSpecInc();
  }
  if (opt_handling_res != OUTCOME_SUCCESS)  return 1;

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  /* prepares the list of properties if no property was selected: */
  if (ltlprop == PROP(NULL)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Ltl);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Ltl);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveLtlInc(prop, k, relative_loop, true) != 0) return 1;
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to solve (a single property): */
    if (Bmc_GenSolveLtlInc(ltlprop, k, relative_loop, true) != 0) return 1;
  }

  return 0;
}
#endif

#if NUSMV_HAVE_INCREMENTAL_SAT
/**Function********************************************************************

  Synopsis           [Usage string for command check_ltlspec_bmc_inc]

  Description        [The function is compiled only if there is at least
  one incremental SAT solver.]

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckLtlSpecBmc]

******************************************************************************/
static int UsageBmcCheckLtlSpecInc(void)
{
  fprintf(nusmv_stderr, "\nusage: check_ltlspec_bmc_inc [-h | -n idx | -p \"formula\" | -P \"name\"] [-k max_length] [-l loopback]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the LTL property specified with <idx> \n"
                        "        \t(using incremental algorithms).\n");
  fprintf(nusmv_stderr, "  -P \"name\"\tChecks the LTL property specified with <name> \n"
                        "        \t(using incremental algorithms).\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified LTL property (using incremental algorithms).\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all LTL properties (using \n"
                        "\t\tincremental algorithms).\n");
  fprintf(nusmv_stderr, "  -k max_length\tChecks the property using <max_length> value instead of using \n\t\tthe variable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "  -l loopback\tChecks the property using <loopback> value instead of using the\n\t\tvariable <bmc_loopback> value.\n\n");
  return 1;
}
#endif

/**Function********************************************************************

  Synopsis           [Generates and dumps the problem for the given
  invariant or for all invariants if no formula is given. SAT solver is not
  invoked.]

  Description        [After command line processing calls Bmc_GenSolveInvar
  to dump the generated invariant problem.
  If you specify the <i>-o "filename"</i> option a dimacs file named
  "filename" will be created, otherwise the environment variable
  <i>bmc_invar_dimacs_filename</i> value will be considered.]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_GenSolveInvar]

  CommandName        [gen_invar_bmc]

  CommandSynopsis    [Generates the given invariant, or all
  invariants if no formula is given]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\] | -P "name"\]
  \[-o filename\]]

  CommandDescription [<p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid INVAR specification
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula" \[IN context\]</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P "name"</tt>
       <dd> Checks the invariant property stored in the properties
       database with name "name"
    <dt> <tt>-o <i>filename</i></tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file,
       without extension. <BR>
       If you
       do not use this option the dimacs file name is taken from the
       environment variable <i>bmc_invar_dimacs_filename</i>. <BR>
       File name may contain special symbols which will be macro-expanded
       to form the real dimacs file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @n: index of the currently processed formula in the properties
       database <BR>
       - @@: the '@' character
  </dl>]

******************************************************************************/
int Bmc_CommandGenInvarBmc(int argc, char** argv)
{
  Prop_ptr invarprop = PROP(NULL);   /* The property being processed */
  Outcome opt_handling_res;
  char* fname = (char*) NULL;
  char* algorithm_name = (char*) NULL;

  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = Bmc_cmd_options_handling(argc, argv,
                                              Prop_Invar, &invarprop,
                                              NULL, NULL,
                                              &algorithm_name, NULL,
                                              &fname, NULL);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcGenInvar();
  }

  if (opt_handling_res != OUTCOME_SUCCESS) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* make sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  if (fname == (char*) NULL) {
    fname = util_strsav(get_bmc_invar_dimacs_filename(OptsHandler_get_instance()));
  }

  /* Checks algorithms: */
  if (algorithm_name == (char*) NULL) {
    algorithm_name = util_strsav((char*) get_bmc_invar_alg(OptsHandler_get_instance()));
  }

  if (strcasecmp(algorithm_name, BMC_INVAR_ALG_CLASSIC) != 0) {
    fprintf (nusmv_stderr,
             "Generation of invariants are allowed only with "
             "'" BMC_INVAR_ALG_CLASSIC "'"
             " algorithm.\n");
    FREE(algorithm_name);
    FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  FREE(algorithm_name);

  /* prepare the list of properties if no property was selected: */
  if (invarprop == PROP(NULL)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Invar);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Invar);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (Bmc_GenSolveInvar(prop,
                            false, /*do not solve */
                            BMC_DUMP_DIMACS, fname) != 0) {
        FREE(fname);
        return 1;
      }
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    /* its time to generate dimacs: */
    if (Bmc_GenSolveInvar(invarprop,
                          false, /*do not solve */
                          BMC_DUMP_DIMACS, fname) != 0) {
      FREE(fname);
      return 1;
    }
  }

  FREE(fname);
  return 0;
}


/**Function********************************************************************

  Synopsis           [Usage string for command gen_invar_bmc]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandGenInvarBmc]

******************************************************************************/
static int UsageBmcGenInvar(void)
{
  fprintf(nusmv_stderr, "\nusage: gen_invar_bmc [-h | -n idx | -p \"formula\" | -P \"name\"] [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");

  fprintf(nusmv_stderr, "  -n idx\tChecks the INVAR property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified INVAR propositional property.\n");
  fprintf(nusmv_stderr, "  -P \"name\"\tChecks the INVAR property specified with <name>.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all INVAR properties.\n");
  fprintf(nusmv_stderr, "  -o filename\tUses <filename> as dimacs file instead of using the\n\t\t\"bmc_invar_dimacs_filename\" variable. <filename> may contain patterns.\n\n");

  return 1;
}


/**Function********************************************************************

  Synopsis           [Generates and solve the given invariant, or all
  invariants if no formula is given]

  Description        [After command line processing calls Bmc_GenSolveInvar
  to solve and eventually dump the generated invariant problem. If you specify
  the <i>-o "filename"</i> option a dimacs file will be generated, otherwise
  no dimacs dump will be performed]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_GenSolveInvar]

  CommandName        [check_invar_bmc]

  CommandSynopsis    [Generates and solve the given invariant, or all
  invariants if no formula is given]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\] | -P "name"\]
  \[-k max_length\] \[-a algorithm\] \[-o filename\] ]

  CommandDescription [<p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid INVAR specification
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the INVARSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> (Use only when selected algorithm is een-sorensson).
            Use to specify the maximal deepth to be reached by the een-sorensson
            invariant checking algorithm. If not specified, the value assigned
            to the system variable <i>bmc_length</i> is taken.
    <dt> <tt>-a <i>algorithm</i></tt>
       <dd> Uses the specified algorithm to solve the invariant. If used, this
            option will override system variable <i>bmc_invar_alg</i>.
            At the moment, possible values are: "classic", "een-sorensson".
    <dt> <tt>-e</i></tt>
       <dd> Uses an additional step clause for algorithm "een-sorensson".</tt>
       <dd> <i>filename</i> is the name of the dumped dimacs file, without
       extension. <BR>
       It may contain special symbols which will be macro-expanded to form
       the real file name. Possible symbols are: <BR>
       - @F: model name with path part <BR>
       - @f: model name without path part <BR>
       - @n: index of the currently processed formula in the properties
       database <BR>
       - @@: the '@' character
  </dl>]

******************************************************************************/
int Bmc_CommandCheckInvarBmc(int argc, char** argv)
{
  Prop_ptr invarprop = PROP(NULL);   /* The property being processed */
  Outcome opt_handling_res;
  char* fname = (char*) NULL;
  char* algorithm_name = (char*) NULL;
  int max_k = -1;
  boolean use_classic_alg = true;
  int res = 0;
  boolean use_extra_step = false;


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = Bmc_cmd_options_handling(argc, argv,
                                              Prop_Invar, &invarprop,
                                              &max_k, NULL,
                                              &algorithm_name,
                                              NULL, &fname,
                                              &use_extra_step);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return UsageBmcCheckInvar();
  }

  if (opt_handling_res != OUTCOME_SUCCESS) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }
  /* ----------------------------------------------------------------------- */

  /* Checks algorithms: */
  if (algorithm_name == (char*) NULL) {
    algorithm_name = util_strsav((char*) get_bmc_invar_alg(OptsHandler_get_instance()));
  }

  if ((strcasecmp(algorithm_name, BMC_INVAR_ALG_CLASSIC) != 0) &&
      (strcasecmp(algorithm_name, BMC_INVAR_ALG_EEN_SORENSSON) != 0)) {
    fprintf (nusmv_stderr,
             "'%s' is an invalid algorithm name.\n"
             "Valid names are "
             "'" BMC_INVAR_ALG_CLASSIC "'"
             " and "
             "'" BMC_INVAR_ALG_EEN_SORENSSON "'.\n", algorithm_name);
    FREE(algorithm_name);
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  /* choses the algorithm: */
  use_classic_alg = (strcasecmp(algorithm_name, BMC_INVAR_ALG_CLASSIC) == 0);
  FREE(algorithm_name);

  /* checks length: */
  if (use_classic_alg && max_k != -1) {
    fprintf (nusmv_stderr,
             "Option -k can be used only when '"
             BMC_INVAR_ALG_EEN_SORENSSON "' algorithm is selected.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* checks extra step: */
  if (use_classic_alg && use_extra_step) {
    fprintf (nusmv_stderr,
             "Option -e can be used only when '"
             BMC_INVAR_ALG_EEN_SORENSSON "' algorithm is selected.\n");
    if (fname != (char*) NULL) FREE(fname);
    return 1;
  }

  /* if not specified, selects length from bmc_pb_length */
  if (max_k == -1) {
    max_k = get_bmc_pb_length(OptsHandler_get_instance());
  }


  /* prepare the list of properties if no property was selected: */
  if (invarprop == PROP(NULL)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Invar);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Invar);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (use_classic_alg) {
        res = Bmc_GenSolveInvar(prop,
                 true, /* solve */
                 (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                        fname);
      }
      else {
        res = Bmc_GenSolveInvar_EenSorensson(prop, max_k,
                     (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                                             fname, use_extra_step);
      }
      if (res != 0) break;
    } /* for loop */

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    if (use_classic_alg) {
      res = Bmc_GenSolveInvar(invarprop,
              true,  /* solve */
              (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
              fname);
    }
    else {
      res = Bmc_GenSolveInvar_EenSorensson(invarprop, max_k,
                   (fname != (char*) NULL) ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                                           fname, use_extra_step);
    }
  }

  if (fname != (char*) NULL) FREE(fname);
  return res;
}

/**Function********************************************************************

  Synopsis           [Usage string for command check_invar_bmc]

  Description        []

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckInvarBmc]

******************************************************************************/
static int UsageBmcCheckInvar(void)
{
  fprintf(nusmv_stderr, "\nusage: check_invar_bmc [-h | -n idx | -p \"formula\" | -P \"name\"] [-k max_len] [-s] [-a alg] [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tChecks the INVAR property specified with <idx>.\n");
  fprintf(nusmv_stderr, "  -p \"formula\"\tChecks the specified INVAR propositional property.\n");
  fprintf(nusmv_stderr, "  -P name\tChecks the INVAR property specified with <name>.\n");
  fprintf(nusmv_stderr, "\t\tIf no property is specified, checks all INVAR properties.\n");
  fprintf(nusmv_stderr, "  -k max_len\tUpper bound for the search.\n"
          "\t\tUse only when " BMC_INVAR_ALG_EEN_SORENSSON " algorithm is selected.\n"
          "\t\tIf not specified, variable bmc_length is taken.\n");
  fprintf(nusmv_stderr, "  -e \t\tPerforms an extra step for finding a proof\n"
          "\t\tCan be used only with the " BMC_INVAR_ALG_EEN_SORENSSON " algorithm\n");
  fprintf(nusmv_stderr, "  -a alg\tUses the specified algorithm. \n");
  fprintf(nusmv_stderr, "\t\tValid values are: "
          BMC_INVAR_ALG_CLASSIC ", " BMC_INVAR_ALG_EEN_SORENSSON
          "\n\t\tDefault value is taken from variable bmc_invar_alg.\n");
  fprintf(nusmv_stderr, "  -o filename\tGenerates dimacs output file too. <filename> may contain patterns.\n\n");

  return 1;
}


#if NUSMV_HAVE_INCREMENTAL_SAT
/**Function********************************************************************

  Synopsis           [Solve the given invariant, or all
  invariants if no formula is given, using incremental algorithms.]

  Description        [The function is compiled only if there is at least
  one incremental SAT solver]

  SideEffects        [Property database may change]

  SeeAlso            [Bmc_CommandCheckInvarBmc]

  CommandName        [check_invar_bmc_inc]

  CommandSynopsis    [Generates and solve the given invariant, or all
  invariants if no formula is given]

  CommandArguments   [\[-h | -n idx | -p "formula" \[IN context\] | -P "name"\]
  \[-k max_length\] \[-a algorithm\] \[-s strategy\] ]

  CommandDescription [<p>
  Command options:<p>
  <dl>
    <dt> <tt>-n <i>index</i></tt>
       <dd> <i>index</i> is the numeric index of a valid INVAR specification
       formula actually located in the properties database. <BR>
       The validity of <i>index</i> value is checked out by the system.
    <dt> <tt>-p "formula \[IN context\]"</tt>
       <dd> Checks the <tt>formula</tt> specified on the command-line. <BR>
            <tt>context</tt> is the module instance name which the variables
            in <tt>formula</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the INVARSPEC property with name <tt>name</tt> in the property
            database.
    <dt> <tt>-k <i>max_length</i></tt>
       <dd> Use to specify the maximal depth to be reached by the incremental
            invariant checking algorithm. If not specified, the value assigned
            to the system variable <i>bmc_length</i> is taken.
    <dt> <tt>-a <i>algorithm</i></tt>
       <dd> Use to specify incremental invariant checking algorithm. Currently
            this can be one of the following values: dual, zigzag,
            falsification.
    <dt> <tt>-s <i>strategy</i></tt>
       <dd> Use to specify closure strategy (this currenly applies to dual
       algorithm only). This can be one of the following values: backward,
       forward.
  </dl>]

******************************************************************************/
int Bmc_CommandCheckInvarBmcInc(int argc, char** argv)
{
  Prop_ptr invarprop = PROP(NULL);   /* The property being processed */
  Outcome opt_handling_res;
  char* algorithm_name = (char*) NULL;
  char* closure_strategy_name = (char*) NULL;

  bmc_invar_algorithm algorithm = ALG_UNDEFINED;
  bmc_invar_closure_strategy closure_strategy = BMC_INVAR_BACKWARD_CLOSURE;

  int res = 0;
  int max_k = get_bmc_pb_length(OptsHandler_get_instance());


  /* ----------------------------------------------------------------------- */
  /* Options handling: */
  opt_handling_res = Bmc_cmd_options_handling(argc, argv,
                                              Prop_Invar, &invarprop,
                                              &max_k, NULL,
                                              &algorithm_name,
                                              &closure_strategy_name,
                                              NULL, NULL);

  if (opt_handling_res == OUTCOME_SUCCESS_REQUIRED_HELP) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    return UsageBmcCheckInvarInc();
  }

  if (opt_handling_res != OUTCOME_SUCCESS) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    return 1;
  }

  /* makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    if (algorithm_name != (char*) NULL) FREE(algorithm_name);
    return 1;
  }

  /* ----------------------------------------------------------------------- */

  /* Checks algorithms: */
  if (algorithm_name == (char*) NULL) {
    algorithm_name = util_strsav((char*) get_bmc_inc_invar_alg(OptsHandler_get_instance()));
  }

  if ((strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_DUAL) != 0) &&
      (strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_FALSIFICATION) != 0) &&
      (strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_ZIGZAG) != 0)) {
    fprintf (nusmv_stderr,
             "'%s' is an invalid algorithm name.\n"
             "Valid names are "
             "'" BMC_INC_INVAR_ALG_DUAL "'"
             ", "
             "'" BMC_INVAR_ALG_FALSIFICATION "'"
             " and "
             "'" BMC_INC_INVAR_ALG_ZIGZAG "'.\n", algorithm_name);
    FREE(algorithm_name);
    return 1;
  }

  /* Checks closure strategy */
  if ((char *) NULL != closure_strategy_name) {

    if (strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_DUAL) != 0) {
      fprintf(nusmv_stderr,
              "Closure strategy can be specified for "
              "'" BMC_INC_INVAR_ALG_DUAL "'"
              " only.\n");
      FREE(closure_strategy_name);
      return 1;
    }

    if ((strcasecmp(closure_strategy_name, BMC_INVAR_BACKWARD) != 0) &&
        (strcasecmp(closure_strategy_name, BMC_INVAR_FORWARD)) != 0) {
    fprintf (nusmv_stderr,
             "'%s' is an invalid closure strategy name.\n"
             "Valid names are "
             "'" BMC_INVAR_BACKWARD "'"
             " and "
             "'" BMC_INVAR_FORWARD "'.\n", closure_strategy_name);
    FREE(closure_strategy_name);
    return 1;
    }
  }

 /* ----------------------------------------------------------------------- */

 /* Algorithm selection */
  if (!strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_DUAL)) {
    algorithm = ALG_DUAL;
  }
  else if (!strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_FALSIFICATION)) {
    algorithm = ALG_FALSIFICATION;
  }
  else if (!strcasecmp(algorithm_name, BMC_INC_INVAR_ALG_ZIGZAG)) {
    algorithm = ALG_ZIGZAG;
  }
  else {
    internal_error("%s:%d:%s unexpected algorithm specified (%s)",
                   __FILE__, __LINE__, __func__, algorithm_name);
  }
  FREE(algorithm_name);

  /* Closure strategy selection */
  if ((char*) NULL != closure_strategy_name) {
    if (!strcasecmp(closure_strategy_name, BMC_INVAR_BACKWARD)) {
      closure_strategy = BMC_INVAR_BACKWARD_CLOSURE;
    }
    else if (!strcasecmp(closure_strategy_name, BMC_INVAR_FORWARD)) {
      closure_strategy = BMC_INVAR_FORWARD_CLOSURE;
    }
    else {
      internal_error("%s:%d:%s unexpected closure strategy specified (%s)",
                     __FILE__, __LINE__, __func__, closure_strategy_name);
    }
    FREE(closure_strategy_name);
  }

  /* prepare the list of properties if no property was selected: */
  if (invarprop == PROP(NULL)) {
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Invar);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Invar);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      switch (algorithm) {
        case ALG_DUAL:
          res = Bmc_GenSolveInvarDual(prop, max_k, closure_strategy);
          break;

        case ALG_FALSIFICATION:
          res = Bmc_GenSolveInvarFalsification(prop, max_k);
          break;

        case ALG_ZIGZAG:
          res = Bmc_GenSolveInvarZigzag(prop, max_k);
          break;

        default: error_unreachable_code(); /* unexpected */
      }

      if (res != 0) break;
    } /* for loop */

    lsDestroy(props, NULL); /* the list is no longer needed */
  }
  else {
    switch (algorithm) {
      case ALG_DUAL:
        res = Bmc_GenSolveInvarDual(invarprop, max_k, closure_strategy);
        break;

      case ALG_FALSIFICATION:
        res = Bmc_GenSolveInvarFalsification(invarprop, max_k);
        break;

      case ALG_ZIGZAG:
        res = Bmc_GenSolveInvarZigzag(invarprop, max_k);
        break;

      default: error_unreachable_code(); /* unexpected */
    }
  }

  return res;
}
#endif


#if NUSMV_HAVE_INCREMENTAL_SAT
/**Function********************************************************************

  Synopsis           [Usage string for command check_invar_bmc_inc]

  Description        [The function is compiled only if there is at least
  one incremental SAT solver]

  SideEffects        [None]

  SeeAlso            [Bmc_CommandCheckInvarBmcInc]

******************************************************************************/
static int UsageBmcCheckInvarInc(void)
{
  fprintf(nusmv_stderr,
          "\nusage: check_invar_bmc_inc [-h | -n idx | -p \"formula\" | "
          "-P \"name\"] [-k max_len] [-a alg] [-s strategy]\n");
  fprintf(nusmv_stderr,
          "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr,
          "  -n idx\tChecks the INVAR property specified with <idx>"
          "\n\t\t(using incremental algorithms).\n");
  fprintf(nusmv_stderr,
          "  -P name\tChecks the INVAR property specified with <name>"
          "\n\t\t(using incremental algorithms).\n");
  fprintf(nusmv_stderr,
          "  -p \"formula\"\tChecks the specified INVAR propositional property"
          "\n\t\t(using incremental algorithms).\n");
  fprintf(nusmv_stderr,
          "\t\tIf no property is specified, checks all INVAR properties"
          "\n\t\t(using incremental algorithms).\n");

  fprintf(nusmv_stderr,
          "  -k max_len\tUpper bound for the search."
          "\n\t\tIf not specified, variable bmc_length is taken.\n");

  fprintf(nusmv_stderr, "  -a alg\tUses the specified algorithm.");

  fprintf(nusmv_stderr, "\n\t\tValid values are: "
          BMC_INC_INVAR_ALG_DUAL ", "
          BMC_INC_INVAR_ALG_ZIGZAG ", "
          BMC_INC_INVAR_ALG_FALSIFICATION
          "\n\t\tDefault value is taken from variable bmc_inc_invar_alg.\n");

  fprintf(nusmv_stderr,
          "  -s strategy\tUses the specified strategy for closure."
          "\n\t\t(currently this applies only to "
          BMC_INC_INVAR_ALG_DUAL ").\n");

  fprintf(nusmv_stderr, "\t\tValid names are "
          "'" BMC_INVAR_BACKWARD "'" " and " "'" BMC_INVAR_FORWARD "'.\n");

  fprintf(nusmv_stderr,
          "\t\tDefault value is '" BMC_INVAR_BACKWARD "'.\n");

  return 1;
}
#endif



/**Function********************************************************************

  Synopsis           [Top-level function for bmc of PSL properties]

  Description        [The parameters are:
  - prop is the PSL property to be checked
  - dump_prob is true if the problem must be dumped as DIMACS file (default filename
  from system corresponding variable)
  - inc_sat is true if incremental sat must be used. If there is no
  support for inc sat, an internal error will occur.
  - single_prob is true if k must be not incremented from 0 to k_max
    (single problem)
  - k and rel_loop are the bmc parameters.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int Bmc_check_psl_property(Prop_ptr prop,
                           boolean dump_prob,
                           boolean inc_sat,
                           boolean single_prob,
                           int k, int rel_loop)
{
  nusmv_assert(prop != PROP(NULL));
  nusmv_assert(Prop_get_type(prop) == Prop_Psl);

  /* checks the property is LTL compatible */
  if (!Prop_is_psl_ltl(prop)) {
    fprintf (nusmv_stderr, "BMC can be used only with Psl/ltl properies.\n");
    return 1;
  }

  /* BMC for ltl: makes sure bmc has been set up */
  if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
    return 1;
  }

  if (inc_sat) {
#if NUSMV_HAVE_INCREMENTAL_SAT
    return Bmc_GenSolveLtlInc(prop, k, rel_loop, !single_prob);
#else
    internal_error("Bmc_check_psl_property: Inc SAT Solving requested when not supported.\n");
#endif
  }

  return Bmc_GenSolveLtl(prop, k, rel_loop,
                         !single_prob, /* incrementally */
                         true, /* solve */
                         dump_prob ? BMC_DUMP_DIMACS : BMC_DUMP_NONE,
                         get_bmc_dimacs_filename(OptsHandler_get_instance()));
}




/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/




/**Function********************************************************************

  Synopsis [Bmc commands options handling for commands (optionally)
  acceping options -k -l -o -a -n -p -P -e]

  Description [ Output variables called res_* are pointers to
  variables that will be changed if the user specified a value for the
  corresponding option. For example if the user specified "-k 2", then
  *res_k will be assigned to 2. The caller can selectively choose which
  options can be specified by the user, by passing either a valid pointer
  as output parameter, or NULL to disable the corresponding option.
  For example by passing NULL as actual parameter of res_l, option -l will
  be not accepted.

  If both specified, k and l will be checked for mutual consistency.
  Loop will contain a relative value, like the one the user specified.

  prop_type is the expected property type, if specified.

  All integers values will not be changed if the corresponding options
  had not be specified by the user, so the caller might assign them to
  default values before calling this function.

  All strings will be allocated by the function if the corresponding
  options had been used by the user. In this case it is responsability
  of the caller to free them. Strings will be assigned to NULL if the
  user had not specified any corresponding option.

  Returns OUTCOME_GENERIC_ERROR if an error has occurred;
  Returns OUTCOME_SUCCESS_REQUIRED_HELP if -h options had been specified;
  Returns OUTCOME_SUCCESS in all other cases.
  ]

  SideEffects        [Result parameters might change]

  SeeAlso            []

******************************************************************************/
Outcome
Bmc_cmd_options_handling(int argc, char** argv,
                         Prop_Type prop_type,
                         /* output parameters: */
                         Prop_ptr* res_prop,
                         int* res_k,
                         int* res_l,
                         char** res_a,
                         char** res_s,
                         char** res_o,
                         boolean* res_e)
{
  int c;
  int prop_idx;
  char* formula_name = (char*) NULL;
  char* str_formula = (char*) NULL;
  char* str_loop = (char*) NULL;

  boolean k_specified = false;
  boolean l_specified = false;
  boolean e_specified = false;

  /* If one or more options are added here, the size of this array
     must be changed. At the momemt eight options are supported.  */
  char opt_string[9*2+1];


  /* ---------------------------------------------------------------------- */
  /* Fills up the string to pass to util_getopt, depending on which
     options are actually required */
  strcpy(opt_string, "h");  /* h is always needed */

  if (res_prop != (Prop_ptr*) NULL) {
    *res_prop = (Prop_ptr) NULL;
    strcat(opt_string, "n:p:P:");
  }
  if (res_k != (int*) NULL) strcat(opt_string, "k:");
  if (res_l != (int*) NULL) strcat(opt_string, "l:");
  if (res_a != (char**) NULL) {
    *res_a = (char*) NULL;
    strcat(opt_string, "a:");
  }
  if (res_s != (char**) NULL) {
    *res_s = (char*) NULL;
    strcat(opt_string, "s:");
  }

  if (res_o != (char**) NULL) {
    *res_o = (char*) NULL;
    strcat(opt_string, "o:");
  }

  if (res_e != (boolean*)NULL) {
    *res_e = false;
    strcat(opt_string, "e");
  }

  util_getopt_reset();
  while ((c = util_getopt((int)argc, (char**) argv, opt_string)) != EOF) {
    switch (c) {

    case 'h':
      return OUTCOME_SUCCESS_REQUIRED_HELP;

    case 'e':
      nusmv_assert(res_e != (boolean*) NULL);

      /* check if a value has already been specified: */
      if (e_specified) {
        fprintf(nusmv_stderr,
                "Option -e cannot be specified more than once.\n");
        return OUTCOME_GENERIC_ERROR;
      }
      *res_e = true;

      e_specified = true;
      break;


    case 'n':
      {
        char* str_prop_idx = (char*) NULL;

        nusmv_assert(res_prop != (Prop_ptr*) NULL);

        /* check if a formula has already been specified: */
        if ((*res_prop != PROP(NULL)) || (str_formula != (char*) NULL) || (formula_name != (char*) NULL)) {
          error_property_already_specified();
          return OUTCOME_GENERIC_ERROR;
        }

        str_prop_idx = util_strsav(util_optarg);

        /* check if property idx is ok */
        prop_idx = PropDb_get_prop_index_from_string(
                                    PropPkg_get_prop_database(), str_prop_idx);
        FREE(str_prop_idx);

        if (prop_idx == -1) {
          /* error messages have already been shown */
          return OUTCOME_GENERIC_ERROR;
        }

        /* here property idx is ok */
        *res_prop = PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                             prop_idx);
        if ( Prop_check_type(*res_prop, prop_type) != 0 ) {
          /* specified property's type is not what the caller expected */
          return OUTCOME_GENERIC_ERROR;
        }

        break;
     } /* case 'n' */

    case 'P':
      {
        nusmv_assert(res_prop != (Prop_ptr*) NULL);

        /* check if a formula has already been specified: */
        if ((*res_prop != PROP(NULL)) || (str_formula != (char*) NULL) || (formula_name != (char*) NULL)) {
          error_property_already_specified();
          return OUTCOME_GENERIC_ERROR;
        }

        formula_name = util_strsav(util_optarg);

        prop_idx = PropDb_prop_parse_name(PropPkg_get_prop_database(),
                                          formula_name);

        if (prop_idx == -1) {
          fprintf(nusmv_stderr, "No property named \"%s\"\n", formula_name);
          FREE(formula_name);
          /* error messages have already been shown */
          return OUTCOME_GENERIC_ERROR;
        }

        FREE(formula_name);

        /* here property idx is ok */
        *res_prop = PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                             prop_idx);
        if ( Prop_check_type(*res_prop, prop_type) != 0 ) {
          /* specified property's type is not what the caller expected */
          return OUTCOME_GENERIC_ERROR;
        }

        break;
      } /* case 'P' */

    case 'p':
      nusmv_assert(res_prop != (Prop_ptr*) NULL);

      /* check if a formula has already been specified: */
      if ((*res_prop != PROP(NULL)) || (str_formula != (char*) NULL) || (formula_name != (char*) NULL)) {
        error_property_already_specified();
        return OUTCOME_GENERIC_ERROR;
      }

      str_formula = util_strsav(util_optarg);
      break;

    case 'k':
      {
        char* str_k;
        int k;

        nusmv_assert(res_k != (int*) NULL);

        /* check if a value has already been specified: */
        if (k_specified) {
          fprintf(nusmv_stderr,
                  "Option -k cannot be specified more than once.\n");
          return OUTCOME_GENERIC_ERROR;
        }

        str_k = util_strsav(util_optarg);

        if (util_str2int(str_k, &k) != 0) {
          error_invalid_number(str_k);
          FREE(str_k);
          return OUTCOME_GENERIC_ERROR;
        }

        if (k < 0) {
          error_invalid_number(str_k);
          FREE(str_k);
          return OUTCOME_GENERIC_ERROR;
        }

        FREE(str_k);
        *res_k = k;
        k_specified = true;
        break;
      }

    case 'l':
      nusmv_assert(res_l != (int*) NULL);

      /* check if a value has already been specified: */
      if (l_specified) {
        fprintf(nusmv_stderr,
                "Option -l cannot be specified more than once.\n");
        return OUTCOME_GENERIC_ERROR;
      }


      str_loop = util_strsav(util_optarg);
      l_specified = true;
      /* checking of loopback value is delayed after command line
         processing to allow any -k option evaluation before (see the
         cheking code below) */
      break;

    case 'a':
      nusmv_assert(res_a != (char**) NULL);

      if (*res_a != (char*) NULL) {
        fprintf(nusmv_stderr, "Algorithm can be specified only once.\n\n");
        return OUTCOME_GENERIC_ERROR;
      }

      *res_a = util_strsav(util_optarg);
      break;

    case 's':
      nusmv_assert(res_s != (char**) NULL);

      if (*res_s != (char*) NULL) {
        fprintf(nusmv_stderr,
                "Closure strategy can be specified only once.\n\n");
        return OUTCOME_GENERIC_ERROR;
      }

      *res_s = util_strsav(util_optarg);
      break;

    case 'o':
      nusmv_assert(res_o != (char**) NULL);

      *res_o = util_strsav(util_optarg);
      break;

    default:  return OUTCOME_GENERIC_ERROR;

    } /* switch case */
  } /* end of cmd line processing */

  /* checks if there are unexpected options: */
  if (argc != util_optind) {
    fprintf(nusmv_stderr, "You specified one or more invalid options.\n\n");
    return OUTCOME_GENERIC_ERROR;
  }


  /* Checking of k,l constrains: */
  if (str_loop != (char*) NULL) {
    Outcome res;
    int rel_loop;

    rel_loop = Bmc_Utils_ConvertLoopFromString(str_loop, &res);

    if (res != OUTCOME_SUCCESS) {
      error_invalid_number(str_loop);
      FREE(str_loop);
      return OUTCOME_GENERIC_ERROR;
    }
    FREE(str_loop);

    if (Bmc_Utils_Check_k_l(*res_k,
                            Bmc_Utils_RelLoop2AbsLoop(rel_loop, *res_k))
        != OUTCOME_SUCCESS) {
      error_bmc_invalid_k_l(*res_k, rel_loop);
      return OUTCOME_GENERIC_ERROR;
    }

    *res_l = rel_loop;
  } /* k,l consistency check */


  /* Formula checking and commitment: */
  if (str_formula != (char*) NULL) {
    int idx;

    /* make sure bmc has been set up */
    if (Bmc_check_if_model_was_built(nusmv_stderr, false)) {
      FREE(str_formula);
      return OUTCOME_GENERIC_ERROR;
    }

    idx = PropDb_prop_parse_and_add(PropPkg_get_prop_database(),
                                    Compile_get_global_symb_table(),
                                    str_formula, prop_type);
    if (idx == -1) {
      FREE(str_formula);
      return OUTCOME_GENERIC_ERROR;
    }

    /* index is ok */
    nusmv_assert(*res_prop == PROP(NULL));
    *res_prop = PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                         idx);

    FREE(str_formula);
  } /* formula checking and commit */

  return OUTCOME_SUCCESS;
}




/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           []

  Description [Creates the BE fsm from the Sexpr FSM. Currently the be
  enc is a singleton global private variable which is shared between
  all the BE FSMs. If not previoulsy committed (because a boolean
  encoder was not available at the time due to the use of coi) the
  determinization layer will be committed to the be encoder]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
static void bmc_build_master_be_fsm()
{
  BeEnc_ptr be_enc;
  BeFsm_ptr fsm_be;

  /* builds the master bmc fsm: */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Building the BMC FSM... \n");
  }

  be_enc = Enc_get_be_encoding();

  if (SymbTable_get_layer(BaseEnc_get_symb_table(BASE_ENC(be_enc)),
                          INLINING_LAYER_NAME) != SYMB_LAYER(NULL)) {
    /* commits the determ layer if not previulsy committed */
    if (!BaseEnc_layer_occurs(BASE_ENC(be_enc), DETERM_LAYER_NAME)) {
      BaseEnc_commit_layer(BASE_ENC(be_enc), DETERM_LAYER_NAME);
    }

    /* commits the inlining layer if not previulsy committed */
    if (!BaseEnc_layer_occurs(BASE_ENC(be_enc), INLINING_LAYER_NAME)) {
      BaseEnc_commit_layer(BASE_ENC(be_enc), INLINING_LAYER_NAME);
    }
  }

  /* :WARNING: The FSM is currently destroyed by the package 'prop',
     but it is built here! */
  fsm_be = BeFsm_create_from_sexp_fsm(be_enc,
               PropDb_master_get_bool_sexp_fsm(PropPkg_get_prop_database()));
  PropDb_master_set_be_fsm(PropPkg_get_prop_database(), fsm_be);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Done \n");
  }
}


/**Function********************************************************************

  Synopsis           [A service for commands, to check if bmc
  has been built]

  Description [If coi is not enabled than bmc must be set up,
  otherwise it is only required bmc to have initialized. Returns 1 if
  the execution should be stopped, and prints an error message if it
  is the case (to the given optional file). If everything is fine,
  returns 0 and prints nothing. If 'forced' is true, than the model is
  required to be built even if coi is enabled, and a message is
  printed accordingly (used by the commands that always require that
  the model is built (e.g. bmc_simulate).]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
int Bmc_check_if_model_was_built(FILE* err, boolean forced)
{
  if (cmp_struct_get_bmc_setup(cmps)) return 0;

  if (cmp_struct_get_bmc_init(cmps) && opt_cone_of_influence(OptsHandler_get_instance()) &&
      !forced) return 0;

  if (Compile_check_if_bool_model_was_built(nusmv_stderr, forced)) return 1;

  if (err != (FILE*) NULL) {
    fprintf (err, "Bmc must be setup before. Use the command \"");

    if (forced && opt_cone_of_influence(OptsHandler_get_instance())) {
      fprintf (err, "bmc_setup -f\" as Cone Of Influence is enabled.\n");
    }
    else fprintf (err, "bmc_setup\".\n");
  }

  return 1;
}

