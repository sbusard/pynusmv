/**CFile***********************************************************************

  FileName    [bmcTest.c]

  PackageName [bmc]

  Synopsis    [Test routines for <tt>bmc</tt> package]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada, Marco Benedetti]

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

#include <math.h>

#include "bmc.h"
#include "bmcInt.h"
#include "bmcUtils.h"
#include "bmcTableau.h"
#include "bmcConv.h"

#include "wff/wff.h"
#include "wff/w2w/w2w.h"

#include "enc/enc.h"
#include "enc/be/BeEnc.h"
#include "be/be.h"

#include "fsm/be/BeFsm.h"

#include "prop/propPkg.h"
#include "parser/symbols.h" /* for constants */
#include "utils/error.h"


static char rcsid[] UTIL_UNUSED = "$Id: bmcTest.c,v 1.20.4.5.2.2.2.5.4.1 2007-06-18 16:50:53 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define GEN_WFF_CONSES_OP_NUMBER 15

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static int generated_formulas = 0;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static node_ptr
bmc_test_mk_loopback_ltl ARGS((const BeEnc_ptr be_enc,
			       const int k, const int l));

static node_ptr
bmc_test_gen_wff ARGS((const BeEnc_ptr be_enc,
		       int max_depth, int max_conns, 
		       boolean usePastOperators));

static node_ptr
bmc_test_gen_tableau ARGS((const BeFsm_ptr be_fsm, const node_ptr ltl_nnf_wff,
			   const int k, const int l, 
			   boolean usePastOperators));

static int UsageBmcTestTableau ARGS((void));

static void
bmc_test_bexpr_output ARGS((const BeEnc_ptr be_enc, FILE* f,
			    const node_ptr bexp, const int output_type));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Call this function to reset the test sub-package (into
  the reset command for example)]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Bmc_TestReset()
{
  generated_formulas = 0;
}


/**Function********************************************************************

  Synopsis           [The first time Bmc_TestTableau is called in the current
  session this function creates a smv file with a model and generates a random
  ltl spec to test tableau. The following times it is called it appends a new
  formula to the file.]

  Description        [If you call this command with a loopback set to
  BMC_ALL_LOOPS you command execution is aborted.]

  SideEffects        []

  SeeAlso            []

  CommandName        [_bmc_test_tableau]

  CommandSynopsis    [Generates a random formula to logically test the
  equivalent tableau]

  CommandArguments   [\[-h\] | \[-n property_index\] | \[\[ -d max_depth\] \[-c max_conns\] \[-o operator\]\]
  ]

  CommandDescription [Use this hidden command to generate random formulae and
  to test the equivalent tableau. The first time this command is called in the
  current NuSMV session it creates a new smv file with a model and generates a
  random ltl spec to test tableau.
  The following times it is called it appends a new formula to the file.
  The generated model contains the same number of non-deterministic variables
  the currently model loaded into NuSMV contains. <BR>
  You cannot call this command if the bmc_loopback is set to '*' (all loops).
  ]

******************************************************************************/
int Bmc_TestTableau(int argc, char ** argv)
{
  BeFsm_ptr be_fsm  = NULL;
  BeEnc_ptr be_enc = NULL;
  node_ptr tableau_test;
  node_ptr wff=NULL;
  int k, l, max_depth, max_conns;
  boolean usePastOperators = false;
  boolean crossComparison = false;

  char szLoopback[16];

  FILE *f,*f1,*f2;

  /* user can generate a random wff based on a specified operator: */
  enum GenWffOperator {
    GWO_None, GWO_Globally, GWO_Future, GWO_Until, GWO_Releases,
              GWO_Historically, GWO_Once, GWO_Since, GWO_Triggered
  } wff_operator;

  nusmv_assert(generated_formulas>=0);

  wff_operator = GWO_None;

  k = get_bmc_pb_length(OptsHandler_get_instance());

  l = Bmc_Utils_ConvertLoopFromString(get_bmc_pb_loop(OptsHandler_get_instance()), NULL);
  l = Bmc_Utils_RelLoop2AbsLoop(l, k);

  if (Bmc_Utils_IsAllLoopbacks(l)) {
    /* not implemented yet */
    fprintf (nusmv_stderr, "Error: the case 'all loops' is not implemented yet.\nPlease set the variable 'bmc_loopback' to another value.\n\n");
    return 1;
  }

  max_depth = -1;  max_conns = 1; /* default values */

  if (cmp_struct_get_bmc_setup(cmps) == 0) {
    fprintf (nusmv_stderr, "Please call bmc_setup before use this command.\n");
    return 1;
  }

  be_fsm  = PropDb_master_get_be_fsm(PropPkg_get_prop_database());
  be_enc = BeFsm_get_be_encoding(be_fsm);


  /* process command options */
  {
    int c;
    char* strNumber = NIL(char);
    char* szOperator= NIL(char);
    int prop_no;
    Prop_ptr ltlprop = PROP(NULL); /* The property being processed */
    node_ptr ltlspec;

    util_getopt_reset();
    while((c = util_getopt(argc, argv, "hn:d:c:po:x")) != EOF) {
      switch (c) {
      case 'h':
	return UsageBmcTestTableau();

      case 'n':
	{
	  char* err_occ[2];
	  strNumber = util_strsav(util_optarg);
	  err_occ[0] = "";
	  prop_no = strtol(strNumber, err_occ, 10);
	  if ((strcmp(err_occ[0], "") != 0)) {
	    fprintf(nusmv_stdout,
		    "Error: \"%s\" is not a valid value (must be integer).\n"
		    , strNumber);
	    return 1;
	  }
	}

	if (prop_no >= PropDb_get_size(PropPkg_get_prop_database()) || prop_no < 0) {
	  fprintf(nusmv_stdout,
		  "Error: \"%s\" is not a valid value, must be in the range [0,%d].\n",
		  strNumber, PropDb_get_size(PropPkg_get_prop_database())-1);
	  return 1;
	}
	
	ltlprop = PropDb_get_prop_at_index(PropPkg_get_prop_database(), prop_no);
	
	if (Prop_get_type(ltlprop) != Prop_Ltl) {
	  fprintf(nusmv_stderr,
		  "Error: property %d is not of type LTL\n", prop_no);
	  return 1;
	}

	/* here prop is ok */
	ltlspec = Prop_get_expr_core(ltlprop);
	ltlspec = Compile_FlattenSexpExpandDefine(Compile_get_global_symb_table(), 
						  ltlspec, Nil);
	wff = Wff2Nnf(Compile_detexpr2bexpr(Enc_get_bdd_encoding(), ltlspec));
	break;

	
      case 'd': /* for max_depth */
	{
	  char* err_occ[2];
	  strNumber = util_strsav(util_optarg);

	  err_occ[0] = "";
	  max_depth = strtol(strNumber, err_occ, 10);
	  if ((strcmp(err_occ[0], "") != 0)) {
	    fprintf(nusmv_stdout,
		    "Error: \"%s\" is not a valid value (must be integer).\n",
		    strNumber);
	    return 1;
	  }
	}
	break;

      case 'c': /* for max_conns */
	{
	  char* err_occ[2];
	  strNumber = util_strsav(util_optarg);

	  err_occ[0] = "";
	  max_conns = strtol(strNumber, err_occ, 10);
	  if ((strcmp(err_occ[0], "") != 0)) {
	    fprintf(nusmv_stdout,
		    "Error: \"%s\" is not a valid value (must be integer).\n",
		    strNumber);
	    return 1;
	  }
	}
	break;

      case 'p': /* for past operators */
	usePastOperators = true;
	break;

      case 'x':
	crossComparison = true;
	break;


      case 'o': /* operator specification */
	szOperator = util_strsav(util_optarg);
	if (strcmp(szOperator, "G")==0) wff_operator = GWO_Globally;
	else if (strcmp(szOperator, "F")==0) wff_operator = GWO_Future;
	else if (strcmp(szOperator, "U")==0) wff_operator = GWO_Until;
	else if (strcmp(szOperator, "R")==0) wff_operator = GWO_Releases;
	else if (strcmp(szOperator, "H")==0) wff_operator = GWO_Historically;
	else if (strcmp(szOperator, "O")==0) wff_operator = GWO_Once;
	else if (strcmp(szOperator, "S")==0) wff_operator = GWO_Since;
	else if (strcmp(szOperator, "T")==0) wff_operator = GWO_Triggered;

	if(!usePastOperators && (wff_operator == GWO_Historically ||
				 wff_operator == GWO_Once  ||
				 wff_operator == GWO_Since ||
				 wff_operator == GWO_Triggered ) ) {
	  fprintf(nusmv_stdout,
		  "Error: operator \"%s\" is not allowed, unless you turn on the \"p\" option.\n",
		  szOperator);
	  return 1;
	}

	if(wff_operator == GWO_None) {
	  fprintf(nusmv_stdout,
		  "Error: operator \"%s\" is not valid. Use G|F|X|U|R|Y|G|H|S|T\n",
		  szOperator);
	  return 1;
	}

	break;
      
      } /* switch */

    } /* while */

    if (argc>8) return UsageBmcTestTableau();
   
  }
  
  if (wff == NULL) {
    /* generates a random wff: */
    switch (wff_operator) {

    case GWO_None:
      wff = bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators);
      break;

    case GWO_Globally:
      wff = Wff_make_globally(bmc_test_gen_wff(be_enc, max_depth, 
						max_conns, usePastOperators));
      break;

    case GWO_Future:
      wff = Wff_make_eventually(bmc_test_gen_wff(be_enc, max_depth, max_conns, 
						  usePastOperators));
      break;

    case GWO_Until:
      wff = Wff_make_until(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
			    bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Releases:
      wff = Wff_make_releases(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
			       bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Historically:
      wff = Wff_make_historically(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Once:
      wff = Wff_make_once(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Since:
      wff = Wff_make_since(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
			    bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));
      break;

    case GWO_Triggered:
      wff = Wff_make_triggered(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
				bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));
      break;


    default:
      nusmv_assert(FALSE); /* no other types are expected here */
    }
    wff = Wff2Nnf(wff);
  }

  if (!crossComparison) {
    /* generates the test tableau */
    tableau_test = bmc_test_gen_tableau(be_fsm, wff, k, l, usePastOperators);

    /* writes down the imply formula */
    if (generated_formulas == 0) {
      int i=0;

      f = fopen("Bmc_TestTableau.smv", "w");
      nusmv_assert(f != NULL);

      /* writes down the non-deterministic model */
      fprintf(f, "MODULE main\nVAR\n");
      for (i = 0; i < BeEnc_get_state_vars_num(be_enc); i++) {
	fprintf(f, "p%d: boolean;\n", i);
      }
    }
    else {
      /* this command has already been invoked */
      f = fopen("Bmc_TestTableau.smv", "a");
      nusmv_assert(f != NULL);
    }

    Bmc_Utils_ConvertLoopFromInteger(l, szLoopback, sizeof(szLoopback));
    fprintf(f, "\n\n-- Property %d (k=%d, l=%s, max_depth=%d, max_conns=%d): \n",
	    generated_formulas, k, szLoopback, max_depth, max_conns);
    fprintf(f, "LTLSPEC ");

    ++generated_formulas;

    fprintf (f, "\n");
    bmc_test_bexpr_output(be_enc, f, tableau_test,
			  BMC_BEXP_OUTPUT_SMV);
    fprintf(f, "\n\n");

    fclose(f);
  }
  else {
    /* writes down the formula */
    if (generated_formulas == 0) {
      int i=0;

      f1 = fopen("Bmc_TestTableau_BMC.smv", "w");
      f2 = fopen("Bmc_TestTableau_BDD.smv", "w");
      nusmv_assert(f1 != NULL);
      nusmv_assert(f2 != NULL);

      /* writes down the non-deterministic model */
      fprintf(f1, "MODULE main\nVAR\n");
      fprintf(f2, "MODULE main\nVAR\n");
      for (i = 0; i < BeEnc_get_state_vars_num(be_enc); i++) {
	fprintf(f1, "p%d: boolean;\n", i);
	fprintf(f2, "p%d: boolean;\n", i);
      }
    }
    else {
      /* this command has already been invoked */
      f1 = fopen("Bmc_TestTableau_BMC.smv", "a");
      f2 = fopen("Bmc_TestTableau_BDD.smv", "a");
      nusmv_assert(f1 != NULL);
      nusmv_assert(f2 != NULL);
    }

    Bmc_Utils_ConvertLoopFromInteger(l, szLoopback, sizeof(szLoopback));
    fprintf(f1, "\n\n-- Property %d (k=%d, l=%s, max_depth=%d, max_conns=%d): \n",
	    generated_formulas, k, szLoopback, max_depth, max_conns);
    fprintf(f1, "LTLSPEC ");
    fprintf(f2, "\n\n-- Property %d (k=%d, l=%s, max_depth=%d, max_conns=%d): \n",
	    generated_formulas, k, szLoopback, max_depth, max_conns);
    fprintf(f2, "LTLSPEC ");

    ++generated_formulas;

    fprintf (f1, "\n");
    fprintf (f2, "\n");

    bmc_test_bexpr_output(be_enc, f1, wff, BMC_BEXP_OUTPUT_SMV);

    wff = Wff_make_implies(bmc_test_mk_loopback_ltl(be_enc, k, l), wff);


    bmc_test_bexpr_output(be_enc, f2, wff, BMC_BEXP_OUTPUT_SMV);

    fprintf(f1, "\n\n");
    fprintf(f2, "\n\n");

    fclose(f1);
    fclose(f2);
  }

  return 0;
}




/**Function********************************************************************

  Synopsis           [Usage string for Bmc_TestTableau]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
static int UsageBmcTestTableau(void)
{
  fprintf (nusmv_stderr,
"usage: _bmc_test_tableau [-h] | [-n <property_index>] | \n \
                         [[ -d <max_depth>] [-c <max_conns>] [-o <operator>] [-p]]\n");
  fprintf (nusmv_stderr, "  -h \t\t\tPrints the command usage.\n");
  fprintf (nusmv_stderr, "  -n <prop_idx> \tTest tableau of property represented by the given index\n");
  fprintf (nusmv_stderr, "  -d <max_depth>\tGenerates a random wff with the given max depth (default value is -1)\n");
  fprintf (nusmv_stderr, "  -c <max_conns>\tGenerates a random wff with the given max number of connectives\n\t\t\t(default value is 1).\n");
  fprintf (nusmv_stderr, "  -p \t\t\tGenerate future and past operators\n\t\t\t(only future operators are generated by default).\n");
  fprintf (nusmv_stderr, "  -o <operator> \tGenerates a random wff based on the specified operator, which will \n\t\t\tbe placed at top level. Valid values are G | F | U | R | H | O | S | T\n");
  fprintf (nusmv_stderr, "  -x \t\t\tGenerate two smv files with the same set of random formulas (not tautologies)\n\t\t\t with and without loop condition, respectively.\n");
  return 1;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [For each variable p in the set of state variables,
  generates the global equivalence of p and X^(loop length), starting from
  the loop start]

  Description [ In the following example we suppose the loop starts
  from 2 and finishes to 6 (the bound).

  <PRE>
        ,-----------.
        V           |
  o--o--o--o--o--o--o--o--o--o--o--o--o- (...continues indefinitely)
  0  1  2  3  4  5  6  7  8  9  10 11 12

  </PRE>


  In general all state variables in time 2 must be forced to be equivalent
  to the corresponding variables timed in 6, the variables in 3 to 7,
  and so on up to the variables in 6 (equivalent to variables in
  10). Then variables in 7 (or 3 again) must be forced to be equivalent
  to the varaibles in 11, and so on indefinitely.
  <BR><BR>
  In formula (let suppose we have only one boolean variable):
  <BR>
  (p2 <-> p6) && (p6 <-> p10) ...
  <BR><BR>
  In a more compact (and finite!) form, related to this example:
  XX(G (p <-> XXXX(p)))

  The first two neXtes force the formula to be effective only from the loop
  starting point.
  The generic formula implemented in the code is the following one:
  <PRE>
  X^(l) (G ((p0 <-> X^(k-l)(p0)) &&
            (p1 <-> X^(k-l)(p1)) &&
	                .
                        .
                        .
            (pn <-> X^(k-l)(pn)))
        )
  </PRE>
 where:
   p0..pn are all boolean variables into the model
   X^(n) is expanded to XXX..X n-times.
 Note that frozen vars can be ignored since they are always equal to their previous
 values]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr
bmc_test_mk_loopback_ltl(const BeEnc_ptr be_enc, const int k, const int l)
{
  node_ptr result;
  node_ptr bigand_vars;
  node_ptr single_var_eq;
  node_ptr var;
  int idx;
  int loop_len = 0;

  nusmv_assert( !Bmc_Utils_IsNoLoopback(l) && (l < k) );
  nusmv_assert( BeEnc_get_state_vars_num(be_enc) > 0 );

  loop_len = k-l;

  /* first cycle is performed manually, in order to optimize a bit */
  idx = BeEnc_get_first_untimed_var_index(be_enc, BE_VAR_TYPE_CURR);
  var = BeEnc_index_to_name(be_enc, idx);
  bigand_vars = Wff_make_iff(var, Wff_make_opnext_times(var, loop_len));

  /* iterates across the remaining variables: */
  idx = BeEnc_get_next_var_index(be_enc, idx, BE_VAR_TYPE_CURR);
  while (BeEnc_is_var_index_valid(be_enc, idx)) {
    var = BeEnc_index_to_name(be_enc, idx);
    single_var_eq = Wff_make_iff(var, Wff_make_opnext_times(var, loop_len));
    bigand_vars = Wff_make_and(bigand_vars, single_var_eq);
    idx = BeEnc_get_next_var_index(be_enc, idx, BE_VAR_TYPE_CURR);
  }

  result = Wff_make_globally(bigand_vars);
  result = Wff_make_opnext_times(result, l); /* shifts to loop starting point */

  return result;
}



/**Function********************************************************************

  Synopsis           [Given a WFF in NNF, converts it into a tableau
  formula, then back to WFF_(k,l) and returns WFF -> WFF_(k,l)]

  Description        [This function is used to test tableau formulae]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr
bmc_test_gen_tableau(const BeFsm_ptr be_fsm, const node_ptr ltl_nnf_wff,
		     const int k, const int l, boolean usePastOperators)
{
  node_ptr tableau_as_wff;
  node_ptr implies_formula;
  be_ptr tableau_k_l_ltl_wff;
  BeEnc_ptr be_enc = BeFsm_get_be_encoding(be_fsm);

  /* generates the tableau (with no fairness): */
  tableau_k_l_ltl_wff = Bmc_GetTestTableau(be_enc, ltl_nnf_wff, k, l);

  /* reconvert the tableau back to a wff_(k,l) */
  tableau_as_wff = Bmc_Conv_Be2Bexp(be_enc, tableau_k_l_ltl_wff);

  /* build the implies: */
  if (Bmc_Utils_IsNoLoopback(l)) {
    implies_formula = Wff_make_implies(tableau_as_wff, ltl_nnf_wff);
  }
  else {
    nusmv_assert(!Bmc_Utils_IsAllLoopbacks(l)); /* all loops are not allowed nowadays */
    implies_formula = Wff_make_implies(
			Wff_make_and(tableau_as_wff,
				      bmc_test_mk_loopback_ltl(be_enc, k, l)),
			ltl_nnf_wff);
  }

  return implies_formula;
}


/**Function********************************************************************

  Synopsis           [Builds a <b>random LTL WFF</b> with specified
  <tt>max</tt> depth and <tt>max</tt> connectives.]

  Description        []

  SideEffects        [node hash may change]

  SeeAlso            []

******************************************************************************/
static node_ptr bmc_test_gen_wff(const BeEnc_ptr be_enc,
				 int max_depth, int max_conns, 
				 boolean usePastOperators)
{
  int rnd;
  double rnd_tmp;

  /* generates a random number which refers to either a state variable or
     an operator. Propositional and future time operators are always
     allowed, whereas past time operators can be only generated when
     the "usePastOperators" flag is true.*/

  do {
   rnd_tmp = floor(rand()) / (RAND_MAX + 1.0);

   rnd = (int) floor((GEN_WFF_CONSES_OP_NUMBER + BeEnc_get_state_vars_num(be_enc))
		     * rnd_tmp) + 1;
  }
  while (!usePastOperators && (rnd>10 && rnd<=15));

  /* if depth or connses of wff are exausted get a random number such that:
     (rnd >= 0) && (rnd < 'number of state variables')... */
  if ((max_depth < 0) || (max_conns < 0)) {
    int idx; 
    rnd = (int) (((float) BeEnc_get_state_vars_num(be_enc) * rand()) /
		 (RAND_MAX + 1.0));

    idx = BeEnc_get_var_index_with_offset(be_enc, 
		  BeEnc_get_first_untimed_var_index(be_enc, BE_VAR_TYPE_CURR), 
		  rnd, BE_VAR_TYPE_CURR);
    
    /* ...then return correspondent state variable to the random integer */
    return BeEnc_index_to_name(be_enc, idx);
  }

  /* exclude atoms from depth and connses decrement contributes */
  if (rnd <= GEN_WFF_CONSES_OP_NUMBER) {
    --max_depth; --max_conns;
  }

  switch (rnd) {
  /* Propositional operators */
  case 1:
    return Wff_make_not(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 2:
    return Wff_make_or (bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
                         bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 3:
    return Wff_make_and(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
                         bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 4:
    return Wff_make_implies(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
                             bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 5:
    return Wff_make_iff(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
                         bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  /* Future operators */
  case 6:
    return Wff_make_opnext    (bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 7:
    return Wff_make_eventually(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 8:
    return Wff_make_globally  (bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 9:
    return Wff_make_until     (bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
                                bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));
  case 10:
    return Wff_make_releases  (bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
                                bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  /* Past operators */
  case 11:
    return Wff_make_opprec      (bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 12:
    return Wff_make_once        (bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 13:
    return Wff_make_historically(bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  case 14:
    return Wff_make_since       (bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
                                  bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));
  case 15:
    return Wff_make_triggered   (bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators),
                                  bmc_test_gen_wff(be_enc, max_depth, max_conns, usePastOperators));

  default: 
    { 
      int idx = BeEnc_get_var_index_with_offset(be_enc, 
	BeEnc_get_first_untimed_var_index(be_enc, BE_VAR_TYPE_CURR), 
	rnd - GEN_WFF_CONSES_OP_NUMBER - 1, BE_VAR_TYPE_CURR);

      return BeEnc_index_to_name(be_enc, idx);
    }
  }
}


/**Function********************************************************************

  Synopsis    [<b>Write</b> to specified FILE stream given node_ptr
  <b>formula</b> with specified <tt>output_type</tt> format. There are
  follow formats: <tt>BMC_BEXP_OUTPUT_SMV, BMC_BEXP_OUTPUT_LB</tt>]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
bmc_test_bexpr_output(const BeEnc_ptr be_enc, FILE* f,
		      const node_ptr bexp, const int output_type)
{
  int type;

  nusmv_assert(f != NULL);

  /* exit from recursion if given formula is Nil */
  if (bexp == Nil) return;

  /* assert that input formula type can't be a NEXT operator, that is
     used in model specification (section ASSIGN). We use here only OP_NEXT
     operator used in the module (section LTLSPEC). */
  nusmv_assert (node_get_type (bexp) != NEXT);

  type = node_get_type (bexp);

  switch (type) {
  case FALSEEXP:                        /* FALSEEXP  */
    fprintf (f, "%s", (output_type == BMC_BEXP_OUTPUT_SMV) ? "0" : "false");
    break;

  case TRUEEXP:                         /* TRUEEXP   */
    fprintf (f, "%s", (output_type == BMC_BEXP_OUTPUT_SMV) ? "1" : "true");
    break;

  case AND:                             /* AND       */
    fprintf(f, "(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf(f, " %s ", (output_type == BMC_BEXP_OUTPUT_SMV) ? "&" : "/\\");
    bmc_test_bexpr_output(be_enc, f, cdr (bexp), output_type);
    fprintf(f, ")");
    break;

  case OR:                              /* OR        */
    fprintf (f, "(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, " %s ", (output_type == BMC_BEXP_OUTPUT_SMV) ? "|" : "\\/");
    bmc_test_bexpr_output(be_enc, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case NOT:                             /* NOT       */
    fprintf (f, "%c", (output_type == BMC_BEXP_OUTPUT_SMV) ? '!' : '~');
    fprintf (f, "(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case IMPLIES:                         /* IMPLIES   */
    fprintf (f, "(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, " -> ");
    bmc_test_bexpr_output(be_enc, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case IFF:                             /* IFF       */
    fprintf (f, "(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, " <-> ");
    bmc_test_bexpr_output(be_enc, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_FUTURE:                       /* OP_FUTURE */
    fprintf (f, "F(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_ONCE:                       /* OP_ONCE */
    fprintf (f, "O(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_GLOBAL:                       /* OP_GLOBAL */
    fprintf (f, "G(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_HISTORICAL:                       /* OP_HISTORICAL */
    fprintf (f, "H(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, ")");
    break;

  case UNTIL:                           /* UNTIL     */
    fprintf (f, "(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, " U ");
    bmc_test_bexpr_output(be_enc, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case SINCE:                           /* SINCE     */
    fprintf (f, "(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, " S ");
    bmc_test_bexpr_output(be_enc, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case RELEASES:                        /* RELEASES  */
    fprintf (f, "(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, " V ");
    bmc_test_bexpr_output(be_enc, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case TRIGGERED:                       /* TRIGGERED  */
    fprintf (f, "(");
    bmc_test_bexpr_output(be_enc, f, car (bexp), output_type);
    fprintf (f, " T ");
    bmc_test_bexpr_output(be_enc, f, cdr (bexp), output_type);
    fprintf (f, ")");
    break;

  case OP_NEXT:                         /* OP_NEXT   */
    {
      node_ptr temp_bexp = bexp;
      int i = 0;
      {
        /* prints out "X(" suffix while OP_NEXT is encountred */
        do {
          fprintf (f, "X(");
          temp_bexp = car(temp_bexp);
          nusmv_assert(temp_bexp != Nil);
          i++;
        } while (node_get_type(temp_bexp) == OP_NEXT);
      }

      /* then print the internal bexp */
      bmc_test_bexpr_output(be_enc, f, temp_bexp, output_type);

      while ((i--) > 0) fprintf (f, ")");
    }
    break;

  case OP_PREC:                         /* OP_PREC   */
    {
      node_ptr temp_bexp = bexp;
      int i = 0;
      {
        /* prints out "Y(" suffix while OP_PREC is encountred */
        do {
          fprintf (f, "Y(");
          temp_bexp = car(temp_bexp);
          nusmv_assert(temp_bexp != Nil);
          i++;
        } while (node_get_type(temp_bexp) == OP_PREC);
      }

      /* then print the internal bexp */
      bmc_test_bexpr_output(be_enc, f, temp_bexp, output_type);

      while ((i--) > 0) fprintf (f, ")");
    }
    break;

  case OP_NOTPRECNOT:                         /* OP_PREC   */
    {
      node_ptr temp_bexp = bexp;
      int i = 0;
      {
  /* prints out "Z(" suffix while OP_NOTPRECNOT is encountred */
        do {
          fprintf (f, "Z(");
          temp_bexp = car(temp_bexp);
          nusmv_assert(temp_bexp != Nil);
          i++;
        } while (node_get_type(temp_bexp) == OP_NOTPRECNOT);
      }

      /* then print the internal bexp */
      bmc_test_bexpr_output(be_enc, f, temp_bexp, output_type);

      while ((i--) > 0) fprintf (f, ")");
    }
    break;


  default:                              /* (default action) */
    {
      be_ptr r;

      /* gets the the be correspondent to the state variable */
      r = BeEnc_name_to_untimed(be_enc, bexp);

      /* if bexp is a really state variable, then prints out the index
         of correspondent be variable */
      if (r != (be_ptr) NULL) {
        fprintf(f, "p%d", Be_Var2Index(BeEnc_get_be_manager(be_enc), r));
      }
      else {
	internal_error("bmc_test_bexpr_output: given wff atom isn\' in BE environ\n");
      }
    }
  }

}





