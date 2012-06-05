/**CFile***********************************************************************

  FileName    [mcTrace.c]

  PackageName [mc]

  Synopsis    [This module contains functions to build traces from bdd lists]

  Description []

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``mc'' package of NuSMV version 2.
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
#include "mc.h"
#include "mcInt.h"

#include "fsm/bdd/BddFsm.h"

#include "trace/pkg_trace.h"
#include "trace/Trace.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define MC_MODEL_DEBUG 0

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
static void mc_trace_step_put_values ARGS((Trace_ptr trace, TraceIter iter,
                                           BddEnc_ptr bdd_enc, bdd_ptr formula,
                                           NodeList_ptr symbols));
#if MC_MODEL_DEBUG
static void mc_model_trace_step_print ARGS((const Trace_ptr trace,
                                            const TraceIter step,
                                            TraceIteratorType it_type,
                                            const char* prefix,
                                            int count));
#endif

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis     [Creates a trace out of a < S (i, S)* >  bdd list]

  Description  [Creates a trace out of a < S (i, S)* >  bdd list.
                The built trace is non-volatile. For more control over
                the built trace, please see
                Mc_fill_trace_from_bdd_state_input_list ]

  SideEffects  [none]

  SeeAlso      [Trace_create, Bmc_create_trace_from_cnf_model,
                Mc_fill_trace_from_bdd_state_input_list]

******************************************************************************/
Trace_ptr
Mc_create_trace_from_bdd_state_input_list(const BddEnc_ptr bdd_enc,
                                          const NodeList_ptr symbols,
                                          const char* desc,
                                          const TraceType type,
                                          node_ptr path)
{
  Trace_ptr trace;

  if (Nil == path) return TRACE(NULL);

  trace = Trace_create(BaseEnc_get_symb_table(BASE_ENC(bdd_enc)),
                       desc, type, symbols, false);

  return Mc_fill_trace_from_bdd_state_input_list(bdd_enc, trace, path);
}

/**Function********************************************************************

  Synopsis     [Fills the given trace out of a < S (i, S)* >  bdd list]

  Description [Fills the given trace out of a < S (i, S)* > bdd list.
                The returned trace is the given one, filled with all
                steps. The given trace MUST be empty. Path must be non-Nil]

  SideEffects  [none]

  SeeAlso      [Trace_create, Bmc_fill_trace_from_cnf_model]

******************************************************************************/
Trace_ptr
Mc_fill_trace_from_bdd_state_input_list(const BddEnc_ptr bdd_enc,
                                        Trace_ptr trace,
                                        node_ptr path)
{
  TraceIter step;

  NodeList_ptr sf_vars;
  NodeList_ptr i_vars;

#if MC_MODEL_DEBUG
  int i = 1; /* for debugging only */
#endif

  nusmv_assert(Nil != path);

  TRACE_CHECK_INSTANCE(trace);
  nusmv_assert(Trace_is_empty(trace));

  sf_vars = Trace_get_sf_vars(trace);
  i_vars = Trace_get_i_vars(trace);
  step = Trace_first_iter(trace);

#if MC_MODEL_DEBUG
  fprintf(nusmv_stderr, "\n--- MC Model extraction ---\n");
#endif

  /* first node of path is initial state */
  mc_trace_step_put_values(trace, step, bdd_enc, BDD_STATES(car(path)), sf_vars);

#if MC_MODEL_DEBUG
  mc_model_trace_step_print(trace, step, TRACE_ITER_SF_VARS, "S", i);
#endif

  path = cdr(path);
  while (path != Nil) {
    /* append one more (i,S) to trace */
    step = Trace_append_step(trace);
    mc_trace_step_put_values(trace, step, bdd_enc, BDD_INPUTS(car(path)), i_vars);

#if MC_MODEL_DEBUG
    mc_model_trace_step_print(trace, step, TRACE_ITER_I_VARS, "I", ++ i);
#endif

    path = cdr(path); nusmv_assert(Nil != path);

    mc_trace_step_put_values(trace, step, bdd_enc, BDD_STATES(car(path)), sf_vars);
    path = cdr(path);

#if MC_MODEL_DEBUG
    mc_model_trace_step_print(trace, step, TRACE_ITER_S_VARS, "S", i);
#endif

  }

#if MC_MODEL_DEBUG
  fprintf(nusmv_stderr, "\n\n");
#endif

  return trace;
}


/**Function********************************************************************

  Synopsis     [Populates a trace step with state assignments]

  Description  []

  SideEffects  [none]

  SeeAlso      []

******************************************************************************/
void Mc_trace_step_put_state_from_bdd(Trace_ptr trace, TraceIter step,
                                      BddEnc_ptr bdd_enc, bdd_ptr bdd)
{
  TRACE_CHECK_INSTANCE(trace);

  mc_trace_step_put_values(trace, step, bdd_enc, bdd, Trace_get_sf_vars(trace));
#if MC_MODEL_DEBUG
  mc_model_trace_step_print(trace, step, TRACE_ITER_SF_VARS, "S", -1);
#endif
}


/**Function********************************************************************

  Synopsis     [Populates a trace step with input assignments]

  Description  []

  SideEffects  [none]

  SeeAlso      []

******************************************************************************/
void Mc_trace_step_put_input_from_bdd(Trace_ptr trace, TraceIter step,
                                      BddEnc_ptr bdd_enc, bdd_ptr bdd)
{
  TRACE_CHECK_INSTANCE(trace);

  mc_trace_step_put_values(trace, step, bdd_enc, bdd, Trace_get_i_vars(trace));
#if MC_MODEL_DEBUG
  mc_model_trace_step_print(trace, step, TRACE_ITER_SF_VARS, "I", -1);
#endif
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
static void mc_trace_step_put_values(Trace_ptr trace, TraceIter step,
                                     BddEnc_ptr bdd_enc, bdd_ptr bdd,
                                     NodeList_ptr symbols)
{
  ListIter_ptr iter;
  add_ptr add, support;

  BDD_ENC_CHECK_INSTANCE(bdd_enc);
  DdManager* dd = BddEnc_get_dd_manager(bdd_enc);
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(bdd_enc));
  TypeChecker_ptr tc = SymbTable_get_type_checker(st);


  /* Restrict the input BDD to only one assignment (i.e. make BDD
     a complete assignment) */
  bdd = bdd_get_one_sparse_sat(dd, bdd);
  add = bdd_to_add(dd, bdd);
  bdd_free(dd, bdd);
  support = add_support(dd, add);

  NODE_LIST_FOREACH(symbols, iter) {
    node_ptr sym = NodeList_get_elem_at(symbols, iter);
    node_ptr sym_cleaned = NEXT == node_get_type(sym) ? car(sym) : sym;
    AddArray_ptr sym_add;
    node_ptr sym_value;
    SymbType_ptr type;

    /* obtain the type */
    if (SymbTable_is_symbol_var(st, sym_cleaned)) {
      type = SymbTable_get_var_type(st, sym_cleaned);
    }
    else if (SymbTable_is_symbol_define(st, sym_cleaned)) {
      type = TypeChecker_get_expression_type(tc, sym_cleaned, Nil);
      nusmv_assert(!SymbType_is_error(type)); /* cannot be an type error */
    }
    else {
      error_unreachable_code(); /* how the type may be not known ? */
      type = SYMB_TYPE(NULL);
    }

    /* check that sym is not a non-encodable symbol (like a real,
       integer, strings). */

    if (!Compile_is_expr_booleanizable(st, sym_cleaned, false, NULL)) {
      continue; /* skip this var */
    }

    /* check that var is a part of the BDD */
    if (!opt_keep_single_value_vars(OptsHandler_get_instance())) {
      boolean printVar = BddEnc_is_var_in_cube(bdd_enc, sym, support);
      if (!printVar) continue; /* skip this var */
    }

    sym_add = BddEnc_expr_to_addarray(bdd_enc, sym, Nil);

    /* if the symbol is a variable (of define) and has a Word type then
       it is necessart to construct a Word value from separate bits.
    */
    if (SymbType_is_word(type)) {
      int width = AddArray_get_size(sym_add);
      WordNumber_ptr one = WordNumber_from_integer(1, width);
      WordNumber_ptr result = WordNumber_from_integer(0, width);

      /* number of bits in ADD should be equal to number of bits in the type */
      nusmv_assert(width == SymbType_get_word_width(type) && width >0);

      /* compute the Word value from bits */
      for (--width; width >=0; --width) {
        add_ptr tmp_add = add_if_then(dd, add,
                                      AddArray_get_n(sym_add, width));
        node_ptr bit = add_value(dd, tmp_add);
        add_free(dd, tmp_add);

        /* the value of a bit can be 0 or 1 only */
        nusmv_assert(Expr_is_false(bit) || Expr_is_true(bit));

        /* words with width 1 cannot be shifted at all */
        if (WordNumber_get_width(result) != 1) {
          result = WordNumber_left_shift(result, 1);
        }
        /* add the bit to the result */
        if (Expr_is_true(bit)) result = WordNumber_plus(result, one);
      } /* for */

      sym_value = find_node(SymbType_is_signed_word(type)
                            ? NUMBER_SIGNED_WORD : NUMBER_UNSIGNED_WORD,
                            (node_ptr)result, Nil);
    }
    /* Else this is a symbol with non-Word type, i.e. it can have only one usual
       ADD, not array.
    */
    else {
      /* sym_add must have only one element in the array */
      add_ptr add_values = add_if_then(dd, add, AddArray_get_add(sym_add));
      sym_value = add_value(dd, add_values);
      add_free(dd, add_values);
    }

    /* populate trace */
    Trace_step_put_value(trace, step, sym, sym_value);

    /* unnecessary ? */
    /* /\* type check the created expression to allow further evaluation, */
    /*    created expression is always well-typed *\/ */
    /* TypeChecker_is_expression_wellformed(bdd_enc->type_checker, sym_value, Nil); */

    AddArray_destroy(dd, sym_add);
  } /* for loop */

  add_free(dd, add);
  add_free(dd, support);
}

#if MC_MODEL_DEBUG
static void mc_model_trace_step_print(const Trace_ptr trace,
                                      const TraceIter step,
                                      TraceIteratorType it_type,
                                      const char* prefix,
                                      int count)
{
  TraceStepIter iter;
  node_ptr symb, val;

  if (0 < count) { fprintf(nusmv_stderr, "%s%d:", prefix, count); }
  else { fprintf(nusmv_stderr, "%s:", prefix); }

  TRACE_STEP_FOREACH(trace, step, it_type, iter, symb, val) {
    print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, "=");
    print_node(nusmv_stderr, val); fprintf(nusmv_stderr, " ");
  }

  fprintf(nusmv_stderr, "\n");
} /* mc_model_trace_step_print */
#endif
