/**CFile***********************************************************************

  FileName    [beCnf.c]

  PackageName [be]

  Synopsis    [Conjunctive Normal Form of boolean extpressions]

  Description [This module defines the Be_Cnf structure and any related
  method. When converting a be into cnf form the Be_ConvertToCnf function
  returns a Be_Cnf structure. The Be_Cnf structure is a base class for the
  structure Bmc_Problem.]

  SeeAlso     [Be_ConvertToCnf, Bmc_Problem]

  Author      [Roberto Cavada, Marco Roveri, Michele Dorigatti]

  Copyright   [
  This file is part of the ``be'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst and University of Trento.
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

#include "be.h"
#include "utils/assoc.h"
#include "parser/symbols.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/**Struct**********************************************************************

  Synopsis    [Private definition for Boolean Expression in CNF form]

  Description [In order to use a Be_Cnf instance see the Be_Cnf_ptr type.]

  SeeAlso     [Be_Cnf_ptr]

******************************************************************************/
typedef struct Be_Cnf_TAG {
  be_ptr originalBe; /* the original BE problem */
  Slist_ptr cnfVars;    /* The list of CNF variables */
  Slist_ptr cnfClauses; /* The list of CNF clauses */
  int    cnfMaxVarIdx;  /* The maximum CNF variable index */

  /* literal assigned to whole CNF formula. (It may be negative)
     If the formula is a constant, see Be_Cnf_ptr. */
  int formulaLiteral;
} Be_Cnf;


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Declarations of internal functions                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void _be_cnf_destroy_clause ARGS((void* data));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Constructor for the Be_Cnf structure]

  Description [When the returned pointer is no longer used,
  call Be_Cnf_Delete]

  SideEffects []

  SeeAlso     [Be_Cnf_Delete]

******************************************************************************/
Be_Cnf_ptr Be_Cnf_Create(const be_ptr be)
{
  Be_Cnf_ptr self = ALLOC(Be_Cnf, 1);
  nusmv_assert(self != NULL);

  self->originalBe = be;
  self->cnfVars = Slist_create();
  self->cnfClauses = Slist_create();
  self->cnfMaxVarIdx = 0;
  self->formulaLiteral = 0;

  return self;
}

/**Function********************************************************************

  Synopsis    [Be_Cnf structure destructor]

  Description []

  SideEffects []

  SeeAlso     [Be_Cnf_Create]

******************************************************************************/
void Be_Cnf_Delete(Be_Cnf_ptr self)
{
  nusmv_assert(self != NULL);

  Slist_destroy_and_free_elements(self->cnfClauses, _be_cnf_destroy_clause);
  Slist_destroy(self->cnfVars);

  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Removes any duplicate literal appearing in single clauses]

  Description [Removes any duplicate literal appearing in single clauses]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_Cnf_RemoveDuplicateLiterals(Be_Cnf_ptr self)
{
  int i, j;
  Siter iter;
  int * clause = (int *)NULL;
  hash_ptr lits = (hash_ptr)NULL;

  nusmv_assert(self != NULL);

  lits = new_assoc();

  for (iter = Slist_first(Be_Cnf_GetClausesList(self));
       !Siter_is_end(iter);
       iter = Siter_next(iter)) {

    clause = (int*) Siter_element(iter);

    i = 0;
    while (clause[i] != 0) {
      if (Nil != find_assoc(lits, NODE_FROM_INT(clause[i]))) {
        j = i+1;
        while (clause[j] != 0) {
          clause[j-1] = clause[j];
          j++;
        }
      }
      else {
        insert_assoc(lits, NODE_FROM_INT(clause[i]), NODE_FROM_INT(1));
      }
      i++;
    }

    /* this clear the hash */
    clear_assoc(lits);
  }

  free_assoc(lits);
}

/**Function********************************************************************

  Synopsis    [Returns the original BE problem this CNF was created from]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
be_ptr Be_Cnf_GetOriginalProblem(const Be_Cnf_ptr self)
{
  return self->originalBe;
}


/**Function********************************************************************

  Synopsis    [Returns the literal assigned to the whole formula.
  It may be negative. If the formula is a constant unspecified value is returned]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Be_Cnf_GetFormulaLiteral(const Be_Cnf_ptr self)
{
  return self->formulaLiteral;
}

/**Function********************************************************************

  Synopsis    [Returns the independent variables list in the CNF
  representation]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Slist_ptr Be_Cnf_GetVarsList(const Be_Cnf_ptr self) { return self->cnfVars; }


/**Function********************************************************************

  Synopsis    [Returns a list of lists which contains the CNF-ed formula]

  Description [Each list in the list is a set of integers which
  represents a single clause. Any integer value depends on the variable
  name and the time which the variasble is considered in, whereas the
  integer sign is the variable polarity in the CNF-ed representation.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Slist_ptr Be_Cnf_GetClausesList(const Be_Cnf_ptr self)
{
  return self->cnfClauses;
}


/**Function********************************************************************

  Synopsis    [Returns the maximum variable index in the list of clauses]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Be_Cnf_GetMaxVarIndex(const Be_Cnf_ptr self)
{
  return self->cnfMaxVarIdx;
}


/**Function********************************************************************

  Synopsis    [Returns the number of independent variables in the given
  Be_Cnf structure]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
size_t Be_Cnf_GetVarsNumber(const Be_Cnf_ptr self)
{
  return Slist_get_size(Be_Cnf_GetVarsList(self));
}


/**Function********************************************************************

  Synopsis    [Returns the number of clauses in the given Be_Cnf structure]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
size_t Be_Cnf_GetClausesNumber(const Be_Cnf_ptr self)
{
  return Slist_get_size(Be_Cnf_GetClausesList(self));
}


/**Function********************************************************************

  Synopsis    [Sets the literal assigned to the whole formula]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_Cnf_SetFormulaLiteral(Be_Cnf_ptr self, const int  formula_literal)
{
  self->formulaLiteral =  formula_literal;
}

/**Function********************************************************************

  Synopsis    [Sets the maximum variable index value]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_Cnf_SetMaxVarIndex(Be_Cnf_ptr self, const int max_idx)
{
  self->cnfMaxVarIdx = max_idx;
}

/**Function********************************************************************

  Synopsis    [Print out some statistics]

  Description [Print out, in this order: the clause number, the var number, the
               highest variable index, the average clause size, the highest
               clause size]

  SideEffects ["outFile" is written]

  SeeAlso     []

******************************************************************************/
void Be_Cnf_PrintStat(const Be_Cnf_ptr self, FILE* outFile, char* prefix)
{
  /* compute values */
  int max_clause_size = 0;
  float sum_clause_size = 0;
  Siter cnf;

  nusmv_assert(self != (Be_Cnf_ptr)NULL);

  SLIST_FOREACH(Be_Cnf_GetClausesList(self), cnf) {
    SLIST_CHECK_INSTANCE(Be_Cnf_GetClausesList(self));

    int* clause = (int*)Siter_element(cnf);
    int clause_size;

    for (clause_size = 0; clause[clause_size] != 0; ++clause_size) { }

    sum_clause_size += clause_size;
    if (clause_size > max_clause_size) max_clause_size = clause_size;
  }

  /* print out values */
    fprintf(outFile,
          "%s Clause number: %i\n"
          "%s Var number: %i\n"
          "%s Max var index: %i\n"
          "%s Average clause size: %.2f\n"
          "%s Max clause size: %i\n",
          prefix,
          (int)Be_Cnf_GetClausesNumber(self),
          prefix,
          (int)Be_Cnf_GetVarsNumber(self),
          prefix,
          Be_Cnf_GetMaxVarIndex(self),
          prefix,
          /* the average clause size */
          sum_clause_size / (float)Slist_get_size(Be_Cnf_GetClausesList(self)),
          prefix,
          max_clause_size);
}



/**Function********************************************************************

  Synopsis    [Frees the array used to store the clause.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void _be_cnf_destroy_clause(void* data) {
  int * _data = (int *)data;

  FREE(_data);
}
/**AutomaticEnd***************************************************************/

