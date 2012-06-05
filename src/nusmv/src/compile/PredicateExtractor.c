/**CFile***********************************************************************

   FileName    [PredicateExtractor.c]

   PackageName [compile]

   Synopsis    [A Predicate-Extractor class]

   Description [See PredicateExtractor.h for more info]

   Author      [Andrei Tchaltsev]

   Copyright   [
   This file is part of the ``compile'' package of NuSMV version 2.
   Copyright (C) 2010 by FBK-irst.

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


#include "PredicateExtractor.h"
#include "compile/compileInt.h"
#include "parser/symbols.h"
#include "utils/WordNumber.h"
#include "utils/utils.h"
#include "utils/assoc.h"
#include "utils/error.h"
#include "compile/symb_table/ResolveSymbol.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Type************************************************************************

   Synopsis    [Preicate Extractor class]

   Description []

   SeeAlso     []

******************************************************************************/
typedef struct PredicateExtractor_TAG
{
  Set_t all_preds;   /* all predicates : Set_t of node_ptr */
  Set_t unclustered_preds;   /* subset of all_preds for which clusters
                                were not computed : Set_t of node_ptr */
  Set_t all_clusters;   /* all clusters : Set_t of Set_t of node_ptr.
                           This is the actual owner of all clusters.*/

  hash_ptr var2cluster; /* var -> cluster it belongs to. node_ptr -> Set_t */
  hash_ptr cluster2preds;   /* cluster -> its predicates. Owner of preds sets.
                               Set_t -> Set_t of node_ptr */

  hash_ptr expr2preds; /* node_ptr -> Set_t of node_ptr.  For
                          not-boolean expr the associated value is set
                          of subparts of predicates in it.  For
                          processed boolean expressions the associated
                          value is one of PREDICATES_TRUE (if the
                          expression can be simplified to constant
                          true), PREDICATES_FALSE (if expression can
                          be simplified to FALSE) or
                          PREDICATES_ARBITRARY (for all other cases).
                          This hash is the owner of preds sets.
                       */
  Set_t special_int_preds[3]; /* array of 3 special predicates subparts:
                                 {0}, {1}, and {0,1} */
  Set_t special_word_preds[3]; /* array of 3 special predicates subparts:
                                  {0d1_0}, {0d1_1}, and {0d1_0,0d1_1} */

  TypeChecker_ptr checker; /* type-checker is used to get type info
                              of processed expressions and type check
                              generated expressions */
  SymbTable_ptr st;  /* the symbol table */
  boolean use_approx; /* if over-approximation has to be used when
                         extracting predicates (see issue 1934) */
  
} PredicateExtractor;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

   Synopsis   [These are special values to mark expressions
   which have been analyzed and which do not have predicate subparts]

   Description [Any boolean expression cannot have predicate subparts because
   boolean may have only complete predicates, not its subparts.
   Apparently, only not-boolean operation expressions may have
   predicates subparts.  These values are used in self->expr2preds
   hash.
   For better optimizations simplifications 3 values are introduced:
   PREDICATES_TRUE -- represent a set of predicates, consisting of {TRUE} only.
   PREDICATES_FALSE -- represent a set of predicates, consisting of {FALSE} only.
   PREDICATES_ARBITRARY -- represent arbitrary set of predicates.

   PREDICATES_OVERAPPROX -- represent approximanted value (to give 
   up the extraction)
   ]


   SeeAlso     []

******************************************************************************/
#define PREDICATES_TRUE ((Set_t)1)
#define PREDICATES_FALSE ((Set_t)2)
#define PREDICATES_ARBITRARY ((Set_t)3)
#define PREDICATES_OVERAPPROX ((Set_t)4)


/**Macro***********************************************************************

   Synopsis    [The threshold used when deciding whether over approximate 
   or not the predicate extraction]

   Description []

   SeeAlso     []

******************************************************************************/
#define OVER_APPROX_THRESHOLD 600000


/* below macro is TRUE iff the set is not an actually a Set_t,
   i.e. it is a valide predicate constant or the constant
   indicating the overapproximation.  
   Note: that expression 'set' should not include function call. */
#define IS_FLAG_PREDICATES(set)                                 \
 (IS_FLAG_VALID_PREDICATES(set) || IS_OVER_APPROX(set))

/* below macro is TRUE iff the set is not an actually a Set_t,
   i.e. it is one of the constant values PREDICATES_TRUE,
   PREDICATES_FALSE or PREDICATES_ARBITRARY.
   Note: that expression 'set' should not include function call. */
#define IS_FLAG_VALID_PREDICATES(set)                           \
 ((set)==PREDICATES_TRUE||(set)==PREDICATES_FALSE||(set)==PREDICATES_ARBITRARY)



/* below macro is TRUE iff the set is not an actually a Set_t, but it represents 
   an over-approximation, i.e. it is PREDICATES_OVERAPPROX */
#define IS_OVER_APPROX(set)                     \
  ((set)==PREDICATES_OVERAPPROX)

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


static void pred_extract_init ARGS((PredicateExtractor_ptr self,
                                    SymbTable_ptr st, boolean use_approx));

static void pred_extract_deinit ARGS((PredicateExtractor_ptr self));

static Set_t pred_extract_process_recur ARGS((PredicateExtractor_ptr self,
                                              node_ptr expr,
                                              node_ptr context));

static boolean pred_extract_is_bool_preds ARGS((Set_t result));

static Set_t pred_extract_fix_any_preds ARGS((PredicateExtractor_ptr self,
                                              Set_t result));
static Set_t pred_extract_bool2int ARGS((PredicateExtractor_ptr self,
                                         node_ptr expr, Set_t preds));

static Set_t pred_extract_apply_unary ARGS((PredicateExtractor_ptr self,
                                            int type,
                                            Set_t childResult));

static Set_t pred_extract_apply_binary ARGS((PredicateExtractor_ptr self,
                                             int type,
                                             Set_t leftResult,
                                             Set_t rightResult));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis    [The constructor creates a predicate-extractor]

   Description [See PredicateExtractor.h for more info on
   predicates and clusters.  The parameter 'checker' is a type checker
   used during predicate extraction and subsequent type checking of
   generated expressions.

   Parameter use_approx can be used to make the extractor
   give up when dealing with too-large expressions. This is
   currently used by the heuristics which extract the variable
   ordering out of the fsm.   

   NOTE that the type checker remembers the type of checked
   expressions (free or reuse nodes with care).]

   SideEffects        []

******************************************************************************/
PredicateExtractor_ptr PredicateExtractor_create(SymbTable_ptr st, 
                                                 boolean use_approx)
{
  PredicateExtractor_ptr self = ALLOC(PredicateExtractor, 1);

  PREDICATE_EXTRACTOR_CHECK_INSTANCE(self);

  pred_extract_init(self, st, use_approx);
  return self;
}


/**Function********************************************************************

   Synopsis    [Class PredicateExtractor destructor]

   Description []

   SideEffects        []

******************************************************************************/
void PredicateExtractor_destroy(PredicateExtractor_ptr self)
{
  PREDICATE_EXTRACTOR_CHECK_INSTANCE(self);

  pred_extract_deinit(self);

  FREE(self);
}


/**Function********************************************************************

   Synopsis    [The function computes and collects
   the predicates of a given expression]

   Description [
   See PredicateExtractor.h for more info on predicates and clusters.

   Note: that normalization of the input expression is not done.  Only
   predicates are computed (the lesser things are done the lesser
   time/memory is spent). See class PredicateNormaliser if
   predicate-normalized expressions are required.

   To additionally get/compute clusters
   PredicateExtractor_get_all_clusters can be used.

   Input expressions may/may not be expanded/normalized/flattened,
   whereas the collected predicates are flattened, expanded and
   created with find_node, in particular all identifiers fully
   resolved.

   WARNING: memoization is done. Providing the same expression a second
   times does not produce any additional predicate.

   Collected clusters/predicates are stored internally and can be
   obtained with PredicateExtractor_get_all_preds and
   PredicateExtractor_get_all_clusters.  ]

   SideEffects  []

******************************************************************************/
void
PredicateExtractor_compute_preds(PredicateExtractor_ptr self,
                                 node_ptr expr)
{
  int lineno_tmp;

  PREDICATE_EXTRACTOR_CHECK_INSTANCE(self);

  if (Nil == expr) return;

  /* new node will be created with for sure error line number */
  lineno_tmp = yylineno;
  yylineno = -1;

  /* sometimes top-level expressions are connected together with CONS
     or AND node created with new_node. Such pseudo-expressions are then
     freed and can be reused later in other places.
     As this function has memoization it may become a problem.

     The solution is to process such high level CONS and AND separately
     without type checking/memoization.

     Note: that if normal expressions with AND or CONS happened to be
     at the top they will be processed correctly, but just not
     memoized (some efficiency may be lost).

     NOTE: the right solution would be to use a special connector
     for high level expressions, not generic AND or CONS.
  */
  if (AND == node_get_type(expr) || CONS == node_get_type(expr)) {
    PredicateExtractor_compute_preds(self, car(expr));
    PredicateExtractor_compute_preds(self, cdr(expr));
  }
  else {
    /* this is a usual expression */
    pred_extract_process_recur(self, expr, Nil);
  }

  yylineno = lineno_tmp; /* restore line number */
  return;
}


/**Function********************************************************************

   Synopsis    [This function applies PredicateExtractor_compute_preds
   to every element of an hierarchy]

   Description [Note that symbol table in self has to correspond to
   the hierarchy, i.e. contains all the required symbols]

   SideEffects []

******************************************************************************/
void
PredicateExtractor_compute_preds_from_hierarchy(PredicateExtractor_ptr self,
                                                FlatHierarchy_ptr fh)
{
  int i;
  node_ptr expr;
  array_t * layers_name;
  const char* a_layer_name;

  node_ptr (*fh_access[])(FlatHierarchy_ptr)  = {
    FlatHierarchy_get_init, FlatHierarchy_get_invar,
    FlatHierarchy_get_trans, FlatHierarchy_get_input,
    FlatHierarchy_get_justice, FlatHierarchy_get_compassion,
    NULL};

  for(i = 0; fh_access[i] != NULL; i++) {
    expr = (*fh_access[i])(fh);
    PredicateExtractor_compute_preds(self, expr);
  }


  /* 1. FlatHierarchy_get_init/invar/trans do NOT return assignments.
     2. FlatHierarchy_lookup_assign return assignments with "running".
     3. FlatHierarchy_get_assign returns assignments without "running".
     4. We do NOT care about "running" and created corresponding CASE-expression
        because boolean vars are ignored by predicate extractor and
        assignment of a var to itself does not create new predicate 
     Thus it is better to use FlatHierarchy_get_assign instead of
     FlatHierarchy_lookup_assign (there will be no need to iterate over all vars).
  */

  /* Assignments require very special handling because
     FlatHierarchy_get_assign returns the assignments without
     CASE-expressions and "running" variables created when there are
     processes.  To obtain the actual assignments it is necessary to
     collects assignments using FlatHierarchy_lookup_assign.

     NOTE: This code is terrible because API in FlatHierarchy does
     not provided the required function (to access actual assignments).
  */
  layers_name = SymbTable_get_class_layer_names(self->st, (const char*) NULL);

  arrayForEachItem(const char*, layers_name, i, a_layer_name) {
    SymbLayer_ptr layer = SymbTable_get_layer(self->st, a_layer_name);
    SymbLayerIter iter;

    SYMB_LAYER_FOREACH(layer, iter, STT_VAR) {
      node_ptr name = SymbLayer_iter_get_symbol(layer, &iter);
      node_ptr init_name = find_node(SMALLINIT, name, Nil);
      node_ptr next_name = find_node(NEXT, name, Nil);
      node_ptr invar_expr = FlatHierarchy_lookup_assign(fh, name);
      node_ptr init_expr = FlatHierarchy_lookup_assign(fh, init_name);
      node_ptr next_expr = FlatHierarchy_lookup_assign(fh, next_name);

      if (invar_expr != Nil) {
        expr = find_node(EQDEF, name, invar_expr);
        PredicateExtractor_compute_preds(self, expr);
      }
      if (init_expr != Nil) {
        expr = find_node(EQDEF, init_name, init_expr);
        PredicateExtractor_compute_preds(self, expr);
      }
      if (next_expr != Nil) {
        expr = find_node(EQDEF, next_name, next_expr);
        PredicateExtractor_compute_preds(self, expr);
      }
    }
  }

  return;
}


/**Function********************************************************************

   Synopsis    [Returns the set of predicates computed so far]

   Description [Predicates are fully expanded and resolved expressions
   created with find_node, i.e. no freeing or modifications are allowed.
   Returned Set_t belongs to self.]

   SideEffects        []

******************************************************************************/
Set_t
PredicateExtractor_get_all_preds(const PredicateExtractor_ptr self)
{
  PREDICATE_EXTRACTOR_CHECK_INSTANCE(self);

  return self->all_preds;
}


/**Function********************************************************************

   Synopsis    [Returns the set of clusters for all so far collected
   predicates]

   Description [This function computes and returns clusters for all so far
   computed predicates.

   Returned result is Set_t of Set_t of fully resolved variables.
   Everything returned belongs to self.

   Note that this function perform computation and may take some time
   (though intermediate results are remembered between calls).

   It is possible to get a group of predicates responsible for a given
   cluster with PredicateExtractor_get_preds_of_a_cluster.

   NOTE: subsequent call of PredicateExtractor_compute_preds makes any
   data returned by this function invalid.]

   SideEffects        []

******************************************************************************/
Set_t
PredicateExtractor_get_all_clusters(const PredicateExtractor_ptr self)
{
  PREDICATE_EXTRACTOR_CHECK_INSTANCE(self);

  /* there are un-clustered predicates => process them at first */
  if (!Set_IsEmpty(self->unclustered_preds)) {

    Set_Iterator_t pred_iter;

    /* iterate over all the predicates */
    SET_FOREACH(self->unclustered_preds, pred_iter) {

      node_ptr predicate = Set_GetMember(self->unclustered_preds, pred_iter);
      Set_t deps = Formula_GetDependencies(self->st, predicate, Nil);

      /* NOTE: if simplification was not done then a predicate may consist
         of constants only. In this case dependency is empty and the predicate
         can be ignored.
      */
      if (!Set_IsEmpty(deps)) {

        /* first var is dealt differently : if a var is not yet in the
           table => create a new cluster for it, otherwise use the
           existing one */
        Set_Iterator_t it = Set_GetFirstIter(deps);
        node_ptr var = Set_GetMember(deps, it);

        Set_t cluster = (Set_t)find_assoc(self->var2cluster, var);

        /* if there is no cluster => create a new one */
        if (NULL == cluster) {
          cluster = Set_MakeSingleton(var);
          insert_assoc(self->var2cluster, var, NODE_PTR(cluster));
          self->all_clusters = Set_AddMember(self->all_clusters, NODE_PTR(cluster));

          /* create new cluster->predicates association and add the predicate */
          insert_assoc(self->cluster2preds, NODE_PTR(cluster),
                       NODE_PTR(Set_MakeSingleton(NODE_PTR(predicate))));
        }
        /* cluster already exist => insert the predicate into existing
           cluster2preds associated */
        else {
          Set_t cl_preds = (Set_t)find_assoc(self->cluster2preds, NODE_PTR(cluster));
          nusmv_assert(NULL != cl_preds); /* every cluster has some predicate */
          Set_t tmp = Set_AddMember(cl_preds, predicate);
          nusmv_assert(tmp == cl_preds); /* debug: the pointer did not change */
        }
        /* note that every cluster always has at least one var, and one
           predicate */

        /* check other vars => insert the var in the cluster or merge
           the clusters */
        for (it = Set_GetNextIter(it); !Set_IsEndIter(it);
             it = Set_GetNextIter(it)) {
          var = Set_GetMember(deps, it);
          Set_t another_cluster = (Set_t) find_assoc(self->var2cluster, var);

          /* var has no cluster => add the var to the cluster of previous var */
          if ((Set_t)NULL == another_cluster) {
            another_cluster = Set_AddMember(cluster, var);
            /* debug: the pointer does not change */
            nusmv_assert(cluster == another_cluster);
            insert_assoc(self->var2cluster, var, (node_ptr)cluster);
          }
          /* var has cluster but it is the same as of prev. var => do nothing */
          else if (cluster == another_cluster) {
            /* do nothing */
          }
          /* the var already has its own cluster => push all the info
             into that other cluster and reset the
             hash: var -> cluster and hash: cluster->preds */
          else {
            /* merge the cluster into the other one.  Because of Set class
               implementation, pointer to another_cluster will not change, i.e.
               only cluster associations have to be changed */
            Set_Iterator_t cl_iter;
            SET_FOREACH(cluster, cl_iter) {
              node_ptr a_var = Set_GetMember(cluster, cl_iter);
              Set_t tmp = Set_AddMember(another_cluster, a_var);
              /* debug: the pointer does not change */
              nusmv_assert(another_cluster == tmp);
              insert_assoc(self->var2cluster, a_var, NODE_PTR(another_cluster));
            }
            /* merge the associated predicates */
            Set_t cl_preds = (Set_t)find_assoc(self->cluster2preds,
                                               NODE_PTR(cluster));
            Set_t other_preds = (Set_t)find_assoc(self->cluster2preds,
                                                  NODE_PTR(another_cluster));
            /* every cluster has at least 1 predicate */
            nusmv_assert(NULL != cl_preds && NULL != other_preds);

            Set_t tmp = Set_Union(other_preds, cl_preds);
            nusmv_assert(tmp == other_preds); /* debug: other_preds is a union now */

            Set_ReleaseSet(cl_preds);
            remove_assoc(self->cluster2preds, NODE_PTR(cluster));

            self->all_clusters = Set_RemoveMember(self->all_clusters,
                                                  NODE_PTR(cluster));
            Set_ReleaseSet(cluster);

            cluster = another_cluster;
          }
        } /* for */
      } /* if predicate is not empty */

      Set_ReleaseSet(deps);
    }

    Set_ReleaseSet(self->unclustered_preds);
    self->unclustered_preds = Set_MakeEmpty();
  }

  return self->all_clusters;
}


/**Function********************************************************************

   Synopsis    [Given a fully resolved var name the function
   returns a cluster the variable belongs to]

   Description [If clusters were not computed before this function
   triggers the cluster computation.

   Returned result is Set_t of fully resolved variables.
   Everything returned belongs to self.

   If a var was not met in any of predicates then NULL is
   returned. (This is always so for boolean vars since boolean vars
   cannot be in predicates).

   NOTE: subsequent call of PredicateExtractor_compute_preds makes any
   data returned by this function invalid.]

   SideEffects        []

******************************************************************************/
Set_t PredicateExtractor_get_var_cluster(const PredicateExtractor_ptr self,
                                         node_ptr var)
{
  /* to trigger cluster computation */
  PredicateExtractor_get_all_clusters(self);

  return (Set_t)find_assoc(self->var2cluster, var);
}


/**Function********************************************************************

   Synopsis    [Returns a set of predicates responsible for a given cluster]

   Description [Given a cluster (Set_t of vars) returned by
   PredicateExtractor_get_all_clusters this function
   returns a set of predicates which caused the given cluster.

   Returned result is not-empty Set_t of fully expanded/resolved expressions
   and belongs to self.

   NOTE: subsequent call of PredicateExtractor_compute_preds makes any
   data returned by this function or
   PredicateExtractor_get_all_clusters invalid.]

   SideEffects        []

******************************************************************************/
Set_t
PredicateExtractor_get_preds_of_a_cluster(const PredicateExtractor_ptr self,
                                          Set_t cluster)
{
  Set_t preds;

  /* PredicateExtractor_compute_preds was called after
     PredicateExtractor_get_all_clusters which is an error */
  nusmv_assert(Set_IsEmpty(self->unclustered_preds));

  preds = (Set_t)find_assoc(self->cluster2preds, NODE_PTR(cluster));

  nusmv_assert(preds != NULL); /* every cluster has predicates */

  return preds;
}


/**Function********************************************************************

   Synopsis    [The function prints out the predicates collected so far
   and clusters computed.]

   Description [Options printPredicates and printClusters
   control what should be printed.
   At least one of them has to be set up.

   If only predicates are printed, then they are printed in the order
   they were obtained.

   Otherwise, clusters are printed and if additionally printPredicates
   is up then the after every cluster its predicates are printed.

   Note that if clusters were not computed so far but asked to be
   printed, they will be computed.
   ]

   SideEffects        []

******************************************************************************/
void
PredicateExtractor_print(const PredicateExtractor_ptr self,
                         FILE* stream,
                         boolean printPredicates,
                         boolean printClusters)
{
  Set_t set;
  Set_Iterator_t iter;
  int clst_num = 0;

  if (!printPredicates && !printClusters) {
    rpterr("Function PredicateExtractor_print needs at least one "
           "of printPredicates and printClusters to be true.");
  }

  /* ----------- print just predicates */
  if (printPredicates && ! printClusters) {
    fprintf(stream, "\nPredicates are :\n-------------------------------\n");
    set = PredicateExtractor_get_all_preds(self);
    SET_FOREACH(set, iter) {
      fprintf(stream, "\n   ");
      print_node(stream, Set_GetMember(set, iter));
    }
    fprintf(stream, "\n------------------------------------\n");
    return;
  }

  /* -------------  print clusters */
  set = PredicateExtractor_get_all_clusters(self);
  SET_FOREACH(set, iter) {
    Set_t cluster = (Set_t) Set_GetMember(set, iter);
    Set_Iterator_t sit;

    /* output the clusters */
    fprintf(stream,
            "\n--------------------------------------------------\n"
            "---- Cluster %d \n \t [\n", clst_num);
    /* Clusters */
    SET_FOREACH(cluster, sit) {
      node_ptr var = Set_GetMember(cluster, sit);
      fprintf(stream, " \t   ");
      print_node(stream, var);
      fprintf(stream, " : ");
      SymbType_print(SymbTable_get_var_type(self->st, var), stream);
      fprintf(stream, "\n");
    }
    fprintf(stream, " \t ]\n");

    /* stream the predicates */
    if (printPredicates) {
      /* Preds */
      Set_t preds = (Set_t)find_assoc(self->cluster2preds,
                                      NODE_PTR(cluster));
      nusmv_assert(NULL != preds); /* every cluster has at least one predicate */

      fprintf(stream, " \t Predicates for Cluster %d\n \t (\n", clst_num);
      SET_FOREACH(preds, sit) {
        node_ptr pr = Set_GetMember(cluster, sit);
        fprintf(stream, " \t   ");
        print_node(stream, pr);
        fprintf(stream, "\n");
      }
      fprintf(stream, " \t )\n\n");
    }

    return;
  }

}


/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis    [initialiser of an object of this class]

   Description []

   SideEffects        []

******************************************************************************/
static void pred_extract_init(PredicateExtractor_ptr self,
                              SymbTable_ptr st, boolean use_approx)
{
  node_ptr w0,w1;

  self->all_preds = Set_MakeEmpty();
  self->unclustered_preds = Set_MakeEmpty();
  self->all_clusters = Set_MakeEmpty();
  self->var2cluster = new_assoc();
  self->cluster2preds = new_assoc();
  self->expr2preds = new_assoc();

  self->special_int_preds[0] = Set_MakeSingleton(zero_number);
  self->special_int_preds[1] = Set_MakeSingleton(one_number);
  self->special_int_preds[2] = Set_AddMember(Set_MakeSingleton(zero_number),
                                             one_number);

  w0 = find_node(NUMBER_UNSIGNED_WORD, NODE_PTR(WordNumber_from_integer(0,1)), Nil);
  w1 = find_node(NUMBER_UNSIGNED_WORD, NODE_PTR(WordNumber_from_integer(1,1)), Nil);
  self->special_word_preds[0] = Set_MakeSingleton(w0);
  self->special_word_preds[1] = Set_MakeSingleton(w1);
  self->special_word_preds[2] = Set_AddMember(Set_MakeSingleton(w0), w1);

  self->st = st;
  self->checker = SymbTable_get_type_checker(st);
  self->use_approx = use_approx;
}


/**Function********************************************************************

   Synopsis    [de-initialiser of an object of this class]

   Description []

   SideEffects        []

******************************************************************************/
static void pred_extract_deinit(PredicateExtractor_ptr self)
{
  assoc_iter iter;
  Set_t cluster, preds;
  node_ptr expr;
  Set_t tmp;

  nusmv_assert(TYPE_CHECKER(NULL) != self->checker);

  /* free 3 special predicate subparts */
  Set_ReleaseSet(self->special_int_preds[0]);
  Set_ReleaseSet(self->special_int_preds[1]);
  Set_ReleaseSet(self->special_int_preds[2]);

  Set_ReleaseSet(self->special_word_preds[0]);
  Set_ReleaseSet(self->special_word_preds[1]);
  Set_ReleaseSet(self->special_word_preds[2]);


  /* free Set_t of predicate subparts in expr2preds.  different
     expressions may point to the same set of predicates (e.g. this
     happens with defines) => collect all sets in one set and then
     release them.  This allows to avoid double releases. */
  tmp = Set_MakeEmpty();

  ASSOC_FOREACH(self->expr2preds, iter, &expr, &preds) {
    /* preds must exist and should not be one of special predicates set */
    nusmv_assert(preds != NULL &&
                 preds != self->special_int_preds[0] &&
                 preds != self->special_int_preds[1] &&
                 preds != self->special_int_preds[2] &&
                 preds != self->special_word_preds[0] &&
                 preds != self->special_word_preds[1] &&
                 preds != self->special_word_preds[2]);

    if (!IS_FLAG_PREDICATES(preds)) {
      tmp = Set_AddMember(tmp, NODE_PTR(preds));
    }
  }
  Set_ReleaseSetOfSet(tmp);
  free_assoc(self->expr2preds);

  /* free Set_t of predicates in cluster2preds */
  ASSOC_FOREACH(self->cluster2preds, iter, &cluster, &preds) {
    nusmv_assert(preds != NULL);
    Set_ReleaseSet(preds);
  }
  free_assoc(self->cluster2preds);

  /* no need to free cluster in var2cluster */
  free_assoc(self->var2cluster);

  /* free clusters which are Set_t of Set_t */
  Set_ReleaseSetOfSet(self->all_clusters);

  /* predicate sets are just Set_t of node_ptr */
  Set_ReleaseSet(self->unclustered_preds);
  Set_ReleaseSet(self->all_preds);


  /* debugging code : setting to NULL */
  self->all_preds = (Set_t)NULL;
  self->unclustered_preds = (Set_t)NULL;
  self->all_clusters = (Set_t)NULL;
  self->var2cluster = (hash_ptr)NULL;
  self->cluster2preds = (hash_ptr)NULL;
  self->expr2preds = (hash_ptr)NULL;
  self->special_int_preds[0] = (Set_t)NULL;
  self->special_int_preds[1] = (Set_t)NULL;
  self->special_int_preds[2] = (Set_t)NULL;
  self->special_word_preds[0] = (Set_t)NULL;
  self->special_word_preds[1] = (Set_t)NULL;
  self->special_word_preds[2] = (Set_t)NULL;

  self->st = SYMB_TABLE(NULL);
  self->checker = TYPE_CHECKER(NULL);
}

/**Function********************************************************************

   Synopsis    [Performs the predicates extraction]

   Description [See PredicateExtractor_compute_preds for more info.

   This is the main function for extraction.

   The function returns the set of predicate subparts, i.e. Set_t of
   node_ptr. For expressions having whole predicates (i.e. boolean
   expressions) PREDICATES_TRUE/PREDICATES_FALSE/PREDICATES_ARBITRARY
   value is returned.

   Returned set of predicate subparts belong to self->expr2preds. The
   expression (predicates subparts are find_node-ed and belong to
   whole system).]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static Set_t pred_extract_process_recur(PredicateExtractor_ptr self,
                                        node_ptr expr,
                                        node_ptr context)
{
  SymbType_ptr type;
  Set_t result, left, right;
  node_ptr tmp;
  node_ptr key;
  int node_type;
  
  nusmv_assert(Nil != expr);

  key = find_node(CONTEXT, context, expr);
  node_type = node_get_type(expr);

  /* is already processed. */
  result = (Set_t) find_assoc(self->expr2preds, key);
  if (NULL != result) return result;

  type = TypeChecker_get_expression_type(self->checker, expr, context);
  nusmv_assert(!SymbType_is_error(type));

  /* for sure incorrect value for debugging */
  result = left = right = (Set_t) -1;

  /* process every kind of an expression */
  switch (node_type) {
  case CONTEXT:
    {
      node_ptr new_ctx = CompileFlatten_concat_contexts(context, car(expr));
      result = pred_extract_process_recur(self, cdr(expr), new_ctx);
    }
    break;

    /* list of simple boolean constants => they are not part of predicates */
  case FAILURE:
    error_unreachable_code(); /* failures are dealt in IF and CASE expressions.
                            it must be deal there because otherwise
                            optimizations with only TRUE/only FALSE
                            predicates may become impossible */
    result = PREDICATES_ARBITRARY;
    break;

  case FALSEEXP:
    result = PREDICATES_FALSE;
    break;

  case TRUEEXP:
    result = PREDICATES_TRUE;
    break;

    /* NUMBER may be boolean and not-boolean. Here we always consider
       it as not-boolean. The outer expression will decide what to do
       with the result. */
  case NUMBER:

    /* list of simple not-boolean constants => they become predicates subpart */
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case UWCONST:
  case SWCONST:
  case TWODOTS:
    tmp = Expr_resolve(self->st, node_type, car(expr), cdr(expr));
    result = Set_MakeSingleton(tmp);
    break;

  case ARRAY: {
    ResolveSymbol_ptr rs;
    node_ptr resolvedName;
    rs = SymbTable_resolve_symbol(self->st, expr, context);
    resolvedName = ResolveSymbol_get_resolved_name(rs);

    if (ResolveSymbol_is_undefined(rs)) {
      /* Array may be an identifier-with-brackets and may be
         expression.  Here an array-expression is detected =>
         expression is to be flattened at first to resolve array
         identifiers-with-brackets (see description of
         compileFlattenSexpRecuras for details) and then general
         predicate extractor is to be invoked */
      node_ptr tmp = Compile_FlattenSexp(self->st, expr, context);
      nusmv_assert(tmp != expr); /* loop in recursion is impossible */
      result = pred_extract_process_recur(self, tmp, Nil);
      break;
    }
    else {
      /* array is actually identifier => process it with other identifiers */
    }
    /* NO BREAK HERE */
  }

  case DOT:
  case ATOM:
  case BIT: {
    /* The expression is a symbol.
       It can be a variable, a define, a constant or a parameter.
       The expression may have been flattened as well as not flattened.

       Note, that NO ERRORS CAN BE HERE, since all the error
       situations have been checked during type-checking of the
       original expression.
    */

    /* First, try to resolve the symbol */
    ResolveSymbol_ptr rs;
    node_ptr resolvedName;

    rs = SymbTable_resolve_symbol(self->st, expr, context);
    resolvedName = ResolveSymbol_get_resolved_name(rs);

    /* Check whether this is a variable */
    if (ResolveSymbol_is_var(rs)) {
      if (!SymbType_is_boolean(type)) {
        result = Set_MakeSingleton(resolvedName);
      } /* boolean vars make predicates have arbitrary values */
      else result = PREDICATES_ARBITRARY;
    }

    /* check whether is a define */
    else if (ResolveSymbol_is_define(rs)) {
      node_ptr def = SymbTable_get_define_body(self->st, resolvedName);
      node_ptr ctx = SymbTable_get_define_context(self->st, resolvedName);

      /* the context is Nil because expr is already flattened */
      result = pred_extract_process_recur(self, def, ctx);

      /* special case: array define may be declared with Integer (or
         higher) subtype and at the same time has a boolean element.
         In this case the boolean element has to be casted to integer.
      */
      if (ARRAY == node_type &&
          !SymbType_is_boolean(type) &&
          SymbType_is_boolean(TypeChecker_get_expression_type(self->checker,
                                                              def, ctx))) {
        /* boolean can be casted to Int, Int-Symb or their Sets only
           thus conversion to integer is enough*/
        nusmv_assert(SymbType_is_integer(type) ||
                     SymbType_is_int_symbolic_enum(type) ||
                     SYMB_TYPE_SET_INT == SymbType_get_tag(type) ||
                     SYMB_TYPE_SET_INT_SYMB == SymbType_get_tag(type));
        if (IS_FLAG_PREDICATES(result)) {
          result = Set_Copy(pred_extract_bool2int(self, def, result));
        }
      }
    }
    /* check whether this symbol is a constant. The ResolveSymbol
       takes care of simple/complex constants */
    else if (ResolveSymbol_is_constant(rs)) {
      result = Set_MakeSingleton(resolvedName);
    }
    else if (ResolveSymbol_is_function(rs)) {
      result = Set_MakeSingleton(resolvedName);
    }
    /* check whether this symbol is a parameter */
    else {
      node_ptr param = Nil;
      node_ptr new_ctx;

      /* it must be a parameter but being a parameter is the last
         possibility */
      nusmv_assert(ResolveSymbol_is_parameter(rs));

      param = SymbTable_get_actual_parameter(self->st, resolvedName);

      new_ctx = SymbTable_get_actual_parameter_context(self->st, resolvedName);

      result = pred_extract_process_recur(self, param, new_ctx);
    }

    break;
  } /* ATOM */

    /* boolean unary expression or boolean binary expressions those
       right child can be ignored and which have not to be optimized */
  case EX: case AX: case EF: case AF: case EG: case AG:
  case OP_NEXT: case OP_PREC: case OP_NOTPRECNOT: case OP_GLOBAL:
  case OP_HISTORICAL: case OP_FUTURE: case OP_ONCE:
  case EBF: case ABF: case EBG: case ABG: /* ignore the number..number part */
  case ABU: case EBU: /* ignore the number..number part */
    nusmv_assert(SymbType_is_boolean(type)); /* only boolean can be here */
    result = pred_extract_process_recur(self, car(expr), context);
    result = PREDICATES_ARBITRARY;
    break;

    /* unary operations which may or may not be boolean */
  case NOT:
    nusmv_assert(Nil == cdr(expr)); /* checking that indeed no right child */
    left = pred_extract_process_recur(self, car(expr), context);

    /* if it is boolean => apply the operator on possible constant
       value */
    if (SymbType_is_boolean(type)) {
      if (result == PREDICATES_TRUE) result = PREDICATES_FALSE;
      else if (result == PREDICATES_FALSE) result = PREDICATES_TRUE;
      else result = PREDICATES_ARBITRARY;
    }
    else { /* otherwise apply the bitwise operator */
      result = pred_extract_apply_unary(self, NOT, left);
    }
    break;

    /* unary operations: cannot have boolean operand */
  case UMINUS:
    nusmv_assert(Nil == cdr(expr));    /* checking that indeed no right child */
    left = pred_extract_process_recur(self, car(expr), context);
    result = pred_extract_apply_unary(self, UMINUS, left);
    break;

    /* binary boolean operations which cannot be optimized.
       CONS is artificial expression, it is boolean and may have empty
       right child */
  case CONS:
  case UNTIL: case SINCE:
  case AU: case EU:
    left = pred_extract_process_recur(self, car(expr), context);

    if (cdr(expr) != Nil) {
      right = pred_extract_process_recur(self, cdr(expr), context);
    }
    result = PREDICATES_ARBITRARY;
    break;

    /* binary expression which may or may not be boolean and
       which can be optimized.
       note: children here always have the same type as the
       expression */
  case AND: case OR: case XOR: case XNOR: case IFF: case IMPLIES:

    if (SymbType_is_boolean(type)) {
      left = pred_extract_process_recur(self, car(expr), context);

      /* optimization : check if the first operator result is enough */
      if ((left == PREDICATES_FALSE && node_type == AND) ||
          (left == PREDICATES_TRUE && node_type == OR)) {
        result = left;
        break;
      }
      else if (left == PREDICATES_FALSE && node_type == IMPLIES) {
        result = PREDICATES_TRUE;
        break;
      }
      /* process the second argument (as optimization did not work) */
      right = pred_extract_process_recur(self, cdr(expr), context);

      /* compute the value if possible */
      switch (node_type) {
      case AND:
        result = (right == PREDICATES_FALSE)
          ? PREDICATES_FALSE
          : ( (left == PREDICATES_TRUE && right == PREDICATES_TRUE)
              ? PREDICATES_TRUE
              : PREDICATES_ARBITRARY
              );
        break;

      case OR:
        result = (right == PREDICATES_TRUE)
          ? PREDICATES_TRUE
          : ( (left == PREDICATES_FALSE && right == PREDICATES_FALSE)
              ? PREDICATES_FALSE
              : PREDICATES_ARBITRARY
              );
        break;

      case XOR:
        result = (left == PREDICATES_ARBITRARY || right == PREDICATES_ARBITRARY)
          ? PREDICATES_ARBITRARY
          : (left != right ? PREDICATES_TRUE : PREDICATES_FALSE);
        break;

      case XNOR:
      case IFF:
        result = (left == PREDICATES_ARBITRARY || right == PREDICATES_ARBITRARY)
          ? PREDICATES_ARBITRARY
          : (left == right ? PREDICATES_TRUE : PREDICATES_FALSE);
        break;

      case IMPLIES:
        result = (right == PREDICATES_TRUE)
          ? PREDICATES_TRUE
          : ( (left == PREDICATES_TRUE && right == PREDICATES_FALSE)
              ? PREDICATES_FALSE
              : PREDICATES_ARBITRARY
              );
        break;

      default: error_unreachable_code(); /* impossible code */
      }
      /* debug: result was set up to a constant */
      nusmv_assert(IS_FLAG_PREDICATES(result));
    }
    else { /* this is not a boolean =>
              it can be only word operations, i.e.
              apply the binary operator to results. */
      left = pred_extract_process_recur(self, car(expr), context);
      right = pred_extract_process_recur(self, cdr(expr), context);

      nusmv_assert(!IS_FLAG_VALID_PREDICATES(left));
      nusmv_assert(!IS_FLAG_VALID_PREDICATES(right));
      result = pred_extract_apply_binary(self, node_type, left, right);
    }
    break;

    /* not-boolean unary operators */
  case CAST_UNSIGNED:
  case CAST_SIGNED:
    nusmv_assert(Nil == cdr(expr));    /* checking that indeed no right child */
    left = pred_extract_process_recur(self, car(expr), context);

    nusmv_assert(!IS_FLAG_VALID_PREDICATES(left)); /* there must be result */

    result = pred_extract_apply_unary(self, node_type, left);
    break;



    /* "next" and "init" are here as normal unary operation because
       EQDEF is a normal binary operation. No cast is done here. */
  case NEXT:
  case SMALLINIT:
    nusmv_assert(Nil == cdr(expr)); /* checking that indeed no right child */
    result = pred_extract_process_recur(self, car(expr), context);

    /* note that here init and next are applied without modifications,
       i.e.  next(x) := 3 will be kept as it is whereas next(x := 3)
       will be kept as x:=3 because next is applied outside of the
       predicate, not inside.

       It does not matter if we (not) get rid of "init" or "next".
    */
    if (!IS_FLAG_PREDICATES(result)) { /* this is not true boolean =>
                                          apply the operator */
      result = pred_extract_apply_unary(self, node_type, result);
    }
    else {
      /* the true/false value is not changed by next/init => do nothing */
    }
    break;


    /* relational operators: they convert not-boolean to boolean  */
  case EQDEF: case SETIN:
  case EQUAL: case NOTEQUAL: case LT: case GT: case LE: case GE: {
    SymbType_ptr type1 = TypeChecker_get_expression_type(self->checker,
                                                         car(expr), context);
    SymbType_ptr type2 = TypeChecker_get_expression_type(self->checker,
                                                         cdr(expr), context);
    nusmv_assert(!SymbType_is_error(type1));
    nusmv_assert(!SymbType_is_error(type2));

    left = pred_extract_process_recur(self, car(expr), context);

    if (IS_OVER_APPROX(left)) {
      result = PREDICATES_ARBITRARY;
      break;
    }
    right = pred_extract_process_recur(self, cdr(expr), context);
    if (IS_OVER_APPROX(right)) {
      result = PREDICATES_ARBITRARY;
      break;
    }

    /* both operands are boolean (or bool-set for EQDEF and SETIN) */
    if ((SymbType_is_boolean(type1) ||
         SYMB_TYPE_SET_BOOL == SymbType_get_tag(type1)) &&
        (SymbType_is_boolean(type2) ||
         SYMB_TYPE_SET_BOOL == SymbType_get_tag(type2))) {

      /* compute the value of expression if possible */
      switch (node_type) {
      case EQDEF:
      case SETIN:
      case EQUAL:
        result = (left == PREDICATES_ARBITRARY || right == PREDICATES_ARBITRARY)
          ? PREDICATES_ARBITRARY
          : (left == right ? PREDICATES_TRUE : PREDICATES_FALSE);
        break;

      case NOTEQUAL:
        result = (left == PREDICATES_ARBITRARY || right == PREDICATES_ARBITRARY)
          ? PREDICATES_ARBITRARY
          : (left != right ? PREDICATES_TRUE : PREDICATES_FALSE);
        break;

      case GT: /* exchange right and left and jump to LT */
        {Set_t tmp = left; left = right; right = tmp;}
        /* no break here! */
      case LT:
        result = (left == PREDICATES_TRUE || right == PREDICATES_FALSE)
          ? PREDICATES_FALSE
          : (left == PREDICATES_FALSE && right == PREDICATES_TRUE)
          ? PREDICATES_TRUE
          : PREDICATES_ARBITRARY;
        break;

      case GE: /* exchange right and left and jump to LE */
        {Set_t tmp = left; left = right; right = tmp;}
        /* no break here! */
      case LE:
        result = (left == PREDICATES_FALSE || right == PREDICATES_TRUE)
          ? PREDICATES_TRUE
          : (left == PREDICATES_TRUE && right == PREDICATES_FALSE)
          ? PREDICATES_FALSE
          : PREDICATES_ARBITRARY;
        break;

      default: error_unreachable_code(); /* impossible code */
      }
    }
    /* both operands are scalar => do bool2int to cast "true
       boolean" operand (if there is one) and apply the binary
       operator */
    else {
      if (IS_FLAG_PREDICATES(left)) {
        left = pred_extract_bool2int(self, car(expr), left);
      }
      if (IS_FLAG_PREDICATES(right)) {
        right = pred_extract_bool2int(self, cdr(expr), right);
      }
      result = pred_extract_apply_binary(self, node_type, left, right);

      /* remember the results */
      result = pred_extract_fix_any_preds(self, result);
    }

    break;
  }

    /* these exprs are always scalar, but there may be
       boolean operands which have to be casted to int.
       no optimizations are done here.
    */
  case TIMES: case DIVIDE: case PLUS :case MINUS: case MOD:
  case LSHIFT: case RSHIFT: /*case LROTATE: case RROTATE: */
  case EXTEND: case WRESIZE:
  case WAREAD: case WAWRITE: { /* extend and these two
                                  WordArray operators cannot have boolean operands
                                  (type checked already). It is OK no to check it
                                  here.  WAWRITE has actually three-operands but
                                  the second and third are dealt recursively
                               */
    left = pred_extract_process_recur(self, car(expr), context);
    right = pred_extract_process_recur(self, cdr(expr), context);

    if (IS_FLAG_PREDICATES(left)) {
      left = pred_extract_bool2int(self, car(expr), left);
    }
    if (IS_FLAG_PREDICATES(right)) {
      right = pred_extract_bool2int(self, cdr(expr), right);
    }

    result = pred_extract_apply_binary(self, node_type, left, right);
    break;
  }

    /* COLON cannot go as a independent operation */
  case COLON: error_unreachable_code();

  case BIT_SELECTION: {
    /* just consistency check */
    nusmv_assert(COLON == node_get_type(cdr(expr)));

    left = pred_extract_process_recur(self, car(expr), context);
    right = Set_MakeSingleton(find_node(COLON, find_atom(car(cdr(expr))),
                                        find_atom(cdr(cdr(expr)))));

    result = pred_extract_apply_binary(self, node_type, left, right);
    break;
  }

  case WSIZEOF: {
    /* sizeof returns the bit-size of word expressions without evaluating
       the expression itself */
    int width;
    nusmv_assert(SymbType_is_word(type));
    width = SymbType_get_word_width(type);
    result = Set_MakeSingleton(find_node(NUMBER, NODE_FROM_INT(width), Nil));
    break;
  }

  case CAST_TOINT: {
    left = pred_extract_process_recur(self, car(expr), context);
    nusmv_assert(Nil == cdr(expr)); /* indeed no right child */

    if (!IS_FLAG_PREDICATES(left)) {
      result = pred_extract_apply_unary(self, node_type, left);
    }
    else {
      result = Set_Copy(pred_extract_bool2int(self, car(expr), left));
    }
    break;
  }

  case COUNT: {
    node_ptr list = car(expr);
    result = Set_MakeEmpty();

    while (Nil != list) {
      Set_t elem = pred_extract_process_recur(self, car(list), context);

      if (!IS_FLAG_PREDICATES(elem)) {
        Set_t tmp = pred_extract_apply_unary(self, node_type, elem);
        result = Set_Union(result, tmp);
        Set_ReleaseSet(tmp);
      }
      else {
        result = Set_Union(result,
                           pred_extract_bool2int(self, car(list), elem));
      }

      list = cdr(list);
    }

    break;
  }

    /* concatenation requires two word arguments (i.e. scalar).
       the only required thing is to convert bool to word1.
    */
  case CONCATENATION: {
    SymbType_ptr type1 = TypeChecker_get_expression_type(self->checker,
                                                         car(expr), context);
    SymbType_ptr type2 = TypeChecker_get_expression_type(self->checker,
                                                         cdr(expr), context);
    nusmv_assert(!SymbType_is_error(type1));
    nusmv_assert(!SymbType_is_error(type2));

    left = pred_extract_process_recur(self, car(expr), context);
    right = pred_extract_process_recur(self, cdr(expr), context);
    result = pred_extract_apply_binary(self, node_type, left, right);
    break;
  }

    /* cast to bool is the same as comparison with 0d1_1 */
  case CAST_BOOL: {
    left = pred_extract_process_recur(self, car(expr), context);
    nusmv_assert(cdr(expr) == Nil); /* indeed no right child */

    result = pred_extract_apply_binary(self, EQUAL, left,
                                       self->special_word_preds[1]);
    /* remember the results */
    result = pred_extract_fix_any_preds(self, result);
    break;
  }

  case CAST_WORD1:
    left = pred_extract_process_recur(self, car(expr), context);
    nusmv_assert(cdr(expr) == Nil);

    /* create a word1 possible values */
    if (left == PREDICATES_FALSE) result = self->special_word_preds[0];
    else if (left == PREDICATES_TRUE) result = self->special_word_preds[1];
    else result = self->special_word_preds[2];
    result = Set_Copy(result); /* create a copy of special predicate set */
    break;

    /* UNION just perform union of possible predicate subparts.
       The same as in PredicateNormaliser.c boolean are always casted
       to int.
       The difference from PredicateNormaliser.c is that here UNION
       is not applied. It is unclear which way to go, .e.g
       should "A in (B union C)" become "(A in B) union (A in C)"
       or not? Especially taking into account that cast
       from bool-set to int does not exist.

       NB: if this code is to be changed then change also the same
       part in PredicateNormaliser.c.
    */
  case UNION:
    left = pred_extract_process_recur(self, car(expr), context);
    right = pred_extract_process_recur(self, cdr(expr), context);

    if (IS_FLAG_PREDICATES(left)) {
      left = pred_extract_bool2int(self, car(expr), left);
    }
    if (IS_FLAG_PREDICATES(right)) {
      right = pred_extract_bool2int(self, cdr(expr), right);
    }

    result = Set_Union(Set_Copy(left), right);
    break;


  case IFTHENELSE:
  case CASE: {
    Set_t cond, then, tail;
    node_ptr simp_cond_expr;

    nusmv_assert(COLON == node_get_type(car(expr)));

    /* simplification added for issue 02590: we first simplify the
     condition, by flattening it as Exp_simplfy does not support
     contextualized expressions */
    simp_cond_expr = Expr_simplify(self->st,
                           Compile_FlattenSexp(self->st, caar(expr), context));
    /* since simp_cond_expr is flattened, context becomes Nil here: */
    cond = pred_extract_process_recur(self, simp_cond_expr, Nil);

    /* if condition is a constant => process only one branch.
       also if tail is FAILURE => ignore it. */
    if (cond == PREDICATES_TRUE || FAILURE == node_get_type(cdr(expr))) {
      result = pred_extract_process_recur(self, cdr(car(expr)), context);
    }
    else if (cond == PREDICATES_FALSE) {
      result = pred_extract_process_recur(self, cdr(expr), context);
    }
    else { /* process both branches */
      /* the only remaining value */
      nusmv_assert(cond == PREDICATES_ARBITRARY);

      then = pred_extract_process_recur(self, cdr(car(expr)), context);
      tail = pred_extract_process_recur(self, cdr(expr), context);

      /* if expression is boolean then get rid of all predicates,
         otherwise make the union of the results */
      if (SymbType_is_boolean(type)) {
        /* optimization : both branches return the same constant =>
           return it as the value of the whole expression */
        if (then == tail) result = then;
        else result = PREDICATES_ARBITRARY;
      }
      else {
        /* make union of the results from then and tail */
        /* it may be necessary to apply bool2int cast */
        if (IS_FLAG_PREDICATES(then)) {
          then = pred_extract_bool2int(self, cdr(car(expr)), then);
        }
        if (IS_FLAG_PREDICATES(tail)) { /* cast bool2int */
          tail = pred_extract_bool2int(self, cdr(expr), tail);
        }

        if (IS_OVER_APPROX(then)) result = then;
        else if (IS_OVER_APPROX(tail)) result = tail;
        else result = Set_Union(Set_Copy(then), tail);
      }
    }
    break;
  }

  case ATTIME: {
    left = pred_extract_process_recur(self, car(expr), context);
    /* do not normalise right operand, it is just number */

    /* this is boolean => just get rid of predicates */
    if (SymbType_is_boolean(type)) { 

      result = left;
    }
    else { /* this is not a boolean => apply the operator */
      right = Set_MakeSingleton(find_atom(cdr(expr)));
      result = pred_extract_apply_binary(self, node_type, left, right);
      Set_ReleaseSet(right);
    }

    break;
  }

  default:
    /* below condition is introduced in PredicateNormalization by RC and
       copied here by AT. */
#ifndef NDEBUG

    left = pred_extract_process_recur(self, car(expr), context);
    right = pred_extract_process_recur(self, cdr(expr), context);
    result = pred_extract_apply_binary(self, node_type, left, right);
    break;
#else
    print_sexp(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "unknown token = %d\n", node_type);
    error_unreachable_code(); /* unknown kind of an expression */
#endif
  } /* switch */

  /* debug that result was properly set up */
  nusmv_assert(result != (Set_t) -1);

  /* preds returned should not be one of special predicates set
     which used only to construct other predicates */
  nusmv_assert(result != self->special_int_preds[0] &&
               result != self->special_int_preds[1] &&
               result != self->special_int_preds[2] &&
               result != self->special_word_preds[0] &&
               result != self->special_word_preds[1] &&
               result != self->special_word_preds[2]);

  /* boolean expressions may have only boolean predicate or nothing.
     not-boolean expressions always have proper predicate set */
  nusmv_assert(!(SymbType_is_boolean(type) ||
                 SYMB_TYPE_SET_BOOL == SymbType_get_tag(type)
                 ) ||
               pred_extract_is_bool_preds(result));
  nusmv_assert((SymbType_is_boolean(type) ||
                SYMB_TYPE_SET_BOOL == SymbType_get_tag(type)
                ) ||
               !IS_FLAG_PREDICATES(result) ||
               IS_OVER_APPROX(result));

  /* remember the processed expression */
  insert_assoc(self->expr2preds, key, NODE_PTR(result));
  return result;
}


/**Function********************************************************************

   Synopsis   [This function returns true iff the result set of predicates
   subparts may belong only to boolean expression]

   Description [There 2 expressions which can be boolean and not
   boolean at the same time: "0" and 1".  These can be considered
   as predicate subparts as well as complete predicates.

   This function returns true iff the result consists of such kind
   of predicates.]

   SideEffects  [pred_extract_process_recur]

******************************************************************************/
static boolean pred_extract_is_bool_preds(Set_t result)
{
  Set_Iterator_t iter;

  /* a flag that expressions is truly boolean => not predicate subparts */
  if (IS_FLAG_PREDICATES(result)) return true;

  SET_FOREACH(result, iter) {
    node_ptr expr = Set_GetMember(result, iter);
    /* there may be "next" or "init" wrapping boolean predicates */
    node_ptr unnexted
      = (node_get_type(expr) == NEXT || node_get_type(expr) == SMALLINIT)
      ? car(expr) : expr;

    /* for 0 and 1 just do nothing. such predicates are useless. */
    if (one_number != unnexted && zero_number != unnexted) {
      return false;
    }
  } /* loop */

  return true;
}


/**Function********************************************************************

   Synopsis [This function put any expression in the set into
   "self" as complete predicates. "result" if released by this
   function.]

   Description []

   SideEffects  [pred_extract_process_recur]

******************************************************************************/
static Set_t pred_extract_fix_any_preds(PredicateExtractor_ptr self,
                                        Set_t result)
{
  Set_Iterator_t iter;
  boolean there_is_0 = false;
  boolean there_is_1 = false;
  boolean there_is_arbit = false;

  /* overapproximation resolves to <T,F> */
  if (IS_OVER_APPROX(result)) return PREDICATES_ARBITRARY;

  nusmv_assert(!IS_FLAG_PREDICATES(result)); /* only proper sets are expected */

  SET_FOREACH(result, iter) {

    node_ptr expr = Set_GetMember(result, iter);

    /* optimization: skip 0 and 1, TRUE and FALSE as useless predicates. */
    if (FALSEEXP == node_get_type(expr)) {
      there_is_0 = true;
    }
    else if (TRUEEXP == node_get_type(expr)) {
      there_is_1 = true;
    }
    else {
      there_is_arbit = true;
      /* remember the obtained predicates (only if it is new) */
      if (!Set_IsMember(self->all_preds, expr)) {
        self->all_preds = Set_AddMember(self->all_preds, expr);
        self->unclustered_preds = Set_AddMember(self->unclustered_preds, expr);
      }
    }
  } /* loop */

  Set_ReleaseSet(result);

  if (there_is_0 && !there_is_1 && !there_is_arbit) return PREDICATES_FALSE;
  if (!there_is_0 && there_is_1 && !there_is_arbit) return PREDICATES_TRUE;

  return PREDICATES_ARBITRARY;
}


/**Function********************************************************************

   Synopsis   [This function perform cast bool-to-int during predicate
   extraction. Result set can be accessed but not remembered
   in self->expr2preds.]

   Description [Normally bool2int cast means the predicate subparts
   are the set {0, 1}. But as optimization this function checks the
   original expression. If it is TRUE then only number 1 is returned,
   if it is FALSE then 0 is returned, and only otherwise {0,1} is
   returned.

   'expr' is the original expression and 'preds' is predicate set computed
   for expr.

   The returned Set_t belongs to self->special_int_preds and cannot be
   used anywhere else. Use Set_Copy to create a copy to store it in
   the expr2preds.

   NOTE: 'expr' is not actually required. currently used for debugging only.
   ]

   SideEffects  [pred_extract_process_recur]

******************************************************************************/
static Set_t pred_extract_bool2int(PredicateExtractor_ptr self,
                                   node_ptr expr, Set_t preds)
{
  if (preds == PREDICATES_FALSE) return self->special_int_preds[0];
  if (preds == PREDICATES_TRUE)  return self->special_int_preds[1];

  /* the only remaining value */
  nusmv_assert(PREDICATES_ARBITRARY == preds || 
               PREDICATES_OVERAPPROX == preds);

  /* constants cannot be here as predicates are not constants */
  nusmv_assert(FALSEEXP != node_get_type(expr) &&
               TRUEEXP != node_get_type(expr));

  return self->special_int_preds[2];
}


/**Function********************************************************************

   Synopsis    [This function take a unary operator,
   the result of predicates extraction of the child expression
   and returns new result for the whole expression.]

   Description [This function is used only by pred_extract_process_recur.
   Created set belongs to the invoker, i.e. to pred_extract_process_recur
   which will insert them into self->expr2preds.

   ]

   SideEffects  [pred_extract_process_recur]

******************************************************************************/
static Set_t pred_extract_apply_unary(PredicateExtractor_ptr self,
                                      int type,
                                      Set_t childResult)
{
  Set_t result = Set_MakeEmpty();
  Set_Iterator_t iter;

  /* keep approximation */
  if (IS_OVER_APPROX(childResult)) {
    return PREDICATES_OVERAPPROX;
  }

  /* child result is properly created set of predicates to apply the operator */
  nusmv_assert(!IS_FLAG_PREDICATES(childResult));

  /* apply the operator to every element of the predicate subparts */

  SET_FOREACH(childResult, iter) {
    node_ptr expr = Set_GetMember(childResult, iter);
    expr = Expr_resolve(self->st, type, expr, Nil);
    result = Set_AddMember(result, expr);
  }

  return result;
}


/**Function********************************************************************

   Synopsis    [This function take a binary operator,
   the results of predicates extraction of the children subexpressions
   and returns new result for the whole expression]

   Description [This function is used only by pred_extract_process_recur.
   Created set belongs to an invoker, pred_extract_process_recur which will
   insert them into self->expr2preds.

   ]

   SideEffects  [pred_extract_process_recur]

******************************************************************************/
static Set_t pred_extract_apply_binary(PredicateExtractor_ptr self,
                                       int type,
                                       Set_t leftResult,
                                       Set_t rightResult)
{
  Set_t result;
  Set_Iterator_t l_iter;

  /* keep approximation */
  if (IS_OVER_APPROX(leftResult) || IS_OVER_APPROX(rightResult)) {
    return PREDICATES_OVERAPPROX;
  }

  /* children results are properly created predicates to apply the operator */
  nusmv_assert(!IS_FLAG_PREDICATES(leftResult) &&
               !IS_FLAG_PREDICATES(rightResult));

  result = Set_MakeEmpty();

  if (self->use_approx && 
      (size_t) Set_GiveCardinality(leftResult) * 
      (size_t) Set_GiveCardinality(rightResult) > OVER_APPROX_THRESHOLD) {
    /* too-big: gives up */    
    return PREDICATES_OVERAPPROX;
  }

  /* create Cartesian produce of predicate subparts and apply the
     operator to every pair */
  SET_FOREACH(leftResult, l_iter) {
    node_ptr l_expr = Set_GetMember(leftResult, l_iter);
    nusmv_assert(Nil != l_expr); /* expression is well-formed */

    Set_Iterator_t r_iter;
    SET_FOREACH(rightResult, r_iter) {
      node_ptr r_expr = Set_GetMember(rightResult, r_iter);
      nusmv_assert(Nil != r_expr); /* expression is well-formed */

      result = Set_AddMember(result, 
                             Expr_resolve(self->st, type, l_expr, r_expr));
    }
  }
  return result;
}
