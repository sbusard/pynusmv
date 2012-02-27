/**CFile***********************************************************************

  FileName    [MasterNodeWalker.c]

  PackageName [node]

  Synopsis    [Implementaion of class 'MasterNodeWalker']

  Description []

  SeeAlso     [MasterNodeWalker.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``node'' package of NuSMV version 2.
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

  Revision    [$Id: MasterNodeWalker.c,v 1.1.2.1 2006-03-20 17:02:30 nusmv Exp $]

******************************************************************************/


#include "MasterNodeWalker.h"
#include "MasterNodeWalker_private.h"

#include "NodeWalker_private.h"
#include "nodeInt.h"

#include "utils/utils.h"


static char rcsid[] UTIL_UNUSED = "$Id: MasterNodeWalker.c,v 1.1.2.1 2006-03-20 17:02:30 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


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
static void master_node_walker_finalize ARGS((Object_ptr object, void* dummy));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The MasterNodeWalker class constructor]

  Description        [The MasterNodeWalker class constructor]

  SideEffects        []

  SeeAlso            [MasterNodeWalker_destroy]

******************************************************************************/
MasterNodeWalker_ptr MasterNodeWalker_create()
{
  MasterNodeWalker_ptr self = ALLOC(MasterNodeWalker, 1);
  MASTER_NODE_WALKER_CHECK_INSTANCE(self);

  master_node_walker_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [The MasterNodeWalker class destructor]

  Description        [The MasterNodeWalker class destructor]

  SideEffects        []

  SeeAlso            [MasterNodeWalker_create]

******************************************************************************/
void MasterNodeWalker_destroy(MasterNodeWalker_ptr self)
{
  MASTER_NODE_WALKER_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

  Synopsis           [Registers a walker.]

  Description [Return true if successfully registered, false if
  already registered, and throws an exception if could not register, due
  to the walker's partition that collides with already registered walkers.

  Warning: If this method succeeds, the walker instance belongs to
  self, and its life cycle will be controlled by self as long as the
  walker is registered within self. The user must not destroy a
  registered walker. ]

  SideEffects        []

  SeeAlso            [unregister_walker]

******************************************************************************/
boolean MasterNodeWalker_register_walker(MasterNodeWalker_ptr self,
                                         NodeWalker_ptr walker)
{
  ListIter_ptr iter;

  MASTER_NODE_WALKER_CHECK_INSTANCE(self);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, "MasterNodeWalker: registering walker '%s'...",
      NodeWalker_get_name(walker));
  }

  iter = NodeList_get_first_iter(self->walkers);
  while (!ListIter_is_end(iter)) {
    NodeWalker_ptr pr =
      NODE_WALKER(NodeList_get_elem_at(self->walkers, iter));

    if (pr == walker) return false; /* already registered */

    if (NodeWalker_collides(walker, pr)) {
      rpterr("The walker '%s' partition collides with the " \
       "registered walker '%s'\n",
       NodeWalker_get_name(walker),
       NodeWalker_get_name(pr));
    }

    iter = ListIter_get_next(iter);
  }

  /* ok, not found and valid partition: appends and sets it up */
  NodeList_append(self->walkers, (node_ptr) walker);
  node_walker_set_master(walker, self);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) fprintf(nusmv_stderr, " ok\n");
  return true;
}


/**Function********************************************************************

  Synopsis           [Unregisters a previously registered walker]

  Description [If the walker was registered returns the walker instance.
  If not registered (not found among the currently registered walkers),
  returns NULL but no error occurs. After this method is called,
  ]

  SideEffects        []

  SeeAlso            [register_walker]

******************************************************************************/
NodeWalker_ptr MasterNodeWalker_unregister_walker(MasterNodeWalker_ptr self,
                                                  const char* name)
{
  ListIter_ptr iter;

  MASTER_NODE_WALKER_CHECK_INSTANCE(self);

  iter = NodeList_get_first_iter(self->walkers);
  while (!ListIter_is_end(iter)) {
    NodeWalker_ptr pr = NODE_WALKER(
          NodeList_get_elem_at(self->walkers, iter));

    if (strcmp(NodeWalker_get_name(pr), name) == 0) {
      NodeList_remove_elem_at(self->walkers, iter); /* unregistration */
      node_walker_set_master(pr, MASTER_NODE_WALKER(NULL));
      return pr;
    }
    iter = ListIter_get_next(iter);
  }

  return NODE_WALKER(NULL); /* not found */
}


/**Function********************************************************************

  Synopsis           [Returns the regostered walker whose name is given]

  Description [If the walker is not found among the registered walkers,
  NULL is returned and no error occurs]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeWalker_ptr
MasterNodeWalker_get_walker(MasterNodeWalker_ptr self, const char* name)
{
  ListIter_ptr iter;

  MASTER_NODE_WALKER_CHECK_INSTANCE(self);

  iter = NodeList_get_first_iter(self->walkers);
  while (!ListIter_is_end(iter)) {
    NodeWalker_ptr pr = NODE_WALKER(
          NodeList_get_elem_at(self->walkers, iter));

    if (strcmp(NodeWalker_get_name(pr), name) == 0) return pr;
    iter = ListIter_get_next(iter);
  }

  return NODE_WALKER(NULL); /* not found */
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The MasterNodeWalker class private initializer]

  Description        [The MasterNodeWalker class private initializer]

  SideEffects        []

  SeeAlso            [MasterNodeWalker_create]

******************************************************************************/
void master_node_walker_init(MasterNodeWalker_ptr self)
{
  /* base class initialization */
  object_init(OBJECT(self));

  /* members initialization */
  self->walkers = NodeList_create();

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = master_node_walker_finalize;
}


/**Function********************************************************************

  Synopsis           [The MasterNodeWalker class private deinitializer]

  Description        [The MasterNodeWalker class private deinitializer]

  SideEffects        []

  SeeAlso            [MasterNodeWalker_destroy]

******************************************************************************/
void master_node_walker_deinit(MasterNodeWalker_ptr self)
{
  /* members deinitialization */
  ListIter_ptr iter = NodeList_get_first_iter(self->walkers);
  while (!ListIter_is_end(iter)) {
    NodeWalker_ptr w = NODE_WALKER(NodeList_get_elem_at(self->walkers, iter));

    /* Prepare the iterator ready to point the next iterator, since
       this element will be removed from the self->walkers list by the
       node_walker_deinit function, which unregisters itself from this
       master walker */
    iter = ListIter_get_next(iter);

    NodeWalker_destroy(w);
  }

  NodeList_destroy(self->walkers);

  /* base class initialization */
  object_deinit(OBJECT(self));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The MasterNodeWalker class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void master_node_walker_finalize(Object_ptr object, void* dummy)
{
  MasterNodeWalker_ptr self = MASTER_NODE_WALKER(object);

  master_node_walker_deinit(self);
  FREE(self);
}


/**AutomaticEnd***************************************************************/

