/**CFile***********************************************************************

  FileName    [addonsCore.c]

  PackageName [addons_core]

  Synopsis    [Initialization/quit routines for addons_core sub-packages]

  Description [Initialization/quit routines for addons_core sub-packages]

  SeeAlso     [None]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``addons_core'' package of NuSMV version 2.
  Copyright (C) 2007 Fondazione Bruno Kessler.

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
#include "nusmv-config.h"
#endif

#include "addonsCore.h"

#if NUSMV_HAVE_COMPASS
#include "addons_core/compass/compass.h"
#endif

#include "opt/opt.h"


static char rcsid[] UTIL_UNUSED = "$Id: $";

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
extern FILE* nusmv_stderr;

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

  Synopsis           [Initialization of the AddonsCore Sub-Packages]

  Description        [Initialization of the AddonsCore Sub-Packages]

  SideEffects        [Sub-Packages are initialized with possible side
  effects on some global variables (e.g., shell commands)]

  SeeAlso            [optional]

******************************************************************************/
void AddonsCore_Init(void) 
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Initializing core addons...\n");
  }
#if NUSMV_HAVE_COMPASS
   Compass_init();
#endif
}

/**Function********************************************************************

  Synopsis           [Reset the Addons core Sub-Packages]

  Description        [Reset the Addons core Sub-Packages]

  SideEffects        [Reset all the structures used by the Addons core 
  Sub-Packages]

  SeeAlso            [optional]

******************************************************************************/
void AddonsCore_Reset(void)
{
  /*-------- DE-INITIALIZATION ------------ */
#if NUSMV_HAVE_COMPASS
   Compass_reset();
#endif
  /*-------- INITIALIZATION ------------ */
}

/**Function********************************************************************

  Synopsis           [Quit the Addons core Sub-Packages]

  Description        [Quit the Addons core Sub-Packages]

  SideEffects        [Quits all the structures used by the Addons core
  Sub-Packages]

  SeeAlso            [optional]

******************************************************************************/
void AddonsCore_Quit(void)
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 2)) {
    fprintf(nusmv_stderr, "Quitting core addons...\n");
  }
#if NUSMV_HAVE_COMPASS
   Compass_quit();
#endif
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



