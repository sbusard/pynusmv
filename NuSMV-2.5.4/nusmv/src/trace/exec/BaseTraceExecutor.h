/**CHeaderFile*****************************************************************

  FileName    [BaseTraceExecutor.h]

  PackageName [trace.exec]

  Synopsis    [Public interface of class 'BaseTraceExecutor']

  Description []

  SeeAlso     [BaseTraceExecutor.c]

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


#ifndef __TRACE_EXECUTOR_H__
#define __TRACE_EXECUTOR_H__

#include "utils/object.h"
#include "utils/utils.h"


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class BaseTraceExecutor]

  Description []

******************************************************************************/
typedef struct BaseTraceExecutor_TAG*  BaseTraceExecutor_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class BaseTraceExecutor]

  Description [These macros must be used respectively to cast and to check
  instances of class BaseTraceExecutor]

******************************************************************************/
#define BASE_TRACE_EXECUTOR(self) \
         ((BaseTraceExecutor_ptr) self)

#define BASE_TRACE_EXECUTOR_CHECK_INSTANCE(self) \
         (nusmv_assert(BASE_TRACE_EXECUTOR(self) != BASE_TRACE_EXECUTOR(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int
BaseTraceExecutor_get_verbosity ARGS((BaseTraceExecutor_ptr self));

EXTERN void
BaseTraceExecutor_set_verbosity ARGS((BaseTraceExecutor_ptr self,
                                  int verbosity));

EXTERN FILE*
BaseTraceExecutor_get_output_stream ARGS((BaseTraceExecutor_ptr self));

EXTERN void
BaseTraceExecutor_set_output_stream ARGS((BaseTraceExecutor_ptr self,
                                      FILE* error_stream));

EXTERN FILE*
BaseTraceExecutor_get_error_stream ARGS((BaseTraceExecutor_ptr self));

EXTERN void
BaseTraceExecutor_set_error_stream ARGS((BaseTraceExecutor_ptr self,
                                     FILE* error_stream));

/* virtual destructor */
VIRTUAL EXTERN void
BaseTraceExecutor_destroy ARGS((BaseTraceExecutor_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_EXECUTOR_H__ */
