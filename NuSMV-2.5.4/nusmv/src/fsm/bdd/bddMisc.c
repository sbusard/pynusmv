/**CFile***********************************************************************

  FileName    [bddMisc.c]

  PackageName [fsm.bdd]

  Synopsis    [Miscellanous routines not really specific to other files]

  Description [Miscellanous routines not really specific to other files:

               - Conversion between
                 BddOregJusticeEmptinessBddAlgorithmType and const
                 char*

               - Ensuring the preconditions for forward Emerson-Lei
                 algorithm are met

              ]

  Author      [Viktor Schuppan]

  Copyright   [
  This file is part of the ``fsm.bdd'' package of NuSMV version 2. 
  Copyright (C) 2008 by FBK-irst. 

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

#include "bdd.h"
#include "bddInt.h"

#include "opt/opt.h"
#include "compile/compile.h" /* for mainFlatHierarchy */

static char rcsid[] UTIL_UNUSED = "$Id: bddMisc.c,v 1.1.2.1 2009-05-06 09:18:14 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD_STRING \
  "EL_bwd"
#define BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD_STRING \
  "EL_fwd"

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis           [Holds the values of those options that might have been
                      overridden to allow execution of forward Emerson-Lei.]

  Description        []

  SideEffects        [n/a]

  SeeAlso            [Bdd_elfwd_check_set_and_save_options,
                      Bdd_elfwd_restore_options]

******************************************************************************/
typedef struct BddELFwdSavedOptions_TAG
{
  boolean forward_search;
  boolean ltl_tableau_forward_search;
  boolean use_reachable_states;
  boolean counter_examples;
} BddELFwdSavedOptions;

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [const char* to BddOregJusticeEmptinessBddAlgorithmType]

  Description        [Converts the given type from string "name" to a
                      BddOregJusticeEmptinessBddAlgorithmType object.]

  SideEffects        [None.]

  SeeAlso            [BddOregJusticeEmptinessBddAlgorithmType_to_string]

******************************************************************************/
BddOregJusticeEmptinessBddAlgorithmType \
  Bdd_BddOregJusticeEmptinessBddAlgorithmType_from_string(const char* name)
{
  BddOregJusticeEmptinessBddAlgorithmType res;

  if (strcmp(name,
             BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD_STRING) == 0) {
    res = BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD;
  }
  else if (strcmp(name,
                  BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD_STRING) == 0) {
    res = BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD;
  }
  else res = BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_INVALID;

  return res;
}

/**Function********************************************************************

  Synopsis           [BddOregJusticeEmptinessBddAlgorithmType to const char*]

  Description        [It takes BddOregJusticeEmptinessBddAlgorithmType of
                      self and returns a string specifying the type of it.
                      Returned string is statically allocated and must not be
                      freed.]

  SideEffects        [None.]

  SeeAlso            [Bdd_BddOregJusticeEmptinessBddAlgorithmType_from_string]

******************************************************************************/
const char* Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string
  (const BddOregJusticeEmptinessBddAlgorithmType self)
{
  switch (self) {
  case BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD:
    return BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD_STRING;
    break;
  case BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD:
    return BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD_STRING;
    break;
  default:
    return "Unknown";
    break;
  }
}

/**Function********************************************************************

  Synopsis           [Prints the BDD-based algorithms to check language
                      emptiness for omega-regular properties the system
                      currently supplies]

  Description        []

  SideEffects        [None.]

  SeeAlso            [BddOregJusticeEmptinessBddAlgorithmType,
                      Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string]

******************************************************************************/
void Bdd_print_available_BddOregJusticeEmptinessBddAlgorithms(FILE *file)
{
  BddOregJusticeEmptinessBddAlgorithmType alg;

  fprintf(file, "The available algorithms are: ");
  for (alg = BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_MIN_VALID;
       alg <= BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_MAX_VALID;
       alg++) {
    fprintf(file,
            "%s ",
            Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string(alg));
  }
  fprintf(file, "\n");
}

/**Function********************************************************************

  Synopsis           [Checks options for forward Emerson-Lei algorithm]

  Description        [Depending on the value of which_options, it checks that
                      forward search, ltl_tableau_forward_search, and
                      use_reachable_states are enabled and counter_examples is
                      disabled. Returns true if the checks are successful,
                      false otherwise. If on_fail_print is true, it prints an
                      error message on failure.]

  SideEffects        [None.]

  SeeAlso            []

******************************************************************************/
boolean Bdd_elfwd_check_options(unsigned int which_options,
                                boolean on_fail_print)
{
  boolean res=true;

  nusmv_assert(Nil == FlatHierarchy_get_compassion(mainFlatHierarchy));
  nusmv_assert(get_oreg_justice_emptiness_bdd_algorithm(OptsHandler_get_instance()) ==
               BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD);

  if ((which_options & BDD_ELFWD_OPT_FORWARD_SEARCH) &&
      !opt_forward_search(OptsHandler_get_instance())) {
    if (on_fail_print) {
      fprintf(nusmv_stderr,
              "Forward Emerson-Lei must be used with option forward_search "\
              "enabled.\n");
    }
    res=false;
  }
  if ((which_options & BDD_ELFWD_OPT_LTL_TABLEAU_FORWARD_SEARCH) &&
      !opt_ltl_tableau_forward_search(OptsHandler_get_instance())) {
    if (on_fail_print) {
      fprintf(nusmv_stderr,
              "Forward Emerson-Lei must be used with option "\
              "ltl_tableau_forward_search enabled.\n");
    }
    res=false;
  }
  if ((which_options & BDD_ELFWD_OPT_USE_REACHABLE_STATES) &&
      !opt_use_reachable_states(OptsHandler_get_instance())) {
    if (on_fail_print) {
      fprintf(nusmv_stderr,
              "Forward Emerson-Lei must be used with option "\
              "use_reachable_states enabled.\n");
    }
    res=false;
  }
  if ((which_options & BDD_ELFWD_OPT_COUNTER_EXAMPLES) &&
      opt_counter_examples(OptsHandler_get_instance())) {
    if (on_fail_print) {
      fprintf(nusmv_stderr,
              "Forward Emerson-Lei must be used with counterexamples "\
              "disabled (feature not implemented yet).\n");
    }
    res=false;
  }

  return res;
}

/**Function********************************************************************

  Synopsis           [Checks, sets and saves previous values of options for
                      forward Emerson-Lei]

  Description        [Which values are actually checked, set, and saved is
                      determined by the value of which_options. If set
                      in which_options, forward search,
                      ltl_tableau_forward_search, and
                      use_reachable_states are enabled and
                      counter_examples is disabled. Previous values
                      are stored and returned.

                      Creates the returned
                      BddELFwdSavedOptions_ptr. It does *not* belong
                      to caller - it will be destroyed by the
                      corresponding call to
                      Bdd_elfwd_restore_options.]

  SideEffects        [Modifies options.]

  SeeAlso            [Bdd_elfwd_restore_options]

******************************************************************************/
BddELFwdSavedOptions_ptr Bdd_elfwd_check_set_and_save_options
  (unsigned int which_options)
{
  BddELFwdSavedOptions_ptr saved_options;

  nusmv_assert(OptsHandler_get_instance());

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    if (!Bdd_elfwd_check_options(which_options, true)) {
      fprintf(nusmv_stderr,
              "Temporarily (un)setting options as required for forward"
              "Emerson-Lei.\n");
    }
  } else {
    (void) Bdd_elfwd_check_options(which_options, false);
  }

  saved_options = ALLOC(BddELFwdSavedOptions, 1);
  nusmv_assert(saved_options);

  if (which_options & BDD_ELFWD_OPT_FORWARD_SEARCH) {
    saved_options->forward_search = opt_forward_search(OptsHandler_get_instance());
    set_forward_search(OptsHandler_get_instance());
  }
  if (which_options & BDD_ELFWD_OPT_LTL_TABLEAU_FORWARD_SEARCH) {
    saved_options->ltl_tableau_forward_search =
      opt_ltl_tableau_forward_search(OptsHandler_get_instance());
    set_ltl_tableau_forward_search(OptsHandler_get_instance());
  }
  if (which_options & BDD_ELFWD_OPT_USE_REACHABLE_STATES) {
    saved_options->use_reachable_states = opt_use_reachable_states(OptsHandler_get_instance());
    set_use_reachable_states(OptsHandler_get_instance());
  }
  if (which_options & BDD_ELFWD_OPT_COUNTER_EXAMPLES) {
    saved_options->counter_examples = opt_counter_examples(OptsHandler_get_instance());
    unset_counter_examples(OptsHandler_get_instance());
  }

  return saved_options;
}

/**Function********************************************************************

  Synopsis           [Restores previous values of options for forward
                      Emerson-Lei]

  Description        [Which values are actually restored from saved_options is
                      determined by the value of which_options.]

  SideEffects        [Modifies options.]

  SeeAlso            [Bdd_elfwd_check_set_and_save_options]

******************************************************************************/
void Bdd_elfwd_restore_options(unsigned int which_options,
                               BddELFwdSavedOptions_ptr saved_options)
{
  nusmv_assert(saved_options);
  nusmv_assert(OptsHandler_get_instance());

  if (which_options & BDD_ELFWD_OPT_FORWARD_SEARCH) {
    if (saved_options->forward_search) set_forward_search(OptsHandler_get_instance());
    else unset_forward_search(OptsHandler_get_instance());
  }
  if (which_options & BDD_ELFWD_OPT_LTL_TABLEAU_FORWARD_SEARCH) {
    if (saved_options->ltl_tableau_forward_search) {
      set_ltl_tableau_forward_search(OptsHandler_get_instance());
    }
    else unset_ltl_tableau_forward_search(OptsHandler_get_instance());
  }
  if (which_options & BDD_ELFWD_OPT_USE_REACHABLE_STATES) {
    if (saved_options->use_reachable_states) set_use_reachable_states(OptsHandler_get_instance());
    else unset_use_reachable_states(OptsHandler_get_instance());
  }
  if (which_options & BDD_ELFWD_OPT_COUNTER_EXAMPLES) {
    if (saved_options->counter_examples) set_counter_examples(OptsHandler_get_instance());
    else unset_counter_examples(OptsHandler_get_instance());
  }

  FREE(saved_options);
}
