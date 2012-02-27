
/**CHeaderFile*****************************************************************

  FileName    [NodeGraph.h]

  PackageName [utils]

  Synopsis    [Public interface of class 'NodeGraph']

  Description []

  SeeAlso     [NodeGraph.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2009 by FBK-irst. 

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

  Revision    [$Id: NodeGraph.h,v 1.1.2.2 2009-08-28 08:03:46 nusmv Exp $]

******************************************************************************/


#ifndef __NODE_GRAPH_H__
#define __NODE_GRAPH_H__


#include "node/node.h"
#include "set/set.h"
#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class NodeGraph]

  Description []

******************************************************************************/
typedef struct NodeGraph_TAG*  NodeGraph_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class NodeGraph]

  Description [These macros must be used respectively to cast and to check
  instances of class NodeGraph]

******************************************************************************/
#define NODE_GRAPH(self) \
         ((NodeGraph_ptr) self)

#define NODE_GRAPH_CHECK_INSTANCE(self) \
         (nusmv_assert(NODE_GRAPH(self) != NODE_GRAPH(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN NodeGraph_ptr NodeGraph_create ARGS((void));

EXTERN void NodeGraph_destroy ARGS((NodeGraph_ptr self));

EXTERN void 
NodeGraph_add_children ARGS((NodeGraph_ptr self, node_ptr var, 
                             const Set_t children));

EXTERN void 
NodeGraph_remove_nodes ARGS((NodeGraph_ptr self, const Set_t nodes));

EXTERN void NodeGraph_clear_removed_nodes ARGS((NodeGraph_ptr self));

EXTERN boolean NodeGraph_is_empty ARGS((const NodeGraph_ptr self));

EXTERN Set_t NodeGraph_get_leaves ARGS((const NodeGraph_ptr self));

EXTERN Set_t NodeGraph_get_parents ARGS((const NodeGraph_ptr self,
                                         node_ptr child));

EXTERN void NodeGraph_print ARGS((const NodeGraph_ptr self, FILE* out));


/**AutomaticEnd***************************************************************/



#endif /* __NODE_GRAPH_H__ */
