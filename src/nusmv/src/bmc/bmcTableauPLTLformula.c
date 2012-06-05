/**CFile***********************************************************************

  FileName    [bmcTableauPLTLformula.c]

  PackageName [bmc]

  Synopsis    [Bmc.TableauPLTL module]

  Description [Implements all the functions needed to build tableaux for
               PLTL formulas.]

  SeeAlso     [bmcTableau.c, bmcGen.c]

  Author      [Marco Benedetti]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
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

#include "bmcInt.h"
#include "bmcUtils.h"
#include "bmcModel.h"

#include "parser/symbols.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcTableauPLTLformula.c,v 1.6.2.5.2.2.2.3.6.2 2009-09-17 11:49:47 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define NO_BACKJUMP -(INT_MAX-1) 
#define INF           INT_MAX-1

#define EVAL_OR          0
#define EVAL_AND         1

#define OPEN             0
#define CLOSED           1

#define  FORWARD        +1
#define BACKWARD        -1

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis           [Set of time instants type]

  Description        [This data structure represents a set of time instants. 
  When direction==FORWARD, this set is
   EITHER
   * the interval \[fromTime,fromTime+steps\[, 
   when backJumpFromTime==NO_BACKJUMP

   OR,
   * \[fromTime,backJumpFromTime\] U \[backJumpToTime,backJumpToTime+steps2\[
     when backJumpFromTime!=NO_BACKJUMP, with
     steps2 = steps-(backJumpFromTime-fromTime)-1.

   The interval \[a,a\[ is assumed to be the empty set for every value a.

   When direction==BACKWARD, the interval is as follows:
   * \]fromTime-steps,fromTime\]  ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
typedef struct EvalSetType {
  int         fromTime;
  int        direction;
  int            steps;
  int backJumpFromTime;
  int   backJumpToTime;
} EvalSet;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis           []

  Description        [The function "rho" projects the time instant "i" 
  onto the main domain of a function f on a (k,l)-loop, 
  where l_f=l+p*tau(f) and k_f=k+p*tau(f)
  (with p=k-l). It is rho(i,l,k)=i, when i<k, and rho(i,l,k)=rho(i-p,l,k)
  otherwise.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define rho(i,l_f,k_f)                                                        \
        ((i)<(k_f)) ? (i) : (l_f) + (((i)-(l_f)) % ((k_f)-(l_f)))


/**Macro***********************************************************************

  Synopsis           []

  Description        [This control structure iterates on all the time 
  instants "i" in the EvalSet "set", according to the semantics of 
  EvalSet given above.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define FOR_EACH_INDEX_IN(i,set)                                              \
          for ((i) = (set).fromTime;                                          \
                                                                              \
               (set).steps != 0;                                              \
                                                                              \
               (i) = ((i==(set).backJumpFromTime)?                            \
                          (set).backJumpToTime :                              \
                        i+(set).direction), (set).steps--)

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static be_ptr
getTableauAtTime ARGS((const BeEnc_ptr be_enc,
                       const node_ptr pltl_wff,
                       const int time,
                       const int k, const int l));

static be_ptr evaluateOn ARGS((const BeEnc_ptr be_enc,
			       const node_ptr pltl_f,
			       const node_ptr pltl_g,
			       const int fromTime, const int toTime,
			       const int k, const int l,
			       const int evalType, const int eval_dir));

static EvalSet projectOntoMainDomain
ARGS((const node_ptr pltl_wff,
      int a, int b,
      const int k, const int l,
      const int interval_type, const int eval_dir));

static int tau ARGS((const node_ptr pltl_wff));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis     [Builds the tableau for a PLTL formula.]

  Description  [Builds both the bounded-tableau and the loop-tableau for a PLTL
                formula "pltl_wff" (depending on the value of l). The time
                the tableau refers to is (implicitly) time zero.]

  SideEffects  []

  SeeAlso      [getTableauAtTime]

******************************************************************************/
be_ptr
Bmc_TableauPLTL_GetTableau(const BeEnc_ptr be_enc,
                           const node_ptr pltl_wff,
                           const int k, const int l)
{
 return getTableauAtTime(be_enc, pltl_wff, 0, k, l);
}


/**Function********************************************************************

  Synopsis     [Builds the conjunction of the tableaux for a PLTL formula
                computed on every time instant along a (k,l)-loop.]

  Description  [This function is a special case of "evaluateOn", thus it
                computes its answer by calling "evaluateOn" with some specifc
                arguments. The only use of this function is in constructing
                optimized tableaux for those depth-one formulas where
                "RELEASES" is the unique operator.]

  SideEffects  []

  SeeAlso      [evaluateOn]

******************************************************************************/
be_ptr
Bmc_TableauPLTL_GetAllTimeTableau(const BeEnc_ptr be_enc,
                                  const node_ptr pltl_wff,
                                  const int k)
{
 return evaluateOn(be_enc, pltl_wff, NULL, 0, INF, k, 0, EVAL_AND, FORWARD);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis     [ Builds the tableau for a PLTL formula "pltl_wff" at time
                 "time".]

  Description  [ Tableaux for constant expressions and (negated) literals are
                 built immediately, while complex formulas are evaluated in a
                 compositional way. In particular, propositional operators are
                 evaluated throught recursive calls to the procedure
                 "getTableauAtTime" itself (no split on time instants is
                 necessary in this case). Time operators are evaluated (in a
                 uniform way) by means of a doubly recursive call, which
                 involves the "evaluateOn" procedure as a counterpart.
                 A concise representation of the set of time instants each time
                 operator refers to (together with its argument(s))
                 is passed to the "evaluateOn" procedure, which is then
                 responsible for recursively evaluating these arguments on the
                 proper set of integer time instants by calling
                 "getTableauAtTime" in turn.]

  SideEffects  []

  SeeAlso      [evaluateOn]

******************************************************************************/
static be_ptr getTableauAtTime(const BeEnc_ptr be_enc,
                               const node_ptr pltl_wff,
                               const int time, const int k, const int l)
{
  be_ptr subfTbl1, subfTbl2, tableau = NULL;
  node_ptr subf1, subf2;
  node_ptr key;

  Be_Manager_ptr beMgr = BeEnc_get_be_manager(be_enc);
  be_ptr truth = Be_Truth(beMgr);

  int evalDir=FORWARD, start, stop, evalType=EVAL_AND, evalTime;

  int nodeType  = node_get_type(pltl_wff);

  if (time<0 || (Bmc_Utils_IsNoLoopback(l) && 
                 (nodeType==OP_GLOBAL || time>k))) {
    return Be_Falsity(beMgr);
  }

  /* memoization */
  key = bmc_tableau_memoization_get_key(pltl_wff, time, k, l);
  tableau = bmc_tableau_memoization_lookup(key);
  if (tableau != (be_ptr) NULL) return tableau;

  switch (getOpClass(nodeType)) {

  case CONSTANT_EXPR:
    tableau =
      (nodeType==TRUEEXP) ? Be_Truth(beMgr):
    (nodeType==FALSEEXP)? Be_Falsity(beMgr):
    NULL;
    nusmv_assert(tableau != NULL);
    return tableau; /* no memoization */

  case LITERAL:
    evalTime = Bmc_Utils_IsNoLoopback(l)? time : rho(time,l,k);
     
    /* checks whether it is an input var at the initial/final state: */
    if (evalTime == k) {
      SymbTable_ptr st;
      boolean is_input;
       
      st = BaseEnc_get_symb_table(BASE_ENC(be_enc));

      if (nodeType==NOT) {
        is_input = SymbTable_is_symbol_input_var(st, car(pltl_wff));
      }
      else is_input = SymbTable_is_symbol_input_var(st, pltl_wff);

      if (is_input) {
        return Be_Falsity(beMgr); /* no memoization */
      }
    }
       
    tableau  =
      (nodeType==DOT)?   BeEnc_name_to_timed(be_enc, pltl_wff, evalTime): 
      (nodeType==BIT)?   BeEnc_name_to_timed(be_enc, pltl_wff, evalTime):
      (nodeType==ARRAY)? BeEnc_name_to_timed(be_enc, pltl_wff, evalTime):
      (nodeType==NOT)?   Be_Not(beMgr, BeEnc_name_to_timed(be_enc, car(pltl_wff), 
                                                           evalTime)):
      NULL;
    nusmv_assert(tableau != NULL);
    return tableau; /* no memoization */

  case PROP_CONNECTIVE:
    subfTbl1 = getTableauAtTime(be_enc,car(pltl_wff),time,k,l);
    subfTbl2 = getTableauAtTime(be_enc,cdr(pltl_wff),time,k,l);

    tableau =
      (nodeType==AND) ? Be_And(beMgr, subfTbl1, subfTbl2):
      (nodeType==OR ) ? Be_Or (beMgr, subfTbl1, subfTbl2):
      (nodeType==IFF) ? Be_Iff(beMgr, subfTbl1, subfTbl2):
      NULL;
    break;

  case TIME_OPERATOR:
    subf1 = car(pltl_wff);
    subf2 = isBinaryOp(nodeType) ? cdr(pltl_wff) : NULL;
    start = time;

    switch (nodeType) {
    case OP_NEXT:       start=stop=time+1;                              break;
    case OP_PREC:       start=stop=time-1;                              break;
    case OP_NOTPRECNOT: if ((start=stop=time-1)==-1) tableau=truth;     break;
    case OP_FUTURE:     stop=INF;  evalType=EVAL_OR;  evalDir=FORWARD;  break;
    case OP_ONCE:       stop=0;    evalType=EVAL_OR;  evalDir=BACKWARD; break;
    case OP_GLOBAL:     stop=INF;  evalType=EVAL_AND; evalDir=FORWARD;  break;
    case OP_HISTORICAL: stop=0;    evalType=EVAL_AND; evalDir=BACKWARD; break;
    case    UNTIL:      stop=INF;  evalType=EVAL_OR;  evalDir=FORWARD;  break;
    case    SINCE:      stop=0;    evalType=EVAL_OR;  evalDir=BACKWARD; break;
    case    RELEASES:   stop=INF;  evalType=EVAL_AND; evalDir=FORWARD;  break;
    case    TRIGGERED:  stop=0;    evalType=EVAL_AND; evalDir=BACKWARD; break;
    }

    if (tableau==NULL)
      tableau = evaluateOn(be_enc,subf1,subf2,
                           start,stop,k,l,evalType,evalDir);
    break;

  default:
    internal_error("Unexpected operator, node type %d", nodeType);
  }

  nusmv_assert(tableau != NULL);
  bmc_tableau_memoization_insert(key, tableau); /* memoizes */
  return tableau;
}


/**Function********************************************************************

  Synopsis     [ Evaluates (either disjunctively or conjunctively) a PLTL
                 formula over an interval of time. ]

  Description  [ When only one argument is passed in (pltl_g==NULL), the
                 tableaux at the proper time instants for that argument are
                 computed (throught recursive calls to "getTableauAtTime")
                 and either disjunctively or conjunctively put together
                 (depending on the value of the "evalType" parameter).
                 When two arguments are given, the second one is evaluated
                 according to the following scheme (here we represent the
                 disjunctive case; "and" and "or" have to be exchanged to
                 obtain the conjunctive case):
                          (Aj or (Bi and Bi+1 and ... and Bj-1))

                 where A is the first argument, B is the second one, j is the
                 time the first argument is currently being evaluated on, and
                 i is the starting time for the whole evaluation (this
                 evaluation scheme is adopted as it is shared by all the
                 binary time operators in the PLTL logic).
                 In both cases, the proper evaluation set is computed
                 by calling the "projectOntoMainDomain" function, which deals
                 with both bounded and loop paths. ]

  SideEffects  []

  SeeAlso      [getTableauAtTime, projectOntoMainDomain]

******************************************************************************/
static be_ptr evaluateOn(const BeEnc_ptr be_enc,
                         const node_ptr pltl_f,
                         const node_ptr pltl_g,
                         const int fromTime, const int toTime,
                         const int k, const int l,
                         const int evalType,
                         const int evalDir)
{
  int j,q;
  boolean isBinary = (pltl_g!=NULL);

  Be_Manager_ptr beMgr = BeEnc_get_be_manager(be_enc);

  EvalSet evalSet = projectOntoMainDomain((isBinary? pltl_g:pltl_f),
                                          fromTime,toTime,k,l,CLOSED,evalDir);

  be_ptr result   = (evalType==EVAL_OR)? Be_Falsity(beMgr) : Be_Truth(beMgr);

  /* This loop evaluates either the (unique) argument of a unary operator
     or the right argument of a binary operator. */
  FOR_EACH_INDEX_IN(j,evalSet) {

    be_ptr tempTbl = getTableauAtTime(be_enc,
                                       (isBinary? pltl_g:pltl_f),
                                       j,k,l);
    if (isBinary) {
      EvalSet evalSet2 = projectOntoMainDomain(pltl_f,fromTime,j,k,l,
                                               OPEN,evalDir);

      /* This loop evaluates the left argument of a binary operator. */
      FOR_EACH_INDEX_IN(q,evalSet2) {

        be_ptr tempTblInner = getTableauAtTime(be_enc,pltl_f,q,k,l);

        tempTbl = (evalType==EVAL_AND)?
                   Be_Or (beMgr, tempTbl, tempTblInner):
                   Be_And(beMgr, tempTbl, tempTblInner);
      }
    }

  result = (evalType==EVAL_OR)?
            Be_Or (beMgr, result, tempTbl) :
            Be_And(beMgr, result, tempTbl) ;
  }

 return result;
}



/**Function********************************************************************

  Synopsis     [ Projects a (possibly open) interval [a,b] of integers
                 (time instants) onto the main domain of the PLTL formula
                 pltl_wff. The result of the projection can be either an
                 interval or the conjunction of two intervals. In both cases,
                 the resulting set is returned as an "EvalSet" structure. ]

  Description  [ For bounded paths, the projection of the interval [a,b] is
                 the interval [a,b] itself, except for the possibly infinite
                 right bound (symbolically represented by the constant "INF")
                 which is substituted by the integer bound k.
                 For infinite paths which are (k,l)-loops, this function
                 explicitly computes the set implicitly defined as
                 EvalSet={rho(i) such that i is in [a,b]}. The function
                 "projectOntoMainDomain" thus extends to intervals the
                 transformation previously defined by "rho" with respect
                 to single time point. ]

  SideEffects  []

  SeeAlso      [rho, evaluateOn]

******************************************************************************/

static EvalSet projectOntoMainDomain(const node_ptr pltl_wff,
                                     int a, int b,
                                     const int k, const int l,
                                     const int interval_type, 
                                     const int eval_dir)
{
  EvalSet evalSet;

  nusmv_assert(!Bmc_Utils_IsAllLoopbacks(l));
  evalSet.direction = eval_dir;

  if (Bmc_Utils_IsNoLoopback(l)) {
    evalSet.fromTime         = a;
    evalSet.backJumpFromTime = NO_BACKJUMP;
    evalSet.backJumpToTime   = NO_BACKJUMP;
    evalSet.steps            = ((b==INF) ? (k-a+1) : (abs(b-a)+1)) -
                               ((interval_type==OPEN) ? 1:0);
  }

  else {
    if (eval_dir==FORWARD) {
      int p     = k-l;
      int tau_f = tau(pltl_wff);
      int   l_f = l+p*tau_f;
      int   k_f = k+p*tau_f;

      assert (b>=a || b>=l);
      while (b<a) b+=p;

      evalSet.fromTime  = rho(a,l_f,k_f);
      evalSet.backJumpFromTime = k_f-1;
      evalSet.backJumpToTime   = l_f;
      evalSet.steps = (interval_type==CLOSED) ?
                       (a<l_f) ? (min(k_f-1,b)-a+1) : min(p,(b-a+1)):
                       (a<l_f) ? (min(k_f  ,b)-a  ) : min(p,(b-a  ));
    }
    else {
      assert(b<=a);
      evalSet.fromTime  = a;
      evalSet.backJumpFromTime = NO_BACKJUMP;
      evalSet.backJumpToTime   = NO_BACKJUMP;
      evalSet.steps =  (a-b+1) - ((interval_type==OPEN) ? 1:0);
    }
  }

 return evalSet;
}


/**Function********************************************************************

  Synopsis     [Gives an upper bound on the past temporal horizon of a
                PLTL formula.]

  Description  [Recursively computes the (maximum) nesting depth of past
                operators in the formula, which is an upper bound on its past
                temporal horizon.]

  SideEffects  []

  SeeAlso      [projectOntoMainDomain]

******************************************************************************/
static int tau(const node_ptr pltl_wff)
{
 int result = 0;
 int nodeType = node_get_type(pltl_wff);

 if (isVariable(nodeType) || isConstantExpr(nodeType)) {
   result = 0;
 }
 else {
   if (isBinaryOp(nodeType)) {
     int sub1 = tau(car(pltl_wff));
     int sub2 = tau(cdr(pltl_wff));

     result = max(sub1,sub2);
   }
   else {
     result = tau(car(pltl_wff));
   }
   if (isPastOp(nodeType)) result++;
 }

 return result;
}

