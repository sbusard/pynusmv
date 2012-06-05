/**CHeaderFile*****************************************************************

  FileName    [sbmcHash.h]

  PackageName [bmc.sbmc]

  Synopsis    [Public interface for the hash for pairs (node_ptr, unsigned).]

  Description [An hash table for pairs (node_ptr, unsigned).]

  SeeAlso     []

  Author      [Timo Latvala]

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2.
  Copyright (C) 2004 by Timo Latvala <timo.latvala@tkk.fi>.

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

  For more information of NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#ifndef sbmchash_h_
#define sbmchash_h_

#include "node/node.h" /*For node_ptr*/

#include "utils/utils.h" /* for ARGS and EXTERN */

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define BMC_HASH_NOTFOUND -1

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct htable *hashPtr;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

struct table_pair {
  node_ptr key;
  int data;
};

struct htable {
  /**Number of slots allocated*/
  unsigned alloc;
  /**Number of slots occupied*/
  unsigned occupied;
  /**The table*/
  struct table_pair *table; 
};

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN hashPtr Bmc_Hash_new_htable ARGS((void));
EXTERN int Bmc_Hash_find ARGS((hashPtr, node_ptr));
EXTERN void Bmc_Hash_insert ARGS((hashPtr, node_ptr, int));
EXTERN void Bmc_Hash_delete_table ARGS((hashPtr hash));
EXTERN unsigned Bmc_Hash_size ARGS((hashPtr hash));

#endif /*sbmchash_h_*/
