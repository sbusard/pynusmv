/**CFile***********************************************************************

  FileName    [bmcTableauLTLformula.c]

  PackageName [bmc]

  Synopsis    [Bmc.Tableau module]

  Description [This module contains all the operations related to the
               construction of tableaux for LTL formulas]

  SeeAlso     [bmcGen.c, bmcModel.c, bmcConv.c, bmcVarMgr.c]

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

#include "bmcInt.h"
#include "bmcUtils.h"
#include "bmcModel.h"

#include "parser/symbols.h"
#include "utils/error.h"
#include "utils/assoc.h"

static char rcsid[] UTIL_UNUSED = "$Id: bmcTableauLTLformula.c,v 1.3.4.5.2.2.2.5.6.2 2009-09-17 11:49:47 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
/**Variable********************************************************************

  Synopsis           [This variable is used by BmcInt_Tableau_GetAtTime]
  Description        [An internal accessor make it possible to free this table
  ewhen needed]
  SideEffects        []
  SeeAlso            [bmc_quit_tableau_ltl_memoization]

******************************************************************************/
static hash_ptr tableau_ltl_memoiz = (hash_ptr) NULL; 
void bmc_quit_tableau_memoization(void)
{
  if (tableau_ltl_memoiz != (hash_ptr) NULL) {
    free_assoc(tableau_ltl_memoiz);
    tableau_ltl_memoiz = (hash_ptr) NULL;
  }
}

node_ptr 
bmc_tableau_memoization_get_key(node_ptr wff, int time, int k, int l)
{
  return find_node(CONS, wff, 
                   find_node(CONS, PTR_FROM_INT(node_ptr, time), 
                             find_node(CONS, PTR_FROM_INT(node_ptr, k), 
                                       PTR_FROM_INT(node_ptr, l)))); 
}
void bmc_tableau_memoization_insert(node_ptr key, be_ptr be)
{
  if (tableau_ltl_memoiz == (hash_ptr) NULL) {
    tableau_ltl_memoiz = new_assoc();
    nusmv_assert(tableau_ltl_memoiz != (hash_ptr) NULL);
  }
  insert_assoc(tableau_ltl_memoiz, key, (node_ptr) be);
}

be_ptr bmc_tableau_memoization_lookup(node_ptr key)
{
  if (tableau_ltl_memoiz == (hash_ptr) NULL) {
    tableau_ltl_memoiz = new_assoc();
    nusmv_assert(tableau_ltl_memoiz != (hash_ptr) NULL);
  }
  return (be_ptr) find_assoc(tableau_ltl_memoiz, key);
}

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static be_ptr
bmc_tableauGetUntilAtTime_aux(const BeEnc_ptr be_enc,
                              const node_ptr p, const node_ptr q,
                              const int time, const int k, const int l,
                              const int steps);

static be_ptr
bmc_tableauGetReleasesAtTime_aux(const BeEnc_ptr be_enc,
                                 const node_ptr p, const node_ptr q,
                                 const int time, const int k, const int l,
                                 const int steps);


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Given a wff expressed in ltl builds the model-independent
  tableau at 'time' of a path formula bounded by \[k, l\]]

  Description        [This function is the entry point of a mutual recursive
  calling stack. All logical connectives are resolved, excepted for NOT, which
  closes the recursive calling stack. Also variables and falsity/truth
  constants close the recursion.]

  SideEffects        []

  SeeAlso            [bmc_tableauGetNextAtTime,
  bmc_tableauGetGloballyAtTime, bmc_tableauGetEventuallyAtTime,
  bmc_tableauGetUntilAtTime, bmc_tableauGetReleasesAtTime]

******************************************************************************/
be_ptr
BmcInt_Tableau_GetAtTime(const BeEnc_ptr be_enc, const node_ptr ltl_wff,
                         const int time, const int k, const int l)
{
  be_ptr result=NULL;
  SymbTable_ptr st;
  Be_Manager_ptr be_mgr;
  node_ptr key;

  /* checks out the validity of [l, k] only if a loop exists: */
  nusmv_assert(Bmc_Utils_IsNoLoopback(l) || (k > l));
  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );

  st = BaseEnc_get_symb_table(BASE_ENC(be_enc));
  be_mgr = BeEnc_get_be_manager(be_enc);

  /* memoization */
  key = bmc_tableau_memoization_get_key(ltl_wff, time, k, l);
  result = bmc_tableau_memoization_lookup(key);
  if (result != (be_ptr) NULL) return result;

  switch (node_get_type(ltl_wff)) {
  case TRUEEXP:
    return Be_Truth(be_mgr); /* not memoized */

  case FALSEEXP:
    return Be_Falsity(be_mgr); /* not memoized */

  case BIT:
  case DOT:
    if ((time == k) && 
        BeEnc_is_index_input_var(be_enc, 
                                 BeEnc_name_to_index(be_enc, ltl_wff))) {
      /* input vars when time == max_time evaluate to false: */
      return Be_Falsity(be_mgr); /* not memoized */
    }
    
    return BeEnc_name_to_timed(be_enc, ltl_wff, time); /* not memoized */

  case ARRAY:
    if (!SymbTable_is_symbol_declared(st, ltl_wff)) {
      internal_error("Unexpected array node\n");      
    }

    if (!SymbTable_is_symbol_bool_var(st, ltl_wff)) {
      fprintf(nusmv_stderr, "Detected scalar array variable '");
      print_node(nusmv_stderr, ltl_wff);
      fprintf(nusmv_stderr, "'");
      internal_error("Scalar array variable has been found where a boolean "
                     "variable had to be used instead.\n"
                     "This might be due to a bug on your model.");
    }

    if ((time == k) && 
        BeEnc_is_index_input_var(be_enc, 
                                 BeEnc_name_to_index(be_enc, ltl_wff))) {
      /* input vars when time == max_time evaluate to false: */
      return Be_Falsity(be_mgr); /* not memoized */
    }

    return BeEnc_name_to_timed(be_enc, ltl_wff, time); /* not memoized */

  case NOT:
    /* checks out that argument of NOT operator is actually a variable: */
    nusmv_assert( node_get_type(car(ltl_wff)) == DOT ||
                  node_get_type(car(ltl_wff)) == BIT || 
                  node_get_type(car(ltl_wff)) == ARRAY);
    
    if (!SymbTable_is_symbol_declared(st, car(ltl_wff))) {
      internal_error("Unexpected scalar or undefined node\n");      
    }

    if ((node_get_type(car(ltl_wff)) == ARRAY) && 
        ! SymbTable_is_symbol_bool_var(st, car(ltl_wff))) {
      fprintf(nusmv_stderr, "Detected scalar array variable '");
      print_node(nusmv_stderr, car(ltl_wff));
      fprintf(nusmv_stderr, "'");
      internal_error("Scalar array variable has been found where a boolean "
                     "variable had to be used instead.\n"
                     "This might be due to a bug on your model.");
    }

    if ((time == k) && 
        BeEnc_is_index_input_var(be_enc, 
                                 BeEnc_name_to_index(be_enc, car(ltl_wff)))) {
      /* input vars when time == max_time evaluate to false: */
      result = Be_Falsity(be_mgr);
      break;
    }

    result = Be_Not(be_mgr, BeEnc_name_to_timed(be_enc, car(ltl_wff), time));
    break;

  case AND:
    result = Be_And(be_mgr,
                    BmcInt_Tableau_GetAtTime(be_enc,car(ltl_wff), time, k, l),
                    BmcInt_Tableau_GetAtTime(be_enc,cdr(ltl_wff), time, k, l));
    break;

  case OR:
    result = Be_Or(be_mgr,
                   BmcInt_Tableau_GetAtTime(be_enc,car(ltl_wff), time, k, l),
                   BmcInt_Tableau_GetAtTime(be_enc,cdr(ltl_wff), time, k, l));
    break;

  case IFF:
    result = Be_Iff(be_mgr,
                    BmcInt_Tableau_GetAtTime(be_enc,car(ltl_wff),time,k,l),
                    BmcInt_Tableau_GetAtTime(be_enc,cdr(ltl_wff),time,k,l));
    break;

  case OP_NEXT:
    result = bmc_tableauGetNextAtTime(be_enc, car(ltl_wff), time, k, l);
    break;

  case OP_GLOBAL:
    result = bmc_tableauGetGloballyAtTime(be_enc, car(ltl_wff), time, k, l);
    break;

  case OP_FUTURE: /* EVENTUALLY */
    result = bmc_tableauGetEventuallyAtTime(be_enc, car(ltl_wff), time, k, l);
              
    break;

  case UNTIL:
    result = bmc_tableauGetUntilAtTime(be_enc, car(ltl_wff), cdr(ltl_wff),
                                       time, k, l);
    break;

  case RELEASES:
    result = bmc_tableauGetReleasesAtTime(be_enc, car(ltl_wff), cdr(ltl_wff),
                                          time, k, l );
    break;

  case IMPLIES:
    internal_error("'Implies' should had been nnf-ed away!\n");

  case ATOM:
  case EX:
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
  case EBF:
  case EBG:
  case ABF:
  case ABG:
    internal_error( "Unexpected CTL operator, node type %d\n",
                    node_get_type(ltl_wff) );

  default:
    /* no other type are available here: */
    error_unreachable_code();
  }

  nusmv_assert(result != NULL); /*it must be assigned! */
  bmc_tableau_memoization_insert(key, result); /* memoizes */
  return result;
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Resolves the NEXT operator, building the tableau for
  its argument]

  Description        [Returns a falsity constants if the next operator leads
  out of \[l, k\] and there is no loop]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr
bmc_tableauGetNextAtTime(const BeEnc_ptr be_enc, const node_ptr ltl_wff,
			 const int time, const int k, const int l)
{
  int succtime;
  be_ptr tableau;

  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );

  /* checks out the validity of [l, k] only if a loop exists: */
  nusmv_assert(Bmc_Utils_IsNoLoopback(l) || (k > l) );

  succtime = Bmc_Utils_GetSuccTime(time, k, l);

  if (!Bmc_Utils_IsNoLoopback(succtime)) {
    tableau = BmcInt_Tableau_GetAtTime(be_enc, ltl_wff, succtime, k, l);
  }
  else {
    tableau = Be_Falsity(BeEnc_get_be_manager(be_enc));
  }

  return tableau;
}


/**Function********************************************************************

  Synopsis           [Resolves the future operator, and builds a conjunctive
  expression of tableaus, by iterating intime up to k in a different manner
  depending on the \[l, k\] interval form]

  Description        [ltl_wff is the 'p' part in 'F p'.
  If intime<=k is out of \[l, k\] or if there is no loop,
  iterates from intime to k, otherwise iterates from l to k]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
be_ptr
bmc_tableauGetEventuallyAtTime(const BeEnc_ptr be_enc,
			       const node_ptr ltl_wff,
			       const int intime, const int k, const int l)
{
  Be_Manager_ptr be_mgr;
  int time;
  be_ptr tableau;
  int stop_time;
  int start_time;

  nusmv_assert((intime < k) || (intime==k && Bmc_Utils_IsNoLoopback(l)) );

  /* checks out the validity of [l, k] only if a loop exists: */
  nusmv_assert(Bmc_Utils_IsNoLoopback(l) || (k > l));

  be_mgr = BeEnc_get_be_manager(be_enc);
  tableau = Be_Falsity(be_mgr);

  /* there exist three cases:
     1) no loop: iterates from k downto intime;
     2) loop, (intime < l): iterates from k-1 downto intime;
     3) loop, (l <= intime < k) : iterates from k-1 downto l */

  if (Bmc_Utils_IsNoLoopback(l)) {
    /* The first case */
    start_time = k;
    stop_time  = intime;
  }
  else {
    /* The second and third case*/
    start_time = k-1;
    stop_time  = min(intime,l);
  }

  for (time = start_time; time>=stop_time; --time) {
    /* lazy evaluation: */
    be_ptr tableau_at_time = BmcInt_Tableau_GetAtTime(be_enc, ltl_wff, 
						      time, k, l);
                   
    if ( Be_IsTrue(be_mgr, tableau_at_time) ) {
      tableau = tableau_at_time;
      break;
    }
    tableau = Be_Or(be_mgr,
           tableau_at_time, tableau);
  } /* loop */

  return tableau;
}

/**Function********************************************************************

  Synopsis           [As bmc_tableauGetEventuallyAtTime, but builds a
  conjunctioned expression in order to be able to assure a global constraint]

  Description        [ltl_wff is the 'p' part in 'G p']

  SideEffects        []

  SeeAlso            [bmc_tableauGetEventuallyAtTime]

******************************************************************************/
be_ptr
bmc_tableauGetGloballyAtTime(const BeEnc_ptr be_enc,
			     const node_ptr ltl_wff,
			     const int intime, const int k, const int l)
{
  Be_Manager_ptr be_mgr;
  int time;
  be_ptr tableau;
  int stop_time;

  nusmv_assert((intime < k) || (intime==k && Bmc_Utils_IsNoLoopback(l)) );

  /* checks out the validity of [l, k] only if a loop exists: */
  nusmv_assert(Bmc_Utils_IsNoLoopback(l) || (k > l));

  be_mgr = BeEnc_get_be_manager(be_enc);

  /* there exist three cases:
     1) no loop: cannot assure nothing, so return falsity;
     2) loop, (intime < l): iterates from intime to k-1;
     3) loop, (l <= intime < k) : iterates from intime to k-1, and then from
        l to intime-1 (so more efficiently from l to k-1.)  */
  if (Bmc_Utils_IsNoLoopback(l)) {
    tableau = Be_Falsity(be_mgr);
  }
  else {
    /* second and third cases */
    tableau = Be_Truth(be_mgr);

    stop_time = min(intime, l);
    for (time=k-1; time >= stop_time; --time) {
      /* lazy evaluation: */
      be_ptr tableau_at_time = BmcInt_Tableau_GetAtTime(be_enc, ltl_wff,
							time, k, l);
      if ( Be_IsFalse(be_mgr, tableau_at_time) ) {
	tableau = tableau_at_time;
	break;
      }
      tableau = Be_And(be_mgr,
		       tableau_at_time, tableau);
    }
  }

  return tableau; 
}



/**Function********************************************************************

  Synopsis           [Builds an expression which evaluates the until operator]

  Description        [Carries out the steps number to be performed, depending
  on l,k and time, then calls bmc_tableauGetUntilAtTime_aux]

  SideEffects        []

  SeeAlso            [bmc_tableauGetUntilAtTime_aux]

******************************************************************************/
be_ptr
bmc_tableauGetUntilAtTime(const BeEnc_ptr be_enc,
			  const node_ptr p, const node_ptr q,
			  const int time, const int k, const int l)
{
  int steps;

  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );

  if (Bmc_Utils_IsNoLoopback(l)) {
    steps = k - time + 1 ; /* no loop, interval [time, k] */
  }
  else {
    steps = (k-1) - min(time,l) + 1; /* loop, full round */
  }

  return bmc_tableauGetUntilAtTime_aux(be_enc, p, q, time, k, l, steps);
}


/**Function********************************************************************

  Synopsis           [Builds an expression which evaluates the release
  operator]

  Description        [Carries out the steps number to be performed, depending
  on l,k and time, then calls bmc_tableauGetReleasesAtTime_aux]

  SideEffects        []

  SeeAlso            [bmc_tableauGetReleasesAtTime_aux]

******************************************************************************/
be_ptr
bmc_tableauGetReleasesAtTime(const BeEnc_ptr be_enc,
			     const node_ptr p, const node_ptr q,
			     const int time, const int k, const int l)
{
  int steps;

  nusmv_assert (time <= k);

  if (Bmc_Utils_IsNoLoopback(l)) {
    steps = k - time + 1 ; /* no loop, interval [time, k] */
  }
  else {
    steps = (k-1) - min(time,l) + 1; /* loop, full round */
  }

  return bmc_tableauGetReleasesAtTime_aux(be_enc, p, q, time, k, l, steps);
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [auxiliary part of bmc_tableauGetUntilAtTime]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
bmc_tableauGetUntilAtTime_aux(const BeEnc_ptr be_enc,
			      const node_ptr p, const node_ptr q,
			      const int time, const int k, const int l,
			      const int steps)
{
  Be_Manager_ptr be_mgr;
  be_ptr tableau_temp; /* for increasing of performances */
  be_ptr tableau_following; /* to increase readability */

  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );
  nusmv_assert (steps >= 1);

  be_mgr = BeEnc_get_be_manager(be_enc);
  tableau_temp = BmcInt_Tableau_GetAtTime(be_enc, q, time, k, l);

  if (steps > 1) {
    tableau_following =
      bmc_tableauGetUntilAtTime_aux(be_enc, p, q,
				    Bmc_Utils_GetSuccTime(time, k, l),
				    k, l, steps - 1);

    tableau_temp =
      Be_Or( be_mgr,
	     tableau_temp,
	     Be_And(be_mgr,
		    BmcInt_Tableau_GetAtTime(be_enc, p, time, k, l),
		    tableau_following) );
  }
  return tableau_temp;
}




/**Function********************************************************************

  Synopsis           [auxiliary part of bmc_tableauGetReleasesAtTime]

  Description        [Builds the release operator expression]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static be_ptr
bmc_tableauGetReleasesAtTime_aux(const BeEnc_ptr be_enc,
				 const node_ptr p, const node_ptr q,
				 const int time, const int k, const int l,
				 const int steps)
{
  be_ptr tableau_p;
  be_ptr tableau_q;
  be_ptr tableau_result;

  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );
  nusmv_assert (steps >= 1);

  tableau_p = BmcInt_Tableau_GetAtTime(be_enc, p, time, k, l);
  tableau_q = BmcInt_Tableau_GetAtTime(be_enc, q, time, k, l);

  if (steps == 1) {
    if (Bmc_Utils_IsNoLoopback(l)) { /* q & p */
      tableau_result = Be_And(BeEnc_get_be_manager(be_enc), 
			      tableau_p, tableau_q);
    } else { /* q */
      tableau_result = tableau_q;
    }
  }
  else { /* q & ( p | X(p R q) ) */
    be_ptr tableau_following =
      bmc_tableauGetReleasesAtTime_aux(be_enc, p, q,
				       Bmc_Utils_GetSuccTime(time, k, l),
				       k, l, steps - 1);
    tableau_result =
      Be_And(BeEnc_get_be_manager(be_enc),
	     tableau_q,
	     Be_Or(BeEnc_get_be_manager(be_enc),
		   tableau_p, tableau_following));
  }

  return tableau_result;
}


