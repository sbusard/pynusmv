/**CHeaderFile*****************************************************************

  FileName    [Trace_private.h]

  PackageName [trace]

  Synopsis    [The private header file for the Trace class]

  Description [optional]

  SeeAlso     [optional]

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
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

  Revision    [$Id: $]

******************************************************************************/

#ifndef _TRACE_PRIVATE_DEFINED
#define _TRACE_PRIVATE_DEFINED

#include "Trace.h"
#include "utils/array.h"
#include "utils/error.h"
#include "utils/assoc.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef enum TraceSection_TAG {
  /* reserved */
  TRACE_SECTION_INVALID = 0,

  TRACE_SECTION_FROZEN_VAR, TRACE_SECTION_STATE_VAR,
  TRACE_SECTION_INPUT_VAR, TRACE_SECTION_STATE_DEFINE,
  TRACE_SECTION_INPUT_DEFINE, TRACE_SECTION_STATE_INPUT_DEFINE,
  TRACE_SECTION_NEXT_DEFINE, TRACE_SECTION_STATE_NEXT_DEFINE,
  TRACE_SECTION_INPUT_NEXT_DEFINE, TRACE_SECTION_STATE_INPUT_NEXT_DEFINE,

  /* reserved */
  TRACE_SECTION_END,
} TraceSection;

/*  frames */
typedef struct TraceFrozenFrame_TAG* TraceFrozenFrame_ptr;
typedef struct TraceVarFrame_TAG* TraceVarFrame_ptr;
typedef struct TraceDefineFrame_TAG* TraceDefineFrame_ptr;

/**Struct**********************************************************************


  Synopsis    [Trace Class]

  Description [ This class contains informations about a Trace:<br>
  <dl>
        <dt><code>id</code>
            <dd>  Unique ID of the registered traces. -1 for unregistered
            traces.
        <dt><code>desc</code>
            <dd>  Description of the trace.
        <dt><code>length</code>
            <dd>  Diameter of the trace. (i.e. the number of transitions)
        <dt><code>type</code>
            <dd>  Type of the trace.
        <dt><code>first_step</code>
            <dd>  Pointer to the first step of the doubly linked list of
            TraceSteps.
        <dt><code>last_node</code>
        <dd> Pointer to the last node of the doubly linked list of
            TraceSteps.
        <dt><code>defines_evaluated</code>
            <dd>  Internal index used to perform lazy evaluation of defines.
        <dt><code>symb2index</code>
            <dd>  Symbol to index hash table for fast look-up.
    </dl>
        <br>
  ]

******************************************************************************/
typedef struct Trace_TAG
{
  /* metadata */
  TraceType type;
  const char* desc;
  int id;

  unsigned length;
  boolean frozen;
  boolean is_volatile;

  SymbTable_ptr st;

  NodeList_ptr symbols;
  NodeList_ptr s_vars;
  NodeList_ptr sf_vars;
  NodeList_ptr i_vars;

  /* first and last frame */
  TraceVarFrame_ptr first_frame;
  TraceVarFrame_ptr last_frame;

  /* Keep frozenvars separated */
  TraceFrozenFrame_ptr frozen_frame;

  /* buckets (first and last unused) */
  unsigned n_buckets[TRACE_SECTION_END];
  node_ptr* buckets[TRACE_SECTION_END];

  /*  lookup aux structures */
  hash_ptr symb2section;
  hash_ptr symb2address;
  hash_ptr symb2layername;

} Trace;

/* frames */
typedef struct TraceVarFrame_TAG
{
  /* metadata */
  node_ptr* state_values;
  node_ptr* input_values;

  /* for frozen traces only */
  boolean loopback;

  /* Defines frames */
  TraceDefineFrame_ptr fwd_define_frame;
  TraceDefineFrame_ptr bwd_define_frame;

  /* doubly linked list */
  TraceVarFrame_ptr next_frame;
  TraceVarFrame_ptr prev_frame;
} TraceVarFrame;

typedef struct TraceFrozenFrame_TAG
{
  node_ptr* frozen_values;
  /* unsigned n_frozen_values; */
} TraceFrozenFrame;

typedef struct TraceDefineFrame_TAG
{
  node_ptr* s_values;
  node_ptr* i_values;
  node_ptr* si_values;
  node_ptr* n_values;
  node_ptr* sn_values;
  node_ptr* in_values;
  node_ptr* sin_values;
} TraceDefineFrame;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#undef CHECK
#if defined TRACE_DEBUG
#define CHECK(cond) nusmv_assert(cond)
#else
#define CHECK(cond)
#endif

#define TRACE_VAR_FRAME(x) \
  ((TraceVarFrame_ptr) x)

#define TRACE_DEFINE_FRAME(x) \
  ((TraceDefineFrame_ptr) x)

#define TRACE_FROZEN_FRAME(x) \
  ((TraceFrozenFrame_ptr) x)

#define TRACE_VAR_FRAME_CHECK_INSTANCE(x) \
  (nusmv_assert(TRACE_VAR_FRAME(x) != TRACE_VAR_FRAME(NULL)))

#define TRACE_DEFINE_FRAME_CHECK_INSTANCE(x) \
  (nusmv_assert(TRACE_DEFINE_FRAME(x) != TRACE_DEFINE_FRAME(NULL)))

#define TRACE_FROZEN_FRAME_CHECK_INSTANCE(x) \
  (nusmv_assert(TRACE_FROZEN_FRAME(x) != TRACE_FROZEN_FRAME(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Trace_ptr
trace_create ARGS((SymbTable_ptr st, const char* desc,
                   const TraceType type, NodeList_ptr symbols,
                   boolean is_volatile));

EXTERN Trace_ptr
trace_copy ARGS((Trace_ptr self, TraceIter until_here,
                 boolean is_volatile));

EXTERN boolean
trace_is_volatile ARGS((const Trace_ptr self));

EXTERN boolean
trace_equals ARGS((const Trace_ptr self, const Trace_ptr other));

EXTERN Trace_ptr
trace_concat ARGS((Trace_ptr self, Trace_ptr* other));

EXTERN void
trace_destroy ARGS((Trace_ptr self));

EXTERN boolean
trace_symbol_fwd_lookup ARGS((Trace_ptr self, node_ptr symb,
                              TraceSection* section, unsigned* index));

EXTERN node_ptr
trace_symbol_bwd_lookup ARGS((Trace_ptr self, TraceSection section,
                              unsigned offset));

EXTERN TraceIter
trace_append_step ARGS((Trace_ptr self));

EXTERN boolean
trace_step_is_loopback ARGS((const Trace_ptr self, const TraceIter step));

EXTERN boolean
trace_step_test_loopback ARGS((Trace_ptr self, const TraceIter step));

EXTERN void
trace_step_force_loopback ARGS((const Trace_ptr self, TraceIter step));

EXTERN boolean
trace_is_frozen ARGS((const Trace_ptr self));

EXTERN boolean
trace_is_thawed ARGS((const Trace_ptr self));

EXTERN void
trace_freeze ARGS((Trace_ptr self));

EXTERN void
trace_thaw ARGS((Trace_ptr self));

EXTERN boolean
trace_symbol_in_language ARGS((const Trace_ptr self, const node_ptr symb));

EXTERN SymbCategory
trace_symbol_get_category ARGS((Trace_ptr self, node_ptr symb));

EXTERN const char*
trace_get_layer_from_symb ARGS((const Trace_ptr self, const node_ptr symb));

EXTERN boolean
trace_symbol_is_assigned ARGS((const Trace_ptr self,
                               const TraceIter step, node_ptr symb));

EXTERN boolean
trace_step_put_value ARGS((Trace_ptr self, const TraceIter step,
                           const node_ptr symb, const node_ptr value));

EXTERN node_ptr
trace_step_get_value ARGS((const Trace_ptr self, const TraceIter step,
                           const node_ptr symb));

EXTERN boolean
trace_is_complete_vars ARGS((const Trace_ptr self, const NodeList_ptr vars,
                             FILE* report_stream));

/* horizontal iterators management */
EXTERN TraceIter
trace_first_iter ARGS((const Trace_ptr self));

EXTERN TraceIter
trace_ith_iter ARGS((const Trace_ptr self, unsigned i));

EXTERN TraceIter
trace_last_iter ARGS((const Trace_ptr self));

EXTERN unsigned
trace_iter_i ARGS((const Trace_ptr self, TraceIter iter));

EXTERN TraceIter
trace_iter_get_next ARGS((const TraceIter iter));

EXTERN TraceIter
trace_iter_get_prev ARGS((const TraceIter iter));

/* vertical iterators management */
EXTERN TraceStepIter
trace_step_iter ARGS((const Trace_ptr self, const TraceIter step,
                      TraceIteratorType iter_type));

EXTERN boolean
trace_step_iter_fetch ARGS((TraceStepIter* step_iter,
                            node_ptr* symb, node_ptr* value));

EXTERN TraceSymbolsIter
trace_symbols_iter ARGS((const Trace_ptr self, TraceIteratorType iter_type));

EXTERN boolean
trace_symbols_iter_fetch ARGS((TraceSymbolsIter* symbols_iter,
                               node_ptr* symb));

/* trace metadata */
EXTERN unsigned
trace_get_length ARGS((const Trace_ptr self));

EXTERN boolean
trace_is_empty ARGS((const Trace_ptr self));

EXTERN NodeList_ptr
trace_get_symbols ARGS((const Trace_ptr self));

EXTERN NodeList_ptr
trace_get_s_vars ARGS((const Trace_ptr self));

EXTERN NodeList_ptr
trace_get_sf_vars ARGS((const Trace_ptr self));

EXTERN NodeList_ptr
trace_get_i_vars ARGS((const Trace_ptr self));

EXTERN NodeList_ptr
trace_get_visible ARGS((const Trace_ptr self));

EXTERN NodeList_ptr
trace_get_hidden ARGS((const Trace_ptr self));

EXTERN SymbTable_ptr
trace_get_symb_table ARGS((Trace_ptr self));

EXTERN void
trace_register ARGS((Trace_ptr self, int id));

EXTERN void
trace_unregister ARGS((Trace_ptr self));

EXTERN boolean
trace_is_registered ARGS((Trace_ptr self));

EXTERN int
trace_get_id ARGS((const Trace_ptr self));

EXTERN TraceType
trace_get_type ARGS((const Trace_ptr self));

EXTERN void
trace_set_type ARGS((Trace_ptr self, TraceType trace_type));

EXTERN const char*
trace_get_desc ARGS((const Trace_ptr self));

EXTERN void
trace_set_desc ARGS((Trace_ptr self, const char* desc));

EXTERN void
trace_step_evaluate_defines ARGS((Trace_ptr self, const TraceIter step));

EXTERN boolean
trace_step_check_defines ARGS((Trace_ptr self, const TraceIter step,
                               NodeList_ptr failures));

/* private conversion functions */
EXTERN SymbCategory
trace_section_to_category ARGS((const TraceSection section));

EXTERN TraceSection
trace_category_to_section ARGS((const SymbCategory category));

EXTERN const char*
trace_symb_category_to_string ARGS((const SymbCategory category));

EXTERN Expr_ptr
trace_simplify_expr ARGS((const SymbTable_ptr st, Expr_ptr expr));

/**AutomaticEnd***************************************************************/

#endif /* _TRACE_PRIVATE_DEFINED */
