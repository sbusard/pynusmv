/**CFile***********************************************************************

  FileName    [rbcCnfCompact.c]

  PackageName [rbc]

  Synopsis    [Conjunctive Normal Form (CNF) conversions.]

  Description [External functions included in this module:
                <ul>
                <li> <b>Rbc_Convert2Cnf()</b>
                </ul>]

  SeeAlso     []

  Author      [Daniel Sheridan, Marco Roveri and Gavin Keighren]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2.
  Copyright (C) 2007 by FBK-irst.

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

  For more information of NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#include "rbc/rbcInt.h"
#include "clg/clg.h"

#include "utils/utils.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: rbcCnfCompact.c,v 1.1.4.7 2010-02-18 10:00:03 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis      [Data passing in compact cnf-DFS.]
  Description   [Data passing in compact cnf-DFS.]
  SeeAlso       []
******************************************************************************/
struct CnfCompactDfsData {
  Rbc_Manager_t*  rbcManager;
  int             maxVar;     /* Maximum variable index so far */
  clause_graph    clauses;    /* List of clauses generated so far */
  Slist_ptr       vars;       /* List of variables used so far */
  clause_graph    posClauses; /* Current clause list for positive polarity */
  clause_graph    negClauses; /* Current clause list for negative polarity */
  int             pol;        /* Current polarity */
  boolean         zeroiff;    /* Current node is a zero-polarity IFF, or a */
};                            /* zero-polarity ITE */

/**Struct**********************************************************************
  Synopsis      [Per-node data in compact cnf-DFS.]
  Description   [Per-node data in compact cnf-DFS.]
  SeeAlso       []
******************************************************************************/
struct CnfCompactDfsNode {
  int                  negRef;     /* Number of negative polarity references */
  int                  posRef;     /* Number of negative polarity references */
  boolean              unseen;     /* Whether this node has been processed */
  clause_graph         posClauses; /* Result of pos conversion at this node */
  clause_graph         negClauses; /* Result of neg conversion at this node */
  clause_graph         ifClauses;  /* Result of pos conversion for IF branch */
};                                 /* of ITE node (not used for other types) */

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct CnfCompactDfsData    CnfCompactDfsData_t;
typedef struct CnfCompactDfsNode    CnfCompactDfsNode_t;


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

static int CnfCompactPolSet(Rbc_t* f, char* cnfData, nusmv_ptrint sign);
static void CnfCompactPolFirstBack(Rbc_t* f, char* cnfData, nusmv_ptrint sign);
static void CnfEmpty(Rbc_t* f, char* cnfData, nusmv_ptrint sign);

static int CnfCompactSet(Rbc_t* f, char* cnfData, nusmv_ptrint sign);
static void CnfCompactFirst(Rbc_t* f, char* cnfData, nusmv_ptrint sign);
static void CnfCompactBack(Rbc_t* f, char* cnfData, nusmv_ptrint sign);
static void CnfCompactLast(Rbc_t* f, char* cnfData, nusmv_ptrint sign);

static int CnfCompactCleanSet(Rbc_t* f, char* cnfData, nusmv_ptrint sign);
static void CnfCompactCleanFirst(Rbc_t* f, char* cnfData, nusmv_ptrint sign);

static void rename_clauses(clause_graph* clauses, int var, clause_graph* saved);

static inline void disjunction(clause_graph* Left, clause_graph* Right, 
                               int* maxVar, clause_graph* clauses, 
                               Rbc_Manager_t* rbcm);

static inline void disjunction2(clause_graph* Left1, clause_graph* Right1, 
                                clause_graph* Left2, clause_graph* Right2, 
                                int* maxVar, clause_graph* clauses, 
                                Rbc_Manager_t* rbcm);

static inline int testSizes(clause_graph left, clause_graph right);

static void CnfCompactCommit(void* data, int* cl, int size);


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Translates the rbc into the corresponding (equisatisfiable)
               set of clauses.]

  Description [Given `rbcManager' and `f', `clauses' is filled with the
               disjunctions corresponding to the rbc nodes according to
               the 'compact' algorithm by Dan Sheridan.
               `vars' is filled with the variables that occurred in `f'
               (original or model variables). It is user's responsibility
               to create `clauses' and `vars' *before* calling the function.
               New variables are added by the conversion: the maximum
               index (the last added variable) is returned by the function.
               The function returns 0 when `f' is true or false. 'polarity'
               defines whether 'f' has to be true, false, or either (1, -1
               or 0 respectively). If 'polarity' is 1/-1 then only the
               clauses representing the true/false RBC are returned. Otherwise,
               both sets are returned.]

  SideEffects [`clauses' and `vars' are filled up. `clauses' is the empty
               list if `f' was true, and contains a single empty clause if
               `f' was false.]

  SeeAlso     []

******************************************************************************/
int Rbc_Convert2CnfCompact(Rbc_Manager_t* rbcManager, Rbc_t* f,
                           int polarity, Slist_ptr clauses, Slist_ptr vars,
                           int* literalAssignedToWholeFormula)
{
  Dag_DfsFunctions_t  cnfFunctions;
  CnfCompactDfsData_t cnfData;
  int renamed = 0;

  /* The caller will ensure this */
  nusmv_assert(*literalAssignedToWholeFormula == INT_MAX);

  /* Setting up the DFS data. */
  cnfData.rbcManager = rbcManager;
  cnfData.clauses    = NULL;
  cnfData.vars       = vars;
  cnfData.posClauses = NULL;
  cnfData.negClauses = NULL;
  cnfData.pol        = polarity;
  cnfData.zeroiff    = 0;
  cnfData.maxVar     = rbcManager->maxCnfVariable;

  /* First, compute the polarity for the whole tree */
  cnfFunctions.Set        = CnfCompactPolSet;
  cnfFunctions.FirstVisit = CnfCompactPolFirstBack;
  cnfFunctions.BackVisit  = CnfCompactPolFirstBack;
  cnfFunctions.LastVisit  = CnfEmpty;
  Dag_Dfs(f, &cnfFunctions, (char*) (&cnfData));


  /* Reset our opinion of the polarity (the previous DFS returns the polarity
     of the top node; we need to start again with a positive polarity to ensure
     that the sign of the top node is correctly honoured) */
  cnfData.pol = 1;

  /* Now, use the polarity to compute the clauses */
  cnfFunctions.Set        = CnfCompactSet;
  cnfFunctions.FirstVisit = CnfCompactFirst;
  cnfFunctions.BackVisit  = CnfCompactBack;
  cnfFunctions.LastVisit  = CnfCompactLast;
  Dag_Dfs(f, &cnfFunctions, (char*) (&cnfData));

  /* Rename the clause sets to be returned. In the case where the polarity is
     zero (where we want to represent 'f' both positively and negatively), the
     variable used to rename each set of clauses must be the same. */
  if (polarity >= 0) {
    renamed = Rbc_get_node_cnf(rbcManager, RBCDUMMY, &(cnfData.maxVar));
    nusmv_assert(0 != renamed);
    rename_clauses(&(cnfData.posClauses), renamed, &(cnfData.clauses));
  }
  if (polarity <= 0) {
    if (polarity < 0) {
      renamed = Rbc_get_node_cnf(rbcManager, RBCDUMMY, &(cnfData.maxVar));
    }
    nusmv_assert(0 != renamed);
    rename_clauses(&(cnfData.negClauses), -renamed, &(cnfData.clauses));
  }

  *literalAssignedToWholeFormula = renamed;

  Clg_Extract(cnfData.clauses, CLG_NUSMV,
              (Clg_Commit) CnfCompactCommit, (void*) &(clauses));
  Clg_Free(cnfData.clauses);


  /* Clean the graph of the allocated data */
  cnfFunctions.Set        = CnfCompactCleanSet;
  cnfFunctions.FirstVisit = CnfCompactCleanFirst;
  cnfFunctions.BackVisit  = CnfEmpty;
  cnfFunctions.LastVisit  = CnfEmpty;
  Dag_Dfs(f, &cnfFunctions, (char*) (&cnfData));

  /* Adjust max var in the RBC manager to be the last generated index. */
  rbcManager->maxCnfVariable = cnfData.maxVar;

  return (cnfData.maxVar);
} /* End of Rbc_Convert2CnfCompact. */

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Dfs Set for CNF conversion.]

  Description [Dfs Set for CNF conversion polarity computation.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int CnfCompactPolSet(Rbc_t* f, char* cnfData, nusmv_ptrint sign)
{
  int result; /* What we will tell dagDfs */
  CnfCompactDfsData_t* cd = (CnfCompactDfsData_t*)cnfData;
  CnfCompactDfsNode_t* nd = (CnfCompactDfsNode_t*)(f->gRef);

  /* This function is called to decide whether to continue down the
     current route. If we get here, and it is a node that has already
     been seen with the current polarity, we still want to update the
     reference counters for this node, but not for its children. */

  /* cd->pol is the polarity of the current part of the search: if a
     polarity has been seen before it is not propogated down -- this
     computes the min of the incoming references */

  /* Create per-node data structure */
  if (nd == NULL) {
    nd = ALLOC(CnfCompactDfsNode_t, 1);
    f->gRef = (char*) nd;

    nd->posRef = 0;
    nd->negRef = 0;
    nd->posClauses = NULL;
    nd->negClauses = NULL;
    nd->ifClauses = NULL;
    nd->unseen = true;

    result = -1; /* unconditionally visit this node */

    /* Set f->iRef to be the change in polarity determined by the
       sign and whether this is an IFF node */
    if (f->symbol == RBCIFF) {
      f->iRef = 0; /* Both a positive and a negative reference */
    } else if (sign) {
      f->iRef = -(cd->pol);
    } else {
      f->iRef = cd->pol;
    }

  } else {

    /* Simple sanity checks */
    nusmv_assert(!nd->posClauses);
    nusmv_assert(!nd->negClauses);
    nusmv_assert(!nd->ifClauses);
    nusmv_assert(nd->unseen);

    /* Set up f->iRef to be the change in polarity due to this node. */

    /* If this node is an IFF then the polarity of the children should be 0  */
    if (f->symbol == RBCIFF) {
      if (( sign && cd->pol >= 0 && nd->negRef == 0) ||
          ( sign && cd->pol <= 0 && nd->posRef == 0) ||
          (!sign && cd->pol >= 0 && nd->posRef == 0) ||
          (!sign && cd->pol <= 0 && nd->negRef == 0)) {
        nusmv_assert(f->iRef == 0);  /* Should already be set to 0 */
        result = -1;
      } else {
        result = 1; /* Don't visit */
      }

    } else if (cd->pol == 0) {

      /* Otherwise, the polarity is compared with the polarity of the
         node so far; if they are the same, we stop. If they are
         different we continue. The special case is when the incoming
         polarity is 0 but we have only seen pos or neg here; then we
         contiune using only the unseen polarity. */

      result = -1; /* Do visit */
      if (nd->negRef > 0 && nd->posRef == 0) {
        f->iRef = 1;
      } else if (nd->negRef == 0 && nd->posRef > 0) {
        f->iRef = -1;
      } else if (nd->negRef == 0 && nd->posRef == 0) { /* Shouldn't happen! */
        f->iRef = 0;
      } else { /* Seen both polarities */
        result = 1;  /* Don't visit */
      }

    } else if (cd->pol > 0) {

      if (!sign && nd->posRef == 0) {
        f->iRef = 1;
        result = -1; /* Do visit */
      } else if (sign && nd->negRef == 0) {
        f->iRef = -1;
        result = -1;  /* Do visit */
      } else {
        result = 1;
      }

    } else { /* cd->pol < 0 */

      if (!sign && nd->negRef == 0) {
        f->iRef = -1;
        result = -1;  /* Do visit */
      } else if (sign && nd->posRef == 0) {
        f->iRef = 1;
        result = -1;  /* Do visit */
      } else {
        result = 1;
      }

    }
  }

  /* Update per-node data structure */
  if (cd->zeroiff) {
    /* Parent was a zero-polarity IFF, or this node
       is the IF branch of a zero-polarity ITE */
    nd->posRef += 2;
    nd->negRef += 2;
  } else if (!sign) { /* Positive */
    if (cd->pol >= 0) {
      nd->posRef += 1;
    }
    if (cd->pol <= 0) {
      nd->negRef += 1;
    }
  } else { /* Negated */
    if (cd->pol >= 0) {
      nd->negRef += 1;
    }
    if (cd->pol <= 0) {
      nd->posRef += 1;
    }
  }

  /* If this node is an ITE, or has polarity zero and is an IFF, we
     need to record that for later. Set a magic value in f->iRef.
     NOTE: magic value is reset for ITE nodes once the IF branch has
           been processed (see CnfCompactPolFirstBack) */
  if ((f->symbol == RBCIFF && cd->pol == 0) || (f->symbol == RBCITE)) {
    f->iRef -= 10;
  }

  return result;
} /* End of CnfCompactPolSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit and BackVisit for CNF conversion.]

  Description [Dfs FirstVisit and BackVisit for CNF conversion polarity
               computation.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void CnfCompactPolFirstBack(Rbc_t* f, char* cnfData, nusmv_ptrint sign)
{
  CnfCompactDfsData_t* cd = (CnfCompactDfsData_t*)cnfData;

  /* Save polarity and check for zero-polarity IFFs, or for ITEs */
  if (f->iRef < -1) {
    if (f->symbol == RBCITE) {
    /* For ITE nodes, the IF branch has to be considered for both polarities,
       whereas the other branches only have to be considered for the polarity
       of the ITE node itself */
      f->iRef += 10;  /* Reset polarity for THEN and ELSE branches */
      cd->pol = 0;    /* IF branch has to be considered in both polarities */

     /* If this is a zero-polarity ITE, then the IF branch will get referencd
         twice for each polarity - the same as for zero-polarity IFF children */
      if (f->iRef == 0) {
        cd->zeroiff = true;
      }

    } else {
      cd->pol = 0;
      cd->zeroiff = true;
    }
  } else {
    cd->pol = f->iRef;
    cd->zeroiff = false;
  }

} /* End of CnfCompactPolFirstBack. */


/**Function********************************************************************

  Synopsis    [Dfs Set for CNF conversion.]

  Description [Dfs Set for CNF conversion.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int CnfCompactSet(Rbc_t* f, char* cnfData, nusmv_ptrint sign)
{
  CnfCompactDfsData_t* cd = (CnfCompactDfsData_t*) cnfData;
  CnfCompactDfsNode_t* nd = (CnfCompactDfsNode_t*) (f->gRef);

  /* The per-node data is intact */
  nusmv_assert(nd != NULL);

  /* This function is called at each node to decide whether to visit
     it. We also use it to manage the reference counted data
     structures. */

  if (nd->unseen) {
    return -1; /* Visit the node */
  }

  /* Point to the results from this clause so that they can be used by
     the parent node */
  if (sign) {
    cd->posClauses = nd->negClauses;
    cd->negClauses = nd->posClauses;
  } else {
    cd->posClauses = nd->posClauses;
    cd->negClauses = nd->negClauses;
  }

  return 1; /* Don't visit this node */

} /* End of CnfCompactSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for CNF conversion.]

  Description [Dfs FirstVisit for CNF conversion.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void CnfCompactFirst(Rbc_t* f, char* cnfData, nusmv_ptrint sign)
{
  CnfCompactDfsNode_t* nd = (CnfCompactDfsNode_t*) (f->gRef);

  nusmv_assert(nd->unseen);
} /* End of CnfCompactFirst. */


/**Function********************************************************************

  Synopsis    [Dfs BackVisit for CNF conversion.]

  Description [Dfs BackVisit for CNF conversion.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void CnfCompactBack(Rbc_t* f, char* cnfData, nusmv_ptrint sign)
{
  CnfCompactDfsData_t* cd = (CnfCompactDfsData_t*) cnfData;
  CnfCompactDfsNode_t* nd = (CnfCompactDfsNode_t*) (f->gRef);

  /* Store copies of the LHS clause sets */
  if (f->symbol != RBCVAR) {
    if (nd->posClauses == NULL && nd->negClauses == NULL) {
      nd->posClauses = cd->posClauses;
      nd->negClauses = cd->negClauses;
    }
    else if (f->symbol == RBCITE && nd->ifClauses == NULL) {
      /* Both nd->posClauses and nd->negClauses should be non-null, since
         the IF branch will have been considered for both polarities */
      nusmv_assert(nd->posClauses);
      nusmv_assert(nd->negClauses);

      /* Save positive IF branch */
      nd->ifClauses = nd->posClauses;

      /* Save (~IF|THEN) and/or (~IF|~THEN) based upon reference counts */
      if (nd->posRef > 0) {
        disjunction(&(nd->negClauses), &(cd->posClauses), &(cd->maxVar),
                    &(cd->clauses), cd->rbcManager);
        nd->posClauses = Clg_Disj(nd->negClauses, cd->posClauses);
      } else nd->posClauses = NULL;
      if (nd->negRef > 0) {
        disjunction(&(nd->negClauses), &(cd->negClauses), &(cd->maxVar),
                    &(cd->clauses), cd->rbcManager);
        nd->negClauses = Clg_Disj(nd->negClauses, cd->negClauses);
      } else nd->negClauses = NULL;
    }
  }
} /* End of CnfCompactBack. */


/**Function********************************************************************

  Synopsis    [Dfs LastVisit for CNF conversion.]

  Description [Dfs LastVisit for CNF conversion.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void CnfCompactLast(Rbc_t* f, char* cnfData, nusmv_ptrint sign)
{
  int cnfVar = 0;  /* Equivalent CNF variable used for RBCVAR node */
  CnfCompactDfsData_t* cd = (CnfCompactDfsData_t*) cnfData;
  CnfCompactDfsNode_t* nd = (CnfCompactDfsNode_t*) (f->gRef);

  /* Aliases to improve readability */
  clause_graph leftPos  = nd->posClauses,
               leftNeg  = nd->negClauses,
               rightPos = cd->posClauses,
               rightNeg = cd->negClauses;

  clause_graph resultPos, resultNeg;

  /* This node has not been seen before */
  nusmv_assert(nd->unseen);

  /* Find, or generate, CNF variable for 'f' if it will be renamed */
  if ((nd->posRef > 1) || (nd->negRef > 1) || (f->symbol == RBCVAR)) {
    cnfVar = Rbc_get_node_cnf(cd->rbcManager, f, &(cd->maxVar));
  }

  switch (f->symbol) {
  case RBCVAR:
    /* Fill in vars list */
    Slist_push(cd->vars, PTR_FROM_INT(void*, cnfVar));

    resultPos = Clg_Lit(cnfVar);
    resultNeg = Clg_Lit(-cnfVar);
    break;

  case RBCAND: /* Conjunction or disjunction */
    if (nd->posRef > 0) {
      resultPos = Clg_Conj(leftPos, rightPos);
    }
    else resultPos = NULL;

    if (nd->negRef > 0) {
      disjunction(&leftNeg, &rightNeg, &(cd->maxVar), &(cd->clauses),
                  cd->rbcManager);
      resultNeg = Clg_Disj(leftNeg, rightNeg);
    }
    else resultNeg = NULL;
    break;

  case RBCIFF:
    if ((nd->posRef) > 0) {
      /* Positive polarity: compute (~a|b)&(a|~b) */
      disjunction2(&leftNeg, &rightPos, &leftPos, &rightNeg,
                   &(cd->maxVar), &(cd->clauses), cd->rbcManager);
      resultPos = Clg_Conj(Clg_Disj(leftNeg, rightPos),
                           Clg_Disj(leftPos, rightNeg));
    }
    else resultPos = NULL;

    if ((nd->negRef) > 0) {
      /* Negative polarity: compute (a|b)&(~a|~b) */
      disjunction2(&leftPos, &rightPos, &leftNeg, &rightNeg, 
                   &(cd->maxVar), &(cd->clauses), cd->rbcManager);
      resultNeg = Clg_Conj(Clg_Disj(leftPos, rightPos),
                           Clg_Disj(leftNeg, rightNeg));
    }
    else resultNeg = NULL;
    break;

  case RBCITE:
    /* The left side of the following conjunctions were saved in
       cd->posClauses and cd->negClauses respectively (provided that
       they were not NULL), and the positive clauses from the IF
       branch in cd->ifClauses */

    if (nd->posRef > 0) {
      /* Positive polarity: compute (~IF|THEN)&(IF|ELSE) */
      disjunction(&(nd->ifClauses), &(cd->posClauses), &(cd->maxVar),
                  &(cd->clauses), cd->rbcManager);

      resultPos = Clg_Conj(nd->posClauses, Clg_Disj(nd->ifClauses,
                                                    cd->posClauses));
    }
    else resultPos = NULL;

    if (nd->negRef > 0) {
      /* Negative polarity: compute (~IF|~THEN)&(IF|~ELSE) */
      disjunction(&(nd->ifClauses), &(cd->negClauses), &(cd->maxVar),
                  &(cd->clauses), cd->rbcManager);

      resultNeg = Clg_Conj(nd->negClauses, Clg_Disj(nd->ifClauses,
                                                    cd->negClauses));
    }
    else resultNeg = NULL;
    break;

  default:
    internal_error("rbcCnf: unexpected node %d\n", f->symbol);
    return;
  }

  /* Rename multiply-referenced vertices. Positive clauses are renamed
     with a positive CNF variable, and negative clauses are renamed
     with a negative CNF variable. This is done so that, with the
     incremental functions, there won't be a risk of the renaming
     variable for both sets having the same polarity, since there is
     no way of knowing which polarities of the subtree have been
     converted. */
  if ((nd->posRef) > 1) {
    nusmv_assert(0 != cnfVar);
    rename_clauses(&resultPos, cnfVar, &(cd->clauses));
  }
  if ((nd->negRef) > 1) {
    nusmv_assert(0 != cnfVar);
    rename_clauses(&resultNeg, -cnfVar, &(cd->clauses));
  }

  nusmv_assert((nd->posRef) == 0 || Clg_Size(resultPos) > 0);
  nusmv_assert((nd->negRef) == 0 || Clg_Size(resultNeg) > 0);

  /* Pass these results back up the graph */
  if (sign) {
    cd->posClauses = nd->negClauses = resultNeg;
    cd->negClauses = nd->posClauses = resultPos;
  }
  else {
    cd->posClauses = nd->posClauses = resultPos;
    cd->negClauses = nd->negClauses = resultNeg;
  }

  /* We've seen this node now */
  nd->unseen = false;

  return;

} /* End of CnfCompactLast. */



/**Function********************************************************************

  Synopsis    [Dfs Set for cleaning.]

  Description [Dfs Set for cleaning.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int CnfCompactCleanSet(Dag_Vertex_t* f, char* cleanData,
                              nusmv_ptrint sign)
{
  /* All the nodes are visited once and only once. */
  return 0;
} /* End of CleanSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for cleaning.]

  Description [Dfs FirstVisit for cleaning.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void CnfCompactCleanFirst(Dag_Vertex_t* f, char* cleanData,
                                 nusmv_ptrint sign)
{
  /* Clean data. */
  FREE(f->gRef);
  f->gRef = NIL(char);
  f->iRef = 0;
  return;
} /* End of CnfCompactCleanFirst. */


/**Function********************************************************************

  Synopsis    [Dfs empty function.]

  Description [Empty function as null operation during DFS]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void CnfEmpty(Rbc_t* f, char* cnfData, nusmv_ptrint sign)
{
  /* Nothing to do */
  return;
} /* End of CnfEmpty */


/**Function********************************************************************

  Synopsis    [Renames a set of clauses.]

  Description [Renames a set of clauses by adding -var to each clause
  and adding each clause to the list of saved clauses. Allocates a new variable
  if var==0. Returns var or the new variable. Refuses to rename a single
  clause; returns 0 in this case]

  SideEffects [clauses refers to the singleton clause set referring to the
  renamed clauses]

  SeeAlso     []

******************************************************************************/
static void rename_clauses(clause_graph* clauses, int var,
                           clause_graph* saved)
{
  clause_graph clause, lit;

  nusmv_assert(0 != var);

  lit = Clg_Lit(-var);
  clause = Clg_Disj(lit, *clauses);
  *saved = Clg_Conj(*saved, clause);
  *clauses = Clg_Lit(var);
} /* End of rename_clauses. */


/**Function********************************************************************

  Synopsis    [Compute the disjunction of two clause sets]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static inline void disjunction(clause_graph* Left, clause_graph* Right,
                               int* maxVar, clause_graph* clauses,
                               Rbc_Manager_t* rbcm)
{
  /* Ensure Left is largest */
  if (Clg_Size(*Left) < Clg_Size(*Right)) {
    clause_graph* temp = Right;
    Right = Left;
    Left = temp;
  }

  if (testSizes(*Right, *Left)) {
    int cnf;
    cnf = Rbc_get_node_cnf(rbcm, RBCDUMMY, maxVar);
    rename_clauses(Left, cnf, clauses);
#ifdef CNF_CONV_SP
    cnf = Rbc_get_node_cnf(rbcm, RBCDUMMY, maxVar);
    rename_clauses(Right, cnf, clauses);
#endif
  }
} /* End of disjunction */


/**Function********************************************************************

  Synopsis    [Compute the disjunction of two clause sets]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static inline void disjunction2(clause_graph* Left1, clause_graph* Right1,
                                clause_graph* Left2, clause_graph* Right2,
                                int* maxVar, clause_graph* clauses,
                                Rbc_Manager_t* rbcm)
{
  int var=0;

  /* Ensure Left is largest */
  if (Clg_Size(*Left1)+Clg_Size(*Left2) < Clg_Size(*Right1)+Clg_Size(*Right2)) {
    clause_graph* temp1 = Right1;
    clause_graph* temp2 = Right2;
    Right1 = Left1;
    Left1 = temp1;
    Right2 = Left2;
    Left2 = temp2;
  }

  if (testSizes(*Right1, *Left1)) {
    var = Rbc_get_node_cnf(rbcm, RBCDUMMY, maxVar);
    nusmv_assert(0 != var);
    rename_clauses(Left1, var, clauses);
  }
  if (testSizes(*Right2, *Left2)) {
    var = Rbc_get_node_cnf(rbcm, RBCDUMMY, maxVar);
    nusmv_assert(0 != var);
    rename_clauses(Left2, -var, clauses);
  }
} /* End of disjunction2 */

/**Function********************************************************************

  Synopsis    [Check whether two clause sets are big enough to require renaming]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static inline int testSizes(clause_graph left, clause_graph right)
{
  int l = Clg_Size(left);
  int r = Clg_Size(right);

#ifdef CNF_CONV_SP
  return 1;
#endif

  if (l>r) {int t = l; l=r; r=t;};

  if (l==2 && r >= 3) return 1;
  if (l > 2) return 1;

  return 0;
}


/**Function********************************************************************

  Synopsis    [Extracts the cnf from the CLG]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void CnfCompactCommit(void* data, int* cl, int size)
{
  if (1 <= size) {
    int i;
    Slist_ptr* clauses = (Slist_ptr*)data;
    int * clause = (int *)NULL;

    clause = ALLOC(int, size+1);
    nusmv_assert((int *)NULL != clause);

    for(i = size; i > 0; i--) {
      clause[size - i] = cl[i-1];
    }
    /* Clauses are terminated by the literal 0 */
    clause[size] = 0;

    Slist_push(*clauses, (void*)clause);
  }
}
