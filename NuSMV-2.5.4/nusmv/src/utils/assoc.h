/**CHeaderFile*****************************************************************

  FileName    [assoc.h]

  PackageName [util]

  Synopsis    [Simple assscoiative list]

  Description [Provides the user with a data structure that
  implemnts an associative list. If there is already an entry with
  the same ky in the table, than the value associated is replaced with
  the new one.]

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2.
  Copyright (C) 1998-2001 by CMU and FBK-irst.

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

  Revision    [$Id: assoc.h,v 1.5.6.4.4.2.6.5 2010-01-24 13:31:33 nusmv Exp $]

******************************************************************************/

#ifndef _ASSOC_H
#define _ASSOC_H

#include "util.h" /* for ARGS and EXTERN */
#include "node/node.h" /* for node_ptr */
#include "st.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define ASSOC_DELETE ST_DELETE
#define ASSOC_CONTINUE ST_CONTINUE
#define ASSOC_STOP ST_STOP

/**Function********************************************************************

  Synopsis           [Iterate over all k-v pairs in the assoc.]

  Description        [Iterate over all k-v pairs in the assoc.

                      IMPORTANT NOTE: If the loop is interrupted
                      (e.g. by a "break" call, the iterator must be
                      freed manually]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define ASSOC_FOREACH(table, iter, key, value)  \
  st_foreach_item(table, iter, key, value)

/**Function********************************************************************

  Synopsis           [Generates a new iterator for the given hash]

  Description        [Generates a new iterator for the given hash]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define assoc_iter_init(table)                  \
  st_init_gen(table)

/**Function********************************************************************

  Synopsis           [Iterate over all k-v pairs in the assoc.]

  Description        [Returns the next k-v pair in the iterator.
                      If there are no more items, returns 0]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define assoc_iter_next(iter, key, value)       \
  st_gen(iter, key, value)

/**Function********************************************************************

  Synopsis           [Generates a new iterator for the given hash]

  Description        [Generates a new iterator for the given hash]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define assoc_iter_free(iter)                   \
  st_free_gen(iter)

/**Function********************************************************************

  Synopsis           [Retrieve the number of elements in the hash]

  Description        [Retrieve the number of elements in the hash]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define assoc_get_size(table)                   \
  st_count(table)

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct st_table * hash_ptr;
typedef enum st_retval assoc_retval;
typedef struct st_generator* assoc_iter;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN hash_ptr new_assoc ARGS((void));
EXTERN hash_ptr new_assoc_with_size ARGS((int initial_size));
EXTERN hash_ptr new_assoc_with_params ARGS((ST_PFICPCP compare_fun,
                                            ST_PFICPI hash_fun));
EXTERN hash_ptr new_assoc_string_key ARGS(());
EXTERN void free_assoc ARGS((hash_ptr hash));

EXTERN hash_ptr copy_assoc ARGS((hash_ptr hash));
EXTERN node_ptr find_assoc ARGS((hash_ptr, node_ptr));
EXTERN void insert_assoc ARGS((hash_ptr, node_ptr, node_ptr));
EXTERN node_ptr remove_assoc ARGS((hash_ptr hash, node_ptr key));
EXTERN node_ptr assoc_get_keys ARGS((hash_ptr hash, boolean ignore_nils));

EXTERN void clear_assoc_and_free_entries ARGS((hash_ptr, ST_PFSR));
EXTERN void
clear_assoc_and_free_entries_arg ARGS((hash_ptr hash, ST_PFSR fn, char* arg));

EXTERN void clear_assoc ARGS((hash_ptr hash));

EXTERN void assoc_foreach ARGS((hash_ptr hash, ST_PFSR fn, char *arg));

#endif /* _ASSOC_H */
