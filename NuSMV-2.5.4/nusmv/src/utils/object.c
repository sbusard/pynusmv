/**CSourceFile*****************************************************************

  FileName    [object.c]

  PackageName [utils]

  Synopsis    [Basic services for object-oriented design]

  Description [Class Object is a simple pure base class, to be used as base 
  for a class hierarchy]

  SeeAlso     [object.h, object_private.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst.

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


#include "object.h"
#include "object_private.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: object.c,v 1.1.2.5.4.1.6.1 2009-06-18 13:47:00 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void object_finalize ARGS((Object_ptr self, void* arg));
static Object_ptr object_copy ARGS((const Object_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Class virtual destructor]

  Description        [Class virtual destructor. Call this to destroy any 
  instance of any derived class.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
VIRTUAL void Object_destroy(Object_ptr self, void* arg)
{
  OBJECT_CHECK_INSTANCE(self);
  self->finalize(self, arg);
}


/**Function********************************************************************

  Synopsis           [Class virtual copy constructor]

  Description        [Call this by passing any class instance derived from
                      Object. Cast the result to the real class type
                      to assign the returned value. 
		      Since Object is a virtual class, it cannot be 
		      really instantiated. This means that the copy constructor
		      must be implemented by derived class if the copy is 
		      a needed operation. ]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
VIRTUAL Object_ptr Object_copy(const Object_ptr self)
{
  return self->copy(self);
}


/*---------------------------------------------------------------------------*/
/* Definition of private functions                                           */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Class private inizializer]

  Description        [This private method must be called by 
  derived class inizializer *before* any other operation]

  SideEffects        []

  SeeAlso            [object_deinit]   
  
******************************************************************************/
void object_init(Object_ptr self)
{
  OVERRIDE(Object, finalize) = object_finalize;
  OVERRIDE(Object, copy)     = object_copy;   
}


/**Function********************************************************************

  Synopsis           [Class private deinizializer]

  Description        [Must be called by 
  derived class inizializer *after* any other operation. The deinizializer
  in derived class must be called only by the finalizer (which is called 
  the destructor). No other operation is allowed on the instance is being 
  to be destroyed.]

  SideEffects        []

  SeeAlso            [object_init]   
  
******************************************************************************/
void object_deinit(Object_ptr self)
{
  OVERRIDE(Object, finalize) = NULL;
  OVERRIDE(Object, copy)     = NULL;   
}


/**Function********************************************************************

  Synopsis           [Copy costructor auxiliary private method]

  Description        [This must be called by any derived class auxiliary copy
  constructor *before* any other operation.]

  SideEffects        []

  SeeAlso            [object_copy]   
  
******************************************************************************/
void object_copy_aux(const Object_ptr self, Object_ptr copy)
{
  copy->finalize = self->finalize;
  copy->copy     = self->copy;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Private copy constructor]

  Description        [In order to provide copy feature, any derived class must 
  implement two copy private functions: copy and copy_aux functions, to split 
  copy creation and actual copy operations. copy creates an instance and 
  passes it to copy_aux. In any derived class, the base class' copy_aux 
  method must be called by the copy_aux method before any other operation. 
  If the derived class does not override the object's copy constructor, 
  and the user tries to copy the derived class instance by calling the 
  Object_copy method, then an assertion is fired since Object is a class 
  that cannot be instantiated. ]

  SideEffects        []

  SeeAlso            [object_copy_aux]   
  
******************************************************************************/
static Object_ptr object_copy(const Object_ptr self)
{
  error_unreachable_code(); /* this is a virtual class, no real instances 
                          are allowed */
  return OBJECT(NULL);
}


/**Function********************************************************************

  Synopsis           [Private pure destructor]

  Description        [Since the Object class cannot be really instantiated, 
  the virtual destructor must be overrided by derived classes. If the derived 
  class does not override the finalizer, then an assertion is fired when 
  the virtual destroyer Object_destroy is called.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
static void object_finalize(Object_ptr self, void* arg)
{
  object_deinit(self);

  error_unreachable_code(); /* this is a virtual class, no real instances 
			  are allowed */
}
