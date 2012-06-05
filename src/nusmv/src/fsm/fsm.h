/**CHeaderFile*****************************************************************

  FileName    [fsm.h]

  PackageName [fsm]

  Synopsis    [Public interfaces for package fsm]

  Description []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm'' package of NuSMV version 2. 
  Copyright (C) 2008 by FBK-irst. 

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

  Revision    [$Id: fsm.h,v 1.1.4.1 2008-10-02 09:46:51 nusmv Exp $]

******************************************************************************/

#ifndef __FSM_H__
#define __FSM_H__


#include "utils/utils.h"


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void Fsm_init ARGS((void));
EXTERN void Fsm_quit ARGS((void));

#endif /* __FSM_INT_H__ */
