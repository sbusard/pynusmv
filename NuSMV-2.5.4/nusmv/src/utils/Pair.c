/**CFile***********************************************************************

  FileName    [Pair.c]

  PackageName [utils]

  Synopsis    [Implementation of class 'Pair']

  Description []

  SeeAlso     [Pair.h]

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2.
  Copyright (C) 2011 by FBK-irst.

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

#include "Pair.h"
#include "utils/utils.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

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
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void pair_init ARGS((Pair_ptr self));
static void pair_deinit ARGS((Pair_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The Pair class constructor]

  Description        [The Pair class constructor]

  SideEffects        []

  SeeAlso            [Pair_destroy]

******************************************************************************/
Pair_ptr Pair_create(void* first, void* second)
{
  Pair_ptr self = ALLOC(Pair, 1);
  PAIR_CHECK_INSTANCE(self);

  pair_init(self);

  Pair_set_values(self, first, second);

  return self;
}

/**Function********************************************************************

  Synopsis           [The Pair class initializer]

  Description        [The Pair class initializer.  Use this function if
                      declaring a Pair in the stack ]

  SideEffects        []

  SeeAlso            [Pair_create]

******************************************************************************/
void Pair_init(Pair_ptr self, void* first, void* second)
{
  pair_init(self);
  Pair_set_values(self, first, second);
}


/**Function********************************************************************

  Synopsis           [The Pair class destructor]

  Description        [The Pair class destructor]

  SideEffects        []

  SeeAlso            [Pair_create]

******************************************************************************/
void Pair_destroy(Pair_ptr self)
{
  PAIR_CHECK_INSTANCE(self);

  pair_deinit(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis           [Mark the Pair instance as read-only]

  Description        [Mark the Pair instance as read-only.
                      This is usefull when debugging, and using a Pair
                      instance as key of an hash table, for example]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Pair_freeze(Pair_ptr self)
{
  self->frozen = true;
}

/**Function********************************************************************

  Synopsis           [Check if the Pair is freezed]

  Description        [Check if the Pair is freezed (i.e. it is
                      read-only)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Pair_is_freezed(const Pair_ptr self)
{
  return self->frozen;
}

/**Function********************************************************************

  Synopsis           [Get the first value of the Pair instance]

  Description        [Get the first value of the Pair instance]]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void* Pair_get_first(const Pair_ptr self)
{
  return self->first;
}

/**Function********************************************************************

  Synopsis           [Get the second value of the Pair instance]

  Description        [Get the second value of the Pair instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void* Pair_get_second(const Pair_ptr self)
{
  return self->second;
}

/**Function********************************************************************

  Synopsis           [Sets the first value for the Pair instance.]

  Description        [Sets the first value for the Pair instance.
                      The Pair must not be frozen]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Pair_set_first(Pair_ptr self, void* first)
{
  nusmv_assert(!Pair_is_freezed(self));
  self->first = first;
}

/**Function********************************************************************

  Synopsis           [Sets the second value for the Pair instance]

  Description        [Sets the second value for the Pair instance.
                      The Pair must not be frozen]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Pair_set_second(Pair_ptr self, void* second)
{
  nusmv_assert(!Pair_is_freezed(self));
  self->second = second;
}

/**Function********************************************************************

  Synopsis           [Sets both the values for the Pair instance]

  Description        [Sets both the values for the Pair instance.
                      The Pair must not be frozen]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Pair_set_values(Pair_ptr self, void* first, void* second)
{
  nusmv_assert(!Pair_is_freezed(self));
  self->first = first;
  self->second = second;
}


/**Function********************************************************************

  Synopsis           [Pair comparison function]

  Description        [Pair comparison function.
                      Returns if the two Pair instances are the
                      equal.  No distinction between frozen / unfrozen
                      instances is made.
                      Can be casted to ST_PFICPCP.

                      Casts to char* are added to prevent "warning: pointer of
                      type ‘void *’ used in subtraction".]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Pair_compare(const Pair_ptr a, const Pair_ptr b)
{
  if (a == b) { return 0; }

  if (a->first != b->first) {
    return (char*)(b->first) - (char*)(a->first);
  }

  if (a->second != b->second) {
    return (char*)(b->second) - (char*)(a->second);
  }

  return 0;
}

/**Function********************************************************************

  Synopsis           [Pair hash function]

  Description        [Pair hash function.
                      No distinction between frozen / unfrozen
                      instances is made.
                      Can be casted to ST_PFICPI]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Pair_hash(const Pair_ptr self, int size)
{
  size_t ret = ((((size_t)self->first) + 31) +
                (((size_t)self->second) << 1));

  return (int)(ret % size);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The Pair class private initializer]

  Description        [The Pair class private initializer]

  SideEffects        []

  SeeAlso            [Pair_create]

******************************************************************************/
static void pair_init(Pair_ptr self)
{
  /* members initialization */
  self->first = NULL;
  self->second = NULL;
  self->frozen = false;
}


/**Function********************************************************************

  Synopsis           [The Pair class private deinitializer]

  Description        [The Pair class private deinitializer]

  SideEffects        []

  SeeAlso            [Pair_destroy]

******************************************************************************/
static void pair_deinit(Pair_ptr self)
{
  /* members deinitialization */
}



/**AutomaticEnd***************************************************************/

