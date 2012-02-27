/**CHeaderFile*****************************************************************

  FileName    [PredicateExtractor.h]

  PackageName [compile]

  Synopsis    [Public interface for a predicate-extractor class]

  Description [

  The purpose of PredicateExtractor class is to extract predicates and
  clusters from expressions. This class is similar to
  PredicateNormaliser but does not normalize the whole input expressions,
  just create predicates and cluster.

  Thus if normalization of the input expression is not required it is
  more efficient to use this class.
  
  A few definitions: 

  predicate -- is a boolean expression which have only scalar (not
    boolean) subexpressions. See PredicateNormaliser.h for more info about
    predicate normalization.

  cluster -- is a set of variables met in one predicates. If a
    variable is met in 2 different predicates then their clusters are
    united in one.  Implicitly, clusters divide predicates in groups,
    i.e. every group is a set of predicates that caused this cluster.

  Note that from the definitions both clusters and predicates can be
  only over scalar (not boolean) variables.

  This class allows computation of only predicate or both predicates
  and cluster.  

  Initially, I ([AT]) tried to implement an option to compute clusters
  only without predicates but this did not work. The problem is that
  1) it is necessary to memoize the results, 2) clusters may disappear
  during computation (i.e. be merged with others). Because of 1) it is
  necessary to hash expr->clusters-in-it. Because of 2) it is
  necessary to hash cluster->expr-where-is-came-from and then any
  merge of clusters may require to update huge number of elements in
  the both above hashes.
  Right now a hash expr->predicate-subparts-in-it is created. This
  allows to get clusters through getting dependencies. Any other
  solution I through of was of about the same efficiency. Thus I
  decided to use the most straightforward one.
  


  This is a stand-alone class. This class needs only a type checker --
  to get the type of input expression and type check the generated
  (returned) expressions.  ]
  
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

  Revision    [$Id: PredicateExtractor.h,v 1.1.2.4 2010-02-11 19:33:25 nusmv Exp $]
******************************************************************************/


#ifndef __PREDICATE_EXTRACTOR_H__
#define __PREDICATE_EXTRACTOR_H__

#include "compile/symb_table/SymbTable.h"
#include "compile/FlatHierarchy.h" /* for FlatHierarchy_ptr */
#include "set/set.h"
#include "utils/utils.h"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct PredicateExtractor_TAG* PredicateExtractor_ptr;

#define PREDICATE_EXTRACTOR(x) \
         ((PredicateExtractor_ptr) x)

#define PREDICATE_EXTRACTOR_CHECK_INSTANCE(x) \
         ( nusmv_assert(PREDICATE_EXTRACTOR(x) != PREDICATE_EXTRACTOR(NULL)) )


/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */

EXTERN PredicateExtractor_ptr 
PredicateExtractor_create ARGS((SymbTable_ptr st, boolean use_approx));

EXTERN void  PredicateExtractor_destroy ARGS((PredicateExtractor_ptr self));

EXTERN void
PredicateExtractor_compute_preds ARGS((PredicateExtractor_ptr self, 
                                       node_ptr expr));
EXTERN void
PredicateExtractor_compute_preds_from_hierarchy ARGS((PredicateExtractor_ptr self, 
                                                      FlatHierarchy_ptr fh));
EXTERN Set_t
PredicateExtractor_get_all_preds ARGS((const PredicateExtractor_ptr self));

EXTERN Set_t
PredicateExtractor_get_all_clusters ARGS((const PredicateExtractor_ptr self));
EXTERN Set_t
PredicateExtractor_get_var_cluster ARGS((const PredicateExtractor_ptr self,
                                        node_ptr var));

EXTERN Set_t PredicateExtractor_get_preds_of_a_cluster 
                                ARGS((const PredicateExtractor_ptr self,
                                      Set_t cluster));

EXTERN void 
PredicateExtractor_print  ARGS((const PredicateExtractor_ptr self,
                                FILE* stream,
                                boolean printPredicates,
                                boolean printClusters));

#endif /* __PREDICATE_EXTRACTOR_H__ */
