/**CHeaderFile*****************************************************************

  FileName    [Sset.h]

  PackageName [addons.omcare]

  Synopsis    [Public interface for a Sset (Sorted Set) class.]

  Description [See Sset.c file for description.]
  
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

  Revision    [$Id: Sset.h,v 1.1.2.3 2009-09-04 11:43:52 nusmv Exp $]
******************************************************************************/


#ifndef __S_SET_H__
#define __S_SET_H__

#include "utils/defs.h" /* for EXTERN */

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct Sset_TAG* Sset_ptr;


#define SSET(x) \
         ((Sset_ptr) x)

#define SSET_CHECK_INSTANCE(x) \
         ( nusmv_assert(SSET(x) != SSET(NULL)) )

/* internal type. it cannot be used outside. */
typedef struct Ssnode_TAG* Ssnode_ptr;

/* Iterator type.
   here a struct definition is used only to create a new type. Thus
   C type checker will be able to catch incorrect use of iterators.
   This does not influence the efficiency */
typedef struct Ssiter_TAG {Ssnode_ptr node;} Ssiter;

/* ---------------------------------------------------------------------- */
/* Public interface                                                       */
/* ---------------------------------------------------------------------- */

EXTERN Sset_ptr Sset_create ARGS((void));
EXTERN void Sset_destroy ARGS((Sset_ptr self));

EXTERN Sset_ptr Sset_copy ARGS((const Sset_ptr self));
EXTERN Sset_ptr Sset_copy_func ARGS((const Sset_ptr self, 
                                     void* (*func)(void*)));

EXTERN boolean Sset_insert ARGS((Sset_ptr self, signed long long int key, 
                                 void* element));

EXTERN Ssiter Sset_find ARGS((Sset_ptr self, signed long long int key));
EXTERN Ssiter Sset_find_le ARGS((Sset_ptr self, signed long long int key));
EXTERN Ssiter Sset_find_ge ARGS((Sset_ptr self, signed long long int key));
EXTERN Ssiter Sset_find_insert ARGS((Sset_ptr self, signed long long int key, 
                                     boolean* is_found));

EXTERN void* Sset_delete ARGS((Sset_ptr self, signed long long int key, 
                               boolean* is_found));
EXTERN void Sset_delete_iter ARGS((Sset_ptr self, Ssiter iter));

EXTERN size_t Sset_get_size ARGS((Sset_ptr self));
EXTERN boolean Sset_is_empty ARGS((Sset_ptr self));
EXTERN Ssiter Sset_first ARGS((Sset_ptr self));
EXTERN Ssiter Sset_last ARGS((Sset_ptr self));

EXTERN Ssiter Ssiter_next ARGS((Ssiter iter));
EXTERN Ssiter Ssiter_prev ARGS((Ssiter iter));

EXTERN boolean Ssiter_is_valid ARGS((Ssiter iter));

EXTERN void* Ssiter_element ARGS((Ssiter iter));
EXTERN signed long long int Ssiter_key ARGS((Ssiter iter));

EXTERN void Ssiter_set_element ARGS((Ssiter iter, void* element));

#endif /* __S_SET_H__ */
