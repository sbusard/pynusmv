
/**CHeaderFile*****************************************************************

  FileName    [BoolEnc_private.h]

  PackageName [enc.bool]

  Synopsis    [Private and protected interface of class 'BoolEnc']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [BoolEnc.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.bool'' package of NuSMV version 2. 
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

  Revision    [$Id: BoolEnc_private.h,v 1.1.2.7.6.2 2008-01-15 14:26:14 nusmv Exp $]

******************************************************************************/


#ifndef __BOOL_ENC_PRIVATE_H__
#define __BOOL_ENC_PRIVATE_H__


#include "BoolEnc.h" 
#include "enc/base/BaseEnc_private.h" 

#include "utils/utils.h" 
#include "utils/assoc.h"


/**Struct**********************************************************************

  Synopsis    [BoolEnc class definition derived from class BaseEnc]

  Description []

  SeeAlso     [Base class BaseEnc]   
  
******************************************************************************/
typedef struct BoolEnc_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(BaseEnc); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  hash_ptr var2enc; /* var -> boolean encoding hash */
  hash_ptr var2mask; /* var -> mask encoding hash */

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} BoolEnc;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
void bool_enc_init ARGS((BoolEnc_ptr self, SymbTable_ptr symb_table));
void bool_enc_deinit ARGS((BoolEnc_ptr self));

void bool_enc_remove_layer ARGS((BaseEnc_ptr enc_base, 
                                 const char* layer_name));

void bool_enc_commit_layer ARGS((BaseEnc_ptr enc_base, 
                                 const char* layer_name));


#endif /* __BOOL_ENC_PRIVATE_H__ */
