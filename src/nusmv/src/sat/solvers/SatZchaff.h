/**CHeaderFile*****************************************************************

  FileName    [SatZchaff.h]

  PackageName [SatZchaff]

  Synopsis    [The header file for the SatZchaff class.]

  Description [Zchaff is an incremental SAT solver. 
  SatZchaff inherits the SatIncSolver (interface) class]

  SeeAlso     []

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``sat'' package of NuSMV version 2. 
  Copyright (C) 2004 by FBK-irst.

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

  Revision    [$Id: SatZchaff.h,v 1.1.2.2.2.2.6.1 2010-02-18 10:00:03 nusmv Exp $]

******************************************************************************/

#ifndef __SAT_SOLVER_SAT_ZCHAFF__H
#define __SAT_SOLVER_SAT_ZCHAFF__H

#include "sat/SatIncSolver.h" 

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct SatZchaff_TAG* SatZchaff_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define SAT_ZCHAFF(x) \
         ((SatZchaff_ptr) x)

#define SAT_ZCHAFF_CHECK_INSTANCE(x) \
         (nusmv_assert(SAT_ZCHAFF(x) != SAT_ZCHAFF(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/* SatZchaff Constructor/Destructors */
EXTERN SatZchaff_ptr SatZchaff_create ARGS((const char* name));
EXTERN void SatZchaff_destroy ARGS((SatZchaff_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __SAT_SOLVER_SAT_ZCHAFF__H */
