/**CHeaderFile*****************************************************************

  FileName    [NodeWalker.h]

  PackageName [node]

  Synopsis    [Public interface of class 'NodeWalker']

  Description [A NodeWalker instance is used to traverse a parse tree. 
  Depending on the purpose, the class must be specialized. For example 
  a node printer, or a type checker would derive from this class. 
  A node walker can handle a partition over the set of node's types, 
  and one instance can live into a 'master' that is responsible for 
  calling the right walker depending on the node it is traversing. 

  See for example classes node.printers.PrinterBase and
  compile.type_checking.checkers.CheckerBase]

  SeeAlso     [NodeWalker.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``node'' package of NuSMV version 2. 
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

  Revision    [$Id: NodeWalker.h,v 1.1.2.2.6.1 2009-09-17 11:49:51 nusmv Exp $]

******************************************************************************/


#ifndef __NODE_WALKER_H__
#define __NODE_WALKER_H__


#include "node/node.h"

#include "utils/object.h" 
#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class NodeWalker]

  Description []

******************************************************************************/
typedef struct NodeWalker_TAG*  NodeWalker_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class NodeWalker]

  Description [These macros must be used respectively to cast and to check
  instances of class NodeWalker]

******************************************************************************/
#define NODE_WALKER(self) \
         ((NodeWalker_ptr) self)

#define NODE_WALKER_CHECK_INSTANCE(self) \
         (nusmv_assert(NODE_WALKER(self) != NODE_WALKER(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void NodeWalker_destroy ARGS((NodeWalker_ptr self));

EXTERN boolean 
NodeWalker_can_handle ARGS((const NodeWalker_ptr self, node_ptr n));

EXTERN const char* NodeWalker_get_name ARGS((const NodeWalker_ptr self));


EXTERN boolean NodeWalker_collides ARGS((const NodeWalker_ptr self, 
                                         const NodeWalker_ptr other));


/**AutomaticEnd***************************************************************/


#endif /* __NODE_WALKER_H__ */
