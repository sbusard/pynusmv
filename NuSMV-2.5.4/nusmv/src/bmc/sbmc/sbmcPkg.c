/**CFile***********************************************************************

  FileName    [sbmcPkg.c]

  PackageName [bmc.sbmc]

  Synopsis    [Bmc.Pkg module]

  Description [This module contains all the bmc package handling functions]

  SeeAlso     []

  Author      [Timo Latvala, Marco Roveri]

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2. 
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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "sbmcCmd.h"
#include "sbmcUtils.h"

#include "bmc/bmcPkg.h"
#include "bmc/bmcInt.h" /* for 'options' */

#include "be/be.h"
#include "sat/sat.h"
#include "cmd/cmd.h"

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

  Synopsis           [Initializes the SBMC sub package]

  Description        [Initializes the SBMC sub package]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void SBmc_Init()
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "Initializing the SBMC package... \n");
  }
  sbmc_reset_unique_id();
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "Done \n");
  }    
}

/**Function********************************************************************

  Synopsis           [Frees all resources allocated for SBMC]

  Description        [Frees all resources allocated for the SBMC model manager]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void SBmc_Quit()
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "Resetting the SBMC module...\n");
  }
  sbmc_reset_unique_id();
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "Done \n");
  }    
}




/**Function********************************************************************

  Synopsis           [Adds all bmc-related commands to the interactive shell]

  Description        []

  SideEffects        []

  SeeAlso            [CInit_Init]

******************************************************************************/
void SBmc_AddCmd() 
{
  /* These commands are re-entrant wrt Ctrl+C */
  Cmd_CommandAdd("check_ltlspec_sbmc", Sbmc_CommandCheckLtlSpecSBmc, 0, true); 
  Cmd_CommandAdd("gen_ltlspec_sbmc", Sbmc_CommandGenLtlSpecSBmc, 0, true); 
  Cmd_CommandAdd("check_ltlspec_sbmc_inc", Sbmc_CommandLTLCheckZigzagInc, 
                 0, true); 
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

