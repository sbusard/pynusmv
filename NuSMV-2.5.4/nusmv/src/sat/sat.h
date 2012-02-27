/**CHeaderFile*****************************************************************

  FileName    [sat.h]

  PackageName [sat]

  Synopsis    [The public interface for the <tt>sat</tt> package]

  Description [This package contains the generic interface to access 
  to sat solvers. A set of specific Sat solvers implementation are internally 
  kept, and are not accessible]

  SeeAlso     []

  Author      [Andrei Tchaltsev, Roberto Cavada]

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

  Revision    [$Id: sat.h,v 1.3.4.1.2.3.2.2 2005-05-04 07:54:51 nusmv Exp $]

******************************************************************************/


#ifndef _SAT_H_
#define _SAT_H_

/* ====================================================================== */
#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "utils/utils.h"
#include "be/be.h"
#include "utils/list.h"
#include "node/node.h"

#include "SatSolver.h"
#include "SatIncSolver.h"
/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro********************************************************************

  Synopsis     [A flag indicating that there is at least one incremental
  SAT solver]

  Description   []

  SideEffects   []
  
  SeeAlso      []

******************************************************************************/
#ifdef NUSMV_HAVE_INCREMENTAL_SAT
#error /* macro NUSMV_HAVE_INCREMENTAL_SAT must not be defined at this point */
#endif

#if NUSMV_HAVE_SOLVER_MINISAT || NUSMV_HAVE_SOLVER_ZCHAFF
#define NUSMV_HAVE_INCREMENTAL_SAT 1
#else
#define NUSMV_HAVE_INCREMENTAL_SAT 0
#endif


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN SatSolver_ptr Sat_CreateNonIncSolver ARGS((const char* satSolver));
EXTERN SatSolver_ptr Sat_CreateNonIncProofSolver ARGS((const char* satSolver));
EXTERN SatIncSolver_ptr Sat_CreateIncSolver ARGS((const char* satSolver));
EXTERN SatIncSolver_ptr Sat_CreateIncProofSolver ARGS((const char* satSolver));

EXTERN const char* Sat_NormalizeSatSolverName ARGS((const char* solverName));
EXTERN void Sat_PrintAvailableSolvers ARGS((FILE* file));
EXTERN char* Sat_GetAvailableSolversString ARGS((void));


/* ====================================================================== */

#endif /* _SAT_H_ */
