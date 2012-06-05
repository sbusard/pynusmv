/**CFile***********************************************************************

  FileName    [SatIncSolver_private.c]

  PackageName [SatIncSolver]

  Synopsis    [The private interface of class SatIncSolver]

  Description [Private definition to be used by derived classes]

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

  Revision    [$Id: SatIncSolver_private.h,v 1.1.2.2.2.2.6.1 2010-02-18 10:00:03 nusmv Exp $]

******************************************************************************/
#ifndef __SAT_INC_SOLVER_PRIVATE__H
#define __SAT_INC_SOLVER_PRIVATE__H

#include "SatIncSolver.h"
#include "SatSolver_private.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [SatIncSolver Class]

  Description [ This class defines a prototype for a generic SatIncSolver. This
  class is virtual and must be specialized. ]

  SeeAlso     []   
  
*******************************************************************************/
typedef struct SatIncSolver_TAG
{
  INHERITS_FROM(SatSolver);

  /* ---------------------------------------------------------------------- */ 
  /* Virtual Methods                                                        */
  /* ---------------------------------------------------------------------- */ 

  /* creates a new group */
  VIRTUAL SatSolverGroup 
  (*create_group) (const SatIncSolver_ptr self);

  /* destroys existing group */
  VIRTUAL void
  (*destroy_group) (const SatIncSolver_ptr self, SatSolverGroup group);
  /* moves formulas from 'group' into permanent one and destroys 'group' */
  VIRTUAL void
  (*move_to_permanent_and_destroy_group) (const SatIncSolver_ptr self,
                                          SatSolverGroup group);

  /* tries to solve formulas in the groups in the given list and the permanent
     group */
  VIRTUAL SatSolverResult
  (*solve_groups) (const SatIncSolver_ptr self, const Olist_ptr groups);

  /* tries to solve the formulas belonging to the solver except those 
     in the group from the given list. Permanent group should not be 
     in the list ever. */
  VIRTUAL SatSolverResult
  (*solve_without_groups) (const SatIncSolver_ptr self,
                           const Olist_ptr groups);
} SatIncSolver;

/**AutomaticStart*************************************************************/ 

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
void sat_inc_solver_init ARGS((SatIncSolver_ptr self, const char* name));
void sat_inc_solver_deinit ARGS((SatIncSolver_ptr self));

/* pure virtual functions */
SatSolverGroup 
sat_inc_solver_create_group ARGS((const SatIncSolver_ptr self));

void
sat_inc_solver_destroy_group ARGS( (const SatIncSolver_ptr self,
                                    SatSolverGroup group));

void
sat_inc_solver_move_to_permanent_and_destroy_group
                                 ARGS((const SatIncSolver_ptr self,
                                       SatSolverGroup group));
SatSolverResult
sat_inc_solver_solve_groups ARGS((const SatIncSolver_ptr self,
                                  const Olist_ptr groups));

SatSolverResult
sat_inc_solver_solve_without_groups ARGS((const SatIncSolver_ptr self,
                                          const Olist_ptr groups));
/**AutomaticEnd***************************************************************/

#endif /* __SAT_INC_SOLVER_PRIVATE__H */
