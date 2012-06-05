/**CFile***********************************************************************

  FileName    [rbcFormula.c]

  PackageName [rbc]

  Synopsis    [Formula constructors.]

  Description [External functions included in this module:
                <ul>
                <li> <b>Rbc_GetOne()</b>        logical truth
                <li> <b>Rbc_GetZero()</b>       logical falsity
                <li> <b>Rbc_GetIthVar</b>       variables
                <li> <b>Rbc_MakeNot()</b>       negation
                <li> <b>Rbc_MakeAnd()</b>       conjunction
                <li> <b>Rbc_MakeOr()</b>        disjunction
                <li> <b>Rbc_MakeIff()</b>       coimplication
                <li> <b>Rbc_MakeXor()</b>       exclusive disjunction
                <li> <b>Rbc_MakeIte()</b>       if-then-else
                <li> <b>Rbc_GetLeftOpnd()</b>   return left operand
                <li> <b>Rbc_GetRightOpnd()</b>  return right operand
                <li> <b>Rbc_GetVarIndex()</b>   return the variable index
                <li> <b>Rbc_Mark()</b>          make a vertex permanent
                <li> <b>Rbc_Unmark()</b>        make a vertex volatile
                <li> <b>Rbc_is_top()</b>        true iff symbol is RBCTOP
                <li> <b>Rbc_is_var()</b>        true iff symbol is RBCVAR
                <li> <b>Rbc_is_and()</b>        true iff symbol is RBCAND
                <li> <b>Rbc_is_iff()</b>        true iff symbol is RBCIFF
                <li> <b>Rbc_is_ite()</b>        true iff symbol is RBCITE
                </ul>]

  SeeAlso     []

  Author      [Armando Tacchella, Tommi Junttila, Michele Dorigatti]

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

******************************************************************************/
#include "rbc/rbcInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/* Used for conditional compilation */
#define RBC_ENABLE_LOCAL_MINIMIZATION_WITHOUT_BLOWUP 1

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
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
/**Macro***********************************************************************

  Synopsis    [Calls several minimization rules.]

  Description [Works in two steps:
               <ul>
               <li> Gets the proper children
               <li> Calls all the rules that apply to the format
                    (s)AND(a,b) = (s)AND(AND(c,d),b) or
                    (s)AND(b,a) = (s)AND(b,AND(c,d))
               </ul>]

  SideEffects [If RBC_ENABLE_LOCAL_MINIMIZATION_WITHOUT_BLOWUP is not 1:
               "a" is modified;
               "changed" is modified, the enclosing while loop will be rerun
               when finished; all the rules will be checked again]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define REDUCE_ONE_POSITIVE_SIDE(a, b, c, d, rbcManager, changed, s)    \
  (!RbcIsSet(a) && (RbcGetRef(a)->symbol == RBCAND))                    \
  {                                                                     \
  if (a == left) {                                                      \
    RBC_GET_CHILDREN_WITH_ASSERT(a, NIL(Rbc_t), c, d, c, d)             \
  }                                                                     \
  /* if (a == right) */                                                 \
  else {                                                                \
    RBC_GET_CHILDREN_WITH_ASSERT(NIL(Rbc_t), a, c, d, c, d)             \
  }                                                                     \
                                                                        \
  if CHECK_ASYMMETRIC_CONTRADICTION_O2(c, d, b, rbcManager, s)          \
                                                                        \
  if (RBC_ENABLE_LOCAL_MINIMIZATION_WITHOUT_BLOWUP == 1) {              \
    if CHECK_IDEMPOTENCE_O2(c, d, b, a, s)                              \
  }                                                                     \
  else {                                                                \
    if (b == c) {                                                       \
      /* (s)AND(AND(a,b),a) = (s)AND(b,a) */                            \
      /* Idempotence, o2 */                                             \
      a = d;                                                            \
      changed = 1;                                                      \
    }                                                                   \
    else if (b == d) {                                                  \
      /* (s)AND(AND(a,b),b) = (s)AND(a,b) */                            \
      /* Idempotence, o2 */                                             \
      a = c;                                                            \
      changed = 1;                                                      \
    }                                                                   \
  }                                                                     \
  }

/**Macro***********************************************************************

  Synopsis    [Calls several minimization rules.]

  Description [Works in two steps:
               <ul>
               <li> Gets the proper children
               <li> Calls all the rules that apply to the format
                    (s)AND(a,b) = (s)AND(~AND(c,d),b) or
                    (s)AND(b,a) = (s)AND(b,~AND(c,d))
               </ul>
               "changed" is passed to those minimization rules which could
               modify a or b, the parent rbcs]

  SideEffects [none]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define REDUCE_ONE_NEGATIVE_SIDE(a, b, c, d, changed, s)                \
  (RbcIsSet(a) && (RbcGetRef(a)->symbol == RBCAND))                     \
  {                                                                     \
  if (a == left) {                                                      \
    RBC_GET_CHILDREN_WITH_ASSERT(a, NIL(Rbc_t), c, d, c, d)             \
  }                                                                     \
  /* if (a == right) */                                                 \
  else {                                                                \
    RBC_GET_CHILDREN_WITH_ASSERT(NIL(Rbc_t), a, c, d, c, d)             \
  }                                                                     \
                                                                        \
  if CHECK_ASYMMETRIC_SUBSTITUTION_O3(c, d, b, a, changed)              \
  else if CHECK_ASYMMETRIC_SUBSTITUTION_O3(d, c, b, a, changed)         \
  else if CHECK_ASYMMETRIC_SUBSUMPTION_O2(c, d, b, s)                   \
  }

/**Macro***********************************************************************

  Synopsis    [Calls several minimization rules.]

  Description [Works in two steps:
               <ul>
               <li> Gets the proper children
               <li> Calls all the rules that apply to the format
                    (s)AND(a,b) = (s)AND(~AND(c,d),AND(e,f)) or
                    (s)AND(b,a) = (s)AND(AND(e,f),~AND(c,d))
               </ul>
               "changed" is passed to those minimization rules which could
               modify a or b, the parent rbcs]

  SideEffects [none]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define REDUCE_ONE_NEG_AND_ONE_POS_SIDES(a, b, c, d, e, f, changed, s)  \
  (RbcIsSet(a) &&                                                       \
   !RbcIsSet(b) &&                                                      \
   (RbcGetRef(a)->symbol == RBCAND) &&                                  \
   (RbcGetRef(b)->symbol == RBCAND))                                    \
  {                                                                     \
    RBC_GET_CHILDREN_WITH_ASSERT(a, b, c, d, e, f)                      \
                                                                        \
    if CHECK_SYMMETRIC_SUBSUMPTION_O2(c, d, e, f, b, s)                 \
    else if CHECK_SYMMETRIC_SUBSTITUTION_O3(c, d, e, f, a, changed, s)  \
    else if CHECK_SYMMETRIC_SUBSTITUTION_O3(d, c, e, f, a, changed, s)  \
  }

/**Macro***********************************************************************

  Synopsis    [Gets the children of the given rbcs]

  Description [Gets the children only of the non-NIL passed rbcs between left
               or right and makes all the needed assertions]

  SideEffects [To l1, l2, r1 and r2 are assigned the pointers of the children
               of the non-NIL passed rbcs]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define RBC_GET_CHILDREN_WITH_ASSERT(left, right, l1, l2, r1, r2)   \
  if (NIL(Rbc_t) != left) {                                         \
    l1 = NIL(Rbc_t);                                                \
    l2 = NIL(Rbc_t);                                                \
                                                                    \
    nusmv_assert(RbcGetRef(left)->numSons == 2);                    \
                                                                    \
    l1 = RbcGetRef(left)->outList[0];                               \
    l2 = RbcGetRef(left)->outList[1];                               \
                                                                    \
    nusmv_assert((l1 != NIL(Rbc_t)) && (l2 != NIL(Rbc_t)));         \
  }                                                                 \
  if (NIL(Rbc_t) != right) {                                        \
    r1 = NIL(Rbc_t);                                                \
    r2 = NIL(Rbc_t);                                                \
                                                                    \
    nusmv_assert(RbcGetRef(right)->numSons == 2);                   \
                                                                    \
    r1 = RbcGetRef(right)->outList[0];                              \
    r2 = RbcGetRef(right)->outList[1];                              \
                                                                    \
    nusmv_assert((r1 != NIL(Rbc_t)) && (r2 != NIL(Rbc_t)));         \
  }

/**Macro***********************************************************************

  Synopsis    [The Biere/Brummayer asymm. contradiction (O2) reduction rule]

  Description [If the rule is satisfied, returns the false or true rbc, based
               on sign:
               (s)AND(AND(a,b),c) = (s)F
               if c = ~a.
               The rule is applied in all its commutative variants.]

  SideEffects [None]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define CHECK_ASYMMETRIC_CONTRADICTION_O2(a, b, c, rbcManager, s)       \
  ((c == RbcId(a, RBC_FALSE)) ||                                        \
   (c == RbcId(b, RBC_FALSE)))                                          \
  {                                                                     \
    return RbcId(rbcManager->zero, s);                                \
  }

/**Macro***********************************************************************

  Synopsis    [The Biere/Brummayer idempotence (O2) reduction rule]

  Description [If the rule is satisfied, returns d, the parent of a and b:
               (s)AND(AND(a,b),c) = (s)AND(a,b) = (s)d
               if c = a
               The rule is applied in all its commutative variants.]

  SideEffects [none]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define CHECK_IDEMPOTENCE_O2(a, b, c, d, s)                             \
  ((c == a) || (c == b))                                                \
  {                                                                     \
    return RbcId(d, s);                                                 \
  }

/**Macro***********************************************************************

  Synopsis    [The Biere/Brummayer asymm. substitution (O3) reduction rule]

  Description [If the rule is satisfied, substitute d, the parent of a and b,
               with the negation of one of its children:
               (s)AND(d,c) = (s)AND(~AND(a,b),c) = (s)AND(~b,c)
               if a = c
               then d = ~b.
               The rule is applied in all its commutative variants.]

  SideEffects [If the rule is satisfied:
               "d" is modified;
               "changed" is modified, the enclosing while loop will be rerun
               when finished; all the rules will be checked again]

  Note        [It is not needed to set "changed" to 1. Indeed the new node
               connects two rbc that was previuosly connected, although in
               a different way: no further minimization is possible, because
               their children was already checked
               1. (s)AND(~b,c) is the result
               2. (s)AND(a,b) is d, one of the parent rbc
               3. but a = c
               4. (s)AND(~b,a) is equivalent to (s)AND(a,b)]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define CHECK_ASYMMETRIC_SUBSTITUTION_O3(a, b, c, d, changed)   \
  (a == c)                                                      \
  {                                                             \
    d = RbcId(b, RBC_FALSE);                                    \
    changed = 1;                                                \
  }

/**Macro***********************************************************************

  Synopsis    [The Biere/Brummayer asymm. subsumption (O2) reduction rule]

  Description [If the rule is satisfied, return the passed rbc:
               (s)AND(~AND(a,b),c) = (s)c
               if a = ~c.
               The rule is applied in all its commutative variants.]

  SideEffects [None]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define CHECK_ASYMMETRIC_SUBSUMPTION_O2(a, b, c, s)                     \
  ((a == RbcId(c, RBC_FALSE)) ||                                        \
   (b == RbcId(c, RBC_FALSE)))                                          \
  {                                                                     \
    return RbcId(c, s);                                                 \
  }

/**Macro***********************************************************************

  Synopsis    [The Biere/Brummayer idempotence (O4) reduction rule]

  Description [If the rule is satisfied, substitute e, the parent of c and d,
               with one of its children.
               (s)AND(AND(a,b),e) = (s)AND(AND(a,b),AND(c,d)) =
               = (s)AND(AND(a,b),d)
               if c = a
               then e = d
               The rule is applied in all its commutative variants.]

  SideEffects [If the rule is satisfied:
               "e" is modified;
               "changed" is modified, the enclosing while loop will be rerun
               when finished; all the rules will be checked again]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define CHECK_IDEMPOTENCE_O4(a, b, c, d, e, changed)                    \
  ((c == a) || (c == b))                                                \
  {                                                                     \
    e = d;                                                              \
    changed = 1;                                                        \
  }

/**Macro***********************************************************************

  Synopsis    [The Biere/Brummayer resolution (O3) reduction rule]

  Description [If the rule is satisfied, return the negation of one of the
               children passed:
               (s)AND(~AND(a,b),~AND(c,d)) = (s)~a
               if c = ~b & d = a.
               The rule is applied in all its commutative variants.]
  SideEffects [None]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define CHECK_RESOLUTION_O3(a, b, c, d, s)                              \
  (((d == a) && (c == RbcId(b, RBC_FALSE))) ||                          \
   ((c == a) && (d == RbcId(b, RBC_FALSE))))                            \
  {                                                                     \
    return RbcId(a, s ^ RBC_FALSE);                                     \
  }

/**Macro***********************************************************************

  Synopsis    [The Biere/Brummayer symm. subsumption (O2) reduction rule]

  Description [If the rule is satisfied, returns e, the parent of c and d:
               (s)AND(~AND(a,b),e) = (s)AND(~AND(a,b),AND(c,d)) = (s)e
               if a = ~c
               The rule is applied in all its commutative variants.]

  SideEffects [None]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define CHECK_SYMMETRIC_SUBSUMPTION_O2(a, b, c, d, e, s)        \
  ((a == RbcId(c, RBC_FALSE)) ||                                \
   (a == RbcId(d, RBC_FALSE)) ||                                \
   (b == RbcId(c, RBC_FALSE)) ||                                \
   (b == RbcId(d, RBC_FALSE)))                                  \
  {                                                             \
    return RbcId(e, s);                                         \
  }

/**Macro***********************************************************************

  Synopsis    [The Biere/Brummayer symm. substitution (O3) reduction rule]

  Description [If the rule is satisfied, substitute e, the parent of a and b,
               with the negation of one of its children.
                 (s)AND(e,AND(c,d)) =
               = (s)AND(~AND(a,b),AND(c,d)) =
               = (s)AND(~a,AND(c,d))
               if b = c.
               The rule is applied in all its commutative variants.]

  SideEffects [If the rule is satisfied:
               "e" is modified;
               "changed" is modified, the enclosing while loop will be rerun
               when finished; all the rules will be checked again]

  SeeAlso     [Rbc_MakeAnd()]
******************************************************************************/
#define CHECK_SYMMETRIC_SUBSTITUTION_O3(a, b, c, d, e, changed, s)      \
  ((b == c) || (b == d))                                                \
  {                                                                     \
    e = RbcId(a, RBC_FALSE);                                            \
    changed = 1;                                                        \
  }

/**AutomaticStart*************************************************************/
/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static Rbc_t*
Reduce(Rbc_Manager_t* rbcManager, int op, Rbc_t* left, Rbc_t* right);

/**AutomaticEnd***************************************************************/
/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Logical constant 1 (truth).]

  Description [Returns the rbc that stands for logical truth.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t* Rbc_GetOne(Rbc_Manager_t* rbcManager)
{
  return(rbcManager->one);
}

/**Function********************************************************************

  Synopsis    [Logical constant 0 (falsity).]

  Description [Returns the rbc that stands for logical falsity.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t* Rbc_GetZero(Rbc_Manager_t* rbcManager)
{
  return (rbcManager->zero);
}

/**Function********************************************************************

  Synopsis           [Returns true if the given rbc is a constant value,
                      such as either False or True]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Rbc_IsConstant(Rbc_Manager_t* manager, Rbc_t* f)
{
  return (Rbc_GetOne(manager) == f || Rbc_GetZero(manager) == f);
}

/**Function********************************************************************

  Synopsis    [Returns a variable.]

  Description [Returns a pointer to an rbc node containing the requested
               variable. Works in three steps:
               <ul>
               <li> the requested variable index exceeds the current capacity:
                    allocated more room up to the requested index;
               <li> the variable node does not exists: inserts it in the dag
                    and makes it permanent;
               <li> returns the variable node.
               </ul>]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t* Rbc_GetIthVar(Rbc_Manager_t* rbcManager,int varIndex)
{
  int i;

  nusmv_assert(0 <= varIndex);

  /* Allocate more room for the varTable if needed. */
  if (rbcManager->varCapacity <= varIndex) {
    rbcManager->varTable =
      REALLOC(Rbc_t*, rbcManager->varTable, varIndex + 1);
    for (i = rbcManager->varCapacity; i < varIndex + 1; i ++) {
      rbcManager->varTable[i] = NIL(Rbc_t);
    }
    rbcManager->varCapacity = varIndex + 1;
  }

  /* Create the variable if needed. */
  if (rbcManager->varTable[varIndex] == NIL(Rbc_t)) {
    rbcManager->varTable[varIndex] =
      Dag_VertexInsert(rbcManager->dagManager,
                       RBCVAR,
                       PTR_FROM_INT(char*, varIndex),
                       (Dag_Vertex_t**) NULL,
                       0);
    /* Make the node permanent. */
    Dag_VertexMark(rbcManager->varTable[varIndex]);
    ++(rbcManager->stats[RBCVAR_NO]);
  }

  /* Return the variable as rbc node. */
  return rbcManager->varTable[varIndex];

} /* End of Rbc_GetIthVar. */

/**Function********************************************************************

  Synopsis    [Returns the complement of an rbc.]

  Description [Returns the complement of an rbc.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t* Rbc_MakeNot(Rbc_Manager_t* rbcManager, Rbc_t* left)
{
  return RbcId(left, RBC_FALSE);
}

/**Function********************************************************************

  Synopsis    [Makes the conjunction of two rbcs.]

  Description [Makes the conjunction of two rbcs.
               Works in three steps:
               <ul>
               <li> performs boolean simplification: if successfull, returns
                    the result of the simplification;
               <li> orders left and right son pointers;
               <li> looks up the formula in the dag and returns it.
               </ul>

               If RBC_ENABLE_LOCAL_MINIMIZATION_WITHOUT_BLOWUP is defined,
               applies all the rules proposed in "R. Brummayer and
               A. Biere. Local Two-Level And-Inverter Graph Minimization
               without Blowup". In Proc. MEMICS 2006.  The expressions o1, o2,
               o3, o4 refers to the four level of optimization proposed in the
               paper.  The rules are implemented as macros in order to avoid
               repetitions]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t*
Rbc_MakeAnd(
  Rbc_Manager_t* rbcManager,
  Rbc_t*         left,
  Rbc_t*         right,
  Rbc_Bool_c     sign)
{
  Rbc_t* rTemp;
  Dag_Vertex_t** sons;
  Rbc_t* l1;
  Rbc_t* l2;
  Rbc_t* r1;
  Rbc_t* r2;

  int changed = 1;

  while (changed) {
    changed = 0;

    /* one level minimization */
    if (left == right) {
      /* (s)AND(x,x) = (s)x */
      /* Idempotence, o1 */
      right = NIL(Rbc_t);
    }
    else if (left == RbcId(right, RBC_FALSE)) {
      /* (s)AND(x,~x) = (s)F */
      /* Contradiction, o1 */
      left = rbcManager->zero;
      right = NIL(Rbc_t);
    }
    else if ((left == rbcManager->zero) ||
             (right == rbcManager->zero)) {
      /* (s)AND(F,y) = (s)AND(x,F) = (s)F */
      /* Boundedness, o1 */
      left = rbcManager->zero;
      right = NIL(Rbc_t);
    }
    else if (left == rbcManager->one) {
      /* (s)AND(T,y) = (s)y */
      /* Neutrality, o1 */
      left = right;
      right = NIL(Rbc_t);
    }
    else if (right == rbcManager->one) {
      /* (s)AND(x,T) = (s)x */
      /* Neutrality, o1 */
      right = NIL(Rbc_t);
    }
    if (right == NIL(Rbc_t)) {
      return RbcId(left, sign);
    }

    nusmv_assert(left != NIL(Rbc_t));
    nusmv_assert(right != NIL(Rbc_t));

    /* here starts two level minimization */
    /* case: AND(AND(a,b),c) */
    if REDUCE_ONE_POSITIVE_SIDE(left, right, l1, l2, rbcManager, changed, sign)
    /* case: AND(~AND(a,b),c) */
    else if REDUCE_ONE_NEGATIVE_SIDE(left, right, l1, l2, changed, sign)
    else {
      l1 = l2 = (Rbc_t*) NULL; /* to silent warnings */
    }

    /* case: AND(c,AND(a,b)) */
    if REDUCE_ONE_POSITIVE_SIDE(right, left, r1, r2, rbcManager, changed, sign)
    /* case: AND(c,~AND(a,b)) */
    else if REDUCE_ONE_NEGATIVE_SIDE(right, left, r1, r2, changed, sign)
    else {
      r1 = r2 = (Rbc_t*) NULL; /* to silent warnings */
    }

#if RBC_ENABLE_LOCAL_MINIMIZATION_WITHOUT_BLOWUP
    /* case: AND(AND(a,b),AND(c,d)) */
    if (!RbcIsSet(left) &&
        !RbcIsSet(right) &&
        (RbcGetRef(left)->symbol == RBCAND) &&
        (RbcGetRef(right)->symbol == RBCAND))
    {
      RBC_GET_CHILDREN_WITH_ASSERT(left, right, l1, l2, r1, r2)

      if ((l1 == RbcId(r1, RBC_FALSE)) ||
          (l1 == RbcId(r2, RBC_FALSE)) ||
          (l2 == RbcId(r1, RBC_FALSE)) ||
          (l2 == RbcId(r2, RBC_FALSE))) {
        /* (s)AND(AND(a,b),AND(~a,c)) = (s)F */
        /* the other conditions simply applies commutativity to the rule */
        /* Symmetric contradiction, o2 */
        return RbcId(rbcManager->zero, sign);
      }
      else if CHECK_IDEMPOTENCE_O4(l1, l2, r1, r2, right, changed)
      else if CHECK_IDEMPOTENCE_O4(r1, r2, l2, l1, left, changed)
      else if CHECK_IDEMPOTENCE_O4(l1, l2, r2, r1, right, changed)
      else if CHECK_IDEMPOTENCE_O4(r1, r2, l1, l2, left, changed)
    }
    /* case: AND(~AND(a,b),~AND(c,d)) */
    else if (RbcIsSet(left) &&
             RbcIsSet(right) &&
             (RbcGetRef(left)->symbol == RBCAND) &&
             (RbcGetRef(right)->symbol == RBCAND))
    {
      RBC_GET_CHILDREN_WITH_ASSERT(left, right, l1, l2, r1, r2)

      if CHECK_RESOLUTION_O3(l1, l2, r1, r2, sign)
      else if CHECK_RESOLUTION_O3(l2, l1, r1, r2, sign)
    }
    /* case: AND(~AND(a,b),AND(c,d)) */
    else if REDUCE_ONE_NEG_AND_ONE_POS_SIDES(left, right, l1, l2, r1, r2, \
                                             changed, sign)
    /* case: AND(AND(a,b),~AND(c,d)) */
    else if REDUCE_ONE_NEG_AND_ONE_POS_SIDES(right, left, r1, r2, l1, l2, \
                                             changed, sign)
#endif
 }

  /* Order the vertices. */
  if (right < left) {
    rTemp = right; right = left; left = rTemp;
  }

  nusmv_assert(left != NIL(Rbc_t));
  nusmv_assert(right != NIL(Rbc_t));
  nusmv_assert(left != right);
  nusmv_assert(left != RbcId(right, RBC_FALSE));

  /* Create the list of sons. */
  sons = ALLOC(Dag_Vertex_t*, 2);

  sons[0] = left;
  sons[1] = right;

  /* Lookup the formula in the dag. */
  rTemp = Dag_VertexLookup(rbcManager->dagManager, RBCAND, NIL(char), sons, 2);

  return RbcId(rTemp, sign);

} /* End of Rbc_MakeAnd. */

/**Function********************************************************************

  Synopsis    [Makes the disjunction of two rbcs.]

  Description [Makes the disjunction of two rbcs: casts the connective to
               the negation of a conjunction using De Morgan's law.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t*
Rbc_MakeOr(
  Rbc_Manager_t* rbcManager,
  Rbc_t*         left,
  Rbc_t*         right,
  Rbc_Bool_c     sign)
{
  /* Use De Morgan's law. */
  return Rbc_MakeAnd(rbcManager,
                     RbcId(left, RBC_FALSE),
                     RbcId(right, RBC_FALSE),
                     (Rbc_Bool_c)(sign ^ RBC_FALSE));

} /* End of Rbc_MakeOr. */

/**Function********************************************************************

  Synopsis    [Makes the coimplication of two rbcs.]

  Description [Makes the coimplication of two rbcs.
               Works in four steps:
               <ul>
               <li> performs boolean simplification: if successfull, returns
                    the result of the simplification;
               <li> orders left and right son pointers;
               <li> re-encodes the negation
               <li> looks up the formula in the dag and returns it.

               <li> If the coimplication mode is disable, expands the connective
                    in three AND nodes.
               </ul>]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t*
Rbc_MakeIff(
  Rbc_Manager_t* rbcManager,
  Rbc_t*         left,
  Rbc_t*         right,
  Rbc_Bool_c     sign)
{
#if RBC_ENABLE_IFF_CONNECTIVE
  Rbc_t* rTemp;
  Dag_Vertex_t** sons;

  rTemp = Reduce(rbcManager, RBCIFF, left, right);

  /* First, perform the reduction stage. */
  if (rTemp != NIL(Rbc_t)) return RbcId(rTemp, sign);


  /* Order the  vertices. */
  if (right < left) {
    rTemp = right; right = left; left = rTemp;
  }

  /* Negation always on top. */
  sign = sign ^ RbcIsSet(left) ^ RbcIsSet(right);
  RbcClear(left);
  RbcClear(right);

  /* Create the list of sons. */
  sons = ALLOC(Dag_Vertex_t*, 2);

  sons[0] = left;
  sons[1] = right;

  /* Lookup the formula in the dag. */
  rTemp = Dag_VertexLookup(rbcManager->dagManager, RBCIFF, NIL(char), sons, 2);

  return RbcId(rTemp, sign);

/* Eliminate the IFF connective changing it in an AND formula. */
#else
  /* (s)IFF(x,y) = (s)AND(~AND(x,~y),~AND(~x,y)) */
  return Rbc_MakeAnd(rbcManager,
                     (Rbc_MakeAnd(rbcManager,
                                  left,
                                  RbcId(right, RBC_FALSE),
                                  RBC_FALSE)),
                     (Rbc_MakeAnd(rbcManager,
                                  RbcId(left, RBC_FALSE),
                                  right,
                                  RBC_FALSE)),
                     (Rbc_Bool_c)(sign));
#endif
} /* End of Rbc_MakeIff. */

/**Function********************************************************************

  Synopsis    [Makes the exclusive disjunction of two rbcs.]

  Description [Makes the exclusive disjunction of two rbcs: casts the
               connective as the negation of a coimplication.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t*
Rbc_MakeXor(
  Rbc_Manager_t* rbcManager,
  Rbc_t*         left,
  Rbc_t*         right,
  Rbc_Bool_c     sign)
{
  /* Simply a negation of a coimplication. */
  return Rbc_MakeIff(rbcManager,
                     left,
                     right,
                     (Rbc_Bool_c)(sign ^ RBC_FALSE));
}

/**Function********************************************************************

  Synopsis    [Makes the if-then-else of three rbcs.]

  Description [Makes the if-then-else of three rbcs: expands the connective
              into the corresponding product-of-sums.

              If the if-then-else mode is disable, expands the connective in
              three AND nodes]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t*
Rbc_MakeIte(
  Rbc_Manager_t* rbcManager,
  Rbc_t*         i,
  Rbc_t*         t,
  Rbc_t*         e,
  Rbc_Bool_c     sign)
{
#if RBC_ENABLE_ITE_CONNECTIVE
  Rbc_t* rTemp;
  Dag_Vertex_t** sons;

  /* Bottom up simplification */
  int changed = 1;
  while(changed) {
    changed = 0;

    if (i == rbcManager->one) {
      /* ITE(T,t,e) == t */
      return RbcId(t, sign);
    }
    else if (i == rbcManager->zero) {
      /* ITE(F,t,e) == e */
      return RbcId(e, sign);
    }
    else if (t == rbcManager->one) {
      /* ITE(i,T,e) == OR(i,e) */
      return Rbc_MakeOr(rbcManager, i, e, sign);
    }
    else if (t == rbcManager->zero) {
      /* ITE(i,F,e) == AND(~i,e) */
      return Rbc_MakeAnd(rbcManager, Rbc_MakeNot(rbcManager, i), e, sign);
    }
    else if (e == rbcManager->one) {
      /* ITE(i,t,T) == OR(~i,t) */
      return Rbc_MakeOr(rbcManager, Rbc_MakeNot(rbcManager, i), t, sign);
    }
    else if (e == rbcManager->zero) {
      /* ITE(i,t,F) == AND(i,t) */
      return Rbc_MakeAnd(rbcManager, i, t, sign);
    }
    if (i == t) {
      /* ITE(i,i,e) == OR(i,e) */
      return Rbc_MakeOr(rbcManager, i, e, sign);
    }
    else if (i == e) {
      /* ITE(i,t,i) == AND(i,t) */
      return Rbc_MakeAnd(rbcManager, i, t, sign);
    }
    else if (t == e) {
      /* ITE(i,t,t) == t */
      return RbcId(t, sign);
    }
    else if (i == RbcId(t, RBC_FALSE)) {
      /* ITE(i,~i,e) == AND(~i,e) */
      return Rbc_MakeAnd(rbcManager, Rbc_MakeNot(rbcManager, i), e, sign);
    }
    else if (i == RbcId(e, RBC_FALSE)) {
      /* ITE(i,t,~i) == OR(~i,t) */
      return Rbc_MakeOr(rbcManager, Rbc_MakeNot(rbcManager, i), t, sign);
    }

    else if (t == RbcId(e, RBC_FALSE)) {
      /* ITE(i,t,~t) == IFF(i,t) */
      return Rbc_MakeIff(rbcManager, i, t, sign);
    }
  }

  /* Create the list of sons. */
  sons = ALLOC(Dag_Vertex_t*, 3);

  sons[0] = i;
  sons[1] = t;
  sons[2] = e;

  /* Lookup the formula in the dag. */
  rTemp = Dag_VertexLookup(rbcManager->dagManager, RBCITE, NIL(char), sons, 3);

  return RbcId(rTemp, sign);

/* Eliminate the ITE connective changing it in an AND formula. */
#else
  /* ITE(i,t,e,s) = (~s)AND(~AND(i,t),~AND(~i,e)) */
  return Rbc_MakeAnd(rbcManager,
                     (Rbc_MakeAnd(rbcManager, i, t, RBC_FALSE)),
                     (Rbc_MakeAnd(rbcManager,
                                  RbcId(i, RBC_FALSE),
                                  e,
                                  RBC_FALSE)),
                     (Rbc_Bool_c)(sign ^ RBC_FALSE));
#endif
} /* End of Rbc_MakeIte. */

/**Function********************************************************************

  Synopsis    [Gets the left operand.]

  Description [Gets the left operand.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t* Rbc_GetLeftOpnd(Rbc_t* f)
{
  if (RbcGetRef(f)->outList != (Dag_Vertex_t**)NULL) {
    /* Reusing f to avoid introduction of new variables. */
    f = (Rbc_t*) RbcGetRef(f)->outList[0];
  }
  return f;
}

/**Function********************************************************************

  Synopsis    [Gets the right operand.]

  Description [Gets the right operand.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
Rbc_t* Rbc_GetRightOpnd(Rbc_t* f)
{
  if (RbcGetRef(f)->outList != (Dag_Vertex_t**)NULL) {
    /* Reusing f to avoid introduction of new variables. */
    f = (Rbc_t*) RbcGetRef(f)->outList[RbcGetRef(f)->numSons -1];
  }
  return f;
}

/**Function********************************************************************

  Synopsis    [Gets the variable index.]

  Description [Returns the variable index,
               -1 if the rbc is not a variable.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
int Rbc_GetVarIndex(Rbc_t* f)
{
  if (RbcGetRef(f)->symbol == RBCVAR) {
    return PTR_TO_INT(RbcGetRef(f)->data);
  }
  return -1;
}

/**Function********************************************************************

  Synopsis    [Makes a node permanent.]

  Description [Marks the vertex in the internal dag. This saves the rbc
               from being wiped out during garbage collection.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
Rbc_Mark(
  Rbc_Manager_t* rbc,
  Rbc_t*         f)
{
  /* To avoid calling another function, do it directly! */
  ++(RbcGetRef(f)->mark);
  return;
}

/**Function********************************************************************

  Synopsis    [Makes a node volatile.]

  Description [Unmarks the vertex in the internal dag. This exposes the rbc
               to garbage collection.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
Rbc_Unmark(
  Rbc_Manager_t* rbc,
  Rbc_t*         f)
{
  if (RbcGetRef(f)->mark > 0) {
    --(RbcGetRef(f)->mark);
  }
  return;
}

/**Function********************************************************************

  Synopsis    [Check if a rbc type is RBCTOP]

  Description [Check if a rbc type is RBCTOP]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Rbc_is_top(Rbc_t* rbc) {
  if(Rbc_get_type(RbcGetRef(rbc)) == RBCTOP) return true;
  else return false;
}

/**Function********************************************************************

  Synopsis    [Check if a rbc type is RBCAND]

  Description [Check if a rbc type is RBCAND]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Rbc_is_and(Rbc_t* rbc) {
  if (Rbc_get_type(RbcGetRef(rbc)) == RBCAND) return true;
  else return false;
}

/**Function********************************************************************

  Synopsis    [Check if a rbc type is RBCIFF]

  Description [Check if a rbc type is RBCIFF]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Rbc_is_iff(Rbc_t* rbc) {
  if (Rbc_get_type(RbcGetRef(rbc)) == RBCIFF) return true;
  else return false;
}

/**Function********************************************************************

  Synopsis    [Check if a rbc type is RBCVAR]

  Description [Check if a rbc type is RBCVAR]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Rbc_is_var(Rbc_t* rbc) {
  if (Rbc_get_type(RbcGetRef(rbc)) == RBCVAR) return true;
  else return false;
}

/**Function********************************************************************

  Synopsis    [Check if a rbc type is RBCITE]

  Description [Check if a rbc type is RBCITE]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Rbc_is_ite(Rbc_t* rbc) {
  if (Rbc_get_type(RbcGetRef(rbc)) == RBCITE) return true;
  else return false;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Reduction (simplification) of rbcs.]

  Description [Reduction (simplification) of rbcs.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/
static Rbc_t*
Reduce(
 Rbc_Manager_t* rbcManager,
 int            op,
 Rbc_t*         left,
 Rbc_t*         right)
{
  switch (op) {
  /* case never entered! */
  case RBCAND:
    if (left == right) {
      /* AND(x,x) = x */
      return left;
    } else if (left == RbcId(right, RBC_FALSE)) {
      /* AND(x,~x) = F */
      return rbcManager->zero;
    } else if ((left == rbcManager->zero) ||
               (right == rbcManager->zero)) {
      /* AND(F,x)  = AND(x,F) = F */
      return rbcManager->zero;
    } else if (left == rbcManager->one) {
      /* AND(T,x) = x */
      return right;
    } else if (right == rbcManager->one) {
      /* AND(x,T) = x */
      return left;
    } else {
      return NIL(Rbc_t);
    }
  case RBCIFF:
    if (left == right) {
      /* IFF(x,x) = T */
      return rbcManager->one;
    } else if (left == RbcId(right, RBC_FALSE)) {
      /* IFF(x,~x) = F */
      return rbcManager->zero;
    } else if (left == rbcManager->zero) {
      /* IFF(F,y) = ~y */
      return RbcId(right, RBC_FALSE);
    } else if (right == rbcManager->zero) {
      /* IFF(x,F) = ~x */
      return RbcId(left, RBC_FALSE);
    } else if (left == rbcManager->one) {
      /* IFF(T,y) = y */
      return right;
    } else if (right == rbcManager->one) {
      /* IFF(x,T) = x */
      return left;
    } else {
      return NIL(Rbc_t);
    }
  }

  return NIL(Rbc_t);
} /* End of Reduce. */

