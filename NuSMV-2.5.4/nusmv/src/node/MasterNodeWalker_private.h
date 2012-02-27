/**CHeaderFile*****************************************************************

  FileName    [MasterNodeWalker_private.h]

  PackageName [node]

  Synopsis    [Private interface of class 'MasterNodeWalker', to be used by 
  derivated classes]

  Description []

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

  Revision    [$Id: MasterNodeWalker_private.h,v 1.1.2.1 2006-03-20 17:02:30 nusmv Exp $]

******************************************************************************/


#ifndef __MASTER_NODE_WALKER_PRIVATE_H__
#define __MASTER_NODE_WALKER_PRIVATE_H__

#include "MasterNodeWalker.h"

#include "utils/object.h" 
#include "utils/object_private.h" 
#include "utils/utils.h" 
#include "utils/NodeList.h"


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [MasterNodeWalker class definition]

  Description []

  SeeAlso     []   
  
******************************************************************************/
typedef struct MasterNodeWalker_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(Object); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  NodeList_ptr walkers; 

} MasterNodeWalker;


/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void master_node_walker_init ARGS((MasterNodeWalker_ptr self));
EXTERN void master_node_walker_deinit ARGS((MasterNodeWalker_ptr self));



/**AutomaticEnd***************************************************************/


#endif /* __MASTER_NODE_WALKER_PRIVATE_H__ */
