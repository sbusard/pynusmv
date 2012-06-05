/**CFile***********************************************************************

  FileName    [Stack.c]

  PackageName [utils]

  Synopsis    [Implementation of Stack class]

  Description [Stack class is very simple but fast stack structure which
               internally uses an array.]

  SeeAlso     []

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
#include "Stack.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define STACK_ARRAY_MIN_SIZE 128

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void stack_init ARGS((Stack_ptr self));
static void stack_init_with_param ARGS((Stack_ptr self, int size));
static void stack_deinit ARGS((Stack_ptr self));

/**AutomaticEnd***************************************************************/
/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Creates an instance of a Stack ]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Stack_ptr Stack_create()
{
  Stack_ptr self = ALLOC(Stack, 1);
  stack_init(self);
  return self;
}

/**Function********************************************************************

  Synopsis    [Creates an instance of a Stack ]

  Description [Allow the user to define the initial size]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Stack_ptr Stack_create_with_param(int size)
{
  Stack_ptr self = ALLOC(Stack, 1);
  stack_init_with_param(self, size);
  return self;
}
/**Function********************************************************************

  Synopsis    [Destroys a stack instance]

  Description [The memory used by the Stack will be freed.
  Note: memory occupied by the elements is not freed! It is the user
  responsibility.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Stack_destroy(Stack_ptr self)
{
  STACK_CHECK_INSTANCE(self);
  stack_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Creates a copy of a given stack]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Stack_ptr Stack_copy(Stack_ptr self)
{
  Stack_ptr copy = ALLOC(Stack, 1);
  STACK_CHECK_INSTANCE(self);

  copy->allocated = self->allocated;
  copy->index = self->index;

  copy->array = ALLOC(void*, self->allocated);
  memcpy(copy->array, self->array, sizeof(void*) * self->allocated);

  return copy;
}

/**Function********************************************************************

  Synopsis           [Pushes an element at the top of the stack]

  Description        []

  SideEffects        []

  SeeAlso            [Stack_pop]

******************************************************************************/
void Stack_push(Stack_ptr self, void* element)
{
  if (self->index == self->allocated) {
    self->allocated *= 2;
    self->array = REALLOC(void*, self->array, self->allocated);
  }

  *(self->array + self->index) = element;
  ++self->index;
}


/**Function********************************************************************

  Synopsis           [Returns the size of the stack]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
size_t Stack_get_size(Stack_ptr self)
{
  STACK_CHECK_INSTANCE(self);
  return self->index;
}

/**Function********************************************************************

  Synopsis           [Removes the element at the top of the stack]

  Description        [The removed element is returned.]

  SideEffects        []

  SeeAlso            [Stack_push]

******************************************************************************/
void* Stack_pop(Stack_ptr self)
{
  STACK_CHECK_INSTANCE(self);
  nusmv_assert(self->index > 0);

  if (self->allocated > STACK_ARRAY_MIN_SIZE && /* Never go below the min size */
      ((double)self->index / (double)self->allocated) < 0.25) {
    self->allocated /= 2;
    self->array = REALLOC(void*, self->array, self->allocated);
  }

  --self->index;
  return *(self->array + self->index);
}


/**Function********************************************************************

  Synopsis           [Returns the element at the top of the stack]

  Description        []

  SideEffects        []

  SeeAlso            [Stack_pop]

******************************************************************************/
void* Stack_top(Stack_ptr self)
{
  STACK_CHECK_INSTANCE(self);
  nusmv_assert(self->index > 0);

  return *(self->array + self->index - 1);
}


/**Function********************************************************************

  Synopsis           [Returns true iff the stack is empty]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Stack_is_empty(Stack_ptr self)
{
  STACK_CHECK_INSTANCE(self);

  return (self->index == 0);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the memory for a Stack instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void stack_init(Stack_ptr self)
{
  STACK_CHECK_INSTANCE(self);

  self->index = 0;
  self->allocated = STACK_ARRAY_MIN_SIZE;
  self->array = ALLOC(void*, self->allocated);
}

/**Function********************************************************************

  Synopsis           [Initializes the memory for a Stack instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void stack_init_with_param(Stack_ptr self, int size)
{
  STACK_CHECK_INSTANCE(self);

  self->index = 0;
  self->allocated = size;
  self->array = ALLOC(void*, self->allocated);
}
/**Function********************************************************************

  Synopsis           [Deinitializes the memory from a Stack]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void stack_deinit(Stack_ptr self)
{
  STACK_CHECK_INSTANCE(self);

  FREE(self->array);
}

