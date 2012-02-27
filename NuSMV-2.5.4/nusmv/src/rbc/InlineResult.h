
/**CHeaderFile*****************************************************************

  FileName    [InlineResult.h]

  PackageName [rbc]

  Synopsis    [Public interface of class 'InlineResult']

  Description []

  SeeAlso     [InlineResult.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2. 
  Copyright (C) 2007 by FBK-irst. 

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

  Revision    [$Id: InlineResult.h,v 1.1.2.2 2007-03-08 12:49:09 nusmv Exp $]

******************************************************************************/


#ifndef __INLINE_RESULT_H__
#define __INLINE_RESULT_H__

#include "rbc.h"
#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class InlineResult]

  Description []

******************************************************************************/
typedef struct InlineResult_TAG*  InlineResult_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class InlineResult]

  Description [These macros must be used respectively to cast and to check
  instances of class InlineResult]

******************************************************************************/
#define INLINE_RESULT(self) \
         ((InlineResult_ptr) self)

#define INLINE_RESULT_CHECK_INSTANCE(self) \
         (nusmv_assert(INLINE_RESULT(self) != INLINE_RESULT(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN InlineResult_ptr 
InlineResult_create ARGS((Rbc_Manager_t* mgr, Rbc_t* f));

EXTERN void InlineResult_destroy ARGS((InlineResult_ptr self));

EXTERN InlineResult_ptr 
InlineResult_copy ARGS((const InlineResult_ptr self));

EXTERN Rbc_t* 
InlineResult_get_original_f ARGS((InlineResult_ptr self));

EXTERN Rbc_t* 
InlineResult_get_inlined_f ARGS((InlineResult_ptr self));

EXTERN Rbc_t* 
InlineResult_get_c ARGS((InlineResult_ptr self));

EXTERN Rbc_t* 
InlineResult_get_inlined_f_and_c ARGS((InlineResult_ptr self));


/**AutomaticEnd***************************************************************/



#endif /* __INLINE_RESULT_H__ */
