/**CHeaderFile*****************************************************************

  FileName    [CompleteTraceExecutor.h]

  PackageName [trace.exec]

  Synopsis    [Public interface of class 'CompleteTraceExecutor']

  Description []

  SeeAlso     [CompleteTraceExecutor.c]

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
#ifndef __COMPLETE_TRACE_EXECUTOR_H__
#define __COMPLETE_TRACE_EXECUTOR_H__

#include "BaseTraceExecutor.h"
#include "utils/utils.h"

#include "trace/Trace.h"

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class CompleteTraceExecutor]

  Description []

******************************************************************************/
typedef struct CompleteTraceExecutor_TAG*  CompleteTraceExecutor_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class CompleteTraceExecutor]

  Description [These macros must be used respectively to cast and to check
  instances of class CompleteTraceExecutor]

******************************************************************************/
#define COMPLETE_TRACE_EXECUTOR(self) \
         ((CompleteTraceExecutor_ptr) self)

#define COMPLETE_TRACE_EXECUTOR_CHECK_INSTANCE(self) \
         (nusmv_assert(COMPLETE_TRACE_EXECUTOR(self) != COMPLETE_TRACE_EXECUTOR(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN boolean CompleteTraceExecutor_execute
ARGS((const CompleteTraceExecutor_ptr self, const Trace_ptr trace,
      int* n_steps));

/**AutomaticEnd***************************************************************/



#endif /* __COMPLETE_TRACE_EXECUTOR_H__ */
