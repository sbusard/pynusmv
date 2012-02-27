/**CHeaderFile*****************************************************************

  FileName    [beRbcManager.h]

  PackageName [be]

  Synopsis    [The interface file for the RBC-dependant Be_Manager 
  implementation.]

  Description [Be_RbcManager is a derived class of Be_Manager]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``be'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by FBK-irst and University of Trento. 

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

#ifndef _BE_RBC_H_
#define _BE_RBC_H_

#include "utils/utils.h" /* for EXTERN and ARGS */
#include "be.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Be_Manager_ptr Be_RbcManager_Create  ARGS((const size_t capacity));
EXTERN void           Be_RbcManager_Delete  ARGS((Be_Manager_ptr self));
EXTERN void           Be_RbcManager_Reserve ARGS((Be_Manager_ptr self, 
                                                  const size_t size));
EXTERN void           Be_RbcManager_Reset   ARGS((const Be_Manager_ptr self));
/**AutomaticEnd***************************************************************/

#endif /* _BE_RBC_H_ */

