/**CFile***********************************************************************

  FileName    [beManager.c]

  PackageName [be]

  Synopsis    [The generic Boolean Expressions Manager implementation]

  Description [This implementation is independent on the low-level structure is
  being used.]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``be'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst and University of Trento.

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

#include "be.h"
#include "beManagerInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Declarations of internal functions                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates a generic Be_Manager]

  Description [spec_manager is the specific structure which is used to manage
  the low-level structure. For example the RbcManager class in the
  RBC dependant implementation.
  This does not assume the ownership of 'spec_manager'. If you have dynamically
  created the spec_manager instance, you are responsible for deleting it after
  you deleted the Be_manager instance.
  This "virtual" function is supplied in order to be called by any
  specific class derived from Be_Manager, in its constructor code.
  spec2be and be2spec converters are gateways in order to polymorphically
  convert the low level support structure (for example a rbc pointer) to
  the generic be_ptr and viceversa.]

  SideEffects []

  SeeAlso     [Be_RbcManager_Create, Be_Manager_Delete]

******************************************************************************/
Be_Manager_ptr Be_Manager_Create(void* spec_manager,
                                 Be_Spec2Be_fun      spec2be_converter,
                                 Be_Be2Spec_fun      be2spec_converter)
{
  Be_Manager_ptr self = NULL;
  nusmv_assert(spec_manager != NULL);

  self = (Be_Manager_ptr) ALLOC(Be_Manager, 1);
  nusmv_assert(self != NULL);

  self->spec_manager = spec_manager;
  self->support_data = NULL;
  self->spec2be_converter      = spec2be_converter;
  self->be2spec_converter      = be2spec_converter;

  return self;
}


/**Function********************************************************************

  Synopsis    [Be_Manager destroyer]

  Description [Call this function from the destructor of the derived class
  that implements the Be_Manager class. Any other use is to be considered
  unusual.]

  SideEffects [self will be deleted from memory.]

  SeeAlso     [Be_RbcManager_Delete, Be_Manager_Create]

******************************************************************************/
void Be_Manager_Delete(Be_Manager_ptr self)
{
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Converts a specific-format boolean expression
  (for example in rbc format) into a generic BE]

  Description [Calls self->spec2be_converter in order to implement the
  polymorphism mechanism]

  SideEffects [Calls self->be2spec_converter in order to implement the
  polymorphism mechanism]

  SeeAlso     [Be_Manager_Be2Spec]

******************************************************************************/
be_ptr Be_Manager_Spec2Be(const Be_Manager_ptr self, void* spec_expr)
{
  return self->spec2be_converter(self, spec_expr);
}


/**Function********************************************************************

  Synopsis    [Converts a generic BE into a specific-format boolean expression
  (for example in rbc format)]

  Description []

  SideEffects []

  SeeAlso     [Be_Manager_Spec2Be]

******************************************************************************/
void* Be_Manager_Be2Spec(const Be_Manager_ptr self, be_ptr be)
{
  return self->be2spec_converter(self, be);
}


/**Function********************************************************************

  Synopsis    [Derived classes data can be retrieved by this method]

  Description [When you instantiate a derived BE manager (for example the
  rbc manager) you can store any useful specific data by using
  Be_Manager_SetData. Those data can be lately retrieved by Be_Manager_GetData
  which gets a generic, structure-independent Be_Manager.]

  SideEffects []

  SeeAlso     [Be_Manager_SetData]

******************************************************************************/
void* Be_Manager_GetData(const Be_Manager_ptr self)
{
  return self->support_data;
}


/**Function********************************************************************

  Synopsis    [Sets specific structure manager data into the generic
  manager]

  Description [You can retieve saved data by using the method
  Be_Manager_GetData. This implements a kind of inheritance mechanism.]

  SideEffects [self will change its internal state.]

  SeeAlso     [Be_Manager_GetData]

******************************************************************************/
void  Be_Manager_SetData(Be_Manager_ptr self, void* data)
{
  self->support_data = data;
}

/**Function********************************************************************

  Synopsis    [Gets the specific manager under the be manager]

  Description [Gets the specific manager under the be manager]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void* Be_Manager_GetSpecManager(Be_Manager_ptr self)
{
  return self->spec_manager;
}

