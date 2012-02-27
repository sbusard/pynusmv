/**CHeaderFile*****************************************************************

  FileName    [hrcCmd.h]

  PackageName [hrc]

  Synopsis    [The header file for the shell interface of the hrc packace]

  Description []

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

  Revision    [$Id: hrcCmd.h,v 1.1.2.3 2009-09-24 14:19:41 nusmv Exp $]

******************************************************************************/

#ifndef _HRC_CMD_H
#define _HRC_CMD_H

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

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
EXTERN int CommandHrcWriteModel ARGS((int argc, char **argv));
EXTERN int CommandHrcDumpModel ARGS((int argc, char **argv));
EXTERN void Hrc_init_cmd ARGS((void));
EXTERN void Hrc_quit_cmd ARGS((void));


/**AutomaticEnd***************************************************************/

#endif /* _HRC_CMD_H */
