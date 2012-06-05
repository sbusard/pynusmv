/**CFile***********************************************************************

  FileName    [MasterNormalizer.c]

  PackageName [node.normalizers]

  Synopsis    [Implementaion of class 'MasterNormalizer', derived from
  MasterNodeWalker]

  Description []

  SeeAlso     [MasterNormalizer.h]

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``node.normalizers'' package of NuSMV version 2.
  Copyright (C) 2006 by FBK-irst.

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

  Revision    [$Id: MasterNormalizer.c,v 1.1.2.6.4.4 2009-09-30 15:34:38 nusmv Exp $]

******************************************************************************/

#include "MasterNormalizer.h"
#include "MasterNormalizer_private.h"

#include "node/MasterNodeWalker_private.h"

#include "NormalizerBase.h"
#include "NormalizerBase_private.h"

#include "utils/utils.h"
#include "utils/error.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [MasterNormalizer class definition]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct MasterNormalizer_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(MasterNodeWalker);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */

  hash_ptr cache;

} MasterNormalizer;

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

static void master_normalizer_init ARGS((MasterNormalizer_ptr self));
static void master_normalizer_deinit ARGS((MasterNormalizer_ptr self));
static void master_normalizer_finalize ARGS((Object_ptr object, void* dummy));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The MasterNormalizer class constructor]

  Description        [The MasterNormalizer class constructor]

  SideEffects        []

  SeeAlso            [MasterNormalizer_destroy]

******************************************************************************/
MasterNormalizer_ptr MasterNormalizer_create()
{
  MasterNormalizer_ptr self = ALLOC(MasterNormalizer, 1);
  MASTER_NORMALIZER_CHECK_INSTANCE(self);

  master_normalizer_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [Traverses the tree, and returns a possibly new tree that
  is a normalized copy of the first. Use for constant-time comparison
  of two trees]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr MasterNormalizer_normalize_node(MasterNormalizer_ptr self, node_ptr n)
{
  node_ptr res;
  MASTER_NORMALIZER_CHECK_INSTANCE(self);
  res = master_normalizer_normalize_node(self, n);

  /* Clear the memoization cache at each call. See issue #1960 for
     further details.  A possible scenario that demonstrates that the
     cache has to be cleared is the following:

       - A node n is created with new_node.

       - node_normalize(n) is called and memoization is updated with
         the pointer to n

       - n is released with free_node

       - a new node n' is created with new_node and n' has the same
         address of n, even if it is different.

       - node_normalize(n') is called, but using the memoization we
         obtain a wrong result.
  */

  clear_assoc(self->cache);

  return res;
}

/**Function********************************************************************

  Synopsis           [Looks in the internal memoization cache for a
                      match. Returns Nil if no memoized data has been found]

  Description        [Looks in the internal memoization cache for a
                      match. Returns Nil if no memoized data has been found]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr MasterNormalizer_lookup_cache(MasterNormalizer_ptr self, node_ptr n)
{
  MASTER_NORMALIZER_CHECK_INSTANCE(self);
  return find_assoc(self->cache, n);
}

/**Function********************************************************************

  Synopsis           [Inserts new data in the internal memoization cache.]

  Description        [Inserts new data in the internal memoization cache.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void MasterNormalizer_insert_cache(MasterNormalizer_ptr self, node_ptr n,
                                   node_ptr norm)
{
  MASTER_NORMALIZER_CHECK_INSTANCE(self);
  insert_assoc(self->cache, n, norm);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Internal version of the method normalize_node, callable
  internally and by normalizers]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr master_normalizer_normalize_node(MasterNormalizer_ptr self,
                                          node_ptr n)
{
  ListIter_ptr iter;
  iter = NodeList_get_first_iter(MASTER_NODE_WALKER(self)->walkers);
  while (!ListIter_is_end(iter)) {
    NormalizerBase_ptr pr =
      NORMALIZER_BASE(NodeList_get_elem_at(MASTER_NODE_WALKER(self)->walkers,
                                           iter));

    if (NodeWalker_can_handle(NODE_WALKER(pr), n)) {

      return NormalizerBase_normalize_node(pr, n);
    }

    iter = ListIter_get_next(iter);
  }

  return Nil;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The MasterNormalizer class private initializer]

  Description        [The MasterNormalizer class private initializer]

  SideEffects        []

  SeeAlso            [MasterNormalizer_create]

******************************************************************************/
static void master_normalizer_init(MasterNormalizer_ptr self)
{
  /* base class initialization */
  master_node_walker_init(MASTER_NODE_WALKER(self));

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = master_normalizer_finalize;

  self->cache = new_assoc();
}


/**Function********************************************************************

  Synopsis           [The MasterNormalizer class private deinitializer]

  Description        [The MasterNormalizer class private deinitializer]

  SideEffects        []
  SeeAlso            [MasterNormalizer_destroy]

******************************************************************************/
static void master_normalizer_deinit(MasterNormalizer_ptr self)
{
  /* base class deinitialization */
  master_node_walker_deinit(MASTER_NODE_WALKER(self));

  free_assoc(self->cache);
}


/**Function********************************************************************

  Synopsis    [The NormalizerBase class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void master_normalizer_finalize(Object_ptr object, void* dummy)
{
  MasterNormalizer_ptr self = MASTER_NORMALIZER(object);

  master_normalizer_deinit(self);
  FREE(self);
}

/**AutomaticEnd***************************************************************/
