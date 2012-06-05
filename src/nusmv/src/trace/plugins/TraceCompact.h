/**CHeaderFile*****************************************************************

  FileName    [TraceCompact.h]

  PackageName [trace.plugins]

  Synopsis    [The header file for the TraceCompact class.]

  Description []

  SeeAlso     []

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2. 
  Copyright (C) 2011 by FBK-irst.

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
#ifndef __TRACE_COMPACT__H
#define __TRACE_COMPACT__H

#include "TracePlugin.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct TraceCompact_TAG* TraceCompact_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE_COMPACT(x) \
	 ((TraceCompact_ptr) x)

#define TRACE_COMPACT_CHECK_INSTANCE(x) \
	 (nusmv_assert(TRACE_COMPACT(x) != TRACE_COMPACT(NULL)))

/**AutomaticStart*************************************************************/ 
/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN TraceCompact_ptr TraceCompact_create ARGS((void));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_COMPACT__H */

