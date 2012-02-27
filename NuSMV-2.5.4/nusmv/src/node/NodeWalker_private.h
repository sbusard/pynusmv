/**CHeaderFile*****************************************************************

  FileName    [NodeWalker_private.h]

  PackageName [node]

  Synopsis    [Private and protected interface of class 'NodeWalker']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [NodeWalker.h]

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

  Revision    [$Id: NodeWalker_private.h,v 1.1.2.2 2006-04-13 09:52:32 nusmv Exp $]

******************************************************************************/


#ifndef __NODE_WALKER_PRIVATE_H__
#define __NODE_WALKER_PRIVATE_H__


#include "NodeWalker.h" 
#include "MasterNodeWalker.h"

#include "utils/object.h" 
#include "utils/object_private.h" 
#include "utils/utils.h" 


/**Struct**********************************************************************

  Synopsis    [NodeWalker class definition derived from
               class Object]

  Description []

  SeeAlso     [Base class Object]   
  
******************************************************************************/
typedef struct NodeWalker_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(Object); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  char* name;
  int low;
  size_t num;
  boolean can_handle_null;

  MasterNodeWalker_ptr master;

} NodeWalker;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only        */
/* ---------------------------------------------------------------------- */
EXTERN NodeWalker_ptr NodeWalker_create ARGS((const char* name, 
                                              int low, size_t num, 
                                              boolean can_handle_null));
EXTERN void 
node_walker_init ARGS((NodeWalker_ptr self, const char* name, 
                       int low, size_t num, boolean can_handle_null));

EXTERN void node_walker_deinit ARGS((NodeWalker_ptr self));

EXTERN void 
node_walker_set_master ARGS((NodeWalker_ptr self, 
                             MasterNodeWalker_ptr master));

EXTERN boolean
node_walker_can_handle_null_node ARGS((const NodeWalker_ptr self));

#endif /* __NODE_WALKER_PRIVATE_H__ */
