/**CFile*****************************************************************

  FileName    [OrdGroups.c]

  PackageName [enc.utils]

  Synopsis    [Represents a list of groups of variables. Each group 
  is expected to be kept grouped at encoder level]

  Description [When the order of bool vars is important, a list of 
  of groups of vars is returned. All vars appearing into a group should be 
  grouped by the specific encoding]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.utils'' package of NuSMV version 2. 
  Copyright (C) 2005 by FBK-irst.

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

******************************************************************************/

#include "OrdGroups.h"

#include "utils/NodeList.h"
#include "utils/assoc.h"

static char rcsid[] UTIL_UNUSED = "$Id: OrdGroups.c,v 1.1.2.4.6.4 2009-10-12 14:41:02 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Set of groups of vars.]

  Description [Variables can be organized in groups, and the class
  provides methods that allow for searching the group one variable
  belongs to, and the set of variables that a group contains.]
  
******************************************************************************/
typedef struct OrdGroups_TAG 
{
  hash_ptr name_to_group; /* associates the name of a symbol to its group */
  NodeList_ptr* groups;  /* dynamic array of groups */
  int groups_size; 
  int groups_capacity;

} OrdGroups;


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* Internally used parameters */
#define ORD_GROUPS_CAPACITY_GROW_FACTOR 2
#define ORD_GROUPS_INITIAL_CAPACITY 2

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void ord_groups_init ARGS((OrdGroups_ptr self));
static void 
ord_groups_copy ARGS((const OrdGroups_ptr self, OrdGroups_ptr other));

static void ord_groups_deinit ARGS((OrdGroups_ptr self));
static int ord_groups_allocate_new_group ARGS((OrdGroups_ptr self));

static int 
ord_groups_name_to_group ARGS((OrdGroups_ptr self, node_ptr name));

static void 
ord_groups_associate_name_to_group ARGS((OrdGroups_ptr self, 
                                         node_ptr name, int group));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Class constructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
OrdGroups_ptr OrdGroups_create()
{
  OrdGroups_ptr self = ALLOC(OrdGroups, 1);

  ORD_GROUPS_CHECK_INSTANCE(self);

  ord_groups_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [Class copy constructor]

  Description        [Returned instance is a copy of self]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
OrdGroups_ptr OrdGroups_copy(const OrdGroups_ptr self)
{
  OrdGroups_ptr other;

  ORD_GROUPS_CHECK_INSTANCE(self);

  other = ALLOC(OrdGroups, 1);
  ORD_GROUPS_CHECK_INSTANCE(other);
  ord_groups_copy(self, other);
  return other;
}


/**Function********************************************************************

  Synopsis           [Class destructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void OrdGroups_destroy(OrdGroups_ptr self)
{
  ORD_GROUPS_CHECK_INSTANCE(self);

  ord_groups_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Creates a new group, and returns the group ID for 
  future reference]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int OrdGroups_create_group(OrdGroups_ptr self)
{
  ORD_GROUPS_CHECK_INSTANCE(self);

  return ord_groups_allocate_new_group(self);
}



/**Function********************************************************************

  Synopsis [Adds a new variable to the groups set.]

  Description [The addition is performed only if the variable has not
  been already added to the same group.  If the variable has been
  already added but to a different group, an error occurs. The group 
  must be already existing.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void OrdGroups_add_variable(OrdGroups_ptr self, node_ptr name, int group)
{
  ORD_GROUPS_CHECK_INSTANCE(self);

  /* this performs all the needed checkings */
  ord_groups_associate_name_to_group(self, name, group);
  
  if (! NodeList_belongs_to(self->groups[group], name)) {
    NodeList_append(self->groups[group], name);
  }
}


/**Function********************************************************************

  Synopsis [Adds a list of variable to the groups set.]

  Description [The addition of each variable is performed only if the
  variable has not been already added to the same group.  If the
  variable has been already added but to a different group, an error
  occurs. The group must be already existing.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void OrdGroups_add_variables(OrdGroups_ptr self, NodeList_ptr vars, int group)
{
  ListIter_ptr iter;

  ORD_GROUPS_CHECK_INSTANCE(self);

  iter = NodeList_get_first_iter(vars);
  while (!ListIter_is_end(iter)) {
    OrdGroups_add_variable(self, NodeList_get_elem_at(vars, iter), group);
    iter = ListIter_get_next(iter);
  }
}


/**Function********************************************************************

  Synopsis [Returns the set of variables that belong to a given group]

  Description        [Returned list instance still belongs to self.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr OrdGroups_get_vars_in_group(const OrdGroups_ptr self, int group)
{
  ORD_GROUPS_CHECK_INSTANCE(self);
  nusmv_assert(group >= 0 && group < self->groups_size);

  return self->groups[group];
}


/**Function********************************************************************

  Synopsis [Given a var name, it returns the group that variable
  belongs to.]

  Description [-1 is returned if the variable does not belong to any
  group.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int OrdGroups_get_var_group(const OrdGroups_ptr self, node_ptr name)
{
  ORD_GROUPS_CHECK_INSTANCE(self);
  return ord_groups_name_to_group(self, name);
}



/**Function********************************************************************

  Synopsis           [Returns the number of available groups]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int OrdGroups_get_size(const OrdGroups_ptr self)
{
  ORD_GROUPS_CHECK_INSTANCE(self);
  return self->groups_size;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private class initializer]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ord_groups_init(OrdGroups_ptr self)
{
  self->name_to_group = new_assoc();
  nusmv_assert(self->name_to_group != (hash_ptr) NULL);

  self->groups_capacity = ORD_GROUPS_INITIAL_CAPACITY;
  self->groups_size = 0;
  self->groups = ALLOC(NodeList_ptr, self->groups_capacity);  
  nusmv_assert(self->groups != (NodeList_ptr*) NULL);
}


/**Function********************************************************************

  Synopsis           [Private class copier]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ord_groups_copy(const OrdGroups_ptr self, OrdGroups_ptr other)
{
  other->name_to_group = copy_assoc(self->name_to_group);
  other->groups_capacity = self->groups_capacity;
  other->groups_size = self->groups_size;
  other->groups = ALLOC(NodeList_ptr, self->groups_capacity); 
  nusmv_assert(other->groups != (NodeList_ptr*) NULL);

  { /* copies the array of groups */
    int g; 
    for (g=0; g < other->groups_size; ++g) {
      other->groups[g] = NodeList_copy(self->groups[g]);
    }
  }
}


/**Function********************************************************************

  Synopsis           [Private deinitializer]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ord_groups_deinit(OrdGroups_ptr self)
{
  int g;
  for (g=0; g < self->groups_size; ++g) NodeList_destroy(self->groups[g]); 
  if (self->groups != (NodeList_ptr*) NULL) { FREE(self->groups); }
  self->groups_capacity = 0;
  self->groups_size = 0;

  free_assoc(self->name_to_group);
}


/**Function********************************************************************

  Synopsis           [Creates a new group, and returns its ID]

  Description [Extends the array of groups if needed. Extension is
  performed with a grow factor.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int ord_groups_allocate_new_group(OrdGroups_ptr self)
{
  if (self->groups_size >= self->groups_capacity) {
    self->groups_capacity *= ORD_GROUPS_CAPACITY_GROW_FACTOR;
    self->groups = REALLOC(NodeList_ptr, self->groups, self->groups_capacity);
    nusmv_assert(self->groups != (NodeList_ptr*) NULL);
  }

  self->groups[self->groups_size] = NodeList_create();
  return (self->groups_size)++;
}


/**Function********************************************************************

  Synopsis           [Given a variable name, it returns the group that variable 
  belongs to, or -1 if the variable has not been added.]

  Description        [use this method to access the hash table name_to_group, 
  as the way goups are stored within it is very tricky.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int ord_groups_name_to_group(OrdGroups_ptr self, node_ptr name)
{
  node_ptr res;

  res = find_assoc(self->name_to_group, name);
  if (res != (node_ptr) NULL) return (NODE_TO_INT(res) - 1);
  else return -1;
}


/**Function********************************************************************

  Synopsis [Associates a var name to an existing group, but only if
  not already associated. An error occurs if the given name is already
  associated to a different group]

  Description        [Use this method to access to the hash name_to_group, 
  as values are stored in a tricky way.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ord_groups_associate_name_to_group(OrdGroups_ptr self, 
                                               node_ptr name, int group)
{
  int cg;
  nusmv_assert(group >= 0 && group < self->groups_size);

  cg = ord_groups_name_to_group(self, name);
  if (cg == group) return;

  nusmv_assert(cg == -1); /* not associated to a different group */

  /* we inser group+1 to allow for NULL (0) values.  
     The method ord_groups_name_to_group will readjust the returned
     value accordingly */
  insert_assoc(self->name_to_group, name, NODE_FROM_INT(group+1));
}
