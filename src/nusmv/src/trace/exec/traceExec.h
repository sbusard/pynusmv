/**CHeaderFile*****************************************************************

  FileName    [traceExec.h]

  PackageName [trace.exec]

  Synopsis    [Header file for the traceExec module]

  Description []

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.exec'' package of NuSMV version 2.
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

  Revision    [$Id: traceExec.h,v 1.1.2.11 2009-12-15 13:56:50 nusmv Exp $]

******************************************************************************/

#ifndef __TRACE_EXECUTE_H
#define __TRACE_EXECUTE_H

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define BDD_EXEC_ENGINE "bdd"
#define SAT_EXEC_ENGINE "sat"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum************************************************************************

  Synopsis    [required]

  Description [optional]

  SeeAlso     [optional]

******************************************************************************/
typedef enum
{
  EXEC_NONE, /* undefined or unknown execution engine */
  EXEC_BDD,  /* BDD based re-execution engine */
  EXEC_SAT,  /* SAT based re-execution engine */

  EXEC_RESERVED,
} execEngine;


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define BDD_EXEC_ENGINE "bdd"
#define SAT_EXEC_ENGINE "sat"

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_EXECUTE_H  */
