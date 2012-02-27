/**CFile***********************************************************************

  FileName    [dagVertex.c]

  PackageName [dag]

  Synopsis    [Vertex handling.]

  Description [External procedures included in this module:
                <ul>
                <li> <b>Dag_VertexLookup()</b> Lookup for a vertex;
                <li> <b>Dag_VertexInsert()</b> Insert a vertex;
                <li> <b>Dag_VertexMark()</b> make a vertex permanent;
                <li> <b>Dag_VertexUnmark()</b> make a vertex volatile;
                </ul>
               Internal procedures included in this module:
                <ul>
                <li> <b>DagVertexInit()</b> Initialize a vertex;
                <li> <b>DagVertexComp()</b> Compare two vertices;
                <li> <b>DagVertexHash()</b> calculate vertex hash code;
                </ul>]

  SeeAlso     [dagManager dagDfs]

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

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#if !defined(NUSMV_SIZEOF_VOID_P) || !defined(NUSMV_SIZEOF_INT)
#error Constant NUSMV_SIZEOF_VOID_P and NUSMV_SIZEOF_INT must be defined
#endif

# define HASH_LOWER   ((((nusmv_ptrint) 1) << (NUSMV_SIZEOF_VOID_P * 8 / 2)) - 1)
# define HASH_UPPER   ((nusmv_ptrint) (~ HASH_LOWER))

# define HASH_INTLOWER   ((((nusmv_ptrint) 1) << (NUSMV_SIZEOF_INT * 8 / 2)) - 1)
# define HASH_INTUPPER   ((nusmv_ptrint) (~ HASH_INTLOWER))


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

  Synopsis    [Vertex lookup.]

  Description [Uniquely adds a new vertex into the DAG and returns a
               reference to it:
               <ul>
               <li> vSymb is a NON-NEGATIVE  integer (vertex label);
               <li> vData is a pointer to generic user data;
               <li> vSons is a list of vertices (possibly NULL).
               </ul>
               Returns NIL(Dag_vertex_t) if there is no dagManager and 
               if vSymb is negative.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Dag_Vertex_t *
Dag_VertexLookup(
  Dag_Manager_t * dagManager,
  int             vSymb,
  char          * vData,
  Dag_Vertex_t**  vSons,
  unsigned        numSons)
{

  char        ** slot;
  int            found;
  Dag_Vertex_t * v;

  /* A vertex cannot be added to an uninitialized dag and vSymb
     cannot be a negative number. */
  if ((dagManager == NIL(Dag_Manager_t)) || (vSymb < 0)) {
    return NIL(Dag_Vertex_t);
  }

  /* Temporary allocate the vertex, and fill in just the basic
     information to calculate the hash code. */
  v = ALLOC(Dag_Vertex_t, 1);
  nusmv_assert(v != (Dag_Vertex_t*) NULL);
  v -> symbol = vSymb;
  v -> data = vData;
  v -> outList = vSons;
  v -> numSons = numSons;
  v -> dag = dagManager;

  /* Lookup the vertex in the hash table. */
  found = st_find_or_add(dagManager -> vTable, (char *) v, &slot);
  nusmv_assert(found == 0 || found == 1); /* no out of memory */

  if (found) {
    /* The key already existed: free temporary allocations and
       let v be the vertex found in the table. */
    if (vSons != (Dag_Vertex_t**) NULL) {
      FREE(vSons);
    }
    FREE(v);
    v = (Dag_Vertex_t*) *slot;
  } else {
    /* The key was not there: store the vertex (the value coincides
       with the key), and make the vertex information complete. */
    *slot = (char *) v;
    DagVertexInit(dagManager, v);
  }

  return v;

} /* End of Dag_VertexLookup. */


/**Function********************************************************************

  Synopsis    [Vertex insert.]

  Description [Adds a vertex into the DAG and returns a
               reference to it:
               <ul>
               <li> vSymb is an integer code (vertex label);
               <li> vData is a generic annotation;
               <li> vSons must be a list of vertices (the intended sons).
               </ul>
               Returns NIL(Dag_vertex_t) if there is no dagManager and
               if vSymb is negative.]
               NOTICE: as opposed to Dag_VertexLookup, the unique table
               is not accessed, so there is no guarantee of uniqueness
               for vertices added with this mechanism.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Dag_Vertex_t *
Dag_VertexInsert(
  Dag_Manager_t * dagManager,
  int             vSymb,
  char          * vData,
  Dag_Vertex_t ** vSons,
  unsigned        numSons)
{

  Dag_Vertex_t * v;

  /* A vertex cannot be added to an uninitialized dag and vSymb
     cannot be a negative number. */
  if ((dagManager == NIL(Dag_Manager_t)) || (vSymb < 0)) {
    return NIL(Dag_Vertex_t);
  }

  /* Allocate the vertex, and fill in the information. */
  v = ALLOC(Dag_Vertex_t, 1);
  v -> symbol = vSymb;
  v -> data = vData;
  v -> outList = vSons;
  v -> numSons = numSons;

  /* Initialize the vertex and return it. */
  DagVertexInit(dagManager, v);

  return v;

} /* End of Dag_VertexInsert. */


/**Function********************************************************************

  Synopsis    [Marks a vertex as permanent.]

  Description [Increments the vertex mark by one, so it cannot be
               deleted by garbage collection unless unmarked.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void Dag_VertexMark(
  Dag_Vertex_t  * v)
{
  ++(Dag_VertexGetRef(v) -> mark);
  return;
} /* End of Dag_VertexMark. */


/**Function********************************************************************

  Synopsis    [Unmarks a vertex (makes it volatile).]

  Description [Decrements the vertex mark by one, so it can be
               deleted by garbage collection when fatherless.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void Dag_VertexUnmark(
  Dag_Vertex_t  * v)
{ 
  if (Dag_VertexGetRef(v) -> mark > 0) {
    --(Dag_VertexGetRef(v) -> mark);
  }
  return;
} /* End of Dag_VertexUnmark. */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Vertex initialization.]

  Description [Performs several tasks:
               <ul>
               <li> connects the vertex to the sons by increasing the sons'
                    marks
               <li> removes sons from the free list if their mark
                    is increased to one for the first time;
               <li> clears the vertex mark and stores the vertex in the 
                    free list;
               <li> clears other internal fields.
               </ul>] 

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
DagVertexInit(
  Dag_Manager_t * dagManager,            
  Dag_Vertex_t  * v)
{
  /* violation of this assert means that implementation of annotation
     bit is incorrect or the alignment of pointers is not set right.
     see description of DAG_ANNOTATION_BIT for more info 
  */
  nusmv_assert(Dag_VertexGetRef(v) == v);

  unsigned       gen;
  Dag_Vertex_t * vSon;

  if (v -> outList != (Dag_Vertex_t**) NULL) {
    for (gen=0; gen<v->numSons; gen++) {
      vSon = v->outList[gen];

      /* Clear vertex from possible bit annotation. */
      vSon = Dag_VertexGetRef(vSon);
      /* Increment the number of fathers for each son: when it becomes one,
         remove the son from the garbage bin. */ 
      if (++(vSon -> mark) == 1) {
        (void) lsRemoveItem(vSon -> vHandle, (lsGeneric*) &vSon);
        vSon -> vHandle = (lsHandle) NULL;
      }
    }
  }
  /* The vertex is created fatherless, and it sits in the garbage bin. */
  v -> mark = 0;
  lsNewBegin(dagManager -> gcList, (lsGeneric) v, &(v -> vHandle));

  /* The vertex is owned by dagManager and it was never visited. */
  v -> dag = dagManager;
  v -> visit = 0;

  /* Update statistics. */
  ++(v -> dag -> stats[DAG_NODE_NO]);

  return;

} /* End of DagVertexInit. */


/**Function********************************************************************

  Synopsis    [Compare two vertices.]

  Description [Gets two vertex pointers v1, v2, (as char pointers) and
               compares the symbol, the generic data reference and the
               pointers to the sons. Returns -1 if v1 < v2, 0 if v1 =
               v2 and 1 if v1 > v2, in lexicographic order of fields.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
DagVertexComp(
  const char * v1,
  const char * v2)
{
  Dag_Vertex_t *e1, *e2;
  unsigned i;
  nusmv_ptrint c;

  e1 = (Dag_Vertex_t*) v1;
  e2 = (Dag_Vertex_t*) v2;

  /* First compare symbols... */
  c = (nusmv_ptrint) (e1->symbol - e2->symbol);
  if (c != 0) return (c > 0) ? 1 : -1;

  /* ...then compare generic data references. */
  c = (nusmv_ptrint) (e1->data - e2->data);
  if (c != 0) return (c > 0) ? 1 : -1;

  /* Verify that both outLists are non NULL. */
  if ((e1->outList != (Dag_Vertex_t**) NULL) &&
      (e2->outList != (Dag_Vertex_t**) NULL)) {

    /* This is an internal vertex: compare how many sons... */
    c = (nusmv_ptrint) (e1->numSons - e2->numSons);
    if (c != 0) return (c > 0) ? 1 : -1;

    /* ... finally compare each son. */
    for (i=0; i<e1->numSons; i++) {
      c = (nusmv_ptrint) e1->outList[i] - (nusmv_ptrint) e2->outList[i];
      if (c != 0) break;
    }
  }

  if (c != 0) return (c > 0) ? 1 : -1;
  return 0;
} /* End of DagVertexComp. */


/**Function********************************************************************

  Synopsis    [Calculate the hash key of a vertex.]

  Description [Calculate a preliminary index as follows:
                  v -> symbol                            + 
                  8 low order bits of (int) (v -> data)  +
                 16 low order bits of each son up to MAXSON +
                  1 for each son whose edge is annotated
               Return the modulus of the index and the actual hash size.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int DagVertexHash(char* v, int modulus)
{
  unsigned      gen;
  int           i;
  unsigned long h;
  static nusmv_ptrint x[DAGMAX_WORDS];
  int           x_length;
  int         * hashFn;
  nusmv_ptrint  son;

  /* Get the symbol. */
  x[0] = (((Dag_Vertex_t*) v) -> symbol & HASH_INTUPPER) >> (NUSMV_SIZEOF_INT * 4);
  x[1] = ((Dag_Vertex_t*) v) -> symbol & HASH_INTLOWER;

  /* Get the data. */
  x[2] = ((nusmv_ptrint)(((Dag_Vertex_t*) v) -> data) & HASH_UPPER) 
    >> DAGWORD_SIZE;
  x[3] = (nusmv_ptrint)(((Dag_Vertex_t*) v) -> data) & HASH_LOWER;

  /* Get the son(s). */
  x_length = 4;
  if (((Dag_Vertex_t*) v)->outList != (Dag_Vertex_t**)NULL) {
    for (gen=0; gen<((Dag_Vertex_t*) v)->numSons; gen++) {
      son = (nusmv_ptrint) ((Dag_Vertex_t*) v)->outList[gen];

      nusmv_assert(x_length + 1 < DAGMAX_WORDS);
      x[x_length++] = (son & HASH_UPPER) >> DAGWORD_SIZE;
      x[x_length++] = son & HASH_LOWER;
    }
  }

  hashFn = ((Dag_Vertex_t*) v) -> dag -> hashFn;
  /* Calculate hash key. */
  for (i = 0, h = 0; i < x_length; i++) {
    h += x[i] * hashFn[i];
  }

  /* Get the key and return it (division method). */
  return (int)(h % modulus);

} /* End of DagVertexHash. */

