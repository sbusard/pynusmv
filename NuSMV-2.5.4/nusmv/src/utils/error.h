/**CHeaderFile*****************************************************************

  FileName    [error.h]

  PackageName [utils]

  Synopsis    [header for the error.c file]

  Description [header for the error.c file.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2.
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

  Revision    [$Id: error.h,v 1.6.2.13.4.19.4.16 2010-02-16 10:53:49 nusmv Exp $]

******************************************************************************/

#ifndef _ERROR_H
#define _ERROR_H

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "utils/utils.h"
#include "node/node.h"
#include "utils/NodeList.h"
#include "prop/Prop.h"


/*---------------------------------------------------------------------------*/
/* Marco declarations                                                        */
/*---------------------------------------------------------------------------*/

/* Define the alternative for nusmv_assert(0) */
#define error_unreachable_code()                                        \
  internal_error("%s:%d:%s: reached invalid code",                      \
                 __FILE__, __LINE__, __func__)

/* Define the alternative for nusmv_assert(0 && "message") */
#define error_unreachable_code_msg(...)                                 \
  do {                                                                  \
    printf(__VA_ARGS__);                                                \
    internal_error("%s:%d:%s: reached invalid code",                    \
                   __FILE__, __LINE__, __func__);                       \
  } while (0)


/**Macro**********************************************************************
  Synopsis     [Checks if the return value of a snprintf call is
                compatible with the size of the buffer passed as first
                argument to the snprintf function call]

  Description  [Checks if the return value of a snprintf call is
                compatible with the size of the buffer passed as first
                argument to the snprintf function call. An internal
                error is thrown if a buffer overflow is found.

                An example of use:

                  char buf[40];
                  int chars = snprintf(buf, 40, "hello world");
                  SNPRINTF_CHECK(chars, 40); ]

  SideEffects  []
  SeeAlso      [snprintf]
******************************************************************************/
#define SNPRINTF_CHECK(chars, buffsize)                         \
  do {                                                          \
    if (chars >= buffsize) {                                    \
      internal_error("%s:%d:%s: String buffer overflow",        \
                     __FILE__, __LINE__, __func__);             \
    }                                                           \
    else if (chars < 0) {                                       \
      internal_error("%s:%d:%s: Error in buffer writing",       \
                     __FILE__, __LINE__, __func__);             \
    }                                                           \
  } while (0)


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef enum FailureKind_TAG {
  FAILURE_DIV_BY_ZERO,
  FAILURE_CASE_NOT_EXHAUSTIVE,
  FAILURE_ARRAY_OUT_OF_BOUNDS,
  FAILURE_UNSPECIFIED
} FailureKind;



/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void init_the_node ARGS((void));
EXTERN void set_the_node ARGS((node_ptr));
EXTERN node_ptr get_the_node ARGS((void));
EXTERN int io_atom_isempty ARGS((void));
EXTERN void io_atom_push ARGS((node_ptr));
EXTERN void io_atom_pop ARGS((void));
EXTERN node_ptr io_atom_head ARGS((void));
EXTERN void print_io_atom_stack ARGS((FILE * fd));
EXTERN void start_parsing_err ARGS((void));

/* here is an example of how NUSMV_FUNCATTR_NORETURN can be used:*/
EXTERN void nusmv_exit ARGS((int)) NUSMV_FUNCATTR_NORETURN;
EXTERN void rpterr ARGS((const char *, ...)) NUSMV_FUNCATTR_NORETURN;

EXTERN void rpterr_node 
ARGS((node_ptr node, const char * fmt, ...)) NUSMV_FUNCATTR_NORETURN;

EXTERN void internal_error ARGS((const char *, ...)) NUSMV_FUNCATTR_NORETURN;
EXTERN void report_failure_node ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;
EXTERN void warning_failure_node ARGS((node_ptr n));
EXTERN void warning_case_not_exhaustive ARGS((node_ptr));
EXTERN void warning_possible_div_by_zero ARGS((node_ptr failure));
EXTERN void error_div_by_zero ARGS((node_ptr expr)) NUSMV_FUNCATTR_NORETURN;
EXTERN void 
error_div_by_nonconst ARGS((node_ptr expr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void warning_possible_array_out_of_bounds ARGS((node_ptr failure));
EXTERN void error_array_out_of_bounds 
ARGS((int index, int low, int high)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_lhs_of_index_is_not_array 
ARGS((void)) NUSMV_FUNCATTR_NORETURN;

EXTERN void range_error ARGS((node_ptr, node_ptr)) NUSMV_FUNCATTR_NORETURN;
EXTERN void range_warning ARGS((node_ptr, node_ptr));
EXTERN void type_error ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_shadowing ARGS((node_ptr s)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_redefining ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_redefining_operational_symbol 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_redefining_input_var 
ARGS((node_ptr s)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_reassigning ARGS((node_ptr s)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_assign_input_var ARGS((node_ptr s)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_assign_frozen_var ARGS((node_ptr s)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_assign_expected_var
ARGS((node_ptr s)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_ambiguous ARGS((node_ptr s)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_undefined ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_circular ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_too_many_vars ARGS((void)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_not_proper_number 
ARGS((const char* op, node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_not_proper_numbers 
ARGS((const char* op, node_ptr, node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_multiple_assignment ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_empty_range 
ARGS((node_ptr, int, int)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_not_constant_resize_width 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_not_constant_extend_width 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_not_constant_wtoint ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_not_word_sizeof ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_not_constant_width_of_word_type 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_not_constant_width_of_word_array_type 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_not_constant_width_of_array_type 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_wrong_word_operand 
ARGS((const char* msg, node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_assign_both 
ARGS((node_ptr, node_ptr, int, int)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_unknown_var_in_order_file 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_var_appear_twice_in_order_file 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_id_appears_twice_in_idlist_file 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void warning_var_appear_twice_in_order_file ARGS((node_ptr));
EXTERN void warning_id_appears_twice_in_idlist_file ARGS((node_ptr n));
EXTERN void warning_variable_not_declared ARGS((node_ptr));
EXTERN void warning_missing_variable ARGS((node_ptr));
EXTERN void warning_missing_variables ARGS((NodeList_ptr vars_list));
EXTERN void warning_non_ag_only_spec ARGS((Prop_ptr));
EXTERN void warning_ag_only_without_reachables ARGS((void));
EXTERN void warning_fsm_init_empty ARGS((void));
EXTERN void warning_fsm_fairness_empty ARGS((void));
EXTERN void warning_fsm_init_and_fairness_empty ARGS((void));
EXTERN void warning_fsm_invar_empty ARGS((void));

EXTERN void error_var_not_in_order_file 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_out_of_memory ARGS((size_t)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_invalid_subrange 
ARGS((node_ptr range)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_invalid_bool_cast 
ARGS((node_ptr expr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_invalid_toint_cast 
ARGS((node_ptr expr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_out_of_bounds_word_toint_cast 
ARGS((node_ptr expr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_invalid_count_operator 
ARGS((node_ptr expr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_invalid_enum_value 
ARGS((node_ptr value)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_game_definition_contains_input_vars 
ARGS((node_ptr var_name)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_property_contains_input_vars 
ARGS((Prop_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_assign_exp_contains_input_vars 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_next_exp_contains_input_vars 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_invar_exp_contains_input_vars 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_init_exp_contains_input_vars 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_second_player_var ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_second_player_next_var 
ARGS((node_ptr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_unknown_preprocessor 
ARGS((const char* prep_name)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_type_system_violation ARGS((void)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_psl_not_supported_feature 
ARGS((void)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_psl_not_supported_feature_next_number 
ARGS((void)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_not_supported_feature 
ARGS((const char* msg)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_expected_number ARGS((void)) NUSMV_FUNCATTR_NORETURN;

EXTERN void warning_psl_not_supported_feature
ARGS((node_ptr psl_spec, int index));

EXTERN void error_psl_repeated_replicator_id 
ARGS((void)) NUSMV_FUNCATTR_NORETURN;

/* this prints only a message, and returns */
EXTERN void error_invalid_number ARGS((const char* szNumber));

/* this prints only a message, and returns */
EXTERN void error_bmc_invalid_k_l ARGS((const int k, const int l));

/* this prints only a message, and returns */
EXTERN void error_property_already_specified ARGS((void));

EXTERN void error_invalid_numeric_value 
ARGS((int value, const char* reason)) NUSMV_FUNCATTR_NORETURN;

EXTERN void error_file_not_found 
ARGS((const char* filename)) NUSMV_FUNCATTR_NORETURN;

EXTERN void warning_processes_deprecated ARGS((void));

EXTERN void error_not_word_wsizeof 
ARGS((node_ptr expr)) NUSMV_FUNCATTR_NORETURN;

EXTERN void finish_parsing_err ARGS((void)) NUSMV_FUNCATTR_NORETURN;
EXTERN void error_reset_and_exit ARGS((int val)) NUSMV_FUNCATTR_NORETURN;

EXTERN void init_memory ARGS((void));

EXTERN node_ptr failure_make 
ARGS((const char* msg, FailureKind kind, int lineno));

EXTERN const char* failure_get_msg ARGS((node_ptr failure));
EXTERN FailureKind failure_get_kind ARGS((node_ptr failure));
EXTERN int failure_get_lineno ARGS((node_ptr failure));



#include <setjmp.h>
/* New versions of cygwin do not need special treatments */
/* #ifdef __CYGWIN__  */
/* #define JMPBUF jmp_buf */
/* #define SETJMP(buf,val) setjmp(buf) */
/* #define LONGJMP(buf,val) longjmp(buf, val) */
/* #else */
#if defined(__MINGW32__) || defined(_MSC_VER)
#define JMPBUF jmp_buf
#define SETJMP(buf,val) setjmp(buf)
#define LONGJMP(buf,val) longjmp(buf, val)
#else
#define JMPBUF sigjmp_buf
#define SETJMP(buf,val) sigsetjmp(buf, val)
#define LONGJMP(buf,val) siglongjmp(buf, val)
#endif
/* #endif */

EXTERN JMPBUF * util_newlongjmp ARGS((void));
EXTERN void util_longjmp ARGS((void));
EXTERN void util_cancellongjmp ARGS((void));
EXTERN void util_resetlongjmp ARGS((void));
#define util_setlongjmp() SETJMP(*(util_newlongjmp()), 1)
/* warning take care not to do something like this:

  CATCH {
     cmd1....
     return 1;
  } FAIL {
     cmd2...
    return 0;
  }

  The right way to use it is:

  {
    type result;

    CATCH {
     cmd1....
     result = value;
    } FAIL {
     cmd2...
     result = 1;
    }
    return(result);
  }
  I.e. return inside CATCH/FAIL may cause damage of the stack
*/
#define CATCH if (util_setlongjmp() == 0) {
#define FAIL  util_cancellongjmp(); } else

#endif /* _ERROR_H */
