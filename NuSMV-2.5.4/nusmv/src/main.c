/**CFile***********************************************************************

  FileName    [smMain.c]

  PackageName [sm]

  Synopsis    [Main NuSMV routine. Parses command line at invocation of NuSMV.]

  Author      [Adapted to NuSMV by Marco Roveri]

  Copyright   [
  This file is part of the ``sm'' package of NuSMV version 2.
  Copyright (C) 1998-2001 by CMU and FBK-irst.

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

#include "cinit/cinit.h"
#include "cinit/cinitInt.h"
#include "addons_core/addonsCore.h"

/*---------------------------------------------------------------------------*/
/* Macro definitions                                                         */
/*---------------------------------------------------------------------------*/
#ifndef  NUSMV_PACKAGE_BUGREPORT
#define  NUSMV_PACKAGE_BUGREPORT "nusmv-users@fbk.eu"
#endif


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void main_init_custom_data ARGS((void));
static void main_init_custom_cmd_options ARGS((void));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int main(int  argc, char ** argv)
{
  int status;
  boolean requires_shutdown = true;
  FP_V_V iq_fns[][2] = {{AddonsCore_Init, AddonsCore_Quit}};

  /* Initializes data such as tool name, tool version, email.. */
  NuSMVCore_init_data();

  /* Customize the library data. */
  main_init_custom_data();

  /* Initializes all packages, having the list of init/quit mfunctions */
  NuSMVCore_init(iq_fns, sizeof(iq_fns)/sizeof(iq_fns[0]));

  /* Adds the command line options of NuSMV */
  NuSMVCore_init_cmd_options();

  /* Add [or remove] custom command line options */
  main_init_custom_cmd_options();

  /* Finally, call the main function */
  requires_shutdown = NuSMVCore_main(argc, argv, &status);

  if (requires_shutdown) {
    NuSMVCore_quit();
  }

  return status;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


static void main_init_custom_data(void)
{
  /* Empty stub */

  /* Example: */
  /* NuSMVCore_set_tool_name("esmc"); */
}

static void main_init_custom_cmd_options(void)
{
  /* Empty stub */

  /* Example: */
  /* NuSMVCore_add_env_command_line_option(...) */
}
