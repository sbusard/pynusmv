/**CHeaderFile*****************************************************************

  FileName    [CompleteTraceExecutor_private.h]

  PackageName [trace.exec]

  Synopsis    [Private and protected interface of class 'CompleteTraceExecutor']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [CompleteTraceExecutor.h]

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
#ifndef __COMPLETE_TRACE_EXECUTOR_PRIVATE_H__
#define __COMPLETE_TRACE_EXECUTOR_PRIVATE_H__

#include "CompleteTraceExecutor.h"
#include "BaseTraceExecutor.h"
#include "BaseTraceExecutor_private.h"
#include "utils/utils.h"

/**Struct**********************************************************************

  Synopsis    [CompleteTraceExecutor class definition derived from
               class TraceExecutor]

  Description []

  SeeAlso     [Base class TraceExecutor]

******************************************************************************/
typedef struct CompleteTraceExecutor_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(BaseTraceExecutor);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */


  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

  /* executes the given trace, returns true iff the whole trace was
     succesfully executed. Number of succesful transitions if returned
     in *n_steps, if a non-NULL pointer is given. */
  VIRTUAL boolean
  (*execute)(const CompleteTraceExecutor_ptr self,
             const Trace_ptr trace, int* n_steps);

} CompleteTraceExecutor;

/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only        */
/* ---------------------------------------------------------------------- */
EXTERN void complete_trace_executor_init ARGS((CompleteTraceExecutor_ptr self));
EXTERN void complete_trace_executor_deinit ARGS((CompleteTraceExecutor_ptr self));

EXTERN boolean
complete_trace_executor_check_loopbacks ARGS((const CompleteTraceExecutor_ptr self,
                                              const Trace_ptr trace));

#endif /* __COMPLETE_TRACE_EXECUTOR_PRIVATE_H__ */
