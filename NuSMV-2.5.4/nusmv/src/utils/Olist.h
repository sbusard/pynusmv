/**CHeaderFile*****************************************************************

  FileName    [Olist.h]

  PackageName [addons.omcare]

  Synopsis    [Public interface for a Olist class]

  Description [See Olist.c for the description.]

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

  Revision    [$Id: Olist.h,v 1.1.2.4 2010-02-18 09:52:38 nusmv Exp $]
******************************************************************************/


#ifndef __O_LIST_H__
#define __O_LIST_H__

#include "utils/defs.h"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct Olist_TAG* Olist_ptr;

#define OLIST(x) \
         ((Olist_ptr) x)

#define OLIST_CHECK_INSTANCE(x) \
         ( nusmv_assert(OLIST(x) != OLIST(NULL)) )

/* internal type. it cannot be used outside. */
typedef struct Onode_TAG* Onode_ptr;
/* here a struct definition is used only to create a new type. Thus
   C type checker will be able to catch incorrect use of iterators */
typedef struct Oiter_TAG {Onode_ptr* node;} Oiter;

/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */

EXTERN Olist_ptr Olist_create ARGS((void));
EXTERN void Olist_destroy ARGS((Olist_ptr self));

EXTERN Olist_ptr Olist_copy ARGS((Olist_ptr self));
EXTERN Olist_ptr Olist_copy_reversed ARGS((Olist_ptr self));
EXTERN Olist_ptr Olist_copy_without_element ARGS((Olist_ptr self,
                                                  void* element));
EXTERN void Olist_reverse ARGS((Olist_ptr self));
EXTERN void Olist_move ARGS((Olist_ptr self, Olist_ptr to_list, 
                             Oiter iter_to));
EXTERN void Olist_move_all ARGS((Olist_ptr self, Olist_ptr to_list));
EXTERN void Olist_clean ARGS((Olist_ptr self));

EXTERN void Olist_prepend ARGS((Olist_ptr self, void* element));
EXTERN void Olist_append ARGS((Olist_ptr self, void* element));
EXTERN void* Olist_delete_first ARGS((Olist_ptr self));

EXTERN int Olist_get_size ARGS((const Olist_ptr self));
EXTERN boolean Olist_is_empty ARGS((Olist_ptr self));
EXTERN Oiter Olist_first ARGS((Olist_ptr self));
EXTERN Oiter Olist_last ARGS((Olist_ptr self));

EXTERN void Oiter_make_end ARGS((Oiter* iter));
EXTERN boolean Oiter_is_end ARGS((Oiter iter));
EXTERN Oiter Oiter_next ARGS((Oiter iter));
EXTERN void* Oiter_element ARGS((Oiter iter));
EXTERN void Oiter_set_element ARGS((Oiter iter, void* element));

EXTERN Oiter Olist_insert_after ARGS((Olist_ptr self, Oiter iter,
                                     void* element));
EXTERN Oiter Olist_insert_before ARGS((Olist_ptr self, Oiter iter,
                                       void* element));
EXTERN Oiter Olist_delete ARGS((Olist_ptr self, Oiter iter, void** element));

EXTERN boolean Olist_iter_is_first ARGS((Olist_ptr self, Oiter iter));
EXTERN boolean Olist_iter_is_last ARGS((Olist_ptr self, Oiter iter));

EXTERN boolean Olist_contains ARGS((const Olist_ptr self, const void* element));

EXTERN boolean Olist_remove ARGS((Olist_ptr self, const void* element));

EXTERN void Olist_sort ARGS((Olist_ptr self, 
                             int (*cmp)(void* el1, void* el2)));

EXTERN void Olist_print_node ARGS((Olist_ptr self, FILE* output));

/* this macro can be used to iterate over a list.
   "list" has to be of type Olist_ptr
   "iter" has to be of type Oiter.
 */

#define OLIST_FOREACH(list, iter) \
 for (iter = Olist_first(list); !Oiter_is_end(iter); iter = Oiter_next(iter))

#endif /* __OLIST_H__ */
