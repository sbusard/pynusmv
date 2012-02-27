/**CHeaderFile*****************************************************************

  FileName    [be.h]

  PackageName [be]

  Synopsis    [The header file for the <tt>be</tt> package]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``be'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst and University of Trento.

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

#ifndef _BE_H_
#define _BE_H_

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Be_Manager is a generic manager required when you must operate
  on Boolean Expressions]

  Description [Any instance of Be_Manager can only be accessed via
  Be_Manager_ptr]

  SeeAlso     []

******************************************************************************/
typedef struct Be_Manager_TAG* Be_Manager_ptr; /* generic be manger */


/**Struct**********************************************************************

  Synopsis    [A Boolean Expression represented in Conjunctive Normal Form]

  Description [Special case -- A CONSTANT: If the formula is a constant,
  Be_Cnf_GetFormulaLiteral() will be INT_MAX,
  if formula is true then:  GetClausesList() will be empty list.
  if formula is false then:  GetClausesList()  will contain a single 
  empty clause.]


  SeeAlso     []

******************************************************************************/
typedef struct Be_Cnf_TAG* Be_Cnf_ptr; /* cnf representation */


/**Struct**********************************************************************

  Synopsis    [The Boolean Expression type]

  Description []

  SeeAlso     []

******************************************************************************/
typedef void* be_ptr;


/**Struct**********************************************************************

  Synopsis    [Specific to generic BE conversion gateway type]

  Description [This is the function type for the Be_Manager gateway that
  provides conversion functionality from specific boolean expression types
  to generic Boolean Expression type (for example from rbc to be_ptr).]

  SeeAlso     []

******************************************************************************/
typedef be_ptr (*Be_Spec2Be_fun)(Be_Manager_ptr self, void* spec_be);


/**Struct**********************************************************************

  Synopsis    [Generic to specific BE conversion gateway type]

  Description [This is the function type for the Be_Manager gateway that
  provides conversion functionality from generic boolean expression types
  to specific Boolean Expression type (for example from be_ptr to rbc).]

  SeeAlso     []

******************************************************************************/
typedef void*  (*Be_Be2Spec_fun)(Be_Manager_ptr self, be_ptr be);


#include <limits.h>
/* ================================================== */
/* Put here any specific boolean expression manager
   interface header: */
#include "beRbcManager.h"
/* ================================================== */

#include "utils/utils.h" /* for EXTERN and ARGS */
#include "utils/Slist.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Constant***********************************************************************

  Synopsis [Represents an invalid value possibly used when
  substituting.]

  Description [If during substitution this value is found, an
  error is raised. This is used for runtime checking of those
  expressions that are not expected to contain variables that
  cannot be substituted.]

  SeeAlso     []

******************************************************************************/
extern const int BE_INVALID_SUBST_VALUE;

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

/* ================================================== */
/* Package constructor/destructor: */
EXTERN void Be_Init ARGS((void));
EXTERN void Be_Quit ARGS((void));
/* ================================================== */


/* ================================================== */
/* Be_Manager public interface: */
EXTERN  be_ptr Be_Manager_Spec2Be ARGS((const Be_Manager_ptr self,
                                        void* spec_expr));
EXTERN  void* Be_Manager_Be2Spec ARGS((const Be_Manager_ptr self, be_ptr be));
EXTERN void* Be_Manager_GetSpecManager ARGS((Be_Manager_ptr self));
/* ================================================== */


/* ==================================================  */
/* Be_Cnf class constructor, destructor and modifiers */
EXTERN Be_Cnf_ptr Be_Cnf_Create ARGS((const be_ptr be));
EXTERN void Be_Cnf_Delete ARGS((Be_Cnf_ptr self));
EXTERN void Be_Cnf_RemoveDuplicateLiterals ARGS((Be_Cnf_ptr self));
/* ==================================================  */

/* ================================================== */
/* Be_Cnf class access members: (for special case,a constant,see Be_Cnf_ptr)*/
EXTERN be_ptr Be_Cnf_GetOriginalProblem ARGS((const Be_Cnf_ptr self));

EXTERN int Be_Cnf_GetFormulaLiteral  ARGS((const Be_Cnf_ptr self));

EXTERN Slist_ptr Be_Cnf_GetVarsList        ARGS((const Be_Cnf_ptr self));

EXTERN Slist_ptr Be_Cnf_GetClausesList     ARGS((const Be_Cnf_ptr self));

EXTERN int Be_Cnf_GetMaxVarIndex     ARGS((const Be_Cnf_ptr self));

EXTERN size_t Be_Cnf_GetVarsNumber      ARGS((const Be_Cnf_ptr self));

EXTERN size_t Be_Cnf_GetClausesNumber   ARGS((const Be_Cnf_ptr self));

EXTERN void Be_Cnf_SetFormulaLiteral  ARGS((const Be_Cnf_ptr self,
                                                const int formula_literal));

EXTERN void Be_Cnf_SetMaxVarIndex     ARGS((const Be_Cnf_ptr self,
                                                const int max_idx));
/* ================================================== */


/* ================================================== */
/* BE logical operations interface: */
EXTERN boolean Be_IsTrue  ARGS((Be_Manager_ptr manager, be_ptr arg));
EXTERN boolean Be_IsFalse ARGS((Be_Manager_ptr manager, be_ptr arg));
EXTERN boolean Be_IsConstant ARGS((Be_Manager_ptr manager, be_ptr arg));
EXTERN be_ptr  Be_Truth   ARGS((Be_Manager_ptr manager));
EXTERN be_ptr  Be_Falsity ARGS((Be_Manager_ptr manager));
EXTERN be_ptr  Be_Not     ARGS((Be_Manager_ptr manager, be_ptr arg));
EXTERN be_ptr
Be_And ARGS((Be_Manager_ptr manager, be_ptr arg1, be_ptr arg2));

EXTERN be_ptr
Be_Or  ARGS((Be_Manager_ptr manager, be_ptr arg1, be_ptr arg2));

EXTERN be_ptr
Be_Xor ARGS((Be_Manager_ptr manager, be_ptr arg1, be_ptr arg2));

EXTERN be_ptr
Be_Implies ARGS((Be_Manager_ptr manager, be_ptr arg1, be_ptr arg2));

EXTERN be_ptr
Be_Iff ARGS((Be_Manager_ptr manager, be_ptr arg1, be_ptr arg2));

EXTERN be_ptr
Be_Ite     ARGS((Be_Manager_ptr manager, be_ptr arg_if,
                 be_ptr arg_then, be_ptr arg_else));

EXTERN be_ptr
Be_LogicalShiftVar  ARGS((Be_Manager_ptr manager, be_ptr f, 
                          int shift, 
                          const int* log2phy, 
                          const int* phy2log));

EXTERN be_ptr
Be_LogicalVarSubst ARGS((Be_Manager_ptr manager, be_ptr f, 
                         int* subst, 
                         const int* log2phy, 
                         const int* phy2log));


/* ================================================== */


/* ================================================== */
/* Utilities interface: */
EXTERN Be_Cnf_ptr 
Be_ConvertToCnf ARGS((Be_Manager_ptr manager, be_ptr f, int polarity));

EXTERN int Be_CnfLiteral2BeLiteral ARGS((const Be_Manager_ptr self,
                                         int cnfLiteral));

EXTERN int Be_BeLiteral2CnfLiteral ARGS((const Be_Manager_ptr self,
                                         int beLiteral));

EXTERN int Be_BeLiteral2BeIndex ARGS((const Be_Manager_ptr self,
                                      int beLiteral));

EXTERN int Be_BeIndex2BeLiteral ARGS((const Be_Manager_ptr self,
                                      int beIndex));

EXTERN int Be_BeIndex2CnfLiteral ARGS((const Be_Manager_ptr self,
                                       int beIndex));


EXTERN Slist_ptr Be_CnfModelToBeModel ARGS((Be_Manager_ptr manager,
                                            const Slist_ptr cnfModel));

EXTERN void
Be_DumpDavinci ARGS((Be_Manager_ptr manager, be_ptr f, FILE* outFile));

EXTERN void
Be_DumpGdl ARGS((Be_Manager_ptr manager, be_ptr f, FILE* outFile));


EXTERN void
Be_DumpSexpr   ARGS((Be_Manager_ptr manager, be_ptr f, FILE* outFile));

/* index<->be conversion layer: */
EXTERN be_ptr Be_Index2Var ARGS((Be_Manager_ptr manager, int varIndex));
EXTERN int Be_Var2Index    ARGS((Be_Manager_ptr manager, be_ptr var));

/* miscellaneous */
EXTERN boolean Be_CnfLiteral_IsSignPositive ARGS((const Be_Manager_ptr self,
                                                  int cnfLiteral));

EXTERN int Be_CnfLiteral_Negate ARGS((const Be_Manager_ptr self,
                                      int cnfLiteral));

EXTERN boolean Be_BeLiteral_IsSignPositive ARGS((const Be_Manager_ptr self,
                                                 int beLiteral));

EXTERN int Be_BeLiteral_Negate ARGS((const Be_Manager_ptr self,
                                     int beLiteral));

EXTERN be_ptr 
Be_apply_inlining ARGS((Be_Manager_ptr self, be_ptr f, boolean add_conj));

EXTERN void
Be_Cnf_PrintStat ARGS((const Be_Cnf_ptr self, FILE* outFile, char* prefix));

/* ================================================== */



/**AutomaticEnd***************************************************************/

#endif /* _BE_H_ */

