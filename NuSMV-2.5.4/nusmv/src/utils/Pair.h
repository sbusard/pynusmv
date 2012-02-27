/**CHeaderFile*****************************************************************

  FileName    [Pair.h]

  PackageName [utils]

  Synopsis    [Public interface of class 'Pair']

  Description []

  SeeAlso     [Pair.c]

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


#ifndef __PAIR_H__
#define __PAIR_H__


#include "utils/utils.h"


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class Pair]

  Description []

******************************************************************************/
typedef struct Pair_TAG*  Pair_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class Pair]

  Description [These macros must be used respectively to cast and to check
  instances of class Pair]

******************************************************************************/
#define PAIR(self) \
         ((Pair_ptr) self)

#define PAIR_CHECK_INSTANCE(self) \
         (nusmv_assert(PAIR(self) != PAIR(NULL)))


/**Struct**********************************************************************

  Synopsis    [Pair class definition]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct Pair_TAG
{
  void* first;
  void* second;
  boolean frozen;
} Pair;


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Pair_ptr Pair_create ARGS((void* first, void* second));

EXTERN void Pair_freeze ARGS((Pair_ptr self));
EXTERN boolean Pair_is_freezed ARGS((const Pair_ptr self));

EXTERN void* Pair_get_first ARGS((const Pair_ptr self));
EXTERN void* Pair_get_second ARGS((const Pair_ptr self));

EXTERN void Pair_set_first ARGS((Pair_ptr self, void* first));
EXTERN void Pair_set_second ARGS((Pair_ptr self, void* second));

EXTERN void Pair_set_values ARGS((Pair_ptr self,
                                  void* first, void* second));

EXTERN void Pair_destroy ARGS((Pair_ptr self));

EXTERN int Pair_compare ARGS((const Pair_ptr a,
                                const Pair_ptr b));

EXTERN int Pair_hash ARGS((const Pair_ptr self, int size));


/**AutomaticEnd***************************************************************/



#endif /* __PAIR_H__ */
