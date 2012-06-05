/**CFile***********************************************************************

  FileName    [bmcConv.c]

  PackageName [bmc]

  Synopsis    [Convertion function of BE to corresponding BDD boolean
               expression, and viceversa]

  Description []

  SeeAlso     []

  Author      [Alessandro Cimatti, Lorenzo Delana, Michele Dorigatti]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst and University of Trento.
  Copyright (C) 2011 by FBK.

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

#include "wff/wff.h"
#include "wff/w2w/w2w.h"

#include "bmcConv.h"

#include "parser/symbols.h" /* for NUMBER et similia */
#include "compile/compile.h"
#include "rbc/rbc.h"

#include "utils/array.h"
#include "utils/assoc.h"
#include "utils/error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Be2bexpDfsData]

  Description [The data structure used for DFS traversal of RBC]

  SeeAlso     []

******************************************************************************/
typedef struct Be2bexpDfsData_TAG {
  BeEnc_ptr be_enc;
  array_t* stack;
  int head;
} Be2bexpDfsData;


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static hash_ptr bexpr2be_hash = (hash_ptr) NULL;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void bmc_conv_set_cache ARGS((node_ptr bexp, be_ptr be));
static be_ptr bmc_conv_query_cache ARGS((node_ptr bexp));

static be_ptr
bmc_conv_bexp2be_recur ARGS((BeEnc_ptr be_enc, node_ptr bexp));

/* Primitives for stack used in BE traversal */
static void Be2bexpDfsData_push ARGS((Be2bexpDfsData*, node_ptr));
static node_ptr Be2bexpDfsData_head ARGS((Be2bexpDfsData*));
static node_ptr Be2bexpDfsData_pop  ARGS((Be2bexpDfsData*));

/* BE traversal functions */
static int  Be2bexp_Set   ARGS((be_ptr be, char* Be2bexpData, nusmv_ptrint sign));
static void Be2bexp_First ARGS((be_ptr be, char* Be2bexpData, nusmv_ptrint sign));
static void Be2bexp_Back  ARGS((be_ptr be, char* Be2bexpData, nusmv_ptrint sign));
static void Be2bexp_Last  ARGS((be_ptr be, char* Be2bexpData, nusmv_ptrint sign));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Given a be, constructs the corresponding boolean
  expression]

  Description        [Descends the structure of the BE with dag-level
  primitives. Uses the be encoding to perform all time-related operations. ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Conv_Be2Bexp(BeEnc_ptr be_enc, be_ptr be)
{
  RbcDfsFunctions_t Be2bexpFunctions;
  Be2bexpDfsData Be2bexpData;
  node_ptr ret;
  Be_Manager_ptr be_mgr;
  Rbc_t* rbc;
  Rbc_Manager_t* rbc_manager;

  be_mgr = BeEnc_get_be_manager(be_enc);
  rbc_manager = (Rbc_Manager_t*)Be_Manager_GetSpecManager(be_mgr);
  rbc = (Rbc_t*)Be_Manager_Be2Spec(be_mgr, be);

  /* Cleaning the user fields. */
  Rbc_Dfs_clean_exported(rbc, rbc_manager);

  /* Setting up the DFS functions. */
  Be2bexpFunctions.Set        = Be2bexp_Set;
  Be2bexpFunctions.FirstVisit = Be2bexp_First;
  Be2bexpFunctions.BackVisit  = Be2bexp_Back;
  Be2bexpFunctions.LastVisit  = Be2bexp_Last;

  /* Setting up the DFS data. */
  /* :TODO??: optimizations on the quantity of nodes */
  Be2bexpData.be_enc = be_enc;
  Be2bexpData.stack = array_alloc(node_ptr, 10);
  Be2bexpData.head = -1;

  /* Calling DFS on f. */
  Rbc_Dfs_exported(rbc, &Be2bexpFunctions, (void*)(&Be2bexpData), rbc_manager);
  ret = Be2bexpDfsData_head(&Be2bexpData);

  array_free(Be2bexpData.stack);

  return ret;
}


/**Function********************************************************************

  Synopsis           [<b>Converts</b> given <b>boolean expression</b> into
  correspondent <b>reduced boolean circuit</b>]

  Description        [Uses the be encoding to perform all
  time-related operations.]

  SideEffects        [be hash may change]

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Conv_Bexp2Be(BeEnc_ptr be_enc, node_ptr bexp)
{
  return bmc_conv_bexp2be_recur(be_enc, bexp);
}


/**Function********************************************************************

  Synopsis           [<b>Converts</b> given <b>boolean expressions list </b>
  into correspondent <b>reduced boolean circuits list</b>]

  Description        []

  SideEffects        [be hash may change]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Conv_BexpList2BeList(BeEnc_ptr be_enc, node_ptr bexp_list)
{
  if (bexp_list == Nil)
  {
    return(Nil);
  }
  else
  {
    return cons( (node_ptr)Bmc_Conv_Bexp2Be(be_enc, car(bexp_list)),
                 Bmc_Conv_BexpList2BeList(be_enc, cdr(bexp_list)) );
  }
}


/**Function********************************************************************

  Synopsis           [Removes from the cache those entries that depend on
  the given symbol]

  Description [Called by the BeEnc when removing a layer, to make safe
  later declaration of symbols with the same name but different
  semantics.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_Conv_cleanup_cached_entries_about(BeEnc_ptr be_enc,
                                           NodeList_ptr symbs)
{
  SymbTable_ptr st;
  assoc_iter iter;
  node_ptr expr;

  st = BaseEnc_get_symb_table(BASE_ENC(be_enc));

  ASSOC_FOREACH(bexpr2be_hash, iter, &expr, NULL) {
    Set_t deps = Formula_GetDependencies(st, expr, Nil);
    ListIter_ptr sym_iter;

    NODE_LIST_FOREACH(symbs, sym_iter) {
      node_ptr name = NodeList_get_elem_at(symbs, sym_iter);

      if (Set_IsMember(deps, name)) {
        /* removes the corresponding entry */
        bmc_conv_set_cache(expr, (be_ptr) NULL);
      }
    }

    Set_ReleaseSet(deps);
  }
}


/**Function********************************************************************

  Synopsis    [This function converts a BE model (i.e. a list of BE
  literals) to symbolic expressions.]

  Description [

  be_model is the model which will be transformed, i.e llList of
  BE literal.

  k is the number of steps (i.e. times+1) in the model.

  The returned results will be provided in:
  *frozen will point to expression over frozen variables,
  *states will point to an array of size k+1 to expressions over state vars.
  *inputs will point to an array of size k+1 to expressions over input vars.

  In arrays every index corresponds to the corresponding time,
  beginning from 0 for initial state.

  Every expressions is a list with AND used as connection and Nil at
  the end, i.e. it can be used as a list and as an expression.
  Every element of the list can have form:
  1) "var" or "!var" (if parameter convert_to_scalars is false)
  2) "var=const" (if parameter convert_to_scalar is true).

  By default BE literals are converted to bits of symbolic
  variables. With parameter convert_to_scalars set up the bits are
  converted to actual symbolic variables and scalar/word/etc
  values. Note however that if BE model does not provide a value for
  particular BE index then the corresponding bit may not be presented
  in the result expressions or may be given some random value
  (sometimes with convert_to_scalars set up). Note that in both cases
  the returned assignments may be incomplete.

  It is the responsibility of the invoker to free all arrays and the
  lists of expressions (i.e. run free_list on *frozen and every
  element of arrays returned).  EQUAL nodes (when convert_to_scalars
  is set up) are created with find_nodes, i.e. no freeing is need.

  No caching or other side-effect are applied ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_Conv_get_BeModel2SymbModel(const BeEnc_ptr be_enc,
                                    const Slist_ptr be_model,
                                    int k,
                                    boolean convert_to_scalars,
                                    node_ptr* frozen,
                                    array_t** states,
                                    array_t** inputs)
{
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  SymbTable_ptr symb_table = BaseEnc_get_symb_table(BASE_ENC(be_enc));

  Siter genLit;
  nusmv_ptrint lit;
  int iter;

  *frozen = Nil;
  *states = array_alloc(node_ptr, k+1);
  *inputs = array_alloc(node_ptr, k+1);

  for (iter=0; iter <= k; ++iter) {
    array_insert(node_ptr, *states, iter, Nil);
    array_insert(node_ptr, *inputs, iter, Nil);
  }


  /* ---- scan list of literals, convert them to bits and add to
          corresponding result expression ---- */
  SLIST_FOREACH(be_model, genLit) {
    int var_idx, ut_index, time;
    node_ptr var_expr;

    lit = (nusmv_ptrint) Siter_element(genLit);

    var_idx = Be_BeLiteral2BeIndex(be_mgr, lit);
    ut_index = BeEnc_index_to_untimed_index(be_enc, var_idx);
    time = BeEnc_index_to_time(be_enc, var_idx);

    var_expr = BeEnc_index_to_name(be_enc, ut_index);
    if (lit < 0) var_expr = Expr_not(var_expr);    /* negate if needed: */

    if (BeEnc_is_index_untimed_curr(be_enc, ut_index)) {
      /* state var can be only timed and in proper range */
      node_ptr expr;

      nusmv_assert(!BeEnc_is_index_untimed(be_enc, var_idx));
      nusmv_assert(0 <= time && time <= k);

      expr = array_fetch(node_ptr, *states, time);
      array_insert(node_ptr, *states, time,
                   new_node(AND, var_expr, expr));
    }
    else if (BeEnc_is_index_untimed_input(be_enc, ut_index)) {
      /* input var can be only timed and in proper range */
      node_ptr expr;

      nusmv_assert(!BeEnc_is_index_untimed(be_enc, var_idx));
      nusmv_assert(0 <= time && time <= k);

      expr = array_fetch(node_ptr, *inputs, time);
      array_insert(node_ptr, *inputs, time,
                   new_node(AND, var_expr, expr));
    }
    else {
      /* frozen vars are always untimed */
      nusmv_assert(BeEnc_is_index_untimed_frozen(be_enc, var_idx));
      nusmv_assert(time == BE_CURRENT_UNTIMED);

      *frozen = new_node(AND, var_expr, *frozen);
    }
  } /* end of literal scan */

  /* if conversion to scalars is not required then job is done */
  if (!convert_to_scalars) return;

  { /* ------ convert to scalar all the collected bits -------- */
    /* This is done the following way:
       for every expression list a hash table is created mapping
       a var to value of its bits. After the list is processed
       a new list is created with var=const expressions.
       NOTE: that some unassigned bits may become assigned in this process
    */

    /* to simplify iteration create a global set of all expressions */
    BoolEnc_ptr bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(be_enc));

    int all_array_iter;
    array_t* all_arrays[3] = {*states, *inputs, array_alloc(node_ptr, 1)};
    array_insert(node_ptr, all_arrays[2], 0, *frozen);

    /* -- iterate over all arrays */
    for(all_array_iter = 0; all_array_iter < 3; ++all_array_iter) {
      array_t* array = all_arrays[all_array_iter];
      int array_iter;
      node_ptr expr_list;
      /* -- iterate over given arrays, i.e. time steps */
      arrayForEachItem(node_ptr, array, array_iter, expr_list) {
        node_ptr new_expr_list = Nil; /* a scalar list instead of bits expr_list */
        node_ptr expr_iter;
        hash_ptr var2BitValues = new_assoc();

        /* -- iterate over a list of expressions */
        for (expr_iter = expr_list; Nil != expr_iter; expr_iter = cdr(expr_iter)) {
          node_ptr expr = car(expr_iter);
          boolean isNegation = node_get_type(expr) == NOT;
          node_ptr bit = isNegation ? car(expr) : expr;
          node_ptr var = BoolEnc_is_var_bit(bool_enc, bit) ?
            BoolEnc_get_scalar_var_from_bit(bool_enc, bit) : bit;
          int index;
          BitValues_ptr bitValues;

          /* we manage to get a proper scalar var */
          nusmv_assert(SymbTable_is_symbol_var(symb_table, var));

          /* if this is a boolean var => insert directly into the new list */
          if (var == bit) {
            expr = find_node(EQUAL, var,
                             isNegation ? Expr_false() : Expr_true());
            new_expr_list = new_node(AND, expr, new_expr_list);
            continue;
          }
          /* otherwise this is a proper bit of scalar var */
          nusmv_assert(bit != var);

          index = BoolEnc_get_index_from_bit(bool_enc, bit);

          /* get/create BitValues for a given var */
          bitValues = BIT_VALUES(find_assoc(var2BitValues, var));
          if (NULL == bitValues) {
            bitValues = BitValues_create(bool_enc, var);
            insert_assoc(var2BitValues, var, NODE_PTR(bitValues));
          }

          /* set the bit value */
          BitValues_set(bitValues, index,
                        isNegation ? BIT_VALUE_FALSE : BIT_VALUE_TRUE);
        } /* all expressions in the list has been processed */

        { /* iterate over elements of the hash table (i.e. scalar
             vars) and add to the new list "var=value" */
          assoc_iter aiter;
          node_ptr var;
          BitValues_ptr bitValues;

          ASSOC_FOREACH(var2BitValues, aiter, &var, &bitValues) {
            node_ptr value = BoolEnc_get_value_from_var_bits(bool_enc,
                                                             bitValues);

            new_expr_list = new_node(AND,
                                     new_node(EQUAL, var, value),
                                     new_expr_list);

            BitValues_destroy(bitValues);
          }
        }
        free_assoc(var2BitValues);

        /* free previous list of expression and insert the new one */
        free_list(expr_list);
        array_insert(node_ptr, array, array_iter, new_expr_list);
      } /* end of one array iteration */
    } /* end of all arrays iteration */

    /* get the frozen expression back */
    *frozen = array_fetch(node_ptr, all_arrays[2], 0);
    array_free(all_arrays[2]);
  }

}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Initializes module Conv]

  Description        [This package function is called by bmcPkg module]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_Conv_init_cache()
{
  nusmv_assert(bexpr2be_hash == (hash_ptr) NULL);

  bexpr2be_hash = new_assoc();
  nusmv_assert(bexpr2be_hash != (hash_ptr) NULL);
}


/**Function********************************************************************

  Synopsis           [De-initializes module Conv]

  Description        [This package function is called by bmcPkg module]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_Conv_quit_cache()
{
  if (bexpr2be_hash != (hash_ptr) NULL) {
    free_assoc(bexpr2be_hash);
    bexpr2be_hash = (hash_ptr) NULL;
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Sets a node into the stack]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static void Be2bexpDfsData_push(Be2bexpDfsData* data,
                                node_ptr value)
{
  (data -> head) ++;

  array_insert(node_ptr, data -> stack, data -> head, value);
}

/**Function********************************************************************

  Synopsis           [Be2bexpDfsData_head]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static node_ptr Be2bexpDfsData_head(Be2bexpDfsData* data)
{
  /* there is space available into the stack: */
  nusmv_assert((data -> head) != (-1));

  return (node_ptr) array_fetch(node_ptr, data->stack, data->head);
}

/**Function********************************************************************

  Synopsis           [Be2bexpDfsData_pop]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static node_ptr Be2bexpDfsData_pop(Be2bexpDfsData* data)
{
  node_ptr value = (node_ptr)Be2bexpDfsData_head(data);

  (data->head) --;

  return(value);
}

/**Function********************************************************************

  Synopsis           [Be2bexpSet]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static int Be2bexp_Set(be_ptr be, char* Be2bexpData, nusmv_ptrint sign)
{
  return -1;
}

/**Function********************************************************************

  Synopsis           [Be2bexpFirst]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static void Be2bexp_First(be_ptr be, char* Be2bexpData, nusmv_ptrint sign)
{
  return;
}

/**Function********************************************************************

  Synopsis           [Be2bexp_Back]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static void Be2bexp_Back(be_ptr be, char* Be2bexpData, nusmv_ptrint sign)
{
  return;
}

/**Function********************************************************************

  Synopsis           [Be2bexp_Last]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static void Be2bexp_Last(be_ptr be, char* Be2bexpData, nusmv_ptrint sign)
{
  int identifier = 0;
  int time, var_id;
  node_ptr left, right;
  Be_Manager_ptr be_mgr;
  Rbc_t* rbc;

  BeEnc_ptr be_enc = ((Be2bexpDfsData*)Be2bexpData)->be_enc;
  nusmv_assert(be_enc != NULL);

  be_mgr = BeEnc_get_be_manager(be_enc);
  rbc = (Rbc_t*)Be_Manager_Be2Spec(be_mgr, be);

  if (Rbc_is_top(rbc)) {
    if (sign == RBC_FALSE) {
      Be2bexpDfsData_push((Be2bexpDfsData*)Be2bexpData, Wff_make_falsity());
    }
    else {
      Be2bexpDfsData_push((Be2bexpDfsData*)Be2bexpData, Wff_make_truth());
    }
  }

  else if (Rbc_is_var(rbc)) {
    /* substitute the variable index, in the stack, with its correspondent
       state or frozen variable */

    time = BeEnc_index_to_time(be_enc, BeEnc_var_to_index(be_enc, rbc));
    /* frozen var has time BE_CURRENT_UNTIMED => make it 0 to avoid adding NEXTs */
    if (BE_CURRENT_UNTIMED == time) time = 0;
    var_id = BeEnc_index_to_untimed_index(be_enc,
                                          BeEnc_var_to_index(be_enc, rbc));

    if (sign == RBC_FALSE) {
      Be2bexpDfsData_push( (Be2bexpDfsData*) Be2bexpData,
        Wff_make_not(Wff_make_opnext_times(BeEnc_index_to_name(be_enc, var_id),
                                         time)) );
    }
    else {
      Be2bexpDfsData_push( (Be2bexpDfsData*) Be2bexpData,
           Wff_make_opnext_times(BeEnc_index_to_name(be_enc, var_id), time) );
    }
  }

  else if ((Rbc_is_and(rbc)) || (Rbc_is_iff(rbc))) {
    /* get the left bexp from the stack */
    right = Be2bexpDfsData_pop((Be2bexpDfsData*) Be2bexpData);

    /* get the right bexp from the stack */
    left = Be2bexpDfsData_pop((Be2bexpDfsData*) Be2bexpData);

    if (Rbc_is_and(rbc)) identifier = AND;
    else if (Rbc_is_iff(rbc)) identifier = IFF;

    if (sign == RBC_FALSE) {
      Be2bexpDfsData_push( (Be2bexpDfsData*) Be2bexpData,
                           Wff_make_not(find_node(identifier, left, right)) );
    }
    else {
      Be2bexpDfsData_push( (Be2bexpDfsData*) Be2bexpData,
                            find_node(identifier, left, right) );
    }
  }

  else {
    /* execution should never be here: */
    internal_error("rbc had an invalid value.\n");
  }
}

/**Function********************************************************************

  Synopsis           [Update the bexpr -> be cache]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void bmc_conv_set_cache(node_ptr bexp, be_ptr be)
{
  nusmv_assert(bexpr2be_hash != (hash_ptr) NULL);
  insert_assoc(bexpr2be_hash, bexp, (node_ptr) be);
}


/**Function********************************************************************

  Synopsis           [Queries the bexpr->be cache]

  Description        [Return NULL if association not found]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr bmc_conv_query_cache(node_ptr bexp)
{
  nusmv_assert(bexpr2be_hash != (hash_ptr) NULL);
  return (be_ptr) find_assoc(bexpr2be_hash, bexp);
}


/**Function********************************************************************

  Synopsis           [Private service for Bmc_Conv_Bexp2Be]

  Description        [Recursive service for Bmc_Conv_Bexp2Be, with caching of
  results]

  SideEffects        []

  SeeAlso            [Bmc_Conv_Bexp2Be]

******************************************************************************/
static be_ptr bmc_conv_bexp2be_recur(BeEnc_ptr be_enc, node_ptr bexp)
{
  be_ptr result = (be_ptr) NULL;

  /* if given expression is Nil, returns truth be */
  /* Nil value can be used in AND sequences, so a true value must
     be returned */
  if (bexp == Nil) return Be_Truth(BeEnc_get_be_manager(be_enc));

  /* queries the cache: */
  result = bmc_conv_query_cache(bexp);
  if (result != (be_ptr) NULL) return result;

  switch (node_get_type(bexp)) {
  case TRUEEXP:
    result = Be_Truth(BeEnc_get_be_manager(be_enc));
    break;

  case FALSEEXP:
    result = Be_Falsity(BeEnc_get_be_manager(be_enc));
    break;

  case NEXT:
    {
      SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(be_enc));

      nusmv_assert(SymbTable_is_symbol_state_frozen_var(st, car(bexp)));

      if (SymbTable_is_symbol_frozen_var(st, car(bexp))) {
        result = BeEnc_name_to_untimed(be_enc, car(bexp));
      }
      else {
        result = BeEnc_name_to_untimed(be_enc, bexp);
      }
    }
    break;

  case NOT:
    /* NOT arg is converted into an be, and its negation is result =ed */
    result = Be_Not( BeEnc_get_be_manager(be_enc),
                     bmc_conv_bexp2be_recur(be_enc, car(bexp)) );
    break;

  case CONS:
  case AND:
    result = Be_And( BeEnc_get_be_manager(be_enc),
                   bmc_conv_bexp2be_recur(be_enc, car(bexp)),
                   bmc_conv_bexp2be_recur(be_enc, cdr(bexp)) );
    break;

  case OR:
    result = Be_Or( BeEnc_get_be_manager(be_enc),
                  bmc_conv_bexp2be_recur(be_enc, car(bexp)),
                  bmc_conv_bexp2be_recur(be_enc, cdr(bexp)) );
    break;

  case XOR:
    result = Be_Xor( BeEnc_get_be_manager(be_enc),
                  bmc_conv_bexp2be_recur(be_enc, car(bexp)),
                  bmc_conv_bexp2be_recur(be_enc, cdr(bexp)) );
    break;

  case XNOR:
    result = Be_Not( BeEnc_get_be_manager(be_enc),
                   Be_Xor( BeEnc_get_be_manager(be_enc),
                           bmc_conv_bexp2be_recur(be_enc, car(bexp)),
                           bmc_conv_bexp2be_recur(be_enc, cdr(bexp)) ));
    break;

  case IFF:
    /* converts IFF args into two BEs, and result = an IFF BE with converted
       BEs as childs */
    result = Be_Iff( BeEnc_get_be_manager(be_enc),
                   bmc_conv_bexp2be_recur(be_enc, car(bexp)),
                   bmc_conv_bexp2be_recur(be_enc, cdr(bexp)) );
    break;

  case IMPLIES:
    /* convert IMPLIES args into two BEs, and result = the IMPLIES BE with
       converted BEs as childs */
    result = Be_Implies( BeEnc_get_be_manager(be_enc),
                       bmc_conv_bexp2be_recur(be_enc, car(bexp)),
                       bmc_conv_bexp2be_recur(be_enc, cdr(bexp)) );
    break;

  case EQUAL:
  case ASSIGN:
    /* converts EQUAL and ASSIGN args into two BEs, and result = an IFF BE
       with converted BEs as childs */
    result = Be_Iff( BeEnc_get_be_manager(be_enc),
                   bmc_conv_bexp2be_recur(be_enc, car(bexp)),
                   bmc_conv_bexp2be_recur(be_enc, cdr(bexp)) );
    break;

  case IFTHENELSE:
  case CASE: {
    /* converts "if"-"then"-"else" args of CASE into three BEs, and result =
       a BE If-Then-Else with converted BEs as childs */

    /* lazy evaluation and simplification is used here to get rid of
       FAILURE node in case-expressions
    */

    be_ptr cond = bmc_conv_bexp2be_recur(be_enc, caar(bexp));
    if (cond == Be_Truth(BeEnc_get_be_manager(be_enc))) {
      result = bmc_conv_bexp2be_recur(be_enc, cdar(bexp));
    }
    else if (cond == Be_Falsity(BeEnc_get_be_manager(be_enc))) {
      result = bmc_conv_bexp2be_recur(be_enc, cdr(bexp));
    }
    /* no simplification possible since the condition is not a constant */
    else result = Be_Ite( BeEnc_get_be_manager(be_enc),
                          cond,
                          bmc_conv_bexp2be_recur(be_enc, cdar(bexp)),
                          bmc_conv_bexp2be_recur(be_enc, cdr(bexp)));
    break;
  }

  case BIT:                              /* Variable */
  case DOT:                              /* Variable */
    result = BeEnc_name_to_untimed(be_enc, bexp);
    break;

  case ARRAY:
    /* Must be a boolean variable: */
    nusmv_assert(SymbTable_is_symbol_bool_var(
                      BaseEnc_get_symb_table(BASE_ENC(be_enc)),
                      bexp));

    result = BeEnc_name_to_untimed(be_enc, bexp);
    break;

  case ATOM:                             /* Variable */
    internal_error("Not DOT node as variable has been found!\n");

  case UNSIGNED_WORD:
    internal_error("Words cannot be met in boolean expressions!\n");
    /* result = bmc_conv_bexp2be_recur(be_enc, car(bexp)); */
    /* break; */

  case FAILURE:
    {
      if (failure_get_kind(bexp) == FAILURE_CASE_NOT_EXHAUSTIVE) {
        warning_case_not_exhaustive(bexp);
        /* forces a default */
        result = Be_Truth(BeEnc_get_be_manager(be_enc));
        break;
      }
      else if (failure_get_kind(bexp) == FAILURE_DIV_BY_ZERO) {
        warning_possible_div_by_zero(bexp);
        /* forces a default */
        result = Be_Truth(BeEnc_get_be_manager(be_enc));
        break;
      }
      else if (failure_get_kind(bexp) == FAILURE_ARRAY_OUT_OF_BOUNDS) {
        warning_possible_array_out_of_bounds(bexp);
        /* forces a default */
        result = Be_Truth(BeEnc_get_be_manager(be_enc));
        break;
      }
      else {
        report_failure_node(bexp); /* some error in the input expr */
      }
    }

  case NUMBER:
    /* zero and one are casted to their respective truth
       values. Other values fall down to an error */
    if (node_get_int(bexp) == 0) {
      result = Be_Falsity(BeEnc_get_be_manager(be_enc));
      break;
    }

    if (node_get_int(bexp) == 1) {
      result = Be_Truth(BeEnc_get_be_manager(be_enc));
      break;
    }

    /* no other values are allowed */
    internal_error("bmc_conv_bexp2be_recur: Unexpected number value [%d]\n",
                   node_get_int(bexp));

  default:
    print_sexp(stderr, bexp);
    internal_error("bmc_conv_bexp2be_recur: Unexpected case value. Node type = %d\n",
                   node_get_type(bexp));
  }

  /* updates the cache and returns result*/
  bmc_conv_set_cache( bexp, result);
  return result;
}

