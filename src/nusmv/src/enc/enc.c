/**CFile*****************************************************************

  FileName    [enc.c]

  PackageName [enc]

  Synopsis    [Package level code of package enc]

  Description []

  SeeAlso     [enc.h, encInt.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.

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

#include "enc.h"
#include "encInt.h"

#include "dd/VarsHandler.h"
#include "parser/symbols.h"
#include "compile/compile.h"
#include "utils/ucmd.h"
#include "utils/error.h"
#include "utils/assoc.h"


/*---------------------------------------------------------------------------*/
/* Macro definition                                                          */
/*---------------------------------------------------------------------------*/
#define  VARS_ORD_STR_INPUTS_BEFORE    "inputs_before"
#define  VARS_ORD_STR_INPUTS_AFTER     "inputs_after"
#define  VARS_ORD_STR_TOPOLOGICAL      "topological"
#define  VARS_ORD_STR_INPUTS_BEFORE_BI "inputs_before_bi"
#define  VARS_ORD_STR_INPUTS_AFTER_BI  "inputs_after_bi"
#define  VARS_ORD_STR_TOPOLOGICAL_BI   "topological_bi"
#define  VARS_ORD_STR_LEXICOGRAPHIC    "lexicographic"
#define  VARS_ORD_STR_UNKNOWN



#define BDD_STATIC_ORDER_HEURISTICS_STR_NONE       "none"
#define BDD_STATIC_ORDER_HEURISTICS_STR_BASIC      "basic"




/**Variable********************************************************************

  Synopsis    [Global bool encoding]

  Description [This instance must be accessed via dedicated methods]

  SeeAlso     []

******************************************************************************/
static BoolEnc_ptr global_bool_enc = BOOL_ENC(NULL);


/**Variable********************************************************************

  Synopsis    [Global encoding for BDDs]

  Description [This instance must be accessed via dedicated methods]

  SeeAlso     []

******************************************************************************/
static BddEnc_ptr global_bdd_enc = BDD_ENC(NULL);


/**Variable********************************************************************

  Synopsis    [Global encoding for BEs]

  Description [This instance must be accessed via dedicated methods]

  SeeAlso     []

******************************************************************************/
static BeEnc_ptr global_be_enc = BE_ENC(NULL);

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void
enc_construct_bdd_order_statically ARGS((FlatHierarchy_ptr flat_hierarchy,
                                         OrdGroups_ptr ord_groups));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the encoding package]

  Description        [This function initializes only data-structures
  global to all encoding.
  To initialize particular incoding, you have to invoke corresponding
  init-functions, such as Enc_init_bool_encoding, etc.]

  SideEffects        []

  SeeAlso            [Enc_init_bool_encoding, Enc_init_bdd_encoding,
  Enc_reinit_bdd_encoding, Enc_quit_encodings]
******************************************************************************/
void Enc_init_encodings()
{
}


/**Function********************************************************************

  Synopsis           [Adds commands related to Enc package]

  Description        [

  SideEffects        []

  SeeAlso            [Enc_init_encodings]
******************************************************************************/
void Enc_add_commands()
{
  enc_add_commands();
}


/**Function********************************************************************

  Synopsis           [Initializes the boolean encoding for this session]

  Description        [Call it to initialize for the current session the
  encoding, before flattening. In the current implementation, you must
  call this *before* the flattening phase. After the flattening,
  you must initialize the bdd encoding as well, and after you created the
  boolean sexp fsm, you must reinitialize the bdd encodings by calling
  Enc_reinit_bdd_encoding. Don't forget to call Enc_quit_encodings when
  the session ends. ]

  SideEffects        []

  SeeAlso            [Enc_init_bdd_encoding, Enc_reinit_bdd_encoding,
  Enc_quit_encodings]
******************************************************************************/
void Enc_init_bool_encoding()
{
  if (BOOL_ENC(NULL) == global_bool_enc) {

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stdout, "\nInitializing global boolean encoding...\n");
    }

    global_bool_enc = BoolEnc_create(Compile_get_global_symb_table());

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stdout, "Global boolean encoding initialized.\n");
    }
  }
}


/**Function********************************************************************

  Synopsis           [Initializes the bdd enc for this session]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Enc_init_bdd_encoding()
{
  extern VarsHandler_ptr dd_vars_handler;
  SymbTable_ptr st;
  OrdGroups_ptr ord_groups;

  BOOL_ENC_CHECK_INSTANCE(global_bool_enc);
  nusmv_assert(global_bdd_enc == BDD_ENC(NULL));


  if (!util_is_string_null(get_input_order_file(OptsHandler_get_instance()))) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
        fprintf(nusmv_stdout, "\nParsing the ordering file '");
        fprintf(nusmv_stdout, "%s", get_input_order_file(OptsHandler_get_instance()));
        fprintf(nusmv_stdout, "'...\n");
    }

   ord_groups = enc_utils_parse_ordering_file(get_input_order_file(OptsHandler_get_instance()),
                                               Enc_get_bool_encoding());
  }
  /* there is no ordering file => use heuristics to construct the order */
  else {
    ord_groups = OrdGroups_create();

    if (get_bdd_static_order_heuristics(OptsHandler_get_instance()) != BDD_STATIC_ORDER_HEURISTICS_NONE) {

      if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
        fprintf(nusmv_stdout, "\nHeuristics \"%s\" is going to be used to create var "
                "ordering statically\n",
                Enc_bdd_static_order_heuristics_to_string(
                            get_bdd_static_order_heuristics(OptsHandler_get_instance())));
      }

      if (mainFlatHierarchy == NULL) {
        fprintf(nusmv_stderr,
                "Warning for addons writers: static BDD variables order heuristics is \n"
                "   set up but flatten hierarchy has not been constructed. Switch off\n"
                "   static order heuristics or provide the flatten hierarchy.\n"
                "   See docs on bdd_static_order_heuristics variable.");
      }
      else {
        enc_construct_bdd_order_statically(mainFlatHierarchy, ord_groups);
      }
    }
    else {
      /* heuristics are switched off => do nothing */
    }
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stdout, "\nInitializing global BDD encoding...\n");
  }

  /* ord_groups will belong to global_bdd_enc */
  st = Compile_get_global_symb_table();
  global_bdd_enc = BddEnc_create(st,
                                 Enc_get_bool_encoding(),
                                 dd_vars_handler,
                                 ord_groups);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stdout, "Global BDD encoding initialized.\n");
  }
}


/**Function********************************************************************

  Synopsis           [Initializes the be enc for this session]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Enc_init_be_encoding()
{
  BOOL_ENC_CHECK_INSTANCE(global_bool_enc);
  nusmv_assert(global_be_enc == BE_ENC(NULL));

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stdout, "\nInitializing global BE encoding...\n");
  }

  global_be_enc = BeEnc_create(Compile_get_global_symb_table(),
                               Enc_get_bool_encoding());

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stdout, "Global BE encoding initialized.\n");
  }
}


/**Function********************************************************************

  Synopsis           [Call to destroy all encodings, when session ends]

  Description        [Call to destroy encodings, when session ends.
  Enc_init_encodings had to be called before calling this function.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Enc_quit_encodings()
{
  Enc_set_bdd_encoding(BDD_ENC(NULL));
  Enc_set_be_encoding(BE_ENC(NULL));
  Enc_set_bool_encoding(BOOL_ENC(NULL));
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
BoolEnc_ptr Enc_get_bool_encoding(void)
{
  BOOL_ENC_CHECK_INSTANCE(global_bool_enc);
  return global_bool_enc;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
BddEnc_ptr Enc_get_bdd_encoding(void)
{
  BDD_ENC_CHECK_INSTANCE(global_bdd_enc);
  return global_bdd_enc;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
BeEnc_ptr Enc_get_be_encoding(void)
{
  BE_ENC_CHECK_INSTANCE(global_be_enc);
  return global_be_enc;
}



/**Function********************************************************************

  Synopsis           [Set the global boolean encoding]

  Description [Set the global boolean encoding. If benc is NULL
  previous encoder is detroyed before the new assignment]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Enc_set_bool_encoding(BoolEnc_ptr benc)
{
  if (global_bool_enc != BOOL_ENC(NULL) && benc == BOOL_ENC(NULL)) {
    BoolEnc_destroy(global_bool_enc);
  }

  global_bool_enc = benc;
}


/**Function********************************************************************

  Synopsis           [Set the global bdd encoding]

  Description [Set the global bdd encoding. If enc is NULL,
  previously set encoder is detroyed before the new assignment]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Enc_set_bdd_encoding(BddEnc_ptr enc)
{
  if (global_bdd_enc != BDD_ENC(NULL) && enc == BDD_ENC(NULL)) {
    BddEnc_destroy(global_bdd_enc);
  }

  global_bdd_enc = enc;
}


/**Function********************************************************************

  Synopsis           [Set the global be encoding]

  Description [Set the global be encoding. If enc is NULL,
  previoulsy set encoder is detroyed before the new assignment]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Enc_set_be_encoding(BeEnc_ptr enc)
{
  if (global_be_enc != BE_ENC(NULL) && enc == BE_ENC(NULL)) {
    BeEnc_destroy(global_be_enc);
  }

  global_be_enc = enc;
}



/**Function********************************************************************

   Synopsis    [Given a boolean variable or a bit the function inserts it
   into the sorted list at proper position]

   Description [
   The higher bits are added to the beginning of the
   list and lower bits are added at the end. The boolean variables
   are added at the beginning of the list before the bits.

   A new element is added at the end of the group of equal elements,
   e.g. a boolean var is added after existing boolean vars but before
   the bit vars.

   Parameter 'sorting_cache' is used to speed up insertion (sorting a
   list will be linear instead of quadratic).  Initially
   'sorting_cache' has to point to a pointer which points to Nil and
   sorted_list has to be an empty list. It is the invoker
   responsibility to free the sorted list and cache (with free_list)
   after last invoking Enc_append_bit_to_sorted_list (the same
   sorting_cache and sorted_list can be used for several runs of this
   function).
   ]

   SideEffects []

   SeeAlso     []

******************************************************************************/
void
Enc_append_bit_to_sorted_list(SymbTable_ptr symb_table,
                              NodeList_ptr sorted_list, node_ptr var,
                              node_ptr* sorting_cache)
{
  node_ptr cache = *sorting_cache;
  int bit_number;
  ListIter_ptr iter;

  /* a var may be only a true bool var or a bit only */
  nusmv_assert(SymbTable_is_symbol_bool_var(symb_table, var) ||
               SymbType_calculate_type_size(SymbTable_get_var_type(symb_table, var)) == 1);

  /* NOTE about cache:
     In sorted list the vars are collected in groups. For example, there
     is a group corresponding to boolean vars, and there is a group for
     N-th bit vars.

     'cache' is a list. nodetype of every element is a number which
     correspond to the bit number of particular group and car is
     ListIter_ptr pointing to the last element of the group in
     sorted_list.  For boolean vars the number is SHRT_MAX.  If there is not
     element with particular number it means the list does not have such
     vars.

     'cache' are sorted by nodetype from higher to lower values (the
     same as in sorted list).
  */
  if (node_get_type(var) != BIT) bit_number = SHRT_MAX; /* a true boolean var */
  else {
    bit_number = NODE_TO_INT(cdr(var));
    nusmv_assert(bit_number < SHRT_MAX);/* implementation limit is not reached */
  }

  if (cache == Nil || node_get_type(cache) < bit_number) {
    /* add a new group at the beginning of both lists (cache and sorted list) */
    NodeList_prepend(sorted_list, var);
    iter = NodeList_get_first_iter(sorted_list);
    cache = new_node(bit_number, (node_ptr)iter, cache);
    *sorting_cache = cache;
    return;
  }

  /* find smaller group */
  while (cdr(cache) != Nil && node_get_type(cdr(cache)) >= bit_number) {
    cache = cdr(cache);
  }

  /* add the var */
  iter = LIST_ITER(car(cache));
  NodeList_insert_after(sorted_list, iter, var);
  iter = ListIter_get_next(iter);

  if (node_get_type(cache) == bit_number) {
    /* such group already exists => just update the iterator*/
    setcar(cache, (node_ptr)iter);
    return;
  }
  else {
    /* we are at the end of the list or before smaller group => create new group */
    nusmv_assert(node_get_type(cache) > bit_number);
    nusmv_assert(cdr(cache) == Nil || node_get_type(cdr(cache)) < bit_number);

    setcdr(cache, new_node(bit_number, (node_ptr)iter, cdr(cache)));
    return;
  }
}


/**Function********************************************************************

  Synopsis           [Returns the string corresponding to give parameter]

  Description        [Returned string does not have to be freed]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
const char* Enc_vars_ord_to_string(VarsOrdType vot)
{
  switch (vot) {

  case VARS_ORD_INPUTS_BEFORE: return VARS_ORD_STR_INPUTS_BEFORE;
  case VARS_ORD_INPUTS_AFTER: return VARS_ORD_STR_INPUTS_AFTER;
  case VARS_ORD_TOPOLOGICAL: return VARS_ORD_STR_TOPOLOGICAL;
  case VARS_ORD_INPUTS_BEFORE_BI: return VARS_ORD_STR_INPUTS_BEFORE_BI;
  case VARS_ORD_INPUTS_AFTER_BI: return VARS_ORD_STR_INPUTS_AFTER_BI;
  case VARS_ORD_TOPOLOGICAL_BI: return VARS_ORD_STR_TOPOLOGICAL_BI;
  case VARS_ORD_UNKNOWN: internal_error("Wrong var ordering type");
  default:
    error_unreachable_code(); /* no other possible cases */
  }
  return NULL;
}


/**Function********************************************************************

  Synopsis           [Converts a string to the corresponding var order type.]

  Description        [VARS_ORD_UNKNOWN is returned when the string does not
  match the given string]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
VarsOrdType Enc_string_to_vars_ord(const char* str)
{
  if (strcmp(str, VARS_ORD_STR_INPUTS_BEFORE) == 0) {
    return VARS_ORD_INPUTS_BEFORE;
  }
  if (strcmp(str, VARS_ORD_STR_INPUTS_AFTER) == 0) {
    return VARS_ORD_INPUTS_AFTER;
  }
  if (strcmp(str, VARS_ORD_STR_TOPOLOGICAL) == 0) {
    return VARS_ORD_TOPOLOGICAL;
  }
  if (strcmp(str, VARS_ORD_STR_INPUTS_BEFORE_BI) == 0) {
    return VARS_ORD_INPUTS_BEFORE_BI;
  }
  if (strcmp(str, VARS_ORD_STR_INPUTS_AFTER_BI) == 0) {
    return VARS_ORD_INPUTS_AFTER_BI;
  }
  if (strcmp(str, VARS_ORD_STR_TOPOLOGICAL_BI) == 0) {
    return VARS_ORD_TOPOLOGICAL_BI;
  }
  /* deprecated feature */
  if (strcmp(str, VARS_ORD_STR_LEXICOGRAPHIC) == 0) {
    fprintf(nusmv_stderr, "Warning: value \"" VARS_ORD_STR_LEXICOGRAPHIC
            "\" is a deprecated feature. Use \"" VARS_ORD_STR_TOPOLOGICAL
            "\" instead.\n");
    return VARS_ORD_TOPOLOGICAL;
  }
  return VARS_ORD_UNKNOWN;
}


/**Function********************************************************************

  Synopsis           [Returns a string of all possible values for
  vars_ord_type]

  Description        [Returned string does not have to be freed]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
const char* Enc_get_valid_vars_ord_types()
{
  return
    VARS_ORD_STR_INPUTS_BEFORE ", " \
    VARS_ORD_STR_INPUTS_AFTER  ", " \
    VARS_ORD_STR_TOPOLOGICAL ", "\
    VARS_ORD_STR_INPUTS_BEFORE_BI ", " \
    VARS_ORD_STR_INPUTS_AFTER_BI  ", " \
    VARS_ORD_STR_TOPOLOGICAL_BI;
}


/**Function********************************************************************

  Synopsis           [Returns the string corresponding to give parameter]

  Description        [Returned string does not have to be freed]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
const char* Enc_bdd_static_order_heuristics_to_string(BddSohEnum value)
{
  switch (value) {
  case BDD_STATIC_ORDER_HEURISTICS_NONE: return BDD_STATIC_ORDER_HEURISTICS_STR_NONE;
  case BDD_STATIC_ORDER_HEURISTICS_BASIC: return BDD_STATIC_ORDER_HEURISTICS_STR_BASIC;
  default:
    error_unreachable_code(); /* no other possible cases */
  }
  return NULL;
}


/**Function********************************************************************

  Synopsis    [Converts a string to the corresponding BDD Static Order Heuristics.]

  Description [BDD_STATIC_ORDER_HEURISTICS_ERROR is returned when the
  string does not match the given string]

  SideEffects []

  SeeAlso     []
******************************************************************************/
BddSohEnum Enc_string_to_bdd_static_order_heuristics(const char* str)
{
  if (strcmp(str, BDD_STATIC_ORDER_HEURISTICS_STR_NONE) == 0) {
    return BDD_STATIC_ORDER_HEURISTICS_NONE;
  }
  if (strcmp(str, BDD_STATIC_ORDER_HEURISTICS_STR_BASIC) == 0) {
    return BDD_STATIC_ORDER_HEURISTICS_BASIC;
  }
  return BDD_STATIC_ORDER_HEURISTICS_ERROR;
}


/**Function********************************************************************

  Synopsis           [Returns a string of all possible values for
  bdd_static_order_heuristics]

  Description        [Returned string does not have to be freed]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
const char* Enc_get_valid_bdd_static_order_heuristics()
{
  return
    BDD_STATIC_ORDER_HEURISTICS_STR_NONE ", " \
    BDD_STATIC_ORDER_HEURISTICS_STR_BASIC;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [The function is trying to construct a BDD var order using
  heuristics by analyzing the flattened model]

  Description [Shell variable vars_order_type is use to infer the initial order
  to begin the analysis. Then analyzing the flattened model and using heuristics
  the order is tried to improve.
  The kind of heuristics to be used is defined by shell variable
  bdd_static_order_heuristics.

  The final order is returned in ord_groups which has to be empty at
  the beginning.]

  SideEffects []

  SeeAlso     []
******************************************************************************/
void enc_construct_bdd_order_statically(FlatHierarchy_ptr flat_hierarchy,
                                        OrdGroups_ptr ord_groups)
{
  /* Discussion about the algorithm.  The only necessary thing to get
     is clusters computed by PredicateExtractor. The actual predicates
     are of no importance.  Unfortunately, it is more difficult to
     compute clusters only, without predicates. See
     PredicateExtractor.h for more into.

     Note: predicate give information only about scalar and word
     vars. Boolean vars interaction are completely ignored.
     A special function would be able to collect all the information */

    SymbTable_ptr st = Compile_get_global_symb_table();
    PredicateExtractor_ptr extractor = PredicateExtractor_create(st, true);

    NodeList_ptr original_var_order;
    SymbTableType filter[3];
    SymbTableIter stiter;
    int i = 0;
    boolean isBitInterleaved = false;
    hash_ptr cache = new_assoc();

    nusmv_assert(flat_hierarchy != NULL);

    /* currently only one heuristics is implemented.
       NONE value has to be detected by the invoker of this function. */
    nusmv_assert(get_bdd_static_order_heuristics(OptsHandler_get_instance()) ==
                 BDD_STATIC_ORDER_HEURISTICS_BASIC);

    /* create the original var order to begin with */
    original_var_order = NodeList_create();
    switch(get_vars_order_type(OptsHandler_get_instance())) {
    case VARS_ORD_INPUTS_BEFORE_BI:
      isBitInterleaved = true;  /* no break here */
    case VARS_ORD_INPUTS_BEFORE:
      filter[0] = STT_INPUT_VAR;
      filter[1] = STT_STATE_VAR | STT_FROZEN_VAR;
      filter[2] = STT_NONE;
      break;

    case VARS_ORD_INPUTS_AFTER_BI:
      isBitInterleaved = true;    /* no break here */
    case VARS_ORD_INPUTS_AFTER:
      filter[0] = STT_STATE_VAR | STT_FROZEN_VAR;
      filter[1] = STT_INPUT_VAR;
      filter[2] = STT_NONE;
      break;

    case VARS_ORD_TOPOLOGICAL_BI:
      isBitInterleaved = true;   /* no break here */
    case VARS_ORD_TOPOLOGICAL:
      filter[0] = STT_VAR;
      filter[1] = STT_NONE;
      break;

    default:
      error_unreachable_code(); /* no other possible cases */
    }

    i = 0;
    while (filter[i] != STT_NONE) {
      SYMB_TABLE_FOREACH(st, stiter, filter[i]) {
        NodeList_append(original_var_order,
                        SymbTable_iter_get_symbol(st, &stiter));
      }
      ++i;
    }

    /* get rid of bits in the list */
    {
      ListIter_ptr iter = NodeList_get_first_iter(original_var_order);
      while (!ListIter_is_end(iter)) {
        node_ptr var = NodeList_get_elem_at(original_var_order, iter);
        ListIter_ptr tmp = iter;
        iter = ListIter_get_next(tmp);
        if (node_get_type(var) == BIT) {
          NodeList_remove_elem_at(original_var_order, tmp);
        }
      }
    }

    /* construct predicates from all the expressions of the hierarchy. */
    PredicateExtractor_compute_preds_from_hierarchy(extractor, flat_hierarchy);

    /* convert vars to bits and add the bits to the list.  The vars in
       cluster are added with bits interleaved.  Bits are added in the
       same order as the vars are met in the model. A set is added at
       the place of its highest var in the list. */
    while (NodeList_get_length(original_var_order) != 0) {
      NodeList_ptr subListVars = NodeList_create();
      NodeList_ptr subListBits = NodeList_create();
      ListIter_ptr subIter;
      node_ptr sorting_cache = Nil;

      node_ptr var = NodeList_get_elem_at(original_var_order,
                                          NodeList_get_first_iter(original_var_order));
      Set_t cluster = PredicateExtractor_get_var_cluster(extractor, var);

      if (NULL == cluster) { /* not grouped var */
        NodeList_append(subListVars, var);

        NodeList_remove_elem_at(original_var_order,
                                NodeList_get_first_iter(original_var_order));

      }
      else { /* a group of vars has to be added */
        /* remove all the vars from the original list and the hash */

        /* It is useless to continue iterating over the NodeList if
           all variables in the cluster have been already
           removed. Keep trace of this with this reverse counter */
        int missing = Set_GiveCardinality(cluster);

        subIter = NodeList_get_first_iter(original_var_order);
        while (!ListIter_is_end(subIter) && (missing > 0)) {
          node_ptr var = NodeList_get_elem_at(original_var_order, subIter);
          ListIter_ptr currIter = subIter;

          subIter = ListIter_get_next(subIter);

          if (Set_IsMember(cluster, (Set_Element_t)var)) {
            node_ptr tmp = NodeList_remove_elem_at(original_var_order, currIter);
            nusmv_assert(var == tmp);
            NodeList_append(subListVars, var);

            /* Found variable, decrease the missing counter */
            --missing;
          }
        }

        nusmv_assert(missing == 0);
      }

      /* Now prepare the cluster for insertion to the order list, i.e.
         divide the order on bits and interleave them if required.
         Here bit interleaving may or may not be used.
       */
      NODE_LIST_FOREACH(subListVars, subIter) {
        var = NodeList_get_elem_at(subListVars, subIter);
        if (SymbTable_is_symbol_bool_var(st, var)) {
          if (isBitInterleaved) {
            /* NB: for boolean interleaving actually may be of no importance? */
            Enc_append_bit_to_sorted_list(st, subListBits, var,
                                          &sorting_cache);
          }
          else NodeList_append(subListBits, var);
        }
        else {

          /* Skip variables that are not booleanizable */
          if (Compile_is_expr_booleanizable(st, var, false, cache)) {
            /* pushes the bits composing the scalar variable.
               Grouping is done by caller function */
            NodeList_ptr bits = BoolEnc_get_var_bits(Enc_get_bool_encoding(),
                                                     var);
            ListIter_ptr bit_iter;
            NODE_LIST_FOREACH(bits,bit_iter) {
              node_ptr varBit = NodeList_get_elem_at(bits, bit_iter);
              if (isBitInterleaved) {
                Enc_append_bit_to_sorted_list(st, subListBits, varBit,
                                              &sorting_cache);
              }
              else NodeList_append(subListBits, varBit);
            }
            NodeList_destroy(bits);
          }

        }
      }

      /* append the bits to the global order list */
      NODE_LIST_FOREACH(subListBits, subIter) {
        /* Currently, every bit is inserted into the ordering individually.
           In future we may want to group the bits of one var as it is done
           in bdd_enc_sort_variables_and_groups_according */

        var = NodeList_get_elem_at(subListVars, subIter);

        /* every bit is inserted only once */
        nusmv_assert(-1 == OrdGroups_get_var_group(ord_groups, var));

        OrdGroups_add_variable(ord_groups, var,
                               OrdGroups_create_group(ord_groups));
      }

      free_list(sorting_cache);
      NodeList_destroy(subListBits);
      NodeList_destroy(subListVars);
    }

    NodeList_destroy(original_var_order);

    PredicateExtractor_destroy(extractor);
    free_assoc(cache);

    /* end of static order computation */
    return;
}

