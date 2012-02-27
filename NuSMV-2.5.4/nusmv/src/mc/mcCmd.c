/**CFile***********************************************************************

  FileName    [mcCmd.c]

  PackageName [mc]

  Synopsis    [Model checking commands.]

  Description [This file contains all the shell command to deal with
  model checking and for counterexample navigation.]

  SeeAlso     [cmdCmd.c]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``mc'' package of NuSMV version 2.
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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "mc.h"
#include "mcInt.h"
#include "parser/symbols.h"
#include "utils/error.h" /* for CATCH */
#include "prop/Prop.h"
#include "prop/PropDb.h"
#include "prop/propPkg.h"
#include "cmd/cmd.h"
#include "utils/utils_io.h"
#include "trace/Trace.h"
#include "trace/TraceManager.h"
#include "enc/enc.h"
#include "utils/ucmd.h" /* for util_str2int */
#include "bmc/bmcUtils.h" /* for Bmc_Utils_ConvertLoopFromString */
#include "bmc/bmcBmc.h"
#include "bmc/sbmc/sbmcCmd.h"
#include "fsm/bdd/bdd.h" /* to check preconditions for EL_fwd */

static char rcsid[] UTIL_UNUSED = "$Id: mcCmd.c,v 1.23.2.18.2.1.2.12.4.14 2009-12-10 16:21:53 nusmv Exp $";

/* prototypes of the command functions */
int CommandCheckCtlSpec ARGS((int argc, char **argv));
int CommandCheckInvar ARGS((int argc, char **argv));
int CommandCheckCompute ARGS((int argc, char **argv));
int CommandCheckPslSpec ARGS((int argc, char **argv));
int CommandLanguageEmptiness ARGS((int argc, char **argv));

int CommandCompute ARGS((int argc, char **argv));
/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int CommandCheckSpec ARGS((int argc, char **argv));
static int UsageCheckCtlSpec ARGS((void));
static int UsageCheckInvar ARGS((void));
static int UsageCheckCompute ARGS((void));
static int UsageCheckPslSpec ARGS((void));
static int UsageLanguageEmptiness ARGS((void));

static int UsageCompute ARGS((void));
static int mc_cmd_check_compute ARGS((int argc, char **argv,
                                      int (*usage_fun)(void)));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the mc package.]

  Description        [Initializes the mc package.]

  SideEffects        []

******************************************************************************/
void Mc_Init(void){
  Cmd_CommandAdd("check_spec", CommandCheckSpec, 0, true);
  Cmd_CommandAdd("check_ctlspec", CommandCheckCtlSpec, 0, true);
  Cmd_CommandAdd("check_invar", CommandCheckInvar, 0, true);
  Cmd_CommandAdd("check_compute", CommandCheckCompute, 0, true);
  Cmd_CommandAdd("check_pslspec", CommandCheckPslSpec, 0, true);
  Cmd_CommandAdd("_language_emptyness", CommandLanguageEmptiness, 0, true);

  /* This one is deprecated */
  Cmd_CommandAdd("compute", CommandCompute, 0, true);
}

/**Function********************************************************************

  Synopsis           [Quit the mc package]

  Description        [Quit the mc package]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Mc_End(void)
{ }


/**Function********************************************************************

  Synopsis           [Top-level function for mc of PSL properties]

  Description        [The parameters are:
  - prop is the PSL property to be checked
  ]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int Mc_check_psl_property(Prop_ptr prop)
{
  int status = 0;
  nusmv_assert(prop != PROP(NULL));
  nusmv_assert(Prop_get_type(prop) == Prop_Psl);

  if (!cmp_struct_get_build_model(cmps) && !opt_cone_of_influence(OptsHandler_get_instance())) {
    fprintf(nusmv_stderr,
            "The current partition method %s has not yet be computed.\n",
            TransType_to_string(get_partition_method(OptsHandler_get_instance())));
    fprintf(nusmv_stderr,
            "Use \t \"build_model -f -m %s\"\nto build the transition " \
            "relation.\n",
            TransType_to_string(get_partition_method(OptsHandler_get_instance())));
    return 1;
  }

  /* Performs mc with bdd technique */
  CATCH { Prop_verify(prop); }
  FAIL { status = 1; }

  return status;
}


/**Function********************************************************************

  Synopsis           [Performs fair CTL model checking.]

  CommandName        [check_ctlspec]

  CommandSynopsis    [Performs fair CTL model checking.]

  CommandArguments   [\[-h\] \[-m | -o output-file\] \[-n number | -p "ctl-expr \[IN context\]" | -P "name"\]]

  CommandDescription [Performs fair CTL model checking.<p>

  A <tt>ctl-expr</tt> to be checked can be specified at command line
  using option <tt>-p</tt>. Alternatively, option <tt>-n</tt> or
  <tt>-P</tt> can be used for checking a particular formula in the
  property database. If neither <tt>-n</tt> nor <tt>-p</tt> are used,
  all the SPEC formulas in the database are checked.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-m</tt>
       <dd> Pipes the output generated by the command in processing
           <tt>SPEC</tt>s to the program specified by the
           <tt>PAGER</tt> shell variable if defined, else
           through the <tt>UNIX</tt> command "more".
    <dt> <tt>-o output-file</tt>
       <dd> Writes the output generated by the command in processing
           <tt>SPEC</tt>s to the file <tt>output-file</tt>.
    <dt> <tt>-p "ctl-expr \[IN context\]"</tt>
       <dd> A CTL formula to be checked. <tt>context</tt> is the module
       instance name which the variables in <tt>ctl-expr</tt> must
       be evaluated in.
    <dt> <tt>-n number</tt>
       <dd> Checks the CTL property with index <tt>number</tt> in the property
            database.
    <dt> <tt>-P name</tt>
       <dd> Checks the CTL property with name <tt>name</tt> in the property
            database.
  </dl><p>

  If the <tt>ag_only_search</tt> environment variable has been set, and
  the set of reachable states has already been computed, then a
  specialized algorithm to check AG formulas is used instead of the
  standard model checking algorithms.]

  SideEffects        []

******************************************************************************/
int CommandCheckCtlSpec(int argc, char **argv)
{
  int c;
  int prop_no = -1;
  char* formula = NIL(char);
  char* formula_name = NIL(char);
  int status = 0;
  int useMore = 0;
  char* dbgFileName = NIL(char);
  FILE* old_nusmv_stdout = NIL(FILE);

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hmo:n:p:P:")) != EOF) {
    switch (c) {
    case 'h': return UsageCheckCtlSpec();
    case 'n':
      {
        if (formula != NIL(char)) return UsageCheckCtlSpec();
        if (prop_no != -1) return UsageCheckCtlSpec();
        if (formula_name != NIL(char)) return UsageCheckCtlSpec();

        prop_no = PropDb_get_prop_index_from_string(PropPkg_get_prop_database(),
                                                    util_optarg);
        if (prop_no == -1) return 1;

        break;
      }
    case 'P':
      {
        if (formula != NIL(char)) return UsageCheckCtlSpec();
        if (prop_no != -1) return UsageCheckCtlSpec();
        if (formula_name != NIL(char)) return UsageCheckCtlSpec();

        formula_name = util_strsav(util_optarg);

        prop_no = PropDb_prop_parse_name(PropPkg_get_prop_database(),
                                         formula_name);

        if (prop_no == -1) {
          fprintf(nusmv_stderr, "No property named \"%s\"\n", formula_name);
          FREE(formula_name);
          return 1;
        }
        FREE(formula_name);
        break;
      }
    case 'p':
      {
        if (prop_no != -1) return UsageCheckCtlSpec();
        if (formula != NIL(char)) return UsageCheckCtlSpec();
        if (formula_name != NIL(char)) return UsageCheckCtlSpec();

        formula = util_strsav(util_optarg);
        break;
      }
    case 'o':
      if (useMore == 1) return UsageCheckCtlSpec();
      dbgFileName = util_strsav(util_optarg);
      fprintf(nusmv_stdout, "Output to file: %s\n", dbgFileName);
      break;
    case 'm':
      if (dbgFileName != NIL(char)) return UsageCheckCtlSpec();
      useMore = 1;
      break;
    default:  return UsageCheckCtlSpec();
    }
  }
  if (argc != util_optind) return UsageCheckCtlSpec();

  /* pre-conditions */
  if (Compile_check_if_model_was_built(nusmv_stderr, false)) return 1;

  if (useMore) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenPipe(useMore);
    if (nusmv_stdout==(FILE*) NULL) {nusmv_stdout=old_nusmv_stdout; return 1;}
  }

  if (dbgFileName != NIL(char)) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenFile(dbgFileName);
    if (nusmv_stdout==(FILE*) NULL) {nusmv_stdout = old_nusmv_stdout; return 1;}
  }

  if (formula != NIL(char)) {
    prop_no = PropDb_prop_parse_and_add(PropPkg_get_prop_database(),
                                        Compile_get_global_symb_table(),
                                        formula, Prop_Ctl);
    if (prop_no == -1) { status = 1; goto check_ctlspec_exit; /* error */ }
    CATCH {
      PropDb_verify_prop_at_index(PropPkg_get_prop_database(), prop_no);
    }
    FAIL {
      status = 1;
    }
  }
  else if (prop_no != -1) {
    if (Prop_check_type(PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                                 prop_no),
                        Prop_Ctl) != 0) {
      status = 1;
    }
    else {
      CATCH {
        PropDb_verify_prop_at_index(PropPkg_get_prop_database(), prop_no);
      }
      FAIL {
        status = 1;
      }
    }
  }
  else {
    CATCH {
      if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
        PropDb_ordered_verify_all_type(PropPkg_get_prop_database(),
                                       mainFlatHierarchy,
                                       Prop_Ctl);
      else PropDb_verify_all_type(PropPkg_get_prop_database(), Prop_Ctl);
    }
    FAIL {
      status = 1;
    }
  }

check_ctlspec_exit:   /* clean exit */
  if (useMore) {
    CmdClosePipe(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }

  if (dbgFileName != NIL(char)) {
    CmdCloseFile(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }

  return status;
}


static int UsageCheckCtlSpec()
{
  fprintf(nusmv_stderr, "usage: check_ctlspec [-h] [-m | -o file] [-n number | -p \"ctl-expr\" | -P \"name\"]\n");
  fprintf(nusmv_stderr, "   -h \t\t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -m \t\t\tPipes output through the program specified\n");
  fprintf(nusmv_stderr, "      \t\t\tby the \"PAGER\" environment variable if defined,\n");
  fprintf(nusmv_stderr, "      \t\t\telse through the UNIX command \"more\".\n");
  fprintf(nusmv_stderr, "   -o file\t\tWrites the generated output to \"file\".\n");
  fprintf(nusmv_stderr, "   -n number\t\tChecks only the SPEC with the given index number.\n");
  fprintf(nusmv_stderr, "   -p \"ctl-expr\"\tChecks only the given CTL formula.\n");
  fprintf(nusmv_stderr, "   -P \"name\"\t\tChecks only the SPEC with the given name\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Performs model checking of invariants]

  CommandName        [check_invar]

  CommandSynopsis    [Performs model checking of invariants]

  CommandArguments [\[-h\] \[-m | -o output-file\] \[-s "strategy"\]
  \[-e "heuristic"\] \[-t number\] \[-k number\] \[-j "heuristic"\]
  \[-n number | -p "invar-expr \[IN context\]" | -P "name"\]]

  CommandDescription [Performs invariant checking on the given
  model. An invariant is a set of states. Checking the invariant is
  the process of determining that all states reachable from the
  initial states lie in the invariant.

  Invariants to be verified can be provided as simple formulas
  (without any temporal operators) in the input file via the
  <tt>INVARSPEC</tt> keyword or directly at command line, using the option
  <tt>-p</tt>.<p>

  Option <tt>-n</tt> can be used for checking a particular invariant
  of the model. If neither <tt>-n</tt> nor <tt>-p</tt> are
  used, all the invariants are checked.<p>

  During checking of invariant all the fairness conditions associated
  with the model are ignored.<p>

  If an invariant does not hold, a proof of failure is demonstrated.
  This consists of a path starting from an initial state to a state
  lying outside the invariant. This path has the property that it is the
  shortest path leading to a state outside the invariant.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-m</tt>
       <dd> Pipes the output generated by the program in processing
           <tt>INVARSPEC</tt>s to the program specified by the
           <tt>PAGER</tt> shell variable if defined, else
           through the UNIX command "more".
    <dt> <tt>-o output-file</tt>
       <dd> Writes the output generated by the command in processing
           <tt>INVARSPEC</tt>s to the file <tt>output-file</tt>.
    <dt> <tt>-s <i>strategy</i></tt>
    <dd> Force the analysis strategy.
    <dt> <tt>-e <i>heuristic</i></tt>
    <dd> Force the search heuristic for the forward-backward strategy.
    <dt> <tt>-t <i>number</i></tt>
    <dd> When using the mixed BDD and BMC approach specify the heuristic threshold.
    <dt> <tt>-k <i>number</i></tt>
    <dd> When using the mixed BDD and BMC approach specify the BMC max k.
    <dt> <tt>-j <i>heuristic</i></tt>
    <dd> Force the switch heuristic for the BDD-BMC strategy.
    <dt> <tt>-p "invar-expr \[IN context\]"</tt>
       <dd> The command line specified invariant formula to be verified.
       <tt>context</tt> is the module instance name which the variables
       in <tt>invar-expr</tt> must be evaluated in.
    <dt> <tt>-P name</tt>
       <dd> Checks the INVARSPEC with name <tt>name</tt> in the property
            database.
  </dl>]

  SideEffects        []

******************************************************************************/
int CommandCheckInvar(int argc, char **argv)
{
  int c;
  int prop_no = -1;
  char* formula = NIL(char);
  char* formula_name = NIL(char);
  int status = 0;
  int useMore = 0;
  char* dbgFileName = NIL(char);
  FILE* old_nusmv_stdout = NIL(FILE);

  boolean used_s, used_e, used_j, used_k, used_t;

  Check_Strategy strategy = FORWARD;
  Check_Strategy old_strategy;
  int threshold, old_threshold, bmck, old_bmck;
  FB_Heuristic fb_heuristic = ZIGZAG_HEURISTIC;
  FB_Heuristic old_fb_heuristic;
  Bdd2bmc_Heuristic old_bdd2bmc_heuristic;
  Bdd2bmc_Heuristic bdd2bmc_heuristic = STEPS_HEURISTIC;

  /* 'Used' variables are initially false */
  used_s = false;
  used_e = false;
  used_j = false;
  used_k = false;
  used_t = false;

  threshold = -1;
  bmck = -1;

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hbt:k:j:e:s:mo:n:p:P:")) != EOF) {
    switch (c) {
    case 'h': return(UsageCheckInvar());
    case 'n':
      {
        if (formula != NIL(char)) return(UsageCheckInvar());
        if (prop_no != -1) return(UsageCheckInvar());
        if (formula_name != NIL(char)) return UsageCheckInvar();

        prop_no = PropDb_get_prop_index_from_string(PropPkg_get_prop_database(),
                                                    util_optarg);
        if (prop_no == -1) return 1;

        break;
      }
    case 'P':
      {
        if (formula != NIL(char)) return UsageCheckInvar();
        if (prop_no != -1) return UsageCheckInvar();
        if (formula_name != NIL(char)) return UsageCheckInvar();

        formula_name = util_strsav(util_optarg);

        prop_no = PropDb_prop_parse_name(PropPkg_get_prop_database(),
                                         formula_name);

        if (prop_no == -1) {
          fprintf(nusmv_stderr, "No property named \"%s\"\n", formula_name);
          FREE(formula_name);
          return 1;
        }
        FREE(formula_name);
        break;
      }
    case 'p':
      {
        if (prop_no != -1) return(UsageCheckInvar());
        if (formula != NIL(char)) return(UsageCheckInvar());
        if (formula_name != NIL(char)) return UsageCheckInvar();
        formula = util_strsav(util_optarg);
        break;
      }
    case 'o':
      if (useMore == 1) return(UsageCheckInvar());
      dbgFileName = util_strsav(util_optarg);
      fprintf(nusmv_stdout, "Output to file: %s\n", dbgFileName);
      break;
    case 'm':
      if (dbgFileName != NIL(char)) return(UsageCheckInvar());
      useMore = 1;
      break;
    case 's':
      if(used_s) {
        fprintf(nusmv_stderr, "You cannot specify -s more than once!\n");
      }
      used_s = true;

      if (0 == strcmp("forward", util_optarg)) {
        strategy = FORWARD;
      }
      else if (0 == strcmp("backward", util_optarg)) {
        strategy = BACKWARD;
      }
      else if (0 == strcmp("forward-backward", util_optarg)) {
        strategy = FORWARD_BACKWARD;
      }
#if NUSMV_HAVE_SAT_SOLVER
      else if (0 == strcmp("bdd-bmc", util_optarg)) {
        strategy = BDD_BMC;
      }
      else {
        fprintf(nusmv_stderr, "Wrong strategy specified; you must choose between {'forward', 'backward', 'forward-backward', 'bdd-bmc'}.\n");
        return 1;
      }
#else
      else {
        fprintf(nusmv_stderr, "Wrong strategy specified; you must choose between {'forward', 'backward', 'forward-backward'}.\n");
        return 1;
      }
#endif
      break;

    case 'e':
      if(used_e) {
        fprintf(nusmv_stderr, "You cannot specify -e more than once!\n");
      }
      used_e = true;

      if (0 == strcmp("zigzag", util_optarg)) {
        fb_heuristic = ZIGZAG_HEURISTIC;
      }
      else if (0 == strcmp("smallest", util_optarg)) {
        fb_heuristic = SMALLEST_BDD_HEURISTIC;
      }
      else {
        fprintf(nusmv_stderr, "Wrong heuristic specified; you must choose between {'zigzag', 'smallest'}.\n");
        return 1;
      }
      break;

    case 'j':
      if(used_j) {
        fprintf(nusmv_stderr, "You cannot specify -j more than once!\n");
      }
      used_j = true;

      if (0 == strcmp("steps", util_optarg)) {
        bdd2bmc_heuristic = STEPS_HEURISTIC;
      }
      else if (0 == strcmp("size", util_optarg)) {
        bdd2bmc_heuristic = SIZE_HEURISTIC;
      }
      else {
        fprintf(nusmv_stderr, "Wrong heuristic specified; you must choose between {'steps', 'size'}.\n");
        return 1;
      }
      break;

    case 't':
      {
        int res;

        if(used_t) {
          fprintf(nusmv_stderr, "You cannot specify -t more than once!\n");
        }
        used_t = true;

        res = sscanf(util_optarg, "%d", &threshold);
        if (res <= 0) {
          fprintf(nusmv_stderr, "You must specify a valid number as threshold\n");
          return 1;
        }
        break;
      }
    case 'k':
      {
        int res;

        if(used_k) {
          fprintf(nusmv_stderr, "You cannot specify -k more than once!\n");
        }
        used_k = true;

        res = sscanf(util_optarg, "%d", &bmck);
        if (res <= 0) {
          fprintf(nusmv_stderr, "You must specify a valid integer number as BMC k\n");
          return 1;
        }
        break;
      }
    default:  return(UsageCheckInvar());
    }
  }
  if (argc != util_optind) return(UsageCheckInvar());

  /* pre-conditions */
  if (Compile_check_if_model_was_built(nusmv_stderr, false)) return 1;

  if ((BDD_BMC == strategy) && \
      (Compile_check_if_bool_model_was_built(nusmv_stderr, false))) return 1;

  /* Save current option values for restoring */
  old_strategy = opt_check_invar_strategy(OptsHandler_get_instance());
  old_bdd2bmc_heuristic = opt_check_invar_bddbmc_heuristic(OptsHandler_get_instance());
  old_fb_heuristic = opt_check_invar_fb_heuristic(OptsHandler_get_instance());
  old_threshold = opt_check_invar_bddbmc_heuristic_threshold(OptsHandler_get_instance());
  old_bmck = get_bmc_pb_length(OptsHandler_get_instance());

  if (used_s) {
    set_check_invar_strategy(OptsHandler_get_instance(), strategy);
  }
  else {
    strategy = old_strategy;
  }

  if (used_e) {
    if ((FORWARD_BACKWARD != strategy) && (BDD_BMC != strategy)) {
      fprintf(nusmv_stderr, "Unable to use an heuristic without forward-backward or bdd-bmc strategy.\n");
      status = 1;
      goto reset_options;
    }

    set_check_invar_fb_heuristic(OptsHandler_get_instance(), fb_heuristic);
  }

  if (used_j) {
    if (BDD_BMC != strategy) {
      fprintf(nusmv_stderr, "Unable to use an heuristic for bdd-bmc while not in bdd-bmc strategy!\n");
      status = 1;
      goto reset_options;
    }

    set_check_invar_bddbmc_heuristic(OptsHandler_get_instance(), bdd2bmc_heuristic);
  }

  if (used_t) {
    if (BDD_BMC != strategy) {
      fprintf(nusmv_stderr, "Unable to use an heuristic threshold for bdd-bmc while not in bdd-bmc strategy!\n");
      status = 1;
      goto reset_options;
    }

    set_check_invar_bddbmc_heuristic_threshold(OptsHandler_get_instance(), threshold);
  }

  if (used_k) {
    if (BDD_BMC != strategy) {
      fprintf(nusmv_stderr, "Unable to set BMC k while not in bdd-bmc strategy!\n");
      status = 1;
      goto reset_options;
    }

#if NUSMV_HAVE_SAT_SOLVER
    set_bmc_pb_length(OptsHandler_get_instance(), bmck);
#endif
  }

  if (useMore) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenPipe(useMore);
    if (nusmv_stdout==(FILE*) NULL) {nusmv_stdout=old_nusmv_stdout; return 1;}
  }

  if (dbgFileName != NIL(char)) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenFile(dbgFileName);
    if (nusmv_stdout==(FILE*) NULL) {nusmv_stdout = old_nusmv_stdout; return 1;}
  }

  if (formula != NIL(char)) {
    prop_no = PropDb_prop_parse_and_add(PropPkg_get_prop_database(),
                                        Compile_get_global_symb_table(),
                                        formula, Prop_Invar);
    if (prop_no == -1) { status = 1; goto check_invar_exit; /* error */ }
    CATCH {
      PropDb_verify_prop_at_index(PropPkg_get_prop_database(), prop_no);
    }
    FAIL {
      status = 1;
    }
  }
  else if (prop_no != -1) {
    if (Prop_check_type(PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                                 prop_no),
                        Prop_Invar) != 0) {
      status = 1;
    }
    else {
      CATCH {
        PropDb_verify_prop_at_index(PropPkg_get_prop_database(), prop_no);
      }
      FAIL {
        status = 1;
      }
    }
  }
  else {
    CATCH {
      if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
        PropDb_ordered_verify_all_type(PropPkg_get_prop_database(),
                                       mainFlatHierarchy,
                                       Prop_Invar);
      else PropDb_verify_all_type(PropPkg_get_prop_database(), Prop_Invar);
    }
    FAIL {
      status = 1;
    }
  }

 check_invar_exit:   /* clean exit */
  if (useMore) {
    CmdClosePipe(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }

  if (dbgFileName != NIL(char)) {
    CmdCloseFile(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }

  /* Restore options */
 reset_options:
  set_check_invar_strategy(OptsHandler_get_instance(), old_strategy);
  set_check_invar_fb_heuristic(OptsHandler_get_instance(), old_fb_heuristic);
  set_check_invar_bddbmc_heuristic(OptsHandler_get_instance(), old_bdd2bmc_heuristic);
  set_check_invar_bddbmc_heuristic_threshold(OptsHandler_get_instance(), old_threshold);
#if NUSMV_HAVE_SAT_SOLVER
  set_bmc_pb_length(OptsHandler_get_instance(), old_bmck);
#endif

  return status;
}

static int UsageCheckInvar()
{
#if NUSMV_HAVE_SAT_SOLVER
  fprintf(nusmv_stderr, "usage: check_invar [-h] [-m| -o file] [-s strategy] [-e heuristic] [-t number] [-k number] [-j heuristic] [-n number | -p \"invar-expr\" | -P \"name\"]\n");
#else
  fprintf(nusmv_stderr, "usage: check_invar [-h] [-m| -o file] [-s strategy] [-e heuristic] [-n number | -p \"invar-expr\" | -P \"name\"]\n");
#endif
  fprintf(nusmv_stderr, "   -h \t\t\tPrints the command usage.\n");
#if NUSMV_HAVE_SAT_SOLVER
  fprintf(nusmv_stderr, "   -s \t\t\tForce the analysis strategy (Overrides options). Possible values are {'forward', 'backward', 'forward-backward', 'bdd-bmc'}\n");
#else
  fprintf(nusmv_stderr, "   -s \t\t\tForce the analysis strategy (Overrides options). Possible values are {'forward', 'backward', 'forward-backward'}\n");
#endif
  fprintf(nusmv_stderr, "   -e \t\t\tForce the heuristic in case of forward-backward strategy (Overrides options). Possible values are {'zigzag', 'smallest'}\n");
#if NUSMV_HAVE_SAT_SOLVER
  fprintf(nusmv_stderr, "   -t \t\t\tWhen using the mixed BDD and BMC approach specify the heuristic threshold\n");
  fprintf(nusmv_stderr, "   -k \t\t\tWhen using the mixed BDD and BMC approach specify the BMC max k\n");
  fprintf(nusmv_stderr, "   -j \t\t\tSpecify the heuristic used to switch from BDD to BMC. Possible values are {'steps', 'size'}\n");
#endif
  fprintf(nusmv_stderr, "   -m \t\t\tPipes output through the program specified by\n");
  fprintf(nusmv_stderr, "      \t\t\tthe \"PAGER\" shell variable if defined,\n");
  fprintf(nusmv_stderr, "      \t\t\telse through the UNIX command \"more\".\n");
  fprintf(nusmv_stderr, "   -o file\t\tWrites the generated output to \"file\".\n");
  fprintf(nusmv_stderr, "   -n number\t\tchecks only the INVARSPEC with the given index number.\n");
  fprintf(nusmv_stderr, "   -p \"invar-expr\"\tchecks only the given invariant formula.\n");
  fprintf(nusmv_stderr, "   -P \"name\"\t\tchecks only the INVARSPEC with the given name.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Performs computation of quantitative characteristics]

  CommandName        [check_compute]

  CommandSynopsis    [Performs computation of quantitative characteristics]

  CommandArguments   [\[-h\] \[-m | -o output-file\] \[-n number |
  -p "compute-expr \[IN context\]" | -P "name"\]]

  CommandDescription [This command deals with the computation of
  quantitative characteristics of real time systems. It is able to
  compute the length of the shortest (longest) path from two given set
  of states.<p>

  <center><code>MAX \[ alpha , beta \]</code></center><br>
  <center><code>MIN \[ alpha , beta \]</code></center><p>

  Properties of the above form can be specified in the input file via
  the keyword <code>COMPUTE</code> or directly at command line,
  using option <tt>-p</tt>.<p>

  Option <tt>-n</tt> can be used for computing a particular expression
  in the model. If neither <tt>-n</tt> nor <tt>-p</tt> nor <tt>-P</tt>
  are used, all the COMPUTE specifications are computed.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-m</tt>
       <dd> Pipes the output generated by the command in processing
           <tt>COMPUTE</tt>s to the program specified by the
           <tt>PAGER</tt> shell variable if defined, else
           through the UNIX command "more".
    <dt> <tt>-o output-file</tt>
       <dd> Writes the output generated by the command in processing
           <tt>COMPUTE</tt>s to the file <tt>output-file</tt>.
    <dt> <tt>-p "compute-expr \[IN context\]"</tt>
       <dd> A COMPUTE formula to be checked. <tt>context</tt> is the module
       instance name which the variables in <tt>compute-expr</tt> must
       be evaluated in.
    <dt> <tt>-n number</tt>
       <dd> Computes only the property with index <tt>number</tt>
    <dt> <tt>-P name</tt>
       <dd> Computes only the property named <tt>name</tt>
  </dl>]

  SideEffects        []

******************************************************************************/
int CommandCheckCompute(int argc, char **argv)
{
  return mc_cmd_check_compute(argc, argv, UsageCheckCompute);
}


static int UsageCheckCompute()
{
  fprintf(nusmv_stderr, "usage: check_compute [-h] [-m | -o file] [-n number | -p \"compute-expr\" | -P \"name\"]\n");
  fprintf(nusmv_stderr, "   -h \t\t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -m \t\t\tPipes output through the program specified by\n");
  fprintf(nusmv_stderr, "      \t\t\tthe \"PAGER\" shell variable if defined,\n");
  fprintf(nusmv_stderr, "      \t\t\telse through the UNIX command \"more\".\n");
  fprintf(nusmv_stderr, "   -o file\t\tWrites the generated output to \"file\".\n");
  fprintf(nusmv_stderr, "   -n number\t\tConsiders only the compute expression with the given index number.\n");
  fprintf(nusmv_stderr, "   -p \"compute-expr\"\tComputes the given expression.\n");
  fprintf(nusmv_stderr, "   -P name\t\tConsiders only the compute expression with the given name.\n");
  return 1;
}

/* Deprecated */
int CommandCompute(int argc, char **argv)
{
  int res = mc_cmd_check_compute(argc, argv, UsageCompute);

  fprintf(nusmv_stderr,
          "\nWarning: command 'compute' is deprecated, use "    \
          "'check_compute' instead.\n");
  return res;
}

static int UsageCompute()
{
  fprintf(nusmv_stderr, "usage: compute [-h] [-m | -o file] [-n number | -p \"compute-expr\" | -P \"name\"]\n");
  fprintf(nusmv_stderr, "   -h \t\t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -m \t\t\tPipes output through the program specified by\n");
  fprintf(nusmv_stderr, "      \t\t\tthe \"PAGER\" shell variable if defined,\n");
  fprintf(nusmv_stderr, "      \t\t\telse through the UNIX command \"more\".\n");
  fprintf(nusmv_stderr, "   -o file\t\tWrites the generated output to \"file\".\n");
  fprintf(nusmv_stderr, "   -n number\t\tConsiders only the compute expression with the given index number.\n");
  fprintf(nusmv_stderr, "   -p \"compute-expr\"\tComputes the given expression.\n");
  fprintf(nusmv_stderr, "   -P name\t\tConsiders only the compute expression with the given name.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Performs fair PSL model checking.]

  CommandName        [check_pslspec]

  CommandSynopsis    [Performs fair PSL model checking.]

  CommandArguments   [\[-h\] \[-m | -o output-file\] \[-n number | -p "psl-expr \[IN context\]" | -P "name"\]
  \[-b|-s \[-i\] \[-c\] \[-N\] \[-g\] \[-1\] \[-k bmc_length\] \[-l loopback\]\]]

]

  CommandDescription [Performs fair PSL model checking.<p>

  A <tt>psl-expr</tt> to be checked can be specified at command line
  using option <tt>-p</tt>. Alternatively, option <tt>-n</tt> can be used
  for checking a particular formula in the property database. If
  neither <tt>-n</tt> nor <tt>-p</tt> are used, all the PSLSPEC formulas in
  the database are checked.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-m</tt>
       <dd> Pipes the output generated by the command in processing
           <tt>SPEC</tt>s to the program specified by the
           <tt>PAGER</tt> shell variable if defined, else
           through the <tt>UNIX</tt> command "more".
    <dt> <tt>-o output-file</tt>
       <dd> Writes the output generated by the command in processing
           <tt>PSLSPEC</tt>s to the file <tt>output-file</tt>.
    <dt> <tt>-p "psl-expr \[IN context\]"</tt>
       <dd> A PSL formula to be checked. <tt>context</tt> is the module
       instance name which the variables in <tt>ctl-expr</tt> must
       be evaluated in.
    <dt> <tt>-n number</tt>
       <dd> Checks the PSL property with index <tt>number</tt> in the property
            database.
    <dt> <tt>-P name</tt>
       <dd> Checks the PSL property named <tt>name</tt> in the property
            database.

    <dt> <tt>-b</tt> When specified, the BMC engine will be used for
    checking those PSL properties that can be converted to LTL
    specifications. The SAT solver to be used will be chosen according
    to the current value of the system variable sat_solver. Non-LTL
    properties will be ignored.

    <dt> <tt>-s</tt> When specified, the SBMC engine will be used for
    checking those PSL properties that can be converted to LTL
    specifications. The SAT solver to be used will be chosen according
    to the current value of the system variable sat_solver. Non-LTL
    properties will be ignored.

    <dt> <tt>-i</tt> Uses incremental SAT solving when this feature is
    available. This option must be mandatorily used in combination
    with the option -b.

    <dt> <tt>-c</tt> Performs completeness check. This option can be
    used only in combination with -s. This options implies also -i

    <dt> <tt>-N</tt> Does not perform virtual unrolling. This option can be
    used only in combination with -s. This options implies also -i


    <dt> <tt>-g</tt> While solving a problem, dumps it as a DIMACS
    file whose name depends on the content of the system variable
    "bmc_dimacs_filename". This feature is not allowed when the option
    -i is used as well.

    <dt> <tt>-1</tt> Generates and solves a single problem instead of
    iterating through 0 and bmc_length.

    <dt> <tt>-k <i>bmc_length</i></tt>
       <dd> <i>bmc_length</i> is the maximum problem bound must be reached if
       the option -1 is not specified. If -1 is specified, <i>bmc_length</i> is
       the exact length of the problem to be generated.
       Only natural number are valid values for this option. If no value
       is given the environment variable <i>bmc_length</i> is considered
       instead.
    <dt> <tt>-l <i>loopback</i></tt>
       <dd> <i>loopback</i> value may be: <BR>
       - a natural number in (0, <i>bmc_length-1</i>). Positive sign ('+') can
       be also used as prefix of the number. Any invalid combination of length
       and loopback will be skipped during the generation/solving process.<BR>
       - a negative number in (-1, -<i>bmc_length</i>). In this case
       <i>loopback</i> is considered a value relative to <i>bmc_length</i>.
       Any invalid combination of length and loopback will be skipped
       during the generation/solving process.<BR>
       - the symbol 'X', which means "no loopback" <BR>
       - the symbol '*', which means "all possible loopback from zero to
       <i>bmc_length-1</i>"

  </dl><p>

  ]

  SideEffects        []

******************************************************************************/
int CommandCheckPslSpec(int argc, char **argv)
{
  int c;

  int k = -1;
  int l = 0;
  char* str_loop = (char*) NULL;
  boolean l_specified = false;
  boolean bmc_dump = false;
  boolean single_bmc_prob = false;
  boolean inc_sat = false;

  int prop_no = -1;
  char* formula = NIL(char);
  char* formula_name = NIL(char);
  int status = 0;
  int useMore = 0;
  char* dbgFileName = NIL(char);
  FILE* old_nusmv_stdout = NIL(FILE);
  boolean use_bmc = false;
  boolean use_sbmc = false;
  boolean check_compl = false;
  boolean does_virtual_unrol = true;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hbsicN1gmo:n:p:P:k:l:")) != EOF) {
    switch (c) {
    case 'h': return UsageCheckPslSpec();
    case 'n':
    {
      if (formula != NIL(char)) return UsageCheckPslSpec();
      if (prop_no != -1) return UsageCheckPslSpec();
      if (formula_name != NIL(char)) return UsageCheckPslSpec();

      prop_no = PropDb_get_prop_index_from_string(PropPkg_get_prop_database(),
                                                  util_optarg);
      if (prop_no == -1) return 1;

      break;
    }
    case 'P':
      {
        if (formula != NIL(char)) return UsageCheckPslSpec();
        if (prop_no != -1) return UsageCheckPslSpec();
        if (formula_name != NIL(char)) return UsageCheckPslSpec();

        formula_name = util_strsav(util_optarg);

        prop_no = PropDb_prop_parse_name(PropPkg_get_prop_database(),
                                         formula_name);

        if (prop_no == -1) {
          fprintf(nusmv_stderr, "No property named \"%s\"\n", formula_name);
          FREE(formula_name);
          return 1;
        }
        FREE(formula_name);
        break;
      }
    case 'b':
      use_bmc = true;
      break;

    case 's':
      use_sbmc = true;
      break;

    case 'N':
#if NUSMV_HAVE_INCREMENTAL_SAT
      inc_sat = true;
#else
      fprintf(nusmv_stderr,
              "Error: the option '-N' cannot be used because the "\
              "system has not been\n" \
              " configured with any support for incremental SAT solving.\n");
      return 1;
#endif

      does_virtual_unrol = false;
      break;

    case 'c':
#if NUSMV_HAVE_INCREMENTAL_SAT
      inc_sat = true;
#else
      fprintf(nusmv_stderr,
              "Error: the option '-c' cannot be used because the "\
              "system has not been\n" \
              " configured with any support for incremental SAT solving.\n");
      return 1;
#endif

      check_compl = true;
      break;

    case 'i':
#if NUSMV_HAVE_INCREMENTAL_SAT
      inc_sat = true;
#else
      fprintf(nusmv_stderr,
              "Error: the option '-i' cannot be used because the "\
              "system has not been\n" \
              " configured with any support for incremental SAT solving.\n");
      return 1;
#endif
      break;


    case 'g':
      bmc_dump = true;
      break;

    case '1':
      single_bmc_prob = true;
      break;

    case 'k':
    {
      char* str_k;

      /* check if a value has already been specified: */
      if (k != -1) {
        fprintf(nusmv_stderr,
                "Option -k cannot be specified more than once.\n");
        return 1;
      }

      str_k = util_strsav(util_optarg);

      if ((util_str2int(str_k, &k) != 0) || k<0) {
        error_invalid_number(str_k);
        FREE(str_k);
        return 1;
      }

      FREE(str_k);
      break;
    }

    case 'l':
      /* check if a value has already been specified: */
      if (l_specified) {
        fprintf(nusmv_stderr,
                "Option -l cannot be specified more than once.\n");
        return 1;
      }

      str_loop = util_strsav(util_optarg);
      l_specified = true;
      /* checking of loopback value is delayed after command line
         processing to allow any -k option evaluation before (see
         the cheking code below) */
      break;

    case 'p':
    {
      if (prop_no != -1) return UsageCheckPslSpec();
      if (formula != NIL(char)) return UsageCheckPslSpec();
      if (formula_name != NIL(char)) return UsageCheckPslSpec();

      formula = util_strsav(util_optarg);
      break;
    }
    case 'o':
      if (useMore == 1) return UsageCheckPslSpec();
      dbgFileName = util_strsav(util_optarg);
      fprintf(nusmv_stdout, "Output to file: %s\n", dbgFileName);
      break;
    case 'm':
      if (dbgFileName != NIL(char)) return UsageCheckPslSpec();
      useMore = 1;
      break;
    default:  return UsageCheckPslSpec();
    }
  }
  if (argc != util_optind) return UsageCheckPslSpec();

  /* A number of options are valid only when -b is specified along
     with them */
  if ((k != -1 || str_loop != (char*) NULL || inc_sat ||
       bmc_dump || single_bmc_prob) && !(use_bmc || use_sbmc)) {
    fprintf(nusmv_stderr,
            "Error: options \"-i, -g, -1, -k and -l\" are allowed only when\n"\
            "the option -b or -s is specified as well.\n");
    return 1;
  }

  if (use_bmc && use_sbmc) {
    fprintf(nusmv_stderr,
            "Error: options \"-b\" and \"-s\" cannot be used at the same time.\n");
  }

  if ((check_compl || !does_virtual_unrol) && !use_sbmc) {
    fprintf(nusmv_stderr,
            "Error: options \"-c\" and \"-N\" can be used only in combination " \
            "with the option \"-s\".\n");
  }

  /* Dumping with incremental sat is not allowed: */
  if (bmc_dump && inc_sat) {
    fprintf(nusmv_stderr,
            "Error: problem generation with incremental solvers is an "\
            "unsupported feature.\n");
    return 1;
  }


  /* ---------------------------------------------------------------------- */
  if (use_bmc || use_sbmc) {
    if (k == -1) k = get_bmc_pb_length(OptsHandler_get_instance()); /* default for k */

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

      if (Bmc_Utils_Check_k_l(k, Bmc_Utils_RelLoop2AbsLoop(rel_loop, k))
          != OUTCOME_SUCCESS) {
        error_bmc_invalid_k_l(k, rel_loop);
        return OUTCOME_GENERIC_ERROR;
      }

      l = rel_loop;
    } /* k,l consistency check */

    else {
      /* A default value for loop */
      l = Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(OptsHandler_get_instance()), NULL);
    }
  }

  /* Checking compilation status */
  if (Compile_check_if_encoding_was_built(nusmv_stderr)) return 1;

  /* Model construction is delayed until property is being checked,
     according to the specific technique that is being used. */

  if (useMore) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenPipe(useMore);
    if (nusmv_stdout==(FILE*) NULL) {nusmv_stdout=old_nusmv_stdout; return 1;}
  }

  if (dbgFileName != NIL(char)) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenFile(dbgFileName);
    if (nusmv_stdout==(FILE*) NULL) {nusmv_stdout = old_nusmv_stdout; return 1;}
  }

  if (formula != NIL(char)) {
    prop_no = PropDb_prop_parse_and_add(PropPkg_get_prop_database(),
                                        Compile_get_global_symb_table(),
                                        formula, Prop_Psl);
    if (prop_no == -1) { status = 1; goto check_psl_exit; /* error */ }
  }

  if (prop_no != -1) {
    /* checks a single property */
    if (Prop_check_type(PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                                 prop_no),
                        Prop_Psl) != 0) {
      status = 1;
    }
    else {
      if (use_bmc) {
        status = Bmc_check_psl_property(
                PropDb_get_prop_at_index(PropPkg_get_prop_database(), prop_no),
                bmc_dump, inc_sat,
                single_bmc_prob, k, l);
      }
      else if (use_sbmc) {
        status = Sbmc_check_psl_property(
            PropDb_get_prop_at_index(PropPkg_get_prop_database(), prop_no),
            bmc_dump,
            inc_sat,
            check_compl,
            does_virtual_unrol,
            single_bmc_prob,
            k, l);
      }
      else {
        status = Mc_check_psl_property(
           PropDb_get_prop_at_index(PropPkg_get_prop_database(), prop_no));
      }
    }
  }
  else {
    /* Checks all the PSLSPEC in the database */
    lsList props;
    lsGen  iterator;
    Prop_ptr prop;

    if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
      props = PropDb_get_ordered_props_of_type(PropPkg_get_prop_database(),
                                               mainFlatHierarchy,
                                               Prop_Psl);
    else props = PropDb_get_props_of_type(PropPkg_get_prop_database(),
                                          Prop_Psl);

    nusmv_assert(props != LS_NIL);

    lsForEachItem(props, iterator, prop) {
      if (use_bmc) {
        if (Prop_is_psl_ltl(prop)) {
          status = Bmc_check_psl_property(prop,
                                          bmc_dump, inc_sat,
                                          single_bmc_prob, k, l);
        }
      }

      else if (use_sbmc) {
        status = Sbmc_check_psl_property(prop,
                                         bmc_dump,
                                         inc_sat,
                                         check_compl,
                                         does_virtual_unrol,
                                         single_bmc_prob,
                                         k, l);
      }

      else status = Mc_check_psl_property(prop);

      if (status != 0) break;
    }

    lsDestroy(props, NULL); /* the list is no longer needed */
  }

check_psl_exit:   /* clean exit */
  if (useMore) {
    CmdClosePipe(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }

  if (dbgFileName != NIL(char)) {
    CmdCloseFile(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }

  return status;

}


static int UsageCheckPslSpec()
{
  fprintf(nusmv_stderr, "usage: check_pslspec [-h] [-m | -o file] [-n number | -p \"psl-expr\" | -P \"name\"]\n");
  fprintf(nusmv_stderr, "                  [-b | -s] [-i] [-g] [-1] [-c] [-N] \n");
  fprintf(nusmv_stderr, "                  [-k bmc_length] [-l loopback]]\n");
  fprintf(nusmv_stderr, "   -h \t\t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -m \t\t\tPipes output through the program specified\n");
  fprintf(nusmv_stderr, "      \t\t\tby the \"PAGER\" environment variable if defined,\n");
  fprintf(nusmv_stderr, "      \t\t\telse through the UNIX command \"more\".\n");
  fprintf(nusmv_stderr, "   -o file\t\tWrites the generated output to \"file\".\n");
  fprintf(nusmv_stderr, "   -n number\t\tChecks only the PSLSPEC with the given index number.\n");
  fprintf(nusmv_stderr, "   -p \"psl-expr\"\tChecks only the given PSL formula.\n");
  fprintf(nusmv_stderr, "   -P \"name\"\t\tChecks only the PSLSPEC with the given name.\n");
  fprintf(nusmv_stderr, "   -b \t\t\tUses the BMC engine instead of the BDD-based one.\n");
  fprintf(nusmv_stderr, "   -s \t\t\tUses the SBMC engine instead of the BDD-based one.\n");
  fprintf(nusmv_stderr, "   Only when -b or -s is specified:\n");
  fprintf(nusmv_stderr, "    -i \t\t\tUses incremental SAT solving if available.\n");
  fprintf(nusmv_stderr, "    -g \t\t\tDumps generated problems in DIMACS format.\n");
  fprintf(nusmv_stderr, "    -1 \t\t\tGenerates and solves single problems.\n");
  fprintf(nusmv_stderr, "    -k bmc_length\tChecks the property using <bmc_length> value instead \n\t\t\tof using the variable <bmc_length> value.\n");
  fprintf(nusmv_stderr, "    -l loopback\t\tChecks the property using <loopback> value\n\t\t\tinstead of using the variable <bmc_loopback> value.\n");
  fprintf(nusmv_stderr, "   Only when -s is specified:\n");
  fprintf(nusmv_stderr, "    -c \t\t\tPerforms completeness check (implies -i)\n");
  fprintf(nusmv_stderr, "    -N \t\t\tDoes not perform virtual unrolling (implies -i).\n\n");


  return 1;
}


/**Function********************************************************************

  Synopsis           [Checks for language emptiness.]

  CommandName        [language_emptiness]

  CommandSynopsis    [Checks for language emptiness.]

  CommandArguments   [\[-h\] \[-v\] \[-a\]]

  CommandDescription [Checks for the language emptiness. <br>

  If <tt>-a</tt> is given the check is performed by verifying whether
  all initial states are included in the set of fair states. If it is
  the case from all initial states there exists a fair path and thus
  the language is not empty. On the other hand, if no <tt>-a</tt> is
  specified, the check is performed by verifying whether there exists
  at least one inital state that is also a fair state. In this case
  there is an initial state from which it starts a fair path and thus
  the lnaguage is not empty.

  if <tt>-v</tt> is specified, then some information on the set of
  initial states is printed out too.  ]

  SideEffects        []

******************************************************************************/
int CommandLanguageEmptiness(int argc, char **argv)
{
  int c;
  boolean allinit = false;
  boolean verbose = false;
  /*
   * Parse the command line.
  */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hva")) != EOF) {
    switch (c) {
    case 'h': return UsageLanguageEmptiness();
    case 'v':
      verbose = true;
      break;
    case 'a':
      allinit = true;
      break;
    default:
      return UsageLanguageEmptiness();
    }
  }

  if (Compile_check_if_model_was_built(nusmv_stderr, true)) return 1;

  if (get_oreg_justice_emptiness_bdd_algorithm(OptsHandler_get_instance()) ==
      BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD) {
    if (allinit) {
      /* VS: See comment in mcLE.c why allinit is not implemented. */
      fprintf(nusmv_stderr,
              "Forward Emerson-Lei cannot be used to check whether all "\
              "initial states are fair.\n");
      return 1;
    }
  }

  fprintf(nusmv_stdout,
          "######################################################################\n");
  Mc_CheckLanguageEmptiness(
              PropDb_master_get_bdd_fsm(PropPkg_get_prop_database()),
              allinit, verbose);
  fprintf(nusmv_stdout,
          "######################################################################\n");

  return 0;
}

static int UsageLanguageEmptiness()
{
  fprintf(nusmv_stderr, "usage: language_emptyness [-h] [-v] [-a]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -v \t\tPrints some information on the list of initial fair states.\n");
  fprintf(nusmv_stderr, "   -a \t\tChecks all initial states for being fair states.\n");
  fprintf(nusmv_stderr, "      \t\tOtherwise checks for the existence of at least a fair initial state.\n");
  return 1;
}



/**Function********************************************************************

  Synopsis           [Deprecated version of CommandCheckCtlSpec]

  Description        [Provided for backward compatibility]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int CommandCheckSpec(int argc, char **argv)
{
  int res = CommandCheckCtlSpec(argc, argv);

  if (res == 0) {
    fprintf(nusmv_stderr,
            "\nWarning: command 'check_spec' is deprecated, use " \
            "'check_ctlspec' instead.\n");
  }
  return res;
}

/**Function********************************************************************

  Synopsis           [helper function of commands check_compute and compute]

  Description        [helper function of commands check_compute and compute]

  SideEffects        []

  SeeAlso            [CommandCheckCompute CommandCompute]

******************************************************************************/
static int mc_cmd_check_compute(int argc, char **argv,
                                int (*usage_fun)(void))
{
  int c;
  int prop_no = -1;
  char * formula = NIL(char);
  char * formula_name = NIL(char);
  int status = 0;
  int useMore = 0;
  char * dbgFileName = NIL(char);
  FILE * old_nusmv_stdout = NIL(FILE);

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hmo:n:p:P:")) != EOF) {
    switch (c) {
    case 'h': return usage_fun();
    case 'n':
      {
        if (formula != NIL(char)) return usage_fun();
        if (prop_no != -1) return usage_fun();
        if (formula_name != NIL(char)) return usage_fun();

        prop_no = PropDb_get_prop_index_from_string(PropPkg_get_prop_database(),
                                                    util_optarg);
        if (prop_no == -1) return 1;

        break;
      }
    case 'P':
      {
        if (formula != NIL(char)) return usage_fun();
        if (prop_no != -1) return usage_fun();
        if (formula_name != NIL(char)) return usage_fun();

        formula_name = util_strsav(util_optarg);

        prop_no = PropDb_prop_parse_name(PropPkg_get_prop_database(),
                                         formula_name);

        if (prop_no == -1) {
          fprintf(nusmv_stderr, "No property named \"%s\"\n", formula_name);
          FREE(formula_name);
          return 1;
        }
        FREE(formula_name);
        break;
      }
    case 'p':
      {
        if (prop_no != -1) return usage_fun();
        if (formula != NIL(char)) return usage_fun();
        if (formula_name != NIL(char)) return usage_fun();

        formula = util_strsav(util_optarg);
        break;
      }
    case 'o':
      if (useMore == 1) return usage_fun();
      dbgFileName = util_strsav(util_optarg);
      fprintf(nusmv_stdout, "Output to file: %s\n", dbgFileName);
      break;
    case 'm':
      if (dbgFileName != NIL(char)) return usage_fun();
      useMore = 1;
      break;

    default: return usage_fun();
    }
  }
  if (argc != util_optind) return usage_fun();

  /* pre-conditions */
  if (Compile_check_if_model_was_built(nusmv_stderr, false)) return 1;

  if (useMore) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenPipe(useMore);
    if (nusmv_stdout==(FILE*) NULL) {nusmv_stdout=old_nusmv_stdout; return 1;}
  }

  if (dbgFileName != NIL(char)) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenFile(dbgFileName);
    if (nusmv_stdout==(FILE*) NULL) {nusmv_stdout = old_nusmv_stdout; return 1;}
  }

  if (formula != NIL(char)) {
    prop_no = PropDb_prop_parse_and_add(PropPkg_get_prop_database(),
                                        Compile_get_global_symb_table(),
                                        formula, Prop_Compute);
    if (prop_no == -1) { status = 1; goto check_compute_exit; /* error */ }
    CATCH {
      PropDb_verify_prop_at_index(PropPkg_get_prop_database(), prop_no);
    }
    FAIL {
      status = 1;
    }
  }
  else if (prop_no != -1) {
    if (Prop_check_type(PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                                 prop_no),
                        Prop_Compute) != 0) {
      status = 1;
    }
    else {
      CATCH {
        PropDb_verify_prop_at_index(PropPkg_get_prop_database(), prop_no);
      }
      FAIL {
        status = 1;
      }
    }
  }
  else {
    CATCH {
      if (opt_use_coi_size_sorting(OptsHandler_get_instance()))
        PropDb_ordered_verify_all_type(PropPkg_get_prop_database(),
                                       mainFlatHierarchy,
                                       Prop_Compute);
      else PropDb_verify_all_type(PropPkg_get_prop_database(), Prop_Compute);
    }
    FAIL {
      status = 1;
    }
  }

check_compute_exit:   /* clean exit */
  if (useMore) {
    CmdClosePipe(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }

  if (dbgFileName != NIL(char)) {
    CmdCloseFile(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }

  return status;

}
