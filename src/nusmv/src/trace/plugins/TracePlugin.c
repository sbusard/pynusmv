/**CFile***********************************************************************

  FileName    [TracePlugin.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TracePlugin object.]

  Description [This file contains the definition of \"TracePlugin\" class.]

  SeeAlso     []

  Author      [Ashutosh Trivedi, Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2.
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
#include "pkg_trace.h"
#include "pkg_traceInt.h"

#include "TracePlugin.h"
#include "TracePlugin_private.h"

#include "TraceManager.h"
#include "Trace_private.h"

#include "compile/compile.h"
#include "parser/symbols.h"

#include "utils/utils_io.h"

static char rcsid[] UTIL_UNUSED = "$Id: TracePlugin.c,v 1.1.2.9.4.1.6.3 2010-02-12 16:25:48 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void trace_plugin_finalize ARGS((Object_ptr object, void *dummy));
static void trace_plugin_prepare_action ARGS((TracePlugin_ptr self,
                                              Trace_ptr trace,
                                              TraceOpt_ptr opt));
static void trace_plugin_cleanup_action ARGS((TracePlugin_ptr self));
/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Action associated with the Class TracePlugin.]

  Description [Executes the different action method, corresponding to which
  derived class instance belongs to, on the trace.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int TracePlugin_action(const TracePlugin_ptr self, const Trace_ptr trace,
                       const TraceOpt_ptr opt)
{
  int res;
  unsigned from, to;

  TRACE_PLUGIN_CHECK_INSTANCE(self);
  TRACE_CHECK_INSTANCE(self);
  TRACE_OPT_CHECK_INSTANCE(opt);

  /* verify iterators sanity before dumping */
  from = TraceOpt_from_here(opt);
  to = TraceOpt_to_here(opt);
  if ((0 < from) && (0 < to) && (to < from)) {
    internal_error("%s:%d:%s: invalid range detected (%d-%d). Aborting dump",
                   __FILE__, __LINE__, __func__, from, to);
  }

  { /* protected block */
    trace_plugin_prepare_action(self, trace, opt);

    CATCH {
      res = self->action(self);
    }
    FAIL { res = -1; }

    trace_plugin_cleanup_action(self);
  } /* protected block */

  return res;
}


/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TracePlugin_print_symbol(const TracePlugin_ptr self, node_ptr symb)
{
  TRACE_PLUGIN_CHECK_INSTANCE(self);
  self->print_symbol(self, symb);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TracePlugin_print_list(const TracePlugin_ptr self, node_ptr list)
{
  TRACE_PLUGIN_CHECK_INSTANCE(self);
  self->print_list(self, list);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void TracePlugin_print_assignment(const TracePlugin_ptr self, node_ptr symb,
                                  node_ptr val)
{
  TRACE_PLUGIN_CHECK_INSTANCE(self);
  self->print_assignment(self, symb, val);
}

/**Function********************************************************************

  Synopsis    [Returns a short description of the plugin.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
char* TracePlugin_get_desc(const TracePlugin_ptr self)
{
  TRACE_PLUGIN_CHECK_INSTANCE(self);

  return self->desc;
}

/**Function********************************************************************

  Synopsis    [Action associated with the Class action.]

  Description [It is a pure virtual function and TracePlugin is an abstract
  base class. Every derived class must ovewrwrite this function. It returns 1
  if operation is successful, 0 otherwise.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int trace_plugin_action(const TracePlugin_ptr self)
{
  error_unreachable_code(); /* Pure Virtual Member Function */
  return 0;
}


void trace_plugin_print_symbol(const TracePlugin_ptr self, node_ptr val)
{
  SymbTable_ptr st = trace_get_symb_table(self->trace);

  print_node(TraceOpt_output_stream(self->opt),
             (hash_ptr)(NULL) != self->obfuscation_map
             ? Compile_obfuscate_expression(st, val, self->obfuscation_map)
             : val);
}

void trace_plugin_print_list(const TracePlugin_ptr self, node_ptr list)
{
  if (Nil == list) return;
  if (UNION == node_get_type(list)) {
    TracePlugin_print_symbol(self, car(list));
    fprintf(TraceOpt_output_stream(self->opt), ", ");
    TracePlugin_print_list(self, cdr(list));
  }
  else {
    TracePlugin_print_symbol(self, list);
  }
}

void trace_plugin_print_assignment(const TracePlugin_ptr self, node_ptr symb,
                                   node_ptr val)
{
  FILE* out = TraceOpt_output_stream(self->opt);

  indent(out);
  TracePlugin_print_symbol(self, symb);
  fprintf(out, " = ");
  TracePlugin_print_list(self, val);
  fprintf(out, "\n");
}


/**Function********************************************************************

  Synopsis    [This function initializes the plugin class.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_plugin_init(TracePlugin_ptr self, char* desc)
{
  object_init(OBJECT(self));

  OVERRIDE(Object, finalize) = trace_plugin_finalize;
  OVERRIDE(TracePlugin, action) = trace_plugin_action;
  OVERRIDE(TracePlugin, print_symbol) = trace_plugin_print_symbol;
  OVERRIDE(TracePlugin, print_list) = trace_plugin_print_list;
  OVERRIDE(TracePlugin, print_assignment) = trace_plugin_print_assignment;

  self->desc = ALLOC(char, strlen(desc) + 1);
  nusmv_assert(self->desc != (char*) NULL);
  strncpy(self->desc, desc, strlen(desc) + 1);

  /* action params initialization */
  self->trace = TRACE(NULL);
  self->opt =  TRACE_OPT(NULL);
  self->visibility_map = (hash_ptr)(NULL);
  self->obfuscation_map = (hash_ptr)(NULL);
}

/**Function********************************************************************

  Synopsis    [This function de-initializes the plugin class.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_plugin_deinit(TracePlugin_ptr self)
{
  FREE(self->desc);
  object_deinit(OBJECT(self));
}

/*---------------------------------------------------------------------------*/
/* Static functions                                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Finalize method of plugin class.]

  Description [Pure virtual function. This must be refined by derived classes.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_plugin_finalize(Object_ptr object, void* dummy)
{
  TracePlugin_ptr self = TRACE_PLUGIN(object);

  trace_plugin_deinit(self);
  error_unreachable_code();
}


/**Function********************************************************************

  Synopsis    [Check that node is printable]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean trace_plugin_is_visible_symbol(TracePlugin_ptr self, node_ptr symb)
{
  boolean res;
  TraceManager_ptr gtm = TracePkg_get_global_trace_manager();
  node_ptr lookup;

  /* private */
  const int SYMB_VISIBLE   = 1;
  const int SYMB_INVISIBLE = -1;

  nusmv_assert(Nil != symb);
  nusmv_assert((hash_ptr)(NULL) != self->visibility_map);

  lookup = find_assoc(self->visibility_map, symb);
  if (Nil != lookup) { return (SYMB_VISIBLE == NODE_TO_INT(lookup)); }

  res = TraceManager_is_visible_symbol(gtm, symb);

  insert_assoc(self->visibility_map, symb,
               NODE_FROM_INT((res) ? SYMB_VISIBLE : SYMB_INVISIBLE));

  return res;
}

static void trace_plugin_prepare_action(TracePlugin_ptr self, Trace_ptr trace,
                                        TraceOpt_ptr opt)
{
  /* 1. setup visibility map */
  nusmv_assert((hash_ptr)(NULL) == self->visibility_map);
  self->visibility_map = new_assoc();

  /* 2. register trace */
  nusmv_assert(TRACE(NULL) == self->trace);
  self->trace = trace;

  /* 3. create obfuscation map if required */
  nusmv_assert((hash_ptr)(NULL) == self->obfuscation_map);
  self->obfuscation_map = TraceOpt_obfuscate(opt)
    ? Compile_get_obfuscation_map(Trace_get_symb_table(trace))
    : (hash_ptr)(NULL);

  /* 4. register options */
  nusmv_assert(TRACE_OPT(NULL) == self->opt);
  self->opt = opt;
}

static void trace_plugin_cleanup_action(TracePlugin_ptr self)
{
  /* 1. dispose visibility map */
  nusmv_assert((hash_ptr)(NULL) != self->visibility_map);
  free_assoc(self->visibility_map);
  self->visibility_map = (hash_ptr)(NULL);

  /* 2. unregister trace */
  self->trace = TRACE(NULL);

  /* 3. dispose obfuscation map (if any) */
  if ((hash_ptr)(NULL) != self->obfuscation_map) {
    free(self->obfuscation_map);
    self->obfuscation_map = (hash_ptr)(NULL);
  }

  /* 4. unregisters options */
  self->opt = TRACE_OPT(NULL);
}


