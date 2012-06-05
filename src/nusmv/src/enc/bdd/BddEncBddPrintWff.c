/**CFile***********************************************************************

  FileName    [BddEncBddPrintWff.c]

  PackageName [bdd.enc]

  Synopsis    [Bdd to Well Formed Formulas conversions/printout]

  Description [This module exports functions to generate or print a
  Bdd as an optimized Well Formed Formula.]

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``bdd.enc'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK.

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

#include <math.h>

#include "enc/enc.h"
#include "enc/bdd/bddInt.h"
#include "utils/utils.h"
#include "utils/assoc.h"
#include "utils/error.h"
#include "parser/symbols.h"
#include "compile/compile.h"

static char rcsid[] UTIL_UNUSED = "$Id: BddEncBddPrintWff.c,v 1.1.2.7 2010-01-21 13:41:44 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [A shorthand to ease reading of bdd_enc_bdd_to_wff_rec]

  Description [Builds a node representing the Bool casting of a single
  word bit.]

  SideEffects  [none]
******************************************************************************/
#define WORD_BIT(wb)                                                    \
  find_node(                                                            \
            CAST_BOOL,                                                  \
            find_node(                                                  \
                      BIT_SELECTION,                                    \
                      (car((wb))),                                      \
                      find_node(                                        \
                                COLON,                                  \
                                find_node(NUMBER, (cdr((wb))), Nil),    \
                                find_node(NUMBER, (cdr((wb))), Nil))),  \
            Nil)

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static bdd_ptr
bdd_enc_get_scalar_essentials ARGS((BddEnc_ptr self, bdd_ptr bdd,
                                    NodeList_ptr vars));
static node_ptr
bdd_enc_bdd_to_wff_rec ARGS((BddEnc_ptr self, NodeList_ptr vars,
                             bdd_ptr bdd, hash_ptr cache));
static assoc_retval
bdd_enc_hash_free_bdd_counted ARGS((char* key, char* data, char* arg));

static NodeList_ptr
bdd_enc_get_preprocessed_vars ARGS((BddEnc_ptr self, NodeList_ptr vars));

#ifdef PRINT_BDD_DEBUG
static void
bdd_enc_debug_bdd_to_wff ARGS((BddEnc_ptr self, bdd_ptr bdd, node_ptr expr));
#endif

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis     [Prints statistical information of a formula.]

   Description  [Prints statistical information about a given formula.
                 It is computed taking care of the encoding and of the
                 indifferent variables in the encoding.]

   SideEffects  []

   SeeAlso      []

******************************************************************************/
void BddEnc_print_formula_info(BddEnc_ptr self,
                               Expr_ptr formula,
                               boolean print_models,
                               boolean print_formula,
                               FILE* out)
{
  bdd_ptr phi;
  double cardinality;

  phi = BddEnc_expr_to_bdd(self, formula, Nil);
  cardinality = BddEnc_get_minterms_of_bdd(self, phi);

  fprintf(out, "formula models: %g (2^%g)\n",
          cardinality, log(cardinality)/log(2.0));

  /* one of these flags can be enabled, not both */
  nusmv_assert(!print_models || !print_formula);
  if (print_models) {
    BddEnc_print_set_of_trans_models(self, phi, /* false, */ out);
  }
  else if (print_formula) {
    BoolEnc_ptr benc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self));

    const array_t* layer_names =
      BaseEnc_get_committed_layer_names(BASE_ENC(self));

    SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(self));
    NodeList_ptr all_vars = SymbTable_get_layers_sf_vars(st, layer_names);
    NodeList_ptr scalar_vars = NodeList_create();
    ListIter_ptr iter;

    /* encoding variables are not allowed in the wff printer */
    NODE_LIST_FOREACH(all_vars, iter) {
      node_ptr v = NodeList_get_elem_at(all_vars, iter);
      if (BoolEnc_is_var_bit(benc, v)) continue;
      NodeList_append(scalar_vars, v);
    }
    NodeList_destroy(all_vars);

    fprintf(nusmv_stdout, "\nFORMULA = \n");
    BddEnc_print_bdd_wff(self, phi, scalar_vars,
                         true, false, 0, out);

    NodeList_destroy(scalar_vars);
  }

  bdd_free(BddEnc_get_dd_manager(self), phi);
}


/**Function********************************************************************

  Synopsis [Prints a BDD as a Well Formed Formula using optional
  sharing]

  Description [The bdd representing the formula to be printed is first
  converted to a wff.

  If sharing is required optimizations are performed on the printout.

  If indentation is required, the start_at_column integer offset is
  used to determine the starting indenting offset to print the
  expression.]

  SideEffects        [prints the expression on the given stream. ]

  SeeAlso            [BddEnc_bdd_to_wff]

******************************************************************************/
void BddEnc_print_bdd_wff(BddEnc_ptr self,
                          bdd_ptr bdd,         /* the input bdd */
                          NodeList_ptr vars,   /* variables */
                          boolean do_sharing,  /* requires dag sharing */
                          boolean do_indent,   /* requires indentation */
                          int start_at_column, /* align to column (indent only) */
                          FILE* out            /* the stream to write to */
                          )
{
  node_ptr expr = BddEnc_bdd_to_wff(self, bdd, vars);

  hash_ptr dag_info_hash = NULL;
  hash_ptr defines_hash = NULL;

  /* sharing pre-processing: if sharing is enabled, convert the
   * expression using compile_convert_to_dag.
   **/
  if (do_sharing) {
    dag_info_hash = new_assoc();
    defines_hash = new_assoc();

    Compile_make_dag_info(expr, dag_info_hash);
    node_ptr dag_expr = Compile_convert_to_dag(
                                         BaseEnc_get_symb_table(BASE_ENC(self)),
                                         expr,
                                         dag_info_hash,
                                         defines_hash);

    /* the expression to print is now the result of the previous operation */
    expr = dag_expr;
  }

  /* print the expression using the appropriate function */
  if (do_indent) {
    /* indented printout */
    print_node_indent_at(out, expr, start_at_column);
  }

  else {
    /* non indented printout */
    print_node(out, expr);
  }

  fprintf(out, "\n");

  /* sharing post-processing: if sharing is enabled, print the defines
   * introduced by the dumper.
  **/
  if (do_sharing) {
    fprintf(out, "\n");
    Compile_write_dag_defines(out, defines_hash);
  }

  /* this cleanup is required only if sharing was enabled. */
  if (do_sharing) {
    Compile_destroy_dag_info(dag_info_hash, defines_hash);

    free_assoc(dag_info_hash);
    free_assoc(defines_hash);
  }
}


/**Function********************************************************************

  Synopsis [Converts a bdd into a Well Formed Formula representing
  it.]

  Description [A new expression is built, that represents the formula
  given as the input bdd.

  The list of variables is used to compute the scalar essentials. Note
  that only the following kinds of variables are allowed in this list.

  1. Pure booleans (i.e. not part of an encoding)
  2. Finite scalars (both ranged and words).

  State, frozen and input variables are all allowed, no NEXT. (It will
  be part of this function's responsibility to add state variables' NEXTs
  as needed.

  SideEffects        [none]

  SeeAlso            [Bddenc_print_wff_bdd]

******************************************************************************/
node_ptr BddEnc_bdd_to_wff(
                           BddEnc_ptr self,
                           bdd_ptr bdd,        /* the input bdd */
                           NodeList_ptr vars   /* variables */
                           )
{
  /* DD manager local reference */
  DdManager *dd = BddEnc_get_dd_manager(self);

  if (bdd_is_false(dd, bdd)) {
    return Expr_false();
  }

  if (bdd_is_true(dd, bdd)) {
    return Expr_true();
  }

  /* preprocess variables list (see bdd_enc_get_preprocessed_vars) */
  NodeList_ptr preprocessed_vars = bdd_enc_get_preprocessed_vars(self, vars);

  /* memoized recursive inner function */
  hash_ptr memoization_hash = new_assoc();
  node_ptr res = bdd_enc_bdd_to_wff_rec(
                                        self,
                                        preprocessed_vars,
                                        bdd,
                                        memoization_hash);

#ifdef PRINT_BDD_DEBUG
  /* correctnesss check (this may halt NuSMV) */
  bdd_enc_debug_bdd_to_wff(self, bdd, res);
#endif

  /* cleanup */
  NodeList_destroy(preprocessed_vars);

  /* underef all the BDDs */
  clear_assoc_and_free_entries_arg(
                                   memoization_hash,
                                   bdd_enc_hash_free_bdd_counted,
                                   (char*) dd);
  free_assoc(memoization_hash);

  return res;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Used to deref bdds in the sharing hashtable]

******************************************************************************/

static assoc_retval
bdd_enc_hash_free_bdd_counted (char* key, char* data, char* arg)
{
  bdd_free((DdManager*) arg, (bdd_ptr) key);
  return ASSOC_DELETE;
}

/**Function********************************************************************

  Synopsis           [Compute scalar essentials of a bdd.]

  Description [Computes the scalar essentials of a bdd, picking
  identifiers from the variables in vars list. Used as part of
  bdd_enc_bdd_to_wff_rec implementation.]

  SeeAlso            [bdd_enc_bdd_to_wff]

******************************************************************************/
static bdd_ptr
bdd_enc_get_scalar_essentials(
                              BddEnc_ptr self,  /* current encoding */
                              bdd_ptr bdd,      /* the formula */
                              NodeList_ptr vars /* variables */
                              )
{
  /* the result of this computation */
  bdd_ptr res_bdd;

  /* DD manager local reference */
  DdManager *dd = BddEnc_get_dd_manager(self);

  /* get binary essentials, 0 and 1 need no further processing */
  bdd_ptr work_bdd = bdd_compute_essentials(dd, bdd);
  if (bdd_is_false(dd, work_bdd) ||
      bdd_is_true(dd, work_bdd)) {

    /* res bdd is referenced */
    res_bdd = work_bdd;
    goto leave;
  }

  /* reference res bdd */
  res_bdd = bdd_true(dd);

  bdd_ptr work_cube = bdd_support(dd, work_bdd);
  add_ptr work_cube_add = bdd_to_add(dd, work_cube);

  /* iterate over variables in binary essentials BDD */
  ListIter_ptr i;
  NODE_LIST_FOREACH(vars, i) {
    node_ptr v = (node_ptr) NodeList_get_elem_at(vars, i);

    /* skip vars that do _not_ appear in the binary essentials bdd */
    if (!BddEnc_is_var_in_cube(self, v, work_cube_add))
      continue;

    /* common variable support for both boolean and scalars  */
    bdd_ptr ass_bdd;
    NodeList_ptr singleton = NodeList_create();
    NodeList_append(singleton, v);
    node_ptr ass_list = BddEnc_assign_symbols(self, work_bdd, singleton,
                                              true, &ass_bdd);

    bdd_ptr ass_cube = bdd_support(dd, ass_bdd);

    if(bdd_entailed(dd, work_cube, ass_cube)) {
      bdd_and_accumulate(dd, &res_bdd, ass_bdd);

      /* recalculate essentials cube and its add representation */
      bdd_ptr tmp_bdd = bdd_forsome(dd, work_bdd, ass_cube );

      bdd_free(dd, work_bdd);
      work_bdd = bdd_dup(tmp_bdd);

      bdd_free(dd, work_cube);
      work_cube = bdd_support(dd, work_bdd);

      add_free(dd, work_cube_add);
      work_cube_add = bdd_to_add(dd, work_cube);

      /* cleanup */
      bdd_free(dd, tmp_bdd);
    }

    /* cleanup */
    NodeList_destroy(singleton);
    free_list(ass_list);

    bdd_free(dd, ass_bdd);
    bdd_free(dd, ass_cube);

    /* if work_bdd is one there's no need to
       iterate over remaining variables */
    if (bdd_is_true(dd, work_bdd)) break;
  }

  /* cleanup */
  add_free(dd, work_cube_add);
  bdd_free(dd, work_cube);
  bdd_free(dd, work_bdd);

 leave:
  return res_bdd;
}

/**Function********************************************************************

  Synopsis [Preprocesses variables list, as part of the
  bdd_enc_bdd_to_wff implementation.]

  Description [This function is used to preprocess variables list to
  provide to bddenc_print_wff_bdd. As the algorithm implemented in the
  latter does not support word variables, word variables (if any)
  shall be substituted with their bit variables
  representatives. Moreover, this function takes care of adding NEXT
  variables for state variables. For this reason no NEXT nor BITS are
  allowed as input to this function.

  The result NodeList must be destroyed by the caller.]

  SideEffects        [none]

  SeeAlso            [BddEnc_bdd_to_wff]

******************************************************************************/
static NodeList_ptr
bdd_enc_get_preprocessed_vars(
                              BddEnc_ptr self,  /* current encoding */
                              NodeList_ptr vars /* variables to be preprocessed */
                              )
{
  NodeList_ptr res = NodeList_create();

  /* Symbol table local reference */
  SymbTable_ptr st =
    BaseEnc_get_symb_table(BASE_ENC(self));

  /* Boolean encoding client */
  BoolEnc_ptr benc =
    BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self));

  /* iterate over variables list */
  ListIter_ptr i;
  NODE_LIST_FOREACH(vars, i) {
    node_ptr v = (node_ptr) NodeList_get_elem_at(vars, i);

    /* no next variables allowed here */
    boolean is_next = (node_get_type(v) == NEXT);
    if (is_next) {
      char *var_name = sprint_node(v);
      internal_error("No NEXT variables allowed here (got %s)", var_name);
      FREE(var_name);
    }

    /* no encoding bits allowed here */
    boolean is_enc = BoolEnc_is_var_bit(benc, v);
    if (is_enc) {
      char *var_name = sprint_node(v);
      internal_error("No bit encoding variables allowed here (got %s)", var_name);
      FREE(var_name);
    }

    /* ok, at this point we're sure there's no NEXTs nor encoding
     * BITs. Substitute words with their bit encoding variables. Add
     * NEXTs if var is a state variable.
     */
    boolean is_word = SymbType_is_word(SymbTable_get_var_type(st, v));
    if (is_word) {
      ListIter_ptr j;
      NodeList_ptr bits = BoolEnc_get_var_bits(benc, v);
      NODE_LIST_FOREACH( bits,  j) {
        node_ptr b = (node_ptr) NodeList_get_elem_at(bits, j);

        NodeList_append(res, b);
        if (SymbTable_is_symbol_state_var(st, b)) {
          NodeList_append(
                          res,
                          find_node(
                                    NEXT,
                                    b,
                                    Nil)
                          );
        }
      } NodeList_destroy(bits);
    }

    /* not a word, add variable to res list. Add NEXT if var is a state variable. */
    else {
      NodeList_append(res, v);
      if (SymbTable_is_symbol_state_var(st, v)) {
        NodeList_append(
                        res,
                        find_node(
                                  NEXT,
                                  v,
                                  Nil)
                        );
      }
    }
  }

  return res;
}

#ifdef PRINT_BDD_DEBUG






/**Function********************************************************************

  Synopsis           [Debug code for BddEnc_bdd_to_wff]

  SideEffects [Halts NuSMV if the expression is not a correct
  representation of bdd.]

  SeeAlso            [BddEnc_bdd_to_wff]

******************************************************************************/
static void
bdd_enc_debug_bdd_to_wff (BddEnc_ptr self, bdd_ptr bdd, node_ptr expr)
{
  /* DD manager local reference */
  DdManager *dd = BddEnc_get_dd_manager(self);

  bdd_ptr test_bdd = BddEnc_expr_to_bdd(
                                        self,
                                        expr,
                                        Nil);

#ifndef PRINT_BDD_DEBUG_MASKS
  nusmv_assert(bdd == test_bdd);

#else
  /* apply masks */
  bdd_ptr one_bdd = bdd_true(dd);

  bdd_ptr masked_bdd =
    BddEnc_apply_state_frozen_input_vars_mask_bdd(
                                                  self,
                                                  test_bdd);

  bdd_ptr m2_bdd =
    BddEnc_state_var_to_next_state_var(
                                       self,
                                       BddEnc_apply_state_frozen_vars_mask_bdd(
                                                                               self,
                                                                               one_bdd));
  bdd_and_accumulate(dd, &masked_bdd, m2_bdd);
  nusmv_assert( bdd == masked_bdd );

  bdd_free(dd, one_bdd);
  bdd_free(dd, masked_bdd);
  bdd_free(dd, m2_bdd);
#endif

  bdd_free(dd, test_bdd);
}
#endif

/**Function********************************************************************

  Synopsis [Recursively build a sexp representing a formula encoded as
  a BDD]

  Description [This function accepts a list of variables as part of
its inputs.The present algorithm assumes that a variable in vars list
is a boolean only in two distinct cases:

1. Pure booleans
2. Boolean

variables belonging to words (i.e. Boolean variables belonging to a
scalar encoding are _not_ allowed in the input list of this
function. This would invariably cause this implementation to
fail). This assumptions are enforced by its public top-level caller.]

  SideEffects        [none]

  SeeAlso            [BddEnc_bdd_to_wff]

******************************************************************************/
static node_ptr
bdd_enc_bdd_to_wff_rec(
                       BddEnc_ptr self,   /* The encoding manager */
                       NodeList_ptr vars, /* Variables list (see below) */
                       bdd_ptr bdd,       /* the BDD to be represented*/
                       hash_ptr cache     /* memoization hashtable for DAG traversal */
                       )


{
  /*
   * BASE: if there is a memoized result or input bdd is either zero or one
   * return an expr accordingly, no need to do any further computation.
   * ------------------------------------------------------------------------------
   */
  node_ptr lookup = find_assoc(cache, (node_ptr) bdd);
  if (Nil != lookup) {
    return lookup;
  }

  /* DD manager local reference */
  DdManager *dd = BddEnc_get_dd_manager(self);

  if (bdd_is_true(dd, bdd)) {
    return Expr_true();
  }

  if (bdd_is_false(dd, bdd)) {
    return Expr_false();
  }

  /*
   * STEP: recursively build a sexp representing bdd.
   * ------------------------------------------------------------------------------
   */

  /* Symbol table local reference */
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(self));

  /* Boolean encoding client */
  BoolEnc_ptr benc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self));

  node_ptr res = Expr_true();

  /* a local copy in order not to make
   * side effects on input bdd.
   **/
  bdd_ptr local_bdd = bdd_dup(bdd);

  /* extract scalar essentials (may be zero or one) */
  bdd_ptr essentials = bdd_enc_get_scalar_essentials(self, local_bdd, vars);
  if (!bdd_is_false(dd, essentials) && !bdd_is_true(dd, essentials)) {

    node_ptr avars = BddEnc_assign_symbols(self, essentials, vars, true, NULL);

    /* build the expression with scalar and booleans assignments */
    node_ptr p;
    for (p = avars; Nil != p; p = cdr(p)) {
      node_ptr v = car(p);

      /* the actual variable, its value and name without next */
      node_ptr var_expr = car(v);
      boolean is_next = (node_get_type(var_expr) == NEXT);
      node_ptr var_value = cdr(v);
      node_ptr var_name = (is_next)
        ? car(var_expr)
        : var_expr;

      /* variables are _not_ necessarily bits here... */
      boolean is_bit = SymbTable_is_symbol_bool_var(st, var_name);

      if (is_bit) {
        boolean is_enc = BoolEnc_is_var_bit(benc, var_name);

        /* (is_bit) && (is_enc) -> it's a wordbit */
        if (is_bit && is_enc) {

          node_ptr x = BoolEnc_get_scalar_var_from_bit(benc, var_name);
          SymbType_ptr x_type = SymbTable_get_var_type(st, x);
          nusmv_assert (SymbType_is_word(x_type));

          var_name = WORD_BIT(var_name);

          /* propagate the variable rewrite handling NEXT */
          var_expr = (!is_next)
            ? var_name
            : find_node(NEXT, var_name, Nil);
        }
      }

      res = Expr_and(
                     res,

                     (is_bit)

                     ? /* boolean case */
                     Expr_is_true (var_value) ? var_expr : Expr_not(var_expr)

                     : /* scalar case */
                     Expr_equal(var_expr,
                                var_value,
                                st));


      /* quantify out the essentials variables from the bdd */
      bdd_ptr cube = bdd_support(dd, essentials);
      bdd_ptr tmp = bdd_forsome(dd, local_bdd, cube);

      /* cleanup */
      bdd_free(dd, cube);
      bdd_free(dd, local_bdd);

      local_bdd = tmp;
    } /* if (!bdd_is_false(dd, essentials) && !bdd_is_true(dd, essentials)) */

    /* at this point the bdd may be 1, in this case no further
       computation is required. */
    if (bdd_is_true(dd, local_bdd)) {
      goto leave;
    }
  }

  /* cofactorization */
  node_ptr cofact_expr = Nil;

  int root_ndx = bdd_index(dd, local_bdd);
  node_ptr root_node = BddEnc_get_var_name_from_index (self, root_ndx);

  /* to determine the type of the the variable we need to take care of
   * the intermediate NEXT node, descending on the left branch.
   */
  boolean is_next = node_get_type(root_node) == NEXT;
  node_ptr var_name = (is_next)
    ? car(root_node)
    : root_node;

  /* determine whether this bit is either:
   * 1. a pure boolean variable
   * 2. a bit belonging to a word
   * 3. a bit belonging to a scalar
   */
  boolean is_enc_bit = BoolEnc_is_var_bit(benc, var_name);
  boolean is_pure_bit= (!is_enc_bit);
  boolean is_word_bit = false;

  if (is_enc_bit) {
    node_ptr x = BoolEnc_get_scalar_var_from_bit(benc, var_name);
    SymbType_ptr x_type = SymbTable_get_var_type(st, x);

    /* word bits need some special handling */
    if (SymbType_is_word(x_type)) {
      is_word_bit = true;
      var_name = WORD_BIT(var_name);
    }
  }

  /* boolean case (pure booleans and word elements) */
  if ((is_pure_bit) || (is_word_bit)) {

    node_ptr var_expr = (!is_next)
      ? var_name
      : find_node(NEXT, var_name, Nil);

    bdd_ptr t_bdd = bdd_then(dd, local_bdd);
    bdd_ptr e_bdd = bdd_else(dd, local_bdd);

    /* BDD normalization */
    if (bdd_iscomplement(dd, local_bdd)) {
      t_bdd = bdd_not(dd, t_bdd);
      e_bdd = bdd_not(dd, e_bdd);
    }

    else {
      /* then and else branches need one more ref */
      bdd_ref(t_bdd);
      bdd_ref(e_bdd);
    }

    /* cofactorization is simple in the boolean case */
    cofact_expr = Expr_ite(
                           var_expr,
                           bdd_enc_bdd_to_wff_rec(self, vars, t_bdd, cache),
                           bdd_enc_bdd_to_wff_rec(self, vars, e_bdd, cache),
                           st);

    /* cleanup */
    bdd_free(dd, t_bdd);
    bdd_free(dd, e_bdd);
  }

  /* scalar case */
  else {
    /* from now on var_name will contain the scalar name */
    var_name = BoolEnc_get_scalar_var_from_bit(benc, var_name);

    node_ptr var_expr = (!is_next)
      ? var_name
      : find_node(NEXT, var_name, Nil);

    add_ptr root_add = BddEnc_expr_to_add(self, var_expr, Nil);
    add_ptr tmp_cube = add_support(dd, root_add);
    bdd_ptr root_cube = add_to_bdd(dd, tmp_cube);

    /* cleanup */
    add_free(dd, root_add);
    add_free(dd, tmp_cube);

    SymbType_ptr root_type = SymbTable_get_var_type(st, var_name);
    node_ptr range = SymbType_get_enum_type_values(root_type);

    /* base condition */
    cofact_expr = Expr_false();

    node_ptr p;
    for (p = range; Nil != p; p = cdr(p)) {
      node_ptr x = car(p);

      /* iterate over all possible values in the range of the scalar variable */
      Expr_ptr clause = Expr_equal(var_expr, x, st);

      /* Existentially quantify root cube out of (phi & (root_scalar = x)) */
      bdd_ptr clause_bdd = BddEnc_expr_to_bdd(self, clause, Nil);
      bdd_ptr tmp_bdd = bdd_and_abstract(dd, local_bdd, clause_bdd, root_cube);

      if (bdd_isnot_false(dd, tmp_bdd)) {
        cofact_expr = Expr_ite(clause,
                               bdd_enc_bdd_to_wff_rec(self,
                                                      vars,
                                                      tmp_bdd,
                                                      cache),
                               cofact_expr,
                               st);
      }

      /* cleanup */
      bdd_free(dd, clause_bdd);
      bdd_free(dd, tmp_bdd);
    }

    add_free(dd, root_cube);
  }

  /* cofactorization */
  res = Expr_and(res, cofact_expr);

 leave:
  bdd_free(dd, local_bdd);
  bdd_free(dd, essentials);

  /* memoize the result and leave */
  bdd_ref(bdd);
  insert_assoc( cache, (node_ptr) bdd, res );

  return res;
}

