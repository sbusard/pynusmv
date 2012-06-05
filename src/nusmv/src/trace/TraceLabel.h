/**CHeaderFile*****************************************************************

  FileName    [TraceLabel.h]

  PackageName [trace]

  Synopsis    [The header file for the TraceLabel class.]

  Description []

  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst.

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

******************************************************************************/
#ifndef __TRACE_LABEL__H
#define __TRACE_LABEL__H

#include "node/node.h"
#include "fsm/bdd/BddFsm.h" 

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef node_ptr TraceLabel;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE_LABEL(x) \
	  ((node_ptr) x)
	
#define TRACE_LABEL_CHECK_INSTANCE(x) \
	  (nusmv_assert(TRACE_LABEL(x) != TRACE_LABEL(NULL)))

#define TRACE_LABEL_INVALID Nil

/**AutomaticStart*************************************************************/ 
/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
/* TraceLabel Constructors */
EXTERN TraceLabel TraceLabel_create ARGS((int trace_id, int state_id));
EXTERN TraceLabel TraceLabel_create_from_string ARGS((const char* label_str));

/* TraceLabel Getters */
EXTERN int TraceLabel_get_state ARGS((TraceLabel self));
EXTERN int TraceLabel_get_trace ARGS((TraceLabel self));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_LABEL__H  */
