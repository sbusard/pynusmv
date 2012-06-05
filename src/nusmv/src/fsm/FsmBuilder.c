/**CFile***********************************************************************

  FileName    [FsmBuilder.c]

  PackageName [fsm]

  Synopsis    [High level object that can contruct FSMs]

  Description [Defines an high-level object that lives at
  top-level, that is used to help contruction of FSMs.
  It can control information that are not shared between
  lower levels, so it can handle with objects that have not
  the full knowledge of the whole system]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm'' package of NuSMV version 2.
  Copyright (C) 2006 by FBK-irst.

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

#include "FsmBuilder.h"
#include "fsmInt.h"

#include "fsm/bdd/FairnessList.h"
#include "parser/symbols.h"
#include "parser/idlist/ParserIdList.h" /* to parse trans ordering file */
#include "compile/compile.h"
#include "utils/assoc.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: FsmBuilder.c,v 1.1.2.1.6.21 2010-02-11 17:13:46 nusmv Exp $";


/* Uncomment the define below to enable the creation of a cluster for
   each bit in assignments over word variables, i.e. for x := f(X)
   that corresponds to /\_i x_i <-> f_i(X) it builds a cluster for
   each x_i <-> f_i(X) insteed of creating a unique cluster for x :=
   f(X) */
// #define DO_CLUSTERS_FOR_BITS_IN_EQDEF_AMONG_WORDS 1


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Type************************************************************************

  Synopsis    [FSM builder class constructor]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct FsmBuilder_TAG
{
  DdManager*  dd; /* for performance, readability, etc. */

  hash_ptr simpl_hash; /* hash used when removing duplicates */

  hash_ptr bdd_fsm_hash;
  hash_ptr sexp_fsm_hash;
  hash_ptr bool_fsm_hash;
} FsmBuilder;

/* This structure contains all data needed for memoizing BDD FSMs */
typedef struct BddFsmMemoize_TAG {
  SexpFsm_ptr sexp_fsm;
  Set_t vars;
  TransType trans_type;
  BddVarSet_ptr state_cube;
  BddVarSet_ptr input_cube;
  BddVarSet_ptr next_cube;
} BddFsmMemoize;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**Macro***********************************************************************

  Synopsis     [Avoid adding duplicate BDD in cluster list]

  Description  [Avoid adding duplicate BDD in cluster list]

  SeeAlso      [fsm_builder_clusterize_expr_aux]

******************************************************************************/
#define AVOID_DUPLICATE_BDDs 1

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void fsm_builder_init ARGS((FsmBuilder_ptr self, DdManager* dd));

static void fsm_builder_deinit ARGS((FsmBuilder_ptr self));

static ClusterList_ptr
fsm_builder_clusterize_expr ARGS((FsmBuilder_ptr self,
                                  BddEnc_ptr enc, Expr_ptr expr));

static void
fsm_builder_clusterize_expr_aux ARGS((const FsmBuilder_ptr self,
                                      BddEnc_ptr enc,
                                      ClusterList_ptr clusters,
                                      Expr_ptr expr_trans,
                                      boolean is_inside_and,
                                      hash_ptr h));

static JusticeList_ptr
fsm_builder_justice_sexp_to_bdd ARGS((FsmBuilder_ptr self,
                                      BddEnc_ptr enc,
                                      node_ptr justice_sexp_list));

static CompassionList_ptr
fsm_builder_compassion_sexp_to_bdd ARGS((FsmBuilder_ptr self,
                                         BddEnc_ptr enc,
                                         node_ptr compassion_sexp_list));

static Set_t
fsm_builder_order_vars_list ARGS((const FsmBuilder_ptr self,
                                  const BddEnc_ptr enc,
                                  const NodeList_ptr vars));

static Expr_ptr
fsm_builder_remove_dupl ARGS((FsmBuilder_ptr self, Expr_ptr expr));

static assoc_retval hash_bdd_key_free ARGS((char* key, char* data, char* arg));

static BddFsm_ptr
fsm_builder_lookup_bdd_fsm ARGS((const FsmBuilder_ptr self,
                                 const SexpFsm_ptr sexp_fsm,
                                 const Set_t vars,
                                 TransType trans_type,
                                 BddVarSet_ptr state_vars_cube,
                                 BddVarSet_ptr input_vars_cube,
                                 BddVarSet_ptr next_state_vars_cube));

static void
fsm_builder_insert_bdd_fsm ARGS((const FsmBuilder_ptr self,
                                 const SexpFsm_ptr sexp_fsm,
                                 const Set_t vars,
                                 TransType trans_type,
                                 BddVarSet_ptr state_cube,
                                 BddVarSet_ptr input_cube,
                                 BddVarSet_ptr next_cube,
                                 const BddFsm_ptr bdd_fsm));

static SexpFsm_ptr
fsm_builder_lookup_sexp_fsm ARGS((const FsmBuilder_ptr self,
                                  const FlatHierarchy_ptr fh,
                                  const Set_t vars));

static void fsm_builder_insert_sexp_fsm ARGS((const FsmBuilder_ptr self,
                                              const FlatHierarchy_ptr fh,
                                              const Set_t vars,
                                              const SexpFsm_ptr sexp_fsm));

static BoolSexpFsm_ptr
fsm_builder_lookup_bool_sexp_fsm ARGS((const FsmBuilder_ptr self,
                                       const FlatHierarchy_ptr fh,
                                       const Set_t vars));

static void
fsm_builder_insert_bool_sexp_fsm ARGS((const FsmBuilder_ptr self,
                                       const FlatHierarchy_ptr fh,
                                       const Set_t vars,
                                       const BoolSexpFsm_ptr sexp_fsm));

static Expr_ptr
fsm_builder_compute_scalar_fsm_id ARGS((const SexpFsm_ptr sexp));

static Expr_ptr
fsm_builder_compute_hierarchy_id ARGS((const FlatHierarchy_ptr fh));

static boolean
fsm_builder_set_contains_infinite_variables ARGS((const SymbTable_ptr st,
                                                  const Set_t vars));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The constructor creates a BddEnc and self handles it]

  Description        []

  SideEffects        []

******************************************************************************/
FsmBuilder_ptr FsmBuilder_create(DdManager* dd)
{
  FsmBuilder_ptr self = ALLOC(FsmBuilder, 1);

  FSM_BUILDER_CHECK_INSTANCE(self);

  fsm_builder_init(self, dd);
  return self;
}


/**Function********************************************************************

  Synopsis           [Class FsmBuilder destructor]

  Description        []

  SideEffects        []

******************************************************************************/
void FsmBuilder_destroy(FsmBuilder_ptr self)
{
  FSM_BUILDER_CHECK_INSTANCE(self);

  fsm_builder_deinit(self);

  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Creates a new scalar sexp fsm]

  Description        [The caller becomes the owner of the returned object]

  SideEffects        []

******************************************************************************/
SexpFsm_ptr
FsmBuilder_create_scalar_sexp_fsm(const FsmBuilder_ptr self,
                                  FlatHierarchy_ptr flat_hierarchy,
                                  const Set_t vars)
{
  FlatHierarchy_ptr fh;
  SexpFsm_ptr res;

  FSM_BUILDER_CHECK_INSTANCE(self);

  res = fsm_builder_lookup_sexp_fsm(self, flat_hierarchy, vars);
  if (SEXP_FSM(NULL) != res) {
    return res;
  }


  /* triggers calculations of constraints if needed */
  FlatHierarchy_calculate_vars_constrains(flat_hierarchy);

  /* prepares a new empty hierarchy. Allows for the original assigns
     that are needed to construct the single vars' fsm. Also,
     preserves all the other fields. */
  fh = FlatHierarchy_copy(flat_hierarchy);
  FlatHierarchy_set_init(fh, Nil);
  FlatHierarchy_set_invar(fh, Nil);
  FlatHierarchy_set_trans(fh, Nil);
  FlatHierarchy_set_input(fh, Nil);

  res = SexpFsm_create(fh, vars);

  FlatHierarchy_destroy(fh);

  fsm_builder_insert_sexp_fsm(self, flat_hierarchy, vars, res);

  return res;
}


/**Function********************************************************************

  Synopsis           [Creates a new boolean sexp fsm, taking into account of
                      the current variables ordering, or the trans
                      ordering file when specified by the
                      user. When used, the latter overrides the
                      former.]

  Description        [The caller becomes the owner of the returned object.
                      An exception may occur if the trans cluster
                      ordering is specified and an error occurs
                      while parsing it.]

  SideEffects        []

******************************************************************************/
BoolSexpFsm_ptr
FsmBuilder_create_boolean_sexp_fsm(const FsmBuilder_ptr self,
                                   FlatHierarchy_ptr flat_hierarchy,
                                   const Set_t vars,
                                   BddEnc_ptr bdd_enc, /* cannot be NULL */
                                   SymbLayer_ptr det_layer)
{
  FlatHierarchy_ptr fh;
  BoolSexpFsm_ptr res;
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(bdd_enc));

  FSM_BUILDER_CHECK_INSTANCE(self);

  if (fsm_builder_set_contains_infinite_variables(st, vars)) {
    rpterr("Impossible to build a boolean FSM"
           " with infinite precision variables");
  }

  res = fsm_builder_lookup_bool_sexp_fsm(self, flat_hierarchy, vars);
  if (BOOL_SEXP_FSM(NULL) != res) {
    return res;
  }


  /* triggers calculations of constraints if needed */
  FlatHierarchy_calculate_vars_constrains(flat_hierarchy); //RC it was mainFlatHierarchy);

  /* prepares a new empty hierarchy. Allows for the original assigns
     that are needed to construct the single vars' fsm. Also,
     preserves all the other fields. */
  fh = FlatHierarchy_copy(flat_hierarchy);
  FlatHierarchy_set_init(fh, Nil);
  FlatHierarchy_set_invar(fh, Nil);
  FlatHierarchy_set_trans(fh, Nil);
  FlatHierarchy_set_input(fh, Nil);

  res = BoolSexpFsm_create(fh, vars, bdd_enc, det_layer);

  FlatHierarchy_destroy(fh);

  fsm_builder_insert_bool_sexp_fsm(self, flat_hierarchy, vars, res);

  return res;
}

/**Function********************************************************************

  Synopsis           [Creates a BddFsm instance from a given SexpFsm]

  Description        [
  Note: all variables from provided encoding will go to the BDD FSM.
  Use FsmBuilder_create_bdd_fsm_of_vars if only SOME variables should be taken
  into account.]

  SideEffects        []

******************************************************************************/
BddFsm_ptr FsmBuilder_create_bdd_fsm(const FsmBuilder_ptr self,
                                     BddEnc_ptr enc,
                                     const SexpFsm_ptr sexp_fsm,
                                     const TransType trans_type)
{
  BddFsm_ptr bddfsm;
  BddVarSet_ptr state_vars_cube, input_vars_cube, next_state_vars_cube;

  state_vars_cube = BddEnc_get_state_vars_cube(enc);
  input_vars_cube = BddEnc_get_input_vars_cube(enc);
  next_state_vars_cube = BddEnc_get_next_state_vars_cube(enc);

  bddfsm = FsmBuilder_create_bdd_fsm_of_vars(self, sexp_fsm, trans_type, enc,
                                             state_vars_cube, input_vars_cube,
                                             next_state_vars_cube);

  bdd_free(self->dd, (bdd_ptr) state_vars_cube);
  bdd_free(self->dd, (bdd_ptr) input_vars_cube);
  bdd_free(self->dd, (bdd_ptr) next_state_vars_cube);

  return bddfsm;
}

/**Function********************************************************************

  Synopsis           [Creates a BddFsm instance from a given SexpFsm]

  Description        [It is the same as FsmBuilder_create_bdd_fsm except that
  the cubes of state, input and next-state variables are given explicitly.

  Note: The functions will take a copy of provided cubes.]

  SideEffects        []

******************************************************************************/
BddFsm_ptr FsmBuilder_create_bdd_fsm_of_vars(const FsmBuilder_ptr self,
                                             const SexpFsm_ptr sexp_fsm,
                                             const TransType trans_type,
                                             BddEnc_ptr enc,
                                             BddVarSet_ptr state_vars_cube,
                                             BddVarSet_ptr input_vars_cube,
                                             BddVarSet_ptr next_state_vars_cube)
{
  /* to construct Bdd Fsm: */
  BddFsm_ptr bddfsm = BDD_FSM(NULL);
  BddTrans_ptr trans;
  JusticeList_ptr justice;
  CompassionList_ptr compassion;
  bdd_ptr init_bdd, invar_bdd, input_bdd;
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));
  Set_t vars = SexpFsm_get_vars(sexp_fsm);

  FSM_BUILDER_CHECK_INSTANCE(self);

  if (fsm_builder_set_contains_infinite_variables(st, vars)) {
    rpterr("Impossible to build a BDD FSM"
           " with infinite precision variables");
  }

  bddfsm = fsm_builder_lookup_bdd_fsm(self, sexp_fsm, vars, trans_type,
                                      state_vars_cube, input_vars_cube,
                                      next_state_vars_cube);
  if (BDD_FSM(NULL) != bddfsm) {
    return bddfsm;
  }


  /* ---------------------------------------------------------------------- */
  /* Trans construction                                                     */
  /* ---------------------------------------------------------------------- */
  { /* here the trans is constructed out of the vars fsm, to make
       it ordered wrt the obtained ordered vars set */
    ClusterList_ptr clusters;
    ClusterOptions_ptr cluster_options;
    Set_t sorted_vars;
    Expr_ptr trans_expr;
    Set_Iterator_t iter;

    sorted_vars = fsm_builder_order_vars_list(self, enc,
                                              SexpFsm_get_vars_list(sexp_fsm));

    trans_expr = SexpFsm_get_trans(sexp_fsm);
    SET_FOREACH(sorted_vars, iter) {
      Expr_ptr vtrans = SexpFsm_get_var_trans(sexp_fsm,
                           (node_ptr) Set_GetMember(sorted_vars, iter));
      trans_expr = Expr_and_nil(trans_expr, vtrans);
    }

    Set_ReleaseSet(sorted_vars); /* no longer needed */

    /* clusters construction */
    clusters = fsm_builder_clusterize_expr(self, enc, trans_expr);
    cluster_options = ClusterOptions_create(OptsHandler_get_instance());

    trans = BddTrans_create(self->dd,
                            clusters,
                            (bdd_ptr) state_vars_cube,
                            (bdd_ptr) input_vars_cube,
                            (bdd_ptr) next_state_vars_cube,
                            trans_type,
                            cluster_options);

    ClusterList_destroy(clusters);
    ClusterOptions_destroy(cluster_options); /* this is no longer needed */
  }


  /* ---------------------------------------------------------------------- */
  /* Bdd Fsm construction                                                   */
  /* ---------------------------------------------------------------------- */
  justice = fsm_builder_justice_sexp_to_bdd(self, enc,
                                            SexpFsm_get_justice(sexp_fsm));

  compassion = fsm_builder_compassion_sexp_to_bdd(self, enc,
                                        SexpFsm_get_compassion(sexp_fsm));

  /* init */
  init_bdd = BddEnc_expr_to_bdd(enc, SexpFsm_get_init(sexp_fsm), Nil);
  /* invar */
  invar_bdd = BddEnc_expr_to_bdd(enc, SexpFsm_get_invar(sexp_fsm), Nil);
  /* input */
  input_bdd = BddEnc_expr_to_bdd(enc, SexpFsm_get_input(sexp_fsm), Nil);

  bddfsm = BddFsm_create(enc,
                         BDD_STATES(init_bdd),
                         BDD_INVAR_STATES(invar_bdd),
                         BDD_INVAR_INPUTS(input_bdd),
                         trans,
                         justice, compassion);

  bdd_free(self->dd, input_bdd);
  bdd_free(self->dd, invar_bdd);
  bdd_free(self->dd, init_bdd);

  fsm_builder_insert_bdd_fsm(self, sexp_fsm,
                             SexpFsm_get_vars(sexp_fsm),
                             trans_type,
                             state_vars_cube, input_vars_cube,
                             next_state_vars_cube, bddfsm);

  return bddfsm;
}


/**Function********************************************************************

  Synopsis [Given an expression, returns a bdd ClusterList with
  each conjuction occurring into expr contained in each cluster of
  the list. ]

  Description [Each cluster into the list represents a piece of
  transition relation. If the given expression contains
  duplicates, they will not occur into the returned cluster
  list. Returned list should be destroyed by the caller.]

  SideEffects        []

******************************************************************************/
ClusterList_ptr FsmBuilder_clusterize_expr(FsmBuilder_ptr self,
                                           BddEnc_ptr enc,
                                           Expr_ptr expr)
{
  Expr_ptr simp_expr;
  FSM_BUILDER_CHECK_INSTANCE(self);

  clear_assoc(self->simpl_hash);
  simp_expr = fsm_builder_remove_dupl(self, expr);
  clear_assoc(self->simpl_hash);

   return fsm_builder_clusterize_expr(self, enc, simp_expr);
}

/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
static void fsm_builder_init(FsmBuilder_ptr self, DdManager* dd)
{
  self->dd = dd;
  self->simpl_hash = new_assoc();
  self->bdd_fsm_hash = new_assoc();
  self->sexp_fsm_hash = new_assoc();
  self->bool_fsm_hash = new_assoc();
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
static void fsm_builder_deinit(FsmBuilder_ptr self)
{
  assoc_iter aiter;

  free_assoc(self->simpl_hash);

  /* Clear the BDD fsm cache */
  {
    BddFsmMemoize* mem;
    BddFsm_ptr bdd_fsm;
    ASSOC_FOREACH(self->bdd_fsm_hash, aiter, &mem, &bdd_fsm) {
      Set_ReleaseSet(mem->vars);
      SexpFsm_destroy(mem->sexp_fsm);
      bdd_free(self->dd, mem->state_cube);
      bdd_free(self->dd, mem->input_cube);
      bdd_free(self->dd, mem->next_cube);
      BddFsm_destroy(bdd_fsm);

      FREE(mem);
    }
  }

  /* Clear the Sexp Fsm cache */
  {
    node_ptr tmp;
    SexpFsm_ptr sexp;

    ASSOC_FOREACH(self->sexp_fsm_hash, aiter, &tmp, &sexp) {
      Set_ReleaseSet((Set_t)cdr(tmp));
      FlatHierarchy_destroy(FLAT_HIERARCHY(car(tmp)));
      SexpFsm_destroy(sexp);
      free_node(tmp);
    }
  }

  /* Clear the Boolean Sexp Fsm cache */
  {
    node_ptr tmp;
    BoolSexpFsm_ptr se;
    ASSOC_FOREACH(self->bool_fsm_hash, aiter, &tmp, &se) {
      Set_ReleaseSet((Set_t)cdr(tmp));
      FlatHierarchy_destroy(FLAT_HIERARCHY(car(tmp)));
      BoolSexpFsm_destroy(se);
      free_node(tmp);
    }
  }

  free_assoc(self->bool_fsm_hash);
  free_assoc(self->bdd_fsm_hash);
  free_assoc(self->sexp_fsm_hash);
}


/**Function********************************************************************

  Synopsis           [Converts an expression into a list of clusters.
  This list can be used to create a BddFsm]

  Description        [Each cluster into the list represents a piece of
  transition relation. One important requirement is that the given expr should
  not contain duplicates. See for example SexpFsm_get_{init, invar, trans} on
  how to obtain a well formed expression]

  SideEffects        []

******************************************************************************/
static ClusterList_ptr fsm_builder_clusterize_expr(FsmBuilder_ptr self,
                                                   BddEnc_ptr enc,
                                                   Expr_ptr expr)
{
  hash_ptr h;
  ClusterList_ptr clusters;

  h = new_assoc();

  clusters = ClusterList_create(self->dd);
  fsm_builder_clusterize_expr_aux(self, enc, clusters, expr, false, h);
  clear_assoc_and_free_entries_arg(h, hash_bdd_key_free, (char *)self->dd);

  free_assoc(h);

  return clusters;
}



/**Function********************************************************************

  Synopsis           [Auxiliary function to recursively traverse the
  given expression, clusterizing each node as bdd. If called from outside,
  parameter is_inside_and is false.]

  Description        []

  SideEffects        [given cluster list will change]

******************************************************************************/
EXTERN AddArray_ptr BddEnc_expr_to_addarray(BddEnc_ptr self, const Expr_ptr expr,
                                            const node_ptr context);

EXTERN bdd_ptr bdd_iff(DdManager * dd, bdd_ptr a, bdd_ptr b);

static void
fsm_builder_clusterize_expr_aux(const FsmBuilder_ptr self,
                                BddEnc_ptr enc,
                                ClusterList_ptr clusters,
                                Expr_ptr expr_trans,
                                boolean is_inside_and,
                                hash_ptr h)
{
  bdd_ptr tmp;
  node_ptr node = (node_ptr) expr_trans;

  if (node != Nil) {
    yylineno = node_get_lineno(node);

    switch (node_get_type(node)) {
    case AND:
      fsm_builder_clusterize_expr_aux(self, enc, clusters, car(node), true, h);
      fsm_builder_clusterize_expr_aux(self, enc, clusters, cdr(node), true, h);

      if (!is_inside_and && (ClusterList_length(clusters) == 0)) {
        /* Due to lazy evaluation, the list is going to be empty (and
           the call is over). Adds a single true cluster */
        bdd_ptr one = bdd_true(self->dd);
        Cluster_ptr cluster = Cluster_create(self->dd);
        Cluster_set_trans(cluster, self->dd, one);
        ClusterList_append_cluster(clusters, cluster);
        bdd_free(self->dd, one);

        if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
          fprintf(nusmv_stderr, "FsmBuilder: created cluster for expression: True\n");
        }
      }

      break;

      /* [MRMR]: and build a cluster for each bit instead of building a monolitic
         [MRMR]: BDD for the assignment. */
#ifdef DO_CLUSTERS_FOR_BITS_IN_EQDEF_AMONG_WORDS
    case EQDEF:
    case EQUAL:
      {
        TypeChecker_ptr tc = SymbTable_get_type_checker(BaseEnc_get_symb_table(BASE_ENC(enc)));

        if (EQDEF == node_get_type(expr_trans)) {
          /* We expect only next() := ... */
          nusmv_assert(NEXT == node_get_type(car(expr_trans)));
        }

        SymbType_ptr type;
        type = TypeChecker_get_expression_type(tc, car(expr_trans), Nil);
        nusmv_assert(!SymbType_is_error(type)); /* cannot be an type error */

        /* traverses the rhs */
        if ((SYMB_TYPE_UNSIGNED_WORD == SymbType_get_tag(type)) ||
            (SYMB_TYPE_SIGNED_WORD == SymbType_get_tag(type))) {
          AddArray_ptr lhs, rhs;
          int i;

          lhs = BddEnc_expr_to_addarray(enc, car(expr_trans), Nil);
          rhs = BddEnc_expr_to_addarray(enc, cdr(expr_trans), Nil);

          nusmv_assert(AddArray_get_size(lhs) == AddArray_get_size(rhs));

          for (i = AddArray_get_size(lhs) - 1; i >= 0; --i) {




            bdd_ptr lhsbddi = add_to_bdd(self->dd, AddArray_get_n(lhs,i));
            bdd_ptr rhsbddi = add_to_bdd(self->dd, AddArray_get_n(rhs,i));
            bdd_ptr ci = bdd_iff(self->dd, lhsbddi, rhsbddi);

            bdd_free(self->dd, lhsbddi);
            bdd_free(self->dd, rhsbddi);
            if (! (bdd_is_true(self->dd, ci))) {
              Cluster_ptr cluster = Cluster_create(self->dd);
              Cluster_set_trans(cluster, self->dd, ci);
              ClusterList_append_cluster(clusters, cluster);

              if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
                fprintf(nusmv_stderr, "FsmBuilder: created cluster for word expression [%d:%d] : ", i, i);
                if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
                  fprintf(nusmv_stderr, "FsmBuilder: expression : ");
                  print_node(nusmv_stderr, expr_trans);
                  fprintf(nusmv_stderr, "[%d:%d]", i, i);
                }
                fprintf(nusmv_stderr, "\n");
              }
            }
            bdd_free(self->dd, ci);
          } /* End for */
          AddArray_destroy(self->dd, lhs);
          AddArray_destroy(self->dd, rhs);
          break;
        } /* if ((SYMB_TYPE_UNSIGNED_WORD == SymbType_get_tag(type)) ||
             (SYMB_TYPE_SIGNED_WORD == SymbType_get_tag(type))) */
      } /* case EQDEF */
#endif
    default:
      tmp = BddEnc_expr_to_bdd(enc, expr_trans, Nil);

#if AVOID_DUPLICATE_BDDs
      if (Nil == find_assoc(h, (node_ptr)tmp)) {
#endif
        if (! (bdd_is_true(self->dd, tmp) && is_inside_and)) {
          Cluster_ptr cluster = Cluster_create(self->dd);
          Cluster_set_trans(cluster, self->dd, tmp);
          ClusterList_append_cluster(clusters, cluster);

          if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
            fprintf(nusmv_stderr, "FsmBuilder: created cluster for expression");
            if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
              fprintf(nusmv_stderr, ": ");
              print_node(nusmv_stderr, expr_trans);
            }
            fprintf(nusmv_stderr, "\n");
          }
        }
#if AVOID_DUPLICATE_BDDs
        insert_assoc(h, (node_ptr)tmp, PTR_FROM_INT(node_ptr, 1));
      }
      else {
#endif
        bdd_free(self->dd, tmp);
#if AVOID_DUPLICATE_BDDs
      }
#endif
    } /* switch */
  }
}



/**Function********************************************************************

  Synopsis           [Converts a list of expressions into a list of
  bdds, wrapped inside a justice list object]

  Description        [The caller becomes the wner of the returned object.
  Internally used by the bdd fsm building code]

  SideEffects        []

******************************************************************************/
static JusticeList_ptr
fsm_builder_justice_sexp_to_bdd(FsmBuilder_ptr self,
                                BddEnc_ptr enc,
                                node_ptr justice_sexp_list)
{
  JusticeList_ptr res;
  node_ptr iter;

  res = JusticeList_create(self->dd);

  iter = justice_sexp_list;
  while (iter != Nil) {
    Expr_ptr expr = EXPR( car(iter) );
    bdd_ptr  p = BddEnc_expr_to_bdd(enc, expr, Nil);
    JusticeList_append_p(res, BDD_STATES(p));

    bdd_free(self->dd, p);
    iter = cdr(iter);
  } /* loop */

  return res;
}


/**Function********************************************************************

  Synopsis [Converts a list of couple of expressions into a list of couple of
  bdds, wrapped inside a compassion list object]

  Description        [The caller becomes the wner of the returned object.
  Internally used by the bdd fsm building code]

  SideEffects        []

******************************************************************************/
static CompassionList_ptr
fsm_builder_compassion_sexp_to_bdd(FsmBuilder_ptr self,
                                   BddEnc_ptr enc,
                                   node_ptr compassion_sexp_list)
{
  CompassionList_ptr res;
  node_ptr iter;

  res = CompassionList_create(self->dd);

  iter = compassion_sexp_list;
  while (iter != Nil) {
    Expr_ptr expr;
    bdd_ptr  p, q;
    node_ptr couple = car(iter);

    expr = car(couple);
    p = BddEnc_expr_to_bdd(enc, expr, Nil);
    expr = cdr(couple);
    q = BddEnc_expr_to_bdd(enc, expr, Nil);

    CompassionList_append_p_q(res, BDD_STATES(p), BDD_STATES(q));

    bdd_free(self->dd, q);
    bdd_free(self->dd, p);

    iter = cdr(iter);
  } /* loop */

  return res;
}


void print_ids_list(FILE* fout, const NodeList_ptr list)
{
  ListIter_ptr iter = NodeList_get_first_iter(list);
  int len = 0;
  while (!ListIter_is_end(iter)) {
    char* name = sprint_node(NodeList_get_elem_at(list, iter));
    int l = strlen(name) + 1;
    if (len + l > 70) { fprintf(fout, "\n"); len = 0; }
    fprintf(fout, "%s ", name);
    FREE(name);
    len += l;
    iter = ListIter_get_next(iter);
  }
  fprintf(fout, "\n");
}


/**Function********************************************************************

  Synopsis           [Private service used by SexpFsm builders]

  Description        [This method orders the given vars list wrt to the
                      trans ordering file as a first attempt, then
                      wrt the current bdd variables
                      ordering. Caller has to free the returned
                      set.]

  SideEffects        []

******************************************************************************/
static Set_t fsm_builder_order_vars_list(const FsmBuilder_ptr self,
                                         const BddEnc_ptr enc,
                                         const NodeList_ptr vars)
{
  Set_t res = Set_MakeEmpty();

  if (opt_trans_order_file(OptsHandler_get_instance())) {
    /* The user has specified a trans ordering file */
    FILE* fl;
    ParserIdList_ptr parser;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr,
              "Reading the trans variable ordering from file '%s'\n",
              get_trans_order_file(OptsHandler_get_instance()));
    }

    fl = fopen(get_trans_order_file(OptsHandler_get_instance()), "r");
    if (fl == (FILE*) NULL) {
      fprintf(nusmv_stderr,
              "While opening the specified trans ordering file:\n");
      error_file_not_found(get_trans_order_file(OptsHandler_get_instance()));
    }

    parser = ParserIdList_create();

    CATCH {
      ParserIdList_parse_from_file(parser, fl);
    }
    FAIL {
      fclose(fl);
      ParserIdList_destroy(parser);
      nusmv_exit(1); /* re-throws the exception */
    }

    fclose(fl);

    res = Compile_make_sorted_vars_list_from_order(
                           BaseEnc_get_symb_table(BASE_ENC(enc)),
                           vars, ParserIdList_get_id_list(parser));

    ParserIdList_destroy(parser);
  }
  else {
    /* reads the vars ordering from the internal BDD vars ordering */
    NodeList_ptr order;

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr,
              "Reading the trans variable ordering from the BDD encoder\n");
    }

    order = BddEnc_get_var_ordering(enc, DUMP_SCALARS_ONLY);
    res = Compile_make_sorted_vars_list_from_order(
                   BaseEnc_get_symb_table(BASE_ENC(enc)), vars, order);

    NodeList_destroy(order);
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Removes duplicates from expression containing AND nodes]

  Description        []

  SideEffects        []

******************************************************************************/
static Expr_ptr fsm_builder_remove_dupl(FsmBuilder_ptr self, Expr_ptr expr)
{
  node_ptr result;

  if ((expr == EXPR(NULL)) ||
      find_assoc(self->simpl_hash, (node_ptr) expr) != Nil) {
    return Expr_true();
  }

  switch (node_get_type(NODE_PTR(expr))) {
  case AND:
    {
      Expr_ptr left = fsm_builder_remove_dupl(self, car(NODE_PTR(expr)));
      Expr_ptr right = fsm_builder_remove_dupl(self, cdr(NODE_PTR(expr)));
      result = Expr_and(left, right);
      break;
    }

  default:
    result = expr;
  } /* switch */

  insert_assoc(self->simpl_hash, (node_ptr) expr, (node_ptr) true);
  return result;
}

/**Function********************************************************************

   Synopsis           [Private service]

   Description        [Used when destroying hash containing bdd_ptr as key]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static assoc_retval hash_bdd_key_free(char* key, char* data, char* arg)
{
  bdd_ptr bdd = (bdd_ptr) key;
  DdManager* dd = (DdManager*) arg;

  if (bdd != (bdd_ptr) NULL) { bdd_free(dd, bdd); }
  return ASSOC_DELETE;
}

/**Function********************************************************************

   Synopsis           [Sexp Fsm cache lookup routine]

   Description        [If any, returns a cached copy of a Sexp Fsm which has
                       been built using the given hierarchy and the given set
                       of variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static SexpFsm_ptr fsm_builder_lookup_sexp_fsm(const FsmBuilder_ptr self,
                                               const FlatHierarchy_ptr fh,
                                               const Set_t vars)
{
  SexpFsm_ptr result = SEXP_FSM(NULL);
  node_ptr fh_id = fsm_builder_compute_hierarchy_id(fh);

  assoc_iter aiter;
  node_ptr couple;
  boolean _break = false;

  ASSOC_FOREACH(self->sexp_fsm_hash, aiter, &couple, NULL) {
    FlatHierarchy_ptr scalar = FLAT_HIERARCHY(car(couple));
    node_ptr scalar_id = fsm_builder_compute_hierarchy_id(scalar);
    Set_t cone = (Set_t)cdr(couple);

    if ((fh_id == scalar_id) && Set_Equals(cone, vars)) {
      result = SEXP_FSM(find_assoc(self->sexp_fsm_hash, couple));
      _break = true;
      break;
    }
  }

  if (_break) { assoc_iter_free(aiter); }

  if (SEXP_FSM(NULL) != result) {
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr,
              "FsmBuilder: Create scalar fsm -> "
              "Returning previously cached FSM\n");
    }
    return SexpFsm_copy(result);
  }

  return SEXP_FSM(NULL);
}

/**Function********************************************************************

   Synopsis           [Adds to the sexp fsm cache the given scalar fsm]

   Description        [Adds to the sexp fsm cache the given scalar fsm,
                       which has been built using the given flat hierarchy
                       and the given set of variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void fsm_builder_insert_sexp_fsm(const FsmBuilder_ptr self,
                                        const FlatHierarchy_ptr fh,
                                        const Set_t vars,
                                        const SexpFsm_ptr sexp_fsm)
{
  node_ptr couple = cons(NODE_PTR(FlatHierarchy_copy(fh)),
                         NODE_PTR(Set_Copy(vars)));

  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "FsmBuilder: Create scalar fsm -> Caching new FSM\n");
  }

  insert_assoc(self->sexp_fsm_hash, couple,
               NODE_PTR(SexpFsm_copy(sexp_fsm)));
}


/**Function********************************************************************

   Synopsis           [Bool Sexp Fsm cache lookup routine]

   Description        [If any, returns a cached copy of a Boolean Fsm which has
                       been built using the given hierarchy and the given set
                       of variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static BoolSexpFsm_ptr
fsm_builder_lookup_bool_sexp_fsm(const FsmBuilder_ptr self,
                                 const FlatHierarchy_ptr fh,
                                 const Set_t vars)
{
  BoolSexpFsm_ptr result = BOOL_SEXP_FSM(NULL);
  assoc_iter aiter;
  node_ptr couple;
  node_ptr fh_id = fsm_builder_compute_hierarchy_id(fh);
  boolean _break = false;

  ASSOC_FOREACH(self->bool_fsm_hash, aiter, &couple, NULL) {
    FlatHierarchy_ptr scalar = FLAT_HIERARCHY(car(couple));
    node_ptr scalar_id = fsm_builder_compute_hierarchy_id(scalar);
    Set_t cone = (Set_t)cdr(couple);

    if ((fh_id == scalar_id) && Set_Equals(cone, vars)) {
      result = BOOL_SEXP_FSM(find_assoc(self->bool_fsm_hash, couple));
      _break = true;
      break;
    }
  }

  if (_break) { assoc_iter_free(aiter);  }

  if (BOOL_SEXP_FSM(NULL) != result) {
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr,
              "FsmBuilder: Create boolean fsm -> "
              "Returning previously cached FSM\n");
    }
    return BoolSexpFsm_copy(result);
  }

  return BOOL_SEXP_FSM(NULL);
}

/**Function********************************************************************

   Synopsis           [Adds to the sexp fsm cache the given scalar fsm]

   Description        [Adds to the sexp fsm cache the given scalar fsm,
                       which has been built using the given flat hierarchy
                       and the given set of variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void fsm_builder_insert_bool_sexp_fsm(const FsmBuilder_ptr self,
                                             const FlatHierarchy_ptr fh,
                                             const Set_t vars,
                                             const BoolSexpFsm_ptr sexp_fsm)
{
  node_ptr couple = cons(NODE_PTR(FlatHierarchy_copy(fh)),
                         NODE_PTR(Set_Copy(vars)));

  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "FsmBuilder: Create boolean fsm -> Caching new FSM\n");
  }

  insert_assoc(self->bool_fsm_hash, couple,
               NODE_PTR(BoolSexpFsm_copy(sexp_fsm)));
}

/**Function********************************************************************

   Synopsis           [Bdd Fsm cache lookup routine]

   Description        [If any, returns a cached copy of a Bdd Fsm which has
                       been built using the given Sexp Fsm and the given set
                       of variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static BddFsm_ptr fsm_builder_lookup_bdd_fsm(const FsmBuilder_ptr self,
                                             const SexpFsm_ptr sexp_fsm,
                                             const Set_t vars,
                                             TransType trans_type,
                                             BddVarSet_ptr state_vars_cube,
                                             BddVarSet_ptr input_vars_cube,
                                             BddVarSet_ptr next_state_vars_cube)
{
  BddFsm_ptr result = BDD_FSM(NULL);
  BddFsm_ptr tmp_fsm = BDD_FSM(NULL);
  node_ptr sexp_fsm_id = fsm_builder_compute_scalar_fsm_id(sexp_fsm);
  boolean _break = false;
  BddFsmMemoize* mem;
  assoc_iter aiter;

  ASSOC_FOREACH(self->bdd_fsm_hash, aiter, &mem, NULL) {
    node_ptr scalar_fsm_id =
      fsm_builder_compute_scalar_fsm_id(mem->sexp_fsm);

    if ((sexp_fsm_id == scalar_fsm_id) &&
        Set_Equals(mem->vars, vars) &&
        (trans_type == mem->trans_type) &&
        (state_vars_cube == mem->state_cube) &&
        (input_vars_cube == mem->input_cube) &&
        (next_state_vars_cube == mem->next_cube)) {
      tmp_fsm = BDD_FSM(find_assoc(self->bdd_fsm_hash,
                                   NODE_PTR(mem)));
      _break = true;
      break;
    }
  }

  if (_break) { assoc_iter_free(aiter); }

  if (BDD_FSM(NULL) != tmp_fsm) {
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr,
              "FsmBuilder: Create bdd fsm -> "
              "Returning previously cached FSM\n");
    }
    result = BddFsm_copy(tmp_fsm);
  }

  return result;
}

/**Function********************************************************************

   Synopsis           [Adds to the bdd fsm cache the given bdd fsm]

   Description        [Adds to the bdd fsm cache the given bdd fsm,
                       which has been built using the given scalar fsm
                       and the given set of variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void fsm_builder_insert_bdd_fsm(const FsmBuilder_ptr self,
                                       const SexpFsm_ptr sexp_fsm,
                                       const Set_t vars,
                                       TransType trans_type,
                                       BddVarSet_ptr state_cube,
                                       BddVarSet_ptr input_cube,
                                       BddVarSet_ptr next_cube,
                                       const BddFsm_ptr bdd_fsm)
{
  BddFsmMemoize* mem = ALLOC(BddFsmMemoize, 1);

  if ((BddFsmMemoize*)NULL == mem) {
    fprintf(nusmv_stderr, "Cannot allocate memory for BDD FSM memoization\n");
    error_unreachable_code();
  }

  mem->sexp_fsm = SexpFsm_copy(sexp_fsm);
  mem->vars = Set_Copy(vars);
  mem->state_cube = bdd_dup(state_cube);
  mem->input_cube = bdd_dup(input_cube);
  mem->next_cube = bdd_dup(next_cube);
  mem->trans_type = trans_type;

  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "FsmBuilder: Create bdd fsm -> Caching new FSM\n");
  }

  insert_assoc(self->bdd_fsm_hash, NODE_PTR(mem),
               NODE_PTR(BddFsm_copy(bdd_fsm)));
}

/**Function********************************************************************

   Synopsis           [Computes the id for the given Flat Hierarchy]

   Description        [Computes the id for the given Flat Hierarchy.
                       The ID is the normalized result of the union of
                       all FSM expressions]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static Expr_ptr
fsm_builder_compute_hierarchy_id(const FlatHierarchy_ptr fh)
{
  Expr_ptr expr;

  expr = FlatHierarchy_get_assign(fh);
  expr = find_node(AND, FlatHierarchy_get_init(fh), expr);
  expr = find_node(AND, FlatHierarchy_get_invar(fh), expr);
  expr = find_node(AND, FlatHierarchy_get_trans(fh), expr);
  expr = find_node(AND, FlatHierarchy_get_input(fh), expr);
  expr = find_node(AND, FlatHierarchy_get_justice(fh), expr);
  expr = find_node(AND, FlatHierarchy_get_compassion(fh), expr);

  return node_normalize(expr);
}

/**Function********************************************************************

   Synopsis           [Computes the id for the given Sexp Fsm]

   Description        [Computes the id for the given Sexp Fsm.
                       The ID is the normalized result of the union of
                       all FSM expressions]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static Expr_ptr
fsm_builder_compute_scalar_fsm_id(const SexpFsm_ptr sexp)
{
  Expr_ptr expr;

  expr = SexpFsm_get_init(sexp);
  expr = find_node(AND, SexpFsm_get_trans(sexp), expr);
  expr = find_node(AND, SexpFsm_get_invar(sexp), expr);
  expr = find_node(AND, SexpFsm_get_input(sexp), expr);
  expr = find_node(AND, SexpFsm_get_justice(sexp), expr);
  expr = find_node(AND, SexpFsm_get_compassion(sexp), expr);

  return node_normalize(expr);
}


/**Function********************************************************************

   Synopsis           [Checks if the given set of variables contains at least
                       one infinite precision variable]

   Description        [Checks if the given set of variables contains at least
                       one infinite precision variable]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean
fsm_builder_set_contains_infinite_variables(const SymbTable_ptr st,
                                                      const Set_t vars)
{
  Set_Iterator_t iter;

  SET_FOREACH(vars, iter) {
    node_ptr var = Set_GetMember(vars, iter);
    SymbType_ptr type;

    nusmv_assert(SymbTable_is_symbol_var(st, var));

    type = SymbTable_get_var_type(st, var);

    if (SymbType_is_infinite_precision(type)) {
      return true;
    }
  }

  return false;
}
