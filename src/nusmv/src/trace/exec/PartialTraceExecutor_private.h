/**CHeaderFile*****************************************************************

  FileName    [PartialTraceExecutor_private.h]

  PackageName [trace.exec]

  Synopsis    [Private and protected interface of class 'PartialTraceExecutor']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [PartialTraceExecutor.h]

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
#ifndef __PARTIAL_TRACE_EXECUTOR_PRIVATE_H__
#define __PARTIAL_TRACE_EXECUTOR_PRIVATE_H__

#include "PartialTraceExecutor.h"
#include "BaseTraceExecutor.h"
#include "BaseTraceExecutor_private.h"

#include "utils/utils.h"


/**Struct**********************************************************************

  Synopsis    [PartialTraceExecutor class definition derived from
               class TraceExecutor]

  Description []

  SeeAlso     [Base class TraceExecutor]

******************************************************************************/
typedef struct PartialTraceExecutor_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(BaseTraceExecutor);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

  /* executes the given trace, returns a complete trace complete on
   given language iff trace was succesfully completed. The number of
   performed steps is stored in *n_steps, if non-NULL (-1 if no
   initial feasible state exists) */
  VIRTUAL Trace_ptr
  (*execute)(const PartialTraceExecutor_ptr self, const Trace_ptr trace,
             NodeList_ptr language, int* n_steps);

} PartialTraceExecutor;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void partial_trace_executor_init ARGS((PartialTraceExecutor_ptr self));
EXTERN void partial_trace_executor_deinit ARGS((PartialTraceExecutor_ptr self));

EXTERN boolean
partial_trace_executor_check_loopbacks ARGS((const PartialTraceExecutor_ptr self,
                                             const Trace_ptr orig_trace,
                                             const Trace_ptr complete_trace));

EXTERN boolean
partial_trace_executor_is_complete_state ARGS((const PartialTraceExecutor_ptr self,
                                               const Trace_ptr trace,
                                               const TraceIter step));

#endif /* __PARTIAL_TRACE_EXECUTOR_PRIVATE_H__ */
