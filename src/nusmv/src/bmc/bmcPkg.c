/**CFile***********************************************************************

  FileName    [bmcPkg.c]

  PackageName [bmc]

  Synopsis    [Bmc.Pkg module]

  Description [This module contains all the bmc package handling functions]

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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "bmcPkg.h"
#include "bmcInt.h" /* for 'options' */
#include "bmcCmd.h"

#include "bmc/sbmc/sbmcPkg.h"

#include "be/be.h"
#include "sat/sat.h"

#include "prop/Prop.h"
#include "cmd/cmd.h"
#include "enc/enc.h"
#include "fsm/be/BeFsm.h"

#ifdef BENCHMARKING
  #include <time.h>
  clock_t start_time;
#endif


static char rcsid[] UTIL_UNUSED = "$Id: bmcPkg.c,v 1.3.2.9.2.4.2.10.4.9 2010-02-12 17:14:49 nusmv Exp $";

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

  Synopsis           [Initializes the BMC structure]

  Description [It builds the vars manager, initializes the package and
  all sub packages, but only if not previously called.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Bmc_Init()
{
  BeEnc_ptr be_enc;

  /* does not initialize the package if previously initialized */
  if (cmp_struct_get_bmc_init(cmps)) return;

  #ifdef BENCHMARKING
    fprintf(nusmv_stdout,":START:benchmarking Bmc_Init\n");
    start_time = clock();
  #endif

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Initializing the BMC package... \n");
  }

  /* Initializes the be generic interface layer: */
  Be_Init();

  /* Initializes bmc internal data */
  Bmc_InitData();

  /* builds the variables manager: */
  Enc_init_be_encoding();
  be_enc = Enc_get_be_encoding();

  { /* commits all default layers */
    const char* name; int i;
    arrayForEachItem(const char*,
                     SymbTable_get_class_layer_names(
                         BaseEnc_get_symb_table(BASE_ENC(be_enc)), NULL),
                     i, name) {
      BaseEnc_commit_layer(BASE_ENC(be_enc), name);
    }
  }

  /* the determinization layer will be committed by the command
     bmc_setup (see bmc_build_master_be_fsm) */

#ifdef BENCHMARKING
  fprintf(nusmv_stdout,":UTIME = %.4f secs.\n",((double)(clock()-start_time))/CLOCKS_PER_SEC);
  fprintf(nusmv_stdout,":STOP:benchmarking Bmc_Init\n");
#endif

  /* Initialize the SBMC sub-package */
  SBmc_Init();

  cmp_struct_set_bmc_init(cmps);

  bmc_simulate_set_curr_sim_trace(TRACE(NULL), -1);
}


/**Function********************************************************************

  Synopsis           [Frees all resources allocated for the BMC model manager]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_Quit()
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Quitting the BMC package... \n");
  }

  bmc_simulate_set_curr_sim_trace(TRACE(NULL), -1);

  /* Shuts down the Be layer: */
  Be_Quit();

  /* shuts down bmc data */
  Bmc_QuitData();

  /* Quits the SBMC sub-package */
  SBmc_Quit();

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Done \n");
  }

  cmp_struct_unset_bmc_init(cmps);
}


/**Function********************************************************************

  Synopsis [Initializes the BMC internal structures, but not all
  dependencies. Call Bmc_Init to initialize everything it is is what
  you need instead.]

  Description []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Bmc_InitData()
{
  /* sets up Conv module: */
  Bmc_Conv_init_cache();
}


/**Function********************************************************************

  Synopsis [De0Initializes the BMC internal structures, but not all
  dependencies. Call Bmc_Quit to deinitialize everything it is is what
  you need instead.]

  Description []

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void Bmc_QuitData()
{
  /* Resets the _bmc_test_tableau command private data: */
  Bmc_TestReset();

  /* quits Conv module */
  Bmc_Conv_quit_cache();

  /* tableaus memoization */
  bmc_quit_tableau_memoization();
}


/**Function********************************************************************

  Synopsis           [Adds all bmc-related commands to the interactive shell]

  Description        []

  SideEffects        []

  SeeAlso            [CInit_Init]

******************************************************************************/
void Bmc_AddCmd()
{
  Cmd_CommandAdd("bmc_setup", Bmc_CommandBmcSetup, 0, false);
  Cmd_CommandAdd("bmc_simulate", Bmc_CommandBmcSimulate, 0, true);
  Cmd_CommandAdd("bmc_inc_simulate", Bmc_CommandBmcIncSimulate, 0, true);
  Cmd_CommandAdd("bmc_pick_state", Bmc_CommandBmcPickState, 0, true);
  Cmd_CommandAdd("bmc_simulate_check_feasible_constraints",
                 Bmc_CommandBmcSimulateCheckFeasibleConstraints, 0, true);

  Cmd_CommandAdd("gen_ltlspec_bmc",
                 Bmc_CommandGenLtlSpecBmc, 0, true);
  Cmd_CommandAdd("gen_ltlspec_bmc_onepb",
                 Bmc_CommandGenLtlSpecBmcOnePb, 0, true);
  Cmd_CommandAdd("check_ltlspec_bmc", Bmc_CommandCheckLtlSpecBmc, 0, true);
  Cmd_CommandAdd("check_ltlspec_bmc_onepb",
                 Bmc_CommandCheckLtlSpecBmcOnePb, 0, true);
#if NUSMV_HAVE_INCREMENTAL_SAT
  Cmd_CommandAdd("check_ltlspec_bmc_inc", Bmc_CommandCheckLtlSpecBmcInc,
                 0, true);
#endif

  Cmd_CommandAdd("gen_invar_bmc",     Bmc_CommandGenInvarBmc, 0, true);
  Cmd_CommandAdd("check_invar_bmc",   Bmc_CommandCheckInvarBmc, 0, true);
#if NUSMV_HAVE_INCREMENTAL_SAT
  Cmd_CommandAdd("check_invar_bmc_inc",   Bmc_CommandCheckInvarBmcInc,
                 0, true);
#endif
  Cmd_CommandAdd("_bmc_test_tableau", Bmc_TestTableau, 0, true);

  /* Add the SBMC sub-package commands */
  SBmc_AddCmd();
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

