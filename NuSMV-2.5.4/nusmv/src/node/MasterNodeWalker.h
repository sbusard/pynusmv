/**CHeaderFile*****************************************************************

  FileName    [MasterNodeWalker.h]

  PackageName [node]

  Synopsis    [Public interface of class 'MasterNodeWalker']

  Description [This class is intended to be a generic container for
  node walkers. To each walker is associated a partition over the set
  of node's types, and the master is responsible for calling the right
  walker depending on the type of the node that is being traversed]

  SeeAlso     [MasterNodeWalker.c]

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

  Revision    [$Id: MasterNodeWalker.h,v 1.1.2.1.6.1 2009-09-17 11:49:51 nusmv Exp $]

******************************************************************************/


#ifndef __MASTER_NODE_WALKER_H__
#define __MASTER_NODE_WALKER_H__

#include "NodeWalker.h"

#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class MasterNodeWalker]

  Description []

******************************************************************************/
typedef struct MasterNodeWalker_TAG*  MasterNodeWalker_ptr;



/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class MasterNodeWalker]

  Description [These macros must be used respectively to cast and to check
  instances of class MasterNodeWalker]

******************************************************************************/
#define MASTER_NODE_WALKER(self) \
         ((MasterNodeWalker_ptr) self)

#define MASTER_NODE_WALKER_CHECK_INSTANCE(self) \
         (nusmv_assert(MASTER_NODE_WALKER(self) != MASTER_NODE_WALKER(NULL)))


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN MasterNodeWalker_ptr MasterNodeWalker_create ARGS((void));

EXTERN void MasterNodeWalker_destroy ARGS((MasterNodeWalker_ptr self));

EXTERN boolean 
MasterNodeWalker_register_walker ARGS((MasterNodeWalker_ptr self, 
                                       NodeWalker_ptr walker));
				     

EXTERN NodeWalker_ptr
MasterNodeWalker_unregister_walker ARGS((MasterNodeWalker_ptr self, 
                                         const char* name));


/**AutomaticEnd***************************************************************/


#endif /* __MASTER_NODE_WALKER_H__ */
