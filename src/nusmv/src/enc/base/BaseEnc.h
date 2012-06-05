/**CHeaderFile*****************************************************************

  FileName    [BaseEnc.h]

  PackageName [enc.base]

  Synopsis    [Public interface of class 'BaseEnc']

  Description []

  SeeAlso     [BaseEnc.c]

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

  Revision    [$Id: BaseEnc.h,v 1.1.2.4.6.3 2009-09-17 11:49:47 nusmv Exp $]

******************************************************************************/


#ifndef __BASE_ENC_H__
#define __BASE_ENC_H__

#include "compile/symb_table/SymbTable.h"

#include "utils/object.h" 
#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class BaseEnc]

  Description []

******************************************************************************/
typedef struct BaseEnc_TAG*  BaseEnc_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class BaseEnc]

  Description [These macros must be used respectively to cast and to check
  instances of class BaseEnc]

******************************************************************************/
#define BASE_ENC(self) \
         ((BaseEnc_ptr) self)

#define BASE_ENC_CHECK_INSTANCE(self) \
         (nusmv_assert(BASE_ENC(self) != BASE_ENC(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN SymbTable_ptr BaseEnc_get_symb_table ARGS((const BaseEnc_ptr self));

EXTERN TypeChecker_ptr 
BaseEnc_get_type_checker ARGS((const BaseEnc_ptr self));

EXTERN boolean 
BaseEnc_layer_occurs ARGS((const BaseEnc_ptr self, const char* layer_name));

EXTERN NodeList_ptr 
BaseEnc_get_committed_layers ARGS((const BaseEnc_ptr self));

EXTERN const array_t* 
BaseEnc_get_committed_layer_names ARGS((BaseEnc_ptr self));


EXTERN VIRTUAL void BaseEnc_destroy ARGS((BaseEnc_ptr self));

EXTERN VIRTUAL void BaseEnc_commit_layer ARGS((BaseEnc_ptr self, 
                                               const char* layer_name));

EXTERN VIRTUAL void BaseEnc_remove_layer ARGS((BaseEnc_ptr self, 
                                               const char* layer_name));

/**AutomaticEnd***************************************************************/



#endif /* __BASE_ENC_H__ */
