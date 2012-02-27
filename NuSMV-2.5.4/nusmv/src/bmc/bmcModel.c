/**CFile***********************************************************************

  FileName    [bmcModel.c]

  PackageName [bmc]

  Synopsis    [Bmc.Model module]

  Description [This module contains all the model-related operations]

  SeeAlso     [bmcGen.c, bmcTableau.c, bmcConv.c, bmcVarMgr.c]

  Author      [Roberto Cavada]

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

#include "bmcModel.h"
#include "bmcUtils.h"


static char rcsid[] UTIL_UNUSED = "$Id: bmcModel.c,v 1.30.2.1.2.1.2.3.4.2 2010-01-29 15:22:32 nusmv Exp $";

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
bmc_model_getSingleFairness ARGS((const BeEnc_ptr be_enc,
                                  const be_ptr one_fairness,
                                  const int k, const int l));


static be_ptr
bmc_model_getFairness_aux ARGS((const BeEnc_ptr be_enc,
                                const node_ptr list,
                                const int k, const int l));


/**AutomaticEnd***************************************************************/



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Retrieves the init states from the given fsm, and
  compiles them into a BE at time 0]

  Description        [Use this function instead of explicitly get the init
  from the fsm and shift them at time 0 using the vars manager layer.]

  SideEffects        []

  SeeAlso            [Bmc_Model_GetInvarAtTime]

******************************************************************************/
be_ptr Bmc_Model_GetInit0(const BeFsm_ptr be_fsm)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  be_ptr init0 = BeEnc_untimed_expr_to_timed(be_enc,
                       Be_And(BeEnc_get_be_manager(be_enc), 
                              BeFsm_get_init(be_fsm),
                              BeFsm_get_invar(be_fsm)),
                                             0);
  return init0;
}

/**Function********************************************************************

  Synopsis           [Retrieves the init states from the given fsm, and
  compiles them into a BE at time i]

  Description        [Use this function instead of explicitly get the init
  from the fsm and shift them at time i using the vars manager layer.]

  SideEffects        []

  SeeAlso            [Bmc_Model_GetInvarAtTime]

******************************************************************************/
be_ptr Bmc_Model_GetInitI(const BeFsm_ptr be_fsm, const int i)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  be_ptr init = BeEnc_untimed_expr_to_timed(be_enc,
                      Be_And(BeEnc_get_be_manager(be_enc), 
                             BeFsm_get_init(be_fsm),
                             BeFsm_get_invar(be_fsm)),
                                            i);
  return init;
}

/**Function********************************************************************

  Synopsis           [Retrieves the invars from the given fsm, and
  compiles them into a BE at the given time]

  Description        [Use this function instead of explicitly get the invar
  from the fsm and shift them at the requested time using the vars
  manager layer.]

  SideEffects        []

  SeeAlso            [Bmc_Model_GetInit0]

******************************************************************************/
be_ptr Bmc_Model_GetInvarAtTime(const BeFsm_ptr be_fsm, const int time)
{
  return BeEnc_untimed_expr_to_timed(BeFsm_get_be_encoding(be_fsm),
                                     BeFsm_get_invar(be_fsm), time);
}


/**Function********************************************************************

  Synopsis           [Retrieves the trans from the given fsm, and compiles
                      it into a MSatEnc at the given time]

  Description        [Use this function instead of explicitly get the trans
                      from the fsm and shift it at the requested
                      time using the vars manager layer]

  SideEffects        [None]

******************************************************************************/
be_ptr Bmc_Model_GetTransAtTime(const BeFsm_ptr be_fsm, const int time)
{
  return BeEnc_untimed_expr_to_timed(BeFsm_get_be_encoding(be_fsm),
                                     BeFsm_get_trans(be_fsm), time);
}


/**Function********************************************************************

  Synopsis           [Unrolls the transition relation from j to k, taking
  into account of invars]

  Description        [Using of invars over next variables instead of the
  previuos variables is a specific implementation aspect]

  SideEffects        []

  SeeAlso            [Bmc_Model_GetPathWithInit, Bmc_Model_GetPathNoInit]

******************************************************************************/
be_ptr
Bmc_Model_GetUnrolling(const BeFsm_ptr be_fsm, const int j, const int k)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);

  be_ptr invar = BeFsm_get_invar(be_fsm);  
  be_ptr trans_invar_j = Be_And(be_mgr, BeFsm_get_trans(be_fsm), invar);
  be_ptr invar_next = BeEnc_shift_curr_to_next(be_enc, invar);
  be_ptr trans_invar = Be_And(be_mgr, trans_invar_j, invar_next);

  return BeEnc_untimed_to_timed_and_interval(be_enc, trans_invar, j, k - 1);
}


/**Function********************************************************************

  Synopsis           [Unrolls the transition relation from j to k, taking
                      into account of invars]

  Description        [Using of invars over previous variables instead of the
                      next variables is a specific implementation aspect]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr
Bmc_Model_Invar_Dual_forward_unrolling(const BeFsm_ptr be_fsm,
                                       const be_ptr invarspec, int i)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);

  be_ptr res = (be_ptr)(NULL);
  nusmv_assert(0 <= i);

  /* Invar[0] */
  if (0 == i) {
    res = Bmc_Model_GetInvarAtTime(be_fsm, 0);
  }

  else {
    /* Trans[i-1] & Invar[i] & Property[i-1] */
    res = Be_And(be_mgr,
                 Be_And(be_mgr, Bmc_Model_GetTransAtTime(be_fsm, i-1),
                        Bmc_Model_GetInvarAtTime(be_fsm, i)),
                 BeEnc_untimed_expr_to_timed(be_enc, invarspec, i-1));
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the path for the model from 0 to k,
  taking into account the invariants (and no init)]

  Description        []

  SideEffects        []

  SeeAlso            [Bmc_Model_GetPathWithInit]

******************************************************************************/
be_ptr Bmc_Model_GetPathNoInit(const BeFsm_ptr be_fsm, const int k)
{
  return Bmc_Model_GetUnrolling(be_fsm, 0, k);
}


/**Function********************************************************************

  Synopsis           [Returns the path for the model from 0 to k,
  taking into account initial conditions and invariants]

  Description        []

  SideEffects        []

  SeeAlso            [Bmc_Model_GetPathNoInit]

******************************************************************************/
be_ptr Bmc_Model_GetPathWithInit(const BeFsm_ptr be_fsm, const int k)
{
  return Be_And(BeEnc_get_be_manager(BeFsm_get_be_encoding(be_fsm)),
                Bmc_Model_GetPathNoInit(be_fsm, k),
                Bmc_Model_GetInit0(be_fsm));
}


/**Function********************************************************************

  Synopsis           [Generates and returns an expression representing
  all fairnesses in a conjunctioned form]

  Description        [Uses bmc_model_getFairness_aux which recursively calls
  itself to conjuctive all fairnesses by constructing a top-level 'and'
  operation.
  Moreover bmc_model_getFairness_aux calls the recursive function
  bmc_model_getSingleFairness, which resolves a single fairness as
  a disjunctioned expression in which each ORed element is a shifting of
  the single fairness across \[l, k\] if a loop exists.
  If no loop exists, nothing can be issued, so a falsity value is returned]

  SideEffects        []

  SeeAlso            [bmc_model_getFairness_aux, bmc_model_getSingleFairness]

******************************************************************************/
be_ptr Bmc_Model_GetFairness(const BeFsm_ptr be_fsm, 
                             const int k, const int l)
{
  node_ptr list = BeFsm_get_fairness_list(be_fsm);
  return bmc_model_getFairness_aux(BeFsm_get_be_encoding(be_fsm), list, k, l);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

static be_ptr bmc_model_getFairness_aux(const BeEnc_ptr be_enc,
                                        const node_ptr list,
                                        const int k, const int l)
{
  be_ptr res = NULL;
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);

  if (list == LS_NIL)  res = Be_Truth(be_mgr);
  else if (Bmc_Utils_IsNoLoopback(l))  res = Be_Falsity(be_mgr);
  else {
    be_ptr singleFairness =
      bmc_model_getSingleFairness(be_enc, (be_ptr) car(list), k, l);

    res = Be_And( be_mgr, singleFairness,
                  bmc_model_getFairness_aux(be_enc, cdr(list), k, l) );
  }

  return res;
}


static be_ptr
bmc_model_getSingleFairness(const BeEnc_ptr be_enc,
                            const be_ptr one_fairness, 
                            const int k, const int l)
{
  nusmv_assert(l < k);
  nusmv_assert(Bmc_Utils_IsSingleLoopback(l));

  return BeEnc_untimed_to_timed_or_interval(be_enc, one_fairness, l, k-1);
}


