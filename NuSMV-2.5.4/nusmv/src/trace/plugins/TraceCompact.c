/**CFile***********************************************************************

  FileName    [TraceCompact.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TraceCompact object.]

  Description [ This file contains the definition of \"TraceCompact\"
  class.]

  SeeAlso     []

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2.
  Copyright (C) 2011 by FBK-irst.

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


#include "TraceCompact_private.h"
#include "TraceCompact.h"
#include "TracePlugin.h"
#include "trace/pkg_traceInt.h"
#include "fsm/bdd/BddFsm.h"
#include "parser/symbols.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void trace_compact_finalize ARGS((Object_ptr object, void* dummy));

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates an Compact Plugin and initializes it.]

  Description [Compact plugin constructor. ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceCompact_ptr TraceCompact_create()
{
  TraceCompact_ptr self = ALLOC(TraceCompact, 1);

  TRACE_COMPACT_CHECK_INSTANCE(self);

  trace_compact_init(self);
  return self;
}

/* ---------------------------------------------------------------------- */
/*     Protected Methods                                                  */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    [Initializes trace compact object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_compact_init(TraceCompact_ptr self)
{
  trace_plugin_init(TRACE_PLUGIN(self), "TRACE COMPACT PLUGIN - Shows the trace in a compact tabular fashion");

  OVERRIDE(Object, finalize) = trace_compact_finalize;
  OVERRIDE(TracePlugin, action) = trace_compact_action;
}


/**Function********************************************************************

  Synopsis    [Deinitializes Explain object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_compact_deinit(TraceCompact_ptr self)
{
  trace_plugin_deinit(TRACE_PLUGIN(self));
}


/**Function********************************************************************

  Synopsis    [Action method associated with TraceCompact class.]

  Description [ The action associated with TraceCompact is to print the trace
  in the specified file in compact format.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int trace_compact_action(const TracePlugin_ptr plugin)

{
  Trace_ptr trace = plugin->trace;
  TraceIter start_iter;
  TraceIter stop_iter;
  TraceIter step;

  TraceIteratorType input_iter_type;
  TraceIteratorType state_iter_type;
  TraceIteratorType combo_iter_type;
  TraceSymbolsIter sym_iter;

  int i;
  node_ptr sym;

  FILE* out = TraceOpt_output_stream(plugin->opt);

  start_iter = (0 != TraceOpt_from_here(plugin->opt))
    ? trace_ith_iter(trace, TraceOpt_from_here(plugin->opt))
    : trace_first_iter(trace);

  stop_iter = (0 != TraceOpt_to_here(plugin->opt))
    ? trace_ith_iter(trace, 1 + TraceOpt_to_here(plugin->opt))
    : TRACE_END_ITER;

  input_iter_type = TraceOpt_show_defines(plugin->opt)
    ? TRACE_ITER_I_SYMBOLS : TRACE_ITER_I_VARS;

  state_iter_type = TraceOpt_show_defines(plugin->opt)
    ? TRACE_ITER_SF_SYMBOLS : TRACE_ITER_SF_VARS;

  combo_iter_type = TraceOpt_show_defines(plugin->opt)
    ? TRACE_ITER_SI_DEFINES : TRACE_ITER_NONE;

  { /* ----- prints the header: first inputs, then states and comb ----- */
    fprintf(out, "Steps\\Vars\t");

    TRACE_SYMBOLS_FOREACH(trace, input_iter_type, sym_iter, sym) {
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

      TracePlugin_print_symbol(plugin, sym);
      fprintf(out, "\t");
    }
    TRACE_SYMBOLS_FOREACH(trace, state_iter_type, sym_iter, sym) {
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

      TracePlugin_print_symbol(plugin, sym);
      fprintf(out, "\t");
    }
    TRACE_SYMBOLS_FOREACH(trace, combo_iter_type, sym_iter, sym) {
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

      TracePlugin_print_symbol(plugin, sym);
      fprintf(out, "\t");
    }
    fprintf(out,"\n");
  } /* header */

  i = MAX(1, TraceOpt_from_here(plugin->opt)); step = start_iter;
  while (stop_iter != step) {

    fprintf(out, "Step%d\t", i);

    /* lazy defines evaluation */
    if (TraceOpt_show_defines(plugin->opt)) {
      trace_step_evaluate_defines(trace, step);
    }

    TRACE_SYMBOLS_FOREACH (trace, input_iter_type, sym_iter, sym) {
      node_ptr val = Trace_step_get_value(trace, step, sym);

      if (Nil != val) { TracePlugin_print_symbol(plugin, val); }
      else { fprintf(out, "-"); }

      fprintf(out, "\t");
    }
    
    TRACE_SYMBOLS_FOREACH(trace, state_iter_type, sym_iter, sym) {
      node_ptr val = Trace_step_get_value(trace, step, sym);
      
      if (Nil != val) { TracePlugin_print_symbol(plugin, val); }
      else { fprintf(out, "-"); }
      
      fprintf(out, "\t");
    }

    TRACE_SYMBOLS_FOREACH(trace, combo_iter_type, sym_iter, sym) {
      node_ptr val = Trace_step_get_value(trace, step, sym);
      
      if (Nil != val) { TracePlugin_print_symbol(plugin, val); }
      else { fprintf(out, "-"); }
      
      fprintf(out, "\t");
    }

    fprintf(out, "\n");
    ++ i; step = TraceIter_get_next(step);
  } /* trace printout */

  return 0;
}

/*---------------------------------------------------------------------------*/
/* Static functions                                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Trace Compact finalize method.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_compact_finalize(Object_ptr object, void* dummy)
{
  TraceCompact_ptr self = TRACE_COMPACT(object);

  trace_compact_deinit(self);
  FREE(self);
}
