/**CFile***********************************************************************

  FileName    [mcLE.c]

  PackageName [mc]

  Synopsis    [Language Emptiness]

  Description [Check for language emptiness]

  SeeAlso     [optional]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``mc'' package of NuSMV version 2. 
  Copyright (C) 2006 FBK-irst. 

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


  All Rights Reserved. This software is for educational purposes only.
  Permission is given to academic institutions to use, copy, and modify
  this software and its documentation provided that this introductory
  message is not removed, that this software and its documentation is
  used for the institutions' internal research and educational purposes,
  and that no monies are exchanged. No guarantee is expressed or implied
  by the distribution of this code.
  Send bug-reports and/or questions to: nusmv-users@fbk.eu. ]

******************************************************************************/

#include "mc.h"
#include "mcInt.h" 

#include "utils/error.h"
#include "utils/utils_io.h"
#include "enc/enc.h"
#include "opt/opt.h"
#include "fsm/bdd/bdd.h" /* to check preconditions for EL_fwd */

#include <math.h>

static char rcsid[] UTIL_UNUSED = "$Id: mcLE.c,v 1.1.2.4 2009-05-06 09:46:49 nusmv Exp $";

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

static void mc_check_language_emptiness_el_bwd ARGS((const BddFsm_ptr fsm,
                                                     boolean allinit,
                                                     boolean verbose));
static void mc_check_language_emptiness_el_fwd ARGS((const BddFsm_ptr fsm,
                                                     boolean allinit,
                                                     boolean verbose));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Checks whether the language is empty]

  Description        [Checks whether the language is empty. Basically just a
  wrapper function that calls the language emptiness algorithm given
  by the value of the oreg_justice_emptiness_bdd_algorithm option.

  If <tt>allinit</tt> is <tt>true</tt> the check is performed by
  verifying whether all initial states are included in the set of fair
  states. If it is the case from all initial states there exists a
  fair path and thus the language is not empty. On the other hand, if
  <tt>allinit</tt> is false, the check is performed by verifying
  whether there exists at least one initial state that is also a fair
  state. In this case there is an initial state from which it starts a
  fair path and thus the lnaguage is not empty. <tt>allinit</tt> is
  not supported for forward Emerson-Lei.

  Depending on the global option use_reachable_states the set of fair
  states computed can be restricted to reachable states only. In this
  latter case the check can be further simplified. Forward Emerson-Lei
  requires forward_search and use_reachable_states to be enabled.
  
  If <tt>verbose</tt> is true, then some information on the set of
  initial states is printed out too. <tt> verbose</tt> is ignored for
  forward Emerson-Lei.  ]

  SideEffects        [None]

  SeeAlso            [mc_check_language_emptiness_el_bwd,
  mc_check_language_emptiness_el_fwd]

******************************************************************************/
void Mc_CheckLanguageEmptiness(const BddFsm_ptr fsm, boolean allinit, 
                               boolean verbose) 
{
  BddOregJusticeEmptinessBddAlgorithmType alg;
  BddELFwdSavedOptions_ptr elfwd_saved_options =
    (BddELFwdSavedOptions_ptr) NULL;

  alg = get_oreg_justice_emptiness_bdd_algorithm(OptsHandler_get_instance());

  if (alg == BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD) {
    elfwd_saved_options =
      Bdd_elfwd_check_set_and_save_options(BDD_ELFWD_OPT_FORWARD_SEARCH |
                                           BDD_ELFWD_OPT_USE_REACHABLE_STATES);
  }

  switch(alg) {
  case BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD:
    mc_check_language_emptiness_el_bwd(fsm, allinit, verbose);
    break;
  case BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD:
    mc_check_language_emptiness_el_fwd(fsm, allinit, verbose);
    break;
  default:
    error_unreachable_code();
    break;
  }

  if (elfwd_saved_options != (BddELFwdSavedOptions_ptr) NULL) {
    Bdd_elfwd_restore_options(BDD_ELFWD_OPT_FORWARD_SEARCH |
                              BDD_ELFWD_OPT_USE_REACHABLE_STATES,
                              elfwd_saved_options);
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Checks whether the language is empty using the backward
  Emerson-Lei algorithm]

  Description        [See Mc_CheckLanguageEmptiness.]

  SideEffects        []

  SeeAlso            [BddFsm_get_fair_states]

******************************************************************************/
static void mc_check_language_emptiness_el_bwd(const BddFsm_ptr fsm,
                                               boolean allinit, 
                                               boolean verbose) 
{
  bdd_ptr fair_states;
  bdd_ptr init, invar;
  BddEnc_ptr bddenc;
  DdManager * dd;

  /* Extracting the DD manager */
  bddenc = BddFsm_get_bdd_encoding(fsm);
  dd = BddEnc_get_dd_manager(bddenc);

  fair_states = BddFsm_get_fair_states(fsm);

  init = BddFsm_get_init(fsm);

  invar = BddFsm_get_state_constraints(fsm);

  /* We restrict the set of initial states and of fair states to the
     set of state constraints */
  bdd_and_accumulate(dd, &init, invar);

  bdd_and_accumulate(dd, &fair_states, invar);


  if (allinit) {
    if (0 == bdd_entailed(dd, init, fair_states)) {
      fprintf(nusmv_stdout, "The language is empty\n");
    }
    else {
      fprintf(nusmv_stdout, "The language is not empty\n");
      if (verbose) {
        fprintf(nusmv_stderr, "Mc_CheckLanguageEmptiness: verbose not yet implemented\n");
      }
    }
  }
  else {
    bdd_ptr fair_init;
    
    fair_init = bdd_and(dd, init, fair_states);

    if (bdd_is_false(dd, fair_init)) {
      fprintf(nusmv_stdout, "The language is empty\n");
    }
    else {
      fprintf(nusmv_stdout, "The language is not empty\n");

      if (verbose) {
        fprintf(nusmv_stderr, "Mc_CheckLanguageEmptiness: verbose not yet implemented\n");
      }
    }

    {/* prints the number of fair-init states */
      double reached_cardinality;
      double search_space_cardinality;
      bdd_ptr mask = BddEnc_get_state_frozen_vars_mask_bdd(bddenc);  
      
      bdd_and_accumulate(dd, &fair_init, mask);
      reached_cardinality = BddEnc_count_states_of_bdd(bddenc, fair_init);
      search_space_cardinality = BddEnc_count_states_of_bdd(bddenc, mask);
      bdd_free(dd, mask);
      fprintf(nusmv_stdout, "fair states: %g (2^%g) out of %g (2^%g)\n",
              reached_cardinality, log(reached_cardinality)/log(2.0),
              search_space_cardinality, log(search_space_cardinality)/log(2.0));
    }
    /* No longer needed */
    bdd_free(dd, fair_init);
  }

  /* No longer needed */
  bdd_free(dd, invar);
  bdd_free(dd, init);
  bdd_free(dd, fair_states);
}

/**Function********************************************************************

  Synopsis           [Checks whether the language is empty using the forward
  Emerson-Lei algorithm]

  Description        [See Mc_CheckLanguageEmptiness.]

  SideEffects        []

  SeeAlso            [BddFsm_get_revfair_states]

******************************************************************************/
static void mc_check_language_emptiness_el_fwd(const BddFsm_ptr fsm,
                                               boolean allinit, 
                                               boolean verbose) 
{
  BddEnc_ptr bddenc;
  DdManager * dd;
  bdd_ptr revfair_states;

  /* Preconditions */
  /* VS: In order to compute the value of allinit some additional work
     would have to be done - note that it is NOT sufficient to
     determine whether an initial state can reach a reverse fair
     state; rather one would really have to compute the set of
     (standard, not reverse) fair states. */
  nusmv_assert(!allinit);
  nusmv_assert(Bdd_elfwd_check_options(BDD_ELFWD_OPT_FORWARD_SEARCH |
                                       BDD_ELFWD_OPT_USE_REACHABLE_STATES,
                                       false));

  /* Extracting the DD manager */
  bddenc = BddFsm_get_bdd_encoding(fsm);
  dd = BddEnc_get_dd_manager(bddenc);

  revfair_states = BddFsm_get_revfair_states(fsm);

  if (bdd_is_false(dd, revfair_states)) {
    fprintf(nusmv_stdout, "The language is empty\n");
  }
  else {
    fprintf(nusmv_stdout, "The language is not empty\n");

    if (verbose) {
      fprintf(nusmv_stderr,
              "Mc_CheckLanguageEmptiness: verbose not yet implemented\n");
    }
  }

  /* No longer needed */
  bdd_free(dd, revfair_states);
}
