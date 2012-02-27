/**CFile***********************************************************************

   FileName    [optCmd.c]

   PackageName [opt]

   Synopsis    [The option command file.]

   Description [optional]

   Author      [Marco Roveri]

   Copyright   [
   This file is part of the ``opt'' package of NuSMV version 2.
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

#include "opt/optInt.h"
#include "bmc/bmcUtils.h"
#include "trans/trans.h"  /* to access TransType interface */
#include "trace/pkg_trace.h"  /* to access TransType interface */
#include "fsm/bdd/bdd.h" /* for BddOregJusticeEmptinessBddAlgorithmType */
#include "opt/OptsHandler.h"
#include "trans/transInt.h"
#include "enc/enc.h"


/* Adds a command which generates tests for the options handler */
#define TEST_OPTS_HANDLER 0

int CommandSetVariable ARGS((int  argc, char** argv));
int CommandUnsetVariable ARGS((int  argc, char** argv));

#if TEST_OPTS_HANDLER
int CommandGenTestOptsHandler ARGS((int  argc, char** argv));
#endif

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macros                                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static char* remove_non_existant_pps ARGS((const char* pp_list));

static const char*
opt_check_invar_strategy_to_string ARGS((Check_Strategy str));
static const char*
opt_check_invar_fb_heuristic_to_string ARGS((FB_Heuristic h));
static const char*
opt_check_invar_bddbmc_heuristic_to_string ARGS((Bdd2bmc_Heuristic h));

/* Options check functions */
static boolean opt_check_sat_solver ARGS((OptsHandler_ptr opts,
                                          const char* val));
static void* opt_get_sat_solver ARGS((OptsHandler_ptr opts,
                                      const char* val));

static boolean opt_check_shown_states ARGS((OptsHandler_ptr opts,
                                            const char* val));
static boolean opt_check_word_format ARGS((OptsHandler_ptr opts,
                                           const char* val));
/* Generic getter functions */
static void* opt_get_integer ARGS((OptsHandler_ptr opts, const char* val));

/* Triggers */
static boolean opt_set_reachable_states_trigger ARGS((OptsHandler_ptr opts,
                                                      const char* opt,
                                                      const char* value,
                                                      Trigger_Action action));

static boolean opt_reorder_method_trigger ARGS((OptsHandler_ptr opts,
                                                const char* opt,
                                                const char* value,
                                                Trigger_Action action));

static boolean opt_trace_plugin_trigger ARGS((OptsHandler_ptr opts,
                                              const char* opt,
                                              const char* value,
                                              Trigger_Action action));

static boolean opt_dynamic_reorder_trigger ARGS((OptsHandler_ptr opts,
                                                 const char* opt,
                                                 const char* value,
                                                 Trigger_Action action));

static boolean opt_trans_order_file_trigger ARGS((OptsHandler_ptr opts,
                                                  const char* opt,
                                                  const char* value,
                                                  Trigger_Action action));

static boolean opt_run_cpp_trigger ARGS((OptsHandler_ptr opts,
                                         const char* opt,
                                         const char* value,
                                         Trigger_Action action));

static boolean opt_pp_list_trigger ARGS((OptsHandler_ptr opts,
                                         const char* opt,
                                         const char* value,
                                         Trigger_Action action));

static boolean opt_input_file_trigger ARGS((OptsHandler_ptr opts,
                                            const char* name,
                                            const char* value,
                                            Trigger_Action action));

static boolean opt_rbc_inlining_lazy_trigger ARGS((OptsHandler_ptr opts,
                                                   const char* opt,
                                                   const char* value,
                                                   Trigger_Action action));

static boolean opt_script_file_trigger ARGS((OptsHandler_ptr opts,
                                             const char* opt,
                                             const char* value,
                                             Trigger_Action action));

#if NUSMV_HAVE_REGEX_H
static boolean
opt_traces_regexp_trigger ARGS((OptsHandler_ptr opts, const char* opt,
                                const char* value, Trigger_Action action));
#endif
/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [Initialize the NuSMV options.]

   Description        [The NuSMV options are initialized. A pointer to
   a structure containing the NuSMV options is allocated, its fields
   are initialized and the pointer is returned.]

   SideEffects        []

******************************************************************************/
void init_options()
{
  OptsHandler_ptr opts = OptsHandler_get_instance();
  boolean res = true;
  char* path;
  char* lib_name;

  lib_name = CInit_NuSMVObtainLibrary();
  path = ALLOC(char, strlen(lib_name) + 3);
  sprintf(path, ".:%s", lib_name);

  OptsHandler_register_generic_option(OptsHandler_get_instance(),
                                      "open_path", path, true);
  FREE(lib_name);
  FREE(path);

  res = OptsHandler_register_generic_option(opts, PROGRAM_NAME,
                                            DEFAULT_PGM_NAME, false);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, PROGRAM_PATH,
                                            DEFAULT_PGM_PATH, false);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, INPUT_FILE,
                                            DEFAULT_INPUT_FILE,
                                            true);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, SCRIPT_FILE,
                                            NULL, false);
  nusmv_assert(res);

  res = OptsHandler_add_option_trigger(opts, SCRIPT_FILE,
                                       opt_script_file_trigger);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, QUIET_MODE,
                                         false, false);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, INPUT_ORDER_FILE,
                                            DEFAULT_INPUT_ORDER_FILE, true);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, OUTPUT_ORDER_FILE,
                                            DEFAULT_OUTPUT_ORDER_FILE, true);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, TRANS_ORDER_FILE,
                                            DEFAULT_TRANS_ORDER_FILE, true);
  nusmv_assert(res);

  res = OptsHandler_add_option_trigger(opts, TRANS_ORDER_FILE,
                                       opt_trans_order_file_trigger);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, OUTPUT_FLATTEN_MODEL_FILE,
                                            (char*)NULL, true);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, OUTPUT_BOOLEAN_MODEL_FILE,
                                            (char*)NULL, true);
  nusmv_assert(res);

  {
    char def[20];
    int c = snprintf(def, sizeof(def), "10");
    SNPRINTF_CHECK(c, sizeof(def));
    res = OptsHandler_register_option(opts, OUTPUT_WORD_FORMAT, def,
                                      (Opts_CheckFnType)opt_check_word_format,
                                      (Opts_ReturnFnType)opt_get_integer,
                                      true, INTEGER_OPTION);
    nusmv_assert(res);
  }

  res = OptsHandler_register_bool_option(opts, BACKWARD_COMPATIBILITY,
                                         DEFAULT_BACKWARD_COMPATIBILITY, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, TYPE_CHECKING_WARNING_ON,
                                         DEFAULT_TYPE_CHECKING_WARNING_ON, true);
  nusmv_assert(res);

  res = OptsHandler_register_int_option(opts, VERBOSE_LEVEL, 0, true);
  nusmv_assert(res);

  {
    res = OptsHandler_register_int_option(opts, DEFAULT_TRACE_PLUGIN, 0, true);
    nusmv_assert(res);

    res = OptsHandler_add_option_trigger(opts, DEFAULT_TRACE_PLUGIN,
                                         opt_trace_plugin_trigger);
    nusmv_assert(res);
  }

  res = OptsHandler_register_generic_option(opts, RUN_CPP, "", false);
  nusmv_assert(res);

  res = OptsHandler_add_option_trigger(opts, RUN_CPP,
                                       opt_run_cpp_trigger);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, PP_LIST, "", true);
  nusmv_assert(res);

  res = OptsHandler_add_option_trigger(opts, PP_LIST, opt_pp_list_trigger);
  nusmv_assert(res);

  {
    char def[20];
    int chars = snprintf(def, 20, "%d", DEFAULT_SHOWN_STATES);
    SNPRINTF_CHECK(chars, 20);

    res = OptsHandler_register_option(opts, SHOWN_STATES, def,
                                      (Opts_CheckFnType)opt_check_shown_states,
                                      (Opts_ReturnFnType)opt_get_integer,
                                      true, INTEGER_OPTION);
    nusmv_assert(res);
  }

  res = OptsHandler_register_bool_option(opts, OPT_CHECK_FSM, false, true);
  nusmv_assert(res);

  /* Those below are batch related */
  res = OptsHandler_register_bool_option(opts, IGNORE_SPEC, false, false);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, IGNORE_COMPUTE, false, false);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, IGNORE_LTLSPEC, false, false);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, IGNORE_PSLSPEC, false, false);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, IGNORE_INVAR, false, false);
  nusmv_assert(res);
  /* Those above were batch related */

  res = OptsHandler_register_bool_option(opts, FORWARD_SEARCH, true, true);
  nusmv_assert(res);

  /* if this option is used, also their use is enabled */
  res = OptsHandler_add_option_trigger(opts, FORWARD_SEARCH,
                                       opt_set_reachable_states_trigger);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, LTL_TABLEAU_FORWARD_SEARCH,
                                         false, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, USE_REACHABLE_STATES,
                                         true, false);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, PRINT_REACHABLE, false, false);
  nusmv_assert(res);

  /* if this option is used, also their use is enabled */
  res = OptsHandler_add_option_trigger(opts, PRINT_REACHABLE,
                                       opt_set_reachable_states_trigger);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, ENABLE_REORDER, false, false);
  nusmv_assert(res);

  res = OptsHandler_add_option_trigger(opts, ENABLE_REORDER,
                                       opt_dynamic_reorder_trigger);
  nusmv_assert(res);

  {
    Opts_EnumRec reorders[20] = {
      {DynOrderTypeConvertToString(REORDER_RANDOM), REORDER_RANDOM},
      {DynOrderTypeConvertToString(REORDER_RANDOM_PIVOT), REORDER_RANDOM_PIVOT},
      {DynOrderTypeConvertToString(REORDER_SIFT), REORDER_SIFT},
      {DynOrderTypeConvertToString(REORDER_GROUP_SIFT), REORDER_GROUP_SIFT},
      {DynOrderTypeConvertToString(REORDER_SIFT_CONV), REORDER_SIFT_CONV},
      {DynOrderTypeConvertToString(REORDER_SYMM_SIFT), REORDER_SYMM_SIFT},
      {DynOrderTypeConvertToString(REORDER_SYMM_SIFT_CONV), REORDER_SYMM_SIFT_CONV},
      {DynOrderTypeConvertToString(REORDER_WINDOW2), REORDER_WINDOW2},
      {DynOrderTypeConvertToString(REORDER_WINDOW3), REORDER_WINDOW3},
      {DynOrderTypeConvertToString(REORDER_WINDOW4), REORDER_WINDOW3},
      {DynOrderTypeConvertToString(REORDER_WINDOW2_CONV), REORDER_WINDOW2_CONV},
      {DynOrderTypeConvertToString(REORDER_WINDOW3_CONV), REORDER_WINDOW3_CONV},
      {DynOrderTypeConvertToString(REORDER_WINDOW4_CONV), REORDER_WINDOW4_CONV},
      {DynOrderTypeConvertToString(REORDER_GROUP_SIFT_CONV), REORDER_GROUP_SIFT_CONV},
      {DynOrderTypeConvertToString(REORDER_ANNEALING), REORDER_ANNEALING},
      {DynOrderTypeConvertToString(REORDER_GENETIC), REORDER_GENETIC},
      {DynOrderTypeConvertToString(REORDER_EXACT), REORDER_EXACT},
      {DynOrderTypeConvertToString(REORDER_LINEAR), REORDER_LINEAR},
      {DynOrderTypeConvertToString(REORDER_LINEAR_CONV), REORDER_LINEAR_CONV},
      {DynOrderTypeConvertToString(REORDER_SAME), REORDER_SAME},
    };

    const char* def = DynOrderTypeConvertToString(DEFAULT_REORDER);
    res = OptsHandler_register_enum_option(opts, REORDER_METHOD, def,
                                           reorders, 20, true);
    nusmv_assert(res);

    res = OptsHandler_add_option_trigger(opts, REORDER_METHOD,
                                         opt_reorder_method_trigger);
    nusmv_assert(res);
  }

  res = OptsHandler_register_bool_option(opts, DYNAMIC_REORDER, false, true);
  nusmv_assert(res);

  res = OptsHandler_add_option_trigger(opts, DYNAMIC_REORDER,
                                       opt_dynamic_reorder_trigger);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, ENABLE_SEXP2BDD_CACHING, true, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, BATCH, true, false);
  nusmv_assert(res);

  {
    Opts_EnumRec pm[3] = {
      {TRANS_TYPE_MONOLITHIC_STRING, TRANS_TYPE_MONOLITHIC},
      {TRANS_TYPE_THRESHOLD_STRING, TRANS_TYPE_THRESHOLD},
      {TRANS_TYPE_IWLS95_STRING, TRANS_TYPE_IWLS95}
    };

    res = OptsHandler_register_enum_option(opts, PARTITION_METHOD,
                                           TRANS_TYPE_THRESHOLD_STRING,
                                           pm, 3, true);
    nusmv_assert(res);
  }

  res = OptsHandler_register_int_option(opts, CONJ_PART_THRESHOLD,
                                        DEFAULT_CONJ_PART_THRESHOLD, true);
  nusmv_assert(res);

  res = OptsHandler_register_int_option(opts, IMAGE_CLUSTER_SIZE,
                                        DEFAULT_IMAGE_CLUSTER_SIZE, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, IWLS95_PREORDER, false, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, AFFINITY_CLUSTERING, true, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, APPEND_CLUSTERS, true, false);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, IGNORE_INIT_FILE, false, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, AG_ONLY_SEARCH, false, true);

  res = OptsHandler_register_bool_option(opts, CONE_OF_INFLUENCE,
                                         false, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, LIST_PROPERTIES, false, false);
  nusmv_assert(res);

  {
    Opts_EnumRec pm[4] = {
      {"name", PROP_PRINT_FMT_NAME},
      {"index", PROP_PRINT_FMT_INDEX},
      {"truncated", PROP_PRINT_FMT_FORMULA_TRUNC},
      {"formula", PROP_PRINT_FMT_FORMULA}
    };

    res = OptsHandler_register_enum_option(opts, PROP_PRINT_METHOD,
                                           "formula", pm, 4, true);
    nusmv_assert(res);
  }

  res = OptsHandler_register_int_option(opts, PROP_NO, -1, false);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, USE_FAIR_STATES, true, false);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, COUNTER_EXAMPLES, true, true);
  nusmv_assert(res);

  res = OptsHandler_register_generic_option(opts, TRACES_HIDING_PREFIX,
                                            DEFAULT_TRACES_HIDING_PREFIX,
                                            true);
  nusmv_assert(res);

  /* This is an internal option not visible at user level */
  res = OptsHandler_register_bool_option(opts, BDD_ENCODE_WORD_BITS,
                                         DEFAULT_BDD_ENCODE_WORD_BITS, false);

#if NUSMV_HAVE_REGEX_H
  res = OptsHandler_register_generic_option(opts,
                                            TRACES_REGEXP, (char*)NULL, true);
  nusmv_assert(res);

  res = OptsHandler_add_option_trigger(opts, TRACES_REGEXP,
                                       opt_traces_regexp_trigger);
  nusmv_assert(res);

#endif

  res = OptsHandler_register_bool_option(opts, ON_FAILURE_SCRIPT_QUITS,
                                         false, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, WRITE_ORDER_DUMPS_BITS, true, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, USE_ANSI_C_DIV_OP, true, false);
  nusmv_assert(res);

  {
    Opts_EnumRec vot[6] = {
      {(char*)Enc_vars_ord_to_string(VARS_ORD_INPUTS_BEFORE),
       VARS_ORD_INPUTS_BEFORE},
      {(char*)Enc_vars_ord_to_string(VARS_ORD_INPUTS_AFTER),
       VARS_ORD_INPUTS_AFTER},
      {(char*)Enc_vars_ord_to_string(VARS_ORD_TOPOLOGICAL),
       VARS_ORD_TOPOLOGICAL},
      {(char*)Enc_vars_ord_to_string(VARS_ORD_INPUTS_BEFORE_BI),
       VARS_ORD_INPUTS_BEFORE_BI},
      {(char*)Enc_vars_ord_to_string(VARS_ORD_INPUTS_AFTER_BI),
       VARS_ORD_INPUTS_AFTER_BI},
      {(char*)Enc_vars_ord_to_string(VARS_ORD_TOPOLOGICAL_BI),
       VARS_ORD_TOPOLOGICAL_BI},
    };
    const char* def = Enc_vars_ord_to_string(VARS_ORD_INPUTS_BEFORE_BI);
    res = OptsHandler_register_enum_option(opts, VARS_ORD_TYPE,
                                           def, vot, 6, true);
    nusmv_assert(res);
  }

  {
    Opts_EnumRec bdd[2] = {
      {(char*)Enc_bdd_static_order_heuristics_to_string(
                                                        BDD_STATIC_ORDER_HEURISTICS_BASIC),
       BDD_STATIC_ORDER_HEURISTICS_BASIC},
      {(char*)Enc_bdd_static_order_heuristics_to_string(
                                                        BDD_STATIC_ORDER_HEURISTICS_NONE),
       BDD_STATIC_ORDER_HEURISTICS_NONE}
    };
    const char* def = Enc_bdd_static_order_heuristics_to_string(
                                                                BDD_STATIC_ORDER_HEURISTICS_BASIC);
    res = OptsHandler_register_enum_option(opts, BDD_STATIC_ORDER_HEURISTICS,
                                           def, bdd, 2, true);
    nusmv_assert(res);
  }

  res = OptsHandler_register_bool_option(opts, SYMB_INLINING,
                                         false, true);
  nusmv_assert(res);

#if NUSMV_HAVE_SAT_SOLVER
  {
    Opts_EnumRec rbc[2] = {
      {RBC_SHERIDAN_CONVERSION_NAME, RBC_SHERIDAN_CONVERSION},
      {RBC_TSEITIN_CONVERSION_NAME, RBC_TSEITIN_CONVERSION}
    };

    res = OptsHandler_register_enum_option(opts, RBC_CNF_ALGORITHM,
                                           RBC_SHERIDAN_CONVERSION_NAME,
                                           rbc, 2, true);
    nusmv_assert(res);
  }

  res = OptsHandler_register_bool_option(opts, RBC_INLINING,
                                         true, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, RBC_INLINING_LAZY,
                                         false, false);
  nusmv_assert(res);

  res = OptsHandler_add_option_trigger(opts, RBC_INLINING_LAZY,
                                       opt_rbc_inlining_lazy_trigger);
  nusmv_assert(res);

#endif

  res = OptsHandler_register_option(opts, A_SAT_SOLVER,
                                    DEFAULT_SAT_SOLVER,
                                    (Opts_CheckFnType)opt_check_sat_solver,
                                    (Opts_ReturnFnType)opt_get_sat_solver,
                                    true, GENERIC_OPTION);
  nusmv_assert(res);


  res = OptsHandler_register_bool_option(opts, SHOW_DEFINES_IN_TRACES,
                                         DEFAULT_SHOW_DEFINES_IN_TRACES, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, SHOW_DEFINES_WITH_NEXT,
                                         DEFAULT_SHOW_DEFINES_WITH_NEXT, true);

  nusmv_assert(res);

  {
#if NUSMV_HAVE_SAT_SOLVER
    Opts_EnumRec strategies[4] = {
#else
    Opts_EnumRec strategies[3] = {
#endif
      {(char*)opt_check_invar_strategy_to_string(FORWARD), FORWARD},
      {(char*)opt_check_invar_strategy_to_string(BACKWARD), BACKWARD},
      {(char*)opt_check_invar_strategy_to_string(FORWARD_BACKWARD),
       FORWARD_BACKWARD}
#if NUSMV_HAVE_SAT_SOLVER
      ,{(char*)opt_check_invar_strategy_to_string(BDD_BMC), BDD_BMC}
#endif
    };
    const char* def =
      opt_check_invar_strategy_to_string(DEFAULT_INVAR_CHECK_STRATEGY);

#if NUSMV_HAVE_SAT_SOLVER
    res = OptsHandler_register_enum_option(opts, INVAR_CHECK_STRATEGY,
                                           def, strategies, 4, true);
#else
    res = OptsHandler_register_enum_option(opts, INVAR_CHECK_STRATEGY,
                                           def, strategies, 3, true);
#endif
    nusmv_assert(res);
  }

  {
    Opts_EnumRec heuristics[2] = {
      {(char*)opt_check_invar_fb_heuristic_to_string(ZIGZAG_HEURISTIC),
       ZIGZAG_HEURISTIC},
      {(char*)opt_check_invar_fb_heuristic_to_string(SMALLEST_BDD_HEURISTIC),
       SMALLEST_BDD_HEURISTIC}
    };
    const char* def =
      opt_check_invar_fb_heuristic_to_string(
                                             DEFAULT_FORWARD_BACKWARD_ANALYSIS_HEURISTIC);

    res = OptsHandler_register_enum_option(opts, CHECK_INVAR_FB_HEURISTIC,
                                           def, heuristics, 2, true);
    nusmv_assert(res);
  }

  {
    Opts_EnumRec heuristics[2] = {
      {(char*)opt_check_invar_bddbmc_heuristic_to_string(STEPS_HEURISTIC),
       STEPS_HEURISTIC},
      {(char*)opt_check_invar_bddbmc_heuristic_to_string(SIZE_HEURISTIC),
       SIZE_HEURISTIC}
    };
    const char* def =
      opt_check_invar_bddbmc_heuristic_to_string(DEFAULT_BDD2BMC_HEURISTIC);

    res = OptsHandler_register_enum_option(opts, CHECK_INVAR_BDDBMC_HEURISTIC,
                                           def, heuristics, 2, true);
    nusmv_assert(res);
  }

  res = OptsHandler_register_int_option(opts,
                                        CHECK_INVAR_BDDBMC_HEURISTIC_THRESHOLD,
                                        DEFAULT_BDD2BMC_HEURISTIC_THRESHOLD, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts,
                                         DAGGIFIER_ENABLED,
                                         DEFAULT_DAGGIFIER_ENABLED, true);
  nusmv_assert(res);

  res = OptsHandler_register_int_option(opts,
                                        DAGGIFIER_COUNTER_THRESHOLD,
                                        DEFAULT_DAGGIFIER_COUNTER_THS, true);
  nusmv_assert(res);

  res = OptsHandler_register_int_option(opts,
                                        DAGGIFIER_DEPTH_THRESHOLD,
                                        DEFAULT_DAGGIFIER_DEPTH_THS, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts,
                                         DAGGIFIER_STATISTICS, false, true);
  nusmv_assert(res);

  {
    Opts_EnumRec oreg[2] = {
      {(char*)Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string(
                             BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD),
       BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD},
      {(char*)Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string(
                             BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD),
       BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD}
    };
    const char* def = Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string(
                                DEFAULT_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM);

    res = OptsHandler_register_enum_option(opts,
                                           OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM,
                                           def, oreg, 2, true);
    nusmv_assert(res);
  }

  res = OptsHandler_register_bool_option(opts,
                                         USE_COI_SIZE_SORTING,
                                         DEFAULT_USE_COI_SIZE_SORTING, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, DISABLE_SYNTACTIC_CHECKS,
                                         false, true);
  nusmv_assert(res);

  res = OptsHandler_register_bool_option(opts, KEEP_SINGLE_VALUE_VARS,
                                         false, true);
  nusmv_assert(res);

  res = OptsHandler_register_int_option(opts, DEFAULT_SIMULATION_STEPS,
                                        10, true);
  nusmv_assert(res);
}

void deinit_options()
{
  OptsHandler_instance_destroy();
}

/**Function********************************************************************

   Synopsis           [Initialize the NuSMV options command]

   Description        [This function declares the interactive shell
   commands necessary to manipulate NuSMV options.]

   SideEffects        []

******************************************************************************/
void init_options_cmd()
{

  Cmd_CommandAdd("set", CommandSetVariable, 0, true);
  Cmd_CommandAdd("unset", CommandUnsetVariable, 0, true);

#if TEST_OPTS_HANDLER
  Cmd_CommandAdd("_gen_test_opts_handler", CommandGenTestOptsHandler, 0, true);
#endif
}

void set_pgm_name(OptsHandler_ptr opt, char* str)
{
  boolean res = OptsHandler_set_option_value(opt, PROGRAM_NAME, str);
  nusmv_assert(res);
}
void reset_pgm_name(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, PROGRAM_NAME);
  nusmv_assert(res);
}
char* get_pgm_name(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, PROGRAM_NAME);
}

void set_script_file(OptsHandler_ptr opt, char* str)
{
  boolean res = OptsHandler_set_option_value(opt, SCRIPT_FILE, str);
  nusmv_assert(res);
}
void reset_script_file(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, SCRIPT_FILE);
  nusmv_assert(res);
}
char* get_script_file(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, SCRIPT_FILE);
}

void set_pgm_path(OptsHandler_ptr opt, char* str)
{
  boolean res = OptsHandler_set_option_value(opt, PROGRAM_PATH, str);
  nusmv_assert(res);
}
void reset_pgm_path(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, PROGRAM_PATH);
  nusmv_assert(res);
}
char* get_pgm_path(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, PROGRAM_PATH);
}

void set_input_file(OptsHandler_ptr opt, char* str)
{
  boolean res = OptsHandler_set_option_value(opt, INPUT_FILE, str);
  nusmv_assert(res);
}
void reset_input_file(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, INPUT_FILE);
  nusmv_assert(res);
}
char* get_input_file(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, INPUT_FILE);
}

void set_input_order_file(OptsHandler_ptr opt, char* str)
{
  boolean res = OptsHandler_set_option_value(opt, INPUT_ORDER_FILE, str);
  nusmv_assert(res);
}
void reset_input_order_file(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, INPUT_ORDER_FILE);
  nusmv_assert(res);
}
char* get_input_order_file(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, INPUT_ORDER_FILE);
}

void set_output_order_file(OptsHandler_ptr opt, char* str)
{
  boolean res = OptsHandler_set_option_value(opt, OUTPUT_ORDER_FILE, str);
  nusmv_assert(res);
}
void reset_output_order_file(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, OUTPUT_ORDER_FILE);
  nusmv_assert(res);
}
char* get_output_order_file(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, OUTPUT_ORDER_FILE);
}
boolean is_default_order_file(OptsHandler_ptr opt)
{
  char* oof;
  oof = get_output_order_file(opt);

  if (oof == NIL(char)) {
    return DEFAULT_OUTPUT_ORDER_FILE == NIL(char);
  }
  if (DEFAULT_OUTPUT_ORDER_FILE == NIL(char)) return false;
  return((strcmp(oof, DEFAULT_OUTPUT_ORDER_FILE) == 0));
}

void set_trans_order_file(OptsHandler_ptr opt, char* str)
{
  boolean res = OptsHandler_set_option_value(opt, TRANS_ORDER_FILE, str);
  nusmv_assert(res);
}
void reset_trans_order_file(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, TRANS_ORDER_FILE);
  nusmv_assert(res);
}
char* get_trans_order_file(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, TRANS_ORDER_FILE);
}
boolean opt_trans_order_file(OptsHandler_ptr opt)
{
  return (get_trans_order_file(opt) != NULL);
}

void set_output_flatten_model_file(OptsHandler_ptr opt, char* str)
{
  boolean res = OptsHandler_set_option_value(opt, OUTPUT_FLATTEN_MODEL_FILE, str);
  nusmv_assert(res);
}
void reset_output_flatten_model_file(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, OUTPUT_FLATTEN_MODEL_FILE);
  nusmv_assert(res);
}
char* get_output_flatten_model_file(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, OUTPUT_FLATTEN_MODEL_FILE);
}

void set_output_boolean_model_file(OptsHandler_ptr opt, char* str)
{
  boolean res = OptsHandler_set_option_value(opt, OUTPUT_BOOLEAN_MODEL_FILE, str);
  nusmv_assert(res);
}
void reset_output_boolean_model_file(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, OUTPUT_BOOLEAN_MODEL_FILE);
  nusmv_assert(res);
}
char* get_output_boolean_model_file(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, OUTPUT_BOOLEAN_MODEL_FILE);
}

void set_output_word_format(OptsHandler_ptr opt, int i)
{
  boolean res = OptsHandler_set_int_option_value(opt, OUTPUT_WORD_FORMAT, i);
  nusmv_assert(res);
}
int get_output_word_format(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, OUTPUT_WORD_FORMAT);
}

void set_backward_comp(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  BACKWARD_COMPATIBILITY, true);
  nusmv_assert(res);
}
void unset_backward_comp(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  BACKWARD_COMPATIBILITY, false);
  nusmv_assert(res);
}
boolean opt_backward_comp(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, BACKWARD_COMPATIBILITY);
}

void set_type_checking_warning_on(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, TYPE_CHECKING_WARNING_ON, true);
  nusmv_assert(res);
}
void unset_type_checking_warning_on(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, TYPE_CHECKING_WARNING_ON, false);
  nusmv_assert(res);
}
boolean opt_type_checking_warning_on(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, TYPE_CHECKING_WARNING_ON);
}

void set_verbose_level(OptsHandler_ptr opt, int level)
{
  boolean res = OptsHandler_set_int_option_value(opt, VERBOSE_LEVEL, level);
  nusmv_assert(res);
}
int get_verbose_level(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, VERBOSE_LEVEL);
}
boolean opt_verbose_level_eq(OptsHandler_ptr opt, int level)
{
  return((get_verbose_level(opt) == level));
}
boolean opt_verbose_level_gt(OptsHandler_ptr opt, int level)
{
  return((get_verbose_level(opt) > level));
}
boolean opt_verbose_level_ge(OptsHandler_ptr opt, int level)
{
  return((get_verbose_level(opt) >= level));
}
boolean opt_verbose_level_lt(OptsHandler_ptr opt, int level)
{
  return((get_verbose_level(opt) < level));
}
boolean opt_verbose_level_le(OptsHandler_ptr opt, int level)
{
  return((get_verbose_level(opt) <= level));
}

void set_pp_list(OptsHandler_ptr opt, char* pp_list)
{
  char* new_pp_list;

  if (strcmp(pp_list,"") != 0) {
    new_pp_list = remove_non_existant_pps(pp_list);
  }
  else {
    new_pp_list = util_strsav("");
  }

  boolean res = OptsHandler_set_option_value(opt, PP_LIST, new_pp_list);
  nusmv_assert(res);
}
char* get_pp_list(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, PP_LIST);
}

void set_shown_states_level(OptsHandler_ptr opt, int level)
{
  boolean res = OptsHandler_set_int_option_value(opt, SHOWN_STATES, level);
  nusmv_assert(res);
}
int opt_shown_states_level(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, SHOWN_STATES);
}

void set_ignore_spec(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_SPEC, true);
  nusmv_assert(res);
}
void unset_ignore_spec(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_SPEC, false);
  nusmv_assert(res);
}
boolean opt_ignore_spec(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, IGNORE_SPEC);
}

void set_ignore_compute(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_COMPUTE, true);
  nusmv_assert(res);
}
void unset_ignore_compute(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_COMPUTE, false);
  nusmv_assert(res);
}
boolean opt_ignore_compute(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, IGNORE_COMPUTE);
}

void set_ignore_ltlspec(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_LTLSPEC, true);
  nusmv_assert(res);
}
void unset_ignore_ltlspec(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_LTLSPEC, false);
  nusmv_assert(res);
}
boolean opt_ignore_ltlspec(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, IGNORE_LTLSPEC);
}

void set_ignore_pslspec(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_PSLSPEC, true);
  nusmv_assert(res);
}
void unset_ignore_pslspec(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_PSLSPEC, false);
  nusmv_assert(res);
}
boolean opt_ignore_pslspec(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, IGNORE_PSLSPEC);
}

void set_ignore_invar(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_INVAR, true);
  nusmv_assert(res);
}
void unset_ignore_invar(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IGNORE_INVAR, false);
  nusmv_assert(res);
}
boolean opt_ignore_invar(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, IGNORE_INVAR);
}

void set_check_fsm(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, OPT_CHECK_FSM, true);
  nusmv_assert(res);
}
void unset_check_fsm(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, OPT_CHECK_FSM, false);
  nusmv_assert(res);
}
boolean opt_check_fsm(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, OPT_CHECK_FSM);
}

void set_forward_search(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, FORWARD_SEARCH, true);
  nusmv_assert(res);
}
void unset_forward_search(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, FORWARD_SEARCH, false);
  nusmv_assert(res);
}
boolean opt_forward_search(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, FORWARD_SEARCH);
}

void set_ltl_tableau_forward_search(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, LTL_TABLEAU_FORWARD_SEARCH, true);
  nusmv_assert(res);
}
void unset_ltl_tableau_forward_search(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, LTL_TABLEAU_FORWARD_SEARCH, false);
  nusmv_assert(res);
}
boolean opt_ltl_tableau_forward_search(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, LTL_TABLEAU_FORWARD_SEARCH);
}

void set_print_reachable(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, PRINT_REACHABLE, true);
  nusmv_assert(res);
}
void unset_print_reachable(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, PRINT_REACHABLE, false);
  nusmv_assert(res);
}
boolean opt_print_reachable(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, PRINT_REACHABLE);
}

void set_reorder(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, ENABLE_REORDER, true);
  nusmv_assert(res);
}
void unset_reorder(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, ENABLE_REORDER, false);
  nusmv_assert(res);
}
boolean opt_reorder(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, ENABLE_REORDER);
}

void set_reorder_method(OptsHandler_ptr opt, unsigned int method)
{
  char* value = DynOrderTypeConvertToString(method);
  boolean res = OptsHandler_set_enum_option_value(opt, REORDER_METHOD, value);
  nusmv_assert(res);
}
unsigned int get_reorder_method(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, REORDER_METHOD);
}

void set_dynamic_reorder(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, DYNAMIC_REORDER, true);
  nusmv_assert(res);
}
void unset_dynamic_reorder(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, DYNAMIC_REORDER, false);
  nusmv_assert(res);
}
boolean opt_dynamic_reorder(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, DYNAMIC_REORDER);
}

/* allows the memoization (caching) of computations
   in sexpr-to-bdd evaluations. */
void set_enable_sexp2bdd_caching(OptsHandler_ptr opt)
{
  boolean res =
    OptsHandler_set_bool_option_value(opt,
                                      ENABLE_SEXP2BDD_CACHING,
                                      true);
  nusmv_assert(res);
}
void unset_enable_sexp2bdd_caching(OptsHandler_ptr opt)
{
  boolean res =
    OptsHandler_set_bool_option_value(opt,
                                      ENABLE_SEXP2BDD_CACHING,
                                      false);
  nusmv_assert(res);
}
boolean opt_enable_sexp2bdd_caching(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt,
                                           ENABLE_SEXP2BDD_CACHING);
}

void set_batch(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, BATCH, true);
  nusmv_assert(res);
}
void unset_batch(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, BATCH, false);
  nusmv_assert(res);
}
boolean opt_batch(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, BATCH);
}

void set_partition_method(OptsHandler_ptr opt, const TransType method)
{
  char* str = NULL;

  switch (method) {
  case TRANS_TYPE_MONOLITHIC:
    str = TRANS_TYPE_MONOLITHIC_STRING; break;
  case TRANS_TYPE_IWLS95:
    str = TRANS_TYPE_IWLS95_STRING; break;
  case TRANS_TYPE_THRESHOLD:
    str = TRANS_TYPE_THRESHOLD_STRING; break;
  default: break;
  }

  boolean res = OptsHandler_set_enum_option_value(opt, PARTITION_METHOD, str);
  nusmv_assert(res);
}
TransType get_partition_method(OptsHandler_ptr opt)
{
  return (TransType)OptsHandler_get_enum_option_value(opt, PARTITION_METHOD);
}
void set_monolithic(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_enum_option_value(opt, PARTITION_METHOD,
                                                  TRANS_TYPE_MONOLITHIC_STRING);
  nusmv_assert(res);
}
void set_conj_partitioning(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_enum_option_value(opt, PARTITION_METHOD,
                                                  TRANS_TYPE_THRESHOLD_STRING);
  nusmv_assert(res);
}
void reset_partitioning_method(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, PARTITION_METHOD);
  nusmv_assert(res);
}
void set_iwls95cp_partitioning(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_enum_option_value(opt, PARTITION_METHOD,
                                                  TRANS_TYPE_IWLS95_STRING);
  nusmv_assert(res);
}
boolean opt_monolithic(OptsHandler_ptr opt)
{
  return((OptsHandler_get_enum_option_value(opt, PARTITION_METHOD) ==
          TRANS_TYPE_MONOLITHIC));
}
boolean opt_conj_partitioning(OptsHandler_ptr opt)
{
  return((OptsHandler_get_enum_option_value(opt, PARTITION_METHOD) ==
          TRANS_TYPE_THRESHOLD));
}
boolean opt_iwls95cp_partitioning(OptsHandler_ptr opt)
{
  return((OptsHandler_get_enum_option_value(opt, PARTITION_METHOD) ==
          TRANS_TYPE_IWLS95));
}

void set_conj_part_threshold(OptsHandler_ptr opt, int threshold)
{
  boolean res = OptsHandler_set_int_option_value(opt, CONJ_PART_THRESHOLD,
                                                 threshold);
  nusmv_assert(res);
}
void reset_conj_part_threshold(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, CONJ_PART_THRESHOLD);
  nusmv_assert(res);
}
int get_conj_part_threshold(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, CONJ_PART_THRESHOLD);
}

void set_image_cluster_size(OptsHandler_ptr opt, int threshold)
{
  boolean res = OptsHandler_set_int_option_value(opt, IMAGE_CLUSTER_SIZE,
                                                 threshold);
  nusmv_assert(res);
}
void reset_image_cluster_size(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, IMAGE_CLUSTER_SIZE);
  nusmv_assert(res);
}
int get_image_cluster_size(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, IMAGE_CLUSTER_SIZE);
}

void set_ignore_init_file(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  IGNORE_INIT_FILE,
                                                  true);
  nusmv_assert(res);
}
void unset_ignore_init_file(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  IGNORE_INIT_FILE,
                                                  false);
  nusmv_assert(res);
}
boolean opt_ignore_init_file(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, IGNORE_INIT_FILE);
}

void set_ag_only(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, AG_ONLY_SEARCH, true);
  nusmv_assert(res);
}
void unset_ag_only(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, AG_ONLY_SEARCH, false);
  nusmv_assert(res);
}
boolean opt_ag_only(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, AG_ONLY_SEARCH);
}

void set_cone_of_influence(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, CONE_OF_INFLUENCE, true);
  nusmv_assert(res);
}
void unset_cone_of_influence(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, CONE_OF_INFLUENCE, false);
  nusmv_assert(res);
}
boolean opt_cone_of_influence(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, CONE_OF_INFLUENCE);
}

void set_list_properties(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, LIST_PROPERTIES, true);
  nusmv_assert(res);
}
void unset_list_properties(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, LIST_PROPERTIES, false);
  nusmv_assert(res);
}
boolean opt_list_properties(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, LIST_PROPERTIES);
}


void set_prop_print_method(OptsHandler_ptr opt, int threshold)
{
  boolean res = OptsHandler_set_int_option_value(opt, PROP_PRINT_METHOD,
                                                 threshold);
  nusmv_assert(res);
}
void reset_prop_print_method(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, PROP_PRINT_METHOD);
  nusmv_assert(res);
}
int get_prop_print_method(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, PROP_PRINT_METHOD);
}


void set_prop_no(OptsHandler_ptr opt, int n)
{
  boolean res = (n >= 0);
  nusmv_assert(res);
  res = OptsHandler_set_int_option_value(opt, PROP_NO, n);
  nusmv_assert(res);
}
int get_prop_no(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, PROP_NO);
}

void print_partition_method (FILE * output_stream)
{
  fprintf(output_stream, "Monolithic, Threshold, Iwls95CP");
}

void set_sat_solver(OptsHandler_ptr opt, const char* satSolver)
{
  boolean res = OptsHandler_set_option_value(opt, A_SAT_SOLVER, satSolver);
  nusmv_assert(res);
}
const char* get_sat_solver(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, A_SAT_SOLVER);
}

boolean set_default_trace_plugin(OptsHandler_ptr opt, int plugin)
{
  return OptsHandler_set_int_option_value(opt,
                                          DEFAULT_TRACE_PLUGIN,
                                          plugin);
}
int get_default_trace_plugin(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, DEFAULT_TRACE_PLUGIN);
}

/* Only for testing purpose right now */
void set_iwls95_preorder(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IWLS95_PREORDER, true);
  nusmv_assert(res);
}

void unset_iwls95_preorder(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, IWLS95_PREORDER, false);
  nusmv_assert(res);
}

boolean opt_iwls95_preorder(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, IWLS95_PREORDER);
}

void set_affinity(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, AFFINITY_CLUSTERING, true);
  nusmv_assert(res);
}

void unset_affinity(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, AFFINITY_CLUSTERING, false);
  nusmv_assert(res);
}

boolean opt_affinity(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, AFFINITY_CLUSTERING);
}

void set_append_clusters(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, APPEND_CLUSTERS, true);
  nusmv_assert(res);
}

void unset_append_clusters(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, APPEND_CLUSTERS, true);
  nusmv_assert(res);
}

boolean opt_append_clusters(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, APPEND_CLUSTERS);
}

void set_counter_examples(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, COUNTER_EXAMPLES, true);
  nusmv_assert(res);
}

void unset_counter_examples(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, COUNTER_EXAMPLES, false);
  nusmv_assert(res);
}

boolean opt_counter_examples(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, COUNTER_EXAMPLES);
}

void set_traces_hiding_prefix(OptsHandler_ptr opt, const char* prefix)
{
  boolean res = OptsHandler_set_option_value(opt, TRACES_HIDING_PREFIX,
                                             prefix);
  nusmv_assert(res);
}

const char* opt_traces_hiding_prefix(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, TRACES_HIDING_PREFIX);
}

void set_bdd_encoding_word_bits(OptsHandler_ptr opt) {
  boolean res = OptsHandler_set_bool_option_value(opt, BDD_ENCODE_WORD_BITS, true);
  nusmv_assert(res);
}

void unset_bdd_encoding_word_bits(OptsHandler_ptr opt) {
  boolean res = OptsHandler_set_bool_option_value(opt, BDD_ENCODE_WORD_BITS, false);
  nusmv_assert(res);
}

void reset_bdd_encoding_word_bits(OptsHandler_ptr opt) {
  boolean res = OptsHandler_reset_option_value(opt, BDD_ENCODE_WORD_BITS);
  nusmv_assert(res);
}

boolean opt_bdd_encoding_word_bits(OptsHandler_ptr opt) {
  return OptsHandler_get_bool_option_value(opt, BDD_ENCODE_WORD_BITS);
}

#if NUSMV_HAVE_REGEX_H
/* Returns true if everything went smooth, false otherwise. In case of
   error, a message is printed */
boolean set_traces_regexp(OptsHandler_ptr opt, const char* re)
{
  boolean res = OptsHandler_set_option_value(opt, TRACES_REGEXP, re);
  return res;
}

const char* opt_traces_regexp(OptsHandler_ptr opt)
{
  return OptsHandler_get_string_option_value(opt, TRACES_REGEXP);
}
#endif


void set_on_failure_script_quits(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  ON_FAILURE_SCRIPT_QUITS,
                                                  true);
  nusmv_assert(res);
}

void unset_on_failure_script_quits(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  ON_FAILURE_SCRIPT_QUITS,
                                                  false);
  nusmv_assert(res);
}

boolean opt_on_failure_script_quits(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, ON_FAILURE_SCRIPT_QUITS);
}

void set_write_order_dumps_bits(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  WRITE_ORDER_DUMPS_BITS,
                                                  true);
  nusmv_assert(res);
}

void unset_write_order_dumps_bits(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  WRITE_ORDER_DUMPS_BITS,
                                                  false);
  nusmv_assert(res);
}

boolean opt_write_order_dumps_bits(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, WRITE_ORDER_DUMPS_BITS);
}

void set_use_fair_states(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, USE_FAIR_STATES, true);
  nusmv_assert(res);
}

void unset_use_fair_states(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, USE_FAIR_STATES, false);
  nusmv_assert(res);
}

boolean opt_use_fair_states(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, USE_FAIR_STATES);
}

void set_use_reachable_states(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  USE_REACHABLE_STATES,
                                                  true);
  nusmv_assert(res);
}

void unset_use_reachable_states(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  USE_REACHABLE_STATES,
                                                  false);
  nusmv_assert(res);
}

boolean opt_use_reachable_states(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, USE_REACHABLE_STATES);
}

void unset_use_ansi_c_div_op(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, USE_ANSI_C_DIV_OP, false);
  nusmv_assert(res);
}
boolean opt_use_ansi_c_div_op(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, USE_ANSI_C_DIV_OP);
}



void set_vars_order_type(OptsHandler_ptr opt, VarsOrdType type)
{
  boolean res = OptsHandler_set_enum_option_value(opt, VARS_ORD_TYPE,
                                                  (char*)Enc_vars_ord_to_string(type));
  nusmv_assert(res);
}

VarsOrdType get_vars_order_type(OptsHandler_ptr opt)
{
  return (VarsOrdType)OptsHandler_get_enum_option_value(opt, VARS_ORD_TYPE);
}

void set_bdd_static_order_heuristics(OptsHandler_ptr opt, BddSohEnum value)
{
  char* val = (char*)Enc_bdd_static_order_heuristics_to_string(value);
  boolean res = OptsHandler_set_enum_option_value(opt,
                                                  BDD_STATIC_ORDER_HEURISTICS,
                                                  val);
  nusmv_assert(res);
}

BddSohEnum get_bdd_static_order_heuristics(OptsHandler_ptr opt)
{
  return (BddSohEnum)OptsHandler_get_enum_option_value(opt,
                                                       BDD_STATIC_ORDER_HEURISTICS);
}

/* RBC2CNF */
void set_rbc2cnf_algorithm(OptsHandler_ptr opt, Rbc_2CnfAlgorithm algo)
{
  char * val = (char*) NULL;

  switch (algo) {
  case RBC_SHERIDAN_CONVERSION: val = RBC_SHERIDAN_CONVERSION_NAME; break;
  case RBC_TSEITIN_CONVERSION: val = RBC_TSEITIN_CONVERSION_NAME; break;
  default: error_unreachable_code();
  }

  boolean res = OptsHandler_set_enum_option_value(opt, RBC_CNF_ALGORITHM, val);
  nusmv_assert(res);
}

void unset_rbc2cnf_algorithm(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt, RBC_CNF_ALGORITHM);
  nusmv_assert(res);
}

Rbc_2CnfAlgorithm get_rbc2cnf_algorithm(OptsHandler_ptr opt)
{
  return (Rbc_2CnfAlgorithm)OptsHandler_get_enum_option_value(opt,
                                                              RBC_CNF_ALGORITHM);
}


/* inlining */
void set_symb_inlining(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, SYMB_INLINING, true);
  nusmv_assert(res);
}

void unset_symb_inlining(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, SYMB_INLINING, false);
  nusmv_assert(res);
}

boolean opt_symb_inlining(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, SYMB_INLINING);
}

void set_rbc_inlining(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, RBC_INLINING, true);
  nusmv_assert(res);
}

void unset_rbc_inlining(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, RBC_INLINING, false);
  nusmv_assert(res);
}

boolean opt_rbc_inlining(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, RBC_INLINING);
}

void set_rbc_inlining_lazy(OptsHandler_ptr opt)
{
  OptsHandler_set_bool_option_value(opt, RBC_INLINING_LAZY, true);
}

void unset_rbc_inlining_lazy(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  RBC_INLINING_LAZY, false);
  nusmv_assert(res);
}

boolean opt_rbc_inlining_lazy(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, RBC_INLINING_LAZY);
}

void set_show_defines_in_traces (OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, SHOW_DEFINES_IN_TRACES,
                                                  true);
  nusmv_assert(res);
}

void unset_show_defines_in_traces (OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, SHOW_DEFINES_IN_TRACES,
                                                  false);
  nusmv_assert(res);
}

boolean opt_show_defines_in_traces (OptsHandler_ptr opt)
{
  nusmv_assert (opt != NULL);
  return OptsHandler_get_bool_option_value(opt, SHOW_DEFINES_IN_TRACES);
}

void set_show_defines_with_next (OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, SHOW_DEFINES_WITH_NEXT,
                                                  true);
  nusmv_assert(res);
}

void unset_show_defines_with_next (OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt, SHOW_DEFINES_WITH_NEXT,
                                                  false);
  nusmv_assert(res);
}

boolean opt_show_defines_with_next (OptsHandler_ptr opt)
{
  nusmv_assert (opt != NULL);
  return OptsHandler_get_bool_option_value(opt, SHOW_DEFINES_WITH_NEXT);
}

/* Invar check strategy */
void set_check_invar_strategy (OptsHandler_ptr opt, Check_Strategy strategy)
{
  char* str = (char*)opt_check_invar_strategy_to_string(strategy);
  boolean res = OptsHandler_set_enum_option_value(opt,
                                                  INVAR_CHECK_STRATEGY,
                                                  str);
  nusmv_assert(res);
}

Check_Strategy opt_check_invar_strategy (OptsHandler_ptr opt)
{
  nusmv_assert (opt != NULL);
  return (Check_Strategy)OptsHandler_get_enum_option_value(opt,
                                                           INVAR_CHECK_STRATEGY);
}

static const char* opt_check_invar_strategy_to_string(Check_Strategy str)
{
  switch (str) {
  case FORWARD:
    return "forward";
  case BACKWARD:
    return "backward";
  case FORWARD_BACKWARD:
    return "forward-backward";
  case BDD_BMC:
    return "bdd-bmc";
  }

  error_unreachable_code();
  return (char*)NULL;
}

const char* opt_check_invar_strategy_as_string (OptsHandler_ptr opt)
{
  nusmv_assert (opt != NULL);

  return opt_check_invar_strategy_to_string(opt_check_invar_strategy(opt));
}


/* Forward-Backward heuristic */
void set_check_invar_fb_heuristic (OptsHandler_ptr opt, FB_Heuristic h)
{
  const char* str = opt_check_invar_fb_heuristic_to_string(h);
  boolean res = OptsHandler_set_enum_option_value(opt,
                                                  CHECK_INVAR_FB_HEURISTIC,
                                                  str);
  nusmv_assert(res);
}

FB_Heuristic opt_check_invar_fb_heuristic (OptsHandler_ptr opt)
{
  nusmv_assert (opt != NULL);
  return (FB_Heuristic)OptsHandler_get_enum_option_value(opt,
                                                         CHECK_INVAR_FB_HEURISTIC);
}

const char* opt_check_invar_fb_heuristic_as_string (OptsHandler_ptr opt)
{
  nusmv_assert (opt != NULL);

  return opt_check_invar_fb_heuristic_to_string(opt_check_invar_fb_heuristic(opt));
}

/* Switch between BDD and BMC heuristic */
void set_check_invar_bddbmc_heuristic (OptsHandler_ptr opt, Bdd2bmc_Heuristic h)
{
  const char* str = opt_check_invar_bddbmc_heuristic_to_string(h);
  boolean res = OptsHandler_set_enum_option_value(opt,
                                                  CHECK_INVAR_BDDBMC_HEURISTIC,
                                                  str);
  nusmv_assert(res);
}

Bdd2bmc_Heuristic opt_check_invar_bddbmc_heuristic (OptsHandler_ptr opt)
{
  return (Bdd2bmc_Heuristic)OptsHandler_get_enum_option_value(opt,
                                                              CHECK_INVAR_BDDBMC_HEURISTIC);
}

const char* opt_check_invar_bddbmc_heuristic_as_string (OptsHandler_ptr opt)
{
  return opt_check_invar_bddbmc_heuristic_to_string(opt_check_invar_bddbmc_heuristic(opt));
}


/* Switch between BDD and BMC heuristic threshold*/
void set_check_invar_bddbmc_heuristic_threshold (OptsHandler_ptr opt, int h)
{
  boolean res = OptsHandler_set_int_option_value(opt,
                                                 CHECK_INVAR_BDDBMC_HEURISTIC_THRESHOLD, h);
  nusmv_assert(res);
}

int opt_check_invar_bddbmc_heuristic_threshold (OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt,
                                          CHECK_INVAR_BDDBMC_HEURISTIC_THRESHOLD);
}

/* Daggifier is enabled? */
boolean opt_is_daggifier_enabled(OptsHandler_ptr opt) {
  return OptsHandler_get_bool_option_value(opt, DAGGIFIER_ENABLED);
}

void opt_enable_daggifier(OptsHandler_ptr opt) {
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  DAGGIFIER_ENABLED,
                                                  true);
  nusmv_assert(res);
}

void opt_disable_daggifier(OptsHandler_ptr opt) {
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  DAGGIFIER_ENABLED,
                                                  false);
  nusmv_assert(res);
}

/* Daggifier options */
int opt_get_daggifier_counter_threshold(OptsHandler_ptr opt) {
  return OptsHandler_get_int_option_value(opt, DAGGIFIER_COUNTER_THRESHOLD);
}

void opt_set_daggifier_counter_threshold(OptsHandler_ptr opt, int x) {
  boolean res = OptsHandler_set_int_option_value(opt,
                                                 DAGGIFIER_COUNTER_THRESHOLD,
                                                 x);
  nusmv_assert(res);
}

int opt_get_daggifier_depth_threshold(OptsHandler_ptr opt) {
  return OptsHandler_get_int_option_value(opt, DAGGIFIER_DEPTH_THRESHOLD);
}

void opt_set_daggifier_depth_threshold(OptsHandler_ptr opt,
                                       int x) {
  boolean res = OptsHandler_set_int_option_value(opt, DAGGIFIER_DEPTH_THRESHOLD, x);
  nusmv_assert(res);
}

/* Daggifier statistics */
boolean opt_get_daggifier_statistics(OptsHandler_ptr opt) {
  return OptsHandler_get_bool_option_value(opt, DAGGIFIER_STATISTICS);
}

void set_daggifier_statistics(OptsHandler_ptr opt) {
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  DAGGIFIER_STATISTICS,
                                                  true);
  nusmv_assert(res);
}

void unset_daggifier_statistics(OptsHandler_ptr opt) {
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  DAGGIFIER_STATISTICS,
                                                  false);
  nusmv_assert(res);
}

/* different BDD-based algorithms to check language emptiness for
   omega-regular properties */

BddOregJusticeEmptinessBddAlgorithmType
get_oreg_justice_emptiness_bdd_algorithm(OptsHandler_ptr opt)
{
  int res;

  res = OptsHandler_get_enum_option_value(opt,
                                          OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM);

  return (BddOregJusticeEmptinessBddAlgorithmType)res;
}

void set_oreg_justice_emptiness_bdd_algorithm(OptsHandler_ptr opt,
                                              BddOregJusticeEmptinessBddAlgorithmType alg)
{
  const char* str = Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string(alg);

  boolean res = OptsHandler_set_enum_option_value(opt,
                                                  OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM, str);
  nusmv_assert(res);
}

void reset_oreg_justice_emptiness_bdd_algorithm(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_reset_option_value(opt,
                                               OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM);
  nusmv_assert(res);
}

void set_use_coi_size_sorting(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  USE_COI_SIZE_SORTING, true);
  nusmv_assert(res);
}

void unset_use_coi_size_sorting(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  USE_COI_SIZE_SORTING, false);
  nusmv_assert(res);
}

boolean opt_use_coi_size_sorting(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, USE_COI_SIZE_SORTING);
}


void opt_disable_syntactic_checks(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  DISABLE_SYNTACTIC_CHECKS,
                                                  true);
  nusmv_assert(res);
}

void opt_enable_syntactic_checks(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  DISABLE_SYNTACTIC_CHECKS,
                                                  false);
  nusmv_assert(res);
}

boolean opt_syntactic_checks_disabled(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, DISABLE_SYNTACTIC_CHECKS);
}

void set_keep_single_value_vars(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  KEEP_SINGLE_VALUE_VARS,
                                                  true);
  nusmv_assert(res);
}

void unset_keep_single_value_vars(OptsHandler_ptr opt)
{
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  KEEP_SINGLE_VALUE_VARS,
                                                  false);
  nusmv_assert(res);
}

boolean opt_keep_single_value_vars(OptsHandler_ptr opt)
{
  return OptsHandler_get_bool_option_value(opt, KEEP_SINGLE_VALUE_VARS);
}


boolean opt_get_quiet_mode(OptsHandler_ptr opt) {
  return OptsHandler_get_bool_option_value(opt, QUIET_MODE);
}

void set_quiet_mode(OptsHandler_ptr opt) {
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  QUIET_MODE,
                                                  true);
  nusmv_assert(res);
}

void unset_quiet_mode(OptsHandler_ptr opt) {
  boolean res = OptsHandler_set_bool_option_value(opt,
                                                  QUIET_MODE,
                                                  false);
  nusmv_assert(res);
}


void set_default_simulation_steps(OptsHandler_ptr opt, int val)
{
  boolean res =
    OptsHandler_set_int_option_value(opt, DEFAULT_SIMULATION_STEPS, val);
  nusmv_assert(res);
}

void reset_default_simulation_steps(OptsHandler_ptr opt)
{
  boolean res =
    OptsHandler_reset_option_value(opt, DEFAULT_SIMULATION_STEPS);
  nusmv_assert(res);
}

int get_default_simulation_steps(OptsHandler_ptr opt)
{
  return OptsHandler_get_int_option_value(opt, DEFAULT_SIMULATION_STEPS);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis          [Sets an environment variable]

   CommandName       [set]

   CommandSynopsis   [Sets an environment variable]

   CommandArguments  [\[-h\] \[&lt;name&gt;\] \[&lt;value&gt;\]]

   CommandDescription [ A variable environment is maintained by the command
   interpreter.
   The "set" command sets a variable to a particular value, and the
   "unset" command removes the definition of a variable.
   If "set" is given no arguments, it prints the current value of all variables.<p>

   Command options:<p>
   <dl> <dt> -h
   <dd> Prints the command usage.
   </dl>
   <dl> <dt> &lt;name&gt;
   <dd> Variable name
   </dl>
   <dl> <dt> &lt;value&gt;
   <dd> Value to be assigned to the variable.
   </dl>

   <p>
   Interpolation of variables is allowed when using the set command. The
   variables are referred to with the prefix of '$'. So for example,
   what follows can be done to check the value of a set variable:<br>
   <code>
   NuSMV> set foo bar<br>
   NuSMV> echo $foo<br>
   bar <br>
   </code>

   The last line "bar" will be the output produced by NuSMV.<p>

   Variables can be extended by using the character ':' to concatenate
   values. For example: <br>
   <code>
   NuSMV> set foo bar<br>
   NuSMV> set foo $foo:foobar<br>
   NuSMV> echo $foo<br>
   bar:foobar<br>
   </code>
   The variable <code> foo </code> is extended with the value <code>
   foobar </code>. <p>

   Whitespace characters may be present within quotes. However, variable
   interpolation lays the restriction that the characters ':' and '/' may
   not be used within quotes. This is to allow for recursive interpolation.
   So for example, the following is allowed<br>
   <code>
   NuSMV> set "foo bar" this<br>
   NuSMV> echo $"foo bar"<br>
   this <br>
   </code>
   The last line will be the output produced by NuSMV. <br>
   But in the following, the  value of the variable <code> foo/bar </code>
   will not be interpreted correctly:<p>
   <code>
   NuSMV> set "foo/bar" this<br>
   NuSMV> echo $"foo/bar"<br>
   foo/bar<br>
   </code>
   If a variable is not set by the "set" command, then the variable is returned
   unchanged.
   <p>

   Different commands use environment information for different purposes.
   The command interpreter makes use of the following parameters:<p>

   <dl>
   <dt><b>autoexec</b>
   <dd>     Defines a command string to be automatically executed after every
   command processed by the command interpreter.
   This is useful for things like timing commands, or tracing the
   progress of optimization.
   </dl>


   <dl><dt><b>open_path</b>
   <dd>      "open_path" (in analogy to the shell-variable PATH) is a list of
   colon-separated strings giving directories to be searched whenever
   a file is opened for read.  Typically the current directory (.) is
   the first item in this list. The standard system library (typically
   $NuSMV_LIBRARY_PATH) is always implicitly appended to the current
   path.
   This provides a convenient short-hand mechanism for reaching
   standard library files.
   </dl>
   <dl><dt> <b>nusmv_stderr </b>
   <dd>   Standard error (normally stderr) can be re-directed to a file
   by setting the variable nusmv_stderr.
   </dl>

   <dl><dt>  <b>nusmv_stdout</b>
   <dd>           Standard output (normally stdout) can be re-directed to a file
   by setting the variable nusmv_stdout.
   </dl>
   ]

   SideEffects        []

   SeeAlso            [unset]

******************************************************************************/
int CommandSetVariable(int  argc, char** argv)
{
  char *flag_value, *key;
  int c;
  boolean has_param = false;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch(c) {
    case 'h':
      goto usage;
      break;
    default:
      goto usage;
    }
  }
  if (argc == 0 || argc > 3) {
    goto usage ;
  }
  else if (argc == 1) {
    OptsHandler_print_all_options(OptsHandler_get_instance(),
                                  nusmv_stdout, false);
    return 0;
  }
  else {
    key = util_strsav(argv[1]);

    flag_value = argc == 2 ? util_strsav("") : util_strsav(argv[2]);
    has_param = (argc == 3);

    if (strcmp(argv[1], "nusmv_stdout") == 0) {
      if (nusmv_stdout != stdout) {
        (void) fclose(nusmv_stdout);
      }
      if (strcmp(flag_value, "") == 0) {
	FREE(flag_value);
        flag_value = util_strsav("-");
      }
      nusmv_stdout = Cmd_FileOpen(flag_value, "w", NIL(char*), 0);
      if (nusmv_stdout == NULL) {
        nusmv_stdout = stdout;
      }
#if NUSMV_HAVE_SETVBUF
# if SETVBUF_REVERSED
      setvbuf(nusmv_stdout, _IOLBF, (char*) NULL, 0);
# else
      setvbuf(nusmv_stdout, (char*) NULL, _IOLBF, 0);
# endif
#endif
    }
    if (strcmp(argv[1], "nusmv_stderr") == 0) {
      if (nusmv_stderr != stderr) {
        (void) fclose(nusmv_stderr);
      }
      if (strcmp(flag_value, "") == 0) {
	FREE(flag_value);
        flag_value = util_strsav("-");
      }
      nusmv_stderr = Cmd_FileOpen(flag_value, "w", NIL(char*), 0);
      if (nusmv_stderr == NULL) {
        nusmv_stderr = stderr;
      }
#if NUSMV_HAVE_SETVBUF
# if SETVBUF_REVERSED
      setvbuf(nusmv_stderr, _IOLBF, (char*) NULL, 0);
# else
      setvbuf(nusmv_stderr, (char*) NULL, _IOLBF, 0);
# endif
#endif
    }
    if (strcmp(argv[1], "history") == 0) {
      if (nusmv_historyFile != NIL(FILE)) {
        (void) fclose(nusmv_historyFile);
      }
      if (strcmp(flag_value, "") == 0) {
        nusmv_historyFile = NIL(FILE);
      }
      else {
        nusmv_historyFile = Cmd_FileOpen(flag_value, "w", NIL(char*), 0);
        if (nusmv_historyFile == NULL) {
          nusmv_historyFile = NIL(FILE);
        }
      }
    }

    /* Add triggers that should only be enabled when using the set
       function (ie: internal uses of the option may do things that
       the user CANNOT!) */
    OptsHandler_add_option_trigger(OptsHandler_get_instance(), INPUT_FILE,
                                   opt_input_file_trigger);

    if (OptsHandler_is_option_registered(OptsHandler_get_instance(), key)) {
      if (OptsHandler_is_option_public(OptsHandler_get_instance(), key)) {

        /* Handle boolean options. These do not need explicitly a flag value. */
        if (!has_param) {
          if (OptsHandler_is_bool_option(OptsHandler_get_instance(), key)) {
            OptsHandler_set_bool_option_value(OptsHandler_get_instance(),
                                              key, true);
          }
          else {
            fprintf(nusmv_stderr,
                    "Please provide a value for option \"%s\"\n", key);
          }
        }
        /* A value has been provided */
        else {
          boolean res = OptsHandler_set_option_value(OptsHandler_get_instance(),
                                                     key, flag_value);
          if (!res) {
            /* If possible values are known, print them */
            if (OptsHandler_is_enum_option(OptsHandler_get_instance(), key) ||
                OptsHandler_is_bool_option(OptsHandler_get_instance(), key)) {
              int i, num;
              char** values;

              OptsHandler_get_enum_option_values(OptsHandler_get_instance(),
                                                 key, &values, &num);

              fprintf(nusmv_stderr, "Possible values are: \"");
              for (i = 0; i < num; ++i) {
                fprintf(nusmv_stderr, "%s%s", values[i],
                        (i == (num - 1) ? "" : " "));
                FREE(values[i]);
              }
              fprintf(nusmv_stderr, "\"\n");
              FREE(values);
            } /* Enum possibilities printing */
            else if (OptsHandler_is_int_option(OptsHandler_get_instance(), key)) {
              fprintf(nusmv_stderr, "The option requires an integer argument\n");
            }

            fprintf(nusmv_stderr,
                    "Cannot assign value \"%s\" to option \"%s\"\n",
                    flag_value, key);

            FREE(flag_value);
            FREE(key);
            return 1;
          } /* Error in setting the new option value */
        } /* Option value has been provided */
      }
      /* Private option, print an error message */
      else {
        fprintf(nusmv_stderr,
                "Option \"%s\" is private. Cannot set value for \"%s\"\n",
                key, key);
        FREE(flag_value);
        FREE(key);
        return 1;
      }
    }
    /* Unregistered options are just added as key -> value */
    else {
      fprintf(nusmv_stdout, "Defining new environment variable \"%s\"\n", key);

      /* Promote the new variable to boolean if no parameter is given.. */
      if (!has_param) {
        FREE(flag_value);
        flag_value = util_strsav("1");
      }

      boolean res = OptsHandler_register_user_option(OptsHandler_get_instance(),
                                                     key, flag_value);
      if (!res) {
        fprintf(nusmv_stderr,
                "Some error occurred while registering option \"%s\"\n", key);
        FREE(flag_value);
        FREE(key);
        return 1;
      }
    }

    FREE(flag_value);
    FREE(key);
  }

  OptsHandler_remove_option_trigger(OptsHandler_get_instance(), INPUT_FILE,
                                    opt_input_file_trigger);


  return 0;

 usage:
  (void) printf("usage: set [-h] [name [value]]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}

#if TEST_OPTS_HANDLER
int CommandGenTestOptsHandler(int  argc, char** argv)
{
  char* filename = (char*)NULL;
  int c;
  int ret = 0;
  boolean unset = false;
  FILE* out = nusmv_stdout;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "huo:")) != EOF) {
    switch(c) {
    case 'h':
      goto test_opt_handler_usage;
      break;
    case 'u':
      if (unset) { goto test_opt_handler_usage; }
      unset = true;
      break;
    case 'o':
      if ((char*)NULL != filename) { goto test_opt_handler_usage; }
      filename = util_strsav(util_optarg);
      break;
    default:
      goto test_opt_handler_usage;
    }
  }

  if ((char*)NULL != filename) {
    out = fopen(filename, "w");
    if ((FILE*)NULL == out) {
      fprintf(nusmv_stderr, "Cannot open %s for writing\n", filename);
      ret = 1;
      goto test_opt_handler_free;
    }
  }

  OptsHandler_generate_test(OptsHandler_get_instance(), out, unset);

  fflush(out);
  if ((char*)NULL != filename) {
    fclose(out);
  }

  goto test_opt_handler_free;

 test_opt_handler_usage:
  ret = 1;
  fprintf(nusmv_stderr, "usage: _gen_test_opts_handler: [-h] [-u] [-o filename]\n");
  fprintf(nusmv_stderr, "       -h      : Show help\n");
  fprintf(nusmv_stderr, "       -u      : Generate test of unset command\n");
  fprintf(nusmv_stderr, "       -o file : Save output to file\n");

 test_opt_handler_free:
  if ((char*)NULL != filename) {
    FREE(filename);
  }

  return ret;
}
#endif


/**Function********************************************************************

   Synopsis          [Unsets an environment variable]

   CommandName       [unset]

   CommandSynopsis   [Unsets an environment variable]

   CommandArguments  [\[-h\] &lt;variables&gt;]

   CommandDescription [A variable environment is maintained by the command
   interpreter.
   The "set" command sets a variable to a particular value, and the
   "unset" command removes the definition of a variable. <p>
   Command options:<p>
   <dl><dt> -h
   <dd> Prints the command usage.
   </dl>
   <dl><dt> &lt;variables&gt;
   <dd> Variables to be unset
   </dl>
   ]

   SideEffects        []

   SeeAlso            [set]

******************************************************************************/
int CommandUnsetVariable(int  argc, char** argv)
{
  int i;
  char *key;
  int c;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "h")) != EOF) {
    switch(c) {
    case 'h':
      goto usage;
      break;
    default:
      goto usage;
    }
  }

  if (argc < 2) {
    goto usage;
  }

  for (i = 1; i < argc; ++i) {
    key = util_strsav(argv[i]);
    if (OptsHandler_is_option_registered(OptsHandler_get_instance(), key)) {
      /* Reset the option value. If the variable is user defined, just
         remove it! */
      if (OptsHandler_is_user_option(OptsHandler_get_instance(), key)) {
        OptsHandler_unregister_option(OptsHandler_get_instance(), key);
      }
      else {
        /* Unset of boolean options means set it to 0 */
        if (OptsHandler_is_bool_option(OptsHandler_get_instance(), key)) {
          OptsHandler_set_bool_option_value(OptsHandler_get_instance(), key, false);
        }
        /* Unset of other options means reset it to default */
        else {
          OptsHandler_reset_option_value(OptsHandler_get_instance(), key);
        }
      }
    }
    else {
      fprintf(nusmv_stderr, "Warning: Option \"%s\" is not registered\n", key);
    }
    FREE(key);
  }

  return 0;


 usage:
  fprintf(nusmv_stderr, "usage: unset [-h] variables \n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return 1;
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis [Finds all preprocessor names occurring in the given string
   that are not actually available, and returns the set of the only
   available ones]

   Description        [Returned string must be freed]

   SideEffects        []

******************************************************************************/
static char* remove_non_existant_pps(const char* pp_list)
{
  char* paths;
  char* open_path = "";
  char* valid_pps;
  char* pp_list_copy;
  char* pp;
  int len;
  OptsHandler_ptr opt = OptsHandler_get_instance();
  len = strlen(pp_list);

  valid_pps = ALLOC(char, len+1);
  nusmv_assert(valid_pps != (char*) NULL);
  valid_pps[0] = '\0';

  pp_list_copy = ALLOC(char, len+2);
  nusmv_assert(pp_list_copy != (char*) NULL);
  strncpy(pp_list_copy, pp_list, len+1);
  pp_list_copy[len+1] = '\0'; /* additional '\0' for strtok below */

  /* gets an operating system variable PATH and NuSMV's variable open_path */
# if NUSMV_HAVE_GETENV
  paths = getenv("PATH");
# else
  paths = "."; /* default is the current dir */
# endif

  if (OptsHandler_is_option_registered(opt, "open_path")) {
    open_path = OptsHandler_get_string_option_value(opt, "open_path");
  }

  pp = strtok(pp_list_copy, " \t\n\r");
  while (pp != (char*) NULL) {
    char* pp_filename;

    pp_filename = get_preprocessor_filename(pp);
    if ((pp_filename != (char*) NULL) &&
        ( Utils_file_exists_in_paths(pp_filename, open_path, ":;") ||
          Utils_file_exists_in_paths(pp_filename, paths, ":;") )) {

      if (valid_pps[0] != '\0') strcat(valid_pps, " ");
      strcat(valid_pps, pp);
    }
    /* "strtok" is not safe and can be changed by other functions.
       "pp + its length" is the pointer to the next string to be parsed.
    */
    pp = pp + strlen(pp) + 1; /* 1 is used to pass past the current '\0' */
    pp = strtok(pp, " \t\n\r");
  } /* while loop */

  FREE(pp_list_copy);
  return valid_pps;
}

/**Function********************************************************************

   Synopsis    [Convert the FB_Heuristic to it's string representation]

   Description [Convert the FB_Heuristic to it's string representation]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static const char* opt_check_invar_fb_heuristic_to_string (FB_Heuristic h)
{
  switch (h) {
  case ZIGZAG_HEURISTIC:
    return "zigzag";
  case SMALLEST_BDD_HEURISTIC:
    return "smallest";
  }
  error_unreachable_code();
}

/**Function********************************************************************

   Synopsis    [Convert the Bdd2bmc_Heuristic to it's string representation]

   Description [Convert the Bdd2bmc_Heuristic to it's string representation]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static const char* opt_check_invar_bddbmc_heuristic_to_string (Bdd2bmc_Heuristic h)
{
  switch (h) {
  case STEPS_HEURISTIC:
    return "steps";
  case SIZE_HEURISTIC:
    return "size";
  }
  error_unreachable_code();
}

/**Function********************************************************************

   Synopsis    [Check function for the sat_solver option.]

   Description [Check function for the sat_solver option.]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_check_sat_solver(OptsHandler_ptr opts, const char* val)
{
  boolean result = false;
  const char* name = Sat_NormalizeSatSolverName(val);
  result = ((const char*)NULL != name);

  if (!result) {
    Sat_PrintAvailableSolvers(nusmv_stderr);
  }

  return result;
}

/**Function********************************************************************

   Synopsis    [get function for the sat_solver option.]

   Description [get function for the sat_solver option.]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static void* opt_get_sat_solver(OptsHandler_ptr opts, const char* val)
{
  return (void*)Sat_NormalizeSatSolverName(val);
}

/**Function********************************************************************

   Synopsis           [Get the integer representation of the given string]

   Description        [Get the integer representation of the given string]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void * opt_get_integer(OptsHandler_ptr opts, const char *value)
{
  void* result;
  char * e[1];

  e[0] = "";
  result = (void*)strtol(value, e, 10);
  if (strcmp(e[0], "") != 0) {
    result = OPTS_VALUE_ERROR;
  }
  return result;
}


/**Function********************************************************************

   Synopsis    [Input file check function]

   Description [Input file check function]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_input_file_trigger(OptsHandler_ptr opts, const char* name,
                                      const char* val, Trigger_Action action)
{
  if (ACTION_SET == action && cmp_struct_get_read_model(cmps))  {
    fprintf(nusmv_stdout,
            "***** Warning: a model is already loaded -- input_file not changed.\n");
    fprintf(nusmv_stdout,
            "***** The model should be reset (e.g., using command \"reset\")\n");
    fprintf(nusmv_stdout,
            "***** before the input_file can be changed.\n");
    return false;
  }

  return true;
}

/**Function********************************************************************

   Synopsis    [Check function for the output word format]

   Description [Check function for the output word format]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_check_word_format(OptsHandler_ptr opts, const char* val)
{
  void * tmp = opt_get_integer(opts, val);
  int new_format = PTR_TO_INT(tmp);

  if (tmp != OPTS_VALUE_ERROR) {
    if ((new_format == 2) || (new_format == 8) ||
        (new_format == 10) || (new_format == 16)) {
      return true;
    }
    else {
      fprintf(nusmv_stderr, "Valid values are 2, 8, 10 and 16.\n");
    }
  }
  return false;
}

/**Function********************************************************************

   Synopsis    [Check function for the number of shown states]

   Description [Check function for the number of shown states]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_check_shown_states(OptsHandler_ptr opts, const char* val)
{
  void * tmp = opt_get_integer(opts, val);
  int int_val = PTR_TO_INT(tmp);

  if (tmp != OPTS_VALUE_ERROR) {
    if ((int_val < 1) || (int_val > MAX_SHOWN_STATES)) {
      fprintf(nusmv_stderr, "Number must be 1 <= n <= %d\n", MAX_SHOWN_STATES);
    }
    else {
      return true;
    }
  }

  return false;
}

/**Function********************************************************************

   Synopsis    [Trigger that sets the use_reachable_states flag if needed]

   Description [Trigger that sets the use_reachable_states flag if needed]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_set_reachable_states_trigger(OptsHandler_ptr opts,
                                                const char* opt,
                                                const char* value,
                                                Trigger_Action action)
{
  if (ACTION_SET == action ||
      ACTION_RESET == action) {
    boolean enable = (strcmp(value, OPTS_TRUE_VALUE) == 0);
    boolean res = OptsHandler_set_bool_option_value(opts,
                                                    USE_REACHABLE_STATES,
                                                    enable);
    if (strcmp(opt, FORWARD_SEARCH) != 0) {
      res &= OptsHandler_set_bool_option_value(opts,
                                               FORWARD_SEARCH,
                                               enable);
    }
    nusmv_assert(res);
  }


  return true;
}

/**Function********************************************************************

   Synopsis    [reordering method trigger: enables / disables dd_autodyn]

   Description [reordering method trigger: enables / disables dd_autodyn]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_reorder_method_trigger(OptsHandler_ptr opts,
                                          const char* opt,
                                          const char* value,
                                          Trigger_Action action)
{
  if (ACTION_SET == action) {
    unsigned int method = StringConvertToDynOrderType((char*)value);

    dd_autodyn_enable(dd_manager, method);

    if (!OptsHandler_get_bool_option_value(opts, DYNAMIC_REORDER)) {
      dd_autodyn_disable(dd_manager);
    }
  }

  return true;
}

/**Function********************************************************************

   Synopsis    [Dynamic reordering trigger: enables / disables dd_autodyn]

   Description [Dynamic reordering trigger: enables / disables dd_autodyn]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_dynamic_reorder_trigger(OptsHandler_ptr opts,
                                           const char* opt,
                                           const char* value,
                                           Trigger_Action action)
{
  switch (action) {
  case ACTION_SET:
    {
      int current_ordering_method =
        OptsHandler_get_int_option_value(opts, REORDER_METHOD);
      dd_autodyn_enable(dd_manager, current_ordering_method);
      break;
    }
  case ACTION_RESET:
    dd_autodyn_disable(dd_manager);
    break;
  default: break;
  }

  return true;
}

/**Function********************************************************************

   Synopsis    [Trigger for the default_trace_plugin option. ]

   Description [Trigger for the default_trace_plugin option: Updates the
   default plugin in the trace pkg.]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_trace_plugin_trigger(OptsHandler_ptr opts,
                                        const char* opt,
                                        const char* value,
                                        Trigger_Action action)
{
  int plug = PTR_TO_INT(opt_get_integer(opts, value));

  if (ACTION_SET == action) {
    return TracePkg_set_default_trace_plugin(plug);
  }

  return true;
}

/**Function********************************************************************

   Synopsis    [Trigger function for the trans_order_file option]

   Description [Trigger function for the trans_order_file option:
   Enables/disables AFFINITY_CLUSTERING if needed]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_trans_order_file_trigger(OptsHandler_ptr opts,
                                            const char* opt,
                                            const char* value,
                                            Trigger_Action action)
{
  switch (action) {
  case ACTION_SET:
  case ACTION_RESET:
    /* without a transition order there is a need for affinity clustering */
    /* with a transition order there is no need for affinity clustering */
    OptsHandler_set_bool_option_value(opts, AFFINITY_CLUSTERING,
                                      (const char*)NULL == value);
    break;
  default: break;
  }

  return true;
}

/**Function********************************************************************

   Synopsis    [Trigger function for the run_cpp option]

   Description [Trigger function for the run_cpp option: Tells that the
   option is deprecated. No side-effect on the option
   value will be performed]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_run_cpp_trigger(OptsHandler_ptr opts,
                                   const char* opt,
                                   const char* value,
                                   Trigger_Action action)
{
  switch (action) {
  case ACTION_SET:
  case ACTION_RESET:
    fprintf(nusmv_stderr,
            "Error: the \"%s\" option is no longer supported", RUN_CPP);
    fprintf(nusmv_stderr, " - use \"%s cpp\" instead.\n", PP_LIST);
    return false;
  default: break;
  }
  return true;
}

/**Function********************************************************************

   Synopsis    [Trigger function for the pp_list option]

   Description [Trigger function for the pp_list option. Checks that
                the given list of preprocessors is valid or not]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_pp_list_trigger(OptsHandler_ptr opts,
                                   const char* opt,
                                   const char* value,
                                   Trigger_Action action)
{
  switch (action) {
  case ACTION_SET:
  case ACTION_RESET:
    {
      char* stripped = remove_non_existant_pps(value);
      if (strcmp(stripped, value) != 0) {
        fprintf(nusmv_stderr, "Some of the specified preprocessors "
                "does not exist\n");
        return false;
      }
    }
  default: break;
  }
  return true;
}

/**Function********************************************************************

   Synopsis    [Trigger function for the run_cpp option]

   Description [Trigger function for the run_cpp option: Tells that the
   option is deprecated. No side-effect on the option
   value will be performed]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_rbc_inlining_lazy_trigger(OptsHandler_ptr opts,
                                             const char* opt,
                                             const char* value,
                                             Trigger_Action action)
{
  switch (action) {
  case ACTION_SET:
    fprintf(nusmv_stderr, "Warning: setting of variable rbc_inlining_lazy"
            "is not currently allowed\n");
    return false;
  default: break;
  }
  return true;
}

#if NUSMV_HAVE_REGEX_H
/**Function********************************************************************

   Synopsis    [Trigger function for the traces_regexp option]

   Description [Trigger function for the counter_examples_show_re
   option: tries to compile regexp pattern and rejects it if
   compilation fails.]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static boolean opt_traces_regexp_trigger(OptsHandler_ptr opts, const char* opt,
                                         const char* value, Trigger_Action act)
{
  boolean res = true; /* no error by default */
  switch (act) {
  case ACTION_SET:
    {
      regex_t re;
      int cr;

      nusmv_assert(NIL(char) != value);
      cr = regcomp(&re, value, REG_NOSUB);

      if (0 != cr) {
        size_t mlen = regerror(cr, &re, NULL, 0);
        char* buf = ALLOC(char, mlen+1);
        regerror (cr, &re, buf, mlen);

        fprintf(nusmv_stderr,
                "Error: could not parse '%s' (%s)\n", value, buf);

        FREE(buf);
        res = false ;
      }

      regfree(&re);
    }
  default: break;
  }

  return res;
}
#endif

static boolean opt_script_file_trigger(OptsHandler_ptr opts,
                                       const char* opt,
                                       const char* value,
                                       Trigger_Action action)
{
  boolean res = true;

  /* The script file option can be only set once, with the option
     -load or -source. No need for checking whether we are
     setting/unsetting/resetting the option */

  unset_batch(opts);

  return res;
}
