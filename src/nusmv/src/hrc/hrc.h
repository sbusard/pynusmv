/**CHeaderFile*****************************************************************

  FileName    [hrc.h]

  PackageName [hrc]

  Synopsis    [The package to manage the NuSMV module hierachy.]

  Description [The package to manage the NuSMV module hierachy.]

  SeeAlso     [optional]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2. 
  Copyright (C) 2009 by FBK-irst. 

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

  Revision    [$Id: hrc.h,v 1.1.2.9 2009-11-18 14:26:05 nusmv Exp $]

******************************************************************************/

#ifndef _hrc_h
#define _hrc_h

#include "HrcNode.h"
#include "hrc/dumpers/HrcDumper.h"

#include "set/set.h"
#include "utils/utils.h"

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

EXTERN void Hrc_init ARGS((void));
EXTERN void Hrc_quit ARGS((void));

EXTERN void Hrc_WriteModel ARGS((HrcNode_ptr self, 
                                 FILE * ofile,
                                 boolean append_suffix));

EXTERN void Hrc_DumpModel ARGS((HrcNode_ptr hrcNode, HrcDumper_ptr dumper));


/**AutomaticEnd***************************************************************/

#endif /* _hrc_h */
