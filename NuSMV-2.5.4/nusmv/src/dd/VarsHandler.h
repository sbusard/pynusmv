/**CHeaderFile*****************************************************************

  FileName    [VarsHandler.h]

  PackageName [dd]

  Synopsis    [Public interface of class 'VarsHandler']

  Description [VarsHandler handles the allocation of new variables,
  and their organization within 'groups' (blocks in dd
  terminology). This is done to allow multiple BddEnc instances to
  share the same dd space.]

  SeeAlso     [VarsHandler.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``dd'' package of NuSMV version 2. 
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

  Revision    [$Id: $]

******************************************************************************/


#ifndef __VARS_HANDLER_H__
#define __VARS_HANDLER_H__

#include "dd/dd.h"
#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class VarsHandler]

  Description []

******************************************************************************/
typedef struct VarsHandler_TAG*  VarsHandler_ptr;


/**Type***********************************************************************

  Synopsis [GroupInfo is an opaque structure which contains the
  information about groups of variables.]

  Description [When manipulating variable groups, a pointer to a GroupInfo is 
  returned and/or accepted by class VarsHandler.]

******************************************************************************/
typedef struct GroupInfo_TAG* GroupInfo_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class VarsHandler]

  Description [These macros must be used respectively to cast and to check
  instances of class VarsHandler]

******************************************************************************/
#define VARS_HANDLER(self) \
         ((VarsHandler_ptr) self)

#define VARS_HANDLER_CHECK_INSTANCE(self) \
         (nusmv_assert(VARS_HANDLER(self) != VARS_HANDLER(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN VarsHandler_ptr VarsHandler_create ARGS((DdManager* dd));

EXTERN void VarsHandler_destroy ARGS((VarsHandler_ptr self));

EXTERN DdManager*
VarsHandler_get_dd_manager ARGS((const VarsHandler_ptr self));

EXTERN GroupInfo_ptr
VarsHandler_reserve_group ARGS((VarsHandler_ptr self,
                                int from_lev, int size, int chunk, 
                                boolean can_share, int* lev_low));

EXTERN boolean VarsHandler_can_group ARGS((const VarsHandler_ptr self,
                                           int from_lev, int size, int chunk));

EXTERN boolean 
VarsHandler_release_group ARGS((VarsHandler_ptr self, GroupInfo_ptr bid));

EXTERN void 
VarsHandler_dissolve_group ARGS((VarsHandler_ptr self, GroupInfo_ptr bid));

EXTERN void 
VarsHandler_promote_group ARGS((VarsHandler_ptr self, GroupInfo_ptr bid));

EXTERN void VarsHandler_update_levels ARGS((VarsHandler_ptr self));

/**AutomaticEnd***************************************************************/



#endif /* __VARS_HANDLER_H__ */
