/**CFile***********************************************************************

  FileName    [rbcCnf.c]

  PackageName [rbc]

  Synopsis    [Conjunctive Normal Form (CNF) conversions.]

  Description [External functions included in this module:
                <ul>
                <li> <b>Rbc_Convert2Cnf()</b>
                <li> <b>Rbc_CnfVar2RbcIndex()</b>
                <li> <b>Rbc_RbcIndex2CnfVar()</b>
                <li> <b>Rbc_CnfConversionAlgorithm2Str()</b>
                <li> <b>Rbc_CnfConversionAlgorithmFromStr()</b>
                <li> <b>Rbc_CnfGetValidRbc2CnfAlgorithms()</b>
                </ul>]

  SeeAlso     []

  Author      [Armando Tacchella, Tommi Junttila, Marco Roveri, 
               Dan Sheridan and Gavin Keighren]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by University of Genova and FBK-irst.

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

#include <limits.h>

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#if NUSMV_HAVE_STRING_H
#include <string.h>
#endif

#include "rbc/rbcInt.h"
#include "utils/error.h" /* for internal_error */


static char rcsid[] UTIL_UNUSED = "$Id: rbcCnf.c,v 1.3.6.3.2.5.2.5.4.7 2010-02-18 10:00:03 nusmv Exp $";

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

  Synopsis    [Translates the rbc into the corresponding (equisatisfiable)
               set of clauses.]

  Description [This calls the user's choice of translation procedure]

  SideEffects [`clauses' and `vars' are filled up. `clauses' is the empty
               list if `f' was true, and contains a single empty clause if
               `f' was false. 'polarity' is used to determine if the clauses
               generated should represent the RBC positively, negatively, or
               both (1, -1 or 0 respectively). For an RBC that is known to be
               true, the clauses that represent it being false are not needed
               (they would be removed anyway by propogating the unit literal
               which states that the RBC is true). Similarly for when the RBC
               is known to be false. This parameter is only used with the
               compact cnf conversion algorithm, and is ignored if the simple
               algorithm is used.]

  SeeAlso     []

******************************************************************************/
int Rbc_Convert2Cnf(Rbc_Manager_t* rbcManager, Rbc_t* f,
                    int polarity, Slist_ptr clauses, Slist_ptr vars,
                    int* literalAssignedToWholeFormula)
{
  int result;
  int i, maxVar;

  *literalAssignedToWholeFormula = INT_MAX;

  /* Handling special cases: f is the constant true or false. */
  if (f == Rbc_GetOne(rbcManager)) {
    return 0;
  }
  if (f == Rbc_GetZero(rbcManager)) {
    /* The empty clause: 0 is used as clause terminator */
    int * fClause = ALLOC(int, 1);
    fClause[0] = 0;
    Slist_push(clauses, (void*) fClause);
    return 0;
  }

  /* Determine the current maximum variable index. */
  maxVar = 0;
  for (i = rbcManager->varCapacity - 1; i >= 0; --i) {
    if (rbcManager->varTable[i] != NIL(Rbc_t)) { maxVar = i; break; }
  }

 /* check whether maxUnchangedRbcVariable can be extended (or whether
     indexes above the maxUnchangedRbcVariable have not been used) */
  if ((rbcManager->maxUnchangedRbcVariable == rbcManager->maxCnfVariable) &&
      (rbcManager->maxUnchangedRbcVariable < maxVar)) {
    rbcManager->maxUnchangedRbcVariable = maxVar;
    rbcManager->maxCnfVariable = maxVar;
  }

  /* Cleaning the user fields. */
  Dag_Dfs(f, &dag_DfsClean, NIL(char));


  /*     [MR2??]: moved up passign argument to the functions */
  switch(get_rbc2cnf_algorithm(OptsHandler_get_instance())) {
  case RBC_TSEITIN_CONVERSION:
    result = Rbc_Convert2CnfSimple(rbcManager, f, clauses, vars,
                                   literalAssignedToWholeFormula);
    break;
  case RBC_SHERIDAN_CONVERSION:
    result = Rbc_Convert2CnfCompact(rbcManager, f, polarity, clauses, vars,
                                    literalAssignedToWholeFormula);
    break;
  default:
    fprintf(nusmv_stderr, 
            "%s: No RBC2CNF conversion algorithm has been enabled.\n",
            __func__);
    error_unreachable_code();
    break;
  }

  return result;
}


/**Function********************************************************************

  Synopsis    [Returns the RBC index corresponding to a particular CNF var]

  Description [Returns -1, if there is no original RBC variable
  corresponding to CNF variable, this may be the case if CNF variable
  corresponds to an internal node (not leaf) of RBC tree. Input CNF
  variable should be a correct variable generated by RBC manager.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Rbc_CnfVar2RbcIndex(Rbc_Manager_t* rbcManager, int cnfVar)
{
  Rbc_t* rbcNode;

  rbcNode = (Rbc_t*) find_assoc(rbcManager->cnfVar2rbcNode_model,
                                NODE_FROM_INT(cnfVar));

  if ((Rbc_t*) NULL == rbcNode) {
    rbcNode = (Rbc_t*) find_assoc(rbcManager->cnfVar2rbcNode_cnf,
                                NODE_FROM_INT(cnfVar));
  }
  
  /* [MP] this is no longer true, as CNF variables may get lost upon resets */
  /* /\* there is always a corresponding RBC node for every (valid) CNF variable *\/ */
  /* nusmv_assert((Rbc_t*) NULL != rbcNode); */
  if ((Rbc_t*) NULL == rbcNode) return -1;

  /* Dummy nodes are artificially added by rbcCnfCompact */
  if (Dag_VertexGetRef(rbcNode) == RBCDUMMY) return -1;

  return Rbc_GetVarIndex(rbcNode);
}


/**Function********************************************************************

  Synopsis    [Returns the associated CNF variable of a given RBC index]

  Description [Returns 0, if there is no original RBC variable
  corresponding to CNF variable. This may be the case if particular RBC
  node (of the given variable) has never been converted into CNF]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Rbc_RbcIndex2CnfVar(Rbc_Manager_t* rbcManager, int rbcIndex)
{
  Rbc_t* rbcNode;
  int var;

  nusmv_assert(rbcIndex >= 0);

  rbcNode = Rbc_GetIthVar(rbcManager, rbcIndex);

  /* mode vars cache */
  var = NODE_TO_INT(find_assoc(rbcManager->rbcNode2cnfVar_model,
                               (node_ptr) Dag_VertexGetRef(rbcNode)));

  if (0 == var) { /* cnf cache as a fallback */
      var = NODE_TO_INT(find_assoc(rbcManager->rbcNode2cnfVar_cnf,
                                   (node_ptr) Dag_VertexGetRef(rbcNode)));
  }

  /* if there is no associated cnf var => 0 is returned automatically */
  return var;
}


/**Function********************************************************************

  Synopsis    [Given a rbc node, this function returns the corrensponding
  CNF var if it had been already allocated one. Otherwise it will allocate a
  new CNF var and will increment given maxvar value. If f is RBCDUMMY,
  a new variable will be always allocated (intended to be a non-terminal var,
  but a corresponding RBC var will be not allocated)]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Rbc_get_node_cnf(Rbc_Manager_t* rbcm, Rbc_t* f, int* maxvar)
{
  int var;
  boolean is_model = true;

  /* try to fetch model var */
  var = NODE_TO_INT(find_assoc(rbcm->rbcNode2cnfVar_model,
                         (node_ptr) Dag_VertexGetRef(f)));

  /* try to fetch cnf var as a fallback */
  if (0 == var) {
    var = NODE_TO_INT(find_assoc(rbcm->rbcNode2cnfVar_cnf,
                                 (node_ptr) Dag_VertexGetRef(f)));
  }

  /* if there is no associated cnf var => create it and associate with
     rbc node*/
  if (0 == var) {

    /* special case: if this is a var node and rbc var is less then
       'max unchanged rbc var' then its cnf index will be the same as
       rbc index. This can occur only when the index is not zero. */
    if ((Dag_VertexGetRef(f) != RBCDUMMY) && (f->symbol == RBCVAR) &&
        (PTR_TO_INT(f->data) <=  rbcm->maxUnchangedRbcVariable) &&
        (0 != PTR_TO_INT(f->data))) {

      /* model var */
      var = PTR_TO_INT(f->data);
    }
    else { /* otherwise just generate a new cnf var */
      is_model = false;
      var = ++(*maxvar);
    }

    /* make the association: RBC node -> CNF var */
    if (Dag_VertexGetRef(f) != RBCDUMMY) {
      insert_assoc(is_model ? rbcm->rbcNode2cnfVar_model : rbcm->rbcNode2cnfVar_cnf,
                   (node_ptr) Dag_VertexGetRef(f),
                   NODE_FROM_INT(var));
    }

    /* make the association: CNF var -> RBC node */
    insert_assoc(is_model ? rbcm->cnfVar2rbcNode_model :  rbcm->cnfVar2rbcNode_cnf,
                 NODE_FROM_INT(var),
                 (node_ptr) Dag_VertexGetRef(f));
  }

  nusmv_assert(0 != var);
  return var;
}

/**Function********************************************************************

  Synopsis    [Conversion from string to CNF conversion algorithm enumerative]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Rbc_2CnfAlgorithm Rbc_CnfConversionAlgorithmFromStr(const char * str) {
  if (strcmp(str, RBC_TSEITIN_CONVERSION_NAME) == 0)
    return RBC_TSEITIN_CONVERSION;
  if (strcmp(str, RBC_SHERIDAN_CONVERSION_NAME) == 0)
    return RBC_SHERIDAN_CONVERSION;
  return RBC_INVALID_CONVERSION;
}


/**Function********************************************************************

  Synopsis    [Conversion from CNF conversion algorithm enumerative to string]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
const char * Rbc_CnfConversionAlgorithm2Str(Rbc_2CnfAlgorithm algo) {
  char * result;
  switch (algo) {
  case RBC_TSEITIN_CONVERSION:
    result = RBC_TSEITIN_CONVERSION_NAME;
    break;
  case RBC_SHERIDAN_CONVERSION:
    result = RBC_SHERIDAN_CONVERSION_NAME;
    break;
  default:
    result = RBC_INVALID_CONVERSION_NAME;
    break;
  }
  return result;
}

/**Function********************************************************************

  Synopsis    [String of valid conversion algorithms]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
const char * Rbc_CnfGetValidRbc2CnfAlgorithms() {
  return RBC_TSEITIN_CONVERSION_NAME ", " \
    RBC_SHERIDAN_CONVERSION_NAME;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
