/**CFile***********************************************************************

  FileName    [NFunction.c]

  PackageName [utils]

  Synopsis    [Implementation of class 'NFunction']

  Description []

  SeeAlso     [NFunction.h]

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2010 by FBK-irst. 

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

  Revision    [$Id: $]

******************************************************************************/

#include "NFunction.h"
#include "utils/utils.h"


static char rcsid[] UTIL_UNUSED = "$Id: $";


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

  Synopsis    [NFunction class definition]

  Description []

  SeeAlso     []   
  
******************************************************************************/
typedef struct NFunction_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */

  NodeList_ptr args;

  SymbType_ptr return_type;

  boolean is_uninterpreted;

  void* body;

  SymbType_ptr main_type;

} NFunction;



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

static void n_function_init ARGS((NFunction_ptr self,
                                  int num_args,
                                  SymbType_ptr* args,
                                  SymbType_ptr ret,
                                  boolean is_uninterpreted,
                                  void* body));

static void n_function_deinit ARGS((NFunction_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The NFunction class constructor]

  Description        [The NFunction class constructor]

  SideEffects        []

  SeeAlso            [NFunction_destroy]   
  
******************************************************************************/
NFunction_ptr NFunction_create_uninterpreted(int num_args,
                                             SymbType_ptr* args,
                                             SymbType_ptr ret)
{
  NFunction_ptr self = ALLOC(NFunction, 1);
  N_FUNCTION_CHECK_INSTANCE(self);

  n_function_init(self, num_args, args, ret, true, NULL);
  return self;
}


/**Function********************************************************************

  Synopsis           [The NFunction class constructor]

  Description        [The NFunction class constructor]

  SideEffects        []

  SeeAlso            [NFunction_destroy]   
  
******************************************************************************/
NFunction_ptr NFunction_create_interpreted(int num_args,
                                           SymbType_ptr* args,
                                           SymbType_ptr ret,
                                           void* body)
{
  NFunction_ptr self = ALLOC(NFunction, 1);
  N_FUNCTION_CHECK_INSTANCE(self);

  n_function_init(self, num_args, args, ret, false, body);
  return self;
}


/**Function********************************************************************

  Synopsis           [The NFunction class destructor]

  Description        [The NFunction class destructor]

  SideEffects        []

  SeeAlso            [NFunction_create]   
  
******************************************************************************/
void NFunction_destroy(NFunction_ptr self)
{
  N_FUNCTION_CHECK_INSTANCE(self);

  n_function_deinit(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis           [The NFunction args_number field getter]

  Description        [The NFunction args_number field getter]

  SideEffects        []

  SeeAlso            [NFunction_set_args_number]   
  
******************************************************************************/
int NFunction_get_args_number(NFunction_ptr self)
{
  N_FUNCTION_CHECK_INSTANCE(self);
  return NodeList_get_length(self->args);
}

/**Function********************************************************************

  Synopsis           [The NFunction args field getter]

  Description        [The NFunction args field getter]

  SideEffects        []

  SeeAlso            [NFunction_set_args]   
  
******************************************************************************/
NodeList_ptr NFunction_get_args(NFunction_ptr self)
{
  N_FUNCTION_CHECK_INSTANCE(self);
  return self->args;
}

/**Function********************************************************************

  Synopsis           [The NFunction main_type field getter]

  Description        [The NFunction main_type field getter]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
SymbType_ptr NFunction_get_main_type(NFunction_ptr self)
{
  N_FUNCTION_CHECK_INSTANCE(self);
  return self->main_type;
}


/**Function********************************************************************

  Synopsis           [The NFunction is_uninterpreted field getter]

  Description        [The NFunction is_uninterpreted field getter]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
boolean NFunction_is_uninterpreted(NFunction_ptr self)
{
  N_FUNCTION_CHECK_INSTANCE(self);
  return self->is_uninterpreted;
}

/**Function********************************************************************

  Synopsis           [The NFunction body field getter]

  Description        [The NFunction body field getter]

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
void* NFunction_get_body(NFunction_ptr self)
{
  N_FUNCTION_CHECK_INSTANCE(self);

  nusmv_assert(!self->is_uninterpreted);

  return self->body;
}

/**Function********************************************************************

  Synopsis           [The NFunction return_type field getter]

  Description        [The NFunction return_type field getter]

  SideEffects        []

  SeeAlso            [NFunction_set_return_type]   
  
******************************************************************************/
SymbType_ptr NFunction_get_return_type(NFunction_ptr self)
{
  N_FUNCTION_CHECK_INSTANCE(self);
  return self->return_type;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The NFunction class private initializer]

  Description        [The NFunction class private initializer]

  SideEffects        []

  SeeAlso            [NFunction_create]

******************************************************************************/
static void n_function_init(NFunction_ptr self,
                            int num_args,
                            SymbType_ptr* args,
                            SymbType_ptr ret,
                            boolean is_uninterpreted,
                            void* body)
{
  int i, type_i = -1;
  boolean have_word = false;
  boolean have_real_int = false;

  /* members initialization */
  self->args = NodeList_create();
  self->return_type = SymbType_copy(ret);
  self->is_uninterpreted = is_uninterpreted;
  self->body = body;
  self->main_type = SYMB_TYPE(NULL);

  for (i = 0; i < num_args; ++i) {
    SymbType_ptr type = args[i];

    if (SymbType_is_word(type)) {
      type_i = i;
      have_word = true;
    }
    else if (!have_word && (SymbType_is_real(type) ||
                            SymbType_is_pure_int_enum(type) ||
                            SymbType_is_integer(type))) {
      type_i = i;
      have_real_int = true;
    }
    else if (!have_real_int && SymbType_is_boolean(type)) {
      type_i = i;
    }

    NodeList_append(self->args, NODE_PTR(SymbType_copy(type)));
  }

  nusmv_assert(type_i >= 0);

  self->main_type = SymbType_copy(args[type_i]);
}

/**Function********************************************************************

  Synopsis           [The NFunction class private deinitializer]

  Description        [The NFunction class private deinitializer]

  SideEffects        []

  SeeAlso            [NFunction_destroy]   
  
******************************************************************************/
static void n_function_deinit(NFunction_ptr self)
{
  /* members deinitialization */
  ListIter_ptr iter;

  NODE_LIST_FOREACH(self->args, iter) {
    SymbType_ptr type = (SymbType_ptr)NodeList_get_elem_at(self->args, iter);
    SymbType_destroy(type);
  }

  NodeList_destroy(self->args);

  SymbType_destroy(self->return_type);
  SymbType_destroy(self->main_type);
}



/**AutomaticEnd***************************************************************/

