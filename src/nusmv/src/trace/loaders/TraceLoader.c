/**CFile***********************************************************************

  FileName    [TraceLoader.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TraceLoader object.]

  Description [This file contains the definition of \"TraceLoader\" class.]

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.loaders'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK.

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
#include "pkg_trace.h"
#include "pkg_traceInt.h"

#include "TraceLoader.h"
#include "TraceLoader_private.h"

#include "compile/compile.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: $";

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void trace_loader_finalize ARGS((Object_ptr object, void *dummy));

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Action associated with the Class TraceLoader.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Trace_ptr TraceLoader_load_trace(TraceLoader_ptr self, const SymbTable_ptr st,
                                 const NodeList_ptr symbols)
{
  TRACE_LOADER_CHECK_INSTANCE(self);

  return self->load(self, st, symbols);
}


/**Function********************************************************************

  Synopsis    [Returns a short description of the loader.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
char* TraceLoader_get_desc(const TraceLoader_ptr self)
{
  TRACE_LOADER_CHECK_INSTANCE(self);

  return self->desc;
}

/**Function********************************************************************

  Synopsis    [Action associated with the Class action.]

  Description [It is a pure virtual function and TraceLoader is an abstract
  base class. Every derived class must ovewrwrite this function. It returns 1
  if operation is successful, 0 otherwise.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Trace_ptr trace_loader_load(TraceLoader_ptr self, const SymbTable_ptr st,
                            const NodeList_ptr symbols)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
  return 0;
}


/**Function********************************************************************

  Synopsis    [This function initializes the loader class.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_loader_init(TraceLoader_ptr self, char* desc)
{
  object_init(OBJECT(self));

  OVERRIDE(Object, finalize) = trace_loader_finalize;
  OVERRIDE(TraceLoader, load) = trace_loader_load;

  self->desc = ALLOC(char, strlen(desc) + 1);
  nusmv_assert(self->desc != (char*) NULL);
  strncpy(self->desc, desc, strlen(desc) + 1);

  self->err = nusmv_stderr;
}

/**Function********************************************************************

  Synopsis    [This function de-initializes the loader class.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_loader_deinit(TraceLoader_ptr self)
{
  FREE(self->desc);
  object_deinit(OBJECT(self));
}

/*---------------------------------------------------------------------------*/
/* Static functions                                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Finalize method of loader class.]

  Description [Pure virtual function. This must be refined by derived classes.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_loader_finalize(Object_ptr object, void* dummy)
{
  TraceLoader_ptr self = TRACE_LOADER(object);

  trace_loader_deinit(self);
  error_unreachable_code();
}

