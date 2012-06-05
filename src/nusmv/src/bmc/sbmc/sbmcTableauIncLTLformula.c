/**CFile***********************************************************************

  FileName    [sbmcTableauIncLTLformula.c]

  PackageName [bmc.sbmc]

  Synopsis    [Bmc.Tableau module]

  Description [This module contains all the operations related to the
               construction of SBMC incremental tableaux for LTL formulas]

  SeeAlso     []

  Author      [Tommi Junttila, Marco Roveri]

  Copyright   [This file is part of the ``sbmc'' package of NuSMV version 2.
  Copyright (C) by 2006 Tommi Junttila

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

  For more information of NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "sbmcStructs.h"
#include "sbmcUtils.h"
#include "sbmcTableauInc.h"

#include "bmc/bmcInt.h"

#include "node/node.h"
#include "utils/assoc.h"
#include "utils/array.h"
#include "utils/list.h"

#include "be/be.h"
#include "enc/enc.h"
#include "enc/be/BeEnc.h"
#include "opt/opt.h"
#include "parser/symbols.h"


static char rcsid[] UTIL_UNUSED = "$Id: sbmcTableauIncLTLformula.c,v 1.1.2.4.4.6 2009-09-17 11:49:47 nusmv Exp $";

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
extern FILE* nusmv_stderr;

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

/*
 */
/**Function********************************************************************

  Synopsis           [Creates the BASE constraints.]

  Description        [Create the BASE constraints.<br>
  Return a list of be_ptr for the created constraints.<br>
  Create the following constraints:<br>
  <ul>
    <li> !LoopExists => ([[f]]_L^d == FALSE) </li> 
    <li> LoopExists => ([[Ff]]_E^pd(Gf) => <<Ff>>_E) </li> 
    <li> LoopExists => ([[Gf]]_E^pd(Gf) <= <<Gf>>_E) </li> 
    <li> LoopExists => ([[fUg]]_E^pd(Gf) => <<Fg>>_E) </li> 
    <li> LoopExists => ([[fRg]]_E^pd(Gf) <= <<Gg>>_E) </li> 
  </ul>
  If do_optimization is true, then create the following constraints:
  <ul>
    <li> [[p]]_E^d <=> p_E </li> 
    <li> [[TRUE]]_E^0 <=> TRUE </li> 
    <li> [[FALSE]]_E^0 <=> FALSE </li> 
    <li> [[f | g]]_E^d <=> [[f]]_E^d | [[g]]_E^d </li> 
    <li> [[f & g]]_E^d <=> [[f]]_E^d & [[g]]_E^d </li> 
    <li> [[!f]]_E^d <=> ![[f]]_E^d </li> 
    <li> [[Ff]]_E^d <=> [[f]]_E^d | [[Ff]]_L^min(d+1,pd(f)) </li> 
    <li> [[Ff]]_E^d+1 => [[Ff]]_E^d </li> 
    <li> <<Ff>>_E => [[Ff]]_E^pd(Ff) </li> 
    <li> [[Gf]]_E^d <=> [[f]]_E^d & [[Gf]]_L^min(d+1,pd(f)) </li> 
    <li> [[Gf]]_E^d => [[Gf]]_E^d+1 </li> 
    <li> [[Gf]]_E^pd(Gf) => <<Gf>>_E </li> 
    <li> [[fUg]]_E^d <=> [[g]]_E^d | ([[f]]_E^d  & [[fUg]]_L^min(d+1,pd(fUg))) </li> 
    <li> [[fRg]]_E^d <=> [[g]]_E^d & ([[f]]_E^d  | [[fRg]]_L^min(d+1,pd(fUg))) </li> 
    <li> [[Xf]]_E^d <=> [[f]]_L^min(d+1,pd(f)) </li> 
    <li> [[Hf]]_E^d+1 => [[Hf]]_E^d </li> 
    <li> [[Of]]_E^d => [[Of]]_E^d+1 </li> 
  </ul>
  ]

  SideEffects        [None]

******************************************************************************/
lsList sbmc_unroll_base(const BeEnc_ptr be_enc,
                        const node_ptr ltlspec,
                        const hash_ptr info_map,
                        const be_ptr be_LoopExists,
                        const int do_optimization)
{
  hash_ptr visit_cache = (hash_ptr)NULL;
  lsList   unprocessed_nodes = (lsList)NULL;
  lsList   created_constraints = (lsList)NULL;
  Be_Manager_ptr be_mgr;
  SymbTable_ptr st = (SymbTable_ptr)NULL;

  nusmv_assert(info_map != (hash_ptr)NULL);

  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert((Be_Manager_ptr)NULL != be_mgr);

  /* Get symbol table */
  st = BaseEnc_get_symb_table(BASE_ENC(be_enc));
  nusmv_assert((SymbTable_ptr)NULL != st);
  
  /* Create list for the constraints */
  created_constraints = lsCreate();

  /* Debug output */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "Creating the base constraints\n");
    fflush(nusmv_stderr);
  }
  
  visit_cache = sbmc_set_create();
  unprocessed_nodes = lsCreate();
  lsNewBegin(unprocessed_nodes, (lsGeneric)ltlspec, LS_NH);

  while(lsLength(unprocessed_nodes) > 0) {
    node_ptr node, lsf, rsf;
    sbmc_node_info * info, *lsf_info, *rsf_info;
    array_t *E_past_array = (array_t *)NULL, *L_past_array = (array_t *)NULL;
    int has_unprocessed_children;

    /* Get node */
    if ((lsFirstItem(unprocessed_nodes, (lsGeneric*)&node, LS_NH) != LS_OK) ||
        ((node_ptr)NULL == node))
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Get info */
    info = sbmc_node_info_assoc_find(info_map, node);
    if ((sbmc_node_info * )NULL == info)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);

    /* Get past_array */
    /* sbmc_init_state_vector should have allocated trans_bes[i] before */
    if (sbmc_node_info_get_trans_bes(info)) {
      nusmv_assert(array_n(sbmc_node_info_get_trans_bes(info)) >= 2);
      L_past_array = array_fetch(array_t *, sbmc_node_info_get_trans_bes(info), sbmc_L_state());
      nusmv_assert((array_t *)NULL != L_past_array);
      E_past_array = array_fetch(array_t *, sbmc_node_info_get_trans_bes(info), sbmc_E_state());
      nusmv_assert((array_t *)NULL != E_past_array);
    }

    /* Already build? */
    if (sbmc_set_is_in(visit_cache, node)) {
      /* Remove node from unprocessed stack */
      if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
        internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      continue;
    }


    /* Traverse children and build info */
    lsf = car(node);
    rsf = cdr(node);      
    has_unprocessed_children = 0;
    switch(node_get_type(node))
      {

      case ATOM:
      case BIT:
      case DOT:
      case ARRAY: {
        if (do_optimization && !SymbTable_is_symbol_input_var(st, node)) {
          /* Optimization:
             If p is not an input variable, add [[p]]_E^d <=> p_E */
          be_ptr be_result = 0;
          nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);
          if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
            /* Definitional translation.
               Already done by sbmc_init_state_vector. */
            nusmv_assert(array_fetch(be_ptr, E_past_array, 0) != (be_ptr)NULL);
          }
          else {
            /* Formula variable translation: Build [[p]]_E^0 <=> p_E */
            be_ptr be_p_E_0, be_realp_E;

            be_p_E_0 = array_fetch(be_ptr, E_past_array, 0);
            nusmv_assert((be_ptr)NULL != be_p_E_0);
            be_realp_E = BeEnc_name_to_timed(be_enc, node, sbmc_E_state());
            nusmv_assert((be_ptr)NULL != be_realp_E);
                    
            be_result = Be_Iff(be_mgr, be_p_E_0, be_realp_E);
            nusmv_assert((be_ptr)NULL != be_result);
                    
            /* Save the created constraint */
            lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);
                    
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[p]]_E^0 <=> p_E: ");
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        }
        break;
      }

      case TRUEEXP: {
        if (do_optimization) {
          be_ptr be_result;
                
          nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);
                
          if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
            /* Definitional translation already done by state vector init */
            nusmv_assert(array_fetch(be_ptr, E_past_array, 0) != (be_ptr)NULL);
          }
          else {
            /* Formula variable translation */
            /* Build [[TRUE]]_E^0 <=> TRUE */
            be_ptr be_TRUE_E_0 = array_fetch(be_ptr, E_past_array, 0);

            nusmv_assert((be_ptr)NULL != be_TRUE_E_0);
                    
            be_result = Be_Iff(be_mgr, be_TRUE_E_0, Be_Truth(be_mgr));
            nusmv_assert((be_ptr)NULL != be_result);
                    
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                    
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[TRUE]]_E^0 <=> TRUE: ");
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        }
        break;
      }

      case FALSEEXP: {
        if (do_optimization) {
          be_ptr be_result;
                
          nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);
                
          if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
            /* Definitional translation already done by state vector init */
            nusmv_assert(array_fetch(be_ptr, E_past_array, 0) != 0);
          }
          else {
            /* Formula variable translation: build [[FALSE]]_E^0 <=> FALSE */
            be_ptr be_FALSE_E_0 = array_fetch(be_ptr, E_past_array, 0);

            nusmv_assert((be_ptr)NULL != be_FALSE_E_0);
                    
            be_result = Be_Iff(be_mgr, be_FALSE_E_0, Be_Falsity(be_mgr));
            nusmv_assert((be_ptr)NULL != be_result);
                    
            /* Save the created constraint */
            lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);
                    
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[FALSE]]_E^0 <=> FALSE: ");
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
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
          array_t *past_array_f_E, *past_array_g_E;

          if (!sbmc_set_is_in(visit_cache, lsf)) {
            lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
            has_unprocessed_children = 1;
          }
          if (!sbmc_set_is_in(visit_cache, rsf)) {
            lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
            has_unprocessed_children = 1;
          }
          if (has_unprocessed_children)
            break;

          lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
          nusmv_assert((sbmc_node_info *)NULL != lsf_info);
          past_array_f_E = array_fetch(array_t *,
                                       sbmc_node_info_get_trans_bes(lsf_info),
                                       sbmc_E_state());
          nusmv_assert((array_t *)NULL != past_array_f_E);

          rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
          nusmv_assert((sbmc_node_info *)NULL != rsf_info);
          past_array_g_E = array_fetch(array_t *,
                                       sbmc_node_info_get_trans_bes(rsf_info),
                                       sbmc_E_state());
          nusmv_assert((array_t *)NULL != past_array_g_E);

          if (do_optimization) {
            /* Add [[f | g]]_E^d <=> [[f]]_E^d | [[g]]_E^d */
            if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
              /* Definitional translation already done by state vector init */
              for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
                nusmv_assert(array_fetch(be_ptr, E_past_array, d) != (be_ptr)NULL);
            }
            else {
              for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
                be_ptr be_fORg_E_d, be_f_E_d, be_g_E_d, be_result;
                const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
                const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));
                      
                be_fORg_E_d = array_fetch(be_ptr, E_past_array, d);
                nusmv_assert((be_ptr)NULL != be_fORg_E_d);
                be_f_E_d = array_fetch(be_ptr, past_array_f_E, d_lsf);
                nusmv_assert((be_ptr)NULL != be_f_E_d);
                be_g_E_d = array_fetch(be_ptr, past_array_g_E, d_rsf);
                nusmv_assert((be_ptr)NULL != be_g_E_d);
                be_result = Be_Iff(be_mgr, be_fORg_E_d,
                                   Be_Or(be_mgr, be_f_E_d, be_g_E_d));
                nusmv_assert((be_ptr)NULL != be_result);
                      
                /* Save the created constraint */
                lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                      
                if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                  fprintf(nusmv_stderr,
                          "  [[f | g]]_E^%d <=> [[f]]_E^%d | [[g]]_E^%d: ",
                          d, d_lsf, d_rsf);
                  Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                  fprintf(nusmv_stderr, "\n");
                }
              }
            }
          }
          break;
        }

      case AND: {
        unsigned int d;
        array_t *past_array_f_E, *past_array_g_E;

        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (!sbmc_set_is_in(visit_cache, rsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
        nusmv_assert((sbmc_node_info *)NULL != lsf_info);
        past_array_f_E = array_fetch(array_t *,
                                     sbmc_node_info_get_trans_bes(lsf_info),
                                     sbmc_E_state());
        nusmv_assert((array_t *)NULL != past_array_f_E);
            
        rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
        nusmv_assert((sbmc_node_info *)NULL != rsf_info);
        past_array_g_E = array_fetch(array_t *,
                                     sbmc_node_info_get_trans_bes(rsf_info),
                                     sbmc_E_state());
        nusmv_assert((array_t *)NULL != past_array_g_E);
            
        if (do_optimization) {
          /* Add [[f & g]]_E^d <=> [[f]]_E^d & [[g]]_E^d */
          if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
            /* Definitional translation already done by
               state vector init */
            for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
              nusmv_assert(array_fetch(be_ptr, E_past_array, d) != (be_ptr)NULL);
          }
          else {
            for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
              be_ptr be_fANDg_E_d, be_f_E_d, be_g_E_d, be_result;
              const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
              const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));
                      
              be_fANDg_E_d = array_fetch(be_ptr, E_past_array, d);
              nusmv_assert((be_ptr)NULL != be_fANDg_E_d);
              be_f_E_d = array_fetch(be_ptr, past_array_f_E, d_lsf);
              nusmv_assert((be_ptr)NULL != be_f_E_d);
              be_g_E_d = array_fetch(be_ptr, past_array_g_E, d_rsf);
              nusmv_assert((be_ptr)NULL != be_g_E_d);
              be_result = Be_Iff(be_mgr, be_fANDg_E_d,
                                 Be_And(be_mgr, be_f_E_d, be_g_E_d));
              nusmv_assert((be_ptr)NULL != be_result);
                      
              /* Save the created constraint */
              lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                      
              if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                fprintf(nusmv_stderr,
                        "  [[f & g]]_E^%d <=> [[f]]_E^%d & [[g]]_E^%d: ",
                        d, d_lsf, d_rsf);
                Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                fprintf(nusmv_stderr, "\n");
              }
            }
          }
        }
        break;
      }

      case NOT: {
        unsigned int d;
        array_t * past_array_f_E;

        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);

        lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
        nusmv_assert((sbmc_node_info *)NULL != lsf_info);
        past_array_f_E = array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                                     sbmc_E_state());
        nusmv_assert((array_t *)NULL != past_array_f_E);
            
        if (do_optimization && !SymbTable_is_symbol_input_var(st, lsf)) {
          /* Optimization:
             if f is not an input variable, add [[!f]]_E^d <=> ![[f]]_E^d */
          if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
            /* Definitional translation already done by state vector init */
            for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
              nusmv_assert(array_fetch(be_ptr, E_past_array, d) != (be_ptr)NULL);
          }
          else {
            /* Formula variable translation */
            for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
              be_ptr be_notf_E_d, be_f_E_d, be_result;

              be_notf_E_d = array_fetch(be_ptr, E_past_array, d);
              nusmv_assert((be_ptr)NULL != be_notf_E_d);
              be_f_E_d = array_fetch(be_ptr, past_array_f_E, d);
              nusmv_assert((be_ptr)NULL != be_f_E_d);
              be_result = Be_Iff(be_mgr, be_notf_E_d,
                                 Be_Not(be_mgr, be_f_E_d));
              nusmv_assert((be_ptr)NULL != be_result);
                      
              /* Save the created constraint */
              lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                      
              if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                fprintf(nusmv_stderr, "  [!f]]_E^%d <=> ![[f]]_E^%d: ",
                        d, d);
                Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                fprintf(nusmv_stderr, "\n");
              }
            }
          }
        }

        break;
      }

      case OP_FUTURE: {
        unsigned int  d;
        array_t * lsf_E_past_array;

        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
        nusmv_assert((array_t *)NULL != L_past_array);
        nusmv_assert((array_t *)NULL != E_past_array);

        lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
        nusmv_assert((sbmc_node_info *)NULL != lsf_info);
        nusmv_assert(sbmc_node_info_get_past_depth(lsf_info) ==
                     sbmc_node_info_get_past_depth(info));
        lsf_E_past_array = array_fetch(array_t *,
                                       sbmc_node_info_get_trans_bes(lsf_info),
                                       sbmc_E_state());
        nusmv_assert((array_t *)NULL != lsf_E_past_array);
            
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          sbmc_print_node(nusmv_stderr, " ", node, "\n");

        {
          /* Add LoopExists => ([[Ff]]_E^pd(Gf) => <<Ff>>_E) */
          be_ptr be_Ff_E_pd, be_auxFf_E, be_result;
          be_Ff_E_pd = array_fetch(be_ptr, E_past_array,
                                   sbmc_node_info_get_past_depth(info));

          nusmv_assert((be_ptr)NULL != be_Ff_E_pd);
          nusmv_assert((node_ptr)NULL != sbmc_node_info_get_aux_F_node(lsf_info));
          be_auxFf_E = BeEnc_name_to_timed(be_enc,
                                           sbmc_node_info_get_aux_F_node(lsf_info),
                                           sbmc_E_state());
          nusmv_assert((be_ptr)NULL != be_auxFf_E);
          be_result = Be_Implies(be_mgr,
                                 be_LoopExists,
                                 Be_Implies(be_mgr,
                                            be_Ff_E_pd,
                                            be_auxFf_E));
              
          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
              
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr,
                    "  LoopExists => ([[Ff]]_E^%d => <<Ff>>_E): ",
                    sbmc_node_info_get_past_depth(info));
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }

        if (do_optimization) {
          /* Optimization: Add [[Ff]]_E^d <=> [[f]]_E^d | [[Ff]]_L^min(d+1,pd(f)) */
          for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
            be_ptr be_aux, be_lsf, be_aux_next, be_result;

            be_aux = array_fetch(be_ptr, E_past_array, d);
            nusmv_assert((be_ptr)NULL != be_aux);
            be_lsf = array_fetch(be_ptr, lsf_E_past_array, d);
            nusmv_assert((be_ptr)NULL != be_lsf);
            be_aux_next = array_fetch(be_ptr, L_past_array,
                                      min(d+1, sbmc_node_info_get_past_depth(info)));
            nusmv_assert((be_ptr)NULL != be_aux_next);
            be_result = Be_Iff(be_mgr, be_aux,
                               Be_Or(be_mgr, be_lsf, be_aux_next));

            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[Ff]]_E^%d <=> [[f]]_E^%d | [[Ff]]_L^min(%d,%d): ",
                      d, d, d+1, sbmc_node_info_get_past_depth(info));
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
          /* Optimization: Add [[Ff]]_E^d+1 => [[Ff]]_E^d */
          for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
            be_ptr be_Ff_E_d, be_Ff_E_dP1, be_result;

            be_Ff_E_d = array_fetch(be_ptr, E_past_array, d);
            nusmv_assert((be_ptr)NULL != be_Ff_E_d);
            be_Ff_E_dP1 = array_fetch(be_ptr, E_past_array, d+1);
            nusmv_assert((be_ptr)NULL != be_Ff_E_dP1);
            be_result = Be_Implies(be_mgr, be_Ff_E_dP1, be_Ff_E_d);

            /* Save the created constraint */
            lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);
                    
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, " [[Ff]]_E^%u => [[Ff]]_E^%u: ",
                      d+1, d);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
          {
            /* Optimization: Add <<Ff>>_E => [[Ff]]_E^pd(Ff) */
            be_ptr be_Ff_E_pd, be_auxFf_E, be_result;
            be_Ff_E_pd = array_fetch(be_ptr, E_past_array,
                                     sbmc_node_info_get_past_depth(info));
            nusmv_assert((be_ptr)NULL != be_Ff_E_pd);
            nusmv_assert(sbmc_node_info_get_aux_F_node(lsf_info));
            be_auxFf_E = BeEnc_name_to_timed(be_enc,
                                             sbmc_node_info_get_aux_F_node(lsf_info),
                                             sbmc_E_state());
            nusmv_assert((be_ptr)NULL != be_auxFf_E);
            be_result = Be_Implies(be_mgr, be_auxFf_E, be_Ff_E_pd);
                
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr,
                      "   <<Ff>>_E => [[Ff]]_E^%d: ",
                      sbmc_node_info_get_past_depth(info));
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }              
          }
        }
        break;
      }

      case OP_GLOBAL: {
          unsigned int  d;
          array_t *      lsf_E_past_array;

          if (!sbmc_set_is_in(visit_cache, lsf)) {
            lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
            has_unprocessed_children = 1;
          }
          if (has_unprocessed_children)
            break;

          nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
          nusmv_assert((array_t *)NULL != L_past_array);
          nusmv_assert((array_t *)NULL != E_past_array);

          lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
          nusmv_assert((sbmc_node_info *)NULL != lsf_info);
          nusmv_assert(sbmc_node_info_get_past_depth(lsf_info) == 
                       sbmc_node_info_get_past_depth(info));
          lsf_E_past_array = array_fetch(array_t *,
                                         sbmc_node_info_get_trans_bes(lsf_info),
                                         sbmc_E_state());
          nusmv_assert((array_t *)NULL != lsf_E_past_array);
            
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
            sbmc_print_node(nusmv_stderr, " ", node, "\n");

          {
            /* Add LoopExists => ([[Gf]]_E^pd(Gf) <= <<Gf>>_E) */
            be_ptr be_Gf_E_pd, be_auxGf_E, be_result;

            be_Gf_E_pd = array_fetch(be_ptr, E_past_array,
                                     sbmc_node_info_get_past_depth(info));
            nusmv_assert((be_ptr)NULL != be_Gf_E_pd);
            nusmv_assert((node_ptr)NULL != sbmc_node_info_get_aux_G_node(lsf_info));
            be_auxGf_E = BeEnc_name_to_timed(be_enc,
                                             sbmc_node_info_get_aux_G_node(lsf_info),
                                             sbmc_E_state());
            nusmv_assert((be_ptr)NULL != be_auxGf_E);
            be_result = Be_Implies(be_mgr,
                                   be_LoopExists,
                                   Be_Implies(be_mgr,
                                              be_auxGf_E,
                                              be_Gf_E_pd));
              
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr,
                      "  LoopExists => ([[Gf]]_E^%d <= <<Gf>>_E^%d): ",
                      sbmc_node_info_get_past_depth(info), sbmc_node_info_get_past_depth(info));
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }              
          }
            
          if (do_optimization) {
            /* Optimization: Add [[Gf]]_E^d <=> [[f]]_E^d & [[Gf]]_L^min(d+1,pd(f)) */
            for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
              be_ptr be_aux, be_lsf, be_aux_next, be_result;

              be_aux = array_fetch(be_ptr, E_past_array, d);
              nusmv_assert((be_ptr)NULL != be_aux);
              be_lsf = array_fetch(be_ptr, lsf_E_past_array, d);
              nusmv_assert((be_ptr)NULL != be_lsf);
              be_aux_next = array_fetch(be_ptr, L_past_array,
                                        min(d+1, sbmc_node_info_get_past_depth(info)));
              nusmv_assert((be_ptr)NULL != be_aux_next);
              be_result = Be_Iff(be_mgr, be_aux,
                                 Be_And(be_mgr, be_lsf, be_aux_next));
                    
              /* Save the created constraint */
              lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                    
              if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                fprintf(nusmv_stderr, "  [[Gf]]_E^%u <=> [[f]]_E^%u & [[Gf]]_L^min(%u+1,%u))",
                        d, d, d, sbmc_node_info_get_past_depth(info));
                Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                fprintf(nusmv_stderr, "\n");
              }
            }
            /* Optimization: Add [[Gf]]_E^d => [[Gf]]_E^d+1 */
            for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
              be_ptr be_Gf_E_d, be_Gf_E_dP1, be_result;

              be_Gf_E_d = array_fetch(be_ptr, E_past_array, d);
              nusmv_assert((be_ptr)NULL != be_Gf_E_d);
              be_Gf_E_dP1 = array_fetch(be_ptr, E_past_array, d+1);
              nusmv_assert((be_ptr)NULL != be_Gf_E_dP1);
              be_result = Be_Implies(be_mgr, be_Gf_E_d, be_Gf_E_dP1);
                    
              /* Save the created constraint */
              lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                    
              if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                fprintf(nusmv_stderr, " [[Gf]]_E^%u => [[Gf]]_E^%u: ",
                        d, d+1);
                Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                fprintf(nusmv_stderr, "\n");
              }
            }
            {
              /* Optimization: Add [[Gf]]_E^pd(Gf) => <<Gf>>_E */
              be_ptr be_Gf_E_pd, be_auxGf_E, be_result;

              be_Gf_E_pd = array_fetch(be_ptr,E_past_array,
                                       sbmc_node_info_get_past_depth(info));
              nusmv_assert((be_ptr)NULL != be_Gf_E_pd);
              nusmv_assert((node_ptr)NULL != sbmc_node_info_get_aux_G_node(lsf_info));
              be_auxGf_E = BeEnc_name_to_timed(be_enc,
                                               sbmc_node_info_get_aux_G_node(lsf_info),
                                               sbmc_E_state());
              nusmv_assert((be_ptr)NULL != be_auxGf_E);
              be_result = Be_Implies(be_mgr, be_Gf_E_pd, be_auxGf_E);
                
              /* Save the created constraint */
              lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                
              if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                fprintf(nusmv_stderr,
                        "  [[Gf]]_E^%d => <<Gf>>_E): ",
                        sbmc_node_info_get_past_depth(info));
                Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                fprintf(nusmv_stderr, "\n");
              }              
            }
          }
          break;
        }

      case UNTIL: {
        unsigned int  d;
        array_t *      past_array_f_E;
        array_t *      past_array_g_E;

        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (!sbmc_set_is_in(visit_cache, rsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
        nusmv_assert((array_t *)NULL != L_past_array);
        nusmv_assert((array_t *)NULL != E_past_array);

        lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
        nusmv_assert((sbmc_node_info *)NULL != lsf_info);
        past_array_f_E = array_fetch(array_t *,
                                     sbmc_node_info_get_trans_bes(lsf_info),
                                     sbmc_E_state());
        nusmv_assert((array_t *)NULL != past_array_f_E);
            
        rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
        nusmv_assert((sbmc_node_info *)NULL != rsf_info);
        past_array_g_E = array_fetch(array_t *,
                                     sbmc_node_info_get_trans_bes(rsf_info),
                                     sbmc_E_state());
        nusmv_assert((array_t *)NULL != past_array_g_E);


        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          sbmc_print_node(nusmv_stderr, " ", node, "\n");

        {
          /* Add LoopExists => ([[fUg]]_E^pd(Gf) => <<Fg>>_E) */
          be_ptr be_fUg_E_pd, be_auxFg_E, be_result;

          be_fUg_E_pd = array_fetch(be_ptr,E_past_array,
                                    sbmc_node_info_get_past_depth(info));
          nusmv_assert((be_ptr)NULL != be_fUg_E_pd);
          nusmv_assert((node_ptr)NULL != sbmc_node_info_get_aux_F_node(rsf_info));
          be_auxFg_E = BeEnc_name_to_timed(be_enc,
                                           sbmc_node_info_get_aux_F_node(rsf_info),
                                           sbmc_E_state());
          nusmv_assert((be_ptr)NULL != be_auxFg_E);
          be_result = Be_Implies(be_mgr,
                                 be_LoopExists,
                                 Be_Implies(be_mgr,
                                            be_fUg_E_pd,
                                            be_auxFg_E));
              
          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
              
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr,
                    "  LoopExists => ([[fUg]]_E^%u => <<Fg>>_E): ",
                    sbmc_node_info_get_past_depth(info));
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }

        if (do_optimization) {
          /* Optimization: [[fUg]]_E^d <=>
           * [[g]]_E^d | ([[f]]_E^d  & [[fUg]]_L^min(d+1,pd(fUg))) */
          for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
            be_ptr be_fUg_E_d, be_f_E_d, be_g_E_d, be_fUg_L_dP1, be_result;

            be_fUg_E_d = array_fetch(be_ptr, E_past_array, d);
            nusmv_assert((be_ptr)NULL != be_fUg_E_d);

            be_f_E_d = array_fetch(be_ptr, past_array_f_E,
                                   min(d, sbmc_node_info_get_past_depth(lsf_info)));
            nusmv_assert((be_ptr)NULL != be_f_E_d);

            be_g_E_d = array_fetch(be_ptr, past_array_g_E,
                                   min(d, sbmc_node_info_get_past_depth(rsf_info)));
            nusmv_assert((be_ptr)NULL != be_g_E_d);

            be_fUg_L_dP1 = array_fetch(be_ptr, L_past_array,
                                       min(d+1, sbmc_node_info_get_past_depth(info)));
            nusmv_assert((be_ptr)NULL != be_fUg_L_dP1);

            be_result = Be_Iff(be_mgr,
                               be_fUg_E_d,
                               Be_Or(be_mgr,
                                     be_g_E_d,
                                     Be_And(be_mgr,
                                            be_f_E_d,
                                            be_fUg_L_dP1)));
                    
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                    
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[fUg]]_E^%u <=> [[g]]_E^%u | ([[f]]_E^%u & [[fUg]]_L^min(%u,%u)): ",
                      d, min(d, sbmc_node_info_get_past_depth(rsf_info)),
                      min(d, sbmc_node_info_get_past_depth(lsf_info)),
                      d+1, sbmc_node_info_get_past_depth(info));
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        }
        break;
      }

      case RELEASES: {
        unsigned int  d;
        array_t *      past_array_f_E;
        array_t *      past_array_g_E;

        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (!sbmc_set_is_in(visit_cache, rsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
        nusmv_assert((array_t *)NULL != L_past_array);
        nusmv_assert((array_t *)NULL != E_past_array);

        lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
        nusmv_assert((sbmc_node_info *)NULL != lsf_info);
        past_array_f_E = array_fetch(array_t *,
                                     sbmc_node_info_get_trans_bes(lsf_info),
                                     sbmc_E_state());
        nusmv_assert((array_t *)NULL != past_array_f_E);
            
        rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
        nusmv_assert((sbmc_node_info *)NULL != rsf_info);
        past_array_g_E = array_fetch(array_t *,
                                     sbmc_node_info_get_trans_bes(rsf_info),
                                     sbmc_E_state());
        nusmv_assert((array_t *)NULL != past_array_g_E);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          sbmc_print_node(nusmv_stderr, " ", node, "\n");

        {
          /* Add LoopExists => ([[fRg]]_E^pd(Gf) <= <<Gg>>_E) */
          be_ptr be_fRg_E_pd, be_auxGg_E, be_result;

          be_fRg_E_pd = array_fetch(be_ptr, E_past_array,
                                    sbmc_node_info_get_past_depth(info));
          nusmv_assert((be_ptr)NULL != be_fRg_E_pd);
          nusmv_assert((node_ptr)NULL != sbmc_node_info_get_aux_G_node(rsf_info));
          be_auxGg_E = BeEnc_name_to_timed(be_enc,
                                           sbmc_node_info_get_aux_G_node(rsf_info),
                                           sbmc_E_state());
          nusmv_assert((be_ptr)NULL != be_auxGg_E);
          be_result = Be_Implies(be_mgr,
                                 be_LoopExists,
                                 Be_Implies(be_mgr,
                                            be_auxGg_E,
                                            be_fRg_E_pd));
              
          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
              
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr,
                    "  LoopExists => ([[fRg]]_E^%u <= <<Gg>>_E): ",
                    sbmc_node_info_get_past_depth(info));
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }


        if (do_optimization) {
          /* Optimization: [[fRg]]_E^d <=>
           * [[g]]_E^d & ([[f]]_E^d  | [[fRg]]_L^min(d+1,pd(fUg))) */
          for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
            be_ptr be_fRg_E_d, be_f_E_d, be_g_E_d, be_fRg_L_dP1, be_result;

            be_fRg_E_d = array_fetch(be_ptr, E_past_array, d);
            nusmv_assert((be_ptr)NULL != be_fRg_E_d);

            be_f_E_d = array_fetch(be_ptr, past_array_f_E,
                                   min(d, sbmc_node_info_get_past_depth(lsf_info)));
            nusmv_assert((be_ptr)NULL != be_f_E_d);

            be_g_E_d = array_fetch(be_ptr, past_array_g_E,
                                   min(d, sbmc_node_info_get_past_depth(rsf_info)));
            nusmv_assert((be_ptr)NULL != be_g_E_d);

            be_fRg_L_dP1 = array_fetch(be_ptr, L_past_array,
                                       min(d+1, sbmc_node_info_get_past_depth(info)));
            nusmv_assert((be_ptr)NULL != be_fRg_L_dP1);

            be_result = Be_Iff(be_mgr,
                               be_fRg_E_d,
                               Be_And(be_mgr,
                                      be_g_E_d,
                                      Be_Or(be_mgr,
                                            be_f_E_d,
                                            be_fRg_L_dP1)));
                    
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                    
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[fRg]]_E^%u <=> [[g]]_E^%u & ([[f]]_E^%u | [[fRg]]_L^min(%u,%u)): ",
                      d, min(d, sbmc_node_info_get_past_depth(rsf_info)),
                      min(d, sbmc_node_info_get_past_depth(lsf_info)),
                      d+1, sbmc_node_info_get_past_depth(info));
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        }
        break;
      }

      case OP_NEXT: {
        unsigned int  d;
        array_t *      lsf_L_past_array;

        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
        nusmv_assert((array_t *)NULL != L_past_array);
        nusmv_assert((array_t *)NULL != E_past_array);

        lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
        nusmv_assert((sbmc_node_info *)NULL != lsf_info);
        nusmv_assert(sbmc_node_info_get_past_depth(lsf_info) ==
                     sbmc_node_info_get_past_depth(info));
        lsf_L_past_array = array_fetch(array_t *,
                                       sbmc_node_info_get_trans_bes(lsf_info), sbmc_L_state());
        nusmv_assert((array_t *)NULL != lsf_L_past_array);
            
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          sbmc_print_node(nusmv_stderr, " ", node, "\n");

        if (do_optimization) {
          /* Add [[Xf]]_E^d <=> [[f]]_L^min(d+1,pd(f)) */
          for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
            be_ptr be_aux, be_lsf_next, be_result;

            be_aux = array_fetch(be_ptr, E_past_array, d);
            nusmv_assert((be_ptr)NULL != be_aux);
            be_lsf_next = array_fetch(be_ptr, lsf_L_past_array,
                                      min(d+1, sbmc_node_info_get_past_depth(info)));
            nusmv_assert((be_ptr)NULL != be_lsf_next);
            be_result = Be_Iff(be_mgr, be_aux, be_lsf_next);

            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr,
                      " [[Xf]]_E^%u <=> [[f]]_L^min(%u+1,%u): ",
                      d, d, sbmc_node_info_get_past_depth(lsf_info));
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        }
        break;
      }

      case OP_HISTORICAL: {
        unsigned int d;

        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
        nusmv_assert((array_t *)NULL != L_past_array);
        nusmv_assert((array_t *)NULL != E_past_array);

        if (do_optimization) {
          /* Optimization: Add [[Hf]]_E^d+1 => [[Hf]]_E^d */
          for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
            be_ptr be_Hf_E_d, be_Hf_E_dP1, be_result;

            be_Hf_E_d = array_fetch(be_ptr, E_past_array, d);
            nusmv_assert((be_ptr)NULL != be_Hf_E_d);
            be_Hf_E_dP1 = array_fetch(be_ptr, E_past_array, d+1);
            nusmv_assert((be_ptr)NULL != be_Hf_E_dP1);
            be_result = Be_Implies(be_mgr, be_Hf_E_dP1, be_Hf_E_d);
                    
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                    
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, " [[Hf]]_E^%u => [[Hf]]_E^%u: ",
                      d+1, d);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        }
        break;
      }

      case OP_ONCE: {
        unsigned int  d;
            
        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
        nusmv_assert((array_t *)NULL != L_past_array);
        nusmv_assert((array_t *)NULL != E_past_array);

        if (do_optimization) {
          /* Optimization: Add [[Of]]_E^d => [[Of]]_E^d+1 */
          for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
            be_ptr be_Of_E_d, be_Of_E_dP1, be_result;

            be_Of_E_d = array_fetch(be_ptr, E_past_array, d);
            nusmv_assert((be_ptr)NULL != be_Of_E_d);
            be_Of_E_dP1 = array_fetch(be_ptr, E_past_array, d+1);
            nusmv_assert((be_ptr)NULL != be_Of_E_dP1);
            be_result = Be_Implies(be_mgr, be_Of_E_d, be_Of_E_dP1);
                    
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, " [[Of]]_E^%u => [[Of]]_E^%u: ", d, d+1);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        }
        break;
      }

      case OP_PREC:
      case OP_NOTPRECNOT: {
        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
        nusmv_assert((array_t *)NULL != L_past_array);
        nusmv_assert((array_t *)NULL != E_past_array);
        break;
      }

      case TRIGGERED:
      case SINCE: {
        if (!sbmc_set_is_in(visit_cache, lsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (!sbmc_set_is_in(visit_cache, rsf)) {
          lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
          has_unprocessed_children = 1;
        }
        if (has_unprocessed_children)
          break;

        nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
        nusmv_assert((array_t *)NULL != L_past_array);
        nusmv_assert((array_t *)NULL != E_past_array);
        break;
      }

      default:
        print_node(stderr, node);
        internal_error(sbmc_SNYI_text, __FILE__, __LINE__);
        break;
      }
    if (has_unprocessed_children)
      continue;

    /* Remove node from unprocessed stack */
    if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);

    /* Mark visited */
    sbmc_set_insert(visit_cache, node);

    /*
     * Constraints common to all subformulae
     */
    if (sbmc_node_info_get_trans_vars(info) != (array_t *)NULL) {
      /***
       * Formula variable translation
       * Add !LoopExists => ([[f]]_L^d == FALSE)
       */
      int d;
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_f_L_d, be_result;

        be_f_L_d = array_fetch(be_ptr, L_past_array, d);
        nusmv_assert((be_ptr)NULL != be_f_L_d);
        be_result = Be_Implies(be_mgr,
                               Be_Not(be_mgr, be_LoopExists),
                               Be_Not(be_mgr, be_f_L_d));
          
        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
              
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          sbmc_print_node(nusmv_stderr, " f: ", node, "\n");
          fprintf(nusmv_stderr,
                  "  !LoopExists => ([[f]]_L^%d <=> FALSE): ", d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
    }
  }
      
  lsDestroy(unprocessed_nodes, NULL);
  sbmc_set_destroy(visit_cache); 
  visit_cache = (hash_ptr)NULL;

  return created_constraints;
}

#if 0
/**Function********************************************************************

  Synopsis           [Create the k-invariant constraints for
  propositional operators at time i.]

  Description        [Create the k-invariant constraints for
  propositional operators at time i. Return a list of be_ptrs for the
  created constraints.]

  SideEffects        [None]

******************************************************************************/
lsList sbmc_unroll_invariant_propositional(const BeEnc_ptr be_enc,
                                           const node_ptr ltlspec,
                                           const unsigned int i_model,
                                           const hash_ptr info_map,
                                           const be_ptr be_InLoop_i,
                                           const be_ptr be_l_i,
                                           const int do_optimization)
{
  unsigned int d;
  Be_Manager_ptr be_mgr;
  hash_ptr visit_cache = (hash_ptr)NULL;
  lsList   unprocessed_nodes = (lsList)NULL;
  lsList   created_constraints = (lsList)NULL;
  const unsigned int i_real = sbmc_real_k(i_model);
  /* Some verbose stuff */
  char *str_debug1 = " Translating formula ";
  char str_debug2[32];

  int c = snprintf(str_debug2, 32, " at timestep %u\n", i_model);
  SNPRINTF_CHECK(c, 32);

  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert((Be_Manager_ptr)NULL != be_mgr);

  /* Create list for the constraints */
  created_constraints = lsCreate();

  /* Verbose output */
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2))
    fprintf(nusmv_stderr,
            "Unrolling k-invariant propositional stuff at time %u\n",
            i_model);

  visit_cache = sbmc_set_create();
  unprocessed_nodes = lsCreate();
  lsNewBegin(unprocessed_nodes, (lsGeneric)ltlspec, LS_NH);
  
  while(lsLength(unprocessed_nodes) > 0) {
    node_ptr node, lsf, rsf;
    sbmc_node_info *info, *lsf_info, *rsf_info;
    array_t *past_array, *lsf_past_array, *rsf_past_array;
    int has_unprocessed_children;
      
    /* Get node */
    if ((lsFirstItem(unprocessed_nodes, (lsGeneric*)&node, LS_NH) != LS_OK) ||
        ((node_ptr)NULL == node))
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Get info */
    info = sbmc_node_info_assoc_find(info_map, node);
    if ((sbmc_node_info *)NULL == info)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Get past_array, i.e., sbmc_node_info_get_trans_bes(info)[i] */
    /* sbmc_init_state_vector should have allocated trans_bes[i] before */
    nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
    nusmv_assert(array_n(sbmc_node_info_get_trans_bes(info)) >= i_real+1);
    past_array = array_fetch(array_t *, sbmc_node_info_get_trans_bes(info), i_real);
    nusmv_assert((array_t *)NULL != past_array);

    /* Already translated? */
    if (sbmc_set_is_in(visit_cache, node)) {
      /* sbmc_node_info_get_trans_bes(info)[i][0...pd] should have
         been built */
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
        nusmv_assert(array_fetch(be_ptr, past_array, d) != 0);
      /* Remove node from unprocessed stack */
      if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
        internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      continue;
    }

    /* Traverse children and make k-invariant constraints */
    lsf = car(node);
    rsf = cdr(node);      
    has_unprocessed_children = 0;
    switch(node_get_type(node)) {
    case ATOM:
    case BIT:
    case DOT:
    case ARRAY: {
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);

      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation already done by
           state vector init */
        nusmv_assert(array_fetch(be_ptr, past_array, 0) != (be_ptr)NULL);
      }
      else {
        /* State variable translation
         * Build [[p]]_i^0 <=> p_i
         */
        be_ptr be_p_i_0, be_realp_i, be_result;

        be_p_i_0 = array_fetch(be_ptr, past_array, 0);
        nusmv_assert((be_ptr)NULL != be_p_i_0);
        be_realp_i = BeEnc_name_to_timed(be_enc, node, i_real);
        nusmv_assert((be_ptr)NULL != be_realp_i);
        be_result = Be_Iff(be_mgr, be_p_i_0, be_realp_i);
        nusmv_assert((be_ptr)NULL != be_result);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  [[p]]_%d^0 <=> p_%d: ", i_model, i_model);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }            
      break;
    }

    case TRUEEXP: {
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);
            
      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);

      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation already done by
           state vector init */
        nusmv_assert(array_fetch(be_ptr, past_array, 0) != (be_ptr)NULL);
      }
      else {
        /*
         * Build [[TRUE]]_i^0 <=> TRUE
         */
        be_ptr be_TRUE_i_0, be_true, be_result;

        be_TRUE_i_0 = array_fetch(be_ptr, past_array, 0);
        nusmv_assert((be_ptr)NULL != be_TRUE_i_0);
        be_true = Be_Truth(be_mgr);
        nusmv_assert((be_ptr)NULL != be_true);
        be_result = Be_Iff(be_mgr, be_TRUE_i_0, be_true);
        nusmv_assert((be_ptr)NULL != be_result);
                
        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  [[TRUE]]_%d^0 <=> TRUE: ", i_model);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      break;
    }

    case FALSEEXP: {
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);
            
      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);
            
      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation already done by
           state vector init */
        nusmv_assert(array_fetch(be_ptr, past_array, 0) != 0);
      }
      else {
        /*
         * Build [[FALSE]]_i^0 <=> FALSE
         */
        be_ptr be_FALSE_i_0 = (be_ptr)NULL, be_false = (be_ptr)NULL,
          be_result = (be_ptr)NULL;

        be_FALSE_i_0 = array_fetch(be_ptr, past_array, 0);
        nusmv_assert((be_ptr)NULL != be_FALSE_i_0);
        be_false = Be_Falsity(be_mgr);
        nusmv_assert((be_ptr)NULL != be_false);
        be_result = Be_Iff(be_mgr, be_FALSE_i_0, be_false);
        nusmv_assert((be_ptr)NULL != be_result);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr,"  [[FALSE]]_%d^0 <=> FALSE: ",i_model);
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
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);
            
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);
            
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(array_t *, 
                                   sbmc_node_info_get_trans_bes(rsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != rsf_past_array);
            
      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation already done by
           state vector init */
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
          nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);
      }
      else {
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
          /* [[f | g]]_i^d <=> [[f]]_i^d | [[g]]_i^d */
          be_ptr be_fORg_i_d, be_f_i_d, be_g_i_d, be_result;
          const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
          const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

          be_fORg_i_d = array_fetch(be_ptr, past_array, d);
          nusmv_assert((be_ptr)NULL != be_fORg_i_d);
          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
          be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
          nusmv_assert((be_ptr)NULL != be_g_i_d);
          be_result = Be_Iff(be_mgr, be_fORg_i_d,
                             Be_Or(be_mgr, be_f_i_d, be_g_i_d));
          nusmv_assert((be_ptr)NULL != be_result);

          /* Save the created constraint */
          lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr,
                    "  [[f | g]]_%d^%d <=> [[f]]_%d^%d | [[g]]_%d^%d: ",
                    i_model, d, i_model, d_lsf, i_model, d_rsf);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      break;
    }

    case AND: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);
            
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);
            
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(rsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != rsf_past_array);
            
      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation already done by
           state vector init */
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
          nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);
      }
      else {
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
          /* [[f & g]]_i^d <=> [[f]]_i^d & [[g]]_i^d */
          be_ptr be_fANDg_i_d, be_f_i_d, be_g_i_d, be_result;
          const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
          const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

          be_fANDg_i_d = array_fetch(be_ptr, past_array, d);
          nusmv_assert((be_ptr)NULL != be_fANDg_i_d);
          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
          be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
          nusmv_assert((be_ptr)NULL != be_g_i_d);
          be_result = Be_Iff(be_mgr,
                             be_fANDg_i_d,
                             Be_And(be_mgr, be_f_i_d, be_g_i_d));
          nusmv_assert((be_ptr)NULL != be_result);

          /* Save the created constraint */
          lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  [[f & g]]_%d^%d <=> [[f]]_%d^%d & [[g]]_%d^%d: ",
                    i_model, d, i_model, d_lsf, i_model, d_rsf);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      break;
    }

    case NOT: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);
            
      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);
            
      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation already done by
           state vector init */
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
          nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);
      }
      else {
        /* State variable translation */
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
          /* Add [[!f]]_i^d <=> ![[f]]_i^d */
          be_ptr be_notf_i_d, be_f_i_d, be_result;

          be_notf_i_d = array_fetch(be_ptr, past_array, d);
          nusmv_assert((be_ptr)NULL != be_notf_i_d);
          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
          be_result = Be_Iff(be_mgr, be_notf_i_d,
                             Be_Not(be_mgr, be_f_i_d));
          nusmv_assert((be_ptr)NULL != be_result);
                    
          /* Save the created constraint */
          lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
            fprintf(nusmv_stderr, "  [!f]]_%d^%d <=> ![[f]]_%d^%d: ",
                    i_model, d, i_model, d),
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr),
              fprintf(nusmv_stderr, "\n");
        }
      }
      break;
    }

    case OP_NEXT:
    case OP_FUTURE:
    case OP_GLOBAL:
    case OP_ONCE:
    case OP_HISTORICAL:
    case OP_PREC:
    case OP_NOTPRECNOT: {
      /* Unary temporal operators */
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;
            
      /* For each future and past formula f,
         sbmc_node_info_get_trans_bes(info)[i][d] should already
         point to a be variable for [[f]]_i^d*/
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
        nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);
            
      /* Not a propositional operator, do nothing */
      break;
    }

    case UNTIL:
    case RELEASES:
    case SINCE:
    case TRIGGERED: {
      /* Binary temporal operators */
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      /* For each future and past formula f,
         sbmc_node_info_get_trans_bes(info)[i][d] should already
         point to a be variable for [[f]]_i^d */

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
        nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);

      /* Not a propositional operator, do nothing */
      break;
    }

    default:
      print_node(stderr, node);
      internal_error(sbmc_SNYI_text, __FILE__, __LINE__);
      break;
    }
    if (has_unprocessed_children)
      continue;

    /* Do the auxiliary translations if necessary */
    if (sbmc_node_info_get_aux_F_node(info)) {
      be_ptr be_aux_i, be_aux_i_minus_1, be_f_i_pd, be_result;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, " Doing <<F f>> translation for ",
                        node, "\n");

      be_aux_i = BeEnc_name_to_timed(be_enc,
                                     sbmc_node_info_get_aux_F_node(info),
                                     i_real);
      nusmv_assert((be_ptr)NULL != be_aux_i);

      if (i_model == 0) {
        /* <<F f>>_0 <=> FALSE */
        be_result = Be_Iff(be_mgr, be_aux_i, Be_Falsity(be_mgr));

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          fprintf(nusmv_stderr, "  <<F f>>_0 <=> FALSE: "),
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr),
            fprintf(nusmv_stderr, "\n");
      }
      else {            
        /* <<F f>>_i <=> <<F f>>_{i-1} | (InLoop_i & [[f]]_i^PD(f)) */
        be_aux_i_minus_1 =
          BeEnc_name_to_timed(be_enc,
                              sbmc_node_info_get_aux_F_node(info),
                              i_real-1);
        nusmv_assert((be_ptr)NULL != be_aux_i_minus_1);
        be_f_i_pd = array_fetch(be_ptr, past_array,
                                sbmc_node_info_get_past_depth(info));
        nusmv_assert((be_ptr)NULL != be_f_i_pd);
        be_result = Be_Iff(be_mgr,
                           be_aux_i,
                           Be_Or(be_mgr,
                                 be_aux_i_minus_1,
                                 Be_And(be_mgr,
                                        be_InLoop_i,
                                        be_f_i_pd)));

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          fprintf(nusmv_stderr,
                  "  <<F f>>_%u <=> <<F f>>_%u | (InLoop_%u & [[f]]_%u^%d): ",
                  i_model, i_model-1, i_model, i_model,
                  sbmc_node_info_get_past_depth(info)),
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr),
            fprintf(nusmv_stderr, "\n");
      }

      /* Save the created constraint */
      lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
    }
    if (((node_ptr)NULL != sbmc_node_info_get_aux_F_node(info)) &&
        do_optimization) {
        /* Optimization: add <<Ff>>_i => <<Ff>>_E */
        be_ptr be_aux_i, be_aux_E, be_result;

        be_aux_i = BeEnc_name_to_timed(be_enc,
                                       sbmc_node_info_get_aux_F_node(info),
                                       i_real);
        nusmv_assert((be_ptr)NULL != be_aux_i);

        be_aux_E = BeEnc_name_to_timed(be_enc,
                                       sbmc_node_info_get_aux_F_node(info),
                                       sbmc_E_state());
        nusmv_assert((be_ptr)NULL != be_aux_E);

        be_result = Be_Implies(be_mgr, be_aux_i, be_aux_E);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
          
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  <<F f>>_%u <=> <<F f>>_E: ", i_model);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }

    if ((node_ptr)NULL != sbmc_node_info_get_aux_G_node(info)) {
      be_ptr be_aux_i, be_aux_i_minus_1, be_f_i_pd, be_result;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, " Doing <<G f>> translation for ",
                        node, "\n");

      be_aux_i = BeEnc_name_to_timed(be_enc,
                                     sbmc_node_info_get_aux_G_node(info),
                                     i_real);
      nusmv_assert((be_ptr)NULL != be_aux_i);

      if (i_model == 0) {
        /* <<G f>>_0 <=> TRUE */
        be_result = Be_Iff(be_mgr, be_aux_i, Be_Truth(be_mgr));

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          fprintf(nusmv_stderr, "  <<G f>>_0 <=> TRUE: "),
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr),
            fprintf(nusmv_stderr, "\n");
      }
      else {            
        /* <<G f>>_i <=> <<G f>>_{i-1} & (!InLoop_i | [[f]]_i^PD(f)) */
        be_aux_i_minus_1 =
          BeEnc_name_to_timed(be_enc,
                              sbmc_node_info_get_aux_G_node(info),
                              i_real-1);
        nusmv_assert((be_ptr)NULL != be_aux_i_minus_1);
        be_f_i_pd = array_fetch(be_ptr, past_array,
                                sbmc_node_info_get_past_depth(info));
        nusmv_assert((be_ptr)NULL != be_f_i_pd);
        be_result = Be_Iff(be_mgr,
                           be_aux_i,
                           Be_And(be_mgr,
                                  be_aux_i_minus_1,
                                  Be_Or(be_mgr,
                                        Be_Not(be_mgr, be_InLoop_i),
                                        be_f_i_pd)));

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          fprintf(nusmv_stderr,
                  "  <<G f>>_%u <=> <<G f>>_%u & (!InLoop_%u | [[f]]_%u^%d): ",
                  i_model, i_model-1, i_model, i_model,
                  sbmc_node_info_get_past_depth(info)),
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr),
            fprintf(nusmv_stderr, "\n");
      }
      /* Save the created constraint */
      lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
    }
    if (((node_ptr)NULL != sbmc_node_info_get_aux_G_node(info))
        && do_optimization) {
      /* Optimization: add <<Gf>>_E => <<Gf>>_i */
      be_ptr be_aux_i, be_aux_E, be_result;

      be_aux_i = BeEnc_name_to_timed(be_enc,
                                     sbmc_node_info_get_aux_G_node(info),
                                     i_real);
      nusmv_assert((be_ptr)NULL != be_aux_i);

      be_aux_E = BeEnc_name_to_timed(be_enc,
                                     sbmc_node_info_get_aux_G_node(info),
                                     sbmc_E_state());
      nusmv_assert((be_ptr)NULL != be_aux_E);

      be_result = Be_Implies(be_mgr, be_aux_E, be_aux_i);

      /* Save the created constraint */
      lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
          
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, "  <<F f>>_E <=> <<F f>>_%u: ", i_model);
        Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
        fprintf(nusmv_stderr, "\n");
      }
    }
      
    if ((array_t *)NULL != sbmc_node_info_get_trans_vars(info)) {
      /*
       * Add (l_i => ([[f]]_L^d <=> [[f]]_i^d)) for
       * formula variable translated subformulae
       */
      unsigned int d;
      array_t * f_L_past_array;

      f_L_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(info),
                                   sbmc_L_state());
      nusmv_assert((array_t *)NULL != f_L_past_array);
          
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr," Doing binding to L for ",node,"\n");
          
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_f_L_d, be_f_i_d, be_result;

        be_f_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_f_i_d);

        be_f_L_d = array_fetch(be_ptr, f_L_past_array, d);
        nusmv_assert((be_ptr)NULL != be_f_L_d);

        be_result = Be_Implies(be_mgr,
                               be_l_i,
                               Be_Iff(be_mgr, be_f_L_d, be_f_i_d));

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
          fprintf(nusmv_stderr, "  l_%u => ([[f]]_L_%d <=> [[f]]_%u^%d): ",
                  i_model, d, i_model, d),
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr),
            fprintf(nusmv_stderr, "\n");
      }
    }

    /* Remove node from unprocessed stack */
    if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Mark visited */
    sbmc_set_insert(visit_cache, node);
  }
      
  lsDestroy(unprocessed_nodes, NULL); 
  unprocessed_nodes = (lsList)NULL;
  sbmc_set_destroy(visit_cache); 
  visit_cache = (hash_ptr)NULL;

  return created_constraints;
}
#endif

/**Function********************************************************************

  Synopsis           [Create the k-invariant constraints for propositional and
  future temporal operators at time i.]

  Description        [Create the k-invariant constraints for propositional and
  future temporal operators at time i. Return a list of be_ptrs for the
  created constraints.]

  SideEffects        [None]

******************************************************************************/
static lsList sbmc_unroll_invariant_f(const BeEnc_ptr be_enc,
                                      const node_ptr ltlspec,
                                      const unsigned int i_model,
                                      const hash_ptr info_map,
                                      const be_ptr be_InLoop_i,
                                      const be_ptr be_l_i,
                                      const be_ptr be_LastState_i,
                                      const be_ptr be_LoopExists,
                                      const int do_optimization)
{
  unsigned int d;
  Be_Manager_ptr be_mgr;
  SymbTable_ptr st = (SymbTable_ptr)NULL;
  hash_ptr visit_cache = (hash_ptr)NULL;
  lsList   unprocessed_nodes = (lsList)NULL;
  lsList   created_constraints = (lsList)NULL;
  const unsigned int i_real = sbmc_real_k(i_model);
  /* Some verbose stuff */
  char *str_debug1 = " Translating formula ";
  char str_debug2[32];

  int chars = snprintf(str_debug2, 32, " at timestep %u\n", i_model);
  SNPRINTF_CHECK(chars, 32);

  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert((Be_Manager_ptr)NULL != be_mgr);

  /* Get symbol table */
  st = BaseEnc_get_symb_table(BASE_ENC(be_enc));
  nusmv_assert((SymbTable_ptr)NULL != st);
  
  /* Create list for the constraints */
  created_constraints = lsCreate();

  /* Verbose output */
  if (opt_verbose_level_ge(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Unrolling k-invariant future stuff at time %u\n",
            i_model);
    fflush(nusmv_stderr);
  }
  

  visit_cache = sbmc_set_create();
  unprocessed_nodes = lsCreate();
  lsNewBegin(unprocessed_nodes, (lsGeneric)ltlspec, LS_NH);
  
  while(lsLength(unprocessed_nodes) > 0) {
    node_ptr node, lsf, rsf;
    sbmc_node_info *info, *lsf_info, *rsf_info;
    array_t *past_array, *lsf_past_array, *rsf_past_array;
    int has_unprocessed_children;
      
    /* Get node */
    if ((lsFirstItem(unprocessed_nodes, (lsGeneric*)&node, LS_NH) != LS_OK) ||
        ((node_ptr)NULL == node))
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Get info */
    info = sbmc_node_info_assoc_find(info_map, node);
    if ((sbmc_node_info *)NULL == info)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Get past_array, i.e., sbmc_node_info_get_trans_bes(info)[i] */
    /* sbmc_init_state_vector should have built trans_bes[i] before */
    nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
    nusmv_assert(array_n(sbmc_node_info_get_trans_bes(info)) >= i_real+1);
    past_array = array_fetch(array_t *,
                             sbmc_node_info_get_trans_bes(info), i_real);
    nusmv_assert((array_t *)NULL != past_array);

    /* Already translated? */
    if (sbmc_set_is_in(visit_cache, node)) {
      /* sbmc_node_info_get_trans_bes(info)[i][0...pd] should have been built */
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
        nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);
      /* Remove node from unprocessed stack */
      if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
        internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      continue;
    }

    /* Traverse children and make k-invariant constraints */
    lsf = car(node);
    rsf = cdr(node);      
    has_unprocessed_children = 0;
    switch(node_get_type(node)) {
    case ATOM:
    case BIT:
    case DOT:
    case ARRAY: {
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);

      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation. */
        /* Should already be constructed in sbmc_init_state_vector. */
        nusmv_assert(array_fetch(be_ptr, past_array, 0) != (be_ptr)NULL);
      }
      else {
        /* Formula variable translation. */
        if(SymbTable_is_symbol_input_var(st, node))
          {
            /* An input variable */
            /* Build [[p]]_i^0 <=> p_i & (!LastState_i | LoopExists) */
            be_ptr be_p_i_0, be_realp_i, be_result;
            nusmv_assert(be_LastState_i != (be_ptr)NULL);

            be_p_i_0 = array_fetch(be_ptr, past_array, 0);
            nusmv_assert((be_ptr)NULL != be_p_i_0);
            be_realp_i = BeEnc_name_to_timed(be_enc, node, i_real);
            nusmv_assert((be_ptr)NULL != be_realp_i);
            be_result = Be_Iff(be_mgr,
                               be_p_i_0,
                               Be_And(be_mgr,
                                      be_realp_i,
                                      Be_Or(be_mgr,
                                            Be_Not(be_mgr, be_LastState_i),
                                            be_LoopExists)));
            nusmv_assert((be_ptr)NULL != be_result);
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr,
                      "  [[p]]_%d^0 <=> p_%d & (LastState_%d | LoopExists): ",
                      i_model, i_model, i_model);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        else
          {
            /* A state variable */
            /* Build [[p]]_i^0 <=> p_i */
            be_ptr be_p_i_0, be_realp_i, be_result;
            
            be_p_i_0 = array_fetch(be_ptr, past_array, 0);
            nusmv_assert((be_ptr)NULL != be_p_i_0);
            be_realp_i = BeEnc_name_to_timed(be_enc, node, i_real);
            nusmv_assert((be_ptr)NULL != be_realp_i);
            be_result = Be_Iff(be_mgr, be_p_i_0, be_realp_i);
            nusmv_assert((be_ptr)NULL != be_result);
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[p]]_%d^0 <=> p_%d: ",
                      i_model, i_model);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
      }
      break;
    }

    case TRUEEXP: {
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);
            
      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);

      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation. */
        /* Should already be constructed in sbmc_init_state_vector. */
        nusmv_assert(array_fetch(be_ptr, past_array, 0) != (be_ptr)NULL);
      }
      else {
        /*
         * Build [[TRUE]]_i^0 <=> TRUE
         */
        be_ptr be_TRUE_i_0, be_true, be_result;

        be_TRUE_i_0 = array_fetch(be_ptr, past_array, 0);
        nusmv_assert((be_ptr)NULL != be_TRUE_i_0);
        be_true = Be_Truth(be_mgr);
        nusmv_assert((be_ptr)NULL != be_true);
        be_result = Be_Iff(be_mgr, be_TRUE_i_0, be_true);
        nusmv_assert((be_ptr)NULL != be_result);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  [[TRUE]]_%d^0 <=> TRUE: ", i_model);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      break;
    }

    case FALSEEXP: {
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);
            
      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);
            
      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation. */
        /* Should already be constructed in sbmc_init_state_vector. */
        nusmv_assert(array_fetch(be_ptr, past_array, 0) != (be_ptr)NULL);
      }
      else {
        /*
         * Build [[FALSE]]_i^0 <=> FALSE
         */
        be_ptr be_FALSE_i_0, be_false, be_result;

        be_FALSE_i_0 = array_fetch(be_ptr, past_array, 0);
        nusmv_assert((be_ptr)NULL != be_FALSE_i_0);
        be_false = Be_Falsity(be_mgr);
        nusmv_assert((be_ptr)NULL != be_false);
        be_result = Be_Iff(be_mgr, be_FALSE_i_0, be_false);
        nusmv_assert((be_ptr)NULL != be_result);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr,"  [[FALSE]]_%d^0 <=> FALSE: ",i_model);
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
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);
            
      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert(lsf_past_array);
            
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(rsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != rsf_past_array);
            
      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation. */
        /* Should already be constructed in sbmc_init_state_vector. */
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
          nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);
      }
      else {
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
          /* [[f | g]]_i^d <=> [[f]]_i^d | [[g]]_i^d */
          be_ptr be_fORg_i_d, be_f_i_d, be_g_i_d, be_result;
          const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
          const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

          be_fORg_i_d = array_fetch(be_ptr, past_array, d);
          nusmv_assert((be_ptr)NULL != be_fORg_i_d);
          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
          be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
          nusmv_assert((be_ptr)NULL != be_g_i_d);
          be_result = Be_Iff(be_mgr, be_fORg_i_d,
                             Be_Or(be_mgr, be_f_i_d, be_g_i_d));
          nusmv_assert((be_ptr)NULL != be_result);

          /* Save the created constraint */
          lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr,
                    "  [[f | g]]_%d^%d <=> [[f]]_%d^%d | [[g]]_%d^%d: ",
                    i_model, d, i_model, d_lsf, i_model, d_rsf);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      break;
    }

    case AND: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t *,sbmc_node_info_get_trans_bes(lsf_info),i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);

      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(rsf_info),
                                   i_real);
      nusmv_assert(rsf_past_array);

      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation. */
        /* Should already be constructed in sbmc_init_state_vector. */
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
          nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);
      }
      else {
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
          /* [[f & g]]_i^d <=> [[f]]_i^d & [[g]]_i^d */
          be_ptr be_fANDg_i_d, be_f_i_d, be_g_i_d, be_result;
          const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
          const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

          be_fANDg_i_d = array_fetch(be_ptr, past_array, d);
          nusmv_assert((be_ptr)NULL != be_fANDg_i_d);
          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
          be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
          nusmv_assert((be_ptr)NULL != be_g_i_d);
          be_result = Be_Iff(be_mgr,
                             be_fANDg_i_d,
                             Be_And(be_mgr, be_f_i_d, be_g_i_d));
          nusmv_assert((be_ptr)NULL != be_result);

          /* Save the created constraint */
          lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  [[f & g]]_%d^%d <=> [[f]]_%d^%d & [[g]]_%d^%d: ",
                    i_model, d, i_model, d_lsf, i_model, d_rsf);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      break;
    }

    case NOT: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);
            
      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);
            
      if (sbmc_node_info_get_trans_vars(info) == (array_t *)NULL) {
        /* Definitional translation. */
        /* Should already be constructed in sbmc_init_state_vector. */
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
          nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);
      }
      else {
        /* Formula variable translation. */
        for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
          if(SymbTable_is_symbol_input_var(st, lsf))
            {
              /* An input variable. */
              /* [[!f]]_i^0 <=> ![[f]]_i^d & (!LastState_i | LoopExists) */
              be_ptr be_notf_i_d, be_f_i_d, be_result;
              nusmv_assert(be_LastState_i != (be_ptr)NULL);

              be_notf_i_d = array_fetch(be_ptr, past_array, d);
              nusmv_assert((be_ptr)NULL != be_notf_i_d);
              be_f_i_d = array_fetch(be_ptr, lsf_past_array, d);
              nusmv_assert((be_ptr)NULL != be_f_i_d);
              be_result = Be_Iff(be_mgr,
                                 be_notf_i_d,
                                 Be_And(be_mgr,
                                        Be_Not(be_mgr, be_f_i_d),
                                        Be_Or(be_mgr,
                                              Be_Not(be_mgr, be_LastState_i),
                                              be_LoopExists)));
              nusmv_assert((be_ptr)NULL != be_result);
              
              /* Save the created constraint */
              lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);
              
              if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                fprintf(nusmv_stderr,
                        "  [!f]]_%d^%d <=> ![[f]]_%d^%d & (!LastState_%d | LoopExists): ",
                        i_model, d, i_model, d, i_model);
                Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                fprintf(nusmv_stderr, "\n");
              }
            }
          else
            {
              /* A state variable. */
              /* [[!f]]_i^d <=> ![[f]]_i^d */
              be_ptr be_notf_i_d, be_f_i_d, be_result;

              be_notf_i_d = array_fetch(be_ptr, past_array, d);
              nusmv_assert((be_ptr)NULL != be_notf_i_d);
              be_f_i_d = array_fetch(be_ptr, lsf_past_array, d);
              nusmv_assert((be_ptr)NULL != be_f_i_d);
              be_result = Be_Iff(be_mgr, be_notf_i_d,
                                 Be_Not(be_mgr, be_f_i_d));
              nusmv_assert((be_ptr)NULL != be_result);
              
              /* Save the created constraint */
              lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);
              
              if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
                fprintf(nusmv_stderr, "  [!f]]_%d^%d <=> ![[f]]_%d^%d: ",
                        i_model, d, i_model, d);
                Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
                fprintf(nusmv_stderr, "\n");
              }
            }
        }
      }
      break;
    }

    case OP_NEXT: {
      array_t * lsf_next_past_array;

      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_next_past_array =
        array_fetch(array_t *,
                    sbmc_node_info_get_trans_bes(lsf_info),
                    i_real + 1);
      nusmv_assert((array_t *)NULL != lsf_next_past_array);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        /* Add [[Xf]]_i^d <=> [[f]]_{i+1}^d */
        be_ptr be_aux, be_lsf_next, be_result;

        be_aux = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_aux);
        be_lsf_next = array_fetch(be_ptr, lsf_next_past_array, d);
        nusmv_assert((be_ptr)NULL != be_lsf_next);
        be_result = Be_Iff(be_mgr, be_aux, be_lsf_next);
        nusmv_assert((be_ptr)NULL != be_result);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  [Xf]]_%d^%d <=> [[f]]_%d^%d: ",
                  i_model, d, i_model+1, d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      break;
    }

    case OP_FUTURE: {
      array_t * next_past_array;

      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;
            
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = 
        array_fetch(array_t *, 
                    sbmc_node_info_get_trans_bes(lsf_info), i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);

      next_past_array = array_fetch(array_t *, 
                                    sbmc_node_info_get_trans_bes(info),
                                    i_real+1);
      nusmv_assert((array_t *)NULL != next_past_array);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        /* Add [[F f]]_i^d <=> [[f]]_i^d | [[F f]]_{i+1}^d */
        be_ptr be_aux, be_lsf, be_aux_next, be_result;

        be_aux = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_aux);
        be_lsf = array_fetch(be_ptr, lsf_past_array, d);
        nusmv_assert((be_ptr)NULL != be_lsf);
        be_aux_next = array_fetch(be_ptr, next_past_array, d);
        nusmv_assert((be_ptr)NULL != be_aux_next);
        be_result = Be_Iff(be_mgr, be_aux,
                           Be_Or(be_mgr, be_lsf, be_aux_next));

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr,
                  " [[F f]]_%d^%d <=> [[f]]_%d^%d | [[F f]]_%d^%d: ",
                  i_model, d, i_model, d, i_model+1, d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }

      if (do_optimization) {
        /* Optimization: <<Ff>>_i => [[Ff]]_i^pd(Ff) */
        be_ptr be_Ff_i_pd, be_auxFf_i, be_result;

        nusmv_assert((node_ptr)NULL != 
                     sbmc_node_info_get_aux_F_node(lsf_info));
        be_auxFf_i =
          BeEnc_name_to_timed(be_enc,
                              sbmc_node_info_get_aux_F_node(lsf_info),
                              i_real);
        nusmv_assert((be_ptr)NULL != be_auxFf_i);

        be_Ff_i_pd = array_fetch(be_ptr, past_array, 
                                 sbmc_node_info_get_past_depth(info));
        nusmv_assert((be_ptr)NULL != be_Ff_i_pd);
                
        be_result = Be_Implies(be_mgr, be_auxFf_i, be_Ff_i_pd);
                    
        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  <<Ff>>_%u => [[Ff]]_%u^%u: ",
                  i_model, i_model, sbmc_node_info_get_past_depth(info));
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
        /* Optimization: InLoop_i => ([[Ff]]_i^d+1 => [[Ff]]_i^d) */
        for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
          be_ptr be_Ff_i_dP1, be_Ff_i_d, be_result;
                    
          be_Ff_i_dP1 = array_fetch(be_ptr, past_array, d+1);
          nusmv_assert((be_ptr)NULL != be_Ff_i_dP1);
                    
          be_Ff_i_d = array_fetch(be_ptr, past_array, d);
          nusmv_assert((be_ptr)NULL != be_Ff_i_d);
                    
          be_result = Be_Implies(be_mgr,
                                 be_InLoop_i,
                                 Be_Implies(be_mgr,
                                            be_Ff_i_dP1,
                                            be_Ff_i_d));
                    
          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                    
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr,
                    "  InLoop_%u => ([[Ff]]_%u^%d => [[Ff]]_%u^%d): ",
                    i_model, i_model, d+1, i_model, d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
        {
          /* Optimization: [[Ff]]_E^d => [[Ff]]_i^d */
          array_t * past_array_Ff_E =
            array_fetch(array_t *, 
                        sbmc_node_info_get_trans_bes(info), 
                        sbmc_E_state());
          nusmv_assert(past_array_Ff_E);

          for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
            be_ptr be_Ff_i_d, be_Ff_E_d, be_result;
                    
            be_Ff_i_d = array_fetch(be_ptr, past_array, d);
            nusmv_assert((be_ptr)NULL != be_Ff_i_d);
                    
            be_Ff_E_d = array_fetch(be_ptr, past_array_Ff_E, d);
            nusmv_assert((be_ptr)NULL != be_Ff_E_d);
                    
            be_result = Be_Implies(be_mgr, be_Ff_E_d, be_Ff_i_d);
                    
            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
                    
            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[Ff]]_E^%u => [[Ff]]_%u^%u: ",
                      d, i_model, d);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        }
      } /* if (do_optimization) */
      break;
    }

    case OP_GLOBAL: {
      array_t * next_past_array;

      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(array_t *,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);

      next_past_array = array_fetch(array_t *,
                                    sbmc_node_info_get_trans_bes(info),
                                    i_real+1);
      nusmv_assert((array_t *)NULL != next_past_array);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        /* Add [[Gf]]_i^d <=> [[f]]_i^d & [[Gf]]_{i+1}^d */
        be_ptr be_Gf_i_d, be_f_i_d, be_Gf_ip1_d, be_result;

        be_Gf_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_Gf_i_d);
        be_f_i_d = array_fetch(be_ptr, lsf_past_array, d);
        nusmv_assert((be_ptr)NULL != be_f_i_d);
        be_Gf_ip1_d = array_fetch(be_ptr, next_past_array, d);
        nusmv_assert((be_ptr)NULL != be_Gf_ip1_d);
        be_result = Be_Iff(be_mgr, be_Gf_i_d,
                           Be_And(be_mgr, be_f_i_d, be_Gf_ip1_d));

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr,
                  "  [[Gf]]_%u^%d <=> [[f]]_%u^%d & [[Gf]]_%u^%d: ",
                  i_model, d, i_model, d, i_model+1, d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }

      if (do_optimization) {
        /* Optimization: [[Gf]]_i^pd(Ff) => <<Gf>>_i */
        be_ptr be_Gf_i_pd, be_auxGf_i, be_result;

        nusmv_assert((node_ptr)NULL !=
                     sbmc_node_info_get_aux_G_node(lsf_info));
        be_auxGf_i =
          BeEnc_name_to_timed(be_enc,
                              sbmc_node_info_get_aux_G_node(lsf_info),
                              i_real);
        nusmv_assert(be_auxGf_i);

        be_Gf_i_pd = array_fetch(be_ptr, past_array, sbmc_node_info_get_past_depth(info));
        nusmv_assert(be_Gf_i_pd);
                
        be_result = Be_Implies(be_mgr, be_Gf_i_pd, be_auxGf_i);
                    
        /* Save the created constraint */
        lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);
                
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  [[Gf]]_%u^%u => <<Gf>>_%u: ",
                  i_model, sbmc_node_info_get_past_depth(info), i_model);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
        /* Optimization: InLoop_i => ([[Gf]]_i^d => [[Gf]]_i^d+1) */
        for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
          be_ptr be_Gf_i_d, be_Gf_i_dP1, be_result;
                    
          be_Gf_i_d = array_fetch(be_ptr, past_array, d);
          nusmv_assert(be_Gf_i_d);
                    
          be_Gf_i_dP1 = array_fetch(be_ptr, past_array, d+1);
          nusmv_assert(be_Gf_i_dP1);
                    
          be_result = Be_Implies(be_mgr,
                                 be_InLoop_i,
                                 Be_Implies(be_mgr,
                                            be_Gf_i_d,
                                            be_Gf_i_dP1));
                    
          /* Save the created constraint */
          lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);
                    
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
            fprintf(nusmv_stderr, "  InLoop_%u => ([[Gf]]_%u^%d => [[Gf]]_%u^%d): ",
                    i_model, i_model, d, i_model, d+1),
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr),
              fprintf(nusmv_stderr, "\n");
        }
        {
          /* Optimization: [[Gf]]_i^d => [[Gf]]_E^d */
          array_t * past_array_Gf_E =
            array_fetch(array_t *, 
                        sbmc_node_info_get_trans_bes(info),
                        sbmc_E_state());
          nusmv_assert((array_t *)NULL != past_array_Gf_E);

          for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
            be_ptr be_Gf_i_d, be_Gf_E_d, be_result;
                    
            be_Gf_i_d = array_fetch(be_ptr, past_array, d);
            nusmv_assert((be_ptr)NULL != be_Gf_i_d);
                    
            be_Gf_E_d = array_fetch(be_ptr, past_array_Gf_E, d);
            nusmv_assert((be_ptr)NULL != be_Gf_E_d);
                    
            be_result = Be_Implies(be_mgr, be_Gf_i_d, be_Gf_E_d);

            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr, "  [[Gf]]_%u^%u => [[Gf]]_E^%u: ",
                      i_model, d, d);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
        }
      } /* if (do_optimization) */
      break;
    }

    case UNTIL: {
      array_t * next_past_array;

      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;
            
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(be_ptr,
                                   sbmc_node_info_get_trans_bes(lsf_info), 
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);
            
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(be_ptr,
                                   sbmc_node_info_get_trans_bes(rsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != rsf_past_array);
            
      next_past_array = array_fetch(array_t *,
                                    sbmc_node_info_get_trans_bes(info),
                                    i_real+1);
      nusmv_assert((array_t *)NULL != next_past_array);
            
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        /* Add [[f U g]]_i^d <=>
         *     [[g]]_i^d | ([[f]]_i^d & [[f U g]]_{i+1}^d)
         */
        be_ptr be_fUg_i_d, be_f_i_d, be_g_i_d, be_fUg_iP1_d, be_result;
        const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
        const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

        be_fUg_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_fUg_i_d);
        be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
        nusmv_assert((be_ptr)NULL != be_f_i_d);
        be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
        nusmv_assert((be_ptr)NULL != be_g_i_d);
        be_fUg_iP1_d = array_fetch(be_ptr, next_past_array, d);
        nusmv_assert((be_ptr)NULL != be_fUg_iP1_d);
        be_result = Be_Iff(be_mgr,
                           be_fUg_i_d,
                           Be_Or(be_mgr,
                                 be_g_i_d,
                                 Be_And(be_mgr,
                                        be_f_i_d,
                                        be_fUg_iP1_d)));
                
        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr,
                  " [[f U g]]_%u^%d <=> [[g]]_%u^%d | ([[f]]_%u^%d & [[f U g]]_%u^%d): ",
                  i_model, d, i_model, d, i_model, d, i_model+1, d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      break;
    }

    case RELEASES: {
      array_t * next_past_array;

      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(be_ptr, 
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);
            
      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(be_ptr,
                                   sbmc_node_info_get_trans_bes(rsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != rsf_past_array);
            
      next_past_array = array_fetch(array_t *, 
                                    sbmc_node_info_get_trans_bes(info),
                                    i_real+1);
      nusmv_assert((array_t *)NULL != next_past_array);
            
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        /* Add [[f R g]]_i^d <=> [[g]]_i^d & ([[f]]_i^d | [[f R g]]_{i+1}^d) */
        be_ptr be_fRg_i_d, be_f_i_d, be_g_i_d, be_fRg_iP1_d, be_result;
        const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
        const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

        be_fRg_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_fRg_i_d);
        be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
        nusmv_assert((be_ptr)NULL != be_f_i_d);
        be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
        nusmv_assert((be_ptr)NULL != be_g_i_d);
        be_fRg_iP1_d = array_fetch(be_ptr, next_past_array, d);
        nusmv_assert((be_ptr)NULL != be_fRg_iP1_d);
        be_result = Be_Iff(be_mgr,
                           be_fRg_i_d,
                           Be_And(be_mgr,
                                  be_g_i_d,
                                  Be_Or(be_mgr,
                                        be_f_i_d,
                                        be_fRg_iP1_d)));

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr,
                  " [[f R g]]_%u^%d <=> [[g]]_%u^%d & ([[f]]_%u^%d | [[f R g]]_%u^%d): ",
                  i_model, d, i_model, d, i_model, d, i_model+1, d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      break;
    }

    case OP_ONCE:
    case OP_HISTORICAL:
    case OP_PREC:
    case OP_NOTPRECNOT: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      /* For each past operator,
         sbmc_node_info_get_trans_bes(info)[i][d] should point to a
         be variable for [[f]]_i^d */
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
        nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);

      /* Not a future operator, do nothing */
      break;
    }

    case SINCE:
    case TRIGGERED: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      /* For each past operator,
         sbmc_node_info_get_trans_bes(info)[i][d] should point to a
         be variable for [[f]]_i^d */
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
        nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);

      /* Not a future operator, do nothing */
      break;
    }
    
    default:
      print_node(stderr, node);
      internal_error(sbmc_SNYI_text, __FILE__, __LINE__);
      break;
    } /* switch */
    if (has_unprocessed_children)
    continue;
  
    /* Do the auxiliary translations if necessary */
    if ((node_ptr)NULL != sbmc_node_info_get_aux_F_node(info)) {
      be_ptr be_aux_i, be_aux_i_minus_1, be_f_i_pd, be_result;
      
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, " Doing <<F f>> translation for ",
                        node, "\n");
      
      be_aux_i = BeEnc_name_to_timed(be_enc, sbmc_node_info_get_aux_F_node(info), i_real);
      nusmv_assert((be_ptr)NULL != be_aux_i);
      
      if (i_model == 0) {
        /* <<F f>>_0 <=> FALSE */
        be_result = Be_Iff(be_mgr, be_aux_i, Be_Falsity(be_mgr));
        
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  <<F f>>_0 <=> FALSE: ");
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      else {            
        /* <<F f>>_i <=> <<F f>>_{i-1} | (InLoop_i & [[f]]_i^PD(f)) */
        be_aux_i_minus_1 = 
          BeEnc_name_to_timed(be_enc,
                              sbmc_node_info_get_aux_F_node(info), i_real-1);
        nusmv_assert((be_ptr)NULL != be_aux_i_minus_1);
        be_f_i_pd = array_fetch(be_ptr, past_array,
                                sbmc_node_info_get_past_depth(info));
        nusmv_assert((be_ptr)NULL != be_f_i_pd);
        be_result = Be_Iff(be_mgr,
                           be_aux_i,
                           Be_Or(be_mgr,
                                 be_aux_i_minus_1,
                                 Be_And(be_mgr,
                                        be_InLoop_i,
                                        be_f_i_pd)));
        
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr,
                  "  <<F f>>_%u <=> <<F f>>_%u | (InLoop_%u & [[f]]_%u^%d): ",
                  i_model, i_model-1, i_model, i_model, sbmc_node_info_get_past_depth(info));
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }

      /* Save the created constraint */
      lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
    }
    if (((node_ptr)NULL != sbmc_node_info_get_aux_F_node(info)) &&
        do_optimization) {
      /* Optimization: add <<Ff>>_i => <<Ff>>_E */
      be_ptr be_aux_i, be_aux_E, be_result;
      
      be_aux_i = BeEnc_name_to_timed(be_enc,
                                     sbmc_node_info_get_aux_F_node(info),
                                     i_real);
      nusmv_assert((be_ptr)NULL != be_aux_i);
      
      be_aux_E = BeEnc_name_to_timed(be_enc,
                                     sbmc_node_info_get_aux_F_node(info),
                                     sbmc_E_state());
      nusmv_assert((be_ptr)NULL != be_aux_E);

      be_result = Be_Implies(be_mgr, be_aux_i, be_aux_E);

      /* Save the created constraint */
      lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, "  <<F f>>_%u <=> <<F f>>_E: ", i_model);
      Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
      fprintf(nusmv_stderr, "\n");
      }
    }

    if ((node_ptr)NULL != sbmc_node_info_get_aux_G_node(info)) {
      be_ptr be_aux_i, be_aux_i_minus_1, be_f_i_pd, be_result;

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, " Doing <<G f>> translation for ",
                        node, "\n");

      be_aux_i = BeEnc_name_to_timed(be_enc, sbmc_node_info_get_aux_G_node(info), i_real);
      nusmv_assert((be_ptr)NULL != be_aux_i);

      if (i_model == 0) {
        /* <<G f>>_0 <=> TRUE */
        be_result = Be_Iff(be_mgr, be_aux_i, Be_Truth(be_mgr));

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  <<G f>>_0 <=> TRUE: ");
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      else {            
        /* <<G f>>_i <=> <<G f>>_{i-1} & (!InLoop_i | [[f]]_i^PD(f)) */
        be_aux_i_minus_1
          = BeEnc_name_to_timed(be_enc,
                                sbmc_node_info_get_aux_G_node(info),
                                i_real-1);
        nusmv_assert((be_ptr)NULL != be_aux_i_minus_1);
        be_f_i_pd = array_fetch(be_ptr, past_array,
                                sbmc_node_info_get_past_depth(info));
        nusmv_assert((be_ptr)NULL != be_f_i_pd);
        be_result = Be_Iff(be_mgr,
                           be_aux_i,
                           Be_And(be_mgr,
                                  be_aux_i_minus_1,
                                  Be_Or(be_mgr,
                                        Be_Not(be_mgr, be_InLoop_i),
                                        be_f_i_pd)));

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  <<G f>>_%u <=> <<G f>>_%u & (!InLoop_%u | [[f]]_%u^%d): ",
                  i_model, i_model-1, i_model, i_model, sbmc_node_info_get_past_depth(info));
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
      /* Save the created constraint */
      lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
    }
    if (((node_ptr)NULL != sbmc_node_info_get_aux_G_node(info)) &&
        do_optimization) {
      /* Optimization: add <<Gf>>_E => <<Gf>>_i */
      be_ptr be_aux_i, be_aux_E, be_result;

      be_aux_i = BeEnc_name_to_timed(be_enc,
                                     sbmc_node_info_get_aux_G_node(info),
                                     i_real);
      nusmv_assert((be_ptr)NULL != be_aux_i);

      be_aux_E = BeEnc_name_to_timed(be_enc,
                                     sbmc_node_info_get_aux_G_node(info),
                                     sbmc_E_state());
      nusmv_assert((be_ptr)NULL != be_aux_E);
          
      be_result = Be_Implies(be_mgr, be_aux_E, be_aux_i);

      /* Save the created constraint */
      lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
          
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, "  <<F f>>_E <=> <<F f>>_%u: ", i_model);
        Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
        fprintf(nusmv_stderr, "\n");
      }
    }

    if ((array_t *)NULL != sbmc_node_info_get_trans_vars(info)) {
      /*
       * Add (l_i => ([[f]]_L^d <=> [[f]]_i^d)) for formula variable
       * translated subformulae
       */
      unsigned int d;
      array_t *f_L_past_array;

      f_L_past_array =
        array_fetch(array_t *, sbmc_node_info_get_trans_bes(info), 
                    sbmc_L_state());
      nusmv_assert((array_t *)NULL != f_L_past_array);
          
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr," Doing binding to L for ",node,"\n");
          
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_f_L_d, be_f_i_d, be_result;

        be_f_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_f_i_d);

        be_f_L_d = array_fetch(be_ptr, f_L_past_array, d);
        nusmv_assert((be_ptr)NULL != be_f_L_d);

        be_result = Be_Implies(be_mgr,
                               be_l_i,
                               Be_Iff(be_mgr, be_f_L_d, be_f_i_d));

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  l_%u => ([[f]]_L_%d <=> [[f]]_%u^%d): ",
                  i_model, d, i_model, d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
    }
  
    /* Remove node from unprocessed stack */
    if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);

    /* Mark visited */
    sbmc_set_insert(visit_cache, node);
  }

  lsDestroy(unprocessed_nodes, NULL); 
  unprocessed_nodes = (lsList)NULL;
  sbmc_set_destroy(visit_cache);
  visit_cache = (hash_ptr)NULL;

  return created_constraints;
}

/**Function********************************************************************

  Synopsis           [Create the k-invariant constraints at time i.]

  Description        [Create the k-invariant constraints at time
  i. Return a list of be_ptrs for the created constraints.]

  SideEffects        [None]

******************************************************************************/
static lsList sbmc_unroll_invariant_p(const BeEnc_ptr be_enc,
                                      const node_ptr ltlspec,
                                      const unsigned int i_model,
                                      const hash_ptr info_map,
                                      const be_ptr be_InLoop_i,
                                      const be_ptr be_l_i,
                                      const int do_optimization)
{
  unsigned int d;
  Be_Manager_ptr be_mgr;
  hash_ptr visit_cache = (hash_ptr)NULL;
  lsList   unprocessed_nodes = (lsList)NULL;
  lsList   created_constraints = (lsList)NULL;
  const unsigned int i_real = sbmc_real_k(i_model);
  /* Some verbose stuff */
  char *str_debug1 = " Translating formula ";
  char str_debug2[32];

  int chars = snprintf(str_debug2, 32, " at timestep %u\n", i_model);
  SNPRINTF_CHECK(chars, 32);

  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert((Be_Manager_ptr)NULL != be_mgr);

  /* Create list for the constraints */
  created_constraints = lsCreate();

  /* Debug output */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "Unrolling k-invariant past stuff at time %u\n",
            i_model);
    fflush(nusmv_stderr);
  }
  
  visit_cache = sbmc_set_create();
  unprocessed_nodes = lsCreate();
  lsNewBegin(unprocessed_nodes, (lsGeneric)ltlspec, LS_NH);
  
  while(lsLength(unprocessed_nodes) > 0) {
    node_ptr node, lsf, rsf;
    sbmc_node_info * info, * lsf_info, * rsf_info;
    array_t * past_array, * lsf_past_array, * rsf_past_array;
    int has_unprocessed_children;
      
    /* Get node */
    if ((lsFirstItem(unprocessed_nodes, (lsGeneric*)&node, LS_NH) != LS_OK) ||
        ((node_ptr)NULL == node))
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Get info */
    info = sbmc_node_info_assoc_find(info_map, node);
    if ((sbmc_node_info *)NULL == info)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      
    /* Get past_array */
    /* sbmc_init_state_vector should have built trans_bes[i] before */
    nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
    nusmv_assert(array_n(sbmc_node_info_get_trans_bes(info)) >= i_real+1);
    past_array = array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                             i_real);
    nusmv_assert((array_t *)NULL != past_array);

    /* Already translated? */
    if (sbmc_set_is_in(visit_cache, node)) {
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)
        nusmv_assert(array_fetch(be_ptr, past_array, d) != (be_ptr)NULL);
      /* Remove node from unprocessed stack */
      if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
        internal_error(sbmc_SNH_text, __FILE__, __LINE__);
      continue;
    }

    /* Traverse children and make k-invariant constraints */
    lsf = car(node);
    rsf = cdr(node);      
    has_unprocessed_children = 0;
    switch(node_get_type(node)) {
    case ATOM:
    case BIT:
    case DOT:
    case ARRAY: {
      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);
      break;
      /* Not a past time operator, do nothing */
    }

    case TRUEEXP:
    case FALSEEXP: {
      nusmv_assert(sbmc_node_info_get_past_depth(info) == 0);
      break;
      /* Not a past time operator, do nothing */
    }

    case XOR:
    case XNOR:
    case IMPLIES:
    case IFF: {
      internal_error("%s:%d: Formula not in NNF\n", __FILE__, __LINE__);
      break;
    }

    case OR:
    case AND: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;
      /* Not a past time operator, do nothing */
      break;
    }

    case NOT: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;
      /* Not a past time operator, do nothing */
      break;
    }

    case OP_NEXT:
    case OP_FUTURE:
    case OP_GLOBAL: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;
      /* Not a past time operator, do nothing */
      break;
    }

    case UNTIL:
    case RELEASES: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;
      /* Not a past time operator, do nothing */
      break;
    }

    case OP_ONCE: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(be_ptr,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_Of_i_d, be_result;
        const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));

        be_Of_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_Of_i_d);

        if (i_model == 0) {
          /* Add [[O f]]_0^d <=> [[f]]_0^0 */
          be_ptr be_f_0_0 = array_fetch(be_ptr, lsf_past_array, 0);
          nusmv_assert((be_ptr)NULL != be_f_0_0);
          be_result = Be_Iff(be_mgr, be_Of_i_d, be_f_0_0);
        }
        else if (i_model >= 1 && d == 0) {
          /* Add [[O f]]_i^0 <=> [[f]]_i^0 | [[O f]]_{i-1}^0*/
          be_ptr be_f_i_d, be_Of_iM1_d;
          array_t * past_array_Of_iM1;

          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);

          past_array_Of_iM1 = array_fetch(array_t *, 
                                          sbmc_node_info_get_trans_bes(info),
                                          i_real - 1);
          nusmv_assert((array_t *)NULL != past_array_Of_iM1);
          be_Of_iM1_d = array_fetch(be_ptr, past_array_Of_iM1, d);
          nusmv_assert((be_ptr)NULL != be_Of_iM1_d);

          be_result = Be_Iff(be_mgr,
                             be_Of_i_d,
                             Be_Or(be_mgr, be_f_i_d, be_Of_iM1_d));
        }
        else if (i_model >= 1 && d > 0) {
          /*
           * Add [[O f]]_i^d <=> * [[f]]_i^d | ITE(l_i,[[O f]]_E^{d-1},[[O f]]_{i-1}^d)
           */
          be_ptr be_prev, be_E, be_f_i_d;
          array_t * prev_past_array;
          array_t * E_past_array;

          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);

          prev_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info), i_real-1);
          nusmv_assert((array_t *)NULL != prev_past_array);
          be_prev = array_fetch(be_ptr, prev_past_array, d);
          nusmv_assert((be_ptr)NULL != be_prev);
                    
          E_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info), sbmc_E_state());
          nusmv_assert((array_t *)NULL != E_past_array);
          be_E = array_fetch(be_ptr, E_past_array, d-1);
          nusmv_assert((be_ptr)NULL != be_E);
                    
          be_result = Be_Iff(be_mgr,
                             be_Of_i_d,
                             Be_Or(be_mgr,
                                   be_f_i_d,
                                   Be_Ite(be_mgr,
                                          be_l_i, be_E, be_prev)));
        }
        else
          internal_error(sbmc_SNH_text,__FILE__,__LINE__);
                
        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  Adding constraint at depth %u: ",d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }


        /* An additional constraint ensuring that the past formula has
         * stabilized at the last unrolling depth */
        if ((i_model >= 1) &&( d == sbmc_node_info_get_past_depth(info))) {
          /*
           * Add [[O f]]_i^d <=> * [[f]]_i^d | ITE(l_i,[[O f]]_E^d,[[O f]]_{i-1}^d)
           */
          be_ptr be_Of_iM1_d, be_Of_E_d, be_f_i_d;
          array_t * Of_iM1_past_array, * Of_E_past_array;

          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);

          Of_iM1_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != Of_iM1_past_array);
          be_Of_iM1_d = array_fetch(be_ptr, Of_iM1_past_array, d);
          nusmv_assert((be_ptr)NULL != be_Of_iM1_d);
                    
          Of_E_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info), sbmc_E_state());
          nusmv_assert((array_t *)NULL != Of_E_past_array);
          be_Of_E_d = array_fetch(be_ptr, Of_E_past_array, d);
          nusmv_assert((be_ptr)NULL != be_Of_E_d);
                    
          be_result = Be_Iff(be_mgr,
                             be_Of_i_d,
                             Be_Or(be_mgr,
                                   be_f_i_d,
                                   Be_Ite(be_mgr,
                                          be_l_i,
                                          be_Of_E_d,
                                          be_Of_iM1_d)));
                
          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  Adding constraint at depth %u: ",d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      } /* for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++)*/

      if (do_optimization) {
        /* Optimization: InLoop_i => ([[Of]]_i^d => [[Of]]_i^d+1) */
        /* Optimization: InLoop_i => ([[Of]]_i^d => [[Of]]_E^d) */
        array_t * past_array_Of_E =
          array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                      sbmc_E_state());
        nusmv_assert((array_t *)NULL != past_array_Of_E);

        for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
          be_ptr be_Of_i_d, be_Of_i_dP1, be_Of_E_d, be_result;
                    
          be_Of_i_d = array_fetch(be_ptr, past_array, d);
          nusmv_assert((be_ptr)NULL != be_Of_i_d);

          be_Of_i_dP1 = array_fetch(be_ptr, past_array, d+1);
          nusmv_assert((be_ptr)NULL != be_Of_i_dP1);
                    
          be_result = Be_Implies(be_mgr,
                                 be_InLoop_i,
                                 Be_Implies(be_mgr,
                                            be_Of_i_d,
                                            be_Of_i_dP1));
                    
          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  InLoop_%u => ([[Of]]_%d^%d => [[Of]]_%d^%d): ",
                    i_model, i_model, d, i_model, d+1);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }

          be_Of_E_d = array_fetch(be_ptr, past_array_Of_E, d);
          nusmv_assert((be_ptr)NULL != be_Of_E_d);
          be_result = Be_Implies(be_mgr,
                                 be_InLoop_i,
                                 Be_Implies(be_mgr,
                                            be_Of_i_d,
                                            be_Of_E_d));

          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  InLoop_%u => ([[Of]]_%d^%d => [[Of]]_E^%d): ",
                    i_model, i_model, d, d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      break;
    }

    case OP_HISTORICAL: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(be_ptr,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_Hf_i_d, be_lsf, be_result;
        const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));

        be_Hf_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert(be_Hf_i_d);

        be_lsf = array_fetch(be_ptr, lsf_past_array, d_lsf);
        nusmv_assert(be_lsf);

        if (i_model == 0) {
          /* Add [[H f]]_0^0 <=> [[f]]_0^0 */
          be_ptr be_f_0_0 = array_fetch(be_ptr, lsf_past_array, 0);
          nusmv_assert(be_f_0_0);
          be_result = Be_Iff(be_mgr, be_Hf_i_d, be_f_0_0);
        }
        else if (i_model >= 1 && d == 0) {
          /* Add [[H f]]_i^0 <=> [[f]]_i^0 & [[H f]]_{i-1}^0*/
          be_ptr be_prev;
          array_t * prev_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != prev_past_array);
          be_prev = array_fetch(be_ptr, prev_past_array, d);
          nusmv_assert((be_ptr)NULL != be_prev);
          be_result = Be_Iff(be_mgr,
                             be_Hf_i_d,
                             Be_And(be_mgr, be_lsf, be_prev));
        }
        else if (i_model >= 1 && d > 0) {
          /*
           * Add [[H f]]_i^d <=>
           * [[f]]_i^d & ITE(l_i,[[H f]]_E^{d-1},[[H f]]_{i-1}^d)
           */
          be_ptr be_prev, be_E;
          array_t * prev_past_array, * E_past_array;

          prev_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != prev_past_array);
          be_prev = array_fetch(be_ptr, prev_past_array, d);
          nusmv_assert((be_ptr)NULL != be_prev);
                    
          E_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        sbmc_E_state());
          nusmv_assert((array_t *)NULL != E_past_array);
          be_E = array_fetch(be_ptr, E_past_array, d-1);
          nusmv_assert((be_ptr)NULL != be_E);

          be_result = Be_Iff(be_mgr,
                             be_Hf_i_d,
                             Be_And(be_mgr,
                                    be_lsf,
                                    Be_Ite(be_mgr,
                                           be_l_i, be_E, be_prev)));
        }
        else
          internal_error(sbmc_SNH_text, __FILE__, __LINE__);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  Adding constraint at depth %u: ",d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }

        /* An additional constraint ensuring that the past formula has
         * stabilized at the last unrolling depth */
        if ((i_model >= 1) && (d == sbmc_node_info_get_past_depth(info))) {
            /*
             * Add [[H f]]_i^d <=> * [[f]]_i^d & ITE(l_i,[[H f]]_E^d,[[H f]]_{i-1}^d)
             */
            be_ptr be_Hf_iM1_d, be_Hf_E_d;
            array_t * Hf_iM1_past_array, * Hf_E_past_array;

            Hf_iM1_past_array =
              array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                          i_real-1);
            nusmv_assert((array_t *)NULL != Hf_iM1_past_array);
            be_Hf_iM1_d = array_fetch(be_ptr, Hf_iM1_past_array, d);
            nusmv_assert((be_ptr)NULL != be_Hf_iM1_d);

            Hf_E_past_array =
              array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                          sbmc_E_state());
            nusmv_assert((array_t *)NULL != Hf_E_past_array);
            be_Hf_E_d = array_fetch(be_ptr, Hf_E_past_array, d);
            nusmv_assert((be_ptr)NULL != be_Hf_E_d);

            be_result = Be_Iff(be_mgr,
                               be_Hf_i_d,
                               Be_And(be_mgr,
                                      be_lsf,
                                      Be_Ite(be_mgr,
                                             be_l_i,
                                             be_Hf_E_d,
                                             be_Hf_iM1_d)));

            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr,
                      "  Adding constraint at depth %u: ",d);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
      }        /* for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) */

      if (do_optimization) {
        /* Optimization: InLoop_i => ([[Hf]]_i^d+1 => [[Hf]]_i^d) */
        /* Optimization: InLoop_i => ([[Hf]]_E^d => [[Hf]]_i^d) */
        array_t * past_array_Hf_E =
          array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                      sbmc_E_state());
        nusmv_assert((array_t *)NULL != past_array_Hf_E);

        for (d = 0; d < sbmc_node_info_get_past_depth(info); d++) {
          be_ptr be_Hf_i_d, be_Hf_i_dP1, be_Hf_E_d, be_result;
                    
          be_Hf_i_d = array_fetch(be_ptr, past_array, d);
          nusmv_assert((be_ptr)NULL !=  be_Hf_i_d);
                    
          be_Hf_i_dP1 = array_fetch(be_ptr, past_array, d+1);
          nusmv_assert((be_ptr)NULL != be_Hf_i_dP1);
                    
          be_result = Be_Implies(be_mgr,
                                 be_InLoop_i,
                                 Be_Implies(be_mgr,
                                            be_Hf_i_dP1,
                                            be_Hf_i_d));

          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  InLoop_%u => ([[Hf]]_%d^%d => [[Hf]]_%d^%d): ",
                    i_model, i_model, d+1, i_model, d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }


          be_Hf_E_d = array_fetch(be_ptr, past_array_Hf_E, d);
          nusmv_assert((be_ptr)NULL != be_Hf_E_d);

          be_result = Be_Implies(be_mgr,
                                 be_InLoop_i,
                                 Be_Implies(be_mgr,
                                            be_Hf_E_d,
                                            be_Hf_i_d));

          /* Save the created constraint */
          lsNewBegin(created_constraints,(lsGeneric)be_result,LS_NH);
                    
          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr,
                    "  InLoop_%u => ([[Hf]]_E^%d => [[Hf]]_%d^%d): ",
                    i_model, d, i_model, d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      }
      break;
    }

    case SINCE: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(be_ptr, 
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);

      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(be_ptr, 
                                   sbmc_node_info_get_trans_bes(rsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != rsf_past_array);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_fSg_i_d, be_result;

        be_fSg_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_fSg_i_d);

        if (i_model == 0) {
          /* Add [[f S g]]_0^d <=> [[g]]_0^0 */
          be_ptr be_g_0_0 = array_fetch(be_ptr, rsf_past_array, 0);
          nusmv_assert((be_ptr)NULL != be_g_0_0);
          be_result = Be_Iff(be_mgr, be_fSg_i_d, be_g_0_0);
        }
        else if (i_model >= 1 && d == 0) {
          /* Add [[f S g]]_i^0 <=> [[g]]_i^0 | ([[f]]_i^0 & [[f S g]]_{i-1}^0)*/
          be_ptr be_f_i_d, be_g_i_d, be_fSg_iM1_d;
          array_t * past_array_fSg_iM1;

          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
                    
          be_g_i_d = array_fetch(be_ptr, rsf_past_array, d);
          nusmv_assert((be_ptr)NULL != be_g_i_d);

          past_array_fSg_iM1 = 
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != past_array_fSg_iM1);
          be_fSg_iM1_d = array_fetch(be_ptr, past_array_fSg_iM1, d);
          nusmv_assert((be_ptr)NULL != be_fSg_iM1_d);

          be_result = Be_Iff(be_mgr,
                             be_fSg_i_d,
                             Be_Or(be_mgr,
                                   be_g_i_d,
                                   Be_And(be_mgr,
                                          be_f_i_d,
                                          be_fSg_iM1_d)));
        }
        else if ((i_model >= 1) && (d > 0)) {
          /*
           * Add [[f S g]]_i^d <=>
           * [[g]]_i^d | ([[f]]_i_d &
           *              ITE(l_i,[[fSg]]_E^{d-1},[[fSg]]_{i-1}^d))
           */
          be_ptr be_f_i_d, be_g_i_d, be_fSg_E_dM1, be_fSg_iM1_d;
          array_t * past_array_fSg_iM1, * past_array_fSg_E;
          const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
          const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
                    
          be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
          nusmv_assert((be_ptr)NULL != be_g_i_d);

          past_array_fSg_E = 
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        sbmc_E_state());
          nusmv_assert((array_t *)NULL != past_array_fSg_E);
          be_fSg_E_dM1 = array_fetch(be_ptr, past_array_fSg_E, d-1);
          nusmv_assert((be_ptr)NULL != be_fSg_E_dM1);

          past_array_fSg_iM1 = 
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != past_array_fSg_iM1);
          be_fSg_iM1_d = array_fetch(be_ptr, past_array_fSg_iM1, d);
          nusmv_assert((be_ptr)NULL != be_fSg_iM1_d);
                    
          be_result = Be_Iff(be_mgr,
                             be_fSg_i_d,
                             Be_Or(be_mgr,
                                   be_g_i_d,
                                   Be_And(be_mgr,
                                          be_f_i_d,
                                          Be_Ite(be_mgr,
                                                 be_l_i,
                                                 be_fSg_E_dM1,
                                                 be_fSg_iM1_d))));
        }
        else
          internal_error(sbmc_SNH_text, __FILE__, __LINE__);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  Adding constraint at depth %u: ",d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }

        /* An additional constraint ensuring that the past formula has
         * stabilized at the last unrolling depth */
        if ((i_model >= 1) && (d == sbmc_node_info_get_past_depth(info))) {
            /* Add [[f S g]]_i^d <=>
             * [[g]]_i^d | ([[f]]_i_d &
             *              ITE(l_i,[[fSg]]_E^d,[[fSg]]_{i-1}^d))
             */
            be_ptr be_f_i_d, be_g_i_d, be_fSg_E_d, be_fSg_iM1_d;
            array_t * past_array_fSg_iM1, * past_array_fSg_E;
            const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
            const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

            be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
            nusmv_assert((be_ptr)NULL != be_f_i_d);
                    
            be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
            nusmv_assert((be_ptr)NULL != be_g_i_d);

            past_array_fSg_E = 
              array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                          sbmc_E_state());
            nusmv_assert((array_t *)NULL != past_array_fSg_E);
            be_fSg_E_d = array_fetch(be_ptr, past_array_fSg_E, d);
            nusmv_assert((be_ptr)NULL != be_fSg_E_d);

            past_array_fSg_iM1 = 
              array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                          i_real-1);
            nusmv_assert((array_t *)NULL != past_array_fSg_iM1);
            be_fSg_iM1_d = array_fetch(be_ptr, past_array_fSg_iM1, d);
            nusmv_assert((be_ptr)NULL != be_fSg_iM1_d);

            be_result = Be_Iff(be_mgr,
                               be_fSg_i_d,
                               Be_Or(be_mgr,
                                     be_g_i_d,
                                     Be_And(be_mgr,
                                            be_f_i_d,
                                            Be_Ite(be_mgr,
                                                   be_l_i,
                                                   be_fSg_E_d,
                                                   be_fSg_iM1_d))));

            /* Save the created constraint */
            lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

            if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
              fprintf(nusmv_stderr,
                      "  Adding constraint at depth %u: ",d);
              Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
              fprintf(nusmv_stderr, "\n");
            }
          }
      } /*for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) */
      break;
    }

    case TRIGGERED: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, rsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)rsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(be_ptr,
                                   sbmc_node_info_get_trans_bes(lsf_info)
                                   , i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);

      rsf_info = sbmc_node_info_assoc_find(info_map, rsf);
      nusmv_assert((sbmc_node_info *)NULL != rsf_info);
      rsf_past_array = array_fetch(be_ptr,
                                   sbmc_node_info_get_trans_bes(rsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != rsf_past_array);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_fTg_i_d, be_result;

        be_fTg_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_fTg_i_d);
                
        if (i_model == 0) {
          /* Add [[f T g]]_0^d <=> [[g]]_0^0 */
          be_ptr be_g_0_0 = array_fetch(be_ptr, rsf_past_array, 0);
          nusmv_assert((be_ptr)NULL != be_g_0_0);
          be_result = Be_Iff(be_mgr, be_fTg_i_d, be_g_0_0);
        }
        else if ((i_model >= 1) && (d == 0)) {
          /* Add [[f T g]]_i^0 <=>
             [[g]]_i^0 & ([[f]]_i^0 | [[f T g]]_{i-1}^0)*/
          be_ptr be_f_i_d, be_g_i_d, be_fTg_iM1_d;
          array_t * past_array_fTg_iM1;

          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
                    
          be_g_i_d = array_fetch(be_ptr, rsf_past_array, d);
          nusmv_assert((be_ptr)NULL != be_g_i_d);

          past_array_fTg_iM1 = 
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != past_array_fTg_iM1);
          be_fTg_iM1_d = array_fetch(be_ptr, past_array_fTg_iM1, d);
          nusmv_assert((be_ptr)NULL != be_fTg_iM1_d);

          be_result = Be_Iff(be_mgr,
                             be_fTg_i_d,
                             Be_And(be_mgr,
                                    be_g_i_d,
                                    Be_Or(be_mgr,
                                          be_f_i_d,
                                          be_fTg_iM1_d)));
        }
        else if ((i_model >= 1) && (d > 0)) {
          /*
           * Add [[f T g]]_i^d <=>
           * [[g]]_i^d & ([[f]]_i_d |
           *              ITE(l_i,[[fTg]]_E^{d-1},[[fTg]]_{i-1}^d))
           */
          be_ptr be_f_i_d, be_g_i_d, be_fTg_E_dM1, be_fTg_iM1_d;
          array_t * past_array_fTg_iM1, * past_array_fTg_E;
          const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
          const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
                    
          be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
          nusmv_assert((be_ptr)NULL != be_g_i_d);

          past_array_fTg_E = 
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        sbmc_E_state());
          nusmv_assert((array_t *)NULL != past_array_fTg_E);
          be_fTg_E_dM1 = array_fetch(be_ptr, past_array_fTg_E, d-1);
          nusmv_assert((be_ptr)NULL != be_fTg_E_dM1);

          past_array_fTg_iM1 = 
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != past_array_fTg_iM1);
          be_fTg_iM1_d = array_fetch(be_ptr, past_array_fTg_iM1, d);
          nusmv_assert((be_ptr)NULL != be_fTg_iM1_d);
                    
          be_result = Be_Iff(be_mgr,
                             be_fTg_i_d,
                             Be_And(be_mgr,
                                    be_g_i_d,
                                    Be_Or(be_mgr,
                                          be_f_i_d,
                                          Be_Ite(be_mgr,
                                                 be_l_i,
                                                 be_fTg_E_dM1,
                                                 be_fTg_iM1_d))));
        }
        else
          internal_error(sbmc_SNH_text, __FILE__, __LINE__);
                
        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  Adding constraint at depth %u: ",d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }

        /* An additional constraint ensuring that the past formula has
         * stabilized at the last unrolling depth */
        if ((i_model >= 1) && (d == sbmc_node_info_get_past_depth(info))) {
          /* Save the created constraint */
          /*
           * Add [[f T g]]_i^d <=>
           * [[g]]_i^d & ([[f]]_i_d |
           *              ITE(l_i,[[fTg]]_E^d,[[fTg]]_{i-1}^d))
           */
          be_ptr be_f_i_d, be_g_i_d, be_fTg_E_d, be_fTg_iM1_d;
          array_t * past_array_fTg_iM1, * past_array_fTg_E;
          const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));
          const unsigned int d_rsf = min(d, sbmc_node_info_get_past_depth(rsf_info));

          be_f_i_d = array_fetch(be_ptr, lsf_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_i_d);
                    
          be_g_i_d = array_fetch(be_ptr, rsf_past_array, d_rsf);
          nusmv_assert((be_ptr)NULL != be_g_i_d);

          past_array_fTg_E = 
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        sbmc_E_state());
          nusmv_assert((array_t *)NULL != past_array_fTg_E);
          be_fTg_E_d = array_fetch(be_ptr, past_array_fTg_E, d);
          nusmv_assert((be_ptr)NULL != be_fTg_E_d);

          past_array_fTg_iM1 = 
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != past_array_fTg_iM1);
          be_fTg_iM1_d = array_fetch(be_ptr, past_array_fTg_iM1, d);
          nusmv_assert((be_ptr)NULL != be_fTg_iM1_d);
                    
          be_result = Be_Iff(be_mgr,
                             be_fTg_i_d,
                             Be_And(be_mgr,
                                    be_g_i_d,
                                    Be_Or(be_mgr,
                                          be_f_i_d,
                                          Be_Ite(be_mgr,
                                                 be_l_i,
                                                 be_fTg_E_d,
                                                 be_fTg_iM1_d))));

          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  Adding constraint at depth %u: ", d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      } /* for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) */
      break;
    }

    case OP_PREC: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(be_ptr,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_Yf_i_d, be_result;
        const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));

        be_Yf_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_Yf_i_d);

        if (i_model == 0) {
          /* Add [[Y f]]_0^d <=> FALSE */
          be_result = Be_Iff(be_mgr, be_Yf_i_d, Be_Falsity(be_mgr));
        }
        else if ((i_model >= 1) && (d == 0)) {
          /* Add [[Y f]]_i^0 <=> [[f]]_{i-1}^0 */
          be_ptr be_f_iM1_0;
          array_t * past_array_f_iM1 =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != past_array_f_iM1);
          be_f_iM1_0 = array_fetch(be_ptr, past_array_f_iM1, d);
          nusmv_assert((be_ptr)NULL != be_f_iM1_0);
          be_result = Be_Iff(be_mgr, be_Yf_i_d, be_f_iM1_0);
        }
        else if ((i_model >= 1) && (d > 0)) {
          /*
           * Add [[Y f]]_i^d <=> ITE(l_i,[[f]]_E^{d-1},[[f]]_{i-1}^min(d,pd(f)))
           */
          be_ptr be_f_iM1_d, be_f_E_dM1;
          array_t * past_array_f_iM1, * past_array_f_E;

          past_array_f_E =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        sbmc_E_state());
          nusmv_assert((array_t *)NULL != past_array_f_E);
          be_f_E_dM1 = array_fetch(be_ptr, past_array_f_E, d-1);
          nusmv_assert((be_ptr)NULL != be_f_E_dM1);

          past_array_f_iM1 =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != past_array_f_iM1);
          be_f_iM1_d = array_fetch(be_ptr, past_array_f_iM1, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_iM1_d);

          be_result = Be_Iff(be_mgr,
                             be_Yf_i_d,
                             Be_Ite(be_mgr,
                                    be_l_i,
                                    be_f_E_dM1,
                                    be_f_iM1_d));
        }
        else
          internal_error(sbmc_SNH_text, __FILE__, __LINE__);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr,
                  "  Adding constraint at depth %u: ", d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
                
        /* An additional constraint ensuring that the past formula has
         * stabilized at the last unrolling depth */
        if ((i_model >= 1) && (d == sbmc_node_info_get_past_depth(info))) {
          /*
           * Add [[Y f]]_i^d <=> ITE(l_i,[[f]]_E^d,[[f]]_{i-1}^d)
           */
          be_ptr be_f_iM1_d, be_f_E_d;
          array_t * past_array_f_iM1, * past_array_f_E;

          past_array_f_E =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        sbmc_E_state());
          nusmv_assert((array_t *)NULL != past_array_f_E);
          be_f_E_d = array_fetch(be_ptr, past_array_f_E, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_E_d);

          past_array_f_iM1 =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != past_array_f_iM1);
          be_f_iM1_d = array_fetch(be_ptr, past_array_f_iM1, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_iM1_d);
                    
          be_result = Be_Iff(be_mgr,
                             be_Yf_i_d,
                             Be_Ite(be_mgr,
                                    be_l_i,
                                    be_f_E_d,
                                    be_f_iM1_d));
          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  Adding constraint at depth %u: ", d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      } /* for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) */
      break;
    }

    case OP_NOTPRECNOT: {
      if (!sbmc_set_is_in(visit_cache, lsf)) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)lsf, LS_NH);
        has_unprocessed_children = 1;
      }
      if (has_unprocessed_children)
        break;

      lsf_info = sbmc_node_info_assoc_find(info_map, lsf);
      nusmv_assert((sbmc_node_info *)NULL != lsf_info);
      lsf_past_array = array_fetch(be_ptr,
                                   sbmc_node_info_get_trans_bes(lsf_info),
                                   i_real);
      nusmv_assert((array_t *)NULL != lsf_past_array);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6))
        sbmc_print_node(nusmv_stderr, str_debug1, node, str_debug2);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_Zf_i_d, be_result;
        const unsigned int d_lsf = min(d, sbmc_node_info_get_past_depth(lsf_info));

        be_Zf_i_d = array_fetch(be_ptr, past_array, d);
        nusmv_assert((be_ptr)NULL != be_Zf_i_d);

        if (i_model == 0) {
          /* Add [[Z f]]_0^d <=> TRUE */
          be_result = Be_Iff(be_mgr, be_Zf_i_d, Be_Truth(be_mgr));
        }
        else if ((i_model >= 1) && (d == 0)) {
          /* Add [[Z f]]_i^0 <=> [[f]]_{i-1}^0 */
          be_ptr be_f_iM1_0;
          array_t * lsf_prev_past_array;

          lsf_prev_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != lsf_prev_past_array);
          be_f_iM1_0 = array_fetch(be_ptr, lsf_prev_past_array, d);
          nusmv_assert((be_ptr)NULL != be_f_iM1_0);
          be_result = Be_Iff(be_mgr, be_Zf_i_d, be_f_iM1_0);
        }
        else if ((i_model >= 1) && (d > 0)) {
          /*
           * Add [[Z f]]_i^d <=> ITE(l_i,[[f]]_E^{d-1},[[f]]_{i-1}^min(d,pd(f)))
           */
          be_ptr be_f_iM1_d, be_f_E_dM1;
          array_t * lsf_prev_past_array, * lsf_E_past_array;

          lsf_E_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        sbmc_E_state());
          nusmv_assert((array_t *)NULL != lsf_E_past_array);
          be_f_E_dM1 = array_fetch(be_ptr, lsf_E_past_array, d-1);
          nusmv_assert((be_ptr)NULL != be_f_E_dM1);
                    
          lsf_prev_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != lsf_prev_past_array);
          be_f_iM1_d = array_fetch(be_ptr,lsf_prev_past_array,d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_iM1_d);
                    
          be_result = Be_Iff(be_mgr,
                             be_Zf_i_d,
                             Be_Ite(be_mgr,
                                    be_l_i,
                                    be_f_E_dM1,
                                    be_f_iM1_d));
        }
        else
          internal_error(sbmc_SNH_text, __FILE__, __LINE__);
                
        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          fprintf(nusmv_stderr, "  Adding constraint at depth %u: ", d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }

        /* An additional constraint ensuring that the past formula has
         * stabilized at the last unrolling depth */
        if ((i_model >= 1) && (d == sbmc_node_info_get_past_depth(info))) {
          /* Add [[Z f]]_i^d <=>  ITE(l_i,[[f]]_E^d,[[f]]_{i-1}^d) */
          be_ptr be_f_iM1_d, be_f_E_d;
          array_t * lsf_prev_past_array, * lsf_E_past_array;

          lsf_E_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        sbmc_E_state());
          nusmv_assert((array_t *)NULL != lsf_E_past_array);
          be_f_E_d = array_fetch(be_ptr, lsf_E_past_array, d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_E_d);
                    
          lsf_prev_past_array =
            array_fetch(array_t *, sbmc_node_info_get_trans_bes(lsf_info),
                        i_real-1);
          nusmv_assert((array_t *)NULL != lsf_prev_past_array);
          be_f_iM1_d = array_fetch(be_ptr,lsf_prev_past_array,d_lsf);
          nusmv_assert((be_ptr)NULL != be_f_iM1_d);

          be_result = Be_Iff(be_mgr,
                             be_Zf_i_d,
                             Be_Ite(be_mgr,
                                    be_l_i,
                                    be_f_E_d,
                                    be_f_iM1_d));
          /* Save the created constraint */
          lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

          if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
            fprintf(nusmv_stderr, "  Adding constraint at depth %u: ", d);
            Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
            fprintf(nusmv_stderr, "\n");
          }
        }
      } /* for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) */
      break;
    }

    default:
      print_node(stderr, node);
      internal_error(sbmc_SNYI_text, __FILE__, __LINE__);
      break;
    }
    if (has_unprocessed_children)
      continue;

    /* Remove node from unprocessed stack */
    if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
      internal_error(sbmc_SNH_text,__FILE__,__LINE__);

    /* Mark visited */
    sbmc_set_insert(visit_cache, node);
  }

  lsDestroy(unprocessed_nodes, NULL);
  sbmc_set_destroy(visit_cache); 
  visit_cache = (hash_ptr)NULL;

  return created_constraints;
}

/**Function********************************************************************

  Synopsis           [Create the formula specific k-dependent constraints.]

  Description        [Create the formula specific k-dependent constraints.
  Return a list of be_ptrs for the created constraints. ]

  SideEffects        [None]

******************************************************************************/
lsList sbmc_formula_dependent(const BeEnc_ptr be_enc,
                                  const node_ptr ltlspec,
                                  const unsigned int k_model,
                                  const hash_ptr info_map)
{
  Be_Manager_ptr be_mgr;
  hash_ptr visit_cache = (hash_ptr)NULL;
  lsList   unprocessed_nodes = (lsList)NULL;
  lsList   created_constraints = (lsList)NULL;

  nusmv_assert((BeEnc_ptr)NULL != be_enc);
  nusmv_assert((node_ptr)NULL != ltlspec);
  nusmv_assert((hash_ptr)NULL != info_map);

  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert((Be_Manager_ptr)NULL != be_mgr);

  /* Create list for the created constraints */
  created_constraints = lsCreate();

  /* Debug output */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr,
            "Creating the formula specific k-dependent constraints for k=%d\n",
            k_model);
    fflush(nusmv_stderr);
  }
  

  visit_cache = sbmc_set_create();
  unprocessed_nodes = lsCreate();
  lsNewBegin(unprocessed_nodes, (lsGeneric)ltlspec, LS_NH);
  
  while(lsLength(unprocessed_nodes) > 0) {
    node_ptr node;
    sbmc_node_info *info;
    array_t *f_E_past_array, *f_L_past_array;
    int has_unprocessed_children;

    /* Get node */
    if ((lsFirstItem(unprocessed_nodes, (lsGeneric*)&node, LS_NH) != LS_OK) ||
        ((node_ptr)NULL == node))
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);

    /* Get info */
    info = sbmc_node_info_assoc_find(info_map, node);
    if ((sbmc_node_info *)NULL == info)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);

    /* Get past_array */
    /* sbmc_make_state_vector should have built trans_bes[i] before */
    nusmv_assert((array_t *)NULL != sbmc_node_info_get_trans_bes(info));
    nusmv_assert(array_n(sbmc_node_info_get_trans_bes(info)) >= 2);
    f_L_past_array = array_fetch(array_t *,
                                 sbmc_node_info_get_trans_bes(info),
                                 sbmc_L_state());
    nusmv_assert((array_t *)NULL != f_L_past_array);
    f_E_past_array = array_fetch(array_t *,
                                 sbmc_node_info_get_trans_bes(info),
                                 sbmc_E_state());
    nusmv_assert((array_t *)NULL != f_E_past_array);

    /* Traverse children */
    has_unprocessed_children = 0;
    switch(node_get_type(node)) {
    case ATOM:
    case BIT:
    case DOT:
    case ARRAY:
    case TRUEEXP:
    case FALSEEXP:
      /* Leaf nodes */
      break;

    case XOR:
    case XNOR:
    case IMPLIES:
    case IFF:
      internal_error("%s:%d: Formula not in NNF\n", __FILE__, __LINE__);
      break;

    case NOT:
    case OP_NEXT:
    case OP_PREC:
    case OP_NOTPRECNOT:
    case OP_ONCE:
    case OP_HISTORICAL:
    case OP_GLOBAL:
    case OP_FUTURE:
      /* Unary operators */
      if (!sbmc_set_is_in(visit_cache, car(node))) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)car(node), LS_NH);
        has_unprocessed_children = 1;
      }
      break;

    case OR:
    case AND:
    case TRIGGERED:
    case SINCE:
    case RELEASES:
    case UNTIL:
      /* Binary operators */
      if (!sbmc_set_is_in(visit_cache, car(node))) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)car(node), LS_NH);
        has_unprocessed_children = 1;
      }
      if (!sbmc_set_is_in(visit_cache, cdr(node))) {
        lsNewBegin(unprocessed_nodes, (lsGeneric)cdr(node), LS_NH);
        has_unprocessed_children = 1;
      }
      break;

    default:
      print_node(stderr, node);
      internal_error("%s:%d: Something not implemented", __FILE__, __LINE__);
      break;
    }
    if (has_unprocessed_children)
      continue;

    /* Remove node from unprocessed stack */
    if (lsDelBegin(unprocessed_nodes, (lsGeneric*)&node) != LS_OK)
      internal_error(sbmc_SNH_text, __FILE__, __LINE__);

    /* Mark visited */
    sbmc_set_insert(visit_cache, node);

    if ((array_t *)NULL != sbmc_node_info_get_trans_vars(info)) {
      /* * Add [[f]]_E^d <=> [[f]]_k^d */
      int d;
      array_t * f_k_past_array =
        array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                    sbmc_real_k(k_model));
      nusmv_assert((array_t *)NULL != f_k_past_array);

      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_f_E_d, be_f_k_d, be_result;

        be_f_E_d = array_fetch(be_ptr, f_E_past_array, d);
        nusmv_assert((be_ptr)NULL != be_f_E_d);
        be_f_k_d = array_fetch(be_ptr, f_k_past_array, d);
        nusmv_assert((be_ptr)NULL != be_f_k_d);
        be_result = Be_Iff(be_mgr, be_f_E_d, be_f_k_d);
            
        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);
            
        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          sbmc_print_node(nusmv_stderr, " f: ", node, "\n");
          fprintf(nusmv_stderr, "  ([[f]]_E^%d <=> [[f]]_%u^%d): ",
                  d, k_model, d);
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
    }

    if ((array_t *)NULL != sbmc_node_info_get_trans_vars(info)) {
      /* Add [[f]]_{k+1}^d <=> [[f]]_L^min(d+1,pd(f)) */
      int d;
      array_t * f_kP1_past_array =
        array_fetch(array_t *, sbmc_node_info_get_trans_bes(info),
                    sbmc_real_k(k_model+1));
      nusmv_assert((array_t *)NULL != f_kP1_past_array);
          
      for (d = 0; d <= sbmc_node_info_get_past_depth(info); d++) {
        be_ptr be_f_kP1_d, be_f_L_dP1, be_result;

        be_f_kP1_d = array_fetch(be_ptr, f_kP1_past_array, d);
        nusmv_assert((be_ptr)NULL != be_f_kP1_d);
        be_f_L_dP1 
          = array_fetch(be_ptr, f_L_past_array,
                        min(d+1, sbmc_node_info_get_past_depth(info)));
        nusmv_assert((be_ptr)NULL != be_f_L_dP1);
        be_result = Be_Iff(be_mgr, be_f_kP1_d, be_f_L_dP1);

        /* Save the created constraint */
        lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

        if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
          sbmc_print_node(nusmv_stderr, " f: ", node, "\n");
          fprintf(nusmv_stderr, "  ([[f]]_%d^%d <=> [[f]]_L^%d): ",
                  k_model+1, d, min(d+1,sbmc_node_info_get_past_depth(info)));
          Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
          fprintf(nusmv_stderr, "\n");
        }
      }
    }

    if ((node_ptr)NULL != sbmc_node_info_get_aux_F_node(info)) {
      /* Add <<Ff>>_E <=> <<Ff>>_k */
      be_ptr be_auxFf_E, be_auxFf_k, be_result;

      be_auxFf_E = BeEnc_name_to_timed(be_enc, 
                                       sbmc_node_info_get_aux_F_node(info),
                                       sbmc_E_state());
      nusmv_assert((be_ptr)NULL != be_auxFf_E);

      be_auxFf_k = BeEnc_name_to_timed(be_enc,
                                       sbmc_node_info_get_aux_F_node(info),
                                       sbmc_real_k(k_model));
      nusmv_assert(be_auxFf_k);

      be_result = Be_Iff(be_mgr, be_auxFf_E, be_auxFf_k);
          
      /* Save the created constraint */
      lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, "  <<Ff>>_E <=> <<Ff>>_%d: ", k_model);
        Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
        fprintf(nusmv_stderr, "\n");
      }
    }


    if (sbmc_node_info_get_aux_G_node(info)) {
      /* Add <<Gf>>_E <=> <<Gf>>_k */
      be_ptr be_auxGf_E, be_auxGf_k, be_result;

      be_auxGf_E = BeEnc_name_to_timed(be_enc,
                                       sbmc_node_info_get_aux_G_node(info),
                                       sbmc_E_state());
      nusmv_assert((be_ptr)NULL != be_auxGf_E);

      be_auxGf_k = BeEnc_name_to_timed(be_enc,
                                       sbmc_node_info_get_aux_G_node(info),
                                       sbmc_real_k(k_model));
      nusmv_assert((be_ptr)NULL != be_auxGf_k);

      be_result = Be_Iff(be_mgr, be_auxGf_E, be_auxGf_k);

      /* Save the created constraint */
      lsNewBegin(created_constraints, (lsGeneric)be_result, LS_NH);

      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, "  <<Gf>>_E <=> <<Gf>>_%d: ", k_model);
        Be_DumpSexpr(be_mgr, be_result, nusmv_stderr);
        fprintf(nusmv_stderr, "\n");
      }
    }

  }
      
  lsDestroy(unprocessed_nodes, NULL); 
  unprocessed_nodes = (lsList)NULL;
  sbmc_set_destroy(visit_cache); 
  visit_cache = (hash_ptr)NULL;

  return created_constraints;
}

/**Function********************************************************************

  Synopsis           [Unroll future and past fragment from
  previous_k+1 upto and including new_k.]

  Description        [Unroll future and past fragment from previous_k+1 
  upto and including new_k. Return a list of constraints.]

  SideEffects        [None]

******************************************************************************/
lsList sbmc_unroll_invariant(const BeEnc_ptr be_enc,
                             const node_ptr bltlspec,
                             const int previous_k,
                             const int new_k,
                             const state_vars_struct *state_vars,
                             array_t * InLoop_array,
                             const hash_ptr info_map,
                             const be_ptr be_LoopExists,
                             const int opt_do_optimization)
{
  int i;
  lsList created_constraints = lsCreate();

  nusmv_assert(previous_k < new_k);
  nusmv_assert(be_LoopExists != (be_ptr)NULL);

  for (i = max(previous_k + 1, 0); i <= new_k; i++) {
    be_ptr be_InLoop_i, be_l_i, be_LastState_i;

    be_InLoop_i = array_fetch(be_ptr, InLoop_array, i);
    nusmv_assert((be_ptr)NULL != be_InLoop_i);
      
    be_l_i = BeEnc_name_to_timed(be_enc,
                                 sbmc_state_vars_get_l_var(state_vars),
                                 sbmc_real_k(i));
    nusmv_assert((be_ptr)NULL != be_l_i);

    be_LastState_i = BeEnc_name_to_timed(be_enc,
                              sbmc_state_vars_get_LastState_var(state_vars),
                              sbmc_real_k(i));
    nusmv_assert((be_ptr)NULL != be_LastState_i);

    {
      /* Future fragment */
      lsList new_constraints = 
        sbmc_unroll_invariant_f(be_enc, bltlspec,
                                i, info_map,
                                be_InLoop_i,
                                be_l_i,
                                be_LastState_i,
                                be_LoopExists,
                                opt_do_optimization);
      lsJoin(created_constraints, new_constraints, 0);
      lsDestroy(new_constraints, NULL);
    }
    {
      /* Past fragment */
      lsList new_constraints = 
        sbmc_unroll_invariant_p(be_enc, bltlspec,
                                    i, info_map,
                                    be_InLoop_i, be_l_i,
                                    opt_do_optimization);
      lsJoin(created_constraints, new_constraints, 0);
      lsDestroy(new_constraints, NULL);
    }
  }
  return created_constraints;
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [Creates several constraints:
  <ul>
  <li>Create the constraint l_{k+1} <=> FALSE</li>
  <li>Create the constraint s_E = s_k</li>
  <li>Create the constraint LoopExists <=> InLoop_k</li>
  <li>Create the formula specific k-dependent constraints</li>
  </ul>]

  SideEffects        [None]

******************************************************************************/
lsList sbmc_dependent(const BeEnc_ptr be_enc,
                          const node_ptr bltlspec,
                          const int k,
                          const state_vars_struct *state_vars,
                          array_t *InLoop_array,
                          const be_ptr be_LoopExists,
                          const hash_ptr info_map)
{
  Be_Manager_ptr be_mgr;
  lsList created_constraints = lsCreate();

  nusmv_assert(k >= 0);
  nusmv_assert((node_ptr)NULL != bltlspec);
  nusmv_assert((BeEnc_ptr)NULL != be_enc);
  nusmv_assert((array_t *)NULL != InLoop_array);
  nusmv_assert((hash_ptr)NULL != info_map);

  /* Get be manager */
  be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert((Be_Manager_ptr)NULL != be_mgr);

  /*
   * Add and force the constraint l_{k+1} <=> FALSE
   */
  {
    be_ptr be_l_kP1 =
      BeEnc_name_to_timed(be_enc,
                          sbmc_state_vars_get_l_var(state_vars),
                          sbmc_real_k(k+1));

    nusmv_assert((be_ptr)NULL != be_l_kP1);

    be_ptr be_constraint = Be_Iff(be_mgr, be_l_kP1, Be_Falsity(be_mgr));
    nusmv_assert((be_ptr)NULL != be_constraint);

    lsNewEnd(created_constraints, be_constraint, LS_NH);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "Forced (l_%d <=> FALSE)", k+1);
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, ": ");
        Be_DumpSexpr(be_mgr, be_constraint, nusmv_stderr);
      }
      fprintf(nusmv_stderr, "\n");
    }
  }

  /*
   * Add and force the constraint s_E = s_k
   */
  {
    be_ptr be_constraint =
      sbmc_equal_vectors_formula(be_enc,
                                 sbmc_state_vars_get_simple_path_system_vars(state_vars),
                                 sbmc_E_state(), sbmc_real_k(k));

    lsNewEnd(created_constraints, be_constraint, LS_NH);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "Forced (s_E = s_%d)", k);
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, ": ");
        Be_DumpSexpr(be_mgr, be_constraint, nusmv_stderr);
      }
      fprintf(nusmv_stderr, "\n");
    }
  }

  /*
   * Add and force the constraint LoopExists <=> InLoop_k
   */
  {
    be_ptr be_InLoop_k, be_constraint;
    be_InLoop_k = array_fetch(be_ptr, InLoop_array, k);
    nusmv_assert((array_t *)NULL != be_InLoop_k);
    be_constraint = Be_Iff(be_mgr, be_LoopExists, be_InLoop_k);

    lsNewEnd(created_constraints, be_constraint, LS_NH);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "Forced (LoopExists <=> InLoop_%d)", k);
      if (opt_verbose_level_ge(OptsHandler_get_instance(), 6)) {
        fprintf(nusmv_stderr, ": ");
        Be_DumpSexpr(be_mgr, be_constraint, nusmv_stderr);
      }
      fprintf(nusmv_stderr, "\n");
    }
  }

  /*
   * Add and force the formula specific k-dependent constraints
   */
  {
    lsList new_constraints = 
      sbmc_formula_dependent(be_enc, bltlspec, k, info_map);
    lsJoin(created_constraints, new_constraints, 0);
    lsDestroy(new_constraints, NULL);
  }
  return created_constraints;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

