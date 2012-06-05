/**CHeaderFile*****************************************************************

  FileName    [BaseTraceExecutor_private.h]

  PackageName [trace.exec]

  Synopsis    [Private and protected interface of class 'BaseTraceExecutor']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [BaseTraceExecutor.h]

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


#ifndef __TRACE_EXECUTOR_PRIVATE_H__
#define __TRACE_EXECUTOR_PRIVATE_H__


#include "BaseTraceExecutor.h"
#include "trace/Trace.h"

#include "utils/object.h"
#include "utils/object_private.h"
#include "utils/utils.h"


/**Struct**********************************************************************

  Synopsis    [BaseTraceExecutor class definition derived from
               class Object]

  Description []

  SeeAlso     [Base class Object]

******************************************************************************/
typedef struct BaseTraceExecutor_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(Object);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */

  /* verbosity level */
  int verbosity;

  /* the output stream */
  FILE* output_stream;

  /* the error stream */
  FILE* error_stream;

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */
} BaseTraceExecutor;

/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void trace_executor_init ARGS((BaseTraceExecutor_ptr self));
EXTERN void trace_executor_deinit ARGS((BaseTraceExecutor_ptr self));

/* currently unused */
EXTERN boolean trace_executor_check_defines ARGS((const BaseTraceExecutor_ptr self,
                                                  Trace_ptr trace));

#endif /* __TRACE_EXECUTOR_PRIVATE_H__ */
