/**CFile***********************************************************************

  FileName    [bmcCheck.c]

  PackageName [bmc]

  Synopsis    [Some useful functions to check propositional formulae.
  Temporary located into the <tt>bmc</tt> package]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

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

#include "bmcCheck.h"

#include "bmcInt.h"   /* for 'options' */
#include "parser/symbols.h"  /* for constants */
#include "utils/error.h"


static char rcsid[] UTIL_UNUSED = "$Id: bmcCheck.c,v 1.13.4.4.2.2.2.3.4.2 2009-06-01 15:44:04 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define MAX_MATCHES 2048

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


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


static int bmc_is_propositional_formula_aux ARGS((node_ptr wff, int index,
						  void* pOpt));

static int bmc_check_if_wff_is_valid ARGS((node_ptr wff, int index,
					   void* aiIndexes));

static void bmc_add_valid_wff_to_list ARGS((node_ptr wff, int index,
					    void* list));


/*---------------------------------------------------------------------------*/
/* Declaration of exported functions                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/





/**Function********************************************************************

  Synopsis           [Helper function to simplify calling to 
  'bmc_check_wff_list' for searching of propositional wff only.  
  Returns a new list of wffs which contains legal wffs only]

  Description        []
  SideEffects        []

  SeeAlso            [bmc_check_wff_list]
******************************************************************************/
node_ptr Bmc_CheckFairnessListForPropositionalFormulae(node_ptr wffList)
{
  unsigned int aiOffendingWffIdxs[MAX_MATCHES];
  char szNumber[6];
  char szBuffer[MAX_MATCHES * (sizeof(szNumber)+2)];

  int iMatches;
  int i;
  char szSingleMatch[]="Warning!\n  One offending fairness formula contains one or more temporal operators.\n  The offending formula has been found at index [%s] into the fairness list.\n  (The first formula is located at index zero.)\n  The invalid formula will be ignored.\n";
  char szMultipleMatches[]="Warning!\n  %d offending fairness formulae contain one or more temporal operators.\n  The offending formulae have been found at indexes [%s] into the fairness list.\n  (The first formula is located at index zero.)\n  All invalid formulae will be ignored.\n";

  node_ptr list_valid_wff=Nil;
  memset(szBuffer, 0, sizeof(szBuffer));
  memset(szNumber, 0, sizeof(szNumber));

  iMatches = Bmc_WffListMatchProperty( wffList,
            &bmc_is_propositional_formula_aux,
            (void*)NULL, /* no check opt args */
            -1, /*search for all occurrences */
            aiOffendingWffIdxs,
            NULL, /* no answer */
            (void*)NULL
          );
  /* prepare output string: */
  for (i=0; i<iMatches; ++i) {
    int chars = snprintf(szNumber, 6, "%d", aiOffendingWffIdxs[i]);
    SNPRINTF_CHECK(chars, 6);

    strcat(szBuffer, szNumber);
    if ( i<(iMatches-1) ) {
      /* not the last index: */
      strcat(szBuffer, ", ");
    }
  }


  /* prepare list of valid wff only: */
  Bmc_WffListMatchProperty( wffList,
             &bmc_check_if_wff_is_valid,
             aiOffendingWffIdxs, /* par for checking fun */
             -1, /*search for all occurrences */
             NULL, /* no matched index array required */
             &bmc_add_valid_wff_to_list,
             &list_valid_wff
           );

  /* reverse list to restore correct order: */
  list_valid_wff = reverse(list_valid_wff);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    if (iMatches > 0 ) {
      /* szBuffer must contain invalid formula's index: */
      nusmv_assert(strlen(szBuffer)>0);

      if (iMatches>1) {
  fprintf(nusmv_stderr, szMultipleMatches, iMatches, szBuffer);
      }
      else {
  fprintf(nusmv_stderr, szSingleMatch, szBuffer);
      }
    }
  }

  return list_valid_wff;
}


/**Function********************************************************************

  Synopsis           [For each element belonging to a given list of wffs,
  calls the given matching function. If function matches, calls given
  answering function]

  Description        [This is a generic searching function for a property
  across a list of wffs. <i>Please note that searching is specific for a list
  of wffs, but the searching semantic and behaviour are generic and
  customizable.</i><br>
  Searching may be stopped after the Nth match, or can be continued till all
  list elements have been checked (specify <B>-1</B> in this case).
  In any case searching cannot be carried out over the <I>MAX_MATCHES</I>
  value.<br><br>
  <TABLE BORDER>
  <CAPTION> <B>Arguments:</B> </CAPTION>
  <TR> <TH> Parameter name </TH>  <TH> Description </TH> </TR>
  <TR> <TD> wffList </TD>         <TD> A list of wffs to iterate in </TD> </TR>
  <TR> <TD> pCheck  </TD>         <TD> Pointer to matching function.
   The checking function type is <B>BMC_PF_MATCH</B>, and has three
   parameters: <BR>
   <B> wff </B> the formula to check for <BR>
   <B> index </B> index of wff into list <BR>
   <B> pOpt </B> generic pointer to custom structure (optional) </TD> </TR>

  <TR> <TD> pCheckOptArgument </TD> <TD> Argument passed to pCheck
  (specify <B>NULL</B> if you do not use it.) </TD> </TR>

  <TR> <TD> iMaxMatches </TD>       <TD> Maximum number of matching to be
  found before return. This must be less of <I>MAX_MATCHES</I>.<BR>
  Specify <B>-1</B> to iterate across the entire list. </TD> </TR>

  <TR> <TD> aiMatchedIndexes </TD>  <TD> Optional <B>int</B> array which
  will contain all match indexes. <BR>
  Specify <B>NULL</B> if you do not need this functionality.
  Array size must be less of <I>MAX_MATCHES</I>. </TD> </TR>

  <TR> <TD> pAnswer </TD>           <TD> Pointer to answer function
  of type <B>BMC_PF_MATCH_ANSWER</B>. This function is called everytime
  a match is found. <BR>
  Specify <B>NULL</B> if you do not need for this functionality.
  The answer function has the following prototype: <BR>
  <I>void answer(node_ptr wff, int index, void* pOpt)</I> <BR>
  where:<BR>

  <B> wff </B> the formula that matches the criteria <BR>
  <B> index </B> is the index of wff into the list
  <B> pOpt  </B> pointer to generic & customizable structure
  (see <I>pAnswerOptArgument</I> below)

  <B> pAnswerOptArgument </B> optional parameter for pAnswer function,
  in order to ensure more flexibility. Specify <B>NULL</B> if you do not need
  for this functionality.) </TD> </TR>
  </TABLE>]

  SideEffects        [Given aiMatchedIndexes array changes if at least one
  match has found out]

  SeeAlso            []

******************************************************************************/
int Bmc_WffListMatchProperty(node_ptr wffList,
			     BMC_PF_MATCH pCheck,
			     void* pCheckOptArgument,
			     int  iMaxMatches,
			     unsigned int* aiMatchedIndexes,
			     BMC_PF_MATCH_ANSWER pAnswer,
			     void* pAnswerOptArgument)
{
  int iMatchesAvail;
  int index=0;
  node_ptr wff=Nil;
  node_ptr wffList_iterator=wffList;

  if (iMaxMatches == -1) {
    iMaxMatches = MAX_MATCHES-1;
  }

  /* index array size is limited to MAX_MATCHES-1, and no other negative
     values but -1 are allowed for the iMaxMatched parameter */
  nusmv_assert((iMaxMatches>=0) && (iMaxMatches < MAX_MATCHES));

  iMatchesAvail=iMaxMatches;

  while ( (iMatchesAvail>0) && (wffList_iterator != Nil) ) {
    wff = car(wffList_iterator);
    nusmv_assert(wff != Nil);
    if (pCheck(wff, index, pCheckOptArgument) == 0) {
      /* here wff matches searching criteria: */
      if (aiMatchedIndexes != NULL) {
  aiMatchedIndexes[iMaxMatches - iMatchesAvail] = index;
      }

      if (pAnswer != NULL) pAnswer(wff, index, pAnswerOptArgument);
      --iMatchesAvail;
    }

    /* continue with the next list element: */
    ++index;
    wffList_iterator = cdr(wffList_iterator);
  } /* end of while cycle */

  /* sign the end of aiMatchedIndexes with a terminator: */
  if (aiMatchedIndexes != NULL) {
    aiMatchedIndexes[iMaxMatches-iMatchesAvail] = -1;
  }


  return (iMaxMatches - iMatchesAvail);
}


/**Function********************************************************************

  Synopsis           [Given a wff returns 1 if wff is a propositional formula,
  zero (0) otherwise.]

  Description        []

  SideEffects        []

  SeeAlso            []
******************************************************************************/

boolean Bmc_IsPropositionalFormula(node_ptr wff)
{
  boolean iTemp = true;

  if (wff==Nil) return true;

  switch(node_get_type(wff)){
  case VAR:
  case ATOM:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP: 
  case TRUEEXP:
  case FALSEEXP:
  case BOOLEAN:
  case BIT:
  case DOT:
  case ARRAY:
    break;

  case CASE:
  case CONS:
  case COLON:
  case AND:
  case OR:
  case IMPLIES:
  case IFF:
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
  case EQUAL:
  case NOTEQUAL:
  case LT:
  case GT:
  case LE:
  case GE:
  case UNION:
  case SETIN:
    iTemp = Bmc_IsPropositionalFormula(car(wff));
    if (iTemp == true) {
      iTemp = Bmc_IsPropositionalFormula(cdr(wff));
    }
    break;

  case NOT:
    iTemp = Bmc_IsPropositionalFormula(car(wff));
    break;

  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
  case RELEASES:
  case TRIGGERED:
  case UNTIL:
  case SINCE:
  case OP_NEXT:
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case AU:
  case EU:
  case EBF:
  case ABF:
  case EBG:
  case ABG:
  case ABU:
  case EBU:
    iTemp = false;
    break;

  case EQDEF:
  case MINU:
  case MAXU:
    internal_error("Unaspected case value (%d)\n", node_get_type(wff));
    break;

  default:
    internal_error("Unknow case value (%d)\n", node_get_type(wff));
    break;
  }

  return iTemp;
}









/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Useful wrapper for
  Bmc_CheckFairnessListForPropositionalFormulae]

  Description        [Wrapper that makes
  Bmc_CheckFairnessListForPropositionalFormulae able to call
  Bmc_IsPropositionalFormula with a mode generic interface.
  Arguments 2 and 3 are practically unused, supplied to respect the generic
  interface only.]

  SideEffects        []

  SeeAlso            [Bmc_CheckFairnessListForPropositionalFormulae]
******************************************************************************/
static int
bmc_is_propositional_formula_aux(node_ptr wff, int index, void* pOpt)
{
  return Bmc_IsPropositionalFormula(wff);
}



/**Function********************************************************************

  Synopsis           [private service for
  Bmc_CheckFairnessListForPropositionalFormulae]

  Description        []

  SideEffects        []

  SeeAlso            [Bmc_CheckFairnessListForPropositionalFormulae]
******************************************************************************/
static int
bmc_check_if_wff_is_valid(node_ptr wff, int index, void* _aiIndexes)
{
  int i=0;
  int bInvalid=0;
  int* aiIndexes=(int*)_aiIndexes;

  /* search into ordered array of invalid wff's indexes the index of wff.
     If found return 0 (wff is invalid), else return 1 */
  while ( (aiIndexes[i] != -1) || (aiIndexes[i]>index) ) {
    if (aiIndexes[i] == index) {
      bInvalid = 1;
      break;
    }
    ++i;
  }

  return bInvalid;
}


/**Function********************************************************************

  Synopsis           [private service for
  Bmc_CheckFairnessListForPropositionalFormulae]

  Description        []

  SideEffects        []

  SeeAlso            [Bmc_CheckFairnessListForPropositionalFormulae]
******************************************************************************/
static void bmc_add_valid_wff_to_list(node_ptr wff, int index, void* _pList)
{
  node_ptr* pList=(node_ptr*)_pList;
  *pList = cons(wff, *pList );
}






