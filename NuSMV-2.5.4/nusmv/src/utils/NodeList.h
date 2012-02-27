/**CHeaderFile*****************************************************************

  FileName    [NodeList.h]

  PackageName [utils]

  Synopsis    [ The header file of NodeList class.]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2.
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

  Revision    [$Id: NodeList.h,v 1.1.2.6.4.5.6.6 2008-07-29 14:40:24 nusmv Exp $]

******************************************************************************/

#ifndef __NODE_LIST_H__
#define __NODE_LIST_H__

#include "node/node.h"
#include "utils.h"


/**Type************************************************************************

  Synopsis           [A list based on (and compatible with) node_ptr lists]

  Description        []

  Notes              []

******************************************************************************/
typedef struct NodeList_TAG* NodeList_ptr;

#define NODE_LIST(x)  \
        ((NodeList_ptr) (x))

#define NODE_LIST_CHECK_INSTANCE(x)  \
        (nusmv_assert(NODE_LIST(x) != NODE_LIST(NULL)))


/**Type************************************************************************

  Synopsis           [Use when iterating on NodeLists]

  Description        []

  Notes              []

******************************************************************************/
typedef struct Link_TAG* ListIter_ptr;

#define LIST_ITER(x)  \
        ((ListIter_ptr) (x))


/**Type************************************************************************

  Synopsis           [A type of a predicate function used by NodeList_search]

  Description        [The function should returns true iff
  a given element "corresponds" to 'arg'. 'arg' can be any datastructure.]

  Notes              []

******************************************************************************/
typedef boolean (*NodeListPred) (node_ptr element, void* arg);


/**Type************************************************************************

  Synopsis           [Used when calling method foreach]

  Description        [Must be a pointer to a user-defined function.
  This function gets:
  - the list which method foreach iterates on
  - the iterator pointing to the current element in the list
  - user data, passed to method foreach

  Must return true to continue iteration, false to interrupt it]

  Notes              []

******************************************************************************/
typedef boolean (*NODE_LIST_FOREACH_FUN_P)(NodeList_ptr list, ListIter_ptr iter,
                                           void* user_data);


#define NODE_LIST_FOREACH(list, iter)                               \
   for (iter=NodeList_get_first_iter(list); !ListIter_is_end(iter); \
        iter=ListIter_get_next(iter))


EXTERN NodeList_ptr NodeList_create ARGS((void));
EXTERN NodeList_ptr NodeList_create_from_list ARGS((node_ptr list));
EXTERN void NodeList_destroy ARGS((NodeList_ptr self));

EXTERN NodeList_ptr NodeList_copy ARGS((NodeList_ptr self));

EXTERN void NodeList_append ARGS((NodeList_ptr self, node_ptr elem));
EXTERN void NodeList_prepend ARGS((NodeList_ptr self, node_ptr elem));
EXTERN void NodeList_insert_before ARGS((NodeList_ptr self, ListIter_ptr iter,
                                         node_ptr elem));
EXTERN void NodeList_insert_after ARGS((NodeList_ptr self, ListIter_ptr iter,
                                        node_ptr elem));

EXTERN node_ptr NodeList_remove_elem_at ARGS((NodeList_ptr self,
                                              ListIter_ptr iter));

EXTERN int NodeList_remove_elems ARGS((NodeList_ptr self,
                                       const NodeList_ptr other,
                                       NodeListPred disposer,
                                       void* disposer_arg));

EXTERN int NodeList_get_length ARGS((const NodeList_ptr self));

EXTERN void NodeList_reverse ARGS((NodeList_ptr self));

EXTERN void NodeList_concat ARGS((NodeList_ptr self, const NodeList_ptr src));
EXTERN void NodeList_concat_unique
ARGS((NodeList_ptr self, const NodeList_ptr src));

EXTERN boolean
NodeList_belongs_to ARGS((const NodeList_ptr self, node_ptr elem));

EXTERN ListIter_ptr
NodeList_search ARGS((const NodeList_ptr self, NodeListPred pred, void* arg));

EXTERN int
NodeList_count_elem ARGS((const NodeList_ptr self, node_ptr elem));

EXTERN ListIter_ptr NodeList_get_first_iter ARGS((const NodeList_ptr self));

EXTERN node_ptr NodeList_get_elem_at ARGS((const NodeList_ptr self,
                                           const ListIter_ptr iter));

EXTERN int
NodeList_foreach ARGS((NodeList_ptr self, NODE_LIST_FOREACH_FUN_P foo,
                       void* user_data));

EXTERN NodeList_ptr NodeList_map ARGS((const NodeList_ptr self, NPFN foo));
EXTERN NodeList_ptr NodeList_filter ARGS((const NodeList_ptr self, BPFN foo));

/* ListIter: */
EXTERN ListIter_ptr ListIter_get_next ARGS((const ListIter_ptr self));

EXTERN boolean ListIter_is_end ARGS((const ListIter_ptr self));
EXTERN ListIter_ptr ListIter_get_end ARGS((void));

EXTERN void NodeList_print_nodes ARGS((const NodeList_ptr self, FILE* out));






#endif /* __NODE_LIST_H__ */
