/**CHeaderFile*****************************************************************

  FileName    [BaseEnc_private.h]

  PackageName [enc.base]

  Synopsis    [Private and protected interface of class 'BaseEnc']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [BaseEnc.h]

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

  Revision    [$Id: BaseEnc_private.h,v 1.1.2.6.6.1 2007-08-29 11:56:40 nusmv Exp $]

******************************************************************************/


#ifndef __BASE_ENC_PRIVATE_H__
#define __BASE_ENC_PRIVATE_H__


#include "BaseEnc.h" 

#include "utils/object.h" 
#include "utils/object_private.h" 
#include "utils/utils.h" 

#include "compile/symb_table/SymbTable.h"

/* Class BaseEnc is friend of SymbLayer */
#include "compile/symb_table/SymbLayer_private.h"


/**Struct**********************************************************************

  Synopsis    [BaseEnc class definition derived from
               class Object]

  Description []

  SeeAlso     [Base class Object]   
  
******************************************************************************/
typedef struct BaseEnc_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(Object); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  SymbTable_ptr symb_table;
  NodeList_ptr committed_layers;
  array_t* layer_names;

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */
  void (*commit_layer)(BaseEnc_ptr self, const char* layer_name);
  void (*remove_layer)(BaseEnc_ptr self, const char* layer_name);
  
} BaseEnc;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only        */
/* ---------------------------------------------------------------------- */
void base_enc_init ARGS((BaseEnc_ptr self, SymbTable_ptr symb_table));

void base_enc_deinit ARGS((BaseEnc_ptr self));

void base_enc_commit_layer ARGS((BaseEnc_ptr self, const char* layer_name));

void base_enc_remove_layer ARGS((BaseEnc_ptr self, const char* layer_name));

#endif /* __BASE_ENC_PRIVATE_H__ */
