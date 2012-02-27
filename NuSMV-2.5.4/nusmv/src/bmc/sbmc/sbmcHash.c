/**CFile***********************************************************************

  FileName    [sbmcHash.c]

  PackageName [bmc.sbmc]

  Synopsis    [An has table for (node, unsigned) pairs]

  Description [An has table for (node, unsigned) pairs]

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

#include <stdlib.h>
#include <stdio.h>

#include "sbmcHash.h"

#include "utils/utils.h"
#include "utils/error.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define HASH_TABLE_SIZE 127
#define HASH_MAX_DENSITY 0.5

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

EXTERN FILE* nusmv_stderr;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int table_hash_fun ARGS((node_ptr, int));
static int node_eq_fun ARGS((node_ptr, node_ptr));
static int node_neq_fun ARGS((node_ptr, node_ptr));
static int find ARGS((hashPtr table, node_ptr));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Create a new hash_table]

  Description        [Create a new hash_table]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
hashPtr Bmc_Hash_new_htable()
{
  int i;
  hashPtr hash_table = (hashPtr) ALLOC(struct htable, 1);
  hash_table->alloc = HASH_TABLE_SIZE;
  hash_table->occupied = 0;

  hash_table->table = (struct table_pair *) ALLOC(struct table_pair, hash_table->alloc);
  if (hash_table->table == NULL) {
    internal_error("Bmc_Hash_new_htable: Out of Memory\n");
  }

  for (i=0; i < hash_table->alloc; ++i) {
    hash_table->table[i].key = NULL;
  }
  return hash_table;
}

/**Function********************************************************************

  Synopsis           [Find a node in the table]

  Description        [Find a node in the table. Return BMC_HASH_NOTFOUND if the
  node is not present ]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int Bmc_Hash_find(hashPtr table, node_ptr node) 
{ 
  int i = find(table, node);
  if (table->table[i].key) 
    return table->table[i].data;
  return BMC_HASH_NOTFOUND;
}

/**Function********************************************************************

  Synopsis           [Return the number of occupied slots]

  Description        [Return the number of occupied slots]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
unsigned Bmc_Hash_size(hashPtr hash) 
{
  return hash->occupied;
}

/**Function********************************************************************

  Synopsis           [Insert an element in the table]

  Description        [Insert an element in the table]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void Bmc_Hash_insert (hashPtr table, node_ptr key, int data)
{
  int i = find(table, key);
  if (table->table[i].key)
    return; /**The node already is in the table*/
  if ((table->occupied+1)/table->alloc > 0.5) { /*a rehash is needed*/
    unsigned j;
    struct table_pair *temp = table->table;
    table->alloc = (table->alloc)*2;
    table->table = (struct table_pair *) ALLOC(struct table_pair, table->alloc);
    nusmv_assert(table->table != NULL);
    
    /**reset new table*/
    for(j = table->alloc; j--; ) {
      (table->table)[j].key = 0;
    }
    
    /**copy the old table to the one*/
    for (j = (table->alloc)/2; j--; ) {
      if (temp[j].key == 0) /**Empty slot*/
        continue;
      int index = find(table, temp[j].key);
      nusmv_assert((table->table)[index].key == 0);
      (table->table)[index].data = temp[j].data;
      (table->table)[index].key = temp[j].key;
    }
    FREE(temp);
    Bmc_Hash_insert(table, key, data);
    return;
  }
  nusmv_assert((table->table)[i].key == 0);
  (table->table)[i].key = key;
  (table->table)[i].data = data;
  (table->occupied)++;
  return;
}

/**Function********************************************************************

  Synopsis           [Delete the table]

  Description        [Delete the table]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void Bmc_Hash_delete_table(hashPtr hash)
{
  FREE(hash->table);
  FREE(hash);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

static int table_hash_fun(node_ptr key, int size)
{ return (int) ((nusmv_ptruint)(key) % size); }

static int node_eq_fun(node_ptr a1, node_ptr a2)
{ return((a1) == (a2)); }

static int node_neq_fun(node_ptr a1, node_ptr a2)
{ return((a1) != (a2)); }


/**Function********************************************************************

  Synopsis           [Return index of node, a free index if the node is not in the table]

  Description        [Return index of node, a free index if the node is not in the table]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static int find(hashPtr table, node_ptr node)
{
  int hash = table_hash_fun(node, table->alloc);
  int i;
  for (i = hash; ; ) {
    if ((table->table)[i].key) {
      if (node_eq_fun(node, (table->table)[i].key))
        return i;
    }
    else {
      break;
    }
    i = (i+1)%(table->alloc);
  }
  return i;
}

