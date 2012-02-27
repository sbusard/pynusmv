 /**CFile***********************************************************************

  FileName    [Trace.c]

  PackageName [trace]

  Synopsis    [This module contains the public part of the implementation
               of the Trace class.]

  Description [A trace is logically defined as follows:

  T := S (i S) *

  That is, a trace consists of an initial state, optionally followed
  by an arbitrary number of <input, next state> pairs. The internal
  representation has been designed to match as closely as possible the
  logical definition given above.

  A trace is internally represented as a doubly linked list of custom
  containers called \"frames\".

  The Trace class provides iterators and methods to populate and query
  its frames. Just for the sake of clarity, A graphical representation
  of the internal representation follows:

 +----+----+-+   +-+----+----+-+   +-+----+----+-+
 |    |    |*|<->|*|    |    |*|<->|*|    |    |0| (terminator)
 | ## | S1 | |   | | i2 | S2 | |   | | i3 | S3 | |
 |    |    |*|   |*|    |    |*|   |*|    |    |*|
 +----+----+++   +++----+----+++   +++----+----+++
            |     |           |     |           |
            v     v           v     v           v
 +---+     +-------+         +-------+          +-------+
 | F |     |  D12  |         |  D23  |          |  D34  |
 +-+-+     +-------+         +-------+          +-------+

Remarks:

 * The list is doubly-linked. It can be traversed in both directions
   using iterators.

 * Frozen values (F) are stored in a separate frame. All the
   steps keep a pointer to the data without reallocating it.

 * Initial step does not contain any input.

 * Defines over (Si, i+1, Si+1) are stored in an auxiliary frame, which
   is accessible from both steps (i-1, Si), (i, Si+1).

   Defines are functions over a certain set of variables (dependency
   set). They can be catoegorized according to the kins of variables
   in their dependency set:

   Thus 7 distinct categories of defines can be distinguished as
   described in the following table:

  +-------------+--------------+---------------+------------------------------+
  | curr. state |     input    |  next state   |    Observable category       |
  +-------------+--------------+---------------+------------------------------+
  |     N       |      N       |      N        |          CONSTANT            |
  |     Y       |      N       |      N        |           STATE              |
  |     N       |      Y       |      N        |           INPUT              |
  |     Y       |      Y       |      N        |        STATE-INPUT           |
  |     N       |      N       |      Y        |           NEXT               |
  |     Y       |      N       |      Y        |        STATE-NEXT            |
  |     N       |      Y       |      Y        |        INPUT_NEXT            |
  |     Y       |      Y       |      Y        |     STATE-INPUT-NEXT         |
  +-------------+--------------+---------------+------------------------------+

  The 2 classes of variables (STATE, INPUT) and the 8 categories of
  defines described above account for 9 distinct sections of symbols
  for each step. FROZENVARs are considered as STATE variables and are
  always prepended in iterators. CONSTANTs are considered as STATE defines]

  SeeAlso     [Trace_private.c]

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

#include "pkg_trace.h"
#include "pkg_traceInt.h"
#include "Trace_private.h"

static char rcsid[] UTIL_UNUSED = "$Id: $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
static const char* TRACE_TYPE_UNSPECIFIED_STRING  = "Unspecified";
static const char* TRACE_TYPE_CNTEXAMPLE_STRING   = "Counterexample";
static const char* TRACE_TYPE_SIMULATION_STRING   = "Simulation";
static const char* TRACE_TYPE_EXECUTION_STRING    = "Execution";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Trace class constructor]

  Description [Allocates and initializes a trace.  In NuSMV, a trace
               is an engine-independent description of a computation
               path for some FSM.  The newly created trace is
               associated with a language, that is a set of symbols
               (variables and defines) that can occur in the trace. In
               addition, a description and a type can be given to a
               trace.

               If the trace is not volatile, all input parameters are
               internally duplicated in an independent copy.  The
               caller is responsible for freeing them. Same for
               volatile traces, made exception for the symbol table,
               since only a reference will be retained. In this case,
               the caller is responsible for freeing them, and take
               care of the symbol table, which must NOT be freed until
               the created trace instance is destroyed.

               Remarks:

               * First step is already allocated for the returned
                 trace.  Use Trace_first_iter to obtain a valid
                 iterator pointing to the initial step. Use
                 Trace_add_step to append further steps.]

  SideEffects []

  SeeAlso     [Trace_first_iter, Trace_append_step, Trace_destroy]

******************************************************************************/
Trace_ptr Trace_create (const SymbTable_ptr st, const char* desc,
                        const TraceType type, const NodeList_ptr symbols,
                        boolean is_volatile)
{
  return trace_create(st, desc, type, symbols, is_volatile);
}


/**Function********************************************************************

  Synopsis    [Trace class copy constructor]

  Description [Returns an independent copy of \"self\" trace. If a
               non-NULL \"until_here\" iterator is passed copying
               process halts when \"until_here\" iterator has been
               reached.  To obtain a full copy, pass TRACE_END_ITER as
               the \"until_here\" paramter. Loopback information is
               propagated to the copy only if a full copy is required.

               If the trace is not volatile, all trace structures are
               internally duplicated in an independent copy. Same for
               volatile traces, made exception for the symbol table,
               since only a reference of the original trace's symbol
               table will be retained. In this case, the caller must
               take care of the original trace symbol table, which
               must NOT be freed until the created trace instance is
               destroyed. In detail:

               - If the original trace is volatile, then it's copy is
                 valid unless the symbol table given at creation time
                 is destroyed. In all other cases, the returned trace
                 is valid.

               - If the original trace is not volatile, then the
                 returned copy is valid as long as the original trace
                 is valid.

               Notice that a volatile copy "C" of a volatile copy "B"
               of a non-volatile trace "A" needs "A" to exist, and so
               on.


               Remarks:

               * The full copy of a frozen trace is a frozen
               trace. Partial copies are always thawed.]

  SideEffects []

  SeeAlso     [Trace_thaw, Trace_freeze, Trace_destroy]

******************************************************************************/
Trace_ptr
Trace_copy (const Trace_ptr self, const TraceIter until_here,
            boolean is_volatile)
{
  TRACE_CHECK_INSTANCE(self);
  return trace_copy(self, until_here, is_volatile);
}


/**Function********************************************************************

  Synopsis    [Trace concatenation]

  Description [*Destructively* concatenates \"other\" to
               \"self\". That is, \"self\" is appended all available
               data about variables and defines from
               \"*other\". Frozen vars and state vars of the
               conjunction state for both \"self\" and \"other\"
               traces are synctactically checked for
               consistency. Their values are merged in the resulting
               trace.

               Warning: an internal error is raised if an
               inconsistency is detected.

               Returned valued is \"self\".]

  SideEffects [\"self\" is extended, \"*other\" is destroyed and its
               pointer is set to NULL.]

  SeeAlso     []

******************************************************************************/
Trace_ptr
Trace_concat (Trace_ptr self, Trace_ptr* other)
{
  TRACE_CHECK_INSTANCE(self);
  return trace_concat(self, other);
}

/**Function********************************************************************

  Synopsis    [Trace class destructor]

  Description [Frees all the resources used by \"self\" trace instance]

  SideEffects []

  SeeAlso     [Trace_create, Trace_copy]

******************************************************************************/
void Trace_destroy(Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  trace_destroy(self);
}


/**Function********************************************************************

  Synopsis    [Returns a trace iterator pointing to the first step of the trace]

  Description [A step is a container for incoming input and next
               state(i.e. it has the form <i, S>)

               The returned step can be used as parameter to all
               Trace_step_xxx functions.

               Remarks:

                 * the first step holds *no* input information.]

  SideEffects []

  SeeAlso     [Trace_last_iter]

*****************************************************************************/
inline TraceIter
Trace_first_iter (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  return trace_first_iter(self);
}


/**Function********************************************************************

  Synopsis    [Returns a trace iterator pointing to the i-th step of the trace]

  Description [Returns a trace iterator pointing to the i-th step of
               the trace.  Counting starts at 1. Thus, here is the
               sequence of first k steps for a trace.

               S1 i2 S2 i3 S3 ... ik Sk

               Remarks:

                 * the first step holds *no* input information.]

  SideEffects []

  SeeAlso     [Trace_first_iter, Trace_last_iter]

*****************************************************************************/
inline TraceIter
Trace_ith_iter (const Trace_ptr self, unsigned i)
{
 TRACE_CHECK_INSTANCE(self);
 nusmv_assert(i > 0);

 return trace_ith_iter(self, i);
}


/**Function********************************************************************

  Synopsis    [Returns a trace iterator pointing to the last step of the trace]

  Description [A step is a container for incoming input and next
               state(i.e. it has the form <i, S>)

               The returned step can be used as parameter to all
               Trace_step_xxx functions]

  SideEffects []

  SeeAlso     [Trace_first_iter]

*****************************************************************************/
inline TraceIter
Trace_last_iter (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  return trace_last_iter(self);
}


/**Function********************************************************************

  Synopsis    [Returns a trace iterator pointing to the next step of the
               trace]

  Description [Returns a trace iterator pointing to the next step of
               the trace. TRACE_END_ITER is returned if no such
               iterator exists]

  SideEffects []

  SeeAlso     [TraceIter_get_prev]

*****************************************************************************/
inline TraceIter
TraceIter_get_next (const TraceIter iter)
{
  TRACE_ITER_CHECK_INSTANCE(iter);
  return trace_iter_get_next(iter);
}


/**Function********************************************************************

  Synopsis    [Returns a trace iterator pointing to the previous
               step of the trace]

  Description [Returns a trace iterator pointing to the previous step
               of the trace. TRACE_END_ITER is returned if no such
               iterator exists]

  SideEffects []

  SeeAlso     [TraceIter_get_next]

*****************************************************************************/
inline TraceIter
TraceIter_get_prev (const TraceIter iter)
{
  TRACE_ITER_CHECK_INSTANCE(iter);
  return trace_iter_get_prev(iter);
}


/**Function********************************************************************

  Synopsis    [Iterator-at-end-of-trace predicate]

  Description []

  SideEffects []

  SeeAlso     []

*****************************************************************************/
inline boolean
TraceIter_is_end(const TraceIter iter)
{
  return (TRACE_END_ITER == iter);
}


/**Function********************************************************************

  Synopsis    [Extends a trace by adding a new step to it]

  Description [A step is a container for incoming input and next
               state(i.e. it has the form <i, S>)

               The returned step can be used as parameter to all
               Trace_step_xxx functions]

  SideEffects []

  SeeAlso     [Trace_create, Trace_step_put_value, Trace_step_get_value,
               Trace_step_get_iter, Trace_step_get_next_value]

*****************************************************************************/
TraceIter
Trace_append_step(Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  return trace_append_step(self);
}


/**Function********************************************************************

  Synopsis    [Tests whether state is \"step\" is a loopback state w.r.t the
               last state in \"self\".]

  Description [This function behaves accordingly to two different modes a trace
               can be: frozen or thawed(default).

               If the trace is frozen, permanent loopback information
               is used to determine if \"step\" has a loopback state.
               No further loopback computation is made.

               If the trace is thawed, dynamic loopback calculation
               takes place, using a variant of Rabin-Karp pattern
               matching algorithm]

  SideEffects []

  SeeAlso     [Trace_create, Trace_step_put_value, Trace_step_get_value,
               Trace_step_get_iter, Trace_step_get_next_value]

*****************************************************************************/
boolean
Trace_step_is_loopback ARGS((Trace_ptr self, TraceIter step))
{
  TRACE_CHECK_INSTANCE(self);
  if (TRACE_END_ITER == step) return false;

  return trace_step_is_loopback(self, step);
}


/**Function********************************************************************

  Synopsis    [Determine whether the \"self\" trace is volatile]

  Description [A volatile trace does not own a symbol table instance,
               so it is valid as long as the symbol table does not
               change and is available. A non-volatile trace instead
               owns a copy of the symbol table given at construction
               time and is completely independand among system changes
               over time]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Trace_is_volatile (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  return trace_is_volatile(self);
}


/**Function********************************************************************

  Synopsis    [Determine whether the \"self\" trace is frozen]

  Description [A frozen trace holds explicit information about
              loopbacks and can not be appended a step, or added a
              variable value.

              Warning: after freezing no automatic looback calculation
              will be performed: it is up to the owner of the trace to
              manually add loopback information using
              Trace_step_force_loopback.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
boolean Trace_is_frozen (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  return trace_is_frozen(self);
}

/**Function********************************************************************

  Synopsis    [Determine whether the \"self\" trace is thawed]

  Description [A thawed trace holds no explicit information about
              loopbacks and can be appended a step and

              Warning: after thawing the trace will not persistently
              retain any loopback information. In particular it is
              *illegal* to force a loopback on a thawed trace.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
boolean
Trace_is_thawed (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  return trace_is_thawed(self);
}

/**Function********************************************************************

  Synopsis    [Freezes a trace]

  Description [A frozen trace holds explicit information about
              loopbacks. Its length and assignments are immutable,
              that is it cannot be appended more steps, nor can it
              accept more values that those already stored in it.

              Still it is possible to register/unregister the trace
              and to change its type or description.]

              Warning: After freezing no automatic looback calculation
              will be performed: it is up to the owner of the trace to
              manually add loopback information using
              Trace_step_force_loopback.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void Trace_freeze (Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  trace_freeze(self);
}

/**Function********************************************************************

  Synopsis    [Thaws a trace]

  Description [A thawed traces holds no explicit information about
              loopbacks and can be appended a step and added values in
              the customary trace building process.

              Warning: after thawing the trace will not persistently
              retain any loopback information. In particular it is
              *illegal* to force a loopback on a thawed trace.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void Trace_thaw (Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);
  trace_thaw(self);
}


/**Function********************************************************************

  Synopsis    [Equality predicate between traces]

  Description [Two traces are equals iff:

              1. They're the same object or NULL.

              or

              2. They have exactly the same language, length,
                 assignments for all variables in all times and
                 the same loopbacks.

                 (Defines are not taken into account for equality.)

              They need not be both frozen of thawed, neither being
              both registered or unregistered. (Of course two traces
              *cannot* have the same ID).]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
boolean Trace_equals(const Trace_ptr self, const Trace_ptr other)
{
  return trace_equals(self, other);
}


/**Function********************************************************************

  Synopsis    [Forces a loopback on a frozen trace]

  Description [Use this function to store explicit loopback information
               in a frozen trace. The trace will retain loopback data
               until being thawed again.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void Trace_step_force_loopback (const Trace_ptr self, TraceIter step)
{
  TRACE_CHECK_INSTANCE(self);
  TRACE_ITER_CHECK_INSTANCE(step);

  trace_step_force_loopback(self, step);
}


/**Function********************************************************************

  Synopsis    [Tests whether a symbol is \"self\"'s language]

  Description [Returns true iff symb is part of the language defined
               for \"self\" defined at creation time.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
boolean Trace_symbol_in_language (Trace_ptr self, node_ptr symb)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_symbol_in_language(self, symb);
}


/**Function********************************************************************

  Synopsis    [Exposes Trace internal symbol table]

  Description [Returns the trace symbol table. The symbol table is
               owned by the trace and should *not* be modified in any
               way.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
SymbTable_ptr Trace_get_symb_table(Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_get_symb_table(self);
}


/**Function********************************************************************

  Synopsis    [Exposes the list of symbols in trace language]

  Description [Returned list belongs to \"self\". Do not change or dispose it.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
NodeList_ptr Trace_get_symbols (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_get_symbols(self);
}


/**Function********************************************************************

  Synopsis    [Exposes the list of state vars in trace language]

  Description [Returned list belongs to \"self\". Do not change or dispose it.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
NodeList_ptr Trace_get_s_vars (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_get_s_vars(self);
}


/**Function********************************************************************

  Synopsis    [Exposes the list of state-frozen vars in trace language]

  Description [Returned list belongs to \"self\". Do not change or dispose it.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
NodeList_ptr Trace_get_sf_vars (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_get_sf_vars(self);
}


/**Function********************************************************************

  Synopsis    [Exposes the list of input vars in trace language]

  Description [Returned list belongs to \"self\". Do not change or dispose it.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
NodeList_ptr Trace_get_i_vars (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_get_i_vars(self);
}

/**Function********************************************************************

  Synopsis     [Checks if a Trace is complete on the given set of vars]

  Description  [Checks if a Trace is complete on the given set of vars]

                A Trace is complete iff in every node, all vars are
                given a value

                Remarks:

                * Only input and state section are taken into account.
                Input vars are not taken into account in the first
                step. Defines are not taken into account at all.

                * If result is false and parameter 'report' is true
                then a message will be output in nusmv_stderr with
                some explanation of why the trace is not complete]

  SideEffects   [None]

  SeeAlso       []

******************************************************************************/
boolean Trace_is_complete (Trace_ptr self, NodeList_ptr vars, boolean report)
{
  TRACE_CHECK_INSTANCE(self);

  FILE* err = (report) ? nusmv_stderr : NIL(FILE);
  return trace_is_complete_vars(self, vars, err);
}


/**Function********************************************************************

  Synopsis    [Returns a string corresponding to a TraceType.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
const char* TraceType_to_string(const TraceType trace_type)
{
  switch (trace_type){
  case TRACE_TYPE_UNSPECIFIED : return TRACE_TYPE_UNSPECIFIED_STRING;
  case TRACE_TYPE_CNTEXAMPLE: return TRACE_TYPE_CNTEXAMPLE_STRING;
  case TRACE_TYPE_SIMULATION: return TRACE_TYPE_SIMULATION_STRING;
  case TRACE_TYPE_EXECUTION: return TRACE_TYPE_EXECUTION_STRING;
  default: internal_error("%s:%d:%s: unexpected trace type. (%d)",
                          __FILE__, __LINE__, __func__, trace_type);
  }

  error_unreachable_code(); /* unreachable */
  return (const char*)(NULL);
}


/**Function********************************************************************

  Synopsis    [Stores an assignment into a trace step]

  Description [A step is a container for incoming input and next
               state(i.e. it has the form <i, S>)

               \"step\" must be a valid step for the trace.  If symb
               belongs to the language associated to the trace at
               creation time, the normalized value of \"value\" is
               stored into the step. Assignment is checked for type
               correctness.

               Returns true iff the value was succesfully assigned to symb
               in given step of self.

               Remarks:

               * Assignments to symbols not in trace language are
               silently ignored.]

  SideEffects []

  SeeAlso     [Trace_append_step, Trace_step_get_value]

*****************************************************************************/
EXTERN boolean Trace_step_put_value (Trace_ptr self, TraceIter step,
                                     node_ptr symb, node_ptr value)
{
  TRACE_CHECK_INSTANCE(self);
  if (TRACE_END_ITER == step) {
    internal_error("%s:%d:%s: invalid iterator.",
                   __FILE__, __LINE__, __func__);
  }

  return trace_step_put_value(self, step, symb, value);
}


/**Function********************************************************************

  Synopsis    [Retrieves an assignment from a trace step]

  Description [A step is a container for incoming input and next
               state(i.e. it has the form <i, S>)

               \"step\" must be a valid step for the trace.  \"symb\"
               must belong to the language associated to the trace at
               creation time. The value stored into the step is
               returned or Nil if no such value exists.

               Remarks: An internal error is raised if \"symb\" is not
               in trace lanaguage.]

  SideEffects []

  SeeAlso     [Trace_create, Trace_step_put_value, Trace_step_get_value,
               Trace_step_get_iter, Trace_step_get_next_value]

*****************************************************************************/
EXTERN node_ptr Trace_step_get_value (Trace_ptr self, TraceIter step,
                                      node_ptr symb)
{
 TRACE_CHECK_INSTANCE(self);
  if (TRACE_END_ITER == step) {
    internal_error("%s:%d:%s: invalid iterator.",
                   __FILE__, __LINE__, __func__);
  }

  return trace_step_get_value(self, step, symb);
}


/**Function********************************************************************

  Synopsis    [Step iterator factory constructor]

  Description [A step is a container for incoming input and next
               state(i.e. it has the form <i, S>)

               \"step\" must be a valid step for the trace. An
               iterator over the assignments in \"step\" is returned.
               This iterator can be used with Trace_step_iter_fetch.

               Hint: do not use this function. Use TRACE_STEP_FOREACH
               macro instead (it is way easier and more readable).]

  SideEffects []

  SeeAlso     [TRACE_STEP_FOREACH]

*****************************************************************************/
EXTERN TraceStepIter Trace_step_iter (const Trace_ptr self, const TraceIter step,
                                      const TraceIteratorType iter_type)
{
  TRACE_CHECK_INSTANCE(self);
  if (TRACE_END_ITER == step) {
    internal_error("%s:%d:%s: invalid iterator.",
                   __FILE__, __LINE__, __func__);
  }

  return trace_step_iter(self, step, iter_type);
}


/**Function********************************************************************

  Synopsis    [Symbols iterator factory constructor]

  Description [An iterator over the symbols in \"self\" is returned.
               This iterator can be used with Trace_symbols_iter_fetch.

               Hint: do not use this function. Use TRACE_SYMBOLS_FOREACH
               macro instead (it is way easier and more readable).]

  SideEffects []

  SeeAlso     [TRACE_SYMBOLS_FOREACH]

*****************************************************************************/
EXTERN TraceSymbolsIter Trace_symbols_iter (const Trace_ptr self,
                                            TraceIteratorType iter_type)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_symbols_iter(self, iter_type);
}


/**Function********************************************************************

  Synopsis    [Step iterator next function]

  Description [A step iterator is a stateful iterator which yields
               an single assignment at each call of this function.

               \"step_iter\" must be a valid step iterator for the
               trace. If a valid assignment was found, True is
               returned.  Otherwise False is returned. This indicates
               end of iteration.

               Hint: do not use this function. Use TRACE_SYMBOLS_FOREACH
               macro instead (it is way easier and more readable).]

  SideEffects []

  SeeAlso     [Trace_step_get_iter]

*****************************************************************************/
EXTERN boolean Trace_step_iter_fetch(TraceStepIter* step_iter,
                                     node_ptr* symb, node_ptr* value)
{
  if (NULL == step_iter) return false;
  return trace_step_iter_fetch(step_iter, symb, value);
}



/**Function********************************************************************

  Synopsis    [Symbols iterator next function]

  Description [A symbols iterator is a stateful iterator which yields
               a symbols in the trace language at each call of this function.

               \"symbols_iter\" must be a valid symbols iterator for
               the trace. If a symbols is found, True is returned.
               Otherwise False is returned. This indicates end of
               iteration.

               Hint: do not use this function. Use TRACE_SYMBOLS_FOREACH
               macro instead (it is way easier and more readable).]

  SideEffects []

  SeeAlso     [Trace_symbols_get_iter]

*****************************************************************************/
extern boolean Trace_symbols_iter_fetch(TraceSymbolsIter* symbols_iter,
                                        node_ptr* symb)
{
  return trace_symbols_iter_fetch(symbols_iter, symb);
}


/**Function********************************************************************

  Synopsis    [Gets the id of given trace.]

  Description [Returns the ID of given trace. A valid id is a
               non-negative number.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Trace_get_id(Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_get_id(self);
}


/**Function********************************************************************

  Synopsis    [Sets the id of given trace.]

  Description [Sets the ID of the given trace. A valid ID is a
               non-negative number.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_register(Trace_ptr self, int id)
{
  TRACE_CHECK_INSTANCE(self);

  trace_register(self, id);
}


/**Function********************************************************************

  Synopsis    [Unregisters a trace]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_unregister(Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  trace_unregister(self);
}


/**Function********************************************************************

  Synopsis    [Checks whether trace is registered with the trace manager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Trace_is_registered(const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_is_registered(self);
}


/**Function********************************************************************

  Synopsis    [Gets the description of given trace.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
const char* Trace_get_desc (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_get_desc(self);
}


/**Function********************************************************************

  Synopsis    [Sets the description of given trace.]

  Description [The string in \"desc\" is duplicated inside the
               trace. The caller can dispose the actual parameter.

               Remarks: NIL(char) is accepted as a non-descriptive
               description.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Trace_set_desc (Trace_ptr self, const char* desc)
{
  TRACE_CHECK_INSTANCE(self);

  trace_set_desc(self, desc);
}


/**Function********************************************************************

  Synopsis    [Gets the type of the trace.]

  Description [For a list of see definition of TraceType enum]

  SideEffects []

  SeeAlso     [TraceType]

******************************************************************************/
TraceType Trace_get_type (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_get_type(self);
}


/**Function********************************************************************

  Synopsis    [Sets the type of the trace.]

  Description [For a list of see definition of TraceType enum]

  SideEffects []

  SeeAlso     [TraceType]

******************************************************************************/
void Trace_set_type(const Trace_ptr self, TraceType trace_type)
{
  TRACE_CHECK_INSTANCE(self);

  trace_set_type(self, trace_type);
}


/**Function********************************************************************

  Synopsis    [Gets the length of the trace.]

  Description [Length for a trace is defined as the number of the
               transitions in it. Thus, a trace consisting only of an
               initial state is a 0-length trace. A trace with two
               states is a 1-length trace and so forth.]

  SideEffects []

  SeeAlso     [TraceType]

******************************************************************************/
unsigned Trace_get_length (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_get_length(self);
}


/**Function********************************************************************

  Synopsis    [Checks whether the trace is empty or not]

  Description [A trace is empty if the length is 0 and there are no
               assignments in the initial states]

  SideEffects []

  SeeAlso     [TraceType]

******************************************************************************/
boolean Trace_is_empty (const Trace_ptr self)
{
  TRACE_CHECK_INSTANCE(self);

  return trace_is_empty(self);
}
