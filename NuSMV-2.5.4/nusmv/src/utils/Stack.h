/**CHeaderFile*****************************************************************

  FileName    [Stack.h]

  PackageName [utils]

  Synopsis    [Public interface for a Stack class]

  Description [See Stack.c for the description.]

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2.
  Copyright (C) 2011 by FBK.

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
#ifndef __STACK_H__
#define __STACK_H__

#include "utils/defs.h" /* for EXTERN */

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
/**Struct*********************************************************************

  Synopsis    [Implementation of Stack class]

  Description []

  SeeAlso     []

******************************************************************************/
struct Stack_TAG {
  size_t allocated;
  size_t index;

  void** array;
};

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct Stack_TAG Stack;
typedef struct Stack_TAG* Stack_ptr;

#define STACK(x) \
         ((Stack_ptr) x)

#define STACK_CHECK_INSTANCE(x) \
         ( nusmv_assert(STACK(x) != STACK(NULL)) )

#define STACK_TOP(self) *(self->array + self->index - 1)
#define STACK_IS_EMPTY(self) (self->index == 0)

/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */

EXTERN Stack_ptr Stack_create ARGS((void));
EXTERN Stack_ptr Stack_create_with_param ARGS((int size));
EXTERN void Stack_destroy ARGS((Stack_ptr self));

EXTERN Stack_ptr Stack_copy ARGS((Stack_ptr self));

EXTERN void Stack_push ARGS((Stack_ptr self, void* element));
EXTERN void* Stack_pop ARGS((Stack_ptr self));
EXTERN void* Stack_top ARGS((Stack_ptr self));

EXTERN boolean Stack_is_empty ARGS((Stack_ptr self));

EXTERN size_t Stack_get_size ARGS((Stack_ptr self));

#endif /* __S_LIST_H__ */
