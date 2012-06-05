/**CFile***********************************************************************

  FileName    [bmcGen.c]

  PackageName [bmc]

  Synopsis    [Bmc.Gen module]

  Description [This module contains all the problems generation functions]

  SeeAlso     [bmcBmc.c, bmcTableau.c, bmcModel.c]

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

#include "bmcGen.h"
#include "bmcInt.h"
#include "bmcModel.h"
#include "bmcTableau.h"
#include "bmcUtils.h"
#include "bmcConv.h"

#include "wff/wff.h"
#include "wff/w2w/w2w.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcGen.c,v 1.3.4.1.2.1.2.2.6.4 2007-03-15 13:47:30 nusmv Exp $";

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

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Builds and returns the invariant problem of the
  given propositional formula]

  Description        [Builds the negation of
                     (I0 imp P0) and ((P0 and R01) imp P1)
                     that must be unsatisfiable.]

  SideEffects        []

  SeeAlso            [Bmc_Gen_InvarBaseStep, Bmc_Gen_InvarInductStep]

******************************************************************************/
be_ptr Bmc_Gen_InvarProblem(const BeFsm_ptr be_fsm, const node_ptr wff)
{
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(BeFsm_get_be_encoding(be_fsm));
  be_ptr base = Bmc_Gen_InvarBaseStep(be_fsm, wff);
  be_ptr induct = Bmc_Gen_InvarInductStep(be_fsm, wff);

  return Be_Not(be_mgr, Be_And(be_mgr, base, induct));
}



/**Function********************************************************************

  Synopsis           [Returns the LTL problem at length k with loopback l
  (single loop, no loop and all loopbacks are allowed)]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Gen_LtlProblem(const BeFsm_ptr be_fsm,
                          const node_ptr ltl_wff,
                          const int k, const int l)
{
  Be_Manager_ptr mgr = BeEnc_get_be_manager(BeFsm_get_be_encoding(be_fsm));
  be_ptr path_k = Bmc_Model_GetPathWithInit(be_fsm, k);
  be_ptr tableau = Bmc_Tableau_GetLtlTableau(be_fsm, ltl_wff, k, l);
  return Be_And(mgr, tableau, path_k);
}


/**Function********************************************************************

  Synopsis           [Returns the base step of the invariant construction]

  Description        [Returns I0 -> P0, where I0 is the init and
  invar at time 0, and P0 is the given formula at time 0]

  SideEffects        []

  SeeAlso            [Bmc_Gen_InvarInductStep]

******************************************************************************/
be_ptr Bmc_Gen_InvarBaseStep(const BeFsm_ptr be_fsm, const node_ptr wff)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);

  be_ptr P_0 = BeEnc_untimed_expr_to_timed(be_enc,
                                           Bmc_Conv_Bexp2Be(be_enc, wff), 0);

  return Be_Implies( be_mgr, Be_And(be_mgr,
                                    Bmc_Model_GetInit0(be_fsm),
                                    Bmc_Model_GetInvarAtTime(be_fsm, 0)),
                     P_0 );
}


/**Function********************************************************************

  Synopsis           [Returns the induction step of the invariant construction]

  Description        [Returns (P0 and R01) -> P1, where P0 is the formula
  at time 0, R01 is the transition (without init) from time 0 to 1,
  and P1 is the formula at time 1]

  SideEffects        []

  SeeAlso            [Bmc_Gen_InvarBaseStep]

******************************************************************************/
be_ptr Bmc_Gen_InvarInductStep(const BeFsm_ptr be_fsm,
                               const node_ptr wff)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);

  be_ptr P = Bmc_Conv_Bexp2Be(be_enc, wff);

  be_ptr trans_01_invar_01 = Bmc_Model_GetPathNoInit(be_fsm, 1);

  be_ptr trans_01_invar_01_P0 =
    Be_And(be_mgr,
           trans_01_invar_01,
           BeEnc_untimed_expr_to_timed(be_enc, P, 0));

  return Be_Implies(be_mgr, trans_01_invar_01_P0,
                    BeEnc_untimed_expr_to_timed(be_enc, P, 1));
}


/**Function********************************************************************

  Synopsis           [Generates i-th fragment of BMC unrolling]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Gen_UnrollingFragment(BeFsm_ptr be_fsm, const int i)
{
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(be_enc);
  nusmv_assert(0<=i);

  /* Init[0] & Invar[0] */
  if (0 == i) {
    return Bmc_Model_GetInit0(be_fsm);
  }

  /* Invar[i-1] & Trans[i-1] & Invar[i] */
  return Be_And(be_mgr,
                Bmc_Model_GetInvarAtTime(be_fsm, i -1),
                Be_And(be_mgr,
                       Bmc_Model_GetTransAtTime(be_fsm, i-1),
                       Bmc_Model_GetInvarAtTime(be_fsm, i)));
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

