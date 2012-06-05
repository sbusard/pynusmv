/**CHeaderFile*****************************************************************

  FileName    [object_private.h]

  PackageName [utils]

  Synopsis    [Basic (private) services for object-oriented design]

  Description [Private interface for class Object. To be used only by 
  Object class implementation, and by any derivate class]

  SeeAlso     [object.c, object.h]

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

******************************************************************************/


#ifndef __OBJECT_PRIVATE_H__
#define __OBJECT_PRIVATE_H__

#include "object.h"
#include "utils/utils.h" 

typedef struct Object_TAG
{
  VIRTUAL void (*finalize)(Object_ptr self, void* arg);
  VIRTUAL Object_ptr (*copy)(const Object_ptr self);
} Object; 



/* ---------------------------------------------------------------------- */
/* Private interface to be used by derivate classes only                  */
/* ---------------------------------------------------------------------- */

EXTERN void object_init ARGS((Object_ptr self));
EXTERN void object_deinit ARGS((Object_ptr self));
EXTERN void object_copy_aux ARGS((const Object_ptr self, Object_ptr copy));



#endif /* __OBJECT_PRIVATE_H__ */
