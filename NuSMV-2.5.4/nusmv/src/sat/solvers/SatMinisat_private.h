/**CFile***********************************************************************

  FileName    [SatMinisat_private.c]

  PackageName [SatMinisat]

  Synopsis    [The private interface of class SatMinisat]

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

  Revision    [$Id: SatMinisat_private.h,v 1.1.2.2.2.2.6.5 2010-02-18 10:00:03 nusmv Exp $]

******************************************************************************/
#ifndef __SAT_MINISAT_PRIVATE__H
#define __SAT_MINISAT_PRIVATE__H

#include "SatMinisat.h"
#include "satMiniSatIfc.h"

#include "sat/SatIncSolver_private.h"
#include "utils/assoc.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [SatMinisat Class]

  Description [ This class defines a prototype for a generic SatMinisat. This
  class is virtual and must be specialized. ]

  SeeAlso     []

*******************************************************************************/
typedef struct SatMinisat_TAG
{
  INHERITS_FROM(SatIncSolver);

  MiniSat_ptr minisatSolver; /* actual instance of minisat */
  /* All input variables are represented by the  internal ones inside the 
     SatMinisat. Bellow two hash table perform the convertion in both ways */
  hash_ptr cnfVar2minisatVar;/* converts CNF variable to internal variable */
  hash_ptr minisatVar2cnfVar;/* converts internal variable into CNF variable */

  /* contains set of conflicting assumptions after using
     SatMinisat_solve_permanent_group_assume */
  Slist_ptr conflict;

  /* A clause and its current maximum length. This was added here in
     order to replace a statically allocated, fixed-size array and
     length indicator in sat_minisat_add. */
  int* minisatClause;

  unsigned int minisatClauseSize;
  unsigned int minisat_itp_group;
} SatMinisat;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
void sat_minisat_init ARGS((SatMinisat_ptr self, const char* name, 
                            boolean enable_proof_logging));
void sat_minisat_deinit ARGS((SatMinisat_ptr self));

int sat_minisat_cnfLiteral2minisatLiteral ARGS((SatMinisat_ptr self,
                                                int cnfLitaral));
int sat_minisat_minisatLiteral2cnfLiteral ARGS((SatMinisat_ptr self,
                                                int minisatLiteral));

/* virtual function from SatSolver */
void sat_minisat_add ARGS((const SatSolver_ptr self,
                           const Be_Cnf_ptr cnfProb,
                           SatSolverGroup group));

void sat_minisat_set_polarity ARGS((const SatSolver_ptr self,
                                    const Be_Cnf_ptr cnfProb,
                                    int polarity,
                                    SatSolverGroup group));

void sat_minisat_set_preferred_variables ARGS((const SatSolver_ptr self,
                                               const Slist_ptr cnfVars));

void sat_minisat_clear_preferred_variables ARGS((const SatSolver_ptr self));

SatSolverResult sat_minisat_solve_all_groups ARGS((const SatSolver_ptr self));

SatSolverResult sat_minisat_solve_permanent_group_assume ARGS((const SatSolver_ptr self, Slist_ptr assumption));

Slist_ptr sat_minisat_get_conflicts ARGS((const SatSolver_ptr));

Slist_ptr sat_minisat_make_model ARGS((const SatSolver_ptr self));

/* virtual functions from SatIncSolver */
SatSolverGroup 
sat_minisat_create_group ARGS((const SatIncSolver_ptr self));

void
sat_minisat_destroy_group ARGS((const SatIncSolver_ptr self,
                                SatSolverGroup group));

void
sat_minisat_move_to_permanent_and_destroy_group
                                ARGS((const SatIncSolver_ptr self,
                                      SatSolverGroup group));
SatSolverResult
sat_minisat_solve_groups ARGS((const SatIncSolver_ptr self,
                               const Olist_ptr groups));

SatSolverResult
sat_minisat_solve_without_groups ARGS((const SatIncSolver_ptr self,
                                       const Olist_ptr groups));

/* the assumptions/conflict interface of MiniSat */
Slist_ptr sat_minisat_make_conflicts ARGS((const SatMinisat_ptr self));

int* sat_minisat_get_minisatClause ARGS((const SatMinisat_ptr self));
int sat_minisat_get_minisatClauseSize ARGS((const SatMinisat_ptr self));
void sat_minisat_enlarge_minisatClause ARGS((const SatMinisat_ptr self,
                                             unsigned int minSize));

/* polarity mode */
void sat_minisat_set_random_mode ARGS((SatSolver_ptr self, double seed));
void sat_minisat_set_polarity_mode ARGS((SatSolver_ptr self, int mode));
int  sat_minisat_get_polarity_mode ARGS((const SatSolver_ptr self));



/**AutomaticEnd***************************************************************/

#endif /* __SAT_MINISAT_PRIVATE__H */
