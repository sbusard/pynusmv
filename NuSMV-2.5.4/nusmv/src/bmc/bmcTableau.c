/**CFile***********************************************************************

  FileName    [bmcTableau.c]

  PackageName [bmc]

  Synopsis    [Bmc.Tableau module]

  Description [This module contains all the tableau-related operations]

  SeeAlso     [bmcModel.c, bmcConv.c, bmcVarMgr.c bmcTableau.c,
               bmcTableauLTLformula.c,
               bmcTableauPLTLformula.c, bmcGen.c]

  Author      [Marco Benedetti, Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst and University of Trento.

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

#include "bmc.h"
#include "bmcInt.h"
#include "bmcTableau.h"
#include "bmcUtils.h"
#include "bmcModel.h"

#include "parser/symbols.h"
#include "utils/error.h"
#include "utils/assoc.h"

#include "wff/wff.h"
#include "wff/w2w/w2w.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcTableau.c,v 1.6.4.5.2.2.2.2.6.2 2008-03-03 17:44:51 nusmv Exp $";

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

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static be_ptr
Bmc_Tableau_GetLoopCondition ARGS((const BeEnc_ptr be_enc,
                                   const int k, const int l));

static be_ptr
Bmc_Tableau_GetAllLoopsDisjunction ARGS((const BeEnc_ptr be_enc, const int k));


/* Tableaux for LTL: */
static be_ptr
Bmc_TableauLTL_GetNoLoop ARGS((const BeFsm_ptr be_fsm,
                               const node_ptr ltl_wff, const int k));
static be_ptr
Bmc_TableauLTL_GetSingleLoop ARGS((const BeFsm_ptr be_fsm,
                                   const node_ptr ltl_wff,
                                   const int k, const int l));
static be_ptr
Bmc_TableauLTL_GetAllLoops ARGS((const BeFsm_ptr be_fsm,
                                 const node_ptr ltl_wff,
                                 const int k, const int l));
static be_ptr
Bmc_TableauLTL_GetAllLoopsDepth1 ARGS((const BeFsm_ptr be_fsm,
                                       const node_ptr ltl_wff, const int k));


/* Tableaux for PLTL: */
static be_ptr
Bmc_TableauPLTL_GetNoLoop ARGS((const BeFsm_ptr be_fsm,
                                const node_ptr ltl_wff, const int k));
static be_ptr
Bmc_TableauPLTL_GetSingleLoop ARGS((const BeFsm_ptr be_fsm,
                                    const node_ptr ltl_wff,
				    const int k, const int l));
static be_ptr
Bmc_TableauPLTL_GetAllLoops ARGS((const BeFsm_ptr be_fsm,
                                  const node_ptr ltl_wff, 
				  const int k, const int l));
static be_ptr
Bmc_TableauPLTL_GetAllLoopsDepth1 ARGS((const BeFsm_ptr be_fsm,
                                        const node_ptr ltl_wff, const int k));

static boolean 
isPureFuture_aux ARGS((const node_ptr pltl_wff, hash_ptr memoiz));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Builds tableau without loop at time zero, taking into
  account of fairness]

  Description        [Fairness evaluate to true if there are not fairness
  in the model, otherwise them evaluate to false because of no loop]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Tableau_GetNoLoop(const BeFsm_ptr be_fsm,
			     const node_ptr ltl_wff, const int k)
{
  return (isPureFuture(ltl_wff) && !opt_bmc_force_pltl_tableau(OptsHandler_get_instance())) ?
        Bmc_TableauLTL_GetNoLoop (be_fsm, ltl_wff, k) :
        Bmc_TableauPLTL_GetNoLoop(be_fsm, ltl_wff, k) ;
}


/**Function********************************************************************

  Synopsis           [Builds tableau for all possible loops in \[l, k\], in
  the particular case in which depth is 1. This function takes into account
  of fairness]

  Description        [Builds the tableau in the case depth==1 as suggested
  by R. Sebastiani]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Tableau_GetSingleLoop (const BeFsm_ptr be_fsm,
				  const node_ptr ltl_wff, const int k, const int l)
{
  return (isPureFuture(ltl_wff) && !opt_bmc_force_pltl_tableau(OptsHandler_get_instance())) ?
        Bmc_TableauLTL_GetSingleLoop  (be_fsm,ltl_wff,k,l) :
        Bmc_TableauPLTL_GetSingleLoop (be_fsm,ltl_wff,k,l) ;
}


/**Function********************************************************************

  Synopsis           [Builds tableau for all possible loops in \[l, k\[,
  taking into account of fairness]

  Description        [Each tableau takes into account of fairnesses relative
  to its step. All tableau are collected together into a disjunctive form.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Tableau_GetAllLoops(const BeFsm_ptr be_fsm,
			       const node_ptr ltl_wff, const int k, const int l)
{
  return (isPureFuture(ltl_wff) && !opt_bmc_force_pltl_tableau(OptsHandler_get_instance())) ?
        Bmc_TableauLTL_GetAllLoops (be_fsm, ltl_wff, k, l) :
        Bmc_TableauPLTL_GetAllLoops(be_fsm, ltl_wff, k, l) ;
}


/**Function********************************************************************

  Synopsis           [Builds tableau for all possible loops in \[l, k\], in
  the particular case in which depth is 1. This function takes into account
  of fairness]

  Description        [Builds the tableau in the case depth==1 as suggested
  by R. Sebastiani]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Tableau_GetAllLoopsDepth1(const BeFsm_ptr be_fsm,
				     const node_ptr ltl_wff, const int k)
{
  return (isPureFuture(ltl_wff) && !opt_bmc_force_pltl_tableau(OptsHandler_get_instance())) ?
        Bmc_TableauLTL_GetAllLoopsDepth1 (be_fsm, ltl_wff, k) :
        Bmc_TableauPLTL_GetAllLoopsDepth1(be_fsm, ltl_wff, k) ;
}

/**Function********************************************************************

  Synopsis           [Builds a tableau for the LTL at length k with loopback l
  (single loop, no loop and all loopbacks are allowed)]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Tableau_GetLtlTableau(const BeFsm_ptr be_fsm, 
                                 const node_ptr ltl_wff,
                                 const int k, const int l)
{
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(BeFsm_get_be_encoding(be_fsm));
  be_ptr res = NULL;

  if (Bmc_Utils_IsAllLoopbacks(l)) {
    /* Generates the problem with all possible loopbacks: */
    be_ptr tableau_noloop = Bmc_Tableau_GetNoLoop(be_fsm, ltl_wff, k);
    be_ptr tableau_loops = NULL;
    
    if (k==0) {
      tableau_loops = Be_Falsity(be_mgr);
    }
    else if (opt_bmc_optimized_tableau(OptsHandler_get_instance())) {
      /* use depth1 optimization: */
      node_ptr fairness = BeFsm_get_fairness_list(be_fsm);
      int depth =  Wff_get_depth(ltl_wff);
      
      if ( (fairness == NULL) && (depth == 0) ) {
        tableau_loops = Be_Falsity(be_mgr);
      }
      else if ( (fairness == NULL) && (depth == 1) ) {
        tableau_loops = Bmc_Tableau_GetAllLoopsDepth1(be_fsm, ltl_wff, k);
      }
      else {
        tableau_loops = Bmc_Tableau_GetAllLoops(be_fsm, ltl_wff, k, 0);
      }
    }
    else {
      /* do not use depth1 optimization: */
      tableau_loops = Bmc_Tableau_GetAllLoops(be_fsm, ltl_wff, k, 0);
    }
    nusmv_assert(tableau_loops != NULL);
    
    res = Be_Or(be_mgr, tableau_noloop, tableau_loops);
  }
  else if (Bmc_Utils_IsNoLoopback(l)) {
    /* Generates the problem with no loopback: */
    res = Bmc_Tableau_GetNoLoop(be_fsm, ltl_wff, k);
  }
  else {
    /* one loopback: */
    nusmv_assert(k>0);
    nusmv_assert(Bmc_Utils_IsSingleLoopback(l)); /* no other choices */
    
    res = Bmc_Tableau_GetSingleLoop(be_fsm, ltl_wff, k, l);
  }

  return res;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/* ====================================================================== */
/*                       Tableaux for LTL :                               */
/* ====================================================================== */

/**Function********************************************************************

  Synopsis           [Builds the tableau at time zero. Loop is allowed,
  fairness are taken into account]

  Description        []

  SideEffects        []

  SeeAlso            [BmcInt_Tableau_GetAtTime]

******************************************************************************/
static be_ptr
Bmc_TableauLTL_GetSingleLoopWithFairness(const BeFsm_ptr be_fsm,
                                         const node_ptr ltl_wff,
                                         const int k, const int l)
{
  be_ptr fairness = Bmc_Model_GetFairness(be_fsm, k, l);
  be_ptr tableau = BmcInt_Tableau_GetAtTime(BeFsm_get_be_encoding(be_fsm),
                                            ltl_wff, 0, k, l);
  return Be_And(BeEnc_get_be_manager(BeFsm_get_be_encoding(be_fsm)),
                tableau, fairness);
}


/**Function********************************************************************

  Synopsis           [Builds the tableau at time zero. Loop is allowed,
  fairness are taken into account]

  Description        []

  SideEffects        []

  SeeAlso            [BmcInt_Tableau_GetAtTime]

******************************************************************************/
static be_ptr
Bmc_TableauLTL_GetSingleLoop (const BeFsm_ptr be_fsm,
                              const node_ptr ltl_wff, const int k, const int l)
{
  be_ptr loopback = Bmc_Tableau_GetLoopCondition(BeFsm_get_be_encoding(be_fsm), 
                                                 k, l);

  be_ptr tableau  = Bmc_TableauLTL_GetSingleLoopWithFairness(be_fsm, ltl_wff, 
                                                             k, l);

  return Be_And(BeEnc_get_be_manager(BeFsm_get_be_encoding(be_fsm)), 
                loopback, tableau);
}


/**Function********************************************************************

  Synopsis           [Builds tableau without loop at time zero, taking into
  account of fairness]

  Description        [Fairness evaluate to true if there are not fairness
  in the model, otherwise them evaluate to false because of no loop]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
Bmc_TableauLTL_GetNoLoop(const BeFsm_ptr be_fsm,
                         const node_ptr ltl_wff, const int k)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  be_ptr fairness_k = Bmc_Model_GetFairness(be_fsm, k, 
                                            Bmc_Utils_GetNoLoopback());

  /* lazy evaluation: */
  if ( Be_IsFalse(BeEnc_get_be_manager(be_enc), fairness_k) ) {
    return fairness_k;
  }
  else {
    be_ptr tableau_k = BmcInt_Tableau_GetAtTime(be_enc, ltl_wff, 0, k,
                                                Bmc_Utils_GetNoLoopback());
    return Be_And(BeEnc_get_be_manager(be_enc), fairness_k, tableau_k);
  }
}


/**Function********************************************************************

  Synopsis           [Builds tableau for all possible loops in \[l, k\[,
  taking into account of fairness]

  Description        [Each tableau takes into account of fairnesses relative
  to its step. All tableau are collected together into a disjunctive form.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
Bmc_TableauLTL_GetAllLoops(const BeFsm_ptr be_fsm,
			   const node_ptr ltl_wff, const int k, const int l)
{
  BeEnc_ptr be_enc  = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  be_ptr result = Be_Falsity(be_mgr);
  int j;

  /* asserts on l, k compatibility */
  nusmv_assert(!Bmc_Utils_IsNoLoopback(l));
  nusmv_assert(l < k);

  for (j = l; j < k; ++j) {
    be_ptr tableau_k_j = 
      Bmc_TableauLTL_GetSingleLoopWithFairness(be_fsm, ltl_wff, k, j);
    be_ptr loop_k_j = Bmc_Tableau_GetLoopCondition(be_enc, k, j);

    /* tableau + loopback + fairness at step k with loop at j = k */
    be_ptr tableau_LP_k_j = Be_And(be_mgr, tableau_k_j, loop_k_j);
    
    /* Accumulates the result: */
    result = Be_Or(be_mgr, result, tableau_LP_k_j);

    if (Be_IsTrue(be_mgr, result))  break; /* laziness: */
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Builds tableau for all possible loops in \[l, k\], in
  the particular case in which depth is 1. This function takes into account
  of fairness]

  Description        [Builds the tableau in the case depth==1 as suggested
  by R. Sebastiani]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
Bmc_TableauLTL_GetAllLoopsDepth1(const BeFsm_ptr be_fsm,
                                 const node_ptr ltl_wff, const int k)
{
  be_ptr result=NULL;

  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);

  switch (node_get_type(ltl_wff)) {
  case OP_FUTURE:
  case UNTIL:
    result = Be_Falsity(BeEnc_get_be_manager(be_enc));
    break;

  case OP_NEXT:
    if (k==0) {
      result = 
        Be_And( BeEnc_get_be_manager(be_enc),
                Bmc_TableauLTL_GetSingleLoopWithFairness(be_fsm, 
                                                         car(ltl_wff), k, 0),
                Bmc_Tableau_GetLoopCondition(be_enc, k, 0) );
    }
    else result = Be_Falsity(BeEnc_get_be_manager(be_enc));
    break;

  case RELEASES:                        /* RELEASES     */
    result = Be_And( BeEnc_get_be_manager(be_enc),
                     Bmc_Tableau_GetAllLoopsDisjunction(be_enc, k),
                     bmc_tableauGetGloballyAtTime(be_enc,
                                                  cdr(ltl_wff), 0, k, 0) );
    break;

  case TRUEEXP:
  case FALSEEXP:
  case BIT:
  case DOT:
  case NOT:
  case ARRAY:
    internal_error("Unexpected formula with depth zero had been found.\n");

  default:
    result = Be_And( BeEnc_get_be_manager(be_enc),
                     Bmc_Tableau_GetAllLoopsDisjunction(be_enc, k),
                     BmcInt_Tableau_GetAtTime(be_enc, ltl_wff, 0, k, 0) );
  }

  nusmv_assert(result != NULL); /* result must exist */
  return result;
}



/* ====================================================================== */
/*                       Tableaux for PLTL :                              */
/* ====================================================================== */

/**Function********************************************************************

  Synopsis           [Returns the tableau for a PLTL formula on a bounded path
                      of length k, reasoning on fairness conditions as well.  ]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
Bmc_TableauPLTL_GetNoLoop(const BeFsm_ptr be_fsm,
                          const node_ptr ltl_wff, const int k)
{
 int noLoopBack = Bmc_Utils_GetNoLoopback();
 BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
 Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
 be_ptr fairness_k = Bmc_Model_GetFairness(be_fsm, k, noLoopBack);
 be_ptr tableau_k = Be_IsFalse(be_mgr, fairness_k) ?
   Be_Falsity(be_mgr) :
   Bmc_TableauPLTL_GetTableau(be_enc, ltl_wff, k, noLoopBack);

 return tableau_k;
}


/**Function********************************************************************

  Synopsis           [Returns the tableau for a PLTL formula on a (k,l)-loop,
                      conjuncted with both fairness conditions and the loop
                      condition on time steps k and l.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
Bmc_TableauPLTL_GetSingleLoop (const BeFsm_ptr be_fsm,
                               const node_ptr ltl_wff, 
			       const int k, const int l)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);

  be_ptr loopback = Bmc_Tableau_GetLoopCondition(be_enc, k, l);
  be_ptr fairness = Bmc_Model_GetFairness(be_fsm, k, l);
  be_ptr  tableau = Bmc_TableauPLTL_GetTableau(be_enc, ltl_wff, k, l);

  return Be_And(be_mgr, loopback, Be_And(be_mgr,fairness,tableau));
}


/**Function********************************************************************

  Synopsis           [Returns the conjunction of the single-loop tableaux for
                      all possible (k,l)-loops for a fixed k. Each single-loop
                      tableau takes into account of both fairness constraints
                      and loop condition.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
Bmc_TableauPLTL_GetAllLoops(const BeFsm_ptr be_fsm,
                            const node_ptr ltl_wff,
                            const int k, const int startFromL)
{
  BeEnc_ptr   be_enc = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  be_ptr result = Be_Falsity(be_mgr);
  int l;

  for (l=startFromL; l<k; l++) {
    be_ptr tableau_k_l = Bmc_TableauPLTL_GetSingleLoop(be_fsm, ltl_wff, k, l);

    if (!Be_IsFalse(be_mgr, tableau_k_l)) {
      result = Be_Or(be_mgr, result, tableau_k_l);
    }
  }

 return result;
}


/**Function********************************************************************

  Synopsis           [Builds tableau for all possible (k,l)-loops for a
                      fixed k, in the particular case depth==1.
                      This function takes into account of fairness.]

  Description        [Builds the tableau in the case depth==1 as suggested
                      by R. Sebastiani.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
Bmc_TableauPLTL_GetAllLoopsDepth1(const BeFsm_ptr be_fsm,
                                  const node_ptr pltl_wff, const int k)
{
   be_ptr result;
   BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
   Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
   int nodeType  = node_get_type(pltl_wff);

   switch (nodeType) {

   /* A formula "Fg" (or "fUg") with depth one is such that g is a purely
      propositional formula.
      As long as g is propositional, the whole formula is not satisfiable,
      provided it already failed to be satisfied in the "no-loop" case. In
      fact, g does not exhibits newer behaviours throught loops, so it
      generates equivalent (admittedly unsatisfiable) formulas on every
      loop path.                                                             */
   case    UNTIL:
   case OP_FUTURE:
     result = Be_Falsity(be_mgr);
     break;

   /* A formula "Xg" with depth one is such that g is a purely propositional
      formula. The standard one-loop function can be used to check the k=1
      case. Conversely, when k>1 we are ensured by the underlying BMC mechanism
      that the case k=1 has already been considered and that it generated an
      unsatisfiable formula. As a consequence of the propositional nature of g
      we can immediately conclude that every k-loop with k>1 generates an
      unsatisfiable formula as well. */
   case OP_NEXT:
     result = (k>1)? Be_Falsity(be_mgr) : 
       Bmc_TableauPLTL_GetSingleLoop(be_fsm,pltl_wff,k,0);
     break;

   /* A formula "fRg" with depth one is such that g is a purely propositional
      formula, which has to hold at the time instant when "fRg" is evaluated,
      due to the semantics of "fRg". So, "g" has to hold everywhere, and it
      suffices to check for the validity of this condition.                  */

   case RELEASES:
     result = 
       Be_And(be_mgr,
	      Bmc_Tableau_GetAllLoopsDisjunction(be_enc, k),
	      Bmc_TableauPLTL_GetAllTimeTableau(be_enc, cdr(pltl_wff), k));

   /* It can be proved that any depth-one PLTL formula is such that its
      tableau on (k-l)-loops does not depend on the value of l. So, the tableau
      itself is a common factor w.r.t. the OR-AND formula representing the set
      of all possible loopbacks, provided k is fixed. Thank to the distributive
      property of the logical AND w.r.t the OR, we are allowed to build the
      tableau once, and then put it together with the disjunction of all the
      loop conditions on k-loops.                                            */
   default:
     result = Be_And(be_mgr,
		     Bmc_Tableau_GetAllLoopsDisjunction(be_enc, k),
		     Bmc_TableauPLTL_GetTableau(be_enc,pltl_wff, k, 0));
   }
   nusmv_assert(result != NULL);

   return result;
}


/**Function********************************************************************

  Synopsis           [Builds a tableau that constraints state k to be equal to
                      state l. This is the condition for a path of length (k+1)
                      to represent a (k-l)loop (new semantics).]

  Description        [State l and state k are forced to represent the same
                      state by conjuncting the if-and-only-if conditions
                      {Vil<->Vik} between Vil (variable i at time l) and Vik
                      (variable i at time k) for each state variable Vi.
                      Note:frozen vars do not participate in this conjunct,
                      since they are implicitly keep their valus over all states]

  SideEffects        []

  SeeAlso            [Bmc_Tableau_GetAllLoopsDisjunction]

******************************************************************************/
static be_ptr
Bmc_Tableau_GetLoopCondition(const BeEnc_ptr be_enc, const int k, const int l)
{
  Be_Manager_ptr be_mgr; 
  be_ptr tableau_iff_constraints; 
  int iter; 

  nusmv_assert(l < k);

  be_mgr = BeEnc_get_be_manager(be_enc);
  tableau_iff_constraints = Be_Truth(BeEnc_get_be_manager(be_enc));

  iter = BeEnc_get_first_untimed_var_index(be_enc, BE_VAR_TYPE_CURR);
  while (BeEnc_is_var_index_valid(be_enc, iter)) {
    tableau_iff_constraints =
      Be_And(be_mgr, tableau_iff_constraints,
	     Be_Iff(be_mgr, 
		    BeEnc_index_to_timed(be_enc, iter, l), 
		    BeEnc_index_to_timed(be_enc, iter, k)));

    iter = BeEnc_get_next_var_index(be_enc, iter, BE_VAR_TYPE_CURR);
  }

 return tableau_iff_constraints;
}


/**Function********************************************************************

  Synopsis           [Builds the disjunction of all the loops conditions
                      for (k-l)-loops with l in \[0, k\[]

  Description        [Builds a formula which is a disjunction over all the
                      loop conditions on k-loops, with l=0,1,...,k-1.]

  SideEffects        []

  SeeAlso            [Bmc_Tableau_GetLoopCondition]

******************************************************************************/
static be_ptr
Bmc_Tableau_GetAllLoopsDisjunction(const BeEnc_ptr be_enc, const int k)
{
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  be_ptr result = Be_Falsity(be_mgr);
  int l;

  for (l=0; l<k; l++) {
    result = Be_Or(be_mgr,
		   Bmc_Tableau_GetLoopCondition(be_enc, k, l),
		   result);
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [ Checks wether a formula contains only future operators ]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
boolean isPureFuture(const node_ptr pltl_wff)
{
  boolean res;
  hash_ptr memoiz = new_assoc();
  nusmv_assert(memoiz != (hash_ptr) NULL);

  res = isPureFuture_aux(pltl_wff, memoiz);
  free_assoc(memoiz);
  return res;
}


/**Function********************************************************************

  Synopsis           [Memoized private service of isPureFuture]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
static boolean isPureFuture_aux(const node_ptr pltl_wff, hash_ptr memoiz)
{
  boolean res;
  int nodeType = node_get_type(pltl_wff);

  int mval = PTR_TO_INT(find_assoc(memoiz, pltl_wff));
  if (mval == 1) return false;
  else if (mval == 2) return true;

  /* not memoized */
  if (isVariable(nodeType) || isConstantExpr(nodeType))  return true;
  
  if (isPastOp(nodeType)) res = false;
  else if (isBinaryOp(nodeType)) {
    res = isPureFuture_aux(car(pltl_wff), memoiz) && 
      isPureFuture_aux(cdr(pltl_wff), memoiz);
  }
  else res = isPureFuture_aux(car(pltl_wff), memoiz);

  insert_assoc(memoiz, pltl_wff, PTR_FROM_INT(node_ptr, res ? 2:1));
  return res;
}
