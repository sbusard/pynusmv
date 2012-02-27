/**CFile***********************************************************************

  FileName    [propPkg.c]

  PackageName [prop]

  Synopsis    [Main routines for the prop package]

  Description []

  SeeAlso     []

  Author      [Marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``prop'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by FBK-irst. 

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

#include "Prop.h"
#include "PropDb.h"
#include "propInt.h" 


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
/* This variable is used to store various global FSMs */
static PropDb_ptr global_prop_database = PROP_DB(NULL);


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

  Synopsis           [Initializes the package: master property and 
  property database are allocated]

  Description        [After you had called this, you must also call 
  PropPkg_init_cmd if you need to use the interactive shell for 
  commands]

  SideEffects        []

******************************************************************************/
void PropPkg_init()
{
  nusmv_assert(PROP_DB(NULL) == global_prop_database);
  global_prop_database = PropDb_create();
}


/**Function********************************************************************

  Synopsis           [Quits the package]

  Description        []

  SideEffects        []

******************************************************************************/
void PropPkg_quit()
{
  PropDb_destroy(global_prop_database);
  global_prop_database = PROP_DB(NULL);
}


/**Function********************************************************************

  Synopsis           [Returns the global property database instance]

  Description        []

  SideEffects        []

******************************************************************************/
PropDb_ptr PropPkg_get_prop_database()
{
  return global_prop_database;
}


/**Function********************************************************************

  Synopsis           [Sets the global property database instance]

  Description        [Simply overwrites global_prop_database with a new
  value. Hence, caller is responsible for freeing
  global_prop_database before calling this function. ]

  SideEffects        []

******************************************************************************/
void PropPkg_set_prop_database(PropDb_ptr db)
{
  global_prop_database = db;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
