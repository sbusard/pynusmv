/**CFile***********************************************************************

FileName    [BeEnc.c]

PackageName [enc.be]

Synopsis    [Implementaion of class 'BeEnc']

Description []

SeeAlso     [BeEnc.h]

Author      [Roberto Cavada, Alessandro Cimatti, Lorenzo Delana]

Copyright   [
This file is part of the ``enc.be'' package of NuSMV version 2.
Copyright (C) 2005 by FBK-irst.

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

Revision    [$Id: BeEnc.c,v 1.1.2.19.4.19 2010-02-19 11:41:00 nusmv Exp $]

******************************************************************************/

#include "BeEnc.h"
#include "BeEnc_private.h"
#include "enc/encInt.h"

#include "bmc/bmcConv.h" /* for cleaning up the cache */
#include "bmc/bmcInt.h" /* for cleaning up the cache */

#include "parser/symbols.h"
#include "utils/utils.h"
#include "utils/utils_io.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: BeEnc.c,v 1.1.2.19.4.19 2010-02-19 11:41:00 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

Synopsis    [shift_memoize_key is a private struct used in shift memoizing]

Description [Used in order to contain the key values in hash operation
involved in shift memoizing]

SeeAlso     []

******************************************************************************/
typedef struct be_enc_shift_memoize_key_TAG
{
  be_ptr  be;
  int     c_time;
  int     f_time;
  int     i_time;
  int     n_time;
} be_enc_shift_memoize_key;


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'BeEnc_private.h' for class 'BeEnc' definition. */

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* An invalid variable index */
#define BE_ENC_INVALID_IDX -1

/* Constants used internally for time shifting
   Warning: all values must be negative. */
#define BE_ENC_NEXT_UNTIMED_TIME       -1 /*must have the greatest value*/
#define BE_ENC_CURRENT_UNTIMED_TIME    BE_ENC_NEXT_UNTIMED_TIME-1
#define BE_ENC_NO_TIME                 BE_ENC_NEXT_UNTIMED_TIME-2
#define BE_ENC_INVALID_TIME            BE_ENC_NEXT_UNTIMED_TIME-3


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void be_enc_finalize ARGS((Object_ptr object, void* dummy));

static void be_enc_add_vars ARGS((BeEnc_ptr self,
                                  const SymbLayer_ptr layer));

static void be_enc_remove_vars ARGS((BeEnc_ptr self,
                                     const SymbLayer_ptr layer));

static void be_enc_compact_log_level ARGS((BeEnc_ptr self));

static void be_enc_instantiate_var ARGS((BeEnc_ptr self,
                                         node_ptr name,
                                         int input_vars_delta,
                                         int state_vars_delta,
                                         int frozen_vars_delta,
                                         int ofs));

static void be_enc_allocate_new_log_space ARGS((BeEnc_ptr self,
                                                int input_vars_num,
                                                int state_vars_num,
                                                int frozen_vars_num));

static void
be_enc_move_log_block ARGS((BeEnc_ptr self, int src_idx, int dest_idx,
                            size_t block_size));

static void be_enc_extend_timed_blocks ARGS((BeEnc_ptr self, int maxtime));

static void
be_enc_allocate_space_for_new_vars ARGS((BeEnc_ptr self, int vars_num));

static void
be_enc_create_be_var ARGS((BeEnc_ptr self, int logical_idx, node_ptr name));

static int be_enc_index_log_untimed_to_timed ARGS((const BeEnc_ptr self,
                                                   int log_idx, int time));

static int
be_enc_index_log_timed_to_untimed ARGS((const BeEnc_ptr self, int log_idx));

static int be_enc_index_log_to_time ARGS((const BeEnc_ptr self, int log_idx));

static int
be_enc_index_log_curr_to_next ARGS((const BeEnc_ptr self, int log_idx));

static int
be_enc_index_log_next_to_curr ARGS((const BeEnc_ptr self, int log_idx));

static int be_enc_get_next_avail_phy_index ARGS((BeEnc_ptr self));

static void be_enc_realloc_subst_array ARGS((BeEnc_ptr self));

static be_ptr
be_enc_shift_exp_at_time ARGS((BeEnc_ptr self, const be_ptr exp, int time));

static be_ptr
be_enc_shift_exp_at_times ARGS((BeEnc_ptr self, const be_ptr exp,
                                int c_time, int f_time,
                                int i_time, int n_time));

static void be_enc_clean_shift_hash ARGS((BeEnc_ptr self));

static int be_enc_shift_hash_key_cmp ARGS((const char* _key1,
                                           const char* _key2));

static int be_enc_shift_hash_key_hash ARGS((char* _key, const int size));

static enum st_retval
be_enc_shift_hash_callback_del_entry_and_key ARGS((char* key, char* record,
                                                   char* dummy));


/*---------------------------------------------------------------------------*/
/* Static inline function definitions                                        */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis    [Returns the size of the untimed vars block]

Description [Returns the sum of input, frozen, current state and next state
variables]

SideEffects []

SeeAlso     []

******************************************************************************/
static inline int be_enc_get_untimed_block_size(const BeEnc_ptr self)
{return self->input_vars_num + self->frozen_vars_num + (self->state_vars_num * 2);}


/**Function********************************************************************

Synopsis    [Returns the size of one single timed vars block]

Description [Returns the sum of input and state variables]

SideEffects []

SeeAlso     []

******************************************************************************/
static inline int be_enc_get_timed_block_size(const BeEnc_ptr self)
{ return self->input_vars_num + self->state_vars_num + self->frozen_vars_num; }


/**Function********************************************************************

Synopsis    [Converts a physical index to a logical index]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static inline int be_enc_index_phy_to_log(BeEnc_ptr self, int phy_idx)
{
  nusmv_assert(phy_idx > 0 && phy_idx <= self->max_used_phy_idx);
  return self->phy2log[phy_idx];
}


/**Function********************************************************************

Synopsis    [Converts a logical index to a physical index]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static inline int be_enc_index_log_to_phy(BeEnc_ptr self, int log_idx)
{
  nusmv_assert((log_idx >= 0) &&
               (log_idx < (be_enc_get_untimed_block_size(self) +
                           be_enc_get_timed_block_size(self) *
                           (self->max_allocated_time + 1))));
  return self->log2phy[log_idx];
}


/**Function********************************************************************

Synopsis    [Returns true if the given logical index is within the
untimed block]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static inline boolean
be_enc_is_log_index_untimed(const BeEnc_ptr self, int log_idx)
{
  return ((self->input_vars_num > 0) ||
          (self->state_vars_num > 0) ||
          (self->frozen_vars_num > 0)) &&
    (log_idx >= 0) && (log_idx < be_enc_get_untimed_block_size(self));
}


/**Function********************************************************************

Synopsis    [Returns true if the given logical index is within the
timed blocks set]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static inline boolean
be_enc_is_log_index_timed(const BeEnc_ptr self, int log_idx)
{
  return ((self->input_vars_num > 0) ||
          (self->state_vars_num > 0) ||
          (self->frozen_vars_num > 0)) &&
    (log_idx >= be_enc_get_untimed_block_size(self) &&
     (log_idx < (be_enc_get_untimed_block_size(self) +
                 be_enc_get_timed_block_size(self) * (self->max_allocated_time + 1))));
}


/**Function********************************************************************

Synopsis    [Returns true if the given logical index is within the
untimed block, and it is a current state variable index]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static inline boolean
be_enc_is_log_index_untimed_curr_state(const BeEnc_ptr self, int log_idx)
{
  return (self->state_vars_num > 0) && (log_idx >= 0) &&
    (log_idx < self->state_vars_num);
}


/**Function********************************************************************

Synopsis    [Returns true if the given logical index is within the
untimed block, and it is a frozen variable index]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static inline boolean
be_enc_is_log_index_untimed_frozen(const BeEnc_ptr self, int log_idx)
{
  return (self->frozen_vars_num > 0) && (log_idx >= self->state_vars_num) &&
    (log_idx < (self->frozen_vars_num + self->state_vars_num));
}


/**Function********************************************************************

Synopsis    [Returns true if the given logical index is within the
untimed block, and it is an input variable index]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static inline boolean
be_enc_is_log_index_untimed_input(const BeEnc_ptr self, int log_idx)
{
  return (self->input_vars_num > 0) &&
    (log_idx >= (self->state_vars_num + self->frozen_vars_num)) &&
    (log_idx < (self->input_vars_num + self->state_vars_num
                + self->frozen_vars_num));
}


/**Function********************************************************************

Synopsis    [Returns true if the given logical index is within the
untimed block, and it is a current state, frozen or input variable index]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static inline boolean
be_enc_is_log_index_untimed_curr_state_frozen_input(const BeEnc_ptr self,
                                                    int log_idx)
{
  return ((self->input_vars_num > 0) ||
          (self->state_vars_num > 0) ||
          (self->frozen_vars_num > 0)) &&
    (log_idx >= 0) &&
    (log_idx < (self->input_vars_num + self->state_vars_num + self->frozen_vars_num));
}


/**Function********************************************************************

Synopsis    [Returns true if the given logical index is within the
untimed block, and it is a next state variable index]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static inline boolean
be_enc_is_log_index_untimed_next_state(const BeEnc_ptr self, int log_idx)
{
  return be_enc_is_log_index_untimed(self, log_idx) &&
    (! be_enc_is_log_index_untimed_curr_state_frozen_input(self, log_idx));
}


/**Function********************************************************************

Synopsis    [Given a logical index within the
untimed block the function returns a type indicating whether the index is
a current state, frozen, input or next state variable.]

Description [if the index is outside of the untimed block, BE_VAR_TYPE_ERROR
is returned]

SideEffects []

SeeAlso     []

******************************************************************************/
static inline  BeVarType
be_enc_type_of_log_index_untimed(const BeEnc_ptr self, int log_idx)
{
  if (log_idx < 0) return BE_VAR_TYPE_ERROR;
  else if ((log_idx -= self->state_vars_num) < 0) return BE_VAR_TYPE_CURR;
  else if ((log_idx -= self->frozen_vars_num) < 0) return BE_VAR_TYPE_FROZEN;
  else if ((log_idx -= self->input_vars_num) < 0) return BE_VAR_TYPE_INPUT;
  else if ((log_idx -= self->state_vars_num) < 0) return BE_VAR_TYPE_NEXT;
  else return BE_VAR_TYPE_ERROR;
}



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [The BeEnc class constructor]

Description        [The BeEnc class constructor]

SideEffects        []

SeeAlso            [BeEnc_destroy]

******************************************************************************/
BeEnc_ptr BeEnc_create(SymbTable_ptr symb_table,
                       BoolEnc_ptr bool_enc)
{
  BeEnc_ptr self = ALLOC(BeEnc, 1);
  BE_ENC_CHECK_INSTANCE(self);

  be_enc_init(self, symb_table, bool_enc);
  return self;
}


/**Function********************************************************************

Synopsis           [The BeEnc class destructor]

Description        [The BeEnc class destructor]

SideEffects        []

SeeAlso            [BeEnc_create]

******************************************************************************/
void BeEnc_destroy(BeEnc_ptr self)
{
  BE_ENC_CHECK_INSTANCE(self);
  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

Synopsis  [Returns the <tt>number of state variables currently
handled by the encoder</tt>]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int BeEnc_get_state_vars_num(const BeEnc_ptr self)
{
  BE_ENC_CHECK_INSTANCE(self);
  return self->state_vars_num;
}


/**Function********************************************************************

Synopsis  [Returns the <tt>number of frozen variables currently
handled by the encoder</tt>]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int BeEnc_get_frozen_vars_num(const BeEnc_ptr self)
{
  BE_ENC_CHECK_INSTANCE(self);
  return self->frozen_vars_num;
}


/**Function********************************************************************

Synopsis  [Returns the <tt>number of input variables currently
handled by the encoder</tt>]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int BeEnc_get_input_vars_num(const BeEnc_ptr self)
{
  BE_ENC_CHECK_INSTANCE(self);
  return self->input_vars_num;
}


/**Function********************************************************************

Synopsis [Returns the <tt>number of input and state variables
currently handled by the encoder</tt>]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int BeEnc_get_vars_num(const BeEnc_ptr self)
{
  BE_ENC_CHECK_INSTANCE(self);
  return BeEnc_get_input_vars_num(self) + BeEnc_get_state_vars_num(self)
    + BeEnc_get_frozen_vars_num(self);
}


/**Function********************************************************************

Synopsis           [<b>Returns</b> the <tt>maximum allocated time </tt>]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int BeEnc_get_max_time(const BeEnc_ptr self)
{
  BE_ENC_CHECK_INSTANCE(self);
  return self->max_allocated_time;
}


/**Function********************************************************************

Synopsis           [<b>Returns</b> the
<tt>Boolean Expressions manager</tt> contained into the variable manager,
to be used by any operation on BEs]

Description        [Warning: do not delete the returned instance of
Be_Manager class, it belongs to self]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
Be_Manager_ptr BeEnc_get_be_manager(const BeEnc_ptr self)
{
  BE_ENC_CHECK_INSTANCE(self);
  return self->be_mgr;
}


/**Function********************************************************************

Synopsis [Given the variable name, returns the corresponding BE
untimed variable]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_name_to_untimed(const BeEnc_ptr self, const node_ptr var_name)
{
  be_ptr curvar;

  BE_ENC_CHECK_INSTANCE(self);

  curvar = (be_ptr) find_assoc(self->name2be, var_name);
  nusmv_assert(curvar != (be_ptr) NULL);

  return curvar;
}


/**Function********************************************************************

Synopsis [Given the index of a be var, returns the symbolic variable
name.]

Description        [Current implementation requires that the variable belongs
to the untimed block]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
node_ptr BeEnc_index_to_name(const BeEnc_ptr self, const int index)
{
  int log_idx;

  BE_ENC_CHECK_INSTANCE(self);

  log_idx = be_enc_index_phy_to_log(self, index);
  nusmv_assert(log_idx < self->index2name_size);

  return self->index2name[log_idx];
}


/**Function********************************************************************

Synopsis [Given the name of a be var, returns the untimed index that
variable is allocated at.]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int BeEnc_name_to_index(const BeEnc_ptr self, const node_ptr name)
{
  BE_ENC_CHECK_INSTANCE(self);

  return Be_Var2Index(self->be_mgr, BeEnc_name_to_untimed(self, name));
}


/**Function********************************************************************

Synopsis [Given a be variable, returns the correponding
variable name]

Description [Current implementation requires the given be variable
to be untimed ]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
node_ptr BeEnc_var_to_name(const BeEnc_ptr self, be_ptr be_var)
{
  int phy_idx;

  BE_ENC_CHECK_INSTANCE(self);

  phy_idx = Be_Var2Index(self->be_mgr, be_var);
  return BeEnc_index_to_name(self, phy_idx);
}


/**Function********************************************************************

Synopsis [Given a variable index, returns the corresponding be variable]

Description [Current implementation requires that the variable index
belongs to the untimed block]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_index_to_var(const BeEnc_ptr self, const int index)
{
  int log_idx;

  BE_ENC_CHECK_INSTANCE(self);

  log_idx = be_enc_index_phy_to_log(self, index);
  nusmv_assert(be_enc_is_log_index_untimed(self, log_idx));

  return Be_Index2Var(self->be_mgr, index);
}


/**Function********************************************************************

Synopsis [Given a be variable, returns the corresponding be index]

Description        []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int BeEnc_var_to_index(const BeEnc_ptr self, const be_ptr var)
{
  BE_ENC_CHECK_INSTANCE(self);
  return Be_Var2Index(self->be_mgr, var);
}


/**Function********************************************************************

Synopsis [Given an untimed variable index, returns the corresponding
BE variable at the given time]

Description [This method expands the maximum allocated time if necessary.

WARNING: If the given index corresponds to an untimed next state
var, the returned timed var will be instantitated at time+1

NOTE: Frozen variables are returned untimed as timed frozen
variables are never instantiated.
]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_index_to_timed(const BeEnc_ptr self, const int index,
                            const int time)
{
  int log_idx;

  BE_ENC_CHECK_INSTANCE(self);

  log_idx = be_enc_index_phy_to_log(self, index);
  nusmv_assert(be_enc_is_log_index_untimed(self, log_idx));

  /* extends the maxtime if needed. Notice that for input and next
     variables one additional time is required */
  if (be_enc_is_log_index_untimed_input(self, log_idx) ||
      be_enc_is_log_index_untimed_next_state(self, log_idx)) {
    be_enc_extend_timed_blocks(self, time+1);
  }
  else be_enc_extend_timed_blocks(self, time);

  /* jumps to time, and returns the correpsonding var */
  log_idx = be_enc_index_log_untimed_to_timed(self, log_idx, time);
  return Be_Index2Var(self->be_mgr, be_enc_index_log_to_phy(self, log_idx));
}


/**Function********************************************************************

Synopsis [Given a timed variable index, returns the corresponding
untimed BE variable index]

Description [Returned index will refer either to an untimed current
state variable, an untimed frozen variable or an untimed input variable.]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int BeEnc_index_to_untimed_index(const BeEnc_ptr self, const int index)
{
  int log_idx;

  BE_ENC_CHECK_INSTANCE(self);

  log_idx = be_enc_index_log_timed_to_untimed(self,
                                              be_enc_index_phy_to_log(self, index));

  return be_enc_index_log_to_phy(self, log_idx);
}


/**Function********************************************************************

Synopsis    [Given a state or input variable index this returns the time
the variable is allocated]

Description [The given state or input variable index must refer a timed
variable.

If the given index refer to a frozen variable then BE_CURRENT_UNTIMED
constant is returned (frozen variables are only allocated in the
untimed block).]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
int BeEnc_index_to_time(const BeEnc_ptr self, const int index)
{
  BE_ENC_CHECK_INSTANCE(self);

  return be_enc_index_log_to_time(self, be_enc_index_phy_to_log(self, index));
}


/**Function********************************************************************

Synopsis [Given an untimed variable, returns the corresponding
BE variable at the given time]

Description [This method expands the maximum allocated time if necessary.

WARNING: If the given variable is an untimed next state, the
returned index will be instantitated at time+1

NOTE: Frozen variables are returned untimed as they are always untimed.
]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_var_to_timed(const BeEnc_ptr self, const be_ptr var,
                          const int time)
{
  BE_ENC_CHECK_INSTANCE(self);
  return BeEnc_index_to_timed(self, Be_Var2Index(self->be_mgr, var), time);
}


/**Function********************************************************************

Synopsis    [Given a timed or untimed variable, returns the corresponding
BE variable in the current state untimed block (current state, frozen
and input vars).]

Description []

SideEffects        [None]

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_var_to_untimed(const BeEnc_ptr self, const be_ptr var)
{
  int log_idx;
  int phy_idx;

  BE_ENC_CHECK_INSTANCE(self);

  log_idx = be_enc_index_phy_to_log(self, Be_Var2Index(self->be_mgr, var));
  phy_idx = be_enc_index_log_to_phy(self,
                                    be_enc_index_log_timed_to_untimed(self, log_idx));

  return Be_Index2Var(self->be_mgr, phy_idx);
}


/**Function********************************************************************

Synopsis [Given the name of an untimed variable, returns the
corresponding BE variable at the given time]

Description [This method expands the maximum allocated time if necessary.

WARNING: If the given variable name corresponds to an untimed next
state var, the returned index will be instantitated at time+1

NOTE: Frozen variables are returned untimed as they are never allocated as
timed variables.
]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_name_to_timed(const BeEnc_ptr self, const node_ptr name,
                           const int time)
{
  BE_ENC_CHECK_INSTANCE(self);
  return BeEnc_index_to_timed(self, BeEnc_name_to_index(self, name), time);
}


/**Function********************************************************************

Synopsis    [Converts an untimed current state variable to the corresponding
untimed variable in the next state untimed block.]

Description [Given variable must be a current state untimed variable]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_var_curr_to_next(const BeEnc_ptr self, const be_ptr curr)
{
  int log_idx;

  BE_ENC_CHECK_INSTANCE(self);

  log_idx = be_enc_index_log_curr_to_next(self,
                                          be_enc_index_phy_to_log(self, Be_Var2Index(self->be_mgr, curr)));
  return Be_Index2Var(self->be_mgr, be_enc_index_log_to_phy(self, log_idx));
}


/**Function********************************************************************

Synopsis    [Converts an untimed next state variable to the corresponding
untimed variable in the current state untimed block.]

Description [Given variable must be a next state untimed variable]

SideEffects        [None]

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_var_next_to_curr(const BeEnc_ptr self, const be_ptr next)
{
  int log_idx;

  BE_ENC_CHECK_INSTANCE(self);

  log_idx = be_enc_index_log_next_to_curr(self,
                                          be_enc_index_phy_to_log(self, Be_Var2Index(self->be_mgr, next)));
  return Be_Index2Var(self->be_mgr, be_enc_index_log_to_phy(self, log_idx));
}


/**Function********************************************************************

Synopsis [Checks whether given index corresponds to a state variable]

Description        [Valid state variables are in current and next state
blocks, and in timed state areas.]

SideEffects        []

SeeAlso            []

******************************************************************************/
boolean BeEnc_is_index_state_var(const BeEnc_ptr self, const int index)
{
  int log_idx;

  BE_ENC_CHECK_INSTANCE(self);

  log_idx = be_enc_index_log_timed_to_untimed(self,
                                              be_enc_index_phy_to_log(self, index));

  return be_enc_is_log_index_untimed_curr_state(self, log_idx) ||
    be_enc_is_log_index_untimed_next_state(self, log_idx);
}


/**Function********************************************************************

Synopsis [Checks whether given index corresponds to an input variable]

Description        [Input variables are in the input untimed block,
or in timed input areas.]

SideEffects        []

SeeAlso            []

******************************************************************************/
boolean BeEnc_is_index_input_var(const BeEnc_ptr self, const int index)
{
  int log_idx;

  BE_ENC_CHECK_INSTANCE(self);

  log_idx = be_enc_index_log_timed_to_untimed(self,
                                              be_enc_index_phy_to_log(self, index));

  return be_enc_is_log_index_untimed_input(self, log_idx);
}


/**Function********************************************************************

Synopsis           [Checks whether given index corresponds to a frozen variable]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
boolean BeEnc_is_index_frozen_var(const BeEnc_ptr self, const int index)
{
  BE_ENC_CHECK_INSTANCE(self);
  return BeEnc_is_index_untimed_frozen(self, index);
}


/**Function********************************************************************

Synopsis [Checks whether given index corresponds to an untimed variable]

Description        [Note: frozen variables are always untimed.]

SideEffects        []

SeeAlso            []

******************************************************************************/
boolean BeEnc_is_index_untimed(const BeEnc_ptr self, const int index)
{
  BE_ENC_CHECK_INSTANCE(self);
  return be_enc_is_log_index_untimed(self,
                                     be_enc_index_phy_to_log(self, index));
}


/**Function********************************************************************

Synopsis [Checks whether given index corresponds to an untimed
current state variable]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
boolean BeEnc_is_index_untimed_curr(const BeEnc_ptr self, const int index)
{
  BE_ENC_CHECK_INSTANCE(self);
  return be_enc_is_log_index_untimed_curr_state(self,
                                      be_enc_index_phy_to_log(self, index));
}


/**Function********************************************************************

Synopsis [Checks whether given index corresponds to an untimed
frozen variable]

Description        [Frozen variables are always untimed. So this function is
exactly the same as BeEnc_is_index_frozen_var]

SideEffects        []

SeeAlso            [BeEnc_is_index_frozen_var]

******************************************************************************/
boolean BeEnc_is_index_untimed_frozen(const BeEnc_ptr self, const int index)
{
  BE_ENC_CHECK_INSTANCE(self);
  return be_enc_is_log_index_untimed_frozen(self,
                                            be_enc_index_phy_to_log(self, index));
}


/**Function********************************************************************

Synopsis [Checks whether given index corresponds to an untimed input
variable]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
boolean BeEnc_is_index_untimed_input(const BeEnc_ptr self, const int index)
{
  BE_ENC_CHECK_INSTANCE(self);
  return be_enc_is_log_index_untimed_input(self,
                                      be_enc_index_phy_to_log(self, index));
}


/**Function********************************************************************

Synopsis [Checks whether given index corresponds to an untimed
current state variable, or an untimed frozen, or an untimed input variable]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
boolean BeEnc_is_index_untimed_curr_frozen_input(const BeEnc_ptr self,
                                                 const int index)
{
  BE_ENC_CHECK_INSTANCE(self);
  return be_enc_is_log_index_untimed_curr_state_frozen_input(self,
                                         be_enc_index_phy_to_log(self, index));
}


/**Function********************************************************************

Synopsis [Checks whether given index corresponds to an untimed
next state variable]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
boolean BeEnc_is_index_untimed_next(const BeEnc_ptr self, const int index)
{
  BE_ENC_CHECK_INSTANCE(self);
  return be_enc_is_log_index_untimed_next_state(self,
                                         be_enc_index_phy_to_log(self, index));
}


/**Function********************************************************************

Synopsis           [Call this to begin an iteration between a given
category of variables]

Description [Use this method to begin an iteration between a given
category of variables, for example the set of input variables, or
the set of current and next state variables. The type is a bitwise
OR combination of types. When the first index is obtained, following
indices can be obtained by calling get_next_var_index, until
is_var_index_valid returns false, that means that the
iteration is over and must be given up.]

SideEffects        [None]

SeeAlso            [BeEnc_get_next_var_index, BeEnc_get_var_index_with_offset,
BeEnc_is_var_index_valid]

******************************************************************************/
int BeEnc_get_first_untimed_var_index(const BeEnc_ptr self, BeVarType type)
{
  int res = BE_ENC_INVALID_IDX;

  BE_ENC_CHECK_INSTANCE(self);

  if (((type & BE_VAR_TYPE_CURR) != 0) && (self->state_vars_num > 0)) {
    res = be_enc_index_log_to_phy(self, 0);
  }
  else if (((type & BE_VAR_TYPE_FROZEN) != 0) && (self->frozen_vars_num > 0)) {
    res = be_enc_index_log_to_phy(self, self->state_vars_num);
  }
  else if (((type & BE_VAR_TYPE_INPUT) != 0) && (self->input_vars_num > 0)) {
    res = be_enc_index_log_to_phy(self,
                                  self->state_vars_num + self->frozen_vars_num);
  }
  else if (((type & BE_VAR_TYPE_NEXT) != 0) && (self->state_vars_num > 0)) {
    res = be_enc_index_log_to_phy(self,
              self->state_vars_num + self->frozen_vars_num + self->input_vars_num);
  }

  return res;
}


/**Function********************************************************************

Synopsis [Use to sequentially iterate over a selected category of
variables.]

Description [Use this method to obtain the index of the variable
that follows the variable whose index is provided. If the iteration
is over, an invalid index will be returned. Use the method
is_var_index_valid to check the validity of the returned index.]

SideEffects        [None]

SeeAlso            [BeEnc_get_first_untimed_var_index,
BeEnc_is_var_index_valid, BeEnc_get_var_index_with_offset]

******************************************************************************/
int BeEnc_get_next_var_index(const BeEnc_ptr self,
                             int var_index, BeVarType type)
{
  BE_ENC_CHECK_INSTANCE(self);
  return BeEnc_get_var_index_with_offset(self, var_index, 1, type);
}


/**Function********************************************************************

Synopsis           [Use to randomly iterate over a selected category
of variables within the untimed block.]

Description [Use this method to obtain the index of the variable
that follows the variable whose index is provided, after offset
positions. If the iteration is over, an invalid index will be
returned. Use the method is_var_index_valid to check the validity of
the returned index.]

SideEffects        [None]

SeeAlso            [BeEnc_get_first_untimed_var_index,
BeEnc_is_var_index_valid, BeEnc_get_next_var_index]

******************************************************************************/
int BeEnc_get_var_index_with_offset(const BeEnc_ptr self,
                                    int from_index, int offset,
                                    BeVarType type)
{
  int log_idx;
  BeVarType oldType, newType;

  BE_ENC_CHECK_INSTANCE(self);

  if (!BeEnc_is_var_index_valid(self, from_index)) return BE_ENC_INVALID_IDX;

  log_idx = be_enc_index_phy_to_log(self, from_index);

  oldType = be_enc_type_of_log_index_untimed(self, log_idx);
  newType = be_enc_type_of_log_index_untimed(self, log_idx + offset);

  nusmv_assert((type & BE_VAR_TYPE_ALL) == type); /* illegal type */
  nusmv_assert(oldType != BE_VAR_TYPE_ERROR); /* the provided index must be in untimed block */
  nusmv_assert(oldType & type); /* the provided index must be correct wrt thh provided type */

  /* new index is of the same type and is required => just return it */
  if (oldType == newType && (newType & type)) {
    return be_enc_index_log_to_phy(self, log_idx + offset);
  }

  /* calculate the next requested type */
  switch (oldType) {
  case BE_VAR_TYPE_NEXT:
    return BE_ENC_INVALID_IDX; /* there are no more vars => return invalid idx */

  case BE_VAR_TYPE_INPUT:
    newType = (type & BE_VAR_TYPE_NEXT);
    break;
  case BE_VAR_TYPE_FROZEN:
    newType = type & (BE_VAR_TYPE_INPUT | BE_VAR_TYPE_NEXT);
    break;
  case BE_VAR_TYPE_CURR:
    newType = type & (BE_VAR_TYPE_FROZEN | BE_VAR_TYPE_INPUT | BE_VAR_TYPE_NEXT);
    break;
  default: error_unreachable_code(); /* impossible code */
  };
  /* return the first variable of the next required type */
  return BeEnc_get_first_untimed_var_index(self, newType);
}


/**Function********************************************************************

Synopsis [Use to check whether an iteration over a set of variables
is over.]

Description [This method returns true whether the index returned by
methods get_first_untimed_var_index, get_next_var_index and
get_var_index_with_offset is valid and can be used later in the
iteration. If false is returned, then the iteration is over.]

SideEffects        [None]

SeeAlso            [BeEnc_get_first_untimed_var_index,
BeEnc_get_next_var_index, BeEnc_get_var_index_with_offset]

******************************************************************************/
boolean BeEnc_is_var_index_valid(const BeEnc_ptr self, int var_index)
{
  BE_ENC_CHECK_INSTANCE(self);
  return var_index != BE_ENC_INVALID_IDX;
}


/**Function********************************************************************

Synopsis           [<b>Shifts</b> given <i>current</i> <b>expression at
next time</b>]

Description [Warning: argument 'exp' must contain only untimed
current state variables and untimed frozen variables,
otherwise results will be unpredictible]

SideEffects        []

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_shift_curr_to_next(BeEnc_ptr self, const be_ptr exp)
{
  BE_ENC_CHECK_INSTANCE(self);

  /* It is impossible to shift just by one delta, because
     frozen vars should not be shifted at all.
     Shifting by many deltas is less efficient but this is not a problem
     because practically only FSM's invariant is shifted
     and memoizing will cause this shifting to be performed only once. */

  return be_enc_shift_exp_at_times(self, exp,
                                   BE_ENC_NEXT_UNTIMED_TIME, /* curr->next */
                                   BE_ENC_NO_TIME, /* no shifting for frozen */
                                   BE_ENC_INVALID_TIME, /* impossible for input */
                                   BE_ENC_INVALID_TIME);/* impossible for next */
}


/**Function********************************************************************

Synopsis           [<b>Shifts</b> given <i>untimed</i> <b>expression at
the given time</b>]

Description [All next state variables will be shifted to time+1.
Maximum time is extended if necessary.
WARNING:
argument 'exp' must contain only untimed variables, otherwise
results will be unpredictible]

SideEffects        []

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_untimed_expr_to_timed(BeEnc_ptr self, const be_ptr exp,
                                   const int time)
{
  BE_ENC_CHECK_INSTANCE(self);

  be_enc_extend_timed_blocks(self, time+1);
  return be_enc_shift_exp_at_time(self, exp, time);
}


/**Function********************************************************************

Synopsis           [Given an untimed expression, shifts current, frozen, input and
next variables to given times]

Description        [Shifts untimed current state vars to time ctime, frozen
untimed vars to ftime, input untimed vars to itime, and untimed next
state vars to ntime.]

SideEffects        []

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_untimed_expr_to_times(BeEnc_ptr self, const be_ptr exp,
                                   const int ctime, const int ftime,
                                   const int itime, const int ntime)
{
  int max_time;
  BE_ENC_CHECK_INSTANCE(self);

  /* extends maxtime, if required */
  max_time = MAX(ctime, ftime);
  max_time = MAX(max_time, itime);
  max_time = MAX(max_time, ntime);

  be_enc_extend_timed_blocks(self, max_time);
  return be_enc_shift_exp_at_times(self, exp, ctime, ftime, itime, ntime);
}


/**Function********************************************************************

Synopsis           [<b>Makes an AND interval</b> of given expression using
<b>range \[<tt>from</tt>, <tt>to</tt>\]</b>]

Description        [Maximum time is extended if necessary.]

SideEffects        []

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_untimed_to_timed_and_interval(BeEnc_ptr self,
                                           const be_ptr exp,
                                           const int from, const int to)
{
  be_ptr res;

  BE_ENC_CHECK_INSTANCE(self);

  /* We accept the cases (from <= to) and (from == to - 1).
     The latter may exist when the unrolling is performed at high level */
  nusmv_assert(from <= to+1);

  if (from > to) {
    /* ends the recursion */
    res = Be_Truth(self->be_mgr);
  }
  else {
    res =  Be_And(self->be_mgr,
                  BeEnc_untimed_to_timed_and_interval(self, exp, from, to-1),
                  BeEnc_untimed_expr_to_timed(self, exp, to));
  }

  return res;
}


/**Function********************************************************************

Synopsis           [<b>Makes an OR interval</b> of given expression using
<b>range \[<tt>from</tt>, <tt>to</tt>\]</b>]

Description        [Maximum time is extended if necessary.]

SideEffects        []

SeeAlso            []

******************************************************************************/
be_ptr BeEnc_untimed_to_timed_or_interval(BeEnc_ptr self,
                                          const be_ptr exp,
                                          const int from, const int to)
{
  be_ptr res;

  BE_ENC_CHECK_INSTANCE(self);

  /* We accept the cases (from <= to) and (from == to - 1).
     The latter may exist when the unrolling is performed at high level */
  nusmv_assert(from <= to+1);

  if (from > to) {
    /* ends the recursion */
    res = Be_Falsity(self->be_mgr);
  }
  else {
    res = Be_Or(self->be_mgr,
                BeEnc_untimed_expr_to_timed(self, exp, from),
                BeEnc_untimed_to_timed_or_interval(self, exp, from + 1, to));
  }

  return res;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [The BeEnc class private initializer]

Description        [The BeEnc class private initializer]

SideEffects        []

SeeAlso            [BeEnc_create]

******************************************************************************/
void be_enc_init(BeEnc_ptr self,
                 SymbTable_ptr symb_table, BoolEnc_ptr bool_enc)
{
  /* base class initialization */
  bool_enc_client_init(BOOL_ENC_CLIENT(self), symb_table, bool_enc);

  /* members initialization */
  self->be_mgr = Be_RbcManager_Create(0);
  self->state_vars_num = 0;
  self->frozen_vars_num = 0;
  self->input_vars_num = 0;

  self->phy_idx_capacity = 0;
  self->max_used_phy_idx = 0;
  self->log_idx_capacity = 0;
  self->grow_excess = 0;
  self->max_allocated_time = -1;

  self->avail_phy_idx_queue = NodeList_create();

  self->name2be = new_assoc();
  nusmv_assert(self->name2be != (hash_ptr) NULL);

  /* arrays: */
  self->index2name = (node_ptr*) NULL;
  self->index2name_size = 0;

  self->log2phy = (int*) NULL;
  self->phy2log = (int*) NULL;

  self->subst_array = (int*) NULL; /* this is allocated on demand */
  self->subst_array_size = 0;

  /* allocates the shifting memoizing hash */
  self->shift_hash =
    st_init_table(&be_enc_shift_hash_key_cmp, &be_enc_shift_hash_key_hash);
  nusmv_assert(self->shift_hash != (st_table*) NULL);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = be_enc_finalize;

  /* inherited by base class: */
  OVERRIDE(BaseEnc, commit_layer) = be_enc_commit_layer;
  OVERRIDE(BaseEnc, remove_layer) = be_enc_remove_layer;
}


/**Function********************************************************************

Synopsis           [The BeEnc class private deinitializer]

Description        [The BeEnc class private deinitializer]

SideEffects        []

SeeAlso            [BeEnc_destroy]

******************************************************************************/
void be_enc_deinit(BeEnc_ptr self)
{
  /* members deinitialization */
  be_enc_clean_shift_hash(self);
  st_free_table(self->shift_hash);

  if (self->subst_array != (int*) NULL) FREE(self->subst_array);
  if (self->phy2log != (int*) NULL) FREE(self->phy2log);
  if (self->log2phy != (int*) NULL) FREE(self->log2phy);
  if (self->index2name != (node_ptr*) NULL) FREE(self->index2name);
  free_assoc(self->name2be);
  NodeList_destroy(self->avail_phy_idx_queue);
  Be_RbcManager_Delete(self->be_mgr);

  /* base class initialization */
  bool_enc_client_deinit(BOOL_ENC_CLIENT(self));
}


/**Function********************************************************************

Synopsis [Encodes all the boolean variables within the given
layer. If the given layer has an associated boolean layer (created
by the BoolEnc), that boolean layer will be encoded as well.]

Description        []

SideEffects        []

SeeAlso            [be_enc_remove_layer]

******************************************************************************/
void be_enc_commit_layer(BaseEnc_ptr enc_base, const char* layer_name)
{
  BeEnc_ptr self;
  SymbLayer_ptr layers[3];
  const char* bool_layer_name;
  int i;

  self = BE_ENC(enc_base);

  /* Calls the base method to add this layer */
  bool_enc_client_commit_layer(enc_base, layer_name);

  layers[0] = SymbTable_get_layer(enc_base->symb_table, layer_name);

  /* tries to retrieve the boolean layer (if there is any) that has
     been created from the layer that is being committed. If there
     exists such a layer, commit is as well. */
  bool_layer_name = BoolEnc_scalar_layer_to_bool_layer(layer_name);
  layers[1] = SymbTable_get_layer(enc_base->symb_table, bool_layer_name);
  if (layers[1] != SYMB_LAYER(NULL)) {
    bool_enc_client_commit_layer(enc_base, bool_layer_name);
  }

  layers[2] = SYMB_LAYER(NULL); /* a terminator */

  /* -------------------------------------------------- */
  /*               Begins the hard work                 */
  /* -------------------------------------------------- */

  i=0;
  while (layers[i] != SYMB_LAYER(NULL)) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr, "BeEnc: encoding layer %s\n",
              SymbLayer_get_name(layers[i]));
      inc_indent_size();
    }

    be_enc_add_vars(self, layers[i]);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) dec_indent_size();

    i += 1;
  } /* end of loop on layers */

  /* cleans up the shifting memoization cache */
  be_enc_clean_shift_hash(self);
}


/**Function********************************************************************

Synopsis           [Removes the encoding of all variables occurring within
the given layer, and those that had been created within the corresponding
boolean layer during the boolean encoding (if there was any).
Then releases both the layers.]

Description        [Cleans up the shifting cache, as well as the memoizing
cache for the CNF-ization, that become potentially invalid.

WARNING: If the layer has been
renamed after having been committed, it is the *new* name (the name
the layer has when it is being removed) that must be used, and *not*
the name that had been used when commiting it.]

SideEffects        []

SeeAlso            [bdd_enc_commit_layer]

******************************************************************************/
void be_enc_remove_layer(BaseEnc_ptr enc_base, const char* layer_name)
{
  BeEnc_ptr self;
  SymbLayer_ptr layers[3];
  const char* bool_layer_name;
  int i;

  self = BE_ENC(enc_base);

  layers[0] = SymbTable_get_layer(enc_base->symb_table, layer_name);

  /* tries to retrieve the boolean layer (if there is any) that has
     been created from the layer that is being committed. If there
     exists such a layer, remove it as well. */
  bool_layer_name = BoolEnc_scalar_layer_to_bool_layer(layer_name);

  layers[1] = SymbTable_get_layer(enc_base->symb_table, bool_layer_name);
  layers[2] = SYMB_LAYER(NULL); /* a terminator */

  /* -------------------------------------------------- */
  /*               Begins the hard work                 */
  /* -------------------------------------------------- */

  i=0;
  while (layers[i] != SYMB_LAYER(NULL)) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
      fprintf(nusmv_stderr, "BeEnc: removing layer %s\n",
              SymbLayer_get_name(layers[i]));
    }

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) inc_indent_size();

    be_enc_remove_vars(self, layers[i]);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) dec_indent_size();

    i += 1;
  } /* end of loop on layers */


  /* finally calls the inherited method: */
  bool_enc_client_remove_layer(enc_base, layer_name);
  if (layers[1] != SYMB_LAYER(NULL)) {
    bool_enc_client_remove_layer(enc_base, bool_layer_name);
  }

  /* cleans up the shifting memoization cache */
  be_enc_clean_shift_hash(self);

  /* cleans up memoization hashes of bmc */
  bmc_quit_tableau_memoization();
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis    [The BeEnc class virtual finalizer]

Description [Called by the class destructor]

SideEffects []

SeeAlso     []

******************************************************************************/
static void be_enc_finalize(Object_ptr object, void* dummy)
{
  BeEnc_ptr self = BE_ENC(object);

  be_enc_deinit(self);
  FREE(self);
}


/**Function********************************************************************

Synopsis [Re-arranges the entire logical level in order to have the
boolean variables contained in the passed layer added to the
encoder. ]

Description [Timed blocks will get resized as well, and new
variables will be created accordingly. Names of added variables cannot occur
already within self.]

SideEffects []

SeeAlso     [be_enc_remove_vars]

******************************************************************************/
static void be_enc_add_vars(BeEnc_ptr self, const SymbLayer_ptr layer)
{
  SymbLayerIter iter;
  int input_vars_num = SymbLayer_get_bool_input_vars_num(layer);
  int state_vars_num = SymbLayer_get_bool_state_vars_num(layer);
  int frozen_vars_num = SymbLayer_get_bool_frozen_vars_num(layer);
  int ofs;

  /* re-arranges the logical level moving forward the existing
     blocks, in order to prepare the space for the variables that
     are being added */
  be_enc_allocate_new_log_space(self, input_vars_num,
                                state_vars_num, frozen_vars_num);

  /* creates input be vars at newly allocated logical addresses */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, "BeEnc: encoding %d input variables...\n",
            input_vars_num);
    inc_indent_size();
  }

  ofs = 0;
  SYMB_LAYER_FOREACH_FILTER(layer, iter, STT_INPUT_VAR,
                            SymbLayer_iter_filter_bool_vars, NULL) {
    node_ptr var = SymbLayer_iter_get_symbol(layer, &iter);

    nusmv_assert(SymbTable_is_symbol_bool_var(BASE_ENC(self)->symb_table, var));
    be_enc_instantiate_var(self, var, input_vars_num,
                           state_vars_num, frozen_vars_num, ofs);
    ++ofs;
  } /* loop over the input vars */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) dec_indent_size();


  /* creates state be vars at newly allocated logical addresses */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, "BeEnc: encoding %d state variables...\n",
            state_vars_num);
    inc_indent_size();
  }

  ofs = 0;
  SYMB_LAYER_FOREACH_FILTER(layer, iter, STT_STATE_VAR,
                            SymbLayer_iter_filter_bool_vars, NULL) {
    node_ptr var = SymbLayer_iter_get_symbol(layer, &iter);

    nusmv_assert(SymbTable_is_symbol_bool_var(BASE_ENC(self)->symb_table, var));
    be_enc_instantiate_var(self, var, input_vars_num,
                           state_vars_num, frozen_vars_num, ofs);
    ++ofs;
  } /* loop over the state vars */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) dec_indent_size();

  /* creates frozen be vars at newly allocated logical addresses */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, "BeEnc: encoding %d frozen variables...\n",
            frozen_vars_num);
    inc_indent_size();
  }

  ofs = 0;
  SYMB_LAYER_FOREACH_FILTER(layer, iter, STT_FROZEN_VAR,
                            SymbLayer_iter_filter_bool_vars, NULL) {
    node_ptr var = SymbLayer_iter_get_symbol(layer, &iter);

    nusmv_assert(SymbTable_is_symbol_bool_var(BASE_ENC(self)->symb_table, var));
    be_enc_instantiate_var(self, var, input_vars_num,
                           state_vars_num, frozen_vars_num, ofs);
    ++ofs;
  } /* loop over the frozen vars */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) dec_indent_size();

  self->input_vars_num += input_vars_num;
  self->state_vars_num += state_vars_num;
  self->frozen_vars_num += frozen_vars_num;
}


/**Function********************************************************************

Synopsis    [Private service of be_enc_remove_var]

Description [Remove any reference to the variable whose physical
index is provided. Sets the logical level to an invalid index where
the removed variable index was contained. This method has been
implemented to factorize code, and it is called only by
be_enc_remove_var]

SideEffects []

SeeAlso     [be_enc_remove_var]

******************************************************************************/
static inline void be_enc_remove_var_aux(BeEnc_ptr self, int phy_idx)
{
  int log_idx;
  node_ptr name = (node_ptr) NULL;

  log_idx = be_enc_index_phy_to_log(self, phy_idx);
  if (log_idx < self->index2name_size) name = self->index2name[log_idx];

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    if (name != (node_ptr) NULL) {
      fprintf(nusmv_stderr, "BeEnc: removing untimed variable '");
      print_node(nusmv_stderr, name);
      fprintf(nusmv_stderr, "' at log index %d, phy index %d\n",
              log_idx, phy_idx);
    }
    else {
      fprintf(nusmv_stderr,
              "BeEnc: removing timed variable at log index %d, phy index %d\n",
              log_idx, phy_idx);
    }
  }

  if (name != (node_ptr) NULL) {
    insert_assoc(self->name2be, name, (node_ptr) NULL);
    self->index2name[log_idx] = (node_ptr) NULL;
  }

  self->phy2log[phy_idx] = BE_ENC_INVALID_IDX;
  self->log2phy[log_idx] = BE_ENC_INVALID_IDX;
  NodeList_append(self->avail_phy_idx_queue, NODE_FROM_INT(phy_idx));

  /* for frozen variables one phisical index correspond to several
     timed logical indexes => remove them all  */
  if (be_enc_is_log_index_untimed_frozen(self, log_idx)) {
    int time;
    for (time = 0; time <= self->max_allocated_time; ++time) {
      int timed_log_idx = be_enc_index_log_untimed_to_timed(self, log_idx, time);
      nusmv_assert(self->log2phy[timed_log_idx] == phy_idx); /* double check */
      self->log2phy[timed_log_idx] = BE_ENC_INVALID_IDX;
    } /* for */
  } /* if */
}


/**Function********************************************************************

Synopsis [Removes a variable whose name is provided]

Description [If it is a state var, it is not requested to explicitly
remove the corresponding next variable, as this operation is
automatically performed. Important: this method removes the variable
from both the logical and physical levels, and makes the
corresponding physical indices available for later new
instantiations. But this method does NOT side effect on the number of
variables, and the logical level have to be compacted be the caller,
that will have to calculate the new size as well.

This method should be called by be_enc_remove_vars only, and a
series of calls to this method must be followed by a call to
be_enc_compact_log_level]

SideEffects []

SeeAlso     [be_enc_remove_vars, be_enc_compact_log_level]

******************************************************************************/
static inline void be_enc_remove_var(BeEnc_ptr self, node_ptr name)
{
  BeVarType type;
  be_ptr be_var;
  int phy_idx, log_idx;
  int time;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr, "Preparing to remove var '");
    print_node(nusmv_stderr, name);
    fprintf(nusmv_stderr, "' ... \n");
  }

  be_var = BeEnc_name_to_untimed(self, name);

  phy_idx = Be_Var2Index(self->be_mgr, be_var);
  log_idx = be_enc_index_phy_to_log(self, phy_idx);
  type = be_enc_type_of_log_index_untimed(self, log_idx);

  nusmv_assert(self->index2name[log_idx] == name); /* A double check */
  nusmv_assert(type != BE_VAR_TYPE_ERROR); /* log_idx must be correct untimed index */
  nusmv_assert(type != BE_VAR_TYPE_NEXT); /* next state vars are removed only with current state vars */
  be_enc_remove_var_aux(self, phy_idx); /* untimed current var */

  if (BE_VAR_TYPE_CURR == type) {
    /* state var: removes untimed next variable: */
    node_ptr next_name;
    be_ptr next_be_var;
    int next_phy_idx, next_log_idx;

    next_name = find_node(NEXT, name, Nil);
    next_be_var = BeEnc_name_to_untimed(self, next_name);
    next_phy_idx = Be_Var2Index(self->be_mgr, next_be_var);
    next_log_idx = be_enc_index_phy_to_log(self, next_phy_idx);

    /* A double check: */
    nusmv_assert(self->index2name[next_log_idx] == next_name);
    be_enc_remove_var_aux(self, next_phy_idx); /* untimed next var */
  }
  /* remove phisical indexes of timed input and state vars.
     A frozen var have only one phisical index => no removes required.
  */
  if (BE_VAR_TYPE_CURR == type || BE_VAR_TYPE_INPUT == type) {
    for (time = 0; time <= self->max_allocated_time; ++time) {
      int timed_log_idx = be_enc_index_log_untimed_to_timed(self, log_idx, time);
      int timed_phy_idx = be_enc_index_log_to_phy(self, timed_log_idx);

      be_enc_remove_var_aux(self, timed_phy_idx);
    }
  }

}


/**Function********************************************************************

Synopsis [Removes the sets of boolean variables in layer, and compacts the
logical level.]

Description [At the end of this call, the boolean variables of the
layer will be removed and the internal status will be ok]

SideEffects []

SeeAlso     [be_enc_add_vars]

******************************************************************************/
static void be_enc_remove_vars(BeEnc_ptr self,
                               const SymbLayer_ptr layer)
{
  SymbLayerIter iter;

  /* Remove only boolean vars: Only bool vars are added to the enc.  */
  SYMB_LAYER_FOREACH_FILTER(layer, iter, STT_VAR,
                            SymbLayer_iter_filter_bool_vars, NULL) {
    node_ptr var = SymbLayer_iter_get_symbol(layer, &iter);
    be_enc_remove_var(self, var);
  }

  /* cleans up the bmc conv cache */
  // Bmc_Conv_cleanup_cached_entries_about(self, input_vars);
  // Bmc_Conv_cleanup_cached_entries_about(self, state_vars);

  /* commits the removals */
  be_enc_compact_log_level(self);
}


/**Function********************************************************************

Synopsis [Called after a serie of calls to be_enc_remove_var have
been done, to compact the logical level and fix the level's size. ]

Description [This method compacts the logical level, actually
removing all the unused indices, and making all the valid logical
indices close each other on the lowest logical indices. It also
adjusts fields input_vars_num and state_vars_num in order to make
them contain the real number of variables currently allocated. See
comments of be_enc_remove_var]

SideEffects []

SeeAlso     [be_enc_remove_var]

******************************************************************************/
static void be_enc_compact_log_level(BeEnc_ptr self)
{
  int rm_ivars = 0; /* counter for removed input vars */
  int rm_svars = 0; /* counter for removed state vars */
  int rm_fvars = 0; /* counter for removed frozen vars */
  int total_vars;
  int total_rm_vars = 0;
  int src_idx;
  int dest_idx;
  Set_t frozen_phy_idx = Set_MakeEmpty();

  total_vars = be_enc_get_untimed_block_size(self) +
    be_enc_get_timed_block_size(self) * (self->max_allocated_time+1);

  dest_idx = 0;
  for (src_idx = 0; src_idx < total_vars; ++src_idx) {
    int phy_idx = self->log2phy[src_idx];

    if (phy_idx != BE_ENC_INVALID_IDX) {
      if (dest_idx != src_idx) {
        if (MAX(dest_idx, src_idx) < self->index2name_size) {
          self->index2name[dest_idx] = self->index2name[src_idx];
          self->index2name[src_idx] = (node_ptr) NULL;
        }
        self->log2phy[dest_idx] = self->log2phy[src_idx];

        /* assigns phy2log only the first time with frozen
           variables as they share physical indices. This is done
           to fix bug #1621 */
        if (be_enc_is_log_index_untimed_frozen(self,
                        be_enc_index_log_timed_to_untimed(self, src_idx))) {
          if (!Set_IsMember(frozen_phy_idx,
                            PTR_FROM_INT(Set_Element_t, phy_idx))) {
            frozen_phy_idx = Set_AddMember(frozen_phy_idx,
                                           PTR_FROM_INT(Set_Element_t, phy_idx));

            self->phy2log[phy_idx] = dest_idx;
          }
        }
        else {
          /* this is not a frozen variable, so no particular care
             is needed wrt #1621 */
          self->phy2log[phy_idx] = dest_idx;
        }
      }

      dest_idx += 1;
    }
    else {
      /* this index was removed: */
      ++total_rm_vars;
      switch (be_enc_type_of_log_index_untimed(self, src_idx)) {
      case BE_VAR_TYPE_INPUT:   ++rm_ivars; break;
      case BE_VAR_TYPE_CURR: ++rm_svars; break;
      case BE_VAR_TYPE_FROZEN:  ++rm_fvars; break;
      default: break;
      }
    }
  }

  Set_ReleaseSet(frozen_phy_idx);

  self->input_vars_num -= rm_ivars;
  self->state_vars_num -= rm_svars;
  self->frozen_vars_num -= rm_fvars;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 5) && (rm_ivars+rm_svars+rm_fvars > 0)) {
    fprintf(nusmv_stderr,
            "BeEnc: compacted the log block of %d untimed indices, " \
            "%d total indices\n", (rm_ivars + rm_fvars + rm_svars*2), total_rm_vars);
  }
}


/**Function********************************************************************

Synopsis    [Private service of method be_enc_add_vars]

Description [Allocates the given variable within a newly created
logical block (at given offset within that block). This function
factorizes code that whould be otherwise mostly duplicated within
be_enc_add_vars to build input, state and frozen vars.  deltas are the size
of the newly created blocks for input and state variables, i.e. the
number of newly created state and input variables. Notice that space within
the untimed and timed blocks must be already re-arranged. ]

SideEffects []

SeeAlso     [be_enc_remove_vars]

******************************************************************************/
static void be_enc_instantiate_var(BeEnc_ptr self,
                                   node_ptr name,
                                   int input_vars_delta,
                                   int state_vars_delta,
                                   int frozen_vars_delta,
                                   int ofs)
{
  int time;
  int idx_log;
  int idx_timed;
  BeVarType type;

  /* It must be a variable not previously encoded */
  nusmv_assert(SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, name));
  nusmv_assert(find_assoc(self->name2be, name) == (node_ptr) NULL);

  if (SymbTable_is_symbol_input_var(BASE_ENC(self)->symb_table, name)) {
    type = BE_VAR_TYPE_INPUT;
  }
  else if (SymbTable_is_symbol_state_var(BASE_ENC(self)->symb_table, name)) {
    type = BE_VAR_TYPE_CURR;
  }
  else if (SymbTable_is_symbol_frozen_var(BASE_ENC(self)->symb_table, name)) {
    type = BE_VAR_TYPE_FROZEN;
  }
  else {
    internal_error("be_enc_instantiate_var: undefined identifier");
  }


  /* allocates the var within the untimed block */
  switch (type) {
  case BE_VAR_TYPE_CURR: /* state var */
    idx_log = self->state_vars_num + ofs;
    be_enc_create_be_var(self, idx_log, name); /* current */
    be_enc_create_be_var(self,
                         idx_log + self->state_vars_num + state_vars_delta
                         + self->frozen_vars_num + frozen_vars_delta
                         + self->input_vars_num + input_vars_delta,
                         find_node(NEXT, name, Nil)); /* next */
    break;

  case BE_VAR_TYPE_FROZEN: /* frozen var */
      idx_log = self->state_vars_num + state_vars_delta
        + self->frozen_vars_num + ofs;
      be_enc_create_be_var(self, idx_log, name);
      break;

  case BE_VAR_TYPE_INPUT : /* input var */
      idx_log = self->state_vars_num + state_vars_delta
        + self->frozen_vars_num + frozen_vars_delta + self->input_vars_num + ofs;
      be_enc_create_be_var(self, idx_log, name);
      break;

  default: internal_error("be_enc_instantiate_var : impossible code(1)");
  } /* switch */

  { /* calculate the new size of timed frame a*/
    int new_timed_frame_size = self->state_vars_num + state_vars_delta
      + self->frozen_vars_num + frozen_vars_delta
      + self->input_vars_num + input_vars_delta;

    /* calculate var's index in the timed frame 0  */
    idx_timed = idx_log + new_timed_frame_size
      + self->state_vars_num + state_vars_delta;

    switch (type) {
      /* for input and state vars allocate new physical indexes within the timed blocks */
    case BE_VAR_TYPE_CURR:
    case BE_VAR_TYPE_INPUT:
      for (time = 0; time <= self->max_allocated_time; ++time) {
        be_enc_create_be_var(self, idx_timed, (node_ptr) NULL);
        idx_timed += new_timed_frame_size;
      }
      break;

      /* for frozen var associate all logical indexes to the given physical one */
    case BE_VAR_TYPE_FROZEN:
      {
        int idx_phy = self->log2phy[idx_log];
        for (time = 0; time <= self->max_allocated_time; ++time) {
          self->log2phy[idx_timed] = idx_phy;
          idx_timed += new_timed_frame_size;
        }
      }
      break;

    default: internal_error("be_enc_instantiate_var : impossible code(2)");
    }
  }
}


/**Function********************************************************************

Synopsis    [Rearranges (increasing) the logical level in order to host
the given number of new input and state variables]

Description [This method cannot be used to decrease the number of
variables. Use method remove_var instead. This method creates
(allocates) the needed space to hold a set of new variables, and
rearranges the logical level in order to contain the new variables
in the right order and place.  Notice that this method does not
create actual BE variables, that later will have to be created by
calling the method create_be_var passing to it the newly
allocated logical addresses. The new logical addresses comes from
the way logical indices are organized, following an internal rule
that the caller well knows.]

SideEffects []

SeeAlso     []

******************************************************************************/
static void be_enc_allocate_new_log_space(BeEnc_ptr self,
                                          int input_vars_num,
                                          int state_vars_num,
                                          int frozen_vars_num)
{
  int time;
  int new_vars; /* total amount of new varaibles that have to be allocated */
  int idx, idx_new, idx_old;
  int new_timed_frame_size;

  new_vars = input_vars_num * (self->max_allocated_time + 2) +
    state_vars_num * (self->max_allocated_time + 3) +
    frozen_vars_num * (self->max_allocated_time + 2);
  if (new_vars <= 0) return;

  /* Allocates the necessary space */
  be_enc_allocate_space_for_new_vars(self, new_vars);

  /* Prepares the space for the names of the new variables */
  self->index2name_size = (self->state_vars_num + state_vars_num) * 2 +
    (self->frozen_vars_num + frozen_vars_num) +
    (self->input_vars_num + input_vars_num);

  self->index2name = REALLOC(node_ptr, self->index2name,
                             self->index2name_size);
  nusmv_assert(self->index2name != (node_ptr*) NULL);

  /* inits the new names */
  for (idx = self->state_vars_num * 2 + self->frozen_vars_num
         + self->input_vars_num;
       idx < self->index2name_size; ++idx) {
    self->index2name[idx] = (node_ptr) NULL;
  }

  /* new size of a timed frame */
  new_timed_frame_size = (self->state_vars_num + state_vars_num) +
    (self->frozen_vars_num + frozen_vars_num) +
    (self->input_vars_num + input_vars_num);
  /* end of the old timed block */
  idx_old = be_enc_get_untimed_block_size(self)
    + be_enc_get_timed_block_size(self) * (self->max_allocated_time + 1);
  /* the end of the new timed block */
  idx_new = (self->state_vars_num + state_vars_num)
    + new_timed_frame_size * (self->max_allocated_time + 2);

  /* -- at first moves the time block -- */
  for (time = self->max_allocated_time; time >= 0; --time) {
    /* moves the input block */
    idx_old -= self->input_vars_num;
    idx_new -= self->input_vars_num + input_vars_num;
    be_enc_move_log_block(self, idx_old, idx_new, self->input_vars_num);

    /* moves the frozen block */
    idx_old -= self->frozen_vars_num;
    idx_new -= self->frozen_vars_num + frozen_vars_num;
    be_enc_move_log_block(self, idx_old, idx_new, self->frozen_vars_num);

    /* moves the state block */
    idx_old -= self->state_vars_num;
    idx_new -= self->state_vars_num + state_vars_num;
    be_enc_move_log_block(self, idx_old, idx_new, self->state_vars_num);
  }
  /* double check */
  nusmv_assert(idx_old == be_enc_get_untimed_block_size(self) &&
               idx_new == (self->state_vars_num + state_vars_num + new_timed_frame_size));

  /* moves the untimed block of next state vars */
  idx_old = self->state_vars_num + self->frozen_vars_num + self->input_vars_num;
  idx_new = idx_old + state_vars_num + frozen_vars_num + input_vars_num;
  be_enc_move_log_block(self, idx_old, idx_new, self->state_vars_num);

  /* moves the untimed block of input vars */
  idx_old = self->state_vars_num + self->frozen_vars_num;
  idx_new = idx_old + state_vars_num + frozen_vars_num;
  be_enc_move_log_block(self, idx_old, idx_new, self->input_vars_num);

  /* moves the untimed block of frozen vars */
  idx_old = self->state_vars_num;
  idx_new = idx_old + state_vars_num;
  be_enc_move_log_block(self, idx_old, idx_new, self->frozen_vars_num);

  /* the untimed  block of current state vars stays right where it is */
}


/**Function********************************************************************

Synopsis [Moves forward of backward a block of logical indices
within the logical level. The movement is carried out of a certain
block size.]

Description [src_idx and dest_idx provides the left starting
position and the destination position respectively. Source and
destination blocks can overlap. This method is a service of
be_enc_allocate_new_log_space ]

SideEffects []

SeeAlso     []

******************************************************************************/
static void
be_enc_move_log_block(BeEnc_ptr self, int src_idx, int dest_idx,
                      size_t block_size)
{
  int from_idx = src_idx;
  int to_idx = dest_idx;
  int count = block_size;
  int inc = 1;

  if (dest_idx > src_idx) {
    /* forward moving, adjusts the parameters */
    from_idx += block_size - 1;
    to_idx += block_size - 1;
    inc = -1;
  }

  /* verbose message */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 5) && (count > 0)) {
    fprintf(nusmv_stderr,
            "BeEnc: moving a log block of %d indices from %d to %d\n",
            count, src_idx, dest_idx);
  }

  while (count > 0) {
    int phy = be_enc_index_log_to_phy(self, from_idx);

    self->log2phy[to_idx] = phy;
    self->phy2log[phy] = to_idx;

    if (MAX(from_idx, to_idx) < self->index2name_size) {
      self->index2name[to_idx] = self->index2name[from_idx];
    }

    from_idx += inc;
    to_idx += inc;
    --count;
  }
}


/**Function********************************************************************

Synopsis    [Extends the memory dedicated to the timed variables, in order
to host at least maxtime times. Then instantiates the variables and
sets up the physical<->logical conversion tables for all the necessary
timed blocks. ]

Description [The capacity might be chosen to be greater than the actual
needed size]

SideEffects []

SeeAlso     []

******************************************************************************/
static void be_enc_extend_timed_blocks(BeEnc_ptr self, int maxtime)
{
  int times;
  times = maxtime - self->max_allocated_time;

  if (times > 0) {
    int timed_block_size = be_enc_get_timed_block_size(self);
    int idx, end_new_idx;

    /* Calculates how many new vars are necessary to create, and
       expands the memory */
    be_enc_allocate_space_for_new_vars(self, timed_block_size * times);

    /* calculate the first and last newly logical indexes */
    idx = be_enc_get_untimed_block_size(self)
      + timed_block_size * (self->max_allocated_time + 1);
    end_new_idx = idx + timed_block_size * times; /* an index AFTER last new index */

    for ( ; idx < end_new_idx; ++idx) {
      int idx_untimed = be_enc_index_log_timed_to_untimed(self, idx);
      if (be_enc_is_log_index_untimed_frozen(self, idx_untimed)) {
        /* for frozen vars associate new logical index to the old physical one */
        self->log2phy[idx] = self->log2phy[idx_untimed];
      }
      else {
        /* for state and input vars create new physical indexes.
           Timed vars are unnamed => NULL is passed */
        be_enc_create_be_var(self, idx, (node_ptr) NULL);
      }
    } /* for */

    self->max_allocated_time = maxtime;
  }
}


/**Function********************************************************************

Synopsis [Extends internal blocks of memory (those who keep the be
variables and the logical/physical level conversions).  After this
method is called, it is assured that there is enough space to hold
vars_num new variables, so vars_num is a delta wrt the currently
existing vars. ]

Description [This method allocates only the necessary space. It is a
task of the caller to actually create the new variables. Use the method
be_enc_create_be_var to actually create the variables.]

SideEffects []

SeeAlso     [be_enc_create_be_var]

******************************************************************************/
static void be_enc_allocate_space_for_new_vars(BeEnc_ptr self, int vars_num)
{
  int num_idx_required;

  /* check if new physical indexes are required */
  num_idx_required = vars_num + self->max_used_phy_idx + 1
    - NodeList_get_length(self->avail_phy_idx_queue);

  if (num_idx_required > self->phy_idx_capacity) {
    /* Parameter grow_excess helps to optimize memory allocation:
       allocate more memory at a given time to avoid next memory allocation.
    */
    self->grow_excess = (self->grow_excess + vars_num) / 2;
    num_idx_required += self->grow_excess;

    Be_RbcManager_Reserve(self->be_mgr, num_idx_required);
    self->phy2log = REALLOC(int, self->phy2log, num_idx_required);
    nusmv_assert(self->phy2log != (int*) NULL);

    self->phy_idx_capacity = num_idx_required;
  }

  /* check if new logical indexes are required */
  num_idx_required = vars_num + be_enc_get_untimed_block_size(self) +
    be_enc_get_timed_block_size(self) * (self->max_allocated_time + 1);

  if (num_idx_required > self->log_idx_capacity) {
    /* Parameter grow_excess helps to optimize memory allocation:
       allocate more memory at a given time to avoid next memory allocation.
    */
    self->grow_excess = (self->grow_excess + vars_num) / 2;
    num_idx_required += self->grow_excess;

    self->log2phy = REALLOC(int, self->log2phy, num_idx_required);
    nusmv_assert(self->log2phy != (int*) NULL);

    self->log_idx_capacity = num_idx_required;
  }
}


/**Function********************************************************************

Synopsis    [Allocates a new be variable at the given logical index]

Description [Before calling this method make sure there exists
enough space to hold the new variable (see the method
be_enc_allocate_space_for_new_vars). If name is not NULL, the newly
created var will be associated to name]

SideEffects []

SeeAlso     [be_enc_allocate_space_for_new_vars]

******************************************************************************/
static void
be_enc_create_be_var(BeEnc_ptr self, int logical_idx, node_ptr name)
{
  int idx;
  be_ptr var;

  /* allocates a new var, or reuse one when possible */
  idx = be_enc_get_next_avail_phy_index(self);
  var = Be_Index2Var(self->be_mgr, idx);
  nusmv_assert(var != (be_ptr) NULL);

  /* verbose message */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    if (name != (node_ptr) NULL) {
      fprintf(nusmv_stderr, "BeEnc: creating untimed variable '");
      print_node(nusmv_stderr, name);
      fprintf(nusmv_stderr, "' at log index %d, phy index %d\n",
              logical_idx, idx);
    }
    else {
      fprintf(nusmv_stderr,
              "BeEnc: creating timed variable at log index %d, phy index %d\n",
              logical_idx, idx);
    }
  }

  /* adjusts arrays */
  self->log2phy[logical_idx] = idx;
  self->phy2log[idx] = logical_idx;

  /* updates the name <-> var maps: */
  if (name != (node_ptr) NULL) {
    insert_assoc(self->name2be, name, (node_ptr) var);
    if (logical_idx < self->index2name_size) {
      self->index2name[logical_idx] = name;
    }
  }

}


/**Function********************************************************************

Synopsis [Converts an untimed logical index to a timed logical index
instantiated at given time]

Description [WARNING: If the given index corresponds to an untimed
next state var, the returned index will correspond to the
corresponding current state variable instantitated at time+1.

Frozen variables are shifted the same way as state variables with the
exception that physical indexes of a timed frozen variable and the
corresponding untimed one are the same.]

SideEffects []

SeeAlso     []

******************************************************************************/
static int be_enc_index_log_untimed_to_timed(const BeEnc_ptr self,
                                             int log_idx, int time)
{
  nusmv_assert(log_idx >= 0 && log_idx < be_enc_get_untimed_block_size(self));
  nusmv_assert(time >=0 && time <= self->max_allocated_time);

  return log_idx + be_enc_get_untimed_block_size(self)
    + be_enc_get_timed_block_size(self) * time;
}


/**Function********************************************************************

Synopsis [Converts a timed logical index to an untimed logical index]

Description [The returned index will be an untimed current state index,
an untimed frozen index or an untimed input index]

SideEffects []

SeeAlso     []

******************************************************************************/
static int be_enc_index_log_timed_to_untimed(const BeEnc_ptr self, int log_idx)
{
  int res;

  if (be_enc_is_log_index_untimed_next_state(self, log_idx)) {
    res = be_enc_index_log_next_to_curr(self, log_idx);
  }
  else if (be_enc_is_log_index_untimed_curr_state_frozen_input(self, log_idx)) {
    res = log_idx;
  }
  else {
    res = (log_idx - self->state_vars_num) %
      (self->state_vars_num + self->frozen_vars_num + self->input_vars_num);
  }

  return res;
}


/**Function********************************************************************

Synopsis    [Returns the time at which given logical index is allocated]

Description [Value BE_CURRENT_UNTIMED is returned for untimed indices]

SideEffects []

SeeAlso     []

******************************************************************************/
static int be_enc_index_log_to_time(const BeEnc_ptr self, int log_idx)
{
  if (be_enc_is_log_index_untimed (self, log_idx)) {
    return BE_CURRENT_UNTIMED;
  }

  nusmv_assert(be_enc_get_timed_block_size(self) >= 0);
  nusmv_assert(be_enc_is_log_index_timed(self, log_idx));

  return (log_idx - be_enc_get_untimed_block_size(self)) /
    be_enc_get_timed_block_size(self);
}


/**Function********************************************************************

Synopsis [Converts a current state untimed logical index to a next
state untimed logical index]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static int be_enc_index_log_curr_to_next(const BeEnc_ptr self, int log_idx)
{
  int res;

  nusmv_assert(be_enc_is_log_index_untimed_curr_state(self, log_idx));
  res = log_idx + self->state_vars_num + self->frozen_vars_num
    + self->input_vars_num;
  nusmv_assert(be_enc_is_log_index_untimed_next_state(self, res));

  return res;
}


/**Function********************************************************************

Synopsis [Converts a next state untimed logical index to a curr
state untimed logical index]

Description []

SideEffects []

SeeAlso     []

******************************************************************************/
static int be_enc_index_log_next_to_curr(const BeEnc_ptr self, int log_idx)
{
  int res;

  nusmv_assert(be_enc_is_log_index_untimed_next_state(self, log_idx));
  res = log_idx - self->state_vars_num - self->frozen_vars_num - self->input_vars_num;
  nusmv_assert(be_enc_is_log_index_untimed_curr_state(self, res));

  return res;
}


/**Function********************************************************************

Synopsis    [Returns the first available physical index for a BE variable]

Description [Takes the index from the removed variables list, or
from the maximum allocated index if the list is empty. If the index
is taken from the list of removed vars, the index is also removed
from the list before returning it.]

SideEffects []

SeeAlso     []

******************************************************************************/
static int be_enc_get_next_avail_phy_index(BeEnc_ptr self)
{
  int res;

  if (NodeList_get_length(self->avail_phy_idx_queue) > 0) {
    res = NODE_TO_INT(NodeList_remove_elem_at(self->avail_phy_idx_queue,
                          NodeList_get_first_iter(self->avail_phy_idx_queue)));
  }
  else {
    nusmv_assert(self->max_used_phy_idx < self->phy_idx_capacity);
    res = ++(self->max_used_phy_idx);
  }

  return res;
}


/**Function********************************************************************

Synopsis    [Reallocates the substitution array if needed]

Description [The substitution array is used when a substitution operation
is required to be performed on a BE expr. The array is allocated on demand
when necessary, i.e. when the size of the array changed wrt the previous
allocation.]

SideEffects []

SeeAlso     []

******************************************************************************/
static void be_enc_realloc_subst_array(BeEnc_ptr self)
{
  int untimed_block_size = be_enc_get_untimed_block_size(self);

  if (self->subst_array_size < untimed_block_size) {
    self->subst_array = REALLOC(int, self->subst_array, untimed_block_size);
    nusmv_assert(self->subst_array != (int*) NULL);
    self->subst_array_size = untimed_block_size;
  }
}


/**Function********************************************************************

Synopsis           [Shifts the given expression exp by shifting
at logical level all the variables in exp at the given time.]

Description        [This is an internal (private) service of
the BE encoding. Memoizing associates the given exp with the given time.
time can be >= 0 for timed shifting, or can be a constant of the family
BE_ENC_*_TIME for untimed shifting.

Note: the expression has to contain only untimed vars otherwise the
behaviour is undefined.]

SideEffects        []

SeeAlso            [be_enc_shift_exp_at_times]

******************************************************************************/
static be_ptr
be_enc_shift_exp_at_time(BeEnc_ptr self, const be_ptr exp, int time)
{
  be_enc_shift_memoize_key key;
  be_ptr result;

  BE_ENC_CHECK_INSTANCE(self);

  /* creates the hash key, then searches for it */
  result = (be_ptr) NULL;
  key.be = exp;
  key.c_time = time;
  key.f_time = time;
  key.i_time = time;
  key.n_time = time+1; /* next is to be brought to further step */

  if ( !st_lookup(self->shift_hash, (char*) &key, (char**) &result) ) {
    /* Duplicates the key. This ALLOC will be undone by the BeEnc destroyer */
    int delta;
    be_enc_shift_memoize_key* key_copy = ALLOC(be_enc_shift_memoize_key, 1);
    nusmv_assert(key_copy != (be_enc_shift_memoize_key*) NULL);

    key_copy->be = key.be;
    key_copy->c_time = key.c_time;
    key_copy->f_time = key.f_time;
    key_copy->i_time = key.i_time;
    key_copy->n_time = key.n_time;

    if (!Be_IsConstant(self->be_mgr, exp)) {
      delta = be_enc_index_log_untimed_to_timed(self, 0, time);
      result =  Be_LogicalShiftVar(self->be_mgr, exp, delta,
                                   self->log2phy, self->phy2log);
    }
    else result = exp;

    st_insert(self->shift_hash, (char*) key_copy, (char*) result );
  }

  return result;
}


/**Function********************************************************************

Synopsis           [Shifts the given expression exp by shifting
the variables in exp by the given times at logical level.]

Description        [An absolute time is given for every type:
current state, frozen, input and next state variables, respectively.
This is an internal (private) service of the BE encoding.

Note: the function implements memoizing. Memoizing assocates the given
exp with the given times.

Note: This function is less efficient than be_enc_shift_exp_at_time
because here an array of the size of the untimed block is created and
initialized. There are no other slowdowns.

Note: the expression have to contain only untimed vars otherwise the
behaviour is undefined.
]

SideEffects        []

SeeAlso            [be_enc_shift_exp_at_time]

******************************************************************************/
static be_ptr
be_enc_shift_exp_at_times(BeEnc_ptr self, const be_ptr exp,
                          int c_time, int f_time, int i_time, int n_time)
{
  be_enc_shift_memoize_key key;
  be_ptr result;

  BE_ENC_CHECK_INSTANCE(self);

  /* creates the hash key, then searches for it */
  result = (be_ptr) NULL;
  key.be = exp;
  key.c_time = c_time;
  key.f_time = f_time;
  key.i_time = i_time;
  key.n_time = n_time;

  if ( !st_lookup(self->shift_hash, (char*) &key, (char**) &result) ) {
    /* Duplicates the key. This ALLOC will be undone by the BeEnc destroyer */
    be_enc_shift_memoize_key* key_copy = ALLOC(be_enc_shift_memoize_key, 1);
    nusmv_assert(key_copy != (be_enc_shift_memoize_key*) NULL);

    key_copy->be = key.be;
    key_copy->c_time = key.c_time;
    key_copy->f_time = key.f_time;
    key_copy->i_time = key.i_time;
    key_copy->n_time = key.n_time;

    /* lazy evaluation */
    if (Be_IsConstant(self->be_mgr, exp)) result = exp;
    else {
      int c_delta = 0;
      int f_delta = 0;
      int i_delta = 0;
      int n_delta = 0;
      int idx, stopper;

      /* allocates the substitution array if needed */
      be_enc_realloc_subst_array(self);

      /* calculate delta for current */
      if (c_time >= 0) c_delta = be_enc_index_log_untimed_to_timed(self, 0, c_time);
      else switch (c_time) {
      case BE_ENC_INVALID_TIME: break; /* handled later */
      case BE_ENC_NO_TIME: case BE_ENC_CURRENT_UNTIMED_TIME: c_delta = 0; break;
      case BE_ENC_NEXT_UNTIMED_TIME:
        c_delta = be_enc_index_log_curr_to_next(self, 0); break;
      default: error_unreachable_code(); /* no other possible values */
      }

      /* calculate delta for frozen */
      if (f_time >= 0) f_delta = be_enc_index_log_untimed_to_timed(self, 0, f_time);
      else switch (f_time) {
      case BE_ENC_INVALID_TIME: break; /* handled later */
      case BE_ENC_NO_TIME: f_delta = 0; break;
      case BE_ENC_CURRENT_UNTIMED_TIME:
      case BE_ENC_NEXT_UNTIMED_TIME: error_unreachable_code(); /* wrong for frozen */
      default: error_unreachable_code(); /* no other possible values */
      }

      /* calculate delta for input */
      if (i_time >= 0) i_delta = be_enc_index_log_untimed_to_timed(self, 0, i_time);
      else switch (i_time) {
      case BE_ENC_INVALID_TIME: break; /* handled later */
      case BE_ENC_NO_TIME: i_delta = 0; break;
      case BE_ENC_CURRENT_UNTIMED_TIME:
      case BE_ENC_NEXT_UNTIMED_TIME: error_unreachable_code(); /* wrong  for input */
      default: error_unreachable_code(); /* no other possible values */
      }

      /* calculate delta for next */
      if (n_time >= 0) {
        /* delta in this case is calculated relative to the first
           logical next untimed variable:

           |  UT   |  0  |  1  |  2   ...
           .-------.-----.-----.----- ...
           v       v     v     v
           |C|F|I|N|C|F|I|C|F|I|C|F|I|....

                 ^       ^
                 |-------|
               delta to time 1
        */
        n_delta = be_enc_index_log_untimed_to_timed(self, 0, n_time) -
          be_enc_index_log_curr_to_next(self, 0);
      }
      else switch (n_time) {
      case BE_ENC_INVALID_TIME: break; /* handled later */
      case BE_ENC_NO_TIME: case BE_ENC_NEXT_UNTIMED_TIME: n_delta = 0; break;
      case BE_ENC_CURRENT_UNTIMED_TIME:
        n_delta = -be_enc_index_log_curr_to_next(self, 0); break;
      default: error_unreachable_code(); /* no other possible values */
      }

      /* prepares the substitution array content */
      idx = 0;
      for (stopper = self->state_vars_num; idx < stopper; ++idx) {/*current*/
        if (BE_ENC_INVALID_TIME == c_time) {
          self->subst_array[idx] = BE_INVALID_SUBST_VALUE;
        }
        else self->subst_array[idx] = idx + c_delta;
      }
      for (stopper += self->frozen_vars_num; idx < stopper; ++idx) {/*frozen*/
        if (BE_ENC_INVALID_TIME == f_time) {
          self->subst_array[idx] = BE_INVALID_SUBST_VALUE;
        }
        else self->subst_array[idx] = idx + f_delta;
      }
      for (stopper += self->input_vars_num; idx < stopper; ++idx) {/*input*/
        if (BE_ENC_INVALID_TIME == i_time) {
          self->subst_array[idx] = BE_INVALID_SUBST_VALUE;
        }
        else self->subst_array[idx] = idx + i_delta;
      }
      for (stopper += self->state_vars_num; idx < stopper; ++idx) {/*next*/
        if (BE_ENC_INVALID_TIME == n_time) {
          self->subst_array[idx] = BE_INVALID_SUBST_VALUE;
        }
        else self->subst_array[idx] = idx + n_delta;
      }

      /* performs the substitution */
      result = Be_LogicalVarSubst(self->be_mgr, exp, self->subst_array,
                                  self->log2phy, self->phy2log);
    } /* if */

    /* memoize */
    st_insert(self->shift_hash, (char*) key_copy, (char*) result );
  }

  return result;
}


/**Function********************************************************************

Synopsis           [Empties the content of the shifting cache]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
static void be_enc_clean_shift_hash(BeEnc_ptr self)
{
  nusmv_assert(self->shift_hash != (st_table*) NULL);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr,
            "BeEnc: Cleaning up the shifting memoizing cache.\n");
  }

  st_foreach(self->shift_hash, &be_enc_shift_hash_callback_del_entry_and_key,
             NULL /*unused*/);
}


/* ------------------------------------------------------------------------  */
/* Shift memoizing internal functions:                                       */

static int be_enc_shift_hash_key_cmp(const char* _key1, const char* _key2)
{
  const be_enc_shift_memoize_key* key1 = (be_enc_shift_memoize_key*) _key1;
  const be_enc_shift_memoize_key* key2 = (be_enc_shift_memoize_key*) _key2;

  return (key1->be != key2->be)
    || (key1->c_time != key2->c_time) || (key1->f_time != key2->f_time)
    || (key1->i_time != key2->i_time) || (key1->n_time != key2->n_time);
}

static int be_enc_shift_hash_key_hash(char* _key, const int size)
{
  be_enc_shift_memoize_key* key = (be_enc_shift_memoize_key*) _key;
  return (int)
    (((((((((((nusmv_ptruint) key->be << 2)
             ^ key->c_time) << 2)
           ^ key->f_time) << 2)
         ^ key->i_time) << 2)
       ^ key->n_time) << 2)
     % size);
}

static enum st_retval
be_enc_shift_hash_callback_del_entry_and_key(char* key, char* record, char* dummy)
{
  FREE(key); /* removes allocated key for this entry */
  return ST_DELETE; /* removes associated element */
}

/**AutomaticEnd***************************************************************/
