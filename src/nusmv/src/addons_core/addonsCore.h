/**CHeaderFile*****************************************************************

  FileName    [addonsCore.h]

  PackageName [addons_core]

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

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

  Revision    []

******************************************************************************/

#ifndef __ADDONS_CORE_H__
#define __ADDONS_CORE_H__

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif 

#include "util.h"

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
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void AddonsCore_Init ARGS((void));
EXTERN void AddonsCore_Reset ARGS((void));
EXTERN void AddonsCore_Quit ARGS((void));

/**AutomaticEnd***************************************************************/

#endif /* __ADDONS_CORE_H__ */
