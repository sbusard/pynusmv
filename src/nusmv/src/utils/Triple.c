/**CFile***********************************************************************

  FileName    [Triple.c]

  PackageName [utils]

  Synopsis    [Implementation of class 'Triple']

  Description []

  SeeAlso     [Triple.h]

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

******************************************************************************/

#include "Triple.h"
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

static void triple_init ARGS((Triple_ptr self));
static void triple_deinit ARGS((Triple_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [The Triple class constructor]

  Description        [The Triple class constructor]

  SideEffects        []

  SeeAlso            [Triple_destroy]

******************************************************************************/
Triple_ptr Triple_create(void* first, void* second, void* third)
{
  Triple_ptr self = ALLOC(Triple, 1);
  TRIPLE_CHECK_INSTANCE(self);

  triple_init(self);

  Triple_set_values(self, first, second, third);

  return self;
}

/**Function********************************************************************

  Synopsis           [The Triple class initializer]

  Description        [The Triple class initializer.  Use this function if
                      declaring a Triple in the stack ]

  SideEffects        []

  SeeAlso            [Triple_create]

******************************************************************************/
void Triple_init(Triple_ptr self, void* first,
                 void* second, void* third)
{
  triple_init(self);
  Triple_set_values(self, first, second, third);
}


/**Function********************************************************************

  Synopsis           [The Triple class destructor]

  Description        [The Triple class destructor]

  SideEffects        []

  SeeAlso            [Triple_create]

******************************************************************************/
void Triple_destroy(Triple_ptr self)
{
  TRIPLE_CHECK_INSTANCE(self);

  triple_deinit(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis           [Mark the Triple instance as read-only]

  Description        [Mark the Triple instance as read-only.
                      This is usefull when debugging, and using a Triple
                      instance as key of an hash table, for example]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Triple_freeze(Triple_ptr self)
{
  self->frozen = true;
}

/**Function********************************************************************

  Synopsis           [Check if the Triple is freezed]

  Description        [Check if the Triple is freezed (i.e. it is
                      read-only)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Triple_is_freezed(const Triple_ptr self)
{
  return self->frozen;
}

/**Function********************************************************************

  Synopsis           [Get the first value of the Triple instance]

  Description        [Get the first value of the Triple instance]]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void* Triple_get_first(const Triple_ptr self)
{
  return self->first;
}

/**Function********************************************************************

  Synopsis           [Get the second value of the Triple instance]

  Description        [Get the second value of the Triple instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void* Triple_get_second(const Triple_ptr self)
{
  return self->second;
}

/**Function********************************************************************

  Synopsis           [Get the third value of the Triple instance]

  Description        [Get the third value of the Triple instance]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void* Triple_get_third(const Triple_ptr self)
{
  return self->third;
}

/**Function********************************************************************

  Synopsis           [Sets the first value for the Triple instance.]

  Description        [Sets the first value for the Triple instance.
                      The Triple must not be frozen]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Triple_set_first(Triple_ptr self, void* first)
{
  nusmv_assert(!Triple_is_freezed(self));
  self->first = first;
}

/**Function********************************************************************

  Synopsis           [Sets the second value for the Triple instance]

  Description        [Sets the second value for the Triple instance.
                      The Triple must not be frozen]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Triple_set_second(Triple_ptr self, void* second)
{
  nusmv_assert(!Triple_is_freezed(self));
  self->second = second;
}

/**Function********************************************************************

  Synopsis           [Sets the third value for the Triple instance]

  Description        [Sets the third value for the Triple instance.
                      The Triple must not be frozen]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Triple_set_third(Triple_ptr self, void* third)
{
  nusmv_assert(!Triple_is_freezed(self));
  self->third = third;
}

/**Function********************************************************************

  Synopsis           [Sets both the values for the Triple instance]

  Description        [Sets both the values for the Triple instance.
                      The Triple must not be frozen]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Triple_set_values(Triple_ptr self,
                       void* first, void* second, void* third)
{
  nusmv_assert(!Triple_is_freezed(self));
  self->first = first;
  self->second = second;
  self->third = third;
}

/**Function********************************************************************

  Synopsis           [Triple comparison function]

  Description        [Triple comparison function.
                      Returns if the two Triple instances are the
                      equal.  No distinction between frozen / unfrozen
                      instances is made.
                      Can be casted to ST_PFICPCP

                      Casts to char* are added to prevent "warning: pointer of
                      type ‘void *’ used in subtraction".]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Triple_compare(const Triple_ptr a, const Triple_ptr b)
{
  if (a == b) { return 0; }

  if (a->first != b->first) {
    return (char*)(b->first) - (char*)(a->first);
  }

  if (a->second != b->second) {
    return (char*)(b->second) - (char*)(a->second);
  }

  if (a->third != b->third) {
    return (char*)(b->third) - (char*)(a->third);
  }

  return 0;
}

/**Function********************************************************************

  Synopsis           [Triple hash function]

  Description        [Triple hash function.
                      No distinction between frozen / unfrozen
                      instances is made.
                      Can be casted to ST_PFICPI]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Triple_hash(const Triple_ptr self, int size)
{
  size_t ret = ((((size_t)self->first) + 31) +
             (((size_t)self->second) << 1) +
             (((size_t)self->third) << 2));

  return (int)(ret % size);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The Triple class private initializer]

  Description        [The Triple class private initializer]

  SideEffects        []

  SeeAlso            [Triple_create]

******************************************************************************/
static void triple_init(Triple_ptr self)
{
  /* members initialization */
  self->first = NULL;
  self->second = NULL;
  self->third = NULL;
  self->frozen = false;
}


/**Function********************************************************************

  Synopsis           [The Triple class private deinitializer]

  Description        [The Triple class private deinitializer]

  SideEffects        []

  SeeAlso            [Triple_destroy]

******************************************************************************/
static void triple_deinit(Triple_ptr self)
{
  /* members deinitialization */

}



/**AutomaticEnd***************************************************************/

