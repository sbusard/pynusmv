/**CHeaderFile*****************************************************************

  FileName    [SatSolver.h]

  PackageName [SatSolver]

  Synopsis    [The header file for the SatSolver class.]

  Description [A non-incremental SAT solver interface]

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

  Revision    [$Id: SatSolver.h,v 1.1.2.2.2.3.4.5 2010-02-18 10:00:03 nusmv Exp $]

******************************************************************************/

#ifndef __SAT_SOLVER_SAT_SOLVER__H
#define __SAT_SOLVER_SAT_SOLVER__H

#include "be/be.h"
#include "utils/object.h"
#include "utils/list.h"
#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define Term void *
#define TermFactoryCallbacksUserData_ptr void *

/* a flag returned by the 'solve' methods */
typedef enum SatSolverResult_TAG
{ SAT_SOLVER_INTERNAL_ERROR=-1,
  SAT_SOLVER_TIMEOUT,
  SAT_SOLVER_MEMOUT,
  SAT_SOLVER_SATISFIABLE_PROBLEM,
  SAT_SOLVER_UNSATISFIABLE_PROBLEM,
  SAT_SOLVER_UNAVAILABLE
} SatSolverResult;

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct SatSolver_TAG* SatSolver_ptr;
typedef nusmv_ptrint SatSolverGroup;
typedef nusmv_ptrint SatSolverItpGroup;

typedef struct TermFactoryCallbacks_TAG {
  Term (*make_false)(TermFactoryCallbacksUserData_ptr user_data);
  Term (*make_true)(TermFactoryCallbacksUserData_ptr user_data);

  Term (*make_and)(Term t1, Term t2, TermFactoryCallbacksUserData_ptr user_data);
  Term (*make_or)(Term t1, Term t2, TermFactoryCallbacksUserData_ptr user_data);
  Term (*make_not)(Term t, TermFactoryCallbacksUserData_ptr user_data);

  Term (*make_var)(int var, TermFactoryCallbacksUserData_ptr user_data);
} TermFactoryCallbacks;

typedef TermFactoryCallbacks* TermFactoryCallbacks_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define SAT_SOLVER(x) \
         ((SatSolver_ptr) x)

#define SAT_SOLVER_CHECK_INSTANCE(x) \
         (nusmv_assert(SAT_SOLVER(x) != SAT_SOLVER(NULL)))

/**AutomaticStart*************************************************************/
/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/* SatSolver Destructors */
EXTERN void SatSolver_destroy ARGS((SatSolver_ptr self));

EXTERN SatSolverGroup
SatSolver_get_permanent_group ARGS((const SatSolver_ptr self));

EXTERN VIRTUAL void
SatSolver_add ARGS((const SatSolver_ptr self,
                    const Be_Cnf_ptr cnfProb,
                    SatSolverGroup group));

EXTERN VIRTUAL void
SatSolver_set_polarity ARGS((const SatSolver_ptr self,
                             const Be_Cnf_ptr cnfProb,
                             int polarity,
                             SatSolverGroup group));

EXTERN VIRTUAL void
SatSolver_set_preferred_variables ARGS((const SatSolver_ptr self,
                                        const Slist_ptr cnfVars));

EXTERN VIRTUAL Slist_ptr
SatSolver_get_conflicts ARGS((const SatSolver_ptr self));

EXTERN VIRTUAL void
SatSolver_clear_preferred_variables ARGS((const SatSolver_ptr self));

EXTERN VIRTUAL SatSolverResult
SatSolver_solve_all_groups ARGS((const SatSolver_ptr self));

EXTERN VIRTUAL SatSolverResult
SatSolver_solve_all_groups_assume ARGS((const SatSolver_ptr self, Slist_ptr assumptions));

EXTERN VIRTUAL Slist_ptr
SatSolver_get_model ARGS((const SatSolver_ptr self));

EXTERN VIRTUAL int
SatSolver_get_cnf_var ARGS((const SatSolver_ptr self, int var));

EXTERN VIRTUAL void
SatSolver_set_random_mode ARGS((SatSolver_ptr self, double seed));

EXTERN VIRTUAL void
SatSolver_set_polarity_mode ARGS((SatSolver_ptr self, int mode));

EXTERN VIRTUAL int
SatSolver_get_polarity_mode ARGS((const SatSolver_ptr self));

EXTERN const char*
SatSolver_get_name ARGS((const SatSolver_ptr self));

EXTERN long
SatSolver_get_last_solving_time ARGS((const SatSolver_ptr self));

EXTERN SatSolverItpGroup
SatSolver_curr_itp_group ARGS((const SatSolver_ptr self));

EXTERN SatSolverItpGroup
SatSolver_new_itp_group ARGS((const SatSolver_ptr self));

EXTERN Term
SatSolver_extract_interpolant ARGS((const SatSolver_ptr self, int nof_ga_groups,
                                    SatSolverItpGroup* ga_groups,
                                    TermFactoryCallbacks_ptr callbacks,
                                    TermFactoryCallbacksUserData_ptr user_data));

/**AutomaticEnd***************************************************************/

#endif /* __SAT_SOLVER_SAT_SOLVER__H  */
