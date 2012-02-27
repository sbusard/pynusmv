
/**CHeaderFile*****************************************************************

  FileName    [BoolEncClient_private.h]

  PackageName [enc.base]

  Synopsis    [Private and protected interface of class 'BoolEncClient']

  Description [This file can be included only by derived and friend classes]

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

  Revision    [$Id: BoolEncClient_private.h,v 1.1.2.3.6.1 2007-04-20 13:05:53 nusmv Exp $]

******************************************************************************/


#ifndef __BOOL_ENC_CLIENT_PRIVATE_H__
#define __BOOL_ENC_CLIENT_PRIVATE_H__


#include "BoolEncClient.h" 

#include "BaseEnc.h" 
#include "BaseEnc_private.h" 
#include "enc/bool/BoolEnc_private.h"

#include "compile/symb_table/SymbTable.h"

#include "utils/utils.h" 


/**Struct**********************************************************************

  Synopsis    [BoolEncClient class definition derived from
               class BaseEnc]

  Description []

  SeeAlso     [Base class BaseEnc]   
  
******************************************************************************/
typedef struct BoolEncClient_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(BaseEnc); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  BoolEnc_ptr bool_enc;

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} BoolEncClient;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
void bool_enc_client_init ARGS((BoolEncClient_ptr self, 
                                SymbTable_ptr symb_table, 
                                BoolEnc_ptr bool_enc));

void bool_enc_client_deinit ARGS((BoolEncClient_ptr self));

void bool_enc_client_commit_layer ARGS((BaseEnc_ptr base_enc, 
                                        const char* layer_name));

void bool_enc_client_remove_layer ARGS((BaseEnc_ptr base_enc, 
                                        const char* layer_name));

#endif /* __BOOL_ENC_CLIENT_PRIVATE_H__ */
