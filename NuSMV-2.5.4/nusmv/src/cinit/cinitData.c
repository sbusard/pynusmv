/**CFile***********************************************************************

   FileName    [cinitData.c]

   PackageName [cinit]

   Synopsis [Helper for the NuSMV library. Helps with extending it or
   creating new tools out of it.]

   Author      [Alessandro Mariotti]

   Copyright   [
   This file is part of the ``cinit'' package of NuSMV version 2.
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

#include "cinitInt.h"
#include "utils/ustring.h"
#include "utils/utils_io.h"

/* Olist are used since they do not have any dependency among packages
   initializations */
#include "utils/Olist.h"

#include "opt/optInt.h"
#include "bmc/bmc.h"
#include "trans/transInt.h"

typedef struct CmdLineOpt_TAG {
  char* name;
  char* usage;
  char* parameter;

  /* Used for special options */
  boolean (*check_and_apply)(OptsHandler_ptr, char*);

  /* Used for options which have an associated env var. */
  char* env_option;

  boolean deprecated;
  boolean public;

  /* These 2 are used after initialization of the system. Until that,
     we use the below structures. */
  string_ptr dependency;
  Olist_ptr conflicts;

  /* TEMP fields */
  char* tmp_dependency;
  char* tmp_conflicts;
} CmdLineOpt;

typedef CmdLineOpt* CmdLineOpt_ptr;

typedef struct CoreData_TAG {
  char* tool_name;
  char* tool_rcfile;
  char* tool_version;
  char* build_date;
  char* prompt_string;
  char* email;
  char* website;
  char* bug_report_message;
  char* linked_addons;

  char* library_name;
  char* library_version;
  char* library_build_date;
  char* library_email;
  char* library_website;
  char* library_bug_report_message;

  void (*print_banner)(FILE*);
  void (*batch)(void);

  int init_quit_funs_num;

  OptsHandler_ptr opt_manager;

  boolean initialized;

  /* This one can be initialized only after the initialization of the
     system. Until that, we use a simple array */
  hash_ptr line_options;

  /* TEMP fields. OList is completely independant, it has no
     dependencies that have to be initialized */
  Olist_ptr tmp_line_options;

  FP_V_V  ** init_quit_funs;
} CoreData;

typedef  CoreData* CoreData_ptr;

static CoreData_ptr core_data = (CoreData_ptr)NULL;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define PRINT_USAGE_IN_NEWLINE 1

/* RETURN VALUES */

/* Everything went smooth */
#define RET_SUCCESS                0

/* The script file does not exits or it contains an error and
   set_on_failure_script_quits is set. */
#define RET_SCRIPT_ERROR          -1

/* The help has been printed out */
#define RET_HELP_PRINT             2

/* An unknown option is given */
#define RET_UNKNOWN_OPTION        -1

/* An error occurred with this option */
#define RET_INVALID_OPTION        -1

/* An invalid parameter is given */
#define RET_INVALID_OPTION_PARAM  -1

/* A required parameter is not given */
#define RET_MISSING_OPTION_PARAM  -1

/* The option depends among another option which is not given */
#define RET_MISSING_OPTION_DEP    -1

/* The option conflicts with another option which is given */
#define RET_CONFLICT_OPTION       -1

/* The option is given more than once */
#define RET_DUPLICATE_OPTION       -1

/* Max width of a printed string (in characters) */
#define MAX_PRINT_WIDTH           75

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static CoreData_ptr nusmv_core_get_instance ARGS((void));
static void nusmv_core_deinit ARGS((void));

static int nusmv_core_parse_line_options ARGS((int argc, char ** argv));

static void nusmv_core_print_usage ARGS((boolean print_banner));

static CmdLineOpt_ptr nusmv_core_init_opt ARGS((void));
static void nusmv_core_deinit_opt ARGS((CmdLineOpt_ptr opt));

static void nusmv_core_print_string ARGS((FILE* out, char* str, int padding));

static void nusmv_core_olist_union ARGS((Olist_ptr a, Olist_ptr b));
static Olist_ptr
nusmv_core_olist_intersection ARGS((Olist_ptr a, Olist_ptr b));

static char* nusmv_core_merge ARGS((Olist_ptr set));
static Olist_ptr nusmv_core_split ARGS((char* str));
static char* nusmv_core_tolower ARGS((char* str));

static void
nusmv_core_free_line_options ARGS((CoreData_ptr core_data));

/* ------------------------- CHECK AND APPLY FUNCTIONS --------------------- */
static boolean
nusmv_core_check_sin_fun ARGS((OptsHandler_ptr opt, char* val));
static boolean
nusmv_core_check_rbc_fun ARGS((OptsHandler_ptr opt, char* val));
static boolean
nusmv_core_set_mono_partition ARGS((OptsHandler_ptr opt, char* val));
static boolean
nusmv_core_set_iwls95_partition ARGS((OptsHandler_ptr opt, char* val));
static boolean
nusmv_core_set_thresh_partition ARGS((OptsHandler_ptr opt, char* val));
static boolean
nusmv_core_set_cpp ARGS((OptsHandler_ptr opt, char* val));
static boolean
nusmv_core_set_dp ARGS((OptsHandler_ptr opt, char* val));
static boolean
core_data_set_fs ARGS((OptsHandler_ptr opt, char* val));
static boolean
nusmv_core_set_pre ARGS((OptsHandler_ptr opt, char* val));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis    [Initializes the NuSMVCore data. This  function has to be called
   _BEFORE_ doing anything else with the library]

   Description [Initializes the NuSMVCore data. The following operations are
   performed:

   1) Initialize the internal class
   2) Sets all fields to default for NuSMV
   ]

   SideEffects []

   SeeAlso     [Package_init_cmd_options Package_init]

******************************************************************************/
void NuSMVCore_init_data()
{
  CoreData_ptr data = nusmv_core_get_instance();

  NuSMVCore_set_tool_name(NUSMV_PACKAGE_NAME);
  NuSMVCore_set_tool_version(NUSMV_PACKAGE_VERSION);
  NuSMVCore_set_build_date(NUSMV_PACKAGE_BUILD_DATE);
  NuSMVCore_set_prompt_string("NuSMV > ");
  NuSMVCore_set_email("nusmv-users@list.fbk.eu");
  NuSMVCore_set_website("http://nusmv.fbk.eu");
  {
    char* fmt = "Please report bugs to <%s>";
    char* tmp = ALLOC(char, strlen(fmt) + strlen(NUSMV_PACKAGE_BUGREPORT) + 1);
    sprintf(tmp, fmt, NUSMV_PACKAGE_BUGREPORT);
    NuSMVCore_set_bug_report_message(tmp);
    FREE(tmp);
  }
  NuSMVCore_set_linked_addons(NUSMV_LINKED_CORE_ADDONS);

  /* Library's fields do not have a SETTER function, since they are
     unchangeable from the tools */
  data->library_name = util_strsav(NUSMV_LIBRARY_NAME);
  data->library_version = util_strsav(NUSMV_LIBRARY_VERSION);
  data->library_build_date = util_strsav(NUSMV_LIBRARY_BUILD_DATE);
  data->library_website = util_strsav(NUSMV_LIBRARY_WEBSITE);
  data->library_email = util_strsav(NUSMV_LIBRARY_EMAIL);
  data->library_bug_report_message = util_strsav(NUSMV_LIBRARY_BUGREPORT);

  data->opt_manager = OptsHandler_get_instance();

  data->init_quit_funs = NULL;
  data->init_quit_funs_num = 0;

  data->batch = cinitBatchMain;
  data->print_banner = CInit_BannerPrint;
}


/**Function********************************************************************

   Synopsis           [Initializes the system]

   Description        [Initializes the system. First calls the
                       NuSMVCore initialization function, and then
                       calls each initialization function that is in
                       the given list. The order of the list is
                       followed.
                       The list must be declared as follows:

                       FP_V_V funs[][2] = {{Init_1, Quit_1},
                                           {Init_2, Quit_2},
                                            ...
                                           {Init_n, Quit_n}
                                           }]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_reset_first_fun NuSMVCore_set_reset_last_fun]

******************************************************************************/
void NuSMVCore_init(FP_V_V funs[][2], int num_funs)
{
  CoreData_ptr data = nusmv_core_get_instance();
  Oiter iter;
  int i;

  if (num_funs > 0) {
    nusmv_assert(NULL != funs);

    data->init_quit_funs = ALLOC(FP_V_V*, num_funs);
    for (i = 0; i < num_funs; i++) {
      data->init_quit_funs[i] = ALLOC(FP_V_V, 2);
      data->init_quit_funs[i][0] = funs[i][0];
      data->init_quit_funs[i][1] = funs[i][1];
    }
  }
  data->init_quit_funs_num = num_funs;

  CInit_init();

  for (i = 0; i < num_funs; ++i) {
    data->init_quit_funs[i][0]();
  }

  if ((hash_ptr)NULL == data->line_options) {
    data->line_options = new_assoc();
  }
  /* Prepares all command line options registered before the call of
     NuSMVCore_init */
  OLIST_FOREACH(data->tmp_line_options, iter) {
    CmdLineOpt_ptr opt = (CmdLineOpt_ptr)Oiter_element(iter);

    if ((char*)NULL != opt->tmp_dependency) {
      opt->dependency = find_string(opt->tmp_dependency);
    }
    if ((char*)NULL != opt->tmp_conflicts) {
      Olist_ptr split = nusmv_core_split(opt->tmp_conflicts);
      nusmv_core_olist_union(opt->conflicts, split);
      Olist_destroy(split);
    }

    insert_assoc(data->line_options,
                 NODE_PTR(find_string(opt->name)), NODE_PTR(opt));
  }

  data->initialized = true;
}


/**Function********************************************************************

   Synopsis    [Initializes all NuSMV library command line options]

   Description [Initializes all NuSMV library command line options.
   All command line options are registered within the
   library.  If standard command line options are needed,
   this function has to be called before NuSMVCore_main and
   after NuSMVCore_init]

   SideEffects []

   SeeAlso     [Package_init Package_main]

******************************************************************************/
void NuSMVCore_init_cmd_options()
{
  char* libraryName = CInit_NuSMVObtainLibrary();
  CoreData_ptr data = nusmv_core_get_instance();

  {
    const char* fmt = "does not read any initialization file "
      "(%s/master%s, ~/%s) (default in batch mode)";
    char* tmp = ALLOC(char, strlen(fmt) + strlen(libraryName) +
                (2 * strlen(data->tool_rcfile)) + 1);

    sprintf(tmp, fmt, libraryName, data->tool_rcfile, data->tool_rcfile);
    NuSMVCore_add_env_command_line_option("-s", tmp, NULL, IGNORE_INIT_FILE,
                                          true, false, NULL, NULL);
    FREE(tmp);
  }

  NuSMVCore_add_env_command_line_option("-old_div_op",
                                        "enables the old semantics of \"/\" "
                                        "and \"mod\" operations instead"
                                        " of ANSI C semantics",
                                        NULL, USE_ANSI_C_DIV_OP,
                                        true, false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-m", "sets the variable ordering "
                                        "method to \"method\". "
                                        "Reordering will be activated",
                                        "method", REORDER_METHOD,
                                        true, false, NULL, NULL);

  {
    BddOregJusticeEmptinessBddAlgorithmType alg, a1, a2;
    alg = DEFAULT_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM;
    a1 = BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD;
    a2 = BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD;

    const char* salg =
      Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string(alg);
    const char* sa1 =
      Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string(a1);
    const char* sa2 =
      Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string(a2);

    const char* fmt = "sets the algorthim used for BDD-based language "
      "emptiness of Buchi fair transition systems "
      "(default is %s). The available algorthims are: %s, %s";

    char* tmp = ALLOC(char, strlen(fmt) + strlen(salg) +
                      strlen(sa1) + strlen(sa2) + 1);

    sprintf(tmp, fmt, salg, sa1, sa2);

    NuSMVCore_add_env_command_line_option("-ojeba", tmp, "str",
                                          OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM,
                                          true, false, NULL, NULL);
    FREE(tmp);
  }

  {
    char* tmp;
    char* preps_tmp;
    const char* fmt = "defines a space-separated list of pre-processors "
      "to run (in the order given) on the input file. "
      "The list must be in double quotes if there is more "
      "than one pre-processor named.\n%s";

    if (get_preprocessors_num() > 0) {
      const char* preps_fmt = "The available preprocessors are: %s";
      char* preps = get_preprocessor_names();
      preps_tmp = ALLOC(char, strlen(preps_fmt) + strlen(preps) + 1);
      sprintf(preps_tmp, preps_fmt, preps);
      FREE(preps);
    }
    else {
      const char* preps_fmt = "Warning: there are no available preprocessors";
      preps_tmp = ALLOC(char, strlen(preps_fmt) + 1);
      sprintf(preps_tmp, preps_fmt);
    }

    tmp = ALLOC(char, strlen(fmt) + strlen(preps_tmp) + 1);
    sprintf(tmp, fmt, preps_tmp);

    NuSMVCore_add_command_line_option("-pre", tmp, "pp_list",
                                      nusmv_core_set_pre,
                                      true, false, NULL, NULL);

    FREE(preps_tmp);
    FREE(tmp);
  }

  {
    const char* fmt = "sets the sat_solver variable, used by BMC. "
      "The available SAT solvers are: %s";

    char* tmp;
    char* solvers = Sat_GetAvailableSolversString();

    tmp = ALLOC(char, strlen(fmt) + strlen(solvers) + 1);
    sprintf(tmp, fmt, solvers);

    NuSMVCore_add_env_command_line_option("-sat_solver", tmp,
                                           "str", A_SAT_SOLVER,
                                          true, false, NULL, NULL);

    FREE(solvers);
    FREE(tmp);
  }

  NuSMVCore_add_command_line_option("-sin", "enables (on) or disables sexp"
                                    " inlining (default is off)", "on|off",
                                    nusmv_core_check_sin_fun, true,
                                    false, NULL, NULL);

  NuSMVCore_add_command_line_option("-rin", "enables (on) or disables rbc"
                                    " inlining (default is on)", "on|off",
                                    nusmv_core_check_rbc_fun, true, false,
                                    NULL, NULL);

  NuSMVCore_add_command_line_option("-mono",
                                    "enables monolithic transition relation",
                                    NULL, nusmv_core_set_mono_partition, true,
                                    false, NULL, "-thresh -iwls95 -cp");

  NuSMVCore_add_command_line_option("-iwls95",
                                    "enables Iwls95 conjunctive "
                                    "partitioning and sets",
                                    "cp_t", nusmv_core_set_iwls95_partition,
                                    true, false, NULL, "-thresh -mono -cp");

  NuSMVCore_add_command_line_option("-thresh",
                                    "conjunctive partitioning with "
                                    "threshold of each partition set"
                                    " to \"cp_t\" (DEFAULT, with cp_t=1000)",
                                    "cp_t", nusmv_core_set_thresh_partition,
                                    true, false, NULL, "-iwls95 -mono -cp");

  NuSMVCore_add_command_line_option("-cp",
                                    "conjunctive partitioning with threshold"
                                    " of each partition set to \"cp_t\" "
                                    "(DEFAULT, with cp_t=1000). "
                                    "Use -thresh instead of this.",
                                    "cp_t", nusmv_core_set_thresh_partition,
                                    true, true, NULL, "-iwls95 -mono -thresh");

#if NUSMV_HAVE_CPP
  /* --------------------------------------------------------------- */
  /* CPP OPTION */
  NuSMVCore_add_command_line_option("-cpp",
                                    "runs preprocessor on SMV files before "
                                    "any specified with -pre. "
# if NUSMV_HAVE_CPP
#   if NUSMV_HAVE_GETENV
                                    "Environment variable 'CPP' can be used to "
                                    "specify a different preprocessor. "
#   endif
# else
                                    "Preprocessor was not found when NuSMV"
                                    " had been configured, then 'cpp' will"
                                    " be searched at runtime when needed"
#   if NUSMV_HAVE_GETENV
                                    ", or the 'CPP' environment variable "
                                    "will be used when defined by the user"
#   endif
                                    "."
# endif
                                    , NULL, nusmv_core_set_cpp,
                                    true, true, NULL, NULL);
  /* --------------------------------------------------------------- */
#endif

  NuSMVCore_add_env_command_line_option("-r",
                                        "enables printing of reachable states",
                                        NULL, PRINT_REACHABLE, true,
                                        false, NULL, NULL);

  NuSMVCore_add_command_line_option("-f",
                                    "computes the reachable states"
                                    " (forward search) (default)",
                                    NULL, core_data_set_fs, true, true,
                                    NULL, "-df");

  NuSMVCore_add_env_command_line_option("-df", "disables the computation of"
                                        " reachable states",
                                        NULL, FORWARD_SEARCH, true, false,
                                        NULL, "-f");

  NuSMVCore_add_command_line_option("-dp", "UNSUPPORTED", NULL,
                                    nusmv_core_set_dp, false, true, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-old",
                                        "keeps backward compatibility"
                                        " with older versions of NuSMV",
                                        NULL, BACKWARD_COMPATIBILITY, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-ctt",
                                        "enables checking for the totality"
                                        " of the transition relation",
                                        NULL, OPT_CHECK_FSM, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-lp",
                                        "lists all properties in SMV model",
                                        NULL, LIST_PROPERTIES, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-n",
                                        "specifies which property of SMV "
                                        "model should be checked", "idx",
                                        PROP_NO, true, false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-is", "does not check SPEC", NULL,
                                        IGNORE_SPEC, true, false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-ic", "does not check COMPUTE",
                                        NULL, IGNORE_COMPUTE, true, false,
                                        NULL, NULL);

  NuSMVCore_add_env_command_line_option("-ils", "does not check LTLSPEC",
                                        NULL, IGNORE_LTLSPEC, true, false,
                                        NULL, NULL);

  NuSMVCore_add_env_command_line_option("-ips", "does not check PSLSPEC",
                                        NULL, IGNORE_PSLSPEC, true, false,
                                        NULL, NULL);

  NuSMVCore_add_env_command_line_option("-ii", "does not check INVARSPEC", NULL,
                                        IGNORE_INVAR, true, false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-flt",
                                        "computes the reachable states"
                                        " also for the LTL Tableau",
                                        NULL, LTL_TABLEAU_FORWARD_SEARCH,
                                        true, false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-i",
                                        "reads order of variables "
                                        "from file \"iv_file\"",
                                        "iv_file", INPUT_ORDER_FILE, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-o",
                                        "prints order of variables"
                                        "to file \"ov_file\"",
                                        "ov_file", OUTPUT_ORDER_FILE, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-t",
                                        "reads order of vars for clustering "
                                        "from file \"tv_file\"",
                                        "tv_file", TRANS_ORDER_FILE, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-AG",
                                        "enables AG only search",
                                        NULL, AG_ONLY_SEARCH, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-reorder",
                                        "enables reordering of "
                                        "variables before exiting",
                                        NULL, ENABLE_REORDER, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-dynamic",
                                        "enables dynamic reordering "
                                        "of variables",
                                        NULL, DYNAMIC_REORDER, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-disable_sexp2bdd_caching",
                                        "disables caching of expressions"
                                        "evaluation to BDD",
                                        NULL, ENABLE_SEXP2BDD_CACHING,
                                        true, false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-bdd_soh",
                                        "sets the static variable ordering "
                                        "heuristics to \"heuristics\"",
                                        "heuristics",
                                        BDD_STATIC_ORDER_HEURISTICS, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-coi",
                                        "enables cone of influence reduction",
                                        NULL, CONE_OF_INFLUENCE, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-noaffinity",
                                        "disables affinity clustering",
                                        NULL, AFFINITY_CLUSTERING, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-iwl95preorder",
                                        "enables iwls95 preordering",
                                        NULL, IWLS95_PREORDER, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-ofm",
                                        "prints flattened model to file "
                                        "\"fn_file\"",
                                        "fn_file", OUTPUT_FLATTEN_MODEL_FILE,
                                        true, false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-disable_daggifier",
                                        "disables the SMV model "
                                        "dumper daggifier",
                                        NULL, DAGGIFIER_ENABLED,
                                        true, false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-obm",
                                        "prints boolean model to"
                                        " file \"bn_file\"",
                                        "bn_file", OUTPUT_BOOLEAN_MODEL_FILE,
                                        true, false, NULL, NULL);
#if NUSMV_HAVE_INCREMENTAL_SAT
  NuSMVCore_add_env_command_line_option("-bmc_length",
                                        "sets bmc_length variable, used by BMC",
                                        "k", BMC_PB_LENGTH, true,
                                        false, "-bmc", NULL);

  NuSMVCore_add_env_command_line_option("-bmc",
                                        "enables BMC instead of "
                                        "BDD model checking",
                                        NULL, BMC_MODE, true,
                                        false, NULL, NULL);
#endif /* HAVE_INCREMENTAL_SAT */

  NuSMVCore_add_env_command_line_option("-int",
                                        "enables interactive mode",
                                        NULL, BATCH, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-dcx",
                                        "disables computation of"
                                        " counter-examples",
                                        NULL, COUNTER_EXAMPLES, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-load",
                                        "executes NuSMV commands from file",
                                        "sc_file", SCRIPT_FILE, true,
                                        true, NULL, "-source");

  NuSMVCore_add_env_command_line_option("-source",
                                        "executes NuSMV commands from file",
                                        "sc_file", SCRIPT_FILE, true,
                                        false, NULL, "-load");

  NuSMVCore_add_env_command_line_option("-quiet",
                                        "Quiet mode",
                                        NULL, QUIET_MODE, false,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-v",
                                        "sets verbose level to \"vl\"",
                                        "vl", VERBOSE_LEVEL, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-disable_syntactic_checks",
                                        "Skips some correctness checks over "
                                        "the input model. Warning: when using "
                                        "this option, the input model MUST be "
                                        "correct, otherwise the tool may crash",
                                        NULL, DISABLE_SYNTACTIC_CHECKS, true,
                                        false, NULL, NULL);

  NuSMVCore_add_env_command_line_option("-keep_single_value_vars",
                                        "Does not convert variables that have "
                                        "only one single possible value into "
                                        "constant DEFINEs",
                                        NULL, KEEP_SINGLE_VALUE_VARS, true,
                                        false, NULL, NULL);

  FREE(libraryName);
}


/**Function********************************************************************

   Synopsis           [Executes the main program.]

   Description        [Executes the main program.]

   SideEffects        []

   SeeAlso            [NuSMVCore_init NuSMVCore_quit]

******************************************************************************/
boolean NuSMVCore_main(int argc, char ** argv, int* status)
{
  CoreData_ptr data = nusmv_core_get_instance();
  int quit_flag = 0;
  boolean requires_shutdown = true;

  nusmv_assert((int*)status != NULL);
  nusmv_assert(data->initialized);

  *status = RET_SUCCESS;

  /* Parse command line options */
  *status = nusmv_core_parse_line_options(argc, argv);

  /* We don't need the command line options anymore. we can free
     them. */
  nusmv_core_free_line_options(data);

  /* Everything went smooth with parsing the command line options */
  if ((*status) == RET_SUCCESS) {
    if (!opt_batch(data->opt_manager)) { /* interactive mode */
      /* initiliazes the commands to handle with options */
      init_options_cmd();

      if (!opt_get_quiet_mode(data->opt_manager)) {
        data->print_banner(nusmv_stdout);
      }

      if (!opt_ignore_init_file(data->opt_manager)) {
        CInit_NusmvrcSource();
      }

      /* There exists a script file */
      if ((char*)NULL != get_script_file(data->opt_manager)) {
        char* script_file = get_script_file(data->opt_manager);
        struct stat cmd_line;

        if (0 == stat(script_file, &cmd_line)) {
          char* command = ALLOC(char, strlen(script_file)
                                + strlen("source ") + 1);
          sprintf(command, "source %s", script_file);
          quit_flag = Cmd_CommandExecute(command);
          FREE(command);
        }
        else {
          fprintf(nusmv_stderr, "No such file or directory. Exiting...\n");
          quit_flag = -5; /* require immediate quit */
        }
      }

      while (quit_flag >= 0) {
        quit_flag = Cmd_CommandExecute("source -ip -");
      }

      *status = RET_SUCCESS;
    }
    else {
      if (!opt_get_quiet_mode(data->opt_manager)) {
        data->print_banner(nusmv_stdout);
      }

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
        fprintf(nusmv_stdout, "Starting the batch interaction.\n");
      }


      data->batch();
    }
  }

  /* Value of "quit_flag" is determined by the "quit" command */
  if (quit_flag == -1 || quit_flag == -2 || quit_flag == -4) {
    *status = RET_SUCCESS;
  }

  /* exits quickly and silently with quit_flag = -4 */
  if (quit_flag == -4) {
    requires_shutdown = false;
  }

  if (quit_flag == -3 || quit_flag == -5) {
    /* Script failed and on_failure_script_quits is set */
    /* Or the script file does not exist. */
    *status = RET_SCRIPT_ERROR;
  }

  return requires_shutdown;
}


/**Function********************************************************************

   Synopsis           [Shuts down the system]

   Description        [Shuts down the system. First all quit functions
                       in the list given to NuSMVCore_init are
                       called. Then all complex structures that have a
                       dependency among some internal packages are
                       deinitialized. After that, the Core is shut
                       down and finally all simple internal structures
                       are freed]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_reset_first_fun NuSMVCore_set_reset_last_fun]

******************************************************************************/
void NuSMVCore_quit()
{
  int i;
  CoreData_ptr data = nusmv_core_get_instance();

  for (i = (data->init_quit_funs_num - 1); i >= 0; --i) {
    data->init_quit_funs[i][1]();
  }

  /* Free line_options if not already done by NuSMVCore_main */
  nusmv_core_free_line_options(core_data);

  CInit_end();

  /* The structure may be useful until the very very
     end. nusmv_core_deinit is garanteed to use only simple
     independant structures */
  nusmv_core_deinit();
}


/**Function********************************************************************

   Synopsis           [Shuts down and restarts the system]

   Description        [Shuts down and restarts the system. 4 steps are done:
   1) Call the reset_first function (if any).
   2) Call the NuSMV package reset_first function
   3) Call the NuSMV package reset_last function
   4) Call the reset_last function (if any)
   ]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_reset_first_fun NuSMVCore_set_reset_last_fun]

******************************************************************************/
void NuSMVCore_reset()
{
  CoreData_ptr data = nusmv_core_get_instance();
  int i = 0;

  if (opt_verbose_level_gt(data->opt_manager, 1)) {
    fprintf(nusmv_stderr, "Shutting down the system...\n");
    inc_indent_size();
  }

  /* Quit all the registered libs */
  for (i = (data->init_quit_funs_num - 1); i >= 0; --i) {
    data->init_quit_funs[i][1]();
  }

  CInit_reset_first();

  if (opt_verbose_level_gt(data->opt_manager, 1)) {
    fprintf(nusmv_stderr, "Shutdown completed, rebooting the system...\n");
  }

  CInit_reset_last();

  for (i = 0; i < data->init_quit_funs_num; ++i) {
    data->init_quit_funs[i][0]();
  }

  if (opt_verbose_level_gt(data->opt_manager, 1)) {
    fprintf(nusmv_stderr, "The system is now up and ready\n");
    dec_indent_size();
  }

}


/**Function********************************************************************

   Synopsis           [Adds a command line option to the system.]

   Description        [Adds a command line option to the system. The command
   line option MUST have an environment option
   associated. When the command line option is
   specified, the environment option is
   automatically set to the correct value (In case
   of boolean options, the current value is
   negated, in any other case, the cmd option
   requires an argument which is set to the
   associated option

   Function arguments:
   1) name -> The name of the cmd line option (e.g. -int)
   2) usage -> The usage string that will be
   printed in the help (i.e. -help)
   3) parameter -> NULL if none, a string value if any.
   e.g: "k" for bmc_length
   4) env_var  -> The associated environment variable name
   5) public -> Tells whether the cmd line option
   is public or not. If not, the usage
   is not printed when invoking the
   tool with -h.
   6) deprecated -> Tells whether the cmd line
   options is deprecated or not
   7) dependency -> The possibly option name on which this
   one dependens on. NULL if none.
   e.g. -bmc_length depends on -bmc
   8) conflict -> The list of option names that
   conflict with this one.
   e.g. -mono conflicts with
   "-thresh -iwls95"
   ]

   SideEffects        []

   SeeAlso            [NuSMVCore_add_command_line_option]

******************************************************************************/
void NuSMVCore_add_env_command_line_option(char* name,
                                           char* usage,
                                           char* parameter,
                                           char* env_var,
                                           boolean public,
                                           boolean deprecated,
                                           char* dependency,
                                           char* conflict)
{
  CoreData_ptr data = nusmv_core_get_instance();
  CmdLineOpt_ptr opt = nusmv_core_init_opt();

  nusmv_assert((char*)NULL != name);
  opt->name = util_strsav(name);

  nusmv_assert((char*)NULL != usage);
  opt->usage = util_strsav(usage);

  if ((char*)NULL != parameter) {
    opt->parameter = util_strsav(parameter);
  }

  nusmv_assert((char*)NULL != env_var);
  opt->env_option = util_strsav(env_var);

  opt->deprecated = deprecated;
  opt->public = public;

  if ((char*)NULL != dependency) {
    /* We have initialized all data, we can use complex structures. */
    if (data->initialized) {
      opt->dependency = find_string(dependency);
    }
    opt->tmp_dependency = util_strsav(dependency);
  }

  if ((char*)NULL != conflict) {
    /* We have initialized all data, we can use complex structures. */
    if (data->initialized) {
      Olist_ptr split = nusmv_core_split(conflict);
      nusmv_core_olist_union(opt->conflicts, split);
      Olist_destroy(split);
    }

    opt->tmp_conflicts = util_strsav(conflict);
  }

  if (data->initialized) {
    nusmv_assert((hash_ptr)NULL != data->line_options);

    insert_assoc(data->line_options,
                 NODE_PTR(find_string(name)),
                 NODE_PTR(opt));
  }
  else {
    Olist_append(data->tmp_line_options, opt);
  }

}


/**Function********************************************************************

   Synopsis           [Adds a command line option to the system.]

   Description        [Adds a command line option to the system.

   When the command line option is specified, the
   check_and_apply function is called, which should
   first check that the (possible) parameter is
   valid, and then perform an action on it.

   Function arguments:
   1) name -> The name of the cmd line option (e.g. -int)
   2) usage -> The usage string that will be
   printed in the help (i.e. -help)
   3) parameter -> NULL if none, a string value if any.
   e.g: "k" for bmc_length
   4) check_and_apply -> The function that checks
   the (possible) parameter
   value and performs an
   action
   5) public -> Tells whether the cmd line option
   is public or not. If not, the usage
   is not printed when invoking the
   tool with -h.
   6) deprecated -> Tells whether the cmd line
   options is deprecated or not
   7) dependency -> The possibly option name on which this
   one dependens on. NULL if none.
   e.g. -bmc_length depends on -bmc
   8) conflict -> The list of option names that
   conflict with this one.
   e.g. -mono conflicts with
   "-thresh -iwls95"
   ]

   SideEffects        []

   SeeAlso            [NuSMVCore_add_env_command_line_option]

******************************************************************************/
void
NuSMVCore_add_command_line_option(char* name,
                                  char* usage,
                                  char* parameter,
                                  boolean (*check_and_apply)(OptsHandler_ptr, char*),
                                  boolean public,
                                  boolean deprecated,
                                  char* dependency,
                                  char* conflict)
{
  CoreData_ptr data = nusmv_core_get_instance();
  CmdLineOpt_ptr opt = nusmv_core_init_opt();

  nusmv_assert((char*)NULL != name);
  opt->name = util_strsav(name);

  nusmv_assert((char*)NULL != usage);
  opt->usage = util_strsav(usage);

  if ((char*)NULL != parameter) {
    opt->parameter = util_strsav(parameter);
  }

  nusmv_assert(NULL != check_and_apply);
  opt->check_and_apply = check_and_apply;

  opt->deprecated = deprecated;
  opt->public = public;

  if ((char*)NULL != dependency) {
    /* We have initialized all data, we can use complex structures. */
    if (data->initialized) {
      opt->dependency = find_string(dependency);
    }
    opt->tmp_dependency = util_strsav(dependency);
  }

  if ((char*)NULL != conflict) {
    /* We have initialized all data, we can use complex structures. */
    if (data->initialized) {
      Olist_ptr split = nusmv_core_split(conflict);
      nusmv_core_olist_union(opt->conflicts, split);
      Olist_destroy(split);
    }

    opt->tmp_conflicts = util_strsav(conflict);
  }

  if (data->initialized) {
    nusmv_assert((hash_ptr)NULL != data->line_options);

    insert_assoc(data->line_options,
                 NODE_PTR(find_string(name)),
                 NODE_PTR(opt));
  }
  else {
    Olist_append(data->tmp_line_options, opt);
  }
}


/**Function********************************************************************

   Synopsis           [The Sm tool_name field getter]

   Description        [The Sm tool_name field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_tool_name]

******************************************************************************/
char* NuSMVCore_get_tool_name()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->tool_name;
}


/**Function********************************************************************

   Synopsis           [The Sm tool rc file name field getter]

   Description        [The Sm tool rc file name field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_tool_name]

******************************************************************************/
char* NuSMVCore_get_tool_rc_file_name()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->tool_rcfile;
}


/**Function********************************************************************

   Synopsis           [The Sm tool_name field setter]

   Description        [The Sm tool_name field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_tool_name]

******************************************************************************/
void NuSMVCore_set_tool_name(char* tool_name)
{
  CoreData_ptr self = nusmv_core_get_instance();
  char* tmp = (char*)NULL;
  char* l_tool_name = (char*)NULL;

  if ((char*)NULL != self->tool_name) {
    FREE(self->tool_name);
  }
  if ((char*)NULL != self->tool_rcfile) {
    FREE(self->tool_rcfile);
  }
  self->tool_name = util_strsav(tool_name);

  l_tool_name = nusmv_core_tolower(tool_name);
  tmp = ALLOC(char, strlen(tool_name) + 4);
  sprintf(tmp, ".%src", l_tool_name);

  self->tool_rcfile = tmp;

  FREE(l_tool_name);
}


/**Function********************************************************************

   Synopsis           [The Sm tool_version field getter]

   Description        [The Sm tool_version field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_tool_version]

******************************************************************************/
char* NuSMVCore_get_tool_version()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->tool_version;
}


/**Function********************************************************************

   Synopsis           [The Sm tool_version field setter]

   Description        [The Sm tool_version field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_tool_version]

******************************************************************************/
void NuSMVCore_set_tool_version(char* tool_version)
{
  CoreData_ptr self = nusmv_core_get_instance();
  if ((char*)NULL != self->tool_version) {
    FREE(self->tool_version);
  }
  self->tool_version = util_strsav(tool_version);
}


/**Function********************************************************************

   Synopsis           [The Sm build_date field getter]

   Description        [The Sm build_date field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_build_date]

******************************************************************************/
char* NuSMVCore_get_build_date()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->build_date;
}


/**Function********************************************************************

   Synopsis           [The Sm build_date field setter]

   Description        [The Sm build_date field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_build_date]

******************************************************************************/
void NuSMVCore_set_build_date(char* build_date)
{
  CoreData_ptr self = nusmv_core_get_instance();
  if ((char*)NULL != self->build_date) {
    FREE(self->build_date);
  }
  self->build_date = util_strsav(build_date);
}


/**Function********************************************************************

   Synopsis           [The Sm prompt_string field getter]

   Description        [The Sm prompt_string field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_prompt_string]

******************************************************************************/
char* NuSMVCore_get_prompt_string()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->prompt_string;
}


/**Function********************************************************************

   Synopsis           [The Sm prompt_string field setter]

   Description        [The Sm prompt_string field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_prompt_string]

******************************************************************************/
void NuSMVCore_set_prompt_string(char* prompt_string)
{
  CoreData_ptr self = nusmv_core_get_instance();
  if ((char*)NULL != self->prompt_string) {
    FREE(self->prompt_string);
  }
  self->prompt_string = util_strsav(prompt_string);
}


/**Function********************************************************************

   Synopsis           [The Sm email field getter]

   Description        [The Sm email field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_email]

******************************************************************************/
char* NuSMVCore_get_email()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->email;
}


/**Function********************************************************************

   Synopsis           [The Sm email field setter]

   Description        [The Sm email field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_email]

******************************************************************************/
void NuSMVCore_set_email(char* email)
{
  CoreData_ptr self = nusmv_core_get_instance();
  if ((char*)NULL != self->email) {
    FREE(self->email);
  }
  self->email = util_strsav(email);
}


/**Function********************************************************************

   Synopsis           [The Sm website field getter]

   Description        [The Sm website field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_website]

******************************************************************************/
char* NuSMVCore_get_website()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->website;
}


/**Function********************************************************************

   Synopsis           [The Sm website field setter]

   Description        [The Sm website field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_website]

******************************************************************************/
void NuSMVCore_set_website(char* website)
{
  CoreData_ptr self = nusmv_core_get_instance();
  if ((char*)NULL != self->website) {
    FREE(self->website);
  }
  self->website = util_strsav(website);
}


/**Function********************************************************************

   Synopsis           [The Sm bug_report_message field getter]

   Description        [The Sm bug_report_message field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_bug_report_message]

******************************************************************************/
char* NuSMVCore_get_bug_report_message()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->bug_report_message;
}


/**Function********************************************************************

   Synopsis           [The Sm bug_report_message field setter]

   Description        [The Sm bug_report_message field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_bug_report_message]

******************************************************************************/
void NuSMVCore_set_bug_report_message(char* bug_report_message)
{
  CoreData_ptr self = nusmv_core_get_instance();
  if ((char*)NULL != self->bug_report_message) {
    FREE(self->bug_report_message);
  }
  self->bug_report_message = util_strsav(bug_report_message);
}


/**Function********************************************************************

   Synopsis           [The Sm linked_addons field getter]

   Description        [The Sm linked_addons field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_set_linked_addons]

******************************************************************************/
char* NuSMVCore_get_linked_addons()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->linked_addons;
}


/**Function********************************************************************

   Synopsis           [The Sm linked_addons field setter]

   Description        [The Sm linked_addons field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_linked_addons]

******************************************************************************/
void NuSMVCore_set_linked_addons(char* linked_addons)
{
  CoreData_ptr self = nusmv_core_get_instance();
  if ((char*)NULL != self->linked_addons) {
    FREE(self->linked_addons);
  }
  self->linked_addons = util_strsav(linked_addons);
}


/**Function********************************************************************

   Synopsis           [The Sm library_name field getter]

   Description        [The Sm library_name field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_tool_name]

******************************************************************************/
char* NuSMVCore_get_library_name()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->library_name;
}


/**Function********************************************************************

   Synopsis           [The Sm library_version field getter]

   Description        [The Sm library_version field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_tool_version]

******************************************************************************/
char* NuSMVCore_get_library_version()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->library_version;
}


/**Function********************************************************************

   Synopsis           [The Sm library_build_date field getter]

   Description        [The Sm library_build_date field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_build_date]

******************************************************************************/
char* NuSMVCore_get_library_build_date()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->library_build_date;
}


/**Function********************************************************************

   Synopsis           [The Sm library_email field getter]

   Description        [The Sm library_email field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_email]

******************************************************************************/
char* NuSMVCore_get_library_email()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->library_email;
}


/**Function********************************************************************

   Synopsis           [The Sm library_website field getter]

   Description        [The Sm library_website field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_website]

******************************************************************************/
char* NuSMVCore_get_library_website()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->library_website;
}


/**Function********************************************************************

   Synopsis           [The Sm library_bug_report_message field getter]

   Description        [The Sm library_bug_report_message field getter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_bug_report_message]

******************************************************************************/
char* NuSMVCore_get_library_bug_report_message()
{
  CoreData_ptr self = nusmv_core_get_instance();
  return self->library_bug_report_message;
}


/**Function********************************************************************

   Synopsis           [The Sm banner_print_fun field setter]

   Description        [The Sm banner_print_fun field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_banner_print_fun]

******************************************************************************/
void NuSMVCore_set_banner_print_fun(void (*banner_print_fun)(FILE *))
{
  CoreData_ptr self = nusmv_core_get_instance();
  self->print_banner = banner_print_fun;
}


/**Function********************************************************************

   Synopsis           [The Sm batch fun field setter]

   Description        [The Sm batch fun field setter]

   SideEffects        []

   SeeAlso            [NuSMVCore_get_batch_fun]

******************************************************************************/
void NuSMVCore_set_batch_fun(void (*batch_fun)(void))
{
  CoreData_ptr self = nusmv_core_get_instance();
  self->batch = batch_fun;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [Initializes (once) the internal core_data variable]

   Description        [Initializes (once) the internal core_data variable]

   SideEffects        []

   SeeAlso            [nusmv_core_deinit]

******************************************************************************/
static CoreData_ptr nusmv_core_get_instance()
{
  if ((CoreData_ptr)NULL == core_data) {
    core_data = ALLOC(CoreData, 1);

    nusmv_assert((CoreData_ptr)NULL != core_data);

    core_data->tool_name = (char*)NULL;
    core_data->tool_rcfile = (char*)NULL;
    core_data->tool_version = (char*)NULL;
    core_data->build_date = (char*)NULL;
    core_data->prompt_string = (char*)NULL;
    core_data->email = (char*)NULL;
    core_data->website = (char*)NULL;
    core_data->bug_report_message = (char*)NULL;
    core_data->linked_addons = (char*)NULL;

    core_data->library_name = (char*)NULL;
    core_data->library_version = (char*)NULL;
    core_data->library_build_date = (char*)NULL;
    core_data->library_email = (char*)NULL;
    core_data->library_website = (char*)NULL;
    core_data->library_bug_report_message = (char*)NULL;

    core_data->opt_manager = OPTS_HANDLER(NULL);

    core_data->print_banner = NULL;
    core_data->batch = NULL;

    core_data->init_quit_funs = NULL;
    core_data->init_quit_funs_num = 0;

    core_data->line_options = (hash_ptr)NULL;
    core_data->initialized = false;

    core_data->tmp_line_options = Olist_create();
  }

  return core_data;
}


/**Function********************************************************************

   Synopsis           [Deinitializes and frees the internal variable
   core_data]

   Description        [Deinitializes and frees the internal variable
   core_data. This function should be called
   only when terminating the program]

   SideEffects        []

   SeeAlso            [nusmv_core_get_instance]

******************************************************************************/
static void nusmv_core_deinit()
{
  if ((CoreData_ptr)NULL != core_data) {
    int i;
    Olist_destroy(core_data->tmp_line_options);

    if ((char*)NULL != core_data->tool_name) {
      FREE(core_data->tool_name);
    }
    if ((char*)NULL != core_data->tool_rcfile) {
      FREE(core_data->tool_rcfile);
    }
    if ((char*)NULL != core_data->tool_version) {
      FREE(core_data->tool_version);
    }
    if ((char*)NULL != core_data->build_date) {
      FREE(core_data->build_date);
    }
    if ((char*)NULL != core_data->prompt_string) {
      FREE(core_data->prompt_string);
    }
    if ((char*)NULL != core_data->email) {
      FREE(core_data->email);
    }
    if ((char*)NULL != core_data->website) {
      FREE(core_data->website);
    }
    if ((char*)NULL != core_data->bug_report_message) {
      FREE(core_data->bug_report_message);
    }
    if ((char*)NULL != core_data->linked_addons) {
      FREE(core_data->linked_addons);
    }

    if ((char*)NULL != core_data->library_name) {
      FREE(core_data->library_name);
    }
    if ((char*)NULL != core_data->library_version) {
      FREE(core_data->library_version);
    }
    if ((char*)NULL != core_data->library_build_date) {
      FREE(core_data->library_build_date);
    }
    if ((char*)NULL != core_data->library_email) {
      FREE(core_data->library_email);
    }
    if ((char*)NULL != core_data->library_bug_report_message) {
      FREE(core_data->library_bug_report_message);
    }
    if ((char*)NULL != core_data->library_website) {
      FREE(core_data->library_website);
    }

    if (core_data->init_quit_funs != NULL) {
      for (i = 0; i < core_data->init_quit_funs_num; ++i) {
        FREE(core_data->init_quit_funs[i]);
      }
      FREE(core_data->init_quit_funs);
    }

    FREE(core_data);
  }
}


/**Function********************************************************************

   Synopsis           [Initializes the internal representation structure
   of a command line option]

   Description        [Initializes the internal representation structure
   of a command line option]

   SideEffects        []

   SeeAlso            [nusmv_core_deinit_opt]

******************************************************************************/
static CmdLineOpt_ptr nusmv_core_init_opt()
{
  CmdLineOpt_ptr opt = ALLOC(CmdLineOpt, 1);

  opt->name = (char*)NULL;
  opt->usage = (char*)NULL;
  opt->parameter = (char*)NULL;

  opt->check_and_apply = NULL;
  opt->env_option = (char*)NULL;

  opt->deprecated = false;
  opt->public = false;

  opt->dependency = (string_ptr)NULL;
  opt->conflicts = Olist_create();

  opt->tmp_dependency = (char*)NULL;
  opt->tmp_conflicts = (char*)NULL;

  return opt;
}


/**Function********************************************************************

   Synopsis           [Deinitializes the internal representation structure
   of a command line option]

   Description        [Deinitializes and frees the internal representation
   structure of a command line option]

   SideEffects        []

   SeeAlso            [nusmv_core_init_opt]

******************************************************************************/
static void nusmv_core_deinit_opt(CmdLineOpt_ptr opt)
{
  nusmv_assert((CmdLineOpt_ptr)NULL != opt);

  nusmv_assert((char*)NULL != opt->name);
  FREE(opt->name);
  nusmv_assert((char*)NULL != opt->usage);
  FREE(opt->usage);

  if ((char*)NULL != opt->parameter) { FREE(opt->parameter); }
  if ((char*)NULL != opt->env_option) { FREE(opt->env_option); }
  if ((char*)NULL != opt->tmp_conflicts) { FREE(opt->tmp_conflicts); }
  if ((char*)NULL != opt->tmp_dependency) { FREE(opt->tmp_dependency); }

  Olist_destroy(opt->conflicts);

  FREE(opt);
}


/**Function********************************************************************

   Synopsis           [Prints the command line option usages]

   Description        [Prints the command line option usages.
   If print_banner is true, also the banner is
   printed out. Printed command line options are
   taken from the list of registered ones.]

   SideEffects        []

   SeeAlso            [NuSMVCore_add_command_line_option
   NuSMVCore_add_env_command_line_option]

******************************************************************************/
static void nusmv_core_print_usage(boolean print_banner)
{
  CoreData_ptr data = nusmv_core_get_instance();

  /* For lexical ordered printing */
  avl_tree* avl = avl_init_table(Utils_strcasecmp);
  avl_generator* gen;
  char* option_name;
  char* option_struct_ptr;

  string_ptr key;
  CmdLineOpt_ptr opt;
  assoc_iter iter;

  /* Prepare the avl tree.. */
  ASSOC_FOREACH(data->line_options, iter, &key, &opt) {
    nusmv_assert((CmdLineOpt_ptr)NULL != opt);

    /* We print the usage only if the option should be available to
       the user */
    if (opt->public) {
      avl_insert(avl, get_text(key), (char*)opt);
    }
  }

  /* Print the banner, if necessary */
  if (!opt_get_quiet_mode(data->opt_manager) && print_banner) {
    data->print_banner(nusmv_stderr);
  }

  fprintf(nusmv_stderr, "Usage:  %s [-h | -help] [options]* [input_file]\n",
          data->tool_name);

  gen = avl_init_gen(avl, AVL_FORWARD);

  fprintf(nusmv_stderr, "   -h | -help\n");
  nusmv_core_print_string(nusmv_stderr, "prints out current message", 6);

  /* Now print all options.. */
  while (avl_gen(gen, &option_name, &option_struct_ptr)) {
    CmdLineOpt_ptr option_struct = (CmdLineOpt_ptr)option_struct_ptr;
    char* tmp;

    /* ---------- OPTION NAME + PARAMETER -------------- */
    if ((char*)NULL != option_struct->parameter) {
      tmp = ALLOC(char, strlen(option_name) +
                  strlen(option_struct->parameter) + 2);
      sprintf(tmp, "%s %s", option_name, option_struct->parameter);
    }
    else {
      tmp = ALLOC(char, strlen(option_name) + 1);
      sprintf(tmp, "%s", option_name);
    }
    nusmv_core_print_string(nusmv_stderr, tmp, 3);

    /* ---------- OPTION USAGE ------------------------- */
    nusmv_core_print_string(nusmv_stderr, option_struct->usage, 6);

    /* ---------- OPTION DEPENDENCIES ------------------ */
    if ((string_ptr)NULL != option_struct->dependency) {
      const char* fmt = "NOTE: Requires option \"%s\"";
      tmp = REALLOC(char, tmp, strlen(fmt) +
                    strlen(get_text(option_struct->dependency)) + 1);

      sprintf(tmp, fmt, get_text(option_struct->dependency));
      nusmv_core_print_string(nusmv_stderr, tmp, 6);
    }

    /* ---------- OPTION CONFICTS ---------------------- */
    if (!Olist_is_empty(option_struct->conflicts)) {
      const char* fmt = "NOTE: Incompatible with option%s %s";
      char* conf = nusmv_core_merge(option_struct->conflicts);
      tmp = REALLOC(char, tmp, strlen(fmt) + strlen(conf) + 2);

      sprintf(tmp, fmt,
              (Olist_get_size(option_struct->conflicts) > 1 ? "s" : ""),
              conf);

      nusmv_core_print_string(nusmv_stderr, tmp, 6);
      FREE(conf);
    }

    /* ---------- OPTION IS DEPRECATED ----------------- */
    if (option_struct->deprecated) {
      const char* fmt = "WARNING: option \"%s\" is deprecated";
      tmp = REALLOC(char, tmp, strlen(fmt) + strlen(option_name) + 1);

      sprintf(tmp, fmt, option_name);
      nusmv_core_print_string(nusmv_stderr, tmp, 6);
    }

    FREE(tmp);
  }

  fprintf(nusmv_stderr, "   input-file\n");
  nusmv_core_print_string(nusmv_stderr,
                          "the file both the model and "
                          "the spec were read from", 6);

  avl_free_gen(gen);
  avl_free_table(avl, 0, 0);
}


/**Function********************************************************************

   Synopsis           [Prints the given string on the given stream, padded by
   the given number.]

   Description        [Prints the given string on the given stream, padded by
   the given number. If the given string is longer
   than MAX_PRINT_WIDTH, then a new-line is added
   and the remaining part of the string is prinded
   (padded as the previous one). The string is
   divided in such way that no words are
   truncated.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void nusmv_core_print_string(FILE* out, char* str, int padding)
{
  int i = 0;
  int j = padding;

  /* Lines cannot be more than MAX_PRINT_WIDTH characters long */
  char buff[MAX_PRINT_WIDTH + 1];

  for (i = 0; i < padding; ++i) {
    buff[i] = ' ';
  }

  for (i = 0; '\0' != str[i]; ++i) {

    if (str[i] == '\n' || j >= MAX_PRINT_WIDTH) {
      /* It is a letter! It may mean that the word will be
         cutted.. */
      if (str[i] != '\n' || str[i] != ' ') {
        int k = 0, z = 0;
        int stolen = 0;
        char tmp[MAX_PRINT_WIDTH + 1] = "";

        /* Find where the last word ends.. */
        while (str[i - z] != ' ' && str[i - z] != '\n') { ++z; }

        /* Same the partial word, should be printed later.. */
        for (k = 0; k < z; k++) { tmp[k] = str[(i - z) + k]; }

        /* Remove the last partial word to the buffer... */
        buff[j - z] = '\0';
        fprintf(out, "%s\n", buff);

        /* ... add it to the next buffer! */
        for (k = 0, j = padding; j < (padding + z); ++k) {
          if (tmp[k] != ' ' && tmp[k] != '\n') {
            buff[j++] = tmp[k];
          }
          else {
            stolen++;
          }
        }
        j -= stolen;
      }
      else {
        buff[j] = '\0';
        fprintf(out, "%s\n", buff);
        j = padding;
      }
    }

    /* In this way, we aviod duplicated new lines, and white spaces at
       the beginning or at the end of a string. */
    if (str[i] != '\n' &&
        !(str[i] == ' ' && (j == padding || j >= MAX_PRINT_WIDTH))) {
      buff[j++] = str[i];
    }

  }

  buff[j] = '\0';
  fprintf(out, "%s\n", buff);

}


/**Function********************************************************************

   Synopsis           [Parses the given command line options.]

   Description        [Parses the given command line options.
   -h, -help and input-file are hardcoded options,
   all other options should be registered using
   NuSMVCore_add_env_command_line_option or
   NuSMVCore_add_command_line_option]

   SideEffects        []

   SeeAlso            [NuSMVCore_add_env_command_line_option
   NuSMVCore_add_command_line_option]

******************************************************************************/
static int nusmv_core_parse_line_options(int argc, char ** argv)
{
  int status = RET_SUCCESS;
  CoreData_ptr data = nusmv_core_get_instance();
  OptsHandler_ptr options = data->opt_manager;

  hash_ptr required = new_assoc();
  hash_ptr conflicting = new_assoc();
  Olist_ptr line_options = Olist_create();

  /* parses the Program Name */
  argc--;
  set_pgm_path(options, *(argv++));

  while (argc > 0) {
    /* -help is hardcoded */
    if (strcmp(*argv, "-help") == 0 ||
        strcmp(*argv, "-h") == 0) {
      argv++; argc--;
      nusmv_core_print_usage(true);
      status = RET_HELP_PRINT;
      break;
    }
    /* The input file (last argument) is hardcoded */
    else if (argc == 1 && (**argv) != '-'){
      set_input_file(options, *(argv++));
      argc--;
    }
    else {
      /* Lookup for the cmd line option. */
      char* opt_string = *argv;
      string_ptr opt_name = find_string(opt_string);
      CmdLineOpt_ptr opt = (CmdLineOpt_ptr)find_assoc(data->line_options,
                                                      NODE_PTR(opt_name));

      /* Skip already given command options. */
      if (Olist_contains(line_options, opt_name)) {
        fprintf(nusmv_stderr,
                "Warning: Option \"%s\" given more than once.\n"
                "Warning: Only the first occurrence will be taken into account\n",
                opt_string);
      }
      else {

        /* Add the cmd line option to the set of all defined cmd options. */
        Olist_append(line_options, opt_name);

        /* The cmd line option exists! */
        if ((CmdLineOpt_ptr)NULL != opt) {
          char* param = (char*)NULL;

          /* Add to the required table the dependencies of this option */
          insert_assoc(required, NODE_PTR(opt_name), NODE_PTR(opt->dependency));

          /* Add to the conflicting table the options conflicting with the
             current one */
          insert_assoc(conflicting, NODE_PTR(opt_name), NODE_PTR(opt->conflicts));

          /* A parameter is requested! */
          if ((char*)NULL != opt->parameter) {
            if (argc < 2) {
              fprintf(nusmv_stderr,
                      "The \"%s\" command line option requires an argument.\n",
                      opt_string);
              status = RET_MISSING_OPTION_PARAM;
              break;
            }
            argc--; argv++;
            param = *argv;
          }

          /* The cmd line option has an env option associated. */
          if ((char*)NULL != opt->env_option) {

            /* The cmd line option should not have both an env var and a
               check&apply fun.*/
            nusmv_assert(NULL == opt->check_and_apply);

            /* The env var has to exist.*/
            nusmv_assert(OptsHandler_is_option_registered(data->opt_manager,
                                                          opt->env_option));

            /* If the option is boolean, invert the value. */
            if (OptsHandler_is_bool_option(data->opt_manager, opt->env_option)) {
              nusmv_assert((char*)NULL == param);
              boolean res;
              boolean curr =
                OptsHandler_get_bool_option_default_value(data->opt_manager,
                                                          opt->env_option);

              res = OptsHandler_set_bool_option_value(data->opt_manager,
                                                      opt->env_option,
                                                      !curr);
              if (!res) {
                fprintf(nusmv_stderr,
                        "An error occurred with option \"%s\"\n", opt_string);
                status = RET_INVALID_OPTION;
              }
            }
            else {
              boolean res;

              nusmv_assert((char*)NULL != param);

              res = OptsHandler_set_option_value(data->opt_manager,
                                                 opt->env_option, param);
              if (!res) {
                fprintf(nusmv_stderr,
                        "Cannot set value \"%s\" to option \"%s\"\n",
                        param, opt_string);
                status = RET_INVALID_OPTION_PARAM;
                break;
              }
            }
          }
          /* The cmd line option has it's own check & apply function */
          else {
            boolean res;

            nusmv_assert(NULL == opt->env_option);

            res = opt->check_and_apply(data->opt_manager, param);

            if (!res) {
              if ((char*)NULL != param) {
                fprintf(nusmv_stderr,
                        "Cannot set value \"%s\" to option \"%s\"\n",
                        param, opt_string);
                status = RET_INVALID_OPTION_PARAM;
              }
              else {
                fprintf(nusmv_stderr, "Cannot use option \"%s\"\n", opt_string);
                status = RET_INVALID_OPTION;
              }
              break;
            }
          }

          if (opt->deprecated) {
            fprintf(nusmv_stderr, "Warning: %s is deprecated\n", opt_string);
          }
        }
        else { /* The option does not exist */
          fprintf(nusmv_stderr,
                  "The command line option \"%s\" is unknown\n",
                  opt_string);
          status = RET_UNKNOWN_OPTION;
          break;
        }
      } /* Command line option not already given */

      argc--; argv++;
    } /* Non-existing or not hardcoded option */
  }

  /* Lookup for conflicts and unsatisfied dependencies */
  if (status == RET_SUCCESS) {
    Oiter iter;
    Olist_ptr printed = Olist_create();

    OLIST_FOREACH(line_options, iter) {
      string_ptr option =
        (string_ptr)Oiter_element(iter);

      Olist_ptr conflicts =
        (Olist_ptr)find_assoc(conflicting, NODE_PTR(option));

      string_ptr dependency =
        (string_ptr)find_assoc(required, NODE_PTR(option));

      /* Found conflicts which have not been already printed! */
      if (!Olist_is_empty(conflicts) &&
          !Olist_contains(printed, option)) {
        Olist_ptr intersect =
          nusmv_core_olist_intersection(conflicts, line_options);

        nusmv_core_olist_union(printed, intersect);

        if (!Olist_is_empty(intersect)) {
          const char* fmt = "Option %s cannot be used with option%s %s";
          char* conf = nusmv_core_merge(intersect);
          char* tmp;

          tmp = ALLOC(char, strlen(conf) + strlen(get_text(option)) +
                      strlen(fmt) + 1 + (Olist_get_size(intersect) > 1));

          sprintf(tmp, fmt, get_text(option),
                  (Olist_get_size(intersect) > 1 ? "s" : ""), conf);
          FREE(conf);

          nusmv_core_print_string(nusmv_stderr, tmp, 0);
          status = RET_CONFLICT_OPTION;
          FREE(tmp);
        }

        Olist_destroy(intersect);
      }

      /* Unmet dependencies */
      if (((string_ptr)NULL != dependency) &&
          (!Olist_contains(line_options, dependency))) {

        fprintf(nusmv_stderr,
                "Option \"%s\" requires option \"%s\"\n",
                get_text(option), get_text(dependency));

        status = RET_MISSING_OPTION_DEP;
      }
    }

    Olist_destroy(printed);
  }

  free_assoc(required);
  free_assoc(conflicting);
  Olist_destroy(line_options);

  return status;
}


/**Function********************************************************************

   Synopsis           [Given a set of unique strings, returns a string
   representing the set of strings, separated by a
   white space]

   Description        [Given a set of unique strings, returns a string
   representing the set of strings, separated by a
   white space]

   SideEffects        []

   SeeAlso            [nusmv_core_split]

******************************************************************************/
static char* nusmv_core_merge(Olist_ptr set)
{
  char* result = ALLOC(char, 1);
  Oiter iter;

  result[0] = '\0';

  OLIST_FOREACH(set, iter) {
    string_ptr conf = (string_ptr)Oiter_element(iter);
    char* str = get_text(conf);
    char* tmp;

    tmp = ALLOC(char, strlen(result) + 1);
    sprintf(tmp, "%s", result);

    result = REALLOC(char, result, strlen(result) + strlen(str) + 2);
    sprintf(result, "%s%s ", tmp, str);
  }

  return result;
}


/**Function********************************************************************

   Synopsis           [Aux function for the nusmv_core_split function]

   Description        [Aux function for the nusmv_core_split function]

   SideEffects        []

   SeeAlso            [nusmv_core_split]

******************************************************************************/
static int nusmv_core_get_next_word_length(char* string)
{
  char* pos;
  nusmv_assert((char*)NULL != string);

  /* Skip prefix white spaces */
  while (string[0] == ' ') { string++; }

  pos = strchr(string, ' ');

  /* No spaces, but a word exists. */
  if ((char*)pos == NULL) {
    return strlen(string);
  }

  return (pos - string);
}


/**Function********************************************************************

   Synopsis           [Given a string of white-space separated strings,
   splits the string and builds a set of unique strings]

   Description        [Given a string of white-space separated strings,
   splits the string and builds a set of unique strings]

   SideEffects        []

   SeeAlso            [nusmv_core_merge]

******************************************************************************/
static Olist_ptr nusmv_core_split(char* string)
{
  Olist_ptr result = Olist_create();
  char* tmp = (char*)NULL;
  int i = 0;
  int j = 0;
  int next_length = nusmv_core_get_next_word_length(string);

  /* There is at least one word */
  if (next_length > 0) {
    /* Allocate the first memory for the first word */
    tmp = ALLOC(char, next_length + 1);

    /* Until the string is empty */
    for (i = 0; '\0' != string[i]; ++i) {
      if (string[i] == ' ') {
        /* Skip consecutives spaces */
        if (j > 0) {
          /* Found a word, put the terminal and push the ustring in
             the list */
          tmp[j] = '\0';
          Olist_append(result, find_string(tmp));

          /* Reset the index and re-allocate memory for the next
             word */
          j = 0;
          next_length = nusmv_core_get_next_word_length(string);

          /* No new words left */
          if (next_length <= 0) { break; }
          tmp = REALLOC(char, tmp, next_length + 1);
        }
      }
      else {
        tmp[j++] = string[i];
      }
    }

    if (j > 0) {
      tmp[j] = '\0';
      Olist_append(result, find_string(tmp));
    }

    FREE(tmp);
  }

  return result;
}


/**Function********************************************************************

   Synopsis           [Lowercases a string]

   Description        [Lowercases a string]

   SideEffects        []

   SeeAlso            [nusmv_core_merge]

******************************************************************************/
static char* nusmv_core_tolower(char* str)
{
  char* ret = ALLOC(char, strlen(str) + 1);
  int i;

  *ret = '\0';
  for (i = 0; '\0' != str[i]; ++i) {
    ret[i] = (isupper(str[i]) ? tolower(str[i]) : str[i]);
  }
  ret[i] = '\0';
  return ret;
}


/**Function********************************************************************

   Synopsis           [Check and apply function for the -sin cmd line opt]

   Description        [Check and apply function for the -sin cmd line opt]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean nusmv_core_check_sin_fun(OptsHandler_ptr opt, char* val)
{
  if (strcmp(val, "off") == 0) {
    return OptsHandler_set_bool_option_value(opt, SYMB_INLINING, false);
  }
  else if (strcmp(val, "on") == 0) {
    return OptsHandler_set_bool_option_value(opt, SYMB_INLINING, true);
  }

  return false;
}


/**Function********************************************************************

   Synopsis           [Check and apply function for the -rbc cmd line opt]

   Description        [Check and apply function for the -rbc cmd line opt]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean nusmv_core_check_rbc_fun(OptsHandler_ptr opt, char* val)
{
  if (strcmp(val, "off") == 0) {
    return OptsHandler_set_bool_option_value(opt, RBC_INLINING, false);
  }
  else if (strcmp(val, "on") == 0) {
    return OptsHandler_set_bool_option_value(opt, RBC_INLINING, true);
  }

  return false;
}


/**Function********************************************************************

   Synopsis           [Check and apply function for the -mono cmd line opt]

   Description        [Check and apply function for the -mono cmd line opt]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean nusmv_core_set_mono_partition(OptsHandler_ptr opt, char* val)
{
  return OptsHandler_set_enum_option_value(opt, PARTITION_METHOD,
                                           TRANS_TYPE_MONOLITHIC_STRING);
}


/**Function********************************************************************

   Synopsis           [Check and apply function for the -iwls95 cmd line opt]

   Description        [Check and apply function for the -iwls95 cmd line opt]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean nusmv_core_set_iwls95_partition(OptsHandler_ptr opt, char* val)
{
  boolean res = OptsHandler_set_enum_option_value(opt, PARTITION_METHOD,
                                                  TRANS_TYPE_IWLS95_STRING);

  res &= OptsHandler_set_option_value(opt, IMAGE_CLUSTER_SIZE, val);

  return res;
}


/**Function********************************************************************

   Synopsis           [Check and apply function for the -thresh cmd line opt]

   Description        [Check and apply function for the -thresh cmd line opt]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean nusmv_core_set_thresh_partition(OptsHandler_ptr opt, char* val)
{
  boolean res = OptsHandler_set_enum_option_value(opt, PARTITION_METHOD,
                                                  TRANS_TYPE_THRESHOLD_STRING);

  res &= OptsHandler_set_option_value(opt, CONJ_PART_THRESHOLD, val);

  return res;
}


/**Function********************************************************************

   Synopsis           [Check and apply function for the -cpp cmd line opt]

   Description        [Check and apply function for the -cpp cmd line opt]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean nusmv_core_set_cpp(OptsHandler_ptr opt, char* val)
{
  char* pp_list = OptsHandler_get_string_option_value(opt, PP_LIST);
  if (strcmp(pp_list, "") == 0) {
    set_pp_list(opt, "cpp");
  }
  else {
    char* new_pp_list;
    new_pp_list = ALLOC(char, strlen(pp_list) + 5);
    strcpy(new_pp_list, "cpp ");
    strcat(new_pp_list, pp_list);
    set_pp_list(opt, new_pp_list);
    FREE(new_pp_list);
  }

  return true;
}


/**Function********************************************************************

   Synopsis           [Check and apply function for the -pre cmd line opt]

   Description        [Check and apply function for the -pre cmd line opt]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean nusmv_core_set_pre(OptsHandler_ptr opt, char* val)
{
  char* pp_list = OptsHandler_get_string_option_value(opt, PP_LIST);
  char* new_value;
  boolean result;

  if (strcmp(pp_list, "") == 0) {
    new_value = util_strsav(val);
  }
  else {
    new_value = ALLOC(char, strlen(pp_list) + strlen(val) + 2);
    sprintf(new_value, "%s %s", val, pp_list);
  }

  result = OptsHandler_set_option_value(opt, PP_LIST, new_value);

  FREE(new_value);
  return result;
}


/**Function********************************************************************

   Synopsis           [Check and apply function for the -dp cmd line opt]

   Description        [Check and apply function for the -dp cmd line opt]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean nusmv_core_set_dp(OptsHandler_ptr opt, char* val)
{
  fprintf(nusmv_stderr,
          "WARNING: Disjunctive partitioning is no longer supported.\n");
  return false;
}


/**Function********************************************************************

   Synopsis           [Check and apply function for the -f cmd line opt]

   Description        [Check and apply function for the -f cmd line opt]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean
core_data_set_fs(OptsHandler_ptr opt, char* val)
{
  return true;
}


/**Function********************************************************************

   Synopsis           [Calculates the intersection list between a and b]

   Description        [Calculates the intersection list between a and b.
                       The returned list must be freed by the caller]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static Olist_ptr nusmv_core_olist_intersection(Olist_ptr a, Olist_ptr b)
{
  Olist_ptr res = Olist_create();
  Oiter iter;

  OLIST_FOREACH(a, iter) {
    void* el = Oiter_element(iter);
    if (Olist_contains(b, el)) {
      Olist_append(res, el);
    }
  }

  return res;
}


/**Function********************************************************************

   Synopsis           [Adds all elements in b to a, if a does
                       not contain it already]

   Description        [Adds all elements in b to a, if a does
                       not contain it already]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void nusmv_core_olist_union(Olist_ptr a, Olist_ptr b)
{
  Oiter iter;

  OLIST_FOREACH(b, iter) {
    void* el = Oiter_element(iter);
    if (!Olist_contains(a, el)) {
      Olist_append(a, el);
    }
  }

}


/**Function********************************************************************

   Synopsis           [Frees the line_options hash and all it's contents]

   Description        [Frees the line_options hash and all it's contents]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void nusmv_core_free_line_options(CoreData_ptr core_data)
{
  /* We need to destroy complex structure BEFORE quitting all
     packages. */

  if ((hash_ptr)NULL != core_data->line_options) {
    string_ptr key;
    CmdLineOpt_ptr opt;
    assoc_iter iter;

    ASSOC_FOREACH(core_data->line_options, iter, &key, &opt) {
      nusmv_core_deinit_opt(opt);
    }
    free_assoc(core_data->line_options);
    core_data->line_options = (hash_ptr)NULL;
  }

}
