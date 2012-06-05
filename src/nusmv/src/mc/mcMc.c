/**CFile***********************************************************************

  FileName    [mcMc.c]

  PackageName [mc]

  Synopsis    [Fair CTL model checking routines.]

  Description [Fair CTL model checking routines.]

  SeeAlso     [mcExplain.c]

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

#include "mc.h"
#include "mcInt.h"
#include "fsm/bdd/FairnessList.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "utils/utils_io.h"
#include "trace/Trace.h"
#include "trace/TraceManager.h"
#include "enc/enc.h"
#include "prop/propPkg.h"


static char rcsid[] UTIL_UNUSED = "$Id: mcMc.c,v 1.13.2.57.2.1.2.6.4.10 2009-07-20 14:02:38 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static BddStatesInputs
Mc_get_fair_si_subset ARGS((BddFsm_ptr fsm,
                            BddStatesInputs si));

static BddStatesInputs
Mc_fair_si_iteration ARGS((BddFsm_ptr fsm,
                           bdd_ptr states,
                           bdd_ptr subspace));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Verifies that M,s0 |= alpha ]

  Description [Verifies that M,s0 |= alpha using the fair CTL model checking.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Mc_CheckCTLSpec(Prop_ptr prop) {
  node_ptr exp;
  Trace_ptr trace;
  bdd_ptr s0, tmp_1, tmp_2;
  BddFsm_ptr fsm;
  BddEnc_ptr enc;
  DdManager* dd;
  Expr_ptr spec  = Prop_get_expr_core(prop);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "evaluating ");
    print_spec(nusmv_stderr, prop);
    fprintf(nusmv_stderr, "\n");
  }

  fsm = Prop_compute_ground_bdd_fsm(prop, global_fsm_builder);
  enc = BddFsm_get_bdd_encoding(fsm);
  dd = BddEnc_get_dd_manager(enc);

  /* Evaluates the spec */
  s0 = eval_ctl_spec(fsm, enc, spec, Nil);

  tmp_1 = bdd_not(dd, s0);
  tmp_2 = BddFsm_get_state_constraints(fsm);
  bdd_and_accumulate(dd, &tmp_2 , tmp_1);
  bdd_free(dd, tmp_1);
  tmp_1 = BddFsm_get_fair_states(fsm);
  if (bdd_is_false(dd, tmp_1)) {
    warning_fsm_fairness_empty();
  }
  bdd_and_accumulate(dd, &tmp_2 , tmp_1);
  bdd_free(dd, tmp_1);
  bdd_free(dd, s0);

  s0 = BddFsm_get_init(fsm);
  bdd_and_accumulate(dd, &s0, tmp_2);
  bdd_free(dd, tmp_2);

  /* Prints out the result, if not true explain. */
  fprintf(nusmv_stdout, "-- ");
  print_spec(nusmv_stdout, prop);

  if (bdd_is_false(dd, s0)) {
    fprintf(nusmv_stdout, "is true\n");
    Prop_set_status(prop, Prop_True);
  }
  else {
    fprintf(nusmv_stdout, "is false\n");
    Prop_set_status(prop, Prop_False);

    if (opt_counter_examples(OptsHandler_get_instance())) {
      char* trace_title = NULL;
      char* trace_title_postfix = " Counterexample";

      tmp_1 = BddEnc_pick_one_state(enc, s0);
      bdd_free(dd, s0);
      s0 = bdd_dup(tmp_1);
      bdd_free(dd, tmp_1);

      exp = reverse(explain(fsm, enc, cons((node_ptr) bdd_dup(s0), Nil),
                            spec, Nil));

      if (exp == Nil) {
        /* The counterexample consists of one initial state */
        exp = cons((node_ptr) bdd_dup(s0), Nil);
      }

      /* The trace title depends on the property type. For example it
       is in the form "LTL Counterexample" */
      trace_title = ALLOC(char, 
                          strlen(Prop_get_type_as_string(prop)) +
                          strlen(trace_title_postfix) + 1);
      nusmv_assert(trace_title != (char*) NULL);
      strcpy(trace_title, Prop_get_type_as_string(prop));
      strcat(trace_title, trace_title_postfix);

      {
        SexpFsm_ptr sexp_fsm; /* needed for trace lanugage */
        sexp_fsm = Prop_get_scalar_sexp_fsm(prop);
        if (SEXP_FSM(NULL) == sexp_fsm) {
          sexp_fsm = \
            PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());
          SEXP_FSM_CHECK_INSTANCE(sexp_fsm);
        }

        trace = \
          Mc_create_trace_from_bdd_state_input_list(enc,
               SexpFsm_get_symbols_list(sexp_fsm), trace_title,
                                                   TRACE_TYPE_CNTEXAMPLE, exp);
      }

      FREE(trace_title);

      fprintf(nusmv_stdout,
              "-- as demonstrated by the following execution sequence\n");

      TraceManager_register_trace(global_trace_manager, trace);
      TraceManager_execute_plugin(global_trace_manager, TRACE_OPT(NULL),
                                  TRACE_MANAGER_DEFAULT_PLUGIN,
                                  TRACE_MANAGER_LAST_TRACE);

      Prop_set_trace(prop, Trace_get_id(trace));

      walk_dd(dd, bdd_free, exp);
      free_list(exp);
    }
  }

  bdd_free(dd, s0);
} /* Mc_CheckCTLSpec */

/**Function********************************************************************

  Synopsis    [Compute quantitative characteristics on the model.]

  Description [Compute the given quantitative characteristics on the model.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Mc_CheckCompute(Prop_ptr prop)
{
  int s0;
  Expr_ptr  spec = Prop_get_expr_core(prop);
  BddFsm_ptr fsm = BDD_FSM(NULL);
  BddEnc_ptr enc;
  DdManager* dd;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "evaluating ");
    print_spec(nusmv_stderr, prop);
    fprintf(nusmv_stderr, "\n");
  }

  fsm = Prop_compute_ground_bdd_fsm(prop, global_fsm_builder);
  BDD_FSM_CHECK_INSTANCE(fsm);

  enc = BddFsm_get_bdd_encoding(fsm);
  dd = BddEnc_get_dd_manager(enc);

  {
    /*
       We force computation of reachable states, as the following
       calls will be performed more efficiently since they are cached.
    */
    bdd_ptr r = BddFsm_get_reachable_states(fsm);
    bdd_free(dd, r);
  }

  s0 = eval_compute(fsm, enc, spec, Nil);

  fprintf(nusmv_stdout, "-- ");
  print_compute(nusmv_stdout, prop);

  if (s0 == -1) {
    fprintf(nusmv_stdout, "is infinity\n");
    Prop_set_number_infinite(prop);
    Prop_set_status(prop, Prop_Number);
  }
  else if (s0 == -2) {
    fprintf(nusmv_stdout, "is undefined\n");
    Prop_set_number_undefined(prop);
    Prop_set_status(prop, Prop_Number);
  }
  else {
    fprintf(nusmv_stdout, "is %d\n", s0);
    Prop_set_number(prop, s0);
    Prop_set_status(prop, Prop_Number);
  }
  
  fflush(nusmv_stdout);
  fflush(nusmv_stderr);
}

/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>EX(g)</i>.]

  Description        [Computes the set of states satisfying <i>EX(g)</i>.]

  SideEffects        []

  SeeAlso            [eu ef eg]

******************************************************************************/
BddStates ex(BddFsm_ptr fsm, BddStates g)
{
  DdManager* dd = BddEnc_get_dd_manager(BddFsm_get_bdd_encoding(fsm));
  bdd_ptr result;
  bdd_ptr tmp = bdd_dup(g);

  {
    /*
       The explicit restriction to fair states is required (it affects
       the result from a logical point of view.)
    */
    bdd_ptr fair_states_bdd = BddFsm_get_fair_states(fsm);

    bdd_and_accumulate(dd, &tmp, fair_states_bdd);
    bdd_free(dd, fair_states_bdd);
  }

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd =  BddFsm_get_reachable_states(fsm);
    bdd_and_accumulate(dd, &tmp, reachable_states_bdd);
    bdd_free(dd, reachable_states_bdd);
  }

  result = BddFsm_get_backward_image(fsm, tmp);
  bdd_free(dd, tmp);

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd =  BddFsm_get_reachable_states(fsm);
    bdd_and_accumulate(dd, &result, reachable_states_bdd);
    bdd_free(dd, reachable_states_bdd);
  }

  return(result);
}

/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>E\[ f U g \]</i>.]

  Description        [Computes the set of states satisfying <i>E\[ f U g \]</i>.]

  SideEffects        []

  SeeAlso            [ebu]

******************************************************************************/
BddStates eu(BddFsm_ptr fsm, BddStates f, BddStates g)
{
  BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);
  DdManager* dd = BddEnc_get_dd_manager(enc);
  bdd_ptr new, oldY;
  bdd_ptr Y = bdd_dup(g);
  int n = 1;

  /* The following simplification may be useful for efficiency since g
     may be unreachable (but they are not fundamental for correctness
     similar simplifications are applied in ex). */

  {
    bdd_ptr fair_states_bdd = BddFsm_get_fair_states(fsm);

    bdd_and_accumulate(dd, &Y, fair_states_bdd);
    bdd_free(dd, fair_states_bdd);
  }

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
      bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(fsm);
      bdd_and_accumulate(dd, &Y, reachable_states_bdd);
      bdd_free(dd, reachable_states_bdd);
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    indent_node(nusmv_stderr, "eu: computing fixed point approximations for ",
                get_the_node()," ...\n");
  }

  oldY = bdd_dup(Y);
  new = bdd_dup(Y);
  while(bdd_isnot_false(dd, new)) {
    bdd_ptr tmp_1, tmp_2;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      double states = BddEnc_count_states_of_bdd(enc, Y);
      int size = bdd_size(dd, Y);

      indent(nusmv_stderr);
      fprintf(nusmv_stderr, "size of Y%d = %g states, %d BDD nodes\n",
	      n++, states, size);

    }
    bdd_free(dd, oldY);
    oldY = bdd_dup(Y);

    tmp_1 = ex(fsm, new);

    tmp_2 = bdd_and(dd, f, tmp_1);

    bdd_free(dd, tmp_1);
    bdd_or_accumulate(dd, &Y, tmp_2);

    bdd_free(dd, tmp_2);
    tmp_1 = bdd_not(dd, oldY);

    bdd_free(dd, new);
    new = bdd_and(dd, Y, tmp_1);

    bdd_free(dd, tmp_1);
  }
  bdd_free(dd, new);
  bdd_free(dd, oldY);

  return(Y);
}


/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>EF(g)</i>.]

  Description        [Computes the set of states satisfying <i>EG(g)</i>.]

  SideEffects        []

  SeeAlso            [eu ex]

******************************************************************************/
BddStates eg(BddFsm_ptr fsm, BddStates g)
{
  DdManager* dd = BddEnc_get_dd_manager(BddFsm_get_bdd_encoding(fsm));
  bdd_ptr fair_transitions;
  bdd_ptr fair_transitions_g;
  bdd_ptr res_si;
  bdd_ptr res;

  /* Lazy evaluation for the case 'EG True' */
  if (bdd_is_true(dd, g)) return BddFsm_get_fair_states(fsm);

  fair_transitions = BddFsm_get_fair_states_inputs(fsm);
  fair_transitions_g = bdd_and(dd, fair_transitions, g);

  res_si = eg_si(fsm, fair_transitions_g);

  res = BddFsm_states_inputs_to_states(fsm, res_si);

  bdd_free(dd, res_si);
  bdd_free(dd, fair_transitions_g);
  bdd_free(dd, fair_transitions);

  return(res);
}


/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>EF(g)</i>.]

  Description        [Computes the set of states satisfying <i>EF(g)</i>.]

  SideEffects        []

  SeeAlso            [eu ex]

******************************************************************************/
BddStates ef(BddFsm_ptr fsm, BddStates g)
{
  DdManager* dd = BddEnc_get_dd_manager(BddFsm_get_bdd_encoding(fsm));
  bdd_ptr result, one;

  one = bdd_true(dd);
  result = eu(fsm, one, g);
  bdd_free(dd, one);

  return(result);
}

/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>A\[f U g\]</i>.]

  Description        [Computes the set of states satisfying <i>A\[f U g\]</i>.]

  SideEffects        []

  SeeAlso            [ax af ex ef]

******************************************************************************/
BddStates au(BddFsm_ptr fsm, BddStates f, BddStates g)
{
  DdManager* dd = BddEnc_get_dd_manager(BddFsm_get_bdd_encoding(fsm));
  bdd_ptr result, tmp_1, tmp_2, tmp_3, tmp_4;

  tmp_1 = bdd_not(dd, f);
  tmp_2 = bdd_not(dd, g);
  tmp_3 = eg(fsm, tmp_2);
  tmp_4 = bdd_and(dd, tmp_1, tmp_2);
  bdd_free(dd, tmp_1);
  tmp_1 = eu(fsm, tmp_2, tmp_4);
  bdd_free(dd, tmp_2);
  tmp_2 = bdd_or(dd, tmp_1, tmp_3);
  result = bdd_not(dd, tmp_2);

  bdd_free(dd, tmp_2);
  bdd_free(dd, tmp_1);
  bdd_free(dd, tmp_4);
  bdd_free(dd, tmp_3);

  return(result);
}

/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>EG(g)</i>.]

  Description        [Computes the set of states satisfying <i>EG(g)</i>.]

  SideEffects        []

  SeeAlso            [eu ex ef]

******************************************************************************/
BddStatesInputs ex_si(BddFsm_ptr fsm, BddStatesInputs si)
{
  DdManager* dd = BddEnc_get_dd_manager(BddFsm_get_bdd_encoding(fsm));
  BddStates states;
  BddStatesInputs si_preimage;

  /* Eliminate input variables */
  states = BddFsm_states_inputs_to_states(fsm, si);

  /* Perform weak preimage */
  si_preimage = BddFsm_get_weak_backward_image(fsm, states);

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(fsm);

    bdd_and_accumulate(dd, &si_preimage, reachable_states_bdd);
    bdd_free(dd, reachable_states_bdd);
  }

  bdd_free(dd, states);

  return si_preimage;
}

/**Function********************************************************************

  Synopsis [Computes the set of state-input pairs that satisfy
  E(f U g), with f and g sets of state-input pairs.]

  Description  []

  SeeAlso      []

  SideEffects  []

******************************************************************************/
BddStatesInputs eu_si(BddFsm_ptr fsm, bdd_ptr f, bdd_ptr g)
{
  int i = 0;
  BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);
  DdManager* dd = BddEnc_get_dd_manager(enc);

  bdd_ptr oldY;
  bdd_ptr resY;
  bdd_ptr newY;
  bdd_ptr rg = bdd_dup(g);

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(fsm);

    bdd_and_accumulate(dd, &rg, reachable_states_bdd);
    bdd_free(dd, reachable_states_bdd);
  }

  oldY = bdd_dup(rg);
  resY = bdd_dup(rg);
  newY = bdd_dup(rg);

  bdd_free(dd, rg);

  while (bdd_isnot_false(dd, newY)) {
    bdd_ptr tmp_1, tmp_2;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr,
              "    size of Y%d = %g <states>x<inputs>, %d BDD nodes\n",
              i++, BddEnc_count_states_inputs_of_bdd(enc, resY),
              bdd_size(dd, resY) );
    }

    bdd_free(dd, oldY);

    oldY = bdd_dup(resY);
    tmp_1 = ex_si(fsm, newY);
    tmp_2 = bdd_and(dd, tmp_1, f);
    bdd_free(dd, tmp_1);

    bdd_or_accumulate(dd, &resY, tmp_2);
    bdd_free(dd, tmp_2);

    tmp_1 = bdd_not(dd, oldY);
    bdd_free(dd, newY);

    newY = bdd_and(dd, resY, tmp_1);
    bdd_free(dd, tmp_1);
  }

  bdd_free(dd, newY);
  bdd_free(dd, oldY);

  return BDD_STATES_INPUTS( resY );
}

/**Function********************************************************************

  Synopsis           [Set of states-inputs satisfying <i>EG(g)</i>.]

  Description        []

  SideEffects        []

  SeeAlso            [eu ex]

******************************************************************************/
bdd_ptr eg_si(BddFsm_ptr fsm, bdd_ptr g_si)
{
  DdManager* dd = BddEnc_get_dd_manager(BddFsm_get_bdd_encoding(fsm));
  bdd_ptr applicable_states_inputs;
  bdd_ptr fair_states_inputs;

  applicable_states_inputs =
    BddFsm_get_states_inputs_constraints(fsm, BDD_FSM_DIR_BWD);
  bdd_and_accumulate(dd, &applicable_states_inputs, g_si);

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(fsm);

    bdd_and_accumulate(dd, &applicable_states_inputs, reachable_states_bdd);
    bdd_free(dd, reachable_states_bdd);
  }

  fair_states_inputs = Mc_get_fair_si_subset(fsm, applicable_states_inputs);

  bdd_free(dd, applicable_states_inputs);

  return fair_states_inputs;
}

/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>E\[f U^{inf..sup} g\]</i>.]

  Description        [Computes the set of states satisfying
                      <i>E\[f U^{inf..sup} g\]</i></i>.]

  SideEffects        []

  SeeAlso            [eu]

******************************************************************************/
BddStates ebu(BddFsm_ptr fsm, BddStates f, BddStates g, int inf, int sup)
{
  BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);
  DdManager* dd = BddEnc_get_dd_manager(enc);

  int i;
  bdd_ptr Y, oldY, tmp_1, tmp_2;
  int n = 1;

  if (inf > sup || inf < 0) return(bdd_false(dd));

  Y = bdd_dup(g);

  {
    bdd_ptr fair_states_bdd = BddFsm_get_fair_states(fsm);

    bdd_and_accumulate(dd, &Y, fair_states_bdd);
    bdd_free(dd, fair_states_bdd);
  }

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(fsm);

    bdd_and_accumulate(dd, &Y, reachable_states_bdd);
    bdd_free(dd, reachable_states_bdd);
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1))
    indent_node(nusmv_stderr, "ebu: computing fixed point approximations for ",
                get_the_node()," ...\n");

  /* compute Y = g | (f & ex(Y)) for states within the bound */
  for (i = sup; i > inf; i--) {
    /* There are more states within the bounds */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      indent(nusmv_stderr);
      fprintf(nusmv_stderr, "size of Y%d = %g states, %d BDD nodes\n",
              n++, BddEnc_count_states_of_bdd(enc, Y),
	      bdd_size(dd, Y));
    }
    oldY = Y;
    tmp_1 = ex(fsm, Y);
    tmp_2 = bdd_and(dd, f, tmp_1);
    bdd_or_accumulate(dd, &Y, tmp_2);
    bdd_free(dd, tmp_1);
    bdd_free(dd, tmp_2);

    if (Y == oldY) {
      /* fixpoint found. collect garbage, and goto next phase */
      break;
    }
  }

  /* compute Y = f & ex(Y) for states before the bound */
  for (i = inf; i > 0; i--) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      indent(nusmv_stderr);
      fprintf(nusmv_stderr, "size of Y%d = %g states, %d BDD nodes\n",
              n++, BddEnc_count_states_of_bdd(enc, Y),
	      bdd_size(dd, Y));
    }
    oldY = bdd_dup(Y);
    tmp_1 = ex(fsm, Y);
    bdd_free(dd, Y);
    Y = bdd_and(dd, f, tmp_1);
    bdd_free(dd, tmp_1);
    bdd_free(dd, oldY);
    if (Y == oldY) {
      /* fixpoint found. collect garbage, and finish */
      break;
    }
  }
  return(Y);
}

/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>EF^{inf..sup}(g)</i>.]

  Description        [Computes the set of states satisfying
                     <i>EF^{inf..sup}(g)</i>.]

  SideEffects        []

  SeeAlso            [ef]

******************************************************************************/
BddStates ebf(BddFsm_ptr fsm, BddStates g, int inf, int sup)
{
  DdManager* dd = BddEnc_get_dd_manager(BddFsm_get_bdd_encoding(fsm));
  bdd_ptr one, result;

  one = bdd_true(dd);
  result = ebu(fsm, one, g, inf, sup);
  bdd_free(dd, one);
  return(result);
}

/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>EG^{inf..sup}(g)</i>.]

  Description        [Computes the set of states satisfying
                      <i>EG^{inf..sup}(g)</i>.]

  SideEffects        []

  SeeAlso            [eg]

******************************************************************************/
BddStates ebg(BddFsm_ptr fsm, BddStates g, int inf, int sup)
{
  BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);
  DdManager* dd = BddEnc_get_dd_manager(enc);
  int i;
  bdd_ptr Y, oldY, tmp_1;
  int n = 1;

  if (inf > sup || inf < 0) return bdd_true(dd);

  Y = bdd_dup(g);

  /* Limitation to fair states should be imposed. */
  {
    bdd_ptr fair_states_bdd = BddFsm_get_fair_states(fsm);

    bdd_and_accumulate(dd, &Y, fair_states_bdd);
    bdd_free(dd, fair_states_bdd);
  }

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(fsm);

    bdd_and_accumulate(dd, &Y, reachable_states_bdd);
    bdd_free(dd, reachable_states_bdd);
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    indent_node(nusmv_stderr, "ebg: computing fixed point approximations for ",
                get_the_node()," ...\n");
  }

  /* compute Y = g & ex(Y) for states within the bound */
  for (i = sup; i > inf; i--) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      indent(nusmv_stderr);
      fprintf(nusmv_stderr, "size of Y%d = %g states, %d BDD nodes\n",
              n++, BddEnc_count_states_of_bdd(enc, Y),
	      bdd_size(dd, Y));
    }
    oldY = bdd_dup(Y);
    tmp_1 = ex(fsm, Y);
    bdd_and_accumulate(dd, &Y, tmp_1);
    bdd_free(dd, tmp_1);
    if (Y == oldY) {
      bdd_free(dd, oldY);
      /* fixpoint found. goto next phase */
      break;
    }
    bdd_free(dd, oldY);
  }
  /* compute Y = ex(Y) for states before the bound */
  for (i = inf; i > 0; i--) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      indent(nusmv_stderr);
      fprintf(nusmv_stderr, "size of Y%d = %g states, %d BDD nodes\n",
              n++, BddEnc_count_states_of_bdd(enc, Y),
	      bdd_size(dd, Y));
    }
    oldY = Y;
    tmp_1 = ex(fsm, Y);
    bdd_free(dd, Y);
    Y = tmp_1;
    if (Y == oldY) {
      break; /* fixpoint found. */
    }
  }
  return Y;
}

/**Function********************************************************************

  Synopsis           [Set of states satisfying <i>A\[f U^{inf..sup} g\]</i>.]

  Description        [Computes the set of states satisfying
                     <i>A\[f U^{inf..sup} g\]</i>.]

  SideEffects        []

  SeeAlso            [au]

******************************************************************************/
BddStates abu(BddFsm_ptr fsm, BddStates f, BddStates g, int inf, int sup)
{
  BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);
  DdManager* dd = BddEnc_get_dd_manager(enc);
  int i;
  bdd_ptr Y, oldY, tmp_1, tmp_2;
  int n = 1;

  if (inf > sup || inf < 0) return(bdd_false(dd));

  Y = bdd_dup(g);

  {
    bdd_ptr fair_states_bdd = BddFsm_get_fair_states(fsm);

    bdd_and_accumulate(dd, &Y, fair_states_bdd);
    bdd_free(dd, fair_states_bdd);
  }

  if (opt_use_reachable_states(OptsHandler_get_instance())) {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(fsm);

    bdd_and_accumulate(dd, &Y, reachable_states_bdd);
    bdd_free(dd, reachable_states_bdd);
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1))
    indent_node(nusmv_stderr, "abu: computing fixed point approximations for ",
                get_the_node(), " ...\n");
  /* compute Y = g | (f & ax(Y)) for states within the bound */
  for (i = sup; i > inf; i--) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)){
      indent(nusmv_stderr);
      fprintf(nusmv_stderr, "size of Y%d = %g states, %d BDD nodes\n",
              n++, BddEnc_count_states_of_bdd(enc, Y),
	      bdd_size(dd, Y));
    }
    oldY = Y;
    tmp_1 = bdd_not(dd, Y);
    tmp_2 = ex(fsm, tmp_1);
    bdd_free(dd, tmp_1);
    tmp_1 = bdd_not(dd, tmp_2);
    bdd_free(dd, tmp_2);
    tmp_2 = bdd_and(dd, f, tmp_1);
    bdd_or_accumulate(dd, &Y, tmp_2);
    bdd_free(dd, tmp_1);
    bdd_free(dd, tmp_2);
    if (Y == oldY) {
      break; /* fixpoint found. goto next phase */
    }
  }
  /* compute Y = f & ax(Y) for states before the bound */
  for (i = inf; i > 0; i--) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      indent(nusmv_stderr);
      fprintf(nusmv_stderr, "size of Y%d = %g states, %d BDD nodes\n",
              n++, BddEnc_count_states_of_bdd(enc, Y),
	      bdd_size(dd, Y));
    }
    oldY = bdd_dup(Y);
    tmp_1 = bdd_not(dd, Y);
    tmp_2 = ex(fsm, tmp_1);
    bdd_free(dd, tmp_1);
    tmp_1 = bdd_not(dd, tmp_2);
    bdd_free(dd, tmp_2);
    bdd_free(dd, Y);
    Y = bdd_and(dd, f, tmp_1);
    bdd_free(dd, oldY);
    bdd_free(dd,tmp_1);

    if (Y == oldY) {
      break; /* fixpoint found. finish */
    }
  }
  return(Y);
}


/**Function********************************************************************

  Synopsis           [Computes the minimum length of the shortest path
  from <i>f</i> to <i>g</i>.]

  Description        [This function computes the minimum length of the
  shortest path from <i>f</i> to <i>g</i>.<br>
  Starts from <i>f</i> and proceeds forward until finds a state in <i>g</i>.
  Notice that this function works correctly only if <code>-f</code>
  option is used.]

  SideEffects        []

  SeeAlso            [maxu]

******************************************************************************/
int minu(BddFsm_ptr fsm, bdd_ptr arg_f, bdd_ptr arg_g)
{
  BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);
  DdManager* dd = BddEnc_get_dd_manager(enc);
  int i;
  int n = 1;
  bdd_ptr R, Rp, tmp_1;
  bdd_ptr f = bdd_dup(arg_f);
  bdd_ptr g = bdd_dup(arg_g);
  bdd_ptr invar_bdd = BddFsm_get_state_constraints(fsm);
  bdd_ptr fair_states_bdd = BddFsm_get_fair_states(fsm);
  bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(fsm);

  R = (bdd_ptr)NULL;

  /* We restrict f and g to the seat of fair states */
  bdd_and_accumulate(dd, &g, fair_states_bdd);
  bdd_and_accumulate(dd, &f, fair_states_bdd);

  /* We restrict to reachable states */
  bdd_and_accumulate(dd, &f, reachable_states_bdd);
  bdd_and_accumulate(dd, &g, reachable_states_bdd);

  bdd_free(dd, reachable_states_bdd);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1))
    indent_node(nusmv_stderr, "minu: computing fixed point approximations for ",
                get_the_node(), " ...\n");
  i = 0;

  Rp = bdd_and(dd, f, invar_bdd); /* starts searching from f */

  do {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      indent(nusmv_stderr);
      fprintf(nusmv_stderr, "size of Rp%d = %g states, %d BDD nodes\n",
              n++, BddEnc_count_states_of_bdd(enc, Rp),
	      bdd_size(dd, Rp));
    }

    tmp_1 = bdd_and(dd, Rp, g);

    if (bdd_isnot_false(dd, tmp_1)) {
      /* If current frontier intersects g return minimum */
      bdd_free(dd, tmp_1);
      bdd_free(dd, f);
      bdd_free(dd, g);
      bdd_free(dd, Rp);
      bdd_free(dd, invar_bdd);
      bdd_free(dd, fair_states_bdd);
      if (R != (bdd_ptr)NULL) bdd_free(dd, R);

      return(i);
    }

    bdd_free(dd, tmp_1);

    if (R != (bdd_ptr)NULL) bdd_free(dd, R);

    R = Rp;

    /* go forward */
    tmp_1 = BddFsm_get_forward_image(fsm, R);

    /* We restrict the image to the set of fair states */
    bdd_and_accumulate(dd, &tmp_1, fair_states_bdd);

    Rp = bdd_or(dd, R, tmp_1);

    bdd_free(dd, tmp_1);

    i++;

  } while ( Rp != R );
  /* could not find g anywhere. A fixpoint has been found. g will not be
     ever found, so return infinity. */
  bdd_free(dd, f);
  bdd_free(dd, g);
  bdd_free(dd, Rp);
  bdd_free(dd, R);
  bdd_free(dd, invar_bdd);
  bdd_free(dd, fair_states_bdd);

  return(-1);
}

/**Function********************************************************************

  Synopsis           [This function computes the maximum length of the
  shortest path from <i>f</i> to <i>g</i>.]

  Description        [This function computes the maximum length of the
  shortest path from <i>f</i> to <i>g</i>. It starts from !g and
  proceeds backward until no states in <i>f</i> can be found. In other
  words, it looks for the maximum length of <i>f->AG!g</i>.
  Notice that this function works correctly only if <code>-f</code>
  option is used.

  Returns -1 if infinity, -2 if undefined]

  SideEffects        []

  SeeAlso            [minu]

******************************************************************************/
int maxu(BddFsm_ptr fsm, bdd_ptr f, bdd_ptr g)
{
  BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);
  DdManager* dd = BddEnc_get_dd_manager(enc);
  int i;
  int n = 1;
  bdd_ptr R, Rp;
  bdd_ptr notg, tmp_1;
  bdd_ptr invar_bdd, fair_states_bdd, reachable_states_bdd;

  invar_bdd = BddFsm_get_state_constraints(fsm);
  fair_states_bdd = BddFsm_get_fair_states(fsm);
  reachable_states_bdd = BddFsm_get_reachable_states(fsm);

  { /* checks if f is empty */
    bdd_ptr tmp = bdd_and(dd, f, invar_bdd);
    bdd_and_accumulate(dd, &tmp, reachable_states_bdd);
    if (!bdd_is_false(dd, fair_states_bdd)) {
      bdd_and_accumulate(dd, &tmp, fair_states_bdd);
    }
    else {
      fprintf(nusmv_stderr, "Warning: fair states are empty. "\
	      "Check FSM totality with check_fsm.\n");
    }

    if (bdd_is_false(dd, tmp)) {
      fprintf(nusmv_stderr, "Warning: in COMPUTE initial state is empty\n");
      bdd_free(dd, tmp);
      bdd_free(dd, reachable_states_bdd);
      bdd_free(dd, fair_states_bdd);
      bdd_free(dd, invar_bdd);
      return -2; /* undefined, as f is empty or not reachable/fair */
    }
    bdd_free(dd, tmp);
  }

  { /* checks if g is empty */
    bdd_ptr tmp = bdd_and(dd, g, invar_bdd);
    bdd_and_accumulate(dd, &tmp, reachable_states_bdd);
    if (!bdd_is_false(dd, fair_states_bdd)) {
      bdd_and_accumulate(dd, &tmp, fair_states_bdd);
    }

    if (bdd_is_false(dd, tmp)) {
      fprintf(nusmv_stderr, "Warning: in COMPUTE final state is empty\n");
      bdd_free(dd, tmp);
      bdd_free(dd, reachable_states_bdd);
      bdd_free(dd, fair_states_bdd);
      bdd_free(dd, invar_bdd);
      return -2; /* undefined, as g is empty or not reachable/fair */
    }

    bdd_free(dd, tmp);
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1))
    indent_node(nusmv_stderr, "maxu: computing fixed point approximations for ",
                get_the_node()," ...\n");

  tmp_1 = bdd_not(dd, g);
  notg = bdd_and(dd, tmp_1, invar_bdd);

  /* We restrict to fair states */
  bdd_and_accumulate(dd, &notg, fair_states_bdd);

  bdd_free(dd, tmp_1);
  bdd_free(dd, invar_bdd);

  i = 0;
  R = bdd_true(dd);
  Rp = bdd_dup(notg); /* starts from !g */


  /* We restrict to reachable states */
  {
    bdd_ptr reachable_states_bdd = BddFsm_get_reachable_states(fsm);
    bdd_and_accumulate(dd, &Rp, reachable_states_bdd);
    bdd_free(dd, reachable_states_bdd);
  }

  /* We restrict to fair states */
  bdd_and_accumulate(dd, &Rp, fair_states_bdd);

  do {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      indent(nusmv_stderr);
      fprintf(nusmv_stderr, "size of Rp%d = %g states, %d BDD nodes\n",
              n++, BddEnc_count_states_of_bdd(enc, Rp),
	      bdd_size(dd, Rp));
    }

    tmp_1 = bdd_and(dd, Rp, f);

    if (bdd_is_false(dd, tmp_1)) {
      /* !g does not intersect f anymore. The maximum length of a path
         completely in !g is i. This is the maximum. */
      bdd_free(dd, tmp_1);
      bdd_free(dd, R);
      bdd_free(dd, Rp);
      bdd_free(dd, notg);
      bdd_free(dd, fair_states_bdd);
      bdd_free(dd, reachable_states_bdd);

      return(i);
    }

    bdd_free(dd, tmp_1);
    bdd_free(dd, R);

    R = Rp;

    tmp_1 = BddFsm_get_backward_image(fsm, R);

    /* We restrict to reachable states */
    bdd_and_accumulate(dd, &tmp_1, reachable_states_bdd);

    /* We restrict to fir states */
    bdd_and_accumulate(dd, &tmp_1, fair_states_bdd);

    Rp = bdd_and(dd, tmp_1, notg);

    bdd_free(dd, tmp_1);

    i++;

  } while (R != Rp);

  /* a fixpoint has been found in which !g & f holds, so return infinity */
  bdd_free(dd, R);
  bdd_free(dd, Rp);
  bdd_free(dd, notg);
  bdd_free(dd, fair_states_bdd);
  bdd_free(dd, reachable_states_bdd);

  return -1;
}


/**Function********************************************************************

  Synopsis           [Prints out a CTL specification]

  Description        [Prints out a CTL specification]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void print_spec(FILE *file, Prop_ptr prop)
{
  indent(file);
  fprintf(file, "specification ");
  Prop_print(prop, file, get_prop_print_method(OptsHandler_get_instance()));
  fprintf(file, " ");
}


/**Function********************************************************************

  Synopsis           [Prints out a COMPUTE specification]

  Description        [Prints out a COMPUTE specification]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void print_compute(FILE *file, Prop_ptr p)
{
  fprintf(file, "the result of ");
  Prop_print(p, file, get_prop_print_method(OptsHandler_get_instance()));
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis [ ]

  Description [Perform one iteration over the list of fairness
  conditions (order is statically determined). Compute states that are
  backward reachable from each of the fairness conditions.

  MAP( ApplicableStatesInputs ) over Fairness constraints

  (Q /\ ex_si ( Z /\ AND_i eu_si(Z, (Z/\ StatesInputFC_i))))

  ]

  SeeAlso      []

  SideEffects  []

******************************************************************************/
static bdd_ptr Mc_fair_si_iteration(BddFsm_ptr fsm,
				    BddStatesInputs states,
				    BddStatesInputs subspace)
{
  bdd_ptr res;
  FairnessListIterator_ptr iter;
  bdd_ptr partial_result;

  res = bdd_true(dd_manager);
  partial_result = bdd_dup(states);

  iter = FairnessList_begin( FAIRNESS_LIST( BddFsm_get_justice(fsm) ) );
  while ( ! FairnessListIterator_is_end(iter) ) {
    bdd_ptr fc_si;
    bdd_ptr constrained_fc_si;
    bdd_ptr temp;

    /* Extract next fairness constraint */
    fc_si = JusticeList_get_p(BddFsm_get_justice(fsm), iter);

    /* Constrain it to current set */
    constrained_fc_si = bdd_and(dd_manager, states, fc_si);

    /* Collect states-input that can reach constrained_fc_si without leaving subspace */
    temp = eu_si(fsm, subspace, constrained_fc_si);

    bdd_free(dd_manager, constrained_fc_si);
    bdd_free(dd_manager, fc_si);

    bdd_and_accumulate(dd_manager, &partial_result, temp);
    bdd_free(dd_manager, temp);

    iter = FairnessListIterator_next(iter);
  }

  /* Compute preimage */
  res = ex_si(fsm, partial_result);
  bdd_free(dd_manager, partial_result);

  return res;
}

/**Function********************************************************************

  Synopsis     []

  Description [Returns the set of state-input pairs in si that are
  fair, i.e. beginning of a fair path.]

  SeeAlso      []

  SideEffects  []

******************************************************************************/
static BddStatesInputs Mc_get_fair_si_subset(BddFsm_ptr fsm,
					     BddStatesInputs si)
{
  int i = 0;
  BddStatesInputs res;
  BddStatesInputs old;
  BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);

  BDD_FSM_CHECK_INSTANCE(fsm);

  res = BDD_STATES_INPUTS(bdd_true(dd_manager));
  old = BDD_STATES_INPUTS(bdd_false(dd_manager));

  /* GFP computation */
  while (res != old) {
    BddStatesInputs new;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "  size of res%d = %g <states>x<input>, %d BDD nodes\n",
              i++, BddEnc_count_states_inputs_of_bdd(enc, res),
              bdd_size(dd_manager, res));
    }

    bdd_free(dd_manager, old);
    old = bdd_dup(res);

    /* One iteration over fairness conditions */
    new = Mc_fair_si_iteration(fsm, res, si);

    bdd_and_accumulate(dd_manager, &res, (bdd_ptr) new);
    bdd_and_accumulate(dd_manager, &res, (bdd_ptr) si);

    bdd_free(dd_manager, (bdd_ptr) new);
  }
  bdd_free(dd_manager, old);

  return res;
}


