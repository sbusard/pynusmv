/**CFile***********************************************************************

  FileName    [ltl.c]

  PackageName [ltl]

  Synopsis    [Routines to perform reduction of LTL model checking to
  CTL model checking.]

  Description [Here we perform the reduction of LTL model checking to
  CTL model checking. The technique adopted has been taken from [1].
  <ol>
    <li>
       O. Grumberg E. Clarke and K. Hamaguchi. "Another Look at LTL
       Model Checking".  <em>Formal Methods in System Design</em>,
       10(1):57--71, February 1997.
    </li>
  </ol>
  ]

  SeeAlso     [mc]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``ltl'' package of NuSMV version 2.
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

#include "ltl/ltl.h"
#include "ltlInt.h"
#include "ltl/ltl2smv/ltl2smv.h"
#include "parser/symbols.h"
#include "parser/parser.h"
#include "prop/propPkg.h"
#include "prop/Prop.h"
#include "fsm/sexp/Expr.h" /* for Expr_ptr */
#include "fsm/bdd/FairnessList.h"
#include "fsm/bdd/bdd.h" /* to check preconditions for EL_fwd */
#include "mc/mc.h"
#include "mc/mcInt.h" /* for Mc_create_trace_from_bdd_state_input_list */
#include "compile/compile.h" /* to check for presence of compassion */

#include "utils/error.h" /* for CATCH */
#include "utils/utils_io.h"
#include "utils/utils.h"
#include "trace/Trace.h"
#include "trace/TraceManager.h"

#include "enc/enc.h"
#include "opt/opt.h"

#include "hrc/HrcNode.h"

static char rcsid[] UTIL_UNUSED = "$Id: ltl.c,v 1.33.4.36.2.1.2.23.4.37 2009-09-25 09:09:08 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct Ltl_StructCheckLtlSpec_TAG Ltl_StructCheckLtlSpec;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

struct Ltl_StructCheckLtlSpec_TAG {
  Prop_ptr prop; /* The property to verify */
  BddFsm_ptr fsm; /* The FSM representing the product model and
                     tableau */
  BddEnc_ptr bdd_enc; /* The BDD encoder */
  DdManager *dd;  /* The BDD package manager */
  SymbTable_ptr symb_table; /* The Symbol Table */
  SymbLayer_ptr tableau_layer; /* The layer where tableau variables
                                  will be added */
  bdd_ptr s0; /* The BDD representing the result of the verification */
  node_ptr spec_formula;
  Ltl_StructCheckLtlSpec_oreg2smv oreg2smv; /* The tableau constructor
                                               to use. This one may
                                               generate additional
                                               LTL, that will be
                                               removed by ltl2smv */
  Ltl_StructCheckLtlSpec_ltl2smv ltl2smv;   /* The tableau constructor
                                               to use. This is used to
                                               remove additional LTL
                                               properties left by
                                               oreg2smv */
  boolean negate_formula; /* flag to keep track wether the formula has
                             to be negated or not */
  boolean removed_layer; /* Flag to inform wether the layer has been
                            removed or not */
  boolean do_rewriting; /* Enables the rewriting to remove input from
                           properties */
};

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static BddFsm_ptr
ltlPropAddTableau ARGS((Ltl_StructCheckLtlSpec_ptr, FlatHierarchy_ptr));
static void
ltl_structcheckltlspec_remove_layer ARGS((Ltl_StructCheckLtlSpec_ptr));
static void ltl_structcheckltlspec_deinit ARGS((Ltl_StructCheckLtlSpec_ptr));
static void ltl_structcheckltlspec_init ARGS((Ltl_StructCheckLtlSpec_ptr,
                                              Prop_ptr prop));
static void ltl_structcheckltlspec_prepare ARGS((Ltl_StructCheckLtlSpec_ptr));
static int
ltl_structcheckltlspec_build_tableau_and_prop_fsm ARGS((Ltl_StructCheckLtlSpec_ptr self));
static void
ltl_structcheckltlspec_check_compassion ARGS((Ltl_StructCheckLtlSpec_ptr self));
static void
ltl_structcheckltlspec_check_el_bwd ARGS((Ltl_StructCheckLtlSpec_ptr self));
static void
ltl_structcheckltlspec_check_el_fwd ARGS((Ltl_StructCheckLtlSpec_ptr self));
static bdd_ptr ltl_clean_bdd ARGS((Ltl_StructCheckLtlSpec_ptr, bdd_ptr));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [The main routine to perform LTL model checking.]

  Description [The main routine to perform LTL model checking. It
  first takes the LTL formula, prints it in a file. It calls the LTL2SMV
  translator on it an reads in the generated tableau. The tableau is
  instantiated, compiled and then conjoined with the original model
  (both the set of fairness conditions and the transition relation are
  affected by this operation, for this reason we save the current
  model, and after the verification of the property we restore the
  original one).

  If already set (The Scalar and the Bdd ones, the FSMs used for
  verification are taken from within the property. Otherwise, global
  FSMs are set within the property and then used for verification.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Ltl_CheckLtlSpec(Prop_ptr prop)
{
  BddELFwdSavedOptions_ptr elfwd_saved_options = (BddELFwdSavedOptions_ptr) NULL;
  Ltl_StructCheckLtlSpec_ptr cls;

  /* save settings */
  if ((Nil == FlatHierarchy_get_compassion(mainFlatHierarchy)) &&
      (get_oreg_justice_emptiness_bdd_algorithm(OptsHandler_get_instance()) ==
       BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD)) {
    elfwd_saved_options = Bdd_elfwd_check_set_and_save_options(BDD_ELFWD_OPT_ALL);
  }

  /* construction */
  cls = Ltl_StructCheckLtlSpec_create(prop);

  /* setup options */
  /* These are now default options.. */
  /* Ltl_StructCheckLtlSpec_set_oreg2smv(cls, ltl2smv); */
  /* Ltl_StructCheckLtlSpec_set_ltl2smv(cls, NULL); */
  /* Ltl_StructCheckLtlSpec_set_negate_formula(cls, true); */
  /* Ltl_StructCheckLtlSpec_set_do_rewriting(cls, true); */

  /* action */
  Ltl_StructCheckLtlSpec_build(cls);
  Ltl_StructCheckLtlSpec_check(cls);
  Ltl_StructCheckLtlSpec_print_result(cls);

  if (bdd_isnot_false(cls->dd, cls->s0) &&
      opt_counter_examples(OptsHandler_get_instance())) {

    SexpFsm_ptr sexp_fsm; /* needed for trace lanugage */
    sexp_fsm = Prop_get_scalar_sexp_fsm(prop);
    /* The scalar fsm is set within the property by the
       Ltl_StructCheckLtlSpec_build procedure. It must exist. */
    SEXP_FSM_CHECK_INSTANCE(sexp_fsm);

    Ltl_StructCheckLtlSpec_explain(cls, SexpFsm_get_symbols_list(sexp_fsm));
  }

  /* cleanup */
  Ltl_StructCheckLtlSpec_destroy(cls);

  /* restore settings */
  if (elfwd_saved_options != (BddELFwdSavedOptions_ptr) NULL) {
    Bdd_elfwd_restore_options(BDD_ELFWD_OPT_ALL, elfwd_saved_options);
  }
}

/**Function********************************************************************

  Synopsis           [Print the LTL specification.]

  Description        [Print the LTL specification.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void print_ltlspec(FILE* file, Prop_ptr prop)
{
  indent(file);
  fprintf(file, "LTL specification ");
  Prop_print(prop, file, get_prop_print_method(OptsHandler_get_instance()));
}

/**Function********************************************************************

  Synopsis           [Create an empty Ltl_StructCheckLtlSpec structure.]

  Description        [Create an empty Ltl_StructCheckLtlSpec structure.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Ltl_StructCheckLtlSpec_ptr Ltl_StructCheckLtlSpec_create(Prop_ptr prop)
{
  Ltl_StructCheckLtlSpec_ptr res;

  res = ALLOC(Ltl_StructCheckLtlSpec, 1);
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(res);

  ltl_structcheckltlspec_init(res, prop);

  return res;
}

/**Function********************************************************************

  Synopsis           [Desrtroy an Ltl_StructCheckLtlSpec structure.]

  Description        [Desrtroy an Ltl_StructCheckLtlSpec structure.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Ltl_StructCheckLtlSpec_destroy(Ltl_StructCheckLtlSpec_ptr self)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  ltl_structcheckltlspec_deinit(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis           [Set the oreg2smv field of an Ltl_StructCheckLtlSpec structure]

  Description        [Set the oreg2smv field of an Ltl_StructCheckLtlSpec structure]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Ltl_StructCheckLtlSpec_set_oreg2smv(Ltl_StructCheckLtlSpec_ptr self,
                                        Ltl_StructCheckLtlSpec_oreg2smv oreg2smv)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  self->oreg2smv = oreg2smv;
}

/**Function********************************************************************

  Synopsis           [Set the ltl2smv field of an Ltl_StructCheckLtlSpec structure]

  Description        [Set the ltl2smv field of an Ltl_StructCheckLtlSpec structure]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Ltl_StructCheckLtlSpec_set_ltl2smv(Ltl_StructCheckLtlSpec_ptr self,
                                       Ltl_StructCheckLtlSpec_ltl2smv ltl2smv)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  self->ltl2smv = ltl2smv;
}

/**Function********************************************************************

  Synopsis           [Set the negate_formula field of an Ltl_StructCheckLtlSpec structure]

  Description        [Set the negate_formula field of an Ltl_StructCheckLtlSpec structure]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Ltl_StructCheckLtlSpec_set_negate_formula(Ltl_StructCheckLtlSpec_ptr self,
                                               boolean negate_formula)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  self->negate_formula = negate_formula;
}

/**Function********************************************************************

  Synopsis           [Set the do_rewriting field of an Ltl_StructCheckLtlSpec
  structure]

  Description        [Set the do_rewriting field of an Ltl_StructCheckLtlSpec
  structure]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Ltl_StructCheckLtlSpec_set_do_rewriting(Ltl_StructCheckLtlSpec_ptr self,
                                            boolean do_rewriting)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  self->do_rewriting = do_rewriting;
}

/**Function********************************************************************

  Synopsis           [Get the s0 field of an Ltl_StructCheckLtlSpec structure]

  Description        [Get the s0 field of an Ltl_StructCheckLtlSpec structure
  Returned bdd is NOT referenced.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr Ltl_StructCheckLtlSpec_get_s0(Ltl_StructCheckLtlSpec_ptr self)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  return self->s0;
}

/**Function********************************************************************

  Synopsis           [Get the s0 field purified by tableu variables]

  Description        [Get the s0 field  of an Ltl_StructCheckLtlSpec structure
  purified by tableu variables]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr Ltl_StructCheckLtlSpec_get_clean_s0(Ltl_StructCheckLtlSpec_ptr self)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  return ltl_clean_bdd(self, self->s0);
}

/**Function********************************************************************

  Synopsis           [Initialize the structure by computing the tableau for
  the LTL property]

  Description        [Initialize the structure by computing the tableau for
  the LTL property and computing the cross-product with the FSM of the model.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Ltl_StructCheckLtlSpec_build(Ltl_StructCheckLtlSpec_ptr self)
{
  int res = 0; /* suppress warning*/

  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);
  nusmv_assert(self->prop != NULL);
  nusmv_assert(self->oreg2smv != NULL);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "evaluating ");
    print_ltlspec(nusmv_stderr, self->prop);
    fprintf(nusmv_stderr, "\n");
  }

  CATCH {
    res = ltl_structcheckltlspec_build_tableau_and_prop_fsm(self);
  }
  FAIL {
    ltl_structcheckltlspec_remove_layer(self);
    fprintf(nusmv_stderr, "An error occured during tableau construction.\n");
    nusmv_exit(1);
  }

  if (res == 1) {
    ltl_structcheckltlspec_remove_layer(self);
    fprintf(nusmv_stderr,
            "Ltl_CheckLtlSpec: Problems in Tableau generation.\n");
    nusmv_exit(1);
  }
}


/**Function********************************************************************

  Synopsis           [Perform the check to see wether the property holds or not]

  Description        [Perform the check to see wether the property holds or not.
  Assumes the Ltl_StructcCheckLtlSpec structure being initialized before with
  Ltl_StructCheckLtlSpec_build.

  If compassion is present it calls the check method for compassion,
  otherwise the check method dedicated to the algorithm given by the
  value of the oreg_justice_emptiness_bdd_algorithm option. ]

  SideEffects        []

  SeeAlso            [ltl_stuctcheckltlspec_check_compassion,
  ltl_structcheckltlspec_check_el_bwd, ltl_structcheckltlspec_check_el_fwd]

******************************************************************************/
void Ltl_StructCheckLtlSpec_check(Ltl_StructCheckLtlSpec_ptr self)
{
  boolean full_fairness;

  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);
  BDD_FSM_CHECK_INSTANCE(self->fsm);

  /* If the compassion list is not empty, then activate the full
     fairness algorithm. */
  full_fairness =
    ! FairnessList_is_empty(FAIRNESS_LIST(BddFsm_get_compassion(self->fsm)));

  if (full_fairness) {
    ltl_structcheckltlspec_check_compassion(self);
  } else {
    /* Check fairness states to be empty or not */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
       bdd_ptr fair =  BddFsm_get_fair_states(self->fsm);

       if (bdd_is_false(self->dd, fair)) {
         warning_fsm_fairness_empty();
       }

       bdd_free(self->dd, fair);
    }

    /* [VS] Making the switch here is probably ok for algorithms
       that vary only the fixed point computation. For things like
       l2s it probably has to be moved up to Ltl_CheckLtlSpec (and
       others). But then algorithms so much different might have
       dedicated NuSMV shell commands, so keeping it here for
       now. */
    switch(get_oreg_justice_emptiness_bdd_algorithm(OptsHandler_get_instance())) {
    case BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD:
      ltl_structcheckltlspec_check_el_bwd(self);
      break;
    case BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD:
      ltl_structcheckltlspec_check_el_fwd(self);
      break;
    default:
      error_unreachable_code();
      break;
    }
  }

  if (bdd_is_false(self->dd, self->s0)) {
    Prop_set_status(self->prop, Prop_True);
  }
  else {
    Prop_set_status(self->prop, Prop_False);
  }
}


/**Function********************************************************************

  Synopsis           [Prints the result of the Ltl_StructCheckLtlSpec_check fun]

  Description        [Prints the result of the Ltl_StructCheckLtlSpec_check fun]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Ltl_StructCheckLtlSpec_print_result(Ltl_StructCheckLtlSpec_ptr self)
{

  /* Prints out the result, if not true explain. */
  fprintf(nusmv_stdout, "-- ");
  print_spec(nusmv_stdout, self->prop);

  if (Prop_get_status(self->prop) == Prop_True){
    fprintf(nusmv_stdout, "is true\n");
  }
  else {
    fprintf(nusmv_stdout, "is false\n");
  }

  fflush(nusmv_stdout);
  fflush(nusmv_stderr);
}

/**Function********************************************************************

  Synopsis           [Perform the computation of a witness for a property]

  Description        [Perform the computation of a witness for a property.
  Assumes the Ltl_StructcCheckLtlSpec structure being initialized before with
  Ltl_StructCheckLtlSpec_build, and that Ltl_StructCheckLtlSpec_build has been
  invoked.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Trace_ptr
Ltl_StructCheckLtlSpec_build_counter_example(Ltl_StructCheckLtlSpec_ptr self,
                                             NodeList_ptr symbols)
{
  boolean full_fairness;
  node_ptr exp;
  Trace_ptr trace;
  char* trace_title = NULL;
  char* trace_title_postfix = " Counterexample";
  bdd_ptr tmp;

  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  full_fairness =
    ! FairnessList_is_empty(FAIRNESS_LIST(BddFsm_get_compassion(self->fsm)));

  nusmv_assert(opt_counter_examples(OptsHandler_get_instance()));
  nusmv_assert(bdd_isnot_false(self->dd, self->s0));
  /* Counterexample construction for forward Emerson-Lei not yet
     implemented. */
  nusmv_assert(full_fairness ||
   !(get_oreg_justice_emptiness_bdd_algorithm(OptsHandler_get_instance()) ==
   BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD));

  tmp = BddEnc_pick_one_state(self->bdd_enc, self->s0);
  bdd_free(self->dd, self->s0);
  self->s0 = tmp;

  if (full_fairness) {
    exp = witness(self->fsm, self->bdd_enc, self->s0);
  }
  else {
    bdd_ref(self->s0); /* to put s0 in the list */
    exp = reverse(explain(self->fsm, self->bdd_enc, cons((node_ptr)self->s0, Nil),
                          self->spec_formula, Nil));
  }
  if (exp == Nil) {
    /* The counterexample consists of one initial state */
    exp = cons((node_ptr)self->s0, Nil);
  }

  /* removes all the tableau variables from the result before
     building the resulting trace. This will make simulation
     working, but may show unexistent loops in the shown trace */
  {
    node_ptr iter = exp;

    while (iter != Nil) {
      bdd_ptr sit;
      bdd_ptr si;

      nusmv_assert(node_get_type(iter) == CONS);
      sit = (bdd_ptr) car(iter);
      si = ltl_clean_bdd(self, sit);

      bdd_free(self->dd, sit);
      node_bdd_setcar(iter, si);

      iter = cdr(iter);
    }
  }

  /* The trace title depends on the property type. For example it
     is in the form "LTL Counterexample" */
  trace_title = ALLOC(char, strlen(Prop_get_type_as_string(self->prop)) +
                      strlen(trace_title_postfix) + 1);
  nusmv_assert(trace_title != (char*) NULL);
  strcpy(trace_title, Prop_get_type_as_string(self->prop));
  strcat(trace_title, trace_title_postfix);

  trace = Mc_create_trace_from_bdd_state_input_list(self->bdd_enc, symbols,
                                        trace_title, TRACE_TYPE_CNTEXAMPLE, exp);

  FREE(trace_title);

  walk_dd(self->dd, bdd_free, exp);
  free_list(exp);

  return trace;
}



/**Function********************************************************************

  Synopsis           [Perform the computation of a witness for a property]

  Description        [Perform the computation of a witness for a property.
  Assumes the Ltl_StructcCheckLtlSpec structure being initialized before with
  Ltl_StructCheckLtlSpec_build, and that Ltl_StructCheckLtlSpec_build has been
  invoked.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Ltl_StructCheckLtlSpec_explain(Ltl_StructCheckLtlSpec_ptr self,
                                        NodeList_ptr symbols)
{
  Trace_ptr trace;

  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  trace = Ltl_StructCheckLtlSpec_build_counter_example(self, symbols);

  fprintf(nusmv_stdout,
          "-- as demonstrated by the following execution sequence\n");

  TraceManager_register_trace(global_trace_manager, trace);
  TraceManager_execute_plugin(global_trace_manager, TRACE_OPT(NULL),
                              TRACE_MANAGER_DEFAULT_PLUGIN,
                              TRACE_MANAGER_LAST_TRACE);

  Prop_set_trace(self->prop, Trace_get_id(trace));
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



/**Function********************************************************************

  Synopsis           [Main routine to add the tableau to the FSM]

  Description        [The bdd fsm into the property will change]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static BddFsm_ptr ltlPropAddTableau(Ltl_StructCheckLtlSpec_ptr self,
                                    FlatHierarchy_ptr hierarchy)
{
  SexpFsm_ptr tableau_sexp_fsm;
  BddFsm_ptr prop_bdd_fsm = BDD_FSM(NULL);
  BddFsm_ptr tableau_bdd_fsm = BDD_FSM(NULL);
  TransType  trans_type;
  BoolEnc_ptr bool_enc;

  bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self->bdd_enc));

  /*
   * After introducing all new variables, commit tableau_layer.
   */
  BaseEnc_commit_layer(BASE_ENC(bool_enc),
                       SymbLayer_get_name(self->tableau_layer));
  BaseEnc_commit_layer(BASE_ENC(self->bdd_enc),
                       SymbLayer_get_name(self->tableau_layer));

  prop_bdd_fsm = Prop_get_bdd_fsm(self->prop);
  BDD_FSM_CHECK_INSTANCE(prop_bdd_fsm);

  { /* Forces all the variable in the model to be declared also in
       the tableau hierarchy.  Notice that thus the set of
       variables of the tableau hiearchy results a superset of the
       needed set, however this approximation helps performances,
       as calculate the exact dependencies of the LTL formula may
       cost much more then approximating */
    FlatHierarchy_ptr fh_model =
      SexpFsm_get_hierarchy(Prop_get_scalar_sexp_fsm(self->prop));
    Set_t vars = FlatHierarchy_get_vars(fh_model);
    Set_Iterator_t iter;
    SET_FOREACH(vars, iter) {
      FlatHierarchy_add_var(hierarchy, Set_GetMember(vars, iter));
    }
  }

  /* Creation of the corresponding FSMs: */
  tableau_sexp_fsm = SexpFsm_create(hierarchy,
                                    FlatHierarchy_get_vars(hierarchy));

  trans_type =
    GenericTrans_get_type( GENERIC_TRANS(BddFsm_get_trans(prop_bdd_fsm)) );

  tableau_bdd_fsm = FsmBuilder_create_bdd_fsm(global_fsm_builder,
                                              self->bdd_enc,
                                              tableau_sexp_fsm, trans_type);

  /* Carries out the reversed synchronous product. This is correct, because
     we are only interested in determining if M x T |= EG True */
  BddFsm_apply_synchronous_product(tableau_bdd_fsm, prop_bdd_fsm);

  return tableau_bdd_fsm;
}



/**Function********************************************************************

  Synopsis   [Takes a LTL formula and applies rewriting to get rid of
  input variables from the formula]

  Description [Rewriting makes side-effect on given hierarchy, and
  can declare new variables inside given layer.
  The resulting expression is flattened and define expanded.
  Invoker has to free returned expression exactly as if it was created by
  Compile_FlattenSexpExpandDefine.]

  SideEffects [layer and outfh are expected to get changed]

  SeeAlso     [Compile_FlattenSexpExpandDefine]

******************************************************************************/
Expr_ptr Ltl_apply_input_vars_rewriting(Expr_ptr spec, SymbTable_ptr st,
                                        SymbLayer_ptr layer,
                                        FlatHierarchy_ptr outfh)
{
  node_ptr ltl_init  = Nil;
  node_ptr ltl_invar = Nil;
  node_ptr ltl_trans = Nil;

  Expr_ptr respec = Ltl_RewriteInput(st, spec, layer,
                                     &ltl_init, &ltl_invar, &ltl_trans,
                                     LTL_REWRITE_STANDARD);

  /* If rewriting had effect, update the output hierarchy accordingly */
  if (ltl_init != Nil) {
    ltl_init = Expr_and_nil(ltl_init, FlatHierarchy_get_init(outfh));
  }
  if (ltl_invar != Nil) {
    ltl_invar = Expr_and_nil(ltl_invar, FlatHierarchy_get_invar(outfh));
  }
  if (ltl_trans != Nil) {
    ltl_trans = Expr_and_nil(ltl_trans, FlatHierarchy_get_trans(outfh));
  }

  { /* adds the language coming from the FSM */
    node_ptr fsm[] = { ltl_init, ltl_invar, ltl_trans };
    int i;
    for (i=0; i<sizeof(fsm)/sizeof(fsm[0]); ++i) {
      Set_t deps = Formula_GetDependencies(st, fsm[i], Nil);
      Set_Iterator_t iter;
      SET_FOREACH(deps, iter) {
        FlatHierarchy_add_var(outfh, Set_GetMember(deps, iter));
      }
      Set_ReleaseSet(deps);
    }
  }

  /* finally sets the fsm within the hierarchy */
  if (ltl_init != Nil) FlatHierarchy_set_init(outfh, ltl_init);
  if (ltl_invar != Nil) FlatHierarchy_set_invar(outfh, ltl_invar);
  if (ltl_trans != Nil) FlatHierarchy_set_trans(outfh, ltl_trans);

  return respec;
}


/**Function********************************************************************

  Synopsis [Takes a formula (with context) and constructs the flat
  hierarchy from it.

  Description        []

  SideEffects        [layer and outfh are expected to get changed]

  SeeAlso            []

******************************************************************************/
void Ltl_spec_to_hierarchy(Expr_ptr spec, node_ptr context,
                           SymbTable_ptr st,
                           node_ptr (*what2smv)(unsigned int id, node_ptr expr),
                           SymbLayer_ptr layer,
                           FlatHierarchy_ptr outfh)
{
  static unsigned int ltl_spec_counter = -1;

  node_ptr module;
  char* module_name;
  FlatHierarchy_ptr modfh;
  int c, module_name_len = strlen(LTL_MODULE_BASE_NAME)+7;

  if (Expr_is_true(spec)) return; /* nothing to be done */

  module_name = ALLOC(char, module_name_len);
  if (module_name == (char*) NULL) {
    internal_error("Unable to allocate module name.");
  }

  ltl_spec_counter += 1;
  c = snprintf(module_name, module_name_len, "%s%u", LTL_MODULE_BASE_NAME, ltl_spec_counter);
  SNPRINTF_CHECK(c, module_name_len);

  /* constructs the module */
  module = what2smv(ltl_spec_counter, spec);

  /* we insert the definition of the current module in the
     module_hash in order to make it available for the
     Compile_FlattenHierarchy routines. */
  CompileFlatten_hash_module(module);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Flattening the generated tableau....");
  }

  /* call Compile_FlattenTableau with the name of the generated
     tableau, and as root name the actual property context. In this
     way local variables of the tableau and local variables of the
     formula will be contextualized to the right module. */
  modfh = Compile_FlattenHierarchy(st, layer,
                sym_intern(module_name),
                context,
                Nil, /* no actual */
                false /*do not create process vars*/,
                true /* carries out calc of vars constr now */,
                HRC_NODE(NULL) /* hrc structure must not be constructed */);
  FREE(module_name);

  FlatHierarchy_mergeinto(outfh, modfh);
  FlatHierarchy_destroy(modfh);
}


/**Function********************************************************************

  Synopsis           [Creates the tableau]

  Description [Creates the tableau for a LTL property.  The FSM of the
  property contains the tableau. Returns 1 if an error is encountered
  during the tableau generation, 0 otherwise]

  SideEffects        [The bdd fsm into the prop will change]

  SeeAlso            []

******************************************************************************/
static int
ltl_structcheckltlspec_build_tableau_and_prop_fsm(Ltl_StructCheckLtlSpec_ptr self)
{
  FlatHierarchy_ptr hierarchy;
  Expr_ptr spec;
  Expr_ptr ltl_formula;
  node_ptr context;

  hierarchy = FlatHierarchy_create(self->symb_table);

  /* performs input rewriting if required */
  if (self->do_rewriting) {
    spec = Prop_get_expr_core(self->prop);
    spec = Ltl_apply_input_vars_rewriting(spec, self->symb_table,
                                          self->tableau_layer,
                                          hierarchy);
  }
  else {
  /* We consider the parsed expression.
     oreg2smv should take care of the case in which the spec is not LTL*/
    spec = Prop_get_expr(self->prop);
  }

  /* finds the context if any */
  if (node_get_type(spec) == CONTEXT) {
    context     = car(spec);
    ltl_formula = cdr(spec);
  }
  else {
    context = Nil;
    ltl_formula = spec;
  }

  /* the formula has to be negated */
  if (self->negate_formula) {
    ltl_formula = Expr_not(ltl_formula);
  }

  /* time to construct the tableau of the (negated) formula */
  Ltl_spec_to_hierarchy(ltl_formula, context,
                        self->symb_table, self->oreg2smv,
                        self->tableau_layer,
                        hierarchy);

  /* handle potentially remaining LTL part (e.g. for PSL) */
  if (Nil != FlatHierarchy_get_ltlspec(hierarchy)) {
    Expr_ptr conj_ltlspecs;
    node_ptr iter;

    nusmv_assert(false == self->do_rewriting);
    nusmv_assert(NULL != self->ltl2smv);

    /* calculates the conjuction of all LTLs */
    conj_ltlspecs = Expr_true();
    for (iter=FlatHierarchy_get_ltlspec(hierarchy); Nil != iter;
         iter = cdr(iter)) {
      node_ptr ctxexpr;
      node_ptr ctx;
      node_ptr expr;

      nusmv_assert(CONS == node_get_type(iter));
      ctxexpr = car(iter);

      /* [AM] Added support for named properties: Tree has now a new
       * node of type LTLSPEC before the CONTEXT node. Property name
       * can be found in the right part of the LTLSPEC node, while the
       * old CONTEXT node is on it's left part
       */
      nusmv_assert(Nil != ctxexpr);
      nusmv_assert(LTLSPEC == node_get_type(ctxexpr));
      ctxexpr = car(ctxexpr);

      nusmv_assert(Nil != ctxexpr);
      nusmv_assert(CONTEXT == node_get_type(ctxexpr));
      ctx = car(ctxexpr);
      nusmv_assert(Nil == ctx);
      expr = cdr(ctxexpr);
      conj_ltlspecs = Expr_and(conj_ltlspecs, expr);
    }

    /* the ltlspecs are no longer needed and must be removed */
    FlatHierarchy_set_ltlspec(hierarchy, Nil);

    /* time to append (and merge) the tableau of the negated LTLs */
    conj_ltlspecs = Expr_not(conj_ltlspecs);
    Ltl_spec_to_hierarchy(conj_ltlspecs, Nil /* nil context */,
                          self->symb_table,
                          self->ltl2smv,
                          self->tableau_layer,
                          hierarchy);
  }

  /* ------------------------------------------------------------ */
  /* Some checks on the generated hierarchy */
  /* Check if we are using an old version of ltl2smv */
  if (FlatHierarchy_get_spec(hierarchy) != Nil) {
    internal_error("Error: CTL specification in tableau construction"
                   " (check version of ltl2smv)\n");
  }
  nusmv_assert(Nil == FlatHierarchy_get_ltlspec(hierarchy));
  nusmv_assert(Nil == FlatHierarchy_get_invarspec(hierarchy));
  nusmv_assert(Nil == FlatHierarchy_get_pslspec(hierarchy));
  nusmv_assert(Nil == FlatHierarchy_get_compute(hierarchy));
  /* ------------------------------------------------------------ */


  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, ".... done\n");
    fprintf(nusmv_stderr, "Creating LTL tableau variables...\n");
  }

  /* The error trapping mechanism is enough in this block. All the
     other errors even external to this block are trapped and the
     else of the CATCH is executed. */
  CATCH {
    self->fsm = ltlPropAddTableau(self, hierarchy);
  }
  FAIL {
    FlatHierarchy_destroy(hierarchy);
    return 1;
  }

  FlatHierarchy_destroy(hierarchy);
  return 0;
}

/**Function********************************************************************

  Synopsis           [Perform the check to see wether the property holds or
  not using an algorithm for strong fairness]

  Description        [Assumes the Ltl_StructcCheckLtlSpec structure being
  initialized before with Ltl_StructCheckLtlSpec_build. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
ltl_structcheckltlspec_check_compassion(Ltl_StructCheckLtlSpec_ptr self)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);
  nusmv_assert(!FairnessList_is_empty(FAIRNESS_LIST(BddFsm_get_compassion(self->fsm))));

  self->s0 = feasible(self->fsm, self->bdd_enc);
}

/**Function********************************************************************

  Synopsis           [Perform the check to see wether the property holds or
  not using the backward Emerson-Lei algorithm]

  Description        [Assumes the Ltl_StructcCheckLtlSpec structure being
  initialized before with Ltl_StructCheckLtlSpec_build. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
ltl_structcheckltlspec_check_el_bwd(Ltl_StructCheckLtlSpec_ptr self)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);
  nusmv_assert(FairnessList_is_empty(FAIRNESS_LIST(BddFsm_get_compassion(self->fsm))));

  bdd_ptr tmp;
  self->spec_formula =
    find_node(NOT,
              find_node(EG,
                        find_node(TRUEEXP,Nil,Nil), Nil), Nil);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    Prop_ptr phi = Prop_create_partial(self->spec_formula, Prop_Ctl);
    fprintf(nusmv_stderr, "Checking CTL ");
    print_spec(nusmv_stderr, phi);
    fprintf(nusmv_stderr, " generated from the tableau.\n");
    Prop_destroy(phi);
  }

  /* Verification of the property: */
  CATCH {
   self->s0 = eval_ctl_spec(self->fsm,
                            self->bdd_enc,
                            self->spec_formula,
                            Nil);
  }
  FAIL {
    ltl_structcheckltlspec_remove_layer(self);
    fprintf(nusmv_stderr,
            "ltl_checkltlspec_el_bwd: Problems in Tableau verification.\n");
    nusmv_exit(1);
    return;
  }

  /* Negate the result */
  tmp = bdd_not(self->dd, self->s0);
  bdd_free(self->dd, self->s0);
  self->s0 = tmp;

  /* Intersect with init, invar and fair states */
  {
    bdd_ptr init  = BddFsm_get_init(self->fsm);
    bdd_ptr invar = BddFsm_get_state_constraints(self->fsm);
    bdd_ptr fair =  BddFsm_get_fair_states(self->fsm);

    bdd_and_accumulate(self->dd, &(self->s0), init);
    bdd_and_accumulate(self->dd, &(self->s0), invar);
    bdd_and_accumulate(self->dd, &(self->s0), fair);
    bdd_free(self->dd, fair);
    bdd_free(self->dd, invar);
    bdd_free(self->dd, init);
  }
}

/**Function********************************************************************

  Synopsis           [Perform the check to see wether the property holds or
  not using the forward Emerson-Lei algorithm]

  Description        [Assumes the Ltl_StructcCheckLtlSpec structure being
  initialized before with Ltl_StructCheckLtlSpec_build. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
ltl_structcheckltlspec_check_el_fwd(Ltl_StructCheckLtlSpec_ptr self)
{
  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);
  nusmv_assert(FairnessList_is_empty(FAIRNESS_LIST(BddFsm_get_compassion(self->fsm))));
  nusmv_assert(Bdd_elfwd_check_options(BDD_ELFWD_OPT_FORWARD_SEARCH |
                                       BDD_ELFWD_OPT_LTL_TABLEAU_FORWARD_SEARCH |
                                       BDD_ELFWD_OPT_USE_REACHABLE_STATES,
                                       false));

  /* Verification of the property: */
  CATCH {
    /* The result below is the set of states that can be reached from
       a rechable cycle through all Buechi fairness
       constraints. Hence, if self->s0 is not empty, then the language
       of the transition system is not empty. */
    self->s0 = BddFsm_get_revfair_states(self->fsm);
  }
  FAIL {
    ltl_structcheckltlspec_remove_layer(self);
    fprintf(nusmv_stderr,
            "ltl_checkltlspec_el_fwd: Problems in Tableau verification.\n");
    nusmv_exit(1);
    return;
  }
}

/**Function********************************************************************

  Synopsis           [Private service that removes the given layer from
  the symbol table, and from both the boolean and bdd encodings.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ltl_structcheckltlspec_remove_layer(Ltl_StructCheckLtlSpec_ptr self)
{
  BoolEnc_ptr bool_enc;

  LTL_STRUCTCHECKLTLSPEC_CHECK_INSTANCE(self);

  nusmv_assert(!self->removed_layer);

  bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self->bdd_enc));

  if (BaseEnc_layer_occurs(BASE_ENC(self->bdd_enc),
                           SymbLayer_get_name(self->tableau_layer))) {
    BaseEnc_remove_layer(BASE_ENC(self->bdd_enc),
                         SymbLayer_get_name(self->tableau_layer));
  }

  if (BaseEnc_layer_occurs(BASE_ENC(bool_enc),
                           SymbLayer_get_name(self->tableau_layer))) {
    BaseEnc_remove_layer(BASE_ENC(bool_enc),
                         SymbLayer_get_name(self->tableau_layer));
  }

  /* remove tableau layer from symbol table */
  if (SymbTable_layer_class_exists(self->symb_table, ARTIFACTS_LAYERS_CLASS)) {
    SymbTable_layer_remove_from_class(self->symb_table,
                                      SymbLayer_get_name(self->tableau_layer),
                                      ARTIFACTS_LAYERS_CLASS);
  }

  SymbTable_remove_layer(self->symb_table, self->tableau_layer);

  self->removed_layer = true;
}



/**Function********************************************************************

  Synopsis           [Quantify out tableau variables]

  Description        [Quantify out tableau variables]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static bdd_ptr ltl_clean_bdd(Ltl_StructCheckLtlSpec_ptr self, bdd_ptr bdd)
{
  BddVarSet_ptr tableau_cube;
  bdd_ptr res;

  tableau_cube = BddEnc_get_layer_vars_cube(self->bdd_enc,
                                            self->tableau_layer,
                                            VFT_ALL);

  res = bdd_forsome(self->dd, bdd, tableau_cube);

  return res;
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void ltl_structcheckltlspec_init(Ltl_StructCheckLtlSpec_ptr self,
                                        Prop_ptr prop)
{
  self->prop = prop;
  self->fsm = BDD_FSM(NULL);
  self->bdd_enc = BDD_ENC(NULL);
  self->dd = (DdManager*)NULL;

  self->symb_table = SYMB_TABLE(NULL);
  self->tableau_layer = SYMB_LAYER(NULL);

  self->removed_layer = false;
  self->spec_formula = Nil;
  self->oreg2smv = ltl2smv;
  self->ltl2smv = NULL;
  self->negate_formula = true;
  self->do_rewriting = true;

  ltl_structcheckltlspec_prepare(self);
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void ltl_structcheckltlspec_deinit(Ltl_StructCheckLtlSpec_ptr self)
{
  if (!self->removed_layer) {
    ltl_structcheckltlspec_remove_layer(self);
  }

  if (BDD_FSM(NULL) != self->fsm) {
    BddFsm_destroy(self->fsm);
  }
  bdd_free(self->dd, self->s0);
}

/**Function********************************************************************

  Synopsis           [Support function for the init function]

  Description        [Support function for the init function]

  SideEffects        []

  SeeAlso            [ltl_structcheckltlspec_init]

******************************************************************************/
static void ltl_structcheckltlspec_prepare(Ltl_StructCheckLtlSpec_ptr self)
{
  BddFsm_ptr bdd_fsm = BDD_FSM(NULL);
  SexpFsm_ptr sexp_fsm = SEXP_FSM(NULL);

  /* Prepare here all structures needed for the LTL MC.*/

  /* Prepare property's FSMs, if needed. Prop_compute_ground_bdd_fsm
     takes care of this */
  bdd_fsm = Prop_compute_ground_bdd_fsm(self->prop, global_fsm_builder);
  sexp_fsm = Prop_get_scalar_sexp_fsm(self->prop);

  SEXP_FSM_CHECK_INSTANCE(sexp_fsm);
  BDD_FSM_CHECK_INSTANCE(bdd_fsm);

  /* Prepare the ST and the BDD_ENC for later uses */
  self->bdd_enc = BddFsm_get_bdd_encoding(bdd_fsm);
  BDD_ENC_CHECK_INSTANCE(self->bdd_enc);

  self->dd = BddEnc_get_dd_manager(self->bdd_enc);
  nusmv_assert((DdManager*)NULL != self->dd);

  self->symb_table = BaseEnc_get_symb_table(BASE_ENC(self->bdd_enc));
  SYMB_TABLE_CHECK_INSTANCE(self->symb_table);

  /* Now that we have a symbol table, prepare the tableau layer */
  nusmv_assert(SYMB_LAYER(NULL) == self->tableau_layer);
  self->tableau_layer = SymbTable_create_layer(self->symb_table,
                                               NULL /* temp name */,
                                               SYMB_LAYER_POS_BOTTOM);

  /* The tableau layer must be added to ARTIFACTS class in order to strip
   * the tableau symbols from resulting ctx trace (if any). */
  SymbTable_layer_add_to_class(self->symb_table,
                               SymbLayer_get_name(self->tableau_layer),
                               ARTIFACTS_LAYERS_CLASS);

  /* Calculate reachable states from the model fsm, if COI is not
     enabled and reachable states are required. This will disable
     calculation of reachable states in the tableau fsm. If option
     ltl_tableau_forward_search is enabled, reachables will be
     re-calculated when needed */
  if ((!opt_cone_of_influence(OptsHandler_get_instance())) &&
      opt_use_reachable_states(OptsHandler_get_instance()) &&
      !opt_ltl_tableau_forward_search(OptsHandler_get_instance())) {
    BddStates states = BddFsm_get_reachable_states(bdd_fsm);
    bdd_free(self->dd, states);
  }
}
