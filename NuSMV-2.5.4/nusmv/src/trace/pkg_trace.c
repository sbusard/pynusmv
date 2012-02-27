/**CFile***********************************************************************

  FileName    [pkg_trace.c]

  PackageName [trace]

  Synopsis    [Routines related to the trace Package.]

  Description [This file contains routines related to Initializing and
               Quitting trace package. ]

  SeeAlso     []

  Author      [Ashutosh Trivedi]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.

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

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "pkg_trace.h"
#include "pkg_traceInt.h"
#include "parser/symbols.h"

#if NUSMV_HAVE_REGEX_H
 #include <regex.h>
#endif


static char rcsid[] UTIL_UNUSED = "$Id: pkg_trace.c,v 1.1.2.16.4.2.6.14 2010-02-12 16:25:47 nusmv Exp $";
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
TraceManager_ptr global_trace_manager = TRACE_MANAGER(NULL);

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Initializes the Trace Package.]

  Description [TraceManager get initialized. ]

  SideEffects []

  SeeAlso     [TracePkg_quit]

******************************************************************************/
void TracePkg_init()
{
  if (global_trace_manager != TRACE_MANAGER(NULL)) {
    TraceManager_destroy(global_trace_manager);
  }

  global_trace_manager = TraceManager_create();
  nusmv_assert(global_trace_manager != TRACE_MANAGER(NULL));

  TraceManager_init_plugins(global_trace_manager);

  TraceManager_register_evaluator(TracePkg_get_global_trace_manager(),
                                  BaseEvaluator_create());
}

/**Function********************************************************************

  Synopsis    [Quits the Trace package.]

  Description []

  SideEffects []

  SeeAlso     [TracePkg_init]

******************************************************************************/
void TracePkg_quit()
{
  if (global_trace_manager != TRACE_MANAGER(NULL)) {
  TraceManager_destroy(global_trace_manager);
  global_trace_manager = TRACE_MANAGER(NULL);
  }
}


/**Function********************************************************************

  Synopsis    [Accessor for the global trace manager]

  Description [Can be called only after initialization of the trace package]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceManager_ptr TracePkg_get_global_trace_manager()
{
  TRACE_MANAGER_CHECK_INSTANCE(global_trace_manager);

  return global_trace_manager;
}


/**Function********************************************************************

  Synopsis    [Called when the user selects a trace plugin to be used as
               default]

  Description [Returns true upon success, false otherwise]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean TracePkg_set_default_trace_plugin(int dp)
{
  TraceManager_ptr gtm = TracePkg_get_global_trace_manager();
  int avail_plugins = 1 + TraceManager_get_plugin_size(gtm);

  /* negative indexes not allowed */
  if (dp < 0) {
    fprintf(nusmv_stderr,"Error: Not a proper plugin to show a trace \n");
    return false;
  }

  /* dp must be one of avail plugins */
  if (avail_plugins < dp) {
    fprintf(nusmv_stderr,"Error: Plugin %d is not currently available\n", dp);
    return false;
  }

  TraceManager_set_default_plugin(gtm, dp);
  return true;
}


/**Function********************************************************************

  Synopsis    [Returns the trace plugin currently selected as default]

  Description [Returns the trace plugin currently selected as default]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TracePkg_get_default_trace_plugin()
{
  return TraceManager_get_default_plugin(TracePkg_get_global_trace_manager());
}


/**Function********************************************************************

  Synopsis    [Returns the filtered list of symbols]

  Description [Returned list is the result of filtering the input
  list, using standard filtering strategies that apply to symbols in
  traces.

  The returned list must be freed by the caller]

  SideEffects []

  SeeAlso     [TraceManager_is_visible_symbol]

******************************************************************************/
NodeList_ptr TracePkg_get_filtered_symbols (const NodeList_ptr symbols)
{
  TraceManager_ptr gtm = TracePkg_get_global_trace_manager();
  NodeList_ptr res = NodeList_create();
  ListIter_ptr iter;

  NODE_LIST_FOREACH(symbols, iter) {
    node_ptr symb = NodeList_get_elem_at(symbols, iter);
    if (TraceManager_is_visible_symbol(gtm, symb)) {
      NodeList_append(res, symb);
    }
  }

  return res;
}

