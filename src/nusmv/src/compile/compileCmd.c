/**CFile***********************************************************************

  FileName    [compileCmd.c]

  PackageName [compile]

  Synopsis    [Shell interface for the compile package.]

  Description [This file contains the interface of the compile package
  with the interactive shell.]

  SeeAlso     [cmdCmd.c]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2.
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

#include "utils/defs.h"
#include "utils/assoc.h"
#include "compileInt.h"
#include "symb_table/SymbTable.h"
#include "symb_table/symb_table.h"
#include "PredicateNormaliser.h"

#include "parser/symbols.h"
#include "parser/parser.h"

#include "cmd/cmd.h"

#include "fsm/FsmBuilder.h"
#include "fsm/sexp/SexpFsm.h"
#include "fsm/sexp/BoolSexpFsm.h"
#include "fsm/bdd/BddFsm.h"

#include "prop/propPkg.h"
#include "mc/mc.h"
#include "enc/enc.h"

#include "trace/pkg_trace.h"
#include "trace/exec/PartialTraceExecutor.h"
#include "trace/exec/BDDPartialTraceExecutor.h"

#include "trace/exec/CompleteTraceExecutor.h"
#include "trace/exec/BDDCompleteTraceExecutor.h"

#include "utils/ucmd.h"
#include "utils/utils_io.h"
#include "utils/error.h" /* for CATCH */
#include "utils/Sset.h"
#include "utils/Olist.h"

#include <stdlib.h> /* for strtol */
#include "utils/portability.h" /* for errno */
static char rcsid[] UTIL_UNUSED = "$Id: compileCmd.c,v 1.42.2.28.2.2.2.46.4.96 2010-03-01 10:27:24 nusmv Exp $";


#define RC_EXPERIMENTAL_CODE_PREDICATES 1


/* prototypes of the command functions */
int CommandProcessModel ARGS((int argc, char **argv));
int CommandFlattenHierarchy ARGS((int argc, char **argv));
int CommandShowVars   ARGS((int argc, char **argv));
int CommandEncodeVariables ARGS((int argc, char **argv));
int CommandBuildModel ARGS((int argc, char **argv));
int CommandBuildFlatModel ARGS((int argc, char **argv));
int CommandBuildBooleanModel ARGS((int argc, char **argv));
int CommandDumpModel ARGS((int argc, char **argv));
int CommandAddTrans ARGS((int argc, char **argv));
int CommandAddInit ARGS((int argc, char **argv));
int CommandAddFairness ARGS((int argc, char **argv));
int CommandRestoreModel ARGS((int argc, char **argv));
int CommandWriteOrder ARGS((int argc, char **argv));
int CommandIwls95PrintOption ARGS((int argc, char **argv));
int CommandCPPrintClusterInfo ARGS((int argc, char **argv));
int CommandPrintFsmStats ARGS((int argc, char **argv));
int CommandGo ARGS((int argc, char **argv));
int CommandGoBmc ARGS((int argc, char **argv));
int CommandGetInternalStatus ARGS((int argc, char **argv));
int CommandWriteModelFlat ARGS((int argc, char **argv));
int CommandWriteModelFlatUdg ARGS((int argc, char **argv));
int CommandWriteModelFlatBool ARGS((int argc, char **argv));
int CommandWriteCoiModel ARGS((int argc, char **argv));
int CommandShowDependencies ARGS((int argc, char **argv));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageProcessModel ARGS((void));
static int UsageFlattenHierarchy ARGS((void));
static int UsageShowVars   ARGS((void));
static int UsageBuildModel ARGS((void));
static int UsageBuildFlatModel ARGS((void));
static int UsageBuildBooleanModel ARGS((void));
static int UsageEncodeVariables ARGS((void));
static int UsageWriteOrder ARGS((void));
static int UsageIwls95PrintOption ARGS((void));
static int UsageGo ARGS((void));
static int UsageGoBmc ARGS((void));
static int UsageGetInternalStatus ARGS((void));
static int UsageWriteModelFlat ARGS((void));
static int UsageWriteModelFlatUdg ARGS((void));
static int UsageWriteModelFlatBool ARGS((void));
static int UsagePrintFsmStats ARGS((void));
static int UsageWriteCoiModel ARGS((void));
static int UsageShowDependencies ARGS((void));

static void compile_create_flat_model ARGS((void));
static void compile_create_boolean_model ARGS((void));

/* For write_coi_model */
static Expr_ptr
compile_cmd_remove_assignments ARGS((Expr_ptr expr));

static void compile_cmd_write_coi_prop_fsm ARGS((FlatHierarchy_ptr fh,
                                                 Set_t cone, Set_t props,
                                                 FILE* output_file));

static void compile_cmd_write_coi_prop ARGS((Set_t cone, Set_t props,
                                             FILE* output_file));

static void print_summary ARGS((FILE * file, SymbTable_ptr st,
                                NodeList_ptr list, const char * str,
                                boolean limit_output));

static int get_bits ARGS((const SymbTable_ptr st, const NodeList_ptr lst));

static void compile_cmd_write_global_coi_fsm ARGS((FlatHierarchy_ptr hierarchy,
                                                   Prop_Type prop_type,
                                                   FILE* output_file));

static int compile_cmd_write_properties_coi ARGS((FlatHierarchy_ptr hierarchy,
                                                  Prop_Type prop_type,
                                                  boolean only_dump_coi,
                                                  const char* file_name));

static void compile_cmd_print_type ARGS((FILE * file, node_ptr ntype,
                                         int threshold));
static node_ptr compile_cmd_get_var_type ARGS((SymbType_ptr type));

/* not declared static b/c these are supposed to be used by the Python
   wrapper */
int compile_encode_variables(void);
int compile_flatten_smv(boolean calc_vars_constrains);
void compile_build_model(boolean force_build);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the compile package.]

  Description        [Initializes the compile package. The set of commands must
  be explicitly initialized later by calling Compile_InitCmd.]

  SideEffects        []

******************************************************************************/
void Compile_init()
{
  cmps = cmp_struct_init();

  init_check_constant_hash();
  init_check_next_hash();
  init_expr2bexp_hash();
  init_consts_hash();
  init_dependencies_hash();
  init_define_dep_hash();
  init_coi_hash();
  init_coi0_hash();

  /* the global fsm builder creation */
  nusmv_assert(global_fsm_builder == FSM_BUILDER(NULL));
  global_fsm_builder = FsmBuilder_create(dd_manager);

  /* modules and sub packages initialization */
  SymbTablePkg_init();

  /* the global predicate-normaliser creation (must be done after
     creation of global type-checker) */
  nusmv_assert(PREDICATE_NORMALISER(NULL) == global_predication_normaliser);
  global_predication_normaliser =
    PredicateNormaliser_create(Compile_get_global_symb_table());
}

/**Function********************************************************************

  Synopsis           [Initializes the commands provided by this package]

  Description        []

  SideEffects        []

******************************************************************************/
void Compile_init_cmd()
{
  Cmd_CommandAdd("process_model", CommandProcessModel, 0, false);
  Cmd_CommandAdd("flatten_hierarchy", CommandFlattenHierarchy, 0, false);
  Cmd_CommandAdd("show_vars", CommandShowVars, 0, true);
  Cmd_CommandAdd("encode_variables", CommandEncodeVariables, 0, false);
  Cmd_CommandAdd("build_model", CommandBuildModel, 0, false);
  Cmd_CommandAdd("build_flat_model", CommandBuildFlatModel, 0, false);
  Cmd_CommandAdd("build_boolean_model", CommandBuildBooleanModel, 0, false);
  Cmd_CommandAdd("write_order", CommandWriteOrder, 0, true);
  Cmd_CommandAdd("print_iwls95options", CommandIwls95PrintOption, 0, true);

  /* this is deprecated in 2.4 */
  Cmd_CommandAdd("print_clusterinfo", CommandCPPrintClusterInfo, 0, true);

  Cmd_CommandAdd("print_fsm_stats", CommandPrintFsmStats, 0, true);
  Cmd_CommandAdd("go", CommandGo, 0, false);

#if NUSMV_HAVE_SAT_SOLVER
  Cmd_CommandAdd("go_bmc", CommandGoBmc, 0, false);
#endif

  Cmd_CommandAdd("get_internal_status", CommandGetInternalStatus, 0, true);
  Cmd_CommandAdd("write_flat_model", CommandWriteModelFlat, 0, true);
  Cmd_CommandAdd("write_flat_model_udg", CommandWriteModelFlatUdg, 0, true);
  Cmd_CommandAdd("write_boolean_model", CommandWriteModelFlatBool, 0, true);

  Cmd_CommandAdd("write_coi_model", CommandWriteCoiModel, 0, true);

  Cmd_CommandAdd("show_dependencies", CommandShowDependencies, 0, true);
}

/**Function********************************************************************

  Synopsis           [Shut down the compile package]

  Description        [Shut down the compile package]

  SideEffects        []

******************************************************************************/
void Compile_quit()
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Shutting down the compiler...\n");
  }

  /* the global predicate-normaliser destrution */
  PREDICATE_NORMALISER_CHECK_INSTANCE(global_predication_normaliser);
  PredicateNormaliser_destroy(global_predication_normaliser);
  global_predication_normaliser = PREDICATE_NORMALISER(NULL);

  /* modules and sub packages initialization */
  SymbTablePkg_quit();
  CompileFlatten_quit_flattener();

  if (FLAT_HIERARCHY(NULL) != mainFlatHierarchy) {
    FlatHierarchy_destroy(mainFlatHierarchy);
    mainFlatHierarchy = FLAT_HIERARCHY(NULL);
  }

  /* local structures: */
  if (global_fsm_builder != FSM_BUILDER(NULL)) {
    FsmBuilder_destroy(global_fsm_builder);
    global_fsm_builder = FSM_BUILDER(NULL);
  }

  clear_coi0_hash();
  clear_coi_hash();
  clear_define_dep_hash();
  clear_dependencies_hash();
  clear_consts_hash();
  clear_expr2bexp_hash();
  clear_check_constant_hash();
  clear_check_next_hash();

  cmp_struct_quit(cmps);
  cmps = (cmp_struct_ptr) NULL;

  deinit_check_constant_hash();
  deinit_check_next_hash();
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Performs the batch steps and then returns
  control to the interactive shell.]

  CommandName        [process_model]

  CommandSynopsis    [Performs the batch steps and then returns control
  to the interactive shell.]

  CommandArguments   [\[-h\] \[-f\] \[-r\] \[-i model-file\] \[-m Method\]]

  CommandDescription [ Reads the model, compiles it into BDD and
  performs the model checking of all the specification contained in
  it. If the environment variable <tt>forward_search</tt> has been set
  before, then the set of reachable states is computed. If the
  option <tt>-r</tt> is specified, the reordering of variables is
  performed accordingly. This command simulates the batch behavior of
  NuSMV and then returns the control to the interactive shell.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-f</tt>
       <dd> Forces model construction even when COI is enabled.
    <dt> <tt>-r</tt>
       <dd> Performs a variable ordering at the end of the
       computation, and dumps the variable ordering as the command
       line option <tt>-reorder</tt> does.
    <dt> <tt>-i model-file</tt>
       <dd> Sets the environment variable <tt>input_file</tt> to file
           <tt>model-file</tt>, and reads the model from file
           <tt>model-file</tt>.
    <dt> <tt>-m Method</tt>
       <dd> Sets the environment variable <tt>partition_method</tt> to
       <tt>Method</tt> and uses it as partitioning method.
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandProcessModel(int argc, char **argv)
{
  int c;
  char * partition_method = NIL(char);
  boolean force_reordering = false;
  boolean force_build = false;

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"hfri:m:")) != EOF){
    switch (c) {
    case 'f': force_build = true; break;
    case 'r': force_reordering = true; break;
    case 'i': {
      set_input_file(OptsHandler_get_instance(), util_optarg);
      break;
    }
    case 'm': {
      partition_method = ALLOC(char, strlen(util_optarg)+1);
      strcpy(partition_method, util_optarg);
      break;
    }
    case 'h': goto CommandProcessModel_exit_usage;
    default: goto CommandProcessModel_exit_usage;
    }
  }

  if (argc != util_optind) goto CommandProcessModel_exit_usage;

  if (get_input_file(OptsHandler_get_instance()) == (char *)NULL) {
    fprintf(nusmv_stderr, "Input file is (null). You must set the input file before.\n");
    goto CommandProcessModel_exit_1;
  }

  if (partition_method != NIL(char)) {
    if (TransType_from_string(partition_method) != TRANS_TYPE_INVALID) {
      set_partition_method(OptsHandler_get_instance(), TransType_from_string(partition_method));
    } else {
      fprintf(nusmv_stderr, "The only possible values for \"-m\" option are:\n\t");
      print_partition_method(nusmv_stderr);
      fprintf(nusmv_stderr, "\n");
      goto CommandProcessModel_exit_1;
    }
  }

  if (cmp_struct_get_read_model(cmps) == 0)
    if (Cmd_CommandExecute("read_model")) goto CommandProcessModel_exit_1;
  if (cmp_struct_get_flatten_hrc(cmps) == 0)
    if (Cmd_CommandExecute("flatten_hierarchy")) goto CommandProcessModel_exit_1;
  if (cmp_struct_get_encode_variables(cmps) == 0)
    if (Cmd_CommandExecute("encode_variables")) goto CommandProcessModel_exit_1;
  if (cmp_struct_get_build_model(cmps) == 0) {
    if (!force_build) {
      if(Cmd_CommandExecute("build_model")) goto CommandProcessModel_exit_1;
    }
    else if(Cmd_CommandExecute("build_model -f")) {
      goto CommandProcessModel_exit_1;
    }
  }
  if (opt_forward_search(OptsHandler_get_instance()))
    if (Cmd_CommandExecute("compute_reachable")) goto CommandProcessModel_exit_1;

  if (opt_check_fsm(OptsHandler_get_instance()))
    if (Cmd_CommandExecute("check_fsm")) goto CommandProcessModel_exit_1;

  if (! opt_ignore_spec(OptsHandler_get_instance()))
    if (Cmd_CommandExecute("check_ctlspec")) goto CommandProcessModel_exit_1;

  if (! opt_ignore_compute(OptsHandler_get_instance()))
    if (Cmd_CommandExecute("check_compute")) goto CommandProcessModel_exit_1;

  if (! opt_ignore_ltlspec(OptsHandler_get_instance()))
    if (Cmd_CommandExecute("check_ltlspec")) goto CommandProcessModel_exit_1;

  if (! opt_ignore_pslspec(OptsHandler_get_instance()))
    if (Cmd_CommandExecute("check_pslspec")) goto CommandProcessModel_exit_1;

  if (! opt_ignore_invar(OptsHandler_get_instance()))
    if (Cmd_CommandExecute("check_invar")) goto CommandProcessModel_exit_1;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0))
    if (Cmd_CommandExecute("print_usage")) goto CommandProcessModel_exit_1;

  if (force_reordering) { /* If the case activate reordering */
    fprintf(nusmv_stdout, "\n========= starting reordering ============\n");
    dd_reorder(dd_manager, get_reorder_method(OptsHandler_get_instance()), DEFAULT_MINSIZE);

    if (Cmd_CommandExecute("write_order")) goto CommandProcessModel_exit_1;

    fprintf(nusmv_stdout, "\n========= after reordering ============\n");

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0))
      if (Cmd_CommandExecute("print_usage")) goto CommandProcessModel_exit_1;
  }

  if (partition_method != NIL(char)) FREE(partition_method);
  return 0;

 CommandProcessModel_exit_1:
  if (partition_method != NIL(char)) FREE(partition_method);
  return 1;

 CommandProcessModel_exit_usage:
  if (partition_method != NIL(char)) FREE(partition_method);
  return(UsageProcessModel());
}

static int UsageProcessModel()
{
  fprintf(nusmv_stderr, "usage: process_model [-r] [-h] [-i model-file] [-m method]\n");
  fprintf(nusmv_stderr, "   -h \t\t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -f \t\t\tForces model construction.\n");
  fprintf(nusmv_stderr, "   -r \t\t\tForces a reordering and dumps the new vars order.\n");
  fprintf(nusmv_stderr, "   -i model-file \tReads the model from file \"model-file\".\n");
  fprintf(nusmv_stderr, "   -m method\t\tUses \"method\" as partition method in model construction.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Flattens the hierarchy of modules]

  CommandName        [flatten_hierarchy]

  CommandSynopsis    [Flattens the hierarchy of modules]

  CommandArguments   [\[-h\] \[-d\]]

  CommandDescription [
  This command is responsible of the instantiation of modules and
  processes. The instantiation is performed by substituting the actual
  parameters for the formal parameters, and then by prefixing the result via
  the instance name.
  <p>
  Command options:<p>
  <dl>
    <dt><tt>-d</tt>
     <dd>Delays the construction of vars constraints until needed
    </dl>
]

  SideEffects        []

******************************************************************************/
int CommandFlattenHierarchy(int argc, char ** argv)
{
  int c;
  boolean calc_vars_constrains = true;

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hd")) != EOF) {
    switch (c) {
    case 'h': return UsageFlattenHierarchy();
    case 'd': calc_vars_constrains = false; break;

    default:  return UsageFlattenHierarchy();
    }
  }
  if (argc != util_optind) return UsageFlattenHierarchy();
  if (cmp_struct_get_read_model(cmps) == 0) {
    fprintf(nusmv_stderr, "A model must be read before. Use the \"read_model\" command.\n");
    return 1;
  }

  if (cmp_struct_get_flatten_hrc(cmps)) {
    fprintf(nusmv_stderr, "The hierarchy has already been flattened.\n");
    return 1;
  }

  if (cmp_struct_get_hrc_built(cmps)) {
    /* the HRC was already built thanks to lax parser, but there are errors */
    fprintf(nusmv_stderr, "The hierarchy cannot be flattened, as errors have been found.\n");
    fprintf(nusmv_stderr, "At this stage you can dump the (partial) HRC, or use the\n");
    fprintf(nusmv_stderr, "command 'reset' to restart.\n");
    return 1;
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, "Flattening hierarchy...\n");
  }

  return compile_flatten_smv(calc_vars_constrains); /* does the work */
}


static int UsageFlattenHierarchy()
{
  fprintf(nusmv_stderr, "usage: flatten_hierarchy [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n");
  fprintf(nusmv_stderr, "   -d \t\tDelays the construction of vars constraints until needed\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Shows model's symbolic variables and their values]

  CommandName        [show_vars]

  CommandSynopsis    [Shows model's symbolic variables and their values]

  CommandArguments   [\[-h\] \[-s\] \[-f\] \[-i\] \[-v\] \[-t|-V\]
                      \[-m | -o output-file\]]

  CommandDescription [
  Prints symbolic input, frozen and state variables of the model with their
  range of values (as defined in the input file).
  <p>
  Command Options:<p>
  <dl>
    <dt> <tt>-s</tt>
       <dd> Prints state variables.
    <dt> <tt>-f</tt>
       <dd> Prints frozen variables.
    <dt> <tt>-i</tt>
       <dd> Prints input variables.
    <dt> <tt>-t</tt>
       <dd> Prints only the number of variables (among selected
            kinds), grouped by type. Incompatible with -V.
    <dt> <tt>-V</tt>
       <dd> Prints only the list of variables with their types (among
            selected kinds), and no other summary
            information. Incompatible with -t.
    <dt> <tt>-v</tt>
       <dd> Prints verbosely. With this option, all scalar variable values are
            printed
    <dt> <tt>-m</tt>
       <dd> Pipes the output to the program specified by the
           <tt>PAGER</tt> shell variable if defined, else through the
           <tt>UNIX</tt> command "more".
    <dt> <tt>-o output-file</tt>
       <dd> Writes the output generated by the command to <tt>output-file</tt>
  </dl> ]

  SideEffects        []

******************************************************************************/
int CommandShowVars(int argc, char ** argv)
{
  int c = 0;
  boolean statevars = false;
  boolean frozenvars = false;
  boolean inputvars = false;
  boolean verbose = false;
  boolean total_only = false;
  boolean vars_only = false;
  boolean defs_only = false;
  short int useMore = 0;
  char* dbgFileName = NIL(char);
  FILE* old_nusmv_stdout = NIL(FILE);
  SymbTable_ptr st = Compile_get_global_symb_table();

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"hsiftVDvmo:")) != EOF){

    switch (c) {
    case 'h': goto show_vars__usage;
    case 's':
      statevars = true;
      break;

    case 'f':
      frozenvars = true;
      break;

    case 'i':
      inputvars = true;
      break;

    case 't':
      if (vars_only) {
        fprintf(nusmv_stderr, "-t and -V are incompatible options.\n");
        goto show_vars__fail;
      }
      if (defs_only) {
        fprintf(nusmv_stderr, "-D and -V are incompatible options.\n");
        goto show_vars__fail;
      }
      total_only = true;
      break;

    case 'V':
      if (total_only) {
        fprintf(nusmv_stderr, "-D and -V are incompatible options.\n");
        goto show_vars__fail;
      }
      vars_only = true;
      break;

    case 'D':


      if (vars_only) {
        fprintf(nusmv_stderr, "-D and -V are incompatible options.\n");
        goto show_vars__fail;
      }
      defs_only = true;
      break;

    case 'v':
      verbose = true;
      break;

    case 'o':
      if (useMore == 1) goto show_vars__usage;
      dbgFileName = util_strsav(util_optarg);
      fprintf(nusmv_stdout, "Output to file: %s\n", dbgFileName);
      break;

    case 'm':
      if (dbgFileName != NIL(char)) goto show_vars__usage;
      useMore = 1;
      break;

    default: goto show_vars__usage;
    }
  }

  if (argc != util_optind) goto show_vars__usage;

  /* if (!statevars && !frozenvars && !inputvars) goto show_vars__usage; */

  /* we need only a flattened hierarchy to be able to see the variables */
  if (Compile_check_if_flattening_was_built(nusmv_stderr)) goto show_vars__fail;

  if (useMore) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenPipe(useMore);
    if (nusmv_stdout==(FILE*) NULL) {
      nusmv_stdout=old_nusmv_stdout;
      goto show_vars__fail;
    }
  }

  if (dbgFileName != NIL(char)) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenFile(dbgFileName);
    if (nusmv_stdout==(FILE*) NULL) {
      nusmv_stdout = old_nusmv_stdout;
      goto show_vars__fail;
    }
  }

  if (!total_only && !defs_only) {
    /* normal printout, not total only */
    NodeList_ptr list = NodeList_create();
    NodeList_ptr alls =  NodeList_create();
    NodeList_ptr allf =  NodeList_create();
    NodeList_ptr alli =  NodeList_create();

    { /* extracts the default class layer names */
      array_t* layer_names = SymbTable_get_class_layer_names(st, NULL);
      char* layer_name;
      int i;

      arrayForEachItem(char*, layer_names, i, layer_name) {
        SymbLayer_ptr layer = SymbTable_get_layer(st, layer_name);
        SymbLayerIter iter;

        SYMB_LAYER_FOREACH(layer, iter, STT_VAR) {
          node_ptr var = SymbLayer_iter_get_symbol(layer, &iter);

          if (SymbTable_is_symbol_state_var(st, var)) {
            if (!vars_only) { NodeList_append(alls, var); }
            if (statevars) { NodeList_append(list, var); }
          }
          else if (SymbTable_is_symbol_frozen_var(st, var)) {
            if (!vars_only) { NodeList_append(allf, var); }
            if (frozenvars) { NodeList_append(list, var); }
          }
          else if (SymbTable_is_symbol_input_var(st, var)) {
            if (!vars_only) { NodeList_append(alli, var); }
            if (inputvars) { NodeList_append(list, var); }
          }
          else { error_unreachable_code(); }
        }
      }
    }

    if (!vars_only) {
      print_summary(nusmv_stdout, st, alli, "Input Variables", !verbose);
      print_summary(nusmv_stdout, st, alls, "State Variables", !verbose);
      print_summary(nusmv_stdout, st, allf, "Frozen Variables", !verbose);
    }

    set_indent_size(1);
    { /* prints the variables */
      ListIter_ptr iter;
      NODE_LIST_FOREACH(list, iter) {
        node_ptr name = NodeList_get_elem_at(list, iter);
        node_ptr ntype;
        char* s_name;
        /* only vars may be met here */
        nusmv_assert(SymbTable_is_symbol_var(st, name));

        s_name = sprint_node(name);
        fprintf(nusmv_stdout, "%s : ", s_name);

        ntype = compile_cmd_get_var_type(SymbTable_get_var_type(st, name));
        compile_cmd_print_type(nusmv_stdout, ntype,
                               (verbose ? 0 : (80 - strlen(s_name) - 3)));

        fprintf(nusmv_stdout, "\n");
        FREE(s_name);
      } /* for */
    }
    reset_indent_size();

    if (!vars_only) {
      /* Print Bits */
      int frozen, input, state;

      input = get_bits(st, alli);
      frozen = get_bits(st, allf);
      state = get_bits(st, alls);

      fprintf(nusmv_stdout,
              "Number of bits: %d (%d frozen, %d input, %d state)\n",
              frozen + input + state,
              frozen,
              input,
              state);
    }

    NodeList_destroy(alls);
    NodeList_destroy(allf);
    NodeList_destroy(alli);
    NodeList_destroy(list);
  }

  else if (!total_only && defs_only) {
    set_indent_size(1);

    { /* prints the defines */
      SymbTableIter iter;
      TypeChecker_ptr tc = SymbTable_get_type_checker(st);

      SYMB_TABLE_FOREACH(st, iter, STT_DEFINE) {
        node_ptr name = SymbTable_iter_get_symbol(st, &iter);
        SymbType_ptr type = TypeChecker_get_expression_type(tc, name, Nil);

        nusmv_assert(SymbTable_is_symbol_define(st, name));

        print_node(nusmv_stdout, name);
        fprintf(nusmv_stdout, " : ");
        SymbType_print(type, nusmv_stdout);
        fprintf(nusmv_stdout, "\n");
      } /* for */
    }
    reset_indent_size();
  }
  else {
    /* total only */
    NodeList_ptr list = NodeList_create();

    /* extracts the default class layer names */
    array_t* layer_names = SymbTable_get_class_layer_names(st, NULL);
    char* layer_name;
    int i;

    arrayForEachItem(char*, layer_names, i, layer_name) {
      SymbLayer_ptr layer = SymbTable_get_layer(st, layer_name);
      SymbLayerIter iter;

      SYMB_LAYER_FOREACH(layer, iter, STT_VAR) {
        node_ptr var = SymbLayer_iter_get_symbol(layer, &iter);

        if (SymbTable_is_symbol_state_var(st, var)) {
          if (statevars) { NodeList_append(list, var); }
        }
        else if (SymbTable_is_symbol_frozen_var(st, var)) {
          if (frozenvars) { NodeList_append(list, var); }
        }
        else if (SymbTable_is_symbol_input_var(st, var)) {
          if (inputvars) { NodeList_append(list, var); }
        }
        else { error_unreachable_code(); }
      }
    }

    print_summary(nusmv_stdout, st, list, "all selected variables", !verbose);

    NodeList_destroy(list);
  }

  if (useMore) {
    CmdClosePipe(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }

  if (dbgFileName != NIL(char)) {
    CmdCloseFile(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
    FREE(dbgFileName);
  }
  return 0;

 show_vars__fail:
  if (dbgFileName != NIL(char)) FREE(dbgFileName);
  return 1;

 show_vars__usage:
  if (dbgFileName != NIL(char)) FREE(dbgFileName);
  return UsageShowVars();
}


static int UsageShowVars ()
{
  fprintf(nusmv_stderr,"usage: show_vars [-h] [-s] [-f] [-i] [-v] [-t | -V] [-m | -o file]\n");
  fprintf(nusmv_stderr,"  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr,"  -s \t\tPrints the state variables.\n");
  fprintf(nusmv_stderr,"  -f \t\tPrints the frozen variables.\n");
  fprintf(nusmv_stderr,"  -i \t\tPrints the input variables.\n");
  fprintf(nusmv_stderr,"  -t \t\tPrints only the number of variables (among selected kinds), grouped by type.\n");
  fprintf(nusmv_stderr,"     \t\tThis option is incompatible with -V.\n");
  fprintf(nusmv_stderr,"  -V \t\tPrints only the list of variables with their types (among selected kinds),\n");
  fprintf(nusmv_stderr,"     \t\twith no summary information. This option is incompatible with -t.\n");
  fprintf(nusmv_stderr,"  -v \t\tPrints verbosely.\n");
  fprintf(nusmv_stderr,"  -m \t\tPipes output through the program specified by the \"PAGER\".\n");
  fprintf(nusmv_stderr,"     \t\tenvironment variable if defined, else through the UNIX command \"more\".\n");
  fprintf(nusmv_stderr,"  -o file\tWrites the generated output to \"file\".\n");
  return 1;
}

static void print_summary(FILE * file, SymbTable_ptr st,
                          NodeList_ptr list, const char * str,
                          boolean limit_output)
{
  int j;
  SymbType_ptr type;
  ListIter_ptr iter;
  hash_ptr h = new_assoc();
  node_ptr name, val, ntype;
  assoc_iter aiter;

  fprintf(file, "Number of %s: %d\n", str, NodeList_get_length(list));

  NODE_LIST_FOREACH(list, iter) {
    j = 0;

    name = NodeList_get_elem_at(list, iter);

    /* only vars may be met here */
    nusmv_assert(SymbTable_is_symbol_var(st, name));

    type = SymbTable_get_var_type(st, name);

    ntype = compile_cmd_get_var_type(type);

    j = PTR_TO_INT(find_assoc(h, ntype));

    if (0 == j) {
      insert_assoc(h, ntype, PTR_FROM_INT(node_ptr, 1));
    }
    else {
      j += 1;
      insert_assoc(h, ntype, PTR_FROM_INT(node_ptr, j));
    }
  } /* for */

  ASSOC_FOREACH(h, aiter, &ntype, &val) {
    j = PTR_TO_INT(val);
    fprintf(file, " %4d: ", j);
    compile_cmd_print_type(file, ntype, (limit_output ? 71 : 0));
  }
  free_assoc(h);
}

/**Function********************************************************************

  Synopsis           [Builds the BDD variables necessary to compile the
  model into BDD.]

  CommandName        [encode_variables]

  CommandSynopsis    [Builds the BDD variables necessary to compile the
  model into BDD.]

  CommandArguments   [\[-h\] \[-i order-file\]]

  CommandDescription [
  Generates the boolean BDD variables and the ADD needed to encode
  propositionally the (symbolic) variables declared in the model.<br>

  The variables are created as default in the order in which they
  appear in a depth first traversal of the hierarchy.<p>

  The input order file can be partial and can contain variables not
  declared in the model. Variables not declared in the model are
  simply discarded. Variables declared in the model which are not
  listed in the ordering input file will be created and appended at the
  end of the given ordering list, according to the default ordering.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-i order-file</tt>
       <dd> Sets the environment variable <tt>input_order_file</tt> to
       <tt>order-file</tt>, and reads the variable ordering to be used from
       file <tt>order-file</tt>. This can be combined with the
       <tt>write_order</tt> command. The variable ordering is written to a
       file, which can be inspected and reordered by the user, and then
       read back in.
  </dl>]

  SideEffects        []

******************************************************************************/
int CommandEncodeVariables(int argc, char ** argv)
{
  int c;
  char* input_order_file_name = NIL(char);
  int res = 1;

  util_getopt_reset();
  while ((c = util_getopt(argc,argv,"i:h")) != EOF) {
    switch (c) {
    case 'i':
      /* Option cannot be defined twice */
      if (NIL(char) != input_order_file_name) { goto encode_variables_usage; }
      input_order_file_name = ALLOC(char, strlen(util_optarg)+1);
      strcpy(input_order_file_name, util_optarg);
      break;

    case 'h': goto encode_variables_usage;
    default:  goto encode_variables_usage;
    }
  }

  if (argc != util_optind) goto encode_variables_usage;

  /* pre-conditions: */
  if (Compile_check_if_flattening_was_built(nusmv_stderr)) {
    goto encode_variables_free;
  }

  if (cmp_struct_get_encode_variables(cmps)) {
    fprintf(nusmv_stderr, "The variables appear to be already built.\n");
    goto encode_variables_free;
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Building variables...");
  }

  if (input_order_file_name != NIL(char)) {
    set_input_order_file(OptsHandler_get_instance(), input_order_file_name);
  }

  {
    int res;

    res = compile_encode_variables();

    if (res != 0) { goto encode_variables_free; }
  }

  cmp_struct_set_encode_variables(cmps);

  if (!opt_reorder(OptsHandler_get_instance())
      && !is_default_order_file(OptsHandler_get_instance())
      && !util_is_string_null(get_output_order_file(OptsHandler_get_instance()))) {
    VarOrderingType dump_type;
    if (opt_write_order_dumps_bits(OptsHandler_get_instance())) dump_type = DUMP_BITS;
    else dump_type = DUMP_DEFAULT;

    BddEnc_write_var_ordering(Enc_get_bdd_encoding(),
            get_output_order_file(OptsHandler_get_instance()),
            dump_type);

    /* batch mode: */
    if (opt_batch(OptsHandler_get_instance())) { nusmv_exit(0); }
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "...done\n");
  }

/* ---------------------------------------------------------------------- */

#define RC_EXPERIMENTAL_CODE_INLINER 0
#if RC_EXPERIMENTAL_CODE_INLINER
# warning "RC: experimental code for testing the inliner"
  /* here it is assumed the model is:
-------------------------------------------------
MODULE main
 VAR a : 0..5;
 VAR b : 0..5;
 VAR c : 0..5;
 VAR d : 0..5;
------------------------------------------------- */
#include "sexp/SexpInliner.h"
#define _PRINT(x) (fprintf(nusmv_stderr, "%s", x))

  void simplify_ami(FlatHierarchy_ptr hierarchy)
  {
    SymbTable_ptr sm = FlatHierarchy_get_symb_table(hierarchy);
    SexpInliner_ptr inliner = SexpInliner_create(sm, 0);
    hierarchy = FlatHierarchy_copy(hierarchy);

    /* Prepare the inliner adding usable invariants */
    Expr_ptr invars = FlatHierarchy_get_invar(hierarchy);

    {
      /* invars = "x = y" */
      InlineRes_ptr res = SexpInliner_inline(inliner, node_normalize(invars), NULL);
      SexpInliner_force_equivalences(inliner, InlineRes_get_equivalences(res));
      print_node(nusmv_stderr, InlineRes_get_result(res));
      InlineRes_destroy(res);
    }

    /* Remove unused variables from assign */
    {
      boolean changed = false;
      Expr_ptr assign = FlatHierarchy_get_assign(hierarchy);
      Expr_ptr res = SexpInliner_inline_no_learning(inliner, node_normalize(assign), &changed);

      _PRINT("\nASSIGN: "); print_node(nusmv_stderr, assign); _PRINT("\n");
      _PRINT("NEW ASSIGN: "); print_node(nusmv_stderr, res); _PRINT("\n");
    }

  }

  {
    SexpFsm_ptr fsm = PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());
    simplify_ami(SexpFsm_get_hierarchy(fsm));
  }

#if 0 // old rc code for testing inliner
  {
    SymbTable_ptr st = Compile_get_global_symb_table();
    SexpInliner_ptr sin = SexpInliner_create(st, 0);
    node_ptr expr;
    boolean changed = false;
    InlineRes_ptr ires;
    int res;

    res = Parser_ReadSimpExprFromString("a=b+1 & b=c+2 & c=d+b & d=3", &expr);
    nusmv_assert(res == 0);
    expr = Compile_FlattenSexp(st, car(expr), Nil);
    expr = Expr_attime(expr, 0, sm);

    ires = SexpInliner_inline(sin, expr, &changed);
  }
#endif
#endif // experiments by RC
  /* ---------------------------------------------------------------------- */

  res = 0;
  goto encode_variables_free;

 encode_variables_usage:
  res = UsageEncodeVariables();
 encode_variables_free:
  if (NIL(char) != input_order_file_name) {
    FREE(input_order_file_name);
  }

  return res;
}


static int UsageEncodeVariables()
{
  fprintf(nusmv_stderr, "usage: encode_variables [-h] [-i <file>]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -i <file> \tReads variable ordering from file <file>.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Compiles the flattened hierarchy into BDD]

  CommandName        [build_model]

  CommandSynopsis    [Compiles the flattened hierarchy into BDD]

  CommandArguments   [\[-h\] \[-f\] \[-m Method\]]

  CommandDescription [
  Compiles the flattened hierarchy into BDD (initial states, invariants,
  and transition relation) using the method specified in the environment
  variable <tt>partition_method</tt> for building the transition relation.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-m Method</tt>
       <dd> Sets the environment variable <tt>partition_method</tt> to
           the value <tt>Method</tt>, and then builds the transition
           relation. Available methods are <code>Monolithic</code>,
           <code>Threshold</code> and <code>Iwls95CP</code>.
    <dt> <tt>-f</tt>
       <dd> Forces model construction. By default, only one partition
            method is allowed. This option allows to overcome this
            default, and to build the transition relation with different
            partitioning methods.
  </dl>]

  SideEffects        []

******************************************************************************/
int CommandBuildModel(int argc, char ** argv)
{
  int c;
  boolean force_build = false;
  char * partition_method = NIL(char);

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"m:fh")) != EOF){
    switch(c){
    case 'm': {
      partition_method = ALLOC(char, strlen(util_optarg)+1);
      strcpy(partition_method, util_optarg);
      break;
    }
    case 'f': {
      force_build = true;
      break;
    }
    case 'h': return(UsageBuildModel());
    default:  return(UsageBuildModel());
    }
  }
  if (argc != util_optind) {
    if (partition_method != NIL(char)) {
      FREE(partition_method);
    }
    return(UsageBuildModel());
  }

  /* pre-conditions: */
  if (Compile_check_if_encoding_was_built(nusmv_stderr)) {
    if (partition_method != NIL(char)) {
      FREE(partition_method);
    }
    return 1;
  }

  if (!force_build && cmp_struct_get_build_model(cmps)) {
    fprintf(nusmv_stderr, "A model appears to be already built from file: %s.\n",
            get_input_file(OptsHandler_get_instance()));
    if (partition_method != NIL(char)) {
      FREE(partition_method);
    }
    return 1;
  }

  if (partition_method != NIL(char)) {
    if (TransType_from_string(partition_method) != TRANS_TYPE_INVALID) {
      if ((force_build) &&
          (TransType_from_string(partition_method) == get_partition_method(OptsHandler_get_instance()))) {
        if (cmp_struct_get_build_model(cmps)) {
          fprintf(nusmv_stderr, "A model for the chosen method has already been constructed.\n");
          FREE(partition_method);
          return 1;
        }
      }
      set_partition_method(OptsHandler_get_instance(), TransType_from_string(partition_method));
    } else {
      fprintf(nusmv_stderr, "The only possible values for \"-m\" option are:\n\t");
      print_partition_method(nusmv_stderr);
      fprintf(nusmv_stderr, "\n");
      FREE(partition_method);
      return 1;
    }
  }

  /* constructs the model only if coi is not enabled */
  if (opt_cone_of_influence(OptsHandler_get_instance()) && !force_build) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr,
              "Construction of BDD model is delayed due to use of COI\n");
    }
    if (partition_method != NIL(char)) {
      FREE(partition_method);
    }
    return 0;
  }

  compile_build_model(force_build); /* does the work */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr,
            "\nThe model has been built from file %s.\n", get_input_file(OptsHandler_get_instance()));
  }

  { /* register BDD complete and partial executors */
    BddFsm_ptr fsm = PropDb_master_get_bdd_fsm(PropPkg_get_prop_database());
    BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);

    TraceManager_register_complete_trace_executor(
                 TracePkg_get_global_trace_manager(),
                 "bdd", "BDD complete trace execution",
                 COMPLETE_TRACE_EXECUTOR(
                          BDDCompleteTraceExecutor_create(fsm, enc)));

    TraceManager_register_partial_trace_executor(
                 TracePkg_get_global_trace_manager(),
                 "bdd", "BDD partial trace execution",
                 PARTIAL_TRACE_EXECUTOR(
                         BDDPartialTraceExecutor_create(fsm, enc)));
  }

  /* We keep track that the master FSM has been built. */
  cmp_struct_set_build_model(cmps);

  if (partition_method != NIL(char)) {
    FREE(partition_method);
  }

  return 0;
}

static int UsageBuildModel()
{
  fprintf(nusmv_stderr, "usage: build_model [-h] [-f] [-m Method]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n");
  fprintf(nusmv_stderr, "   -m Method \tUses \"Method\" as partitioning method, and set it as default method\n");
  fprintf(nusmv_stderr, "\t\tto be used in the following image computations.\n");
  fprintf(nusmv_stderr, "\t\tThe currently available methods are:\n\t\t");
  print_partition_method(nusmv_stderr);
  fprintf(nusmv_stderr, "\n   -f \t\tForces the model re-construction, even if a model has already been built\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Compiles the flattened hierarchy into SEXP]

  CommandName        [build_flat_model]

  CommandSynopsis    [Compiles the flattened hierarchy into SEXP]

  CommandArguments   [\[-h\]]

  CommandDescription [
  Compiles the flattened hierarchy into SEXP (initial states, invariants,
  and transition relation).<p>]

  SideEffects        []

******************************************************************************/
int CommandBuildFlatModel(int argc, char ** argv)
{
  int c;

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"h")) != EOF){
    switch(c){
    case 'h': return(UsageBuildFlatModel());
    default:  return(UsageBuildFlatModel());
    }
  }
  if (argc != util_optind) return(UsageBuildFlatModel());

  /* pre-conditions: */
  if (Compile_check_if_flattening_was_built(nusmv_stderr)) return 1;

  if (cmp_struct_get_build_flat_model(cmps)) {
    fprintf(nusmv_stderr, "A model appears to be already built from file: %s.\n",
            get_input_file(OptsHandler_get_instance()));
    return 1;
  }

  /* does the work: */
  compile_create_flat_model();

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr,
            "\nThe sexp model has been built from file %s.\n",
      get_input_file(OptsHandler_get_instance()));
  }

  return 0;
}

static int UsageBuildFlatModel()
{
  fprintf(nusmv_stderr, "usage: build_flat_model [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Compiles the flattened hierarchy into boolean SEXP]

  CommandName        [build_boolean_model]

  CommandSynopsis    [Compiles the flattened hierarchy into boolean SEXP]

  CommandArguments   [\[-h\] \[-f\]]

  CommandDescription [
  Compiles the flattened hierarchy into boolean SEXP
  (initial states, invariants, and transition relation).<p>]

  SideEffects        []

******************************************************************************/
int CommandBuildBooleanModel(int argc, char ** argv)
{
  int c;
  boolean forced = false;

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"hf")) != EOF){
    switch(c){
    case 'h': return(UsageBuildBooleanModel());
    case 'f': forced = true; break;
    default:  return(UsageBuildBooleanModel());
    }
  }
  if (argc != util_optind) return(UsageBuildBooleanModel());

  /* pre-conditions: */
  if (Compile_check_if_encoding_was_built(nusmv_stderr)) return 1;

  if (cmp_struct_get_build_bool_model(cmps) && !forced) {
    fprintf(nusmv_stderr, "A model appears to be already built from file: %s.\n",
            get_input_file(OptsHandler_get_instance()));
    return 1;
  }

  /* constructs the model only if coi is not enabled */
  if (opt_cone_of_influence(OptsHandler_get_instance()) && !forced) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr,
        "Construction of boolean model is delayed due to use of COI\n");
    }
    return 0;
  }

  /* creates the flat fsm */
  compile_create_flat_model();

  /* creates the boolean fsm */
  compile_create_boolean_model();

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr,
            "\nThe boolean sexp model has been built from file %s.\n",
      get_input_file(OptsHandler_get_instance()));
  }

  /* We keep track that the master FSM has been built. */
  cmp_struct_set_build_bool_model(cmps);
  return 0;
}

static int UsageBuildBooleanModel()
{
  fprintf(nusmv_stderr, "usage: build_boolean_model [-h][-f]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "   -f \t\tForces the boolean model construction.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Writes variable order to file.]

  CommandName        [write_order]

  CommandSynopsis    [Writes variable order to file.]

  CommandArguments   [\[-h\] \[-b\] \[(-o | -f) order-file\]]

  CommandDescription [Writes the current order of BDD variables in the
  file specified via the -o option. If no option is specified the environment
  variable <tt>output_order_file</tt> will be considered. If the variable
  <tt>output_order_file</tt> is unset (or set to an empty value) then standard
  output will be used. The option <tt>-b</tt> forces the dumped
  variable ordering to contain only boolean variables.
  All the scalar variables will be substituted by those variables bits
  that encode them.  The variables bits will occur within the dumped
  variable ordering depending on the position they have within the
  system when the command is executed.
  <p>
  Command options:<p>
  <dl>
    <dt> <tt>-b</tt>
       <dd> Dumps bits of scalar variables instead of the single
       scalar variables. When specified, this option temporary
       overloads the current value of the system variable
       <tt>write_order_dumps_bits</tt>.

    <dt> <tt>-o order-file</tt>
       <dd> Sets the environment variable <tt>output_order_file</tt>
       to <tt>order-file</tt> and then dumps the ordering list into that file.
    <dt> <tt>-f order-file</tt>
       <dd> Alias for -o option. Supplied for backward compatibility.
  </dl>]

  SideEffects        []

******************************************************************************/
int CommandWriteOrder(int argc, char **argv)
{
  int c;
  char* order_output_fname = NIL(char);
  VarOrderingType dump_type;

  if (opt_write_order_dumps_bits(OptsHandler_get_instance())) dump_type = DUMP_BITS;
  else dump_type = DUMP_DEFAULT;

  /*
   * Parse the command line.
  */
  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "o:f:hb")) != EOF) {
    switch (c) {
    case 'h':
      if (order_output_fname != NIL(char)) FREE(order_output_fname);
      return(UsageWriteOrder());

    case 'b':
      dump_type = DUMP_BITS;
      break;

    case 'o':
    case 'f':
      if (order_output_fname != NIL(char)) {
        /* already called (via the alias): exit */
        FREE(order_output_fname);
        return UsageWriteOrder();
      }
      order_output_fname = ALLOC(char, strlen(util_optarg)+1);
      nusmv_assert(order_output_fname);
      strcpy(order_output_fname, util_optarg);
      break;

    default:
      if (order_output_fname != NIL(char)) FREE(order_output_fname);
      return(UsageWriteOrder());
    }
  }

  /* side effect on variable output_order_file: */
  if (order_output_fname != NIL(char)) {
    set_output_order_file(OptsHandler_get_instance(), order_output_fname);
    FREE(order_output_fname);
  }

  if (dd_manager == NIL(DdManager)) {
    fprintf(nusmv_stderr, "The DD Manager has not been created yet.\n");
    return 1;
  }

  /* pre-conditions: */
  if (Compile_check_if_encoding_was_built(nusmv_stderr)) return 1;

  BddEnc_write_var_ordering(Enc_get_bdd_encoding(),
          get_output_order_file(OptsHandler_get_instance()),
          dump_type);

  /* batch mode: */
  if (opt_batch(OptsHandler_get_instance()) && !opt_reorder(OptsHandler_get_instance()))  { nusmv_exit(0); }

  return 0;
}

static int UsageWriteOrder()
{
  fprintf(nusmv_stderr, "usage: write_order [-h] | [-b] [(-o | -f) <file>]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr,
    "   -b \t\tDumps bits of scalar variables instead of the single \n"\
    "      \t\tscalar variables. \n"\
    "      \t\tSee also the system variable write_order_dumps_bits.\n");
  fprintf(nusmv_stderr, "   -o <file>\tWrites ordering to file <file>.\n");
  fprintf(nusmv_stderr, "   -f <file>\tThe same of option -o. Supplied for backward compatibility.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints out information about the clustering.
  ** DEPRECATED in 2.4 **]

  CommandName        [print_clusterinfo]

  CommandSynopsis [Prints out  information about the
  clustering. This command is *deprecated* in 2.4]

  CommandArguments   [\[-h\] \| \[-m\] \| \[-o output-file\]]

  CommandDescription [Deprecated in 2.4: use print_fsm_stats instead.

  This command prints out information
  regarding each cluster. In particular for each cluster it prints
  out the cluster number, the size of the cluster (in BDD nodes), the
  variables occurring in it, the size of the cube that has to be
  quantified out relative to the cluster and the variables to be
  quantified out.<p>

  Command options:<p>
  <dl>
    <dt> <tt>-m</tt>
       <dd> Pipes the output generated by the command through the
            program specified by the <tt>PAGER</tt> shell variable if
            defined, or through the UNIX utility "more".
    <dt> <tt>-o output-file</tt>
       <dd> Redirects the generated output to the file
            <tt>output-file</tt>.
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandCPPrintClusterInfo(int argc, char ** argv)
{
  int res;

  fprintf(nusmv_stderr, "----------------------------------------------------------------------\n");
  fprintf(nusmv_stderr, "--             Deprecated in 2.4: use 'print_fsm_stats'             --\n");
  fprintf(nusmv_stderr, "----------------------------------------------------------------------\n");

  res = CommandPrintFsmStats(argc, argv);

  fprintf(nusmv_stderr, "----------------------------------------------------------------------\n");
  fprintf(nusmv_stderr, "--             Deprecated in 2.4: use 'print_fsm_stats'             --\n");
  fprintf(nusmv_stderr, "----------------------------------------------------------------------\n");

  return res;
}


/**Function********************************************************************

  Synopsis           [Prints out information about the fsm and clustering.]

  CommandName        [print_fsm_stats]

  CommandSynopsis    [Prints out information about the fsm and clustering.]

  CommandArguments   [\[-h\] \| \[-m\] \| \[-p\] \| \[-o output-file\]]

  CommandDescription [This command prints out information
  regarding the fsm and each cluster. In particular for each cluster
  it prints out the cluster number, the size of the cluster (in BDD
  nodes), the variables occurring in it, the size of the cube that has
  to be quantified out relative to the cluster and the variables to be
  quantified out.<p>

   Also the command can print all the normalized predicates the FMS
   consists of. A normalized predicate is a boolean expression which
   does not have other boolean sub-expressions. For example,
   expression (b<0 ? a/b : 0) = c is normalized into (b<0
   ? a/b=c : 0=c) which has 3 normalized predicates inside:
   b<0, a/b=c, 0=c.

  Command options:<p>
  <dl>
    <dt> <tt>-m</tt>
       <dd> Pipes the output generated by the command through the
            program specified by the <tt>PAGER</tt> shell variable if
            defined, or through the UNIX utility "more".
    <dt> <tt>-p</tt>
       <dd> Prints out the normalized predicates the FSM consists of.
    <dt> <tt>-o output-file</tt>
       <dd> Redirects the generated output to the file
            <tt>output-file</tt>.
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandPrintFsmStats(int argc, char ** argv)
{
  int c;
  int useMore = 0;
  int retval = 0;
  boolean printPreds = false;
  char* dbgFileName = (char*) NULL;
  FILE* old_nusmv_stdout = NIL(FILE);

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hmpo:")) != EOF) {
    switch (c) {
    case 'h':
      retval = UsagePrintFsmStats();
      goto CommandPrintFsmStats_exit;

    case 'o':
      if (useMore == 1) {
        retval = UsagePrintFsmStats();
        goto CommandPrintFsmStats_exit;
      }
      if ((char*) NULL != dbgFileName) FREE(dbgFileName);
      dbgFileName = util_strsav(util_optarg);
      nusmv_assert((char*) NULL != dbgFileName);
      break;

    case 'm':
      if (dbgFileName != NIL(char)) {
        retval = UsagePrintFsmStats();
        goto CommandPrintFsmStats_exit;
      }
      useMore = 1;
      break;

    case 'p':
      printPreds = true;
      break;

    default:
      retval = UsagePrintFsmStats();
      goto CommandPrintFsmStats_exit;
    }
  } /* while */

  if (printPreds && !cmp_struct_get_flatten_hrc(cmps)) {
    fprintf(nusmv_stderr, "\nError: option -p of print_fsm_stats requires "
            "the model be flattened. Use command \"flatten_hierarchy\".\n");
    retval = 1;
    goto CommandPrintFsmStats_exit;
  }

  if (useMore) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenPipe(useMore);
    if (nusmv_stdout==(FILE*) NULL) {
      retval = 1;
      goto CommandPrintFsmStats_exit;
    }
  }

  if (dbgFileName != NIL(char)) {
    old_nusmv_stdout = nusmv_stdout;
    nusmv_stdout = CmdOpenFile(dbgFileName);
    if (nusmv_stdout==(FILE*) NULL) {
      retval = 1;
      goto CommandPrintFsmStats_exit;
    }
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "Output to file: %s\n", dbgFileName);
    }
  }

  /* print BDD stats */
  {
    BddFsm_ptr fsm = PropDb_master_get_bdd_fsm(PropPkg_get_prop_database());

    if (fsm != BDD_FSM(NULL)) {
      BddFsm_print_info(fsm, nusmv_stdout);
    }
  }

  /* print predicates */
  if (printPreds) {
    nusmv_assert(mainFlatHierarchy != NULL); /* the model has been
                                                flattened and kept in this var */


    /* ,i.e. normalized expressions are not used later on. it would be
       better to keep the results somewhere and reuse it in other modules of
       nusmv, where normalization is required. */

    /* NOTE: here the flatten hierarchy is normalized to collect
       predicates.  Another solution is to normalize the Sexp FSM.
       There is no strong opinion toward any solution. Normalization
       of flatten hierarchy was chose just because it does not require
       initialization of encoding or FSM creation which may be useful
       in some cases. */

    SymbTable_ptr st = Compile_get_global_symb_table();
    Set_t predicates = Set_MakeEmpty();
    PredicateNormaliser_ptr normalizer = Compile_get_global_predicate_normaliser();
    node_ptr tmp;
    array_t* layers_name;
    int i;
    const char* a_layer_name;

    nusmv_assert(mainFlatHierarchy != NULL);

    /* normalize subparts of the hierarchy and collect the predicates.
       Note the normalized expressions are not freed because they are created with find_node.
    */
    tmp = FlatHierarchy_get_init(mainFlatHierarchy);
    tmp = PredicateNormaliser_normalise_expr(normalizer, tmp);
    PredicateNormaliser_get_predicates_only(normalizer, &predicates, tmp);

    tmp = FlatHierarchy_get_invar(mainFlatHierarchy);
    tmp = PredicateNormaliser_normalise_expr(normalizer, tmp);
    PredicateNormaliser_get_predicates_only(normalizer, &predicates, tmp);

    tmp = FlatHierarchy_get_trans(mainFlatHierarchy);
    tmp = PredicateNormaliser_normalise_expr(normalizer, tmp);
    PredicateNormaliser_get_predicates_only(normalizer, &predicates, tmp);

    tmp = FlatHierarchy_get_input(mainFlatHierarchy);
    tmp = PredicateNormaliser_normalise_expr(normalizer, tmp);
    PredicateNormaliser_get_predicates_only(normalizer, &predicates, tmp);

    tmp = FlatHierarchy_get_justice(mainFlatHierarchy);
    tmp = PredicateNormaliser_normalise_expr(normalizer, tmp);
    PredicateNormaliser_get_predicates_only(normalizer, &predicates, tmp);

    tmp = FlatHierarchy_get_compassion(mainFlatHierarchy);
    tmp = PredicateNormaliser_normalise_expr(normalizer, tmp);
    PredicateNormaliser_get_predicates_only(normalizer, &predicates, tmp);

    /* assignments require very special handling because
       FlatHierarchy_get_assign returns the assignments without
       CASE-expressions and "running" variables created when there are processes.
       To obtain the actual assignments it is necessary to collects assignments
       using FlatHierarchy_lookup_assign.

       NOTE: This code is terrible because API in FlatHierarchy does
       not provided there required function (to access actual assignments).
    */
    layers_name = SymbTable_get_class_layer_names(st, (const char*) NULL);

    arrayForEachItem(const char*, layers_name, i, a_layer_name) {
      SymbLayer_ptr layer = SymbTable_get_layer(st, a_layer_name);
      SymbLayerIter iter;

      SYMB_LAYER_FOREACH(layer, iter, STT_VAR) {
        node_ptr name = SymbLayer_iter_get_symbol(layer, &iter);
        node_ptr init_name = find_node(SMALLINIT, name, Nil);
        node_ptr next_name = find_node(NEXT, name, Nil);
        node_ptr invar_expr = FlatHierarchy_lookup_assign(mainFlatHierarchy, name);
        node_ptr init_expr = FlatHierarchy_lookup_assign(mainFlatHierarchy, init_name);
        node_ptr next_expr = FlatHierarchy_lookup_assign(mainFlatHierarchy, next_name);

        if (invar_expr != Nil) {
          tmp = PredicateNormaliser_normalise_expr(normalizer,
                                                   find_node(EQDEF, name, invar_expr));
          PredicateNormaliser_get_predicates_only(normalizer, &predicates, tmp);
        }
        if (init_expr != Nil) {
          tmp = PredicateNormaliser_normalise_expr(normalizer,
                                                   find_node(EQDEF, init_name, init_expr));
          PredicateNormaliser_get_predicates_only(normalizer, &predicates, tmp);
        }
        if (next_expr != Nil) {
          tmp = PredicateNormaliser_normalise_expr(normalizer,
                                                   find_node(EQDEF, next_name, next_expr));
          PredicateNormaliser_get_predicates_only(normalizer, &predicates, tmp);
        }
      }
    }

    fprintf(nusmv_stdout, "\nFSM consists of the following Predicates:\n");
    Set_Iterator_t iter;
    SET_FOREACH(predicates, iter) {
      node_ptr pred = Set_GetMember(predicates, iter);
      print_node(nusmv_stdout, pred);
      fprintf(nusmv_stdout, "\n\n");
    }

    Set_ReleaseSet(predicates);
  }


CommandPrintFsmStats_exit:
  if (useMore && (FILE*) NULL != old_nusmv_stdout) {
    if (nusmv_stdout != (FILE*) NULL) CmdClosePipe(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
  }
  if (dbgFileName != NIL(char) && (FILE*) NULL != old_nusmv_stdout) {
    CmdCloseFile(nusmv_stdout);
    nusmv_stdout = old_nusmv_stdout;
    FREE(dbgFileName);
  }

  return retval;
}

static int UsagePrintFsmStats()
{
  fprintf(nusmv_stderr, "usage: print_fsm_stats [-h] [-m] [-o file] [-p]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n");
  fprintf(nusmv_stderr, "   -m \t\tPipes output through the program specified by\n");
  fprintf(nusmv_stderr, "      \t\tthe \"PAGER\" shell variable if defined,\n");
  fprintf(nusmv_stderr, "      \t\t else through the UNIX command \"more\"\n");
  fprintf(nusmv_stderr, "   -p \t\tPrints out the normalized predicates the FSM\n");
  fprintf(nusmv_stderr, "      \t\tconsists of. A normalized predicate is a boolean\n");
  fprintf(nusmv_stderr, "      \t\texpression without boolean sub-expressions.\n");
  fprintf(nusmv_stderr, "   -o file\tWrites the output to \"file\".\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints the Iwls95 Options.]

  CommandName        [print_iwls95options]

  CommandSynopsis    [Prints the Iwls95 Options.]

  CommandArguments   [\[-h\]]

  CommandDescription [This command prints out the configuration
  parameters of the IWLS95 clustering algorithm, i.e.
  <tt>image_verbosity</tt>, <tt>image_cluster_size</tt> and
  <tt>image_W{1,2,3,4}</tt>.]

  SideEffects        []

******************************************************************************/
int CommandIwls95PrintOption(int argc, char ** argv)
{
  int c;
  ClusterOptions_ptr opts;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch (c) {
    case 'h':
      return(UsageIwls95PrintOption());
    default:
      return(UsageIwls95PrintOption());
    }
  } /* while */

  opts = ClusterOptions_create(OptsHandler_get_instance());
  ClusterOptions_print(opts, nusmv_stdout);
  ClusterOptions_destroy(opts);

  return 0;
}

static int UsageIwls95PrintOption()
{
  fprintf(nusmv_stderr, "usage: print_iwls95options [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Implements the go command]

  CommandName        [go]

  CommandSynopsis    [Initializes the system for the verification.]

  CommandArguments   [\[-h\]\[-f\]]

  CommandDescription [This command initializes the system for
  verification. It is equivalent to the command sequence
  <tt>read_model</tt>, <tt>flatten_hierarchy</tt>,
  <tt>build_flat_model</tt>, <tt>encode_variables</tt>,
  <tt>build_model</tt>.<p>
  If some commands have already been
  executed, then only the remaining ones will be invoked.<p>
  Command options:<p>
  <dl><dt> -f
  <dd> Forces the model contruction.<p>
  <dt> -h
  <dd> Prints the command usage.<p>
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandGo(int argc, char ** argv)
{
  int c;
  boolean forced = false;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hf")) != EOF) {
    switch (c) {
    case 'h': return UsageGo();
    case 'f': forced = true; break;
    default: return UsageGo();
    }
  } /* while */

  if (cmp_struct_get_read_model(cmps) == 0)
    if (Cmd_CommandExecute("read_model")) return 1;
  if (cmp_struct_get_flatten_hrc(cmps) == 0)
    if (Cmd_CommandExecute("flatten_hierarchy")) return 1;
  if (cmp_struct_get_build_flat_model(cmps) == 0)
    if(Cmd_CommandExecute("build_flat_model")) return 1;
  if (cmp_struct_get_encode_variables(cmps) == 0)
    if (Cmd_CommandExecute("encode_variables")) return 1;
  if (cmp_struct_get_build_model(cmps) == 0) {
    if (!forced) { if (Cmd_CommandExecute("build_model")) return 1; }
    else if (Cmd_CommandExecute("build_model -f")) return 1;
  }
  return 0;
}

static int UsageGo()
{
  fprintf(nusmv_stderr, "usage: go [-h] | [-f]\n");
  fprintf(nusmv_stderr, "   -f \t\tForces the model construction\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Implements the go_bmc command]

  CommandName        [go_bmc]

  CommandSynopsis    [Initializes the system for the BMC verification.]

  CommandArguments   [\[-h\] | \[-f\]]

  CommandDescription [This command initializes the system for
  verification. It is equivalent to the command sequence
  <tt>read_model</tt>, <tt>flatten_hierarchy</tt>,
  <tt>encode_variables</tt>, <tt>build_boolean_model</tt>, <tt>bmc_setup</tt>.
  If some commands have already been
  executed, then only the remaining ones will be invoked.<p>
  Command options:<p>
  <dl>
  <dt> -f
  <dd> Forces the model construction.<p>
  <dt> -h
  <dd> Prints the command usage.<p>
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandGoBmc(int argc, char ** argv)
{
  int c;
  boolean forced = false;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hf")) != EOF) {
    switch (c) {
    case 'h': return(UsageGoBmc());
    case 'f': forced = true; break;
    default: return(UsageGoBmc());
    }
  } /* while */

  if (cmp_struct_get_read_model(cmps) == 0)
    if (Cmd_CommandExecute("read_model")) return 1;
  if (cmp_struct_get_flatten_hrc(cmps) == 0)
    if (Cmd_CommandExecute("flatten_hierarchy")) return 1;
  if (cmp_struct_get_encode_variables(cmps) == 0)
    if (Cmd_CommandExecute("encode_variables")) return 1;
  if (cmp_struct_get_build_bool_model(cmps) == 0) {
    if (!forced) { if (Cmd_CommandExecute("build_boolean_model")) return 1; }
    else if (Cmd_CommandExecute("build_boolean_model -f")) return 1;
  }
  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    if (!forced) { if (Cmd_CommandExecute("bmc_setup")) return 1; }
    else if (Cmd_CommandExecute("bmc_setup -f")) return 1;
  }

  return 0;
}

static int UsageGoBmc()
{
  fprintf(nusmv_stderr, "usage: go_bmc [-h] | [-f]\n");
  fprintf(nusmv_stderr, "   -f \t\tForces the model contruction\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage\n");
  return 1;
}

/**Function********************************************************************

  Synopsis           [Implements the get_internal_status command]

  CommandName        [get_internal_status]

  CommandSynopsis    [Returns the internal status of the system.]

  CommandArguments   [\[-h\]]

  CommandDescription [Prints out the internal status of the system. i.e.
  <ul>
  <li> -1 : read_model has not yet been executed or an error occurred
            during its execution. </li>
  <li>  0 : flatten_hierarchy has not yet been executed or an error
            occurred during its execution. </li>
  <li>  1 : encode_variables has not yet been executed or an error
            occurred during its execution. </li>
  <li>  2 : build_model has not yet been executed or an error occurred
            during its execution. </li>
  </ul>
  Command options:<p>
  <dl><dt> -h
  <dd> Prints the command usage.<p>
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandGetInternalStatus(int argc, char ** argv)
{
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch (c) {
    case 'h':
      return(UsageGetInternalStatus());
    default:
      return(UsageGetInternalStatus());
    }
  } /* while */

  if (cmp_struct_get_read_model(cmps) == 0){
    fprintf(nusmv_stderr, "The internal status is: -1\n");
    return 0;
  }
  if (cmp_struct_get_flatten_hrc(cmps) == 0){
    fprintf(nusmv_stderr, "The internal status is: 0\n");
    return 0;
  }
  if (cmp_struct_get_encode_variables(cmps) == 0){
    fprintf(nusmv_stderr, "The internal status is: 1\n");
    return 0;
  }
  if (cmp_struct_get_build_model(cmps) == 0){
    fprintf(nusmv_stderr, "The internal status is: 2\n");
    return 0;
  }
  return 0;
}

static int UsageGetInternalStatus()
{
  fprintf(nusmv_stderr, "usage: get_internal_status [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Writes the currently loaded SMV model in the
  specified file, after having flattened it]

  CommandName        [write_flat_model]

  CommandSynopsis    [Writes a flat model of a given SMV file]

  CommandArguments   [\[-h\] \[-o filename\] \[-A\] \[-m\]]

  CommandDescription [Processes are eliminated
  and a corresponding equivalent model is printed out.
  If no file is specified, the file specified with the environment variable
  <tt>output_flatten_model_file</tt> is used if any, otherwise standard output
  is used as output.
  <p>
  Command options:<p>
  <dl>
     <dt><tt>-o filename</tt>
       <dd> Attempts to write the flat SMV model in <tt>filename</tt>.
     <dt><tt>-A</tt>
       <dd> Write the model using variables and defines rewriting to
       make it anonimized.
     <dt><tt>-m</tt>
       <dd> Disable printing of key map when writing anonimized model
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandWriteModelFlat(int argc, char **argv)
{
  int c = 0;
  int rv = 0;
  char* output_file = NIL(char);
  FILE* ofileid = NIL(FILE);
  int bSpecifiedFilename = FALSE;

  boolean obfuscated = false;
  boolean print_map = true;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "hAo:m")) != EOF) {
    switch (c) {
    case 'h':
      if (bSpecifiedFilename == TRUE) FREE(output_file);
      return UsageWriteModelFlat();

    case 'o':
      output_file = ALLOC(char, strlen(util_optarg)+1);
      nusmv_assert(output_file);
      strcpy(output_file, util_optarg);
      bSpecifiedFilename = TRUE;
      break;

    case 'A':
      obfuscated = true;
      break;

    case 'm':
      print_map = false;
      break;

    default:
      break;
    }
  }

  if (argc != util_optind) {
    if (bSpecifiedFilename == TRUE) FREE(output_file);
    return UsageWriteModelFlat();
  }

  if (output_file == NIL(char)) {
    output_file = get_output_flatten_model_file(OptsHandler_get_instance());
  }
  if (output_file == NIL(char)) {
    ofileid = nusmv_stdout;
  }
  else {
    ofileid = fopen(output_file, "w");
    if (ofileid == NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", output_file);
      if (bSpecifiedFilename == TRUE)  FREE(output_file);
      return 1;
    }
  }

  /* pre-conditions: */
  if (Compile_check_if_flattening_was_built(nusmv_stderr)) return 1;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Writing flat model into file \"%s\"..",
      output_file == (char *)NULL ? "stdout" : output_file);
  }

  CATCH {
    SymbTable_ptr st = Compile_get_global_symb_table();

    if (!obfuscated) {
      Compile_WriteFlattenModel(ofileid, st,
                                SymbTable_get_class_layer_names(st,
                                                            (const char*) NULL),
                                "MODULE main", mainFlatHierarchy, true);
    }
    else {
      Compile_WriteObfuscatedFlattenModel(ofileid, st,
                                          SymbTable_get_class_layer_names(st,
                                                            (const char*) NULL),
                                          "MODULE main", mainFlatHierarchy,
                                          print_map, true);
    }

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, ".. done.\n");
    }
  }
  FAIL {
    rv = 1;
  }
  fflush(ofileid);

  if (ofileid != nusmv_stdout) {
    fclose(ofileid);
    if (bSpecifiedFilename) FREE(output_file);
  }
  return rv;
}

static int UsageWriteModelFlat(void)
{
  fprintf(nusmv_stderr, "usage: write_flat_model [-h] [-A] [-m] [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -o filename\tWrites output to \"filename\"\n");
  fprintf(nusmv_stderr, "  -A Write the model using variables and defines rewriting to make it anonimized.\n");
  fprintf(nusmv_stderr, "  -m Disable printing of key map when writing anonimized model\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Writes the currently loaded SMV model in the
                      specified uDraw file, after having flattened it]

  CommandName        [write_flat_model_udg]

  CommandSynopsis    [Writes a flat model of a given SMV file in uDraw format]

  CommandArguments   [\[-h\] \[-o filename\]]

  CommandDescription [Processes are eliminated
  and a corresponding equivalent model is printed out.
  If no file is specified, the file specified with the environment variable
  <tt>output_flatten_model_file</tt> is used if any, otherwise standard output
  is used as output.
  <p>
  Command options:<p>
  <dl>
     <dt><tt>-o filename</tt>
       <dd> Attempts to write the flat SMV model in <tt>filename</tt>.
  </dl>
  ]

  SideEffects        []

******************************************************************************/
int CommandWriteModelFlatUdg(int argc, char **argv)
{
  int c = 0;
  int rv = 0;
  char* output_file = NIL(char);
  FILE* ofileid = NIL(FILE);
  int bSpecifiedFilename = FALSE;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "ho:")) != EOF) {
    switch (c) {
    case 'h': return UsageWriteModelFlatUdg();
    case 'o':
      output_file = ALLOC(char, strlen(util_optarg)+1);
      nusmv_assert(output_file);
      strcpy(output_file, util_optarg);
      bSpecifiedFilename = TRUE;
      break;

    default:
      break;
    }
  }

  if (argc != util_optind) return UsageWriteModelFlatUdg();

  if (output_file == NIL(char)) {
    output_file = get_output_flatten_model_file(OptsHandler_get_instance());
  }
  if (output_file == NIL(char)) {
    ofileid = nusmv_stdout;
  }
  else {
    ofileid = fopen(output_file, "w");
    if (ofileid == NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", output_file);
      if (bSpecifiedFilename == TRUE)  FREE(output_file);
      return 1;
    }
  }

  /* pre-conditions: */
  if (Compile_check_if_flattening_was_built(nusmv_stderr)) return 1;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Writing flat model udg into file \"%s\"..",
      output_file == (char *)NULL ? "stdout" : output_file);
  }

  CATCH {

    SymbTable_ptr st = Compile_get_global_symb_table();

    Compile_WriteFlattenModel_udg(ofileid, st,
                                  SymbTable_get_class_layer_names(st, (const char*) NULL),
                                  "MODULE main", mainFlatHierarchy);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, ".. done.\n");
    }
  }
  FAIL {
    rv = 1;
  }
  fflush(ofileid);

  if (ofileid != nusmv_stdout) {
    fclose(ofileid);
    if (bSpecifiedFilename) FREE(output_file);
  }
  return rv;
}

static int UsageWriteModelFlatUdg(void)
{
  fprintf(nusmv_stderr, "usage: write_flat_model_udg [-h] [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -o filename\tWrites output to \"filename\"\n");
  return 1;
}



/**Function********************************************************************

  Synopsis [Writes a flat and boolean model of a given SMV file]

  CommandName        [write_boolean_model]

  CommandSynopsis    [Writes a flattened and booleanized model of a
  given SMV file]

  CommandArguments   [\[-h\] \[-o filename\]]

  CommandDescription [Writes the currently loaded SMV model in the
  specified file, after having flattened and booleanized it. Processes
  are eliminated and a corresponding equivalent model is printed
  out.

  If no file is specified, the file specified via the environment
  variable <tt>output_boolean_model_file</tt> is used if any,
  otherwise standard output is used.
  <p>
  Command options:<p>
  <dl>
     <dt><tt>-o filename</tt>
       <dd> Attempts to write the flat and boolean SMV model in
       <tt>filename</tt>.
  </dl>

  ** New in 2.4.0 and later **
  Scalar variables are dumped as DEFINEs whose body is their boolean
  encoding.

  This allows the user to still express and see parts of the generated
  boolean model in terms of the original model's scalar variables
  names and values, and still keeping the generated model purely boolean.

  Also, symbolic constants are dumped within a CONSTANTS statement to
  declare the values of the original scalar variables' for future
  reading of the generated file.]

  SideEffects        []

******************************************************************************/
int CommandWriteModelFlatBool(int argc, char **argv)
{
  int c = 0;
  int rv = 0;
  char* output_file = NIL(char);
  FILE* ofileid = NIL(FILE);
  int bSpecifiedFilename = FALSE;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "ho:")) != EOF) {
    switch (c) {
    case 'h':
      if (bSpecifiedFilename == TRUE) FREE(output_file);
      return UsageWriteModelFlatBool();

    case 'o':
      output_file = ALLOC(char, strlen(util_optarg)+1);
      nusmv_assert(output_file);
      strcpy(output_file, util_optarg);
      bSpecifiedFilename = TRUE;
      break;

    default:
      break;
    }
  }

  if (argc != util_optind) {
    if (bSpecifiedFilename == TRUE) FREE(output_file);
    return UsageWriteModelFlatBool();
  }

  if (output_file == NIL(char)) {
    output_file = get_output_boolean_model_file(OptsHandler_get_instance());
  }

  if (output_file == NIL(char)) {
    ofileid = nusmv_stdout;
  }
  else {
    ofileid = fopen(output_file, "w");
    if (ofileid == NULL) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", output_file);
      if (bSpecifiedFilename == TRUE)  FREE(output_file);
      return 1;
    }
  }

  if (Compile_check_if_bool_model_was_built(nusmv_stderr, true)) {
    if (ofileid != nusmv_stdout) {
      fclose(ofileid);
      if (bSpecifiedFilename == TRUE) FREE(output_file);
    }
    return 1;
  }


  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Writing boolean model into file \"%s\"..",
            output_file == (char *)NULL ? "stdout" : output_file);
  }

  CATCH {
    BddEnc_ptr enc = Enc_get_bdd_encoding();
    SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));
    NodeList_ptr layers = NodeList_create();
    int i;
    const char* name;

    /* fills in layers list with those list that are in default class and have
       been committed */
    arrayForEachItem(const char*, SymbTable_get_class_layer_names(st, NULL),
                     i, name) {
      SymbLayer_ptr layer = SymbTable_get_layer(st, name);
      if (NodeList_belongs_to(BaseEnc_get_committed_layers(BASE_ENC(enc)),
                              (node_ptr) layer)) {
        NodeList_append(layers, (node_ptr) layer);
      }
      { /* gets and add the corresponding boolean layer if exists */
        const char* bname = BoolEnc_scalar_layer_to_bool_layer(name);
        layer = SymbTable_get_layer(st, bname);
        if (layer != SYMB_LAYER(NULL) &&
            !NodeList_belongs_to(layers, (node_ptr) layer) &&
            NodeList_belongs_to(BaseEnc_get_committed_layers(BASE_ENC(enc)),
                                (node_ptr) layer)) {
          NodeList_append(layers, (node_ptr) layer);
        }
      }
    }

    Compile_WriteBoolModel(ofileid, enc, layers, "MODULE main",
                           PropDb_master_get_bool_sexp_fsm(PropPkg_get_prop_database()), true);

    NodeList_destroy(layers);

    if ((BddEnc_get_reordering_count(enc) > 0) &&
        (get_output_order_file(OptsHandler_get_instance()) != (char*) NULL) &&
        (ofileid != nusmv_stdout)) {
      /* there was a reordering, so a variable ordering file is dumped */
      BddEnc_write_var_ordering(enc, get_output_order_file(OptsHandler_get_instance()),
                                opt_write_order_dumps_bits(OptsHandler_get_instance()) ?
                                DUMP_BITS : DUMP_DEFAULT);

      fprintf(nusmv_stderr,
              "%d reordering(s) occurred. Dumped variable ordering to '%s'\n",
              BddEnc_get_reordering_count(enc),
              get_output_order_file(OptsHandler_get_instance()));
      BddEnc_reset_reordering_count(enc);
    }

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, ".. done.\n");
    }
  } FAIL {
    rv = 1;
  }
  fflush(ofileid);

  if (ofileid != nusmv_stdout) {
    fclose(ofileid);
    if (bSpecifiedFilename == TRUE)  FREE(output_file);
  }
  return rv;
}

static int UsageWriteModelFlatBool(void)
{
  fprintf(nusmv_stderr, "usage: write_boolean_model [-h] [-o filename]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -o filename\tWrites output to \"filename\".\n");
  return 1;
}


/**Function********************************************************************

  Synopsis [Writes a flat model of a given SMV file, restricted to the
            COI of the model properties]

  CommandName        [write_coi_model]

  CommandSynopsis    [Writes a flat model of SMV file, restricted to the COI
  of the model properties]

  CommandArguments   [\[-h\] \[-o filename\] \[-n <prop> | -p <expr>
                      | -P <name>\] | \[-c\] | \[-l\] | \[-i\] | \[-s\] |
                      \[-q\] | \[-p expr\] | \[-C\] | \[-g\]]

  CommandDescription [Writes the currently loaded SMV model in the
  specified file, after having flattened it. If a property is
  specified, the dumped model is the result of applying the COI over
  that property. otherwise, a restricted SMV model is dumped for each
  property in the property database.

  Processes are eliminated and a corresponding equivalent model is
  printed out.

  If no file is specified, stderr is used for output
  <p>
  Command options:<p>
  <dl>
     <dt><tt>-o filename</tt>
       <dd> Attempts to write the flat and boolean SMV model in
       <tt>filename</tt>.
    <dt> <tt>-c</tt>
       <dd> Dumps COI model for all CTL properties
    <dt> <tt>-l</tt>
       <dd> Dumps COI model for all LTL properties
    <dt> <tt>-i</tt>
       <dd> Dumps COI model for all INVAR properties
    <dt> <tt>-s</tt>
       <dd> Dumps COI model for all PSL properties
    <dt> <tt>-q</tt>
       <dd> Dumps COI model for all COMPUTE properties
     <dt><tt>-p expr</tt>
       <dd> Applies COI for the given expression "expr"
     <dt><tt>-n idx</tt>
       <dd> Applies COI for property stored at index "idx"
     <dt><tt>-P name</tt>
       <dd> Applies COI for property named "name" idx
     <dt><tt>-C</tt>
       <dd> Only prints the list of variables that are in
           the COI of properties
     <dt><tt>-g</tt>
       <dd> Dumps the COI model that represents the union of all COI properties
  </dl>

  ]
  SideEffects        []

******************************************************************************/
int CommandWriteCoiModel(int argc, char **argv)
{
  int res = 1;
  int c;
  char* formula = NIL(char);
  char* formula_name = NIL(char);
  char* file_name = NIL(char);
  FILE* output_file = nusmv_stderr;
  boolean print_coi = false;
  boolean global_coi_model = false;
  int prop_no = -1;

  Prop_Type prop_type = Prop_NoType;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "ho:p:P:n:qclisCg")) != EOF) {
    switch (c) {
    case 'h':
      goto write_coi_usage;

    case 'g':
      if (global_coi_model || print_coi) goto write_coi_usage;
      global_coi_model = true;
      break;

    case 'C':
      if (global_coi_model || print_coi) goto write_coi_usage;
      print_coi = true;
      break;

    case 'q':
      if ((NIL(char) != formula_name) || (prop_no != -1) ||
          (prop_type != Prop_NoType)) {
        goto write_coi_usage;
      }
      prop_type = Prop_Compute;
      break;
    case 'c':
      if ((NIL(char) != formula_name) || (prop_no != -1) ||
          (prop_type != Prop_NoType)) {
        goto write_coi_usage;
      }
      prop_type = Prop_Ctl;
      break;
    case 'l':
      if ((NIL(char) != formula_name) || (prop_no != -1) ||
          (prop_type != Prop_NoType)) {
        goto write_coi_usage;
      }
      prop_type = Prop_Ltl;
      break;
    case 'i':
      if ((NIL(char) != formula_name) || (prop_no != -1) ||
          (prop_type != Prop_NoType)) {
        goto write_coi_usage;
      }
      prop_type = Prop_Invar;
      break;
    case 's':
      if ((NIL(char) != formula_name) || (prop_no != -1) ||
          (prop_type != Prop_NoType)) {
        goto write_coi_usage;
      }
      prop_type = Prop_Psl;
      break;

    case 'n':
      if ((NIL(char) != formula) ||
          (NIL(char) != formula_name) || (prop_no != -1) ||
          (prop_type != Prop_NoType)) {
        goto write_coi_usage;
      }

      prop_no = PropDb_get_prop_index_from_string(PropPkg_get_prop_database(),
                                                  util_optarg);
      if (prop_no == -1) { goto write_coi_free; }
      break;
    case 'P':
      if ((NIL(char) != formula) ||
          (NIL(char) != formula_name) || (prop_no != -1) ||
          (prop_type != Prop_NoType)) {
        goto write_coi_usage;
      }
      formula_name = util_strsav(util_optarg);

      prop_no = PropDb_prop_parse_name(PropPkg_get_prop_database(),
                                       formula_name);

      if (prop_no == -1) {
        fprintf(nusmv_stderr, "No property named \"%s\"\n", formula_name);
        goto write_coi_free;
      }
      break;
    case 'p':
      if ((NIL(char) != formula) ||
          (NIL(char) != formula_name) || (prop_no != -1)) {
        goto write_coi_usage;
      }
      formula = util_strsav(util_optarg);
      break;
    case 'o':
      if (NIL(char) != file_name) { goto write_coi_usage; }
      file_name = util_strsav(util_optarg);
      break;
    default:
      goto write_coi_usage;
    }
  }

  if (argc != util_optind) { goto write_coi_usage; }

  /* pre-conditions: */
  if (Compile_check_if_flattening_was_built(nusmv_stderr)) {
    goto write_coi_free;
  }

  if (NIL(char) != formula) {
    if (Prop_NoType == prop_type) {
      fprintf(nusmv_stderr, "No property type specified. Use one of the "
              "-l, -i, -s, -c or -q options\n");
      goto write_coi_usage;
    }
    prop_no = PropDb_prop_parse_and_add(PropPkg_get_prop_database(),
                                        Compile_get_global_symb_table(),
                                        formula, prop_type);
    if (prop_no == -1) { goto write_coi_free; }
  }

  {
    /* Write COI model only for the requested property */
    if (prop_no != -1) {
      Prop_ptr prop = PropDb_get_prop_at_index(PropPkg_get_prop_database(),
                                               prop_no);
      Set_t cone = Prop_compute_cone(prop, mainFlatHierarchy,
                                     Compile_get_global_symb_table());
      Set_t props = Set_MakeSingleton((Set_Element_t)prop);

      if (NIL(char) != file_name) {
        output_file = fopen(file_name, "w");
        if ((FILE*) NULL == output_file) {
          fprintf(nusmv_stderr, "Cannot open file '%s' for writing\n", file_name);
          goto write_coi_free;
        }
      }

      if (print_coi) {
        compile_cmd_write_coi_prop(cone, props, output_file);
      }
      else {
        compile_cmd_write_coi_prop_fsm(mainFlatHierarchy, cone,
                                       props, output_file);
      }
      fflush(output_file);
      if (NIL(char) != file_name) {
        fclose(output_file);
      }

      Set_ReleaseSet(props);
      Set_ReleaseSet(cone);
    }
    /* Write COI model for all property in the DB */
    else {
      /* User requested the global COI model: this is the result of
         all properties COI union */
      if (global_coi_model) {
        if (NIL(char) != file_name) {
          output_file = fopen(file_name, "w");
          if ((FILE*) NULL == output_file) {
            fprintf(nusmv_stderr, "Cannot open file '%s' for writing\n", file_name);
            goto write_coi_free;
          }
        }

        compile_cmd_write_global_coi_fsm(mainFlatHierarchy, prop_type, output_file);

        fflush(output_file);
        if (NIL(char) != file_name) {
          fclose(output_file);
        }
      }
      else {
        /* Dump shared COI informations */
        res = compile_cmd_write_properties_coi(mainFlatHierarchy, prop_type,
                                               print_coi, file_name);
        if (res != 0) goto write_coi_free;
      }
    }
    /* Everything went OK */
    res = 0;
    goto write_coi_free;
  }
 write_coi_usage:
  res = UsageWriteCoiModel();
 write_coi_free:
  if (NIL(char) != formula) { FREE(formula); }
  if (NIL(char) != formula_name) { FREE(formula_name); }
  if (NIL(char) != file_name) { FREE(file_name); }
  return res;
}

/**Function********************************************************************

  Synopsis           [Prints the usage for the write_coi_command]

  Description        []

  SideEffects        []

******************************************************************************/
static int UsageWriteCoiModel()
{
  fprintf(nusmv_stderr, "usage: write_coi_model [-h] [-o filename]"
          " [-n \"index\" | -p \"expr\"| -P \"name\"] [-c | -l | -i | -s | -q] [-C] [-g]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -n idx\tWrites COI model for property with index \"idx\"\n");
  fprintf(nusmv_stderr, "  -p expr\tWrites COI model for the given property. The property type has to be specified\n");
  fprintf(nusmv_stderr, "  -P name\tWrites COI model for property named \"name\"\n");
  fprintf(nusmv_stderr, "  -C \t\tOnly print the list of variables in the COI\n");
  fprintf(nusmv_stderr, "  -g \t\tGroups properties COI and generates one model\n");
  fprintf(nusmv_stderr, "  -i \t\tWrite COI model only for INVAR properties\n");
  fprintf(nusmv_stderr, "  -l \t\tWrite COI model only for LTL properties\n");
  fprintf(nusmv_stderr, "  -s \t\tWrite COI model only for PSL properties\n");
  fprintf(nusmv_stderr, "  -q \t\tWrite COI model only for COMPUTE properties\n");
  fprintf(nusmv_stderr, "  -c \t\tWrite COI model only for CTL properties\n");
  fprintf(nusmv_stderr, "  -o filename\tWrites output to \"filename\".\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Show expression dependencies]

  CommandName        [show_dependencies]

  CommandSynopsis    [Shows the expression dependencies]

  CommandArguments   [\[-h\] \[-k bound\] -e expr]

  CommandDescription [
  Shows the dependencies of the given expression
  <p>
  Command options:<p>
  <dl>
     <dt><tt>-k bound</tt>
       <dd> Stop dependencies computation at step "bound"
     <dt><tt>-e expr</tt>
       <dd> The expression on which the dependencies are computed on
  </dl>
  ]
  SideEffects        []

******************************************************************************/
int CommandShowDependencies (int argc, char **argv)
{
  int c;
  int res = 0;
  int fixpoint = -1;
  char* next_formula_str = NIL(char);
  util_getopt_reset();

  while ((c = util_getopt(argc, argv, "hk:e:")) != EOF) {
    switch (c) {
    case 'h':
      goto show_deps_usage;
    case 'k':
      {
        int res;
        if (fixpoint != -1) goto show_deps_usage;

        res = util_str2int(util_optarg, &fixpoint);

        if (res != 0 || (fixpoint < 0)) {
          fprintf(nusmv_stderr,
                  "Error: '%s' is not a valid fixpoint\n",  util_optarg);
          goto show_deps_usage;
        }
        break;
      }
    case 'e':
      {
        if (NIL(char) != next_formula_str) goto show_deps_usage;
        next_formula_str = util_strsav(util_optarg);
        break;
      }
    default:
      goto show_deps_usage;
    }
  }
  /* More arguments than necessary */
  if (argc != util_optind) goto show_deps_usage;

  /* No expression specified */
  if (NIL(char) == next_formula_str) goto show_deps_usage;

  {
    node_ptr expression;
    Set_t dependencies;
    SymbTable_ptr symb_table = Compile_get_global_symb_table();
    TypeChecker_ptr type_checker = SymbTable_get_type_checker(symb_table);

    int parse_result = Parser_ReadNextExprFromString(next_formula_str, &expression);

    if (parse_result != 0 || Nil == expression) {
      fprintf(nusmv_stderr, "Parsing error: expected an next expression.\n");
      goto show_deps_usage;
    }

    expression = car(expression);

    if (!TypeChecker_is_expression_wellformed(type_checker, expression, Nil)) {
      goto show_deps_usage;
    }

    dependencies = ComputeCOIFixpoint(symb_table, mainFlatHierarchy,
                                      expression, fixpoint, (boolean*)NULL);

    Set_PrintSet(nusmv_stderr, dependencies, NULL, NULL);
    fprintf(nusmv_stderr, "\n");

    Set_ReleaseSet(dependencies);
  }

  goto show_deps_free;

 show_deps_usage:
  res = UsageShowDependencies();
 show_deps_free:
  if (NIL(char) != next_formula_str) {
    FREE(next_formula_str);
  }
  return res;
}

static int UsageShowDependencies()
{
  fprintf(nusmv_stderr, "show_dependencies [-h] [-k bound] -e <next_expr>\n");
  fprintf(nusmv_stderr, "\t-k bound\tStop searching dependencies at step \"bound\"\n");
  fprintf(nusmv_stderr, "\t-e expr\tThe expression on which dependencies are computed on\n");
  return 1;
}

/**Function********************************************************************

  Synopsis    [ Encodes variables in the model (BDD only). ]

  Description [ ]

  SideEffects [ ]

******************************************************************************/
int compile_encode_variables()
{
  BoolEnc_ptr bool_enc;
  BddEnc_ptr bdd_enc;


  /* Creates the bool encoding, and commit the model layer, that was
     created during the flattening phase */
  Enc_init_bool_encoding();
  bool_enc = Enc_get_bool_encoding();
  BaseEnc_commit_layer(BASE_ENC(bool_enc), MODEL_LAYER_NAME);

  /* Creates the bdd encoding, and again commit the model layer */
  Enc_init_bdd_encoding();
  bdd_enc = Enc_get_bdd_encoding();
  BaseEnc_commit_layer(BASE_ENC(bdd_enc), MODEL_LAYER_NAME);

  #if 0
  // RC: test begins
  {
    extern VarsHandler_ptr dd_vars_handler;

    SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(bdd_enc));
    BddEnc_ptr bdd_enc2 = BddEnc_create(st,
                                        bool_enc,
                                        dd_vars_handler,
                                        NULL);

    BaseEnc_commit_layer(BASE_ENC(bdd_enc2), MODEL_LAYER_NAME);

    {
      OrdGroups_ptr groups = OrdGroups_create();
      NodeList_ptr list;
      ListIter_ptr iter;

      list = SymbTable_get_vars(st);
      NODE_LIST_FOREACH(list, iter) {
        node_ptr var = NodeList_get_elem_at(list, iter);
        int grp = OrdGroups_create_group(groups);
        OrdGroups_add_variable(groups, var, grp);
      }

      BddEnc_force_order(bdd_enc2, groups);

      OrdGroups_destroy(groups);
    }

    BaseEnc_remove_layer(BASE_ENC(bdd_enc), MODEL_LAYER_NAME);
    BaseEnc_remove_layer(BASE_ENC(bdd_enc2), MODEL_LAYER_NAME);

    fprintf(nusmv_stderr, "Enc test done\n");
  }

#if 0
  { // test for Olist sort
    Olist_ptr list1 = Olist_create();

    int i = 0;
    for (i=0; i < 10; ++i) {
      Olist_append(list1, utils_random() % 20);
    }

    void list_print(Olist_ptr list, const char* name)
    {
      Oiter iter;
      if (name) fprintf(nusmv_stderr, "%s: ", name);
      OLIST_FOREACH(list, iter) {
        int el = Oiter_element(iter);
        fprintf(nusmv_stderr, "%d ", el);
      }
      fprintf(nusmv_stderr, "\n");
    }
    int int_cmp(int a, int b) { return a - b; }

    list_print(list1, "list1");
    Olist_sort(list1, &int_cmp);
    list_print(list1, "list1");


    Olist_ptr list2 = Olist_create();
    for (i=0; i < 10; ++i) {
      Olist_append(list2, i+20);
    }
    list_print(list2, "list2");

    //    Olist_move(list1, list2, Olist_last(list2));
    Olist_move(list1, list2, Oiter_next(Olist_first(list2)));
    list_print(list2, "list2");

    Olist_destroy(list2);
    Olist_destroy(list1);
  }
#endif

  // RC: test ends
  #endif

  return 0;
}


/**Function********************************************************************

  Synopsis    [ Traverses the parse tree coming from the smv parser and
                flattens the smv file. ]

  Description [ ]

  SideEffects [ ]

******************************************************************************/
int compile_flatten_smv(boolean calc_vars_constrains)
{
  /* Initializes the flattener, that must be initialized *after* the
     parsing phase */
  CompileFlatten_init_flattener();

  /* Processing of the parse tree and constructions of all the
    expressions for the state machine(s). Here the expansions are
    performed so that modules and processes are created. The expansion
    of modules is such that the formal parameters (if any) are
    replaced by the actual ones and the machine is replicated.
  */
  {
    SymbTable_ptr st;
    SymbLayer_ptr layer;
    int propErr;

    st = Compile_get_global_symb_table();
    layer = SymbTable_create_layer(st, MODEL_LAYER_NAME,
                                   SYMB_LAYER_POS_BOTTOM);

    /* register the new layer to the "model" layer class, and set
       the class to be the default */
    SymbTable_layer_add_to_class(st, MODEL_LAYER_NAME, MODEL_LAYERS_CLASS);
    SymbTable_set_default_layers_class_name(st,  MODEL_LAYERS_CLASS);

    nusmv_assert(FLAT_HIERARCHY(NULL) == mainFlatHierarchy);

    /* If this is not the first call of compile_flatten_smv, it may
       happen that the mainHrcNode contains corrupted data. This may
       happen when a parsing error occurs, and the error is fixed
       without closing NuSMV. See mantis issue 1850 for details */
    if (HRC_NODE(NULL) != mainHrcNode) {
      HrcNode_cleanup(mainHrcNode);
    }

    mainFlatHierarchy = Compile_FlattenHierarchy(st, layer, sym_intern("main"),
                                                 Nil, Nil, true,
                                                 calc_vars_constrains,
                                                 mainHrcNode);

    /* Processes or Isa have been found, destroy the HRC hierarchy */
    if (mainHrcNode != HRC_NODE(NULL)) {
      if ((void*) NULL != HrcNode_get_undef(mainHrcNode)) {
        HrcNode_destroy_recur(mainHrcNode);
        mainHrcNode = HRC_NODE(NULL);
      }
    }

    /* We store properties in the DB of properties */
    propErr = PropDb_fill(PropPkg_get_prop_database(),
                          st,
                          FlatHierarchy_get_spec(mainFlatHierarchy),
        FlatHierarchy_get_compute(mainFlatHierarchy),
        FlatHierarchy_get_ltlspec(mainFlatHierarchy),
        FlatHierarchy_get_pslspec(mainFlatHierarchy),
        FlatHierarchy_get_invarspec(mainFlatHierarchy));

    if (0 != propErr) {
      /* cleans up the initialized internal structures */
      FlatHierarchy_destroy(mainFlatHierarchy);
      mainFlatHierarchy = FLAT_HIERARCHY(NULL);
      SymbTable_remove_layer(st, layer);
      goto flattening_failed; /* error */
    }
  }

  TraceManager_register_layer(TracePkg_get_global_trace_manager(),
                              MODEL_LAYER_NAME);

  /* if syntax errors have been found when parser mode is set to lax */
  if (Parser_get_syntax_errors_list() != Nil) {
    fprintf(nusmv_stderr, "\nWarning! Syntax errors have been found, no "
            "flattening is possible.\n");
    fprintf(nusmv_stderr, "However, as option '%s' is set, a partial "
            "construction\nof the HRC was done.\n",  OPT_PARSER_IS_LAX);
    fprintf(nusmv_stderr, "This allows you to dump the HRC.\n");
    cmp_struct_set_hrc_built(cmps);
    return 0;
  }

  /* everything went ok */
  cmp_struct_set_hrc_built(cmps);
  cmp_struct_set_flatten_hrc(cmps);
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "...done\n");
  }

  return 0;

  /* Exception handling */
 flattening_failed:
  PropDb_clean(PropPkg_get_prop_database());
  CompileFlatten_quit_flattener();
  cmp_struct_unset_read_model(cmps); /* resets also the command read_model */
  return 1; /* error */
}

/**Function********************************************************************

  Synopsis           [creates the  master scalar fsm if needed]

  Description        []

  SideEffects        []

******************************************************************************/
static void compile_create_flat_model()
{
  SymbTable_ptr st;
  Set_t vars;
  SexpFsm_ptr sexp_fsm;
  SymbLayer_ptr layer;
  SymbLayerIter iter;

  if (PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database()) != SEXP_FSM(NULL)) return;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "\nCreating the scalar FSM...\n");
  }

  st = Compile_get_global_symb_table();

  layer = SymbTable_get_layer(st, MODEL_LAYER_NAME);

  SymbLayer_gen_iter(layer, &iter, STT_VAR);
  vars = SymbLayer_iter_to_set(layer, iter);

  /* scalar fsm */
  sexp_fsm = FsmBuilder_create_scalar_sexp_fsm(global_fsm_builder,
                                               mainFlatHierarchy,
                                               vars);

  Set_ReleaseSet(vars);

  /* Builds the sexp FSM of the whole read model */
  PropDb_master_set_scalar_sexp_fsm(PropPkg_get_prop_database(), sexp_fsm);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "Successfully created the scalar FSM\n");
  }

  /* We keep track that the master FSM has been built. */
  cmp_struct_set_build_flat_model(cmps);
}


/**Function********************************************************************

  Synopsis  [Creates the  master boolean fsm if needed.
  A new layer called DETERM_LAYER_NAME
  will be added if the bool fsm is created.]

  Description [The newly created layer will be committed to both the
  boolean and bdd encodings. Warning: it is assumed here that the flat model
  has been already created]

  SideEffects        []

******************************************************************************/
static void compile_create_boolean_model()
{
  if (BOOL_SEXP_FSM(NULL) == PropDb_master_get_bool_sexp_fsm(
                                          PropPkg_get_prop_database())) {
    BddEnc_ptr benc;
    SexpFsm_ptr scalar_fsm;
    BoolSexpFsm_ptr bool_fsm;
    SymbTable_ptr st;
    SymbLayer_ptr det_layer;

    int reord_status;
    dd_reorderingtype rt;
    DdManager* dd;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "\nCreating the boolean FSM...\n");
    }


    benc = Enc_get_bdd_encoding();
    st = BaseEnc_get_symb_table(BASE_ENC(benc));
    dd = BddEnc_get_dd_manager(benc);

    /* temporary disables reordering */
    reord_status = dd_reordering_status(dd, &rt);
    if (reord_status == 1) { dd_autodyn_disable(dd); }

    /* Creates a new layer for determinization vars */
    det_layer = SymbTable_create_layer(st, DETERM_LAYER_NAME,
                                       SYMB_LAYER_POS_BOTTOM);

    SymbTable_layer_add_to_class(st, DETERM_LAYER_NAME, NULL);
    SymbTable_layer_add_to_class(st, DETERM_LAYER_NAME,
                                 ARTIFACTS_LAYERS_CLASS);
    scalar_fsm = PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());

    CATCH {
      bool_fsm = BoolSexpFsm_create_from_scalar_fsm(scalar_fsm,
                                                    benc, det_layer);
    }
    FAIL {
      SymbTable_remove_layer(st, det_layer);
      rpterr(NULL); /* re-throw */
    }

    PropDb_master_set_bool_sexp_fsm(PropPkg_get_prop_database(), bool_fsm);

    /* Possibly added determinization variables introduced while
       building the boolean sexp model must be committed to the
       encodings */
    BaseEnc_commit_layer(BASE_ENC(Enc_get_bool_encoding()),
                         DETERM_LAYER_NAME);
    BaseEnc_commit_layer(BASE_ENC(Enc_get_bdd_encoding()),
                         DETERM_LAYER_NAME);

    /* If dynamic reordering was enabled, then it is re-enabled */
    if (reord_status == 1) { dd_autodyn_enable(dd, rt); }

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "Successfully created the boolean FSM\n");
    }

  }
}


/**Function********************************************************************

  Synopsis    [ Builds the BDD fsm. ]

  Description [ ]

  SideEffects [ ]

******************************************************************************/
void compile_build_model(boolean force_build)
{
  SexpFsm_ptr sexp_fsm;
  BddFsm_ptr  bdd_fsm;

  /* creates the model only if required (i.e. build_flat_model not called) */
  compile_create_flat_model();

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "\nCreating the BDD FSM...\n");
  }

  sexp_fsm = PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());

  bdd_fsm = FsmBuilder_create_bdd_fsm(global_fsm_builder,
                                      Enc_get_bdd_encoding(),
                                      sexp_fsm,
                                      get_partition_method(OptsHandler_get_instance()));

  /* Finally stores the built FSMs: */
  if (PropDb_master_get_bdd_fsm(PropPkg_get_prop_database()) != BDD_FSM(NULL)) {
    nusmv_assert(force_build);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "Forcing destruction of previoulsy created BDD FSM...\n");
    }

    /* destroys the previously created fsm, but first reuse cache */
    BddFsm_copy_cache(bdd_fsm, PropDb_master_get_bdd_fsm(PropPkg_get_prop_database()),
                      true /*keeps family*/);

    /* this will destroy the current fsm */
    PropDb_master_set_bdd_fsm(PropPkg_get_prop_database(), BDD_FSM(NULL));
  }

  /* Finally stores the built FSMs: */
  PropDb_master_set_bdd_fsm(PropPkg_get_prop_database(), bdd_fsm);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "Successfully created the BDD FSM\n");
  }
}


/**Function********************************************************************

  Synopsis           [Removes expression in the form "a := b" from the given
                      expression]

  Description        [Removes expression in the form "a := b" from the given
                      expression. The new expression is returned]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Expr_ptr compile_cmd_remove_assignments(Expr_ptr expr)
{
  if (Nil == expr) return Expr_true();

  if (AND == node_get_type(expr)) {
    return Expr_and(compile_cmd_remove_assignments(car(expr)),
                    compile_cmd_remove_assignments(cdr(expr)));
  }
  else if (EQDEF == node_get_type(expr)) {
    return Expr_true();
  }

  return expr;
}

/**Function********************************************************************

  Synopsis           [Dumps the COI for the given property]

  Description        [Dumps the COI for the given property]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void compile_cmd_write_coi_prop(Set_t cone, Set_t props,
                                       FILE* output_file)
{
  Set_Iterator_t iter;
  boolean multiple = (Set_GiveCardinality(props) > 1);
  boolean keep;

  fprintf(output_file, "Propert%s",
          (multiple ? "ies\n" : "y "));

  if (multiple) {
    int count = 1;
    SET_FOREACH(props, iter) {
      Prop_ptr prop = PROP(Set_GetMember(props, iter));
      fprintf(output_file, "\t%d) ", (count++));
      Prop_print(prop, output_file,
                 get_prop_print_method(OptsHandler_get_instance()));
      fprintf(output_file, "\n");
    }
  }
  else {
    Prop_ptr prop;
    iter = Set_GetFirstIter(props);
    prop = PROP(Set_GetMember(props, iter));
    Prop_print(prop, output_file,
               get_prop_print_method(OptsHandler_get_instance()));
  }

  fprintf(output_file, "%s COI:\n",
          (multiple ? "share" : "has"));
  fprintf(output_file, "   {\n   ");

  iter = Set_GetFirstIter(cone);
  keep = !Set_IsEndIter(iter);
  while (keep) {
    print_node(output_file, (node_ptr) Set_GetMember(cone, iter));
    iter = Set_GetNextIter(iter);
    keep = !Set_IsEndIter(iter);
    if (keep) fprintf(output_file, ",\n   ");
  }
  fprintf(output_file, "\n   }");
  fprintf(output_file, "\n");
}

/**Function********************************************************************

  Synopsis           [Dumps the model applied to COI for the given property]

  Description        [Dumps the model applied to COI for the given property]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void compile_cmd_write_coi_prop_fsm(FlatHierarchy_ptr fh,
                                           Set_t cone, Set_t props,
                                           FILE* output_file)
{
  SymbTable_ptr symb_table = FlatHierarchy_get_symb_table(fh);
  array_t* layers = SymbTable_get_class_layer_names(symb_table,
                                                    (const char*) NULL);
  SexpFsm_ptr fsm;
  FlatHierarchy_ptr coi_hierarchy = FLAT_HIERARCHY(NULL);
  FlatHierarchy_ptr fsm_fh = FLAT_HIERARCHY(NULL);
  FsmBuilder_ptr builder = FsmBuilder_create(dd_manager);

  fsm = FsmBuilder_create_scalar_sexp_fsm(builder, fh, cone);
  nusmv_assert(SEXP_FSM(NULL) != fsm);

  fsm_fh = SexpFsm_get_hierarchy(fsm);

  /* Do some post-processings over the COI hierarchy */
  {
    /* ----------------------------------------------------------------- */
    /* 1. -> INIT, TRANS, INVAR, INPUT, JUSTICE, COMPASSION. */
    {
      Expr_ptr init = FlatHierarchy_get_init(fsm_fh);
      Expr_ptr trans = FlatHierarchy_get_trans(fsm_fh);
      Expr_ptr invar = FlatHierarchy_get_invar(fsm_fh);

      coi_hierarchy =
        FlatHierarchy_create_from_members(symb_table,
                                          compile_cmd_remove_assignments(init),
                                          compile_cmd_remove_assignments(invar),
                                          compile_cmd_remove_assignments(trans),
                                          FlatHierarchy_get_input(fsm_fh),
                                          FlatHierarchy_get_justice(fsm_fh),
                                          FlatHierarchy_get_compassion(fsm_fh));
    }

    /* ----------------------------------------------------------------- */
    /* 2. -> VARIABLES and ASSIGNMENTS. */
    {
      /* Add variables of the COI into the new Flat Hierarchy, and all
         it's relative assignments  */
      Set_Iterator_t iter;
      SET_FOREACH(cone, iter) {
        node_ptr var = Set_GetMember(cone, iter);
        FlatHierarchy_add_var(coi_hierarchy, var);

        {
          /* Add assignments for all variables in COI */
          node_ptr init_var = find_node(SMALLINIT, var, Nil);
          node_ptr next_var = find_node(NEXT, var, Nil);
          node_ptr tmp;

          /* First try with normal assignments */
          tmp = FlatHierarchy_lookup_assign(fsm_fh, var);
          if (Nil != tmp) {
            FlatHierarchy_insert_assign(coi_hierarchy, var, tmp);
          }
          /* Try with NEXT and INIT */
          else {
            /* init(var) := expr */
            tmp = FlatHierarchy_lookup_assign(fsm_fh, init_var);
            if (Nil != tmp) {
              FlatHierarchy_insert_assign(coi_hierarchy, init_var, tmp);
            }
            /* next(var) := expr */
            tmp = FlatHierarchy_lookup_assign(fsm_fh, next_var);
            if (Nil != tmp) {
              FlatHierarchy_insert_assign(coi_hierarchy, next_var, tmp);
            }
          }
        } /* Assignments treating */
      } /* SET_FOREACH(cone, iter) */
    }

    /* ----------------------------------------------------------------- */
    /* 3. -> SPECIFICATIONS. */
    {
      Set_Iterator_t iter;

      FlatHierarchy_set_ltlspec(coi_hierarchy, Nil);
      FlatHierarchy_set_spec(coi_hierarchy, Nil);
      FlatHierarchy_set_pslspec(coi_hierarchy, Nil);
      FlatHierarchy_set_compute(coi_hierarchy, Nil);
      FlatHierarchy_set_invarspec(coi_hierarchy, Nil);

      SET_FOREACH(props, iter) {
        Prop_ptr prop = PROP(Set_GetMember(props, iter));
        Expr_ptr prop_expr = Prop_get_expr(prop);
        node_ptr prop_name = Prop_get_name(prop);

        /* Add only the property for which we are dumping the model */
        switch (Prop_get_type(prop)) {
        case Prop_Ltl:
          FlatHierarchy_set_ltlspec(coi_hierarchy,
                                    cons(find_node(LTLSPEC, prop_expr, prop_name),
                                         FlatHierarchy_get_ltlspec(coi_hierarchy)));
          break;
        case Prop_Ctl:
          FlatHierarchy_set_spec(coi_hierarchy,
                                 cons(find_node(SPEC, prop_expr, prop_name),
                                      FlatHierarchy_get_spec(coi_hierarchy)));
          break;
        case Prop_Psl:
          FlatHierarchy_set_pslspec(coi_hierarchy,
                                    cons(find_node(PSLSPEC, prop_expr, prop_name),
                                         FlatHierarchy_get_pslspec(coi_hierarchy)));
          break;
        case Prop_Compute:
          FlatHierarchy_set_compute(coi_hierarchy,
                                    cons(find_node(COMPUTE, prop_expr, prop_name),
                                         FlatHierarchy_get_compute(coi_hierarchy)));
          break;
        case Prop_Invar:
          FlatHierarchy_set_invarspec(coi_hierarchy,
                                      cons(find_node(INVARSPEC, prop_expr, prop_name),
                                           FlatHierarchy_get_invarspec(coi_hierarchy)));
          break;
        default:
          fprintf(nusmv_stderr, "Unhandled property \"");
          Prop_print(prop, nusmv_stderr,
                     get_prop_print_method(OptsHandler_get_instance()));
          fprintf(nusmv_stderr, "\"\n");
          return;
        }
      }
    }
  } /* end of Hierarchy post-processings */

  Compile_WriteRestrictedFlattenModel(output_file, symb_table, layers,
                                      "MODULE main", coi_hierarchy, true);

  FlatHierarchy_destroy(coi_hierarchy);
  FsmBuilder_destroy(builder);
  SexpFsm_destroy(fsm);
}


/**Function********************************************************************

  Synopsis           [Computes the total bit number of symbols in the given
                      list]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int get_bits(const SymbTable_ptr st, const NodeList_ptr lst)
{
  ListIter_ptr iter;
  int res;

  res = 0;

  NODE_LIST_FOREACH(lst, iter) {
    node_ptr var;
    SymbType_ptr type;

    var = NodeList_get_elem_at(lst, iter);
    type = SymbTable_get_var_type(st, var);

    if (SymbType_is_boolean(type)) {
      res += 1;
    }
    else if (SymbType_is_enum(type)) {
      res += Utils_log2_round(llength(SymbType_get_enum_type_values(type)));
    }
    else if (SymbType_is_word(type)) {
      res += SymbType_get_word_width(type);
    }
    else if (SymbType_is_wordarray(type)){
      res += (SymbType_get_wordarray_awidth(type) *
              SymbType_get_wordarray_vwidth(type));
    }
    else {
      fprintf(nusmv_stderr,
              "**WARNING** Unknown variable type in bit counting: ");
      SymbType_print(type, nusmv_stderr);
      fprintf(nusmv_stderr,
              ".\n");
    }
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Dumps on output_file the global coi FSM]

  Description        [Dumps on output_file the FSM built using the union of all
                      properties cone of influence. Properties can be filtered
                      by type using prop_type: if prop_type == Prop_NoType,
                      all properties are used]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void compile_cmd_write_global_coi_fsm(FlatHierarchy_ptr hierarchy,
                                             Prop_Type prop_type,
                                             FILE* output_file)
{
  int prop_no;
  Set_t coi_union = Set_MakeEmpty();
  Set_t properties = Set_MakeEmpty();
  SymbTable_ptr symb_table = FlatHierarchy_get_symb_table(hierarchy);

  for (prop_no = 0; prop_no < PropDb_get_size(PropPkg_get_prop_database()); ++prop_no) {
    Prop_ptr prop = PropDb_get_prop_at_index(PropPkg_get_prop_database(), prop_no);

    if ((prop_type == Prop_NoType) || (prop_type == Prop_get_type(prop))) {
      Set_t cone = Prop_compute_cone(prop, hierarchy, symb_table);
      coi_union = Set_Union(coi_union, cone);
      properties = Set_AddMember(properties, (Set_Element_t)prop);
      Set_ReleaseSet(cone);
    }
  }

  compile_cmd_write_coi_prop_fsm(hierarchy, coi_union,
                                 properties, output_file);

  Set_ReleaseSet(coi_union);
  Set_ReleaseSet(properties);
}

/**Function********************************************************************

  Synopsis           [Dumps properties shared COI FSMs or sets]

  Description        [Dumps properties shared COI informations.
                      If only_dump_coi is true, only the set of
                      variables in the cone of each property is
                      dumped. Otherwise, an FSM is created and
                      dumped. Properties with the same COI will appear
                      in the same FSM. Properties can be filtered by
                      type using prop_type: if prop_type ==
                      Prop_NoType, all properties are used]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int compile_cmd_write_properties_coi(FlatHierarchy_ptr hierarchy,
                                            Prop_Type prop_type,
                                            boolean only_dump_coi,
                                            const char* file_name)
{
  NodeList_ptr order =
    PropDb_get_coi_grouped_properties(PropPkg_get_prop_database(),
                                      hierarchy);
  ListIter_ptr iter;
  int prop_no = 0;
  FILE* output_file = nusmv_stdout;

  NODE_LIST_FOREACH(order, iter) {
    node_ptr couple = NodeList_get_elem_at(order, iter);
    Set_t cone = (Set_t)car(couple);
    Set_t props = (Set_t)cdr(couple);
    Set_t filtered;
    char* new_file_name = NIL(char);

    if (Prop_NoType == prop_type) {
      filtered = Set_Copy(props);
    }
    else {
      Set_Iterator_t iter;
      filtered = Set_MakeEmpty();
      SET_FOREACH(props, iter) {
        Prop_ptr prop = PROP(Set_GetMember(props, iter));
        if (Prop_get_type(prop) == prop_type) {
          filtered = Set_AddMember(filtered, (Set_Element_t)prop);
        }
      }
    }

    if (!Set_IsEmpty(filtered)) {

      if (NIL(char) != file_name) {
        int max_len = strlen(file_name) + 5;
        int chars;

        new_file_name = ALLOC(char, max_len);
        chars = snprintf(new_file_name, max_len, "%s.%d", file_name, prop_no);
        SNPRINTF_CHECK(chars, max_len);

        output_file = fopen(new_file_name, "w");
        if ((FILE*) NULL == output_file) {
          fprintf(nusmv_stderr, "Cannot open file '%s' for writing\n",
                  new_file_name);
          return 1;
        }
      }

      if (only_dump_coi) {
        compile_cmd_write_coi_prop(cone, filtered, output_file);
      }
      else {
        compile_cmd_write_coi_prop_fsm(hierarchy, cone,
                                       filtered, output_file);
      }

      fflush(output_file);
      if (NIL(char) != file_name) {
        fclose(output_file);
        FREE(new_file_name);
      }

      ++prop_no;
    }

    Set_ReleaseSet(cone);
    Set_ReleaseSet(props);
    Set_ReleaseSet(filtered);
    free_node(couple);
  }
  NodeList_destroy(order);

  return 0;
}

/**Function********************************************************************

  Synopsis           [Prints the given type to the given stream]

  Description        [Prints the given type to the given stream.

                      The type must be created with the
                      compile_cmd_get_var_type function. If the type
                      is scalar, then values are printed until
                      "threshold" number of characters are reached. If
                      some values are missing because of the
                      threshold, then "other # values" is added in
                      output]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void compile_cmd_print_type(FILE * file, node_ptr ntype, int threshold)
{
      switch(node_get_type(ntype)) {

    case BOOLEAN:
      fprintf(file, "boolean\n");
      break;
    case SCALAR:
      {
        int i = 0, missing = 0;
        node_ptr l = car(ntype);
        Olist_ptr values_list = Olist_create();
        Oiter iter;
        const char* fmt = "(other %d values)";

        fprintf(file, "{");
        while (l != Nil) {
          node_ptr val = car(l);
          l = cdr(l);

          /* One element has been already added to the missing set,
             all other elements must be skipped. */
          if (missing > 0) {
            /* Do not add the string representation of the node if it
               is not necessary.. */
            Olist_append(values_list, NULL);
            ++missing;
          }
          else {
            char* tmp = sprint_node(val);
            Olist_append(values_list, tmp);

            i += strlen(tmp) + (l != Nil ? 2 : 0);

            /* Threshold == 0 means no limit.. */
            if (i > threshold && (threshold != 0)) {
              /* Restore the latests maximum length, so that the "fmt"
                 string can be printed within the threshold length.. */
              Olist_ptr rev = Olist_copy_reversed(values_list);
              Oiter ri;

              OLIST_FOREACH(rev, ri) {
                char* v = Oiter_element(ri);
                int l = (strlen(v) + 2);

                ++missing;
                i -= l;

                /* +2: Save some space for big numbers: if the number
                   of missing values is very high, the number printed
                   in the fmt will use alot of characters. In this
                   way, we are assured that we can print correctly up
                   to 9999 values. */
                if (i <= (threshold - (strlen(fmt) + 2))) {
                  break;
                }
              }
              Olist_destroy(rev);
            }
          }
        }

        i = 0;
        OLIST_FOREACH(values_list, iter) {
          char* el = (char*)Oiter_element(iter);

          if (i < (Olist_get_size(values_list) - missing)) {
            fprintf(file, "%s%s", el,
                    (++i == Olist_get_size(values_list) ? "" : ", "));
          }

          if ((char*)NULL != el) {
            FREE(el);
          }
        }
        if (missing > 0) { fprintf(file, fmt, missing); }

        fprintf(file, "}\n");
        Olist_destroy(values_list);
        break;
      }
    case SIGNED_WORD:
      fprintf(file, "signed word[%d]\n", PTR_TO_INT(car(ntype)));
      break;
    case UNSIGNED_WORD:
      fprintf(file, "unsigned word[%d]\n", PTR_TO_INT(car(ntype)));
      break;
    case REAL:
      fprintf(file, "real\n");
      break;
    case INTEGER:
      fprintf(file, "integer\n");
      break;
    default:
      rpterr("Unsupported type found.");
      error_unreachable_code();
    }

}

/**Function********************************************************************

  Synopsis           [Creates an internal representaion of the symbol type]

  Description        [Creates an internal representaion of the symbol type.
                      The representation of the type returned is
                      intended to be used only with the
                      compile_cmd_print_type procedure. If 2 types are
                      the same, the same node is returned]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr compile_cmd_get_var_type(SymbType_ptr type)
{
  node_ptr ntype = Nil;
  int j;

  switch (SymbType_get_tag(type)) {
  case SYMB_TYPE_BOOLEAN:
    ntype = find_node(BOOLEAN, Nil, Nil);
    break;
  case SYMB_TYPE_ENUM:
    ntype = find_node(SCALAR,
                      node_normalize(SymbType_get_enum_type_values(type)), Nil);
    break;
    /* (infinite-precision) integer */

  case SYMB_TYPE_INTEGER:
    ntype = find_node(INTEGER, Nil, Nil);
    break;

    /* (infinite-precision) rational */
  case SYMB_TYPE_REAL:
    ntype = find_node(REAL, Nil, Nil);
    break;

    /* word is like an arrary of booleans + signed arithmetic */
  case SYMB_TYPE_SIGNED_WORD:
    j = SymbType_get_word_width(type);
    ntype = find_node(SIGNED_WORD, PTR_FROM_INT(node_ptr, j), Nil);
    break;

    /* word is like an arrary of booleans + unsigned arithmetic */
  case SYMB_TYPE_UNSIGNED_WORD:
    j = SymbType_get_word_width(type);
    ntype = find_node(UNSIGNED_WORD, PTR_FROM_INT(node_ptr, j), Nil);
    break;

  default:
    rpterr("Unsupported type found.");
    error_unreachable_code();
  }

  return ntype;
}
