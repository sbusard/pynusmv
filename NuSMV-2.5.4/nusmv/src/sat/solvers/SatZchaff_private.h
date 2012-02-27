/**CFile***********************************************************************

  FileName    [SatZchaff_private.c]

  PackageName [SatZchaff]

  Synopsis    [The private interface of class SatZchaff]

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

  Revision    [$Id: SatZchaff_private.h,v 1.1.2.2.2.2.6.3 2010-02-18 10:00:03 nusmv Exp $]

******************************************************************************/
#ifndef __SAT_ZCHAFF_PRIVATE__H
#define __SAT_ZCHAFF_PRIVATE__H

#include "SatZchaff.h"
#include "satZChaffIfc.h"


#include "sat/SatIncSolver_private.h"
#include "utils/assoc.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [SatZchaff Class]

  Description [ This class defines a prototype for a generic SatZchaff. This
  class is virtual and must be specialized. ]

  SeeAlso     []

*******************************************************************************/
typedef struct SatZchaff_TAG
{
  INHERITS_FROM(SatIncSolver);

  SAT_Manager zchaffSolver; /* actual instance of zchaff */
  /* All input variables are represented by the  internal ones inside the 
     SatZchaff. Bellow two hash table perform the convertion in both ways */
  hash_ptr cnfVar2zchaffVar;/* converts CNF variable to internal variable */
  hash_ptr zchaffVar2cnfVar;/* converts internal variable into CNF variable */

  /* contains set of conflicting assumptions after using
     SatMinisat_solve_permanent_group_assume */
  Slist_ptr conflict;
} SatZchaff;

/**AutomaticStart*************************************************************/ 

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
void sat_zchaff_init ARGS((SatZchaff_ptr self, const char* name));
void sat_zchaff_deinit ARGS((SatZchaff_ptr self));

int sat_zchaff_cnfLiteral2zchaffLiteral ARGS((SatZchaff_ptr self,
                                              int cnfLitaral));
int sat_zchaff_zchaffLiteral2cnfLiteral ARGS((SatZchaff_ptr self,
                                              int zchaffLiteral));


/* virtual function from SatSolver */
void sat_zchaff_add ARGS((const SatSolver_ptr self,
                          const Be_Cnf_ptr cnfProb,
                          SatSolverGroup group));

void sat_zchaff_set_polarity ARGS((const SatSolver_ptr self,
                                   const Be_Cnf_ptr cnfProb,
                                   int polarity,
                                   SatSolverGroup group));

void sat_zchaff_set_preferred_variables ARGS((const SatSolver_ptr self,
                                              const Slist_ptr cnfVars));

void sat_zchaff_clear_preferred_variables ARGS((const SatSolver_ptr self));

SatSolverResult sat_zchaff_solve_all_groups ARGS((const SatSolver_ptr self));

Slist_ptr sat_zchaff_make_model ARGS((const SatSolver_ptr self));

/* virtual functions from SatIncSolver */
SatSolverGroup 
sat_zchaff_create_group ARGS((const SatIncSolver_ptr self));

void
sat_zchaff_destroy_group ARGS((const SatIncSolver_ptr self,
                               SatSolverGroup group));

void
sat_zchaff_move_to_permanent_and_destroy_group
                                ARGS((const SatIncSolver_ptr self,
                                      SatSolverGroup group));
SatSolverResult
sat_zchaff_solve_groups ARGS((const SatIncSolver_ptr self,
                              const Olist_ptr groups));

SatSolverResult
sat_zchaff_solve_without_groups ARGS((const SatIncSolver_ptr self,
                                      const Olist_ptr groups));

SatSolverResult sat_zchaff_solve_permanent_group_assume
ARGS((const SatSolver_ptr self, Slist_ptr assumption));

Slist_ptr sat_zchaff_get_conflicts ARGS((const SatSolver_ptr));

/**AutomaticEnd***************************************************************/

#endif /* __SAT_ZCHAFF_PRIVATE__H */
