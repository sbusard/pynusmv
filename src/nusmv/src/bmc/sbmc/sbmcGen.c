/**CFile***********************************************************************

  FileName    [sbmcGen.c]

  PackageName [bmc.sbmc]

  Synopsis    [Bmc.Gen module]

  Description [This module contains all the problems generation functions]

  SeeAlso     [bmcBmc.c, bmcTableau.c, bmcModel.c]

  Author      [Timo Latvala, Marco Roveri]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
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

#include "sbmcGen.h"
#include "sbmcTableau.h"

#include "bmc/bmcInt.h"
#include "bmc/bmcModel.h"
#include "bmc/bmcUtils.h"

static char rcsid[] UTIL_UNUSED = "$Id: sbmcGen.c,v 1.1.2.6.4.3 2007-05-14 16:05:42 nusmv Exp $";

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

  Synopsis           [Returns the LTL problem at length k with loopback l
  (single loop, no loop and all loopbacks are allowed)]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Bmc_Gen_SBMCProblem(const BeFsm_ptr be_fsm,
                          const node_ptr ltl_wff,
                          const int k, const int l)
{
  Be_Manager_ptr be_mgr = BeEnc_get_be_manager(BeFsm_get_be_encoding(be_fsm));
  be_ptr res = NULL;
  be_ptr path_k = Bmc_Model_GetPathWithInit(be_fsm, k);

  if (Bmc_Utils_IsAllLoopbacks(l)) {
    /* Generates the problem with all possible loopbacks: */
    be_ptr tableau_loops = NULL;
    
    tableau_loops = Bmc_SBMCTableau_GetAllLoops(be_fsm, ltl_wff, k, l);    
    res = Be_And( be_mgr, path_k, tableau_loops);
  }
  else if (Bmc_Utils_IsNoLoopback(l)) {
    /* Generates the problem with no loopback: */
    be_ptr tableau = Bmc_SBMCTableau_GetNoLoop(be_fsm, ltl_wff, k);
    res =  Be_And(be_mgr, path_k, tableau);
  }
  else {
    /* one loopback: */
    be_ptr tableau_loopback = NULL;
    
    nusmv_assert(Bmc_Utils_IsSingleLoopback(l)); /* no other choices */
    
    tableau_loopback = Bmc_SBMCTableau_GetSingleLoop(be_fsm, ltl_wff, k, l);
    res = Be_And(be_mgr, path_k, tableau_loopback);
  }

  return res;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

