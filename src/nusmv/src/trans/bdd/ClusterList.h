/**CHeaderFile*****************************************************************

  FileName    [ClusterList.h]

  PackageName [trans.bdd]

  Synopsis    [ The header file of ClusterList class.]

  Description [ ]

  SeeAlso     []

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

  Revision    [$Id: ClusterList.h,v 1.1.2.3.4.1.6.2 2007-04-30 16:41:18 nusmv Exp $]

******************************************************************************/

#ifndef __TRANS_BDD_CLUSTER_LIST_H__
#define __TRANS_BDD_CLUSTER_LIST_H__

#include "Cluster.h"
#include "utils/utils.h" /* for EXTERN adn ARGS */
#include "dd/dd.h"
#include "node/node.h"


typedef struct ClusterList_TAG* ClusterList_ptr;
typedef node_ptr ClusterListIterator_ptr; 

#define CLUSTER_LIST(x)  \
        ((ClusterList_ptr) x)

#define CLUSTER_LIST_CHECK_INSTANCE(x)  \
        (nusmv_assert(CLUSTER_LIST(x) != CLUSTER_LIST(NULL)))

#define CLUSTER_LIST_ITERATOR(x) \
        ((ClusterListIterator_ptr) x)

/* ---------------------------------------------------------------------- */

/* 
   This define controls the way a cluster is put into the cluster list
   If this macro is not defined, clusters will be appended instead of
   being prepended.
*/
/*#define CLUSTER_LIST_PREPEND_CLUSTER*/
#undef CLUSTER_LIST_PREPEND_CLUSTER

/* 
   This decides how the affinity among two BDDs is computed. If this
   symbol is defined, then affinity is the ratio between the number of
   shared variables and the number of the union of all variables
   (intersection/union) as as suggested by Moon, Hachtel, Somenzi in
   BBT paper. Otherwise a variation to the previous one is used: this
   is possibly more expensive than the previous one.
*/
#define MHS_AFFINITY_DEFINITION

/* ---------------------------------------------------------------------- */

EXTERN ClusterList_ptr ClusterList_create ARGS((DdManager* dd));
EXTERN void ClusterList_destroy ARGS((ClusterList_ptr self));
EXTERN ClusterList_ptr ClusterList_copy ARGS((const ClusterList_ptr self));


EXTERN ClusterListIterator_ptr
ClusterList_begin ARGS((const ClusterList_ptr self));

EXTERN Cluster_ptr 
ClusterList_get_cluster ARGS((const ClusterList_ptr self, 
                              const ClusterListIterator_ptr iter));

EXTERN void ClusterList_set_cluster ARGS((ClusterList_ptr self, 
                                          const ClusterListIterator_ptr iter, 
                                          Cluster_ptr cluster));


EXTERN int ClusterList_length ARGS((const ClusterList_ptr self));

EXTERN void 
ClusterList_prepend_cluster ARGS((ClusterList_ptr self, Cluster_ptr cluster));

EXTERN void 
ClusterList_append_cluster ARGS((ClusterList_ptr self, Cluster_ptr cluster));

EXTERN ClusterListIterator_ptr 
ClusterListIterator_next ARGS((const ClusterListIterator_ptr self));

EXTERN boolean 
ClusterListIterator_is_end ARGS((const ClusterListIterator_ptr self));

EXTERN void ClusterList_reverse ARGS((ClusterList_ptr self));

EXTERN int 
ClusterList_remove_cluster ARGS((ClusterList_ptr self, Cluster_ptr cluster));


EXTERN ClusterList_ptr 
ClusterList_apply_monolithic ARGS((const ClusterList_ptr self));

EXTERN ClusterList_ptr 
ClusterList_apply_threshold ARGS((const ClusterList_ptr self, 
                                  const ClusterOptions_ptr cl_options));


EXTERN ClusterList_ptr 
ClusterList_apply_iwls95_partition ARGS((const ClusterList_ptr self, 
                                         bdd_ptr state_vars_cube, 
                                         bdd_ptr input_vars_cube, 
                                         bdd_ptr next_state_vars_cube, 
                                         const ClusterOptions_ptr cl_options));

EXTERN void 
ClusterList_apply_synchronous_product ARGS((ClusterList_ptr self, 
                                            const ClusterList_ptr other));

EXTERN bdd_ptr
ClusterList_get_monolithic_bdd ARGS((const ClusterList_ptr self));

EXTERN bdd_ptr
ClusterList_get_clusters_cube ARGS((const ClusterList_ptr self));

EXTERN void 
ClusterList_build_schedule ARGS((ClusterList_ptr self, 
                                 bdd_ptr state_vars_cube, 
                                 bdd_ptr input_vars_cube));

EXTERN bdd_ptr 
ClusterList_get_image_state ARGS((const ClusterList_ptr self, bdd_ptr s));

EXTERN bdd_ptr 
ClusterList_get_image_state_input ARGS((const ClusterList_ptr self, bdd_ptr s));

EXTERN bdd_ptr 
ClusterList_get_k_image_state ARGS((const ClusterList_ptr self, bdd_ptr s, int k));

EXTERN bdd_ptr 
ClusterList_get_k_image_state_input ARGS((const ClusterList_ptr self, bdd_ptr s, int k));

EXTERN void 
ClusterList_print_short_info ARGS((const ClusterList_ptr self, FILE* file));

EXTERN boolean ClusterList_check_equality ARGS((const ClusterList_ptr self,
                                                const ClusterList_ptr other));

EXTERN boolean ClusterList_check_schedule ARGS((const ClusterList_ptr self));


#endif /* __TRANS_BDD_CLUSTER_LIST_H__ */
