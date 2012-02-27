/**CFile***********************************************************************

  FileName    [GenericTrans.c]

  PackageName [trans.generic]

  Synopsis    [Class definition for a generic Transition Relation]

  Description [This file contains the definition of the \"GenericTrans\" class
  definition]

  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans.generic'' package of NuSMV version 2. 
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

#include "GenericTrans.h"
#include "GenericTrans_private.h"
#include "trans/transInt.h"

static char rcsid[] UTIL_UNUSED = "$Id: GenericTrans.c,v 1.1.2.2.4.1 2005-03-03 12:32:23 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Static functions declaration                                              */
/*---------------------------------------------------------------------------*/

static Object_ptr generic_trans_copy ARGS((const Object_ptr object));

static void generic_trans_finalize ARGS((Object_ptr object, void* dummy));



/* ---------------------------------------------------------------------- */ 
/*     Public Methods                                                     */
/* ---------------------------------------------------------------------- */   


/**Function********************************************************************

  Synopsis           [Builds the transition relation]

  Description        [None of given arguments will become owned by self. 
  You should destroy cl_options by yourself.]

  SideEffects        []

  SeeAlso            [Object_destroy]

******************************************************************************/
GenericTrans_ptr GenericTrans_create(const TransType trans_type)
{
  GenericTrans_ptr self = ALLOC(GenericTrans, 1);
  
  GENERIC_TRANS_CHECK_INSTANCE(self);
  
  generic_trans_init(self, trans_type);
  return self;
}


/**Function********************************************************************

  Synopsis           [Retrives the type of trans structure.]

  Description        [Returns the type of the transition relation structure passed as 
  the arguments. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
TransType GenericTrans_get_type(const GenericTrans_ptr self)
{
  GENERIC_TRANS_CHECK_INSTANCE(self);  
  return self->_type;
}



/**Function********************************************************************

  Synopsis           [ string to TransType]

  Description        [Converts the given transition type from string "name" to
  TransType object. The possible values of name can be "Monolithic",
  "Threshold", or "Iwls95CP".]

  SideEffects        [None.]

  SeeAlso            [ TransType_to_string ]

******************************************************************************/
TransType TransType_from_string(const char* name)
{
  TransType res;

  if (strcmp(name, TRANS_TYPE_MONOLITHIC_STRING) == 0) {
    res =TRANS_TYPE_MONOLITHIC;
  }
  else if (strcmp(name, TRANS_TYPE_THRESHOLD_STRING) == 0) {
    res = TRANS_TYPE_THRESHOLD;
  }
  else if (strcmp(name, TRANS_TYPE_IWLS95_STRING) == 0) {
    res = TRANS_TYPE_IWLS95;
  }
  else res = TRANS_TYPE_INVALID;
  
  return res;
}

/**Function********************************************************************

  Synopsis           [ TransType to string ]

  Description        [ It takes TransType of self and returns a string
  specifying the type of the transition relation. Returned string is statically
  allocated and must not be freed. ]

  SideEffects        []

  SeeAlso            [TransType_from_string]

******************************************************************************/
const char* TransType_to_string(const TransType self)
{ 
  char* res;

  switch (self) {
  case TRANS_TYPE_MONOLITHIC: res = TRANS_TYPE_MONOLITHIC_STRING; break;
  case TRANS_TYPE_THRESHOLD:  res = TRANS_TYPE_THRESHOLD_STRING;  break;
  case TRANS_TYPE_IWLS95:     res = TRANS_TYPE_IWLS95_STRING;     break;
  default: res = "Unknown";
  }

  return res;
}



/* ---------------------------------------------------------------------- */ 
/*     Private Methods                                                    */
/* ---------------------------------------------------------------------- */   

void generic_trans_init(GenericTrans_ptr self, const TransType trans_type)
{
  object_init(OBJECT(self));

  self->_type = trans_type;

  OVERRIDE(Object, finalize)   = generic_trans_finalize;
  OVERRIDE(Object, copy)       = generic_trans_copy;
}

void generic_trans_deinit(GenericTrans_ptr self)
{
  object_deinit(OBJECT(self));
}


void generic_trans_copy_aux(const GenericTrans_ptr self, GenericTrans_ptr copy)
{
   /* copies the base class: */
  object_copy_aux(OBJECT(self), OBJECT(copy));

  /* copies class members: */
  copy->_type = self->_type;
}


/*---------------------------------------------------------------------------*/
/* Static functions definitions                                              */
/*---------------------------------------------------------------------------*/

static Object_ptr generic_trans_copy(const Object_ptr object) 
{
  GenericTrans_ptr self = GENERIC_TRANS(object);
  GenericTrans_ptr copy;
  
  copy = ALLOC(GenericTrans, 1);
  GENERIC_TRANS_CHECK_INSTANCE(copy);

  generic_trans_copy_aux(self, copy);
  return OBJECT(copy);
}


static void generic_trans_finalize(Object_ptr object, void* dummy)
{
  GenericTrans_ptr self = GENERIC_TRANS(object);

  generic_trans_deinit(self);
  FREE(self);
}

