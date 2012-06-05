/**CHeaderFile*****************************************************************

  FileName    [beManagerInt.h]

  PackageName [be]

  Synopsis    [The private interface of the Be_Manager class]

  Description [This interface is privately used into this package only.
  Be_Manager can be considered as a "virtual base class"
  which must be instantiated via inheritance by more specific classes 
  whose implementations depend on the real low-level structure them use 
  (i.e. the rbc manager)
  Files beRbc.{h,c} define and implement the derived class which implements 
  the RBC layer. Look at them as a possible template and example.]

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

#ifndef _BE_MANAGER_INT_H_
#define _BE_MANAGER_INT_H_

#include "be.h"
#include "utils/utils.h" /* for EXTERN and ARGS */

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [The generic Boolean Expressions Manager (private declaration)]

  Description [To access this structure you must use the Be_Manager_ptr type.]

  SeeAlso     [Be_Manager_ptr]

******************************************************************************/
typedef struct Be_Manager_TAG {
  void* spec_manager; /* the low-level manager */
  void* support_data; /* any support structure can be stored here */
  
  /* Gateway: */
  Be_Spec2Be_fun       spec2be_converter;
  Be_Be2Spec_fun       be2spec_converter;

} Be_Manager;


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
EXTERN Be_Manager_ptr 
Be_Manager_Create ARGS((void* spec_manager, 
			Be_Spec2Be_fun      spec2be_converter, 
			Be_Be2Spec_fun      be2spec_converter));

EXTERN void Be_Manager_Delete ARGS((Be_Manager_ptr self));

/* Access: */
EXTERN void* Be_Manager_GetData ARGS((const Be_Manager_ptr self));
EXTERN void  Be_Manager_SetData ARGS((Be_Manager_ptr self, void* data));


/**AutomaticEnd***************************************************************/

#endif /* _BE_MANAGER_INT_H_ */

