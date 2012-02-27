/**CFile***********************************************************************

  FileName    [sbmcTableauInc.c]

  PackageName [bmc.sbmc]

  Synopsis    [High level generic tableau routines for incremental SBMC.]

  Description [High level generic tableau routines for incremental SBMC.]

  Author      [Tommi Junttila, Marco Roveri]

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2. 
  Copyright (C) 2006 by Tommi Junttila.

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

#include <stdlib.h>
#include <stdio.h>

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "sbmcUtils.h"
#include "sbmcTableauInc.h"

#include "bmc/bmcInt.h"

#include "utils/list.h"
#include "utils/assoc.h"
#include "node/node.h"
#include "parser/symbols.h" /* for tokens */
#include "be/be.h"
#include "enc/enc.h"
#include "enc/be/BeEnc.h"
#include "opt/opt.h"

static char rcsid[] UTIL_UNUSED = "$Id: sbmcTableauInc.c,v 1.1.2.5.4.4 2009-09-17 11:49:47 nusmv Exp $";

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
EXTERN FILE * nusmv_stderr;

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

  Synopsis           [Makes the BE formula "\land_{v \in vars} s_i = s_j"]

  Description        [Creates the BE for the formula "\land_{v \in vars} s_i = s_j"]

  SideEffects        [None]

******************************************************************************/
be_ptr sbmc_equal_vectors_formula(const BeEnc_ptr be_enc,
                                  lsList vars,
                                  const unsigned int i,
                                  const unsigned int j)
{
  lsGen    iterator;
  node_ptr node_sv;
  be_ptr be_constraint;
  Be_Manager_ptr be_mgr;

  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert((Be_Manager_ptr)NULL != be_mgr);

  /* Initialize constraint */
  be_constraint = Be_Truth(be_mgr);

  lsForEachItem(vars, iterator, node_sv) {
    /* Get state variable sv at time i */
    be_ptr be_sv_i = BeEnc_name_to_timed(be_enc, node_sv, i);
    /* Get state variable sv at time j */
    be_ptr be_sv_j = BeEnc_name_to_timed(be_enc, node_sv, j);

    be_constraint = Be_And(be_mgr,
                           be_constraint,
                           Be_Iff(be_mgr, be_sv_i, be_sv_j));
  }

  return be_constraint;
}


/**Function********************************************************************

  Synopsis           [Associates each subformula node of ltlspec with
  a sbmc_LTL_info.]

  Description        [Associates each subformula node of ltlspec with
  a sbmc_LTL_info. Returns a hash from node_ptr to sbmc_LTL_info*.
  New state variables named #LTL_t'i' can be allocate to 'layer'.
  The new state vars are inserted in state_vars_formula_??? appropriately.]

  SideEffects        [None]

******************************************************************************/
hash_ptr sbmc_init_LTL_info(SymbLayer_ptr layer, node_ptr ltlspec,
                                lsList state_vars_formula_pd0,
                                lsList state_vars_formula_pdx,
                                lsList state_vars_formula_aux,
                                const int opt_force_state_vars,
                                const int opt_do_virtual_unrolling)
{
  hash_ptr info_map = (hash_ptr)NULL;
  lsList   unprocessed_nodes = (lsList)NULL;
  unsigned int new_var_index = 0;

  nusmv_assert((lsList)NULL != state_vars_formula_pd0);
  nusmv_assert((lsList)NULL != state_vars_formula_pdx);
  nusmv_assert((lsList)NULL != state_vars_formula_aux);

  /* Debug output */
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 4))
    sbmc_print_node(nusmv_stderr, "Computing the past depth information and"
                   " allocating variables for ", ltlspec, "\n");

  info_map = sbmc_node_info_assoc_create();
  nusmv_assert(info_map != (hash_ptr)NULL);

  unprocessed_nodes = lsCreate();
  lsNewBegin(unprocessed_nodes, (lsGeneric)ltlspec, LS_NH);
  
  while (lsLength(unprocessed_nodes) > 0) {
    node_ptr node = (node_ptr)NULL, lsf = (node_ptr)NULL, rsf = (node_ptr)NULL;
    sbmc_node_info * info = (sbmc_node_info *)NULL;
    sbmc_node_info * lsf_info = (sbmc_node_info *)NULL;
    sbmc_node_info * rsf_info = (sbmc_node_info *)NULL;
    int has_unprocessed_children;

    /* Get node */
    if ((lsFirstItem(unprocessed_nodes, (lsGeneric*)&node, LS_NH) != LS_OK) ||
        ((node_ptr)NULL == node))
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);

    /* Is info already computed? */
    if (sbmc_node_info_assoc_find(info_map, node) != 0) {
      /* Remove from unprocessed stack */
      if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
        internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      continue;
    }
      
    /* Traverse children and build info */
    lsf = car(node);
    rsf = cdr(node);      
    has_unprocessed_children = 0;
    switch(node_get_type(node)) {
    case ATOM:
    case BIT:
    case DOT:
    case ARRAY:
    case TRUEEXP:
    case FALSEEXP:
      /* Base case: no children */
      info = sbmc_alloc_node_info();
      sbmc_node_info_set_past_depth(info, 0);
      if (opt_force_state_vars) {
        /* Formula variable translation */
        sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                                 state_vars_formula_pdx, &new_var_index);
        sbmc_print_varmap(nusmv_stderr, node, info);
      }
      else {
        /* Definitional translation */
        sbmc_node_info_set_past_trans_vars(info, 0);
      }
      sbmc_node_info_assoc_insert(info_map, node, info);
      break;

    case AND:
    case OR:
    case XOR:
    case XNOR:
    case IMPLIES:
    case IFF:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if ((sbmc_node_info *)NULL == rsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, max(sbmc_node_info_get_past_depth(lsf_info),
                                                    sbmc_node_info_get_past_depth(rsf_info)));
      else
        sbmc_node_info_set_past_depth(info, 0);

      if (opt_force_state_vars) {
        /* Formula variable translation */
        sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                                 state_vars_formula_pdx, &new_var_index);
        sbmc_print_varmap(nusmv_stderr, node, info);
      }
      else {
        /* Definitional translation */
        sbmc_node_info_set_past_trans_vars(info, 0);
      }
      sbmc_node_info_assoc_insert(info_map, node, info);
      break;
          
    case NOT:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, sbmc_node_info_get_past_depth(lsf_info));
      else
        sbmc_node_info_set_past_depth(info, 0);

      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);

      if (opt_force_state_vars) {
        /* Formula variable translation */
        sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                                 state_vars_formula_pdx, &new_var_index);
        sbmc_print_varmap(nusmv_stderr, node, info);
      }
      else {
        /* Definitional translation */
        sbmc_node_info_set_past_trans_vars(info, 0);
      }
      sbmc_node_info_assoc_insert(info_map, node, info);
      break;

          
    case OP_FUTURE:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, sbmc_node_info_get_past_depth(lsf_info));
      else
        sbmc_node_info_set_past_depth(info, 0);

      sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                               state_vars_formula_pdx, &new_var_index);
      sbmc_print_varmap(nusmv_stderr, node, info);
      sbmc_node_info_assoc_insert(info_map, node, info);

      /* Aux_F_node for the subformula */
      if (sbmc_node_info_get_aux_F_node(lsf_info) == 0) {
        sbmc_node_info_set_aux_F_node(lsf_info, sbmc_1_fresh_state_var(layer, &new_var_index));
        lsNewBegin(state_vars_formula_aux,
                   (lsGeneric)sbmc_node_info_get_aux_F_node(lsf_info), LS_NH);
        sbmc_print_Fvarmap(nusmv_stderr, sbmc_node_info_get_aux_F_node(lsf_info), lsf);
      }
      break;


    case OP_GLOBAL:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, sbmc_node_info_get_past_depth(lsf_info));
      else
        sbmc_node_info_set_past_depth(info, 0);

      sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                               state_vars_formula_pdx, &new_var_index);
      sbmc_print_varmap(nusmv_stderr, node, info);
      sbmc_node_info_assoc_insert(info_map, node, info);

      /* Aux_G_node for the subformula */
      if (sbmc_node_info_get_aux_G_node(lsf_info) == 0) {
        sbmc_node_info_set_aux_G_node(lsf_info, sbmc_1_fresh_state_var(layer, &new_var_index));
        lsNewBegin(state_vars_formula_aux,
                   (lsGeneric)sbmc_node_info_get_aux_G_node(lsf_info), LS_NH);
        sbmc_print_Gvarmap(nusmv_stderr, sbmc_node_info_get_aux_G_node(lsf_info), lsf);
      }
      break;


    case OP_NEXT:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, sbmc_node_info_get_past_depth(lsf_info));
      else
        sbmc_node_info_set_past_depth(info, 0);

      sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                               state_vars_formula_pdx, &new_var_index);
      sbmc_print_varmap(nusmv_stderr, node, info);
      sbmc_node_info_assoc_insert(info_map, node, info);

      /* Subformula referred in the L and k+1 state,
         make sure it can be free i.e. force it to be translated via var */
      if (sbmc_node_info_get_trans_vars(lsf_info) == 0) {
        sbmc_allocate_trans_vars(lsf_info, layer, state_vars_formula_pd0,
                                 state_vars_formula_pdx, &new_var_index);
        sbmc_print_varmap(nusmv_stderr, lsf, lsf_info);
      }
      break;


    case OP_HISTORICAL:
    case OP_ONCE:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, sbmc_node_info_get_past_depth(lsf_info) + 1);
      else
        sbmc_node_info_set_past_depth(info, 0);

      sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                               state_vars_formula_pdx, &new_var_index);
      sbmc_print_varmap(nusmv_stderr, node, info);
      sbmc_node_info_assoc_insert(info_map, node, info);
      break;



    case OP_PREC:
    case OP_NOTPRECNOT:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, sbmc_node_info_get_past_depth(lsf_info) + 1);
      else
        sbmc_node_info_set_past_depth(info, 0);

      sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                               state_vars_formula_pdx, &new_var_index);
      sbmc_print_varmap(nusmv_stderr, node, info);
      sbmc_node_info_assoc_insert(info_map, node, info);

      /* Subformula referred in the E state, make sure it can be free */
      if (sbmc_node_info_get_trans_vars(lsf_info) == 0) {
        sbmc_allocate_trans_vars(lsf_info, layer, state_vars_formula_pd0,
                                 state_vars_formula_pdx, &new_var_index);
        sbmc_print_varmap(nusmv_stderr, lsf, lsf_info);
      }
      break;


    case UNTIL:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if ((sbmc_node_info *)NULL == rsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, max(sbmc_node_info_get_past_depth(lsf_info),
                                                    sbmc_node_info_get_past_depth(rsf_info)));
      else
        sbmc_node_info_set_past_depth(info, 0);

      sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                               state_vars_formula_pdx, &new_var_index);
      sbmc_print_varmap(nusmv_stderr, node, info);
      sbmc_node_info_assoc_insert(info_map, node, info);

      /* Aux_F_node for the right subformula */
      if (sbmc_node_info_get_aux_F_node(rsf_info) == 0) {
        sbmc_node_info_set_aux_F_node(rsf_info, sbmc_1_fresh_state_var(layer, &new_var_index));
        lsNewBegin(state_vars_formula_aux,
                   (lsGeneric)sbmc_node_info_get_aux_F_node(rsf_info), LS_NH);
        sbmc_print_Fvarmap(nusmv_stderr, sbmc_node_info_get_aux_F_node(rsf_info), rsf);
      }
      break;


    case RELEASES:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if ((sbmc_node_info *)NULL == rsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, max(sbmc_node_info_get_past_depth(lsf_info),
                                                    sbmc_node_info_get_past_depth(rsf_info)));
      else
        sbmc_node_info_set_past_depth(info, 0);

      sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                               state_vars_formula_pdx, &new_var_index);
      sbmc_print_varmap(nusmv_stderr, node, info);
      sbmc_node_info_assoc_insert(info_map, node, info);

      /* Aux_G_node for the right subformula */
      if (sbmc_node_info_get_aux_G_node(rsf_info) == 0) {
        sbmc_node_info_set_aux_G_node(rsf_info, sbmc_1_fresh_state_var(layer, &new_var_index));
        lsNewBegin(state_vars_formula_aux,
                   (lsGeneric)sbmc_node_info_get_aux_G_node(rsf_info), LS_NH);
        sbmc_print_Gvarmap(nusmv_stderr, sbmc_node_info_get_aux_G_node(rsf_info), rsf);
      }
      break;


    case TRIGGERED:
    case SINCE:
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      if ((sbmc_node_info *)NULL == lsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if ((sbmc_node_info *)NULL == rsf_info) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      info = sbmc_alloc_node_info();

      if (opt_do_virtual_unrolling)
        sbmc_node_info_set_past_depth(info, max(sbmc_node_info_get_past_depth(lsf_info),
                                                    sbmc_node_info_get_past_depth(rsf_info)) + 1);
      else
        sbmc_node_info_set_past_depth(info, 0);

      sbmc_allocate_trans_vars(info, layer, state_vars_formula_pd0,
                               state_vars_formula_pdx, &new_var_index);
      sbmc_print_varmap(nusmv_stderr, node, info);
      sbmc_node_info_assoc_insert(info_map, node, info);
      break;

          
    default:
      internal_error(sbmc_SNYI_text, __FILE__, __LINE__);
      break;
    }
    if (has_unprocessed_children)
      continue;
      
    nusmv_assert(sbmc_node_info_assoc_find(info_map, node) != 0);

    /* Remove from unprocessed stack */
    if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);

    /* Debug output */
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
      sbmc_print_node(nusmv_stderr, " The past depth of ", node, " is ");
      fprintf(nusmv_stderr, "%u\n", sbmc_node_info_get_past_depth(info));
    }
  }
      
  lsDestroy(unprocessed_nodes, NULL);

  return info_map;
}

/**Function********************************************************************

  Synopsis           [Initialize trans_bes[i][d] for each sub-formula.]

  Description        [ Initialize trans_bes[i][d], 0<=d<=pd, to
  <ul>
    <li> the formula [[f]]_i^d for definitionally translated subformulae</li>
    <li> the [[f]]_i^d be variable for variable translated subformulae</li>
  </ul> ]

  SideEffects        [None]

******************************************************************************/
void sbmc_init_state_vector(const BeEnc_ptr be_enc,
                            const node_ptr ltlspec,
                            const hash_ptr info_map,
                            const unsigned int i_real,
                            const node_ptr LastState_var,
                            const be_ptr be_LoopExists)
{
  unsigned int d;
  Be_Manager_ptr be_mgr;
  SymbTable_ptr st = (SymbTable_ptr)NULL;
  hash_ptr visit_cache = (hash_ptr)NULL;
  lsList   unprocessed_nodes = (lsList)NULL;
  char *i_str = sbmc_real_k_string(i_real);

  nusmv_assert(info_map != (hash_ptr)NULL);

  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);

  /* Get symbol table */
  st = BaseEnc_get_symb_table(BASE_ENC(be_enc));
  nusmv_assert((SymbTable_ptr)NULL != st);

  /* Debug output */
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Initializing state vector at time %s\n", i_str);
    fflush(nusmv_stderr);
  }
  
  
  visit_cache = sbmc_set_create();
  unprocessed_nodes = lsCreate();
  lsNewBegin(unprocessed_nodes, (lsGeneric)ltlspec, LS_NH);
  
  while(lsLength(unprocessed_nodes) > 0) {
    /* Nodes for the current formula and its subformulae */
    node_ptr f_node, lsf_node, rsf_node;
    /* The corresponding node info structures */
    sbmc_node_info *f_info, *lsf_info, *rsf_info;
    array_t *past_array, *lsf_past_array, *rsf_past_array;
    int has_unprocessed_children;

    /* Get the current formula to f_node */
    if ((lsFirstItem(unprocessed_nodes, (lsGeneric*)&f_node, LS_NH) != LS_OK) ||
        ((node_ptr)NULL == f_node))
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Get the info structure of the formula */
    f_info = sbmc_node_info_assoc_find(info_map, f_node);
    if ((sbmc_node_info *)NULL == f_info)
      internal_error(sbmc_SNH_text,__FILE__,__LINE__);
      
    /* Get past_array */
    {
      array_t * bearray = sbmc_node_info_get_trans_bes(f_info);

      nusmv_assert((array_t *)NULL != bearray);
      while(array_n(bearray) <= i_real)
        array_insert(array_t *, bearray, array_n(bearray), (array_t *)NULL);
      past_array = array_fetch(array_t *, bearray, i_real);
    }

    /* Already visited? */
    if (sbmc_set_is_in(visit_cache, f_node)) {
      nusmv_assert((array_t *)NULL != past_array);
      /* Remove node from unprocessed stack */
      if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&f_node) != LS_OK)
        internal_error(sbmc_SNH_text,__FILE__,__LINE__);
      continue;
    }
      

    /* Initialize past_array[i] if not already done */
    if ((array_t *)NULL == past_array) {
      past_array = array_alloc(be_ptr, sbmc_node_info_get_past_depth(f_info) + 1);
      for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++)
        array_insert(be_ptr, past_array, d, (be_ptr)NULL);
      array_insert(array_t *, sbmc_node_info_get_trans_bes(f_info), i_real, past_array);
    }

    for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++)
      nusmv_assert(array_fetch(be_ptr, past_array, d) == (be_ptr)NULL);      

    /* Traverse children and build past_array */
    lsf_node = car(f_node);
    rsf_node = cdr(f_node);
    has_unprocessed_children = 0;
    switch(node_get_type(f_node)) {
          
    case ATOM:
    case BIT:
    case DOT:
    case ARRAY: {
      be_ptr be_result = (be_ptr)NULL;
            
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, " ", f_node, "\n");
            
      nusmv_assert(sbmc_node_info_get_past_depth(f_info) == 0);

      if (sbmc_node_info_get_trans_vars(f_info) != (array_t *)NULL) {
        /* main_array[i][0] = [[p]]_i */
        node_ptr node_p;

        node_p = array_fetch(node_ptr, sbmc_node_info_get_trans_vars(f_info), 0);
        nusmv_assert((node_ptr)NULL != node_p);
        be_result = BeEnc_name_to_timed(be_enc, node_p, i_real);
        nusmv_assert((be_ptr)NULL != be_result);
        array_insert(be_ptr, past_array, 0, be_result);
                    
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  at depth 0: ");
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      else {
        /* Definitional translation */
        if(LastState_var == (node_ptr)NULL ||
           !SymbTable_is_symbol_input_var(st, f_node))
          {
            /* main_array[i][0] := p_i */
            be_result = BeEnc_name_to_timed(be_enc, f_node, i_real);
            nusmv_assert((be_ptr)NULL != be_result);
            array_insert(be_ptr, past_array, 0, be_result);
            
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[p]]_%s^0 := p_%s: ", i_str, i_str);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        else
          {
            /* main_array[i][0] := p_i & (!LastState_i | LoopExists)*/
            be_ptr be_LastState_i;
            nusmv_assert(LastState_var != (node_ptr)NULL);

            be_LastState_i =
              BeEnc_name_to_timed(be_enc, LastState_var, i_real);
            be_result = Be_And(be_mgr,
                               BeEnc_name_to_timed(be_enc, f_node, i_real),
                               Be_Or(be_mgr,
                                     Be_Not(be_mgr, be_LastState_i),
                                     be_LoopExists));
            nusmv_assert((be_ptr)NULL != be_result);
            array_insert(be_ptr, past_array, 0, be_result);
            
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[p]]_%s^0 := p_%s & (!LastState_%s | LoopExists): ", i_str, i_str, i_str);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
      }
      break;
    }

    case TRUEEXP: {
        be_ptr be_result = (be_ptr)NULL;
            
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          sbmc_print_node(nusmv_stderr, " ", f_node, "\n");
            
        nusmv_assert(sbmc_node_info_get_past_depth(f_info) == 0);

        if (sbmc_node_info_get_trans_vars(f_info) != (array_t *)NULL) {
          /* main_array[i][0] = [[TRUE]]_i */
          node_ptr node_aux;

          node_aux = array_fetch(node_ptr, sbmc_node_info_get_trans_vars(f_info), 0);
          nusmv_assert((node_ptr)NULL != node_aux);
          be_result = BeEnc_name_to_timed(be_enc, node_aux, i_real);
          nusmv_assert((be_ptr)NULL != be_result);
          array_insert(be_ptr, past_array, 0, be_result);
                
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  at depth %u: ", d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
        else {
          /* Definitional translation */
          /* main_array[i][0] := TRUE */
          be_result = Be_Truth(be_mgr);
          array_insert(be_ptr, past_array, 0, be_result);
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  [[TRUE]]_%s^0 := TRUE: ", i_str);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
        break;
      }

    case FALSEEXP:
      {
        be_ptr be_result = 0;
            
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          sbmc_print_node(nusmv_stderr, " ", f_node, "\n");
            
        nusmv_assert(sbmc_node_info_get_past_depth(f_info) == 0);

        if (sbmc_node_info_get_trans_vars(f_info) != (array_t *)NULL) {
          /* Formula variable translation */
          /* main_array[i][0] := [[FALSE]]_i */
          node_ptr node_aux;

          node_aux = array_fetch(node_ptr, sbmc_node_info_get_trans_vars(f_info), 0);
          nusmv_assert((node_ptr)NULL != node_aux);
          be_result = BeEnc_name_to_timed(be_enc, node_aux, i_real);
          nusmv_assert((be_ptr)NULL != be_result);
          array_insert(be_ptr, past_array, 0, be_result);
                    
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  at depth 0: ");
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
        else {
          /* Definitional translation */
          /* main_array[i][0] := FALSE */
          be_result = Be_Falsity(be_mgr);
          array_insert(be_ptr, past_array, 0, be_result);
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  [[FALSE]]_%s^0 := FALSE: ", i_str);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
        break;
      }


    case XOR:
    case XNOR:
    case IMPLIES:
    case IFF: {
      internal_error("%s:%d: Formula not in NNF\n", __FILE__, __LINE__);
      break;
    }
          
    case OR: {
      unsigned int d;

      if (!sbmc_set_is_in(visit_cache, lsf_node)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf_node, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf_node)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf_node, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;
            
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, " ", f_node, "\n");
            
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf_node);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info), i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);
            
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf_node);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(array_t *, sbmc_node_info_get_trans_bes(rsf_info), i_real);
      nusmv_assert((array_t *)NULL != rsf_past_array);
            
      if (sbmc_node_info_get_trans_vars(f_info) != (array_t *)NULL) {
        array_t * tvars = sbmc_node_info_get_trans_vars(f_info);
          /* Formula variable translation */
        for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++) {
          /* main_array[i][d] = [[f | g]]_i^d */
          node_ptr node_aux;
          be_ptr   be_result;

          node_aux = array_fetch(node_ptr, tvars, d);
          nusmv_assert((node_ptr)NULL != node_aux);
          be_result = BeEnc_name_to_timed(be_enc, node_aux, i_real);
          nusmv_assert((be_ptr)NULL != be_result);
          array_insert(be_ptr, past_array, d, be_result);
                    
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  at depth %u: ", d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      else {
        for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++) {
            /* Definitional translation */
            /* [[f | g]]_i^d := [[f]]_i^d | [[g]]_i^d */
            be_ptr be_lsf, be_rsf, be_result;
            const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
            const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

            be_lsf = array_fetch(be_ptr, lsf_past_array, d_lsf);
            nusmv_assert((be_ptr)NULL != be_lsf);
            be_rsf = array_fetch(be_ptr, rsf_past_array, d_rsf);
            nusmv_assert((be_ptr)NULL != be_rsf);
            be_result = Be_Or(be_mgr, be_lsf, be_rsf);
            nusmv_assert(array_fetch(be_ptr, past_array, d) == (be_ptr)NULL);
            array_insert(be_ptr, past_array, d, be_result);

            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr,
                      "  [[f | g]]_%s^%d := [[f]]_%s^%d | [[g]]_%s^%d: ",
                      i_str, d, i_str, d_lsf, i_str, d_rsf);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");        
            }
          }
      }
      break;
    }

    case AND: {
      unsigned int d;

      if (!sbmc_set_is_in(visit_cache, lsf_node)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf_node, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf_node)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf_node, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;
            
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, " ", f_node, "\n");
            
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf_node);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info), i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);
            
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf_node);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(array_t *, sbmc_node_info_get_trans_bes(rsf_info), i_real);
      nusmv_assert((array_t *)NULL != rsf_past_array);
            
      if (sbmc_node_info_get_trans_vars(f_info) != (array_t *)NULL) {
        array_t * tvars = sbmc_node_info_get_trans_vars(f_info);
        /* Formula variable translation */
        for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++) {
          /* main_array[i][d] = [[f & g]]_i^d */
          node_ptr node_aux;
          be_ptr   be_result;

          node_aux = array_fetch(node_ptr, tvars, d);
          nusmv_assert((node_ptr)NULL != node_aux);
          be_result = BeEnc_name_to_timed(be_enc, node_aux, i_real);
          nusmv_assert((be_ptr)NULL != be_result);
          array_insert(be_ptr, past_array, d, be_result);
          
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  at depth %u: ", d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      else {
        for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++) {
          /* Definitional translation */
          /* [[f | g]]_i^d := [[f]]_i^d & [[g]]_i^d */
          be_ptr be_lsf, be_rsf, be_result;
          const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
          const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

          be_lsf = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_lsf);
          be_rsf = array_fetch(be_ptr, rsf_past_array, d_rsf);
          nusmv_assert((be_ptr)NULL != be_rsf);
          be_result = Be_And(be_mgr, be_lsf, be_rsf);
          nusmv_assert(array_fetch(be_ptr, past_array, d) == (be_ptr)NULL);
          array_insert(be_ptr, past_array, d, be_result);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  [[f & g]]_%s^%d := [[f]]_%s^%d & [[g]]_%s^%d: ",
                    i_str, d, i_str, d_lsf, i_str, d_rsf);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      break;
    }

    case NOT: {
      unsigned int d;

      if (!sbmc_set_is_in(visit_cache, lsf_node)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf_node, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      nusmv_assert(sbmc_node_info_get_past_depth(f_info) == 0);

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf_node);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t*, sbmc_node_info_get_trans_bes(lsf_info), i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);
            
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, " ", f_node, "\n");

      if (sbmc_node_info_get_trans_vars(f_info) != (array_t *)NULL) {
        /* Formula variable translation */
        array_t * tvars = sbmc_node_info_get_trans_vars(f_info);

        for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++) {
          /* main_array[i][d] = [[!f]]_i^d */
          node_ptr node_aux;
          be_ptr   be_result;

          node_aux = array_fetch(node_ptr, tvars, d);
          nusmv_assert((node_ptr)NULL != node_aux);
          be_result = BeEnc_name_to_timed(be_enc, node_aux, i_real);
          nusmv_assert((be_ptr)NULL != be_result);
          array_insert(be_ptr, past_array, d, be_result);
                    
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  at depth %u: ", d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      else {
        /* Definitional translation */
        for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++) {
          if(LastState_var == (node_ptr)NULL ||
             !SymbTable_is_symbol_input_var(st, lsf_node))
            {
              /* [[!f]]_i^d := ![[f]]_i^d */
              be_ptr be_lsf, be_result;
              
              be_lsf = array_fetch(be_ptr, lsf_past_array, d);
              nusmv_assert((be_ptr)NULL != be_lsf);
              be_result = Be_Not(be_mgr, be_lsf);
              nusmv_assert(array_fetch(be_ptr, past_array, d) == (be_ptr)NULL);
              array_insert(be_ptr, past_array, d, be_result);

              if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                fprintf(nusmv_stderr, "  [!f]]_%s^%d := ![[f]]_%s^%d: ",
                        i_str, d, i_str, d);
                Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                fprintf(nusmv_stderr, "\n");
              }
            }
          else
            {
              /* [[!f]]_i^d := ![[f]]_i^d & (!LastState_i | LoopExists)*/
              be_ptr be_LastState_i, be_lsf, be_result;
              nusmv_assert(LastState_var != (node_ptr)NULL);

              be_LastState_i =
                BeEnc_name_to_timed(be_enc, LastState_var, i_real);
              be_lsf = array_fetch(be_ptr, lsf_past_array, d);
              nusmv_assert((be_ptr)NULL != be_lsf);
              be_result = Be_And(be_mgr,
                                 Be_Not(be_mgr, be_lsf),
                                 Be_Or(be_mgr,
                                       Be_Not(be_mgr, be_LastState_i),
                                       be_LoopExists));
              nusmv_assert(array_fetch(be_ptr, past_array, d) == (be_ptr)NULL);
              array_insert(be_ptr, past_array, d, be_result);

              if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                fprintf(nusmv_stderr,
                        "  [!f]]_%s^%d := "
                        "![[f]]_%s^%d & (!LastState_%s | LoopExists): ",
                        i_str, d, i_str, d, i_str);
                Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                fprintf(nusmv_stderr, "\n");
              }
            }
        }
      }
      break;
    }

    case UNTIL:
    case RELEASES:
    case TRIGGERED:
    case SINCE: {
        unsigned int d;
        if (!sbmc_set_is_in(visit_cache, lsf_node)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf_node, LS_NH);
          has_unprocessed_children = 1;
        }
        if (!sbmc_set_is_in(visit_cache, rsf_node)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)rsf_node, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          sbmc_print_node(nusmv_stderr, " ", f_node, "\n");

        array_t * tvars = sbmc_node_info_get_trans_vars(f_info);

        nusmv_assert((array_t *)NULL != tvars);

        for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++) {
          /* main_array[i][d] = [[f OP g]]_i^d */
          node_ptr node_aux;
          be_ptr   be_result;

          node_aux = array_fetch(node_ptr, tvars, d);
          nusmv_assert((node_ptr)NULL != node_aux);
          be_result = BeEnc_name_to_timed(be_enc, node_aux, i_real);
          nusmv_assert((be_ptr)NULL != be_result);
          array_insert(be_ptr, past_array, d, be_result);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  at depth %u: ", d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
        break;
      }

    case OP_FUTURE:
    case OP_GLOBAL:
    case OP_NEXT:
    case OP_HISTORICAL:
    case OP_ONCE:
    case OP_PREC:
    case OP_NOTPRECNOT: {
      unsigned int d;

      if (!sbmc_set_is_in(visit_cache, lsf_node)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf_node, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, " ", f_node, "\n");

      array_t * tvars = sbmc_node_info_get_trans_vars(f_info);

      nusmv_assert((array_t *)NULL != tvars);

      for (d = 0; d <= sbmc_node_info_get_past_depth(f_info); d++) {
        /* main_array[i][d] = [[OP f]]_i^d */
        node_ptr node_aux;
        be_ptr   be_result;

        node_aux = array_fetch(node_ptr, tvars, d);
        nusmv_assert((node_ptr)NULL != node_aux);
        be_result = BeEnc_name_to_timed(be_enc, node_aux, i_real);
        nusmv_assert((be_ptr)NULL != be_result);
        array_insert(be_ptr, past_array, d, be_result);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  at depth %u: ", d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      break;
    }
          
    default:
      print_node(stderr, f_node);
      internal_error("%s:%d: Something not implemented", __FILE__, __LINE__);
      break;
    }
    if (has_unprocessed_children)
      continue;
      
    /* Remove node from unprocessed stack */
    if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&f_node) != LS_OK)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Mark visited */
    sbmc_set_insert(visit_cache, f_node);
  }

  lsDestroy(unprocessed_nodes, NULL);
  sbmc_set_destroy(visit_cache); 
  visit_cache = (hash_ptr)NULL;
  FREE(i_str); 
  i_str = (char *)NULL;
}

/**Function********************************************************************

  Synopsis           [Build InLoop_i]

  Description        [Build InLoop_i stuff
  Define InLoop_i = (InLoop_{i-1} || l_i)<br>
  Returns the BE constraint InLoop_{i-1} => !l_i  (or 0 when i=0)]

  SideEffects        [None]

******************************************************************************/
be_ptr sbmc_build_InLoop_i(const BeEnc_ptr be_enc,
                               const state_vars_struct * state_vars,
                               array_t *InLoop_array,
                               const unsigned int i_model)
{
  Be_Manager_ptr be_mgr;
  be_ptr be_l_i = (be_ptr)NULL;
  be_ptr be_InLoop_i = (be_ptr)NULL;
  be_ptr be_InLoop_i_minus_1 = (be_ptr)NULL;
  be_ptr be_constraint = (be_ptr)NULL;
  
  nusmv_assert((BeEnc_ptr)NULL != be_enc);
  nusmv_assert((state_vars_struct *)NULL != state_vars);
  nusmv_assert((array_t *)NULL != InLoop_array);

  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert((Be_Manager_ptr)NULL != be_mgr);

  if (array_n(InLoop_array) <= i_model)
    array_insert(be_ptr, InLoop_array, i_model, 0);
    
  if (0 == i_model) {
    /* Build InLoop_0 := FALSE */
    be_InLoop_i = array_fetch(be_ptr, InLoop_array, i_model);
    if ((be_ptr)NULL == be_InLoop_i) {
      be_InLoop_i = Be_Falsity(be_mgr);
      array_insert(be_ptr, InLoop_array, i_model, be_InLoop_i);
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
        fprintf(nusmv_stderr, "Constructed InLoop_%d := FALSE", i_model);
      }
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, ": ");
        Be_DumpSexpr(be_mgr, be_InLoop_i, nusmv_stderr);
      }
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 2))
        fprintf(nusmv_stderr, "\n");
    }
    return 0;
  }

  /* Get previous InLoop */
  be_InLoop_i_minus_1 = array_fetch(be_ptr, InLoop_array, i_model-1);
  nusmv_assert((be_ptr)NULL != be_InLoop_i_minus_1);
    
  /* Get l_i */
  be_l_i = BeEnc_name_to_timed(be_enc,
                               sbmc_state_vars_get_l_var(state_vars),
                               sbmc_real_k(i_model));
  
  /* Get the constraint InLoop_{i-1} => !l_i */
  be_constraint = Be_Implies(be_mgr,
                             be_InLoop_i_minus_1,
                             Be_Not(be_mgr, be_l_i));
  
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2))
    fprintf(nusmv_stderr, "Created the constraint (InLoop_%d => !l_%d)",
            i_model-1, i_model);
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
    fprintf(nusmv_stderr, ": ");
    Be_DumpSexpr(be_mgr, be_constraint, nusmv_stderr);
  }
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2))
    fprintf(nusmv_stderr, "\n");

  /* Build InLoop_i := (InLoop_{i-1} || l_i) */
  be_InLoop_i = array_fetch(be_ptr, InLoop_array, i_model);
  if ((be_ptr)NULL == be_InLoop_i) {
    be_InLoop_i = Be_Or(be_mgr, be_InLoop_i_minus_1, be_l_i);
    array_insert(be_ptr, InLoop_array, i_model, be_InLoop_i);
    
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 2))
      fprintf(nusmv_stderr, "Constructed InLoop_%d := InLoop_%d | l_%d",
              i_model, i_model-1, i_model);
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
      fprintf(nusmv_stderr, ": ");
      Be_DumpSexpr(be_mgr, be_InLoop_i, nusmv_stderr);
    }
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 2))
      fprintf(nusmv_stderr, "\n");
  }

  return be_constraint;
}

/**Function********************************************************************

  Synopsis           [Build SimplePath_{i,k} for each 0<=i<k]

  Description        [Build SimplePath_{i,k} for each 0<=i<k]

  SideEffects        [None]

******************************************************************************/
lsList sbmc_SimplePaths(const BeEnc_ptr be_enc,
                            const state_vars_struct *state_vars,
                            array_t *InLoop_array,
                            const unsigned int k)
{
  Be_Manager_ptr be_mgr;
  unsigned int j;
  lsList created_constraints = lsCreate();

  nusmv_assert((BeEnc_ptr)NULL != be_enc);
  nusmv_assert((array_t *)NULL != InLoop_array);
  
  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert((Be_Manager_ptr)NULL != be_mgr);

  for (j = 0; j < k; j++) {
    be_ptr be_equal_system_vars;
    be_ptr be_equal_pd0_translation_vars;
    be_ptr be_equal_pdx_translation_vars;
    be_ptr be_equal_aux_translation_vars;
    be_ptr be_InLoop_k, be_InLoop_j;
    be_ptr be_equal_InLoops;
    be_ptr be_constraint;
    be_ptr be_c2;
      
    /* the collected variables should not take frozen vars since they
       implicitly always keep their valus from state to state */
    be_equal_system_vars =
      sbmc_equal_vectors_formula(be_enc,
                                 sbmc_state_vars_get_simple_path_system_vars(state_vars),
                                 sbmc_real_k(j), sbmc_real_k(k));

    be_equal_pd0_translation_vars =
      sbmc_equal_vectors_formula(be_enc,
                                 sbmc_state_vars_get_translation_vars_pd0(state_vars),
                                 sbmc_real_k(j), sbmc_real_k(k));
      
    be_equal_pdx_translation_vars =
      sbmc_equal_vectors_formula(be_enc,
                                 sbmc_state_vars_get_translation_vars_pdx(state_vars),
                                 sbmc_real_k(j), sbmc_real_k(k));
      
    be_equal_aux_translation_vars =
      sbmc_equal_vectors_formula(be_enc,
                                 sbmc_state_vars_get_translation_vars_aux(state_vars),
                                 sbmc_real_k(j), sbmc_real_k(k));
      
    be_InLoop_k = array_fetch(be_ptr, InLoop_array, k);
    nusmv_assert((be_ptr)NULL != be_InLoop_k);
      
    be_InLoop_j = array_fetch(be_ptr, InLoop_array, j);
    nusmv_assert((be_ptr)NULL != be_InLoop_j);
      
    be_equal_InLoops = Be_Iff(be_mgr, be_InLoop_j, be_InLoop_k);
      
    /* (s_j != s_k) */
    be_constraint = Be_Not(be_mgr, be_equal_system_vars);

    /* (s_j != s_k) | (InLoop_j != InLoop_k) */
    be_constraint = Be_Or(be_mgr,
                          be_constraint,
                          Be_Not(be_mgr, be_equal_InLoops));

    /* (s_j != s_k) | (InLoop_j != InLoop_k) | ([[f]]_j^0 != [[f]]_k^0) */
    be_constraint = Be_Or(be_mgr,
                          be_constraint,
                          Be_Not(be_mgr,
                                 be_equal_pd0_translation_vars));

    be_c2 = Be_And(be_mgr,
                   Be_And(be_mgr, be_InLoop_j, be_InLoop_k),
                   Be_Or(be_mgr,
                         Be_Not(be_mgr,
                                be_equal_pdx_translation_vars),
                         Be_Not(be_mgr,
                                be_equal_aux_translation_vars)));

    be_constraint = Be_Or(be_mgr, be_constraint, be_c2);
      
    lsNewEnd(created_constraints, be_constraint, LS_NH);
      
    if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
      fprintf(nusmv_stderr, "Created SimplePath_{%d,%d}", j, k);
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, ": ");
        Be_DumpSexpr(be_mgr, be_constraint, nusmv_stderr);
      }
      fprintf(nusmv_stderr, "\n"); 
    }
  }
  return created_constraints;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



