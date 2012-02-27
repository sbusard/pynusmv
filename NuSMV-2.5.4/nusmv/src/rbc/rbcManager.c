/**CFile***********************************************************************

  FileName    [rbcManager.c]

  PackageName [rbc]

  Synopsis    [RBC manager main routines.]

  Description [External procedures included in this module:
                <ul>
                <li> <b>Rbc_ManagerAlloc()</b>    allocates the manager;
                <li> <b>Rbc_ManagerReserve()</b>  makes room for more variables
                <li> <b>Rbc_ManagerCapacity()</b> returns available variables
                <li> <b>Rbc_ManagerFree()</b>     deallocates the manager;
                <li> <b>Rbc_ManagerGC()</b>       forces internal garbage coll.
                </ul>]

  SeeAlso     []

  Author      [Armando Tacchella]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by University of Genova.

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

******************************************************************************/

#include "rbc/rbcInt.h"

#include "utils/assoc.h"

static char rcsid[] UTIL_UNUSED = "$Id: rbcManager.c,v 1.4.6.1.2.2.2.1.6.4 2010-02-18 10:00:03 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Creates a new RBC manager.]

  Description [Creates a new RBC manager:
               <ul>
               <li> <i>varCapacity</i> how big is the variable index
                    (this number must be strictly greater than 0) 
               </ul>
               Returns the allocated manager if varCapacity is greater than 0,
               and NIL(Rbc_Manager_t) otherwise.]

  SideEffects [none]

  SeeAlso     [Rbc_ManagerFree]

******************************************************************************/
Rbc_Manager_t *
Rbc_ManagerAlloc(
  int varCapacity
)
{

  Rbc_Manager_t * rbcManager;
  int             i;

  /* Do not allow negative or null capacities. */
  if (varCapacity < 0) {
    return NIL(Rbc_Manager_t);
  }

  /* Allocate the manager. */
  rbcManager = ALLOC(Rbc_Manager_t, 1);
  nusmv_assert(rbcManager != (Rbc_Manager_t*) NULL);

  /* Allocate the vertex manager (dagManager) and 
     the variable index (varTable). */
  rbcManager -> dagManager = Dag_ManagerAlloc();
  rbcManager -> varTable = ALLOC(Rbc_t*, varCapacity);
  nusmv_assert(rbcManager->varTable != (Rbc_t**) NULL);
  rbcManager -> varCapacity = varCapacity;

  /* sets all things associated with CNF convertion */
  rbcManager -> rbcNode2cnfVar_model = new_assoc();
  rbcManager -> rbcNode2cnfVar_cnf = new_assoc();

  rbcManager -> cnfVar2rbcNode_model =  new_assoc();
  rbcManager -> cnfVar2rbcNode_cnf =  new_assoc();

  rbcManager -> maxUnchangedRbcVariable = 0;
  rbcManager -> maxCnfVariable = 0;

  /* Initialize varTable. */
  for (i = 0; i < varCapacity; i++) {
    rbcManager -> varTable[i] = NIL(Rbc_t);
  }

  /* Initialize statistics. */
  for (i = 0; i < RBCMAX_STAT; i++) {
    rbcManager -> stats[i] = 0;
  }

  /* Create 0 and 1 (permanent) logical constants. */
  rbcManager -> one = Dag_VertexInsert(rbcManager -> dagManager,
                                       RBCTOP,
                                       NIL(char),
                                       (Dag_Vertex_t**)NULL,
                                       0);
  Dag_VertexMark(rbcManager -> one);
  /* 0 is simply the complement of 1. */
  rbcManager -> zero = RbcId(rbcManager -> one, RBC_FALSE);

  return rbcManager;

} /* End of Rbc_ManagerAlloc. */


/**Function********************************************************************

  Synopsis    [Reserves more space for new variables.]

  Description [If the requested space is bigger than the current one
               makes room for more variables in the varTable.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
Rbc_ManagerReserve(
  Rbc_Manager_t * rbcManager,
  int             newVarCapacity
)
{

  int i;

  /* Do not allow shrinking the variables. */
  if (newVarCapacity <= rbcManager -> varCapacity) {
    return;
  }

  /* Reallocate the varTable and initialize the new entries. */
  rbcManager -> varTable = 
    REALLOC(Rbc_t*, rbcManager -> varTable, newVarCapacity);
  for (i = rbcManager -> varCapacity; i < newVarCapacity; i++) {
    rbcManager -> varTable[i] = NIL(Rbc_t);
  }
  rbcManager -> varCapacity = newVarCapacity;

  return;

} /* End of Rbc_ManagerReserve. */


/**Function********************************************************************

  Synopsis    [Returns the current variable capacity of the rbc.]

  Description [This number is the maximum number of variables (starting from 0)
               that can be requested without causing any memory allocation.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
int
Rbc_ManagerCapacity(
  Rbc_Manager_t * rbcManager             
)
{

  return rbcManager -> varCapacity;

} /* End of Rbc_ManagerCapacity. */


/**Function********************************************************************

  Synopsis    [Deallocates an RBC manager.]

  Description [Frees the variable index and the internal dag manager.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
Rbc_ManagerFree(
  Rbc_Manager_t * rbcManager           
)
{

  /* Cannot deallocate an empty rbc. */
  if (rbcManager == NIL(Rbc_Manager_t)) {
    return;
  }

  /* Free the variable index and the dag manager. */
  FREE(rbcManager -> varTable);
  Dag_ManagerFree(rbcManager -> dagManager, 
                  (Dag_ProcPtr_t) NULL,
                  (Dag_ProcPtr_t) NULL);
  
  /* Free the hash tables used in CNF convertions */
  free_assoc(rbcManager -> rbcNode2cnfVar_model);
  free_assoc(rbcManager -> rbcNode2cnfVar_cnf);

  free_assoc(rbcManager -> cnfVar2rbcNode_model);
  free_assoc(rbcManager -> cnfVar2rbcNode_cnf);

  /* Free the rbc itself.  */
  FREE(rbcManager);

  return;

} /* End of Rbc_ManagerFree. */


/**Function********************************************************************

  Synopsis    [Resets RBC manager]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
Rbc_ManagerReset(
  Rbc_Manager_t * rbcManager
)
{

  /* Cannot deallocate an empty rbc. */
  if (rbcManager == NIL(Rbc_Manager_t)) {
    return;
  }

  /* Free the hash tables used in CNF conversion (cnf only) */
  if ((hash_ptr)(NULL) != rbcManager -> rbcNode2cnfVar_cnf) {
    clear_assoc(rbcManager -> rbcNode2cnfVar_cnf); 
  }
  if ((hash_ptr)(NULL) != rbcManager -> cnfVar2rbcNode_cnf) {
    clear_assoc(rbcManager -> cnfVar2rbcNode_cnf);
  }

  return;

} /* End of Rbc_ManagerFree. */


/**Function********************************************************************

  Synopsis    [Garbage collection in the RBC manager.]

  Description [Relies on the internal DAG garbage collector.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void
Rbc_ManagerGC(
  Rbc_Manager_t * rbcManager          
)
{
  
  /* Cannot garbage collect an empty Rbc. */
  if (rbcManager == NIL(Rbc_Manager_t)) {
    return;
  }

  /* Call DAG garbage collector. */
  Dag_ManagerGC(rbcManager -> dagManager, 
                (Dag_ProcPtr_t) NULL,
                (Dag_ProcPtr_t) NULL);

  return;

} /* End of Rbc_ManagerGC. */
