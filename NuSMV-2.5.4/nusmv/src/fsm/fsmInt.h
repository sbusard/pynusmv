/**CHeaderFile*****************************************************************

  FileName    [fsmInt.h]

  PackageName [fsm]

  Synopsis    [Internal interfaces for package fsm]

  Description []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm'' package of NuSMV version 2. 
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

  Revision    [$Id: fsmInt.h,v 1.1.2.1.6.1 2008-10-02 11:45:49 nusmv Exp $]

******************************************************************************/

#ifndef __FSM_INT_H__
#define __FSM_INT_H__


#include "FsmBuilder.h"

#include "compile/FlatHierarchy.h"
#include "opt/opt.h"
#include "utils/utils.h"


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN FsmBuilder_ptr global_fsm_builder; 
EXTERN FlatHierarchy_ptr mainFlatHierarchy;


EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;

EXTERN int yylineno;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

#endif /* __FSM_INT_H__ */
