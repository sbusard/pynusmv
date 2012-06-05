/**CHeaderFile*****************************************************************

  FileName    [opt.h]

  PackageName [opt]

  Synopsis    [The option header file.]

  Description [This file conatins a data structure to manage all the
  command line options of the NuSMV system.]

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

  Revision    [$Id: opt.h,v 1.36.4.10.2.2.2.24.4.38 2010-02-12 16:25:47 nusmv Exp $]

******************************************************************************/

#ifndef __OPT_H__
#define __OPT_H__

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#if NUSMV_HAVE_REGEX_H
# include <regex.h>
#endif

#include "utils/utils.h"
#include "sat/sat.h" /* for SAT Solver */
#include "trans/trans.h" /* for TransType */
#include "enc/enc.h" /* for VarsOrderType and BddSohEnum*/
#include "fsm/bdd/bdd.h" /* for BddOregJusticeEmptinessBddAlgorithmType */
#include "rbc/rbc.h" /* For RBC2CNF algorithms */
#include "opt/OptsHandler.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* Opts names */
#define DEFAULT_PGM_NAME   NUSMV_PACKAGE_NAME
#define DEFAULT_PGM_PATH   (char *)NULL
#define DEFAULT_INPUT_FILE (char *)NULL
#define DEFAULT_INPUT_ORDER_FILE (char *)NULL
#define DEFAULT_OUTPUT_ORDER_FILE "temp.ord"
#define DEFAULT_TRANS_ORDER_FILE (char *)NULL

/* outputs warning instead of errors in type checking */
#define DEFAULT_BACKWARD_COMPATIBILITY false
/* allows warning messages to be printed during type checking */
#define DEFAULT_TYPE_CHECKING_WARNING_ON true
#define DEFAULT_CONJ_PART_THRESHOLD 1000
#define DEFAULT_IMAGE_CLUSTER_SIZE 1000
#define DEFAULT_SHOWN_STATES 25
/* maximum number of states shown during an interactive simulation step*/
#define MAX_SHOWN_STATES 65535

#if NUSMV_HAVE_SOLVER_MINISAT
#define DEFAULT_SAT_SOLVER        "MiniSat"
#else
#if NUSMV_HAVE_SOLVER_ZCHAFF
#define DEFAULT_SAT_SOLVER        "zchaff"
#else
#define DEFAULT_SAT_SOLVER        (char*)NULL
#endif
#endif

#define OPT_USER_POV_NULL_STRING  "" /* user pov of the null string */

#define DEFAULT_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM \
  BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD

#define DEFAULT_SHOW_DEFINES_IN_TRACES true

#define DEFAULT_SHOW_DEFINES_WITH_NEXT true

#define DEFAULT_USE_COI_SIZE_SORTING true

#define DEFAULT_BDD_ENCODE_WORD_BITS true

typedef enum {
  FORWARD,
  BACKWARD,
  FORWARD_BACKWARD,
  BDD_BMC
} Check_Strategy;

#define DEFAULT_INVAR_CHECK_STRATEGY FORWARD

typedef enum {
  ZIGZAG_HEURISTIC,
  SMALLEST_BDD_HEURISTIC
} FB_Heuristic;
#define DEFAULT_FORWARD_BACKWARD_ANALYSIS_HEURISTIC ZIGZAG_HEURISTIC


typedef enum {
  STEPS_HEURISTIC,
  SIZE_HEURISTIC
} Bdd2bmc_Heuristic;
#define DEFAULT_BDD2BMC_HEURISTIC STEPS_HEURISTIC

#define DEFAULT_DAGGIFIER_ENABLED true

#define DEFAULT_DAGGIFIER_COUNTER_THS 3

#define DEFAULT_DAGGIFIER_DEPTH_THS 2

#define DEFAULT_BDD2BMC_HEURISTIC_THRESHOLD 10

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct options_TAG*  options_ptr;


EXTERN void set_use_reachable_states ARGS((OptsHandler_ptr));
EXTERN void unset_use_reachable_states ARGS((OptsHandler_ptr));
EXTERN boolean opt_use_reachable_states ARGS((OptsHandler_ptr));

EXTERN void set_use_ltl_tableau_reachable_states ARGS((OptsHandler_ptr));
EXTERN void unset_use_ltl_tableau_reachable_states ARGS((OptsHandler_ptr));
EXTERN boolean opt_use_ltl_tableau_reachable_states ARGS((OptsHandler_ptr));

EXTERN void set_use_fair_states ARGS((OptsHandler_ptr));
EXTERN void unset_use_fair_states ARGS((OptsHandler_ptr));
EXTERN boolean opt_use_fair_states ARGS((OptsHandler_ptr));


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void init_options ARGS((void));
EXTERN void deinit_options ARGS((void));
EXTERN void init_options_cmd ARGS((void));

EXTERN void    set_pgm_name ARGS((OptsHandler_ptr, char *));
EXTERN void    reset_pgm_name ARGS((OptsHandler_ptr));
EXTERN char *  get_pgm_name ARGS((OptsHandler_ptr));
EXTERN void    set_script_file ARGS((OptsHandler_ptr, char *));
EXTERN void    reset_script_file ARGS((OptsHandler_ptr));
EXTERN char *  get_script_file ARGS((OptsHandler_ptr));
EXTERN void    set_pgm_path ARGS((OptsHandler_ptr, char *));
EXTERN void    reset_pgm_path ARGS((OptsHandler_ptr));
EXTERN char *  get_pgm_path ARGS((OptsHandler_ptr));
EXTERN void    set_input_file ARGS((OptsHandler_ptr, char *));
EXTERN void    reset_input_file ARGS((OptsHandler_ptr));
EXTERN char *  get_input_file ARGS((OptsHandler_ptr));
EXTERN void    set_input_order_file ARGS((OptsHandler_ptr, char *));
EXTERN void    reset_input_order_file ARGS((OptsHandler_ptr));
EXTERN char *  get_input_order_file ARGS((OptsHandler_ptr));
EXTERN void    set_output_order_file ARGS((OptsHandler_ptr, char *));
EXTERN void    reset_output_order_file ARGS((OptsHandler_ptr));
EXTERN char *  get_output_order_file ARGS((OptsHandler_ptr));
EXTERN boolean is_default_order_file ARGS((OptsHandler_ptr opt));
EXTERN void    set_trans_order_file ARGS((OptsHandler_ptr, char *));
EXTERN void    reset_trans_order_file ARGS((OptsHandler_ptr));
EXTERN char *  get_trans_order_file ARGS((OptsHandler_ptr));
EXTERN boolean opt_trans_order_file ARGS((OptsHandler_ptr));
EXTERN void    set_output_flatten_model_file ARGS((OptsHandler_ptr, char *));
EXTERN void    reset_output_flatten_model_file ARGS((OptsHandler_ptr));
EXTERN char *  get_output_flatten_model_file ARGS((OptsHandler_ptr));
EXTERN void    set_output_boolean_model_file ARGS((OptsHandler_ptr, char *));
EXTERN void    reset_output_boolean_model_file ARGS((OptsHandler_ptr));
EXTERN char *  get_output_boolean_model_file ARGS((OptsHandler_ptr));
EXTERN void    set_output_word_format ARGS((OptsHandler_ptr, int i));
EXTERN int     get_output_word_format ARGS((OptsHandler_ptr));
EXTERN void    set_backward_comp ARGS((OptsHandler_ptr));
EXTERN void    unset_backward_comp ARGS((OptsHandler_ptr));
EXTERN boolean opt_backward_comp ARGS((OptsHandler_ptr));
EXTERN void    set_type_checking_warning_on ARGS((OptsHandler_ptr));
EXTERN void    unset_type_checking_warning_on ARGS((OptsHandler_ptr));
EXTERN boolean opt_type_checking_warning_on ARGS((OptsHandler_ptr));
EXTERN void    set_verbose_level ARGS((OptsHandler_ptr, int));
EXTERN int     get_verbose_level ARGS((OptsHandler_ptr));
EXTERN boolean opt_verbose_level_eq ARGS((OptsHandler_ptr, int));
EXTERN boolean opt_verbose_level_gt ARGS((OptsHandler_ptr, int));
EXTERN boolean opt_verbose_level_ge ARGS((OptsHandler_ptr, int));
EXTERN boolean opt_verbose_level_lt ARGS((OptsHandler_ptr, int));
EXTERN boolean opt_verbose_level_le ARGS((OptsHandler_ptr, int));
EXTERN void    set_pp_list ARGS((OptsHandler_ptr, char *));
EXTERN char *  get_pp_list ARGS((OptsHandler_ptr));
EXTERN void    set_shown_states_level ARGS((OptsHandler_ptr, int));
EXTERN int     opt_shown_states_level ARGS((OptsHandler_ptr));
EXTERN void    set_ignore_spec ARGS((OptsHandler_ptr));
EXTERN void    unset_ignore_spec ARGS((OptsHandler_ptr));
EXTERN boolean opt_ignore_spec ARGS((OptsHandler_ptr));
EXTERN void    set_ignore_compute ARGS((OptsHandler_ptr));
EXTERN void    unset_ignore_compute ARGS((OptsHandler_ptr));
EXTERN boolean opt_ignore_compute ARGS((OptsHandler_ptr));
EXTERN void    set_ignore_ltlspec ARGS((OptsHandler_ptr));
EXTERN void    unset_ignore_ltlspec ARGS((OptsHandler_ptr));
EXTERN boolean opt_ignore_ltlspec ARGS((OptsHandler_ptr));
EXTERN void    set_ignore_pslspec ARGS((OptsHandler_ptr));
EXTERN void    unset_ignore_pslspec ARGS((OptsHandler_ptr));
EXTERN boolean opt_ignore_pslspec ARGS((OptsHandler_ptr));
EXTERN void    set_check_fsm ARGS((OptsHandler_ptr));
EXTERN void    unset_check_fsm ARGS((OptsHandler_ptr));
EXTERN boolean opt_check_fsm ARGS((OptsHandler_ptr));
EXTERN void    set_ignore_invar ARGS((OptsHandler_ptr));
EXTERN void    unset_ignore_invar ARGS((OptsHandler_ptr));
EXTERN boolean opt_ignore_invar ARGS((OptsHandler_ptr));
EXTERN void    set_forward_search ARGS((OptsHandler_ptr));
EXTERN void    unset_forward_search ARGS((OptsHandler_ptr));
EXTERN boolean opt_forward_search ARGS((OptsHandler_ptr));
EXTERN void set_ltl_tableau_forward_search ARGS((OptsHandler_ptr opt));
EXTERN void unset_ltl_tableau_forward_search ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_ltl_tableau_forward_search ARGS((OptsHandler_ptr opt));
EXTERN void    set_print_reachable ARGS((OptsHandler_ptr));
EXTERN void    unset_print_reachable ARGS((OptsHandler_ptr));
EXTERN boolean opt_print_reachable ARGS((OptsHandler_ptr));
EXTERN void    set_reorder ARGS((OptsHandler_ptr));
EXTERN void    unset_reorder ARGS((OptsHandler_ptr));
EXTERN boolean opt_reorder ARGS((OptsHandler_ptr));
EXTERN void    set_reorder_method ARGS((OptsHandler_ptr, unsigned int));
EXTERN unsigned int get_reorder_method ARGS((OptsHandler_ptr));
EXTERN void    set_dynamic_reorder ARGS((OptsHandler_ptr));
EXTERN void    unset_dynamic_reorder ARGS((OptsHandler_ptr));
EXTERN boolean opt_dynamic_reorder ARGS((OptsHandler_ptr));
EXTERN void    set_enable_sexp2bdd_caching ARGS((OptsHandler_ptr));
EXTERN void    unset_enable_sexp2bdd_caching ARGS((OptsHandler_ptr));
EXTERN boolean opt_enable_sexp2bdd_caching ARGS((OptsHandler_ptr));
EXTERN void    set_batch ARGS((OptsHandler_ptr));
EXTERN void    unset_batch ARGS((OptsHandler_ptr));
EXTERN boolean opt_batch ARGS((OptsHandler_ptr));
EXTERN void set_partition_method ARGS((OptsHandler_ptr, const TransType));
EXTERN TransType get_partition_method ARGS((OptsHandler_ptr opt));
EXTERN void    reset_partitioning_method ARGS((OptsHandler_ptr));
EXTERN void    set_monolithic ARGS((OptsHandler_ptr));
EXTERN void    set_conj_partitioning ARGS((OptsHandler_ptr));
EXTERN void    set_iwls95cp_partitioning ARGS((OptsHandler_ptr));
EXTERN boolean opt_monolithic ARGS((OptsHandler_ptr));
EXTERN boolean opt_conj_partitioning ARGS((OptsHandler_ptr));
EXTERN boolean opt_iwls95cp_partitioning ARGS((OptsHandler_ptr));
EXTERN void    set_conj_part_threshold ARGS((OptsHandler_ptr, int));
EXTERN void    reset_conj_part_threshold ARGS((OptsHandler_ptr));
EXTERN int     get_conj_part_threshold ARGS((OptsHandler_ptr));
EXTERN void set_image_cluster_size ARGS((OptsHandler_ptr, int));
EXTERN void reset_image_cluster_size ARGS((OptsHandler_ptr));
EXTERN int get_image_cluster_size ARGS((OptsHandler_ptr));
EXTERN void    set_ignore_init_file ARGS((OptsHandler_ptr));
EXTERN void    unset_ignore_init_file ARGS((OptsHandler_ptr));
EXTERN boolean opt_ignore_init_file ARGS((OptsHandler_ptr));
EXTERN void    set_ag_only ARGS((OptsHandler_ptr));
EXTERN void    unset_ag_only ARGS((OptsHandler_ptr));
EXTERN boolean opt_ag_only ARGS((OptsHandler_ptr));
EXTERN void    set_cone_of_influence ARGS((OptsHandler_ptr));
EXTERN void    unset_cone_of_influence ARGS((OptsHandler_ptr));
EXTERN boolean opt_cone_of_influence ARGS((OptsHandler_ptr));
EXTERN void    set_list_properties ARGS((OptsHandler_ptr));
EXTERN void    unset_list_properties ARGS((OptsHandler_ptr));
EXTERN boolean opt_list_properties ARGS((OptsHandler_ptr));


EXTERN void set_prop_print_method ARGS((OptsHandler_ptr opt, int threshold));
EXTERN void reset_prop_print_method ARGS((OptsHandler_ptr opt));
EXTERN int get_prop_print_method ARGS((OptsHandler_ptr opt));

EXTERN void    set_prop_no ARGS((OptsHandler_ptr, int n));
EXTERN int     get_prop_no ARGS((OptsHandler_ptr));
EXTERN void print_partition_method  ARGS((FILE *));
EXTERN void set_sat_solver ARGS((OptsHandler_ptr, const char*));
EXTERN const char* get_sat_solver ARGS((OptsHandler_ptr));
EXTERN boolean set_default_trace_plugin ARGS((OptsHandler_ptr opt, int plugin));
EXTERN int get_default_trace_plugin ARGS((OptsHandler_ptr opt));
EXTERN void set_iwls95_preorder ARGS((OptsHandler_ptr opt));
EXTERN void unset_iwls95_preorder ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_iwls95_preorder ARGS((OptsHandler_ptr opt));
EXTERN void set_affinity ARGS((OptsHandler_ptr));
EXTERN void unset_affinity ARGS((OptsHandler_ptr));
EXTERN boolean opt_affinity ARGS((OptsHandler_ptr));
EXTERN void set_append_clusters ARGS((OptsHandler_ptr));
EXTERN void unset_append_clusters ARGS((OptsHandler_ptr));
EXTERN boolean opt_append_clusters ARGS((OptsHandler_ptr));

/* counter examples */
EXTERN void set_counter_examples ARGS((OptsHandler_ptr));
EXTERN void unset_counter_examples ARGS((OptsHandler_ptr));
EXTERN boolean opt_counter_examples ARGS((OptsHandler_ptr));

EXTERN void set_traces_hiding_prefix ARGS((OptsHandler_ptr, const char*));
EXTERN const char* opt_traces_hiding_prefix ARGS((OptsHandler_ptr));

EXTERN void set_bdd_encoding_word_bits ARGS((OptsHandler_ptr opt));
EXTERN void unset_bdd_encoding_word_bits ARGS((OptsHandler_ptr opt));
EXTERN void reset_bdd_encoding_word_bits ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_bdd_encoding_word_bits ARGS((OptsHandler_ptr opt));

#if NUSMV_HAVE_REGEX_H
EXTERN const char* opt_traces_regexp ARGS((OptsHandler_ptr));
EXTERN boolean set_traces_regexp ARGS((OptsHandler_ptr, const char*));

EXTERN boolean set_traces_show_regexp ARGS((OptsHandler_ptr, const char*));
EXTERN const char* opt_traces_show_regexp ARGS((OptsHandler_ptr));
#endif

/* others */
EXTERN void set_on_failure_script_quits ARGS((OptsHandler_ptr));
EXTERN void unset_on_failure_script_quits ARGS((OptsHandler_ptr));
EXTERN boolean opt_on_failure_script_quits ARGS((OptsHandler_ptr));
EXTERN void set_write_order_dumps_bits ARGS((OptsHandler_ptr));
EXTERN void unset_write_order_dumps_bits ARGS((OptsHandler_ptr));
EXTERN boolean opt_write_order_dumps_bits ARGS((OptsHandler_ptr));

EXTERN void unset_use_ansi_c_div_op ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_use_ansi_c_div_op ARGS((OptsHandler_ptr opt));

EXTERN void set_vars_order_type ARGS((OptsHandler_ptr, VarsOrdType));
EXTERN VarsOrdType get_vars_order_type ARGS((OptsHandler_ptr));

EXTERN void set_bdd_static_order_heuristics ARGS((OptsHandler_ptr, BddSohEnum value));
EXTERN BddSohEnum get_bdd_static_order_heuristics ARGS((OptsHandler_ptr));

/* inlining */
EXTERN void set_symb_inlining ARGS((OptsHandler_ptr opt));
EXTERN void unset_symb_inlining ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_symb_inlining ARGS((OptsHandler_ptr opt));

EXTERN void set_rbc_inlining ARGS((OptsHandler_ptr opt));
EXTERN void unset_rbc_inlining ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_rbc_inlining ARGS((OptsHandler_ptr opt));

EXTERN void set_rbc_inlining_lazy ARGS((OptsHandler_ptr opt));
EXTERN void unset_rbc_inlining_lazy ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_rbc_inlining_lazy ARGS((OptsHandler_ptr opt));

EXTERN void set_use_coi_size_sorting ARGS((OptsHandler_ptr opt));
EXTERN void unset_use_coi_size_sorting ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_use_coi_size_sorting ARGS((OptsHandler_ptr opt));

EXTERN void opt_disable_syntactic_checks ARGS((OptsHandler_ptr opt));
EXTERN void opt_enable_syntactic_checks ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_syntactic_checks_disabled ARGS((OptsHandler_ptr opt));

EXTERN void set_keep_single_value_vars ARGS((OptsHandler_ptr opt));
EXTERN void unset_keep_single_value_vars ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_keep_single_value_vars ARGS((OptsHandler_ptr opt));

EXTERN void set_show_defines_in_traces ARGS((OptsHandler_ptr opt));
EXTERN void unset_show_defines_in_traces ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_show_defines_in_traces ARGS((OptsHandler_ptr opt));

EXTERN void set_show_defines_with_next ARGS((OptsHandler_ptr opt));
EXTERN void unset_show_defines_with_next ARGS((OptsHandler_ptr opt));
EXTERN boolean opt_show_defines_with_next ARGS((OptsHandler_ptr opt));

EXTERN void
set_check_invar_strategy ARGS((OptsHandler_ptr opt, Check_Strategy strategy));
EXTERN Check_Strategy opt_check_invar_strategy ARGS((OptsHandler_ptr opt));
EXTERN const char* opt_check_invar_strategy_as_string ARGS((OptsHandler_ptr opt));

EXTERN void
set_check_invar_fb_heuristic ARGS((OptsHandler_ptr opt, FB_Heuristic strategy));
EXTERN FB_Heuristic opt_check_invar_fb_heuristic ARGS((OptsHandler_ptr opt));
EXTERN
const char* opt_check_invar_fb_heuristic_as_string ARGS((OptsHandler_ptr opt));

EXTERN void
set_check_invar_bddbmc_heuristic ARGS((OptsHandler_ptr opt,
                                       Bdd2bmc_Heuristic strategy));
EXTERN Bdd2bmc_Heuristic
opt_check_invar_bddbmc_heuristic ARGS((OptsHandler_ptr opt));
EXTERN const char*
opt_check_invar_bddbmc_heuristic_as_string ARGS((OptsHandler_ptr opt));

EXTERN void
set_check_invar_bddbmc_heuristic_threshold ARGS((OptsHandler_ptr opt, int t));
EXTERN int
opt_check_invar_bddbmc_heuristic_threshold ARGS((OptsHandler_ptr opt));

/* Daggifier on/off */
EXTERN boolean opt_is_daggifier_enabled ARGS((OptsHandler_ptr opt));
EXTERN void opt_enable_daggifier ARGS((OptsHandler_ptr opt));
EXTERN void opt_disable_daggifier ARGS((OptsHandler_ptr opt));

EXTERN int opt_get_daggifier_counter_threshold ARGS((OptsHandler_ptr opt));
EXTERN void opt_set_daggifier_counter_threshold ARGS((OptsHandler_ptr opt,
                                                      int x));

EXTERN int opt_get_daggifier_depth_threshold ARGS((OptsHandler_ptr opt));
EXTERN void opt_set_daggifier_depth_threshold ARGS((OptsHandler_ptr opt,
                                                    int x));

EXTERN boolean opt_get_quiet_mode ARGS((OptsHandler_ptr opt));
EXTERN void set_quiet_mode ARGS((OptsHandler_ptr opt));
EXTERN void unset_quiet_mode ARGS((OptsHandler_ptr opt));

/* Daggifier statistics */
EXTERN boolean opt_get_daggifier_statistics ARGS((OptsHandler_ptr opt));
EXTERN void set_daggifier_statistics ARGS((OptsHandler_ptr opt));
EXTERN void unset_daggifier_statistics ARGS((OptsHandler_ptr opt));

/* different BDD-based algorithms to check language emptiness for
   omega-regular properties */
EXTERN BddOregJusticeEmptinessBddAlgorithmType \
  get_oreg_justice_emptiness_bdd_algorithm ARGS((OptsHandler_ptr opt));
EXTERN void set_oreg_justice_emptiness_bdd_algorithm \
  ARGS((OptsHandler_ptr opt, BddOregJusticeEmptinessBddAlgorithmType alg));
EXTERN void reset_oreg_justice_emptiness_bdd_algorithm \
  ARGS((OptsHandler_ptr opt));

/* RBC2CNF */
EXTERN void
set_rbc2cnf_algorithm ARGS((OptsHandler_ptr opt, Rbc_2CnfAlgorithm algo));
EXTERN void
unset_rbc2cnf_algorithm ARGS((OptsHandler_ptr opt));
EXTERN Rbc_2CnfAlgorithm
get_rbc2cnf_algorithm ARGS((OptsHandler_ptr opt));

EXTERN void set_default_simulation_steps ARGS((OptsHandler_ptr, int));
EXTERN void reset_default_simulation_steps ARGS((OptsHandler_ptr));
EXTERN int get_default_simulation_steps ARGS((OptsHandler_ptr));

#endif /* __OPT_H__ */
