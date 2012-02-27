/**CHeaderFile*****************************************************************

  FileName    [SatIncSolver.h]

  PackageName [SatIncSolver]

  Synopsis    [The header file for the SatIncSolver class.]

  Description [An incremental SAT solver interface. 
  SatIncSolver inherits the SatSolver class]

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

  Revision    [$Id: SatIncSolver.h,v 1.1.2.2.2.2.6.1 2010-02-18 10:00:03 nusmv Exp $]

******************************************************************************/

#ifndef __SAT_SOLVER_SAT_INC_SOLVER__H
#define __SAT_SOLVER_SAT_INC_SOLVER__H

#include "SatSolver.h"
#include "utils/Olist.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct SatIncSolver_TAG* SatIncSolver_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define SAT_INC_SOLVER(x) \
         ((SatIncSolver_ptr) x)

#define SAT_INC_SOLVER_CHECK_INSTANCE(x) \
         (nusmv_assert(SAT_INC_SOLVER(x) != SAT_INC_SOLVER(NULL)))

/**AutomaticStart*************************************************************/
/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
/* SatIncSolver Destructors */
EXTERN void SatIncSolver_destroy ARGS((SatIncSolver_ptr self));

EXTERN VIRTUAL SatSolverGroup
SatIncSolver_create_group ARGS((const SatIncSolver_ptr self));

EXTERN VIRTUAL void
SatIncSolver_destroy_group ARGS((const SatIncSolver_ptr self,
                                 SatSolverGroup group));

EXTERN VIRTUAL void
SatIncSolver_move_to_permanent_and_destroy_group
                                       ARGS((const SatIncSolver_ptr self,
                                             SatSolverGroup group));

EXTERN VIRTUAL SatSolverResult
SatIncSolver_solve_groups ARGS((const SatIncSolver_ptr self,
                                const Olist_ptr groups));

EXTERN VIRTUAL SatSolverResult
SatIncSolver_solve_without_groups ARGS((const SatIncSolver_ptr self,
                                        const Olist_ptr groups));

/**AutomaticEnd***************************************************************/

#endif /* __SAT_SOLVER_SAT_INC_SOLVER__H */
