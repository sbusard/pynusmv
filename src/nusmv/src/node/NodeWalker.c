/**CFile***********************************************************************

  FileName    [NodeWalker.c]

  PackageName [node]

  Synopsis    [Implementaion of class 'NodeWalker']

  Description []

  SeeAlso     [NodeWalker.h]

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

  Revision    [$Id: NodeWalker.c,v 1.1.2.3.4.1 2009-08-05 13:57:59 nusmv Exp $]

******************************************************************************/

#include "NodeWalker.h" 
#include "NodeWalker_private.h" 
#include "MasterNodeWalker_private.h"
#include "nodeInt.h"

#include "utils/utils.h" 
#include "utils/defs.h" 

static char rcsid[] UTIL_UNUSED = "$Id: NodeWalker.c,v 1.1.2.3.4.1 2009-08-05 13:57:59 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'NodeWalker_private.h' for class 'NodeWalker' definition. */ 

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

static void node_walker_finalize ARGS((Object_ptr object, void* dummy));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Creates and initializes a walker.
  To be usable, the walker will have to be registered to a MasterNodeWalker]

  Description        [To each walker is associated a partition of
  consecutive indices over the symbols set. The lowest index of the
  partition is given through the parameter low, while num is the
  partition size. Name is used to easily identify walker instances. 

  Constructor is private, as this class is a virtual base class.

  can_handle_null must be set to true if the walker can handle the
  null case.  The null case is trasversal to the partitions set, so
  only the first registered walker that can handle null case will be
  called to handle the null node.]

  SideEffects        []

  SeeAlso            [NodeWalker_destroy]   
  
******************************************************************************/
NodeWalker_ptr 
NodeWalker_create(const char* name, int low, size_t num, 
                  boolean can_handle_null)
{
  NodeWalker_ptr self = ALLOC(NodeWalker, 1);
  NODE_WALKER_CHECK_INSTANCE(self);

  node_walker_init(self, name, low, num, can_handle_null); 
  return self;
}


/**Function********************************************************************

  Synopsis           [The NodeWalker class destructor]

  Description [The NodeWalker class destructor. If registerd to a
  master, it unregisters itself before finalizing.]

  SideEffects        []

  SeeAlso            [NodeWalker_create]   
  
******************************************************************************/
void NodeWalker_destroy(NodeWalker_ptr self)
{
  NODE_WALKER_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

  Synopsis    [Returns true if the given node belongs to the partition 
  associated to this walker]

  Description [Returns true if the given node belongs to the partition 
  associated to this walker. If n is Nil then the specific walker will be 
  asked]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
boolean NodeWalker_can_handle(const NodeWalker_ptr self, node_ptr n)
{
  NODE_WALKER_CHECK_INSTANCE(self);

  if (n != (node_ptr) NULL) {
    int i = node_get_type(n);
    return (i >= self->low) && (i < (self->low + self->num));
  }

  /* for NULL case ask the specific walker */
  return node_walker_can_handle_null_node(self);
}


/**Function********************************************************************

  Synopsis           [Returns the walker name as a string]

  Description        [The returned string belongs to self, do not deallocate 
  or change it.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
const char* NodeWalker_get_name(const NodeWalker_ptr self)
{
  NODE_WALKER_CHECK_INSTANCE(self);
  return (const char*) self->name;
}


/**Function********************************************************************

 Synopsis [Checks if self collides with other in terms of their
 respective symbol sets]

  Description        [Returns true if self and other's symbols set collide
  (i.e. are not partitions). Returns false if they are ok.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
boolean NodeWalker_collides(const NodeWalker_ptr self, 
                            const NodeWalker_ptr other)
{
  int l1, l2, h1, h2;

  NODE_WALKER_CHECK_INSTANCE(self);

  l1 = self->low; l2 = other->low;
  h1 = l1 + self->num - 1; h2 = l2 + other->num - 1;

  return !((l2 > h1) || (l1 > h2) || (h1 < l2) || (h2 < l1));
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The NodeWalker class private initializer]

  Description        [The NodeWalker class private initializer]

  SideEffects        []

  SeeAlso            [NodeWalker_create]   
  
******************************************************************************/
void node_walker_init(NodeWalker_ptr self, const char* name, 
                      int low, size_t num, boolean can_handle_null)
{
  /* base class initialization */
  object_init(OBJECT(self));
  
  /* members initialization */
  if (name != (const char*) NULL) {
    self->name = ALLOC(char, strlen(name) + 1);
    nusmv_assert(self->name != (char*) NULL);
    strcpy(self->name, name);
  }
  else self->name = (char*) NULL;

  self->master = MASTER_NODE_WALKER(NULL);
  self->low = low;
  self->num = num;
  self->can_handle_null = can_handle_null;

  /* virtual methods settings */  
  OVERRIDE(Object, finalize) = node_walker_finalize;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 8)) { 
    fprintf(nusmv_stderr, 
            "Created walker '%s' to handle nodes from %d to %" PRIuPTR, 
            name, low, low+num-1);
    if (can_handle_null) fprintf(nusmv_stderr, " (and NULL nodes)\n");
    else fprintf(nusmv_stderr, "\n");
  }
}


/**Function********************************************************************

  Synopsis           [The NodeWalker class private deinitializer]

  Description        [The NodeWalker class private deinitializer]

  SideEffects        []

  SeeAlso            [NodeWalker_destroy]   
  
******************************************************************************/
void node_walker_deinit(NodeWalker_ptr self)
{
  /* members deinitialization */  
  if (self->master != MASTER_NODE_WALKER(NULL)) {
    MasterNodeWalker_unregister_walker(self->master, self->name);
  }

  if (self->name != (char*) NULL) FREE(self->name);

  /* base class de-initialization */
  object_deinit(OBJECT(self));
}



/**Function********************************************************************

  Synopsis [This method is privately called by master while registering the 
  walker]

  Description        [If already assigned to a master, it unregisters itself 
  from the old master before setting the new master]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
void node_walker_set_master(NodeWalker_ptr self, MasterNodeWalker_ptr master)
{
  if (self->master == master) return; /* not the same master */

  if ((master != MASTER_NODE_WALKER(NULL)) && /* not unregistering */
      (self->master != MASTER_NODE_WALKER(NULL))) {
    /* unregister from the previous master, but only if it is not
       unregistering */
    MasterNodeWalker_unregister_walker(self->master, self->name);
  }

  self->master = master;
}


/**Function********************************************************************

  Synopsis           [Returns true if the walker can handle the null case]

  Description        [The null case is trasversal to the partitions set, so
  only the first registered walker that can handle null case will be
  called to handle the null node.]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
boolean node_walker_can_handle_null_node(const NodeWalker_ptr self)
{ return self->can_handle_null; }



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The NodeWalker class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void node_walker_finalize(Object_ptr object, void* dummy) 
{
  NodeWalker_ptr self = NODE_WALKER(object);

  node_walker_deinit(self);
  FREE(self);
}


/**AutomaticEnd***************************************************************/

