/**CFile***********************************************************************

  FileName    [simulateCmd.c]

  PackageName [simulate]

  Synopsis    [Model Checker Simulator Commands]

  Description [This file contains commands to be used for the simulation feature.]

  SeeAlso     [simulate.c]

  Author      [Andrea Morichetti]

  Copyright   [
  This file is part of the ``simulate'' package of NuSMV version 2.
  Copyright (C) 1998-2001 by CMU and FBK-irst.

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

#include "simulateInt.h"
#include "utils/error.h" /* for CATCH */
#include "utils/utils_io.h"
#include "prop/Prop.h"
#include "prop/PropDb.h"
#include "prop/propPkg.h"
#include "parser/symbols.h"
#include "parser/parser.h"
#include "compile/symb_table/SymbTable.h"
#include "utils/error.h"
#include "mc/mc.h"
#include "enc/enc.h"
#include "trace/pkg_trace.h"
#include "cmd/cmd.h"
#include "utils/ucmd.h"

static char rcsid[] UTIL_UNUSED = "$Id: simulateCmd.c,v 1.14.2.26.4.8.4.15 2010-02-23 07:06:22 nusmv Exp $";


/* Prototypes of command functions */
int CommandSimulate   ARGS((int argc, char **argv));
int CommandPickState  ARGS((int argc, char **argv));
int CommandGotoState ARGS((int argc, char **argv));
int CommandPrintCurrentState ARGS((int argc, char **argv));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [It's used to store the current state when the interactive
  model inspection feature is enabled.]

  Description [It's used to store the current state when the interactive
  model inspection feature is enabled.]

******************************************************************************/
static bdd_ptr current_state_bdd = (bdd_ptr) NULL;


/**Variable********************************************************************

  Synopsis    [It's used to store the current state label when the interactive
  model inspection feature is enabled.]

  Description [It's used to store the current state label when the interactive
  model inspection feature is enabled.]

******************************************************************************/
static TraceLabel current_state_label = TRACE_LABEL_INVALID;


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageSimulate   ARGS((void));
static int UsagePickState  ARGS((void));
static int UsageGotoState  ARGS((void));
static int UsagePrintCurrentState ARGS((void));
static boolean current_state_exist ARGS((void));
static void current_state_bdd_set ARGS((bdd_ptr state));
static boolean current_state_bdd_exist ARGS((void));
static TraceLabel current_state_label_get ARGS((void));
static void current_state_label_set ARGS((TraceLabel label));
static boolean current_state_label_exist ARGS((void));

static void
simulate_extend_print_curr_trace ARGS((BddEnc_ptr enc, node_ptr fragment,
                                       boolean printyesno, 
                                       boolean only_changes,
                                       NodeList_ptr symbols));

static void current_state_label_reset ARGS((void));
static void current_state_bdd_free ARGS((void));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the simulate package.]

  Description        [Initializes the simulate package.]

  SideEffects        []

******************************************************************************/
void Simulate_Init(void)
{
  Cmd_CommandAdd("simulate", CommandSimulate, 0, true);
  Cmd_CommandAdd("pick_state", CommandPickState, 0, true);
  Cmd_CommandAdd("goto_state", CommandGotoState, 0, true);
  Cmd_CommandAdd("print_current_state", CommandPrintCurrentState, 0, true);
}

/**Function********************************************************************

  Synopsis           [Quits the simulate package]

  Description        [Quits the simulate package]

  SideEffects        []

******************************************************************************/
void Simulate_End(void)
{
  current_state_bdd_free();
  current_state_label_reset();

  /* free the buffers */
  if (simulation_buffer_size != 0) {
    simulation_buffer_size = 0;
    FREE(simulation_buffer);
    simulation_buffer = NULL;
  }
}

/**Function********************************************************************

  Synopsis           [Picks a state from the set of initial states]

  CommandName        [pick_state]

  CommandSynopsis    [Picks a state from the set of initial states]

  CommandArguments   [\[-h\] \[-v\] \[-r | -i \[-a\]\] \[-c "constraints" | -s trace.state\]]

  CommandDescription [

  Chooses an element from the set of initial states, and makes it the
  <tt>current state</tt> (replacing the old one). The chosen state is
  stored as the first state of a new trace ready to be lengthened by
  <tt>steps</tt> states by the <tt>simulate</tt> command. The state can be
  chosen according to different policies which can be specified via command
  line options. By default the state is chosen in a deterministic way.
  <p>
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
       <dd> Enables the user to interactively pick up an initial state. The
       user is requested to choose a state from a list of possible items
       (every item in the list doesn't show state variables unchanged with
       respect to a previous item). If the number of possible states is too
       high, then the user has to specify some further constraints as
       "simple expression".
    <dt> <tt>-a</tt>
       <dd> Displays all state variables (changed and unchanged with respect
       to a previous item) in an interactive picking. This option
       works only if the <tt>-i</tt> options has been specified.
    <dt> <tt>-c "constraints"</tt>
       <dd> Uses <tt>constraints</tt> to restrict the set of initial states
       in which the state has to be picked.
    <dt> <tt>-s trace.state</tt>
       <dd> Picks state from trace.state label. A new simulation trace will
       be created by copying prefix of the source trace up to specified state.
  </dl> ]

  SideEffects        [The state chosen is stored in the traces_hash table as
  the first state of a new trace]

  SeeAlso            [goto_state simulate]

******************************************************************************/
int CommandPickState(int argc, char ** argv)
{
  int res = 1;
  int c = 0;
  boolean verbose = false;
  int display_all = 0;
  char *strConstr = NIL(char);
  char *strLabel = NIL(char);
  Simulation_Mode mode = Deterministic;
  short int usedMode = 0;
  TraceManager_ptr gtm = TracePkg_get_global_trace_manager();
  int tr_number;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hriavc:s:")) != EOF) {
    switch(c){
    case 'h': return UsagePickState();
    case 'r':
      if (++usedMode > 1) goto simulate_cmd_pick_state_usage;
      mode = Random;
      break;
    case 'i':
      if (++usedMode > 1) goto simulate_cmd_pick_state_usage;
      mode = Interactive;
      break;
    case 'a':
      display_all = 1;
      break;
    case 'v':
      verbose = true;
      break;
    case 'c':
      strConstr = util_strsav(util_optarg);
      break;
    case 's':
      strLabel = util_strsav(util_optarg);
      break;
    default:
      goto simulate_cmd_pick_state_usage;
    }
  }

  if ((mode != Interactive) && (display_all == 1))
    goto simulate_cmd_pick_state_usage;

  if (argc != util_optind)
    goto simulate_cmd_pick_state_usage;

  /* pre-conditions */
  if (Compile_check_if_model_was_built(nusmv_stderr, true))
    goto simulate_cmd_pick_state_free;

  if (strLabel != (char*) NULL) {
    TraceLabel label;

    if (strConstr != (char*) NULL) {
      fprintf(nusmv_stderr,
              "Options -c and -s cannot be used at the same time\n");
      res = 1;
      goto simulate_cmd_pick_state_free;
    }

    label = TraceLabel_create_from_string(strLabel);
    if (label == TRACE_LABEL_INVALID ||   \
        !TraceManager_is_label_valid(gtm, label)) {

      fprintf(nusmv_stderr, "Label \"%s\" is invalid\n", strLabel);
      res = 1;
      goto simulate_cmd_pick_state_free;
    }

    { /* constructs a new trace from given label */
      Trace_ptr from_trace = \
        TraceManager_get_trace_at_index(gtm, TraceLabel_get_trace(label));
      TraceIter iter = TraceManager_get_iterator_from_label(gtm, label);

      Trace_ptr new_trace  = TRACE(NULL);
      bdd_ptr state;
      TraceLabel new_label;
      BddEnc_ptr enc = Enc_get_bdd_encoding();

      /* create new simulation trace from previous one */
      new_trace = Trace_copy(from_trace, iter, false);

      Trace_set_desc(new_trace, "Simulation Trace");
      Trace_set_type(new_trace, TRACE_TYPE_SIMULATION);

      tr_number = TraceManager_register_trace(gtm, new_trace);
      TraceManager_set_current_trace_number(gtm, tr_number);

      /* Now the new label we get would be the label of the next
       * trace that is going to be registered. */
      new_label = TraceLabel_create(
           TraceManager_get_size(gtm),
           TraceManager_get_abs_index_from_label(gtm, label));

      state = TraceUtils_fetch_as_bdd(from_trace, iter,
                                      TRACE_ITER_SF_VARS, enc);

      current_state_set(state, new_label);
    }
  }
  else {
    tr_number = Simulate_CmdPickOneState(PropDb_master_get_bdd_fsm(
                                              PropPkg_get_prop_database()),
                                         mode, display_all, strConstr);
  }

  /* results presentation */
  if (tr_number != -1) {
    if (verbose) {
      TraceManager_execute_plugin(gtm, TRACE_OPT(NULL),
                                  TRACE_MANAGER_DEFAULT_PLUGIN,
                                  TRACE_MANAGER_LAST_TRACE);
    }
    /* Everything went ok */
    res = 0;
  }
  else {
    if ((char*) NULL == strConstr) {
      fprintf(nusmv_stderr, "No trace: initial state is inconsistent\n");
    }
    else {
      fprintf(nusmv_stderr,
              "No trace: constraint and initial state are inconsistent\n");
    }
    res = 1;
  }

  goto simulate_cmd_pick_state_free;

 simulate_cmd_pick_state_usage:
  res = UsagePickState();

 simulate_cmd_pick_state_free:
  if (NIL(char) != strLabel) { FREE(strLabel); }
  if (NIL(char) != strConstr) { FREE(strConstr); }

  return res;
}

static int UsagePickState () {
  fprintf(nusmv_stderr, "usage: pick_state [-h] [-v] [-r | -i [-a]] [-c \"constr\" | -s trace.state]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -v \t\tVerbosely prints picked state.\n");
  fprintf(nusmv_stderr, "  -r \t\tRandomly picks a state from the set of the initial states\n");
  fprintf(nusmv_stderr, "     \t\t(otherwise choice is deterministic).\n");
  fprintf(nusmv_stderr, "  -i \t\tLets the user interactively pick a state from\n");
  fprintf(nusmv_stderr, "     \t\tthe set of initial ones.\n");
  fprintf(nusmv_stderr, "  -a \t\tDisplays all the state variables (changed and\n");
  fprintf(nusmv_stderr, "   \t\tunchanged) in an interactive session.\n");
  fprintf(nusmv_stderr, "   \t\tIt works only together with -i option.\n");
  fprintf(nusmv_stderr, "  -c \"constr\"   Sets constraints for the initial set of states.\n");
  fprintf(nusmv_stderr, "  -s state\tPicks state from trace.state label.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Performs a simulation from the current selected state]

  SideEffects        [Generated referenced states traces are stored to be
  analyzed by the user in a second time]

  SeeAlso            [pick_state goto_state]

  CommandName        [simulate]

  CommandSynopsis    [Performs a simulation from the current selected state]

  CommandArguments   [\[-h\] \[-p | -v\] \[-r | -i \[-a\]\]
  [\[-c "constraints"\] | \[-t "constraints"\] ] \[-k steps\]
  ]

  CommandDescription [
  Generates a sequence of at most <tt>steps</tt> states (representing a
  possible execution of the model), starting from the <em>current state</em>.
  The current state must be set via the <em>pick_state</em> or
  <em>goto_state</em> commands.<p>
  It is possible to run the simulation in three ways (according to different
  command line policies):
  deterministic (the default mode), random and interactive.<p>
  The resulting sequence is stored in a trace indexed with an integer number
  taking into account the total number of traces stored in the system. There is
  a different behavior in the way traces are built, according to how
  <em>current state</em> is set: <em>current state</em> is always put at
  the beginning of a new trace (so it will contain at most <it>steps + 1</it>
  states) except when it is the last state of an existent old trace.
  In this case the old trace is lengthened by at most <it>steps</it> states.
  <p>
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
       <dd> Enables the user to interactively choose every state of the trace,
       step by step. If the number of possible states is too high, then
       the user has to specify some constraints as simple expression.
       These constraints are used only for a single simulation step and
       are <em>forgotten</em> in the following ones. They are to be intended
       in an opposite way with respect to those constraints eventually entered
       with the pick_state command, or during an interactive simulation
       session (when the number of future states to be displayed is too high),
       that are <em>local</em> only to a single step of the simulation and
       are <em>forgotten</em> in the next one.
    <dt> <tt>-a</tt>
       <dd> Displays all the state variables (changed and unchanged) during
       every step of an interactive session. This option works only if the
       <tt>-i</tt> option has been specified.
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
    </dl> ]

******************************************************************************/
int CommandSimulate(int argc, char **argv)
{
  BddEnc_ptr enc;
  DdManager* dd;
  bdd_ptr bdd_constraints = (bdd_ptr) NULL;
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
  while((c = util_getopt(argc,argv,"t:c:hpvriak:")) != EOF){
    switch(c){
    case 'h': return UsageSimulate();
    case 'p':
      if (printrace == true) return UsageSimulate();
      printrace = true;
      only_changes = true;
      break;
    case 'v':
      if (printrace == true) return UsageSimulate();
      printrace = true;
      only_changes = false;
      break;
    case 'r':
      if (mode == Interactive) return UsageSimulate();
      mode = Random;
      break;
    case 'i':
      if (mode == Random) return UsageSimulate();
      mode = Interactive;
      break;
    case 'a':
      display_all = 1;
      break;
    case 'c':
      if (NIL(char) != strConstr) return UsageSimulate();
      strConstr = util_strsav(util_optarg);
      isconstraint = true;
      time_shift = true;
      break;
    case 't':
      if (NIL(char) != strConstr) return UsageSimulate();
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
      return UsageSimulate();
    }
  }

  if ((mode != Interactive) && (display_all == 1)) return UsageSimulate();

  if (argc == util_optind + 1) {
    char* strNumber;

    fprintf(nusmv_stderr, "*** Warning: Parameter \"steps\" is deprecated. "
            "Use option \"-k\" instead\n");

    if (k_specified) {
      fprintf(nusmv_stderr, "Error: Parameter \"steps\" conflicts with option -k\n");
      return 1;
    }

    strNumber = util_strsav(argv[util_optind]);

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
  }
  else if (argc != util_optind) {
    return UsageSimulate();
  }

  /* pre-conditions */
  if (Compile_check_if_model_was_built(nusmv_stderr, true)) return 1;

  if (!(current_state_exist())) {
    fprintf(nusmv_stderr,
            "No current state set. Use the \"pick_state\" command.\n");
    return 1;
  }

  enc = Enc_get_bdd_encoding();
  dd = BddEnc_get_dd_manager(enc);

  if (isconstraint == true) {
    bdd_constraints = simulate_get_constraints_from_string(strConstr, enc,
                                                           !time_shift,
                                                           true /*inputs*/);
    FREE(strConstr);
    if (bdd_constraints == (bdd_ptr) NULL) return 1; /* error */
  }
  else bdd_constraints = bdd_true(dd);

  {
    SexpFsm_ptr sexp_fsm; /* needed for trace lanugage */
    sexp_fsm = \
        PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());

    node_ptr current_trace = Nil;
    BddFsm_ptr fsm;
    TraceLabel curr_lbl;

    fsm = PropDb_master_get_bdd_fsm(PropPkg_get_prop_database());

    curr_lbl = current_state_label_get();
    nusmv_assert(curr_lbl != TRACE_LABEL_INVALID);

    fprintf( nusmv_stdout, "********  Simulation Starting From State %d.%d "
             "  ********\n",
             TraceLabel_get_trace(curr_lbl) + 1,
             TraceLabel_get_state(curr_lbl) + 1);

    /* Important: the simulation ALWAYS starts from the current selected state */
    current_trace = Simulate_MultipleSteps(fsm, bdd_constraints, time_shift,
                                           mode, steps, display_all);
    if (current_trace == Nil) {
      bdd_free(dd, bdd_constraints);
      return 1;
    }

    /* extends and prints the current simulation trace */
    simulate_extend_print_curr_trace(enc, current_trace, printrace,
                                     only_changes,
                                     SexpFsm_get_symbols_list(sexp_fsm));

    /* Update the current state. */
    {
      int trace_id;
      Trace_ptr curr_trace;
      BddStates curr_state;
      TraceLabel new_label;

      trace_id = TraceManager_get_current_trace_number(global_trace_manager);

      curr_trace = TraceManager_get_trace_at_index(global_trace_manager,
                                                   trace_id);

      new_label = TraceLabel_create(trace_id, Trace_get_length(curr_trace));

      curr_state = TraceUtils_fetch_as_bdd(curr_trace,
                                           Trace_last_iter(curr_trace),
                                           TRACE_ITER_SF_VARS, enc);

      current_state_set(curr_state, new_label);

      bdd_free(BddEnc_get_dd_manager(enc),curr_state);
    }

    walk_dd(dd, bdd_free, current_trace);
    bdd_free(dd, bdd_constraints);
  }
  return 0;
} /* Command Simulate */

static int UsageSimulate(){
  fprintf(nusmv_stderr,
          "usage: simulate [-h] [-p | -v] [-r | -i [-a]] [[-c \"constr\"] "
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

  Synopsis           [Goes to a given state of a trace]

  CommandName        [goto_state]

  CommandSynopsis    [Goes to a given state of a trace]

  CommandArguments   [\[-h\] state]

  CommandDescription [Makes <tt>state</tt> the <em>current
  state</em>. This command is used to navigate alongs traces
  produced by NuSMV. During the navigation, there is a <em>current
  state</em>, and the <em>current trace</em> is the trace the
  <em>current state</em> belongs to.
    Command options:<p>
    <dl>
      <dt><tt>state: </tt>
      <dd> The state of a trace (trace.state) to be picked.
    </dl>
    ]

  SideEffects        [<em>state</em> became the current state.]

******************************************************************************/
int CommandGotoState(int argc, char **argv)
{
  int c;
  int status = 0;
  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"h")) != EOF) {
    switch (c) {
    case 'h': return UsageGotoState();
    default:  return UsageGotoState();
    }
  }
  if (argc == 1) return UsageGotoState();

  /* pre-conditions */
  if (Compile_check_if_model_was_built(nusmv_stderr, true)) return 1;

  {
    TraceLabel label;

    argv += util_optind-1;
    argc -= util_optind-1;
    label = TraceLabel_create_from_string(argv[1]);

    if (label != TRACE_LABEL_INVALID) {
      if (TraceManager_is_label_valid(global_trace_manager, label)) {
        Trace_ptr from_trace, new_trace;
        TraceIter iter;
        bdd_ptr state;
        int new_trace_id;
        node_ptr new_label;
        int from_trace_no = TraceLabel_get_trace(label);
        BddEnc_ptr enc = Enc_get_bdd_encoding();
        PropDb_ptr propDB = PropPkg_get_prop_database();
        SexpFsm_ptr scalar_fsm = PropDb_master_get_scalar_sexp_fsm(propDB);

        from_trace = TraceManager_get_trace_at_index(global_trace_manager,
                                                     from_trace_no);
        iter = TraceManager_get_iterator_from_label(global_trace_manager,
                                                    label);

        state = TraceUtils_fetch_as_bdd(from_trace, iter,
                                        TRACE_ITER_SF_VARS, enc);

        /* create new trace copying from given one up to given iter */
        new_trace = Trace_copy(from_trace, iter, false);
        Trace_set_desc(new_trace, "Simulation Trace");
        Trace_set_type(new_trace, TRACE_TYPE_SIMULATION);

        /* Now the new label we get would be the label of the next
         * trace that is going to be registered. */
        new_label = TraceLabel_create(
           TraceManager_get_size(global_trace_manager),
           TraceManager_get_abs_index_from_label(global_trace_manager, label));

        new_trace_id = TraceManager_register_trace(global_trace_manager,
                                                   new_trace);
        TraceManager_set_current_trace_number(global_trace_manager,
                                              new_trace_id);
        current_state_set(state, new_label);

        fprintf(nusmv_stdout, "The current state for new trace is:\n");

        fprintf(nusmv_stdout, "-> State %d.%d <-\n",
                TraceLabel_get_trace(new_label)+1,
                TraceLabel_get_state(new_label)+1);

        BddEnc_print_bdd_begin(enc, SexpFsm_get_vars_list(scalar_fsm), true);
        set_indent_size(2);
        BddEnc_print_bdd(enc, state, (VPFNNF) NULL, nusmv_stdout);
        reset_indent_size();
        BddEnc_print_bdd_end(enc);
      }
      else {
        fprintf(nusmv_stderr, "The label %d.%d is invalid.\n",
                TraceLabel_get_trace(label) + 1,
                TraceLabel_get_state(label) + 1);
      }
    }
    else {
      fprintf(nusmv_stderr, "Parsing error: expected "                  \
              "\"goto_state <trace_number>.<state_number>\".\n");
      status = 1;
    }
  }
  return status;
}

static int UsageGotoState()
{
  fprintf(nusmv_stderr, "usage: goto_state [-h] state\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   state \tSets current state to \"state\".\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Prints the current state]

  CommandName        [print_current_state]

  CommandSynopsis    [Prints out the current state]

  CommandArguments   [\[-h\] \[-v\]]

  CommandDescription [Prints the name of the <em>current state</em> if
  defined.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-v</tt>
       <dd> Prints the value of all the state variables of the <em>current
       state</em>.
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandPrintCurrentState(int argc, char ** argv)
{
  int c;
  int Verbosely = 1;
  PropDb_ptr propDB;
  SexpFsm_ptr scalar_fsm;

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hv")) != EOF) {
    switch (c) {
    case 'h': return UsagePrintCurrentState();
    case 'v': {
      Verbosely = 0;
      break;
    }
    default:  return UsagePrintCurrentState();
    }
  }

  if (argc != util_optind) return UsagePrintCurrentState();

  propDB = PropPkg_get_prop_database();
  scalar_fsm = PropDb_master_get_scalar_sexp_fsm(propDB);

  if ((current_state_label != TRACE_LABEL_INVALID) &&
      (current_state_bdd != (bdd_ptr)NULL)) {
    BddEnc_ptr enc = Enc_get_bdd_encoding();

    fprintf(nusmv_stdout, "Current state is %d.%d\n",
            TraceLabel_get_trace(current_state_label) + 1,
            TraceLabel_get_state(current_state_label) + 1);

    if (Verbosely == 0) {
      BddEnc_print_bdd_begin(enc, SexpFsm_get_vars_list(scalar_fsm), false);
      BddEnc_print_bdd(enc, current_state_bdd, (VPFNNF) NULL, nusmv_stdout);
      BddEnc_print_bdd_end(enc);
    }
  }
  else {
    if (TraceManager_get_current_trace_number(global_trace_manager) >= 0) {
      fprintf(nusmv_stdout, "The current state has not yet been defined.\n");
      fprintf(nusmv_stdout,
              "Use \"goto_state\" to define the current state.\n");
    }
    else {
      fprintf(nusmv_stdout,
              "There is no trace actually stored in the system.\n");
      fprintf(nusmv_stdout,
              "Use \"pick_state\" to define the current state.\n");
    }
    return 1;
  }
  return 0;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

bdd_ptr current_state_bdd_get()
{
  return (current_state_exist() == true) ?
    bdd_dup(current_state_bdd) : (bdd_ptr) NULL;
}

void current_state_set(bdd_ptr state, TraceLabel label)
{
  current_state_bdd_set(state);
  current_state_label_set(label);
}

static void current_state_label_reset()
{
  if (current_state_label_exist()) {
    current_state_label = TRACE_LABEL_INVALID;
  }
}

static void current_state_bdd_free()
{
  if (current_state_bdd_exist()) {
    bdd_free(dd_manager, current_state_bdd);
    current_state_bdd = (bdd_ptr) NULL;
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

static int UsagePrintCurrentState()
{
  fprintf(nusmv_stderr, "usage: print_current_state [-h] [-v]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr,
          "   -v \t\tPrints the value of each state variable in the current state.\n");
  return 1;
}


static boolean current_state_exist(void)
{
  return (current_state_label_exist() && current_state_bdd_exist());
}

static void current_state_bdd_set(bdd_ptr state)
{
  if (current_state_exist()) { current_state_bdd_free(); }
  current_state_bdd = bdd_dup(state);
}

static boolean current_state_bdd_exist(void)
{
  return (current_state_bdd != (bdd_ptr) NULL);
}

static TraceLabel current_state_label_get(void)
{
  return (current_state_exist() == true) ?
    current_state_label : TRACE_LABEL(NULL);
}

static void current_state_label_set(TraceLabel label)
{
  current_state_label = label;
}

static boolean current_state_label_exist(void)
{
  return (current_state_label != TRACE_LABEL_INVALID);
}


/**Function********************************************************************

  Synopsis     [Extends current simulation trace and prints it]

  Description  [Extends current simulation trace by creating a new
                trace for simulation fragment and concatenating it to
                existing one.]

                The trace is printed it if the variable printyesno is
                true (this is set by the user via the command simulate
                options -p or -v). It returns the index of the stored
                trace inside the trace-manager.]

  SideEffects  []

  SeeAlso      [Trace_concat]

******************************************************************************/
static void simulate_extend_print_curr_trace(BddEnc_ptr enc,
                                             node_ptr fragment,
                                             boolean printyesno,
                                             boolean only_changes,
                                             NodeList_ptr symbols)
{
  Trace_ptr trace;
  Trace_ptr extension;

  unsigned prev_length;

  trace = \
    TraceManager_get_trace_at_index(global_trace_manager,
                    TraceManager_get_current_trace_number(global_trace_manager));

  prev_length = Trace_get_length(trace);

  /*  extend simulation trace */
  extension = \
    Mc_create_trace_from_bdd_state_input_list(enc, symbols, NIL(char),
                                              TRACE_TYPE_UNSPECIFIED,
                                              fragment);

  /* extend existing simulation trace */
  trace = Trace_concat(trace, &extension);
  nusmv_assert(TRACE(NULL) == extension);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1) && printyesno) {

    fprintf(nusmv_stdout,
            "#####################################################\n"
            "######         Print Of Current Trace         #######\n"
            "#####################################################\n");
  }

  if (printyesno) {
    TracePlugin_ptr plugin;

    /* only the TraceExplainer plugin can be used here: */

    if (only_changes) {
      plugin = TraceManager_get_plugin_at_index(global_trace_manager, 0);
    }
    else {
      plugin = TraceManager_get_plugin_at_index(global_trace_manager, 1);
    }

    {
      TraceOpt_ptr trace_opt = \
        TraceOpt_create_from_env(OptsHandler_get_instance());

      TraceOpt_set_from_here(trace_opt, prev_length);
      TracePlugin_action(plugin, trace, trace_opt);

      TraceOpt_destroy(trace_opt);
    }
  } /* if printyesno */

  return ;
}
