/**CHeaderFile*****************************************************************

  FileName    [CheckerBase.h]

  PackageName [compile.type_checking.checkers]

  Synopsis    [Public interface of class 'CheckerBase']

  Description []

  SeeAlso     [CheckerBase.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile.type_checking.checkers'' 
  package of NuSMV version 2. Copyright (C) 2006 by FBK-irst. 

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

  Revision    [$Id: CheckerBase.h,v 1.1.2.2.6.1 2007-04-04 12:00:33 nusmv Exp $]

******************************************************************************/


#ifndef __CHECKER_BASE_H__
#define __CHECKER_BASE_H__

#include "node/node.h"
#include "node/NodeWalker.h"

#include "compile/symb_table/SymbType.h"
#include "utils/object.h" 
#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class CheckerBase]

  Description []

******************************************************************************/
typedef struct CheckerBase_TAG*  CheckerBase_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class CheckerBase]

  Description [These macros must be used respectively to cast and to check
  instances of class CheckerBase]

******************************************************************************/
#define CHECKER_BASE(self) \
         ((CheckerBase_ptr) self)

#define CHECKER_BASE_CHECK_INSTANCE(self) \
         (nusmv_assert(CHECKER_BASE(self) != CHECKER_BASE(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN VIRTUAL SymbType_ptr
CheckerBase_check_expr ARGS((CheckerBase_ptr self, 
			     node_ptr expr, node_ptr context));



/**AutomaticEnd***************************************************************/


#endif /* __CHECKER_BASE_H__ */
