/**CHeaderFile*****************************************************************

  FileName    [Trace.h]

  PackageName [trace]

  Synopsis    [The header file for the Trace class]

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

  You should have received a copy of the GNU Lesser General Publi
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

  Revision    [$Id: $]

******************************************************************************/
#ifndef _TRACE_DEFINED
#define _TRACE_DEFINED

#include "set/set.h"
#include "fsm/sexp/Expr.h"
#include "compile/symb_table/SymbTable.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct Trace_TAG* Trace_ptr;
typedef struct TraceVarFrame_TAG* TraceIter;


/**Macro***********************************************************************

  Synopsis     [Macro to iterate over trace (horizontal iterator) ]

  Description  [Use this macro to iterate from the first step to the
                last.]

  SideEffects  [none]

  SeeAlso      [none]

******************************************************************************/
#define TRACE_FOREACH(trace, iter)                                  \
  for ((iter)=Trace_first_iter(trace); TRACE_END_ITER != (iter);    \
       (iter)=TraceIter_get_next(iter))


/**Macro***********************************************************************

  Synopsis     [Macro to iterate over trace step (vertical iterator) ]

  Description  [Use this macro to iterate over assignments for a given
                step.]

  SideEffects  [symbol and value are assigned for each iteration]

  SeeAlso      [TRACE_SYMBOLS_FOREACH]

******************************************************************************/
#define TRACE_STEP_FOREACH(trace, step, type, iter, symbol, value)   \
  iter = Trace_step_iter((trace), (step), (type));                   \
  while (Trace_step_iter_fetch((&iter), (&symbol), (&value)))


/**Macro***********************************************************************

  Synopsis     [Macro to iterate over symbols (vertical iterator) ]

  Description  [Use this macro to iterate over symbols of a trace.]

  SideEffects  [symbol is assigned for each iteration]

  SeeAlso      [TRACE_STEP_FOREACH]

******************************************************************************/
#define TRACE_SYMBOLS_FOREACH(trace, type, iter, symbol)            \
  iter = Trace_symbols_iter((trace), (type));                       \
  while (Trace_symbols_iter_fetch((&iter), (&symbol)))


/**Enum************************************************************************

  Synopsis    [Trace type enumeration]

  Description []

  SeeAlso     []

******************************************************************************/
typedef enum TraceType_TAG {
  TRACE_TYPE_UNSPECIFIED = -1, /* reserved */

  TRACE_TYPE_CNTEXAMPLE = 0,
  TRACE_TYPE_SIMULATION,
  TRACE_TYPE_EXECUTION,

  TRACE_TYPE_END,
} TraceType;


/**Enum************************************************************************

  Synopsis    [Trace vertical iterator kind enum]

  Description [Specific kind of iterators can be required using
               the appropriate value from this enumeration]

  SeeAlso     [TraceStepIter, Trace_step_iter]

******************************************************************************/
typedef enum TraceIteratorType_TAG {

  TRACE_ITER_NONE=0,

  /* vars */
  TRACE_ITER_F_VARS=0x2,
  TRACE_ITER_S_VARS=0x4,
  TRACE_ITER_I_VARS=0x8,

  /* var groups */
  TRACE_ITER_SF_VARS=0x6,
  TRACE_ITER_ALL_VARS=0xe,

  /* defines */
  TRACE_ITER_S_DEFINES=0x10,
  TRACE_ITER_I_DEFINES=0x20,

  TRACE_ITER_SI_DEFINES=0x40,
  TRACE_ITER_N_DEFINES=0x80,
  TRACE_ITER_SN_DEFINES=0x100,
  TRACE_ITER_IN_DEFINES=0x200,
  TRACE_ITER_SIN_DEFINES=0x400,

  /* vars+defines groups */
  TRACE_ITER_SF_SYMBOLS = 0x16,
  TRACE_ITER_S_SYMBOLS = 0x14,
  TRACE_ITER_I_SYMBOLS = 0x28,

  /* transitional groups: the following iterator types are used to
     describe defines across a transition: COMBINATORIAL holds all the
     defines which depend on (S, I), (N), (S, N), (I, N), (S, I, N).
     In addition to all the defines aforementioned, TRANSITIONAL
     contains INPUT defines as well. */
  TRACE_ITER_COMBINATORIAL=0x7c0,
  TRACE_ITER_TRANSITIONAL=0x07e0,

} TraceIteratorType;


/**Struct**********************************************************************

  Synopsis    [Trace vertical iterator type]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
typedef struct TraceStepIter_TAG
{
  Trace_ptr trace;
  TraceIter step;
  TraceIteratorType type;

  unsigned section;
  unsigned cursor;
} TraceStepIter;


/**Struct**********************************************************************

  Synopsis    [Trace vertical iterator type]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
typedef struct TraceSymbolsIter_TAG
{
  Trace_ptr trace;
  TraceIteratorType type;

  unsigned section;
  unsigned cursor;
} TraceSymbolsIter;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE(x) \
  ((Trace_ptr) x)

#define TRACE_CHECK_INSTANCE(x) \
  (nusmv_assert(TRACE(x) != TRACE(NULL)))

#define TRACE_ITER(x) \
  ((TraceIter) x)

#define TRACE_ITER_CHECK_INSTANCE(x) \
  (nusmv_assert(TRACE_ITER(x) != TRACE_ITER(NULL)))

#define TRACE_STEP_ITER(x) \
  ((TraceStepIter) x)

#define TRACE_STEP_ITER_CHECK_INSTANCE(x) \
  (nusmv_assert(TRACE_STEP_ITER(x) != TRACE_STEP_ITER(NULL)))

#define TRACE_SYMBOLS_ITER(x) \
  ((TraceStepIter) x)

#define TRACE_SYMBOLS_ITER_CHECK_INSTANCE(x) \
  (nusmv_assert(TRACE_SYMBOLS_ITER(x) != TRACE_SYMBOLS_ITER(NULL)))


/**Macro***********************************************************************

  Synopsis     [Iterator ends]

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define TRACE_END_ITER TRACE_ITER(NULL)
#define TRACE_STEP_END_ITER TRACE_STEP_ITER(NULL)
#define TRACE_SYMBOLS_END_ITER TRACE_SYMBOLS_ITER(NULL)

/* reserved for Trace Manager */
#define TRACE_UNREGISTERED -1

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/* lifecycle */
EXTERN Trace_ptr
Trace_create ARGS((SymbTable_ptr st, const char* desc,
                   const TraceType type, NodeList_ptr symbols,
                   boolean is_volatile));

EXTERN Trace_ptr
Trace_copy ARGS((const Trace_ptr self, const TraceIter until_here,
                 boolean is_volatile));

EXTERN Trace_ptr
Trace_concat ARGS((Trace_ptr self, Trace_ptr* other));

EXTERN void
Trace_destroy ARGS((Trace_ptr self));

/* metadata */
EXTERN const char*
Trace_get_desc ARGS((const Trace_ptr self));

EXTERN void
Trace_set_desc ARGS((const Trace_ptr self, const char* desc));

EXTERN boolean
Trace_is_volatile ARGS((const Trace_ptr self));

EXTERN int
Trace_get_id ARGS((const Trace_ptr self));

EXTERN boolean
Trace_is_registered ARGS((const Trace_ptr self));

EXTERN void
Trace_register ARGS((const Trace_ptr self, int id));

EXTERN void
Trace_unregister ARGS((const Trace_ptr self));

EXTERN TraceType
Trace_get_type ARGS((const Trace_ptr self));

EXTERN void
Trace_set_type ARGS((Trace_ptr self, TraceType trace_type));

EXTERN unsigned
Trace_get_length ARGS((const Trace_ptr self));

EXTERN boolean
Trace_is_empty ARGS((const Trace_ptr self));

/* freeze/thaw */
EXTERN boolean
Trace_is_frozen ARGS((const Trace_ptr self));

EXTERN boolean
Trace_is_thawed ARGS((const Trace_ptr self));

EXTERN void
Trace_freeze ARGS((Trace_ptr self));

EXTERN void
Trace_thaw ARGS((Trace_ptr self));

EXTERN boolean
Trace_equals ARGS((const Trace_ptr self, const Trace_ptr other));

/* step management */
EXTERN TraceIter
Trace_append_step ARGS((Trace_ptr self));

EXTERN boolean
Trace_step_is_loopback ARGS((const Trace_ptr self, TraceIter step));

EXTERN void
Trace_step_force_loopback ARGS((const Trace_ptr self, TraceIter step));

/* data accessors */
EXTERN boolean
Trace_step_put_value ARGS((Trace_ptr self, TraceIter step,
                            node_ptr symb, node_ptr value));

EXTERN node_ptr
Trace_step_get_value ARGS((const Trace_ptr self, TraceIter step,
                           node_ptr symb));

/* horizontal iterators, used to traverse a trace */
EXTERN TraceIter
Trace_first_iter ARGS((const Trace_ptr self));

EXTERN TraceIter
Trace_ith_iter ARGS((const Trace_ptr self, unsigned i));

EXTERN TraceIter
Trace_last_iter ARGS((const Trace_ptr self));

EXTERN TraceIter
TraceIter_get_next ARGS((const TraceIter iter));

EXTERN TraceIter
TraceIter_get_prev ARGS((const TraceIter iter));

EXTERN boolean
TraceIter_is_end ARGS((const TraceIter iter));

/* vertical iterators (hint: use macros instead) */
EXTERN TraceStepIter
Trace_step_iter ARGS((const Trace_ptr self, const TraceIter step,
                      const TraceIteratorType iter_type));

EXTERN boolean
Trace_step_iter_fetch ARGS((TraceStepIter* step_iter,
                            node_ptr* symb, node_ptr* value));

EXTERN TraceSymbolsIter
Trace_symbols_iter ARGS((const Trace_ptr self,
                         const TraceIteratorType iter_type));

EXTERN boolean
Trace_symbols_iter_fetch ARGS((TraceSymbolsIter* symbols_iter,
                               node_ptr *symb));

/* language queries */
EXTERN SymbTable_ptr
Trace_get_symb_table ARGS((Trace_ptr self));

EXTERN NodeList_ptr
Trace_get_symbols ARGS((const Trace_ptr self));

EXTERN NodeList_ptr
Trace_get_s_vars ARGS((const Trace_ptr self));

EXTERN NodeList_ptr
Trace_get_sf_vars ARGS((const Trace_ptr self));

EXTERN NodeList_ptr
Trace_get_i_vars ARGS((const Trace_ptr self));

EXTERN boolean
Trace_symbol_in_language ARGS((const Trace_ptr self, node_ptr symb));

EXTERN boolean
Trace_covers_language ARGS((const Trace_ptr self, NodeList_ptr  symbols));

EXTERN SymbCategory
Trace_symbol_get_category ARGS((const Trace_ptr self, node_ptr symb));

EXTERN boolean
Trace_symbol_is_assigned ARGS((Trace_ptr self, TraceIter step, node_ptr symb));

EXTERN boolean
Trace_is_complete ARGS((Trace_ptr self, NodeList_ptr vars, boolean report));

const char* TraceType_to_string ARGS((const TraceType self));

/**AutomaticEnd***************************************************************/

#endif /* _TRACE_DEFINED_ */
