/**CFile***********************************************************************

  FileName    [ClusterList.c]

  PackageName [trans.bdd]

  Synopsis    [Routines related to list of transition clusters. ]

  Description [ This file contains ClusterList class and modules related to
  it.]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans.bdd'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.

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

#include "ClusterList.h"
#include "transInt.h"
#include "enc/operators.h"
#include "utils/error.h"

#include "utils/heap.h"
#include <float.h>


static char rcsid[] UTIL_UNUSED = "$Id: ClusterList.c,v 1.1.2.7.4.2.6.4 2009-10-29 18:17:51 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define END_ITERATOR \
       CLUSTER_LIST_ITERATOR(Nil)  /* Represent the end of ClusterList iterator
                                    */

#define INVALID_ITERATOR \
       CLUSTER_LIST_ITERATOR(FAILURE_NODE) /* Represents Invalid iterator */

/**Macro***********************************************************************

  Synopsis [Number of allowed clusters whose BDD size is below the
            partitioning threshold while using affinity.]

  Description [This number specifies the number of clusters whose BDD
               size is below the partitioning threshold. If the number
               of clusters whose size is below the partitioning
               threshold exceeds this limit, then clustering via
               affinity is not performed (too expensive) and "simple"
               clustering is performed. With this value the initial
               size of the heap used by the clustering via affinity is
               100!/(2*(100-2)!) = 4950, i.e. the combination of N=100
               elements in pair: C(N,2). Allowing larger numbers is
               possible, but can lead to enourmous consumption of
               memory.]

  SeeAlso [ClusterList_apply_threshold, cluster_list_apply_threshold_affinity]

******************************************************************************/
#define CLUSTER_LIST_SIZE_INHIBIT_AFFINITY 100

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [ClusterList Class.]

  Description [ This class forms a list of clusters.]

  SeeAlso     []

******************************************************************************/
typedef struct ClusterList_TAG
{
  ClusterListIterator_ptr first;
  ClusterListIterator_ptr last;  /* to optimize appending ops */

  DdManager* dd;

} ClusterList;

/**Struct**********************************************************************

  Synopsis    [record of the list to be used with the heap]

  Description [This is the record of the list to be used with the heap, basic
  blocks of optimized affinity clustering.]

  SeeAlso     []

******************************************************************************/
typedef struct af_support_list_entry_TAG
{
  boolean     exists;  /* tells if the cluster exists */
  Cluster_ptr cluster; /* pointer to the element (cluster) of the original
                          cluster list */
  boolean owns_cluster; /* tells if the cluster must be freed when
                           the structure gets destroyed */
} af_support_list_entry;


/* ASLE stays here for Affinity Support List Entry */
#define ASLE_exists(asle)                  ((asle)->exists)
#define ASLE_set_exists(asle, f)           ((asle)->exists = f)
#define ASLE_get_cluster(asle)             ((asle)->cluster)
#define ASLE_set_cluster(asle, cl, owns_it) \
  {(asle)->cluster = cl; (asle)->owns_cluster = owns_it;}
#define ASLE_owns_cluster(asle)            ((asle)->owns_cluster)

/**Struct**********************************************************************

  Synopsis    [pair of pointers to the list.]

  Description [This is the record of the heap that point to the support list.]

  SeeAlso     []

******************************************************************************/
typedef struct af_support_pair_struct {
  af_support_list_entry* c1;
  af_support_list_entry* c2;
} af_support_pair;


#define ASPair_get_c1(p)     ((p)->c1)
#define ASPair_set_c1(p, c)  ((p)->c1 = c)
#define ASPair_get_c2(p)     ((p)->c2)
#define ASPair_set_c2(p, c)  ((p)->c2 = c)



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static bdd_ptr
cluster_list_get_image
ARGS( (const ClusterList_ptr self, bdd_ptr s,
       bdd_ptr (*cluster_getter)(const Cluster_ptr cluster)) );

static bdd_ptr
cluster_list_get_k_image
ARGS( (const ClusterList_ptr self, bdd_ptr s, int k,
       bdd_ptr (*cluster_getter)(const Cluster_ptr cluster)) );


static ClusterList_ptr
cluster_list_iwls95_order ARGS((const ClusterList_ptr self,
                                bdd_ptr state_vars_cube,
                                bdd_ptr input_vars_cube,
                                bdd_ptr next_state_vars_cube,
                                const ClusterOptions_ptr cl_options));

static ClusterList_ptr
cluster_list_apply_iwls95_info ARGS((const ClusterList_ptr self,
                                     bdd_ptr state_vars_cube,
                                     bdd_ptr input_vars_cube,
                                     bdd_ptr next_state_vars_cube,
                                     const ClusterOptions_ptr cl_options));

static bdd_ptr
cluster_list_get_supp_Q_Ci ARGS((const ClusterList_ptr self,
                                 const Cluster_ptr Ci));

static void
clusterlist_build_schedule_recur ARGS((ClusterList_ptr self,
                                       const ClusterListIterator_ptr iter,
                                       const bdd_ptr s_cube,
                                       const bdd_ptr si_cube,
                                       bdd_ptr* acc_s, bdd_ptr* acc_si));

static void
cluster_list_destroy_weak ARGS((ClusterList_ptr self));

static ClusterList_ptr
cluster_list_copy ARGS((const ClusterList_ptr self, const boolean weak_copy));


static ClusterList_ptr
cluster_list_apply_threshold ARGS((const ClusterList_ptr self,
                                   const int threshold,
                                   const boolean append));

static ClusterList_ptr
cluster_list_apply_threshold_affinity ARGS((const ClusterList_ptr self,
                                            const int threshold,
                                            const boolean append));


static int
clusterlist_affinity_move_clusters ARGS((const ClusterList_ptr self,
                                         ClusterList_ptr new_list,
                                         const int threshold,
                                         const boolean append,
                                         node_ptr* list_ref, heap _heap));


/* ---------------------------------------------------------------------- */
/* For affinity support:                                                  */
/* ---------------------------------------------------------------------- */
static af_support_list_entry* support_list_entry_create ARGS((void));

static void support_list_del ARGS((af_support_list_entry* asle, 
                                   DdManager* dd));

static af_support_pair* af_support_pair_create ARGS((void));

static node_ptr
support_list_heap_add ARGS((node_ptr list, heap _heap, DdManager* dd,
                            Cluster_ptr cluster, boolean owns_cluster));


static double compute_bdd_affinity ARGS((DdManager* dd,
                                         bdd_ptr a, bdd_ptr b));


/* ---------------------------------------------------------------------- */
/*                            Public methods                              */
/* ---------------------------------------------------------------------- */


/**Function********************************************************************

  Synopsis           [ Class ClusterList Constructor. ]

  Description        [ The reference to DdManager passed here is internally
  stored but self does not become owner of it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterList_ptr ClusterList_create(DdManager* dd)
{
  ClusterList_ptr self = ALLOC(ClusterList, 1);
  CLUSTER_LIST_CHECK_INSTANCE(self);

  self->first = END_ITERATOR;
  self->last  = END_ITERATOR;

  self->dd = dd;
  return self;
}


/**Function********************************************************************

  Synopsis           [ ClusterList Class dectructor.]

  Description        [ Destroys the cluster list and all cluster instances
  inside it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ClusterList_destroy(ClusterList_ptr self)
{
  /* This function currently iterates twice on the list, but it can be
     implemented in such a way that only one iteration is performed */
  ClusterListIterator_ptr iter;

  CLUSTER_LIST_CHECK_INSTANCE(self);

  iter = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(iter) ) {
    Cluster_ptr cluster = ClusterList_get_cluster(self, iter);
    Object_destroy(OBJECT(cluster), self->dd);

    iter = ClusterListIterator_next(iter);
  }

  cluster_list_destroy_weak(self);
}



/**Function********************************************************************

  Synopsis           [Returns a copy of the "self".]

  Description        [Duplicates self and each cluster inside it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterList_ptr ClusterList_copy(const ClusterList_ptr self)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);
  return cluster_list_copy(self, false);
}



/**Function********************************************************************

  Synopsis           [ Reverses the list of clusters. ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ClusterList_reverse(ClusterList_ptr self)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);

  self->last = self->first;
  self->first = CLUSTER_LIST_ITERATOR( reverse((node_ptr) self->first) );
}


/**Function********************************************************************

  Synopsis           [Deletes every occurrence of the given cluster from the
  self.]

  Description        [Returns the number of removed occurrences. Clusters found
  won't be destroyed, simply their references will be removed from the list]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int ClusterList_remove_cluster(ClusterList_ptr self, Cluster_ptr cluster)
{
  /* This function can be improved by optimizing the search of the
     element within the list */
  int count = 0;
  ClusterListIterator_ptr prev = INVALID_ITERATOR;
  ClusterListIterator_ptr curr;
  ClusterListIterator_ptr next = END_ITERATOR;

  CLUSTER_LIST_CHECK_INSTANCE(self);
  CLUSTER_CHECK_INSTANCE(cluster);

  curr = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(curr) ) {
    Cluster_ptr elem = ClusterList_get_cluster(self, curr);
    next = ClusterListIterator_next(curr);

    if ( Cluster_is_equal(elem, cluster) ) {
      /* Removes this cluster from the list */
      if (prev != INVALID_ITERATOR) {
        setcdr((node_ptr) prev, (node_ptr) next);
      }
      else {
        /* first element */
        self->first = next;
      }

      /* adjusts the last member consistance if it is the case */
      if (self->last == curr) {
        if (prev != INVALID_ITERATOR)  self->last = prev;
        else self->last = END_ITERATOR;
      }

      free_node( (node_ptr) curr );
      ++count;
    }

    prev = curr;
    curr = next;
  }

  return count;
}


/**Function********************************************************************

  Synopsis           [ It returns a monolithic transition cluster corresponding
  to the cluster list of the "self".]

  Description        ["self" remains unchanged. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterList_ptr ClusterList_apply_monolithic(const ClusterList_ptr self)
{
  ClusterList_ptr res;
  Cluster_ptr cluster;
  bdd_ptr mono;

  CLUSTER_LIST_CHECK_INSTANCE(self);

  mono = ClusterList_get_monolithic_bdd(self);
  cluster = Cluster_create(self->dd);
  Cluster_set_trans(cluster, self->dd, mono);
  bdd_free(self->dd, mono);

  res = ClusterList_create(self->dd);
  ClusterList_prepend_cluster(res, cluster); /* res becomes the owner */
  return res;
}


/**Function********************************************************************

  Synopsis           [It returns a threshold based cluster list corresponding
  to the cluster list of the "self".]

  Description        ["self" remains unchanged.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterList_ptr ClusterList_apply_threshold(const ClusterList_ptr self,
                                            const ClusterOptions_ptr cl_options)
{
  ClusterList_ptr res;

  CLUSTER_LIST_CHECK_INSTANCE(self);
  CLUSTER_OPTIONS_CHECK_INSTANCE(cl_options);

  if ((CLUSTER_LIST_SIZE_INHIBIT_AFFINITY >= ClusterList_length(self)) &&
      ClusterOptions_is_affinity(cl_options)) {
    res = cluster_list_apply_threshold_affinity(self,
                            ClusterOptions_get_threshold(cl_options),
                            ClusterOptions_clusters_appended(cl_options));
  }
  else {
    res = cluster_list_apply_threshold(self,
                            ClusterOptions_get_threshold(cl_options),
                            ClusterOptions_clusters_appended(cl_options));
  }

  return res;
}



/**Function********************************************************************

  Synopsis           [ Returns the number of the clusters stored in "self".]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int ClusterList_length(const ClusterList_ptr self)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);

  return llength((node_ptr) self->first);
}


/**Function********************************************************************

  Synopsis           [Prepends given cluster to the list]

  Description        [List becomes the owner of the given cluster]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ClusterList_prepend_cluster(ClusterList_ptr self, Cluster_ptr cluster)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);
  CLUSTER_CHECK_INSTANCE(cluster);

  self->first =  CLUSTER_LIST_ITERATOR( cons((node_ptr) cluster,
                                             (node_ptr) self->first) );

  if (self->last == END_ITERATOR) self->last = self->first;
}


/**Function********************************************************************

  Synopsis           [Appends given cluster to the list]

  Description        [List becomes the owner of the given cluster, if the user
  is going to call standard destructor]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void  ClusterList_append_cluster(ClusterList_ptr self, Cluster_ptr cluster)
{
  node_ptr new;

  CLUSTER_LIST_CHECK_INSTANCE(self);
  CLUSTER_CHECK_INSTANCE(cluster);

  new = cons((node_ptr) cluster, (node_ptr) END_ITERATOR);

  if (self->last != END_ITERATOR) {
    setcdr((node_ptr) self->last, new);
  }

  self->last = new;

  if (self->first == END_ITERATOR) {
    self->first = self->last;
  }
}


/**Function********************************************************************

  Synopsis           [ Returns an Iterator to iterate the self.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterListIterator_ptr ClusterList_begin(const ClusterList_ptr self)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);
  return self->first;
}


/**Function********************************************************************

  Synopsis           [ Returns the cluster kept at the position given by the
  iterator]

  Description        [self keeps the ownership of the returned cluster]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Cluster_ptr ClusterList_get_cluster(const ClusterList_ptr self,
                                    const ClusterListIterator_ptr iter)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);
  nusmv_assert(iter != END_ITERATOR);

  return CLUSTER( car((node_ptr) iter) );
}


/**Function********************************************************************

  Synopsis           [ Sets the cluster of the "self" at the position given by
  iterator "iter" to cluster "cluster".]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ClusterList_set_cluster(ClusterList_ptr self,
                             const ClusterListIterator_ptr iter,
                             Cluster_ptr cluster)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);
  CLUSTER_CHECK_INSTANCE(cluster);
  nusmv_assert(iter != END_ITERATOR);

  setcar((node_ptr) iter, (node_ptr) cluster);
}


/**Function********************************************************************

  Synopsis           [It builds the quantification schedule of the variables
  inside the clusters of the "self".]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ClusterList_build_schedule(ClusterList_ptr self,
                                bdd_ptr state_vars_cube,
                                bdd_ptr input_vars_cube)
{
  ClusterListIterator_ptr iter;
  bdd_ptr acc_s, acc_si, s_cube, si_cube;

  CLUSTER_LIST_CHECK_INSTANCE(self);

  iter = ClusterList_begin(self);
  si_cube = bdd_and(self->dd, input_vars_cube, state_vars_cube);
  s_cube  = bdd_dup(state_vars_cube);

  clusterlist_build_schedule_recur(self, iter, s_cube, si_cube, &acc_s, &acc_si);

  bdd_free(self->dd, acc_s);
  bdd_free(self->dd, acc_si);
  bdd_free(self->dd, s_cube);
  bdd_free(self->dd, si_cube);
  return;
}



/**Function********************************************************************

  Synopsis           [ Computes the image of the given bdd "s" using the
  clusters of the "self" while quantifying state vars only.]

  Description        [Returned bdd is referenced]]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr ClusterList_get_image_state(const ClusterList_ptr self, bdd_ptr s)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);
  return cluster_list_get_image(self, s,
                                &Cluster_get_quantification_state_input);
}


/**Function********************************************************************

  Synopsis           [Computes the image of the given bdd "s" using the
  clusters of the "self" while quantifying both state and input vars.]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr
ClusterList_get_image_state_input(const ClusterList_ptr self, bdd_ptr s)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);
  return cluster_list_get_image(self, s, &Cluster_get_quantification_state);
}


/**Function********************************************************************

  Synopsis           [ Computes the k image of the given bdd "s" using the
  clusters of the "self" while quantifying state vars only.]

  Description        [Returned bdd is referenced]]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr ClusterList_get_k_image_state(const ClusterList_ptr self, bdd_ptr s, int k)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);
  return cluster_list_get_k_image(self, s, k,
                                  &Cluster_get_quantification_state_input);
}


/**Function********************************************************************

  Synopsis           [Computes the k image of the given bdd "s" using the
  clusters of the "self" while quantifying both state and input vars.]

  Description        [Returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr
ClusterList_get_k_image_state_input(const ClusterList_ptr self, bdd_ptr s, int k)
{
  CLUSTER_LIST_CHECK_INSTANCE(self);
  return cluster_list_get_k_image(self, s, k, &Cluster_get_quantification_state);
}


/**Function********************************************************************

  Synopsis           [Returns the monolithic bdd corresponding to the "self".]

  Description        [The returned bdd is referenced]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr ClusterList_get_monolithic_bdd(const ClusterList_ptr self)
{
  ClusterListIterator_ptr iter;
  bdd_ptr result;

  CLUSTER_LIST_CHECK_INSTANCE(self);

  result = bdd_true(self->dd);

  iter = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(iter) ) {

    Cluster_ptr cluster = ClusterList_get_cluster(self, iter);
    bdd_ptr tmp = Cluster_get_trans(cluster);

    bdd_and_accumulate(self->dd, &result, tmp);
    bdd_free(self->dd, tmp);
    iter = ClusterListIterator_next(iter);
  }

  return result;
}

/**Function********************************************************************

  Synopsis [Computes the cube of the set of support of all the clusters]

  Description [Given a list of clusters, it computes their set of support.
  Returned bdd is referenced.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
bdd_ptr ClusterList_get_clusters_cube(const ClusterList_ptr self)
{
  ClusterListIterator_ptr iter;
  bdd_ptr result;

  CLUSTER_LIST_CHECK_INSTANCE(self);

  result = bdd_true(self->dd);

  iter = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(iter) ) {
    Cluster_ptr cluster = ClusterList_get_cluster(self, iter);

    bdd_ptr ti = Cluster_get_trans(cluster);
    bdd_ptr supp = bdd_support(self->dd, ti);
    bdd_and_accumulate(self->dd, &result, supp);

    bdd_free(self->dd, ti);
    bdd_free(self->dd, supp);

    iter = ClusterListIterator_next(iter);
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Orders the clusters according to the IWLS95 algo. to
  perform image computation.]

  Description        [This function builds the
  data structures to perform image computation. <br>
  This process consists of the following steps:<br>
  <ol>
  <li> Ordering of the clusters given as input accordingly with the
       heuristic described in IWLS95.</li>
  <li> Clustering of the result of previous step accordingly the
       threshold value stored in the option \"image_cluster_size\".</li>
  <li> Ordering of the result of previous step accordingly with the
       heuristic described in IWLS95.</li>
  </ol>]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterList_ptr
ClusterList_apply_iwls95_partition(const ClusterList_ptr self,
                                   bdd_ptr state_vars_cube,
                                   bdd_ptr input_vars_cube,
                                   bdd_ptr next_state_vars_cube,
                                   const ClusterOptions_ptr cl_options)
{
  ClusterList_ptr source = self;
  ClusterList_ptr with_threshold;
  ClusterList_ptr result;

  CLUSTER_LIST_CHECK_INSTANCE(self);
  CLUSTER_OPTIONS_CHECK_INSTANCE(cl_options);

  if (ClusterOptions_is_iwls95_preorder(cl_options)) {

    /* (pre)Ordering , clustering, reordering */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "...Performing clusters preordering...");
    }
    source = cluster_list_iwls95_order(self,
                                       state_vars_cube,
                                       input_vars_cube,
                                       next_state_vars_cube,
                                       cl_options);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "...done\n");
    }
  } /* preordering */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Applying threshold to clusters...");
  }

  if ((CLUSTER_LIST_SIZE_INHIBIT_AFFINITY >= ClusterList_length(self)) &&
      ClusterOptions_is_affinity(cl_options)) {
    with_threshold = cluster_list_apply_threshold_affinity(source,
                           ClusterOptions_get_cluster_size(cl_options),
                           ClusterOptions_clusters_appended(cl_options));
  }
  else {
    with_threshold = cluster_list_apply_threshold(source,
                                  ClusterOptions_get_cluster_size(cl_options),
                                  ClusterOptions_clusters_appended(cl_options));
  }

  if (self != source)  ClusterList_destroy(source);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "done\nOrdering clusters...");
  }
  result = cluster_list_iwls95_order(with_threshold,
                                     state_vars_cube,
                                     input_vars_cube,
                                     next_state_vars_cube,
                                     cl_options);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "...done\n");
  }

  ClusterList_destroy(with_threshold);
  return result;
}


/**Function********************************************************************

  Synopsis    [Performs the synchronous product between two cluster lists]

  Description [All clusters into other are simply appended to "self".
  The result goes into "self", no changes on other. The scheduling
  is done with the variables from both cluster lists.
  Precondition: both lists should have scheduling done.]

  SideEffects [self will change]

******************************************************************************/
void ClusterList_apply_synchronous_product(ClusterList_ptr self,
                                           const ClusterList_ptr other)
{
  ClusterListIterator_ptr iter;
  Cluster_ptr cluster;
  bdd_ptr state_vars_cube = bdd_true(self->dd);
  bdd_ptr input_vars_cube = bdd_true(self->dd);
  bdd_ptr tmp;

  CLUSTER_LIST_CHECK_INSTANCE(self);

  /* collect variable cubes */
  for (iter = ClusterList_begin(self);
       ! ClusterListIterator_is_end(iter);
       iter = ClusterListIterator_next(iter)) {
    cluster = ClusterList_get_cluster(self, iter);

    tmp = Cluster_get_quantification_state(cluster);
    nusmv_assert(NULL != tmp);
    bdd_and_accumulate(self->dd, &state_vars_cube, tmp);
    bdd_free(self->dd, tmp);

    tmp = Cluster_get_quantification_state_input(cluster);
    nusmv_assert(NULL != tmp);
    bdd_and_accumulate(self->dd, &input_vars_cube, tmp);
    bdd_free(self->dd, tmp);
  }
 
  for (iter = ClusterList_begin(other);
       ! ClusterListIterator_is_end(iter);
       iter = ClusterListIterator_next(iter)) {
    cluster = ClusterList_get_cluster(other, iter);

    tmp = Cluster_get_quantification_state(cluster);
    nusmv_assert(NULL != tmp);
    bdd_and_accumulate(self->dd, &state_vars_cube, tmp);
    bdd_free(self->dd, tmp);

    tmp = Cluster_get_quantification_state_input(cluster);
    nusmv_assert(NULL != tmp);
    bdd_and_accumulate(self->dd, &input_vars_cube, tmp);
    bdd_free(self->dd, tmp);
  }

  /* get pure input var cube without states */
  tmp = bdd_cube_diff(self->dd, input_vars_cube, state_vars_cube);
  bdd_free(self->dd, input_vars_cube);
  input_vars_cube = tmp;
       
  /* appends a copy of 'other' to self */
  for (iter = ClusterList_begin(other);
       ! ClusterListIterator_is_end(iter);
       iter = ClusterListIterator_next(iter)) {
    cluster = ClusterList_get_cluster(other, iter);
    ClusterList_append_cluster(self, CLUSTER( Object_copy(OBJECT(cluster)) ));
  } /* loop on clusters */

  /* ClusterList_apply_threshold is probably useless, but
     ClusterList_apply_monolithic may make a different.
   */

  ClusterList_build_schedule(self, state_vars_cube, input_vars_cube);
}



/**Function********************************************************************

  Synopsis           [Prints size of each cluster of the "self"]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ClusterList_print_short_info(const ClusterList_ptr self, FILE* file)
{
  ClusterListIterator_ptr iter;
  int i = 0;
  CLUSTER_LIST_CHECK_INSTANCE(self);

  iter = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(iter) ) {
    Cluster_ptr cluster = ClusterList_get_cluster(self, iter);

    bdd_ptr t = Cluster_get_trans(cluster);
    fprintf(file, "cluster %d\t:\tsize %d\n", ++i, bdd_size(self->dd, t));
    bdd_free(self->dd, t);

    iter = ClusterListIterator_next(iter);
  }
}


/**Function********************************************************************

  Synopsis           [Use to iterate a list]

  Description        [Advances the iterator by one.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ClusterListIterator_ptr
ClusterListIterator_next(const ClusterListIterator_ptr self)
{
  ClusterListIterator_ptr res = END_ITERATOR;

  if (self != END_ITERATOR) {
    res = CLUSTER_LIST_ITERATOR( cdr((node_ptr)self) );
  }
  return res;
}

/**Function********************************************************************

  Synopsis           [Use to check if iterator is at the end of list]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ClusterListIterator_is_end(const ClusterListIterator_ptr self)
{
  return self == END_ITERATOR;
}


/**Function********************************************************************

  Synopsis           [Returns true if two clusters list are logically equivalent]

  Description        [It compares BDDs not Clusters.]

  SideEffects        []

******************************************************************************/
boolean ClusterList_check_equality(const ClusterList_ptr self,
                                   const ClusterList_ptr other)
{
  boolean res;
  bdd_ptr acc1, acc2;

  /* The two DD managers must be the same */
  nusmv_assert((self->dd == other->dd));

  CLUSTER_LIST_CHECK_INSTANCE(self);
  CLUSTER_LIST_CHECK_INSTANCE(other);

  acc1 = ClusterList_get_monolithic_bdd(self);
  acc2 = ClusterList_get_monolithic_bdd(other);

  res = (acc1 == acc2);

  bdd_free(self->dd, acc1);
  bdd_free(self->dd, acc2);

  return res;
}



/**Function********************************************************************

  Synopsis           [Check the schedule for self. Call after you applied the
  schedule]

  Description        [Let Ci and Ti be the ith cube and relation in the list.
  The schedule is correct iff<br>
  <ol>
  <li> For all Tj: j > i, S(Tj) and S(Ci) do not intersect, i.e., the
  variables which are quantified in Ci should not appear in the
  Tj for j>i.</li>
  </ol><br>

  where S(T) is the set of support of the BDD T.
  Returns true if the schedule is correct, false otherwise.
  This function is implemented for checking the correctness of the
  clustering algorithm only.<br>
  This function returns true if schedule is correct, false otherwise.]

  SideEffects        []

******************************************************************************/
boolean ClusterList_check_schedule(const ClusterList_ptr self)
{
  ClusterListIterator_ptr iter_i;

  CLUSTER_LIST_CHECK_INSTANCE(self);

  iter_i = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(iter_i) ) {

    ClusterListIterator_ptr iter_j;
    Cluster_ptr ci = ClusterList_get_cluster(self, iter_i);

    bdd_ptr si_ci = Cluster_get_quantification_state_input(ci);
    bdd_ptr trans_ci = Cluster_get_trans(ci);
    bdd_ptr support_ci = bdd_support(self->dd, trans_ci);

    bdd_free(self->dd, trans_ci);

    iter_j = ClusterListIterator_next(iter_i);
    while ( ! ClusterListIterator_is_end(iter_j) ) {
      bdd_ptr intersect;
      bdd_ptr si_cj;

      Cluster_ptr cj = ClusterList_get_cluster(self, iter_j);

      si_cj = Cluster_get_quantification_state_input(cj);
      intersect = bdd_cube_diff(self->dd, si_cj, support_ci);

      if (intersect != si_cj) {
        /* Violates the condition "a" */
        bdd_free(self->dd, intersect);
        bdd_free(self->dd, si_cj);
        bdd_free(self->dd, si_ci);
        bdd_free(self->dd, support_ci);

        return false;
      }

      bdd_free(self->dd, intersect);
      bdd_free(self->dd, si_cj);

      iter_j = ClusterListIterator_next(iter_j);
    } /* loop on j */


    bdd_free(self->dd, si_ci);
    bdd_free(self->dd, support_ci);

    iter_i = ClusterListIterator_next(iter_i);
  } /* loop on i */

  return true;
}

/*---------------------------------------------------------------------------*/
/* Static functions                                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [ Computes the image from a given set of states
  "s".]

  Description        [ The parameters passed to this function includes pointer
  to "self", set of states "s", and a function pointer that retrives from any
  cluster in "self" a cube of variables for existential quantification. ]

  SideEffects        []

******************************************************************************/
static bdd_ptr
cluster_list_get_image(const ClusterList_ptr self,
                       bdd_ptr s,
                       bdd_ptr (*cluster_getter)(const Cluster_ptr cluster))
{
  ClusterListIterator_ptr iter;
  bdd_ptr cur_prod;
  long maxsize = 0;

  cur_prod = bdd_dup(s);

  iter = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(iter) ) {
    Cluster_ptr cluster = ClusterList_get_cluster(self, iter);

    bdd_ptr tmp = Cluster_get_trans(cluster);
    bdd_ptr ex  = cluster_getter(cluster);
    bdd_ptr new_p = bdd_and_abstract(self->dd, cur_prod, tmp, ex);
    bdd_free(self->dd, ex);
    bdd_free(self->dd, tmp);

    /* verbosity */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      long intermediateSize = bdd_size(self->dd, new_p);

      fprintf(nusmv_stdout,
              "          Size of intermediate product = %10ld (BDD nodes).\n",
              intermediateSize);
      if (maxsize < intermediateSize)  maxsize = intermediateSize;
    }

    bdd_free(self->dd, cur_prod);
    cur_prod = new_p;

    iter = ClusterListIterator_next(iter);
  } /* iteration */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stdout,
            "Max. BDD size for intermediate product = %10ld (BDD nodes)\n",
            maxsize);
  }

  return cur_prod;
}

/**Function********************************************************************

  Synopsis           [ Use to compute the k image from a given set of states
  "s".]

  Description        [ The parameters passed to this function includes pointer
  to "self", set of states "s", value "k", and a function pointer that retrives
  from any cluster in "self" a cube of variables for existential quantification. ]

  SideEffects        []

******************************************************************************/
static bdd_ptr
cluster_list_get_k_image(const ClusterList_ptr self,
                         bdd_ptr s, int k,
                         bdd_ptr (*cluster_getter)(const Cluster_ptr cluster))
{
  ClusterListIterator_ptr iter;
  add_ptr cur_prod;
  bdd_ptr result;
  long maxsize = 0;

  cur_prod = bdd_to_01_add(self->dd, s);

  iter = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(iter) ) {
    Cluster_ptr cluster = ClusterList_get_cluster(self, iter);

    bdd_ptr tmp, ex;
    add_ptr tmpa, tmpb;

    tmp = Cluster_get_trans(cluster);
    tmpa = bdd_to_01_add(self->dd, tmp);
    bdd_free(self->dd, tmp);

    tmpb = add_apply(self->dd, node_times, cur_prod, tmpa);
    add_free(self->dd, tmpa);
    add_free(self->dd, cur_prod);

    ex = cluster_getter(cluster);

    cur_prod = add_exist_abstract(self->dd, tmpb, ex);
    add_free(self->dd, tmpb);
    bdd_free(self->dd, ex);

    /* verbosity */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      long intermediateSize = add_size(self->dd, cur_prod);

      fprintf(nusmv_stdout,
              "          Size of intermediate product = %10ld (ADD nodes).\n",
              intermediateSize);
      if (maxsize < intermediateSize)  maxsize = intermediateSize;
    }

    iter = ClusterListIterator_next(iter);
  } /* iteration */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stdout,
            "Max. ADD size for intermediate product = %10ld (ADD nodes)\n",
            maxsize);
  }

  result = add_to_bdd_strict_threshold(self->dd,cur_prod,k-1);
  add_free(self->dd, cur_prod);

  return result;
}


/**Function********************************************************************

  Synopsis           [It orders a copy of the "self" according to the IWLS95
  algorithm and returns the copy.]

  Description        ["self" remains unchanged.]

  SideEffects        []

******************************************************************************/
static ClusterList_ptr
cluster_list_iwls95_order(const ClusterList_ptr self,
                          bdd_ptr state_vars_cube,
                          bdd_ptr input_vars_cube,
                          bdd_ptr next_state_vars_cube,
                          const ClusterOptions_ptr cl_options)
{
  ClusterList_ptr weak_copy;
  ClusterList_ptr result;
  ClusterListIterator_ptr iter;

  result = ClusterList_create(self->dd);

  /* Creates a weak copy, i.e. a copy that does not copy clusters
     stored inside it.  In this way we avoid to destroy clusters when
     the weak copy is destroyed: */
  weak_copy = cluster_list_copy(self, true);

  iter = ClusterList_begin(weak_copy);
  while ( ! ClusterListIterator_is_end(iter) ) {
    double best_benefit = (- DBL_MAX);
    ClusterIwls95_ptr best_cluster = CLUSTER_IWLS95(NULL);

    ClusterList_ptr clusters_iwls;
    ClusterListIterator_ptr iter_iwls;


    clusters_iwls =  cluster_list_apply_iwls95_info(weak_copy,
                                                    state_vars_cube,
                                                    input_vars_cube,
                                                    next_state_vars_cube,
                                                    cl_options);
    iter_iwls = ClusterList_begin(clusters_iwls);

    /* finds best_benefit and best_cluster */
    while ( ! ClusterListIterator_is_end(iter_iwls) ) {
      ClusterIwls95_ptr cluster_iwls;
      double benefit;

      cluster_iwls = CLUSTER_IWLS95(ClusterList_get_cluster(clusters_iwls,
                                                            iter_iwls));
      benefit = ClusterIwls95_get_benefit(cluster_iwls);

      if (benefit > best_benefit) {
        best_benefit = benefit;
        best_cluster = cluster_iwls;
      }

      iter_iwls = ClusterListIterator_next(iter_iwls);
    } /* loop for iter_iwls */

    /* best_cluster must be valid here: */
    CLUSTER_IWLS95_CHECK_INSTANCE(best_cluster);

    /* remove from the source list any similar clusters */
    ClusterList_remove_cluster(weak_copy, CLUSTER(best_cluster));

    /* since list clusters_iwls95 is going to be destroyed, and since
       it owns best_cluster, we must copy best_cluster */
    ClusterList_append_cluster(result,
                               CLUSTER( Object_copy(OBJECT(best_cluster)) ));

    ClusterList_destroy(clusters_iwls);

    /* we always stay on the first element since we are removing
       clusters from the list */
    iter = ClusterList_begin(weak_copy);
  } /* loop for iter */

  cluster_list_destroy_weak(weak_copy);
  return result;
}


/**Function********************************************************************

  Synopsis           [ It applies iwls95 info passed as parameters to a copy of
  the "self" and returns it.]

  Description        ["self" remains unchanged. ]

  SideEffects        []

******************************************************************************/
static ClusterList_ptr
cluster_list_apply_iwls95_info(const ClusterList_ptr self,
                               bdd_ptr state_vars_cube,
                               bdd_ptr input_vars_cube,
                               bdd_ptr next_state_vars_cube,
                               const ClusterOptions_ptr cl_options)
{
  ClusterList_ptr result;
  ClusterListIterator_ptr iter;
  bdd_ptr pspi;
  double x_c, z_c, max_c;

  result = ClusterList_create(self->dd);

  pspi = bdd_and(self->dd, state_vars_cube, input_vars_cube);

  {
    /* Calculates values x_c, z_c, max_c */
    bdd_ptr acc = ClusterList_get_clusters_cube(self);
    bdd_ptr acc_pspi = bdd_cube_intersection(self->dd, acc, pspi);
    bdd_ptr acc_next_state_vars = bdd_cube_intersection(self->dd, acc,
                                                        next_state_vars_cube);

    x_c   = (double) bdd_size(self->dd, acc_pspi);
    z_c   = (double) bdd_size(self->dd, acc_next_state_vars);
    max_c = (double) bdd_get_lowest_index(self->dd, acc_pspi);

    bdd_free(self->dd, acc);
    bdd_free(self->dd, acc_pspi);
    bdd_free(self->dd, acc_next_state_vars);
  }


  iter = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(iter) ) {

    ClusterIwls95_ptr cluster_iwls;
    Cluster_ptr cluster = ClusterList_get_cluster(self, iter);

    bdd_ptr tmp;
    bdd_ptr ti          = Cluster_get_trans(cluster);
    bdd_ptr sti         = bdd_support(self->dd, ti);
    bdd_ptr sti_pspi    = bdd_cube_intersection(self->dd, sti, pspi);
    bdd_ptr sti_ns      = bdd_cube_intersection(self->dd,
                                                sti, next_state_vars_cube);

    bdd_ptr supp_Q_Ci = cluster_list_get_supp_Q_Ci(self, cluster);
    bdd_ptr state_input = bdd_cube_diff(self->dd, sti_pspi, supp_Q_Ci);

    /* constructor limits values to be greater or equal to zero: */
    cluster_iwls = ClusterIwls95_create( self->dd,
                                         cl_options,
                                         bdd_size(self->dd, state_input) - 1,
                                         bdd_size(self->dd, sti_pspi) - 1,
                                         x_c,
                                         bdd_size(self->dd, sti_ns) - 1,
                                         z_c,
                                         bdd_get_lowest_index(self->dd, sti_pspi),
                                         max_c );

    /* -------------------------------------------------------------------- */
    /* updates the iwls95 cluster's members:                                */
    Cluster_set_trans(CLUSTER(cluster_iwls), self->dd, ti);

    tmp = Cluster_get_quantification_state(cluster);
    Cluster_set_quantification_state(CLUSTER(cluster_iwls), self->dd, tmp);
    bdd_free(self->dd, tmp);

    Cluster_set_quantification_state_input(CLUSTER(cluster_iwls), self->dd,
                                           state_input);
    /* -------------------------------------------------------------------- */

    ClusterList_append_cluster(result, CLUSTER(cluster_iwls));

    bdd_free(self->dd, state_input);
    bdd_free(self->dd, supp_Q_Ci);
    bdd_free(self->dd, sti_ns);
    bdd_free(self->dd, sti_pspi);
    bdd_free(self->dd, sti);
    bdd_free(self->dd, ti);

    iter = ClusterListIterator_next(iter);
  }

  bdd_free(self->dd, pspi);
  return result;
}



/**Function********************************************************************

  Synopsis           [Computes the set Supp_Q_Ci.]

  Description        [Computes the set of present an primary input variables
  that belong to the set of support of cluster Ci, and do not belong to the
  set of support of each cluster Cj, for j != i and Cj belonging to the set
  of the not yet ordered clusters. The set Supp_Q_Ci is formally defined as:
  Supp_Q_Ci = {v \in (PS U PI) /\ v \not\in S(T_Cj), Cj != Ci, Cj \in Q}]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static bdd_ptr cluster_list_get_supp_Q_Ci(const ClusterList_ptr self,
                                          const Cluster_ptr Ci)
{
  bdd_ptr result = bdd_true(self->dd);
  ClusterListIterator_ptr iter = ClusterList_begin(self);

  while ( ! ClusterListIterator_is_end(iter) ) {
    Cluster_ptr Cj = ClusterList_get_cluster(self, iter);

    if (! Cluster_is_equal(Cj, Ci)) {
      bdd_ptr tmp = Cluster_get_trans(Cj);
      bdd_ptr supp = bdd_support(self->dd, tmp);
      bdd_free(self->dd, tmp);

      bdd_and_accumulate(self->dd, &result, supp);

      bdd_free(self->dd, supp);
    }

    iter = ClusterListIterator_next(iter);
  }

  return result;
}

/**Function********************************************************************

  Synopsis           [ Helps to compute the quantification schedule]

  Description        [ Auxiliary recursive private function that computes the
  quantification schedule. The <tt>acc_s</tt> and <tt>acc_si</tt> must be freed
  by the caller.]

  SideEffects        [<tt>acc_s</tt> and <tt>acc_si</tt> are modified
  and must be freed by the caller.]

  SeeAlso            []

******************************************************************************/
static void
clusterlist_build_schedule_recur(ClusterList_ptr self,
                                 const ClusterListIterator_ptr iter,
                                 const bdd_ptr s_cube, const bdd_ptr si_cube,
                                 bdd_ptr* acc_s, bdd_ptr* acc_si)
{
  if (ClusterListIterator_is_end(iter)) {
    *acc_s = bdd_true(self->dd);
    *acc_si = bdd_true(self->dd);
  }
  else {
    Cluster_ptr C;
    bdd_ptr si, s, T, supp;
    bdd_ptr tacc_s, tacc_si;

    clusterlist_build_schedule_recur(self, ClusterListIterator_next(iter),
                                     s_cube, si_cube,
                                     &tacc_s, &tacc_si);
    /* computing the variables to quantify out */
    s  = bdd_cube_diff(self->dd, s_cube , tacc_s);
    si = bdd_cube_diff(self->dd, si_cube, tacc_si);

    C = ClusterList_get_cluster(self, iter);

    /* Storing the quantification variables in the cluster */
    Cluster_set_quantification_state_input(C, self->dd, si);
    Cluster_set_quantification_state(C, self->dd, s);

    /* computing the variables to send to upper levels */
    T = Cluster_get_trans(C);
    supp = bdd_support(self->dd, T);

    *acc_s = bdd_and(self->dd, tacc_s , supp);
    *acc_si = bdd_and(self->dd, tacc_si, supp);

    bdd_free(self->dd, supp);
    bdd_free(self->dd, T);
    bdd_free(self->dd, si);
    bdd_free(self->dd, s);
    bdd_free(self->dd, tacc_s);
    bdd_free(self->dd, tacc_si);
  }
  return;
}

/**Function********************************************************************

  Synopsis           [private function to weakly destroy the "self" ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void cluster_list_destroy_weak(ClusterList_ptr self)
{
  free_list( (node_ptr) self->first );
  self->first = END_ITERATOR;
  self->last  = END_ITERATOR;

  FREE(self);
}

/**Function********************************************************************

  Synopsis           [Dups a given list of clusters, copying clusters
  depending on the value in weak_copy]

  Description        [If weak_copy is true (internal use only) copied list must
  be destroyed by calling the weak private destructor
  cluster_list_destroy_weak]

  SideEffects        []

  SeeAlso            [cluster_list_destroy_weak]

******************************************************************************/
static ClusterList_ptr cluster_list_copy(const ClusterList_ptr self,
                                         const boolean weak_copy)
{
  ClusterList_ptr copy;
  ClusterListIterator_ptr iter;

  copy = ClusterList_create(self->dd);

  iter = ClusterList_begin(self);
  while ( ! ClusterListIterator_is_end(iter) ) {
    Cluster_ptr cluster = ClusterList_get_cluster(self, iter);
    Cluster_ptr cluster_copy;
    if (weak_copy)  cluster_copy = cluster;
    else            cluster_copy = CLUSTER(Object_copy( OBJECT(cluster) ));

    ClusterList_append_cluster(copy, cluster_copy);

    iter = ClusterListIterator_next(iter);
  }

  return copy;
}


/**Function********************************************************************

  Synopsis           [Forms the clusters of relations based on BDD
  size heuristic]

  Description        [The clusters are formed by taking the product in order.
  Once the BDD size of the current cluster reaches a threshold, a new cluster
  is created. It takes the value of "threshold" as parameter and returns the
  cluster of relation based on BDD size heuristic.]

  SideEffects        []

******************************************************************************/
static ClusterList_ptr
cluster_list_apply_threshold(const ClusterList_ptr self, const int threshold,
                             const boolean append)
{
  ClusterListIterator_ptr iter;
  bdd_ptr bdd_cluster;
  ClusterList_ptr result;
  boolean is_first_cluster, is_last_cluster;

  CLUSTER_LIST_CHECK_INSTANCE(self);
  nusmv_assert(threshold >= 0);


  result = ClusterList_create(self->dd);
  bdd_cluster = bdd_true(self->dd);

  is_first_cluster = true;

  iter = ClusterList_begin(self);
  is_last_cluster = ClusterListIterator_is_end(iter);
  while (!is_last_cluster) {
    boolean can_accumulate;
    Cluster_ptr cluster  = ClusterList_get_cluster(self, iter);
    bdd_ptr bdd_relation = Cluster_get_trans(cluster);

    can_accumulate = is_first_cluster ||
      ( (bdd_size(self->dd, bdd_cluster)  <= threshold) &&
        (bdd_size(self->dd, bdd_relation) <= threshold) );

    if (can_accumulate) {
      bdd_and_accumulate(self->dd, &bdd_cluster, bdd_relation);
      is_first_cluster = false;
      iter = ClusterListIterator_next(iter);
      is_last_cluster = ClusterListIterator_is_end(iter);
    }
    else {
      Cluster_ptr new_cluster = Cluster_create(self->dd);
      Cluster_set_trans(new_cluster, self->dd, bdd_cluster);

      /* result becomes the owner of new_cluster, so we do not destroy it */
      if (append) ClusterList_append_cluster(result, new_cluster);
      else ClusterList_prepend_cluster(result, new_cluster);

      bdd_free(self->dd, bdd_cluster);
      bdd_cluster = bdd_true(self->dd);
      is_first_cluster = true;
    }
    if (is_last_cluster) {
      /* collect the clusters set */
      Cluster_ptr new_cluster = Cluster_create(self->dd);
      Cluster_set_trans(new_cluster, self->dd, bdd_cluster);

      /* result becomes the owner of new_cluster, so we do not destroy it */
      if (append) ClusterList_append_cluster(result, new_cluster);
      else ClusterList_prepend_cluster(result, new_cluster);
    }
    bdd_free(self->dd, bdd_relation);
  }
  bdd_free(self->dd, bdd_cluster);
  return result;
}


/**Function********************************************************************

  Synopsis           [OPTIMIZED affinity clustering]

  Description        [This function aggregate clusters conjoining
  clusters that have highest affinity measure until they exceeds the
  specified threshold. <br>

  <b>Remark:</b> The number of clusters in self whose BDD size is
  below the threshold has a drammatic impact on the performance of
  this function. Indeed, the size of the heap used to order pair of
  clusters w.r.t. their affinity measure is proportional to the
  combination of N elements of class 2: C(N,K) = N!*(K!*(N-K)!).]

  SideEffects        []

******************************************************************************/
static ClusterList_ptr
cluster_list_apply_threshold_affinity(const ClusterList_ptr self,
                                      const int threshold,
                                      const boolean append)
{
  ClusterList_ptr result;
  af_support_pair* pair;
  node_ptr list = Nil;
  heap _heap;
  int n;

  CLUSTER_LIST_CHECK_INSTANCE(self);

  result = ClusterList_create(self->dd);
  _heap = heap_create();

  n = clusterlist_affinity_move_clusters(self, result, threshold,
                                         append, &list, _heap);

  /* clustering */
  while (n > 1) {

    pair = (af_support_pair*) heap_getmax(_heap);

    if ( ASLE_exists(ASPair_get_c1(pair)) &&
         ASLE_exists(ASPair_get_c2(pair)) ) {
      bdd_ptr t1, t2, t12;
      Cluster_ptr new_cluster;

      t1  = Cluster_get_trans( ASLE_get_cluster(ASPair_get_c1(pair)) );
      t2  = Cluster_get_trans( ASLE_get_cluster(ASPair_get_c2(pair)) );
      t12 = bdd_and(self->dd, t1, t2);

      bdd_free(self->dd, t2);
      bdd_free(self->dd, t1);
      support_list_del(ASPair_get_c1(pair), self->dd);
      support_list_del(ASPair_get_c2(pair), self->dd);

      new_cluster = Cluster_create(self->dd);
      Cluster_set_trans(new_cluster, self->dd, t12);

      if (bdd_size(self->dd, t12) > threshold) {

        /* result becomes the owner of new_cluster, so we do not destroy it */
        if (append) ClusterList_append_cluster(result, new_cluster);
        else ClusterList_prepend_cluster(result, new_cluster);

        n = n - 2;
      }
      else {
        list = support_list_heap_add(list, _heap, self->dd, new_cluster, 
                                     true /* owns the cluster */);
        --n;
      }

      bdd_free(self->dd, t12);
    }

    FREE(pair); /* the pair was removed from the heap, it is no longer used */
  } /* while loop */

  /* there can be a last one small cluster */
  if (n == 1) {
    node_ptr iter = list;

    while ( (iter != Nil) &&
            (! ASLE_exists((af_support_list_entry*) car(iter))) ) {
      iter = cdr(iter);
    }

    if (iter != Nil) {
      bdd_ptr t;
      Cluster_ptr cluster;

      cluster = Cluster_create(self->dd);
      t = Cluster_get_trans(ASLE_get_cluster((af_support_list_entry*)
                                             car(iter)));
      Cluster_set_trans(cluster, self->dd, t);
      bdd_free(self->dd, t);

      /* result becomes the owner of new_cluster, so we do not destroy it */
      if (append) ClusterList_append_cluster(result, cluster);
      else ClusterList_prepend_cluster(result, cluster);
    }
    else {
      fprintf(nusmv_stdout,"Affinity Optimized Inconsistency!!!\n");
      error_unreachable_code();
    }
  } /* if (n == 1) */

  /* freeing no more used structures */
  {
    node_ptr iter = list;
    while (iter != Nil) {
      af_support_list_entry* tmp = (af_support_list_entry*)car(iter);
      support_list_del(tmp, self->dd);

      FREE(tmp);
      iter = cdr(iter);
    }

    free_list(list);

    while (! heap_isempty(_heap)) {
      pair = heap_getmax(_heap);
      FREE(pair);
    }
    heap_destroy(_heap);
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Copy over threshold clusters in result list or in support
  list & heap.]

  Description        [It doesn't modify the input list. ]

  SideEffects        []

******************************************************************************/
static int
clusterlist_affinity_move_clusters(const ClusterList_ptr self,
                                   ClusterList_ptr new_list,
                                   const int threshold,
                                   const boolean append,
                                   node_ptr* list_ref, heap _heap)
{
  int n = 0;
  ClusterListIterator_ptr iter;

  CLUSTER_LIST_CHECK_INSTANCE(new_list);
  nusmv_assert(ClusterList_length(new_list) == 0);

  iter = ClusterList_begin(self);
  while (! ClusterListIterator_is_end(iter) ) {

    Cluster_ptr cluster = ClusterList_get_cluster(self, iter);
    bdd_ptr trans = Cluster_get_trans(cluster);

    if (bdd_size(self->dd, trans) > threshold) {
      Cluster_ptr new_cluster = Cluster_create(self->dd);
      Cluster_set_trans(new_cluster, self->dd, trans);

      /* new_list becomes the owner of new_cluster, so we do not destroy it */
      if (append) ClusterList_append_cluster(new_list, new_cluster);
      else ClusterList_prepend_cluster(new_list, new_cluster);
    }
    else {
      *list_ref = support_list_heap_add(*list_ref, _heap, self->dd, cluster, 
                                        false /* does not own the cluster */);
      n++;
    }

    bdd_free(self->dd, trans);
    iter = ClusterListIterator_next(iter);
  }

  return n;
}


/**Function********************************************************************

  Synopsis           [Compute the Affinity of two BDD clusters.]

  Description        [Compute the Affinity between two BDD clusters as
  suggested by Moon, Hachtel, Somenzi in BBT paper. Affinity is the ratio
  between the number of shared variables and the number of the union of
  all variables (intersection/union)]

  SideEffects        []

******************************************************************************/
#ifdef MHS_AFFINITY_DEFINITION
static double compute_bdd_affinity(DdManager* dd, bdd_ptr a, bdd_ptr b)
{
  double result = 0.0;
  bdd_ptr supp_a, supp_b;
  bdd_ptr I, U;

  supp_a = bdd_support(dd, a);
  supp_b = bdd_support(dd, b);
  I = bdd_cube_intersection(dd, supp_a, supp_b);
  U = bdd_cube_union(dd, supp_a, supp_b);
  bdd_free(dd, supp_a);
  bdd_free(dd, supp_b);

  nusmv_assert(bdd_size(dd, U) != 0); /* a,b are empty cubes!*/

  result = ((double) bdd_size(dd, I)) / ((double) bdd_size(dd, U));

  bdd_free(dd, I);
  bdd_free(dd, U);

  return result;
}
#else
/**Function********************************************************************

  Synopsis           [Compute the Affinity of two BDD clusters.]

  Description        [Compute the Affinity between two BDDs. This is
  an alternative definition to the one suggested by by Moon, Hachtel,
  Somenzi in BBT paper.]

  SideEffects        []

******************************************************************************/
static double compute_bdd_affinity(DdManager* dd, bdd_ptr a, bdd_ptr b)
{
  double result;
  double s_a_b;
  bdd_ptr c;

  s_a_b = (double) (bdd_size(dd, a) + bdd_size(dd, b));

  nusmv_assert(s_a_b != 0); /* a,b are empty bdds! */

  c = bdd_and(dd, a, b);
  result = ((double) bdd_size(dd, c)) / s_a_b;
  bdd_free(dd, c);

  return result;
}
#endif



/**Function********************************************************************

  Synopsis           [Allocates an af_support_list_entry]

  Description        []

  SideEffects        []

******************************************************************************/
static af_support_list_entry* support_list_entry_create()
{
  af_support_list_entry* le;

  le = ALLOC(af_support_list_entry, 1);
  le->exists       = false;
  le->cluster      = CLUSTER(NULL);
  return le;
}


/**Function********************************************************************

  Synopsis           [Allocates a pair]

  Description        [Allocates a pair]

  SideEffects        []

******************************************************************************/
static af_support_pair* af_support_pair_create()
{
  af_support_pair* p;

  p = ALLOC(af_support_pair, 1);
  p->c1          = (af_support_list_entry*)NULL;
  p->c2          = (af_support_list_entry*)NULL;
  return p;
}


/**Function********************************************************************

  Synopsis           [Add a new entry in support list and new pairs in heap.]

  Description        [Pairs with a dead cluster are skipped]

  SideEffects        []

******************************************************************************/
static node_ptr support_list_heap_add(node_ptr list, heap _heap,
                                      DdManager* dd,
                                      Cluster_ptr cluster,
                                      boolean owns_cluster)
{
  bdd_ptr t1;
  af_support_list_entry* new_entry;
  node_ptr iter = list;

  new_entry = support_list_entry_create();
  ASLE_set_exists(new_entry, true);
  ASLE_set_cluster(new_entry, cluster, owns_cluster);

  t1 = Cluster_get_trans(cluster);
  while (iter != Nil) {
    af_support_list_entry* le = (af_support_list_entry*) car(iter);

    if (ASLE_exists(le)) {
      double affinity;
      af_support_pair* p = af_support_pair_create();
      bdd_ptr t2 = Cluster_get_trans(ASLE_get_cluster(le));
      affinity = compute_bdd_affinity(dd, t1, t2);
      bdd_free(dd, t2);

      ASPair_set_c1(p, new_entry);
      ASPair_set_c2(p, le);
      heap_add(_heap, affinity, p);
    }
    iter = cdr(iter);
  }

  bdd_free(dd, t1);
  list = cons((node_ptr)new_entry, list);
  return list;
}


/**Function********************************************************************

  Synopsis           [Delete a cluster in support list.]

  Description        []

  SideEffects        []

******************************************************************************/
static void support_list_del(af_support_list_entry* asle, DdManager* dd)
{
  nusmv_assert(asle != (af_support_list_entry*)NULL);
  ASLE_set_exists(asle, false);
  if (ASLE_owns_cluster(asle)) {
    Object_destroy(OBJECT(ASLE_get_cluster(asle)), dd);
    ASLE_set_cluster(asle, CLUSTER(NULL), false);
  }
}

