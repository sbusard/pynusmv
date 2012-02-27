
/**CFile***********************************************************************

  FileName    [BoolEncClient.c]

  PackageName [enc.base]

  Synopsis    [Implementaion of class 'BoolEncClient']

  Description []

  SeeAlso     [BoolEncClient.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.base'' package of NuSMV version 2. 
  Copyright (C) 2004 by FBK-irst. 

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

  Revision    [$Id: BoolEncClient.c,v 1.1.2.4.6.2 2007-04-20 13:05:53 nusmv Exp $]

******************************************************************************/

#include "BoolEncClient.h" 
#include "BoolEncClient_private.h" 

#include "utils/utils.h" 

static char rcsid[] UTIL_UNUSED = "$Id: BoolEncClient.c,v 1.1.2.4.6.2 2007-04-20 13:05:53 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'BoolEncClient_private.h' for class 'BoolEncClient' definition. */ 

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

static void bool_enc_client_finalize ARGS((Object_ptr object, void* dummy));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [The BoolEncClient class destructor]

  Description [The BoolEncClient class destructor. Since this class is
  pure, only a virtual destructor is provided, and there is no
  constructor]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
void BoolEncClient_destroy(BoolEncClient_ptr self)
{
  BOOL_ENC_CLIENT_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

  Synopsis           [Returns the contained (used) BoolEnc instance]

  Description        [REturned instance belongs to self, do not 
  destroy it.]

  SideEffects        []

  SeeAlso            []   
  
******************************************************************************/
BoolEnc_ptr BoolEncClient_get_bool_enc(const BoolEncClient_ptr self)
{
  BOOL_ENC_CLIENT_CHECK_INSTANCE(self);
  return self->bool_enc;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The BoolEncClient class private initializer]

  Description        [The BoolEncClient class private initializer]

  SideEffects        []

  SeeAlso            [BoolEncClient_create]   
  
******************************************************************************/
void bool_enc_client_init(BoolEncClient_ptr self, 
                          SymbTable_ptr symb_table, BoolEnc_ptr bool_enc)
{
  /* base class initialization */
  base_enc_init(BASE_ENC(self), symb_table);
  
  /* members initialization */
  self->bool_enc = bool_enc;

  /* virtual methods settings */  
  OVERRIDE(Object, finalize) = bool_enc_client_finalize;

  OVERRIDE(BaseEnc, commit_layer) = bool_enc_client_commit_layer; 
  OVERRIDE(BaseEnc, remove_layer) = bool_enc_client_remove_layer; 
}


/**Function********************************************************************

  Synopsis           [The BoolEncClient class private deinitializer]

  Description        [The BoolEncClient class private deinitializer]

  SideEffects        []

  SeeAlso            [BoolEncClient_destroy]   
  
******************************************************************************/
void bool_enc_client_deinit(BoolEncClient_ptr self)
{
  /* members deinitialization */


  /* base class deinitialization */
  base_enc_deinit(BASE_ENC(self));
}


/**Function********************************************************************

  Synopsis           [Checks that the given layer occurs within the used bool 
  encoder, and then adds this layer to the set of committed layers. ]

  Description        [This method must always be called by derived classes]

  SideEffects        []

  SeeAlso            [BoolEncClient_destroy]   
  
******************************************************************************/
void bool_enc_client_commit_layer(BaseEnc_ptr base_enc, const char* layer_name)
{
  BoolEncClient_ptr self;

  self = BOOL_ENC_CLIENT(base_enc);

  nusmv_assert(BaseEnc_layer_occurs(BASE_ENC(self->bool_enc), layer_name));
  
  base_enc_commit_layer(base_enc, layer_name);
}


/**Function********************************************************************

  Synopsis           []

  Description        [This method must always be called by the derived classes 
  overrode method]

  SideEffects        []

  SeeAlso            [BoolEncClient_destroy]   
  
******************************************************************************/
void bool_enc_client_remove_layer(BaseEnc_ptr base_enc, const char* layer_name)
{
  base_enc_remove_layer(base_enc, layer_name);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The BoolEncClient class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void bool_enc_client_finalize(Object_ptr object, void* dummy) 
{
  BoolEncClient_ptr self = BOOL_ENC_CLIENT(object);

  bool_enc_client_deinit(self);
  FREE(self);
}



/**AutomaticEnd***************************************************************/

