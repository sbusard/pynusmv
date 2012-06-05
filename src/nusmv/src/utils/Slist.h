/**CHeaderFile*****************************************************************

  FileName    [Slist.h]

  PackageName [utils]

  Synopsis    [Public interface for a Slist (Simple List) class]

  Description [See Slist.c for the description.]

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2.
  Copyright (C) 2008 by FBK.

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

  Revision    [$Id: Slist.h,v 1.1.2.7 2010-02-24 15:45:05 nusmv Exp $]
******************************************************************************/


#ifndef __S_LIST_H__
#define __S_LIST_H__

#include "utils/defs.h" /* for EXTERN */

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct Slist_TAG* Slist_ptr;

#define SLIST(x) \
         ((Slist_ptr) x)

#define SLIST_CHECK_INSTANCE(x) \
         ( nusmv_assert(SLIST(x) != SLIST(NULL)) )

#define SLIST_FOREACH(list, iter)                               \
   for (iter=Slist_first(list); !Siter_is_end(iter); \
        iter=Siter_next(iter))


/* internal type. it cannot be used outside. */
typedef struct Snode_TAG* Snode_ptr;

/* Iterator type.
   here a struct definition is used only to create a new type. Thus
   C type checker will be able to catch incorrect use of iterators.
   This does not influence on efficiency
*/
typedef struct Siter_TAG {Snode_ptr node;} Siter;

/* Frre function type */
typedef void (*Slist_free_function)(void*);

/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */

EXTERN Slist_ptr Slist_create ARGS((void));
EXTERN void Slist_destroy ARGS((Slist_ptr self));

EXTERN void Slist_destroy_and_free_elements ARGS((Slist_ptr self,
                                                  Slist_free_function f));

EXTERN Slist_ptr Slist_copy ARGS((Slist_ptr self));
EXTERN Slist_ptr Slist_copy_reversed ARGS((Slist_ptr self));
EXTERN void Slist_reverse ARGS((Slist_ptr self));

EXTERN void Slist_push ARGS((Slist_ptr self, void* element));
EXTERN void* Slist_pop ARGS((Slist_ptr self));
EXTERN void* Slist_top ARGS((Slist_ptr self));

EXTERN boolean Slist_is_empty ARGS((Slist_ptr self));
EXTERN Siter Slist_first ARGS((Slist_ptr self));

EXTERN void Siter_make_end ARGS((Siter* iter));
EXTERN boolean Siter_is_end ARGS((Siter iter));
EXTERN Siter Siter_next ARGS((Siter iter));
EXTERN void* Siter_element ARGS((Siter iter));
EXTERN void Siter_set_element ARGS((Siter iter, void* new_value));

EXTERN Siter Slist_find ARGS((Slist_ptr self, const void* element));

EXTERN boolean Slist_contains ARGS((Slist_ptr self, const void* element));

EXTERN boolean Slist_remove ARGS((Slist_ptr self, const void* element));

EXTERN void Slist_append ARGS((Slist_ptr self, const Slist_ptr other));

EXTERN boolean Slist_equals ARGS((const Slist_ptr self, const Slist_ptr other));

EXTERN unsigned int Slist_get_size ARGS((Slist_ptr self));

EXTERN void Slist_sort ARGS((Slist_ptr self, int (*cmp)(void* el1, void* el2)));

EXTERN void Slist_clear ARGS((Slist_ptr self));

#endif /* __S_LIST_H__ */
