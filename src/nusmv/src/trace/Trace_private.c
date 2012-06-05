/**CFile***********************************************************************

  FileName    [Trace_private.c]

  PackageName [trace]

  Synopsis    [This module contains internal functions that are part of
               Trace class implementation]

  Description []

  SeeAlso     [Trace.c]

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

******************************************************************************/

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include <stdint.h>

#include "Trace_private.h"
#include "pkg_traceInt.h"
#include "node/node.h"
#include "set/set.h"
#include "opt/opt.h"
#include "parser/symbols.h" /* for FAILURE */

#include <string.h>

static char rcsid[] UTIL_UNUSED = "$Id: $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/* used only for debugging */
EXTERN FILE* nusmv_stderr;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* sentinel node used to optmize iterations for speed */
#define SENTINEL failure_make("EOS", FAILURE_UNSPECIFIED, -1)

/* canonized boolean/integer constants */
#define EXPR_ONE find_node(NUMBER, NODE_FROM_INT(1), Nil)
#define EXPR_ZERO find_node(NUMBER, NODE_FROM_INT(0), Nil)
#define EXPR_TRUE Expr_true()
#define EXPR_FALSE Expr_false()

/* non-descriptive description ;) */
#define TRACE_DEFAULT_DESC "<generic trace>"

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/* trace metadata getters/setters */
static inline void
trace_reset_length ARGS((const Trace_ptr self));

static inline void
trace_inc_length ARGS((const Trace_ptr self));

static inline void
trace_set_language ARGS((Trace_ptr self, SymbTable_ptr st,
                         NodeList_ptr symbols, boolean is_volatile));

/* frame base getters/setters */
static inline node_ptr*
trace_frozen_frame_get_base ARGS((const TraceFrozenFrame_ptr frame));

static inline node_ptr*
trace_var_frame_get_state_base ARGS((const TraceVarFrame_ptr frame));

static inline void
trace_var_frame_set_state_base ARGS((const TraceVarFrame_ptr frame,
                                     node_ptr* base));
static inline node_ptr*
trace_var_frame_get_input_base ARGS((const TraceVarFrame_ptr frame));

static inline void
trace_var_frame_set_input_base ARGS((const TraceVarFrame_ptr frame,
                                     node_ptr* base));

static inline node_ptr*
trace_define_frame_get_state_base ARGS((const TraceDefineFrame_ptr frame));

static inline void
trace_define_frame_set_state_base ARGS((TraceDefineFrame_ptr frame,
                                        node_ptr* base));

static inline node_ptr*
trace_define_frame_get_input_base ARGS((const TraceDefineFrame_ptr frame));

static inline void
trace_define_frame_set_input_base ARGS((TraceDefineFrame_ptr frame,
                                        node_ptr* base));

static inline node_ptr*
trace_define_frame_get_state_input_base ARGS((const TraceDefineFrame_ptr frame));

static inline void
trace_define_frame_set_state_input_base ARGS((TraceDefineFrame_ptr frame,
                                              node_ptr* base));

static inline node_ptr*
trace_define_frame_get_next_base ARGS((const TraceDefineFrame_ptr frame));

static inline void
trace_define_frame_set_next_base ARGS((TraceDefineFrame_ptr frame,
                                       node_ptr* base));

static inline node_ptr*
trace_define_frame_get_state_next_base ARGS((const TraceDefineFrame_ptr frame));

static inline void
trace_define_frame_set_state_next_base ARGS((TraceDefineFrame_ptr frame,
                                             node_ptr* base));

static inline node_ptr*
trace_define_frame_get_input_next_base ARGS((const TraceDefineFrame_ptr frame));

static inline void
trace_define_frame_set_input_next_base ARGS((TraceDefineFrame_ptr frame,
                                             node_ptr* base));

static inline node_ptr*
trace_define_frame_get_state_input_next_base
ARGS((const TraceDefineFrame_ptr frame));

static inline void
trace_define_frame_set_state_input_next_base
ARGS((TraceDefineFrame_ptr frame, node_ptr* base));

static inline unsigned
trace_get_n_state_vars ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_frozen_vars ARGS((const Trace_ptr self));

static inline unsigned
 trace_get_n_input_vars ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_state_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_input_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_state_input_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_next_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_state_next_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_input_next_defines ARGS((const Trace_ptr self));

static inline void
trace_set_first_frame ARGS((Trace_ptr self, TraceVarFrame_ptr var_frame));

static inline void
trace_set_last_frame ARGS((Trace_ptr self, TraceVarFrame_ptr var_frame));

static inline TraceFrozenFrame_ptr
trace_get_frozen_frame ARGS((Trace_ptr self));

static inline void
trace_set_frozen_frame ARGS((Trace_ptr self, TraceFrozenFrame_ptr frozen_frame));

/* frame management */
static inline TraceFrozenFrame_ptr
trace_frozen_frame_create ARGS((Trace_ptr self));

static inline void
trace_frozen_frame_destroy ARGS((TraceFrozenFrame_ptr frozen_frame));

static inline TraceVarFrame_ptr
trace_var_frame_create ARGS((Trace_ptr self));

static inline void
trace_var_frame_destroy ARGS((TraceVarFrame_ptr frame));

static inline TraceDefineFrame_ptr
trace_define_frame_create ARGS((Trace_ptr self));

static inline void
trace_define_frame_destroy ARGS((TraceDefineFrame_ptr def_frame));

static inline void
trace_var_frame_set_prev ARGS((TraceVarFrame_ptr frame, TraceVarFrame_ptr prev));

static inline TraceVarFrame_ptr
trace_var_frame_get_prev ARGS((TraceVarFrame_ptr frame));

static inline void
trace_var_frame_set_next ARGS((TraceVarFrame_ptr frame, TraceVarFrame_ptr next));

static inline TraceVarFrame_ptr
trace_var_frame_get_next ARGS((TraceVarFrame_ptr frame));

static inline TraceDefineFrame_ptr
trace_var_frame_get_fwd_define_frame ARGS((TraceVarFrame_ptr var_frame));

static inline TraceDefineFrame_ptr
trace_var_frame_get_bwd_define_frame ARGS((TraceVarFrame_ptr var_frame));

static inline node_ptr*
trace_iter_get_section_base ARGS((Trace_ptr trace, TraceIter iter,
                                  TraceSection sect_type, boolean create));

/* lookup management */
static inline void
trace_insert_lookup_info ARGS((Trace_ptr trace, node_ptr symb_name,
                               TraceSection section, unsigned offset));
static inline void
trace_setup_lookup_section ARGS((Trace_ptr self,
                                 TraceSection section));
static inline void
trace_init_lookup_data ARGS((Trace_ptr self));

static inline void
trace_copy_lookup_data ARGS((Trace_ptr dst, Trace_ptr src));

static inline void
trace_setup_lookup_cache ARGS((Trace_ptr self));

static inline void
trace_dispose_lookup_cache ARGS((Trace_ptr self));

static inline void
trace_print_alloc_stats ARGS((Trace_ptr self));

/* frames internals */
static inline TraceDefineFrame_ptr
trace_var_frame_get_fwd_define_frame ARGS((TraceVarFrame_ptr var_frame));

static inline TraceDefineFrame_ptr
trace_var_frame_get_bwd_define_frame ARGS((TraceVarFrame_ptr var_frame));

static inline void
trace_var_frame_set_fwd_define_frame ARGS((TraceVarFrame_ptr var_frame,
                                           TraceDefineFrame_ptr def_frame));

static inline void
trace_var_frame_set_bwd_define_frame ARGS((TraceVarFrame_ptr var_frame,
                                           TraceDefineFrame_ptr def_frame));

static inline TraceVarFrame_ptr
trace_var_frame_get_next_frame ARGS((TraceVarFrame_ptr var_frame));

static inline TraceVarFrame_ptr
trace_var_frame_get_prev_frame ARGS((TraceVarFrame_ptr var_frame));

static inline void
trace_var_frame_set_next_frame ARGS((TraceVarFrame_ptr var_frame,
                                     TraceVarFrame_ptr next_frame));

static inline void
trace_var_frame_set_prev_frame ARGS((TraceVarFrame_ptr var_frame,
                                     TraceVarFrame_ptr prev_frame));

/* internal getters/setters */
static inline void
trace_inc_n_section_symbols ARGS((Trace_ptr self,
                                  TraceSection section));

static inline unsigned
trace_get_n_section_symbols ARGS((const Trace_ptr self,
                                  TraceSection section));

static inline node_ptr*
trace_get_section_symbols ARGS((Trace_ptr self,
                                TraceSection section));

static inline unsigned
trace_get_n_frozen_vars ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_state_vars ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_input_vars ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_state_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_input_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_state_input_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_next_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_state_next_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_input_next_defines ARGS((const Trace_ptr self));

static inline unsigned
trace_get_n_state_input_next_defines ARGS((const Trace_ptr self));

/* low level management */
static inline void
trace_init ARGS((Trace_ptr self));

static inline void
trace_frozen_frame_init ARGS((TraceFrozenFrame_ptr self));

static inline void
trace_var_frame_init ARGS((TraceVarFrame_ptr self));

static inline void
trace_define_frame_init ARGS((TraceDefineFrame_ptr self));

static inline node_ptr*
trace_setup_section_storage ARGS((unsigned n_symbs));

static inline void
trace_copy_section_storage ARGS((node_ptr* dest, node_ptr* src,
                                 unsigned n_symbs));
static inline int
trace_compare_section_storage ARGS((node_ptr* dest, node_ptr* src,
                                    unsigned n_symbs));

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

Trace_ptr
trace_create (SymbTable_ptr st, const char* desc,
              const TraceType type, NodeList_ptr symbols,
              boolean is_volatile)
{
  Trace_ptr res = ALLOC(Trace, 1);

  TRACE_CHECK_INSTANCE(res);
  trace_init (res);

  { /* phase 0: initialize instance fields */
    /* metadata */
    trace_thaw(res);
    trace_set_type(res, type);
    trace_set_desc(res, desc);
    trace_set_language(res, st, symbols, is_volatile);
    trace_unregister(res);
    trace_reset_length(res);

    trace_setup_lookup_cache(res);
    trace_init_lookup_data(res);

    trace_print_alloc_stats(res);
  } /* 0 */

  { /* phase 1: initial frame setup */
    TraceDefineFrame_ptr define_frame;
    TraceVarFrame_ptr initial_frame;

    /* setup frozen frame */
    trace_set_frozen_frame(res, trace_frozen_frame_create(res));

    /* assign the first and final frame to the initial var frame */
    initial_frame = trace_var_frame_create(res);

    trace_set_first_frame(res, initial_frame);
    trace_set_last_frame(res, initial_frame);

    /* setup  define frame for initial var frame */
    define_frame = trace_define_frame_create(res);
    trace_var_frame_set_fwd_define_frame(initial_frame, define_frame);
    trace_var_frame_set_bwd_define_frame(initial_frame,
                                         TRACE_DEFINE_FRAME(NULL));
  }

  return res;
} /* trace_create */


void
trace_destroy(Trace_ptr self)
{
  TraceVarFrame_ptr var_frame = TRACE_VAR_FRAME(trace_last_iter(self));
  TraceVarFrame_ptr prev_var_frame;

  /* destroying the trace backwards may improve performance w.r.t. OS
     caching */
  while (TRACE_END_ITER != TRACE_ITER(var_frame)) {
    TraceDefineFrame_ptr def_frame =
      trace_var_frame_get_fwd_define_frame(var_frame);

    /* forward frames always exist by construction */
    CHECK(TRACE_DEFINE_FRAME(NULL) != def_frame);
    trace_define_frame_destroy(def_frame);

    prev_var_frame = var_frame->prev_frame;
    trace_var_frame_destroy(var_frame);

    -- (self->length);
    var_frame = prev_var_frame;
  }

  /* make sure all the frames have been freed */
  CHECK(~0 == self->length);

  /* destroy frozen frame */
  trace_frozen_frame_destroy(trace_get_frozen_frame(self));

  /* get rid of extra lookup info */
  trace_dispose_lookup_cache(self);

  /* destroy internal symb table */
  if (!self->is_volatile) {
    SymbTable_destroy(self->st);
  }

  FREE(self);
} /* trace_destroy */


Trace_ptr
trace_copy (Trace_ptr self, TraceIter until_here,
            boolean is_volatile)
{
  Trace_ptr res = ALLOC(Trace, 1);

  TRACE_CHECK_INSTANCE(res);
  trace_init(res);

  { /* phase 0: initialize instance fields */
    trace_thaw(res); /* create it thawed, will be frozen as the last
                        step if a full copy of a frozen has been
                        required. */

    trace_set_type(res, trace_get_type(self));
    trace_set_desc(res, trace_get_desc(self));
    trace_set_language(res, trace_get_symb_table(self),
                       trace_get_symbols(self), is_volatile);
    trace_unregister(res);
    trace_reset_length(res);

    trace_setup_lookup_cache(res);
    trace_copy_lookup_data(res, self);

    trace_print_alloc_stats(res);
  } /* 0 */

  { /* phase 1: initial frame setup */
    TraceDefineFrame_ptr define_frame;
    TraceVarFrame_ptr initial_frame;

    /* setup frozen frame */
    trace_set_frozen_frame(res, trace_frozen_frame_create(res));

    /* assign the first and final frame to the initial var frame */
    initial_frame = trace_var_frame_create(res);

    trace_set_first_frame(res, initial_frame);
    trace_set_last_frame(res, initial_frame);

    /* setup  define frame for initial var frame */
    define_frame = trace_define_frame_create(res);
    trace_var_frame_set_fwd_define_frame(initial_frame, define_frame);
    trace_var_frame_set_bwd_define_frame(initial_frame,
                                         TRACE_DEFINE_FRAME(NULL));
  }

  /* phase 3: copy frozen data */
  {
    TraceFrozenFrame_ptr dst = trace_get_frozen_frame(res);
    TraceFrozenFrame_ptr src = trace_get_frozen_frame(self);
    trace_copy_section_storage(trace_frozen_frame_get_base(dst),
                               trace_frozen_frame_get_base(src),
                               trace_get_n_frozen_vars(res));
  }

  /* phase 4: copy var and define frame data until given iterator */
  {
    TraceIter src_iter = trace_first_iter(self);
    TraceIter dst_iter = trace_first_iter(res);

    while (TRACE_END_ITER != src_iter) {
      unsigned section;
      node_ptr* src_base;

      /* frozenvars have already been copied above */
      for (section = TRACE_SECTION_STATE_VAR;
           section < TRACE_SECTION_END; ++ section ) {

        src_base = trace_iter_get_section_base(self, src_iter, section, false);

        if ((node_ptr*)(NULL) != src_base) {
          trace_copy_section_storage(trace_iter_get_section_base(res, dst_iter,
                                                                 section, true),
                                     src_base,
                                     trace_get_n_section_symbols(self, section));
        }
      }
      if (src_iter == until_here) break; /* up to (included) until_here */
      src_iter = trace_iter_get_next(src_iter);

      /* append a new step to dest trace */
      dst_iter = trace_append_step(res);
    }
  }

  /* phase 5: (optional) copy loopback data from original trace if a
     complete copy was made and the original trace was frozen */
  {
    if (trace_is_frozen(self)) {
      TraceIter res_iter = trace_first_iter(res);
      TraceIter src_iter = trace_first_iter(self);

      while (TRACE_END_ITER != res_iter) {
        res_iter->loopback = src_iter->loopback;

        res_iter = trace_iter_get_next(res_iter);
        src_iter = trace_iter_get_next(src_iter);
      }

      /* ... and freeze copy in order to preserve loopbacks */
      trace_freeze(res);
    }
  }

  return res;
}  /* trace_copy */

/* returns true iff self and other have the same language */
static inline boolean
trace_cmp_language (Trace_ptr self, Trace_ptr other)
{
  NodeList_ptr l0 = trace_get_symbols(self);
  NodeList_ptr l1 = trace_get_symbols(other);
  ListIter_ptr i;

  if (NodeList_get_length(l0) != \
      NodeList_get_length(l1)) { return false; }

  /* if two sets have the same cardinality single inclusion is a
     sufficient condition for equality */
  NODE_LIST_FOREACH(l0, i) {
    if (! NodeList_belongs_to(l1,
                              NodeList_get_elem_at(l0, i))) {
      return false;
    }
  }

  return true;
} /* trace_cmp_language */


Trace_ptr
trace_concat (Trace_ptr self, Trace_ptr* other)
{
  nusmv_assert((Trace_ptr*)(NULL) != other); /* preventing nasty usages */

  /* Concat thaws self */
  trace_thaw(self);

  /* other is an empty trace */
  if (TRACE(NULL) == *other) return self;

  if (false == trace_cmp_language(self, *other)) {
    internal_error("%s:%d:%s: Trace languages mismatch detected. "
                   "[L(t1) != L(t2)]", __FILE__, __LINE__, __func__);
  }

  /* 1. Concat halts if other is registered (see Trace_concat docs) */
  if (trace_is_registered(*other)) {
    internal_error("%s:%d:%s: Cannot concatenate a registered trace",
                   __FILE__, __LINE__, __func__);
  }

  /* 2. merge frozen frames from both traces */
  {
    TraceSymbolsIter i0;
    node_ptr fv;

    TRACE_SYMBOLS_FOREACH(self, TRACE_ITER_F_VARS, i0, fv) {
      node_ptr v0 = trace_step_get_value(self, TRACE_END_ITER, fv);
      node_ptr v1 = trace_step_get_value(*other, TRACE_END_ITER, fv);

      /* if they're both non-Nil, they must be equal */
      nusmv_assert( !((Nil != v0) && (Nil != v1)) || v0 == v1 );

      /* merge frozenvars values (this can't fail) */
      trace_step_put_value(self, TRACE_END_ITER, fv, (Nil != v0) ? v0 : v1);
      }
  } /* merge frozens */

  /* 3. merge frontier state frames from both traces */
  {
    TraceSymbolsIter i0;
    TraceIter lhs = trace_last_iter(self);
    TraceIter rhs = trace_first_iter(*other);
    node_ptr fv;

    TRACE_SYMBOLS_FOREACH(self, TRACE_ITER_F_VARS, i0, fv) {
      node_ptr v0 = trace_step_get_value(self, lhs, fv);
      node_ptr v1 = trace_step_get_value(*other, rhs, fv);

      /* if they're both non-Nil, they must be equal */
      nusmv_assert( !((Nil != v0) && (Nil != v1)) || v0 == v1 );

      /* merge state vars values (this can't fail) */
      trace_step_put_value(self, lhs, fv, (Nil != v0) ? v0 : v1);
    }
  } /* merge frontier state */

  /* phase 4: consume "other" trace, link its data to self */
  {
    TraceIter lhs_link = trace_last_iter(self);
    TraceIter rhs_link = trace_iter_get_next(trace_first_iter(*other));

    trace_var_frame_set_next(lhs_link, TRACE_VAR_FRAME(rhs_link));
    if (TRACE_END_ITER != rhs_link) {
      TraceIter rhs_last_iter = trace_last_iter(*other);
      trace_set_last_frame(self, TRACE_VAR_FRAME(rhs_last_iter));
      self->length += (*other)->length;

      trace_var_frame_set_prev(rhs_link, TRACE_VAR_FRAME(lhs_link));
      trace_var_frame_set_bwd_define_frame(rhs_link,
                trace_var_frame_get_fwd_define_frame(TRACE_VAR_FRAME(lhs_link)));
    }

    /* dispose first var frame for other */
    trace_var_frame_destroy(TRACE_VAR_FRAME(trace_first_iter(*other)));

    if (!((*other)->is_volatile)) {
      SymbTable_destroy(trace_get_symb_table(*other));
    }

    /* dispose extra lookup info for other */
    trace_dispose_lookup_cache(*other);

    /* as a last step, FREE other and set it explicitly to NULL */
    FREE(*other); *other = TRACE(NULL);
  }

  return self;
} /* trace_concat */


/* horizontal iterators */
TraceIter
trace_iter_get_next(const TraceIter iter)
{
  return TRACE_ITER( TRACE_VAR_FRAME(iter) -> next_frame );
}

TraceIter
trace_iter_get_prev(const TraceIter iter)
{
  return TRACE_ITER( TRACE_VAR_FRAME(iter) -> prev_frame );
}

/* step iterator factory */
TraceStepIter
trace_step_iter (const Trace_ptr trace, const TraceIter step,
                 TraceIteratorType iter_type)
{
  TraceStepIter res;

  /* setting up iterator */
  res.trace = trace;
  res.step = step;
  res.type = iter_type;
  res.cursor = 0;

  /* pick first non empty section */
  res.section = TRACE_SECTION_FROZEN_VAR; do {
    if ((0 != (res.type & (1 << res.section))) &&
        (node_ptr*)(NULL) != \
        trace_iter_get_section_base(res.trace, res.step,
                                    res.section, false)) {
      /* found section, return */
      break;
    }

    ++ res.section;
  } while (res.section != TRACE_SECTION_END);

  return res;
} /* trace_step_iter */


/* symbols iterator factory */
TraceSymbolsIter
trace_symbols_iter (const Trace_ptr trace, TraceIteratorType iter_type)
{
  TraceSymbolsIter res;

  /* setting up iterator */
  res.trace = trace;
  res.type = iter_type;
  res.cursor = 0;

  /* pick first non empty section */
  res.section = TRACE_SECTION_FROZEN_VAR; do {
    if ((0 != (res.type & (1 << res.section))))  {
      /* found section, return */
      break;
    }

    ++ res.section;
  } while (res.section != TRACE_SECTION_END);

  return res;
} /* trace_symbols_iter */


/* DANGER: hazardous area */
boolean
trace_step_iter_fetch (TraceStepIter* iter,
                       node_ptr* symb_name, node_ptr* symb_value)
{
  node_ptr* base;
  node_ptr value = Nil;

  while(true) {
    /* end of iteration? */
    if (TRACE_SECTION_END == iter->section) return false;

    base = trace_iter_get_section_base(iter->trace, iter->step,
                                       iter->section, false);
    CHECK((node_ptr*)(NULL) != base);

    /* run through the value vector until a non-Nil node is
       encountered, this cycle is guaranteed to terminate due to
       sentinel nodes */
    while (Nil == value) {
      value = *(base + iter->cursor) ;
      ++ (iter->cursor);
    }

    /* found value, yield assignment */
    if (FAILURE != node_get_type(value)) break;

    /* get next non-empty section */
    while (TRACE_SECTION_END != iter->section) {
      ++ (iter->section);
      if ((0 != (iter->type & (1 << iter->section))) &&
          (node_ptr*)(NULL) !=  \
          trace_iter_get_section_base(iter->trace, iter->step,
                                      iter->section, false)) {

        /* found section, resume iteration */
        iter->cursor = 0; value = Nil;
        break;
      }
    }
  } /* infinite loop */

  /* if this point has been reached, there is a valid assignment,
     perform bwd lookup to determine symbol name */
  (*symb_value) = value;
  (*symb_name) = trace_symbol_bwd_lookup(iter->trace,
                                         iter->section, iter->cursor-1);

  return true; /* hit */
} /* trace_step_iter_fetch */


/* DANGER: hazardous area */
boolean
trace_symbols_iter_fetch (TraceSymbolsIter* iter, node_ptr* symb_name)
{
  node_ptr* base;
  node_ptr* addr;

  while(true) {
    /* end of iteration? */
    if (TRACE_SECTION_END == iter->section) return false;

    /* run through the symbol vector until a sentinel node is
       encountered, this cycle is guaranteed to terminate due to
       sentinel nodes */
    base = trace_get_section_symbols(iter->trace, iter->section);
    if ((node_ptr*)(NULL) != base) {
      addr =  base + iter->cursor;
      ++ iter->cursor;

      /* yield symbol */
      if (FAILURE != node_get_type(*addr)) break;
    }

    /* get next non-empty section */
    while (TRACE_SECTION_END != iter->section) {
      ++ (iter->section);
      if ((0 != (iter->type & (1 << iter->section)))) {

        /* found section, resume iteration */
        iter->cursor = 0;
        break;
      }
    }
  } /* infinite loop */

  /* if this point has been reached, there is a valid symbol, */
  (*symb_name) =  (*addr);

  return true; /* hit */
} /* trace_symbols_iter_fetch */


/* low level initializers */
static inline void
trace_init (Trace_ptr self)
{
  memset(self, 0, sizeof(struct Trace_TAG));
}

static inline void
trace_frozen_frame_init(TraceFrozenFrame_ptr self)
{
  memset(self, 0, sizeof(struct TraceFrozenFrame_TAG));
}

static inline void
trace_var_frame_init(TraceVarFrame_ptr self)
{
  memset(self, 0, sizeof(struct TraceVarFrame_TAG));
}

static inline void
trace_define_frame_init(TraceDefineFrame_ptr self)
{
  memset(self, 0, sizeof(struct TraceDefineFrame_TAG));
}

/* trace metadata getters/setters */
static inline void
trace_reset_length(const Trace_ptr self)
{
  self->length = 0;
}

static inline void
trace_inc_length(const Trace_ptr self)
{
  ++ ( self->length );
}

unsigned
trace_get_length(const Trace_ptr self)
{
  return self->length;
}

boolean
trace_is_empty(const Trace_ptr self)
{
  TraceSymbolsIter i0;
  TraceIter iter;
  node_ptr fv;

  if (0 != self->length) return false;

  iter = trace_first_iter(self);

  TRACE_SYMBOLS_FOREACH(self, TRACE_ITER_SF_SYMBOLS, i0, fv) {
    node_ptr v1 = trace_step_get_value(self, iter, fv);
    if (Nil != v1) return false;
  }
  return true;
}

/* process a language at trace creation time (create, copy) */
static inline void
trace_set_language(Trace_ptr self, SymbTable_ptr st,
                   NodeList_ptr symbols, boolean is_volatile)
{
  ListIter_ptr iter;
  SymbTableIter stiter;
  TypeChecker_ptr tc;

  nusmv_assert(SYMB_TABLE(NULL) == self->st);
  SYMB_TABLE_CHECK_INSTANCE(st);

  self->is_volatile = is_volatile;
  if (is_volatile) {
    self->st = st;
  }
  else {
    self->st = SymbTable_copy(st, Set_MakeEmpty());
  }
  tc = SymbTable_get_type_checker(self->st);

  self->symbols = NodeList_create();
  if (NODE_LIST(NULL) != symbols) {
    NODE_LIST_FOREACH(symbols, iter) {
      node_ptr sym  = NodeList_get_elem_at(symbols, iter);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        char *repr = sprint_node(sym);
        fprintf(nusmv_stderr,
                "%s:%d:%s: considering symbol '%s' for inclusion in language\n",
                __FILE__, __LINE__, __func__, repr);
        FREE(repr);
      }

      /* get rid of encoding bits */
      if (BIT == (node_get_type(sym))) continue;

      {
        SymbType_ptr sym_type = TypeChecker_get_expression_type(tc, sym, Nil);

        /* get rid of array defines */
        if (SymbType_is_array(sym_type)) {
          continue;
        }
      }

      { /* only valid symbols are allowed into traces */
        SymbCategory cat = SymbTable_get_symbol_category(st, sym);

        if (SYMBOL_INVALID == cat) {
          internal_error("%s:%d:%s: Invalid symbol detected (%s)",
                         __FILE__, __LINE__, __func__, sprint_node(sym));
        }
      }

      { /* skip variables in layers registered in ARTIFACTS_LAYERS_CLASS
           (e.g. LTL tableau) */
        const char* layer_name = \
          SymbLayer_get_name(SymbTable_symbol_get_layer(st, sym));

        nusmv_assert(NIL(char) != layer_name);

        if (SymbTable_layer_class_exists(st, ARTIFACTS_LAYERS_CLASS) && \
            SymbTable_is_layer_in_class(st, layer_name, ARTIFACTS_LAYERS_CLASS)){
          continue ;
        }
      }

      /* here the symbol can become part of the trace language */
      CHECK(!NodeList_belongs_to(self->symbols, sym));

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        char *repr = sprint_node(sym);
        fprintf(nusmv_stderr, "%s:%d:%s: adding symbol '%s' to language\n",
                __FILE__, __LINE__, __func__, repr);
        FREE(repr);
      }

      NodeList_append(self->symbols, sym);
    }
  } /* symbols != NULL */

  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4) && \
      0 == NodeList_get_length(self->symbols)) {

    fprintf(nusmv_stderr, "Warning: Trace has empty language.\n");
  }

  /* build s, sf and i vars lists */
  nusmv_assert(NODE_LIST(NULL) == self->s_vars);
  self->s_vars = NodeList_create();
  SYMB_TABLE_FOREACH(st, stiter, STT_STATE_VAR) {
    node_ptr v = SymbTable_iter_get_symbol(st, &stiter);
    if (trace_symbol_in_language(self, v)) {
      NodeList_append(self->s_vars, v);
    }
  }
  nusmv_assert(NODE_LIST(NULL) == self->sf_vars);
  self->sf_vars = NodeList_create();
  SYMB_TABLE_FOREACH(st, stiter, STT_STATE_VAR | STT_FROZEN_VAR) {
    node_ptr v = SymbTable_iter_get_symbol(st, &stiter);
    if (trace_symbol_in_language(self, v)) {
      NodeList_append(self->sf_vars, v);
    }
  }
  nusmv_assert(NODE_LIST(NULL) == self->i_vars);
  self->i_vars = NodeList_create();
  SYMB_TABLE_FOREACH(st, stiter, STT_INPUT_VAR) {
    node_ptr v = SymbTable_iter_get_symbol(st, &stiter);
    if (trace_symbol_in_language(self, v)) {
      NodeList_append(self->i_vars, v);
    }
  }
} /* trace_set_language */

NodeList_ptr
trace_get_symbols(const Trace_ptr self)
{
  CHECK(NODE_LIST(NULL) != self->symbols);
  return self->symbols;
}

SymbTable_ptr
trace_get_symb_table(const Trace_ptr self)
{
  CHECK(SYMB_TABLE(NULL) != self->st);
  return self->st;
}

NodeList_ptr
trace_get_s_vars(const Trace_ptr self)
{
  CHECK(NODE_LIST(NULL) != self->s_vars);
  return self->s_vars;
}

NodeList_ptr
trace_get_sf_vars(const Trace_ptr self)
{
  CHECK(NODE_LIST(NULL) != self->sf_vars);
  return self->sf_vars;
}

NodeList_ptr
trace_get_i_vars(const Trace_ptr self)
{
  CHECK(NODE_LIST(NULL) != self->i_vars);
  return self->i_vars;
}

boolean
trace_is_registered(Trace_ptr self)
{
  return (TRACE_UNREGISTERED != trace_get_id(self));
}

void
trace_register(Trace_ptr self, int id)
{
  nusmv_assert(id != TRACE_UNREGISTERED); /* reserved */
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr, "trace is now registered (id = %d)\n", id);
  }

  self->id = id;
}

void
trace_unregister(Trace_ptr  self)
{
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr, "trace is now unregistered\n");
  }

  self->id = TRACE_UNREGISTERED;
}

int
trace_get_id(const Trace_ptr self)
{
  return self->id;
}

TraceType
trace_get_type (const Trace_ptr self)
{
  return self->type;
}

void
trace_set_type(const Trace_ptr self, TraceType type)
{
  nusmv_assert((TRACE_TYPE_UNSPECIFIED <= type) && (type < TRACE_TYPE_END));
  self->type = type;
}

const char*
trace_get_desc(const Trace_ptr self)
{
  return NIL(char) != self->desc ? self->desc : TRACE_DEFAULT_DESC;
}

void
trace_set_desc(Trace_ptr self, const char* desc)
{
  if (NIL(const char) != self->desc) {
    FREE(self->desc);
  }

  if (NIL(char) != desc) {
    self->desc = util_strsav(desc);
  }
  else {
    self->desc = NIL(char);
  }
}

boolean
trace_is_volatile(const Trace_ptr self)
{
  return self->is_volatile;
}

/* horizontal iterators */
TraceIter trace_first_iter(const Trace_ptr self)
{
  return TRACE_ITER(self->first_frame);
}


/* efficiently retrieves ith iter (bi-directional search) */
TraceIter trace_ith_iter(const Trace_ptr self, unsigned i)
{
  unsigned fwd_distance = i - 1;
  unsigned bwd_distance = 1 + trace_get_length(self) - i;
  TraceIter res;

  nusmv_assert(i > 0);

  if (fwd_distance < bwd_distance) {
    res = trace_first_iter(self);
    while (0 != fwd_distance) {
      CHECK(TRACE_END_ITER != res);
      res = trace_iter_get_next(res);
      -- fwd_distance;
    }
  }
  else { /* bwd_distance <= fwd_distance */
    res = trace_last_iter(self);
    while (0 != bwd_distance) {
      CHECK(TRACE_END_ITER != res);
      res = trace_iter_get_prev(res);
      -- bwd_distance;
    }
  }

  return res;
} /* trace_ith_iter */

/* iter -> i (dual of the previous) */
unsigned trace_iter_i(const Trace_ptr self, TraceIter iter)
{
  unsigned  i = 1;
  TraceIter step = trace_first_iter(self);
  while  (step != iter) {
    ++ i;
    step = trace_iter_get_next(step);
  }

  return i;
}

TraceIter trace_last_iter(const Trace_ptr self)
{
  return TRACE_ITER(self->last_frame);
}

/* frames management */
static inline TraceFrozenFrame_ptr
trace_get_frozen_frame (const Trace_ptr self)
{
  return self->frozen_frame;
}

static inline void
trace_set_frozen_frame(Trace_ptr self, TraceFrozenFrame_ptr frozen_frame)
{
  self->frozen_frame = frozen_frame;
}

static inline void
trace_set_first_frame(Trace_ptr self, TraceVarFrame_ptr var_frame)
{
  self->first_frame = var_frame;
}

static inline void
trace_set_last_frame(Trace_ptr self, TraceVarFrame_ptr var_frame)
{
  self->last_frame = var_frame;
}

/* frame constructors/destructors */
static TraceVarFrame_ptr
trace_var_frame_create (Trace_ptr self)
{
  TraceVarFrame_ptr res = ALLOC(TraceVarFrame, 1);
  TRACE_VAR_FRAME_CHECK_INSTANCE(res);
  trace_var_frame_init(res);

  trace_var_frame_set_state_base(res,
     trace_setup_section_storage(trace_get_n_state_vars(self)));

  trace_var_frame_set_input_base(res,
     trace_setup_section_storage(trace_get_n_input_vars(self)));

  trace_var_frame_set_fwd_define_frame(res, TRACE_DEFINE_FRAME(NULL));
  trace_var_frame_set_bwd_define_frame(res, TRACE_DEFINE_FRAME(NULL));

  trace_var_frame_set_next_frame(res, TRACE_VAR_FRAME(NULL));
  trace_var_frame_set_prev_frame(res, TRACE_VAR_FRAME(NULL));

  return res;
}

static inline void
trace_var_frame_destroy(TraceVarFrame_ptr var_frame)
{
  TRACE_VAR_FRAME_CHECK_INSTANCE(var_frame);

  CHECK((node_ptr*)(NULL) != var_frame->state_values);
  FREE(var_frame->state_values);

  CHECK((node_ptr*)(NULL) != var_frame->input_values);
  FREE(var_frame->input_values);

  FREE(var_frame);
}

TraceDefineFrame_ptr
trace_define_frame_create (Trace_ptr self)
{
  TraceDefineFrame_ptr res = ALLOC(TraceDefineFrame, 1);
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(res);

  memset(res, 0, sizeof(TraceDefineFrame));
  return res;
}

static inline void
trace_define_frame_destroy(TraceDefineFrame_ptr def_frame)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(def_frame);

  if ((node_ptr*)(NULL) != def_frame->s_values) { FREE(def_frame->s_values); }
  if ((node_ptr*)(NULL) != def_frame->i_values) { FREE(def_frame->i_values); }
  if ((node_ptr*)(NULL) != def_frame->si_values) { FREE(def_frame->si_values); }
  if ((node_ptr*)(NULL) != def_frame->n_values) { FREE(def_frame->n_values); }
  if ((node_ptr*)(NULL) != def_frame->sn_values) { FREE(def_frame->sn_values); }
  if ((node_ptr*)(NULL) != def_frame->in_values) { FREE(def_frame->in_values); }
  if ((node_ptr*)(NULL) != def_frame->sin_values) { FREE(def_frame->sin_values);}

  FREE(def_frame);
}

static TraceFrozenFrame_ptr
trace_frozen_frame_create (Trace_ptr self)
{
  TraceFrozenFrame_ptr res = ALLOC(TraceFrozenFrame, 1);
  TRACE_FROZEN_FRAME_CHECK_INSTANCE(res);
  trace_frozen_frame_init(res);

  res->frozen_values = \
    trace_setup_section_storage(trace_get_n_frozen_vars(self));

  return res;
}

static void
trace_frozen_frame_destroy(TraceFrozenFrame_ptr frozen_frame)
{
  TRACE_FROZEN_FRAME_CHECK_INSTANCE(frozen_frame);

  CHECK((node_ptr*)(NULL) != frozen_frame->frozen_values);
  FREE(frozen_frame->frozen_values);

  FREE(frozen_frame);
}

TraceIter trace_append_step(Trace_ptr self)
{
  TraceVarFrame_ptr var_frame = trace_var_frame_create(self);
  TraceVarFrame_ptr last_frame = TRACE_VAR_FRAME(trace_last_iter(self));

  CHECK(trace_is_thawed(self));

  trace_var_frame_set_prev(var_frame, last_frame);
  if (TRACE_VAR_FRAME(NULL) != last_frame) {
    trace_var_frame_set_next(last_frame, var_frame);
    trace_var_frame_set_bwd_define_frame(var_frame,
              trace_var_frame_get_fwd_define_frame(last_frame));
  }

  trace_var_frame_set_fwd_define_frame(var_frame,
                                       trace_define_frame_create(self));

  /* update trace info */
  trace_inc_length(self);
  trace_set_last_frame(self, var_frame);

  return trace_last_iter(self);
}

boolean
trace_symbol_in_language (Trace_ptr self, node_ptr symb)
{
  return NodeList_belongs_to(self->symbols, symb);
}

SymbCategory
trace_symbol_get_category (Trace_ptr self, node_ptr symb)
{
  TraceSection section;
  unsigned offset; /* don't care */
  boolean hit;

  hit = trace_symbol_fwd_lookup(self, symb, &section, &offset);
  if (!hit) {
    internal_error("%s:%d:%s: Symbol not found (%s)",
                   __FILE__, __LINE__, __func__, sprint_node(symb));
  }

  return trace_section_to_category(section);
}

boolean
trace_symbol_is_assigned (Trace_ptr self, TraceIter step, node_ptr symb)
{
  TraceSection section;
  unsigned offset;
  node_ptr value;
  node_ptr *base;
  boolean hit;

  hit = trace_symbol_fwd_lookup(self, symb, &section, &offset);
  if (!hit) {
    internal_error("%s:%d:%s: Symbol not found (%s)",
                   __FILE__, __LINE__, __func__, sprint_node(symb));
  }

  base = trace_iter_get_section_base(self, step, section, false);
  if ((node_ptr*)(NULL) == base) return false;

  value = *(base + offset);
  return (Nil != value);
}

/* exported loopback predicate */
boolean
trace_step_is_loopback(Trace_ptr self, const TraceIter step)
{
  /* loopback has no semantics in simulation */
  if (TRACE_TYPE_SIMULATION == trace_get_type(self)) return false;

  /* 0-length loopbacks are not valid  */
  if (step == trace_last_iter(self)) return false;

  if (trace_is_thawed(self)) {
    return trace_step_test_loopback(self, step);
  }
  else { /* trace_is_frozen(self) */
    return step->loopback;
  }
} /* trace_step_is_loopback */


/* low level loopback predicate */
boolean
trace_step_test_loopback(Trace_ptr self, const TraceIter step)
{
  unsigned nvars = trace_get_n_state_vars(self);

  node_ptr* step_base ;
  node_ptr* last_base ;

  step_base = trace_var_frame_get_state_base(TRACE_VAR_FRAME(step));

  last_base = \
    trace_var_frame_get_state_base(TRACE_VAR_FRAME(trace_last_iter(self)));

  /* perform low level full comparison, only state vars are relevant
     w.r.t. loopback calculation */
  return (0 == trace_compare_section_storage(step_base, last_base, nvars));
} /* trace_step_test_loopback */

void trace_step_force_loopback (const Trace_ptr self, TraceIter step)
{
  nusmv_assert(trace_is_frozen(self));

  if (!trace_step_test_loopback(self, step)) {
    fprintf(nusmv_stderr, "WARNING: state not a loopback state\n");
  }

  step->loopback = true;
}

boolean trace_is_frozen (const Trace_ptr self)
{
  return self->frozen;
}

boolean trace_is_thawed (const Trace_ptr self)
{
  return !self->frozen;
}

void trace_freeze (Trace_ptr self)
{
  /* if trace was previously thawed we clear explicit loopback
     information */
  if (trace_is_thawed(self)) {
    TraceIter step;
    TRACE_FOREACH(self, step) { step->loopback = false; }
  }
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr, "trace (id = %d) is now frozen\n", trace_get_id(self));
  }
  self->frozen = true;
}

void trace_thaw (Trace_ptr self)
{
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr, "trace (id = %d) is now thawed\n", trace_get_id(self));
  }
  self->frozen = false;
}

/* equality predicate among traces */
boolean trace_equals(const Trace_ptr self, const Trace_ptr other)
{
  TraceIter self_step;
  TraceIter other_step;

  if (self == other) return true;

  /* they can't be both NULL here */
  if (TRACE(NULL) == self || TRACE(NULL) == other) return false;

  if (trace_get_length(self) != trace_get_length(other)) return false;

  if (!trace_cmp_language(self, other)) return false;

  /* test frozenvars for equality */
  if (0 != trace_compare_section_storage(
                trace_frozen_frame_get_base(trace_get_frozen_frame(self)),
                trace_frozen_frame_get_base(trace_get_frozen_frame(other)),
                trace_get_n_frozen_vars(self))) {

    return 1;
  }

  self_step = trace_first_iter(self);
  CHECK(TRACE_END_ITER != self_step);

  other_step = trace_first_iter(other);
  CHECK(TRACE_END_ITER != other_step);

  while (TRACE_END_ITER != self_step) {

    if (0 != trace_compare_section_storage(
                  trace_var_frame_get_input_base(TRACE_VAR_FRAME(self_step)),
                  trace_var_frame_get_input_base(TRACE_VAR_FRAME(other_step)),
                  trace_get_n_input_vars(self))) {

      return false;
    }

    if (0 != trace_compare_section_storage(
                  trace_var_frame_get_state_base(TRACE_VAR_FRAME(self_step)),
                  trace_var_frame_get_state_base(TRACE_VAR_FRAME(other_step)),
                  trace_get_n_state_vars(self))) {

      return false;
    }

    /* one is loopback the other is not */
    if ((trace_step_is_loopback(self, self_step) &&
        !trace_step_is_loopback(other, other_step)) ||

        (!trace_step_is_loopback(self, self_step) &&
         trace_step_is_loopback(other, other_step))) {

      return false;
    }

    self_step = trace_iter_get_next(self_step);
    other_step = trace_iter_get_next(other_step);

    if (TRACE_END_ITER == self_step) {
      CHECK(TRACE_END_ITER == other_step);
      break;
    }
  }

  /* We've come to the end of both traces without findind any
     difference. Traces are equals */
  return true;
}


/* 
/* 
/* static inline boolean trace_expr_is_false(const Expr_ptr expr) */
/* { */
/*   return node_get_type(expr) == FALSEEXP || */
/*     (node_get_type(expr) == NUMBER && (0 == node_get_int(expr))); */
/* } */

/* static inline boolean trace_expr_is_true(const Expr_ptr expr) */
/* { */
/*   return node_get_type(expr) == TRUEEXP || */
/*     (node_get_type(expr) == NUMBER && (1 == node_get_int(expr))); */
/* } */

/* returns true iff a valid value for a var in language is given. */
boolean  trace_step_put_value(Trace_ptr self, TraceIter step,
                              node_ptr symb, node_ptr value)
{
  TraceSection section;
  unsigned offset;
  boolean res = false;
  SymbTable_ptr symb_table = Trace_get_symb_table(self);

  if (trace_symbol_fwd_lookup(self, symb, &section, &offset)) {
    node_ptr* base;
    node_ptr* addr;

    /* when trace is frozen only defines are allowed */
    CHECK(section >= TRACE_SECTION_STATE_DEFINE || trace_is_thawed(self));

    base = trace_iter_get_section_base(self, step, section, true);
    CHECK((node_ptr*)(NULL) != base); /* base is non NULL */

    addr = base + offset;
    CHECK(Nil == *addr || *addr == value); /* value is blank or matching */

    res = (TypeChecker_is_expression_wellformed(            \
    SymbTable_get_type_checker(trace_get_symb_table(self)), \
    UNION == node_get_type(value)                      \
    ? Expr_setin(symb, value, symb_table)                          \
    : Expr_equal(symb, value, symb_table), Nil));

#if defined TRACE_DEBUG
    if (!res) { /* type error */
      fprintf(nusmv_stderr, "--> suspicious assignment was: %s\n",
              sprint_node(UNION == node_get_type(value)
                          ? Expr_setin(symb, value, symb_table)
                          : Expr_equal(symb, value, symb_table)));
    }
#endif

    /* write data */
    (*addr) = value;
  } /* lookup miss, value is ignored */

  return res;
} /* trace_step_put_value */

/* returns a valid node_ptr iff a symbol in in trace language is
   given. If a symbol not in language is give raises an internal
   error */
 node_ptr
trace_step_get_value(Trace_ptr self, TraceIter step, node_ptr symb)
{
  TraceSection section;
  unsigned offset;

  if (trace_symbol_fwd_lookup(self, symb, &section, &offset)) {
    node_ptr* base;

    base = trace_iter_get_section_base(self, step, section, false);
    if ((node_ptr*)(NULL) == base) return Nil; /* sect not allocated */

    return *(base + offset);
  }

  /* symbol not in language */
  internal_error("%s:%d:%s:  symbol not in language (%s).",
                 __FILE__, __LINE__, __func__, sprint_node(symb));
  return Nil;
} /* trace_step_get_value */


/* low-level storage functions */
static inline node_ptr*
trace_setup_section_storage (unsigned n_symbs)
{
  /* allocate and setup storage for symbols (1 extra location at the end
     of the section storage is used for sentinel node) */
  node_ptr* res = ALLOC(node_ptr, 1 + n_symbs);

  nusmv_assert((node_ptr*)(NULL) != res);
  memset(res, 0, n_symbs * sizeof(node_ptr));
  *(res + n_symbs) = SENTINEL;

  return res;
}

static inline void
trace_insert_lookup_info(Trace_ptr trace, node_ptr symb_name,
                         TraceSection section, unsigned offset)
{
  CHECK(Nil == find_assoc(trace->symb2section, symb_name));
  insert_assoc(trace->symb2section, symb_name, NODE_FROM_INT(section));

  CHECK(Nil == find_assoc(trace->symb2address, symb_name));
  insert_assoc(trace->symb2address, symb_name, NODE_FROM_INT(offset));

  {
    SymbTable_ptr st = trace_get_symb_table(trace);
    SymbLayer_ptr layer = SymbTable_symbol_get_layer(st, symb_name);
    SYMB_LAYER_CHECK_INSTANCE(layer);
    insert_assoc(trace->symb2layername, symb_name,
                 (node_ptr)(SymbLayer_get_name(layer)));
  }

  CHECK((node_ptr*)(NULL) != trace->buckets[section]);
  *(trace->buckets[section] + offset) = symb_name;
}

const char*
trace_get_layer_from_symb (const Trace_ptr trace, const node_ptr symb)
{
  CHECK(Nil != symb);
  return (const char*)(find_assoc(trace->symb2layername, symb));
}

static inline void
trace_print_alloc_stats(Trace_ptr self)
{
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4)) {
    TraceSymbolsIter iter;
    node_ptr symb;

    fprintf(nusmv_stderr, "\n\n=======================\n");
    fprintf(nusmv_stderr, "Trace allocation stats:\n");
    fprintf(nusmv_stderr, "=======================\n\n");

    fprintf(nusmv_stderr, "frozen variables [ %d ] : ",
            trace_get_n_frozen_vars(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_F_VARS, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }
    fprintf(nusmv_stderr, "\n");

    fprintf(nusmv_stderr, "state variables: [ %d ] : ",
            trace_get_n_state_vars(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_S_VARS, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }
    fprintf(nusmv_stderr, "\n");

    fprintf(nusmv_stderr, "input variables: [ %d ] : ",
            trace_get_n_input_vars(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_I_VARS, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }
    fprintf(nusmv_stderr, "\n");

    fprintf(nusmv_stderr, "state defines:  [ %d ] : ",
            trace_get_n_state_defines(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_S_DEFINES, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }
    fprintf(nusmv_stderr, "\n");

    fprintf(nusmv_stderr, "input defines: [ %d ] : ",
            trace_get_n_input_defines(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_I_DEFINES, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }
    fprintf(nusmv_stderr, "\n");

    fprintf(nusmv_stderr, "state-input defines: [ %d ] : ",
            trace_get_n_state_input_defines(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_SI_DEFINES, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }
    fprintf(nusmv_stderr, "\n");

    fprintf(nusmv_stderr, "next defines: [ %d ] : ",
            trace_get_n_next_defines(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_N_DEFINES, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }
    fprintf(nusmv_stderr, "\n");

    fprintf(nusmv_stderr, "state-next defines: [ %d ] : ",
            trace_get_n_state_next_defines(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_SN_DEFINES, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }
    fprintf(nusmv_stderr, "\n");

    fprintf(nusmv_stderr, "input-next defines: [ %d ] : ",
            trace_get_n_input_next_defines(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_IN_DEFINES, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }
    fprintf(nusmv_stderr, "\n");

    fprintf(nusmv_stderr, "state-input-next defines: [ %d ] : ",
            trace_get_n_state_input_next_defines(self));
    TRACE_SYMBOLS_FOREACH(self,  TRACE_ITER_SIN_DEFINES, iter, symb) {
      print_node(nusmv_stderr, symb); fprintf(nusmv_stderr, " ");
    }

    fprintf(nusmv_stderr, "\n");
    fprintf(nusmv_stderr, "\n");
  }
} /* trace_print_alloc_stats */

static inline void
trace_init_lookup_data(Trace_ptr self)
{
  SymbTable_ptr st = trace_get_symb_table(self);
  NodeList_ptr symbols;
  ListIter_ptr iter;
  int i;

  unsigned counters[TRACE_SECTION_END]; /* all set to zero */
  memset(counters, 0,  sizeof(counters));

  symbols = trace_get_symbols(self);

/* Categorize each symbol and increment section
     appropriate bucket counter */
  NODE_LIST_FOREACH(symbols, iter) {
    node_ptr symb_name = NodeList_get_elem_at(symbols, iter);
    TraceSection section =
      trace_category_to_section(SymbTable_get_symbol_category(st, symb_name));
    trace_inc_n_section_symbols(self, section);
  }

  /* allocate sections */
  for (i = TRACE_SECTION_FROZEN_VAR; i < TRACE_SECTION_END; ++ i) {
    trace_setup_lookup_section(self, i);
  }

  NODE_LIST_FOREACH(symbols, iter) {
    node_ptr symb_name = NodeList_get_elem_at(symbols, iter);
    TraceSection section = \
      trace_category_to_section(SymbTable_get_symbol_category(st, symb_name));

    trace_insert_lookup_info(self, symb_name, section, counters[section] ++ );
  }
} /* trace_init_lookup_data */

static inline void
trace_copy_lookup_data(Trace_ptr self, Trace_ptr src)
{
  SymbTable_ptr st = trace_get_symb_table(self);

  NodeList_ptr symbols;
  ListIter_ptr iter;
  int i;

  unsigned counters[TRACE_SECTION_END]; /* all set to zero */
  memset(counters, 0,  sizeof(counters));

  /* copies cardinality information for buckets */
  memcpy(self->n_buckets, src->n_buckets, TRACE_SECTION_END * sizeof(unsigned));

  /* Copy lookup structures */
  for (i = TRACE_SECTION_FROZEN_VAR; i < TRACE_SECTION_END; ++ i) {
    trace_setup_lookup_section(self, i);
  }

  symbols = trace_get_symbols(self);
  NODE_LIST_FOREACH(symbols, iter) {
    node_ptr symb_name = NodeList_get_elem_at(symbols, iter);
    TraceSection section = \
      trace_category_to_section(SymbTable_get_symbol_category(st, symb_name));

    trace_insert_lookup_info(self, symb_name, section, counters[section] ++ );
  }
}

static inline void
trace_copy_section_storage (node_ptr* dest, node_ptr* src,
                            unsigned n_symbs)
{
  CHECK((node_ptr*)(NULL) != dest && (node_ptr*)(NULL) != src);
  memcpy(dest, src, n_symbs * sizeof(node_ptr));
}

static inline int
trace_compare_section_storage(node_ptr* dest, node_ptr* src,
                              unsigned n_symbs)
{
  CHECK((node_ptr*)(NULL) != dest && (node_ptr*)(NULL) != src);
  return memcmp(dest, src, n_symbs * sizeof(node_ptr));
}

/* var frames getters/setters */
static inline void
trace_var_frame_set_prev (TraceVarFrame_ptr frame, TraceVarFrame_ptr prev)
{
  frame->prev_frame = prev;
}

static inline TraceVarFrame_ptr
trace_var_frame_get_prev (TraceVarFrame_ptr frame)
{
  return frame->prev_frame;
}

static inline void
trace_var_frame_set_next (TraceVarFrame_ptr frame, TraceVarFrame_ptr next)
{
  frame->next_frame = next;
}

static inline TraceVarFrame_ptr
trace_var_frame_get_next (TraceVarFrame_ptr frame)
{
  return frame->next_frame;
}

static inline TraceDefineFrame_ptr
trace_var_frame_get_fwd_define_frame(TraceVarFrame_ptr var_frame)
{
  return var_frame->fwd_define_frame;
}

static inline TraceDefineFrame_ptr
trace_var_frame_get_bwd_define_frame(TraceVarFrame_ptr var_frame)
{
  return var_frame->bwd_define_frame;
}

static inline void
trace_var_frame_set_fwd_define_frame(TraceVarFrame_ptr var_frame,
                                     TraceDefineFrame_ptr def_frame)
{
  var_frame->fwd_define_frame  = def_frame;
}

static inline void
trace_var_frame_set_bwd_define_frame(TraceVarFrame_ptr var_frame,
                                     TraceDefineFrame_ptr def_frame)
{
  var_frame->bwd_define_frame = def_frame;
}

static inline TraceVarFrame_ptr
trace_var_frame_get_next_frame(TraceVarFrame_ptr var_frame)
{
  return var_frame->next_frame;
}

static inline TraceVarFrame_ptr
trace_var_frame_get_prev_frame(TraceVarFrame_ptr var_frame)
{
  return var_frame->prev_frame;
}

static inline void
trace_var_frame_set_next_frame(TraceVarFrame_ptr var_frame,
                               TraceVarFrame_ptr next_frame)
{
  var_frame->next_frame = next_frame;
}

static inline void
trace_var_frame_set_prev_frame(TraceVarFrame_ptr var_frame,
                               TraceVarFrame_ptr prev_frame)
{
  var_frame->prev_frame = prev_frame;
}

/* low level addressing base getter given (trace, iter, section)
   coordinates.  If section is not found and create to true, section
   is created on demand and base address returned (this applies to
   define sections only). */
static inline node_ptr*
trace_iter_get_section_base(Trace_ptr trace, TraceIter iter,
                            TraceSection section, boolean create)
{
  switch (section) {

  case TRACE_SECTION_FROZEN_VAR:
    CHECK( (node_ptr*) NULL !=                                          \
           trace_frozen_frame_get_base(trace_get_frozen_frame(trace)) );
    return trace_frozen_frame_get_base(trace_get_frozen_frame(trace));

  case TRACE_SECTION_STATE_VAR:
    CHECK( TRACE_END_ITER != iter );
    CHECK( (node_ptr*) NULL !=                                          \
           trace_var_frame_get_state_base(TRACE_VAR_FRAME(iter)) );
    return trace_var_frame_get_state_base(TRACE_VAR_FRAME(iter));

  case TRACE_SECTION_INPUT_VAR:
    CHECK( TRACE_END_ITER != iter );
    CHECK( (node_ptr*) NULL !=                                          \
           trace_var_frame_get_input_base(TRACE_VAR_FRAME(iter)) );
    return trace_var_frame_get_input_base(TRACE_VAR_FRAME(iter));

  case TRACE_SECTION_STATE_DEFINE:
    {
      TraceVarFrame_ptr vframe;
      TraceDefineFrame_ptr dframe;
      node_ptr* res;

      vframe = TRACE_VAR_FRAME(iter);
      CHECK (TRACE_VAR_FRAME(NULL) != vframe);

      dframe  = trace_var_frame_get_fwd_define_frame(vframe);
      CHECK (TRACE_DEFINE_FRAME(NULL) != dframe);

      res = trace_define_frame_get_state_base(dframe);
      if ((node_ptr*)(NULL) == res && create) {
        res =  trace_setup_section_storage(trace_get_n_state_defines(trace));
        trace_define_frame_set_state_base(dframe, res);
      }

      return res;
    }

  case TRACE_SECTION_INPUT_DEFINE:
    {
      TraceVarFrame_ptr vframe;
      TraceDefineFrame_ptr dframe;
      node_ptr* res;

      vframe = TRACE_VAR_FRAME(iter);
      CHECK (TRACE_VAR_FRAME(NULL) != vframe);

      dframe  = trace_var_frame_get_bwd_define_frame(vframe);
      if (TRACE_DEFINE_FRAME(NULL) == dframe) {
        return (node_ptr*)(NULL);
      }
      res = trace_define_frame_get_input_base(dframe);
      if ((node_ptr*)(NULL) == res && create) {
        res =  trace_setup_section_storage(trace_get_n_input_defines(trace));
        trace_define_frame_set_input_base(dframe, res);
      }

      return res;
    }

  case TRACE_SECTION_STATE_INPUT_DEFINE:
    {
      TraceVarFrame_ptr vframe;
      TraceDefineFrame_ptr dframe;
      node_ptr* res;

      vframe = TRACE_VAR_FRAME(iter);
      CHECK (TRACE_VAR_FRAME(NULL) != vframe);

      dframe  = trace_var_frame_get_bwd_define_frame(vframe);
      if (TRACE_DEFINE_FRAME(NULL) == dframe) {
        return (node_ptr*)(NULL);
      }

      res = trace_define_frame_get_state_input_base(dframe);
      if ((node_ptr*)(NULL) == res && create) {
        res =  \
          trace_setup_section_storage(trace_get_n_state_input_defines(trace));
        trace_define_frame_set_state_input_base(dframe, res);
      }

      return res;
    }

  case TRACE_SECTION_NEXT_DEFINE:
    {
      TraceVarFrame_ptr vframe;
      TraceDefineFrame_ptr dframe;
      node_ptr* res;

      vframe = TRACE_VAR_FRAME(iter);
      CHECK (TRACE_VAR_FRAME(NULL) != vframe);

      dframe  = trace_var_frame_get_bwd_define_frame(vframe);
      if (TRACE_DEFINE_FRAME(NULL) == dframe) {
        return (node_ptr*)(NULL);
      }

      res = trace_define_frame_get_next_base(dframe);
      if ((node_ptr*)(NULL) == res && create) {
        res =  trace_setup_section_storage(trace_get_n_next_defines(trace));
        trace_define_frame_set_next_base(dframe, res);
      }

      return res;
    }

  case TRACE_SECTION_STATE_NEXT_DEFINE:
    {
      TraceVarFrame_ptr vframe;
      TraceDefineFrame_ptr dframe;
      node_ptr* res;

      vframe = TRACE_VAR_FRAME(iter);
      CHECK (TRACE_VAR_FRAME(NULL) != vframe);

      dframe  = trace_var_frame_get_bwd_define_frame(vframe);
      if (TRACE_DEFINE_FRAME(NULL) == dframe) {
        return (node_ptr*)(NULL);
      }

      res = trace_define_frame_get_state_next_base(dframe);
      if ((node_ptr*)(NULL) == res && create) {
        res =  \
          trace_setup_section_storage(trace_get_n_state_next_defines(trace));
        trace_define_frame_set_state_next_base(dframe, res);
      }

      return res;
    }

  case TRACE_SECTION_INPUT_NEXT_DEFINE:
    {
      TraceVarFrame_ptr vframe;
      TraceDefineFrame_ptr dframe;
      node_ptr* res;

      vframe = TRACE_VAR_FRAME(iter);
      CHECK (TRACE_VAR_FRAME(NULL) != vframe);

      dframe  = trace_var_frame_get_bwd_define_frame(vframe);
      if (TRACE_DEFINE_FRAME(NULL) == dframe) {
        return (node_ptr*)(NULL);
      }

      res = trace_define_frame_get_input_next_base(dframe);
      if ((node_ptr*)(NULL) == res && create) {
        res =  \
          trace_setup_section_storage(trace_get_n_input_next_defines(trace));
        trace_define_frame_set_input_next_base(dframe, res);
      }

      return res;
    }

  case TRACE_SECTION_STATE_INPUT_NEXT_DEFINE:
    {
      TraceVarFrame_ptr vframe;
      TraceDefineFrame_ptr dframe;
      node_ptr* res;

      vframe = TRACE_VAR_FRAME(iter);
      CHECK (TRACE_VAR_FRAME(NULL) != vframe);

      dframe  = trace_var_frame_get_bwd_define_frame(vframe);
      if (TRACE_DEFINE_FRAME(NULL) == dframe) {
        return (node_ptr*)(NULL);
      }

      res = trace_define_frame_get_state_input_next_base(dframe);
      if ((node_ptr*)(NULL) == res && create) {
        res =  \
          trace_setup_section_storage(
                                    trace_get_n_state_input_next_defines(trace));
        trace_define_frame_set_state_input_next_base(dframe, res);
      }

      return res;
    }

  default: internal_error("%s:%d:%s: invalid section (%d)",
                          __FILE__, __LINE__, __func__, section);
  } /* of switch */

  /* unreachable */
  error_unreachable_code();
  return (node_ptr*) (NULL);
} /* trace_iter_get_section_base */

static inline void
trace_setup_lookup_cache(Trace_ptr self)
{
  memset(self->n_buckets, 0, TRACE_SECTION_END * sizeof(unsigned));
  memset(self->buckets, 0, TRACE_SECTION_END * sizeof(node_ptr*));

  self->symb2section = new_assoc();
  self->symb2address = new_assoc();
  self->symb2layername = new_assoc();
}

static inline void
trace_dispose_lookup_cache(Trace_ptr self)
{
  CHECK((hash_ptr)(NULL) != self->symb2section);
  free_assoc(self->symb2section);

  CHECK((hash_ptr)(NULL) != self->symb2address);
  free_assoc(self->symb2address);

  CHECK((hash_ptr)(NULL) != self->symb2layername);
  free_assoc(self->symb2layername);

  CHECK(NODE_LIST(NULL) != self->symbols);
  NodeList_destroy(self->symbols);

  CHECK(NODE_LIST(NULL) != self->s_vars);
  NodeList_destroy(self->s_vars);

  CHECK(NODE_LIST(NULL) != self->sf_vars);
  NodeList_destroy(self->sf_vars);

  CHECK(NODE_LIST(NULL) != self->i_vars);
  NodeList_destroy(self->i_vars);
}

static inline void
trace_setup_lookup_section (Trace_ptr self, TraceSection section)
{
  unsigned sect_size = trace_get_n_section_symbols(self, section);
  if (0 < sect_size) {
    self->buckets[section] = trace_setup_section_storage(sect_size);
  }
}

/* core lookup functions FORWARD: symb -> (section, offset) */
boolean
trace_symbol_fwd_lookup(Trace_ptr self, node_ptr symb,
                        TraceSection* section, unsigned* offset)
{
  TraceSection sect = NODE_TO_INT (find_assoc(self->symb2section, symb));
  if (sect == TRACE_SECTION_INVALID) {
    CHECK(!trace_symbol_in_language(self, symb));
    return false;
  }

  *section = sect;
  *offset = NODE_TO_INT(find_assoc(self->symb2address, symb));

  CHECK(trace_symbol_in_language(self, symb));
  return true; /* hit */
}

/* core lookup functions BACKWARD: (section, offset) -> symb */
node_ptr
trace_symbol_bwd_lookup(Trace_ptr self, TraceSection section,
                        unsigned offset)
{
  node_ptr *addr;

  nusmv_assert( offset <= trace_get_n_section_symbols(self, section) );
  addr = (trace_get_section_symbols(self, section) + offset);
  CHECK(NodeList_belongs_to(trace_get_symbols(self), *addr));

  return *addr;
}

/* private conversion functions */
SymbCategory
trace_section_to_category (const TraceSection section)
{
  switch (section) {
  case TRACE_SECTION_FROZEN_VAR: return SYMBOL_FROZEN_VAR;
  case TRACE_SECTION_STATE_VAR: return SYMBOL_STATE_VAR;
  case TRACE_SECTION_INPUT_VAR: return SYMBOL_INPUT_VAR;
  case TRACE_SECTION_STATE_DEFINE: return SYMBOL_STATE_DEFINE;
  case TRACE_SECTION_INPUT_DEFINE: return SYMBOL_INPUT_DEFINE;
  case TRACE_SECTION_STATE_INPUT_DEFINE: return SYMBOL_STATE_INPUT_DEFINE;
  case TRACE_SECTION_NEXT_DEFINE: return SYMBOL_NEXT_DEFINE;
  case TRACE_SECTION_STATE_NEXT_DEFINE: return SYMBOL_NEXT_DEFINE;
  case TRACE_SECTION_INPUT_NEXT_DEFINE: return SYMBOL_INPUT_NEXT_DEFINE;
  case TRACE_SECTION_STATE_INPUT_NEXT_DEFINE: return \
      SYMBOL_STATE_INPUT_NEXT_DEFINE;
  default: error_unreachable_code(); /* unreachable */
  }

  /* unreachable */
  error_unreachable_code();
  return SYMBOL_INVALID;
}

TraceSection
trace_category_to_section (const SymbCategory category)
{
  switch (category) {
  case SYMBOL_FROZEN_VAR:return TRACE_SECTION_FROZEN_VAR;
  case SYMBOL_STATE_VAR: return TRACE_SECTION_STATE_VAR;
  case SYMBOL_INPUT_VAR: return TRACE_SECTION_INPUT_VAR;
  case SYMBOL_CONSTANT:
  case SYMBOL_STATE_DEFINE: return TRACE_SECTION_STATE_DEFINE ;
  case SYMBOL_INPUT_DEFINE : return TRACE_SECTION_INPUT_DEFINE;
  case SYMBOL_STATE_INPUT_DEFINE: return TRACE_SECTION_STATE_INPUT_DEFINE;
  case SYMBOL_NEXT_DEFINE : return TRACE_SECTION_NEXT_DEFINE;
  case SYMBOL_STATE_NEXT_DEFINE : return TRACE_SECTION_STATE_NEXT_DEFINE;
  case SYMBOL_INPUT_NEXT_DEFINE : return TRACE_SECTION_INPUT_NEXT_DEFINE;
  case SYMBOL_STATE_INPUT_NEXT_DEFINE :return \
      TRACE_SECTION_STATE_INPUT_NEXT_DEFINE;
  default: error_unreachable_code();
  }

  /* unreachable */
  error_unreachable_code();
  return  TRACE_SECTION_INVALID;
}

const char* trace_symb_category_to_string(const SymbCategory category)
{
  switch (category) {
  case SYMBOL_CONSTANT: return "constant";
  case SYMBOL_FROZEN_VAR: return "frozen variable";
  case SYMBOL_STATE_VAR: return "state variable";
  case SYMBOL_INPUT_VAR: return "input variable";
  case SYMBOL_STATE_DEFINE: return "state define";
  case SYMBOL_INPUT_DEFINE : return "input define";
  case SYMBOL_STATE_INPUT_DEFINE: return "state-input define";
  case SYMBOL_NEXT_DEFINE : return "next define";
  case SYMBOL_STATE_NEXT_DEFINE : return "state-next define";
  case SYMBOL_INPUT_NEXT_DEFINE : return "input-next define";
  case SYMBOL_STATE_INPUT_NEXT_DEFINE :return \
      "state-input-next define";
  default: error_unreachable_code();
  }

  /* unreachable */
  error_unreachable_code();
  return (const char*)(NULL);
}

/* internal getters/setters */

/* generic symbols */
static inline unsigned
trace_get_n_section_symbols(const Trace_ptr self, TraceSection section)
{
  nusmv_assert(TRACE_SECTION_INVALID < section && section <= TRACE_SECTION_END);
  return self->n_buckets[section];
}

static inline void
trace_inc_n_section_symbols(Trace_ptr self, TraceSection section)
{
  nusmv_assert(TRACE_SECTION_INVALID < section && section <= TRACE_SECTION_END);
  ++ ( self->n_buckets[section] );
}

static inline node_ptr*
trace_get_section_symbols(Trace_ptr self, TraceSection section)
{
  nusmv_assert(TRACE_SECTION_INVALID < section && section <= TRACE_SECTION_END);
  return self->buckets[section];
}

static inline unsigned
trace_get_n_frozen_vars (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_FROZEN_VAR];
}

static inline unsigned
trace_get_n_state_vars (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_STATE_VAR];
}

static inline unsigned
trace_get_n_input_vars (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_INPUT_VAR];
}

static inline unsigned
trace_get_n_state_defines (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_STATE_DEFINE];
}

static inline unsigned
trace_get_n_input_defines (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_INPUT_DEFINE];
}

static inline unsigned
trace_get_n_state_input_defines (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_STATE_INPUT_DEFINE];
}

static inline unsigned
 trace_get_n_next_defines (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_NEXT_DEFINE];
}

static inline unsigned
trace_get_n_state_next_defines (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_STATE_NEXT_DEFINE];
}

static inline unsigned
trace_get_n_input_next_defines (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_INPUT_NEXT_DEFINE];
}

static inline unsigned
trace_get_n_state_input_next_defines (const Trace_ptr self)
{
  return self->n_buckets[TRACE_SECTION_STATE_INPUT_NEXT_DEFINE];
}

/* frozen frame base getter/setter */
static inline node_ptr*
trace_frozen_frame_get_base (const TraceFrozenFrame_ptr frame)
{
  TRACE_FROZEN_FRAME_CHECK_INSTANCE(frame);
  return frame->frozen_values;
}

/* var frame base getters/setters */
static inline node_ptr*
trace_var_frame_get_state_base (const TraceVarFrame_ptr frame)
{
  TRACE_VAR_FRAME_CHECK_INSTANCE(frame);
  return frame->state_values;
}

static inline void
trace_var_frame_set_state_base (const TraceVarFrame_ptr frame,
                                node_ptr* base)
{
  TRACE_VAR_FRAME_CHECK_INSTANCE(frame);
  frame->state_values = base;
}

static inline node_ptr*
trace_var_frame_get_input_base (const TraceVarFrame_ptr frame)
{
  TRACE_VAR_FRAME_CHECK_INSTANCE(frame);
  return frame->input_values;
}

static inline void
trace_var_frame_set_input_base (const TraceVarFrame_ptr frame,
                                node_ptr* base)
{
  TRACE_VAR_FRAME_CHECK_INSTANCE(frame);
  frame->input_values = base;
}

/* defines frame base getters/setters */
static inline node_ptr*
trace_define_frame_get_state_base (const TraceDefineFrame_ptr frame)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  return frame->s_values;
}

static inline void
trace_define_frame_set_state_base (TraceDefineFrame_ptr frame,
                                   node_ptr* base)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  frame->s_values = base;
}

static inline node_ptr*
trace_define_frame_get_input_base (const TraceDefineFrame_ptr frame)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  return frame->i_values;
}

static inline void
trace_define_frame_set_input_base (TraceDefineFrame_ptr frame,
                                   node_ptr* base)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  frame->i_values = base;
}

static inline node_ptr*
trace_define_frame_get_state_input_base (const TraceDefineFrame_ptr frame)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  return frame->si_values;
}

static inline void
trace_define_frame_set_state_input_base (TraceDefineFrame_ptr frame,
                                         node_ptr* base)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  frame->si_values = base;
}

static inline node_ptr*
trace_define_frame_get_next_base (const TraceDefineFrame_ptr frame)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  return frame->n_values;
}

static inline void
trace_define_frame_set_next_base (TraceDefineFrame_ptr frame,
                                  node_ptr* base)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  frame->n_values = base;
}

static inline node_ptr*
trace_define_frame_get_state_next_base (const TraceDefineFrame_ptr frame)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  return frame->sn_values;
}

static inline void
trace_define_frame_set_state_next_base (TraceDefineFrame_ptr frame,
                                        node_ptr* base)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  frame->sn_values = base;
}

static inline node_ptr*
trace_define_frame_get_input_next_base (const TraceDefineFrame_ptr frame)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  return frame->in_values;
}

static inline void
trace_define_frame_set_input_next_base (TraceDefineFrame_ptr frame,
                                        node_ptr* base)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  frame->in_values = base;
}

static inline node_ptr*
trace_define_frame_get_state_input_next_base (const TraceDefineFrame_ptr frame)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  return frame->sin_values;
}

static inline void
trace_define_frame_set_state_input_next_base (const TraceDefineFrame_ptr frame,
                                              node_ptr* base)
{
  TRACE_DEFINE_FRAME_CHECK_INSTANCE(frame);
  frame->sin_values = base;
}

boolean
trace_is_complete_vars (const Trace_ptr self, const NodeList_ptr vars,
                        FILE* report_stream)
{
  const TraceIter initial = trace_first_iter(self);
  TraceSection section;
  TraceIter step;
  int step_count = 0;
  unsigned dummy;
  ListIter_ptr list_iter;
  boolean res = true;

  TRACE_FOREACH(self, step) {
    ++ step_count; /* time starts at 1 */

    NODE_LIST_FOREACH(vars, list_iter) {
      node_ptr var = NodeList_get_elem_at(vars, list_iter);

      if (trace_symbol_fwd_lookup(self, var, &section, &dummy)) {

        nusmv_assert(TRACE_SECTION_FROZEN_VAR == section ||
                     TRACE_SECTION_STATE_VAR  == section ||
                     TRACE_SECTION_INPUT_VAR  == section);

        /* skip inputs for initial state */
        if ((initial == step) &&  (TRACE_SECTION_INPUT_VAR == section)) continue;
        if (!trace_symbol_is_assigned(self, step, var)) {
          res = false;
          if (NIL(FILE) != report_stream) {
            fprintf(report_stream,
                    "Trace is missing a value for variable at step %d : ",
                    step_count); print_node(report_stream, var);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      else { /* found a variable not in trace language, trace is incomplete */
        res = false;
        if (NIL(FILE) != report_stream) {
          fprintf(report_stream,
                  "Variable does not belong to trace language : ");
          print_node(report_stream, var); fprintf(nusmv_stderr, "\n");
        }
      }

      if (!res) { break; } /* halt at first error */
    } /* foreach var */
  } /* foreach step */

  return res;
}
