/**CHeaderFile*****************************************************************

  FileName    [rbcInt.h]

  PackageName [rbc]

  Synopsis    [Formula handling with Reduced Boolean Circuits (RBCs).]

  Description [Internal functions and data structures of the rbc package.]

  SeeAlso     []

  Author      [Armando Tacchella and Tommi Junttila]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2.
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

  Revision    [$Id: rbcInt.h,v 1.3.6.2.2.2.2.2.6.9 2010-02-18 10:00:03 nusmv Exp $]

******************************************************************************/

#ifndef _RBCINT
#define _RBCINT

#include "rbc/rbc.h"
#include "rbc/InlineResult.h"

#include "opt/opt.h"
#include "utils/utils.h"
#include "utils/assoc.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/* both of them used for conditional compilation */
#define RBC_ENABLE_ITE_CONNECTIVE 1
#define RBC_ENABLE_IFF_CONNECTIVE 1

/* Rbc operators (symbols) */
#define RBCTOP   (int) 0
#define RBCVAR   (int) 1
#define RBCAND   (int) 2
#define RBCIFF   (int) 3
#define RBCITE   (int) 4

/* special value for a rbc node.
   The constant can be any illegal pointer value with proper alignment
   (see the description in definition of DAG_ANNOTATION_BIT for more info
   about alignment).
*/
#define RBCDUMMY ((Rbc_t*) 4)

/* Rbc statistics. */
#define RBCVAR_NO   (int)  0  /* How many variables. */
#define RBCMAX_STAT (int)  1

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************
  Synopsis      [RBC manager.]
  Description   [Handles rbcs:
                 <ul>
                 <li> dagManager, to handle the associated pool of vertices;
                 <li> varTable, to index variable vertices;
                 <li> varCapacity, the maximum number of variables;
                 <li> one and zero, the logical constants true and false;
                 <li> rbcNode2cnfVar: RBC node -> CNF var
                      (used only in CNF convertion);
                 <li> cnfVar2rbcNode: CNF var -> RBC node;
                      (used only to obtain original variables from
                       CNF formula solution);
                 <li> maxUnchangedRbcVariable is the maximal RBC var
                      that will have the same index in CNF
                      (used for ease the readability of CNF formulas)
                      It is set during the first invocation of Rbc_Convert2Cnf;
                 <li> maxCnfVariable is maximal variable used in CNF formula,
                      used to generate new unique CNF variables.
                 <li> stats, for bookkeeping.
                 </ul>]
  SeeAlso       []
******************************************************************************/
struct RbcManager {
  Dag_Manager_t* dagManager;
  Rbc_t** varTable;
  int varCapacity;
  Rbc_t* one;
  Rbc_t* zero;

  /* splitted cache mapping in two sets (model, cnf) */
  hash_ptr rbcNode2cnfVar_model;
  hash_ptr rbcNode2cnfVar_cnf;

  hash_ptr cnfVar2rbcNode_model;
  hash_ptr cnfVar2rbcNode_cnf;

  int maxUnchangedRbcVariable;
  int maxCnfVariable;

  int stats[RBCMAX_STAT];
};


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/**Macro**********************************************************************
  Synopsis    [Control the way compact CNF conversion is performed]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
//#define CNF_CONV_SP 0

/**Macro***********************************************************************

   Synopsis    [Get the leftmost child.]

   Description [Get the leftmost child.]


   SideEffects []

   SeeAlso     []

******************************************************************************/
#define RBC_GET_LEFTMOST_CHILD(rbc) (rbc->outList[0])

/**Macro***********************************************************************

   Synopsis    [Get the right children.]

   Description [Get the right children.]


   SideEffects []

   SeeAlso     []

******************************************************************************/
#define RBC_GET_SECOND_CHILD(rbc) (rbc->outList[1])

/**Macro**********************************************************************
  Synopsis    [Rbc interface to underlying package]
  Description [Rbc interface to underlying package]
  SideEffects []
  SeeAlso     []
******************************************************************************/
#define RbcGetRef(p) Dag_VertexGetRef(p)
#define RbcSet(p) Dag_VertexSet(p)
#define RbcClear(p) Dag_VertexClear(p)
#define RbcIsSet(p) Dag_VertexIsSet(p)
#define RbcId(r,s) DagId(r,s)
#define Rbc_get_type(rbc) rbc->symbol

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

int Rbc_Convert2CnfSimple ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f,
                                Slist_ptr clauses, Slist_ptr vars,
                                int* literalAssignedToWholeFormula));

int Rbc_Convert2CnfCompact ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f,
                                 int polarity,
                                 Slist_ptr clauses, Slist_ptr vars,
                                 int* literalAssignedToWholeFormula));

int Rbc_get_node_cnf ARGS((Rbc_Manager_t* rbcm, Rbc_t* f, int* maxvar));

/* inlining cache control */
void rbc_inlining_cache_init ARGS((void));
void rbc_inlining_cache_quit ARGS((void));
void rbc_inlining_cache_add_result ARGS((Rbc_t* f, InlineResult_ptr res));
InlineResult_ptr rbc_inlining_cache_lookup_result ARGS((Rbc_t* f));

void Rbc_Dfs ARGS((Rbc_t* dfsRoot,
                   RbcDfsFunctions_t* dfsFun,
                   void* dfsData,
                   Rbc_Manager_t* manager));

void Rbc_Dfs_clean ARGS((Rbc_t* dfsRoot,
                         Rbc_Manager_t* manager));

void Rbc_Dfs_do_only_last_visit ARGS((Rbc_t* dfsRoot,
                                      RbcDfsFunctions_t* dfsFun,
                                      void* dfsData,
                                      Rbc_Manager_t* manager));

/**AutomaticEnd***************************************************************/

#endif /* _RBCINT */
