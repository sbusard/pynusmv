/**CHeaderFile*****************************************************************

  FileName    [nodeInt.h]

  PackageName [node]

  Synopsis    [The internal header of the <tt>node</tt> package.]

  Description [None]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``node'' package of NuSMV version 2. 
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

  Revision    [$Id: nodeInt.h,v 1.3.4.2.4.4.6.5 2010-03-04 16:52:53 nusmv Exp $]

******************************************************************************/

#ifndef _node_int_h
#define _node_int_h

#include <stdio.h>

#include "node/node.h"
#include "opt/opt.h"

#include "utils/utils.h"
#include "utils/error.h"
#include "parser/symbols.h"
#include "utils/assoc.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define NODE_MEM_CHUNK 1022

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct node_mgr_ node_mgr_;
typedef struct node_profile_info_TAG node_profile_info;
typedef struct node_profile_info_TAG* node_profile_info_ptr;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [The data structure of the <tt>node</tt> manager.]

  Description [The <tt>node</tt> manager. It provides memory
  management, and hashing.]

  SeeAlso     [DdManager]

******************************************************************************/
struct node_mgr_ {
  size_t allocated;          /* Number of nodes allocated till now */
  size_t hashed;             /* Number of nodes find_noded till now */
  size_t memused;            /* Total memory allocated by the node manager */

  node_ptr* nodelist;        /* The node hash table */
  node_ptr* memoryList;      /* Memory manager for symbol table */
  node_ptr nextFree;         /* List of free nodes */
  hash_ptr subst_hash;       /* The substitution hash */
  unsigned int nodelist_size; 
  unsigned char nodelist_size_idx;
};

/*---------------------------------------------------------------------------*/
/* Constants declarations                                                    */
/*---------------------------------------------------------------------------*/

static const unsigned int node_primes[] = {
  49157, 98317, 196613, 393241, 
  786433, 1572869, 3145739, 6291469, 
  12582917, 25165843, 50331653, 100663319, 
  201326611, 402653189, 805306457, 1610612741, 
  3221225081u
}; 
#define NODE_PRIMES_SIZE (sizeof(node_primes) / sizeof(node_primes[0]))


#ifdef PROFILE_NODE
/**Struct**********************************************************************

  Synopsis    [The data structure used for profiling the internal hashtable]

  Description []

  SeeAlso     []

******************************************************************************/
struct node_profile_info_TAG {
  node_ptr bucket;
  unsigned long load;
  unsigned index;
};
#endif

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


EXTERN FILE* nusmv_stdout;
EXTERN FILE* nusmv_stderr;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void node_init ARGS((void));
EXTERN void node_quit ARGS((void));

#endif /* _node_int_h */
