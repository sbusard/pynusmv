/**CFile***********************************************************************

  FileName    [hrc.c]

  PackageName [hrc]

  Synopsis    [Package level routines for hrc package.]

  Description [This file contains the package level routines for the
  hrc package. Among these routines there are the init and quit
  routines that initializes/deinitializes the package global variable
  (mainHrcNode) and the package commands.]

  SeeAlso     [hrcCmd.c]

  Author      [Sergio Mover]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2. 
  Copyright (C) 2009 by FBK.

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

#include "hrcInt.h" 
#include "hrcCmd.h" 

static char rcsid[] UTIL_UNUSED = "$Id: hrc.c,v 1.1.2.3 2009-10-22 12:42:26 nusmv Exp $";

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

  Synopsis    [Initializes the hrc package.]

  Description [Initializes the hrc package. The initialization
  consists of the allocation of the mainHrcNode global variable and
  the initialization of the hrc package commands.]

  SideEffects []

  SeeAlso     [TracePkg_quit]

******************************************************************************/
void Hrc_init()
{
  nusmv_assert(HRC_NODE(NULL) == mainHrcNode);

  mainHrcNode = HrcNode_create();
  Hrc_init_cmd();
}

/**Function********************************************************************

  Synopsis    [Quits the hrc package.]

  Description [Quits the hrc package, freeing the global variable
  mainHrcNode and removing the hrc commands.]

  SideEffects []

  SeeAlso     [TracePkg_init]

******************************************************************************/
void Hrc_quit()
{
  if (HRC_NODE(NULL) != mainHrcNode) {
    HrcNode_destroy_recur(mainHrcNode);
    mainHrcNode = HRC_NODE(NULL);
  }

  Hrc_quit_cmd();
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/



