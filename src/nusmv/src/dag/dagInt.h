/**CHeaderFile*****************************************************************

  FileName    [dagInt.h]

  PackageName [dag]

  Synopsis    [Directed acyclic graphs with sharing.]

  Description [Internal functions and data structures of the dag package.]

  SeeAlso     []

  Author      [Armando Tacchella and Tommi Junttila]

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

#ifndef _DAGINT
#define _DAGINT

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "dag.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define DAGMAX_WORDS  ((int) 10)
#define DAGWORD_SIZE  ((int) (NUSMV_SIZEOF_VOID_P * 4))


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis      [DAG manager.]
  Description   [Holds the vertices of a dag:
                 <ul> 
                 <li> vTable, the vertices hash (maintains uniqueness);
                 <li> gcList, the free list (candidates for GC);
                 <li> dfsCode, initially 0 is the code of the current DFS;
		 <li> stats, for bookkeeping.
                 </ul>]
  SeeAlso       []
******************************************************************************/
struct DagManager {
  st_table     * vTable;
  int            hashFn[DAGMAX_WORDS];
  lsList         gcList;
  int            dfsCode;
  
  int            stats[DAG_MAX_STAT];
};

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void DagVertexInit(Dag_Manager_t * dagManager, Dag_Vertex_t * v);
EXTERN int DagVertexComp(const char * v1, const char * v2);
EXTERN int DagVertexHash(char * v, int modulus);

/**AutomaticEnd***************************************************************/

#endif /* _DAGINT */
