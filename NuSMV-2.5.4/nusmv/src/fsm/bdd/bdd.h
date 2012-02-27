/**CHeaderFile*****************************************************************

  FileName    [bdd.h]

  PackageName [fsm.bdd]

  Synopsis    [Declares the public interface for the package fsm.bdd]

  Description []
  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.bdd'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst. 

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

#ifndef __FSM_BDD_BDD_H__
#define __FSM_BDD_BDD_H__

#include "utils/utils.h"  /* for EXTERN and ARGS */
#include "dd/dd.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define BDD_ELFWD_OPT_FORWARD_SEARCH 1
#define BDD_ELFWD_OPT_LTL_TABLEAU_FORWARD_SEARCH 2
#define BDD_ELFWD_OPT_USE_REACHABLE_STATES 4
#define BDD_ELFWD_OPT_COUNTER_EXAMPLES 8
#define BDD_ELFWD_OPT_ALL (BDD_ELFWD_OPT_FORWARD_SEARCH | \
                           BDD_ELFWD_OPT_LTL_TABLEAU_FORWARD_SEARCH | \
                           BDD_ELFWD_OPT_USE_REACHABLE_STATES | \
                           BDD_ELFWD_OPT_COUNTER_EXAMPLES)

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef bdd_ptr BddStates;
#define BDD_STATES(x) \
          ((BddStates) x)

typedef bdd_ptr BddInputs;
#define BDD_INPUTS(x) \
          ((BddInputs) x)

typedef bdd_ptr BddStatesInputs;
#define BDD_STATES_INPUTS(x) \
          ((BddStatesInputs) x)

typedef bdd_ptr BddStatesInputsNexts;
#define BDD_STATES_INPUTS_NEXTS(x) \
          ((BddStatesInputsNexts) x)

typedef bdd_ptr BddInvarStates;
#define BDD_INVAR_STATES(x) \
          ((BddInvarStates) x)

typedef bdd_ptr BddInvarInputs;
#define BDD_INVAR_INPUTS(x) \
          ((BddInvarInputs) x)

typedef struct BddELFwdSavedOptions_TAG* BddELFwdSavedOptions_ptr;

/**Type************************************************************************

  Synopsis     [Enumeration of algorithms for determining language
                emptiness of a Büchi fair transition system with BDDs]

  Description  [Currently only has backward and forward variants of
                Emerson-Lei.

                The ..._MIN/MAX_VALID values can be used to
                determine the least and greatest valid elements so as to
                eliminate some need for change when other algorithms are
                added. These values might need to be adapted below when new
                algorithms are added.]

  Notes        []

******************************************************************************/

enum BddOregJusticeEmptinessBddAlgorithmType_TAG {
  BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_INVALID = -1,
  BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD  =  0,
  BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD,

  BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_MIN_VALID =
    BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_BWD,
  BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_MAX_VALID =
    BDD_OREG_JUSTICE_EMPTINESS_BDD_ALGORITHM_EL_FWD
};
typedef enum BddOregJusticeEmptinessBddAlgorithmType_TAG
  BddOregJusticeEmptinessBddAlgorithmType;

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
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void Bdd_Init ARGS((void));
EXTERN void Bdd_End ARGS((void));

EXTERN BddOregJusticeEmptinessBddAlgorithmType
  Bdd_BddOregJusticeEmptinessBddAlgorithmType_from_string
  ARGS((const char* name));
EXTERN const char* Bdd_BddOregJusticeEmptinessBddAlgorithmType_to_string
  ARGS((const BddOregJusticeEmptinessBddAlgorithmType self));
EXTERN void
  Bdd_print_available_BddOregJusticeEmptinessBddAlgorithms ARGS((FILE *file));

EXTERN boolean Bdd_elfwd_check_options ARGS((unsigned int which_options,
                                             boolean on_fail_print));
EXTERN BddELFwdSavedOptions_ptr Bdd_elfwd_check_set_and_save_options
  ARGS((unsigned int which_options));
EXTERN void Bdd_elfwd_restore_options
  ARGS((unsigned int which_options, BddELFwdSavedOptions_ptr saved_options));

/**AutomaticEnd***************************************************************/


#endif /* __FSM_BDD_BDD_H__ */
