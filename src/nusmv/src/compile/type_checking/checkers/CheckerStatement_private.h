/**CHeaderFile*****************************************************************

  FileName    [CheckerStatement_private.h]

  PackageName [compile.type_checking.checkers]

  Synopsis    [Private and protected interface of class 'CheckerStatement']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [CheckerStatement.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile.type_checking.checkers'' package of NuSMV version 2. 
  Copyright (C) 2006 by FBK-irst. 

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

  Revision    [$Id: CheckerStatement_private.h,v 1.1.2.1.6.2 2007-07-05 07:31:56 nusmv Exp $]

******************************************************************************/


#ifndef __CHECKER_STATEMENT_PRIVATE_H__
#define __CHECKER_STATEMENT_PRIVATE_H__


#include "CheckerStatement.h" 

#include "CheckerCore.h" 
#include "CheckerCore_private.h"

#include "utils/utils.h" 


/**Struct**********************************************************************

  Synopsis    [CheckerStatement class definition derived from
               class CheckerCore]

  Description []

  SeeAlso     [Base class CheckerCore]   
  
******************************************************************************/
typedef struct CheckerStatement_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(CheckerCore); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  boolean inside_attime; /* to check ATTIME is not nested */

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} CheckerStatement;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void checker_statement_init ARGS((CheckerStatement_ptr self));
EXTERN void checker_statement_deinit ARGS((CheckerStatement_ptr self));



#endif /* __CHECKER_STATEMENT_PRIVATE_H__ */
