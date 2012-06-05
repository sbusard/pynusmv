/**CHeaderFile*****************************************************************

  FileName    [propPkg.h]

  PackageName [prop]

  Synopsis    [Prop package-level declarations]

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

  Revision    [$Id: prop.h,v 1.14.2.6.4.15.4.17 2010-01-11 15:07:54 nusmv Exp $]

******************************************************************************/

#ifndef __PROP_PKG_H__
#define __PROP_PKG_H__


#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "PropDb.h"
#include "utils/utils.h"
 

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

/* package handling */
EXTERN void PropPkg_init_cmd ARGS((void));
EXTERN void PropPkg_quit_cmd ARGS((void));

EXTERN void PropPkg_init ARGS((void));
EXTERN void PropPkg_quit ARGS((void));

EXTERN PropDb_ptr PropPkg_get_prop_database ARGS((void));
EXTERN void PropPkg_set_prop_database ARGS((PropDb_ptr db));


/**AutomaticEnd***************************************************************/

#endif /* __PROP_PKG_H__ */
