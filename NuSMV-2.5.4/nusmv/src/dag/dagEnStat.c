/**CFile***********************************************************************

  FileName    [dagEnStat.c]

  PackageName [dag]

  Synopsis    []

  Description []

  SeeAlso     []

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
#include "dag.h"
#include "dagInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define MAX_DEGREE 10000
#define MAX_DEPTH  100000

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct Statistics Statistics_t;
typedef struct StatData StatData_t;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
struct Statistics {
  int degree_stat    [MAX_DEGREE];
  int depth_stat     [MAX_DEPTH];
  int var_depth_stat [MAX_DEPTH];
  int nodes_num;
};

/**Struct**********************************************************************

  Synopsis    []

  Description []

  SeeAlso     []

******************************************************************************/
struct StatData{
  int fatherNum;
  int depth;
  int knownDepthFatherNum;
};

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/**Macro***********************************************************************

  Synopsis    [Makes the code more readable]

  Description [Makes the code more readable]

  SideEffects []

  SeeAlso     []
******************************************************************************/
#define FatherNum(N)           ((StatData_t*)((N)->gRef))->fatherNum
#define KnownDepthFatherNum(N) ((StatData_t*)((N)->gRef))->knownDepthFatherNum
#define Depth(N)               ((StatData_t*)((N)->gRef))->depth

/**AutomaticStart*************************************************************/
/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void
doNothingAndReturnVoid(Dag_Vertex_t* f, char* visData, nusmv_ptrint sign);

static int
doNothingAndReturnZero(Dag_Vertex_t* f, char * visData, nusmv_ptrint sign);

static void ResetStat(Statistics_t* stat);

static int
ComputeFatherAndSonNum(Dag_Vertex_t* f, char * visData, nusmv_ptrint sign);

static void ComputeDepth(Dag_Vertex_t* v, int p_depth, Statistics_t* stat);

static void _PrintStat(Statistics_t* stat, FILE* statFile, char* prefix);

/**AutomaticEnd***************************************************************/
/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Visit a DAG to compute some statistics]

  Description        [Calls Depth First Search on the DAG dfsRoot to populate
                      the struct Statistics.
                      Then calls _PrintStat to print out them.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void PrintStat(Dag_Vertex_t* dfsRoot, FILE* statFile, char* prefix)
{
  if (dfsRoot != (Dag_Vertex_t*)NULL) {

    Dag_DfsFunctions_t fathnsonDFS, statDFS;
    Statistics_t stat;
    ResetStat(&stat);

    fathnsonDFS.FirstVisit = statDFS.FirstVisit =
      fathnsonDFS.BackVisit  = statDFS.BackVisit  =
      fathnsonDFS.LastVisit  = statDFS.LastVisit  = doNothingAndReturnVoid;

    fathnsonDFS.Set = ComputeFatherAndSonNum;
    statDFS.Set     = doNothingAndReturnZero;

    Dag_Dfs     (dfsRoot, &dag_DfsClean,   NIL(char));
    Dag_Dfs     (dfsRoot, &fathnsonDFS, (char*)&stat);
    ComputeDepth(dfsRoot, 0           ,        &stat);
    Dag_Dfs     (dfsRoot, &statDFS    , (char*)&stat);

    _PrintStat(&stat, statFile, prefix);
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Dfs function doing nothing]

  Description        [Dfs function doing nothing]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void
doNothingAndReturnVoid(Dag_Vertex_t* f, char* visData, nusmv_ptrint sign)
{}

/**Function********************************************************************

  Synopsis           [Dfs function returning zero]

  Description        [Dfs function returning zero]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
doNothingAndReturnZero(Dag_Vertex_t* f, char * visData, nusmv_ptrint sign)
{ return 0; }

/**Function********************************************************************

  Synopsis           [Reset the statistics data]

  Description        [Reset the statistics data]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ResetStat(Statistics_t* stat)
{
  int i;

  stat->nodes_num = 0;

  for (i=0;i<MAX_DEGREE; i++)
    (stat->degree_stat)[i]=0;

  for (i=0;i<MAX_DEPTH; i++)
    (stat->var_depth_stat)[i]=(stat->depth_stat)[i]=0;
}

/**Function********************************************************************

  Synopsis           [Dfs function]

  Description        [Dfs function]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
ComputeFatherAndSonNum(Dag_Vertex_t* f, char * visData, nusmv_ptrint sign)
{
  if (f->gRef == (char*)NULL) {

    f->gRef = (char *)ALLOC(StatData_t,1);
    KnownDepthFatherNum(f) = FatherNum(f) = Depth(f) = 0;

    (((Statistics_t*)visData)->
      degree_stat[((f->outList)==NULL) ? 0 : f->numSons])++;
  }

  FatherNum(f)++;
  return (0);
}

/**Function********************************************************************

  Synopsis           [Dfs function]

  Description        [Dfs function]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void ComputeDepth(Dag_Vertex_t* v, int p_depth, Statistics_t* stat)
{
  unsigned gen;
  Dag_Vertex_t* vSon;

  v = Dag_VertexGetRef(v);

  Depth(v) = MAX(p_depth,Depth(v));

  if ((++(KnownDepthFatherNum(v))) == FatherNum(v)) {

     ((stat->depth_stat)[Depth(v)])++;

    if (v -> outList != (Dag_Vertex_t**) NULL) {
      if ((v->numSons)==0) {
         ((stat->var_depth_stat)[Depth(v)])++;
      }

      for (gen=0; gen<v->numSons; gen++) {
        vSon = v->outList[gen];

        ComputeDepth(vSon, Depth(v)+1,stat);
      }
    }
    else {
       ((stat->var_depth_stat)[Depth(v)])++;
    }
  }

  return;
}

/**Function********************************************************************

  Synopsis           [Print out the number of nodes by degree and depth]

  Description        [Print these data:
                      1. Total nodes per number of children;
                      2. Total nodes and total leaves per depth.]

  SideEffects        [data are appended to statFile]

  SeeAlso            [PrintStat()]

******************************************************************************/
static void _PrintStat(Statistics_t* stat, FILE* statFile, char* prefix)
{
  int i;

  for (i=0;i<MAX_DEGREE; i++)
    if ((stat->degree_stat)[i]>0)
      fprintf(statFile,
              "%s Nodes with %i sons: %i\n",
              prefix,
              i,
              (stat->degree_stat)[i]);

  for (i=0;i<MAX_DEPTH; i++)
    if ((stat->depth_stat)[i]>0)
      fprintf(statFile,
              "%s Nodes at depth %i: %i, leaves among them: %i\n",
              prefix,
              i,
              (stat->depth_stat)[i],
              (stat->var_depth_stat)[i]);
}

