/**CFile***********************************************************************

  FileName    [TraceLoader_private.c]

  PackageName [trace.loaders]

  Synopsis    [The private interface of class TraceLoader]

  Description [Private definition to be used by derived classes]

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
#ifndef __TRACE_LOADER_PRIVATE__H
#define __TRACE_LOADER_PRIVATE__H

#include "TraceLoader.h"
#include "compile/compile.h"

#include "utils/utils.h"
#include "utils/object.h"
#include "utils/object_private.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [TraceLoader Class]

  Description [This class defines a prototype for a generic
               TraceLoader. This class is virtual and must be
               specialized.]

  SeeAlso     []

******************************************************************************/
typedef struct TraceLoader_TAG
{
  INHERITS_FROM(Object);

  char* desc; /* Short description of the loader */

  FILE* err; /* error messages stream */
  /* ---------------------------------------------------------------------- */
  /*     Virtual Methods                                                    */
  /* ---------------------------------------------------------------------- */

  /* action */
  VIRTUAL Trace_ptr (*load)(TraceLoader_ptr self, const SymbTable_ptr st,
                            const NodeList_ptr symbols);

} TraceLoader;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void trace_loader_init ARGS((TraceLoader_ptr self, char* desc));

EXTERN void trace_loader_deinit ARGS((TraceLoader_ptr self));

EXTERN Trace_ptr trace_loader_load ARGS((TraceLoader_ptr self,
                                         const SymbTable_ptr st,
                                         const NodeList_ptr symbols));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_LOADER_PRIVATE__H */
