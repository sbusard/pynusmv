/**CFile***********************************************************************

  FileName    [bmcInt.c]

  PackageName [bmc]

  Synopsis    [Private interfaces implementation of package bmc]

  Description []

  SeeAlso     []

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

#include "bmc.h"
#include "bmcInt.h"
#include "utils/assoc.h"
#include "mc/mc.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcInt.c,v 1.4.4.1.2.1.2.2.6.4 2010-01-29 15:22:32 nusmv Exp $";

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

  Synopsis           []
  Description        []
  SideEffects        []
  SeeAlso            []

******************************************************************************/
be_ptr Bmc_GetTestTableau (const BeEnc_ptr be_enc,
                           const node_ptr ltl_wff,
                           const int k, const int l)
{
 return (isPureFuture(ltl_wff) && !opt_bmc_force_pltl_tableau(OptsHandler_get_instance())) ?
        BmcInt_Tableau_GetAtTime(be_enc,ltl_wff,0,k,l) :
        Bmc_TableauPLTL_GetTableau(be_enc,ltl_wff,k,l);
}


/**Function********************************************************************

   Synopsis           [ Rewrites an invariant specification containing input 
   variables or next with an observer state variable ]

   Description        [ Returns a rewrited property ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
Prop_ptr Bmc_rewrite_invar(const Prop_ptr prop,
                           const BddEnc_ptr bdd_enc,
                           SymbLayer_ptr layer) 
{
  SexpFsm_ptr sexpfsm;
  BoolSexpFsm_ptr boolsexpfsm;
  BeFsm_ptr new_fsm;
  BeEnc_ptr be_enc;
  Prop_ptr res;
  node_ptr monitor_var;
  BoolEnc_ptr bool_enc;

  /* Assert that this function will be useful */
  nusmv_assert(Prop_needs_rewriting(prop));

  /* get the needed encoders */
  be_enc = BeFsm_get_be_encoding(Prop_get_be_fsm(prop));
  bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(bdd_enc));

  /* Create the SexpFsm with the monitor variable and its transition
     relation */
  sexpfsm = Mc_rewrite_invar_get_sexp_fsm(prop, layer, &monitor_var);

  /* Commit the temporary layer on the encoders */
  BaseEnc_commit_layer(BASE_ENC(bool_enc), SymbLayer_get_name(layer));
  BaseEnc_commit_layer(BASE_ENC(be_enc), SymbLayer_get_name(layer));
  BaseEnc_commit_layer(BASE_ENC(bdd_enc), SymbLayer_get_name(layer));

  /* Booleanize the sexp fsm */
  /* Note: determinization variables goes in the new tableau layer */
  boolsexpfsm = BoolSexpFsm_create_from_scalar_fsm(sexpfsm, bdd_enc, layer);

  /* Create a new BeFsm from the built SexpFsm */
  new_fsm = BeFsm_create_from_sexp_fsm(be_enc, boolsexpfsm);
  BE_FSM_CHECK_INSTANCE(new_fsm);

  BeFsm_apply_synchronous_product(new_fsm, Prop_get_be_fsm(prop));

  /* Create a new InvarSpec property with the formula ''monitor_variable'' */
  res = Prop_create_partial(monitor_var, Prop_Invar);

  /* Set the built BddFsm in the property */
  Prop_set_be_fsm(res, new_fsm);

  /* Set the built BddFsm in the property */
  Prop_set_bool_sexp_fsm(res, Prop_get_bool_sexp_fsm(prop));

  /* Update the cone from the old property */
  Prop_set_cone(res, Set_AddMember(Prop_get_cone(prop), monitor_var));

  /* Destroy the previously created SexpFsm */
  SexpFsm_destroy(sexpfsm);

  /* Destroy the previously created BoolSexpFsm */
  BoolSexpFsm_destroy(boolsexpfsm);

  /* Destroy the previously created BeFsm */
  BeFsm_destroy(new_fsm);

  return res;
}


/**Function********************************************************************

   Synopsis           [ Crean up the memory after the rewritten property check ]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void Bmc_rewrite_cleanup(Prop_ptr rewritten_prop,
                         const BddEnc_ptr bdd_enc,
                         SymbLayer_ptr layer) {
  BeEnc_ptr be_enc;
  BoolEnc_ptr bool_enc;

  /* get the needed encoders */
  be_enc = BeFsm_get_be_encoding(Prop_get_be_fsm(rewritten_prop));
  bool_enc = BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(bdd_enc));

  /* Destroy the rewritten property */
  Prop_destroy(rewritten_prop);
  
  /* Clean the encoders */
  BaseEnc_remove_layer(BASE_ENC(bdd_enc), SymbLayer_get_name(layer));
  BaseEnc_remove_layer(BASE_ENC(be_enc), SymbLayer_get_name(layer));
  BaseEnc_remove_layer(BASE_ENC(bool_enc), SymbLayer_get_name(layer));
}
