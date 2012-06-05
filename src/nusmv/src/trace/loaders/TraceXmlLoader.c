/**CFile***********************************************************************

  FileName    [TraceXmlLoader.c]

  PackageName [trace.plugins]

  Synopsis    [Routines related to TraceXmlLoader class]

  Description [This file contains the definition of TraceXmlLoader
               class.]

  SeeAlso     []

  Author      [Ashutosh Trivedi, Roberto Cavada, Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.loader'' package of NuSMV version 2.
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
#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "utils/defs.h"

#include "TraceXmlLoader.h"
#include "TraceXmlLoader_private.h"
#include "Trace_private.h"

#include "parser/parser.h"
#include "parser/symbols.h"

#include <stdio.h>

static char rcsid[] UTIL_UNUSED = "$Id: TraceXmlLoader.c,v 1.1.2.3.4.7.4.29 2010-03-04 16:58:57 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* a few constants */
const char* state_section = "STATE";
const char* input_section = "INPUT";
const char* combo_section = "COMBINATORIAL";

/* strings identifying a category of a symbol */
const char* undecl_symb   = "undeclared symbol";
const char* input_var = "input var";
const char* input_def = "input define";
const char* state_var = "state var";
const char* state_def = "state define";
const char* state_input_def = "state-input define";
const char* state_input_next_def = "state-input-next define";

const int invalid_undefined_symbol = 0;
const int invalid_wrong_section = 1;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef void (*TagStartFunction_ptr)(void* , const char*, const char**);
typedef void (*TagEndFunction_ptr)(void* , const char*);
typedef void (*CharHandlerFunction_ptr)(void*, const char*, int);

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void trace_xml_loader_finalize ARGS((Object_ptr object, void* dummy));

static void trace_xml_loader_prepare ARGS((TraceXmlLoader_ptr self,
                                           const SymbTable_ptr st,
                                           const NodeList_ptr symbols));

static void trace_xml_loader_reset ARGS((TraceXmlLoader_ptr self));

static void
trace_xml_loader_tag_begin ARGS((TraceXmlLoader_ptr self,
                                 const char* name, const char** atts));

static void trace_xml_loader_tag_end ARGS((TraceXmlLoader_ptr self,
                                           const char *name));

static void trace_xml_loader_char_handler ARGS((TraceXmlLoader_ptr self,
                                                const char *txt, int txtlen));

static void
trace_xml_report_invalid_assignment ARGS((TraceXmlLoader_ptr self,
                                          node_ptr symbol, int reason));
static inline int
trace_xml_load_put_expr ARGS((TraceXmlLoader_ptr self, node_ptr eq));

static inline node_ptr
trace_xml_loader_flatten_symbol ARGS((node_ptr symbol));

static void trace_xml_loader_store_loopbacks ARGS((TraceXmlLoader_ptr self));

/**Function********************************************************************

  Synopsis    [Constructor]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
TraceXmlLoader_ptr TraceXmlLoader_create(const char* xml_filename,
                                         boolean halt_on_undefined_symbols,
                                         boolean halt_on_wrong_section)
{
  TraceXmlLoader_ptr self = ALLOC(TraceXmlLoader, 1);

  TRACE_XML_LOADER_CHECK_INSTANCE(self);

  trace_xml_loader_init(self, xml_filename,
                        halt_on_undefined_symbols, halt_on_wrong_section);
  return self;
}


/* ---------------------------------------------------------------------- */
/*   Protected Methods                                                    */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_xml_loader_init(TraceXmlLoader_ptr self,
                           const char* xml_filename,
                           boolean halt_on_undefined_symbols,
                           boolean halt_on_wrong_section)
{
  trace_loader_init(TRACE_LOADER(self), "TRACE XML LOADER");

  /* pre-allocate current symbol buf */
  self->curr_symb = ALLOC(char, MAX_ID_LEN);
  nusmv_assert(NIL(char) != self->curr_symb);
  memset(self->curr_symb, 0, MAX_ID_LEN * sizeof(char));

  /* pre-allocate current value buf */
  self->curr_val = ALLOC(char, MAX_VL_LEN);
  nusmv_assert(NIL(char) != self->curr_val);
  memset(self->curr_val, 0, MAX_VL_LEN * sizeof(char));

  /* pre-allocate current equality buf */
  self->curr_eq = ALLOC(char, MAX_EQ_LEN);
  nusmv_assert(NIL(char) != self->curr_eq);
  memset(self->curr_eq, 0, MAX_EQ_LEN);

  /* pre-allocate expat parser buf */
  self->stream_buf = ALLOC(char, EXPAT_BUFSIZE);
  nusmv_assert(NIL(char) != self->stream_buf);
  memset(self->stream_buf, 0, EXPAT_BUFSIZE * sizeof(char));

  self->nusmv_input_file = NIL(char);
  self->parser = (XML_Parser)(NULL);
  self->trace = TRACE(NULL);

  /* storing data from constructor parameters */
  self->xml_filename = ALLOC(char, strlen(xml_filename) + 1);
  nusmv_assert(self->xml_filename != (char*) NULL);
  strncpy(self->xml_filename, xml_filename, strlen(xml_filename) + 1);

  self->halt_on_undefined_symbols = halt_on_undefined_symbols;
  self->halt_on_wrong_section =  halt_on_wrong_section;

  /* virtual methods overriding: */
  OVERRIDE(Object, finalize) = trace_xml_loader_finalize;
  OVERRIDE(TraceLoader, load) = trace_xml_loader_load;
}


/**Function********************************************************************

  Synopsis    [Deallocates internal structures]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void trace_xml_loader_deinit(TraceXmlLoader_ptr self)
{
  if (self->parser != (XML_Parser) NULL) {
    XML_ParserFree(self->parser);
  }

  trace_loader_deinit(TRACE_LOADER(self));
}


/**Function********************************************************************

  Synopsis    [Read the trace from the XML file]

  Description [Returns a valid trace]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Trace_ptr trace_xml_loader_load(TraceLoader_ptr loader,
                                const SymbTable_ptr st,
                                const NodeList_ptr symbols)
{
  FILE* stream;

  const TraceXmlLoader_ptr self = TRACE_XML_LOADER(loader);
  nusmv_assert(TRACE(NULL) == self->trace);

  stream = fopen(self->xml_filename, "rt");
  if (NIL(FILE) != stream) {

    /* inits the parser */
    trace_xml_loader_prepare(self, st, symbols);

    CATCH {
      /* parses the XML file */
      do {
        size_t len;
        boolean ok;

        /* read one chunk of data */
        len = fread(self->stream_buf, sizeof(char), EXPAT_BUFSIZE, stream);

        if (ferror(stream)) {
          fprintf(loader->err,
                  "I/O Error occurred while reading file '%s'\n",
                  self->xml_filename);

          trace_destroy(self->trace); self->trace = TRACE(NULL);
          break;
        }

        /* done with I/O, launches the parser */
        CATCH {
          ok = (XML_Parse(self->parser, self->stream_buf, len, len == 0) \
                && (!self->parse_error));
        }
        FAIL { ok = false; }

        if (!ok) {
          const char* msg = XML_ErrorString(XML_GetErrorCode(self->parser));

          /* an error occurred */
          if (msg != (const char*) NULL) {
            fprintf(loader->err, "At line %d: '%s'\n",
                    (int)(XML_GetCurrentLineNumber(self->parser)), msg);
          }

          trace_destroy(self->trace); self->trace = TRACE(NULL);
          break;
        }
      } while (!feof(stream));

      /* store loopback information into the new trace and freeze it */
      if (TRACE(NULL) != self->trace) {
        trace_xml_loader_store_loopbacks(self);
      }
    }

    /* catch any error silently */
    FAIL { }

    /* shuts down the parser */
    trace_xml_loader_reset(self);

    nusmv_assert(NIL(FILE) != stream);
    fclose(stream);
  } /* NIL(FILE) != stream */

  /* traces produced by the XML trace loader are frozen */
  nusmv_assert(TRACE(NULL) == self->trace || trace_is_frozen(self->trace));

  return self->trace;
}


/* ---------------------------------------------------------------------- */
/*     Private Methods                                                    */
/* ---------------------------------------------------------------------- */


/**Function********************************************************************

  Synopsis    [Virtual destructor]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_xml_loader_finalize(Object_ptr object, void* dummy)
{
  TraceXmlLoader_ptr self = TRACE_XML_LOADER(object);

  trace_xml_loader_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_xml_loader_prepare(TraceXmlLoader_ptr self,
                                     const SymbTable_ptr st,
                                     const NodeList_ptr symbols)
{
  /* setup XML parser based on expat library */
  nusmv_assert(self->parser == (XML_Parser) NULL);

  self->parser = XML_ParserCreate(NULL);
  nusmv_assert(self->parser != (XML_Parser) NULL);

  self->parse_error = false;

  /* to allow the parser access self data during parsing: */
  XML_SetUserData(self->parser, self);

  /* setting up handlers for the parser: */
  XML_SetElementHandler(self->parser,
                        (TagStartFunction_ptr) &trace_xml_loader_tag_begin,
                        (TagEndFunction_ptr) &trace_xml_loader_tag_end);

  XML_SetCharacterDataHandler(self->parser,
                     (CharHandlerFunction_ptr) &trace_xml_loader_char_handler);

  /* creates a new trace, and prepares internal structures */
  self->trace = trace_create(st, "(no description available)",
                             TRACE_TYPE_UNSPECIFIED, symbols, false);

  self->step = trace_first_iter(self->trace);
  self->last_time = 1; /* initial time */
  self->requires_value = false;
  self->all_wrong_symbols = new_assoc();
  self->loopback_states = NodeList_create();

  /* backup nusmv parser internal information */
  self->nusmv_yylineno = yylineno;
  nusmv_assert(NIL(char) == self->nusmv_input_file);
  self->nusmv_input_file = \
    util_strsav(get_input_file(OptsHandler_get_instance()));

  /* try to provide more informative messages upon parsing errors */
  yylineno = -1;
  set_input_file(OptsHandler_get_instance(), self->xml_filename);

  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(TRACE_LOADER(self)->err, "xml parser ready\n");
  }
}

/**Function********************************************************************

  Synopsis    [Cleans up after reading of xml source]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_xml_loader_reset(TraceXmlLoader_ptr self)
{
  self->curr_parsing = TRACE_XML_INVALID_TAG;
  self->parse_error = false;

  memset(self->stream_buf, 0, EXPAT_BUFSIZE * sizeof(char));

  nusmv_assert(self->parser != (XML_Parser) NULL);
  XML_ParserFree(self->parser);
  self->parser = (XML_Parser) NULL;

  free_assoc(self->all_wrong_symbols);
  self->all_wrong_symbols = (hash_ptr)(NULL);

  /* restore parser internal information */
  yylineno = self->nusmv_yylineno;
  set_input_file(OptsHandler_get_instance(), self->nusmv_input_file);
  FREE(self->nusmv_input_file); self->nusmv_input_file = NIL(char);

  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(TRACE_LOADER(self)->err, "xml parser reset\n");
  }
}


/**Function********************************************************************

  Synopsis    [Store loopback information and freezes loaded trace]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void trace_xml_loader_store_loopbacks(TraceXmlLoader_ptr self)
{
  ListIter_ptr liter;

  Trace_freeze(self->trace); /* loopbacks can be added only to frozen traces */
  NODE_LIST_FOREACH(self->loopback_states, liter) {
    TraceIter loop_iter = trace_ith_iter(self->trace,
                NODE_TO_INT(NodeList_get_elem_at(self->loopback_states, liter)));

    Trace_step_force_loopback(self->trace, loop_iter);
  }
}

/**Function********************************************************************

  Synopsis         [Function that gets called when parser encounter start of
                    some tag.]

  Description      []

  SideEffects      []

  SeeAlso          []

******************************************************************************/
static void
trace_xml_loader_tag_begin(TraceXmlLoader_ptr self,
                           const char* name, const char** atts)
{
  const TraceLoader_ptr loader = TRACE_LOADER(self);
  int i, time = self->last_time;

  /* clear buffers */
  memset(self->curr_symb, 0, MAX_ID_LEN * sizeof(char));
  memset(self->curr_val, 0, MAX_VL_LEN * sizeof(char));

  /* By default, tags do not have a text value. See specific tags for
     exceptions (e.g. TRACE_XML_VALUE_TAG) */
  self->requires_value = false;

  switch (TraceXmlTag_from_string(name)) {

  case TRACE_XML_CNTX_TAG:
    /* Attributes. */
    for (i = 0; atts[i]; i += 2) {

      if (! strncmp("type", atts[i], 4)) {
        trace_set_type(self->trace, atoi(atts[i+1]));
      }

      if (! strncmp("desc", atts[i], 4))  {
        trace_set_desc(self->trace, atts[i+1]);
      }
    }
    break;

    /*  deprecated tag, we plainly ignore it */
  case TRACE_XML_NODE_TAG:
    break;

  case TRACE_XML_STATE_TAG:
    /* Attributes. */
    for (i = 0; atts[i]; i += 2) {
      if (! strncmp("id", atts[i], 2)) {
        time = atoi(atts[i+1]);
      }
    }
    self->curr_parsing = TRACE_XML_STATE_TAG;
    break;

  case TRACE_XML_COMB_TAG:
     /* Attributes. */
    for (i = 0; atts[i]; i += 2) {
      if (! strncmp("id", atts[i], 2)) {
        time = atoi(atts[i+1]);
      }
    }
    self->curr_parsing = TRACE_XML_COMB_TAG;
    break;

  case TRACE_XML_INPUT_TAG:
     /* Attributes. */
    for (i = 0; atts[i]; i += 2) {
      if (! strncmp("id", atts[i], 2)) {
        time = atoi(atts[i+1]);
      }
    }
    self->curr_parsing = TRACE_XML_INPUT_TAG;
    break;

  case TRACE_XML_VALUE_TAG:
    /* This tag requires a text value to be read */
    self->requires_value = true;

    for (i = 0; atts[i]; i += 2) {
      if (! strncmp("variable", atts[i], 8)) {
        strncpy(self->curr_symb, atts[i+1], MAX_ID_LEN);
      }
    }
    break;

  case TRACE_XML_LOOPS_TAG:
    /* This tag requires a text value to be read */
    self->requires_value = true;
    self->curr_parsing = TRACE_XML_LOOPS_TAG;
    break;

  case TRACE_XML_INVALID_TAG:
    fprintf(loader->err, "Invalid TAG : <%s> Encountered in XML File\n", name);
    self->parse_error = true;
    return;

  default:
    /* unknown tag */
    internal_error("%s:%d:%s: trace_xml_loader_tag_begin: unknown tag '%s'\n",
                   __FILE__, __LINE__, __func__, name);
  }

  /* time must be monotonic not-decreasing */
  if (time < self->last_time) {
    fprintf(loader->err, "Invalid time : <%d> detected in XML File\n", time);
    self->parse_error = true;
  }

  /* add more steps as required, this implementation allows "holes" in
     the XML traces managing them correctly. Entire sections can be
     missing. Naturally, such traces are partial. */
  while (time > self->last_time) {

    ++ self->last_time;
    self->step = (TRACE_END_ITER != trace_iter_get_next(self->step))
      ? trace_iter_get_next(self->step)
      : trace_append_step(self->trace);
  }
} /* trace_xml_loader_tag_begin */


/**Function********************************************************************

  Synopsis        [Function that gets called when end of any tag is
                   encountered by the parser.]

  Description     []

  SideEffects     []

  SeeAlso         []

******************************************************************************/
static void trace_xml_loader_tag_end(TraceXmlLoader_ptr self, const char *name)
{
  node_ptr parsed;

  switch (TraceXmlTag_from_string(name)) {
  case TRACE_XML_CNTX_TAG:
    break;

  case TRACE_XML_NODE_TAG:
    break;

  case TRACE_XML_STATE_TAG:
    self->curr_parsing = TRACE_XML_INVALID_TAG;
    break;

  case TRACE_XML_COMB_TAG:
    self->curr_parsing = TRACE_XML_INVALID_TAG;
    break;

  case TRACE_XML_INPUT_TAG:
    self->curr_parsing = TRACE_XML_INVALID_TAG;
    break;

  case TRACE_XML_VALUE_TAG:
    /* [MP] solution may improve dramatically throughput of the loader */
    if (!self->parse_error) { /* if already in error state do nothing. */
      /* an hack to support non-deterministic assignments in traces
         (see issue #1802) */
      if (NIL(char) == strchr(self->curr_val, ',')) {
        int c = snprintf(self->curr_eq, MAX_EQ_LEN, "%s = %s", self->curr_symb, self->curr_val);
        SNPRINTF_CHECK(c, MAX_EQ_LEN);
      }
      else {
        int c = snprintf(self->curr_eq, MAX_EQ_LEN, "%s in { %s }", self->curr_symb, self->curr_val);
        SNPRINTF_CHECK(c, MAX_EQ_LEN);
      }

      if (0 == Parser_ReadSimpExprFromString(self->curr_eq, &parsed)) {
        if (0 != trace_xml_load_put_expr(self, cdar(parsed))) {
          self->parse_error = true;
        }
        free_node(parsed);
      } else self->parse_error = true;
    }
    break;

    case TRACE_XML_LOOPS_TAG:
      {
        const char COMMA_CHR = ',';
        const char TERM_CHR = '\0';

        char* p0 = self->curr_val;
        char *p = p0;
        char* q;

        /* the loopbacks list is initially empty */
        nusmv_assert(0 == NodeList_get_length(self->loopback_states));
        do { /* iterate over a comma separated list */
          if ((q=strchr(p, COMMA_CHR))) {
            /* side effects on p */
            (*q) = TERM_CHR; q++;
          }

          int node_idx = atoi(p);
          if (0 < node_idx) { /* state indexing starts at 1 */
            NodeList_append(self->loopback_states,
                            PTR_FROM_INT(node_ptr,node_idx ));
          }

          p=q;
        } while (p); /* is there anything left to parse? */

        break;
      } /* XML LOOPS TAG */

    case TRACE_XML_INVALID_TAG:
      self->parse_error = true;

  default:
    /* unknown tag */
    internal_error("trace_xml_loader_tag_end: unknown tag '%s'\n", name);
  } /* switch */

}

static inline int trace_xml_load_put_expr(TraceXmlLoader_ptr self, node_ptr eq)
{
  SymbTable_ptr st;
  TraceSection section;
  unsigned dummy;
  node_ptr symb, value;
  int time_ofs = 0;

  nusmv_assert(EQUAL == node_get_type(eq) || SETIN == node_get_type(eq));
  st = trace_get_symb_table(self->trace);

  /* here Compile_FlattenSexp cannot be used, because we need */
  /* to avoid rewriting for arrays (see issue #1243, note 2064) */
  symb = trace_xml_loader_flatten_symbol(car(eq));
  value = node_normalize(Compile_FlattenSexp(st, cdr(eq), Nil));

  if (trace_symbol_fwd_lookup(self->trace, symb, &section, &dummy)) {

    switch (section) {
      /* state */
    case TRACE_SECTION_FROZEN_VAR:
    case TRACE_SECTION_STATE_VAR:
    case TRACE_SECTION_STATE_DEFINE:
      if (self->curr_parsing != TRACE_XML_STATE_TAG) {

        if (self->curr_parsing == TRACE_XML_INPUT_TAG || \
            self->curr_parsing == TRACE_XML_COMB_TAG) { time_ofs = +1; }

        trace_xml_report_invalid_assignment(self, symb, invalid_wrong_section);
        if (self->halt_on_wrong_section) return 1;
      }
      break;

      /* input */
    case TRACE_SECTION_INPUT_VAR:
    case TRACE_SECTION_INPUT_DEFINE:
      if (self->curr_parsing != TRACE_XML_INPUT_TAG) {

        if (self->curr_parsing == TRACE_XML_STATE_TAG) { time_ofs = -1; }

        trace_xml_report_invalid_assignment(self, symb, invalid_wrong_section);
        if (self->halt_on_wrong_section) return 1;
      }
      break;

      /* combinatorials */
    case TRACE_SECTION_STATE_INPUT_DEFINE:
    case TRACE_SECTION_NEXT_DEFINE:
    case TRACE_SECTION_INPUT_NEXT_DEFINE:
    case TRACE_SECTION_STATE_NEXT_DEFINE:
    case TRACE_SECTION_STATE_INPUT_NEXT_DEFINE:
      if (self->curr_parsing !=  TRACE_XML_COMB_TAG) {

        if (self->curr_parsing == TRACE_XML_STATE_TAG) { time_ofs = -1; }

        trace_xml_report_invalid_assignment(self, symb, invalid_wrong_section);
        if (self->halt_on_wrong_section) return 1;
      }
      break;

    default:  error_unreachable_code(); /* unreachable */
    } /* switch */

    { /* time adjusting */
      TraceIter step = TRACE_END_ITER;

      if (0 < time_ofs) { /* move backward */
        step = trace_iter_get_prev(self->step);
      }
      else if (time_ofs < 0) { /* move forward */
        step = trace_iter_get_next(self->step);
        if (TRACE_END_ITER == step) { step = trace_append_step(self->trace); }
      }
      else { step = self->step; } /* no time adjusting necessary */

     nusmv_assert(TRACE_END_ITER != step);
     if (!trace_step_put_value(self->trace, step, symb, value)) {
        return 1; /* put value reported a type error */
      }
    }
  } /* lookup */

  else  {
    trace_xml_report_invalid_assignment(self, symb, invalid_undefined_symbol);
    if (self->halt_on_undefined_symbols) return 1;
  }

  return 0; /* success */
} /* trace_xml_load_put_expr */


/**Function********************************************************************

Synopsis           [Character Handler used by parser.]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
static void trace_xml_loader_char_handler(TraceXmlLoader_ptr self,
                                          const char *txt, int txtlen)
{
  /* There is no need for reading these chars.. Ignore them */
  if (!self->requires_value) return;

  strncat(self->curr_val, txt, txtlen);
  nusmv_assert(strlen(self->curr_val)  < MAX_VL_LEN);
}


/**Function********************************************************************

  Synopsis           [Private service of trace_xml_loader_fill_trace]

  Description        [Private service of trace_xml_loader_fill_trace]

  SideEffects        []

  SeeAlso            [trace_xml_loader_fill_trace]

******************************************************************************/
static inline node_ptr
trace_xml_loader_flatten_symbol(node_ptr symbol)
{
  int op;

  op = node_get_type(symbol);
  if (op == DOT && car(symbol) == Nil) {
    return node_normalize(find_node(DOT, Nil, cdr(symbol)));
  }

  if (op == ATOM) {
    return node_normalize(find_node(DOT, Nil, symbol));
  }

  if (op == ARRAY) {
    nusmv_assert(Nil != cdr(symbol));

    /* Indexes in array are not parsed correctly: The unary minus node
       is left (e.g. UMINUS NUMBER <positive_number>) instead of
       having NUMBER <negative_number>. Fix the tree here */
    if (UMINUS == node_get_type(cdr(symbol))) {
      symbol = find_node(op, car(symbol),
                         Expr_unary_minus(car(cdr(symbol))));
    }
  }

  return find_node(op,
                   trace_xml_loader_flatten_symbol(car(symbol)),
                   node_normalize(cdr(symbol)));
}


/**Function********************************************************************

  Synopsis    [This function reports an error/warning message when LHS of
               an assignment is found to be in invalid section]

  Description [Here assignment is invalid if LHS is undefined symbol
               or a symbol which is inconsistent with a given section,
               e.g. input var cannot be in STATE section.

               'all_wrong_symbols' is a hash to remember already
               reported symbols and report them only once. 'isError'
               is a flag to report an error; warning is reported
               otherwise

               The returned value is isError argument. ]

  SideEffects []

  SeeAlso     [trace_xml_loader_fill_trace]

******************************************************************************/
static void trace_xml_report_invalid_assignment(TraceXmlLoader_ptr self,
                                                node_ptr symbol, int reason)
{
  TraceLoader_ptr loader = TRACE_LOADER(self);

  const char* cat_repr = \
    (trace_symbol_in_language(self->trace, symbol))
    ? trace_symb_category_to_string(trace_symbol_get_category(self->trace,
                                                              symbol))
    : ""; /* should never see this */

  if (Nil == find_assoc(self->all_wrong_symbols, symbol)) {

    if (invalid_wrong_section == reason) {
      fprintf(loader->err, "%s: %s '", (self->halt_on_wrong_section)
              ? "Error" : "Warning", cat_repr);
    }
    else if (invalid_undefined_symbol == reason) {
      fprintf(loader->err, "%s: undefined symbol '",
              (self->halt_on_undefined_symbols)
              ? "Error" : "Warning");
    }
    else error_unreachable_code(); /* unreachable */

    print_node(loader->err, symbol);
    fprintf(loader->err, "' is in section %s (time %d).\n",
            TraceXmlTag_to_string (self->curr_parsing), self->last_time);

    insert_assoc(self->all_wrong_symbols, symbol, NODE_FROM_INT(true));
    fprintf(loader->err, "(Each symbol is reported only once)\n\n");
  } /* Nil == find_assoc */
}

