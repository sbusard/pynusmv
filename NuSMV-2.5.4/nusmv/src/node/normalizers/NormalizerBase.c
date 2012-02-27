/**CFile***********************************************************************

  FileName    [NormalizerBase.c]

  PackageName [node.normalizers]

  Synopsis    [Implementaion of class 'NormalizerBase']

  Description []

  SeeAlso     [NormalizerBase.h]

  Author      [Mariotti Alessandro]

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

  Revision    [$Id: NormalizerBase.c,v 1.1.2.5.4.1 2009-03-23 18:13:22 nusmv Exp $]

******************************************************************************/

#include "NormalizerBase.h"
#include "NormalizerBase_private.h"

#include "MasterNormalizer.h"
#include "MasterNormalizer_private.h"

#include "node/MasterNodeWalker.h"
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
/* See 'NormalizerBase_private.h' for class 'NormalizerBase' definition. */

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

static void normalizer_base_finalize ARGS((Object_ptr object, void* dummy));

static node_ptr
normalizer_base_normalize_node ARGS((NormalizerBase_ptr self, node_ptr n));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Creates and initializes a normalizer.
  To be usable, the normalizer will have to be registered to a MasterNormalizer.]

  Description        [To each normalizer is associated a partition of
  consecutive indices over the symbols set. The lowest index of the
  partition is given through the parameter low, while num is the
  partition size. Name is used to easily identify normalizer instances.

  This constructor is private, as this class is virtual]

  SideEffects        []

  SeeAlso            [NormalizerBase_destroy]

******************************************************************************/
NormalizerBase_ptr
NormalizerBase_create(const char* name, int low, size_t num)
{
  NormalizerBase_ptr self = ALLOC(NormalizerBase, 1);
  NORMALIZER_BASE_CHECK_INSTANCE(self);

  normalizer_base_init(self, name, low, num, true);
  return self;
}


/**Function********************************************************************

  Synopsis           [Prints the given node]

  Description [This is virtual method. BEfore calling, please ensure
  the given node can be handled by self, by calling
  NormalizerBase_can_handle.

  Note: This method will be never called by the user]

  SideEffects        []

  SeeAlso            [NormalizerBase_can_handle]

******************************************************************************/
VIRTUAL node_ptr
NormalizerBase_normalize_node(NormalizerBase_ptr self, node_ptr n)
{
  node_ptr res;
  MasterNormalizer_ptr master;
  NORMALIZER_BASE_CHECK_INSTANCE(self);

  master = MASTER_NORMALIZER(NODE_WALKER(self)->master);

  /* Lookup in the cache, return cached data if found */
  res = MasterNormalizer_lookup_cache(master, n);
  if (Nil != res) return res;

  res = self->normalize_node(self, n);

  MasterNormalizer_insert_cache(master, n, res);

  return res;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The NormalizerBase class private initializer]

  Description        [The NormalizerBase class private initializer]

  SideEffects        []

  SeeAlso            [NormalizerBase_create]

******************************************************************************/
void normalizer_base_init(NormalizerBase_ptr self, const char* name,
                          int low, size_t num, boolean can_handle_null)
{
  /* base class initialization */
  node_walker_init(NODE_WALKER(self), name, low, num, can_handle_null);

  /* members initialization */

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = normalizer_base_finalize;
  OVERRIDE(NormalizerBase, normalize_node) = normalizer_base_normalize_node;
}


/**Function********************************************************************

  Synopsis           [The NormalizerBase class private deinitializer]

  Description        [The NormalizerBase class private deinitializer]

  SideEffects        []

  SeeAlso            [NormalizerBase_destroy]

******************************************************************************/
void normalizer_base_deinit(NormalizerBase_ptr self)
{
  /* members deinitialization */

  /* base class initialization */
  node_walker_deinit(NODE_WALKER(self));
}


/**Function********************************************************************

  Synopsis           [This method must be called by the virtual method
  print_node to recursively print sub nodes]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr normalizer_base_throw_normalize_node(NormalizerBase_ptr self, node_ptr n)
{
  if (NodeWalker_can_handle(NODE_WALKER(self), n)) {
    /* checks if self can handle the node without need of re-throw
       to the master */
    return NormalizerBase_normalize_node(self, n);
  }
  return master_normalizer_normalize_node(
           MASTER_NORMALIZER(NODE_WALKER(self)->master), n);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The NormalizerBase class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void normalizer_base_finalize(Object_ptr object, void* dummy)
{
  NormalizerBase_ptr self = NORMALIZER_BASE(object);

  normalizer_base_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Virtual menthod that prints the given node]

  Description [This is a pure virtual method, to be implemented by derived
  class, and cannot be called]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static node_ptr
normalizer_base_normalize_node(NormalizerBase_ptr self, node_ptr n)
{
  internal_error("NormalizerBase: Pure virtual method normalize_node "  \
                 "not implemented\n");
  return Nil;
}

/**AutomaticEnd***************************************************************/
