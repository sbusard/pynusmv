/**CFile***********************************************************************

  FileName    [bddCmd.c]

  PackageName [bdd]

  Synopsis    [Bdd FSM commands]

  Description [This file contains all the shell commands to dela with
  computation and printing of reachable states, fair states and fair
  transitions.]

  Author      [Marco Roveri]

  Copyright   [ This file is part of the ``mc'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.

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

  To contact the NuSMV development board, email to <nusmv@fbk.eu>.]

******************************************************************************/

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "bdd.h"
#include "bddInt.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "cmd/cmd.h"
#include "utils/utils_io.h"
#include "enc/enc.h"
#include "compile/compile.h"
#include "prop/propPkg.h"
#include "parser/parser.h"

#include <math.h> /* for log10 */


static char rcsid[] UTIL_UNUSED = "$Id: bddCmd.c,v 1.1.2.4.4.5.4.12 2010-02-09 16:14:09 nusmv Exp $";

int CommandCheckFsm ARGS((int argc, char **argv));
int CommandComputeReachable ARGS((int argc, char **argv));
int CommandPrintReachableStates ARGS((int argc, char **argv));
int CommandPrintFairStates ARGS((int argc, char **argv));
int CommandPrintFairTransitions ARGS((int argc, char **argv));

int CommandDumpFsm ARGS((int argc, char **argv));

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
EXTERN cmp_struct_ptr cmps;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageCheckFsm ARGS((void));
static int UsageComputeReachable ARGS((void));
static int UsagePrintReachableStates ARGS((void));
static int UsagePrintFairStates ARGS((void));
static int UsagePrintFairTransitions ARGS((void));
static int UsageDumpFsm ARGS((void));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the BddFsm package.]

  Description        [Initializes the BddFsm package.]

  SideEffects        []

******************************************************************************/
void Bdd_Init(void){
  Cmd_CommandAdd("check_fsm", CommandCheckFsm, 0, false);
  Cmd_CommandAdd("print_reachable_states", CommandPrintReachableStates,
                 0, false);
  Cmd_CommandAdd("compute_reachable", CommandComputeReachable, 0, false);
  Cmd_CommandAdd("print_fair_states", CommandPrintFairStates, 0, false);
  Cmd_CommandAdd("print_fair_transitions", CommandPrintFairTransitions,
                 0, false);
  Cmd_CommandAdd("dump_fsm", CommandDumpFsm, 0, false);  
}

/**Function********************************************************************

  Synopsis           [Quit the BddFsm package]

  Description        [Quit the BddFsm package]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bdd_End(void)
{ }

/**Function********************************************************************

  Synopsis           [Checks the fsm for totality and deadlock states.]

  CommandName        [check_fsm]

  CommandSynopsis    [Checks the transition relation for totality.]

  CommandArguments   [\[-h\] \[-m | -o output-file\]]

  CommandDescription [
  Checks if the transition relation is total. If the transition
  relation is not total then a potential deadlock state is shown out.
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-m</tt>
       <dd> Pipes the output generated by the command to the program
            specified by the <tt>PAGER</tt> shell variable if
            defined, else through the UNIX command "more".
    <dt> <tt>-o output-file</tt>
       <dd> Writes the output generated by the command to the file
       <tt>output-file</tt>.
  </dl>
  At the beginning reachable states are computed in order to guarantee
  that deadlock states are actually reachable.]

  SideEffects        []

******************************************************************************/
int CommandCheckFsm(int argc, char **argv)
{
  int c;
  int useMore = 0;
  char * dbgFileName = NIL(char);
#if NUSMV_HAVE_GETENV
  char * pager;
#endif
  FILE * old_nusmv_stdout = NIL(FILE);

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hmo:")) != EOF) {
    switch (c) {
    case 'h': return UsageCheckFsm();
    case 'o':
      if (useMore == 1) return UsageCheckFsm();
      dbgFileName = util_strsav(util_optarg);
      fprintf(nusmv_stdout, "Output to file: %s\n", dbgFileName);
      break;
    case 'm':
      if (dbgFileName != NIL(char)) return UsageCheckFsm();
      useMore = 1;
      break;
    default:  return UsageCheckFsm();
    }
  }

  if (Compile_check_if_model_was_built(nusmv_stderr, true)) return 1;

  if (argc != util_optind) return UsageCheckFsm();

  if (useMore) {
#if NUSMV_HAVE_POPEN
    old_nusmv_stdout = nusmv_stdout;
#if NUSMV_HAVE_GETENV
    pager = getenv("PAGER");
    if (pager == NULL) {
      nusmv_stdout = popen("more", "w");
      if (nusmv_stdout == NULL) {
        fprintf(nusmv_stderr, "Unable to open pipe with \"more\".\n");
        nusmv_stdout = old_nusmv_stdout;
        return 1;
      }
    }
    else {
      nusmv_stdout = popen(pager, "w");
      if (nusmv_stdout == NULL) {
        fprintf(nusmv_stderr, "Unable to open pipe with \"%s\".\n", pager);
        nusmv_stdout = old_nusmv_stdout;
        return 1;
      }
    }
#else /* NUSMV_HAVE_GETENV */
    nusmv_stdout = popen("more", "w");
    if (nusmv_stdout == NULL) {
      fprintf(nusmv_stderr, "Unable to open pipe with \"more\".\n");
      nusmv_stdout = old_nusmv_stdout;
      return 1;
    }
#endif /* NUSMV_HAVE_GETENV */

#else /* NUSMV_HAVE_POPEN */
    fprintf(nusmv_stderr, "Pipe is not supported\n");
    return 1;
#endif /* NUSMV_HAVE_POPEN */
  }
  if (dbgFileName != NIL(char)) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = fopen(dbgFileName, "w");
    if (nusmv_stdout == NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", dbgFileName);
      nusmv_stdout = old_nusmv_stdout;
      return 1;
    }
  }

  BddFsm_check_machine(PropDb_master_get_bdd_fsm(PropPkg_get_prop_database()));
  set_forward_search(OptsHandler_get_instance());

#if NUSMV_HAVE_POPEN
  if (useMore) {
    pclose(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }
#endif

  if (dbgFileName != NIL(char)) {
    fflush(nusmv_stdout);
    fclose(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }
  return 0;
}

static int UsageCheckFsm()
{
  fprintf(nusmv_stderr, "usage: check_fsm [-h] [-m | -o file]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -m \t\tPipes output through the program specified by\n");
  fprintf(nusmv_stderr, "      \t\tthe \"PAGER\" environment variable if defined,\n");
  fprintf(nusmv_stderr, "      \t\telse through the UNIX command \"more\".\n");
  fprintf(nusmv_stderr, "   -o file\tWrites the generated output to \"file\".\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Computates the set of reachable states]

  CommandName        [compute_reachable]

  CommandSynopsis    [Computes the set of reachable states]

  CommandArguments   [\[-h\] \[-k number\]]

  CommandDescription [The set of reachable states is used to simplify
  image and preimage computations. This can result in improved
  performances for models with sparse state spaces.
  <p>
  Command Options:<p>
  <dl>
    <dt> <tt>-k number</tt>
    <dd> Provides an explicit bound to perform at most "number"
    steps.
    <dt> <tt>-t number</tt> <dd> Provides a fail cut-off maximum
    CPU time to halt the computation. This option can be used to limit
    execution time.
  </dl>]

  SideEffects        []

******************************************************************************/
int CommandComputeReachable(int argc, char **argv)
{
  int c, k, t, diameter;
  boolean used_k, used_t, completed;
  BddFsm_ptr fsm;
  BddStates* layers;

  used_k = false;
  used_t = false;

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"t:k:h")) != EOF) {
    switch (c) {
    case 'h': return UsageComputeReachable();
    case 'k':
      {
        int res;

        if(used_k) {
          fprintf(nusmv_stderr, "You cannot specify -k more than once!\n");
        }
        used_k = true;

        res = sscanf(util_optarg, "%d", &k);
        if (res <= 0) {
          fprintf(nusmv_stderr, "You must specify a valid integer number as k!\n");
          return 1;
        }
        if (k <= 0) {
          fprintf(nusmv_stderr, "You must specify a positive number as k!\n");
          return 1;
        }

        break;
      }
    case 't':
      {
        int res;

        if(used_t) {
          fprintf(nusmv_stderr, "You cannot specify -t more than once!\n");
        }
        used_t = true;

        res = sscanf(util_optarg, "%d", &t);
        if (res <= 0) {
          fprintf(nusmv_stderr, "You must specify a valid integer number as time!\n");
          return 1;
        }
        if (t <= 0) {
          fprintf(nusmv_stderr, "You must specify a positive number as time!\n");
          return 1;
        }

        break;
      }
    default:  return UsageComputeReachable();
    }
  }
  if (argc != util_optind) return UsageComputeReachable();

  if (Compile_check_if_model_was_built(nusmv_stderr, true)) return 1;

  fsm = PropDb_master_get_bdd_fsm(PropPkg_get_prop_database());

  if(!used_t) {
    t = -1; /* No limit */
  }

  if(!used_k) {
    k = -1; /* No limit */
  }

  /* Expand the cached reachable states */
  BddFsm_expand_cached_reachable_states(fsm, k, t);

  completed = BddFsm_get_cached_reachable_states(fsm, &layers, &diameter);

  if (completed) {
    fprintf(nusmv_stderr,
            "The computation of reachable states has been completed.\n");
    fprintf(nusmv_stderr,
            "The diameter of the FSM is %d.\n",
            diameter);
  }
  else {
    fprintf(nusmv_stderr,
            "The computation of reachable states has not been completed yet.\n");
    fprintf(nusmv_stderr,
            "The number of performed steps is %d.\n",
            diameter);
  }
  return 0;
}

static int UsageComputeReachable()
{
  fprintf(nusmv_stderr, "usage: compute_reachable [-h] [-k number] [-t time ]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -k \t\tLimit the forward search to number steps forward starting from the last reached frontier.\n");
  fprintf(nusmv_stderr, "   -t \t\tLimit the forward search to time seconds (The limit can be exceeded for the duration of the last cycle).\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Prints the reachable states.]

  CommandName        [print_reachable_states]

  CommandSynopsis    [Prints out information about reachable states]

  CommandArguments   [\[-h\] \[-v\] \[-f\] \[-d\] \[-o filename\] ]

  CommandDescription [Prints the number of reachable states of the
  given model. In verbose mode, prints also the list of all reachable
  states.  The reachable states are computed if needed.<p>

  Command Options:
  <dl>
  <dt> <tt>-v</tt>
  <dd> Verbosely prints the list of reachable states.
  <dt> <tt>-f</tt>
  <dd> Print the list of reachable states as a formula.
  <dt> <tt>-d</tt>
  <dd> Prints the list of reachable states with defines (Requires -v).
  <dt> <tt>-o filename </tt>
  <dd> Prints the result on the specified filename instead of on standard output
  </dl>
]

  SideEffects        []

******************************************************************************/
int CommandPrintReachableStates(int argc, char **argv)
{
  int c;
  boolean verbose = false;
  boolean formula = false;
  boolean print_defines = false;
  char* filename;

  filename = (char*) NULL;

  /* Parse the command line */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hfvdo:")) != EOF) {
    switch (c) {
    case 'h': return UsagePrintReachableStates();
    case 'f':
      formula = true;
      break;
    case 'v':
      verbose = true;
      break;
    case 'd':
      print_defines = true;
      break;
    case 'o':
      if ((char*)NULL != filename) {
        FREE(filename);
        return UsagePrintReachableStates();
      }

      filename = util_strsav(util_optarg);
      break;
    default:
      if ((char*)NULL != filename) FREE(filename);
      return UsagePrintReachableStates();
    }
  }

  if (verbose && formula) {
    if ((char*)NULL != filename) FREE(filename);
    return UsagePrintReachableStates();
  }

  if (print_defines && formula) {
    fprintf(nusmv_stderr, "-f and -d are non combinable!\n");
    if ((char*)NULL != filename) FREE(filename);
    return UsagePrintReachableStates();
  }

  if(print_defines && !(verbose)) {
    fprintf(nusmv_stderr, "-d requires -v option!\n");
    if ((char*)NULL != filename) FREE(filename);
    return UsagePrintReachableStates();
  }

  if (Compile_check_if_model_was_built(nusmv_stderr, true)) return 1;

  set_forward_search(OptsHandler_get_instance());
  set_print_reachable(OptsHandler_get_instance());

  /* Printing */
  FILE* stream = nusmv_stdout;
  CATCH {
    if ((char*)NULL != filename) {
      stream = fopen(filename, "w");

      if ((FILE*) NULL == stream) {
        fprintf(nusmv_stderr, "Unable to open specified file.\n");
        FREE(filename);
        return 1;
      }
    }

    fprintf(stream,
            "#####################################"\
            "#################################\n");
    BddFsm_print_reachable_states_info(
                      PropDb_master_get_bdd_fsm(PropPkg_get_prop_database()),
                      verbose, print_defines, formula, stream);
    fprintf(stream,
            "#####################################"\
            "#################################\n");

    if ((char*)NULL != filename) {
      fclose(stream);
      FREE(filename);
    }
  }
  FAIL {
    if ((char*)NULL != filename) {
      fclose(stream);
      FREE(filename);
    }
  }

  return 0;
}

static int UsagePrintReachableStates()
{
  fprintf(nusmv_stderr,
          "usage: print_reachable_states [-h] [-v] [-d] [-f] [-o filename]\n");

fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -v \t\tPrints the list of reachable states.\n");
  fprintf(nusmv_stderr, "   -d \t\tPrints the list of reachable states with ");
  fprintf(nusmv_stderr, "defines (Requires -v).\n");
  fprintf(nusmv_stderr, "   -f \t\tPrints the formula representing the ");
  fprintf(nusmv_stderr, "reachable states.\n");
  fprintf(nusmv_stderr, "   -o filename\tPrints the result on the specified ");
  fprintf(nusmv_stderr, "filename instead of on standard output\n");

  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints the fair states.]

  CommandName        [print_fair_states]

  CommandSynopsis    [Prints out information about fair states]

  CommandArguments   [\[-h\] \[-v\]]

  CommandDescription [This command provides information about the fair
  states of the current model.number of fair states. In verbose mode,
  prints also the list of fair states.<p>

  Command Options:
  <dl>
    <dt> <tt>-v</tt>
    <dd> Verbosely prints the list of fair states.
  </dl>]

  SideEffects        []

******************************************************************************/
int CommandPrintFairStates(int argc, char **argv)
{
  int c;
  boolean verbose = false;
  /*
   * Parse the command line.
  */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hv")) != EOF) {
    switch (c) {
    case 'h': return UsagePrintFairStates();
    case 'v':
      verbose = true;
      break;
    default:
      return UsagePrintFairStates();
    }
  }

  if (Compile_check_if_model_was_built(nusmv_stderr, true)) return 1;

  fprintf(nusmv_stdout,
          "######################################################################\n");
  BddFsm_print_fair_states_info(
               PropDb_master_get_bdd_fsm(PropPkg_get_prop_database()),
               verbose, nusmv_stdout);
  fprintf(nusmv_stdout,
          "######################################################################\n");

  return 0;
}

static int UsagePrintFairStates()
{
  fprintf(nusmv_stderr, "usage: print_fair_states [-h] [-v]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -v \t\tPrints the list of fair states.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints the fair transitions.]

  CommandName        [print_fair_transitions]

  CommandSynopsis    [Prints the number of fair transitions]

  CommandArguments   [\[-h\] \[-v\]]

  CommandDescription [Prints the number of fair transitions. In
  verbose mode, prints also the list of fair transitions.<p>

  Remark:
    Not really transitions, but rather state_input pairs.<p>

    Command Options:
    <dl>
    <dt> <tt>-v</tt>
    <dd> Verbosely prints the list of fair states.
  </dl>]
  ]

  SideEffects        []

******************************************************************************/
int CommandPrintFairTransitions(int argc, char **argv)
{
  int c;
  boolean verbose = false;

  /* Parse the command line */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hv")) != EOF) {
    switch (c) {
    case 'h': return UsagePrintFairTransitions();
    case 'v':
      verbose = true;
      break;
    default:
      return UsagePrintFairTransitions();
    }
  }

  if (Compile_check_if_model_was_built(nusmv_stderr, true)) return 1;

  fprintf(nusmv_stdout,
          "######################################################################\n");
  BddFsm_print_fair_transitions_info(
                PropDb_master_get_bdd_fsm(PropPkg_get_prop_database()),
                verbose, nusmv_stdout);
  fprintf(nusmv_stdout,
          "######################################################################\n");

  return 0;
}

static int UsagePrintFairTransitions()
{
  fprintf(nusmv_stderr, "usage: print_fair_transitions [-h] [-v]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -v \t\tPrints the list of fair transitions.\n");
  return 1;
}



/**Function********************************************************************

  Synopsis           [Dumps selected parts of the bdd fsm, with optional 
  expression]

  CommandName        [dump_fsm]

  CommandSynopsis [Dumps (in DOT format) selected parts of the bdd
  fsm, with optional expression]

  CommandArguments   [\[-h\] -o filename \[-i\] \[-I\] \[-t\] \[-f\] \[-r\] \[-e expression\]]

  CommandDescription [Dumps selected parts of the bdd fsm, with
  optional expression, in DOT format. At least one among options
  \[iIte\] must be specified.

    Command Options:

    <dt> <tt>-o filename</tt>
    <dd> Dumps to the specified file name.

    <dt> <tt>-i</tt>
    <dd> Dumps the initial states of the FSM, among with other
    selected outputs.

    <dt> <tt>-I</tt>
    <dd> Dumps the invariant states of the FSM, among with other
    selected outputs.

    <dt> <tt>-t</tt>
    <dd> Dumps the (monolithic) transition relation of the FSM, among with other
    selected outputs.

    <dt> <tt>-F</tt>
    <dd> Dumps the (monolithic) fair states of the FSM, among with other
    selected outputs.

    <dt> <tt>-r</tt>
    <dd> Dumps the (monolithic) reachable states of the FSM, among with other
    selected outputs.

    <dt> <tt>-e</tt>
    <dd> Dumps the specified expression, among with other
    selected outputs (see also command dump_expr).
  </dl>]
  ]

  SideEffects        []

******************************************************************************/
int CommandDumpFsm(int argc, char **argv)
{
  int c;
  int res = 0;
  boolean init = false;
  boolean invar = false;
  boolean trans = false;
  boolean fair = false;
  boolean reachable = false;
  char* str_constr = (char*) NULL;
  char* fname = (char*) NULL;
  FILE* outfile = (FILE*) NULL; 
  
  /* Parse the command line */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "ho:e:iItFr")) != EOF) {
    switch (c) {
    case 'h': 
      res = UsageDumpFsm();
      goto dump_fsm_quit;

    case 'i': init = true; break;
    case 'I': invar = true; break;
    case 't': trans = true; break;
    case 'F': fair = true; break;
    case 'r': reachable = true; break;

    case 'e': 
      if ((char*) NULL != str_constr) FREE(str_constr);
      str_constr = util_strsav(util_optarg);
      break;

    case 'o':
      if ((char*) NULL != fname) FREE(fname);
      fname = util_strsav(util_optarg);
      break;

    default:
      res = 1;
      goto dump_fsm_quit;
    }
  }

  /* preconditions */
  if (Compile_check_if_model_was_built(nusmv_stderr, true)) {
    res = 1;
    goto dump_fsm_quit;
  }

  /* checks and processes arguments */
  if ((char*) NULL == fname) {
    fprintf(nusmv_stderr, "Output file must be specified\n");
    res = 1;
    goto dump_fsm_quit;
  }
  
  if ((((char*) NULL != str_constr) + init + trans + invar + 
       fair + reachable) == 0) {
    fprintf(nusmv_stderr, "At least one option in 'eiItFr' must be specified.\n");
    res = 1;
    goto dump_fsm_quit;
  }
  
  outfile = fopen(fname, "w");
  if ((FILE*) NULL == outfile) {
    fprintf(nusmv_stderr, "Problems opening output file '%s'.\n", fname);
    res = 1;
    goto dump_fsm_quit;     
  }

  {
    BddFsm_ptr fsm = PropDb_master_get_bdd_fsm(PropPkg_get_prop_database());
    BddEnc_ptr bdd_enc = BddFsm_get_bdd_encoding(fsm);
    DdManager* dd = BddEnc_get_dd_manager(bdd_enc);

    AddArray_ptr aar_expr = (AddArray_ptr) NULL;
    AddArray_ptr addarray;
    const char** labels;

    /* handling of the (possibly specified) expression */
    if ((char*) NULL != str_constr) {
      /* flattens and checks the given cosntraint */
      SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(bdd_enc));
      TypeChecker_ptr tc = SymbTable_get_type_checker(st);
      SymbType_ptr tp;
      node_ptr parsed_expr = Nil;
      node_ptr node_expr;
  
      if (Parser_ReadNextExprFromString(str_constr, &parsed_expr)) {
        res = 1;
        goto dump_fsm_quit;
      }

      node_expr =  Compile_FlattenSexp(st, car(parsed_expr), Nil);

      tp = TypeChecker_get_expression_type(tc, node_expr, Nil);
      if (SymbType_is_error(tp)) {
        fprintf(nusmv_stderr, "Type of expression is not correct.\n");
        res = 1;
        goto dump_fsm_quit;
      }
      if (SymbType_is_real(tp) ||
          SymbType_is_string(tp) || 
          SymbType_is_statement(tp)) {
        fprintf(nusmv_stderr, "Type of expression is not supported.\n");
        res = 1;
        goto dump_fsm_quit;
      }

      aar_expr = BddEnc_expr_to_addarray(bdd_enc, node_expr, Nil);
    }
    
    /* here we construct the input for BddEnc_dump_addarray_dot */
    {
      struct {
        boolean enabled;
        const char* label;
        bdd_ptr bdd;        
      } info[] = {
        { init, "Init", BddFsm_get_init(fsm) },
        { invar, "Invar", BddFsm_get_state_constraints(fsm) },
        { trans, "Trans", BddFsm_get_monolithic_trans_bdd(fsm) },
        { fair, "Fair", BddFsm_get_fair_states(fsm) },
        { reachable, "Reachables", BddFsm_get_reachable_states(fsm) },
      };
      int i, idx;
      int entries = init + invar + trans + fair + reachable;     
      if ((AddArray_ptr) NULL != aar_expr) {
        entries += AddArray_get_size(aar_expr);
      }

      nusmv_assert(entries > 0);
      addarray = AddArray_create(entries);
      labels = ALLOC(const char*, entries);
      nusmv_assert((const char**) NULL != labels);
 
      for (idx=0, i=0; i<sizeof(info)/sizeof(info[0]) && idx<entries; ++i) {
        if (info[i].enabled) {
          labels[idx] = util_strsav(info[i].label);
          AddArray_set_n(addarray, idx, bdd_to_add(dd, info[i].bdd));
          ++idx;
        }
      }

      /* adds all labels and adds coming possibly from the given expression */
      if ((AddArray_ptr) NULL != aar_expr) {        
        const char* oname_fmt = "%s[%0*d]";
        const int digits = (int) log10(AddArray_get_size(aar_expr));
        const int oname_len = (strlen(str_constr) + strlen(oname_fmt) + digits + 1);

        /* keeps going from last reached idx */
        for (i=idx; i<entries; ++i) {
          char* oname = ALLOC(char, oname_len);
          int c;
          nusmv_assert((char*) NULL != oname);
          c = snprintf(oname, oname_len, oname_fmt, str_constr, digits, i-idx);
          SNPRINTF_CHECK(c, oname_len);

          labels[i] = oname;
          AddArray_set_n(addarray, i, add_dup(AddArray_get_n(aar_expr, i-idx)));
        }
      }

      res = BddEnc_dump_addarray_dot(bdd_enc, addarray, labels, outfile);      
      
      /* cleanup */
      for (i=0; i<entries; ++i) { FREE(labels[i]); }
      FREE(labels); 
      for (i=0; i<sizeof(info)/sizeof(info[0]); ++i) {
        if ((bdd_ptr) NULL != info[i].bdd) { bdd_free(dd, info[i].bdd); }
      }
      
      AddArray_destroy(dd, addarray);
      if ((AddArray_ptr) NULL != aar_expr) { AddArray_destroy(dd, aar_expr); }
    }
  }

 dump_fsm_quit:
  if ((char*) NULL != str_constr) { FREE(str_constr); }
  if ((char*) NULL != fname) { FREE(fname); }
  if ((FILE*) NULL != outfile) { 
    fclose(outfile);
  }
  return res;  
}


static int UsageDumpFsm()
{
  fprintf(nusmv_stderr, "usage: dump_fsm [-h] -o <fname> [-i][-I][-t][-F][-r][-e <expr>\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -o fname \tDumps to the specified file.\n");
  fprintf(nusmv_stderr, "   -e expr \tDumps the specified expression.\n");
  fprintf(nusmv_stderr, "   -i \t\tDumps the initial states.\n");
  fprintf(nusmv_stderr, "   -I \t\tDumps the invariant states.\n");
  fprintf(nusmv_stderr, "   -t \t\tDumps the transition relation.\n");
  fprintf(nusmv_stderr, "   -F \t\tDumps the fair states.\n");
  fprintf(nusmv_stderr, "   -r \t\tDumps the reachable states.\n");
  return 1;
}
