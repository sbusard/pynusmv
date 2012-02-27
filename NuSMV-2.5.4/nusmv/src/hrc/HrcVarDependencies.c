/**CFile***********************************************************************

  FileName    [HrcVarDependencies.c]

  PackageName [hrc]

  Synopsis    [Implementation of class 'HrcVarDependencies']

  Description ['HrcVarDependencies' keeps a set of dependencies. The
  dependencies are stored in four sets: a set contains variables, a
  set contains define, a set contains formal parameter and a set
  contains actual parameters.

  This class is used in hrcLocalize to change multiple sets in
  different function.]

  SeeAlso     [HrcVarDependencies.h]

  Author      [Sergio Mover]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2. 
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

  Revision    [$Id: HrcVarDependencies.c,v 1.1.2.2 2009-09-02 15:14:20 nusmv Exp $]

******************************************************************************/

#include "HrcVarDependencies.h" 

#include "set/set.h"
#include "utils/assoc.h" 
#include "utils/utils.h" 

static char rcsid[] UTIL_UNUSED = "$Id: HrcVarDependencies.c,v 1.1.2.2 2009-09-02 15:14:20 nusmv Exp $";


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

  Synopsis    [HrcVarDependencies class definition]

  Description [Definition of the HrcVarDepedencies class.]

  SeeAlso     []   
  
******************************************************************************/
typedef struct HrcVarDependencies_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  Set_t variables_set;
  Set_t defines_set;
  Set_t formal_par_set;
  Set_t actual_par_set;

} HrcVarDependencies;



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

static void hrc_var_dependencies_init ARGS((HrcVarDependencies_ptr self));
static void hrc_var_dependencies_deinit ARGS((HrcVarDependencies_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The HrcVarDependencies class constructor]

  Description        [The HrcVarDependencies class constructor]

  SideEffects        []

  SeeAlso            [HrcVarDependencies_destroy]   
  
******************************************************************************/
HrcVarDependencies_ptr HrcVarDependencies_create()
{
  HrcVarDependencies_ptr self = ALLOC(HrcVarDependencies, 1);
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);

  hrc_var_dependencies_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [The HrcVarDependencies class destructor]

  Description        [The HrcVarDependencies class destructor]

  SideEffects        []

  SeeAlso            [HrcVarDependencies_create]   
  
******************************************************************************/
void HrcVarDependencies_destroy(HrcVarDependencies_ptr self)
{
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);

  hrc_var_dependencies_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Add a variable to the variable set.]

  Description        [Add a variable to the variable set.
  No checks are performed to ensure that variable is really a variable.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void HrcVarDependencies_add_variable(HrcVarDependencies_ptr self,
                                     node_ptr variable)
{
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);

  self->variables_set = Set_AddMember(self->variables_set, (Set_Element_t) variable);
}

/**Function********************************************************************

  Synopsis           [Add a define to the define set.]

  Description        [Add a define to the define set.
  No checks are performed to ensure that define is really a define.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void HrcVarDependencies_add_define(HrcVarDependencies_ptr self,
                                   node_ptr define)
{
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);

  self->defines_set = Set_AddMember(self->defines_set, (Set_Element_t) define);
}

/**Function********************************************************************

  Synopsis           [Adds a formal and an actual parameter.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void HrcVarDependencies_add_parameter(HrcVarDependencies_ptr self,
                                      node_ptr formal_name,
                                      node_ptr actual)
{
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);

  self->formal_par_set = Set_AddMember(self->formal_par_set, (Set_Element_t) find_atom(formal_name));
  self->actual_par_set = Set_AddMember(self->actual_par_set, (Set_Element_t) actual);
}

/**Function********************************************************************

  Synopsis           [Returns the set that contains all the variables.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t HrcVarDependencies_get_variables_set(HrcVarDependencies_ptr self)
{
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);

  return self->variables_set;
}

/**Function********************************************************************

  Synopsis           [Returns the set that contains all the defines.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t HrcVarDependencies_get_defines_set(HrcVarDependencies_ptr self)
{
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);

  return self->defines_set;
}

/**Function********************************************************************

  Synopsis           [Returns the set that contains all the formal
  parameters.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t HrcVarDependencies_get_formal_par_set(HrcVarDependencies_ptr self)
{
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);

  return self->formal_par_set;
}

/**Function********************************************************************

  Synopsis           [Returns the set that contains all the actual
  parameters.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t HrcVarDependencies_get_actual_par_set(HrcVarDependencies_ptr self)
{
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);

  return self->actual_par_set;
}

/**Function********************************************************************

  Synopsis           [Checks if formal parameter is contained in the
  formal_par_set.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean HrcVarDependencies_has_formal_parameter(HrcVarDependencies_ptr self,
                                                 node_ptr formal)
{
  HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self);
  
  return Set_IsMember(self->formal_par_set, (Set_Element_t) find_atom(formal));
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The HrcVarDependencies class private initializer]

  Description        [The HrcVarDependencies class private initializer]

  SideEffects        []

  SeeAlso            [HrcVarDependencies_create]   
  
******************************************************************************/
static void hrc_var_dependencies_init(HrcVarDependencies_ptr self)
{
  /* members initialization */
  self->variables_set = Set_MakeEmpty();
  self->defines_set = Set_MakeEmpty();
  self->formal_par_set = Set_MakeEmpty();
  self->actual_par_set = Set_MakeEmpty();
}


/**Function********************************************************************

  Synopsis           [The HrcVarDependencies class private deinitializer]

  Description        [The HrcVarDependencies class private deinitializer]

  SideEffects        []

  SeeAlso            [HrcVarDependencies_destroy]   
  
******************************************************************************/
static void hrc_var_dependencies_deinit(HrcVarDependencies_ptr self)
{
  /* members deinitialization */
  Set_ReleaseSet(self->variables_set);
  Set_ReleaseSet(self->defines_set);
  Set_ReleaseSet(self->formal_par_set);
  Set_ReleaseSet(self->actual_par_set);
}



/**AutomaticEnd***************************************************************/

