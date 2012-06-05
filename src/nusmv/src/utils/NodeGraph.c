
/**CFile***********************************************************************

  FileName    [NodeGraph.c]

  PackageName [utils]

  Synopsis    [Implementation of class 'NodeGraph']

  Description []

  SeeAlso     [NodeGraph.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2009 by FBK-irst. 

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

  Revision    [$Id: NodeGraph.c,v 1.1.2.2 2009-08-28 08:03:44 nusmv Exp $]

******************************************************************************/

#include "NodeGraph.h"

#include "utils/utils.h"
#include "utils/assoc.h"


static char rcsid[] UTIL_UNUSED = "$Id: NodeGraph.c,v 1.1.2.2 2009-08-28 08:03:44 nusmv Exp $";


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

  Synopsis    [NodeGraph class definition]

  Description []

  SeeAlso     []   
  
******************************************************************************/
typedef struct NodeGraph_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  hash_ptr graph_to;
  hash_ptr graph_from;  
  Set_t graph_removed;
  Set_t graph_nodes;

} NodeGraph;



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

static void node_graph_init ARGS((NodeGraph_ptr self));
static void node_graph_deinit ARGS((NodeGraph_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The NodeGraph class constructor]

  Description        [The NodeGraph class constructor]

  SideEffects        []

  SeeAlso            [NodeGraph_destroy]   
  
******************************************************************************/
NodeGraph_ptr NodeGraph_create()
{
  NodeGraph_ptr self = ALLOC(NodeGraph, 1);
  NODE_GRAPH_CHECK_INSTANCE(self);

  node_graph_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [The NodeGraph class destructor]

  Description        [The NodeGraph class destructor]

  SideEffects        []

  SeeAlso            [NodeGraph_create]   
  
******************************************************************************/
void NodeGraph_destroy(NodeGraph_ptr self)
{
  NODE_GRAPH_CHECK_INSTANCE(self);

  node_graph_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
void NodeGraph_add_children(NodeGraph_ptr self, node_ptr var, 
                            const Set_t children)
{
  Set_Iterator_t iter;
  Set_t down;

  NODE_GRAPH_CHECK_INSTANCE(self);
  
  self->graph_nodes = Set_AddMember(self->graph_nodes, var);
  self->graph_nodes = Set_Union(self->graph_nodes, children);

  down = (Set_t) find_assoc(self->graph_to, var);
  if ((Set_t) NULL == down) {
    down = Set_MakeEmpty();
  }
  
  SET_FOREACH(children, iter) {
    node_ptr child = Set_GetMember(children, iter);
    Set_t up = (Set_t) find_assoc(self->graph_from, child);
    if ((Set_t) NULL == up) up = Set_MakeEmpty();    

    if (child != var) { /* self-loops are ignored */
      down = Set_AddMember(down, child);
      up = Set_AddMember(up, var);  
    }
    insert_assoc(self->graph_from, child, (Set_Element_t) up);
  }

  insert_assoc(self->graph_to, var, (Set_Element_t) down);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
void NodeGraph_remove_nodes(NodeGraph_ptr self, const Set_t nodes)
{
  NODE_GRAPH_CHECK_INSTANCE(self);
  self->graph_removed = Set_Union(self->graph_removed, nodes);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
void NodeGraph_clear_removed_nodes(NodeGraph_ptr self)
{
  NODE_GRAPH_CHECK_INSTANCE(self);
  Set_ReleaseSet(self->graph_removed);
  self->graph_removed = Set_MakeEmpty();
}


/**Function********************************************************************

  Synopsis           [Returns true if the graph is empty, taking into account
                      of all removed vertices]

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
boolean NodeGraph_is_empty(const NodeGraph_ptr self)
{
  int diff;

  NODE_GRAPH_CHECK_INSTANCE(self);

  diff = (Set_GiveCardinality(self->graph_nodes) - 
          Set_GiveCardinality(self->graph_removed));
  nusmv_assert(diff >= 0);
  return diff == 0;
}


/**Function********************************************************************

  Synopsis           [Returns the nodes which have the given number of
                      children, but those nodes that have been
                      removed. Set must be freed by the caller]

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
Set_t NodeGraph_get_leaves(const NodeGraph_ptr self)
{
  Set_t res;
  Set_Iterator_t iter;

  NODE_GRAPH_CHECK_INSTANCE(self);

  res = Set_MakeEmpty();
  if (NodeGraph_is_empty(self)) return res;

  SET_FOREACH(self->graph_nodes, iter) {
    node_ptr var = Set_GetMember(self->graph_nodes, iter);

    if (!Set_IsMember(self->graph_removed, var)) {
      Set_t set_to = (Set_t) find_assoc(self->graph_to, var);
      if (Set_Contains(self->graph_removed, set_to)) {
        res = Set_AddMember(res, (Set_Element_t) var);
      }
    }
  }

  return res;
}


/**Function********************************************************************

  Synopsis    [Returns all the parents of a given node]

  Description [Returns a set of all parents of a give node (add with 
  NodeGraph_add_children and with child being among children).
  If a parent node has been marked as removed it is not returned.
  
  The returned set has to be returned by caller.]

  SideEffects []

  SeeAlso     []
  
******************************************************************************/
Set_t NodeGraph_get_parents(const NodeGraph_ptr self, node_ptr child)
{
  NODE_GRAPH_CHECK_INSTANCE(self);
  
  Set_t parents = (Set_t) find_assoc(self->graph_from, child);

  if ((Set_t) NULL == parents) return Set_MakeEmpty();    

  parents = Set_Copy(parents);
  if (Set_IsEmpty(self->graph_removed)) return parents; /* optimization */
  else return Set_Difference(parents, self->graph_removed);
}


/**Function********************************************************************

  Synopsis           [Prints out the graph]

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
void NodeGraph_print(const NodeGraph_ptr self, FILE* out)
{
  Set_Iterator_t iter;

  NODE_GRAPH_CHECK_INSTANCE(self);

  SET_FOREACH(self->graph_nodes, iter) {
    node_ptr var = Set_GetMember(self->graph_nodes, iter);

    if (!Set_IsMember(self->graph_removed, var)) {
      Set_t set_to = (Set_t) find_assoc(self->graph_to, var);
      Set_t set_rem = Set_Difference(Set_Copy(set_to), self->graph_removed);
      print_node(out, var);
      fprintf(out, " ==> ");
      Set_PrintSet(out, set_rem, NULL, NULL);
      fprintf(out, "\n");

      Set_ReleaseSet(set_rem);
    }
  }

  fprintf(out, "Removed nodes are:\n");
  Set_PrintSet(out, self->graph_removed, NULL, NULL);  
  fprintf(out, "\n");
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
static assoc_retval node_graph_assoc_free_set(char* key, char* data, char* arg)
{
  Set_t set = (Set_t) data;
  if ((Set_t) NULL != set) Set_ReleaseSet(set);
  return ASSOC_DELETE;
}


/**Function********************************************************************

  Synopsis           [The NodeGraph class private initializer]

  Description        [The NodeGraph class private initializer]

  SideEffects        []

  SeeAlso            [NodeGraph_create]   
  
******************************************************************************/
static void node_graph_init(NodeGraph_ptr self)
{
  /* members initialization */
  self->graph_to = new_assoc();
  self->graph_from = new_assoc();
  self->graph_removed = Set_MakeEmpty();
  self->graph_nodes = Set_MakeEmpty();
}


/**Function********************************************************************

  Synopsis           [The NodeGraph class private deinitializer]

  Description        [The NodeGraph class private deinitializer]

  SideEffects        []

  SeeAlso            [NodeGraph_destroy]   
  
******************************************************************************/
static void node_graph_deinit(NodeGraph_ptr self)
{
  /* members deinitialization */
  Set_ReleaseSet(self->graph_nodes);
  Set_ReleaseSet(self->graph_removed);

  clear_assoc_and_free_entries(self->graph_from, node_graph_assoc_free_set);
  free_assoc(self->graph_from);

  clear_assoc_and_free_entries(self->graph_to, node_graph_assoc_free_set);
  free_assoc(self->graph_to);
}



/**AutomaticEnd***************************************************************/

