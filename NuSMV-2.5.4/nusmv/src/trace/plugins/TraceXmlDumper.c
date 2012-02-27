/**CFile***********************************************************************

  FileName    [TraceXmlDumper.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TraceXmlDumper class]

  Description [ This file contains the definition of TraceXmlDumper
  class.]

  SeeAlso     []

  Author      [Ashutosh Trivedi, Roberto Cavada, Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2.
  Copyright (C) 2004 by FBK-irst.

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

#include "TraceXmlDumper.h"
#include "TraceXmlDumper_private.h"
#include "trace/Trace_private.h"

#include "compile/symb_table/SymbTable.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: TraceXmlDumper.c,v 1.1.2.3.4.6.6.15 2010-02-12 16:25:48 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
static const char* TRACE_XML_VERSION_INFO_STRING =      \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void trace_xml_dumper_finalize ARGS((Object_ptr object, void* dummy));

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Action method associated with TraceXmlDumper class.]

  Description [ Given trace is written into the file pointed by
  given additional parameter ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int trace_xml_dumper_action(TracePlugin_ptr self)
{
  const Trace_ptr trace = self->trace;
  TraceIter start_iter;
  TraceIter stop_iter;
  TraceIter step;
  TraceIteratorType input_iter_type;
  TraceIteratorType state_iter_type;
  TraceIteratorType combo_iter_type;
  int i;

  NodeList_ptr loops = NodeList_create(); /* contains loops information */
  ListIter_ptr loops_iter;

  FILE* out = TraceOpt_output_stream(self->opt);

  boolean first_node = true;

  start_iter = (0 != TraceOpt_from_here(self->opt))
    ? trace_ith_iter(trace, TraceOpt_from_here(self->opt))
    : trace_first_iter(trace);

  /* safe way to skip one more step */
  stop_iter =
    (0 != TraceOpt_to_here(self->opt))
    ? trace_ith_iter(trace, 1 + TraceOpt_to_here(self->opt))
    : TRACE_END_ITER;

  input_iter_type = TraceOpt_show_defines(self->opt)
    ? TRACE_ITER_I_SYMBOLS : TRACE_ITER_I_VARS;

  state_iter_type = TraceOpt_show_defines(self->opt)
    ? TRACE_ITER_SF_SYMBOLS : TRACE_ITER_SF_VARS;

  combo_iter_type = TraceOpt_show_defines(self->opt)
    ? TRACE_ITER_COMBINATORIAL : TRACE_ITER_NONE;

  fprintf(out,"%s\n", TRACE_XML_VERSION_INFO_STRING);
  fprintf(out,"<%s type=\"%d\" desc=\"%s\" >\n",
          TRACE_XML_CNTX_TAG_STRING, Trace_get_type(trace),
          Trace_get_desc(trace));

  first_node = true;
  i = MAX(1, TraceOpt_from_here(self->opt)); step = start_iter;
  while (stop_iter != step) {
    TraceStepIter iter;
    node_ptr symb, val;

    boolean combo_header = false;
    boolean input_header = false;

    /* lazy defines evaluation */
    if (TraceOpt_show_defines(self->opt)) {
      trace_step_evaluate_defines(trace, step);
    }

    if (Trace_step_is_loopback(trace, step)) {
      NodeList_append(loops, NODE_FROM_INT(i));
    }

    TRACE_STEP_FOREACH(trace, step, combo_iter_type, iter, symb, val){
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(self, symb)) continue;

      if (false == combo_header) {
        fprintf(out, "\t\t<%s id=\"%d\">\n", TRACE_XML_COMB_TAG_STRING, i);
        combo_header = true;
      }

      TracePlugin_print_assignment(self, symb, val);
    } /* foreach COMBINATORIAL */

    if (combo_header) {
      fprintf(out, "\t\t</%s>\n", TRACE_XML_COMB_TAG_STRING);
    }

    TRACE_STEP_FOREACH(trace, step, input_iter_type, iter, symb, val) {
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(self, symb)) continue;

      if (false == input_header) {
        fprintf(out, "\t\t<%s id=\"%d\">\n", TRACE_XML_INPUT_TAG_STRING, i);
        input_header = true;
      }

      TracePlugin_print_assignment(self, symb, val);
    } /* foreach INPUT */

    if (input_header) {
      fprintf(out, "\t\t</%s>\n", TRACE_XML_INPUT_TAG_STRING);
    }

    /*   [MP] this representation does not match logical trace definition*/
    /* </node> */
    if (!first_node) {
      fprintf(out, "\t</%s>\n", TRACE_XML_NODE_TAG_STRING);
    }

    /*   [ATRC]: state and inputs may be dependent each other, so
                 values chosen her might be not consistent with values chosen
                 in comb part
         [ATRC]: Possible solution: use last parameter of BddEnc_assign_symbols */

    /* <node> */
    fprintf(out, "\t<%s>\n", TRACE_XML_NODE_TAG_STRING);  first_node = false;
    fprintf(out, "\t\t<%s id=\"%d\">\n", TRACE_XML_STATE_TAG_STRING,i);

    TRACE_STEP_FOREACH(trace, step, state_iter_type, iter, symb, val) {
      /* skip non-visible symbols */
      if (!trace_plugin_is_visible_symbol(self, symb)) continue;

      TracePlugin_print_assignment(self, symb, val);
    } /* foreach SF_SYMBOLS */

    fprintf(out, "\t\t</%s>\n", TRACE_XML_STATE_TAG_STRING);

    ++ i; step = TraceIter_get_next(step);
  } /* TRACE_FOR_EACH */

  /*     [MP] this representation does not match logical trace definition */
  /* <node> */
  fprintf(out,"\t</%s>\n", TRACE_XML_NODE_TAG_STRING);

  /* dumps loop info  */
  fprintf(out,"\t<%s> ", TRACE_XML_LOOPS_TAG_STRING);
  loops_iter=NodeList_get_first_iter(loops) ;
  if (!ListIter_is_end(loops_iter)) {
    do {
      fprintf(out, "%d ", NODE_TO_INT(NodeList_get_elem_at(loops, loops_iter)));

      loops_iter = ListIter_get_next(loops_iter);
      if (ListIter_is_end(loops_iter)) break;

      fprintf(out, ",");
    } while (true);
  }

  fprintf(out,"</%s>\n", TRACE_XML_LOOPS_TAG_STRING);
  fprintf(out,"</%s>\n", TRACE_XML_CNTX_TAG_STRING);

  NodeList_destroy(loops);

  return 0;
}


/**Function********************************************************************

  Synopsis    [Creates a XML Plugin for dumping and initializes it.]

  Description [XML plugin constructor. Using this plugin, a trace can
               be dumped to file in XML format]

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceXmlDumper_ptr TraceXmlDumper_create()
{
  TraceXmlDumper_ptr self = ALLOC(TraceXmlDumper, 1);

  TRACE_XML_DUMPER_CHECK_INSTANCE(self);

  trace_xml_dumper_init(self);
  return self;
}


/* ---------------------------------------------------------------------- */
/*     Protected Methods                                                  */
/* ---------------------------------------------------------------------- */

void trace_xml_dumper_print_symbol(TracePlugin_ptr self, node_ptr symb)
{
  const SymbTable_ptr st = trace_get_symb_table(self->trace);
  FILE* out = TraceOpt_output_stream(self->opt);
  char* symb_repr = \
    sprint_node((hash_ptr)(NULL) != self->obfuscation_map
                ? Compile_obfuscate_expression(st, symb, self->obfuscation_map)
                : symb);

  /* substituting XML entities */
  Utils_str_escape_xml_file(symb_repr, out);
  FREE(symb_repr);
}

void trace_xml_dumper_print_assignment(TracePlugin_ptr self,
                                       node_ptr symb, node_ptr val)
{
  FILE* out = TraceOpt_output_stream(self->opt);

  fprintf(out, "\t\t\t<%s variable=\"", TRACE_XML_VALUE_TAG_STRING);

  TracePlugin_print_symbol(self, symb);
  fprintf(out, "\">");

  TracePlugin_print_list(self, val);
  fprintf(out, "</%s>\n", TRACE_XML_VALUE_TAG_STRING);
}


/**Function********************************************************************

  Synopsis    [Class initializer]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_xml_dumper_init(TraceXmlDumper_ptr self)
{
  trace_plugin_init(TRACE_PLUGIN(self),"TRACE XML DUMP PLUGIN");

  /* virtual methods overriding: */
  OVERRIDE(Object, finalize) = trace_xml_dumper_finalize;
  OVERRIDE(TracePlugin, action) = trace_xml_dumper_action;
  OVERRIDE(TracePlugin, print_symbol) = trace_xml_dumper_print_symbol;
  OVERRIDE(TracePlugin, print_assignment) = trace_xml_dumper_print_assignment;
}


/**Function********************************************************************

  Synopsis    [Deinitializes the TraceXmlDumper Plugin object.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_xml_dumper_deinit(TraceXmlDumper_ptr self)
{
  trace_plugin_deinit(TRACE_PLUGIN(self));
}



/* ---------------------------------------------------------------------- */
/*     Private Methods                                                    */
/* ---------------------------------------------------------------------- */


/**Function********************************************************************

  Synopsis    [Plugin finalize method.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_xml_dumper_finalize(Object_ptr object, void* dummy)
{
  TraceXmlDumper_ptr self = TRACE_XML_DUMPER(object);

  trace_xml_dumper_deinit(self);
  FREE(self);
}
