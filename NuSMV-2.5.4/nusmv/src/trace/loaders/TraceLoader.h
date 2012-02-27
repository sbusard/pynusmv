/**CHeaderFile*****************************************************************

  FileName    [TraceLoader.h]

  PackageName [trace.loaders]

  Synopsis    [The header file for the TraceLoader class.]

  Description [This Class implements a generic Loader class.]

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.loaders'' package of NuSMV version 2.
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

******************************************************************************/
#ifndef __TRACE_LOADER__H
#define __TRACE_LOADER__H

#include "trace/Trace.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct TraceLoader_TAG* TraceLoader_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE_LOADER(x) \
         ((TraceLoader_ptr) x)

#define TRACE_LOADER_CHECK_INSTANCE(x) \
         (nusmv_assert(TRACE_LOADER(x) != TRACE_LOADER(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN VIRTUAL Trace_ptr
TraceLoader_load_trace ARGS((const TraceLoader_ptr self, SymbTable_ptr st,
                             NodeList_ptr symbols));

EXTERN char* TraceLoader_get_desc ARGS((const TraceLoader_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_LOADER__H */

