/**CHeaderFile*****************************************************************

  FileName    [BDDCompleteTraceExecutor_private.h]

  PackageName [trace.exec]

  Synopsis    [Private and protected interface of class 'BDDCompleteTraceExecutor']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [BDDCompleteTraceExecutor.h]

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


#ifndef __BDD_COMPLETE_TRACE_EXECUTOR_PRIVATE_H__
#define __BDD_COMPLETE_TRACE_EXECUTOR_PRIVATE_H__


#include "BDDCompleteTraceExecutor.h"
#include "CompleteTraceExecutor.h" /* fix this */
#include "CompleteTraceExecutor_private.h" /* fix this */
#include "utils/utils.h"


/**Struct**********************************************************************

  Synopsis    [BDDCompleteTraceExecutor class definition derived from
               class CompleteTraceExecutor]

  Description []

  SeeAlso     [Base class CompleteTraceExecutor]

******************************************************************************/
typedef struct BDDCompleteTraceExecutor_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(CompleteTraceExecutor);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  BddFsm_ptr fsm;
  BddEnc_ptr enc;

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} BDDCompleteTraceExecutor;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void
bdd_complete_trace_executor_init ARGS((BDDCompleteTraceExecutor_ptr self,
                                 const BddFsm_ptr fsm, const BddEnc_ptr enc));

EXTERN void
bdd_complete_trace_executor_deinit ARGS((BDDCompleteTraceExecutor_ptr self));

#endif /* __BDD_COMPLETE_TRACE_EXECUTOR_PRIVATE_H__ */
