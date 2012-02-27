/**CHeaderFile*****************************************************************

  FileName    [object.h]

  PackageName [utils]

  Synopsis    [Basic services for object-oriented design]

  Description [Class Object is a simple pure base class, to be used as base 
  for a class hierarchy]

  SeeAlso     [object.c, object_private.h]

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


#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <memory.h>
#include <assert.h>

#include "utils.h"

typedef struct Object_TAG*  Object_ptr;

#define OBJECT(x)  \
        ((Object_ptr) x)

#define OBJECT_CHECK_INSTANCE(x)  \
        (nusmv_assert(OBJECT(x) != OBJECT(NULL)))


/* ---------------------------------------------------------------------- */
/* OO keywords:                                                           */
#define VIRTUAL 

#define INHERITS_FROM(x) \
       x  __parent__

#define OVERRIDE(Class, virtual_method) \
       ((Class*) self)->virtual_method 
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */

EXTERN VIRTUAL void Object_destroy ARGS((Object_ptr self, void* arg));
EXTERN VIRTUAL Object_ptr Object_copy ARGS((const Object_ptr self));


#endif /* __OBJECT_H__ */
