/**CFile*****************************************************************

  FileName    [compassPkg.c]

  PackageName [compass]

  Synopsis    []

  Description []

  SeeAlso     [compass.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compass'' package of NuSMV version 2. 
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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif 

#include "compass.h"
#include "compassInt.h"

#include "opt/opt.h"

#include <stdarg.h>
#include <stdio.h>

static char rcsid[] UTIL_UNUSED = "$Id: compassPkg.c,v 1.1.2.3 2008-12-02 15:20:09 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Initializes the addon]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Compass_init(void)
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Initializing the Compass package... \n");
  }
  Compass_init_cmd(); 
}


/**Function********************************************************************

  Synopsis           [Reinitializes the addon]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Compass_reset(void)
{
}


/**Function********************************************************************

  Synopsis           [Deinitializes the addon]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Compass_quit(void)
{
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Quitting the Compass package... \n");
  }
}


