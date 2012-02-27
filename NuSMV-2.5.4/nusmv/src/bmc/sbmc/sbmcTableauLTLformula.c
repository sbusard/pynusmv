/**CFile***********************************************************************

  FileName    [sbmcTableauLTLformula.c]

  PackageName [bmc.sbmc]

  Synopsis    [Bmc.Tableau module]

  Description [This module contains all the operations related to the
               construction of SBMC tableaux for LTL formulas]

  SeeAlso     [bmcGen.c, bmcModel.c, bmcConv.c, bmcVarMgr.c]

  Author      [Timo Latvala, Marco Roveri]

  Copyright   [This file is part of the ``sbmc'' package of NuSMV version 2.
  Copyright (C) 2004 Timo Latvala <timo.latvala@tkk.fi>

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

#include "sbmcHash.h"
#include "sbmcNodeStack.h"
#include "sbmcTableau.h"
#include "sbmcUtils.h"

#include "bmc/bmc.h"
#include "bmc/bmcInt.h"
#include "bmc/bmcUtils.h"
#include "bmc/bmcModel.h"

#include "parser/symbols.h"
#include "opt/opt.h"


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

/**Array for caching f(i) values*/
static be_ptr *bmc_cache_f = (be_ptr *)NULL;
static int bmc_cache_f_dim = -1;

/**Array for caching g(i) values*/
static be_ptr *bmc_cache_g = (be_ptr *)NULL;
static int bmc_cache_g_dim = -1;

/**Array for caching il(i) values*/
static be_ptr *bmc_cache_il = (be_ptr *)NULL;
static int bmc_cache_il_dim = -1;

/**Variable for storing the formula depth*/
static unsigned bmc_tab_past_depth;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void bmc_cache_init ARGS((const int count, const int k,
                                 const unsigned pastdepth));

static void bmc_cache_delete ARGS((void));

static unsigned bmc_past_depth ARGS((const node_ptr ltl_wff));

static be_ptr bmc_cache_fetch_f ARGS((const node_ptr ltl_wff, const int time,
                                      const int k, const unsigned pastdepth, 
                                      hashPtr table));

static be_ptr bmc_cache_fetch_g ARGS((const node_ptr ltl_wff, const int time,
                                      const int k, const unsigned pastdepth, 
                                      hashPtr table));

static be_ptr bmc_cache_insert_f ARGS((const node_ptr ltl_wff, const int time,
                                       const int k, const unsigned pastdepth,
                                       hashPtr table, be_ptr result));

static be_ptr bmc_cache_insert_g ARGS((const node_ptr ltl_wff, const int time,
                                       const int k, const unsigned pastdepth,
                                       hashPtr table, be_ptr result));

static be_ptr bmc_cache_insert_il ARGS((const int time, const int k,
                                        be_ptr result));

static be_ptr bmc_cache_fetch_il ARGS((const int time, const int k));

static int formulaMap ARGS((hashPtr table, const node_ptr ltl_wff,
                            unsigned TLcount));

static be_ptr bmcSBMC_tableau_GF_FG_last ARGS((const BeEnc_ptr be_enc,
                                               const node_ptr ltl_wff, 
                                               const int k, const int l,
                                               const unsigned pastdepth,
                                               hashPtr table, hash_ptr memoiz));
static be_ptr last_g ARGS((const BeEnc_ptr be_enc,node_ptr ltl_wff,
                           hashPtr table,hash_ptr memoiz,const int l,const int k,
                           const unsigned pastdepth)); 

static be_ptr last_f ARGS((const BeEnc_ptr be_enc,node_ptr ltl_wff,
                           hashPtr table, hash_ptr memoiz, const int l, const int k,
                           const unsigned pastdepth)); 

static be_ptr get_f_at_time ARGS((const BeEnc_ptr be_enc,
                                  const node_ptr ltl_wff, hashPtr table, 
                                  hash_ptr memoiz,
                                  const int time, const int k, const int l,
                                  const unsigned pastdepth));

static be_ptr get_Globally_at_time ARGS((const BeEnc_ptr be_enc, 
                                         const node_ptr ltl_wff, hashPtr table, 
                                         hash_ptr memoiz,
                                         const int time, const int k, const int l,
                                         const unsigned pastdepth));

static be_ptr get_Eventually_at_time ARGS((const BeEnc_ptr be_enc,
                                           const node_ptr ltl_wff, 
                                           hashPtr table, hash_ptr memoiz,
                                           const int time, const int k, const int l,
                                           const unsigned pastdepth));

static be_ptr get_Until_at_time ARGS((const BeEnc_ptr be_enc,
                                      const node_ptr ltl_wff, 
                                      hashPtr table, hash_ptr memoiz,
                                      const int time, const int k, const int l,
                                      const unsigned pastdepth));

static be_ptr get_V_at_time ARGS((const BeEnc_ptr be_enc,
                                  const node_ptr ltl_wff, 
                                  hashPtr table, hash_ptr memoiz,
                                  const int time, const int k, const int l,
                                  const unsigned pastdepth));

static be_ptr get_Since_at_time ARGS((const BeEnc_ptr be_enc,
                                      const node_ptr ltl_wff, 
                                      hashPtr table, hash_ptr memoiz,
                                      const int time, const int k, const int l,
                                      const unsigned pastdepth));

static be_ptr get_Trigger_at_time ARGS((const BeEnc_ptr be_enc,
                                        const node_ptr ltl_wff, 
                                        hashPtr table, hash_ptr memoiz,
                                        const int time, const int k, const int l,
                                        const unsigned pastdepth));

static be_ptr get_Historically_at_time ARGS((const BeEnc_ptr be_enc,
                                             const node_ptr ltl_wff, 
                                             hashPtr table, hash_ptr memoiz,
                                             const int time, const int k, const int l,
                                             const unsigned pastdepth));

static be_ptr get_Once_at_time ARGS((const BeEnc_ptr be_enc,
                                     const node_ptr ltl_wff, 
                                     hashPtr table, hash_ptr memoiz,
                                     const int time, const int k, const int l,
                                     const unsigned pastdepth));

static be_ptr get_ZY_at_time ARGS((const BeEnc_ptr be_enc,
                                   const node_ptr ltl_wff, 
                                   hashPtr table, hash_ptr memoiz,
                                   const int time, const int k, const int l,
                                   const unsigned pastdepth));

static be_ptr get_g_at_time ARGS((const BeEnc_ptr be_enc,
                                  const node_ptr ltl_wff, 
                                  hashPtr table, hash_ptr memoiz,
                                  const int time, const int k, const int l,
                                  const unsigned pastdepth));

static be_ptr get_el_at_time ARGS((const BeEnc_ptr be_enc, const int time,
                                   const int k));

static be_ptr AtMostOnce ARGS((const BeEnc_ptr be_enc, const int k));

static be_ptr Loop ARGS((const BeEnc_ptr be_enc, const int k));

static be_ptr get_il_at_time ARGS((const BeEnc_ptr be_enc, const int time,
                                   const int k));

static be_ptr get_loop_exists ARGS((const BeEnc_ptr be_enc, const int k));

static be_ptr bmc_tableauGetEventuallyIL_opt ARGS((const BeEnc_ptr be_enc,
                                                   const node_ptr ltl_wff,
                                                   const int k, const int l,
                                                   const unsigned pastdepth,
                                                   hashPtr table, 
                                                   hash_ptr memoiz));

static be_ptr bmc_tableauGetGloballyIL_opt ARGS((const BeEnc_ptr be_enc,
                                                 const node_ptr ltl_wff,
                                                 const int k, const int l,
                                                 const unsigned pastdepth,
                                                 hashPtr table, 
                                                 hash_ptr memoiz));


/**AutomaticEnd***************************************************************/



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Given a wff expressed in ltl builds the model-independent
  tableau at 'time' of a path formula bounded by \[k, l\]]

  Description [The function generates the necessary auxilliary
               predicates (loop, atmostonce) and calls on
               get_f_at_time to generate the tableau for the ltl
               formula.]

  SideEffects        []

  SeeAlso            [AtMostOnce, Loop, get_f_at_time]

******************************************************************************/
be_ptr
BmcInt_SBMCTableau_GetAtTime(const BeEnc_ptr be_enc, const node_ptr ltl_wff,
                             const int time, const int k, const int l)
{
  int count;
  hashPtr formulatable;
  hash_ptr memoiz;
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  be_ptr tableau = Be_Truth(be_mgr);
  be_ptr loop;

  bmc_tab_past_depth = bmc_past_depth(ltl_wff);

  bmc_cache_f = (be_ptr)NULL;
  bmc_cache_g = (be_ptr)NULL;
  bmc_cache_il = (be_ptr)NULL;
  
  formulatable = Bmc_Hash_new_htable();
  memoiz = new_assoc();

  /**Set up cache*/  
  count = formulaMap(formulatable, ltl_wff, 0);
  bmc_cache_init(count, k, bmc_tab_past_depth);

  if (!Bmc_Utils_IsAllLoopbacks(l)) {
    /**No or single loopback*/
    tableau = get_f_at_time(be_enc, ltl_wff, formulatable, memoiz, 
                            0, k, l, 0);
    free_assoc(memoiz);
    Bmc_Hash_delete_table(formulatable);
    bmc_cache_delete();
    return tableau;
  }
 
  /**Generate loop condition*/
  loop = Be_And(be_mgr, Loop(be_enc, k), AtMostOnce(be_enc, k));
  /**require that the formula is true in the initial state*/
  tableau = get_f_at_time(be_enc, ltl_wff, formulatable, memoiz, 
                          0, k, l, 0);
  
  free_assoc(memoiz);
  Bmc_Hash_delete_table(formulatable);
  bmc_cache_delete();
  /**loop_condition \wedge tableau*/
  return Be_And(be_mgr, tableau, loop);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions functions                                  */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************
   
Synopsis           [Construct f(k) att full pastdepth for the GF-,F-,FG-, or G-operator]

Description        [Checks if the il-optimisation is enabled and generates
f(k) accordingly] 

SideEffects        []

SeeAlso            [bmc_tableau_GetEventuallyIL_opt,
                    bmc_tableau_GetGloballyIL_opt]

******************************************************************************/
static be_ptr
bmcSBMC_tableau_GF_FG_last(const BeEnc_ptr be_enc, const node_ptr ltl_wff,
                           const int k, const int l, const unsigned pastdepth, 
                           hashPtr table, hash_ptr memoiz)
{
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  be_ptr tableau = Be_Falsity(be_mgr);

  /**This function should only be called when the full pastdepth has been reached*/ 
  nusmv_assert(pastdepth==bmc_past_depth(ltl_wff));

  if (!opt_bmc_sbmc_il_opt(OptsHandler_get_instance()) || k == 0) { /**normal translation*/    
    be_ptr res; 
    boolean unset = (k==0) && opt_bmc_sbmc_il_opt(OptsHandler_get_instance());

    if (unset) unset_bmc_sbmc_il_opt(OptsHandler_get_instance());
    res = last_g(be_enc, ltl_wff, table, memoiz, l, k, pastdepth);
    if (unset) set_bmc_sbmc_il_opt(OptsHandler_get_instance());
    return res;
  }
  else { /*il-optimisations enabled*/
    if (node_get_type(ltl_wff) == OP_FUTURE) {
      if (opt_bmc_sbmc_gf_fg_opt(OptsHandler_get_instance()) &&
          (node_get_type(car(ltl_wff)) == OP_GLOBAL)) {
        /**FG p*/
        return bmc_tableauGetGloballyIL_opt(be_enc, ltl_wff, k, l,
                                            pastdepth, table, memoiz);
      }
      /** Fp*/
      return bmc_tableauGetEventuallyIL_opt(be_enc, ltl_wff, k, l,
                                            pastdepth, table, memoiz);
    }    
    else if (node_get_type(ltl_wff) == OP_GLOBAL) {
      if (opt_bmc_sbmc_gf_fg_opt(OptsHandler_get_instance()) &&
          (node_get_type(car(ltl_wff)) == OP_FUTURE)) {
        /**GF p*/ 
        return bmc_tableauGetEventuallyIL_opt(be_enc, ltl_wff, k, l,
                                              pastdepth, table, memoiz);
      }      
      /**G p*/
      return bmc_tableauGetGloballyIL_opt(be_enc, ltl_wff, k, l,
                                          pastdepth, table, memoiz);
    }
    else {
      error_unreachable_code();
    }    
  }
  /**never reachable*/
  error_unreachable_code();
  return tableau;
}

/**Function********************************************************************

  Synopsis           [Generate the last f(k) for operators that use the 
  auxillary encoding g.] 

  Description        [The function checks which loop setting is active 
                      and genrates f(k) accordingly.]
  
  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr 
last_g(const BeEnc_ptr be_enc, node_ptr ltl_wff,
       hashPtr table, hash_ptr memoiz, 
       const int l, const int k, const unsigned pastdepth)
{
  /*If il_opt is active only UNTIL and RELEASES can call this function*/
  nusmv_assert(!opt_bmc_sbmc_il_opt(OptsHandler_get_instance()) || 
               ((node_get_type(ltl_wff) == UNTIL) ||
                (node_get_type(ltl_wff) == RELEASES))); 
  nusmv_assert((node_get_type(ltl_wff) == UNTIL) ||
               (node_get_type(ltl_wff) == RELEASES) ||
               (node_get_type(ltl_wff) == OP_FUTURE) ||
               (node_get_type(ltl_wff) == OP_GLOBAL));

  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);

  if (Bmc_Utils_IsAllLoopbacks(l)) {
    be_ptr temp = Be_Falsity(be_mgr);
    int i;
    /** f(k):= \vee_{i=0}^{k-1} el(i) & g(i+1)*/    
    for (i = k; i--; ) {
      temp=Be_Or(be_mgr, temp,
                 Be_And(be_mgr, 
                        get_g_at_time(be_enc, ltl_wff, table, memoiz, 
                                      i+1, k, l, pastdepth), 
                        get_el_at_time(be_enc, i, k)));
    }
    return temp;
  }
  else if (Bmc_Utils_IsSingleLoopback(l)) {
    /**f(k):=g(l)*/
    return get_g_at_time(be_enc, ltl_wff, table, memoiz, 
                         l, k, l, pastdepth);
  }
  else { /**No loopback*/
    /** f(k):= false*/
    return Be_Falsity(be_mgr);
  }
  error_unreachable_code();
  return Be_Falsity(be_mgr);
}


/**Function********************************************************************

  Synopsis [Generate f(k,pastdepth) when pastdepth is less than
  maximum pastdepth, except for OP_NEXT where pastdepth can also
  be the maximum.]

  Description        [The function checks which loop setting is active 
                      and genrates f(k) accordingly.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr 
last_f(const BeEnc_ptr be_enc, node_ptr ltl_wff, hashPtr table, 
       hash_ptr memoiz, const int l, const int k, const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  
  if (Bmc_Utils_IsAllLoopbacks(l)) {
    be_ptr temp = Be_Falsity(be_mgr);
    int i;      
    /** \vee_{i=0}^{k-1} el(i) & f(i+1)*/    
    for (i = k; i--; ) {
      temp = 
        Be_Or(be_mgr, temp,
            Be_And(be_mgr,
                   get_f_at_time(be_enc, ltl_wff, table, memoiz,
                                 i+1, k, l, pastdepth),
                   get_el_at_time(be_enc, i, k)));
    }
    return temp;
  }
  else if (Bmc_Utils_IsSingleLoopback(l)) {
    /**f(k):=f1(l)*/
    return get_f_at_time(be_enc, ltl_wff, table, memoiz, l, k, l, pastdepth);
  }
  else { /**No loopback*/
    /** f(k):= false*/
    return Be_Falsity(be_mgr);
  }
  error_unreachable_code();
  return Be_Falsity(be_mgr);
}


/**Function********************************************************************

  Synopsis           [Map temporal subformulas to an integer, returns the
                      number subformulas with temporal connectives]

  Description        [Stores the nodes of the ltl expression in a table and
                      maps each formula to an integer. Temporal
                      subformulas are numbered from 0...N while all
                      other subformulas are mapped to -2]
                     
  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
formulaMap(hashPtr table, const node_ptr ltl_wff, unsigned TLcount) 
{
  /**Number of temporal connectives in the formula*/
  /*unsigned TLcount=Bmc_Hash_size(table);*/
  Bmc_Stack_ptr thestack = Bmc_Stack_new_stack();  
  Bmc_Stack_push(thestack, ltl_wff);

  while (Bmc_Stack_size(thestack) > 0) {
    node_ptr formula = Bmc_Stack_pop(thestack);
    
    if (Bmc_Hash_find(table, formula) != -1) {      
      continue;
    }
    switch (node_get_type(formula)) {

    case TRUEEXP:
    case FALSEEXP:
    case BIT:
    case DOT:
    case ARRAY:
      Bmc_Hash_insert(table, formula, -2);
      break;
    case NOT:
      /* checks out that argument of NOT operator is actually a variable: */
      nusmv_assert((node_get_type(car(formula)) == DOT) ||
                   (node_get_type(car(formula)) == BIT) ||
                   (node_get_type(car(formula)) == ARRAY));
      Bmc_Hash_insert(table, formula, -2);
      Bmc_Stack_push(thestack, car(formula)); 
      break;
    case AND:
    case OR:
    case IFF:
      Bmc_Hash_insert(table, formula, -2);
      Bmc_Stack_push(thestack, cdr(formula)); 
      Bmc_Stack_push(thestack, car(formula)); 
      break;
    case SINCE:
    case TRIGGERED:
    case UNTIL:
    case RELEASES:
      Bmc_Hash_insert(table, formula, TLcount++);
      Bmc_Stack_push(thestack, cdr(formula)); 
      Bmc_Stack_push(thestack, car(formula)); 
      break;
    case OP_PREC:       
    case OP_NOTPRECNOT: 
    case OP_ONCE:       
    case OP_HISTORICAL: 
    case OP_NEXT:
      Bmc_Hash_insert(table, formula, TLcount++);
      Bmc_Stack_push(thestack, car(formula)); 
      break;
    case OP_GLOBAL:
      if (opt_bmc_sbmc_gf_fg_opt(OptsHandler_get_instance())) {
        if (node_get_type(car(formula)) == OP_FUTURE) {
          Bmc_Hash_insert(table, formula, TLcount++);
          Bmc_Stack_push(thestack, car(car(formula))); 
          break;
        }
      }
      Bmc_Hash_insert(table, formula, TLcount++);
      Bmc_Stack_push(thestack, car(formula)); 
      break;
    case OP_FUTURE: /* EVENTUALLY */
      if (opt_bmc_sbmc_gf_fg_opt(OptsHandler_get_instance())) {
        if (node_get_type(car(formula)) == OP_GLOBAL) {
          Bmc_Hash_insert(table, formula, TLcount++);
          Bmc_Stack_push(thestack, car(car(formula))); 
          break;
        }
      }
      Bmc_Hash_insert(table, formula, TLcount++);
      Bmc_Stack_push(thestack, car(formula)); 
      break;
    default:
      error_unreachable_code();
    }
  }
  Bmc_Stack_delete(thestack);
  return TLcount;
}

/**Function********************************************************************

  Synopsis           [Genrates a boolean expression which is true iff the ltl
                      formula ltl_wff is true at time]

  Description        [The function recursively traverses the formula and genrates
                      the needed boolean expression.]

  SideEffects        []

  SeeAlso            [get_g_at_time]

******************************************************************************/
static be_ptr
get_f_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, hashPtr table,
              hash_ptr memoiz, 
              const int time, const int k, const int l, const unsigned pastdepth) 
{
  SymbTable_ptr st;
  Be_Manager_ptr be_mgr;
  int data;
  node_ptr memkey;
  be_ptr result = (be_ptr) NULL;

  nusmv_assert((time <= k) && (time >= 0) && pastdepth <= bmc_tab_past_depth);

  data = Bmc_Hash_find(table, ltl_wff);
  nusmv_assert(data != -1);

  if (data >= 0) { /**A TL subformula, check cache*/
    result = bmc_cache_fetch_f(ltl_wff, time, k, pastdepth, table);
    if (result != (be_ptr) NULL) {
      return result;
    }
  }

  /* memoizing */
  memkey = find_node(CONS, ltl_wff, 
             find_node(CONS, PTR_FROM_INT(node_ptr, time), 
              find_node(CONS, PTR_FROM_INT(node_ptr, k),
                find_node(CONS, PTR_FROM_INT(node_ptr, l), 
                                PTR_FROM_INT(node_ptr, pastdepth))))); 
  result = find_assoc(memoiz, memkey);
  if (result != (be_ptr) NULL) return result;

  st = BaseEnc_get_symb_table(BASE_ENC(be_enc));
  be_mgr = BeEnc_get_be_manager(be_enc);
  
  switch (node_get_type(ltl_wff)) {
  case TRUEEXP:  return Be_Truth(be_mgr);
  case FALSEEXP: return Be_Falsity(be_mgr);       
  case AND:
    result = Be_And(be_mgr,
                    get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                  time, k, l, pastdepth),
                    get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz,
                                  time, k, l, pastdepth));
    break;

  case OR:
    result = Be_Or(be_mgr,
                    get_f_at_time(be_enc, car(ltl_wff), table, memoiz,
                                  time, k, l, pastdepth),
                    get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz,
                                  time, k, l, pastdepth));
    break;

  case IFF:
    result = Be_Iff(be_mgr,
                    get_f_at_time(be_enc, car(ltl_wff), table, memoiz,
                                  time, k, l, pastdepth),
                    get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz,
                                  time, k, l, pastdepth));
    break;

  case DOT: 
  case BIT: 
    if ((time == k) &&
        BeEnc_is_index_input_var(be_enc, 
                                 BeEnc_name_to_index(be_enc, ltl_wff))) {
      /* input vars when time == max_time evaluate to false: */
      return Be_Falsity(be_mgr);
    }
    return BeEnc_name_to_timed(be_enc, ltl_wff, time);
  
  case ARRAY: 
    if (!SymbTable_is_symbol_declared(st, ltl_wff)) {
      internal_error("Unexpected array node\n");      
    }

    if (!SymbTable_is_symbol_bool_var(st, ltl_wff)) {
      fprintf(nusmv_stderr, "Detected scalar array variable '");
      print_node(nusmv_stderr, ltl_wff);
      fprintf(nusmv_stderr, "'");
      internal_error("Scalar array variable has been found where a boolean "
                     "variable had to be used instead.\n"
                     "This might be due to a bug on your model.");
    }

    if ((time == k) && 
        BeEnc_is_index_input_var(be_enc, 
                                 BeEnc_name_to_index(be_enc, ltl_wff))) {
      /* input vars when time == max_time evaluate to false: */
      return Be_Falsity(be_mgr);
    }

    return BeEnc_name_to_timed(be_enc, ltl_wff, time);
  
  case NOT:  
    /* checks out that argument of NOT operator is actually a variable: */
    nusmv_assert( node_get_type(car(ltl_wff)) == DOT ||
                  node_get_type(car(ltl_wff)) == BIT || 
                  node_get_type(car(ltl_wff)) == ARRAY);
  
    if (!SymbTable_is_symbol_declared(st, car(ltl_wff))) {
      internal_error("Unexpected scalar or undefined node\n");      
    }

    if ((node_get_type(car(ltl_wff)) == ARRAY) && 
        ! SymbTable_is_symbol_bool_var(st, car(ltl_wff))) {
      fprintf(nusmv_stderr, "Detected scalar array variable '");
      print_node(nusmv_stderr, car(ltl_wff));
      fprintf(nusmv_stderr, "'");
      internal_error("Scalar array variable has been found where a boolean "
                     "variable had to be used instead.\n"
                     "This might be due to a bug on your model.");
    }
    if ((time == k) && 
        BeEnc_is_index_input_var(be_enc, 
                                 BeEnc_name_to_index(be_enc, car(ltl_wff)))) {
      /* input vars when time == max_time evaluate to false: */
      return Be_Falsity(be_mgr);
    }
    return Be_Not(be_mgr, BeEnc_name_to_timed(be_enc, car(ltl_wff), time));
  
  case OP_NEXT: 
    if (time < k) {
      /**f(time):=f(time+1)*/
      result = get_f_at_time(be_enc, car(ltl_wff), table,  memoiz,
                             time+1, k, l, pastdepth);
    }
    else { /**time=k*/
      result = last_f(be_enc, car(ltl_wff), table, memoiz, l, k,
                      ((pastdepth < bmc_past_depth(ltl_wff)) ? 
                       pastdepth + 1 :
                       bmc_past_depth(ltl_wff)));  
    }
    break;
  
  case OP_NOTPRECNOT:
  case OP_PREC: 
    result = get_ZY_at_time(be_enc, ltl_wff, table, memoiz, 
                            time, k, l, pastdepth);  
    break;

  case OP_GLOBAL: 
    result = bmc_cache_insert_f(ltl_wff, time, k, pastdepth, table,
                                get_Globally_at_time(be_enc, ltl_wff, 
                                                     table, memoiz, 
                                                     time, k, l, pastdepth));
    break;

  case OP_FUTURE: 
    result = bmc_cache_insert_f(ltl_wff, time, k, pastdepth, table,
                                get_Eventually_at_time(be_enc, ltl_wff, 
                                                       table, memoiz, 
                                                       time, k, l, pastdepth));
    break;

  case UNTIL: 
    result = bmc_cache_insert_f(ltl_wff, time, k, pastdepth, table,
                                get_Until_at_time(be_enc, ltl_wff, table, memoiz,
                                                  time, k, l, pastdepth));
    break;

  case RELEASES:
    result = bmc_cache_insert_f(ltl_wff, time, k, pastdepth, table,
                                get_V_at_time(be_enc, ltl_wff, table, memoiz, 
                                              time, k, l, pastdepth));
    break;

  case SINCE:
    result = bmc_cache_insert_f(ltl_wff, time, k, pastdepth, table,
                                get_Since_at_time(be_enc, ltl_wff, table, memoiz,
                                                  time, k, l, pastdepth));
    break;

  case TRIGGERED: 
    result = bmc_cache_insert_f(ltl_wff, time, k, pastdepth, table,
                                get_Trigger_at_time(be_enc, ltl_wff, table, 
                                                    memoiz, 
                                                    time, k, l, pastdepth));
    break;

  case OP_ONCE:  
    result = bmc_cache_insert_f(ltl_wff, time, k, pastdepth, table,
                                get_Once_at_time(be_enc, ltl_wff, table, memoiz,
                                                 time, k, l, pastdepth));
    break;

  case OP_HISTORICAL: 
    result = bmc_cache_insert_f(ltl_wff, time, k, pastdepth, table,
                                get_Historically_at_time(be_enc, ltl_wff,
                                         table, memoiz, time, k, l, pastdepth));
    break;

  case IMPLIES:
      internal_error("'Implies' should had been nnf-ed away!\n");
  case ATOM:
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
  case EBF:
  case EBG:
  case ABF:
  case ABG:
    internal_error("f: Unexpected CTL operator, node type %d\n",
                    node_get_type(ltl_wff) );
  default:
    internal_error("f: Unexpected operator, node type %d\n",
                    node_get_type(ltl_wff) );
    /* no other type are available here: */
    error_unreachable_code();
  }

  nusmv_assert(result != (be_ptr) NULL);
  /* memoizing */
  insert_assoc(memoiz, memkey, (node_ptr) result);
  
  return result;
}

/**Function********************************************************************

  Synopsis           [Generates a boolean expression which is true iff the ltl
                      formula ltl_wff is true at time, handles the GLOBALLY 
                      operator]

  Description        [The function recursively traverses the formula and
                      generates the needed boolean expression.]

  SideEffects        []

  SeeAlso            [get_f_at_time, get_g_at_time]

******************************************************************************/
static be_ptr
get_Globally_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
                     hashPtr table, hash_ptr memoiz,                     
                     const int time, const int k, const int l,
                     const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  nusmv_assert((time >= 0) && (time <= k));
  nusmv_assert(node_get_type(ltl_wff) == OP_GLOBAL);

  be_mgr = BeEnc_get_be_manager(be_enc);


  if (time < k) {
    if (opt_bmc_sbmc_gf_fg_opt(OptsHandler_get_instance()) && 
        (node_get_type(car(ltl_wff)) == OP_FUTURE)) {
      /**f(i):=f(k)*/
      return get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                           k, k, l, pastdepth);
    }
    /*f(time):=f1(time)\wedge f(time+1)*/
    return Be_And(be_mgr, 
                  get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                time, k, l, pastdepth),
                  get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                time+1, k, l, pastdepth));
  }
  /**time=k*/ 
  if (opt_bmc_sbmc_gf_fg_opt(OptsHandler_get_instance()) && 
     (node_get_type(car(ltl_wff)) == OP_FUTURE)) {
    /**GF p only depends on f(k, pastdepth(ltl_wff))*/
    return bmcSBMC_tableau_GF_FG_last(be_enc, ltl_wff, k, l,
                                      bmc_past_depth(ltl_wff), 
                                      table, memoiz);
  }
  /*For pastdepth<bmc_past_depth(ltl_wff): f(k):=f1(k)\wedge last_f
    else: f(k):=f1(k)\wedge bmcSBMC_tableau_GF_FG(k,pastdepth(ltl_wff)) */
  return Be_And(be_mgr,
                get_f_at_time(be_enc, car(ltl_wff), table, memoiz, time, k, l,
                              pastdepth),
                (pastdepth < bmc_past_depth(ltl_wff)) ? 
                last_f(be_enc, ltl_wff, table, memoiz, l, k, pastdepth + 1) :
                bmcSBMC_tableau_GF_FG_last(be_enc, ltl_wff, k, l,
                                           bmc_past_depth(ltl_wff), 
                                           table, memoiz));
}

/**Function********************************************************************

Synopsis           [Genrates a boolean expression which is true iff the ltl
formula ltl_wff is true at time, handles the FINALLY 
operator]

Description        [The function recursively traverses the formula and genrates
the needed boolean expression.]

SideEffects        []

SeeAlso            [get_f_at_time, get_g_at_time]

******************************************************************************/
static be_ptr
get_Eventually_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
                       hashPtr table, hash_ptr memoiz, 
                       const int time, const int k, const int l,
                       const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  nusmv_assert((time >= 0) && (time <= k));
  nusmv_assert(node_get_type(ltl_wff) == OP_FUTURE);

  be_mgr = BeEnc_get_be_manager(be_enc);

  if (time < k) {
    if (opt_bmc_sbmc_gf_fg_opt(OptsHandler_get_instance()) && 
       (node_get_type(car(ltl_wff)) == OP_GLOBAL)) {
      /**f(i):=f(k)*/
      return get_f_at_time(be_enc, ltl_wff, table, memoiz, k, k, l, pastdepth);
    }
    /*f(time):=f1(time)\vee f(time+1)*/
    return Be_Or(be_mgr,
                 get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                               time, k, l, pastdepth),
                 get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                               time+1, k, l, pastdepth));
  }
  /**time=k*/
  if (opt_bmc_sbmc_gf_fg_opt(OptsHandler_get_instance()) && 
     (node_get_type(car(ltl_wff)) == OP_GLOBAL)) {
    /**FG p only depends on f(k, pastdepth(ltl_wff))*/
    return bmcSBMC_tableau_GF_FG_last(be_enc, ltl_wff, k, l,
                                      bmc_past_depth(ltl_wff), table, memoiz);
  }
  /*For pastdepth<bmc_past_depth(ltl_wff): f(k):=f1(k)\vee last_f
    else: f(k):=f1(k)\vee bmcSBMC_tableau_GF_FG(k,pastdepth(ltl_wff)) */
  return Be_Or(be_mgr,
               get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                             time, k, l, pastdepth),
               (pastdepth < bmc_past_depth(ltl_wff)) ? 
               last_f(be_enc, ltl_wff, table, memoiz, l, k, pastdepth + 1) :
               bmcSBMC_tableau_GF_FG_last(be_enc, ltl_wff, k, l, 
                                          bmc_past_depth(ltl_wff), 
                                          table, memoiz));
}

/**Function********************************************************************

  Synopsis           [Genrates a boolean expression which is true iff the ltl
                      formula ltl_wff is true at time, handles the UNTIL 
                      operator]

  Description        [The function recursively traverses the formula and genrates
                      the needed boolean expression.]

  SideEffects        []

  SeeAlso            [get_f_at_time, get_g_at_time]

******************************************************************************/
static be_ptr
get_Until_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
                  hashPtr table, hash_ptr memoiz, 
                  const int time, const int k, const int l, 
                  const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  const unsigned pd = bmc_past_depth(ltl_wff);
  nusmv_assert((time >= 0) && (time <= k));
  nusmv_assert(node_get_type(ltl_wff) == UNTIL);

  be_mgr = BeEnc_get_be_manager(be_enc);

  if (time < k) {
    /*f(time):=f2(time)\vee (f1(time)\wedge f(time+1))*/
    return Be_Or(be_mgr,
                 get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                               time, k, l, pastdepth),
                 Be_And(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        get_f_at_time(be_enc, ltl_wff, table, memoiz,
                                      time+1, k, l, pastdepth)));
  }     
  /**time=k*/
  if (pastdepth < bmc_past_depth(ltl_wff)) {
    return Be_Or(be_mgr,
                 get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                               time, k, l, pastdepth),
                 Be_And(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        last_f(be_enc, ltl_wff, table, memoiz, 
                               l, k, pastdepth+1)));
  }
  /**pastdepth==n_\psi*/
  return Be_Or(be_mgr,
               get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                             time, k, l, pd),
               Be_And(be_mgr,
                      get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                    time, k, l, pd),
                      last_g(be_enc, ltl_wff, table, memoiz, l, k, pd)));
}

/**Function********************************************************************

  Synopsis           [Genrates a boolean expression which is true iff the ltl
                      formula ltl_wff is true at time, handles the RELEASE 
                      operator]

  Description        [The function recursively traverses the formula and
                      genrates the needed boolean expression.]

  SideEffects        []

  SeeAlso            [get_f_at_time, get_g_at_time]

******************************************************************************/
static be_ptr
get_V_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
              hashPtr table, hash_ptr memoiz, 
              const int time, const int k, const int l,
              const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  const unsigned pd = bmc_past_depth(ltl_wff);
  nusmv_assert((time >= 0) && (time <= k));
  nusmv_assert(node_get_type(ltl_wff) == RELEASES);

  be_mgr = BeEnc_get_be_manager(be_enc);

  if (time < k) {
    /*f(time):=f2(time)\wedge (f1(time)\vee f(time+1))*/
    return Be_And(be_mgr,
                  get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                                time, k, l, pastdepth),
                  Be_Or(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                      time+1, k, l, pastdepth)));
  }
  /**time=k*/
  if (pastdepth < bmc_past_depth(ltl_wff)) {
    return Be_And(be_mgr,
                  get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                                time, k, l, pastdepth),
                  Be_Or(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        last_f(be_enc, ltl_wff, table, memoiz, 
                               l, k, pastdepth+1)));
  }
  /**pastdepth==n_\psi*/
  return Be_And(be_mgr,
                get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                              time, k, l, pd),
                Be_Or(be_mgr,
                      get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                    time, k, l, pd),
                      last_g(be_enc, ltl_wff, table, memoiz, l, k, pd)));
}

/**Function********************************************************************

  Synopsis           [Genrates a boolean expression which is true iff the ltl
                      formula ltl_wff is true at time, handles the SINCE 
                      operator]

  Description        [The function recursively traverses the formula and
                      genrates the needed boolean expression.]

  SideEffects        []

  SeeAlso            [get_f_at_time, get_g_at_time]

******************************************************************************/
static be_ptr
get_Since_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
                  hashPtr table, hash_ptr memoiz, 
                  const int time, const int k, const int l, 
                  const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  nusmv_assert((node_get_type(ltl_wff) == SINCE) && (time < k+1));

  be_mgr = BeEnc_get_be_manager(be_enc);

  if ((time == 0) && (pastdepth == 0)) {
    return get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                         time, k, l, pastdepth); 
  }
  else if ((pastdepth == 0) && (time > 0) && (time <= k)) {
    /*f(i):=f2(i)\vee (f1(i) \wedge f(i-1))*/
    return Be_Or(be_mgr,
                 get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                               time, k, l, pastdepth),
                 Be_And(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                      time-1, k, l, pastdepth)));
  }
  else if ((pastdepth > 0) && (time > 1) && (time <= k)) {
    /**f(i, d):=f2(i, d)\vee(f1(i, d)\wedge (el(i)->f(k, d-1), f(i-1, d)))*/ 
    return Be_Or(be_mgr,
                 get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                               time, k, l, pastdepth),
                 Be_And(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        Be_Ite(be_mgr,
                               get_el_at_time(be_enc, time-1, k),
                               get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                             k, k, l, pastdepth-1),
                               get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                             time-1, k, l, pastdepth))));
  } 
  else if ((pastdepth > 0) && (time == 1)) {
    /**f(1,d):=f2(i,d)\vee(f1(i,d)\wedge f(k,d-1))*/ 
    return Be_Or(be_mgr,
                 get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                               time, k, l, pastdepth),
                 Be_And(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                      k, k, l, pastdepth - 1)));    
  }
  error_unreachable_code();
  return 0; /**Should not be reached*/
}

/**Function********************************************************************

  Synopsis           [Genrates a boolean expression which is true iff the ltl
                      formula ltl_wff is true at time, handles the TRIGGER 
                      operator]

  Description        [The function recursively traverses the formula and
                      genrates the needed boolean expression.]

  SideEffects        []

  SeeAlso            [get_f_at_time, get_g_at_time]

******************************************************************************/
static be_ptr
get_Trigger_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
                    hashPtr table, hash_ptr memoiz, 
                    const int time, const int k, const int l,
                    const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  nusmv_assert((node_get_type(ltl_wff) == TRIGGERED) && (time < k+1));

  be_mgr = BeEnc_get_be_manager(be_enc);

  if ((time == 0) && (pastdepth == 0)) {
    return get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                         time, k, l, pastdepth); 
  }
  else if ((pastdepth == 0) && (time > 0) && (time <= k)) {
    /*f(time):=f2(time)\wedge (f1(time)\vee f(time-1))*/
    return Be_And(be_mgr,
                  get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                                time, k, l, pastdepth),
                  Be_Or(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                      time-1, k, l, pastdepth)));
  }
  else if ((pastdepth > 0) && (time > 1) && (time <= k)) {
    /**f(i,d):=f2(i,d)\wedge(f1(i,d)\vee (el(i)->f(k,d-1)),f(i-1,d))*/ 
    return Be_And(be_mgr,
                  get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                                time, k, l, pastdepth),
                  Be_Or(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        Be_Ite(be_mgr,
                               get_el_at_time(be_enc, time-1, k),
                               get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                             k, k, l, pastdepth-1),
                               get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                             time-1, k, l, pastdepth)))); 
  } 
  else if ((pastdepth > 0) && (time == 1)) {
    /**f(i,d):=f2(1,d)\wedge(f1(1,d)\vee f(k,d-1))*/ 
    return Be_And(be_mgr,
                  get_f_at_time(be_enc, cdr(ltl_wff), table, memoiz, 
                                time, k, l, pastdepth),
                  Be_Or(be_mgr,
                        get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                      time, k, l, pastdepth),
                        get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                      k, k, l, pastdepth-1)));
  }
  error_unreachable_code();
  return 0; /**Should not be reached*/
}

/**Function********************************************************************

  Synopsis           [Genrates a boolean expression which is true iff the ltl
                      formula ltl_wff is true at time, handles the HISTORICALLY
                      operator]

  Description        [The function recursively traverses the formula and genrates
                      the needed boolean expression.]

  SideEffects        []

  SeeAlso            [get_f_at_time, get_g_at_time]

******************************************************************************/
static be_ptr
get_Historically_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
                         hashPtr table, hash_ptr memoiz, 
                         const int time, const int k, const int l,
                         const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  nusmv_assert((node_get_type(ltl_wff) == OP_HISTORICAL) && (time < k+1));

  be_mgr = BeEnc_get_be_manager(be_enc);

  if ((time == 0) && (pastdepth == 0)) {
    /**f1(i)*/
    return get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                         time, k, l, pastdepth);   
  }
  else if ((pastdepth == 0) && (time > 0) && (time <= k)) {
    /**f(i):=f1(i)\wedge f(i-1)*/
    return Be_And(be_mgr,
                  get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                time, k, l, pastdepth),
                  get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                time-1, k, l, pastdepth));
  }
  else if ((pastdepth > 0) && (time > 1) && (time <= k)) {
    /**f(i,d):= f1(i,d)\wedge (el(i)->f(k,d-1),f(i-1,d))*/
    return Be_And(be_mgr,
                  get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                time, k, l, pastdepth),
                  Be_Ite(be_mgr,
                         get_el_at_time(be_enc, time-1, k),
                         get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                       k, k, l, pastdepth-1),
                         get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                       time-1, k, l, pastdepth)));
  }
  else if ((pastdepth > 0) && (time == 1)) {
    /**f(1,d):= f1(i,d)\wedge f(k,d-1)*/
    return Be_And(be_mgr,
                  get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                time, k, l, pastdepth),
                  get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                k, k, l, pastdepth-1));
  }
  error_unreachable_code();
  return 0; /**Should not be reached*/
}

/**Function********************************************************************

  Synopsis           [Genrates a boolean expression which is true iff the ltl
                      formula ltl_wff is true at time, handles the ONCE 
                      operator]

  Description        [The function recursively traverses the formula and genrates
                      the needed boolean expression.]

  SideEffects        []

  SeeAlso            [get_f_at_time, get_g_at_time]

******************************************************************************/
static be_ptr
get_Once_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
                 hashPtr table, hash_ptr memoiz, 
                 const int time, const int k, const int l,
                 const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  nusmv_assert((node_get_type(ltl_wff) == OP_ONCE) && (time < k+1));

  be_mgr = BeEnc_get_be_manager(be_enc);

  if ((time == 0) && (pastdepth == 0)) {
    /**f1(i)*/
    return get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                         time, k, l, pastdepth);
  }
  else if ((pastdepth == 0) && (time > 0) && (time <= k)) {
    /**f(i):=f1(i)\vee f(i-1)*/
    return Be_Or(be_mgr,
                 get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                               time, k, l, pastdepth),
                 get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                               time-1, k, l, pastdepth));
  }
  else if ((pastdepth > 0) && (time > 1) && (time <= k)) {
    /**f(i,d):= f1(i,d)\vee (el(i)->f(k,d-1),f(i-1,d))*/
    return Be_Or(be_mgr,
                 get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                               time, k, l, pastdepth),
                 Be_Ite(be_mgr,
                        get_el_at_time(be_enc, time-1, k),
                        get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                      k, k, l, pastdepth-1),
                        get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                                      time-1, k, l, pastdepth)));
  }
  else if ((pastdepth > 0) && (time == 1)) {
    /**f(1,d):= f1(1,d)\vee f(k,d-1)*/
    return Be_Or(be_mgr,
                 get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                               time, k, l, pastdepth),
                 get_f_at_time(be_enc, ltl_wff, table, memoiz, 
                               k, k, l, pastdepth-1));
  }
  error_unreachable_code();
  return 0; /**Should not be reached*/
}

static be_ptr
get_ZY_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
               hashPtr table, hash_ptr memoiz, 
               const int time, const int k, const int l,
               const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  nusmv_assert((node_get_type(ltl_wff) == OP_PREC) || 
               (node_get_type(ltl_wff) == OP_NOTPRECNOT));
  nusmv_assert(time < k+1);

  be_mgr = BeEnc_get_be_manager(be_enc);
  
  if ((time == 0) && (pastdepth == 0) && 
      (node_get_type(ltl_wff) == OP_NOTPRECNOT)) {
    return Be_Truth(be_mgr);
  }
  if ((time == 0) && (pastdepth == 0)) {
    return Be_Falsity(be_mgr);
  }
  else if ((pastdepth == 0) && (time > 0) && (time < k+1)) {
    /**f(i):=f1(i-1)*/
    return get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                         time-1, k, l, pastdepth);
  }
  else if ((pastdepth > 0) && (time > 1) && (time < k+1)) {
    /**f(i,d):=el(i)->f1(i-1,d), f(k,d-1)*/
    return Be_Ite(be_mgr,
           get_el_at_time(be_enc, time-1, k),
                  get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                k, k, l, pastdepth-1),
                  get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                time-1, k, l, pastdepth));
  }
  else if ((pastdepth > 0) && (time == 1)) {
    return get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                         k, k, l, pastdepth-1);
  }
  error_unreachable_code();
  return 0; /**Should not be reached*/
}

/**Function********************************************************************

  Synopsis           []

  Description        [Returns a pointer to the g_i(time) variable]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
get_g_at_time(const BeEnc_ptr be_enc, const node_ptr ltl_wff, 
              hashPtr table, hash_ptr memoiz, 
              const int time, const int k, const int l,
              const unsigned pastdepth)
{
  Be_Manager_ptr be_mgr;

  nusmv_assert((time < k+1) && (time>=0));
  nusmv_assert(pastdepth == bmc_past_depth(ltl_wff));

  int data = Bmc_Hash_find(table, ltl_wff);
  nusmv_assert(data != -1);

  be_mgr = BeEnc_get_be_manager(be_enc);

  if (data >= 0) { /**A TL subformula, check cache*/
    if (bmc_cache_fetch_g(ltl_wff, time, k, pastdepth, table) != 0) {
      return bmc_cache_fetch_g(ltl_wff, time, k, pastdepth, table);
    }
  }
  
  switch (node_get_type(ltl_wff)) {
  case TRUEEXP:
  case FALSEEXP:
  case DOT:
  case BIT:
  case ARRAY:
  case NOT:
  case AND:
  case OR:
  case IFF:
  case OP_NEXT:
    /**The next operator and the normal 
     * Boolean connectives do not use g-variables*/
    error_unreachable_code();
    break;
    
  case OP_GLOBAL:
    nusmv_assert(!opt_bmc_sbmc_il_opt(OptsHandler_get_instance()));
    if (time < k) {
      /*g(time):=f1(time)\wedge g(time+1)*/
      return bmc_cache_insert_g(ltl_wff, time, k, pastdepth, table,
                                Be_And(be_mgr,
                                       get_f_at_time(be_enc, car(ltl_wff),
                                                     table, memoiz, 
                                                     time, k, l, pastdepth),
                                       get_g_at_time(be_enc, ltl_wff, 
                                                     table, memoiz, 
                                                     time+1, k, l, pastdepth)));
    }
    /**time=k*/
    return bmc_cache_insert_g(ltl_wff, time, k, pastdepth, table,
                              get_f_at_time(be_enc, car(ltl_wff), 
                                            table, memoiz, 
                                            time, k, l, pastdepth));
   
  case OP_FUTURE: /* EVENTUALLY */
    nusmv_assert(!opt_bmc_sbmc_il_opt(OptsHandler_get_instance()));
    if (time < k) {
      /*g(time):=f1(time)\vee g(time+1))*/
      return bmc_cache_insert_g(ltl_wff, time, k, pastdepth, table,
                                Be_Or(be_mgr,
                                      get_f_at_time(be_enc, car(ltl_wff),
                                                    table, memoiz, 
                                                    time, k, l, pastdepth),
                                      get_g_at_time(be_enc, ltl_wff, 
                                                    table, memoiz, 
                                                    time+1, k, l, pastdepth)));
    }     
    /**time=k*/
    return bmc_cache_insert_g(ltl_wff, time, k, pastdepth, table,
                              get_f_at_time(be_enc, car(ltl_wff), table, memoiz, 
                                            time, k, l, pastdepth));
    
  case UNTIL:
    if (time < k) {
        /*g(time):=f2(time)\vee (f1(time)\wedge g(time+1))*/
      return bmc_cache_insert_g(ltl_wff, time, k, pastdepth, table,
                                Be_Or(be_mgr,
                                      get_f_at_time(be_enc, cdr(ltl_wff),
                                                    table, memoiz, 
                                                    time, k, l, pastdepth),
                                      Be_And(be_mgr,
                                             get_f_at_time(be_enc, car(ltl_wff),
                                                           table, memoiz, 
                                                           time, k, l, pastdepth),
                                             get_g_at_time(be_enc, ltl_wff, 
                                                           table, memoiz, 
                                                           time+1, k, l, 
                                                           pastdepth))));
    }     
    /**time=k*/
    return bmc_cache_insert_g(ltl_wff, time, k, pastdepth, table,
                              get_f_at_time(be_enc, cdr(ltl_wff), 
                                            table, memoiz, 
                                            time, k, l, pastdepth));
    
  case RELEASES:
    if (time < k) {
      /*g(time):=f2(time)\wedge (f1(time)\vee g(time+1))*/
      return bmc_cache_insert_g(ltl_wff, time, k, pastdepth, table,
                                Be_And(be_mgr,
                                       get_f_at_time(be_enc, cdr(ltl_wff),
                                                     table, memoiz, 
                                                     time, k, l, pastdepth),
                                       Be_Or(be_mgr,
                                             get_f_at_time(be_enc, car(ltl_wff),
                                                           table, memoiz, 
                                                           time, k, l, pastdepth),
                                             get_g_at_time(be_enc, ltl_wff, 
                                                           table, memoiz, 
                                                           time+1, k, l, 
                                                           pastdepth))));
    }
    /**time=k*/
    return bmc_cache_insert_g(ltl_wff, time, k, pastdepth, table,
                              get_f_at_time(be_enc, cdr(ltl_wff), 
                                            table, memoiz, 
                                            time, k, l, pastdepth));
  case IMPLIES:
    internal_error("'Implies' should had been nnf-ed away!\n");
  case ATOM:
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
  case EBF:
  case EBG:
  case ABF:
  case ABG:
    internal_error("g: Unexpected CTL operator, node type %d\n",
                   node_get_type(ltl_wff) );
  default:
    /* no other type are available here: */
    error_unreachable_code();
  }
  error_unreachable_code();
  return 0; /**Should never be reached*/
}

/**Function********************************************************************

  Synopsis           [Returns a pointer to the el(time) variable]

  Description        [The variables el(time) describe if the state s_time 
                      should be equivalent with s_k]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
get_el_at_time(const BeEnc_ptr be_enc, const int time, const int k)
{
  return BeEnc_name_to_timed(be_enc, sbmc_loop_var_name_get(), time);
}


/**Function********************************************************************

  Synopsis           [Creates an expression which allows at most one el_i to
                      be true]

  Description        []

  SideEffects        []

  SeeAlso            [get_el_at_time]

******************************************************************************/
static be_ptr
AtMostOnce(const BeEnc_ptr be_enc, const int k)
{  
  int i;
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  be_ptr smaller_exists_i = Be_Falsity(be_mgr);
  be_ptr at_most_once = Be_Truth(be_mgr);

  for (i = 1; i < k; i++) {
    /*smaller_exists_i:= smaller_exists_{i-1} \vee el_{i-1}*/
    smaller_exists_i = Be_Or(be_mgr, 
                             smaller_exists_i, 
                             get_el_at_time(be_enc, i-1, k));

    /*bad_index_i:=smaller_exists_i -> \neg el(time)*/
    be_ptr bad_index_i = Be_Implies(be_mgr,
                                    smaller_exists_i,
                                    Be_Not(be_mgr,
                                           get_el_at_time(be_enc, i, k)));
    /**\wedge_{i=1}^{k-1} SE_i -> \neg el_i*/
    at_most_once= Be_And(be_mgr, bad_index_i, at_most_once);
  }
  return at_most_once;
}

/**Function********************************************************************

  Synopsis           [Creates the expression: \wedge_{i=0}^{k-1} el_i =>
                      (s_i <=> s_k)]

  Description        [The expression when coupled with AtMostOnce forces
                      state i to be equivalent with state k, if el_i
                      is true.

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr Loop(const BeEnc_ptr be_enc, const int k)
{
  int i;
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  be_ptr loop_constraints = Be_Truth(be_mgr);

  /**set \wedge_{i=0}^{k-1} el(i) => Equiv(s_i, s_k)*/
  for (i = k; i--; ) {
    loop_constraints = Be_And(be_mgr, loop_constraints,
                              Be_Implies(be_mgr,
                                         get_el_at_time(be_enc, i, k),
                                         Bmc_SBMCTableau_GetLoopCondition(be_enc,
                                                                          k, i)));
  }
  return loop_constraints;
}

/**Function********************************************************************

  Synopsis           [Returns a pointer to the il(time) variable]

  Description        [The il(i) variable describes if the state 'i' is a 
                      a state of the loop.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
get_il_at_time(const BeEnc_ptr be_enc, const int time, const int k) 
{
  Be_Manager_ptr be_mgr;

  /**The variables are only allocated if the il-optimisation is active*/
  nusmv_assert(time<=k && time>0);

  be_mgr = BeEnc_get_be_manager(be_enc);

  /**check cache*/
  if (bmc_cache_fetch_il(time, k) != (be_ptr)NULL) {
    return bmc_cache_fetch_il(time, k);
  }

  if (time == 1) {
    return bmc_cache_insert_il(time, k, get_el_at_time(be_enc, 0, k));
  }

  return bmc_cache_insert_il(time, k,
                          Be_Or(be_mgr, 
                                get_il_at_time(be_enc, time-1, k),
                                get_el_at_time(be_enc, time-1, k)));
}

/**Function********************************************************************

  Synopsis           [Returns a pointer to the le variable]

  Description        [The le variable is true if a loop exists.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
get_loop_exists(const BeEnc_ptr be_enc, const int k)
{
/**The variables are only allocated if the il-optimisation is active*/
  return get_il_at_time(be_enc, k, k);  
}


/**Function********************************************************************

  Synopsis           [Returns an expression which initialises f(k+1) for 
                      an F or an GF formula when we use the il-optimisation.]

  Description       [Creates the expression f(k+1):=\vee_{i=1}^k il(i)\wedge
                     f1(i)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
bmc_tableauGetEventuallyIL_opt(const BeEnc_ptr be_enc, const node_ptr ltl_wff,
                               const int k, const int l,
                               const unsigned pastdepth, hashPtr table, 
                               hash_ptr memoiz)
{
  int i;
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  be_ptr tableau = Be_Falsity(be_mgr);

  nusmv_assert(pastdepth == bmc_past_depth(ltl_wff));
  
  if (Bmc_Utils_IsAllLoopbacks(l)) {
    /**set f(k):= \vee_{i=1}^k il(i) \wedge f1(i) */
    for (i = 1; i <= k; i++) {        
      tableau=Be_Or(be_mgr,
                    Be_And(be_mgr,
                           get_il_at_time(be_enc, i, k),                        
                           (node_get_type(ltl_wff) == OP_GLOBAL) ?
                           /**we're dealing with GF p, skip to f_p(i)*/
                           get_f_at_time(be_enc, car(car(ltl_wff)), 
                                         table, memoiz, 
                                         i, k, l, pastdepth) :
                           /**normal case: F p*/
                           get_f_at_time(be_enc, car(ltl_wff), 
                                         table, memoiz,
                                         i, k, l, pastdepth)),
                    tableau);
    }
  } 
  else if (Bmc_Utils_IsSingleLoopback(l)) {
    /**Set f(k):=\vee_{i=l}^k f1(i)*/
    for (i = l; i <= k; i++) {
      tableau=Be_Or(be_mgr,
                    (node_get_type(ltl_wff) == OP_GLOBAL) ?
                    /**we're dealing with GF p, skip to f_p(i)*/
                    get_f_at_time(be_enc, car(car(ltl_wff)), 
                                  table, memoiz, 
                                  i, k, l, pastdepth) :
                    /**normal case: G p*/
                    get_f_at_time(be_enc, car(ltl_wff), 
                                  table, memoiz, 
                                  i, k, l, pastdepth),
                    tableau);
    }
  }
  else { /**No loopback*/
    tableau=Be_Falsity(be_mgr);
  }
  return tableau;

}

/**Function********************************************************************

  Synopsis           [Returns an expression which initialises f(k+1) for 
                      a 'globally' or an FG formula when we use the il-optimisation.]

  Description        [Creates the expression f(k+1):=le\wedge \wedge_{i=1}^k \neg il(i)\vee f1(i)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
bmc_tableauGetGloballyIL_opt(const BeEnc_ptr be_enc, const node_ptr ltl_wff,
                             const int k, const int l,
                             const unsigned pastdepth, 
                             hashPtr table, hash_ptr memoiz)
{
  int i;
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  be_ptr tableau = Be_Truth(be_mgr);

  nusmv_assert(pastdepth == bmc_past_depth(ltl_wff));

  if (Bmc_Utils_IsAllLoopbacks(l)) {
    /**set f(k):= le\wedge \wedge_{i=1}^k \neg il(i) \vee f1(i) */
    for (i = 1; i <= k; i++) {        
      tableau = Be_And(be_mgr,
                       Be_Or(be_mgr,
                             Be_Not(be_mgr, 
                                    get_il_at_time(be_enc, i, k)),
                             (node_get_type(ltl_wff) == OP_FUTURE) ?
                             /**we're dealing with FG p, skip to f_p(i)*/
                             get_f_at_time(be_enc, car(car(ltl_wff)), 
                                           table, memoiz,  
                                           i, k, l, pastdepth) :
                             /**normal case G p*/
                             get_f_at_time(be_enc, car(ltl_wff), 
                                           table, memoiz, 
                                           i, k, l, pastdepth)),
                     tableau);
    }
    tableau = Be_And(be_mgr, tableau, get_loop_exists(be_enc, k));
  } 
  else if (Bmc_Utils_IsSingleLoopback(l)) {
    /**Set f(k):=\wedge_{i=l}^k f1(i)*/
    for (i = l; i <= k; i++) {
      tableau = Be_And(be_mgr,
                  (node_get_type(ltl_wff) == OP_FUTURE) ?
                  /**we're dealing with GF p, skip to f_p(i)*/
                  get_f_at_time(be_enc, car(car(ltl_wff)), 
                                table, memoiz, 
                                i, k, l, pastdepth) :
                  /**normal case: F p*/
                  get_f_at_time(be_enc, car(ltl_wff), 
                                table, memoiz, 
                                i, k, l, pastdepth),
                  tableau);        
    }
  }
  else { /**No loopback*/
    tableau=Be_Falsity(be_mgr);
  }
  return tableau;
}

/**Function********************************************************************

  Synopsis           [Initialises the chache used to store f_i(time) and g_(time) 
                      values.]

  Description        [The function allocates an array of size (k+1)*count for
                      both f and g. The array is used to cache values
                      of f_i(time) and g_i(time). Only values for
                      temporal formulas are stored.]

  SideEffects        []

  SeeAlso            [bmc_delete_cache]

******************************************************************************/
static void bmc_cache_init(const int count, const int k, const unsigned pastdepth)
{
  if (opt_bmc_sbmc_cache(OptsHandler_get_instance())) { 
    int i;
  
    nusmv_assert((count >= 0) && (k >= 0));

    bmc_cache_f_dim = (count * (k+1) * (pastdepth + 1));
    bmc_cache_f = ALLOC(be_ptr, bmc_cache_f_dim);
    bmc_cache_g_dim = (count * (k+1) * (pastdepth + 1));
    bmc_cache_g = ALLOC(be_ptr, bmc_cache_g_dim);

    nusmv_assert(bmc_cache_g_dim == bmc_cache_f_dim);

    for (i = 0; i < bmc_cache_f_dim; i++) {
      bmc_cache_f[i] = (be_ptr)NULL;
      bmc_cache_g[i] = (be_ptr)NULL;
    }

    if (opt_bmc_sbmc_il_opt(OptsHandler_get_instance())) {
      bmc_cache_il_dim = k;
      bmc_cache_il = ALLOC(be_ptr, k);

      for(i = 0; i < k; ++i) {
        bmc_cache_il[i] = (be_ptr)NULL;
      }
    }
  }
}

/**Function********************************************************************

  Synopsis           [Frees the arrays used by the cache]

  Description        []

  SideEffects        []

  SeeAlso            [bmc_init_cache]

******************************************************************************/
static void bmc_cache_delete()
{
  if (opt_bmc_sbmc_cache(OptsHandler_get_instance())) {
    FREE(bmc_cache_f);
    bmc_cache_f = (be_ptr *)NULL;
    bmc_cache_f_dim = -1;
    FREE(bmc_cache_g);
    bmc_cache_g_dim = -1;
    bmc_cache_g = (be_ptr *)NULL;
    if (opt_bmc_sbmc_il_opt(OptsHandler_get_instance())) {
      FREE(bmc_cache_il);
      bmc_cache_il = (be_ptr *)NULL;
      bmc_cache_il_dim = -1;
    }
  }
}

static be_ptr bmc_cache_fetch_f(const node_ptr ltl_wff, const int time,
                                const int k, const unsigned pastdepth,
                                hashPtr table)
{
  nusmv_assert((time < k+1) && (time >= 0) &&
               (pastdepth <= bmc_tab_past_depth));
  int data = Bmc_Hash_find(table, ltl_wff);
  /**It is an error if the formula is not in the table or ltl_wff is
     not a temporal operator*/
  nusmv_assert((data != -1) && (data != -2));
    
  if (bmc_cache_f != (be_ptr *)NULL) {
    int i = (pastdepth + (bmc_tab_past_depth + 1) * data) * (k + 1) + time;

    nusmv_assert(i < bmc_cache_f_dim);

    return bmc_cache_f[i];
  }
  return (be_ptr)NULL;
}

static be_ptr bmc_cache_fetch_g(const node_ptr ltl_wff, const int time,
                                const int k, const unsigned pastdepth,
                                hashPtr table) 
{
  nusmv_assert((time < k+1) && (time >= 0) &&
               (pastdepth <= bmc_tab_past_depth));
  int data = Bmc_Hash_find(table, ltl_wff);
  /**It is an error if the formula is not in the table or ltl_wff is
     not a temporal operator*/
  nusmv_assert((data != -1) && (data != -2));

  if (bmc_cache_g != (be_ptr *)NULL) {
    int i = (pastdepth + (bmc_tab_past_depth + 1) * data) * (k + 1) + time;

    nusmv_assert(i < bmc_cache_g_dim);

    return bmc_cache_g[i];
  }
  return (be_ptr)NULL;

}

static be_ptr bmc_cache_insert_f(const node_ptr ltl_wff, const int time,
                                 const int k, const unsigned pastdepth,
                                 hashPtr table, be_ptr result)

{
  nusmv_assert((time < k+1) && (time >= 0) &&
               (pastdepth <= bmc_tab_past_depth));
  int data = Bmc_Hash_find(table, ltl_wff);
  /**It is an error if the formula is not in the table or ltl_wff is
     not a temporal operator*/

  nusmv_assert((data != -1) && (data != -2));
  nusmv_assert(result != (be_ptr)NULL);

  if (bmc_cache_f != (be_ptr *)NULL) { 
    int i = (pastdepth + (bmc_tab_past_depth + 1) * data) * (k + 1) + time;

    nusmv_assert(i < bmc_cache_f_dim);

    bmc_cache_f[i] = result;
  }
  return result;
}

static be_ptr bmc_cache_insert_g(const node_ptr ltl_wff, const int time,
                                 const int k, const unsigned pastdepth,
                                 hashPtr table, be_ptr result)
{
  nusmv_assert((time < k+1) && (time >= 0) &&
               (pastdepth <= bmc_tab_past_depth));
  int data = Bmc_Hash_find(table, ltl_wff);
  /**It is an error if the formula is not in the table or ltl_wff is
     not a temporal operator*/
  nusmv_assert((data != -1) && (data != -2));

  nusmv_assert(result != (be_ptr)NULL);

  if (bmc_cache_g != (be_ptr *)NULL) {
    int i = (pastdepth + (bmc_tab_past_depth + 1) * data ) * (k + 1) + time;

    nusmv_assert(i < bmc_cache_g_dim);

    bmc_cache_g[i] = result;
  }
  return result;
}

static be_ptr 
bmc_cache_insert_il(const int time, const int k, be_ptr result)
{
  nusmv_assert((time >= 1) && (time <= k) && opt_bmc_sbmc_il_opt(OptsHandler_get_instance()));

  if (bmc_cache_il != (be_ptr *)NULL) {

    nusmv_assert((time - 1) < bmc_cache_il_dim);

    bmc_cache_il[time - 1] = result;
  }
  return result;
}

static be_ptr 
bmc_cache_fetch_il(const int time, const int k)
{
  nusmv_assert((time > 0) && (time <= k) && opt_bmc_sbmc_il_opt(OptsHandler_get_instance()));
  
  if (bmc_cache_il != (be_ptr *)NULL) {

    nusmv_assert((time - 1) < bmc_cache_il_dim);

    return bmc_cache_il[time - 1];
  }
  return (be_ptr)NULL;
}


/**Function********************************************************************

  Synopsis           [Computes the maximum nesting depth of past operators in PLTL formula]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static unsigned
bmc_past_depth(const node_ptr ltl_wff)
{
  switch(node_get_type(ltl_wff)) {
  case TRUEEXP:
  case FALSEEXP:
    /* A variable */
  case DOT:
  case BIT:
  case ARRAY:
    return 0;
  case NOT:
    nusmv_assert((node_get_type(car(ltl_wff)) == DOT) ||
                 (node_get_type(car(ltl_wff)) == BIT) || 
                 (node_get_type(car(ltl_wff)) == ARRAY));
    return 0;
  case AND:
  case OR:
  case IFF:
  case UNTIL:
  case RELEASES: {
    unsigned left = bmc_past_depth(car(ltl_wff));
    unsigned right = bmc_past_depth(cdr(ltl_wff));
    return (left > right)? left : right;
  }
    
  case SINCE:
  case TRIGGERED: {
    unsigned left = bmc_past_depth(car(ltl_wff));
    unsigned right = bmc_past_depth(cdr(ltl_wff));
    return (left > right) ? left + 1 : right + 1;
  } 
    
  case OP_NEXT:
  case OP_GLOBAL:
  case OP_FUTURE: 
    return bmc_past_depth(car(ltl_wff));
    
  case OP_PREC:       
  case OP_NOTPRECNOT: 
  case OP_ONCE:       
  case OP_HISTORICAL: 
    return bmc_past_depth(car(ltl_wff)) + 1;
    
  default:
    error_unreachable_code();
  }  
  return 0;
}

