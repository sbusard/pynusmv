/**CFile***********************************************************************

  FileName    [simulateTransSet.c]

  PackageName [simulate]

  Synopsis    [Module that exports a class representing a transitions set]

  Description []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``simulate'' package of NuSMV version 2.
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


#include "simulateTransSet.h"
#include "simulateInt.h"

#include "compile/symb_table/SymbTable.h"
#include "utils/utils_io.h"
#include "trace/pkg_trace.h"

static char rcsid[] UTIL_UNUSED = "$Id: simulateTransSet.c,v 1.1.2.8.4.9.4.5 2009-09-17 11:49:52 nusmv Exp $";



/**Struct**********************************************************************

  Synopsis [Holds information about states and associated
  transitions. Used during interactive simulation only.]

  Description [During simulation several actions can be taken, with
  associated states. This structure holds a set of future states the
  simulation can go in, and for each future state it holds a set of
  associated inputs. This structure is privately used by the function
  that must take a choice about the next state/input pair during
  simulation]

******************************************************************************/
typedef struct SimulateTransSet_TAG {
  SymbTable_ptr symb_table;
  BddEnc_ptr enc;
  DdManager* dd;

  bdd_ptr from_state;

  int next_states_num;
  bdd_ptr* next_states_array;

  int* inputs_num_per_state;
  bdd_ptr** inputs_per_state;

} SimulateTransSet;



/**Function********************************************************************

  Synopsis          [Class constructor]

  Description [from_state can be NULL when the set of initial states
  must be queried. next_states_count is checked to be in (1,INT_MAX) ]

  SideEffects       []

  SeeAlso           []

******************************************************************************/
SimulateTransSet_ptr
SimulateTransSet_create(BddFsm_ptr fsm, BddEnc_ptr enc,
                        bdd_ptr from_state, bdd_ptr next_states_set,
                        double next_states_count)
{
  SimulateTransSet_ptr self;
  bdd_ptr inputs_mask = (bdd_ptr) NULL;
  boolean res;

  nusmv_assert(next_states_count > 0 && next_states_count <= INT_MAX);

  self = ALLOC(SimulateTransSet, 1);
  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);

  self->enc = enc;
  self->dd = BddEnc_get_dd_manager(enc);
  self->symb_table = BaseEnc_get_symb_table(BASE_ENC(enc));

  self->next_states_num = (int) next_states_count;
  self->next_states_array = ALLOC(bdd_ptr, self->next_states_num);
  nusmv_assert(self->next_states_array != (bdd_ptr*) NULL);


  { /* counts the number of state variables only within the layers
       that have been committed to the passed encoding */
    NodeList_ptr layers;
    ListIter_ptr iter;
    boolean state_frozen_var_exist = false;

    layers = BaseEnc_get_committed_layers(BASE_ENC(enc));
    iter = NodeList_get_first_iter(layers);

    while ((!ListIter_is_end(iter)) && (!state_frozen_var_exist)) {
      SymbLayer_ptr layer = SYMB_LAYER(NodeList_get_elem_at(layers, iter));
      state_frozen_var_exist = SymbLayer_get_state_vars_num(layer) > 0
        || SymbLayer_get_frozen_vars_num(layer) > 0;
      iter = ListIter_get_next(iter);
    }

    /* next states set: */
    if (state_frozen_var_exist) {
      res = BddEnc_pick_all_terms_states(enc, next_states_set,
                                         self->next_states_array,
                                         self->next_states_num);
      nusmv_assert(!res); /* res is true if an error occurred */
    }
    else {
      /* there are no state or frozen variables at all */
      self->next_states_array[0] = bdd_true(self->dd);
    }
  }

  if (from_state != (bdd_ptr) NULL) {
    int s;
    int num_inputs;

    self->from_state = bdd_dup(from_state);

    /* inputs related fields are filled only if inputs areactually  used */
    self->inputs_num_per_state = ALLOC(int, next_states_count);
    nusmv_assert(self->inputs_num_per_state != (int*) NULL);

    self->inputs_per_state = ALLOC(bdd_ptr*, next_states_count);
    nusmv_assert(self->inputs_per_state != (bdd_ptr**) NULL);

    inputs_mask = BddEnc_get_input_vars_mask_bdd(enc);

    { /* calculates the number of input variables in the default
         class */
      array_t* layers;
      char* name;
      int i;

      num_inputs = 0;
      layers = SymbTable_get_class_layer_names(self->symb_table,
                                               (const char*) NULL);
      arrayForEachItem(char*, layers, i, name) {
        num_inputs += SymbLayer_get_input_vars_num(
                         SymbTable_get_layer(self->symb_table, name));
      }
    }

    /* calculates the set of inputs: */
    for (s=0; s < next_states_count; ++s) {
      if (num_inputs > 0) {
        bdd_ptr* array_of_inputs;
        bdd_ptr inputs_per_state;

        inputs_per_state =
          BddFsm_states_to_states_get_inputs(fsm, self->from_state,
                                             self->next_states_array[s]);

        bdd_and_accumulate(self->dd, &inputs_per_state, inputs_mask);

        self->inputs_num_per_state[s] =
          BddEnc_count_inputs_of_bdd(enc, inputs_per_state);

        array_of_inputs = ALLOC(bdd_ptr, self->inputs_num_per_state[s]);
        nusmv_assert(array_of_inputs != (bdd_ptr*) NULL);

        res = BddEnc_pick_all_terms_inputs(enc, inputs_per_state,
                                           array_of_inputs,
                                           self->inputs_num_per_state[s]);
        nusmv_assert(!res); /* res is true if an error occurred */

        self->inputs_per_state[s] = array_of_inputs;
      }
      else {
        /* there are no input variables at all */
        self->inputs_num_per_state[s] = 0;
        self->inputs_per_state[s] = (bdd_ptr*) NULL;
      }
    }

  }
  else {
    /* inputs are not used here (the initial states are being queried) */
    self->from_state = (bdd_ptr) NULL;
    self->inputs_num_per_state = (int*) NULL;
    self->inputs_per_state = (bdd_ptr**) NULL;
  }

  return self;
}


/**Function********************************************************************

  Synopsis          [Class destructor]

  Description       []

  SideEffects       []

  SeeAlso           []

******************************************************************************/
void SimulateTransSet_destroy(SimulateTransSet_ptr self)
{
  int s;

  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);

  if (self->from_state != (bdd_ptr) NULL) {
    bdd_free(self->dd, self->from_state);
  }

  for (s=0; s < self->next_states_num; ++s) {
    bdd_free(self->dd, self->next_states_array[s]);
  }
  FREE(self->next_states_array);

  if (self->inputs_num_per_state != (int*) NULL) {
    for (s=0; s < self->next_states_num; ++s) {
      int i;
      for (i = 0; i < self->inputs_num_per_state[s]; ++i) {
        if (self->inputs_per_state[s][i] != (bdd_ptr) NULL) {
          bdd_free(self->dd, self->inputs_per_state[s][i]);
        }
      }
      if (self->inputs_per_state != (bdd_ptr**) NULL) {
        FREE(self->inputs_per_state[s]);
      }
    }
    FREE(self->inputs_num_per_state);
  }
}


/**Function********************************************************************

  Synopsis [Getter for the state the transition set is originating from]

  Description       [Returned BDD is referenced. NULL can be returned if
  this transition set target states are the initial states set]

  SideEffects       []

  SeeAlso           []

******************************************************************************/
bdd_ptr SimulateTransSet_get_from_state(const SimulateTransSet_ptr self)
{
  bdd_ptr res;

  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);

  if (self->from_state != (bdd_ptr) NULL) {
    res = bdd_dup(self->from_state);
  }
  else res = (bdd_ptr) NULL;

  return res;
}


/**Function********************************************************************

  Synopsis          [Returns the cardinality of the target set of states]

  Description       []

  SideEffects       []

  SeeAlso           []

******************************************************************************/
int SimulateTransSet_get_next_state_num(const SimulateTransSet_ptr self)
{
  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);

  return self->next_states_num;
}


/**Function********************************************************************

  Synopsis          [Returns the Nth element of the target set of states]

  Description       [Returned BDD is referenced]

  SideEffects       []

  SeeAlso           []

******************************************************************************/
bdd_ptr SimulateTransSet_get_next_state(const SimulateTransSet_ptr self,
                                        int state_index)
{
  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);
  nusmv_assert((state_index >= 0) && (state_index < self->next_states_num));

  return bdd_dup(self->next_states_array[state_index]);
}


/**Function********************************************************************

  Synopsis          [Returns the cardinality of the inputs set going to
  a given state, represented by its index in the set of target states]

  Description       [Returned BDD is referenced. NULL can be returned
  if self represent the initial states set]

  SideEffects       []

  SeeAlso           []

******************************************************************************/
int SimulateTransSet_get_inputs_num_at_state(const SimulateTransSet_ptr self,
                                             int state_index)
{
  int res = 0;

  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);
  nusmv_assert((state_index >= 0) && (state_index < self->next_states_num));

  if (self->inputs_num_per_state != (int*) NULL) {
    res = self->inputs_num_per_state[state_index];
  }

  return res;
}


/**Function********************************************************************

  Synopsis          [Returns the Ith input from the set of inputs
  going to the Nth state in the set of target states]

  Description       []

  SideEffects       []

  SeeAlso           []

******************************************************************************/
bdd_ptr SimulateTransSet_get_input_at_state(const SimulateTransSet_ptr self,
                                            int state_index, int input_index)
{
  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);
  nusmv_assert((state_index >= 0) && (state_index < self->next_states_num));
  nusmv_assert( (input_index >= 0) &&
                (input_index <
                 SimulateTransSet_get_inputs_num_at_state(self,
                                                          state_index)) );

  return bdd_dup(self->inputs_per_state[state_index][input_index]);
}


/**Function********************************************************************

  Synopsis          []

  Description [Index is the number corresponding to the index the user
  chose after having seen the result of the print method. state and
  input will contain referenced bdds representing the chose
  state-input pair, but input might be NULL for the initial state]

  SideEffects       []

  SeeAlso           []

******************************************************************************/
void SimulateTransSet_get_state_input_at(const SimulateTransSet_ptr self,
                                         int index,
                                         bdd_ptr* state, bdd_ptr* input)
{
  int count;
  int s, states_num;

  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);

  *state = NULL;
  *input = NULL;

  states_num = SimulateTransSet_get_next_state_num(self);
  count = 0;
  for (s=0; s < states_num; ++s) {
    int inputs_num;

    inputs_num = SimulateTransSet_get_inputs_num_at_state(self, s);

    if (inputs_num > 0) {
      if (index < count + inputs_num) {
        /* found the state and the input: */
        *state = SimulateTransSet_get_next_state(self, s);
        *input = SimulateTransSet_get_input_at_state(self, s, index - count);
        break;
      }
      count += inputs_num;
    }
    else {
      if (index == count) {
        *state = SimulateTransSet_get_next_state(self, s);
        *input = (bdd_ptr) NULL;
        break;
      }
      count += 1;
    }
  } /* for each state */
}


/**Function********************************************************************

  Synopsis          []

  Description       []

  SideEffects       []

  SeeAlso           []

******************************************************************************/
void SimulateTransSet_get_state_input_rand(const SimulateTransSet_ptr self,
                                           bdd_ptr* state, bdd_ptr* input)
{
  int s;

  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);

  *state = NULL;
  *input = NULL;

  s = utils_random() % SimulateTransSet_get_next_state_num(self);
  *state = SimulateTransSet_get_next_state(self, s);

  if (SimulateTransSet_get_inputs_num_at_state(self, s) > 0) {
    int i;
    i = utils_random() % SimulateTransSet_get_inputs_num_at_state(self, s);

    *input = SimulateTransSet_get_input_at_state(self, s, i);
  }
}


/**Function********************************************************************

  Synopsis          []

  Description       []

  SideEffects       []

  SeeAlso           []

******************************************************************************/
void SimulateTransSet_get_state_input_det(const SimulateTransSet_ptr self,
                                          bdd_ptr* state, bdd_ptr* input)
{
  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);

  *state = NULL;
  *input = NULL;

  *state = SimulateTransSet_get_next_state(self, 0);

  if (SimulateTransSet_get_inputs_num_at_state(self, 0) > 0) {
    *input = SimulateTransSet_get_input_at_state(self, 0, 0);
  }
}


/**Function********************************************************************

  Synopsis          []

  Description       [Returned value is the maximum index that can be chosen by
  user in interactive mode]

  SideEffects       []

  SeeAlso           []

******************************************************************************/
int SimulateTransSet_print(const SimulateTransSet_ptr self,
                           boolean show_changes_only,
                           FILE* output)
{
  int states_num;
  int count;

  SIMULATE_TRANS_SET_CHECK_INSTANCE(self);

  states_num = SimulateTransSet_get_next_state_num(self);

  if (states_num > 0) {
    fprintf(output,
            "\n***************  AVAILABLE STATES  *************\n");
  }
  else {
    fprintf(output,
            "\n*******  THERE ARE NO AVAILABLE STATES  *******\n");
  }

  inc_indent_size();

  {
    array_t* ls = SymbTable_get_class_layer_names(self->symb_table,
                                                  (const char*) NULL);
    NodeList_ptr unfiltered_state_symbs =
      SymbTable_get_layers_sf_symbols(self->symb_table, ls);
    NodeList_ptr state_symbs =
      TracePkg_get_filtered_symbols(unfiltered_state_symbs);

    NodeList_ptr unfiltered_input_symbs =
      SymbTable_get_layers_i_symbols(self->symb_table, ls);
    NodeList_ptr input_symbs =
      TracePkg_get_filtered_symbols(unfiltered_input_symbs);

    int s;

    /* these are no longer needed */
    NodeList_destroy(unfiltered_input_symbs);
    NodeList_destroy(unfiltered_state_symbs);

    BddEnc_print_bdd_begin(self->enc, state_symbs, show_changes_only);

    count = 0;
    for (s=0; s < states_num; ++s) {
      int inputs_num;
      bdd_ptr state;

      inputs_num = SimulateTransSet_get_inputs_num_at_state(self, s);

      fprintf(output, "\n================= State =================\n");
      if (inputs_num == 0) {
        fprintf(output, "%d) -------------------------\n", count);
      }

      state = SimulateTransSet_get_next_state(self, s);

      BddEnc_print_bdd(self->enc, state, (VPFNNF) NULL, output);
      bdd_free(self->dd, state);

      if (inputs_num > 0) {
        int i;

        inc_indent_size();
        BddEnc_print_bdd_begin(self->enc, input_symbs, show_changes_only);

        fprintf(output, "\nThis state is reachable through:");

        for (i=0; i < inputs_num; ++i) {
          bdd_ptr input;

          input = SimulateTransSet_get_input_at_state(self, s, i);

          fprintf(output, "\n%d) -------------------------\n", count);

          BddEnc_print_bdd(self->enc, input, (VPFNNF) NULL, output);
          bdd_free(self->dd, input);

          if (i < inputs_num - 1) count += 1;
        } /* for each input */

        BddEnc_print_bdd_end(self->enc);
        dec_indent_size();
      }

      count += 1;
      fprintf(output, "\n");
    } /* for each state */

    BddEnc_print_bdd_end(self->enc);

    NodeList_destroy(input_symbs);
    NodeList_destroy(state_symbs);
  }

  dec_indent_size();
  return count - 1;
}
