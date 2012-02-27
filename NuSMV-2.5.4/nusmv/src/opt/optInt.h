/**CHeaderFile*****************************************************************

  FileName    [optInt.h]

  PackageName [opt]

  Synopsis    [The internal header file of the opt package.]

  Description [The internal header file of the opt package.]

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

  Revision    [$Id: optInt.h,v 1.5.4.6.2.2.2.22.4.33 2010-02-12 16:25:47 nusmv Exp $]

******************************************************************************/

#ifndef _OPTINTH
#define _OPTINTH

#include <stdio.h>
#include <limits.h>

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "utils/utils.h"
#include "opt/opt.h"
#include "cinit/cinit.h"
#include "util.h"
#include "node/node.h"
#include "set/set.h"
#include "dd/dd.h"
#include "rbc/rbc.h"
#include "cmd/cmd.h"
#include "compile/compile.h"
#include "prop/Prop.h"
#include "opt/opt.h"
#include "utils/ucmd.h"
#include "trans/trans.h" /* for TransType */
#include "fsm/bdd/bdd.h" /* for BddOregJusticeEmptinessBddAlgorithmType */

/**Macro***********************************************************************

  Synopsis     [Makes append_cluster option visible to the shell interface]

  Description  [Makes append_cluster option visible to the shell
  interface. The default is that it is not visible. Notice that only
  the printing hide it. The set unset function handle it.]

******************************************************************************/
#define APPEND_CLUSTERS_VISIBLE 0


#define PROGRAM_NAME      "program_name"
#define PROGRAM_PATH      "program_path"
#define INPUT_FILE        "input_file"
#define SCRIPT_FILE       "script_file"
#define INPUT_ORDER_FILE  "input_order_file"
#define OUTPUT_ORDER_FILE "output_order_file"
#define TRANS_ORDER_FILE  "trans_order_file"
#define OUTPUT_FLATTEN_MODEL_FILE "output_flatten_model_file"
#define OUTPUT_BOOLEAN_MODEL_FILE "output_boolean_model_file"
#define OUTPUT_WORD_FORMAT "output_word_format"
#define BACKWARD_COMPATIBILITY "backward_compatibility"
#define TYPE_CHECKING_WARNING_ON "type_checking_warning_on"
#define VERBOSE_LEVEL     "verbose_level"
#define RUN_CPP           "run_cpp"
#define PP_LIST           "pp_list"
#define SHOWN_STATES      "shown_states"
#define IGNORE_SPEC       "ignore_spec"
#define IGNORE_COMPUTE    "ignore_compute"
#define IGNORE_LTLSPEC    "ignore_ltlspec"
#define IGNORE_PSLSPEC    "ignore_pslspec"
#define OPT_CHECK_FSM   "check_fsm"
#define IGNORE_INVAR      "ignore_invar"
#define FORWARD_SEARCH    "forward_search"
#define LTL_TABLEAU_FORWARD_SEARCH "ltl_tableau_forward_search"
#define PRINT_REACHABLE   "print_reachable"
#define ENABLE_REORDER    "enable_reorder"
#define REORDER_METHOD    "reorder_method"
#define DYNAMIC_REORDER   "dynamic_reorder"
#define ENABLE_SEXP2BDD_CACHING   "enable_sexp2bdd_caching"
#define PARTITION_METHOD  "partition_method"
#define CONJ_PART_THRESHOLD "conj_part_threshold"
#define IMAGE_CLUSTER_SIZE "image_cluster_size"
#define IGNORE_INIT_FILE  "ignore_init_file"
#define AG_ONLY_SEARCH    "ag_only_search"
#define CONE_OF_INFLUENCE "cone_of_influence"
#define LIST_PROPERTIES "list_properties"
#define PROP_PRINT_METHOD "prop_print_method"
#define PROP_NO         "prop_no"
#define A_SAT_SOLVER "sat_solver"
#define IWLS95_PREORDER  "iwls95preorder"
#define AFFINITY_CLUSTERING  "affinity"
#define APPEND_CLUSTERS  "append_clusters"
#define USE_REACHABLE_STATES  "use_reachable_states"
#define USE_FAIR_STATES  "use_fair_states"
#define COUNTER_EXAMPLES  "counter_examples"
#define TRACES_HIDING_PREFIX  "traces_hiding_prefix"
#define DEFAULT_TRACES_HIDING_PREFIX  "__"
#define BDD_ENCODE_WORD_BITS "bdd_encode_word_bits"

#if NUSMV_HAVE_REGEX_H
#define TRACES_REGEXP  "traces_regexp"
#endif

#define DEFAULT_TRACE_PLUGIN  "default_trace_plugin"
#define ON_FAILURE_SCRIPT_QUITS "on_failure_script_quits"
#define WRITE_ORDER_DUMPS_BITS "write_order_dumps_bits"

#define USE_ANSI_C_DIV_OP "use_ansi_c_div_op"
#define VARS_ORD_TYPE "vars_order_type"
#define BDD_STATIC_ORDER_HEURISTICS   "bdd_static_order_heuristics"
#define RBC_CNF_ALGORITHM "rbc_rbc2cnf_algorithm"
#define SYMB_INLINING "sexp_inlining"
#define RBC_INLINING "rbc_inlining"
#define RBC_INLINING_LAZY "rbc_inlining_lazy"

#define SHOW_DEFINES_IN_TRACES "traces_show_defines"
#define SHOW_DEFINES_WITH_NEXT "traces_show_defines_with_next"

#define INVAR_CHECK_STRATEGY "check_invar_strategy"
#define CHECK_INVAR_FB_HEURISTIC "check_invar_forward_backward_heuristic"
#define CHECK_INVAR_BDDBMC_HEURISTIC "check_invar_bddbmc_heuristic"
#define CHECK_INVAR_BDDBMC_HEURISTIC_THRESHOLD "check_invar_bddbmc_threshold"
#define DAGGIFIER_ENABLED "daggifier_enabled"
#define DAGGIFIER_COUNTER_THRESHOLD "daggifier_counter_threshold"
#define DAGGIFIER_DEPTH_THRESHOLD "daggifier_depth_threshold"
#define DAGGIFIER_STATISTICS "daggifier_statistics"
#define OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM    \
  "oreg_justice_emptiness_bdd_algorithm"
#define USE_COI_SIZE_SORTING "use_coi_size_sorting"
#define BATCH "batch"
#define QUIET_MODE "quiet_mode"
#define DISABLE_SYNTACTIC_CHECKS "disable_syntactic_checks"
#define KEEP_SINGLE_VALUE_VARS "keep_single_value_vars"
#define DEFAULT_SIMULATION_STEPS "default_simulation_steps"

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

extern DdManager * dd_manager;
extern cmp_struct_ptr cmps;

#endif /* _OPTINTH */
