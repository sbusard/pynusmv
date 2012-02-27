/**CHeaderFile*****************************************************************

  FileName    [PartialTraceExecutor.h]

  PackageName [trace.exec]

  Synopsis    [Public interface of class 'PartialTraceExecutor']

  Description []

  SeeAlso     [PartialTraceExecutor.c]

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.exec'' package of NuSMV version 2.
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


#ifndef __PARTIAL_TRACE_EXECUTOR_H__
#define __PARTIAL_TRACE_EXECUTOR_H__


#include "BaseTraceExecutor.h" /* fix this */
#include "utils/utils.h"

#include "trace/Trace.h"

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class PartialTraceExecutor]

  Description []

******************************************************************************/
typedef struct PartialTraceExecutor_TAG*  PartialTraceExecutor_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class PartialTraceExecutor]

  Description [These macros must be used respectively to cast and to check
  instances of class PartialTraceExecutor]

******************************************************************************/
#define PARTIAL_TRACE_EXECUTOR(self) \
         ((PartialTraceExecutor_ptr) self)

#define PARTIAL_TRACE_EXECUTOR_CHECK_INSTANCE(self) \
         (nusmv_assert(PARTIAL_TRACE_EXECUTOR(self) != PARTIAL_TRACE_EXECUTOR(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN Trace_ptr PartialTraceExecutor_execute
ARGS((const PartialTraceExecutor_ptr self, const Trace_ptr trace,
      const NodeList_ptr language, int* n_steps));

/**AutomaticEnd***************************************************************/



#endif /* __PARTIAL_TRACE_EXECUTOR_H__ */
