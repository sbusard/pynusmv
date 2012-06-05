/**CFile***********************************************************************

  FileName    [wffPkg.c]

  PackageName [wff]

  Synopsis    [Initialization and deinitialization for package wff and
  subpackages]

  Description [Initialization and deinitialization for package wff and
  subpackages]

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``wff'' package of NuSMV version 2.
  Copyright (C) 2011 by FBK-irst.

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

#include "wff.h"
#include "w2w/w2wInt.h"


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

  Synopsis           [Initializes the wff package]

  Description        []

  SideEffects        []

  SeeAlso            [wff_pkg_quit]

******************************************************************************/
void wff_pkg_init()
{
  w2w_init_wff2nnf();
}


/**Function********************************************************************

  Synopsis           [Deinitializes the wff package]

  Description        []

  SideEffects        []

  SeeAlso            [wff_pkg_init]

******************************************************************************/
void wff_pkg_quit()
{
  w2w_quit_wff2nnf();
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

