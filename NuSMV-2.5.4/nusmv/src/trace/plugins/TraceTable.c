/**CFile***********************************************************************

  FileName    [TraceTable.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TraceTable object.]

  Description [ This file contains the definition of \"TraceTable\"
  class.]

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


#include "TraceTable_private.h"
#include "TraceTable.h"
#include "TracePlugin.h"
#include "trace/pkg_traceInt.h"
#include "fsm/bdd/BddFsm.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: TraceTable.c,v 1.1.2.20.4.5.6.11 2010-02-12 16:25:48 nusmv Exp $";

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
static void trace_table_finalize ARGS((Object_ptr object, void* dummy));

static int trace_table_print_row_style ARGS((const TraceTable_ptr self));

static int trace_table_print_column_style ARGS((const TraceTable_ptr self));

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates an Table Plugin and initializes it.]

  Description [Table plugin constructor. As arguments it takes variable style
  which decides the style of printing the trace. The possible values of the
  style variable may be: TRACE_TABLE_TYPE_ROW and TRACE_TABLE_TYPE_COLUMN.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceTable_ptr TraceTable_create(TraceTableStyle style)
{
  TraceTable_ptr self = ALLOC(TraceTable, 1);

  TRACE_TABLE_CHECK_INSTANCE(self);

  trace_table_init(self, style);
  return self;
}

/* ---------------------------------------------------------------------- */
/*     Protected Methods                                                  */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    [Initializes trace table object.]

  Description [As arguments it takes variable /"style /" to print the trace.
  The possible values for the style may be : TRACE_TABLE_TYPE_ROW and
  TRACE_TABLE_TYPE_COLUMN.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_table_init(TraceTable_ptr self, TraceTableStyle style)
{
  if (style == TRACE_TABLE_TYPE_COLUMN) {
    trace_plugin_init(TRACE_PLUGIN(self),
                      "TRACE TABLE PLUGIN - symbols on column");
  }
  else {
    trace_plugin_init(TRACE_PLUGIN(self),
                      "TRACE TABLE PLUGIN - symbols on row");
  }

  OVERRIDE(Object, finalize) = trace_table_finalize;
  OVERRIDE(TracePlugin, action) = trace_table_action;

  self->style = style;
}


/**Function********************************************************************

  Synopsis    [Deinitializes Explain object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_table_deinit(TraceTable_ptr self)
{
  trace_plugin_deinit(TRACE_PLUGIN(self));
}


/**Function********************************************************************

  Synopsis    [Action method associated with TraceTable class.]

  Description [ The action associated with TraceTable is to print the trace
  in the specified file in table format. There are two ways a trace can be
  printed: i). where states are listed row-wise. ii) Where states are listed
  column-wise. This depends on the style variable assoicated with the plugin.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int trace_table_action(const TracePlugin_ptr plugin)

{
  const TraceTable_ptr self = TRACE_TABLE(plugin);

  return (self->style == TRACE_TABLE_TYPE_COLUMN)
    ? trace_table_print_column_style(self)
    : trace_table_print_row_style(self);
}

/*---------------------------------------------------------------------------*/
/* Static functions                                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Print Trace in Table format with each symbols on a seperate
  column.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static int
trace_table_print_column_style(const TraceTable_ptr self)
{
  const TracePlugin_ptr plugin = TRACE_PLUGIN(self);
  Trace_ptr trace = plugin->trace;

  TraceIter start_iter;
  TraceIter stop_iter;
  TraceIter step;

  TraceIteratorType input_iter_type;
  TraceIteratorType state_iter_type;
  TraceIteratorType combo_iter_type;
  TraceSymbolsIter sym_iter;

  int i,j;
  node_ptr sym;

  unsigned n_s_symbs = 0;
  unsigned n_si_symbs = 0;
  unsigned n_i_symbs = 0;

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

  { /* ----- prints the header: first states, then comb and inputs ----- */
    fprintf(out, "Name\t");

    TRACE_SYMBOLS_FOREACH(trace, state_iter_type, sym_iter, sym) {
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

      ++ n_s_symbs;
      TracePlugin_print_symbol(plugin, sym);
      fprintf(out, "\t");
    }
    TRACE_SYMBOLS_FOREACH(trace, combo_iter_type, sym_iter, sym) {
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

      ++ n_si_symbs;
      TracePlugin_print_symbol(plugin, sym);
      fprintf(out, "\t");
    }
    TRACE_SYMBOLS_FOREACH(trace, input_iter_type, sym_iter, sym) {
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

      ++ n_i_symbs;
      TracePlugin_print_symbol(plugin, sym);
      fprintf(out, "\t");
    }
    fprintf(out,"\n");
  } /* header */

  i = MAX(1, TraceOpt_from_here(plugin->opt)); step = start_iter;
  while (stop_iter != step) {

    /* lazy defines evaluation */
    if (TraceOpt_show_defines(plugin->opt)) {
      trace_step_evaluate_defines(trace, step);
    }

    /* skip COMBO and INPUT on the first step */
    if (Trace_first_iter(trace) != step) {
      fprintf(out, "C%d\t", i);

      for (j=0; j < n_s_symbs; ++j) { fprintf(out, "-\t"); }
      TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_SI_DEFINES, sym_iter, sym) {
        /* skip non-visible symbols */
        if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

        node_ptr val = Trace_step_get_value(trace, step, sym);

        if (Nil != val) { TracePlugin_print_symbol(plugin, val); }
        else { fprintf(out, "-"); }

        fprintf(out, "\t");
      } /* COMBO */
      for (j=0; j < n_i_symbs; ++j) { fprintf(out, "-\t"); }
      fprintf(out, "\n");

      fprintf(out, "I%d\t", i);
      for (j=0; j < (n_s_symbs + n_si_symbs); ++j) { fprintf(out, "-\t"); }
      TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_I_SYMBOLS, sym_iter, sym) {
        /* skip non-visible symbols */
        if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

        node_ptr val = Trace_step_get_value(trace, step, sym);

        if (Nil != val) { TracePlugin_print_symbol(plugin, val); }
        else { fprintf(out, "-"); }
        fprintf(out, "\t");

      } /* INPUT */
      fprintf(out, "\n");
    }

    fprintf(out, "S%d\t", i);
    TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_SF_SYMBOLS, sym_iter, sym) {
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

      node_ptr val = Trace_step_get_value(trace, step, sym);

      if (Nil != val) { TracePlugin_print_symbol(plugin, val); }
      else { fprintf(out, "-"); }

      fprintf(out, "\t");
    } /* STATE */
    for (j=0; j < n_i_symbs + n_si_symbs; ++j) { fprintf(out, "-\t"); }
    fprintf(out, "\n");

    ++ i; step = TraceIter_get_next(step);
  } /* loop on steps */

  return 0;
}


/**Function********************************************************************

  Synopsis    [Print Trace in Table format with each symbol on a seperate
  row.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static int
trace_table_print_row_style(const TraceTable_ptr self)
{
  const TracePlugin_ptr plugin = TRACE_PLUGIN(self);
  Trace_ptr trace = plugin->trace;

  TraceIter start_iter;
  TraceIter stop_iter;
  TraceIter step;

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

  /* ----- prints the header: first states, then comb and inputs ----- */
  fprintf(out, "Step\t");

  i = MAX(1, TraceOpt_from_here(plugin->opt)); step = start_iter;
  while (stop_iter != step) {

    /* lazy defines evaluation */
    if (TraceOpt_show_defines(plugin->opt)) {
      trace_step_evaluate_defines(trace, step);
    }

    /* skip COMBO and INPUT on the first step */
    if (Trace_first_iter(trace) != step) {
      fprintf(out, "C%d\t", i);
      fprintf(out, "I%d\t", i);
    }

    fprintf(out, "S%d\t", i);
    ++ i; step = TraceIter_get_next(step);
  } /* loop on steps */
  fprintf(out,"\n");
  /* end of header */

  TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_SF_SYMBOLS, sym_iter, sym) {
    /* skip non-visible symbols */
    if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

    /* Print the symbol name */
    TracePlugin_print_symbol(plugin, sym); fprintf(out, "\t");

    step = start_iter;
    while (stop_iter != step) {
      node_ptr val = Trace_step_get_value(trace, step, sym);
    
      if (Nil != val) { TracePlugin_print_symbol(plugin, val); }
      else { fprintf(out, "-"); }
    
      step = TraceIter_get_next(step);

      if (stop_iter != step) {
        fprintf(out, "\t-\t-\t");
      }
    } /* loop on steps */

    fprintf(out,"\n");
  } /* STATE */

  TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_SI_DEFINES, sym_iter, sym) {
    /* skip non-visible symbols */
    if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

    /* Print the symbol name */
    TracePlugin_print_symbol(plugin, sym); fprintf(out, "\t");

    step = TraceIter_get_next(start_iter);
    while (stop_iter != step) {
      node_ptr val = Trace_step_get_value(trace, step, sym);
    
      fprintf(out, "-\t");
      if (Nil != val) { TracePlugin_print_symbol(plugin, val); }
      else { fprintf(out, "-"); }
    
      step = TraceIter_get_next(step);

      fprintf(out, "\t-\t");
    } /* loop on steps */

    fprintf(out,"-\n");
  } /* COMBO */


  TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_I_SYMBOLS, sym_iter, sym) {
    /* skip non-visible symbols */
    if (!trace_plugin_is_visible_symbol(plugin, sym)) continue;

    /* Print the symbol name */
    TracePlugin_print_symbol(plugin, sym); fprintf(out, "\t");

    step = TraceIter_get_next(start_iter);
    while (stop_iter != step) {
      node_ptr val = Trace_step_get_value(trace, step, sym);
    
      fprintf(out, "-\t-\t");
      if (Nil != val) { TracePlugin_print_symbol(plugin, val); }
      else { fprintf(out, "-"); }
    
      step = TraceIter_get_next(step);

      fprintf(out, "\t");
    } /* loop on steps */

    fprintf(out,"-\n");
  } /* INPUT */

  return 0;
}


/**Function********************************************************************

  Synopsis    [Trace Table finalize method.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_table_finalize(Object_ptr object, void* dummy)
{
  TraceTable_ptr self = TRACE_TABLE(object);

  trace_table_deinit(self);
  FREE(self);
}
