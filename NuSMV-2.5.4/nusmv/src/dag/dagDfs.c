/**CFile***********************************************************************

  FileName    [dagDfs.c]

  PackageName [dag]

  Synopsis    [Depth First Search routines.]

  Description [External procedures included in this module:
                <ul>
                <li> <b>Dag_Dfs()<b> Generic depth first search engine.
                </ul>]

  SeeAlso     [dagManager.c dagVertex.c]

  Author      [Armando Tacchella, Michele Dorigatti]

  Copyright   [
  This file is part of the ``dag'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by University of Genova. 
  Copyright (C) 2011 by FBK.

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
#include "dag/dagInt.h"
#include "utils/Stack.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define DFS_STACK_INITIAL_SIZE 1024

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/**Macro***********************************************************************

   Synopsis    [Check if a vertex is a leaf]

   Description [Check if a vertex is a leaf]

   SideEffects []

   SeeAlso     []

******************************************************************************/
#define IS_LEAF(vertex) ((Dag_Vertex_t**)NULL == vertex->outList)

/**AutomaticStart*************************************************************/
/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int return_zero(Dag_Vertex_t* v, char* cleanData, nusmv_ptrint sign);
static void clean_first(Dag_Vertex_t* v, char* cleanData, nusmv_ptrint sign);
static void do_nothing(Dag_Vertex_t* v, char* cleanData, nusmv_ptrint sign);

#ifdef DAG_DFS_RECURSION
static void
DFS(Dag_Vertex_t* v,
    Dag_DfsFunctions_t* dfsFun,
    char* dfsData,
    nusmv_ptrint vBit);
#endif

/**AutomaticEnd***************************************************************/
/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
Dag_DfsFunctions_t dag_DfsClean = {return_zero,
                                   clean_first,
                                   do_nothing,
                                   do_nothing};

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Performs a generic, iterative, DFS on the dag.]

  Description [The parameters are:
               <ul>
               <li> current, the dag vertex where to start the DFS
               <li> dfs_fun, the functions to perform the DFS steps
               <li> dfs_data, a reference to generic data
               </ul>
               The function increments the DFS code for the dag manager owning
               dfsRoot. Increment of the code guarantees that each node is
               visited once and only once. dfs_fun->Set() may change the default
               behaviour by forcing the DFS to visit nodes more than once
               (by returning -1), or by preventing DFS to do a complete visit
               (by returning 1).]

  SideEffects [node->dag->dfsCode: is incremented by one.]

  SeeAlso     []

******************************************************************************/
#ifndef DAG_DFS_RECURSION
void Dag_Dfs(Dag_Vertex_t* current,
             Dag_DfsFunctions_t* dfs_fun,
             char* dfs_data)
{
  int set;
  int dfsCode;
  Stack_ptr parent_stack;
  Dag_Vertex_t* current_ref;
  nusmv_ptrint current_bit;
  Dag_Vertex_t* parent;
  Dag_Vertex_t* parent_ref;
  nusmv_ptrint parent_bit;
  Dag_Vertex_t* processed;

  if ((Dag_Vertex_t*)NULL == current) return;

  current_bit = Dag_VertexIsSet(current);
  current_ref = Dag_VertexGetRef(current);

  dfsCode = ++(current_ref->dag->dfsCode);

  /* Process the root */
  set = dfs_fun->Set(current_ref, dfs_data, current_bit);

  if (1 == set) return;

  current_ref->visit = dfsCode;

  dfs_fun->FirstVisit(current_ref, dfs_data, current_bit);

  if (!IS_LEAF(current_ref)) {
    parent_stack = Stack_create_with_param(DFS_STACK_INITIAL_SIZE);
    Stack_push(parent_stack, DagId(current_ref, current_bit));
    current = current_ref->outList[0];
    current_bit = Dag_VertexIsSet(current);
    current_ref = Dag_VertexGetRef(current);
  }
  else {
    dfs_fun->LastVisit(current_ref, dfs_data, current_bit);
    return;
  }

  while (true) {
    /* Is it to be processed? */
    set = dfs_fun->Set(current_ref, dfs_data, current_bit);

    if ((-1 == set) ||
        ((0 == set) && (current_ref->visit != dfsCode))) {

      current_ref->visit = dfsCode;

      dfs_fun->FirstVisit(current_ref, dfs_data, current_bit);

      if (!IS_LEAF(current_ref)) {
        Stack_push(parent_stack, DagId(current_ref, current_bit));
        current = current_ref->outList[0];
        current_bit = Dag_VertexIsSet(current);
        current_ref = Dag_VertexGetRef(current);
        continue;
      }
      else {
        dfs_fun->LastVisit(current_ref, dfs_data, current_bit);
      }
    }

    /* Subtree processed. Back on the parent. */
    processed = DagId(current_ref, current_bit);

    while (true) {
      parent = (Dag_Vertex_t*)STACK_TOP(parent_stack);
      parent_bit = Dag_VertexIsSet(parent);
      parent_ref = Dag_VertexGetRef(parent);

      dfs_fun->BackVisit(parent_ref, dfs_data, parent_bit);

      /* Is the last processed node my last (rightmost) child? */
      if (processed == parent_ref->outList[parent_ref->numSons - 1]) {
        /* Subtree processed. Back on the parent, if any. */
        dfs_fun->LastVisit(parent_ref, dfs_data, parent_bit);
        processed = (Dag_Vertex_t*)Stack_pop(parent_stack);

        if (STACK_IS_EMPTY(parent_stack)) {
          Stack_destroy(parent_stack);
          return;
        }
      }
      else {
        /* Check needed for ITE. What is the next child of parent? */
        if (processed == parent_ref->outList[0]) {
          current = parent_ref->outList[1];
        }
        else current = parent_ref->outList[2];

        current_bit = Dag_VertexIsSet(current);
        current_ref = Dag_VertexGetRef(current);
        break;
      }
    }
  } /* End of the external cycle */
} /* End of Dag_Dfs. */

#else
/**Function********************************************************************

  Synopsis    [Performs a generic DFS on the dag.]

  Description [The parameters are:
               <ul>
               <li> dfsRoot, the dag vertex where to start the DFS
               <li> dfsFun, the functions to perform the DFS steps
               <li> dfsData, a reference to generic data
               </ul>
               The function increments the DFS code for the dag
               manager owning dfsRoot and starts the DFS. Increment of
               the code guarantees that each node is visited once and
               only once. dfsFun -> Set() may change the default behaviour by
               forcing to DFS to visit nodes more than once, or by preventing
               DFS to do a complete visit.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void Dag_Dfs(Dag_Vertex_t* dfsRoot, Dag_DfsFunctions_t* dfsFun,
             char* dfsData)
{
  /* DFS cannot start from a NULL vertex. */
  if (dfsRoot == NIL(Dag_Vertex_t)) {
    return;
  }

  /* Increment the current DFS code for the dag manager. */
  ++(Dag_VertexGetRef(dfsRoot) -> dag -> dfsCode);

  /* Start the real thing. */
   DFS(Dag_VertexGetRef(dfsRoot), dfsFun, dfsData, Dag_VertexIsSet(dfsRoot));

   return;
} /* End of Dag_Dfs. */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Performs a generic (recursive) DFS on the dag.]

  Description [The parameters are:
               <ul>
               <li> v, the current dag vertex
               <li> dfsFun, the functions to perform the DFS
               <li> dfsData, a reference to generic data
               <li> vBit, the incoming link annotation (0 or not-0)
               </ul>]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
static void
DFS(
  Dag_Vertex_t       * v,
  Dag_DfsFunctions_t * dfsFun,
  char               * dfsData,
  nusmv_ptrint                  vBit)
{
  unsigned gen;
  Dag_Vertex_t* vSon;
  int set;

  /* dfsFun -> Set() is -1 if the node is to be visited and 1 if the node
     is not to be visited; 0 means that the DFS should decide what to do. */
  set = dfsFun -> Set(v, dfsData, vBit);
  if ((set == 1) || ((set == 0) && (v -> visit == v -> dag -> dfsCode))) {
    return;
  } else {
    v -> visit = v -> dag -> dfsCode;
  }

  /* Do the first visit. */
  (dfsFun -> FirstVisit)(v, dfsData, vBit);

  /* Visit each son (if any). */
  if (v -> outList != (Dag_Vertex_t **) NULL) {
    for (gen = 0; gen < v->numSons; gen++) {
      vSon = v->outList[gen];

      DFS(Dag_VertexGetRef(vSon), dfsFun, dfsData, Dag_VertexIsSet(vSon));
      /* Do the back visit. */
      (dfsFun -> BackVisit)(v, dfsData, vBit);
    }
  }
  /* Do the last visit and return . */
  (dfsFun -> LastVisit)(v, dfsData, vBit);

  return;

} /* End of DFS. */

#endif /* DAG_DFS_RECURSION */

/**Function********************************************************************

  Synopsis    [Dfs SetVisit for cleaning.]

  Description [Dfs SetVisit for cleaning.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int return_zero(Dag_Vertex_t* f, char* cleanData, nusmv_ptrint sign)
{
  return 0;
}

/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for cleaning.]

  Description [Dfs FirstVisit for cleaning.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void clean_first(Dag_Vertex_t* f, char* cleanData, nusmv_ptrint sign)
{
  f->iRef = 0;
  f->gRef = (char*)NULL;
}

/**Function********************************************************************

  Synopsis    [Dfs Back & Last visit for cleaning.]

  Description [Dfs Back & Last visit for cleaning.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void do_nothing(Dag_Vertex_t* f, char* cleanData, nusmv_ptrint sign)
{
}

