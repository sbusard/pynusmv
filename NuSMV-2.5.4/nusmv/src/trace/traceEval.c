/**CFile***********************************************************************

  FileName    [traceEval.c]

  PackageName [trace]

  Synopsis    [This module contains defines evaluation code]

  Description []

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK.

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
#include "Trace.h"
#include "Trace_private.h"

#include "node/node.h"
#include "parser/symbols.h"
#include "enc/bdd/BddEnc.h" /* For BDD Encoder */
#include "utils/assoc.h"
#include "utils/NodeList.h"
#include "bmc/bmcConv.h"
#include "compile/symb_table/SymbTable.h"

#include "pkg_trace.h"
#include "pkg_traceInt.h"

#include "trace/eval/BaseEvaluator.h"

static char rcsid[] UTIL_UNUSED = "$Id: $";

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

static node_ptr
trace_make_failure ARGS((const char* tmpl, node_ptr symbol));

static hash_ptr
trace_eval_make_environment ARGS((Trace_ptr trace, TraceIter step));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
boolean trace_step_check_defines(const Trace_ptr trace, const TraceIter step,
                                 NodeList_ptr failures)
{
  TraceSymbolsIter sym_iter;

  node_ptr sym;
  node_ptr val;
  boolean res = true; /* no error */

  const BaseEvaluator_ptr evaluator = \
    TraceManager_get_evaluator(TracePkg_get_global_trace_manager());

  BASE_EVALUATOR_CHECK_INSTANCE(evaluator);
  nusmv_assert(TRACE_END_ITER != step);
  NODE_LIST_CHECK_INSTANCE(failures);
  nusmv_assert(0 == NodeList_get_length(failures));

  {  /* To evaluate a set of expressions, we first set context for
        evaluation. Then, in two distinct phases state and
        transitional defines are evaluated. 'State' belong to current
        step, 'transitional' belong to next */
    hash_ptr env = trace_eval_make_environment(trace, step);
    const SymbTable_ptr st = Trace_get_symb_table(trace);

    BaseEvaluator_set_context(evaluator, st, env);

    /* 1. state defines */
    TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_S_DEFINES, sym_iter, sym) {
      if (Nil != trace_step_get_value(trace, step, sym)) {

        val = BaseEvaluator_evaluate(evaluator,
                                     SymbTable_get_define_flatten_body(st, sym));

        if (FAILURE != node_get_type(val)) {
          node_ptr exp_val = trace_step_get_value(trace, step, sym);
          if (exp_val != val) {
            SymbCategory cat = trace_section_to_category(sym_iter.section);

            const char* fail_tmpl = \
              "Value mismatch for symbol %s (%s) calculated: %s, expected: %s";

            const char* cat_repr = trace_symb_category_to_string(cat);

            char *symb_repr = sprint_node(sym);
            char* calc_repr = sprint_node(val);
            char* expd_repr = sprint_node(exp_val);

            char *fail_repr = ALLOC(char, 1 +            \
                                    strlen(fail_tmpl) +  \
                                    strlen(symb_repr) +  \
                                    strlen(cat_repr)  +  \
                                    strlen(calc_repr) +  \
                                    strlen(expd_repr));

            sprintf(fail_repr,
                    fail_tmpl, symb_repr, cat_repr, calc_repr, expd_repr);

            NodeList_append(failures, trace_make_failure(fail_repr, Nil));

            FREE(symb_repr);
            FREE(calc_repr);
            FREE(expd_repr);
            FREE(fail_repr);

            res = false;
          }
        }
      }
    }

    { /* 2. transitional defines (exist only if there's next state) */
      TraceIter next = trace_iter_get_next(step);

      if (TRACE_END_ITER != next) {
        TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_TRANSITIONAL, sym_iter, sym) {
          if (Nil != trace_step_get_value(trace, next, sym)) {

            /* this is a bit tricky: evaluation takes place in 'step'
               but being transitional, the resulting value belongs to
               next and must be checked accordingly. */
            val = BaseEvaluator_evaluate(evaluator,
                                         SymbTable_get_define_flatten_body(st,
                                                                           sym));

            if (FAILURE != node_get_type(val)) {
              node_ptr exp_val = trace_step_get_value(trace, next, sym);
              if (exp_val != val) {
                SymbCategory cat = trace_section_to_category(sym_iter.section);

                const char* fail_tmpl = \
                  "Value mismatch for symbol %s (%s) calculated: %s, "
                  "expected: %s";

                const char* cat_repr = trace_symb_category_to_string(cat);

                char *symb_repr = sprint_node(sym);
                char* calc_repr = sprint_node(val);
                char* expd_repr = sprint_node(exp_val);

                char *fail_repr = ALLOC(char, 1 +            \
                                        strlen(fail_tmpl) +  \
                                        strlen(symb_repr) +  \
                                        strlen(cat_repr)  +  \
                                        strlen(calc_repr) +  \
                                        strlen(expd_repr));

                sprintf(fail_repr,
                        fail_tmpl, symb_repr, cat_repr, calc_repr, expd_repr);

                NodeList_append(failures, trace_make_failure(fail_repr, Nil));

                /* cleanup */
                FREE(symb_repr);
                FREE(calc_repr);
                FREE(expd_repr);
                FREE(fail_repr);

                res = false;
              }
            }
          }
        }
      }
    }

    /* destroy the environment */
    free_assoc(env);
  }

  return res;
} /* trace_step_check_defines */


/**Function********************************************************************

  Synopsis    [Evaluates defines for a trace]

  Description [Evaluates define for a trace, based on assignments to
               state, frozen and input variables.

               If a previous value exists for a define, The mismatch
               is reported to the caller by appending a failure node
               describing the error to the "failures" list. If
               "failures" is NULL failures are silently discarded.  If
               no previous value exists for a given define, assigns
               the define to the calculated value according to vars
               assignments. The "failures" list must be either NULL
               or a valid, empty list.

               0 is returned if no mismatching were detected, 1
               otherwise ]

  SideEffects [The trace is filled with defines, failures list is
               populated as necessary.]

  SeeAlso     []

******************************************************************************/
void trace_step_evaluate_defines(Trace_ptr trace, const TraceIter step)
{
  TraceSymbolsIter sym_iter;
  node_ptr sym;
  node_ptr val;

  const BaseEvaluator_ptr evaluator = \
    TraceManager_get_evaluator(TracePkg_get_global_trace_manager());

  BASE_EVALUATOR_CHECK_INSTANCE(evaluator);
  nusmv_assert(TRACE_END_ITER != step);

  {  /* To evaluate a set of expressions, we first set context for
        evaluation. Then, in two distinct phases state and
        transitional defines are evaluated. 'State' belong to current
        step, 'transitional' belong to next */
    hash_ptr env = trace_eval_make_environment(trace, step);
    const SymbTable_ptr st = Trace_get_symb_table(trace);

    BaseEvaluator_set_context(evaluator, st, env);

    /* 1 . state defines */
    TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_S_DEFINES, sym_iter, sym) {
      if (Nil == trace_step_get_value(trace, step, sym)) {

        val = BaseEvaluator_evaluate(evaluator,
                                     SymbTable_get_define_flatten_body(st, sym));

        if (FAILURE != node_get_type(val)) {
          trace_step_put_value(trace, step, sym, val);
        }
      }
    } /* foreach state define */

    { /* 2. transitional defines (exist only if there's next state) */
      TraceIter next = trace_iter_get_next(step);
      if (TRACE_END_ITER != next) {
        TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_TRANSITIONAL, sym_iter, sym) {
          if (Nil == trace_step_get_value(trace, next, sym)) {

            val = BaseEvaluator_evaluate(evaluator,
                                         SymbTable_get_define_flatten_body(st,
                                                                           sym));
            if (FAILURE != node_get_type(val)) {
              trace_step_put_value(trace, next, sym, val);
            }
          }
        }
      }
    } /* foreach transitional define */

    /* destroy the environment */
    free_assoc(env);
  }

} /* trace_step_evaluate_defines */

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private service of trace_evaluate_expr_recur]

  Description        [Private service of trace_evaluate_expr_recur]

  SideEffects        []

  SeeAlso            [Private service of trace_evaluate_expr_recur]

******************************************************************************/
static node_ptr
trace_make_failure(const char* tmpl, node_ptr symbol)
{
  char *symb_str = sprint_node(symbol);
  char *buf = ALLOC(char, 1 + strlen(tmpl) + strlen(symb_str));
  node_ptr res;

  sprintf(buf, tmpl, symb_str);
  res = failure_make(buf, FAILURE_UNSPECIFIED, -1);

  FREE(buf);
  FREE(symb_str);

  return res;
}

/**Function********************************************************************

  Synopsis [Private service of trace_step_evaluate_defines and
  trace_step_check_defines]

  Description [This function builds a local environment for constant
  expressions evaluation]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
static hash_ptr trace_eval_make_environment(Trace_ptr trace, TraceIter step)
{
  hash_ptr res = new_assoc(); /* empty env */
  TraceIter next_step = TraceIter_get_next(step); /* the next step */
  TraceSymbolsIter sym_iter;
  node_ptr sym;

  /* 1. frozen vars always have the same value for x and x' */
  TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_F_VARS, sym_iter, sym) {
    insert_assoc(res, sym,
                 trace_step_get_value(trace, TRACE_END_ITER, sym));
    insert_assoc(res, find_node(NEXT, sym, Nil),
                 trace_step_get_value(trace, TRACE_END_ITER, sym));
  }

  /* 2. state vars for x and x' (if any) */
  TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_S_VARS, sym_iter, sym) {
    insert_assoc(res, sym, trace_step_get_value(trace, step, sym));
    if (TRACE_END_ITER != next_step) {
      insert_assoc(res, find_node(NEXT, sym, Nil),
                   trace_step_get_value(trace, next_step, sym));
    }
  }

  /* 3. input vars for i (value is in next step) */
  TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_I_VARS, sym_iter, sym) {
    if (TRACE_END_ITER != next_step) {
      insert_assoc(res, sym,
                   trace_step_get_value(trace, next_step, sym));
    }
  }

  return res;
}
