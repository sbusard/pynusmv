/**CHeaderFile*****************************************************************

  FileName    [TracePlugin.h]

  PackageName [trace.plugins]

  Synopsis    [The header file for the TracePlugin class.]

  Description [This Class implements a generic Plugin class.]

  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2.
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
#ifndef __TRACE_PLUGIN__H
#define __TRACE_PLUGIN__H

#include "trace/TraceOpt.h"
#include "trace/Trace.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct TracePlugin_TAG* TracePlugin_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE_PLUGIN(x) \
         ((TracePlugin_ptr) x)

#define TRACE_PLUGIN_CHECK_INSTANCE(x) \
         (nusmv_assert(TRACE_PLUGIN(x) != TRACE_PLUGIN(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN VIRTUAL int
TracePlugin_action ARGS((const TracePlugin_ptr self, const Trace_ptr trace,
                         const TraceOpt_ptr opt));

EXTERN char* TracePlugin_get_desc ARGS((const TracePlugin_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_PLUGIN__H */

