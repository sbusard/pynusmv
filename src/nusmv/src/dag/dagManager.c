/**CFile***********************************************************************

  FileName    [dagManager.c]

  PackageName [dag]

  Synopsis    [DAG manager main routines.]

  Description [External procedures included in this module:
              <ul>
              <li> <b>Dag_ManagerAlloc()</b> allocates a DAG Manager;
              <li> <b>Dag_ManagerAllocWithParams()</b> user-driven allocation;
              <li> <b>Dag_ManagerFree()</b> deallocates a DAG Manager;
              <li> <b>Dag_ManagerGC()</b> forces a garbage collection.
              </ul>]

  SeeAlso     [dagVertex.c dagDfs.c]

  Author      [Armando Tacchella]

  Copyright   [
  This file is part of the ``dag'' package of NuSMV version 2. 
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

  Revision    [v. 1.0]

******************************************************************************/

#include "dagInt.h"
#include "utils/utils.h"

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

static void GC(Dag_Vertex_t * v, Dag_ProcPtr_t freeData, Dag_ProcPtr_t freeGen);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Creates a new DAG manager.]

  Description [Allocates the unique table (vTable) and the free list (gcList).
               Initializes the counters for various statistics (stats).
               Returns the pointer to the dag manager.]

  SideEffects [none]

  SeeAlso     [Dag_ManagerAllocWithParams Dag_ManagerFree]

******************************************************************************/
Dag_Manager_t *
Dag_ManagerAlloc()
{

  int             i;
  Dag_Manager_t * dagManager = ALLOC(Dag_Manager_t, 1);
  nusmv_assert(dagManager != (Dag_Manager_t*) NULL);

  /* Number of bins is DAG_INIT_VERTICES_NO, number of max entries per bin is
     20, growth factor is 1.5, no reordering occurs. */
  dagManager -> vTable = 
    st_init_table_with_params(DagVertexComp, DagVertexHash,
                              DAG_DEFAULT_VERTICES_NO, 
                              DAG_DEFAULT_DENSITY, 
                              DAG_DEFAULT_GROWTH, 0);

  /* Calculate universal hash function parameters. */
  utils_random_set_seed();
  for (i = 0; i < DAGMAX_WORDS; i++) {
    dagManager -> hashFn[i] = utils_random() % DAGWORD_SIZE;
  }

  /* A list for garbage collection. */
  dagManager -> gcList = lsCreate();

  /* Initial value of dfsCode is zero: */
  dagManager -> dfsCode = 0;

  /* Initialize statistics. */
  for (i = 0; i < DAG_MAX_STAT; i++) {
    dagManager -> stats[i] = 0;
  }
  
  return dagManager;

} /* End of Dag_ManagerAlloc. */


/**Function********************************************************************

  Synopsis    [Deallocates a DAG manager.]

  Description [Forces a total garbage collection and then deallocates the
               dag manager. `freeData' can be used to deallocate `data'
               fields (user data pointers) in the nodes, while `freeGen' 
               is applied to `gRef' fields (user generic pointers). 
               `freeData' and `freeGen' are in the form `void f(char * r)'.]

  SideEffects [none]

  SeeAlso     [Dag_ManagerGC] 

******************************************************************************/
void
Dag_ManagerFree(
  Dag_Manager_t * dagManager,           
  Dag_ProcPtr_t   freeData,
  Dag_ProcPtr_t   freeGen)
{

  lsGen          gen;
  Dag_Vertex_t * v;

  if (dagManager == NIL(Dag_Manager_t)) {
    return;
  }

  /* Collect everything (including permanent nodes) as garbage. */
  gen = lsStart(dagManager -> gcList);
  while (lsNext(gen, (lsGeneric*) &v, LS_NH) == LS_OK) {
    v = Dag_VertexGetRef(v);
    GC(v, freeData, freeGen);
  }
  lsFinish(gen);
  
  /* Free vertices table and vertices list. */
  st_free_table(dagManager -> vTable);
  lsDestroy(dagManager -> gcList, (void (*)()) NULL);
  
  /* Free the dag itself. */
  FREE(dagManager);

} /* End of Dag_ManagerFree. */


/**Function********************************************************************

  Synopsis    [Garbage collects the DAG manager.]

  Description [Sweeps out useless vertices, i.e., vertices that are not 
               marked as permanent, that are not descendants
               of permanent vertices, or whose brother (if any) is neither
               permanent nor descendant of a permanent vertex.
               The search starts from vertices that are in the garbage
               bin and whose mark is 0. 
               `freeData' can be used to deallocate `data'
               fields (user data pointers) in the nodes, while `freeGen' 
               is applied to `gRef' fields (user generic pointers). 
               `freeData' and `freeGen' are in the form `void f(char * r)'.]

  SideEffects [none]

  SeeAlso     [Dag_ManagerFree]

******************************************************************************/
void
Dag_ManagerGC(
  Dag_Manager_t * dagManager,          
  Dag_ProcPtr_t   freeData,
  Dag_ProcPtr_t   freeGen)
{
  
  Dag_Vertex_t  * v;
  lsGen           gen;
  
  if (dagManager == NIL(Dag_Manager_t)) {
    return;
  }

  /* Start from fatherless and non-permanent vertices. */
  gen = lsStart(dagManager -> gcList);
  while (lsNext(gen, (lsGeneric*) &v, LS_NH) == LS_OK) {
    v = Dag_VertexGetRef(v);
    if (v -> mark == 0) {
      GC(v, freeData, freeGen);
    }
  }
  lsFinish(gen);
    
  return;

} /* End of Dag_ManagerGC. */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

  
/**Function********************************************************************

  Synopsis    [Depth-first garbage collection.]

  Description [Gets a vertex to be freed. If the vertex has permanent or 
               non-orphan brothers it is rescued. Otherwise the brother is
               unconnected and the sons marks are updated. GC is then 
               propagated to each fatherless son.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
static void
GC(
  Dag_Vertex_t  * v,
  Dag_ProcPtr_t   freeData,
  Dag_ProcPtr_t   freeGen)
{
  unsigned       gen;
  Dag_Vertex_t * vTemp;

  /* While the vertex is still intact remove it from the hash table. */
  st_delete(v -> dag -> vTable, (char**) &v, (char**) &vTemp);

  /* If deallocating functions are provided, use them on data and gRef. */
  if (freeData != (void (*)()) NULL) {
    (*freeData)(v -> data);
  }
  if (freeGen != (void (*)()) NULL) {
    (*freeData)(v -> gRef);
  }

  /* Decrement the mark of the sons and possibly propagate
     the garbage collection. */
  if (v -> outList != (Dag_Vertex_t**) NULL) {

    for (gen=0; gen<v->numSons; gen++) {
      vTemp = v->outList[gen];
      vTemp = Dag_VertexGetRef(vTemp);
      --(vTemp -> mark);
      if (vTemp -> mark == 0) {
        GC(vTemp, freeData, freeGen);
      }
    }

    /* Deallocate out edges. */
    FREE(v->outList);
  }

  /* If the vertex has an handle to the garbage bin, remove the vertex
     from the bin. */
  if (v ->vHandle != (lsHandle) NULL) {
    lsRemoveItem(v -> vHandle, (lsGeneric*) &v);
  }

  /* Update GC statistics and free the vertex. */
  ++(v -> dag -> stats[DAG_GC_NO]);
  FREE(v);

  return;

} /* End of GC. */


