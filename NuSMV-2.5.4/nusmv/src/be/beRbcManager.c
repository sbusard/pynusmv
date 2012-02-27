/**CFile***********************************************************************

  FileName    [beRbcManager.c]

  PackageName [be]

  Synopsis    [Implementation for the RBC-based Boolean Expressions module. ]

  Description [This implementation is a wrapper for the RBC structure.]

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

#include "beRbcManager.h"
#include "be.h"
#include "beInt.h"

#include "rbc/rbc.h"
#include "rbc/InlineResult.h"

#include "opt/opt.h"
#include "utils/defs.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
const int BE_INVALID_SUBST_VALUE = RBC_INVALID_SUBST_VALUE;

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro********************************************************************

  Synopsis    [Given a be_manager returns the contained rbc manager.]

  Description [This is a macro which can be used to simplify the code.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
#define GET_RBC_MGR(be_manager) \
  (Rbc_Manager_t*) be_manager->spec_manager


/**Macro********************************************************************

  Synopsis    [Converts a rbc into a be]

  Description [This is a macro which can be used to simplify the code.]

  SideEffects []

  SeeAlso     [RBC]

******************************************************************************/
#define BE(be_manager, spec) \
  be_manager->be2spec_converter(be_manager, (void*)spec)


/**Macro********************************************************************

  Synopsis    [Converts a be into a rbc]

  Description [This is a macro which can be used to simplify the code.]

  SideEffects []

  SeeAlso     [BE]

******************************************************************************/
#define RBC(be_manager, be) \
  (Rbc_t*) be_manager->spec2be_converter(be_manager, be)



/*---------------------------------------------------------------------------*/
/* Declarations of internal functions                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void* beRbc_Be2Rbc(Be_Manager_ptr mgr, be_ptr be);
static be_ptr beRbc_Rbc2Be(Be_Manager_ptr mgr, void* rbc);


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates a rbc-specific Be_Manager]

  Description [You must call Be_RbcManager_Delete when the created instance
  is no longer used.]

  SideEffects []

  SeeAlso     [Be_RbcManager_Delete]

******************************************************************************/
Be_Manager_ptr Be_RbcManager_Create(const size_t capacity)
{
  Rbc_Manager_t* spec = Rbc_ManagerAlloc(capacity);
  Be_Manager_ptr self = Be_Manager_Create(spec, &beRbc_Rbc2Be, &beRbc_Be2Rbc);
  return self;
}


/**Function********************************************************************

  Synopsis    [Destroys the given Be_MAnager instance you previously
  created by using Be_RbcManager_Create]

  Description []

  SideEffects []

  SeeAlso     [Be_RbcManager_Create]

******************************************************************************/
void Be_RbcManager_Delete(Be_Manager_ptr self)
{
  Rbc_ManagerFree((Rbc_Manager_t*) self->spec_manager);
  Be_Manager_Delete(self);
}


/**Function********************************************************************

  Synopsis    [Changes the maximum number of variables the rbc manager can
  handle]

  Description []

  SideEffects [The given rbc manager will possibly change]

  SeeAlso     []

******************************************************************************/
void Be_RbcManager_Reserve(Be_Manager_ptr self, const size_t size)
{
  Rbc_ManagerReserve((Rbc_Manager_t*)self->spec_manager, size);
}


/**Function********************************************************************

  Synopsis    [Resets RBC cache]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_RbcManager_Reset (const Be_Manager_ptr self)
{
  Rbc_ManagerReset((Rbc_Manager_t*)self->spec_manager);
}

/**Function********************************************************************

  Synopsis           [Returns true if the given be is the true value,
  otherwise returns false]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Be_IsTrue(Be_Manager_ptr manager, be_ptr arg)
{
  return (arg == Be_Truth(manager))? true : false;
}


/**Function********************************************************************

  Synopsis           [Returns true if the given be is the false value,
  otherwise returns false]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Be_IsFalse(Be_Manager_ptr manager, be_ptr arg)
{
  return (arg == Be_Falsity(manager))? true : false;
}


/**Function********************************************************************

  Synopsis           [Returns true if the given be is a constant value,
  such as either False or True]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Be_IsConstant(Be_Manager_ptr manager, be_ptr arg)
{
  return (Be_IsTrue(manager, arg) || Be_IsFalse(manager, arg));
}


/**Function********************************************************************

  Synopsis           [Builds a 'true' constant value]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Truth(Be_Manager_ptr manager)
{
  return BE(manager, Rbc_GetOne(GET_RBC_MGR(manager)));
}


/**Function********************************************************************

  Synopsis           [Builds a 'false' constant value]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Falsity(Be_Manager_ptr manager)
{
  return BE(manager, Rbc_GetZero(GET_RBC_MGR(manager)));
}


/**Function********************************************************************

  Synopsis           [Negates its argument]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Not(Be_Manager_ptr manager, be_ptr left)
{
  return BE( manager, Rbc_MakeNot(GET_RBC_MGR(manager), RBC(manager, left)) );
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the conjunction between
  its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_And(Be_Manager_ptr manager, be_ptr left, be_ptr right)
{
  return BE(manager, Rbc_MakeAnd(GET_RBC_MGR(manager),
                                 RBC(manager, left),
                                 RBC(manager, right),
                                 RBC_TRUE));
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the disjunction of
  its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Or(Be_Manager_ptr manager, be_ptr left, be_ptr right)
{
  return BE(manager, Rbc_MakeOr(GET_RBC_MGR(manager),
                                RBC(manager, left),
                                RBC(manager, right),
                                RBC_TRUE));
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the exclusive-disjunction
  of its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Xor(Be_Manager_ptr manager, be_ptr left, be_ptr right)
{
  return BE(manager, Rbc_MakeXor(GET_RBC_MGR(manager),
                                 RBC(manager, left),
                                 RBC(manager, right),
                                 RBC_TRUE));
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the implication between
  its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Implies(Be_Manager_ptr manager, be_ptr arg1, be_ptr arg2)
{
  /* (a -> b) <-> !(a & !b) <-> (!a | b) */
  return Be_Or(manager, Be_Not(manager, arg1), arg2);
}


/**Function********************************************************************

  Synopsis           [Builds a new be which is the logical equivalence
  between its two arguments]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr Be_Iff(Be_Manager_ptr manager, be_ptr left, be_ptr right)
{
  return BE(manager, Rbc_MakeIff(GET_RBC_MGR(manager),
                                 RBC(manager, left),
                                 RBC(manager, right),
                                 RBC_TRUE));
}


/**Function********************************************************************

  Synopsis           [Builds an if-then-else operation be]

  Description        []

  SideEffects        [...]

  SeeAlso            []

******************************************************************************/
be_ptr Be_Ite(Be_Manager_ptr manager, be_ptr c, be_ptr t, be_ptr e)
{
  return BE( manager, Rbc_MakeIte(GET_RBC_MGR(manager),
                                  RBC(manager, c),
                                  RBC(manager, t),
                                  RBC(manager, e),
                                  RBC_TRUE) );
}


/**Function********************************************************************

  Synopsis    [Creates a fresh copy G(X') of the be F(X) by shifting
  each variable index of a given amount]

  Description [Shifting operation replaces each occurence of the
               variable x_i in `f' with the variable x_(i + shift).  A
               simple lazy mechanism is implemented to optimize that
               cases which given expression is a constant in.

               The two indices arrays log2phy and phy2log map
               respectively the logical level to the physical level,
               and the physical level to the logical levels. They
               allow the be encoder to freely organize the variables
               into a logical and a physical level. This feature has
               been introduced with NuSMV-2.4 that ships dynamic
               encodings.

               !!!! WARNING !!!!
                 Since version 2.4 memoizing has been moved to BeEnc,
                 as there is no way of calculating a good hashing key
                 as the time would be requested, but timing
                 information are not available at this stage.
               ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
be_ptr Be_LogicalShiftVar(Be_Manager_ptr manager, be_ptr f, int shift,
                          const int* log2phy, const int* phy2log)
{
  /* lazy evaluation: */
  if (Be_IsConstant(manager, f)) return f;

  return BE(manager,
            Rbc_LogicalShift(GET_RBC_MGR(manager),
                             RBC(manager, f), shift,
                             log2phy, phy2log));
}


/**Function********************************************************************

  Synopsis    [Replaces all variables in f with other variables, taking
               them at logical level]

  Description [Replaces every occurence of the variable x_i in in `f'
               with the variable x_j provided that subst[i] = j.

               Notice that in this context, 'i' and 'j' are LOGICAL
               indices, not physical, i.e. the substitution array is
               provided in terms of logical indices, and is related
               only to the logical level.

               For a substitution at physical level, see Be_VarSubst.

               There is no need for `subst' to contain all the
               variables, but it should map at least the variables in
               `f' in order for the substitution to work properly.

               The two indices arrays log2phy and phy2log map
               respectively the logical level to the physical level,
               and the physical level to the logical levels. They
               allow the be encoder to freely organize the variables
               into a logical and a physical level. This feature has
               been introduced with NuSMV-2.3 that ships dynamic
               encodings.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
be_ptr Be_LogicalVarSubst(Be_Manager_ptr manager, be_ptr f, int* subst,
                          const int* log2phy, const int* phy2log)
{
  return BE( manager, Rbc_LogicalSubst(GET_RBC_MGR(manager), RBC(manager, f),
                                       subst, log2phy, phy2log) );
}


/**Function********************************************************************

  Synopsis    [Converts the given be into the corresponding CNF-ed be]

  Description [Since it creates a new Be_Cnf structure, the caller
  is responsible for deleting it when it is no longer used
  (via Be_Cnf_Delete).

  'polarity' is used to determine if the clauses generated should
   represent the RBC positively, negatively, or both (1, -1 or 0
   respectively). For an RBC that is known to be true, the clauses
   that represent it being false are not needed (they would be removed
   anyway by propogating the unit literal which states that the RBC is
   true). Similarly for when the RBC is known to be false. This
   parameter is only used with the compact cnf conversion algorithm,
   and is ignored if the simple algorithm is used.]

  SideEffects []

  SeeAlso     [Be_Cnf_Delete]

******************************************************************************/
Be_Cnf_ptr Be_ConvertToCnf(Be_Manager_ptr manager, be_ptr f, int polarity)
{
  Be_Cnf_ptr cnf;
  int max_var_idx;
  int literalAssignedToWholeFormula = INT_MIN;

  /* performs the cnf conversion: */
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "\nConverting the BE problem into CNF problem...\n");
  }

  cnf = Be_Cnf_Create(f);
  max_var_idx = Rbc_Convert2Cnf(GET_RBC_MGR(manager),
                                RBC(manager, f),
                                polarity,
                                Be_Cnf_GetClausesList(cnf),
                                Be_Cnf_GetVarsList(cnf),
                                &literalAssignedToWholeFormula);

  nusmv_assert(literalAssignedToWholeFormula >= INT_MIN);

  Be_Cnf_RemoveDuplicateLiterals(cnf);

  Be_Cnf_SetMaxVarIndex(cnf, max_var_idx);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, " Conversion returned maximum variable index = %d\n",
            Be_Cnf_GetMaxVarIndex(cnf));
    fprintf(nusmv_stderr, " Length of list of clauses = %" PRIuPTR "\n",
            Be_Cnf_GetClausesNumber(cnf));
    fprintf(nusmv_stderr, " Length of list of variables = %" PRIuPTR "\n",
            Be_Cnf_GetVarsNumber(cnf));
  }

  Be_Cnf_SetFormulaLiteral(cnf, literalAssignedToWholeFormula);
  return cnf;
}


/**Function********************************************************************

  Synopsis    [Converts the given CNF model into BE model]

  Description [Since it creates a new lsit , the caller
  is responsible for deleting it when it is no longer used
  (via lsDestroy)]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Slist_ptr Be_CnfModelToBeModel(Be_Manager_ptr manager, Slist_ptr cnfModel)
{
  Slist_ptr beModel = Slist_create();
  nusmv_ptrint cnfLiteral, beLiteral;
  Siter iter;

  SLIST_FOREACH(cnfModel, iter) {
    cnfLiteral = (nusmv_ptrint) Siter_element(iter);

    beLiteral = (nusmv_ptrint) Be_CnfLiteral2BeLiteral(manager, cnfLiteral);
    /* if there is corresponding rbc variable => remember it */
    if (0 != beLiteral) {
      Slist_push(beModel, (void*)beLiteral);
    }
  }

  return beModel;
}


/**Function********************************************************************

  Synopsis    [Converts a CNF literal into a BE literal]

  Description [The function returns 0 if there is no BE index
  associated with the given CNF index.  A given CNF literal should be
  created by given BE manager (through Be_ConvertToCnf).]

  SideEffects []

  SeeAlso     [Be_ConvertToCnf]

******************************************************************************/
int Be_CnfLiteral2BeLiteral(const Be_Manager_ptr self, int cnfLiteral)
{
  int cnfIndex;
  int rbcIndex;

  /* literal is always != 0, otherwise the sign cannot be represented. */
  nusmv_assert(0 != cnfLiteral);

  cnfIndex = abs(cnfLiteral);
  rbcIndex = Rbc_CnfVar2RbcIndex(GET_RBC_MGR(self), cnfIndex);

  if (-1 != rbcIndex) return (cnfLiteral > 0) ? (rbcIndex+1) : (-rbcIndex-1);
  else return 0;
}


/**Function********************************************************************

  Synopsis    [Converts a BE literal into a CNF literal (sign is taken into
  account)]

  Description []

  SideEffects []

  SeeAlso     [Be_ConvertToCnf]

******************************************************************************/
int Be_BeLiteral2CnfLiteral(const Be_Manager_ptr self, int beLiteral)
{
  int be_idx = Be_BeLiteral2BeIndex(self, beLiteral);
  return (beLiteral > 0) ?
    Be_BeIndex2CnfLiteral(self, be_idx) :
    -Be_BeIndex2CnfLiteral(self, be_idx);
}



/**Function********************************************************************

  Synopsis    [Converts a BE literal into a CNF literal]

  Description []

  SideEffects []

  SeeAlso     [Be_ConvertToCnf]

******************************************************************************/
int Be_BeLiteral2BeIndex(const Be_Manager_ptr self, int beLiteral)
{
  nusmv_assert(beLiteral != 0);
  return abs(beLiteral)-1;
}


/**Function********************************************************************

  Synopsis    [Converts a BE index into a BE literal (always positive)]

  Description []

  SideEffects []

  SeeAlso     [Be_ConvertToCnf]

******************************************************************************/
int Be_BeIndex2BeLiteral(const Be_Manager_ptr self, int beIndex)
{
  nusmv_assert(beIndex >= 0);
  return beIndex+1;
}


/**Function********************************************************************

  Synopsis [Returns a CNF literal (always positive) associated with a
  given BE index]

  Description [If no CNF index is associated with a given BE index, 0
  is returned. BE indexes are associated with CNF indexes through
  function Be_ConvertToCnf.  ]

  SideEffects []

  SeeAlso     [Be_ConvertToCnf]

******************************************************************************/
int Be_BeIndex2CnfLiteral(const Be_Manager_ptr self, int beIndex)
{
  return Rbc_RbcIndex2CnfVar(GET_RBC_MGR(self), beIndex);
}


/**Function********************************************************************

  Synopsis    [Dumps the given be into a file with Davinci format]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_DumpDavinci(Be_Manager_ptr manager, be_ptr f, FILE* outFile)
{
  Rbc_OutputDaVinci(GET_RBC_MGR(manager), RBC(manager, f), outFile);
}


/**Function********************************************************************

  Synopsis    [Dumps the given be into a file with Davinci format]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_DumpGdl(Be_Manager_ptr manager, be_ptr f, FILE* outFile)
{
  Rbc_OutputGdl(GET_RBC_MGR(manager), RBC(manager, f), outFile);
}


/**Function********************************************************************

  Synopsis    [Dumps the given be into a file]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_DumpSexpr(Be_Manager_ptr manager, be_ptr f, FILE* outFile)
{
  Rbc_OutputSexpr(GET_RBC_MGR(manager), RBC(manager, f), outFile);
}


/**Function********************************************************************

  Synopsis    [Converts the given variable index into the corresponding be]

  Description [If corresponding index had not been previously
  allocated, it will be allocated. If corresponding node does not
  exist in the dag, it will be inserted.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
be_ptr Be_Index2Var(Be_Manager_ptr manager, int varIndex)
{
  return BE(manager, Rbc_GetIthVar(GET_RBC_MGR(manager), varIndex));
}


/**Function********************************************************************

  Synopsis    [Converts the given variable (as boolean expression) into
  the corresponding index]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Be_Var2Index(Be_Manager_ptr manager, be_ptr var)
{
  return Rbc_GetVarIndex( RBC(manager, var) );
}


/**Function********************************************************************

  Synopsis    [Prints out some statistical data about the underlying
  rbc structure]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Be_PrintStats(Be_Manager_ptr manager, int clustSize, FILE* outFile)
{
  Rbc_PrintStats(GET_RBC_MGR(manager), clustSize, outFile);
}


/**Function********************************************************************

  Synopsis    [Returns true iff sign of literal is positive.]

  Description []

  SideEffects []

  SeeAlso     [Be_CnfLiteral_Negate, Be_BeLiteral_IsSignPositive]

******************************************************************************/
boolean Be_CnfLiteral_IsSignPositive(const Be_Manager_ptr self, int cnfLiteral)
{
  nusmv_assert(cnfLiteral != 0);
  return cnfLiteral > 0;
}



/**Function********************************************************************

  Synopsis    [Returns negated literal.]

  Description []

  SideEffects []

  SeeAlso     [Be_CnfLiteral_IsSignPositive, Be_BeLiteral_Negate]

******************************************************************************/
int Be_CnfLiteral_Negate(const Be_Manager_ptr self, int cnfLiteral)
{
  nusmv_assert(cnfLiteral != 0);
  return -cnfLiteral;
}



/**Function********************************************************************

  Synopsis    [Returns true iff sign of literal is positive.]

  Description []

  SideEffects []

  SeeAlso     [Be_BeLiteral_Negate, Be_CnfLiteral_IsSignPositive]

******************************************************************************/
boolean Be_BeLiteral_IsSignPositive(const Be_Manager_ptr self, int beLiteral)
{
  nusmv_assert(beLiteral != 0);
  return beLiteral > 0;
}



/**Function********************************************************************

  Synopsis    [Returns negated literal.]

  Description []

  SideEffects []

  SeeAlso     [Be_BeLiteral_IsSignPositive, Be_CnfLiteral_Negate]

******************************************************************************/
int Be_BeLiteral_Negate(const Be_Manager_ptr self, int beLiteral)
{
  nusmv_assert(beLiteral != 0);
  return -beLiteral;
}


/**Function********************************************************************

  Synopsis [Performs the inlining of f, either including or not
  the conjuction set.]

  Description [If add_conj is true, the conjuction set is included, otherwise
        only the inlined formula is returned for a lazy SAT solving.]

  SideEffects []

  SeeAlso     [InlineResult]

******************************************************************************/
be_ptr Be_apply_inlining(Be_Manager_ptr mgr, be_ptr f, boolean add_conj)
{
  be_ptr res;
  InlineResult_ptr ir;

  /* lazy evaluation: */
  if (Be_IsConstant(mgr, f)) return f;

  ir = RbcInline_apply_inlining(GET_RBC_MGR(mgr), RBC(mgr, f));

  if (add_conj) res = BE(mgr, InlineResult_get_inlined_f_and_c(ir));
  else res = BE(mgr, InlineResult_get_inlined_f(ir));

  /* [MR] Here we destroy the structure computed by the inlining
     routines Also the caching of this structure in
     rbcInline.c:RbcInline_apply_inlining is disabled. This is
     needed because if a large number of calls to inliner are
     performes (e.g. in the incremental SBMC) then the usage of
     memory is too large. */
  InlineResult_destroy(ir);

  return res;
}


/*---------------------------------------------------------------------------*/
/* Definitions of internal functions                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definitions of static functions                                           */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Converts a be into a rbc]

  Description [The current implementation is really a simple type renaming.
  Internally used by Be_Manager via the inheritance mechanism.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void* beRbc_Be2Rbc(Be_Manager_ptr mgr, be_ptr be)
{
  return (Rbc_t*) be;
}


/**Function********************************************************************

  Synopsis    [Converts a rbc into a be]

  Description [The current implementation is really a simple type renaming.
  Internally used by Be_Manager via the inheritance mechanism.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static be_ptr beRbc_Rbc2Be(Be_Manager_ptr mgr, void* rbc)
{
  return (be_ptr) rbc;
}
