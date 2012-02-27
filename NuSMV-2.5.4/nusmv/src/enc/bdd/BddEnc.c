/**CFile***********************************************************************

  FileName    [BddEnc.c]

  PackageName [enc.bdd]

  Synopsis    [Implementaion of class 'BddEnc']

  Description []

  SeeAlso     [BddEnc.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.bdd'' package of NuSMV version 2.
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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

 ******************************************************************************/

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "enc/encInt.h"
#include "BddEnc.h"
#include "BddEnc_private.h"

#include "bddInt.h"

#include "parser/symbols.h"
#include "compile/compile.h"
#include "compile/symb_table/ResolveSymbol.h"
#include "compile/symb_table/SymbTable.h"
#include "enc/operators.h"

#include "parser/ord/ParserOrd.h"

#include "utils/WordNumber.h"
#include "utils/utils.h"
#include "utils/range.h"
#include "utils/error.h"
#include "utils/ustring.h"
#include "utils/utils_io.h"
#include "utils/ucmd.h"
#include "utils/array.h"
#include "utils/defs.h"
#include "cinit/cinit.h"

#include <math.h>


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
/*    See 'BddEnc_private.h' for class 'BddEnc' definition.               */
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/**Struct**********************************************************************

   Synopsis    [Private structure used to print BDDs]

   Description [The BddEnc class provides support for printing of BDDs
   via methods print_bdd_begin, print_bdd, print_bdd_end. Since these
   calls can be nested, a stack of BddEncPrintInfo instances is used]

******************************************************************************/
typedef struct BddEncPrintInfo_TAG
{
  hash_ptr hash;
  NodeList_ptr symbols;
  boolean  changes_only;
} BddEncPrintInfo;


/* used by private callbacks */
typedef AddArray_ptr (*ADD_ARRAY_UNARY_OP)(DdManager *, AddArray_ptr);
typedef AddArray_ptr (*ADD_ARRAY_BINARY_OP)(DdManager *,
                                            AddArray_ptr, AddArray_ptr);


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define BDD_ENC_FREE_ADD(add)                   \
  if (add != (add_ptr) NULL) {                  \
    add_free(self->dd, add);                    \
    add = (add_ptr) NULL;                       \
  }

#define BDD_ENC_FREE_BDD(bdd)                   \
  if (bdd != (bdd_ptr) NULL) {                  \
    bdd_free(self->dd, bdd);                    \
    bdd = (bdd_ptr) NULL;                       \
  }


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void bdd_enc_finalize ARGS((Object_ptr object, void* dummy));


static int
bdd_enc_reserve_consecutive_vars ARGS((BddEnc_ptr self,
                                       const SymbLayer_ptr layer,
                                       size_t block_size,
                                       size_t idx_chunk_size));

static void bdd_enc_add_input_var ARGS((BddEnc_ptr self, node_ptr name,
                                        int index));

static void bdd_enc_add_state_var ARGS((BddEnc_ptr self, SymbLayer_ptr layer,
                                        node_ptr name,
                                        int curr_index, int next_index));

static void bdd_enc_add_frozen_var ARGS((BddEnc_ptr self,
                                         node_ptr name, int index));

static void
bdd_enc_add_input_var_to_minterm ARGS((BddEnc_ptr self, int index));

static void
bdd_enc_add_state_var_to_minterm ARGS((BddEnc_ptr self, int index));

static void
bdd_enc_add_next_state_var_to_minterm ARGS((BddEnc_ptr self, int index));

static void
bdd_enc_add_frozen_var_to_minterm ARGS((BddEnc_ptr self, int index));

static void
bdd_enc_accumulate_state_var_cube ARGS((BddEnc_ptr self, add_ptr curr,
                                        add_ptr next));

static void
bdd_enc_accumulate_input_var_cube ARGS((BddEnc_ptr self, add_ptr input));

static void
bdd_enc_accumulate_frozen_var_cube ARGS((BddEnc_ptr self, add_ptr frozen));

static void bdd_enc_remove_input_var ARGS((BddEnc_ptr self, node_ptr name));

static void bdd_enc_remove_state_var ARGS((BddEnc_ptr self, node_ptr name));

static void bdd_enc_remove_frozen_var ARGS((BddEnc_ptr self, node_ptr name));

static void bdd_enc_remove_var ARGS((BddEnc_ptr self, node_ptr name));

static void
bdd_enc_remove_var_from_minterm ARGS((BddEnc_ptr self, int var_idx,
                                      array_t* minterms_array,
                                      int minterms_array_len));

static node_ptr
bdd_enc_get_next_state_var_name ARGS((node_ptr name));

static int
bdd_enc_compact_minterms_array ARGS((BddEnc_ptr self,
                                     array_t* minterms_array,
                                     int minterms_array_len));

static NodeList_ptr
bdd_enc_sort_variables_and_groups ARGS((BddEnc_ptr self,
                                        SymbLayer_ptr layer,
                                        OrdGroups_ptr res_group,
                                        boolean* needs_reshuffle,
                                        boolean* interleaved));

static void bdd_enc_lock_reordering ARGS((BddEnc_ptr self));
static void bdd_enc_unlock_reordering ARGS((BddEnc_ptr self));

static void
bdd_enc_insert_gap ARGS((BddEnc_ptr self, int index, size_t gap_size));

static void
bdd_enc_remove_gap ARGS((BddEnc_ptr self, int index, size_t size));

static int bdd_enc_name_to_index ARGS((const BddEnc_ptr self, node_ptr name));

static assoc_retval hash_node_free ARGS((char* key, char* data, char* arg));

static assoc_retval hash_add_key_free ARGS((char* key, char* data, char* arg));

static add_ptr
bdd_enc_get_vars_list_mask ARGS((BddEnc_ptr self, const SymbLayer_ptr layer,
                                 SymbLayerIter* iter));

static add_ptr
bdd_enc_get_var_mask_recur ARGS((BddEnc_ptr self, add_ptr var_encoding,
                                 add_ptr cube));

static AddArray_ptr
bdd_enc_eval ARGS((BddEnc_ptr self, Expr_ptr expr, node_ptr context));

static AddArray_ptr
bdd_enc_eval_recur ARGS((BddEnc_ptr self, Expr_ptr expr, node_ptr ctx));

static AddArray_ptr
bdd_enc_eval_recur_case_atom ARGS((BddEnc_ptr self, Expr_ptr expr,
                                   node_ptr ctx));

static AddArray_ptr
bdd_enc_eval_recur_case_dot_array ARGS((BddEnc_ptr self, Expr_ptr expr,
                                        node_ptr ctx));

static boolean bdd_enc_is_bit_vector ARGS((BddEnc_ptr self,
                                           node_ptr expr, node_ptr context,
                                           boolean* isSigned));

static AddArray_ptr
bdd_enc_unary_add_op ARGS((BddEnc_ptr self, FP_A_DA op,
                           node_ptr n, node_ptr context));

static AddArray_ptr
bdd_enc_binary_add_op ARGS((BddEnc_ptr self, FP_A_DAA op,
                            node_ptr n, node_ptr context));

static AddArray_ptr
bdd_enc_unary_node_op ARGS((BddEnc_ptr self, NPFNN/*NPFCVT*/ op,
                            node_ptr n, node_ptr context));

static AddArray_ptr
bdd_enc_binary_node_op ARGS((BddEnc_ptr self, NPFNN op, node_ptr n,
                             node_ptr context));

static AddArray_ptr
bdd_enc_if_then_else_op ARGS((BddEnc_ptr self, node_ptr node,
                              node_ptr context));

static AddArray_ptr
bdd_enc_unary_vector_op ARGS((BddEnc_ptr self,
                              ADD_ARRAY_UNARY_OP op,
                              node_ptr n, node_ptr context));

static AddArray_ptr
bdd_enc_binary_vector_op ARGS((BddEnc_ptr self, ADD_ARRAY_BINARY_OP op,
                               node_ptr n, node_ptr context));

static NodeList_ptr
bdd_enc_append_bool_vars_from_layers ARGS((BddEnc_ptr self,
                                           NodeList_ptr layers,
                                           boolean* layers_interleaved));

static node_ptr
bdd_enc_add2expr_recur ARGS((BddEnc_ptr self, SymbLayer_ptr det_layer,
                             add_ptr bool_add, hash_ptr lc, boolean is_scalar));

static boolean
bdd_enc_is_index_not_allocated ARGS((const BddEnc_ptr self, int index));

static NodeList_ptr
bdd_enc_sort_variables_and_groups_according ARGS((BddEnc_ptr self,
                                                  OrdGroups_ptr old_group,
                                                  OrdGroups_ptr res_group,
                                                  boolean* interleaved));

static NodeList_ptr
bdd_enc_get_var_booleanizations ARGS((BddEnc_ptr self, Set_t vars));


static int bdd_enc_create_block ARGS((BddEnc_ptr self,
                                      SymbLayer_ptr layer,
                                      int lev_min, size_t block_size,
                                      size_t idx_chunk_size,
                                      boolean shared));

static void bdd_enc_dissolve_committed_groups ARGS((BddEnc_ptr self));

static int bdd_enc_dump_addarray_dot_davinci ARGS((BddEnc_ptr self,
                                                   AddArray_ptr addarray,
                                                   const char** labels,
                                                   FILE* outfile,
                                                   boolean use_dot));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The BddEnc class constructor]

   Description [The BddEnc class constructor. ord_groups can be NULL if
   ordering is not used. self become the owner of the given ord_groups
   instance]

   SideEffects        []

   SeeAlso            [BddEnc_destroy]

******************************************************************************/
BddEnc_ptr BddEnc_create(SymbTable_ptr symb_table,
                         BoolEnc_ptr bool_enc, VarsHandler_ptr dd_vars_hndr,
                         OrdGroups_ptr ord_groups)
{
  BddEnc_ptr self = ALLOC(BddEnc, 1);
  BDD_ENC_CHECK_INSTANCE(self);

  bdd_enc_init(self, symb_table, bool_enc, dd_vars_hndr, ord_groups);
  return self;
}


/**Function********************************************************************

   Synopsis           [The BddEnc class destructor]

   Description        [The BddEnc class destructor]

   SideEffects        []

   SeeAlso            [BddEnc_create]

******************************************************************************/
VIRTUAL void BddEnc_destroy(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

   Synopsis           [Gets the DD vars handler this encoding refers to.]

   Description        [Gets the DD vars handler this encoding refers to.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
VarsHandler_ptr BddEnc_get_dd_vars_handler(const BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  return self->dd_vars_hndr;
}


/**Function********************************************************************

   Synopsis           [Gets the DD manager this encoding refers to.]

   Description        [Gets the DD manager this encoding refers to.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
DdManager* BddEnc_get_dd_manager(const BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  return self->dd;
}


/**Function********************************************************************

   Synopsis [Returns the internally used order that was specified
   when creating the instance. Order is used when committing a layer and
   when forcing a reordering]

   Description [Can be NULL. The returned instance belongs to
   self. Do not change it if you do not know well what you are
   doing.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
OrdGroups_ptr BddEnc_get_ord_groups(const BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  return self->ord_groups;
}


/**Function********************************************************************

   Synopsis           [Gets the support of the set of state variables]

   Description        [Returned bdd is referenced, the caller must free it after
   it is no longer used. Result is cached if not previously converted from
   internal ADD representation.  Returns NULL if an error occurred.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_state_vars_cube(const BddEnc_ptr self)
{
  bdd_ptr res;

  BDD_ENC_CHECK_INSTANCE(self);

  if (self->state_vars_bdd != (bdd_ptr) NULL) {
    res = bdd_dup(self->state_vars_bdd);
  }
  else if (self->state_vars_add != (add_ptr) NULL) {
    self->state_vars_bdd = add_to_bdd(self->dd, self->state_vars_add);
    res = bdd_dup(self->state_vars_bdd);
  }
  else res = (bdd_ptr) NULL;

  return BDD_VAR_SET(res);
}


/**Function********************************************************************

   Synopsis           [Gets the support of the set of next-state variables]

   Description        [Returned bdd is referenced, the caller must free it after
   it is no longer used.  Result is cached if not previously converted from
   internal ADD representation. Returns NULL if an error occurred. ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_next_state_vars_cube(const BddEnc_ptr self)
{
  bdd_ptr res;

  BDD_ENC_CHECK_INSTANCE(self);

  if (self->next_state_vars_bdd != (bdd_ptr) NULL) {
    res = bdd_dup(self->next_state_vars_bdd);
  }
  else if (self->next_state_vars_add != (add_ptr) NULL) {
    self->next_state_vars_bdd = add_to_bdd(self->dd,
                                           self->next_state_vars_add);
    res = bdd_dup(self->next_state_vars_bdd);
  }
  else res = (bdd_ptr) NULL;

  return BDD_VAR_SET(res);
}


/**Function********************************************************************

   Synopsis           [Gets the support of the set of frozen variables]

   Description        [Returned bdd is referenced, the caller must free it after
   it is no longer used.  Result is cached if not previously converted from
   internal ADD representation. Returns NULL if an error occurred. ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_frozen_vars_cube(const BddEnc_ptr self)
{
  bdd_ptr res;

  BDD_ENC_CHECK_INSTANCE(self);

  if (self->frozen_vars_bdd != (bdd_ptr) NULL) {
    res = bdd_dup(self->frozen_vars_bdd);
  }
  else if (self->frozen_vars_add != (add_ptr) NULL) {
    self->frozen_vars_bdd = add_to_bdd(self->dd,
                                       self->frozen_vars_add);
    res = bdd_dup(self->frozen_vars_bdd);
  }
  else res = (bdd_ptr) NULL;

  return BDD_VAR_SET(res);
}


/**Function********************************************************************

   Synopsis           [Gets the support of the set of state and frozen variables]

   Description        [The result is a conjunct of BddEnc_get_state_vars_cube and
   BddEnc_get_frozen_vars_cube.

   Returned bdd is referenced, the caller must free it after
   it is no longer used.  Result is cached if not previously converted from
   internal ADD representation. Returns NULL if an error occurred. ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_state_frozen_vars_cube(const BddEnc_ptr self)
{
  bdd_ptr res;

  BDD_ENC_CHECK_INSTANCE(self);
  res = self->state_frozen_vars_bdd;

  if (res == (bdd_ptr) NULL) {
    BddVarSet_ptr state_vars = BddEnc_get_state_vars_cube(self);
    BddVarSet_ptr frozen_vars = BddEnc_get_frozen_vars_cube(self);

    res = bdd_and(self->dd, state_vars, frozen_vars);
    self->state_frozen_vars_bdd = res;

    bdd_free(self->dd, state_vars);
    bdd_free(self->dd, frozen_vars);
  }
  return BDD_VAR_SET(bdd_dup(res));
}


/**Function********************************************************************

   Synopsis           [Gets the support of the set of input variables]

   Description        [Returned bdd is referenced, the caller must free it after
   it is no longer used.  Result is cached if not previously converted from
   internal ADD representation.  Returns NULL if an error occurred. ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_input_vars_cube(const BddEnc_ptr self)
{
  bdd_ptr res;

  BDD_ENC_CHECK_INSTANCE(self);

  if (self->input_vars_bdd != (bdd_ptr) NULL) {
    res = bdd_dup(self->input_vars_bdd);
  }
  else if (self->input_vars_add != (add_ptr) NULL) {
    self->input_vars_bdd = add_to_bdd(self->dd, self->input_vars_add);
    res = bdd_dup(self->input_vars_bdd);
  }
  else res = (bdd_ptr) NULL;

  return BDD_VAR_SET(res);
}


/**Function********************************************************************

   Synopsis           [Given a layer the function produces a cube of all
   layer's variables]

   Description [vt can be a combination of VFT_CURRENT, VFT_NEXT, VFT_FROZEN,
   VFT_INPUT (see SymbFilterType for combination shortcuts). Returned
   bdd is referenced, the caller must free it after it is no longer
   used.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_layer_vars_cube(const BddEnc_ptr self,
                                         SymbLayer_ptr layer,
                                         SymbFilterType vt)
{
  SymbTable_ptr st;
  NodeList_ptr booled_vars;
  NodeList_ptr layer_list;
  BddVarSet_ptr result;
  ListIter_ptr iter;
  boolean interleaved;

  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(BaseEnc_layer_occurs(BASE_ENC(self),
                                    SymbLayer_get_name(layer)));

  st = BaseEnc_get_symb_table(BASE_ENC(self));
  layer_list = NodeList_create();
  NodeList_append(layer_list, (node_ptr)layer);
  result = bdd_true(self->dd);

  /* get all vars in the booleanized form */
  booled_vars = bdd_enc_append_bool_vars_from_layers(self, layer_list,
                                                     &interleaved);
  nusmv_assert(!interleaved); /* there is only one layer! */

  /* convert every var into BDD and add to the cube */
  for (iter = NodeList_get_first_iter(booled_vars); !ListIter_is_end(iter);
       iter = ListIter_get_next(iter)) {
    node_ptr vname = NodeList_get_elem_at(booled_vars, iter);
    bdd_ptr curr = BddEnc_expr_to_bdd(self, vname, Nil);

    if ( ((vt & VFT_INPUT) && SymbTable_is_symbol_input_var(st, vname)) ||
         ((vt & VFT_CURRENT) && SymbTable_is_symbol_state_var(st, vname)) ||
         ((vt & VFT_FROZEN) && SymbTable_is_symbol_frozen_var(st, vname))) {
      bdd_and_accumulate(self->dd, &result, curr);
    }
    if ((vt & VFT_NEXT) && SymbTable_is_symbol_state_var(st, vname)) {
      bdd_ptr next = BddEnc_state_var_to_next_state_var(self, curr);
      bdd_and_accumulate(self->dd, &result, next);
      bdd_free(self->dd, next);
    }

    bdd_free(self->dd, curr);
  }

  NodeList_destroy(layer_list);
  NodeList_destroy(booled_vars);
  return result;
}


/**Function********************************************************************

   Synopsis    [Given a set of variables the function produces a cube of
                them filtering out some of them]

   Description [vt is a filter and can be a combination of
   VFT_CURRENT, VFT_NEXT, VFT_FROZEN, VFT_INPUT (see SymbFilterType
   for combination shortcuts).
   vars has to contain variables only without NEXT or anything else.
   E.g. if vars contains state var V and vt includes VFT_CURRENT and
   VFT_NEXT then the result will contains both current and next bits
   of V.

   Returned bdd is referenced, the caller must free it after it is no
   longer used.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_vars_cube(const BddEnc_ptr self,
                                   Set_t vars,
                                   SymbFilterType vt)
{
  SymbTable_ptr st;
  NodeList_ptr booled_vars;
  BddVarSet_ptr result;
  ListIter_ptr iter;

  BDD_ENC_CHECK_INSTANCE(self);

  st = BaseEnc_get_symb_table(BASE_ENC(self));

  result = bdd_true(self->dd);

  /* get all vars in the booleanized form */
  booled_vars = bdd_enc_get_var_booleanizations(self, vars);

  /* convert every var into BDD and add to the cube */
  NODE_LIST_FOREACH(booled_vars, iter) {
    node_ptr vname;
    bdd_ptr curr;

    vname = NodeList_get_elem_at(booled_vars, iter);
    curr = BddEnc_expr_to_bdd(self, vname, Nil);

    if ( ((vt & VFT_INPUT) && SymbTable_is_symbol_input_var(st, vname)) ||
         ((vt & VFT_CURRENT) && SymbTable_is_symbol_state_var(st, vname)) ||
         ((vt & VFT_FROZEN) && SymbTable_is_symbol_frozen_var(st, vname))) {
      bdd_and_accumulate(self->dd, &result, curr);
    }
    if ((vt & VFT_NEXT) && SymbTable_is_symbol_state_var(st, vname)) {
      bdd_ptr next = BddEnc_state_var_to_next_state_var(self, curr);
      bdd_and_accumulate(self->dd, &result, next);
      bdd_free(self->dd, next);
    }

    bdd_free(self->dd, curr);
  }

  NodeList_destroy(booled_vars);
  return result;
}


/**Function********************************************************************

   Synopsis    [Given a set of variables the function produces a cube of
                all of them]

   Description [this function is similar to BddEnc_get_vars_cube
   with 2 differences:
   1. all variables in 'vars' are put into the result cube
   2. in order to compute next-state variable bits of variable V
     'vars' has to contain NEXT(V).

   Returned bdd is referenced, the caller must free it after it is no
   longer used.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddVarSet_ptr BddEnc_get_unfiltered_vars_cube(const BddEnc_ptr self, Set_t vars)
{
  BddVarSet_ptr result;
  BddVarSet_ptr normal_result;
  BddVarSet_ptr next_result;

  Set_t normal_vars = Set_MakeEmpty();
  Set_t next_vars = Set_MakeEmpty();
  Set_Iterator_t viter;

  BDD_ENC_CHECK_INSTANCE(self);

  result = bdd_true(self->dd);

  /* at first, divide vars into 2 sets, with and without NEXT */
  SET_FOREACH(vars, viter) {
    node_ptr var = Set_GetMember(vars, viter);
    if (NEXT != node_get_type(var)) {
      normal_vars = Set_AddMember(normal_vars, var);
    }
    else {
      next_vars = Set_AddMember(next_vars, car(var));
    }
  }

  /* get cube for each of sets and then merge them */
  normal_result = BddEnc_get_vars_cube(self, normal_vars,
                                       VFT_CURRENT | VFT_INPUT | VFT_FROZEN);
  next_result = BddEnc_get_vars_cube(self, next_vars, VFT_NEXT);
  result = bdd_and(self->dd, normal_result, next_result);

  /* free all temporary data */
  bdd_free(self->dd, normal_result);
  bdd_free(self->dd, next_result);

  Set_ReleaseSet(normal_vars);
  Set_ReleaseSet(next_vars);

  return result;
}


/**Function********************************************************************

   Synopsis           [Returns true if the variable is in the cube and false
   otherwise]

   Description [ Parameter name is a fully-resolved name of variable.
   The cube of this variable is subtracted from the cube given in
   'cube' parameter, and the result is compared with the original
   'cube'. If they are different then at least a part (one bit, for
   example) of the variable is in the input cube. Therefore true is
   returned.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
boolean BddEnc_is_var_in_cube(const BddEnc_ptr self,
                              node_ptr name, add_ptr cube)
{
  int width;
  AddArray_ptr add_sym;

  BDD_ENC_CHECK_INSTANCE(self);

  add_sym = bdd_enc_eval(self, name, Nil);

  /* Subtracts the cube of a var (or expression) from the given
     cube. ADD can be a list of bits, therefore subtract all the bits
     from the cube. */
  for (width = AddArray_get_size(add_sym) - 1; width >=0; --width) {
    /* every bit of a variable is subtracted from the given cube */
    add_ptr bit_cube = add_support(self->dd,
                                   AddArray_get_n(add_sym, width));
    add_ptr tmp = add_cube_diff(self->dd, cube, bit_cube);

    /* arbitrary value below -1 to exit with true*/
    if (tmp != cube) width = -10;

    add_free(self->dd, tmp);
    add_free(self->dd, bit_cube);
  }

  AddArray_destroy(self->dd, add_sym);
  return (-1 > width);
}


/**Function********************************************************************

   Synopsis           [Returns the ADD representing the expression expr, in
   the given context]

   Description         [Returned add is referenced. A NULL value of the
   provided expression (expr) corresponds to a true ADD returned as result.

   NOTE: Mostly expressions must be type checked before being
   evaluated. For example, use TypeChecker_is_expression_wellformed to
   type check generated expression. FSM should be checked with
   TypeCheckingPkg_check_constrains, and a property should be checked
   with TypeCheckingPkg_check_property]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_expr_to_add(BddEnc_ptr self, const Expr_ptr expr,
                           const node_ptr context)
{
  AddArray_ptr array;
  add_ptr add;

  BDD_ENC_CHECK_INSTANCE(self);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    inc_indent_size();
    indent_node(nusmv_stderr, "BddEnc: evaluating expression ", expr, "\n");
  }

  array = bdd_enc_eval(self, expr, context);

  add = AddArray_get_add(array);/* array must contain only one element */
  add_ref(add);
  AddArray_destroy(self->dd, array);

  /* check for absence of FAILURE node in the expression if needed */
  if (find_assoc(self->failures_hash, (node_ptr) add) == (node_ptr) NULL) {
    add_walkleaves(Utils_failure_node_check, add);
    insert_assoc(self->failures_hash, (node_ptr) add_dup(add),
                 (node_ptr) true);
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    indent_node(nusmv_stderr, "size of ", expr, " = ");
    fprintf(nusmv_stderr, "%d ADD nodes\n", add_size(self->dd, add));
    dec_indent_size();
  }

  return add;
}


/**Function********************************************************************

   Synopsis           [Returns the ADD array representing the
   expression expr, in the given context]

   Description         [Each element of the returned add array is
   referenced. A NULL value of the provided expression (expr)
   corresponds to a true ADD array returned as result.

   The returned array will belong to the invoker.

   NOTE: Mostly expressions must be type checked before being
   evaluated. For example, use TypeChecker_is_expression_wellformed to
   type check generated expression. FSM should be checked with
   TypeCheckingPkg_check_constrains, and a property should be checked
   with TypeCheckingPkg_check_property]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
AddArray_ptr BddEnc_expr_to_addarray(BddEnc_ptr self, const Expr_ptr expr,
                                     const node_ptr context)
{
  AddArray_ptr result;

  BDD_ENC_CHECK_INSTANCE(self);

  result = bdd_enc_eval(self, expr, context);

  return result;
}

/**Function********************************************************************

   Synopsis           [Returns the BDD representing the expression expr, in
   the given contex]

   Description        [Returned bdd is referenced.

   NOTE: Mostly expressions must be type checked before being
   evaluated. For example, use TypeChecker_is_expression_wellformed to
   type check generated expression. FSM should be checked with
   TypeCheckingPkg_check_constrains, and a property should be checked
   with TypeCheckingPkg_check_property]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_expr_to_bdd(BddEnc_ptr self, const Expr_ptr expr,
                           const node_ptr context)
{
  bdd_ptr res;
  add_ptr tmp;

  BDD_ENC_CHECK_INSTANCE(self);

  tmp = BddEnc_expr_to_add(self, expr, context);

  CATCH {
    res = add_to_bdd(self->dd, tmp);
  }
  FAIL {
    internal_error("BddEnc_expr_to_bdd: cannot convert non-propositional" \
                   " expression\n");
  }

  add_free(self->dd, tmp);
  return res;
}


/**Function********************************************************************

   Synopsis           [Converts a ADD into the corresponding (boolean)
   expression.]

   Description        [Takes an ADD with leaves FALSE, TRUE, or {FALSE,TRUE}.

   The case of {FALSE,TRUE} leaves is determinized if a valid layer is
   passed, otherwise it is preserved.

   Important: if a valid determinization layer is given, the layer
   cannot be alreay commited to the encoder, and will have to be
   possibly committed later if the returned expression is intended to
   be used by this encoder in terms of ADD or BDD.

   Recurs down on the structure of the ADD, and maps each non terminal
   node into an if-then-else expression, maps FALSE and TRUE terminal nodes
   into true and false expressions, and maps {FALSE,TRUE} into a newly
   introduced variable to determinize the expression.]

   SideEffects        [A new boolean variable can be declared within det_layer]

   SeeAlso            [bdd_enc_add2expr_recur]

******************************************************************************/
node_ptr BddEnc_add_to_expr(BddEnc_ptr self, const add_ptr add,
                            SymbLayer_ptr det_layer)
{
  node_ptr result;

  BDD_ENC_CHECK_INSTANCE(self);


  hash_ptr lc = st_init_table(st_ptrcmp, st_ptrhash);
  nusmv_assert(lc != (hash_ptr) NULL);

  /* If dynamic reordering is enabled, it is temporarily disabled */
  bdd_enc_lock_reordering(self);
  result = bdd_enc_add2expr_recur(self, det_layer, add, lc, false);
  bdd_enc_unlock_reordering(self);

  st_free_table(lc);
  return result;
}


/**Function********************************************************************

   Synopsis           [Converts a ADD into the corresponding (possibly scalar)
   expression.]

   Description [Takes an ADD and converts it to the corresponding
   scalar expression.

   Non deterministic leaves will be determinized only if a valid layer is passed.
   Important: if a valid determinization layer is given, the layer cannot
   be alreay commited to the encoder, and will have to be possibly committed
   later if the returned expression is intended to be used by this encoder in
   terms of ADD or BDD.

   Recurs down on the structure of the ADD, and maps each non terminal
   node into an if-then-else expression]

   SideEffects        [A new scalar variable may be declared within det_layer]

   SeeAlso            [bdd_enc_add2expr_recur]

******************************************************************************/
node_ptr BddEnc_add_to_scalar_expr(BddEnc_ptr self, const add_ptr add,
                                   SymbLayer_ptr det_layer)
{
  node_ptr result;

  BDD_ENC_CHECK_INSTANCE(self);


  hash_ptr lc = st_init_table(st_ptrcmp, st_ptrhash);
  nusmv_assert(lc != (hash_ptr) NULL);

  /* If dynamic reordering is enabled, it is temporarily disabled */
  bdd_enc_lock_reordering(self);
  result = bdd_enc_add2expr_recur(self, det_layer, add, lc, true);
  bdd_enc_unlock_reordering(self);

  st_free_table(lc);
  return result;
}


/**Function********************************************************************

   Synopsis           [Converts a BDD into the corresponding (boolean)
   expression.]

   Description        []

   SideEffects        []

   SeeAlso            [bdd_enc_add2expr_recur]

******************************************************************************/
node_ptr BddEnc_bdd_to_expr(BddEnc_ptr self, const bdd_ptr bdd)
{
  node_ptr res;
  add_ptr add;

  BDD_ENC_CHECK_INSTANCE(self);

  add = bdd_to_add(BddEnc_get_dd_manager(self), bdd);

  /* 'add' is boolean and deterministic by construction, so the
     deterministic layer can be NULL */
  res = BddEnc_add_to_expr(self, add, SYMB_LAYER(NULL));
  add_free(BddEnc_get_dd_manager(self), add);

  return res;
}



/**Function********************************************************************

   Synopsis            [Exchange next state variables for state variables, in
   terms of ADD]

   Description         [Given an ADD whose variables are STATE variables,
   returns an isomorphic ADD where NEXT-STATE
   variables have been substituted for the
   corrisponding STATE variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_state_var_to_next_state_var_add(const BddEnc_ptr self,
                                               add_ptr add)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return add_permute(self->dd, add, array_fetch_p(int, self->current2next, 0));
}


/**Function********************************************************************

   Synopsis           [Exchange state variables for next state variables in terms
   of ADD]

   Description        [Given an ADD whose variables are NEXT-STATE variables,
   returns an isomorphic ADD where STATE variables
   have been substituted for the corrisponding
   STATE variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_next_state_var_to_state_var_add(const BddEnc_ptr self,
                                               add_ptr add)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return add_permute(self->dd, add, array_fetch_p(int, self->next2current, 0));
}


/**Function********************************************************************

   Synopsis           [Exchange next state variables for state variables]

   Description        [Given a BDD whose variables are STATE variables,
   returns an isomorphic BDD where NEXT-STATE
   variables have been substituted for the
   corrisponding STATE variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_state_var_to_next_state_var(const BddEnc_ptr self, bdd_ptr bdd)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_permute(self->dd, bdd, array_fetch_p(int, self->current2next, 0));
}


/**Function********************************************************************

   Synopsis           [Exchange state variables for next state variables]

   Description        [Given a BDD whose variables are NEXT-STATE variables,
   returns an isomorphic BDD where STATE variables
   have been substituted for the corrisponding
   STATE variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_next_state_var_to_state_var(const BddEnc_ptr self, bdd_ptr bdd)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_permute(self->dd, bdd, array_fetch_p(int, self->next2current, 0));
}


/**Function********************************************************************

   Synopsis           [Call before a group of BddEnc_print_bdd calls]

   Description [This sets some fileds used by BddEnc_print_bdd.  Also
   clears the table used when printing only changed states.  After
   having called BddEnc_print_bdd, call BddEnc_print_bdd_end.  If
   <tt>changes_only</tt> is true, than only state and frozen variables which
   assume a different value from the previous printed one are printed
   out.]

   SideEffects        []

******************************************************************************/
void BddEnc_print_bdd_begin(BddEnc_ptr self, NodeList_ptr symbols,
                            boolean changes_only)
{
  BddEncPrintInfo* info;

  BDD_ENC_CHECK_INSTANCE(self);

  info = ALLOC(BddEncPrintInfo, 1);
  nusmv_assert(info != (BddEncPrintInfo*) NULL);

  info->hash = new_assoc();
  info->symbols = symbols;
  info->changes_only = changes_only;

  self->print_stack = cons( (node_ptr) info, self->print_stack);
}


/**Function********************************************************************

   Synopsis           [Must be called after each call to
   BddEnc_print_bdd_begin]

   Description        [Must be called after each call to
   BddEnc_print_bdd_begin, in order to clean up some internal structure]

   SideEffects        []

******************************************************************************/
void BddEnc_print_bdd_end(BddEnc_ptr self)
{
  BddEncPrintInfo* info;
  node_ptr curr;

  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(self->print_stack != Nil); /*print_bdd_begin previously called*/

  curr = self->print_stack;
  self->print_stack = cdr(curr);

  info = ( BddEncPrintInfo*) car(curr);
  nusmv_assert(info != (BddEncPrintInfo*) NULL);

  clear_assoc_and_free_entries(info->hash, hash_node_free);
  free_assoc(info->hash);
  FREE(info);
  free_node(curr);
}


/**Function********************************************************************

   Synopsis        [Prints the given bdd. In particular prints only the
   symbols occuring in the symbols list passed to print_bdd_begin. Individual
   assignments may be printed using a user-defined function, passed as a
   parameter]

   Description     [Before calling this method, you must call
   print_bdd_begin. Then you can call this method once or more, but
   eventually you will have to call print_bdd_end to commit.
   Returns the number of symbols actually printed]

   SideEffects     []

******************************************************************************/
int BddEnc_print_bdd(BddEnc_ptr self, bdd_ptr bdd, VPFNNF p_fun, FILE* file)
{
  BddEncPrintInfo* info;
  node_ptr iter;
  node_ptr valueList;
  int count;


  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(self->print_stack != Nil); /*print_bdd_begin previously called*/

  if (bdd_is_false(self->dd, bdd)) {
    fprintf(file, "FALSE\n");
    return 0;
  }

  info = ( BddEncPrintInfo*) car(self->print_stack);

  valueList = BddEnc_assign_symbols(self, bdd, info->symbols,
                                    false, (bdd_ptr*)NULL);

  for (count = 0, iter = valueList; iter != Nil; iter = cdr(iter)) {
    node_ptr cur_sym = car(car(iter));
    node_ptr cur_sym_value = cdr(car(iter));

    /* if required, print only symbols with changed values */
    if (info->changes_only) {
      if (cur_sym_value == find_assoc(info->hash, cur_sym)) continue;
      insert_assoc(info->hash, cur_sym, cur_sym_value);
    }

    if (p_fun == (VPFNNF) NULL) {
      /* Default printing */
      indent_node(file, "", cur_sym, " = ");
      print_node(file, cur_sym_value);
      fprintf(file, "\n");
    } else {
      /* Custom printing */
      (*p_fun)(file, cur_sym, cur_sym_value);
    }

    ++count;
  } /* while loop */

  free_list(valueList);

  return count;
}


/**Function********************************************************************

   Synopsis           [Prints a set of states. Individual assignments may be
   printed using a user-defined function, passed as a parameter]

   Description        [Note: states are represented by state and frozen variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddEnc_print_set_of_states(BddEnc_ptr self, bdd_ptr states,
                                boolean changes_only,
                                boolean show_defines,
                                VPFNNF p_fun,
                                FILE* file)
{
  bdd_ptr* array;
  int array_size, j;
  boolean res;

  NodeList_ptr committed_vars;

  BDD_ENC_CHECK_INSTANCE(self);

  array_size = BddEnc_count_states_of_bdd(self, states);
  array = ALLOC(bdd_ptr, array_size);
  nusmv_assert(array != (bdd_ptr*) NULL);

  res = BddEnc_pick_all_terms_states(self, states, array, array_size);
  nusmv_assert(!res); /* an error occurred */

  /* Retrieve the vars list from committed layers */
  {
    const array_t* layer_names;
    const char* layer_name;
    int i;

    SymbTableIter sfiter;
    SymbTable_ptr st;

    layer_names = BaseEnc_get_committed_layer_names(BASE_ENC(self));
    committed_vars = NodeList_create();
    st = BaseEnc_get_symb_table(BASE_ENC(self));

    /* Retrieve ALL state frozen symbols */
    SYMB_TABLE_FOREACH_FILTER(st, sfiter, STT_DEFINE | STT_VAR,
                              SymbTable_iter_filter_sf_symbols, NULL) {

      node_ptr symbol = SymbTable_iter_get_symbol(st, &sfiter);

      arrayForEachItem(const char*, layer_names, i, layer_name) {
        SymbLayer_ptr layer;

        layer = SymbTable_get_layer(BASE_ENC(self)->symb_table, layer_name);

        /* Only symbols in committed layers will be kept */
        if (SymbLayer_is_symbol_in_layer(layer, symbol)) {
          if (SymbTable_is_symbol_define(st, symbol)) {
            /* Append defines only if show_defines option is true */
            if (show_defines) {
              NodeList_append(committed_vars, symbol);
            }
          }
          else {
            BoolEnc_ptr bool_enc;

            nusmv_assert(SymbTable_is_symbol_var(st, symbol));

            bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self));

            /* Append everything but bits */
            if (!BoolEnc_is_var_bit(bool_enc, symbol)) {
              NodeList_append(committed_vars, symbol);
            }
          }
        }
      }
    }
  }

  BddEnc_print_bdd_begin(self, committed_vars, changes_only);

  inc_indent_size();
  for (j=0; j < array_size; ++j) {
    fprintf(file, "------- State %4.d ------\n", j+1);

    BddEnc_print_bdd(self, array[j], p_fun, file);
    bdd_free(self->dd, array[j]);
  }
  fprintf(file, "-------------------------\n");
  dec_indent_size();

  BddEnc_print_bdd_end(self);

  NodeList_destroy(committed_vars);
  FREE(array);
}


/**Function********************************************************************

   Synopsis           [Prints a set of input pairs. Individual assignments may
   be printed using a user-defined function, passed as a parameter]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddEnc_print_set_of_inputs(BddEnc_ptr self, bdd_ptr inputs,
                                boolean changes_only, VPFNNF p_fun, FILE* file)
{
  bdd_ptr* array;
  int array_size, j;
  boolean res;
  NodeList_ptr input_list;
  SymbTable_ptr st;
  SymbTableIter iter;

  BDD_ENC_CHECK_INSTANCE(self);

  st = BASE_ENC(self)->symb_table;

  array_size = BddEnc_count_inputs_of_bdd(self, inputs);
  array = ALLOC(bdd_ptr, array_size);
  nusmv_assert(array != (bdd_ptr*) NULL);

  res = BddEnc_pick_all_terms_inputs(self, inputs, array, array_size);
  nusmv_assert(!res); /* an error occurred */

  SymbTable_gen_iter(st, &iter, STT_INPUT_VAR);
  input_list = SymbTable_iter_to_list(st, iter);

  BddEnc_print_bdd_begin(self, input_list, changes_only);
  inc_indent_size();
  for (j=0; j < array_size; ++j) {
    fprintf(file, "------- Input %4.d ------\n", j+1);

    BddEnc_print_bdd(self, array[j], p_fun, file);
    bdd_free(self->dd, array[j]);
  }
  fprintf(file, "-------------------------\n");
  dec_indent_size();

  BddEnc_print_bdd_end(self);
  NodeList_destroy(input_list);
  FREE(array);
}


/**Function********************************************************************

   Synopsis           [Prints a set of state-input pairs. Individual
   assignments may be printed using a user-defined function, passed as a
   parameter]

   Description        [Note: states are represented by state and frozen variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddEnc_print_set_of_state_input_pairs (BddEnc_ptr self,
                                            bdd_ptr state_input_pairs,
                                            boolean changes_only,
                                            VPFNNF p_fun,
                                            FILE* file)
{
  bdd_ptr* array;
  int array_size, j;
  NodeList_ptr states, inputs;
  SymbTable_ptr st;
  SymbTableIter iter;
  boolean res;

  BDD_ENC_CHECK_INSTANCE(self);

  st = BASE_ENC(self)->symb_table;

  array_size = BddEnc_get_minterms_of_bdd(self, state_input_pairs);
  array = ALLOC(bdd_ptr, array_size);
  nusmv_assert(array != (bdd_ptr*) NULL);

  res = BddEnc_pick_all_terms_states_inputs(self, state_input_pairs,
                                            array, array_size);
  nusmv_assert(!res); /* an error occurred */

  SymbTable_gen_iter(st, &iter, STT_STATE_VAR | STT_FROZEN_VAR);
  states = SymbTable_iter_to_list(st, iter);

  SymbTable_gen_iter(st, &iter, STT_INPUT_VAR);
  inputs = SymbTable_iter_to_list(st, iter);

  inc_indent_size();
  BddEnc_print_bdd_begin(self, states, changes_only);
  for (j=0; j < array_size; ++j) {
    fprintf(file, "------- State-Input Pair %4.d ------\n", j+1);

    /* prints the set of states... */
    BddEnc_print_bdd(self, array[j], p_fun, file);

    /* ...and then the set of (corresponding) inputs: */
    inc_indent_size();
    BddEnc_print_bdd_begin(self, inputs, changes_only);
    BddEnc_print_bdd(self, array[j], p_fun, file);
    BddEnc_print_bdd_end(self);
    bdd_free(self->dd, array[j]);
    dec_indent_size();
  }

  fprintf(file, "-------------------------\n");
  dec_indent_size();
  BddEnc_print_bdd_end(self);

  NodeList_destroy(states);
  NodeList_destroy(inputs);
  FREE(array);
}

/**Function********************************************************************

  Synopsis           [Prints a set of models for given trans]

  Description        []

  SideEffects        [none]

******************************************************************************/
void BddEnc_print_set_of_trans_models(BddEnc_ptr self, bdd_ptr bdd_to_print,
                                      FILE *file)
{
  bdd_ptr bdd;
  NodeList_ptr vars_list;
  NodeList_ptr sv_list;
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(self));
  DdManager * dd = BddEnc_get_dd_manager(self);

  if (bdd_is_false(dd, bdd_to_print)) {
    fprintf(file, "-- The BDD is the constant 0\n");
    return;
  }

  vars_list = SymbTable_get_layers_sf_i_vars(st,
                SymbTable_get_class_layer_names(st, (const char*) NULL));

  sv_list = SymbTable_get_layers_sf_vars(st,
                SymbTable_get_class_layer_names(st, (const char*) NULL));

  { /* We append next variables */
    ListIter_ptr iter;

    NODE_LIST_FOREACH(sv_list, iter) {
      node_ptr v = (node_ptr)NodeList_get_elem_at(sv_list, iter);

      if (SymbTable_is_symbol_state_var(st, v)) {
        NodeList_append(vars_list, find_node(NEXT, v, Nil));
      }
    }
  }

  bdd = bdd_dup(bdd_to_print);

  do {
    bdd_ptr bdd_of_assigns, tmp;
    node_ptr varValueList = BddEnc_assign_symbols(self, bdd, vars_list,
                                                  true, &bdd_of_assigns);

    {
      node_ptr l;
      for (l = varValueList; Nil != l; l = cdr(l)) {
        print_node(file, car(car(l)));
        fprintf(file, " = ");
        print_node(file, cdr(car(l)));
        fprintf(file, "\t");
      }
      fprintf(file, "\n");
    }
    free_list(varValueList);

    tmp = bdd_not(dd, bdd_of_assigns);
    bdd_and_accumulate(dd, &bdd, tmp);
    bdd_free(dd, tmp);
    bdd_free(dd, bdd_of_assigns);
  } while (bdd_isnot_false(dd, bdd));

  NodeList_destroy(vars_list);
}

/**Function********************************************************************

   Synopsis           [This function is similar to
   BddEnc_print_set_of... functions except that instead of
   printing values of variables, this funtion creates a list of pairs
   var-itsValue.
   ]

   Description [
   This functions takes a BDD and a list of symbols (variables or
   defines, both can be wrapped in NEXT), and returns a list of
   (symb, symb_value) which makes BDD not false (input BDD) should not
   be false constant).  Returned list is a list of AND nodes with Nil
   at the end. Every element is a EQUAL node with symbol on the left
   and its value on the right.

   Order of symbols in the returned list is the same of provided
   symbols list. If parameter 'onlyRequiredSymbs' is true then
   symbols whose values are not constrained by provided BDD will be
   skipped. Otherwise, some legal arbitrary values for such symbols
   will be created and returned list will contain all the symbols
   from 'symbols'.

   If parameter resultAssignment is not null pointer, then it
   returns the produced assignments in the form of BDD, i.e. a
   conjunct of all generated equations "symbol = itsValue".

   The input BDD may or may not be a complete assignment. The invoker
   should free the returned list (with free_list) and returned BDD (if any).
   Note, that EQUAL nodes should not be freed/modified as created with find_node.
   ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr BddEnc_assign_symbols(BddEnc_ptr self, bdd_ptr bdd,
                               NodeList_ptr symbols,
                               boolean onlyRequiredSymbs,
                               bdd_ptr* resultAssignment)
{
  node_ptr returnList = Nil;
  add_ptr add;
  hash_ptr cache;
  ListIter_ptr iter;

  BDD_ENC_CHECK_INSTANCE(self);

  cache = new_assoc();

  /* Restrict the input BDD to only one assignment (i.e. make BDD
     a complete assignment) */
  bdd = bdd_get_one_sparse_sat(self->dd, bdd);
  add = bdd_to_add(self->dd, bdd);
  bdd_free(self->dd, bdd);

  if (resultAssignment != (bdd_ptr*)NULL) *resultAssignment = bdd_true(self->dd);

  NODE_LIST_FOREACH(symbols, iter) {
    node_ptr sym = NodeList_get_elem_at(symbols, iter);
    node_ptr sym_cleaned = NEXT == node_get_type(sym) ? car(sym) : sym;
    AddArray_ptr sym_add;
    node_ptr sym_value;
    SymbType_ptr type;

    /* obtain the type */
    if (SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, sym_cleaned)) {
      type = SymbTable_get_var_type(BASE_ENC(self)->symb_table, sym_cleaned);
    }
    else if (SymbTable_is_symbol_define(BASE_ENC(self)->symb_table, sym_cleaned)) {
      type = TypeChecker_get_expression_type(self->type_checker, sym_cleaned, Nil);
      nusmv_assert(!SymbType_is_error(type)); /* cannot be an type error */
    }
    else {
      error_unreachable_code(); /* how the type may be not known ? */
      type = SYMB_TYPE(NULL);
    }

    /* check that sym is not a non-encodable symbol (like a real,
       string or integer) */
    if (!Compile_is_expr_booleanizable(BASE_ENC(self)->symb_table,
                                       sym_cleaned,
                                       false, cache)) {
      continue; /* skip this var */
    }

    /* (if required) check that var is a part of the BDD */
    if (onlyRequiredSymbs) {
      add_ptr support = add_support(self->dd, add);
      boolean printVar = BddEnc_is_var_in_cube(self, sym, support);
      add_free(self->dd, support);
      if (!printVar) continue; /* skip this var */
    }

    sym_add = bdd_enc_eval(self, sym, Nil);

    /* if the symbol is a variable (of define) and has a Word type then
       it is necessart to construct a Word value from separate bits.
    */
    if (SymbType_is_word(type)) {
      int width = AddArray_get_size(sym_add);
      WordNumber_ptr one = WordNumber_from_integer(1, width);
      WordNumber_ptr result = WordNumber_from_integer(0, width);

      /* number of bits in ADD should be equal to number of bits in the type */
      nusmv_assert(width == SymbType_get_word_width(type) && width >0);

      /* compute the Word value from bits */
      for (--width; width >=0; --width) {
        add_ptr tmp_add = add_if_then(self->dd, add,
                                      AddArray_get_n(sym_add, width));
        node_ptr bit = add_value(self->dd, tmp_add);
        add_free(self->dd, tmp_add);

        /* the value of a bit can be 0 or 1 only */
        nusmv_assert(Expr_is_true(bit) || Expr_is_false(bit));

        /* words with width 1 cannot be shifted at all */
        if (WordNumber_get_width(result) != 1) {
          result = WordNumber_left_shift(result, 1);
        }
        /* add the bit to the result */
        if (Expr_is_true(bit)) result = WordNumber_plus(result, one);
      } /* for */

      sym_value = find_node(SymbType_is_signed_word(type)
                            ? NUMBER_SIGNED_WORD : NUMBER_UNSIGNED_WORD,
                            (node_ptr)result, Nil);
    }
    /* Else this is a symbol with non-Word type, i.e. it can have only one usual
       ADD, not array.
    */
    else {
      /* sym_add must have only one element in the array */
      add_ptr add_values = add_if_then(self->dd, add, AddArray_get_add(sym_add));
      sym_value = add_value(self->dd, add_values);
      add_free(self->dd, add_values);
    }

    sym_value = find_node(EQUAL, sym, sym_value);
    returnList = new_node(AND, sym_value, returnList);
    /* type check the created expression to allow further evaluation,
       created expression is always well-typed
    */
    TypeChecker_is_expression_wellformed(self->type_checker, sym_value, Nil);

    /* collect the obtained assignment in BDD if it is required */
    if (resultAssignment != (bdd_ptr*)NULL) {
      bdd_ptr eq = BddEnc_expr_to_bdd(self, sym_value, Nil);
      bdd_and_accumulate(self->dd, resultAssignment, eq);
      bdd_free(self->dd, eq);
    }

    AddArray_destroy(self->dd, sym_add);
  } /* for loop */

  add_free(self->dd, add);

  returnList = reverse(returnList);

  /* Free the Compile_is_expr_booleanizable cache */
  free_assoc(cache);

  return returnList;
}


/**Function********************************************************************

   Synopsis           [Prints out the symbolic names of boolean
   variables stored in a cube.]

   Description        [Given a cube of boolean BDD variables, this
   function prints out the symbolic names of the corresponding
   variables. The symbolic name of the variables to be printed out are
   listed in <tt>list_of_sym</tt>.]

   SideEffects        [None]

******************************************************************************/
void BddEnc_print_vars_in_cube(BddEnc_ptr self, bdd_ptr cube,
                               node_ptr list_of_sym,
                               FILE* file)
{
  node_ptr los;
  add_ptr add_cube;

  BDD_ENC_CHECK_INSTANCE(self);

  add_cube = bdd_to_add(self->dd, cube);

  fprintf(file, "Current VARS:\n");
  for (los = list_of_sym; los != Nil; los = cdr(los)) {
    node_ptr sym = car(los);
    boolean printVar = BddEnc_is_var_in_cube(self, sym, add_cube);
    if (printVar) indent_node(file, "", sym, " ");
  } /* while loop */

  fprintf(file, "\nNext VARS:\n");
  for (los = list_of_sym; los != Nil; los = cdr(los)) {
    node_ptr sym = bdd_enc_get_next_state_var_name(car(los));
    boolean printVar = BddEnc_is_var_in_cube(self, sym, add_cube);
    if (printVar) indent_node(file, "", sym, " ");
  } /* while loop */

  add_free(self->dd, add_cube);
  fprintf(file,"\n");
}


/**Function********************************************************************

   Synopsis    [Finds a set of prime implicants for a formula
   represented as a BDD.]

   Description [Finds the set of prime implicants of a BDD b. Each
   element of the resulting array is a prime implicant of the BDD
   b. The prime implicant is represented as a list of pairs

   (: <vname> <value>)

   The meaning is that the variable <vname> is equal to <value>,
   i.e. <vname> = <value>. A further post-process of the result can
   write it in a better way, expecially for non boolean variables,
   where several prime implicants can be combined by writing complex
   predicates instead of simple equalities.

   The list of layers can be declared for instance as:

   char ** layers = {MODEL_LAYER_NAME, "BA_ABSTRACTION", NULL};

   Where MODEL_LAYER_NAME is the name of the model layer,
   "BA_ABSTRACTION" is the name of a new user created layer.
   Very inportant is the NULL at the end to terminate the list.]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
array_t*
BddEnc_ComputePrimeImplicants(BddEnc_ptr self, const array_t* layer_names,
                              bdd_ptr formula)
{
  int i;
  bdd_ptr fbdd, bddprime, mask;
  array_t * primes, *result;
  NodeList_ptr syms, syms2;

  /* Extracting state and input symbols declared in the layers */
  syms = SymbTable_get_layers_sf_i_symbols(
                      BaseEnc_get_symb_table(BASE_ENC(self)), layer_names);
  syms2 = SymbTable_get_layers_sf_symbols(
                      BaseEnc_get_symb_table(BASE_ENC(self)), layer_names);
  NodeList_concat(syms, syms2);
  NodeList_destroy(syms2);
  syms2 = SymbTable_get_layers_i_symbols(
                      BaseEnc_get_symb_table(BASE_ENC(self)), layer_names);
  NodeList_concat(syms, syms2);
  NodeList_destroy(syms2);

  /* Applying the mask to the BDD to take care of don't care in
     variables whose type is a set of cardinality not a power of 2 */

  fbdd = bdd_dup(formula);
  mask = BddEnc_get_state_frozen_input_vars_mask_bdd(self);
  bdd_and_accumulate(self->dd, &fbdd, mask);
  bdd_free(self->dd, mask);

  primes = bdd_compute_primes(self->dd, fbdd);
  /* Freeing fbdd, no longer needed */
  bdd_free(self->dd, fbdd);

  result = array_alloc(node_ptr, 0);


  arrayForEachItem(bdd_ptr, primes, i, bddprime) {
    ListIter_ptr iter;
    node_ptr listpair = Nil;
    add_ptr addprime = bdd_to_add(self->dd, bddprime);
    add_ptr support = add_support(self->dd, addprime);
    boolean present = false;

    NODE_LIST_FOREACH(syms, iter) {
      node_ptr sym = NodeList_get_elem_at(syms, iter);
      node_ptr sym_cleaned = (NEXT == node_get_type(sym)) ? car(sym) : sym;

      present = BddEnc_is_var_in_cube(self, sym, support);
      if (true == present) {
        SymbType_ptr type;
        AddArray_ptr sym_add;
        node_ptr sym_value;

        nusmv_assert(SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, sym_cleaned));
        type = SymbTable_get_var_type(BASE_ENC(self)->symb_table, sym_cleaned);
        sym_add = bdd_enc_eval(self, sym, Nil);

        if (SymbType_is_word(type)) {
          internal_error("BddEnc_ComputePrimeImplicants: words not yet handled\n");
        }
        else {
          /* sym_add must have only one element in the array */
          add_ptr add_values = add_if_then(self->dd, addprime, AddArray_get_add(sym_add));
          sym_value = add_value(self->dd, add_values);
          add_free(self->dd, add_values);
        }
        listpair = cons(find_node(COLON, sym, sym_value), listpair);

        AddArray_destroy(self->dd, sym_add);
      }
    }
    add_free(self->dd, support);
    add_free(self->dd, addprime);
    array_insert_last(node_ptr, result, listpair);
    /* Releasing the current prime implicant, as to avoid a second
       pass to primes to free it */
    bdd_free(self->dd, bddprime);
  }

  /* Freeing must be done here. See documentation of NodeList */
  NodeList_destroy(syms);
  syms = (NodeList_ptr)NULL;

  /* releasing the memory to store the BDD prime implicants */
  array_free(primes);
  return result;
}


/**Function********************************************************************

   Synopsis           [Return the number of states of a given ADD.]

   Description        [Return the number of minterms (i.e. states)
   represented by an ADD.
   Note: states are represented by state and frozen variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
double BddEnc_count_states_of_add(const BddEnc_ptr self, add_ptr add)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return add_count_minterm(self->dd, add,
                           (self->state_vars_num / 2) + self->frozen_vars_num);
}


/**Function********************************************************************

   Synopsis           [Return the number of states of a given BDD.]

   Description        [Return the number of states represented by a BDD.
   Note: states are represented by state and frozen variables.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
double BddEnc_count_states_of_bdd(const BddEnc_ptr self, bdd_ptr bdd)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_count_minterm(self->dd, bdd,
                           (self->state_vars_num / 2) + self->frozen_vars_num);
}


/**Function********************************************************************

   Synopsis           [Return the number of inputs of a given BDD.]

   Description        [Return the number of inputs represented by a BDD.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
double BddEnc_count_inputs_of_bdd(const BddEnc_ptr self, bdd_ptr bdd)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_count_minterm(self->dd, bdd, self->input_vars_num);
}


/**Function********************************************************************

   Synopsis           [Return the number of states inputs of a given BDD.]

   Description        [Return the number of states inputs represented by a BDD.
   Note: states are represented by state and frozen variables]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
double BddEnc_count_states_inputs_of_bdd(const BddEnc_ptr self, bdd_ptr bdd)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_count_minterm(self->dd, bdd,
                           self->input_vars_num +
                           self->frozen_vars_num +
                           (self->state_vars_num / 2));
}


/**Function********************************************************************

   Synopsis           [Return the number of minterms of a given ADD.]

   Description        [Return the number of minterms
   represented by a ADD.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
double BddEnc_get_minterms_of_add(const BddEnc_ptr self, add_ptr add)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return add_count_minterm(self->dd, add,
                           self->minterm_state_frozen_input_vars_dim);
}


/**Function********************************************************************

   Synopsis           [Return the number of minterms of a given BDD.]

   Description        [Return the number of minterms
   represented by a BDD.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
double BddEnc_get_minterms_of_bdd(const BddEnc_ptr self, bdd_ptr bdd)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_count_minterm(self->dd, bdd,
                           self->minterm_state_frozen_input_vars_dim);
}


/**Function********************************************************************

   Synopsis           [Extracts a minterm from a given BDD.]

   Description        [Extracts a minterm from a given BDD. Returned
   bdd is referenced.
   Note: states are represented by state and frozen variables.]

   SideEffects        []

   SeeAlso            [bdd_pick_one_minterm]

******************************************************************************/
bdd_ptr BddEnc_pick_one_state(const BddEnc_ptr self, bdd_ptr states)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_pick_one_minterm(self->dd, states,
                              array_fetch_p(bdd_ptr, self->minterm_state_frozen_vars, 0),
                              self->minterm_state_frozen_vars_dim);
}


/**Function********************************************************************

   Synopsis           [Extracts a minterm from a given BDD.]

   Description        [Extracts a minterm from a given BDD. Returned
   bdd is referenced]

   SideEffects        []

   SeeAlso            [bdd_pick_one_minterm]

******************************************************************************/
bdd_ptr BddEnc_pick_one_input(const BddEnc_ptr self, bdd_ptr inputs)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_pick_one_minterm(self->dd, inputs,
                              array_fetch_p(bdd_ptr, self->minterm_input_vars, 0),
                              self->minterm_input_vars_dim);
}


/**Function********************************************************************

   Synopsis           [Returns the array of All Possible Minterms]

   Description        [Takes a minterm and returns an array of all its terms,
   according to internally kept vars. Notice that
   the array of the result has to be previously allocated, and its size
   must be greater or equal the number of the minterms.
   The returned array contains referenced BDDs so it is necessary to
   dereference them after their use. Returns true if an error occurred.

   Note: states are represented by state and frozen variables.
   ]

   SideEffects        [result_array will change]

   SeeAlso            [bdd_pick_all_terms]

******************************************************************************/
boolean BddEnc_pick_all_terms_states_inputs(const BddEnc_ptr self,
                                            bdd_ptr bdd,
                                            bdd_ptr* result_array,
                                            const int array_len)
{
  int res = 1;
  BDD_ENC_CHECK_INSTANCE(self);

  if (self->minterm_state_frozen_input_vars_dim > 0) {
    res = bdd_pick_all_terms(self->dd, bdd,
                             array_fetch_p(bdd_ptr, self->minterm_state_frozen_input_vars, 0),
                             self->minterm_state_frozen_input_vars_dim,
                             result_array, array_len);
  }
  return res == 1;
}


/**Function********************************************************************

   Synopsis           [Returns the array of All Possible Minterms]

   Description        [Takes a minterm and returns an array of all its terms,
   according to internally kept vars. Notice that
   the array of the result has to be previously allocated, and its size
   must be greater or equal the number of the minterms.
   The returned array contains referenced BDD so it is necessary to
   dereference them after their use. Returns true if an error occurred.

   Note: states are represented by state and frozen variables.]

   SideEffects        [result_array will change]

   SeeAlso            [bdd_pick_all_terms]

******************************************************************************/
boolean BddEnc_pick_all_terms_states(const BddEnc_ptr self, bdd_ptr bdd,
                                     bdd_ptr* result_array,
                                     const int array_len)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (self->minterm_state_frozen_vars_dim > 0) {
    int res;

    res = bdd_pick_all_terms(self->dd, bdd,
                             array_fetch_p(bdd_ptr,
                                           self->minterm_state_frozen_vars,
                                           0),
                             self->minterm_state_frozen_vars_dim,
                             result_array,
                             array_len);
    return res == 1;
  }
  else {
    int i;

    /*
      We have no varibles, therefore we reference a bdd_true for each
      state
    */
    for (i = 0; i<array_len; i++) {
      result_array[i] = bdd_true(self->dd);
    }

    return false;
  }
}


/**Function********************************************************************

   Synopsis           [Returns the array of All Possible Minterms]

   Description        [Takes a minterm and returns an array of all its terms,
   according to internally kept vars. Notice that
   the array of the result has to be previously allocated, and its size
   must be greater or equal the number of the minterms.
   The returned array contains referenced BDD so it is necessary to
   dereference them after their use. Returns true if an error occurred]

   SideEffects        [result_array will change]

   SeeAlso            [bdd_pick_all_terms]

******************************************************************************/
boolean BddEnc_pick_all_terms_inputs(const BddEnc_ptr self, bdd_ptr bdd,
                                     bdd_ptr* result_array,
                                     const int array_len)
{
  int res = 1;
  BDD_ENC_CHECK_INSTANCE(self);

  if (self->minterm_input_vars_dim > 0) {
    res = bdd_pick_all_terms(self->dd, bdd,
                             array_fetch_p(bdd_ptr, self->minterm_input_vars, 0),
                             self->minterm_input_vars_dim,
                             result_array, array_len);
  }
  return res == 1;
}


/**Function********************************************************************

   Synopsis           [Extracts a random minterm from a given BDD.]

   Description        [Extracts a random minterm from a given BDD.
   Returned bdd is referenced.

   Note: states are represented by state and frozen variables.]

   SideEffects        []

   SeeAlso            [bdd_pick_one_minterm_rand]

******************************************************************************/
bdd_ptr BddEnc_pick_one_state_rand(const BddEnc_ptr self, bdd_ptr states)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_pick_one_minterm_rand(self->dd, states,
                                   array_fetch_p(bdd_ptr, self->minterm_state_frozen_vars, 0),
                                   self->minterm_state_frozen_vars_dim);
}


/**Function********************************************************************

   Synopsis           [Extracts a random minterm from a given BDD.]

   Description        [Extracts a random minterm from a given BDD.
   Returned bdd is referenced]

   SideEffects        []

   SeeAlso            [bdd_pick_one_minterm_rand]

******************************************************************************/
bdd_ptr BddEnc_pick_one_input_rand(const BddEnc_ptr self, bdd_ptr inputs)
{
  BDD_ENC_CHECK_INSTANCE(self);

  return bdd_pick_one_minterm_rand(self->dd, inputs,
                                   array_fetch_p(bdd_ptr, self->minterm_input_vars, 0),
                                   self->minterm_input_vars_dim);
}


/**Function********************************************************************

   Synopsis           [Given a variable index, this method return the
   symbolic name of the correpsonding variable]

   Description        []

   SideEffects        [required]

   SeeAlso            [BddEnc_get_var_index_from_name]

******************************************************************************/
node_ptr BddEnc_get_var_name_from_index(const BddEnc_ptr self, int index)
{
  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert((index >= 0) && (index < array_n(self->index2name)));

  return array_fetch(node_ptr, self->index2name, index);
}


/**Function********************************************************************

   Synopsis           [Given a variable index, this method return true iff
   the given variable belongs to the encoder]

   Description        []

   SideEffects        [required]

   SeeAlso            [BddEnc_get_var_name_from_index]

******************************************************************************/
boolean BddEnc_has_var_at_index(const BddEnc_ptr self, int index)
{
  BDD_ENC_CHECK_INSTANCE(self);
  return (index >= 0) && (index < array_n(self->index2name));
}


/**Function********************************************************************

   Synopsis           [Returns the DD index of the given variable]

   Description        [The input variable should be boolean]

   SideEffects        [required]

   SeeAlso            [BddEnc_get_var_name_from_index]

******************************************************************************/
int BddEnc_get_var_index_from_name(const BddEnc_ptr self, node_ptr name)
{
  add_ptr add;
  int res;

  BDD_ENC_CHECK_INSTANCE(self);
  if (NEXT != node_get_type(name)) {
    nusmv_assert(SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, name));
  }
  else {
    nusmv_assert(SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, car(name)));
  }

  add = BddEncCache_lookup_boolean_var(self->cache, name);
  nusmv_assert(add != (add_ptr) NULL);
  /* add = BddEnc_get_symbol_add(self, name); */

  res = add_index(self->dd, add);
  add_free(self->dd, add);
  return res;
}


/**Function********************************************************************

   Synopsis           [Returns the ADD leaf corresponding to the given atom]

   Description        [Returns the ADD leaf corresponding to the given atom,
   if defined, NULL otherwise. The returned ADD - if any - is referenced.
   If the inner flag enforce_constant is set,

   Suppose to have a declaration of this kind:<br>
   <pre>
   VAR
   condition : {idle, stopped}
   </pre>
   then in the constant hash for the atom <tt>idle</tt> there is the
   corresponding leaf ADD, i.e. the ADD whose value is the symbol
   <tt>idle</tt>.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_constant_to_add(const BddEnc_ptr self, node_ptr constant)
{
  add_ptr add;

  BDD_ENC_CHECK_INSTANCE(self);

  add = BddEncCache_lookup_constant(self->cache, constant);
  nusmv_assert(add != (add_ptr) NULL);

  return add;
}


/**Function********************************************************************

   Synopsis           [Complements an ADD according to a flag.]

   Description        [Given the ADD <code>a</code>, this function returns
   the negation of ADD <code>a</code> or <code>a</code> itself according the
   value of <code>flag</code>. If <code>flag = -1</code> then returns <code>not
   a</code>, else returns <code>a</code>. It is important that the ADD is a
   FALSE/TRUE ADD (i.e. it has only FALSE or TRUE as leaf).]

   SideEffects        []

   SeeAlso            [bdd_enc_eval]

******************************************************************************/
add_ptr BddEnc_eval_sign_add(BddEnc_ptr self, add_ptr a, int flag)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (flag == -1) a = add_not(self->dd, a);
  else add_ref(a);

  return a;
}


/**Function********************************************************************

   Synopsis           [Complements a BDD according to a flag.]

   Description [Given the BDD <code>a</code>, this function returns
   the negation of BDD <code>a</code> or <code>a</code> itself
   according the value of <code>flag</code>. If <code>flag =
   -1</code> then returns <code>not a</code>, else returns
   <code>a</code>. It is important that the BDD has only FALSE or
   TRUE as leaves.

   SideEffects        []

   SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_eval_sign_bdd(BddEnc_ptr self, bdd_ptr a, int flag)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (flag == -1) a = bdd_not(self->dd, a);
  else bdd_ref(a);

  return a;
}


/**Function********************************************************************

   Synopsis           [Evaluates a number in a context.]

   Description        [Evaluate the <em>NUMBER</em> represented by <code>e</code>
   in context <code>context</code>. <em>NUMBERS</em> can be encoded in
   different ways in different processes.]

   SideEffects        []

   SeeAlso            [bdd_enc_eval]

******************************************************************************/
int BddEnc_eval_num(BddEnc_ptr self, node_ptr e, node_ptr context)
{
  node_ptr n;
  AddArray_ptr ar;
  boolean old;

  BDD_ENC_CHECK_INSTANCE(self);

  old = self->enforce_constant;
  self->enforce_constant = true;
  CATCH {
    ar = bdd_enc_eval(self, e, context);
  }
  FAIL {
    self->enforce_constant = old;
    rpterr("BddEnc_eval_num: an error occurred during evaluation.\n");
  }

  self->enforce_constant = old;

  /* only one element should be in the array */
  n = add_get_leaf(self->dd, AddArray_get_add(ar));
  AddArray_destroy(self->dd, ar);

  if (node_get_type(n) != NUMBER) {
    rpterr("BddEnc_eval_num: numeric constant required");
  }

  return NODE_TO_INT(car(n));
}


/**Function********************************************************************

   Synopsis           [Evaluates a constant expression.]

   Description        [Evaluate a constant expression. If the
   expression does not evaluate to a constant, then an internal error
   is generated. Returned add is referenced.]

   SideEffects        []

   SeeAlso            [eval eval_num]

******************************************************************************/
add_ptr BddEnc_eval_constant(BddEnc_ptr self, Expr_ptr expr, node_ptr context)
{
  AddArray_ptr evaluation;
  add_ptr result;

  boolean enforce_constant_saved;

  enforce_constant_saved = self->enforce_constant;
  self->enforce_constant = true;
  evaluation = bdd_enc_eval(self, expr, context);
  self->enforce_constant = enforce_constant_saved;
  result = AddArray_get_n(evaluation, 0);

  if (AddArray_get_size(evaluation)!=1 || add_isleaf(result) == 0){
    internal_error("BddEnc_eval_constant: Evaluating a non" \
                   " constant expression");
  }
  add_ref(result);
  AddArray_destroy(self->dd, evaluation);

  return result;
}


/**Function********************************************************************

   Synopsis           [Given a variable, define or process constant
   the corresponding ADD array is returned.]

   Description        [Given an identifier (as an expanded identifier
   <code>name</code>), this function returns the ADD array of its
   definition, or NULL if not defined. If the variable is
   of a Word type then the returned array may contain several elements (ADDs).
   For all other kinds of expressions only one element can be in the
   array.  Errors occurs if the identifier is a define which is
   circularly declared. The returned array will belong to the invoker.]

   SideEffects        []

   SeeAlso            [BddEnc_expr_to_add]

******************************************************************************/
AddArray_ptr BddEnc_get_symbol_add(BddEnc_ptr self, node_ptr name)
{
  /* NB for developers: there cannot be any ambiguity in the name 'name'
     because VAR and DEFINE cannot be declared with the same name.
     Only 'running' (processes's) constants can be here but there is special
     check that 'running' symbol is not redeclared by the user
  */

  AddArray_ptr res;

  BDD_ENC_CHECK_INSTANCE(self);

  /* the variable has already been evaluated */
  res = BddEncCache_get_evaluation(self->cache, name);
  if (res == BDD_ENC_EVALUATING) {
    /* only define may potentially have circular dependency.
       a variable cannot be already in the evaluation
    */
    nusmv_assert(SymbTable_is_symbol_define(BASE_ENC(self)->symb_table, name));
    error_circular(name);
  }

  if (res != (AddArray_ptr) NULL) return res;


  if (!SymbTable_is_symbol_declared(BASE_ENC(self)->symb_table, name)) {
    error_undefined(name);
    return ADD_ARRAY(NULL);
  }

  /* Otherwise starts the evaluation... */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    inc_indent_size();
    indent_node(nusmv_stderr, "BddEnc: evaluating ", name, "\n");
  }

  /* do we required symbol to be a constant? */
  if ( self->enforce_constant
       && !SymbTable_is_symbol_constant(BASE_ENC(self)->symb_table, name) ) {
    self->enforce_constant = false;
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) { dec_indent_size(); }
    rpterr("constant required");
  }

  /* Is it a constant? */
  if (SymbTable_is_symbol_constant(BASE_ENC(self)->symb_table, name) ) {
    res = AddArray_from_add(BddEnc_constant_to_add(self, name));
  }

  /* Is it a define? */
  else if (SymbTable_is_symbol_define(BASE_ENC(self)->symb_table, name)) {
    /* [AT] it is better to evaluate not-flattened defines. Issue #2349

       1. From efficiency point of view evaluating flattened and
       not-flattened define bodies are the same. That's because
       flattening does not "find-node" expressions. Thus the same expressions
       (but different pointers) will be evaluated in that 2 cases.

       2. Not-flattened expressions are better for debugging because
       with high verbosity NuSMV outputs expressions before/after
       translating them to BDD. For bigger model it is often
       impossible to print flattened expressions whereas original
       expressions are quite printable. */
#if 1 /* new code about issue 2349 is enabled */
    node_ptr body = SymbTable_get_define_body(BASE_ENC(self)->symb_table,name);
    node_ptr new_context = SymbTable_get_define_context(BASE_ENC(self)->symb_table,
                                                         name);
    io_atom_push(name); /* for error reporting */
    res = bdd_enc_eval(self, body, new_context);
    io_atom_pop();

#else /* this is the old code prior issue 2349 (disabled by
         default, but kept for record) */
    node_ptr def = SymbTable_get_define_flatten_body(BASE_ENC(self)->symb_table,
                                                     name);

    BddEncCache_set_evaluation(self->cache, name, BDD_ENC_EVALUATING);
    io_atom_push(name); /* for error reporting */
    res = bdd_enc_eval(self, def, Nil);
    io_atom_pop();
#endif
  } /* end of define */


  /* gets the encoding associated with possible variable */
  else if (SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, name)) {
    SymbType_ptr var_type = SymbTable_get_var_type(BASE_ENC(self)->symb_table, name);
    SymbTypeTag type = SymbType_get_tag(var_type);

    /* Skip ARRAY_TYPE variables */
    if (!(SYMB_TYPE_ARRAY == type)) {
      /* the variable is boolean => it is in the special boolean cache already */
      if (SymbTable_is_symbol_bool_var(BASE_ENC(self)->symb_table, name)) {
        add_ptr tmp = BddEncCache_lookup_boolean_var(self->cache, name);
        nusmv_assert(tmp != (add_ptr) NULL);
        res = AddArray_from_add(tmp);
      }
      /* usual (not boolean) variable => evaluate its boolean encoding */
      else {
        node_ptr def = BoolEnc_get_var_encoding(BOOL_ENC_CLIENT(self)->bool_enc,
                                                name);
        BddEncCache_set_evaluation(self->cache, name, BDD_ENC_EVALUATING);
        io_atom_push(name); /* for error reporting */
        res = bdd_enc_eval(self, def, Nil);
        io_atom_pop();
      }
    }
  } /* end of variable */

  if (res) {
    BddEncCache_set_evaluation(self->cache, name, AddArray_duplicate(res));

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
      indent_node(nusmv_stderr, "size of ", name, " = ");
      fprintf(nusmv_stderr, "%" PRIuPTR " ADD nodes\n",
              AddArray_get_add_size(res, self->dd));
      dec_indent_size();
    }
  }

  return res;
}


/**Function********************************************************************

   Synopsis           [Returns the mask (as an ADD) in terms of frozen and
   state variables]

   Description        [Returned add is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_get_state_frozen_vars_mask_add(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (self->state_frozen_vars_mask_add == (add_ptr) NULL) {
    add_ptr res;
    ListIter_ptr iter_layer;

    res = add_true(self->dd);

    iter_layer = NodeList_get_first_iter(BASE_ENC(self)->committed_layers);
    while (! ListIter_is_end(iter_layer)) {
      add_ptr mask;
      SymbLayer_ptr layer;
      SymbLayerIter iter;

      layer = SYMB_LAYER(NodeList_get_elem_at(BASE_ENC(self)->committed_layers,
                                              iter_layer));

      SymbLayer_gen_iter(layer, &iter, STT_STATE_VAR | STT_FROZEN_VAR);
      mask = bdd_enc_get_vars_list_mask(self, layer, &iter);
      nusmv_assert(mask != (add_ptr) NULL);

      add_and_accumulate(self->dd, &res, mask);
      add_free(self->dd, mask);
      iter_layer = ListIter_get_next(iter_layer);
    }

    self->state_frozen_vars_mask_add = add_dup(res);
    add_free(self->dd, res);
  }

  return add_dup(self->state_frozen_vars_mask_add);
}


/**Function********************************************************************

   Synopsis           [Returns the mask (as an ADD) in terms of input variables]

   Description        [Returned add is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_get_input_vars_mask_add(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (self->input_vars_mask_add == (add_ptr) NULL) {
    add_ptr res;
    ListIter_ptr iter_layer;

    res = add_true(self->dd);

    iter_layer = NodeList_get_first_iter(BASE_ENC(self)->committed_layers);
    while (! ListIter_is_end(iter_layer)) {
      add_ptr mask;
      SymbLayer_ptr layer;
      SymbLayerIter iter;

      layer = SYMB_LAYER(NodeList_get_elem_at(BASE_ENC(self)->committed_layers,
                                              iter_layer));

      SymbLayer_gen_iter(layer, &iter, STT_INPUT_VAR);
      mask = bdd_enc_get_vars_list_mask(self, layer, &iter);
      nusmv_assert(mask != (add_ptr) NULL);

      add_and_accumulate(self->dd, &res, mask);
      add_free(self->dd, mask);
      iter_layer = ListIter_get_next(iter_layer);
    }

    self->input_vars_mask_add = add_dup(res);
    add_free(self->dd, res);
  }

  return add_dup(self->input_vars_mask_add);
}


/**Function********************************************************************

   Synopsis           [Returns the mask (as ADD) in terms of state, frozen and
   input variables]

   Description        [Returned add is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_get_state_frozen_input_vars_mask_add(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (self->state_frozen_input_vars_mask_add == (add_ptr) NULL) {
    add_ptr inp_mask = BddEnc_get_input_vars_mask_add(self);
    add_ptr sta_mask = BddEnc_get_state_frozen_vars_mask_add(self);

    self->state_frozen_input_vars_mask_add = add_and(self->dd, inp_mask, sta_mask);
    nusmv_assert(self->state_frozen_input_vars_mask_add != (add_ptr) NULL);
    add_free(self->dd, sta_mask);
    add_free(self->dd, inp_mask);
  }

  return add_dup(self->state_frozen_input_vars_mask_add);
}


/**Function********************************************************************

   Synopsis           [Returns the mask (as BDD) in terms of frozen and state
   variables]

   Description        [Returned bdd is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_get_state_frozen_vars_mask_bdd(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (self->state_frozen_vars_mask_bdd == (bdd_ptr) NULL) {
    add_ptr mask_add;

    mask_add = BddEnc_get_state_frozen_vars_mask_add(self);
    self->state_frozen_vars_mask_bdd = add_to_bdd(self->dd, mask_add);
    add_free(self->dd, mask_add);
  }

  return bdd_dup(self->state_frozen_vars_mask_bdd);
}


/**Function********************************************************************

   Synopsis           [Returns the mask (as BDD) in terms of input variables]

   Description        [Returned bdd is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_get_input_vars_mask_bdd(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (self->input_vars_mask_bdd == (bdd_ptr) NULL) {
    add_ptr mask_add;

    mask_add = BddEnc_get_input_vars_mask_add(self);
    self->input_vars_mask_bdd = add_to_bdd(self->dd, mask_add);
    add_free(self->dd, mask_add);
  }

  return bdd_dup(self->input_vars_mask_bdd);
}


/**Function********************************************************************

   Synopsis           [Returns the mask (as BDD) in terms of frozen, state and input
   variables]

   Description        [Returned bdd is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
bdd_ptr BddEnc_get_state_frozen_input_vars_mask_bdd(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);

  if (self->state_frozen_input_vars_mask_bdd == (bdd_ptr) NULL) {
    add_ptr mask_add;

    mask_add = BddEnc_get_state_frozen_input_vars_mask_add(self);
    self->state_frozen_input_vars_mask_bdd = add_to_bdd(self->dd, mask_add);
    add_free(self->dd, mask_add);
  }

  return bdd_dup(self->state_frozen_input_vars_mask_bdd);
}


/**Function********************************************************************

   Synopsis           [Applies a mask to the given add which must contain only
   frozen and state variables]

   Description        [Returned add is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_apply_state_frozen_vars_mask_add(BddEnc_ptr self, add_ptr states)
{
  add_ptr mask, res;

  BDD_ENC_CHECK_INSTANCE(self);

  mask = BddEnc_get_state_frozen_vars_mask_add(self);
  res = add_and(self->dd, states, mask);
  add_free(self->dd, mask);

  return res;
}


/**Function********************************************************************

   Synopsis           [Applies a mask to the given add which must contain only
   input variables]

   Description [Returned add is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_apply_input_vars_mask_add(BddEnc_ptr self, add_ptr inputs)
{
  add_ptr mask, res;

  BDD_ENC_CHECK_INSTANCE(self);

  mask = BddEnc_get_input_vars_mask_add(self);
  res = add_and(self->dd, inputs, mask);
  add_free(self->dd, mask);

  return res;
}


/**Function********************************************************************

   Synopsis           [Applies a mask to the given add which must contain
   frozen, state and input variables]

   Description [Returned add is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_apply_state_frozen_input_vars_mask_add(BddEnc_ptr self,
                                                      add_ptr states_inputs)
{
  add_ptr mask, res;

  BDD_ENC_CHECK_INSTANCE(self);

  mask = BddEnc_get_state_frozen_input_vars_mask_add(self);
  res = add_and(self->dd, states_inputs, mask);
  add_free(self->dd, mask);

  return res;
}


/**Function********************************************************************

   Synopsis           [Applies a mask to the given BDD which must contain only
   frozen and state variables]

   Description [Returned bdd is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddStates BddEnc_apply_state_frozen_vars_mask_bdd(BddEnc_ptr self,
                                                  BddStates states)
{
  bdd_ptr mask;
  BddStates res;

  BDD_ENC_CHECK_INSTANCE(self);

  mask = BddEnc_get_state_frozen_vars_mask_bdd(self);
  res = BDD_STATES( bdd_and(self->dd, states, mask) );
  bdd_free(self->dd, mask);

  return res;
}


/**Function********************************************************************

   Synopsis           [Applies a mask to the given BDD which must contain only
   input variables]

   Description [Returned bdd is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddInputs BddEnc_apply_input_vars_mask_bdd(BddEnc_ptr self, BddInputs inputs)
{
  bdd_ptr mask;
  BddInputs res;

  BDD_ENC_CHECK_INSTANCE(self);

  mask = BddEnc_get_input_vars_mask_bdd(self);
  res = BDD_INPUTS( bdd_and(self->dd, inputs, mask) );
  bdd_free(self->dd, mask);

  return res;
}


/**Function********************************************************************

   Synopsis           [Applies a mask to the given BDD which must contain
   frozen, state and input variables]

   Description [Returned bdd is referenced. Calculated mask will be
   cached for future use. The mask will be applicable only to variable that
   occur within the layers committed to self]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
BddStatesInputs
BddEnc_apply_state_frozen_input_vars_mask_bdd(BddEnc_ptr self,
                                              BddStatesInputs states_inputs)
{
  bdd_ptr mask;
  BddStatesInputs res;

  BDD_ENC_CHECK_INSTANCE(self);

  mask = BddEnc_get_state_frozen_input_vars_mask_bdd(self);
  res = BDD_STATES_INPUTS( bdd_and(self->dd, states_inputs, mask) );
  bdd_free(self->dd, mask);

  return res;
}


/**Function********************************************************************

   Synopsis           [Given a variable, it returns the mask of its encoding]

   Description        [Returns the mask that removes repetitions of leaves in
   a variable encoding. Returned ADD is
   referenced. Automatic reordering, if enabled, is
   temporary disabled during this computation.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
add_ptr BddEnc_get_var_mask(BddEnc_ptr self, node_ptr var_name)
{
  node_ptr mask;

  mask = BoolEnc_get_var_mask(BOOL_ENC_CLIENT(self)->bool_enc, var_name);
  return BddEnc_expr_to_add(self, mask, Nil);
}


/**Function********************************************************************

   Synopsis           [Return the list of variables corresponding
   to the current order of variables in the encoding]

   Description [It returns the list of variables corresponding to the
   current order of variables in the encoding. The list will not
   contain NEXT variables (for state variables) and boolean NEXT
   variables are assumed to be close to the corresponding current
   variable.  Notice that in scalar variables current and next bits are
   interleaved (i.e. in current/next pair bits). The returned list must
   be destroyed by the caller.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
NodeList_ptr BddEnc_get_var_ordering(const BddEnc_ptr self,
                                     const VarOrderingType ord_type)
{
  int cvl, max_level;

  NodeList_ptr current_ordering;

  current_ordering = NodeList_create();
  max_level = dd_get_size(self->dd);

  /* we do not use bdd index/level 0, and start from 1. Index 0 is
     reserved */
  for (cvl = 1; cvl < max_level; ++cvl) {
    int index = dd_get_index_at_level(self->dd, cvl);
    node_ptr name = BddEnc_get_var_name_from_index(self, index);

    /* avoid adding NEXT variables */
    if (name != Nil && (node_get_type(name) != NEXT)) {
      if (BoolEnc_is_var_bit(BOOL_ENC_CLIENT(self)->bool_enc, name) &&
          (ord_type != DUMP_BITS)) {
        /* retrieve the corresponding scalar variable: */
        name = BoolEnc_get_scalar_var_from_bit(BOOL_ENC_CLIENT(self)->bool_enc,
                                               name);
      }

      if (!NodeList_belongs_to(current_ordering, name)) {
        NodeList_append(current_ordering, name);
      }
    }
  }

  return current_ordering;
}


/**Function********************************************************************

   Synopsis           [Writes on a file the variable order.]

   Description [This function writes the variable order currently in
   use in the system in the specified output file. The file generated
   as output can be used as input order file for next computations. If
   the specified output file is an empty string ("" or NULL, see
   util_is_string_null) output is redirected to stdout.  The output
   content depends on the value of dump_type, and can be either pure
   scalar (for backward compatibility) or single bits]

   SideEffects        []

   SeeAlso            [Compile_ReadOrder]

******************************************************************************/
void BddEnc_write_var_ordering(const BddEnc_ptr self,
                               const char* output_order_file_name,
                               const VarOrderingType dump_type)
{
  FILE* oof;

  NodeList_ptr out_vars;
  ListIter_ptr iter;

  BDD_ENC_CHECK_INSTANCE(self);

  /* the result list */
  out_vars = BddEnc_get_var_ordering(self, dump_type);

  /* The _process_select_ variable is inserted at the top of the
     ordering, if not otherwise specified */
  if (Nil != proc_selector_internal_vname) {
    NodeList_ptr proc_sel_vars;

    if (dump_type != DUMP_BITS) {
      proc_sel_vars = NodeList_create();
      NodeList_append(proc_sel_vars, proc_selector_internal_vname);
    }
    else {
      proc_sel_vars = BoolEnc_get_var_bits(BOOL_ENC_CLIENT(self)->bool_enc,
                                           proc_selector_internal_vname);
    }

    /* adds the process selector's variables */
    iter = NodeList_get_first_iter(proc_sel_vars);
    while (!ListIter_is_end(iter)) {
      node_ptr name = NodeList_get_elem_at(proc_sel_vars, iter);
      if (!NodeList_belongs_to(out_vars, name)) {
        NodeList_prepend(out_vars, name);
      }
      iter = ListIter_get_next(iter);
    }

    NodeList_destroy(proc_sel_vars);
  }

  /* Opens the file: */
  if (!util_is_string_null(output_order_file_name)) {
    oof = fopen(output_order_file_name, "w");
    if (oof == NULL) {
      rpterr("output_order: unable to open file %s", output_order_file_name);
    }
  }
  else oof = nusmv_stdout;  /* uses stdout */

  /* Actually dump the list: */
  iter = NodeList_get_first_iter(out_vars);
  while (! ListIter_is_end(iter)) {
    node_ptr name = NodeList_get_elem_at(out_vars, iter);
    print_node(oof, name);
    fprintf(oof, "\n");

    iter = ListIter_get_next(iter);
  }

  NodeList_destroy(out_vars);

  /* close the file: */
  if (oof != nusmv_stdout) {
    if (fclose(oof) == EOF) {
      rpterr("cannot close %s", output_order_file_name);
    }
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    if (output_order_file_name != NULL) {
      fprintf(nusmv_stderr, "%s: variable order output to file %s\n",
              NuSMVCore_get_tool_name(), output_order_file_name);
    }
  }
}


/**Function********************************************************************

   Synopsis [Returns the number of reorderings that have been carried
   out since either the self construction or the last call to method
   reset_reordering_count]

   Description [Returns the number of reorderings performed by CUDD
   since the instance creation, or since the last call to method
   reset_reordering_count. Explicit and auto-triggered reorderings are
   counted. Notice that forced ordering due to layers commitment may
   increment the orderings count.]

   SideEffects        []

   SeeAlso            [BddEnc_reset_reordering_count]

******************************************************************************/
int BddEnc_get_reordering_count(const BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  return (dd_get_reorderings(self->dd) - self->curr_reorderings);
}


/**Function********************************************************************

   Synopsis [Resets the reordering count. The value returned by any
   following call to method get_reordering_count will be relative to
   the moment this method had been called]

   Description [Resets the reordering count. The value returned by any
   following call to method get_reordering_count will be relative to
   the moment this method had been called.]

   SideEffects        []

   SeeAlso            [BddEnc_get_reordering_count]

******************************************************************************/
void BddEnc_reset_reordering_count(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  self->curr_reorderings = dd_get_reorderings(self->dd);
}


/**Function********************************************************************

   Synopsis           [Forces a variable ordering in the BDD encoding.]

   Description        [It takes an OrdGroups structure representing the
   possibly partial ordering and the routine complete it with the
   possible missing variables. It is assumed all the variables in the
   given ordering group have been previously allocated within the BDD
   package.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddEnc_force_order(BddEnc_ptr self, OrdGroups_ptr new_po_grps)
{
  NodeList_ptr new_order;
  OrdGroups_ptr new_ord_groups;
  boolean interleaved;

  BDD_ENC_CHECK_INSTANCE(self);

  ORD_GROUPS_CHECK_INSTANCE(new_po_grps);

  new_ord_groups = OrdGroups_create();

  /* We complete the possibly partial order group */
  new_order =
    bdd_enc_sort_variables_and_groups_according(self, new_po_grps,
                                                new_ord_groups,
                                                &interleaved);
  /*
   * At this point new_ord_groups and new_order contain respectively
   * the new grouping of variables and the new order of variables
   * respectively.
   */

  /* We dissolve all the groups previously associated to each layer */
  if (interleaved) {
    bdd_enc_dissolve_committed_groups(self);
  }

  { /* performs the reordering according to new_ord_groups */
    node_ptr name;
    NodeList_ptr vars;
    int group_id, low_index, index, groups_size;
    size_t size;
    SymbLayer_ptr current_layer = SYMB_LAYER(NULL);

    struct GroupInfo {
      SymbLayer_ptr layer;
      int min_idx;
      size_t size;
      int chunk;
    } *group2info; /* this is used later when reconstructing the var groups */

    groups_size = OrdGroups_get_size(new_ord_groups);
    group2info = ALLOC(struct GroupInfo, groups_size);
    nusmv_assert(NULL != group2info);

    for (group_id = 0; group_id < groups_size; ++group_id) {
      ListIter_ptr iter;
      int chunk = -1;

      current_layer = SYMB_LAYER(NULL);
      size = 0;
      low_index = -1;
      index = -1;

      vars = OrdGroups_get_vars_in_group(new_ord_groups, group_id);
      NODE_LIST_FOREACH (vars, iter) {
        name = NodeList_get_elem_at(vars, iter);

        /* Here we must check that the variables of the group belongs
           to the same layer */
        nusmv_assert(SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, name));

        if (SYMB_LAYER(NULL) == current_layer) {
          current_layer = SymbTable_variable_get_layer(BASE_ENC(self)->symb_table, name);
          nusmv_assert(SYMB_LAYER(NULL) != current_layer);
        }

        /* To guarantee the variables of a group belong to the same layer */
        nusmv_assert(SymbLayer_is_variable_in_layer(current_layer, name));

        if (-1 == low_index) {
          low_index = bdd_enc_name_to_index(self, name);
          index = low_index;
        }
        else {
          index = bdd_enc_name_to_index(self, name);
        }

        if (SymbTable_is_symbol_state_var(BASE_ENC(self)->symb_table, name)) {
          if (-1 == chunk) chunk = 2;
          else { nusmv_assert(2 == chunk); }
          index += 1;
        }
        else { /* input and frozen var */
          nusmv_assert(SymbTable_is_symbol_input_var(BASE_ENC(self)->symb_table, name) ||
                       SymbTable_is_symbol_frozen_var(BASE_ENC(self)->symb_table, name));
          if (-1 == chunk) chunk = 1;
          else { nusmv_assert(1 == chunk); }
        }

        nusmv_assert(1 <= chunk && 2 >= chunk);
        size += chunk;
      } /* loop on variables */

      nusmv_assert(-1 != low_index);

      /* saves information which will be used later */
      group2info[group_id].layer = current_layer;
      group2info[group_id].min_idx = low_index;
      group2info[group_id].size = size;
      group2info[group_id].chunk = chunk;
    } /* loop on groups */

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "BddEnc: Reshuffling all layers....");
    }
    bdd_enc_shuffle_variables_order(self, new_order);
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "done\n");
    }

    VarsHandler_update_levels(self->dd_vars_hndr);

    { /* now it is time to construct the new variable blocks */
      for (group_id = 0; group_id < groups_size; ++group_id) {
        int min_lev = dd_get_level_at_index(self->dd,
                                            group2info[group_id].min_idx);
        int real_lev = bdd_enc_create_block(self,
                                            group2info[group_id].layer,
                                            min_lev,
                                            group2info[group_id].size,
                                            group2info[group_id].chunk,
                                            true);
        nusmv_assert(real_lev == min_lev);
      }
    } /* end of group re-construction */

    FREE(group2info);
  }

  OrdGroups_destroy(new_ord_groups);
  NodeList_destroy(new_order);
}

/**Function*******************************************************************

   Synopsis           [Forces a variable ordering in the BDD encoding
   reading it from a file.]

   Description        [It reads an order file and then forces it
   within the BDD package. The order file may be partial. Thanks to
   <tt>BddEnc_force_order</tt> the ordering is  completed with the
   possible missing variables.]

   SideEffects        []

   SeeAlso            [BddEnc_force_order]

*****************************************************************************/
void BddEnc_force_order_from_file(BddEnc_ptr self, FILE * orderfile)
{
  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert((FILE*)NULL != orderfile);
  {
    BoolEnc_ptr bool_enc;
    OrdGroups_ptr grps;
    ParserOrd_ptr parser = ParserOrd_create();

    /* Read the order file and store it in vars */
    ParserOrd_parse_from_file(parser, orderfile);

    bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self));

    grps = enc_utils_create_vars_ord_groups(bool_enc,
                                            ParserOrd_get_vars_list(parser));

    /* Destroy the parser of the ordering file */
    ParserOrd_destroy(parser);

    /* Force the BDD ordering */
    BddEnc_force_order(self, grps);

    OrdGroups_destroy(grps);
  }
}

/**Function********************************************************************

   Synopsis           [Clean the internal cache which contains the results
   of evaluation of expressions to ADD or BDD form.]

   Description        [
   NB: NuSMV option "enable_sexp2bdd_caching" allows to disable the cache
   completely]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void BddEnc_clean_evaluation_cache(BddEnc_ptr self)
{
  BDD_ENC_CHECK_INSTANCE(self);
  BddEncCache_clean_evaluation(self->cache);
}


/**Function********************************************************************

  Synopsis           [Dumps the given AddArray in DOT format]

  Description [Labels is an array of strings to be used as roots
  labels. The size of the array must be equal to the size of the
  AddArray.
  Returns 0 in case of success, 1 otherwise]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
int BddEnc_dump_addarray_dot(BddEnc_ptr self,
                             AddArray_ptr addarray,
                             const char** labels,
                             FILE* outfile)
{
  BDD_ENC_CHECK_INSTANCE(self);
  return bdd_enc_dump_addarray_dot_davinci(self, addarray, labels,
                                           outfile, true);
}


/**Function********************************************************************

  Synopsis           [Dumps the given AddArray in DAVINCI format]

  Description [Labels is an array of strings to be used as roots
  labels. The size of the array must be equal to the size of the
  AddArray.
  Returns 0 in case of success, 1 otherwise]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
int BddEnc_dump_addarray_davinci(BddEnc_ptr self,
                                 AddArray_ptr addarray,
                                 const char** labels,
                                 FILE* outfile)
{
  BDD_ENC_CHECK_INSTANCE(self);
  return bdd_enc_dump_addarray_dot_davinci(self, addarray, labels,
                                           outfile, false);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis           [The BddEnc class private initializer]

   Description        [The BddEnc class private initializer]

   SideEffects        []

   SeeAlso            [BddEnc_create]

******************************************************************************/
void bdd_enc_init(BddEnc_ptr self,
                  SymbTable_ptr symb_table,
                  BoolEnc_ptr bool_enc, VarsHandler_ptr dd_vars_hndr,
                  OrdGroups_ptr ord_groups)
{
  int i;

  /* base class initialization */
  bool_enc_client_init(BOOL_ENC_CLIENT(self), symb_table, bool_enc);

  /* members initialization */
  self->type_checker = SymbTable_get_type_checker(symb_table);

  self->dd_vars_hndr = dd_vars_hndr;
  self->dd = VarsHandler_get_dd_manager(self->dd_vars_hndr);
  self->cache = BddEncCache_create(symb_table, self->dd);

  /* ord_groups can be NULL */
  if (ord_groups != ORD_GROUPS(NULL)) self->ord_groups = ord_groups;
  else self->ord_groups = OrdGroups_create();

  /* dynamic arrays */
  self->level2index = array_alloc(int, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->level2index != (array_t*) NULL);

  self->index2name = array_alloc(node_ptr, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->index2name != (array_t*) NULL);

  self->current2next = array_alloc(int, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->current2next != (array_t*) NULL);

  self->next2current = array_alloc(int, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->next2current != (array_t*) NULL);


  self->minterm_input_vars = array_alloc(bdd_ptr, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->minterm_input_vars != (array_t*) NULL);

  self->minterm_state_vars = array_alloc(bdd_ptr, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->minterm_state_vars != (array_t*) NULL);

  self->minterm_next_state_vars = array_alloc(bdd_ptr, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->minterm_next_state_vars != (array_t*) NULL);

  self->minterm_frozen_vars = array_alloc(bdd_ptr, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->minterm_frozen_vars != (array_t*) NULL);

  self->minterm_state_frozen_vars = array_alloc(bdd_ptr, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->minterm_state_frozen_vars != (array_t*) NULL);

  self->minterm_state_frozen_input_vars = array_alloc(bdd_ptr, BDD_ENC_INIT_VAR_NUM);
  nusmv_assert(self->minterm_state_frozen_input_vars != (array_t*) NULL);


  self->reord_locked_num = 0;

  self->print_stack = Nil;
  self->enforce_constant = false;

  self->input_vars_num = 0;
  self->state_vars_num = 0;
  self->frozen_vars_num = 0;

  /* initializes the arrays containing variables info: */
  for (i=0; i < BDD_ENC_INIT_VAR_NUM; ++i) {
    array_insert(node_ptr, self->index2name, i, (node_ptr) NULL);
    array_insert(int, self->current2next, i, 0);
    array_insert(int, self->next2current, i, 0);
    array_insert(bdd_ptr, self->minterm_input_vars, i, (bdd_ptr) NULL);
    array_insert(bdd_ptr, self->minterm_state_vars, i, (bdd_ptr) NULL);
    array_insert(bdd_ptr, self->minterm_next_state_vars, i, (bdd_ptr) NULL);
    array_insert(bdd_ptr, self->minterm_frozen_vars, i, (bdd_ptr) NULL);
    array_insert(bdd_ptr, self->minterm_state_frozen_vars, i, (bdd_ptr) NULL);
    array_insert(bdd_ptr, self->minterm_state_frozen_input_vars, i, (bdd_ptr) NULL);
  }

  /* minterms frontiers */
  self->minterm_input_vars_dim = 0;
  self->minterm_state_vars_dim = 0;
  self->minterm_next_state_vars_dim = 0;
  self->minterm_frozen_vars_dim = 0;
  self->minterm_state_frozen_vars_dim = 0;
  self->minterm_state_frozen_input_vars_dim = 0;

  self->index_gaps = NodeList_create();
  self->used_indices_frontier = 0;

  /* hashes: */
  self->name2index = new_assoc();
  self->layer2groups = new_assoc();

  /* ADDs and BDDs: */
  self->input_vars_add = add_true(self->dd);
  self->state_vars_add = add_true(self->dd);
  self->next_state_vars_add = add_true(self->dd);
  self->frozen_vars_add = add_true(self->dd);

  /* BDDs are calculated only on demand, converting the corresponding ADDs */
  self->input_vars_bdd = (bdd_ptr) NULL;
  self->state_vars_bdd = (bdd_ptr) NULL;
  self->next_state_vars_bdd = (bdd_ptr) NULL;
  self->frozen_vars_bdd = (bdd_ptr) NULL;
  self->state_frozen_vars_bdd = (bdd_ptr) NULL;

  /* masks: */
  self->input_vars_mask_add = (add_ptr) NULL;
  self->state_frozen_vars_mask_add = (add_ptr) NULL;
  self->state_frozen_input_vars_mask_add = (add_ptr) NULL;
  self->input_vars_mask_bdd = (bdd_ptr) NULL;
  self->state_frozen_vars_mask_bdd = (bdd_ptr) NULL;
  self->state_frozen_input_vars_mask_bdd = (bdd_ptr) NULL;

  /* reorderings counters */
  BddEnc_reset_reordering_count(self);

  /* failures candidates */
  self->failures_hash = new_assoc();

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = bdd_enc_finalize;

  /* inherited by base class: */
  OVERRIDE(BaseEnc, commit_layer) = bdd_enc_commit_layer;
  OVERRIDE(BaseEnc, remove_layer) = bdd_enc_remove_layer;
}


/**Function********************************************************************

   Synopsis           [The BddEnc class private deinitializer]

   Description        [The BddEnc class private deinitializer]

   SideEffects        []

   SeeAlso            [BddEnc_destroy]

******************************************************************************/
void bdd_enc_deinit(BddEnc_ptr self)
{
  int i;

  /* members deinitialization */
  BddEncCache_destroy(self->cache);
  OrdGroups_destroy(self->ord_groups);

  /* initializes the arrays containing variables info: */
  for (i=0; i < self->minterm_input_vars_dim; ++i) {
    bdd_free(self->dd, array_fetch(bdd_ptr, self->minterm_input_vars, i));
    array_insert(bdd_ptr, self->minterm_input_vars, i, NULL);
  }

  for (i=0; i < self->minterm_state_vars_dim; ++i) {
    bdd_free(self->dd, array_fetch(bdd_ptr, self->minterm_state_vars, i));
    array_insert(bdd_ptr, self->minterm_state_vars, i, NULL);
  }

  for (i=0; i < self->minterm_next_state_vars_dim; ++i) {
    bdd_free(self->dd, array_fetch(bdd_ptr, self->minterm_next_state_vars, i));
    array_insert(bdd_ptr, self->minterm_next_state_vars, i, NULL);
  }

  for (i=0; i < self->minterm_frozen_vars_dim; ++i) {
    bdd_free(self->dd, array_fetch(bdd_ptr, self->minterm_frozen_vars, i));
    array_insert(bdd_ptr, self->minterm_frozen_vars, i, NULL);
  }

  for (i=0; i < self->minterm_state_frozen_vars_dim; ++i) {
    bdd_free(self->dd, array_fetch(bdd_ptr, self->minterm_state_frozen_vars, i));
    array_insert(bdd_ptr, self->minterm_state_frozen_vars, i, NULL);
  }

  for (i=0; i < self->minterm_state_frozen_input_vars_dim ; ++i) {
    bdd_free(self->dd,
             array_fetch(bdd_ptr, self->minterm_state_frozen_input_vars, i));
    array_insert(bdd_ptr, self->minterm_state_frozen_input_vars, i, NULL);
  }

  NodeList_destroy(self->index_gaps);

  /* ADDs and BDDs: */
  BDD_ENC_FREE_ADD(self->input_vars_add);
  BDD_ENC_FREE_ADD(self->state_vars_add);
  BDD_ENC_FREE_ADD(self->next_state_vars_add);
  BDD_ENC_FREE_ADD(self->frozen_vars_add);

  BDD_ENC_FREE_BDD(self->input_vars_bdd);
  BDD_ENC_FREE_BDD(self->state_vars_bdd);
  BDD_ENC_FREE_BDD(self->next_state_vars_bdd);
  BDD_ENC_FREE_BDD(self->frozen_vars_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_vars_bdd);

  /* masks: */
  BDD_ENC_FREE_ADD(self->input_vars_mask_add);
  BDD_ENC_FREE_ADD(self->state_frozen_vars_mask_add);
  BDD_ENC_FREE_ADD(self->state_frozen_input_vars_mask_add);
  BDD_ENC_FREE_BDD(self->input_vars_mask_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_vars_mask_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_input_vars_mask_bdd);

  /* hashes */
  {
    SymbLayer_ptr layer;
    array_t* groups;
    assoc_iter aiter;

    ASSOC_FOREACH(self->layer2groups, aiter, &layer, &groups) {
      if ((array_t*) NULL != groups) {
        int giter;
        GroupInfo_ptr binfo;

        if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
          fprintf(nusmv_stderr,
                  "BddEnc: releasing dd groups associated with layer '%s'\n",
                  SymbLayer_get_name(layer));
        }

        /* releases all groups */
        arrayForEachItem(GroupInfo_ptr, groups, giter, binfo) {
          /* this releases and frees binfo which was returned by
             VarsHandler_reserve_group */
          VarsHandler_release_group(self->dd_vars_hndr, binfo);
        }

        array_free(groups);
      }
    }
  }
  free_assoc(self->layer2groups);
  free_assoc(self->name2index);

  clear_assoc_and_free_entries_arg(self->failures_hash, hash_add_key_free,
                                   (char*) self->dd);
  free_assoc(self->failures_hash);

  /* dynamic arrays */
  array_free(self->level2index);
  array_free(self->index2name);
  array_free(self->current2next);
  array_free(self->next2current);
  array_free(self->minterm_input_vars);
  array_free(self->minterm_state_vars);
  array_free(self->minterm_next_state_vars);
  array_free(self->minterm_frozen_vars);
  array_free(self->minterm_state_frozen_vars);
  array_free(self->minterm_state_frozen_input_vars);

  /* base class deinitialization */
  bool_enc_client_deinit(BOOL_ENC_CLIENT(self));
}


/**Function********************************************************************

   Synopsis           [Encodes all variables within the given layer. If the
   given layer has an associated boolean layer (created by the BoolEnc), that
   boolean layer will be encoded as well.]

   Description        []

   SideEffects        []

   SeeAlso            [bdd_enc_remove_layer]

******************************************************************************/
void bdd_enc_commit_layer(BaseEnc_ptr enc_base, const char* layer_name)
{
  BddEnc_ptr self;
  SymbTable_ptr symbTable;
  SymbLayer_ptr layer;
  SymbLayer_ptr layers[3];
  const char* bool_layer_name;
  int idx;

  self = BDD_ENC(enc_base);
  symbTable = BASE_ENC(self)->symb_table;

  /* Calls the base method to add this layer */
  bool_enc_client_commit_layer(enc_base, layer_name);

  layer = SymbTable_get_layer(symbTable, layer_name);
  layers[0] = layer;

  /* tries to retrieve the boolean layer (if there is any) that has
     been created from the layer that is being committed. If there
     exists such a layer, commit that as well. */
  bool_layer_name = BoolEnc_scalar_layer_to_bool_layer(layer_name);

  layers[1] = SymbTable_get_layer(symbTable, bool_layer_name);
  if (layers[1] != SYMB_LAYER(NULL)) {
    bool_enc_client_commit_layer(enc_base, bool_layer_name);
  }

  layers[2] = SYMB_LAYER(NULL); /* a terminator for the set of layers */

  /* -------------------------------------------------- */
  /*               Begins the hard work                 */
  /* -------------------------------------------------- */

  /* encodes constants */
  idx = 0;
  while (layers[idx] != SYMB_LAYER(NULL)) {
    SymbLayerIter iter;

    /* first all the constants are encoded: */
    SYMB_LAYER_FOREACH(layers[idx], iter, STT_CONSTANT) {
      node_ptr constant = SymbLayer_iter_get_symbol(layers[idx], &iter);
      add_ptr add_constant;

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
        fprintf(nusmv_stderr, "BddEnc: encoding constant ");
        print_node(nusmv_stderr, constant);
        fprintf(nusmv_stderr, "\n");
      }

      add_constant = add_leaf(self->dd, constant);
      BddEncCache_new_constant(self->cache, constant, add_constant);
      add_free(self->dd, add_constant);
    }

    idx += 1;
  } /* end of iteration on layers */


  /* Retrieves the list of sorted vars, and creates the sorted groups
     of those variables that must be created. Then iterates on the
     groups, and creates corresponding bdd indices and variables */
  {
    NodeList_ptr ord_vars;
    OrdGroups_ptr groups;
    int group_id, groups_num;
    boolean needs_reshuffle = false;
    boolean dissolve_groups = false;

    groups = OrdGroups_create();
    ord_vars = bdd_enc_sort_variables_and_groups(self, layer, groups,
                                                 &needs_reshuffle,
                                                 &dissolve_groups);

    groups_num = OrdGroups_get_size(groups);
    for (group_id = 0; group_id < groups_num; ++group_id) {
      int group_size = 0;
      int group_chunk = -1; /* this can be either 1 or 2 */
      NodeList_ptr group = OrdGroups_get_vars_in_group(groups, group_id);
      ListIter_ptr gr_iter;

      /* calculates the group size by checking the number and the type
         of required variables */
      NODE_LIST_FOREACH(group, gr_iter) {
        node_ptr name = NodeList_get_elem_at(group, gr_iter);
        nusmv_assert(SymbTable_is_symbol_var(symbTable, name));

        if (SymbTable_is_symbol_state_var(symbTable, name)) {
          if (-1 == group_chunk) group_chunk = 2;

          /* not mixed input/frozen and state in the same group */
          nusmv_assert(2 == group_chunk);
        }
        else { /* frozen of input */
          if (-1 == group_chunk) group_chunk = 1;
          /* not mixed input/frozen and state in the same group */
          nusmv_assert(1 == group_chunk);
        }
        group_size += group_chunk;
      }

      if (group_size > 0) {
        /* books groups_size indices and creates the set of variables,
           allocating the bdd blocks */
        SymbLayer_ptr block_layer;
        int level;

        bdd_enc_lock_reordering(self); /* locks reordering */

        block_layer = SymbTable_get_layer(symbTable, layer_name);
        level = bdd_enc_reserve_consecutive_vars(self, block_layer,
                                                 group_size, group_chunk);

        gr_iter = NodeList_get_first_iter(group);
        while (!ListIter_is_end(gr_iter)) {
          node_ptr name = NodeList_get_elem_at(group, gr_iter);

          /* state variable */
          if (SymbTable_is_symbol_state_var(symbTable, name)) {
            int cindex = dd_get_index_at_level(self->dd, level++);
            int nindex = dd_get_index_at_level(self->dd, level++);
            bdd_enc_add_state_var(self, block_layer, name, cindex, nindex);
          }

          /* input variable */
          else if (SymbTable_is_symbol_input_var(symbTable, name)) {
            int cindex = dd_get_index_at_level(self->dd, level++);
            bdd_enc_add_input_var(self, name, cindex);
          }

          /* frozen variable */
          else {
            int cindex;
            nusmv_assert(SymbTable_is_symbol_frozen_var(symbTable, name));
            cindex = dd_get_index_at_level(self->dd, level++);
            bdd_enc_add_frozen_var(self, name, cindex);
          }

          gr_iter = ListIter_get_next(gr_iter);
        } /* group vars loop */

        bdd_enc_unlock_reordering(self); /* release reordering lock */
      }
    } /* for groups */


    /* reshuffle the dd level, by changing the bdd level order, but
       only if needed) */
    if (needs_reshuffle) {

      VarsHandler_update_levels(self->dd_vars_hndr);

      /* We dissolve all the groups previously associated to each layer */
      if (dissolve_groups) {
        bdd_enc_dissolve_committed_groups(self);
      }

      if (BddEnc_get_reordering_count(self) == 0) {
        if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
          fprintf(nusmv_stderr, "BddEnc: Reshuffling layer '%s'...\n",
                  layer_name);
        }
        bdd_enc_shuffle_variables_order(self, ord_vars);
      }
      else {
        if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
          fprintf(nusmv_stderr, "BddEnc: Warning:\n"
                  "Skipping reshuffle as dynamic reordering occurred.\n");
        }
      }

      VarsHandler_update_levels(self->dd_vars_hndr);

      if (dissolve_groups) {
        /* now minimal groups (for current and next variables) have
           to be constructed */
        ListIter_ptr iter;
        NODE_LIST_FOREACH(ord_vars, iter) {
          node_ptr name = NodeList_get_elem_at(ord_vars, iter);
          int chunk = (SymbTable_is_symbol_state_var(
                           BASE_ENC(self)->symb_table, name)) ? 2 : 1;
          if (chunk > 1) {
            SymbLayer_ptr current_layer = SymbTable_variable_get_layer(
                                               BASE_ENC(self)->symb_table, name);

            int idx = BddEnc_get_var_index_from_name(self, name);
            int lev = dd_get_level_at_index(self->dd, idx);
            int real_lev = bdd_enc_create_block(self, current_layer,
                                                lev, chunk, chunk, false);
            /* Since groups are new (not shared) we must obtain
               exactly the requested level */
            nusmv_assert(real_lev == lev);
          }
        } /* loop over vars */
      }
    }

    NodeList_destroy(ord_vars);
    OrdGroups_destroy(groups);
  }

}


/**Function********************************************************************

   Synopsis           [Removes the encoding of all variables occurring within
   the given layer, and those that had been created within the corresponding
   boolean layer during the boolean encoding (if there was any).
   Then releases both the layers.]

   Description        [  WARNING: If the layer has been
   renamed after having been committed, it is the *new* name (the name
   the layer has when it is being removed) that must be used, and *not*
   the name that had been used when commiting it.]

   SideEffects        []

   SeeAlso            [bdd_enc_commit_layer]

******************************************************************************/
void bdd_enc_remove_layer(BaseEnc_ptr enc_base, const char* layer_name)
{
  BddEnc_ptr self;
  SymbLayer_ptr layers[3];
  const char* bool_layer_name;
  int i;

  self = BDD_ENC(enc_base);

  layers[0] = SymbTable_get_layer(BASE_ENC(self)->symb_table, layer_name);

  /* tries to retrieve the boolean layer (if there is any) that has
     been created from the layer that is being committed. If there
     exists such a layer, remove that as well. */
  bool_layer_name = BoolEnc_scalar_layer_to_bool_layer(layer_name);

  layers[1] = SymbTable_get_layer(BASE_ENC(self)->symb_table, bool_layer_name);
  layers[2] = SYMB_LAYER(NULL); /* a terminator */

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) inc_indent_size();

  /* -------------------------------------------------- */
  /*               Begins the hard work                 */
  /* -------------------------------------------------- */

  { /* At first dissolves the associated groups */
    array_t* groups;

    groups = (array_t*) find_assoc(self->layer2groups, (node_ptr) layers[0]);
    if (groups != (array_t*) NULL) {
      boolean _frc; /* for debugging */
      int iter;
      GroupInfo_ptr binfo;

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
        fprintf(nusmv_stderr,
                "BddEnc: removing dd groups associated with layer '%s'\n",
                SymbLayer_get_name(layers[0]));
      }

      bdd_enc_lock_reordering(self);

      arrayForEachItem(GroupInfo_ptr, groups, iter, binfo) {
        _frc = VarsHandler_release_group(self->dd_vars_hndr, binfo);
      }

      bdd_enc_unlock_reordering(self);

      array_free(groups);
      insert_assoc(self->layer2groups, (node_ptr) layers[0], (node_ptr) NULL);
    }
  } /* end of blocks removal */

  /* removes constant and variables */
  i=0;
  while (layers[i] != SYMB_LAYER(NULL)) {
    SymbLayerIter iter;

    SYMB_LAYER_FOREACH(layers[i], iter, STT_CONSTANT) {
      node_ptr constant = SymbLayer_iter_get_symbol(layers[i], &iter);

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
        fprintf(nusmv_stderr, "BddEnc: removing constant ");
        print_node(nusmv_stderr, constant);
        fprintf(nusmv_stderr, "\n");
      }

      BddEncCache_remove_constant(self->cache, constant);
    }

    SYMB_LAYER_FOREACH_FILTER(layers[i], iter, STT_VAR,
                              SymbLayer_iter_filter_bool_vars, NULL) {
      node_ptr name = SymbLayer_iter_get_symbol(layers[i], &iter);

      nusmv_assert(SymbTable_is_symbol_bool_var(BASE_ENC(self)->symb_table, name));

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
        fprintf(nusmv_stderr, "BddEnc: removing variable ");
        print_node(nusmv_stderr, name);
        fprintf(nusmv_stderr, " (index = %d)\n",
                BddEnc_get_var_index_from_name(self, name));
      }

      bdd_enc_remove_var(self, name);
    }

    i += 1;
  } /* end of for on layer and boolean */

  /* cleans up all symbols into the scalar and boolean layer */
  {
    SymbLayerIter iter;
    NodeList_ptr syms;

    SymbLayer_gen_iter(layers[0], &iter, STT_ALL);
    syms = SymbLayer_iter_to_list(layers[0], iter);

    BddEncCache_clean_evaluation_about(self->cache, syms);

    NodeList_destroy(syms);

    if (SYMB_LAYER(NULL) != layers[1]) {
      SymbLayer_gen_iter(layers[1], &iter, STT_ALL);
      syms = SymbLayer_iter_to_list(layers[1], iter);

      BddEncCache_clean_evaluation_about(self->cache, syms);

      NodeList_destroy(syms);
    }
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) dec_indent_size();

  /* compacts the minterms arrays: */
  self->minterm_input_vars_dim =
    bdd_enc_compact_minterms_array(self, self->minterm_input_vars,
                                   self->minterm_input_vars_dim);
  self->minterm_state_vars_dim =
    bdd_enc_compact_minterms_array(self, self->minterm_state_vars,
                                   self->minterm_state_vars_dim);
  self->minterm_next_state_vars_dim =
    bdd_enc_compact_minterms_array(self, self->minterm_next_state_vars,
                                   self->minterm_next_state_vars_dim);
  self->minterm_frozen_vars_dim =
    bdd_enc_compact_minterms_array(self, self->minterm_frozen_vars,
                                   self->minterm_frozen_vars_dim);
  self->minterm_state_frozen_vars_dim =
    bdd_enc_compact_minterms_array(self, self->minterm_state_frozen_vars,
                                   self->minterm_state_frozen_vars_dim);
  self->minterm_state_frozen_input_vars_dim =
    bdd_enc_compact_minterms_array(self, self->minterm_state_frozen_input_vars,
                                   self->minterm_state_frozen_input_vars_dim);

  /* finally calls the inherited method: */
  bool_enc_client_remove_layer(enc_base, layer_name);
  if (layers[1] != SYMB_LAYER(NULL)) {
    bool_enc_client_remove_layer(enc_base, bool_layer_name);
  }
}


/**Function********************************************************************

   Synopsis           [Reshuffle the ordering of bdds. The ordering is
   taken from the provided list.]

   Description        [The given list can be partial wrt the total number of
   variables within the dd manager. All remaining vars are moved *after* the
   last variable occurring within the provided list of vars.]

   SideEffects        []

   SeeAlso            [bdd_enc_commit_layer]

******************************************************************************/
void bdd_enc_shuffle_variables_order(BddEnc_ptr self, NodeList_ptr vars)
{
  boolean changed = false;
  int level;
  int po_vars_num = NodeList_get_length(vars);
  int tot_vars_num = dd_get_size(self->dd);
  int res = 1;
  ListIter_ptr iter;
  NodeList_ptr used_indices;

  nusmv_assert(po_vars_num <= tot_vars_num);

  /* some verbose information about what is happening to indices
     and levels (continues below with the results) */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
    const int str_len = (int) (log10(tot_vars_num)+1);
    int idx;

    fprintf(nusmv_stderr, "BddEnc:        Index: \t");
    for (idx = 0; idx < tot_vars_num; ++idx) {
      fprintf(nusmv_stderr, "%*d ", str_len, idx);
    }
    fprintf(nusmv_stderr, "\nBddEnc: Before Level: \t");
    for (idx = 0; idx < tot_vars_num; ++idx) {
      fprintf(nusmv_stderr, "%*d ", str_len,
              dd_get_level_at_index(self->dd, idx));
    }
    fprintf(nusmv_stderr, "\n");
  }

  used_indices = NodeList_create();

  array_insert(int, self->level2index, 0, 0);
  level = 1;
  NODE_LIST_FOREACH(vars, iter) {
    node_ptr name = NodeList_get_elem_at(vars, iter);
    int idx = BddEnc_get_var_index_from_name(self, name);
    if (level != dd_get_level_at_index(self->dd, idx)) changed = true;
    array_insert(int, self->level2index, level++, idx);
    NodeList_append(used_indices, NODE_FROM_INT(idx));

    if (SymbTable_is_symbol_state_var(BASE_ENC(self)->symb_table, name) &&
        (node_get_type(name) != NEXT)) {
      node_ptr next_name = bdd_enc_get_next_state_var_name(name);
      if (!NodeList_belongs_to(vars, next_name)) {
        /* it is assumed here that next is always closed to current */
        int nidx = array_fetch(int, self->current2next, idx);
        if (level != dd_get_level_at_index(self->dd, nidx)) changed = true;

        array_insert(int, self->level2index, level++, nidx);
        NodeList_append(used_indices, NODE_FROM_INT(nidx));
      }
    }

  } /* loop over vars */

  if (level < tot_vars_num) {
    /* the dd contains variables that are not occurring within the
       list vars. Those must be put at the end of the ordering. */
    int index;
    /* index 0 is not used */
    for (index = 1; index < tot_vars_num; ++index) {
      if (!NodeList_belongs_to(used_indices, NODE_FROM_INT(index))) {
        if (level != dd_get_level_at_index(self->dd, index)) changed = true;
        array_insert(int, self->level2index, level++, index);
      }
    }
  }

  /* get rid of unused list */
  NodeList_destroy(used_indices);

  /* performs the reordering, if needed */
  if (changed) {
    res = dd_set_order(self->dd, array_fetch_p(int, self->level2index, 0));
  }

  /* We verbosely show the results here */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
    if (changed) {
      const int str_len = (int) (log10(tot_vars_num)+1);
      int idx;

      fprintf(nusmv_stderr, "BddEnc:   Lev2Index:  \t");
      for (idx = 0; idx < tot_vars_num; ++idx) {
        fprintf(nusmv_stderr, "%*d ", str_len,
                array_fetch(int, self->level2index, idx));
      }
      fprintf(nusmv_stderr, "\n");

      {
        char* req_lev = ALLOC(char, (str_len+1) * tot_vars_num + 1);

        for (idx=0; idx < (str_len+1)*tot_vars_num; ++idx) {
          req_lev[idx] = ' ';
        }
        req_lev[(str_len+1)*tot_vars_num] = '\0';

        for (idx = 0; idx < tot_vars_num; ++idx) {
          int ofs = array_fetch(int, self->level2index, idx)*(str_len+1);
          char trail = *(req_lev+ofs+str_len+1);
          sprintf(req_lev+ofs, "%*d ", str_len, idx);
          *(req_lev+ofs+str_len+1) = trail;
        }

        fprintf(nusmv_stderr, "BddEnc:    Req Level: \t");
        fprintf(nusmv_stderr, "%s\n", req_lev);
        FREE(req_lev);
      }

      fprintf(nusmv_stderr, "BddEnc:  After Level: \t");
      for (idx = 0; idx < tot_vars_num; ++idx) {
        fprintf(nusmv_stderr, "%*d ", str_len,
                dd_get_level_at_index(self->dd, idx));
      }
      fprintf(nusmv_stderr, "\n");

    }
    else {
      fprintf(nusmv_stderr, "(No reordering was needed)\n");
    }
  } /* end of verbose info */

  nusmv_assert(res == 1); /* success? */
  /* adds a forced reordering count as cudd does not count it */
  /* if (changed) self->curr_reorderings -= 1; */
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis    [The BddEnc class virtual finalizer]

   Description [Called by the class destructor]

   SideEffects []

   SeeAlso     []

******************************************************************************/
static void bdd_enc_finalize(Object_ptr object, void* dummy)
{
  BddEnc_ptr self = BDD_ENC(object);

  bdd_enc_deinit(self);
  FREE(self);
}


/**Function********************************************************************

   Synopsis           [Private service of bdd_enc_reserve_consecutive_vars,
   to perform quicksort on integer arrays.]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int bdd_enc_int_qsort(const void* e1, const void* e2)
{
  int a = *((int*) e1); int b = *((int*) e2);
  if (a < b) return -1;
  if (a == b) return 0;
  return 1;
}

/**Function********************************************************************

   Synopsis [Returns a first level of the created group]

   Description [The list of gaps is searched, and a position is
   used if there is an available gap. Otherwise a new index is
   taken. The returned level is guaranteed to be the first of
   block_size levels, already grouped.

   Furthermore, the set of levels is guarantees to contain repeated
   chunks of 'idx_chunk_size' consecutive indices.

   The list of indices is chosen by looking at fist at the possible
   list of previoulsy removed variables (gaps). A set of chunks of
   consecutive levels is searched and collected taking into account the
   index chunk size. Then the smaller set is chosen to be returned as
   variable indices.

   An error will occur if the maximum number of variables is reached.]

   SideEffects [If the indices are taken from the list of gaps,
   those indices will be removed from the list.]

   SeeAlso            []

******************************************************************************/
static int bdd_enc_reserve_consecutive_vars(BddEnc_ptr self,
                                            const SymbLayer_ptr layer,
                                            size_t block_size,
                                            size_t idx_chunk_size)
{
  const int INVALID_LEV = -2; /* not supposed to be changed! */

  ListIter_ptr iter;
  array_t* levels;
  boolean enough_levels;

  nusmv_assert(idx_chunk_size <= block_size);

  levels = array_alloc(int, block_size);
  nusmv_assert(levels != (array_t*) NULL);

  /* searches in the gaps list large enough gaps of consecutives indices: */
  enough_levels = false;
  NODE_LIST_FOREACH(self->index_gaps, iter) {
    node_ptr el;
    int gap_idx, gap_size;
    int lev_count = 0; /* number of contiguous levels */
    int curr_lev_init = -1;
    int prev_lev = INVALID_LEV;
    int idx;

    if (enough_levels) break;

    el = NodeList_get_elem_at(self->index_gaps, iter);
    gap_idx = NODE_TO_INT(car(el));
    gap_size = NODE_TO_INT(cdr(el));

    /* pushes into the array 'levels' those levels that are good
       candidates for being reused */
    for (idx = 0; idx < gap_size; ++idx) {
      int lev = dd_get_level_at_index(self->dd, idx+gap_idx);
      nusmv_assert(lev != -1);

      if (curr_lev_init == -1) curr_lev_init = lev;
      if (prev_lev == INVALID_LEV || lev == prev_lev+1) lev_count += 1;

      if ((prev_lev != INVALID_LEV && lev != prev_lev+1) || idx == gap_size-1) {
        if (lev_count >= idx_chunk_size &&
            VarsHandler_can_group(self->dd_vars_hndr, curr_lev_init,
                                  lev_count, idx_chunk_size)) {
          int i;
          for (i=0; i < lev_count; ++i) {
            array_insert_last(int, levels, curr_lev_init+i);
          }

          /* is it enough? (this is an optimization to avoid searching
             minimal set when all that the caller needs a contiguous
             set of indices and levels) */
          if (idx_chunk_size == block_size) {
            enough_levels = true;
            break;
          }
        }
        curr_lev_init = lev;
        lev_count = 1;
      }

      prev_lev = lev;
    } /* for */
  } /* while on gaps */

  array_sort(levels, bdd_enc_int_qsort);

  /* Searches within the available level chunks the smaller one, to
     optimize future calls that will more likely reuse available gaps.
     At the end array_min will contain the indices chosen from the set
     of gaps. */
  {
    int lev_min = -1;
    int size_min = -1;

    int lev_count = 0;
    int curr_lev_init = -1;
    int prev_lev = INVALID_LEV;
    int idx;

    for (idx = 0; idx < array_n(levels); ++idx) {
      int lev = array_fetch(int, levels, idx);

      if (curr_lev_init == -1) curr_lev_init = lev;
      if (prev_lev == INVALID_LEV || lev == prev_lev+1) lev_count += 1;

      if ((prev_lev != INVALID_LEV && lev != prev_lev+1) ||
          idx == array_n(levels)-1) {

        if (lev_count >= block_size &&
            (size_min == -1 || lev_count < size_min)) {
          size_min = lev_count;
          lev_min = curr_lev_init;
        }

        curr_lev_init = lev;
        lev_count = 1;
      }

      prev_lev = lev;
    } /* for */

    array_free(levels); /* this array is no longer needed */

    if (size_min != -1) {
      /* removes the gaps corresponding to the given indices */
      int i;

      nusmv_assert(size_min >= block_size);

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
        fprintf(nusmv_stderr,
                "BddEnc: removing gaps for reusing of %" PRIuPTR \
                " variables from level %d, index %d\n",
                block_size, lev_min, dd_get_index_at_level(self->dd, lev_min));
      }
      for (i=0; i < block_size; i += idx_chunk_size) {
        int idx = dd_get_index_at_level(self->dd, lev_min+i);
        bdd_enc_remove_gap(self, idx, idx_chunk_size);
      }
    }
    else { /* no reuse of gaps: adds levels taking them from the
              index frontier */
      lev_min = self->used_indices_frontier+1;
      self->used_indices_frontier += block_size;
    }

    return bdd_enc_create_block(self, layer, lev_min, block_size,
                                idx_chunk_size, true);
  }
}


/**Function********************************************************************

   Synopsis [Allocates a block by reserving their respective
   levels, and associating the block to the respective layer.]

   Description [Returns a first level of the created
   group. Returned level may be different from the requested one.
   This is a private service used only when dealing with groups.]

   SeeAlso     []

******************************************************************************/
static int bdd_enc_create_block(BddEnc_ptr self,
                                SymbLayer_ptr layer,
                                int lev_min, size_t block_size,
                                size_t idx_chunk_size,
                                boolean shared)
{
  int real_lev;
  GroupInfo_ptr gid = VarsHandler_reserve_group(self->dd_vars_hndr,
                                                lev_min, block_size,
                                                idx_chunk_size,
                                                shared, &real_lev);
  array_t* blocks = (array_t*) find_assoc(self->layer2groups,
                                          (node_ptr) layer);
  if (blocks == (array_t*) NULL) {
    /* creates one */
    blocks = array_alloc(GroupInfo_ptr, 1);
    nusmv_assert((array_t*) NULL != blocks);
    insert_assoc(self->layer2groups, (node_ptr) layer, (node_ptr) blocks);
  }
  array_insert_last(GroupInfo_ptr, blocks, gid);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
    fprintf(nusmv_stderr, "BddEnc: created a (%s) block of %" PRIuPTR \
            " variables: " \
            "Asked from level %d, obtained from level %d to level %"  \
            PRIuPTR "\n",
            shared ? "shared" : "not shared",
            block_size, lev_min, real_lev, real_lev+block_size-1);
  }

  return real_lev;
}


/**Function********************************************************************

   Synopsis           [Adds a new boolean input variable to the DD package.]

   Description [This function is used to create the boolean variables
   needed to encode input variables. Both an ADD and a BDD will be created.
   The variable will be created at the given index.
   Minterms arrays will be updated, as well as ADD cubes. ]

   SideEffects        []

   SeeAlso            [bdd_enc_add_state_var]

******************************************************************************/
static void bdd_enc_add_input_var(BddEnc_ptr self, node_ptr name, int index)
{
  add_ptr add;

  nusmv_assert(bdd_enc_is_index_not_allocated(self, index));

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "BddEnc: creating input variable ");
    print_node(nusmv_stderr, name);
    fprintf(nusmv_stderr, "\n  BDD input variable index %d, level %d\n",
            index, dd_get_level_at_index(self->dd, index));
  }

  /* Creates the add */
  add = add_new_var_with_index(self->dd, index);
  BddEncCache_new_boolean_var(self->cache, name, add);
  bdd_enc_accumulate_input_var_cube(self, add);
  add_free(self->dd, add);

  /* keeps track of the creation of a new input var: */
  self->input_vars_num += 1;

  /* creates the bdd */
  bdd_enc_add_input_var_to_minterm(self, index);

  /* indices: */
  array_insert(int, self->current2next, index, index);
  array_insert(int, self->next2current, index, index);

  /* associates dd index and name */
  array_insert(node_ptr, self->index2name, index, name);
  insert_assoc(self->name2index, name, NODE_FROM_INT(index));

  /* cleans up a set of cached values: */
  BDD_ENC_FREE_ADD(self->input_vars_mask_add);
  BDD_ENC_FREE_ADD(self->state_frozen_input_vars_mask_add);
  BDD_ENC_FREE_BDD(self->input_vars_mask_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_input_vars_mask_bdd);
}


/**Function********************************************************************

   Synopsis           [Adds a new boolean state variable to the DD package.]

   Description [This function is used to create the boolean variables
   needed to encode state variables. Two ADDs and two BDDs will be
   created, to represent current and next state variables. These two
   pairs are garanteeded to live close each other, and a group will be
   created to prevent dynamic reordering to separate them. The created group
   will be associated to the given layer. No group will be created if
   layer is NULL.

   Minterms arrays will be updated, as well as ADD cubes. ]

   SideEffects        []

   SeeAlso            [bdd_enc_add_input_var]

******************************************************************************/
static void bdd_enc_add_state_var(BddEnc_ptr self, SymbLayer_ptr layer,
                                  node_ptr name,
                                  int curr_index, int next_index)
{
  add_ptr curr, next;
  node_ptr next_name;

  /* not already allocated */
  nusmv_assert(bdd_enc_is_index_not_allocated(self, curr_index));
  nusmv_assert(bdd_enc_is_index_not_allocated(self, next_index));

  /* either new or consecutive*/
  nusmv_assert((dd_get_level_at_index(self->dd, curr_index) == -1 &&
                dd_get_level_at_index(self->dd, next_index) == -1) ||
               (dd_get_level_at_index(self->dd, curr_index)+1 ==
                dd_get_level_at_index(self->dd, next_index)));

  /* ---------------------- */
  /*   Current state var    */
  /* ---------------------- */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "BddEnc: creating state variable ");
    print_node(nusmv_stderr, name);
    fprintf(nusmv_stderr, "\n  BDD variable index %d, level %d\n",
            curr_index, dd_get_level_at_index(self->dd, curr_index));
  }

  curr = add_new_var_with_index(self->dd, curr_index);
  BddEncCache_new_boolean_var(self->cache, name, curr);
  bdd_enc_add_state_var_to_minterm(self, curr_index);
  array_insert(node_ptr, self->index2name, curr_index, name);
  insert_assoc(self->name2index, name, NODE_FROM_INT(curr_index));
  self->state_vars_num += 1;

  /* ---------------------- */
  /*     Next state var     */
  /* ---------------------- */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "  BDD next state variable index %d, level %d\n",
            next_index, dd_get_level_at_index(self->dd, curr_index));
  }

  next_name = bdd_enc_get_next_state_var_name(name);
  next = add_new_var_with_index(self->dd, next_index);
  BddEncCache_new_boolean_var(self->cache, next_name, next);
  bdd_enc_add_next_state_var_to_minterm(self, next_index);
  array_insert(node_ptr, self->index2name, next_index, next_name);
  insert_assoc(self->name2index, next_name, NODE_FROM_INT(next_index));
  self->state_vars_num += 1;

  /* the cubes: */
  bdd_enc_accumulate_state_var_cube(self, curr, next);
  add_free(self->dd, next);
  add_free(self->dd, curr);

  /* indices: */
  array_insert(int, self->current2next, curr_index, next_index);
  /* identity for next var */
  array_insert(int, self->current2next, next_index, next_index);
  array_insert(int, self->next2current, next_index, curr_index);
  /* identity for curr var */
  array_insert(int, self->next2current, curr_index, curr_index);

  /* cleans up a set of cached values: */
  BDD_ENC_FREE_ADD(self->state_frozen_vars_mask_add);
  BDD_ENC_FREE_ADD(self->state_frozen_input_vars_mask_add);
  BDD_ENC_FREE_BDD(self->state_frozen_vars_mask_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_input_vars_mask_bdd);
}


/**Function********************************************************************

   Synopsis           [Adds a new boolean frozen variable to the DD package.]

   Description [This function is used to create the boolean variables
   needed to encode a frozen variables. Both ADD and BDD will be
   created, to represent the varriable.
   The variable will be created at the given index.
   Minterms arrays will be updated, as well as ADD cubes.]

   SideEffects        []

   SeeAlso            [bdd_enc_add_input_var, bdd_enc_add_state_var]

******************************************************************************/
static void bdd_enc_add_frozen_var(BddEnc_ptr self, node_ptr name, int index)
{
  add_ptr add;

  /* not already allocated */
  nusmv_assert(bdd_enc_is_index_not_allocated(self, index));

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "BddEnc: creating frozen variable ");
    print_node(nusmv_stderr, name);
    fprintf(nusmv_stderr, "\n  BDD variable index %d, level %d\n",
            index, dd_get_level_at_index(self->dd, index));
  }

  add = add_new_var_with_index(self->dd, index);
  BddEncCache_new_boolean_var(self->cache, name, add);

  /* add to cube */
  bdd_enc_accumulate_frozen_var_cube(self, add);
  add_free(self->dd, add);

  /* add to minterm */
  bdd_enc_add_frozen_var_to_minterm(self, index);

  /* add association: name <-> index */
  array_insert(node_ptr, self->index2name, index, name);
  insert_assoc(self->name2index, name, NODE_FROM_INT(index));

  /* add association: current <-> next */
  array_insert(int, self->current2next, index, index);
  array_insert(int, self->next2current, index, index);

  self->frozen_vars_num += 1;

  /* cleans up a set of cached values: */
  BDD_ENC_FREE_ADD(self->state_frozen_vars_mask_add);
  BDD_ENC_FREE_ADD(self->state_frozen_input_vars_mask_add);
  BDD_ENC_FREE_BDD(self->state_frozen_vars_mask_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_input_vars_mask_bdd);
}


/**Function********************************************************************

   Synopsis [Adds an input variable to the arrays necessary to extract
   minterms from a BDD containing either both inputs and states, or
   inputs only.]

   Description [Adds an input variable to the arrays necessary to
   extract minterms from a BDD containing either both inputs and
   states, or inputs only. The minterms arrays are assumed to be always
   compacted, that means that its size directly depends on the number
   of vars currently allocated.

   index is the same index that has been used to create the
   corresponding ADD, typically returned by the method
   get_avail_input_var_index]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bdd_enc_add_input_var_to_minterm(BddEnc_ptr self, int index)
{
  bdd_ptr bdd;

  nusmv_assert(index > 0);  /* vars indices start from 1 */

  /* index has already been checked */
  bdd = bdd_new_var_with_index(self->dd, index);

  array_insert(bdd_ptr, self->minterm_input_vars,
               self->minterm_input_vars_dim, bdd_dup(bdd));
  self->minterm_input_vars_dim += 1;

  array_insert(bdd_ptr, self->minterm_state_frozen_input_vars,
               self->minterm_state_frozen_input_vars_dim, bdd_dup(bdd));
  self->minterm_state_frozen_input_vars_dim += 1;

  bdd_free(self->dd, bdd);
}


/**Function********************************************************************

   Synopsis [Adds a state boolean variable to the arrays necessary to extract
   minterms from a BDD containing either input-state-frozen, state-frozen or state
   variables only.
   ]

   Description [The minterms arrays are assumed to be always
   compacted, that means that its size directly depends on the number
   of vars currently allocated.

   index is the same index that has been used to create the
   corresponding ADD, typically returned by the method
   get_avail_state_var_index]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bdd_enc_add_state_var_to_minterm(BddEnc_ptr self, int index)
{
  bdd_ptr bdd;

  nusmv_assert(index > 0);  /* vars indices start from 1 */

  /* index has been already checked */
  bdd = bdd_new_var_with_index(self->dd, index);

  array_insert(bdd_ptr, self->minterm_state_vars,
               self->minterm_state_vars_dim, bdd_dup(bdd));
  self->minterm_state_vars_dim += 1;

  array_insert(bdd_ptr, self->minterm_state_frozen_vars,
               self->minterm_state_frozen_vars_dim, bdd_dup(bdd));
  self->minterm_state_frozen_vars_dim += 1;

  array_insert(bdd_ptr, self->minterm_state_frozen_input_vars,
               self->minterm_state_frozen_input_vars_dim, bdd_dup(bdd));
  self->minterm_state_frozen_input_vars_dim += 1;

  bdd_free(self->dd, bdd);
}


/**Function********************************************************************

   Synopsis [Adds a next state boolean variable to the array necessary to
   extract minterms from a BDD containing next states]

   Description [Adds a next state variable to the array necessary to
   extract minterms from a BDD containing next states.
   The minterm array is assumed to be always
   compacted, that means that its size directly depends on the number
   of next state vars currently allocated.

   index is the same index that has been used to create the
   corresponding next state var ADD, typically the value returned by
   the method get_avail_state_var_index + 1 ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bdd_enc_add_next_state_var_to_minterm(BddEnc_ptr self, int index)
{
  bdd_ptr bdd;

  nusmv_assert(index > 0);  /* vars indices start from 1 */

  /* index already checked */

  /* vars indices start from 1 */
  bdd = bdd_new_var_with_index(self->dd, index);
  array_insert(bdd_ptr, self->minterm_next_state_vars,
               self->minterm_next_state_vars_dim, bdd_dup(bdd));
  self->minterm_next_state_vars_dim += 1;
  bdd_free(self->dd, bdd);
}


/**Function********************************************************************

   Synopsis [Adds a state boolean variable to the arrays necessary to extract
   minterms from a BDD containing either only frozen, state-frozen or
   state-frozen-input variables.]

   Description [The minterms arrays are assumed to be always
   compacted, that means that its size directly depends on the number
   of vars currently allocated.

   index is the same index that has been used to create the
   corresponding ADD, typically returned by the method
   get_avail_state_var_index]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bdd_enc_add_frozen_var_to_minterm(BddEnc_ptr self, int index)
{
  bdd_ptr bdd;

  nusmv_assert(index > 0);  /* vars indices start from 1 */

  /* index has been already checked */
  bdd = bdd_new_var_with_index(self->dd, index);

  array_insert(bdd_ptr, self->minterm_frozen_vars,
               self->minterm_frozen_vars_dim, bdd_dup(bdd));
  self->minterm_frozen_vars_dim += 1;

  array_insert(bdd_ptr, self->minterm_state_frozen_vars,
               self->minterm_state_frozen_vars_dim, bdd_dup(bdd));
  self->minterm_state_frozen_vars_dim += 1;

  array_insert(bdd_ptr, self->minterm_state_frozen_input_vars,
               self->minterm_state_frozen_input_vars_dim, bdd_dup(bdd));
  self->minterm_state_frozen_input_vars_dim += 1;

  bdd_free(self->dd, bdd);
}


/**Function********************************************************************

   Synopsis [Accumulates given adds representing the curr and next
   state var encoding to the internal cubes of current and next state
   vars]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void
bdd_enc_accumulate_state_var_cube(BddEnc_ptr self, add_ptr curr, add_ptr next)
{
  nusmv_assert((curr != (add_ptr) NULL) && (next != (add_ptr) NULL));

  /* current */
  BDD_ENC_FREE_BDD(self->state_vars_bdd); /* to reset the bdd */
  BDD_ENC_FREE_BDD(self->state_frozen_vars_bdd);
  if (self->state_vars_add != (add_ptr) NULL) {
    add_and_accumulate(self->dd, &(self->state_vars_add), curr);
  }
  else self->state_vars_add = add_dup(curr);

  /* next */
  BDD_ENC_FREE_BDD(self->next_state_vars_bdd); /* to reset the bdd */
  if (self->next_state_vars_add != (add_ptr) NULL) {
    add_and_accumulate(self->dd, &(self->next_state_vars_add), next);
  }
  else self->next_state_vars_add = add_dup(next);
}


/**Function********************************************************************

   Synopsis [Accumulates given adds representing the frozen var encoding
   to the internal cube of frozen vars]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void
bdd_enc_accumulate_frozen_var_cube(BddEnc_ptr self, add_ptr frozen)
{
  nusmv_assert(frozen != (add_ptr) NULL);

  BDD_ENC_FREE_BDD(self->frozen_vars_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_vars_bdd);

  if (self->frozen_vars_add != (add_ptr) NULL) {
    add_and_accumulate(self->dd, &(self->frozen_vars_add), frozen);
  }
  else self->frozen_vars_add = add_dup(frozen);
}


/**Function********************************************************************

   Synopsis [Accumulates given adds representing the input var encoding
   to the internal cube of input vars]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void
bdd_enc_accumulate_input_var_cube(BddEnc_ptr self, add_ptr input)
{
  nusmv_assert(input != (add_ptr) NULL);

  BDD_ENC_FREE_BDD(self->input_vars_bdd);

  if (self->input_vars_add != (add_ptr) NULL) {
    add_and_accumulate(self->dd, &(self->input_vars_add), input);
  }
  else self->input_vars_add = add_dup(input);
}


/**Function********************************************************************

   Synopsis [Removes the given input variable from the internal cubes
   of input vars]

   Description        [This is a service of method remove_var. DO NOT call
   this directly.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bdd_enc_remove_input_var(BddEnc_ptr self, node_ptr name)
{
  add_ptr input_add, tmp_add;
  int index;

  /* -------------------------------------- */
  /*     Cube                               */
  /* -------------------------------------- */
  input_add = BddEncCache_lookup_boolean_var(self->cache, name);
  tmp_add = add_cube_diff(self->dd, self->input_vars_add, input_add);
  BDD_ENC_FREE_ADD(self->input_vars_add);
  BDD_ENC_FREE_BDD(self->input_vars_bdd);
  self->input_vars_add = add_dup(tmp_add);
  add_free(self->dd, tmp_add);
  add_free(self->dd, input_add);

  /* -------------------------------------- */
  /*     Minterms                           */
  /* -------------------------------------- */
  index = bdd_enc_name_to_index(self, name);
  bdd_enc_remove_var_from_minterm(self, index,
                                  self->minterm_input_vars,
                                  self->minterm_input_vars_dim);

  bdd_enc_remove_var_from_minterm(self, index,
                                  self->minterm_state_frozen_input_vars,
                                  self->minterm_state_frozen_input_vars_dim);

  /* -------------------------------------- */
  /*     Gaps for index reuse               */
  /* -------------------------------------- */
  bdd_enc_insert_gap(self, index, 1);


  /* -------------------------------------- */
  /*     Cache and permutation arrays       */
  /* -------------------------------------- */
  BddEncCache_remove_boolean_var(self->cache, name);
  array_insert(node_ptr, self->index2name, index, NULL);
  array_insert(int, self->current2next, index, 0);
  array_insert(int, self->next2current, index, 0);

  /* -------------------------------------- */
  /*     Hashes                             */
  /* -------------------------------------- */
  insert_assoc(self->name2index, name, (node_ptr) NULL);

  /* -------------------------------------- */
  /*     Counters                           */
  /* -------------------------------------- */
  self->input_vars_num -= 1;

  /* -------------------------------------- */
  /*     Masks                              */
  /* -------------------------------------- */
  BDD_ENC_FREE_ADD(self->input_vars_mask_add);
  BDD_ENC_FREE_ADD(self->state_frozen_input_vars_mask_add);
  BDD_ENC_FREE_BDD(self->input_vars_mask_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_input_vars_mask_bdd);
}


/**Function********************************************************************

   Synopsis [Removes the given state variable (and the corresponding next var)
   from the internal cubes]

   Description        [This is a service of method remove_var. DO NOT call
   this directly.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bdd_enc_remove_state_var(BddEnc_ptr self, node_ptr name)
{
  add_ptr curr_add, next_add, tmp_add;
  int curr_index;
  node_ptr next_name;
  int next_index;

  next_name = bdd_enc_get_next_state_var_name(name);


  /* -------------------------------------- */
  /*     Cubes                              */
  /* -------------------------------------- */
  curr_add = BddEncCache_lookup_boolean_var(self->cache, name);
  tmp_add = add_cube_diff(self->dd, self->state_vars_add, curr_add);
  BDD_ENC_FREE_ADD(self->state_vars_add);
  BDD_ENC_FREE_BDD(self->state_vars_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_vars_bdd);
  self->state_vars_add = add_dup(tmp_add);
  add_free(self->dd, tmp_add);
  add_free(self->dd, curr_add);

  next_add = BddEncCache_lookup_boolean_var(self->cache, next_name);
  tmp_add = add_cube_diff(self->dd, self->next_state_vars_add, next_add);
  BDD_ENC_FREE_ADD(self->next_state_vars_add);
  BDD_ENC_FREE_BDD(self->next_state_vars_bdd);
  self->next_state_vars_add = add_dup(tmp_add);
  add_free(self->dd, tmp_add);
  add_free(self->dd, next_add);

  /* -------------------------------------- */
  /*     Minterms                           */
  /* -------------------------------------- */
  curr_index = bdd_enc_name_to_index(self, name);
  bdd_enc_remove_var_from_minterm(self, curr_index,
                                  self->minterm_state_vars,
                                  self->minterm_state_vars_dim);

  bdd_enc_remove_var_from_minterm(self, curr_index,
                                  self->minterm_state_frozen_vars,
                                  self->minterm_state_frozen_vars_dim);

  bdd_enc_remove_var_from_minterm(self, curr_index,
                                  self->minterm_state_frozen_input_vars,
                                  self->minterm_state_frozen_input_vars_dim);

  next_index = bdd_enc_name_to_index(self, next_name);
  nusmv_assert(array_fetch(int, self->current2next, curr_index)
               == next_index);

  bdd_enc_remove_var_from_minterm(self, next_index,
                                  self->minterm_next_state_vars,
                                  self->minterm_next_state_vars_dim);


  /* -------------------------------------- */
  /*     Gaps for index reuse               */
  /* -------------------------------------- */
  bdd_enc_insert_gap(self, curr_index, 1);
  bdd_enc_insert_gap(self, next_index, 1);

  /* -------------------------------------- */
  /*     Cache and permutation arrays       */
  /* -------------------------------------- */
  BddEncCache_remove_boolean_var(self->cache, name);
  BddEncCache_remove_boolean_var(self->cache, next_name);
  array_insert(node_ptr, self->index2name, curr_index, NULL);
  array_insert(node_ptr, self->index2name, next_index, NULL);

  array_insert(int, self->current2next, curr_index, 0);
  array_insert(int, self->current2next, next_index, 0);
  array_insert(int, self->next2current, next_index, 0);
  array_insert(int, self->next2current, curr_index, 0);

  /* -------------------------------------- */
  /*     Hashes                             */
  /* -------------------------------------- */
  insert_assoc(self->name2index, name, (node_ptr) NULL);
  insert_assoc(self->name2index, next_name, (node_ptr) NULL);


  /* -------------------------------------- */
  /*     Counters                           */
  /* -------------------------------------- */
  self->state_vars_num -= 2;

  /* -------------------------------------- */
  /*     Masks                              */
  /* -------------------------------------- */
  BDD_ENC_FREE_ADD(self->state_frozen_vars_mask_add);
  BDD_ENC_FREE_ADD(self->state_frozen_input_vars_mask_add);
  BDD_ENC_FREE_BDD(self->state_frozen_vars_mask_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_input_vars_mask_bdd);
}


/**Function********************************************************************

   Synopsis [Removes the given frozen variable from the internal cubes
   of frozen vars]

   Description        [This is a service of method remove_var. DO NOT call
   this directly.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bdd_enc_remove_frozen_var(BddEnc_ptr self, node_ptr name)
{
  add_ptr frozen_add, tmp_add;
  int index;

  /* -------------------------------------- */
  /*     Cube                               */
  /* -------------------------------------- */
  frozen_add = BddEncCache_lookup_boolean_var(self->cache, name);
  tmp_add = add_cube_diff(self->dd, self->frozen_vars_add, frozen_add);
  BDD_ENC_FREE_ADD(self->frozen_vars_add);
  BDD_ENC_FREE_BDD(self->frozen_vars_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_vars_bdd);
  self->frozen_vars_add = add_dup(tmp_add);
  add_free(self->dd, tmp_add);
  add_free(self->dd, frozen_add);

  /* -------------------------------------- */
  /*     Minterms                           */
  /* -------------------------------------- */
  index = bdd_enc_name_to_index(self, name);
  bdd_enc_remove_var_from_minterm(self, index,
                                  self->minterm_frozen_vars,
                                  self->minterm_frozen_vars_dim);

  bdd_enc_remove_var_from_minterm(self, index,
                                  self->minterm_state_frozen_vars,
                                  self->minterm_state_frozen_vars_dim);

  bdd_enc_remove_var_from_minterm(self, index,
                                  self->minterm_state_frozen_input_vars,
                                  self->minterm_state_frozen_input_vars_dim);

  /* -------------------------------------- */
  /*     Gaps for index reuse               */
  /* -------------------------------------- */
  bdd_enc_insert_gap(self, index, 1);


  /* -------------------------------------- */
  /*     Cache and permutation arrays       */
  /* -------------------------------------- */
  BddEncCache_remove_boolean_var(self->cache, name);
  array_insert(node_ptr, self->index2name, index, NULL);
  array_insert(int, self->current2next, index, 0);
  array_insert(int, self->next2current, index, 0);

  /* -------------------------------------- */
  /*     Hashes                             */
  /* -------------------------------------- */
  insert_assoc(self->name2index, name, (node_ptr) NULL);

  /* -------------------------------------- */
  /*     Counters                           */
  /* -------------------------------------- */
  self->frozen_vars_num -= 1;

  /* -------------------------------------- */
  /*     Masks                              */
  /* -------------------------------------- */
  BDD_ENC_FREE_ADD(self->state_frozen_vars_mask_add);
  BDD_ENC_FREE_ADD(self->state_frozen_input_vars_mask_add);
  BDD_ENC_FREE_BDD(self->state_frozen_vars_mask_bdd);
  BDD_ENC_FREE_BDD(self->state_frozen_input_vars_mask_bdd);
}


/**Function********************************************************************

   Synopsis           [Removes the given var from the encoder]

   Description        [A bunch of calls to this method, must be followed by
   a call to compact_minterms_array, to compact the minterms arrays]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void bdd_enc_remove_var(BddEnc_ptr self, node_ptr name)
{
  int curr_index;

  boolean is_state_var;

  /* ------------------------------ */
  /*    A few checks                */
  /* ------------------------------ */
  nusmv_assert(SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, name));

  is_state_var = SymbTable_is_symbol_state_var(BASE_ENC(self)->symb_table,
                                               name);

  curr_index = bdd_enc_name_to_index(self, name);

  /* if it is a NEXT state variable, is must still be a current var,
     i.e. next state variable is type NEXT NEXT */
  if (node_get_type(name) == NEXT && is_state_var) {
    nusmv_assert(array_fetch(int, self->next2current,
                             array_fetch(int, self->current2next, curr_index))
                 == curr_index);
  }
  /* ------------------------------ */
  /*    The actual removal          */
  /* ------------------------------ */
  if (is_state_var) bdd_enc_remove_state_var(self, name);
  else if (SymbTable_is_symbol_input_var(BASE_ENC(self)->symb_table, name)) {
    bdd_enc_remove_input_var(self, name);
  }
  else {
    nusmv_assert(SymbTable_is_symbol_frozen_var(BASE_ENC(self)->symb_table, name));
    bdd_enc_remove_frozen_var(self, name);
  }
}


/**Function********************************************************************

   Synopsis           [Searches linearly within the given array (up to
   the given limit) for the given var index, and remove it]

   Description        [The array must be compacted after all the vars have
   been removed]

   SideEffects        []

   SeeAlso            [bdd_enc_compact_minterms_array]

******************************************************************************/
static void bdd_enc_remove_var_from_minterm(BddEnc_ptr self, int var_idx,
                                            array_t* minterms_array,
                                            int minterms_array_len)
{
  int i;

  for (i=0; i < minterms_array_len; ++i) {
    if (array_fetch(bdd_ptr, minterms_array, i) == (bdd_ptr) NULL) continue;
    if (var_idx == bdd_index(self->dd,
                             array_fetch(bdd_ptr, minterms_array, i))) {
      bdd_free(self->dd, array_fetch(bdd_ptr, minterms_array, i));
      array_insert(bdd_ptr, minterms_array, i, NULL);
      break;
    }
  }
}


/**Function********************************************************************

   Synopsis [Builds the name of the next state variable corresponding
   to the current state variable name passed as an argument.]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static node_ptr
bdd_enc_get_next_state_var_name(node_ptr name)
{
  return find_node(NEXT, name, Nil);
}


/**Function********************************************************************

   Synopsis    [Private service of method sort_variables_and_groups, that
   takes a list of scalar layers and appends all the variables
   declared within them to the returned list.]

   Description [Creates a list of all the variables
   declared within the given layers, that cannot be layers deriving
   from the booleanization process. Iterates on the variables declared
   within the given layers. If a variable is boolean, it is appended to
   the list as it is; if the variable is scalar, its bits are taken and
   pushed in the order they occur.

   'layers' is a list of SymbLayer_ptr.

   The returned list will be ordered accordingly to the value of
   system variable 'vars_order_type'. If bit-interleaving is
   involved, it is applied among variables in all layers. In this
   case output parameter layers_interleaved gets assigned to true,
   but only of there are two or more layers involved.

   It is the responsibility of an invoker to free the returned list.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static NodeList_ptr
bdd_enc_append_bool_vars_from_layers(BddEnc_ptr self, NodeList_ptr layers,
                                     boolean* layers_interleaved)
{
  NodeList_ptr result = NodeList_create();
  node_ptr sorting_cache = Nil; /* used only if bit-interleaving is set up */

  SymbTable_ptr symb_table = BASE_ENC(self)->symb_table;
  ListIter_ptr layer_iter;
  hash_ptr cache = new_assoc();
  boolean isBitInterleaved = false;

  *layers_interleaved = false;

  /* iterate over layers */
  NODE_LIST_FOREACH(layers, layer_iter) {
    SymbTableType types_array[3];
    int i;
    SymbLayer_ptr layer = SYMB_LAYER(NodeList_get_elem_at(layers, layer_iter));

    /* the layer has to be a scalar layer, i.e. there must exist a
       boolean layer for it. */
    nusmv_assert(SymbTable_get_layer(symb_table,
                       BoolEnc_scalar_layer_to_bool_layer(
                              SymbLayer_get_name(layer))) != SYMB_LAYER(NULL));


    switch (get_vars_order_type(OptsHandler_get_instance())) {
    case VARS_ORD_INPUTS_BEFORE_BI:
      isBitInterleaved = true;
      /* no break here */
    case VARS_ORD_INPUTS_BEFORE:
      types_array[0] = STT_INPUT_VAR;
      types_array[1] = STT_STATE_VAR | STT_FROZEN_VAR;
      types_array[2] = STT_NONE;
      break;

    case VARS_ORD_INPUTS_AFTER_BI:
      isBitInterleaved = true;
      /* no break here */
    case VARS_ORD_INPUTS_AFTER:
      types_array[0] = STT_STATE_VAR | STT_FROZEN_VAR;
      types_array[1] = STT_INPUT_VAR;
      types_array[2] = STT_NONE;
      break;

    case VARS_ORD_TOPOLOGICAL_BI:
      isBitInterleaved = true;
      /* no break here */
    case VARS_ORD_TOPOLOGICAL:
      types_array[0] = STT_VAR;
      types_array[1] = STT_NONE;
      break;

    default:
      error_unreachable_code(); /* no other possible cases */
    }

    i = 0;
    while (types_array[i] != STT_NONE) {
      SymbLayerIter iter;

      SYMB_LAYER_FOREACH(layer, iter, types_array[i]) {
        node_ptr var = SymbLayer_iter_get_symbol(layer, &iter);

        if (SymbTable_is_symbol_bool_var(symb_table, var)) {
          if (isBitInterleaved) {
            Enc_append_bit_to_sorted_list(symb_table, result,
                                          var, &sorting_cache);
          }
          else NodeList_append(result, var);   /* pushes it as it is */
        }
        else {
          /* Skip variables that are not booleanizable. */
          if (Compile_is_expr_booleanizable(symb_table, var,
               !opt_bdd_encoding_word_bits(OptsHandler_get_instance()),
               cache)) {
            /* pushes the bits composing the scalar variable.
               Grouping is done by caller function */
            NodeList_ptr bits = BoolEnc_get_var_bits(
                   BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self)), var);
            ListIter_ptr bit_iter;
            NODE_LIST_FOREACH(bits, bit_iter) {
              node_ptr var = NodeList_get_elem_at(bits, bit_iter);
              if (isBitInterleaved) {
                Enc_append_bit_to_sorted_list(symb_table, result,
                                              var, &sorting_cache);
              }
              else NodeList_append(result, var);   /* pushes it as it is */
            }
            NodeList_destroy(bits);
          }
        }
      } /* loop on vars */

      i += 1;
    } /* outer loop */
  }

  free_list(sorting_cache);
  free_assoc(cache);

  if (NodeList_get_length(layers) > 1) *layers_interleaved = isBitInterleaved;
  return result;
}


/**Function********************************************************************

   Synopsis    [Private service of method BddEnc_get_vars_cube, that
   takes a set of varioables and returns their booleanization]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static NodeList_ptr
bdd_enc_get_var_booleanizations(BddEnc_ptr self, Set_t vars)
{
  NodeList_ptr result;
  SymbTable_ptr symb_table;
  Set_Iterator_t viter;
  hash_ptr cache = new_assoc();

  result = NodeList_create();
  symb_table = BASE_ENC(self)->symb_table;

  SET_FOREACH(vars, viter) {
    node_ptr var;

    var = Set_GetMember(vars, viter);

    if (SymbTable_is_symbol_bool_var(symb_table, var)) {
      NodeList_append(result, var);   /* pushes it as it is */
    }
    else {
      if (Compile_is_expr_booleanizable(symb_table, var, false, cache)) {
        NodeList_ptr bits;
        ListIter_ptr bit_iter;

        bits = BoolEnc_get_var_bits(BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self)),
                                    var);

        NODE_LIST_FOREACH(bits,bit_iter) {
          node_ptr bvar;

          bvar = NodeList_get_elem_at(bits, bit_iter);
          NodeList_append(result, bvar);   /* pushes it as it is */
        }
        NodeList_destroy(bits);
      }
    }
  }

  free_assoc(cache);
  return result;
}


/**Function********************************************************************

   Synopsis [Returns the ordered list of variables, and a groups set of
   variables that must be created]

   Description [It takes an OrdGroup representing a partial order to be
   followed, and build a new OrdGroup and the ordered list of
   variables. The caller is responsible for the disposal of the
   returned list. Output parameter needs_reshuffle is set to false if a
   reshuffle is _not_ needed because the ordering will not change from
   the default (implemented for the sake of performances). res_group
   will contain the groups of the variables that will have to be
   created, whereas the returned list is the 'flat' version
   (i.e. without grouping information) of those variables.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static NodeList_ptr
bdd_enc_sort_variables_and_groups_according(BddEnc_ptr self,
                                            OrdGroups_ptr old_group,
                                            OrdGroups_ptr res_group,
                                            boolean* interleaved)
{
  const char* bool_name;
  NodeList_ptr all_po_vars, res_ord_vars;
  ListIter_ptr iter;
  NodeList_ptr layer_list;

  BDD_ENC_CHECK_INSTANCE(self);

  /* This variable is used to control reshuffle ouput parameter */
  res_ord_vars = NodeList_create();

  layer_list = NodeList_create();

  /* fills the partially ordered list of all variables that had been
     committed so far, in order to return the fully ordered list */
  for (iter = NodeList_get_first_iter(BASE_ENC(self)->committed_layers);
      !ListIter_is_end(iter);
      iter = ListIter_get_next(iter)) {
    SymbLayer_ptr com_layer
      = SYMB_LAYER(NodeList_get_elem_at(BASE_ENC(self)->committed_layers, iter));

    /* checks that it is not a layer deriving from the booleanization
       process: get the name that the boolean layer would have, and if
       that boolean layer exists, than com_layer is not a boolean layer */
    bool_name = BoolEnc_scalar_layer_to_bool_layer(SymbLayer_get_name(com_layer));
    if (SymbTable_get_layer(BASE_ENC(self)->symb_table, bool_name) !=
        SYMB_LAYER(NULL)) {
      /* layer has an associated boolean layer, so it is a scalar layer */
      NodeList_append(layer_list, (node_ptr)com_layer);
    }
  }
  all_po_vars = bdd_enc_append_bool_vars_from_layers(self, layer_list,
                                                     interleaved);

  /* creates the final ordered list, taking into account the ordering
     groups, and generates a new groups set to be returned, and to be
     used later to group the variables that come within
     group_layers */
  {
    int curr_group = 0;
    BoolEnc_ptr bool_enc;
    bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self));
    node_ptr name, var_name;
    int group, gr_id, new_group_id;
    NodeList_ptr gr_vars;
    ListIter_ptr gr_iter;

    for (iter = NodeList_get_first_iter(all_po_vars);
        !ListIter_is_end(iter);
        iter = ListIter_get_next(iter)) {

      name = NodeList_get_elem_at(all_po_vars, iter);

      /* check whether it is a grouped var */
      group = OrdGroups_get_var_group(old_group, name);

      if (-1 == group) {
        /* pushes the variable within the result list */
        if (!NodeList_belongs_to(res_ord_vars, name)) {
          NodeList_append(res_ord_vars, name);
        }
      }
      else {
        /*
         * it is grouped, insert all the previous not inserted groups
         * up to the group that has been just found
         */
        for (gr_id = curr_group; gr_id <= group; ++gr_id) {
          /*
           * inserts all the existing variables that occur within the
           * group, in the same order they occur, but only if they are
           * existing variables
           *
           * a new group is created within the result group, but only
           * if it contains at least one variable
           */
          new_group_id = -1;

          gr_vars = OrdGroups_get_vars_in_group(old_group, gr_id);

          for (gr_iter = NodeList_get_first_iter(gr_vars);
              !ListIter_is_end(gr_iter);
              gr_iter = ListIter_get_next(gr_iter)) {

            var_name = NodeList_get_elem_at(gr_vars, gr_iter);

            if (NodeList_belongs_to(all_po_vars, var_name) &&
                !NodeList_belongs_to(res_ord_vars, var_name)) {

              NodeList_append(res_ord_vars, var_name);
              /*
               * it is also a variable that must be created. Creates
               * a new group if not already created, and add the
               * variable to that group
               */
              if (new_group_id == -1) {
                new_group_id = OrdGroups_create_group(res_group);
              }
              OrdGroups_add_variable(res_group, var_name, new_group_id);
            } /* if (NodeList_belongs_to(all_po_vars, var_name) &&
                 !NodeList_belongs_to(res_ord_vars, var_name)) */
          } /* loop on vars in the group */
        } /* for all groups up to the current one */

        curr_group = group + 1; /* passes to the following group */
      } /* if (-1 == group) */

    } /* main loop on variables */
  }

  /* disposes lists */
  NodeList_destroy(layer_list);
  NodeList_destroy(all_po_vars);

  return res_ord_vars;
}

/**Function********************************************************************

   Synopsis [Returns the ordered list of variables, and a groups set of
   variables that must be created]

   Description [The given layer (and the associated boolean layer if
   exists) contains the variables that must be created. As this method is
   called when a layer is committed, the parameter layer represent just
   the layer that is being committed. The caller is responsible for the
   disposal of the returned list. Output parameter needs_reshuffle is
   set to false if a reshuffle is _not_ needed because the ordering
   will not change from the default (implemented for the sake of
   performances). res_group will contain the groups of the variables
   that will have to be created, whereas the returned list is the
   'flat' version (i.e. without grouping information) of those
   variables.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static NodeList_ptr
bdd_enc_sort_variables_and_groups(BddEnc_ptr self, SymbLayer_ptr layer,
                                  OrdGroups_ptr res_group,
                                  boolean* needs_reshuffle,
                                  boolean* interleaved)
{
  NodeList_ptr new_po_vars;
  NodeList_ptr all_po_vars;
  NodeList_ptr res_ord_vars = NodeList_create();
  ListIter_ptr iter;

  NodeList_ptr layer_list = NodeList_create();

  /* This variable is used to control reshuffle ouput parameter */
  enum { needs_reshuffle_wait,
         needs_reshuffle_yes,
         needs_reshuffle_no } reshuffle = needs_reshuffle_wait;

  NodeList_append(layer_list, (node_ptr)layer);

  new_po_vars = bdd_enc_append_bool_vars_from_layers(self, layer_list,
                                                     interleaved);
  nusmv_assert(!(*interleaved)); /* there is only one layer! */

  NodeList_remove_elem_at(layer_list, NodeList_get_first_iter(layer_list));

  /* fills the partially ordered list of all variables that had been
     committed so far, in order to return the fully ordered list */
  iter = NodeList_get_first_iter(BASE_ENC(self)->committed_layers);
  while (!ListIter_is_end(iter)) {
    const char* bool_name;
    SymbLayer_ptr com_layer =
      SYMB_LAYER(NodeList_get_elem_at(BASE_ENC(self)->committed_layers, iter));

    /* checks that it is not a layer deriving from the booleanization
       process: get the name that the boolean layer would have, and if
       that boolean layer exists, than com_layer is not a boolean layer */
    bool_name =
      BoolEnc_scalar_layer_to_bool_layer(SymbLayer_get_name(com_layer));

    if (SymbTable_get_layer(BASE_ENC(self)->symb_table, bool_name) !=
        SYMB_LAYER(NULL)) {
      /* layer has an associated boolean layer, so it is a scalar layer */
      NodeList_append(layer_list, (node_ptr)com_layer);
    }

    iter = ListIter_get_next(iter);
  }

  all_po_vars = bdd_enc_append_bool_vars_from_layers(self, layer_list,
                                                     interleaved);

  /* creates the final ordered list, taking into account the ordering
     groups, and generates a new groups set to be returned, and to be
     used later to group the variables that come within
     group_layers */
  {
    int curr_group = 0;
    node_ptr curr_scalar_var = Nil;
    int curr_scalar_group = -1;
    BoolEnc_ptr bool_enc;
    bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self));

    iter = NodeList_get_first_iter(all_po_vars);
    while (!ListIter_is_end(iter)) {
      node_ptr name;
      int group;

      name = NodeList_get_elem_at(all_po_vars, iter);

      /* check whether it is a grouped var */
      group = OrdGroups_get_var_group(self->ord_groups, name);
      if (group == -1) {
        /* pushes the variable within the result list */
        if (!NodeList_belongs_to(res_ord_vars, name)) {
          NodeList_append(res_ord_vars, name);
          if (NodeList_belongs_to(new_po_vars, name)) {

            if (BoolEnc_is_var_bit(bool_enc, name)) {
              node_ptr scalar_name = BoolEnc_get_scalar_var_from_bit(bool_enc, name);
              if (curr_scalar_var == Nil || scalar_name != curr_scalar_var) {
                curr_scalar_group = OrdGroups_create_group(res_group);
                curr_scalar_var = scalar_name;
              }
            }
            else {
              curr_scalar_group = OrdGroups_create_group(res_group);
              curr_scalar_var = Nil;
            }

            /* it is an ungrouped variable to be also created. Add a
               new stand alone group to the result group, or use the
               same group for ungroupped bits of scalar vars */
            nusmv_assert(curr_scalar_group >= 0);
            OrdGroups_add_variable(res_group, name, curr_scalar_group);
            if (reshuffle == needs_reshuffle_wait) reshuffle = needs_reshuffle_no;
          }
          else { /* it is not a new var */
            if (reshuffle == needs_reshuffle_no) reshuffle = needs_reshuffle_yes;
          }
        }
      }
      else {
        /* it is grouped, insert all the previous not inserted groups
           up to the group that has been just found */
        int gr_id;
        for (gr_id = curr_group; gr_id <= group; ++gr_id) {
          /* inserts all the existing variables that occur within
             the group, in the same order they occur, but only if
             they are existing variables */
          NodeList_ptr gr_vars;
          ListIter_ptr gr_iter;
          /* a new group is created within the result group, but
             only if it contains at least one variable */
          int new_group_id = -1;

          gr_vars = OrdGroups_get_vars_in_group(self->ord_groups, gr_id);
          gr_iter = NodeList_get_first_iter(gr_vars);
          while (!ListIter_is_end(gr_iter)) {
            node_ptr var_name = NodeList_get_elem_at(gr_vars, gr_iter);

            if (NodeList_belongs_to(all_po_vars, var_name) &&
                !NodeList_belongs_to(res_ord_vars, var_name)) {
              NodeList_append(res_ord_vars, var_name);
              if (NodeList_belongs_to(new_po_vars, var_name)) {
                /* it is also a variable that must be created. Creates
                   a new group if not already created, and add the
                   variable to that group */
                if (new_group_id == -1) {
                  new_group_id = OrdGroups_create_group(res_group);
                }
                OrdGroups_add_variable(res_group, var_name, new_group_id);

                if (reshuffle == needs_reshuffle_wait) {
                  reshuffle = needs_reshuffle_no;
                }
              }
              else {
                /* not a new var */
                if (reshuffle == needs_reshuffle_no) {
                  reshuffle = needs_reshuffle_yes;
                }
              }
            }

            gr_iter = ListIter_get_next(gr_iter);
          } /* loop on vars in the group */
        } /* for all groups up to the current one */

        curr_group = group + 1; /* passes to the following group */
      }

      iter = ListIter_get_next(iter);
    } /* main loop on variables */
  }

  /* disposes lists */
  NodeList_destroy(layer_list);
  NodeList_destroy(new_po_vars);
  NodeList_destroy(all_po_vars);

  *needs_reshuffle = (reshuffle == needs_reshuffle_yes);

  return res_ord_vars;
}


/**Function********************************************************************

   Synopsis           [Compacts the given array (i.e. intermediate
   gaps are removed)]

   Description        [Returns the actual length of the compacted array]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int bdd_enc_compact_minterms_array(BddEnc_ptr self,
                                          array_t* minterms_array,
                                          int minterms_array_len)
{
  int ofs = 0;
  int i;

  for (i=0; i < minterms_array_len; ++i) {
    if (array_fetch(bdd_ptr, minterms_array, i) == (bdd_ptr) NULL) ++ofs;
    else if (ofs > 0) {
      array_insert(bdd_ptr, minterms_array, i-ofs,
                   array_fetch(bdd_ptr, minterms_array, i));
      array_insert(bdd_ptr, minterms_array, i, NULL);
    }
  }

  return minterms_array_len - ofs;
}


/**Function********************************************************************

   Synopsis           [If dynamic reordering is enabled, it is temporarily
   disabled]

   Description        [If dynamic reordering is enabled, it is temporarily
   disabled]

   SideEffects        []

   SeeAlso            [bdd_enc_unlock_reordering]

******************************************************************************/
static void bdd_enc_lock_reordering(BddEnc_ptr self)
{
  if (self->reord_locked_num == 0) {
    self->reord_status = dd_reordering_status(self->dd,
                                              &(self->curr_reord_type));
    if (self->reord_status == 1) { dd_autodyn_disable(self->dd); }
  }

  self->reord_locked_num += 1;
}


/**Function********************************************************************

   Synopsis [Re-enable a previously disabled reordering, if it was
   enabled when locked.]

   Description        []

   SideEffects        []

   SeeAlso            [bdd_enc_lock_reordering]

******************************************************************************/
static void bdd_enc_unlock_reordering(BddEnc_ptr self)
{
  nusmv_assert(self->reord_locked_num > 0); /* locked at least once */

  self->reord_locked_num -= 1;
  if ((self->reord_locked_num == 0) && (self->reord_status == 1)) {
    dd_autodyn_enable(self->dd, self->curr_reord_type);
  }
}


/**Function********************************************************************

   Synopsis           [Inserts a new gap within the list of gaps]

   Description        [This method guarantees that the list of gaps
   is ordered]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void
bdd_enc_insert_gap(BddEnc_ptr self, int index, size_t gap_size)
{
  /* searches the insertion point within the list of gaps: */
  ListIter_ptr iter;
  ListIter_ptr prev = LIST_ITER(NULL);
  boolean inserted = false;

  iter = NodeList_get_first_iter(self->index_gaps);
  while (!ListIter_is_end(iter)) {
    node_ptr el = NodeList_get_elem_at(self->index_gaps, iter);
    int iter_idx = NODE_TO_INT(car(el));
    if (index < iter_idx) {
      if (prev != LIST_ITER(NULL)) { /* not to the head */
        int prev_idx, prev_size;
        el = NodeList_get_elem_at(self->index_gaps, prev);
        prev_idx = NODE_TO_INT(car(el));
        prev_size = NODE_TO_INT(cdr(el));
        /* checks whether this group can be 'attached' to an existing one */
        nusmv_assert(index >= prev_idx + prev_size); /* no overlap */
        if (index == prev_idx + prev_size) {
          /* attaches to the previous gap */
          setcdr(el, NODE_FROM_INT(prev_size + gap_size));
        }
        else {
          /* creates a new gaps group */
          NodeList_insert_after(self->index_gaps, prev,
                                cons(NODE_FROM_INT(index),
                                     NODE_FROM_INT(gap_size)));
        }
      }
      else { /* to the head */
        nusmv_assert(index + gap_size <= iter_idx); /* no overlap */
        if (index + gap_size == iter_idx) {
          /* reuses the existing first node */
          setcar(el, NODE_FROM_INT(index));
          setcdr(el, NODE_FROM_INT(NODE_TO_INT(cdr(el)) + gap_size));
        }
        else {
          /* prepend a new gap */
          NodeList_prepend(self->index_gaps,
                           cons(NODE_FROM_INT(index), NODE_FROM_INT(gap_size)));
        }
      }
      inserted = true;
      break;
    }

    prev = iter;
    iter = ListIter_get_next(iter);
  } /* while loop */

  if (!inserted) { /* to the tail */
    if (prev != LIST_ITER(NULL)) {
      node_ptr el = NodeList_get_elem_at(self->index_gaps, prev); /* last */
      int prev_idx = NODE_TO_INT(car(el));
      int prev_size = NODE_TO_INT(cdr(el));
      nusmv_assert(index >= prev_idx+prev_size); /* no overlap */
      if (index == prev_idx+prev_size) {
        /* attaches to the last gap */
        setcdr(el, NODE_FROM_INT(prev_size + gap_size));
      }
      else {
        NodeList_append(self->index_gaps,
                        cons(NODE_FROM_INT(index), NODE_FROM_INT(gap_size)));
      }
    }
    else { /* the list was empty */
      NodeList_append(self->index_gaps,
                      cons(NODE_FROM_INT(index), NODE_FROM_INT(gap_size)));
    }
  }

}


/**Function********************************************************************

   Synopsis [Removes a previoulsy inserted gap from the list of gaps]

   Description [This method guarantees that the list of gaps keeps its
   order. If found, a gap might be either removed, reduced in size, or
   splitted into to smaller gaps. If the gap is not found, an internal
   error occurs.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static void
bdd_enc_remove_gap(BddEnc_ptr self, int index, size_t size)
{
  /* searches the removal point within the list of gaps: */
  ListIter_ptr iter;
  NODE_LIST_FOREACH(self->index_gaps, iter) {
    node_ptr el;
    int gap_idx, gap_size;

    el = NodeList_get_elem_at(self->index_gaps, iter);
    gap_idx = NODE_TO_INT(car(el));
    gap_size = NODE_TO_INT(cdr(el));

    if ((index >= gap_idx) && (index < gap_idx+gap_size)) {
      /* found the gap to be affected */
      nusmv_assert(size <= gap_idx+gap_size-index); /* enough space */

      if (index == gap_idx && size == gap_size) {
        /* removes the whole gap */
        node_ptr el;
        el = NodeList_remove_elem_at(self->index_gaps, iter);
        free_node(el);

        return;
      }

      if (index == gap_idx) { /* at the begin */
        setcdr(el, NODE_FROM_INT(gap_size - size));
        setcar(el, NODE_FROM_INT(gap_idx + size));
      }
      else if (index + size == gap_idx + gap_size) { /* at the end */
        setcdr(el, NODE_FROM_INT(gap_size - size));
      }
      else { /* in the middle, splits the gap */
        setcdr(el, NODE_FROM_INT(index - gap_idx)); /* left gap adjust size */

        /* adds a new gap */
        NodeList_insert_after(self->index_gaps, iter,
                              cons(NODE_FROM_INT(index+size),
                                   NODE_FROM_INT(gap_size - (index+size-gap_idx))));
      }
      return;
    }
  } /* for all gaps in self->index_gaps */

  internal_error("bdd_enc_remove_gap: gap index=%d size=%d not found",
                 index, size); /* gap not found */
}


/**Function********************************************************************

   Synopsis           [Given a var name, it returns the corresponding index
   in the dd manager]

   Description        [An assertion will occur whether the given name does not
   correspond to any variable. If name is a next variable, then it is checked
   that a corresponding current state var exists. If it does not, again an
   assertion will be thrown.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static int bdd_enc_name_to_index(const BddEnc_ptr self, node_ptr name)
{
  int index;

  index = NODE_TO_INT(find_assoc(self->name2index, name));
  nusmv_assert(index != 0);

  if (node_get_type(name) == NEXT) {
    /* a next var, checks that a corresponding current var has been added,
       and that each correspond to the other */
    int curr_idx = NODE_TO_INT(find_assoc(self->name2index, car(name)));
    nusmv_assert(curr_idx != 0);
    nusmv_assert((array_fetch(int, self->current2next, curr_idx) == index) &&
                 (array_fetch(int, self->next2current, index) == curr_idx));
  }

  return index;
}



/**Function********************************************************************

   Synopsis           [Private service]

   Description        [Used when destroying node list containing node_ptr]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static assoc_retval hash_node_free(char* key, char* data, char* arg)
{
  node_ptr element = (node_ptr) data;

  if (element != Nil) { free_node(element); }
  return ASSOC_DELETE;
}



/**Function********************************************************************

   Synopsis           [Private service]

   Description        [Used when destroying hash containing add_ptr as key]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static assoc_retval hash_add_key_free(char* key, char* data, char* arg)
{
  add_ptr add = (add_ptr) key;
  DdManager* dd = (DdManager*) arg;

  if (add != (add_ptr) NULL) { add_free(dd, add); }
  return ASSOC_DELETE;
}



/**Function********************************************************************

   Synopsis    [Given a list of variables, it returns the corresponding
   mask]

   Description        [Private service used by higher level mask-related
   methods]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static add_ptr bdd_enc_get_vars_list_mask(BddEnc_ptr self, const SymbLayer_ptr layer,
                                          SymbLayerIter* iter)
{
  add_ptr mask;

  mask = add_true(self->dd);

  while (!SymbLayer_iter_is_end(layer, iter)) {
    add_ptr var_mask;
    node_ptr var = SymbLayer_iter_get_symbol(layer, iter);
    var_mask = BddEnc_get_var_mask(self, var);
    add_and_accumulate(self->dd, &mask, var_mask);
    add_free(self->dd, var_mask);

    SymbLayer_iter_next(layer, iter);
  }

  return mask;
}


/**Function********************************************************************

   Synopsis           [Calculates the mask of the var encoding passed as
   argument]

   Description        [Private method that recursively calculates the mask of
   the var encoding passed as argument. Passed cube is the cube of
   the encoding, used while visiting the encoding, in order to find
   indexes of sub-variable. Returned add is referenced, and the
   automatic reordering *must* be disabled before calling this
   method. ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static add_ptr
bdd_enc_get_var_mask_recur(BddEnc_ptr self, add_ptr var_encoding, add_ptr cube)
{
  add_ptr res;

  if (add_isleaf(cube))  {
    /* base step: visit is over, exits: */
    nusmv_assert(add_isleaf(var_encoding));
    res = add_true(self->dd);
  }
  else {
    /* inductive step, evaluates whether it is a constant or a var: */

    if (add_isleaf(var_encoding)) {
      /* It is a constant: fills the remaining empty nodes */
      add_ptr t;
      add_ptr zero = add_false(self->dd);

      t = bdd_enc_get_var_mask_recur(self, var_encoding,
                                     add_then(self->dd, cube));
      nusmv_assert(t != (add_ptr) NULL);

      res = add_build(self->dd, add_index(self->dd, cube), zero, t);
      nusmv_assert(res != (add_ptr) NULL);

      add_free(self->dd, t);
      add_free(self->dd, zero);
    }
    else {
      /* it is a variable, keep visiting the dag, searching for gaps
         to fill */
      add_ptr t, e;

      t =  bdd_enc_get_var_mask_recur(self,
                                      add_then(self->dd, var_encoding),
                                      add_then(self->dd, cube));
      nusmv_assert(t != (add_ptr) NULL);

      e =  bdd_enc_get_var_mask_recur(self,
                                      add_else(self->dd, var_encoding),
                                      add_then(self->dd, cube));
      nusmv_assert(e != (add_ptr) NULL);

      res = add_build(self->dd, add_index(self->dd, cube), t, e);
      nusmv_assert(res != (add_ptr) NULL);

      add_free(self->dd, e);
      add_free(self->dd, t);
    }
  }

  return res;
}


/**Function********************************************************************

   Synopsis           [Given an expression the corresponding hybrid ADD is
   returned back.]

   Description        [This function takes an expression in input and
   returns the corresponding list of ADD. This list may contain several
   elements for Word expressions, but only one element for all other
   kinds of expressions.<p>

   If a domain variables is received as input, the returned value is
   the ADD representing its boolean encoding. It has as
   leaves the value associated to that path.<br>
   For instance consider the declaration:<br>
   <code>VAR x : 1..6;</code><br>
   it is encoded with three boolean variables as below:
   <pre>

   x1
   /\\
   T/  \\F
   /    \\
   /      \\
   x2       x2
   /\\        / \\
   /  \\       |  \\
   x3    \\      x3  \\
   /  \\   \\    /  \\  \\
   /    \\  \\   /    \\  \\
   1    5    3   2     6   4

   </pre>

   A Word variable is encoded as a list of its boolean elements.<br>
   For example:<br>
   <code>VAR x : word[3];</code><br>
   is encoded with three boolean variables and represented as a list of
   three ADD as below:
   <pre>
   x2        x1         x0
   / \\      / \\       / \\
   F   T     F   T      F   T

   </pre>

   If the expression is complex, then the function is recursively applied to the
   operands, and then the operation is applied to the obtained values, returning
   the resulting (hybrid) ADD. All returned ADD are referenced]

   SideEffects        []

   SeeAlso            [eval_recur, AddArray_ptr]

******************************************************************************/
static AddArray_ptr bdd_enc_eval(BddEnc_ptr self, Expr_ptr expr,
                                 node_ptr context)
{
  AddArray_ptr res = NULL;
  node_ptr hash_entry;

  if (expr == Nil) return AddArray_from_add(add_true(self->dd));

  hash_entry = find_node(CONTEXT, context, expr);
  res = BddEncCache_get_evaluation(self->cache, hash_entry);

  if ((res == (AddArray_ptr) NULL) || (res == BDD_ENC_EVALUATING)) {
    /* debugging code */
    if (res == BDD_ENC_EVALUATING) {
      rpterr("WARNING : THE SAME EXPR IS EVALUATED TWICE!\n");
    }
    int temp = yylineno;
    yylineno = node_get_lineno(expr);
    res = bdd_enc_eval_recur(self, expr, context);
    yylineno = temp;

    /* inserts the evaluated add array in the cache */
    BddEncCache_set_evaluation(self->cache, hash_entry, AddArray_duplicate(res));
  }

  return res;
}


/**Function********************************************************************

   Synopsis           [Performs the <code>eval</code>
   function on an atom-expression.]

   Description        [The atom-expression <code>expr</code> is
   compiled in ADD:
   <ul>
   <li> If it is a program parameter, then its actual name is
   extracted from the parameter hash, and it is evaluated. The
   result of evaluation is returned.</li>
   <li> If it is a constant, then the corresponding ADD is returned
   back.</li>
   <li> If it is a variable, then it is converted to node_ptr
   binary encoding and then the corresponding ADD is returned.
   back.</li>
   </ul>
   ]

   SideEffects        []

   SeeAlso            [eval get_definition]

******************************************************************************/
static AddArray_ptr
bdd_enc_eval_recur_case_atom(BddEnc_ptr self, Expr_ptr expr, node_ptr ctx)

{
  ResolveSymbol_ptr rs;
  node_ptr resName;
  node_ptr param = (node_ptr) NULL;
  add_ptr constant = (add_ptr)NULL;

  rs = SymbTable_resolve_symbol(BASE_ENC(self)->symb_table, expr, ctx);
  resName = ResolveSymbol_get_resolved_name(rs);

  /* If any error happened, throw it. */
  if (ResolveSymbol_is_error(rs)) ResolveSymbol_throw_error(rs);

  if (ResolveSymbol_is_constant(rs)) {
    constant = BddEncCache_lookup_constant(self->cache, resName);
  }
  if (ResolveSymbol_is_parameter(rs)) {
    param = SymbTable_get_flatten_actual_parameter(BASE_ENC(self)->symb_table,
                                                   resName);
  }

  /* the atom is a parameter */
  if (param != (node_ptr) NULL) {
    /* no need to free 'constant' because it must be zero here */
    nusmv_assert(constant == (add_ptr) NULL);

    return bdd_enc_eval(self, param, ctx);
  }

  /* the atom is a constant */
  if (constant != (add_ptr) NULL) {
    return AddArray_from_add(constant); /*already referenced*/
  }

  return bdd_enc_eval_recur_case_dot_array(self, expr, ctx);
}


/**Function********************************************************************

   Synopsis           [Performs the <code>eval</code>
   function on an atom-expression representing a variable, define or constant
   representing the name of a process]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static AddArray_ptr
bdd_enc_eval_recur_case_dot_array(BddEnc_ptr self, Expr_ptr expr, node_ptr ctx)
{
  ResolveSymbol_ptr rs;
  node_ptr name;
  AddArray_ptr res;

  rs = SymbTable_resolve_symbol(BASE_ENC(self)->symb_table, expr, ctx);
  name = ResolveSymbol_get_resolved_name(rs);

  /* here array-expressions cannot be, only identifiers-with-brackets  */

  res = BddEnc_get_symbol_add(self, name);
  if (res == ADD_ARRAY(NULL)) error_undefined(name);

  return res;
}


/**Function********************************************************************

   Synopsis           [Recursive subroutine of bdd_enc_eval]

   Description        [The returned list belongs to the invoker.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static AddArray_ptr bdd_enc_eval_recur(BddEnc_ptr self, Expr_ptr expr,
                                       node_ptr ctx)
{
  boolean tmp_bool;

  if (expr == Nil)  return AddArray_from_add(add_true(self->dd));

  switch (node_get_type(expr)) {

  case ATOM:
    return bdd_enc_eval_recur_case_atom(self, expr, ctx);

  case BIT:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(BASE_ENC(self)->symb_table, expr, ctx);
      name = ResolveSymbol_get_resolved_name(rs);

      return AddArray_from_add(
                               BddEncCache_lookup_boolean_var(self->cache, name));
    }

  case ARRAY:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(BASE_ENC(self)->symb_table, expr, ctx);
      name = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_undefined(rs)) {
        /* Array may be an identifier-with-brackets and may be
           expression.  Here an array-expression is detected =>
           expression is to be flattened at first to resolve
           identifiers-with-brackets (see description of
           compileFlattenSexpRecuras for details) and then general
           function is to be invoked */
        node_ptr tmp = Compile_FlattenSexp(BASE_ENC(self)->symb_table,
                                           expr, ctx);
        nusmv_assert(tmp != expr); /* loop in recursion is impossible */
        return bdd_enc_eval_recur(self, tmp, Nil);
      }
      else {
        /* array is actually identifier => process it with other identifiers */
        return bdd_enc_eval_recur_case_dot_array(self, expr, ctx);
      }
    }

  case DOT:
    return bdd_enc_eval_recur_case_dot_array(self, expr, ctx);

  case CONTEXT: return bdd_enc_eval(self, cdr(expr), car(expr));

  case TRUEEXP:  return AddArray_from_add(add_true(self->dd));
  case FALSEEXP: return AddArray_from_add(add_false(self->dd));

  case NOT: return bdd_enc_unary_add_op(self, add_not, expr, ctx);
  case CONS:
    if (Nil == cdr(expr)) { /* list of one element. return the element */
      return bdd_enc_eval(self, car(expr), ctx);
    }
    /* otherwise the CONS behaves the same ways a AND */
    /* NOTE: for all logical operations if there are more than 1 bit (element)
       in ADD arrays then the operations are applied by to every
       corresponding pair of bits, i.e.
       [a2,a1,a0] op [b2,b1,b0] === [a2 op b2, a1 op b2, a0 op b0]
    */
  case AND: return bdd_enc_binary_add_op(self, add_and, expr, ctx);
  case OR:       return bdd_enc_binary_add_op(self, add_or, expr, ctx);
  case IMPLIES:  return bdd_enc_binary_add_op(self, add_implies, expr, ctx);
  case IFF:      return bdd_enc_binary_add_op(self, add_iff, expr, ctx);
  case XOR:      return bdd_enc_binary_add_op(self, add_xor, expr, ctx);
  case XNOR:     return bdd_enc_binary_add_op(self, add_xnor, expr, ctx);

    /* if-then-else is the same for both Word and usual expressions */
  case IFTHENELSE:
  case CASE:  return bdd_enc_if_then_else_op(self, expr, ctx);

  case EQUAL:
    /* this is Word expression (coded as array of ADD) */
    if (bdd_enc_is_bit_vector(self, car(expr), ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_equal, expr, ctx);
    }
    /* This is a usual (scalar) expression */
    else return bdd_enc_binary_node_op(self, node_equal, expr, ctx);

  case NOTEQUAL:
    /* this is Word expression (coded as array of ADD) */
    if (bdd_enc_is_bit_vector(self, car(expr), ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_not_equal, expr, ctx);
    }
    else return bdd_enc_binary_node_op(self, node_not_equal, expr, ctx);

  case LT:
    /* operands are Word expressions (coded as arrays of ADD) */
    if (bdd_enc_is_bit_vector(self, car(expr), ctx, &tmp_bool)) {
      return bdd_enc_binary_vector_op(self,
                                      tmp_bool ? AddArray_word_signed_less
                                      : AddArray_word_unsigned_less,
                                      expr, ctx);
    }
    else return bdd_enc_binary_node_op(self, node_lt, expr, ctx);

  case GT:
    /* operands are Word expressions (coded as arrays of ADD) */
    if (bdd_enc_is_bit_vector(self, car(expr), ctx, &tmp_bool)) {
      return bdd_enc_binary_vector_op(self,
                                      tmp_bool ? AddArray_word_signed_greater
                                      : AddArray_word_unsigned_greater,
                                      expr, ctx);
    }
    else return bdd_enc_binary_node_op(self, node_gt, expr, ctx);

  case LE:
    /* operands are Word expressions (coded as arrays of ADD) */
    if (bdd_enc_is_bit_vector(self, car(expr), ctx, &tmp_bool)) {
      return bdd_enc_binary_vector_op(self,
                                      tmp_bool ? AddArray_word_signed_less_equal
                                      : AddArray_word_unsigned_less_equal,
                                      expr, ctx);
    }
    else return bdd_enc_binary_node_op(self, node_le, expr, ctx);

  case GE:
    /* operands are Word expressions (coded as arrays of ADD) */
    if (bdd_enc_is_bit_vector(self, car(expr), ctx, &tmp_bool)) {
      return bdd_enc_binary_vector_op(self,
                                      tmp_bool ? AddArray_word_signed_greater_equal
                                      : AddArray_word_unsigned_greater_equal,
                                      expr,
                                      ctx);
    }
    else return bdd_enc_binary_node_op(self, node_ge, expr, ctx);

  case PLUS:
    if (bdd_enc_is_bit_vector(self, expr, ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_plus, expr, ctx);
    }
    else return bdd_enc_binary_node_op(self, node_plus, expr, ctx);

  case UMINUS:
    if (bdd_enc_is_bit_vector(self, expr, ctx, NULL)) {
      return bdd_enc_unary_vector_op(self, AddArray_word_unary_minus, expr, ctx);
    }
    return bdd_enc_unary_node_op(self, node_unary_minus, expr, ctx);

  case MINUS:
    if (bdd_enc_is_bit_vector(self, expr, ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_minus, expr, ctx);
    }
    else return bdd_enc_binary_node_op(self, node_minus, expr, ctx);

  case TIMES:
    if (bdd_enc_is_bit_vector(self, expr, ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_times, expr, ctx);
    }
    else return bdd_enc_binary_node_op(self, node_times, expr, ctx);

  case DIVIDE:
    if (bdd_enc_is_bit_vector(self, expr, ctx, &tmp_bool)) {
      return bdd_enc_binary_vector_op(self,
                                      tmp_bool ? AddArray_word_signed_divide
                                      : AddArray_word_unsigned_divide,
                                      expr, ctx);
    }
    else return bdd_enc_binary_node_op(self, node_divide, expr, ctx);

  case MOD:
    if (bdd_enc_is_bit_vector(self, expr, ctx, &tmp_bool)) {
      return bdd_enc_binary_vector_op(self,
                                      tmp_bool ? AddArray_word_signed_mod
                                      : AddArray_word_unsigned_mod,
                                      expr, ctx);
    }
    else return bdd_enc_binary_node_op(self, node_mod, expr, ctx);

  case LSHIFT:
    if (bdd_enc_is_bit_vector(self, expr, ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_left_shift, expr, ctx);
    }
    else internal_error("l-shift has always to be a word exp");

  case RSHIFT:
    if (bdd_enc_is_bit_vector(self, expr, ctx, &tmp_bool)) {
      return bdd_enc_binary_vector_op(self,
                                      tmp_bool ? AddArray_word_signed_right_shift :
                                      AddArray_word_unsigned_right_shift,
                                      expr,ctx);
    }
    else internal_error("r-shift has always to be a word exp");

  case LROTATE:
    if (bdd_enc_is_bit_vector(self, expr, ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_left_rotate, expr,ctx);
    }
    else internal_error("l-rotate has always to be a word exp");

  case RROTATE:
    if (bdd_enc_is_bit_vector(self, expr, ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_right_rotate,expr,ctx);
    }
    else internal_error("r-rotate has always to be a word exp");

  case CONCATENATION:
    if (bdd_enc_is_bit_vector(self, expr, ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_concatenation,
                                      expr, ctx);
    }
    else internal_error("concatenation has always to be a word exp");

  case BIT_SELECTION:
    if (bdd_enc_is_bit_vector(self, expr, ctx, NULL)) {
      return bdd_enc_binary_vector_op(self, AddArray_word_bit_selection, expr,
                                      ctx);
    }
    else internal_error("bit selection has always to be a word exp");

  case COLON: /* used only as an argument of bit selection */
    return bdd_enc_binary_node_op(self, node_bit_range, expr, ctx);

  case UNION: return bdd_enc_binary_node_op(self, node_union, expr, ctx);
  case SETIN: return bdd_enc_binary_node_op(self, node_setin, expr, ctx);

  case NEXT: {
    int i;
    AddArray_ptr res;

    {
      add_ptr add = BddEncCache_lookup_boolean_var(self->cache, expr);
      if (add != (add_ptr)NULL) return AddArray_from_add(add);
    }

    res = bdd_enc_eval(self, car(expr), ctx);

    set_the_node(expr);
    /* process every bit (shift current variables to the next ones) */
    for (i = 0; i < AddArray_get_size(res); ++i) {
      add_ptr tmp = AddArray_get_n(res, i);
      AddArray_set_n(res, i, BddEnc_state_var_to_next_state_var_add(self, tmp));
      add_free(self->dd, tmp);
    }
    return res;
  }

  case ATTIME: internal_error("unexpected attime node");

    /* Assignment */
  case EQDEF: {
    node_ptr t1, t2, name;

    switch (node_get_type(car(expr))) {
    case SMALLINIT:
      /* we deal with statements of this kind: init(x) := init_expr */
      t1 = t2 = car(car(expr)); /* (ATOM x Nil) */
      break;

    case NEXT: /* we deal wit statements of this kind: next(x) := next_expr */
      t1 = car(expr); /* (NEXT (ATOM x Nil) Nil) */
      t2 = car(car(expr)); /* (ATOM x Nil) */
      break;

    default: /* we deal wit statements of this kind: x := simple_expr */
      t1 = t2 = car(expr); /* (ATOM x Nil) */
    }

    /* we contextualize "x" and we extract its information */
    {
      ResolveSymbol_ptr rs;
      rs = SymbTable_resolve_symbol(BASE_ENC(self)->symb_table, t2, ctx);
      name = ResolveSymbol_get_resolved_name(rs);
    }

    if (! SymbTable_is_symbol_declared(BASE_ENC(self)->symb_table, name)) {
      error_undefined(t2);
    }
    if (! SymbTable_is_symbol_var(BASE_ENC(self)->symb_table, name)) {
      error_redefining(t2);
    }

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
      inc_indent_size();
      indent_node(nusmv_stderr, "evaluating operands of assignment of ", t1, ":\n");
    }

    /* We evaluate both the left and the right hand side of the
       assignment (lhs := rhs) */
    {
      AddArray_ptr lhs, rhs;
      SymbType_ptr type;
      add_ptr res;

      lhs = bdd_enc_eval(self, t1, ctx);
      rhs = bdd_enc_eval(self, cdr(expr), ctx);

      nusmv_assert(AddArray_get_size(lhs) == AddArray_get_size(rhs));

      /* We check that the value of the rhs are admissible values
         for the lhs */
      type = SymbTable_get_var_type(BASE_ENC(self)->symb_table, name);

      /* traverses the rhs */
      switch (SymbType_get_tag(type)) {
      case SYMB_TYPE_BOOLEAN: break;
      case SYMB_TYPE_ENUM:
        Utils_set_data_for_range_check(t2, /* var and range */
                                       SymbType_get_enum_type_values(type));
        add_walkleaves(Utils_range_check, AddArray_get_add(rhs));
        break;
      case SYMB_TYPE_INTEGER: error_unreachable_code(); /* not implemented */
      case SYMB_TYPE_REAL:    error_unreachable_code(); /* not implemented */
      case SYMB_TYPE_STRING:    error_unreachable_code(); /* not implemented */
      case SYMB_TYPE_UNSIGNED_WORD: /*do nothing(type checking guarantees correctness) */
      case SYMB_TYPE_SIGNED_WORD:
        break;
      case SYMB_TYPE_ERROR:   error_unreachable_code(); /* not implemented */
      default:  error_unreachable_code(); /* unknown type  */
      } /* switch */

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
        inc_indent_size();
        indent_node(nusmv_stderr, "construction of assignment of ", t1, ":\n");
      }

      /* We perform set inclusion of the evaluation of the rhs with the lhs
         (for every ADD separately (if it is an array) and then a big AND of
         all ADDs)
      */
      {
        int i;
        res = add_true(self->dd);
        for (i = AddArray_get_size(lhs) - 1; i >= 0; --i) {

          add_ptr bit = add_apply(self->dd, node_setin, AddArray_get_n(lhs,i),
                                  AddArray_get_n(rhs, i));
          add_ptr tmp = add_and(self->dd, res, bit);
          add_free(self->dd, res);
          add_free(self->dd, bit);
          res = tmp;
        } /* for */
      }
      AddArray_destroy(self->dd, lhs);
      AddArray_destroy(self->dd, rhs);

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
        indent_node(nusmv_stderr, "size of assignment of ", t1, " = ");
        fprintf(nusmv_stderr, "%d ADD nodes\n", add_size(self->dd, res));
        dec_indent_size();
      }
      return AddArray_from_add(res);
    }

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 4)) {
      indent_node(nusmv_stderr, "evaluating operands of assignment of ", t1, " has finished\n");
      dec_indent_size();
    }


  } /* case EQDEF */

  case NUMBER:   return AddArray_from_add(add_leaf(self->dd, find_atom(expr)));

  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD: {
    /* ADD encoding of Word */
    WordNumber_ptr word = WORD_NUMBER(car(expr));
    return AddArray_from_word_number(self->dd, word);
  }

  case UWCONST:
  case SWCONST: {
    /* here we rely on the flattener, which is in charge of getting
       rid of SWCONST and UWCONST */
    node_ptr wn = Compile_FlattenSexp(BASE_ENC(self)->symb_table, expr, ctx);
    return bdd_enc_eval_recur(self, wn, (node_ptr) NULL);
  }

  case WSIZEOF: {
    /* here we rely on the flattener, which is in charge of getting
       rid of WSIZEOF */
    node_ptr wn = Compile_FlattenSexp(BASE_ENC(self)->symb_table, expr, ctx);
    return bdd_enc_eval_recur(self, wn, (node_ptr) NULL);
  }

  case WRESIZE: {
    boolean isWord = bdd_enc_is_bit_vector(self, car(expr), ctx, &tmp_bool);
    nusmv_assert(isWord); /* only word as bit vectors can be here */
    /* operands are Word expressions (coded as arrays of ADD) */
    return bdd_enc_binary_vector_op(self,
                                    tmp_bool ? AddArray_word_signed_resize
                                    : AddArray_word_unsigned_resize,
                                    expr, ctx);
  }

  case SIGNED_WORD:

    /* cannot be met in expression. only UNSIGNED_WORD can be
     * used. See below */
    error_unreachable_code();

    /* this artificial expression is used to code (signed and unsigned)
       Word variables as array of bits.
       The expr is a list (wrapped in UNSIGNED_WORD) of boolean
       variables.  See the function bool_enc_encode_var about details.
    */
  case UNSIGNED_WORD: {
    int i;
    AddArray_ptr res;
    node_ptr iter;

    /* retrieves the number of bits */
    nusmv_assert(node_get_type(cdr(expr)) == NUMBER);
    i = node_get_int(cdr(expr));

    /* create an array to return and then evaluate the bits consecutively*/
    res = AddArray_create(i);
    for (--i, iter = car(expr); iter != Nil; iter = cdr(iter), --i) {
      AddArray_ptr tmp = bdd_enc_eval(self, car(iter), Nil);
      /* only one ADD (one bit) can be in every bit encoding */
      add_ptr add = AddArray_get_add(tmp);
      add_ref(add);
      /* set the bit encoding evaluation */
      AddArray_set_n(res, i, add);
      AddArray_destroy(self->dd, tmp);
    }
    return res;
  }

  case COUNT:
    {
      node_ptr flat = Compile_FlattenSexp(BASE_ENC(self)->symb_table,
                                          expr, ctx);
      return bdd_enc_eval_recur(self, flat, Nil);
    }

  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP :
    return AddArray_from_add(add_leaf(self->dd, find_atom(expr)));

  case TWODOTS: /* this expression is used in bit selection */
    {
      int dim1, dim2, i;
      node_ptr t = Nil;

      dim1 = BddEnc_eval_num(self, car(expr), ctx);
      dim2 = BddEnc_eval_num(self, cdr(expr), ctx);
      for (i = dim2; i >= dim1; --i) {
        t = find_node(CONS, find_node(NUMBER, NODE_FROM_INT(i), Nil), t);
      }

      if (t == Nil) { rpterr("empty range: %d..%d", dim1, dim2); }
      return AddArray_from_add(add_leaf(self->dd, t));
    }

  case CAST_BOOL: {
    node_ptr tmp = Compile_FlattenSexp(BASE_ENC(self)->symb_table, expr, ctx);
    AddArray_ptr arg = bdd_enc_eval(self, tmp, Nil);

    /* The resulting expression must be boolean */
    nusmv_assert(AddArray_get_size(arg) == 1);

    /* Word is encoded as a list of ADD => just return the result(i.e. one bit)*/
    return arg;
  }

  case CAST_TOINT: {
    /* here we rely on the flattener, which is in charge of getting
       rid of CAST_TOINT */
    node_ptr wn = Compile_FlattenSexp(BASE_ENC(self)->symb_table, expr, ctx);
    return bdd_enc_eval_recur(self, wn, Nil);
  }

  case CAST_WORD1: {
    AddArray_ptr arg = bdd_enc_eval(self, car(expr), ctx);
    /* the operand can be of the boolean type only */
    nusmv_assert(AddArray_get_size(arg) == 1);

    /*Word is encoded as a list of ADD => just return the result (i.e. one bit)*/
    return arg;
  }

    /* cast between signed and unsigned words. The representation does not change.*/
  case CAST_SIGNED:
  case CAST_UNSIGNED:
    return bdd_enc_eval(self, car(expr), ctx);

  case EXTEND: {
    boolean isWord = bdd_enc_is_bit_vector(self, car(expr), ctx, &tmp_bool);
    nusmv_assert(isWord); /* only word as bit vectors can be here */
    /* operands are Word expressions (coded as arrays of ADD) */
    return bdd_enc_binary_vector_op(self,
                                    tmp_bool ? AddArray_word_signed_extend
                                    : AddArray_word_unsigned_extend,
                                    expr, ctx);
  }

  case FAILURE:
    return AddArray_from_add(add_leaf(self->dd, find_atom(expr)));

  default:
    internal_error("bdd_enc_eval_recur: type = %d\n", node_get_type(expr));
  } /* switch */

  error_unreachable_code(); /* impossible code */
  return (AddArray_ptr) NULL;
}


/**Function********************************************************************

   Synopsis           [Returns true, if an expression is a bit verctor, i.e.
   of type Word. They are encoded as arrays of ADD (not as single ADD).
   Otherwise false is returned.]

   Description        [the expression should already be checked by the
   type-checker associated with the given BDD Encoder.
   If the function returns true and isSigned parameter is not NULL then
   *isSigned is set to:
   true -- if the expression is of signed word type, and
   false -- if the expression if of unsigned word type.]

   SideEffects        []

   SeeAlso            [bdd_enc_eval, bdd_enc_binary_op]

******************************************************************************/
static boolean bdd_enc_is_bit_vector(BddEnc_ptr self, node_ptr expr,
                                     node_ptr context, boolean* isSigned)
{
  SymbType_ptr type = TypeChecker_get_expression_type(self->type_checker,
                                                      expr, context);
  /* the expression type should be ok */
  if (type == SYMB_TYPE(NULL) || SymbType_is_error(type)) {
    internal_error("bdd_enc_is_bit_vector: invalid or empty type");
  }

  if (!SymbType_is_word(type)) return false;

  /* this is a Word expression and it is coded as a list of ADD */
  if (NULL != isSigned) *isSigned = SymbType_is_signed_word(type);

  return true;
}


/**Function********************************************************************

   Synopsis           [Applies unary operation.]

   Description        [Takes in input the expression <code>n</code> and a
   unary operation <code>op</code>. Evaluates <code>n</n> and applies to this
   partial result the unary operator <code>op</code>.

   If the operand have more than one ADD, then the operation is consecutively
   applied to every its ADD.

   NB: The size of both given arrays should be the same (and positive).
   ]

   SideEffects        []

   SeeAlso            [bdd_enc_eval, bdd_enc_binary_op]

******************************************************************************/
static AddArray_ptr bdd_enc_unary_add_op(BddEnc_ptr self, FP_A_DA op,
                                         node_ptr n, node_ptr context)
{
  AddArray_ptr arg1 = bdd_enc_eval(self, car(n), context);
  AddArray_ptr res;

  set_the_node(n);
  res = AddArray_word_apply_unary(self->dd, arg1, op);
  AddArray_destroy(self->dd, arg1);

  return res;
}


/**Function********************************************************************

   Synopsis           [Applies binary add operation.]

   Description        [Takes in input the expression <code>n</code> and a
   binary operation <code>op</code>. Extracts from <code>n</n> the
   operands and evaluates them.]

   SideEffects        []

   SeeAlso            [bdd_enc_eval, bdd_enc_unary_op]

******************************************************************************/
static AddArray_ptr bdd_enc_binary_add_op(BddEnc_ptr self, FP_A_DAA op,
                                          node_ptr n, node_ptr context)
{
  /* It can even further optimized by simply looking at car(n)
     and/or cdr(n) to see if they are the constant TRUE/FALSE and
     act accordingly */
  AddArray_ptr arg1, arg2, res;

  arg1 = bdd_enc_eval(self, car(n), context);

  if ((add_and == op) && (1 == AddArray_get_size(arg1)) &&
      add_is_false(self->dd, AddArray_get_add(arg1))) {
    return arg1;
  }
  else if (((add_or == op) && (1 == AddArray_get_size(arg1)) &&
      add_is_true(self->dd, AddArray_get_add(arg1)))) {
    return arg1;
  }
  else {
    arg2 = bdd_enc_eval(self, cdr(n), context);
    res = AddArray_word_apply_binary(self->dd, arg1, arg2, op);
    AddArray_destroy(self->dd, arg1);
    AddArray_destroy(self->dd, arg2);

    return res;
  }
}


/**Function********************************************************************

   Synopsis           [Applies unary operation.]

   Description        [Takes in input the expression <code>n</code> and a
   unary operation <code>op</code>. Evaluates <code>n</n> and applies to this
   partial result the unary operator <code>op</code>.

   NOTE: At the moment CUDD does not have unary 'apply', so you have
   to provide a binary op, which is actually unary and applies to
   the first operand only.

   If the operand have more than one ADD, then the operation is consecutively
   applied to every its ADD.

   NB: The size of both given arrays should be the same (and positive).

   ]

   SideEffects        []

   SeeAlso            [bdd_enc_eval, bdd_enc_binary_op]

******************************************************************************/
static AddArray_ptr bdd_enc_unary_node_op(BddEnc_ptr self, NPFNN/*NPFCVT*/ op,
                                          node_ptr n, node_ptr context)
{
  AddArray_ptr arg = bdd_enc_eval(self, car(n), context);
  int i;
  const int width = AddArray_get_size(arg);

  nusmv_assert(width > 0);

  set_the_node(n);

  /* process one bit for an interation and collect the result in arg */
  for (i = 0; i < width; ++i) {
    add_ptr tmp = AddArray_get_n(arg, i);
    /* apply and ref the result of the application of "op" to previous arg. */
    AddArray_set_n(arg, i, add_monadic_apply(self->dd, op, tmp));
    add_free(self->dd, tmp);
  }

  return arg;
}


/**Function********************************************************************

   Synopsis           [Applies binary operation.]

   Description        [Takes in input the expression <code>n</code> and a
   binary operation <code>op</code>. Extracts from <code>n</n> the
   operands and evaluates them.
   The binary operator <code>op</code> is then applied to these partial
   results with the help of add_apply, i.e. op should be a function
   which can be applied on two constant nodes.

   If the operands have more than one ADD, then the operation is consecutively
   applied to every corresponding pair of ADDs (one from left operand, and
   one from the right operand).

   ]

   SideEffects        []

   SeeAlso            [bdd_enc_eval, bdd_enc_unary_node_op]

******************************************************************************/
static AddArray_ptr bdd_enc_binary_node_op(BddEnc_ptr self, NPFNN op,
                                           node_ptr n, node_ptr context)
{
  AddArray_ptr arg1 = bdd_enc_eval(self, car(n), context);
  AddArray_ptr arg2 = bdd_enc_eval(self, cdr(n), context);
  int i;
  const int width = AddArray_get_size(arg1);

  /* the size of array should be the same and positive */
  nusmv_assert(width == AddArray_get_size(arg2) && width > 0);

  set_the_node(n);
  /* at every interation, process one bit and collect the result in arg1 */
  for (i = 0; i < width; ++i) {
    add_ptr tmp = AddArray_get_n(arg1, i);
    AddArray_set_n(arg1, i,
                   add_apply(self->dd, op, tmp, AddArray_get_n(arg2, i)));
    add_free(self->dd, tmp);
  }

  AddArray_destroy(self->dd, arg2);

  return arg1;
  /* If at some time the implementation of the ADD-application
     functions will be change, do not forget to
     change them in AddArray.c also
  */
}


/**Function********************************************************************

   Synopsis           [Evaluates if_then_else expressions returning
   the ADDs representing <em>IF ifarg THEN thenarg ELSE elsarg</em>.]

   Description        [Evaluates if_then_else expressions returning the
   ADD representing <em>IF ifarg THEN thenarg ELSE elsarg</em>, where
   <code>ifarg</code>, <code>thenarg</code>, <code>elsearg</code> are the ADD
   obtained by evaluating <code>ifexp</code>, <code>thenexp</code>,
   <code>elseexp</code> respectively in context <code>context</code>.
   THEN and ELSE part can be an array of ADDs, then of-then-else is
   applied to every pair of corresponding ADDs.]

   SideEffects        []

   SeeAlso            [add_ifthenelse]

******************************************************************************/
static AddArray_ptr bdd_enc_if_then_else_op(BddEnc_ptr self, node_ptr node,
                                            node_ptr context)
{
  AddArray_ptr res;
  AddArray_ptr ifarg, thenarg, elsearg;
  add_ptr add;

  ifarg = bdd_enc_eval(self, car(car(node)), context);

  add = AddArray_get_add(ifarg); /* array must contain only one
                                    element since boolean */

  if (add_is_true(self->dd, add)) {
    /* ITE(1, A, B) = A */
    res = bdd_enc_eval(self, cdr(car(node)), context);
  }
  else if (add_is_false(self->dd, add)) {
    /* ITE(0, A, B) = B */
    res = bdd_enc_eval(self, cdr(node), context);
  }
  else {
    thenarg = bdd_enc_eval(self, cdr(car(node)), context);
    elsearg = bdd_enc_eval(self, cdr(node), context);

    res = AddArray_word_ite(self->dd, ifarg, thenarg, elsearg);

    /* free temporary results */
    AddArray_destroy(self->dd, elsearg);
    AddArray_destroy(self->dd, thenarg);
  }

  AddArray_destroy(self->dd, ifarg);

  return res;
}


/**Function********************************************************************

   Synopsis           [Applies a unary operation to an array of ADDs.]

   Description        [Takes the expression <code>n</code> and a
   unary array operation <code>op</code>. Extracts from <code>n</n> the
   operand and evaluates it.
   The unary operator <code>op</code> is then applied to this partial
   result.]

   SideEffects        []

   SeeAlso            [bdd_enc_eval, bdd_enc_binary_vector_op, bdd_enc_binary_op]

******************************************************************************/
static AddArray_ptr bdd_enc_unary_vector_op(BddEnc_ptr self,
                                            ADD_ARRAY_UNARY_OP op,
                                            node_ptr n, node_ptr context)
{
  AddArray_ptr res;
  AddArray_ptr arg = bdd_enc_eval(self, car(n), context);

  set_the_node(n);/* for error reporting */
  res = op(self->dd, arg);
  AddArray_destroy(self->dd, arg);

  return res;
}


/**Function********************************************************************

   Synopsis           [Applies a binary operation to an array of ADDs.]

   Description        [Takes the expression <code>n</code> and a
   binary array operation <code>op</code>. Extracts from <code>n</n> the
   operands and evaluates them.
   The binary operator <code>op</code> is then applied to these partial
   results.]

   SideEffects        []

   SeeAlso            [bdd_enc_eval, bdd_enc_unary_vector_op, bdd_enc_binary_op]

******************************************************************************/
static AddArray_ptr bdd_enc_binary_vector_op(BddEnc_ptr self,
                                             ADD_ARRAY_BINARY_OP op,
                                             node_ptr n, node_ptr context)
{
  AddArray_ptr res;
  AddArray_ptr arg1 = bdd_enc_eval(self, car(n), context);
  AddArray_ptr arg2 = bdd_enc_eval(self, cdr(n), context);

  set_the_node(n);/* for error reporting */
  res = op(self->dd, arg1, arg2);

  AddArray_destroy(self->dd, arg1);
  AddArray_destroy(self->dd, arg2);
  return res;
}


/**Function********************************************************************

   Synopsis           [Converts a ADD into the corresponding expression.]

   Description [Auxiliary function for bdd_enc_add2expr.  if is_scalar
   is false, then the add is assumed to be boolean, and the result
   will be a boolean expression. If det_layer is NULL, if the add
   contains undeterministic leaves, the returned expression will still
   contain undeterministic values. If det_layer is not NULL, then the
   expression will be determinized, and the returned layer will
   contain new determinization variables.]

   SideEffects        [A new boolean variable might be introduced.]

   SeeAlso            [bdd_enc_add2expr]

******************************************************************************/
static node_ptr bdd_enc_add2expr_recur(BddEnc_ptr self,
                                       SymbLayer_ptr det_layer,
                                       add_ptr add_expr,
                                       hash_ptr lc,
                                       boolean is_scalar)
{
  DdManager* dd = BddEnc_get_dd_manager(self);
  node_ptr result = (node_ptr) NULL;
  SymbTable_ptr symb_table = BASE_ENC(self)->symb_table;

  nusmv_assert((add_expr != (add_ptr) NULL));

  /* base case */
  if (add_isleaf(add_expr)) {
    node_ptr leaf;

    leaf = add_get_leaf(dd, add_expr);

    /* handles set singleton */
    if (CONS == node_get_type(leaf) && llength(leaf) == 1) leaf = car(leaf);

    /* checks boolean case */
    if (Expr_is_true(leaf) || Expr_is_false(leaf)) {
      nusmv_assert(!is_scalar);
      return leaf;
    }

    /* checks if leaf is a set (non-deterministic expression) */
    if (CONS == node_get_type(leaf)) {

      nusmv_assert(llength(leaf) > 1); /* singleton are not allowed here */

      if (det_layer == SYMB_LAYER(NULL)) {
        /* no determinization has been required. Converts the list
           of values to a set and returns it. */
        node_ptr leaf_iter;
        result = Nil;
        for (leaf_iter=leaf; Nil != leaf_iter; leaf_iter = cdr(leaf_iter)) {
          nusmv_assert(CONS == node_get_type(leaf_iter));
          /* leaves have alredy been checked to be boolean when
             needed */
          node_ptr elem = car(leaf_iter);

          if (elem == one_number || elem == zero_number) {
            nusmv_assert(is_scalar);
          }

          if (Nil == result) result = elem;
          else result = find_node(UNION, elem, result);
        }
        return result;
      }

      { /* makes the leaf deterministic. Calculates the
           boolean encoding of the non-deterministic set */
        Set_t det_vars = Set_MakeEmpty();
        Set_Iterator_t det_vars_iter;
        node_ptr det_enc;

        det_enc = BoolEnc_get_values_bool_encoding(
                          BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(self)),
                          leaf, /* this is the non-deterministic range */
                          &det_vars /* the set of determinization bits */);

        if (!is_scalar) {
          /* simplify when possible: this reduces "v ? T : F" to
             "v" and "v ? F : T" to "!v" */
          det_enc = Expr_simplify(symb_table, det_enc);
        }

        /* now declares the determinization symbols into the
           determinization layer */
        SET_FOREACH(det_vars, det_vars_iter) {
          node_ptr bit = (node_ptr) Set_GetMember(det_vars, det_vars_iter);
          SymbLayer_declare_state_var(det_layer, bit,
                                      SymbType_create(SYMB_TYPE_BOOLEAN, Nil));
        }

        Set_ReleaseSet(det_vars);
        return det_enc; /* returns the boolean encoding */
      }

    } /* end of set handling, execution returns from this case */

    /* leaf is not a set here, and boolean case was already handled */
    nusmv_assert(CONS != node_get_type(leaf) && is_scalar);
    return leaf;
  }

  else {
    /* step case */
    node_ptr t, e, var;
    int index;

    if (st_lookup(lc, (char*) add_expr, (char**) &result)) return result;

    index = add_index(dd, add_expr);

    t = bdd_enc_add2expr_recur(self, det_layer, add_then(dd, add_expr), lc,
                               is_scalar);
    if (t == (node_ptr) NULL) return (node_ptr) NULL;

    e = bdd_enc_add2expr_recur(self, det_layer, add_else(dd, add_expr), lc,
                               is_scalar);
    if (e == (node_ptr) NULL) return (node_ptr) NULL;

    var = BddEnc_get_var_name_from_index(self, index);
    if (var == Nil) {
      fprintf(nusmv_stderr,
              "bdd_enc_add2expr_recur: No variable associated to BDD variable %d\n",
              index);

      return (node_ptr) NULL;
    }

    result = NODE_PTR(Expr_simplify(symb_table,
                                    Expr_ite(EXPR(var),
                                             EXPR(t),
                                             EXPR(e),
                                             symb_table)));
    if (result == (node_ptr)NULL) return (node_ptr)NULL;

    if (st_add_direct(lc, (char*) add_expr, (char*) result) == ST_OUT_OF_MEM) {
      internal_error("bdd_enc_add2expr_recur: Unable to insert result in local hash.\n");
    }

    return result;
  }
}

/**Function********************************************************************

   Synopsis           [Returns true if given variable index is not currently
   allocated]

   Description        [Used internally to check indices]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
static boolean bdd_enc_is_index_not_allocated(const BddEnc_ptr self, int index)
{
  BDD_ENC_CHECK_INSTANCE(self);
  nusmv_assert(index >= 0);

  return ((index >= array_n(self->index2name)) ||
          BddEnc_get_var_name_from_index(self, index) == (node_ptr) NULL);
}


/**Function********************************************************************

   Synopsis [Used when it is needed to dissolve the variable blocks
   created by this encoder. This is needed when interleaving
   variables among several layers.]

   Description []

   SideEffects []

   SeeAlso     []

******************************************************************************/
static void bdd_enc_dissolve_committed_groups(BddEnc_ptr self)
{
  ListIter_ptr iter;

  bdd_enc_lock_reordering(self);
  for (iter = NodeList_get_first_iter(BASE_ENC(self)->committed_layers);
       !ListIter_is_end(iter);
       iter = ListIter_get_next(iter)) {
    SymbLayer_ptr com_layer = SYMB_LAYER(NodeList_get_elem_at(
                                      BASE_ENC(self)->committed_layers, iter));
    array_t* groups = (array_t*) find_assoc(self->layer2groups,
                                            (node_ptr) com_layer);

    if ((array_t*) NULL != groups) {
      int giter;
      GroupInfo_ptr binfo;

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
        fprintf(nusmv_stderr,
                "BddEnc: dissolving dd groups associated with layer '%s'\n",
                SymbLayer_get_name(com_layer));
      }

      /* promote and remove all groups */
      arrayForEachItem(GroupInfo_ptr, groups, giter, binfo) {
        /* this releases and frees binfo which was returned by
           VarsHandler_reserve_group */
        VarsHandler_dissolve_group(self->dd_vars_hndr, binfo);
      }

      /* We destroy the groups array and we remove assoc as not to
       * have groups within this layer */
      array_free(groups);
    }
    insert_assoc(self->layer2groups, (node_ptr) com_layer, (node_ptr) NULL);
  }
  bdd_enc_unlock_reordering(self);
}


/**Function********************************************************************

  Synopsis           [Dumps the given AddArray.]

  Description [Labels is an array of strings to be used as roots
  labels. The size of the array must be equal to the size of the
  AddArray.

  Returns 0 in case of success, 1 otherwise]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
static int bdd_enc_dump_addarray_dot_davinci(BddEnc_ptr self,
                                             AddArray_ptr addarray,
                                             const char** labels,
                                             FILE* outfile,
                                             boolean use_dot)
{
  DdManager* dd = BddEnc_get_dd_manager(self);
  const int dd_size = dd_get_size(dd);
  const int adr_size = AddArray_get_size(addarray);
  add_ptr* dd_array;
  const char** inames;
  int lev;
  int res = 0;

  nusmv_assert(adr_size >= 1);

  /* prepares the array of adds */
  dd_array = array_data(add_ptr, AddArray_get_array(addarray));

  /* calculates inames */
  inames = ALLOC(const char*, dd_size);
  nusmv_assert((const char**) NULL != inames);
  for (lev=0; lev<dd_size; ++lev) {
    int idx = dd_get_index_at_level(dd, lev);
    if (BddEnc_has_var_at_index(self, idx)) {
      inames[lev] = (const char*) sprint_node(
		      BddEnc_get_var_name_from_index(self, idx));
    }
    else inames[lev] = (const char*) NULL;
  }

  /* invocation of the main routine */
  if (use_dot) {
    int tmp = dd_dump_dot(dd, adr_size, dd_array, inames, labels, outfile);
    if (tmp == 0) res = (tmp ==0) ? 1 : 0;
  }
  else {
    int tmp = dd_dump_davinci(dd, adr_size, dd_array, inames, labels, outfile);
    if (tmp == 0) res = (tmp ==0) ? 1 : 0;
  }

  /* cleanup */
  {
    int i;
    /* free inames */
    for (i=0; i<dd_size; ++i) {
      if ((const char*) NULL != inames[i]) { FREE(inames[i]); }
    }
    FREE(inames);
  }

  return res;
}


/**AutomaticEnd***************************************************************/
