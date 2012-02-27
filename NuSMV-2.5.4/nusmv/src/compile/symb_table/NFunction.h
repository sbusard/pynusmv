/**CHeaderFile*****************************************************************

  FileName    [NFunction.h]

  PackageName [utils]

  Synopsis    [Public interface of class 'NFunction']

  Description []

  SeeAlso     [NFunction.c]

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


#ifndef __N_FUNCTION_H__
#define __N_FUNCTION_H__


#include "utils/utils.h" 
#include "compile/symb_table/SymbType.h"
#include "utils/NodeList.h"

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class NFunction]

  Description []

******************************************************************************/
typedef struct NFunction_TAG*  NFunction_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class NFunction]

  Description [These macros must be used respectively to cast and to check
  instances of class NFunction]
  
******************************************************************************/
#define N_FUNCTION(self)                        \
  ((NFunction_ptr) self)

#define N_FUNCTION_CHECK_INSTANCE(self)                 \
  (nusmv_assert(N_FUNCTION(self) != N_FUNCTION(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN NFunction_ptr NFunction_create_uninterpreted ARGS((int num_args,
                                                          SymbType_ptr* args,
                                                          SymbType_ptr ret));

EXTERN NFunction_ptr NFunction_create_interpreted ARGS((int num_args,
                                                        SymbType_ptr* args,
                                                        SymbType_ptr ret,
                                                        void* body));

EXTERN void NFunction_destroy ARGS((NFunction_ptr self));

EXTERN int NFunction_get_args_number ARGS((NFunction_ptr self));

EXTERN NodeList_ptr NFunction_get_args ARGS((NFunction_ptr self));

EXTERN SymbType_ptr NFunction_get_return_type ARGS((NFunction_ptr self));

EXTERN SymbType_ptr NFunction_get_main_type ARGS((NFunction_ptr self));

EXTERN boolean NFunction_is_uninterpreted ARGS((NFunction_ptr self));

EXTERN void* NFunction_get_body ARGS((NFunction_ptr self));

/**AutomaticEnd***************************************************************/



#endif /* __N_FUNCTION_H__ */
