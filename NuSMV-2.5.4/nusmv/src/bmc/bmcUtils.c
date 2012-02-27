/**CFile***********************************************************************

  FileName    [bmcUtils.c]

  PackageName [bmc]

  Synopsis    [Utilities for the bmc package]

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

#include "bmcInt.h"
#include "bmcUtils.h"
#include "bmcConv.h"

#include "parser/parser.h"
#include "parser/symbols.h"

#include <limits.h>

static char rcsid[] UTIL_UNUSED = "$Id: bmcUtils.c,v 1.19.12.3.2.5.6.15 2010-02-19 15:05:22 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define BMC_NO_LOOP   -(INT_MAX-1) /* must be negative! */
#define BMC_ALL_LOOPS BMC_NO_LOOP+1

/* ---------------------------------------------------------------------- */
/* You can define your own symbols for these: */
#define BMC_ALL_LOOPS_USERSIDE_SYMBOL "*"
#define BMC_NO_LOOP_USERSIDE_SYMBOL   "X"
/* ---------------------------------------------------------------------- */

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

static be_ptr
bmc_utils_costraint_from_string ARGS((BeEnc_ptr be_enc,
                                      BddEnc_ptr bdd_enc,
                                      const char* str,
                                      boolean accept_next_expr,
                                      Expr_ptr* node_expr));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Given a string representing a loopback possible value,
               returns the corresponding integer.  The (optional)
               parameter result will be assigned to OUTCOME_SUCCESS if the
               conversion has been successfully performed, otherwise
               to OUTCOME_GENERIC_ERROR is the conversion failed. If result is
               NULL, OUTCOME_SUCCESS is the aspected value, and an assertion
               is implicitly performed to check the conversion
               outcome.]

  Description [Use this function to correctly convert a string
               containing a loopback user-side value to the internal
               representation of the same loopback value]

  SideEffects [result will change if supplied]

  SeeAlso     []

******************************************************************************/
int Bmc_Utils_ConvertLoopFromString(const char* strValue, Outcome* result)
{
  Outcome res = OUTCOME_SUCCESS;
  int l = 0;

  if (strValue == NIL(char)) {
    res = OUTCOME_GENERIC_ERROR;
  }
  else if (Bmc_Utils_IsAllLoopbacksString(strValue))  {
    l = Bmc_Utils_GetAllLoopbacks();
  }
  else if (Bmc_Utils_IsNoLoopbackString(strValue))  {
    l = Bmc_Utils_GetNoLoopback();
  }
  else if (util_str2int(strValue, &l) == 0) {
    /* User could have supplied a private integer value which
       corresponds to a reserved value:: */
    if (Bmc_Utils_IsAllLoopbacks(l) || Bmc_Utils_IsNoLoopback(l)) {
      res = OUTCOME_GENERIC_ERROR;
    }
  }
  else res = OUTCOME_GENERIC_ERROR; /* bad string value */

  /* This implements the auto-check (to simplify coding): */
  if (result == NULL)  {nusmv_assert(res == OUTCOME_SUCCESS);}
  else  *result = res;

  return l;
}


/**Function********************************************************************

  Synopsis    [Given an integer containing the inner representation of
               the loopback value, returns as parameter the
               corresponding user-side value as string]

  Description [Inverse semantic of
               Bmc_Utils_ConvertLoopFromString. bufsize is the maximum
               buffer size]

  SideEffects [String buffer passed as argument will change]

  SeeAlso     [Bmc_Utils_ConvertLoopFromString]

******************************************************************************/
void Bmc_Utils_ConvertLoopFromInteger(const int iLoopback, char* szLoopback,
                                      const int _bufsize)
{
  int iCheck; /* for buffer operations checking only */
  int bufsize = _bufsize-2; /* to store terminator */

  nusmv_assert(bufsize > 0); /* one character+terminator at least! */
  szLoopback[bufsize+1] = '\0'; /* put terminator */

  if (Bmc_Utils_IsAllLoopbacks(iLoopback)) {
    iCheck = snprintf(szLoopback, bufsize, "%s",
          BMC_ALL_LOOPS_USERSIDE_SYMBOL);
    SNPRINTF_CHECK(iCheck, bufsize);
  }
  else if (Bmc_Utils_IsNoLoopback(iLoopback)) {
    iCheck = snprintf(szLoopback, bufsize, "%s", BMC_NO_LOOP_USERSIDE_SYMBOL);
    SNPRINTF_CHECK(iCheck, bufsize);
  }
  else {
    /* value is ok, convert to string: */
    iCheck = snprintf(szLoopback, bufsize, "%d", iLoopback);
    SNPRINTF_CHECK(iCheck, bufsize);
  }
}


/**Function********************************************************************

  Synopsis    [Returns true if l has the internally encoded "no loop"
               value]

  Description [This is supplied in order to hide the internal value of
               loopback which corresponds to the "no loop" semantic.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Bmc_Utils_IsNoLoopback(const int l)
{
  return (l == BMC_NO_LOOP)? true : false;
}


/**Function********************************************************************

  Synopsis    [Returns true if the given string represents the no
               loopback value]

  Description [This is supplied in order to hide the internal value of
               loopback which corresponds to the "no loop" semantic.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Bmc_Utils_IsNoLoopbackString(const char* str)
{
  return (strcmp(str, BMC_NO_LOOP_USERSIDE_SYMBOL) == 0)? true : false;
}


/**Function********************************************************************

  Synopsis    [Returns true if the given loop value represents a single
               (relative or absolute) loopback]

  Description [Both cases "no loop" and "all loops" make this function
               returning false, since these values are not single
               loops.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Bmc_Utils_IsSingleLoopback(const int l)
{
  return (Bmc_Utils_IsNoLoopback(l) == false)
    && (Bmc_Utils_IsAllLoopbacks(l) == false);
}


/**Function********************************************************************

  Synopsis    [Returns true if the given loop value represents the "all
               possible loopbacks" semantic]

  Description [This is supplied in order to hide the internal value of
               loopback which corresponds to the "all loops"
               semantic.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Bmc_Utils_IsAllLoopbacks(const int l)
{
  return (l == BMC_ALL_LOOPS)? true : false;
}


/**Function********************************************************************

  Synopsis    [Returns true if the given string represents the "all
               possible loops" value.]

  Description [This is supplied in order to hide the internal value of
               loopback which corresponds to the "all loops"
               semantic.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Bmc_Utils_IsAllLoopbacksString(const char* str)
{
  return (strcmp(str, BMC_ALL_LOOPS_USERSIDE_SYMBOL) == 0)? true : false;
}


/**Function********************************************************************

  Synopsis    [Returns the integer value which represents the "no loop"
               semantic]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Bmc_Utils_GetNoLoopback() { return BMC_NO_LOOP; }


/**Function********************************************************************

  Synopsis    [Returns the integer value which represents the "all loops"
               semantic]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Bmc_Utils_GetAllLoopbacks() { return BMC_ALL_LOOPS; }


/**Function********************************************************************

  Synopsis    [Returns a constant string which represents the "all loops"
               semantic.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
const char* Bmc_Utils_GetAllLoopbacksString()
{
  return BMC_ALL_LOOPS_USERSIDE_SYMBOL;
}


/**Function********************************************************************

  Synopsis    [Converts a relative loop value (wich can also be an
               absolute loop value) to an absolute loop value]

  Description [For example the -4 value when k is 10 is the value 6,
               but the value 4 (absolute loop value) is still 4]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Bmc_Utils_RelLoop2AbsLoop(const int upov_loop, const int k)
{
  if ((Bmc_Utils_IsNoLoopback(upov_loop))
      || (Bmc_Utils_IsAllLoopbacks(upov_loop))
      || (upov_loop >=0)) {
    return upov_loop;
  }
  else return k + upov_loop;
}


/**Function********************************************************************

  Synopsis    [Checks the (k,l) couple. l must be absolute.]

  Description [Returns OUTCOME_SUCCESS if k and l are compatible, otherwise
               return OUTCOME_GENERIC_ERROR]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Outcome Bmc_Utils_Check_k_l(const int k, const int l)
{
  Outcome ret = OUTCOME_GENERIC_ERROR;

  if ( (k>=0) &&                    /* k has to be non-negative in all cases */
       ( Bmc_Utils_IsNoLoopback(l)  /* the no-loop case */
         ||
         Bmc_Utils_IsAllLoopbacks(l) /* the all-loops case */
         ||
         ( (l>=0) && (l<k) )  /* the single-loop case with the new semantics */
        )
      )
    ret = OUTCOME_SUCCESS;

  return ret;
}


/**Function********************************************************************

  Synopsis    [Given time<=k and a \[l, k\] interval, returns next time,
               or BMC_NO_LOOP if time is equal to k and there is no
               loop]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Bmc_Utils_GetSuccTime(const int time, const int k, const int l)
{
  nusmv_assert((time < k) || (time==k && Bmc_Utils_IsNoLoopback(l)) );

  if (Bmc_Utils_IsNoLoopback(l))
    if (time<k)
      return (time + 1);
    else
      return l;
  else
    if (time < k-1)
      return (time + 1);
    else
      return l;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Search into a given string any symbol which belongs to a
               determined set of symbols, and expand each found
               symbol, finally returning the resulting string]

  Description [This function is used in order to perform the macro
               expansion of filenames. table_ptr is the pointer to a
               previously prepared table which fixes any
               corrispondence from symbol to strings to be
               substituited from.  table_len is the number of rows in
               the table (i.e. the number of symbols to search for.)]

  SideEffects [filename_expanded string data will change]

  SeeAlso     []

******************************************************************************/
void Bmc_Utils_ExpandMacrosInFilename(const char* filename_to_be_expanded,
                                      const SubstString* table_ptr,
                                      const size_t table_len,
                                      char* filename_expanded,
                                      size_t buf_len)
{
  int i;
  /* copy the source string into the destination one: */
  strncpy(filename_expanded, filename_to_be_expanded, buf_len);

  for(i=0; i < table_len; ++i) {
      apply_string_macro_expansion(table_ptr + i, filename_expanded, buf_len);
    } /* for each symbol template */
}




/**Function********************************************************************

  Synopsis    [Given a problem, and a solver containing a model for that
               problem, generates and prints a counter-example]

  Description [A trace is generated and printed using the currently
               selected plugin. Generated trace is returned, in order
               to make possible for the caller to do some other
               operation, like association with the checked
               property. Returned trace object *cannot* be destroyed
               by the caller.]

  SideEffects []

  SeeAlso     [Bmc_Utils_generate_cntexample Bmc_Utils_fill_cntexample]

******************************************************************************/
Trace_ptr Bmc_Utils_generate_and_print_cntexample(BeEnc_ptr be_enc,
                                                  SatSolver_ptr solver,
                                                  be_ptr be_prob,
                                                  const int k,
                                                  const char* trace_name,
                                                  NodeList_ptr symbols)
{
  Trace_ptr trace = \
    Bmc_Utils_generate_cntexample(be_enc, solver, be_prob,
                                  k, trace_name, symbols);

  /* Print the trace using default plugin */
  fprintf(nusmv_stdout,
          "-- as demonstrated by the following execution sequence\n");

  TraceManager_register_trace(global_trace_manager, trace);
  TraceManager_execute_plugin(global_trace_manager, TRACE_OPT(NULL),
                              TRACE_MANAGER_DEFAULT_PLUGIN,
                              TRACE_MANAGER_LAST_TRACE);
  return trace;
}



/**Function********************************************************************

  Synopsis   [Given a problem, and a solver containing a model for that
              problem, generates a counter-example]

  Description [Generated trace is returned, in order to make possible
               for the caller to do some other operation, like
               association with the checked property. Returned trace
               is non-volatile]

  SideEffects  []

  SeeAlso      [Bmc_Utils_generate_and_print_cntexample]

******************************************************************************/
Trace_ptr Bmc_Utils_generate_cntexample(BeEnc_ptr be_enc,
                                        SatSolver_ptr solver,
                                        be_ptr be_prob,
                                        const int k,
                                        const char* trace_name,
                                        NodeList_ptr symbols)
{
  return Bmc_create_trace_from_cnf_model(be_enc, symbols, trace_name,
             TRACE_TYPE_CNTEXAMPLE, SatSolver_get_model(solver), k);
}

/**Function********************************************************************

  Synopsis   [Given a solver containing a model for a
              problem, fills the given counter-example correspondingly]

  Description [The filled trace is returned. The given trace must be empty]

  SideEffects  []

  SeeAlso      [Bmc_fill_trace_from_cnf_model Bmc_Utils_generate_cntexample]

******************************************************************************/
Trace_ptr Bmc_Utils_fill_cntexample(BeEnc_ptr be_enc,
                                    SatSolver_ptr solver,
                                    const int k, Trace_ptr trace)
{
  return Bmc_fill_trace_from_cnf_model(be_enc, SatSolver_get_model(solver),
                                       k, trace);
}

/**Function********************************************************************

  Synopsis    [Creates a list of BE variables that are intended to be
               used by the routine that makes the state unique in
               invariant checking.]

  Description [If coi is enabled, than the returned list will contain
               only those boolean state variable the given property
               actually depends on.  Otherwise the full set of state
               boolean vars will occur in the list.  Frozen variables
               are not required, since they do not change from state
               to state, thus, cannot make a state distinguishable
               from other states.

               Returned list must be destroyed by the called.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
lsList Bmc_Utils_get_vars_list_for_uniqueness(BeEnc_ptr be_enc,
                                              Prop_ptr invarprop)
{
  SexpFsm_ptr bool_sexp_fsm;

  bool_sexp_fsm = SEXP_FSM(Prop_get_bool_sexp_fsm(invarprop));

  return Bmc_Utils_get_vars_list_for_uniqueness_fsm(be_enc, bool_sexp_fsm);
}


/**Function********************************************************************

  Synopsis    [Creates a list of BE variables that are intended to be
               used by the routine that makes the state unique in
               invariant checking.]

  Description [If coi is enabled, than the returned list will contain
               only those boolean state variable the given property
               actually depends on.  Otherwise the full set of state
               boolean vars will occur in the list.  Frozen variables
               are not required, since they do not change from state
               to state, thus, cannot make a state distinguishable
               from other states.

               Returned list must be destroyed by the called.]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
lsList Bmc_Utils_get_vars_list_for_uniqueness_fsm(BeEnc_ptr be_enc,
                                                  SexpFsm_ptr bool_sexp_fsm)
{
  SymbTable_ptr st;
  lsList crnt_state_be_vars;

  st = BaseEnc_get_symb_table(BASE_ENC(be_enc));
  crnt_state_be_vars = lsCreate();

  SEXP_FSM_CHECK_INSTANCE(bool_sexp_fsm);
  /* if coi was performed the list will not contain unnecessary vars */

  {
    NodeList_ptr vars = SexpFsm_get_vars_list(bool_sexp_fsm);
    ListIter_ptr iter;
    NODE_LIST_FOREACH(vars, iter) {
      node_ptr sexp_var = NodeList_get_elem_at(vars, iter);

      if (SymbTable_is_symbol_state_var(st, sexp_var)) {
        be_ptr be_var;
        lsStatus status;

        if (SymbTable_is_symbol_bool_var(st, sexp_var)) {
          be_var = BeEnc_name_to_untimed(be_enc, sexp_var);
          status = lsNewEnd(crnt_state_be_vars, (lsGeneric) be_var, 0);
          nusmv_assert(LS_OK == status);
        }
        else {
          /* scalar var, retrieves the list of bits that make its encoding */
          NodeList_ptr bits;
          ListIter_ptr bits_iter;
          bits = BoolEnc_get_var_bits(
            BoolEncClient_get_bool_enc(BOOL_ENC_CLIENT(be_enc)), sexp_var);
          NODE_LIST_FOREACH(bits, bits_iter) {
            node_ptr bit = NodeList_get_elem_at(bits, bits_iter);
            be_var = BeEnc_name_to_untimed(be_enc, bit);
            status = lsNewEnd(crnt_state_be_vars, (lsGeneric) be_var, 0);
            nusmv_assert(LS_OK == status);
          }
          NodeList_destroy(bits);
        }
      }
    } /* loop */
  }

  return crnt_state_be_vars;
}


/**Function********************************************************************

  Synopsis    [Applies inlining taking into account of current user
               settings]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
be_ptr Bmc_Utils_apply_inlining(Be_Manager_ptr be_mgr, be_ptr f)
{
  if (!opt_rbc_inlining(OptsHandler_get_instance())) return f;

  return Be_apply_inlining(be_mgr, f,
                           (!opt_rbc_inlining_lazy(OptsHandler_get_instance()) &&
                            opt_counter_examples(OptsHandler_get_instance())));
}


/**Function********************************************************************

  Synopsis    [Applies inlining forcing inclusion of the conjunct
               set. Useful in the incremental SAT applications to
               guarantee soundness]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
be_ptr Bmc_Utils_apply_inlining4inc(Be_Manager_ptr be_mgr, be_ptr f)
{
  if (!opt_rbc_inlining(OptsHandler_get_instance())) return f;

  return Be_apply_inlining(be_mgr, f, true);
}


/**Function********************************************************************

  Synopsis    [Reads a simple expression and builds the corresponding BE
               formula.]

  Description [Reads a simple expression and builds the corresponding
               BE formula. Exceptions are raised if the expression
               cannot be parsed or has type errors.]

  SideEffects [None]

  SeeAlso     [Bmc_Utils_next_costraint_from_string]

******************************************************************************/
be_ptr Bmc_Utils_simple_costraint_from_string(BeEnc_ptr be_enc,
                                              BddEnc_ptr bdd_enc,
                                              const char* str,
                                              Expr_ptr* node_expr)
{
  return bmc_utils_costraint_from_string(be_enc, bdd_enc, str,
                                         false, node_expr);
}


/**Function********************************************************************

  Synopsis    [Reads a next expression and builds the corresponding BE
               formula.]

  Description [Reads a next expression and builds the corresponding BE
               formula. Exceptions are raised if the expression cannot
               be parsed or has type errors. If node_expr is not NULL,
               it will be set to the parsed expression.]

  SideEffects [None]

  SeeAlso     [Bmc_Utils_simple_costraint_from_string]

******************************************************************************/
be_ptr Bmc_Utils_next_costraint_from_string(BeEnc_ptr be_enc,
                                            BddEnc_ptr bdd_enc,
                                            const char* str,
                                            Expr_ptr* node_expr)
{
  return bmc_utils_costraint_from_string(be_enc, bdd_enc, str,
                                         true, node_expr);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Reads an expression and builds the corresponding BE
               formula. If accept_next_expr is true, then a next
               expression is parsed, otherwise a simple expression is
               parsed. ]

  Description [Reads a either simple or next expression and builds the
               corresponding BE formula. Exceptions are raised if the
               expression cannot be parsed or has type
               errors. Internal service.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static be_ptr
bmc_utils_costraint_from_string(BeEnc_ptr be_enc,
                                BddEnc_ptr bdd_enc,
                                const char* str,
                                boolean accept_next_expr,
                                Expr_ptr* node_expr)
{
  node_ptr parsed_expr = Nil;
  be_ptr result = (be_ptr) NULL;

  int (*parse_fun)(const char*, node_ptr*) = accept_next_expr ? \
    Parser_ReadNextExprFromString : Parser_ReadSimpExprFromString;
  int node_type = accept_next_expr ? NEXTWFF : SIMPWFF;

  if (0 == parse_fun(str, &parsed_expr) &&
      Nil != parsed_expr &&
      node_type == node_get_type(parsed_expr)) {
    node_ptr bool_constraints;
    node_ptr constraints = car(parsed_expr);

    CATCH {
      if (!TypeChecker_is_expression_wellformed(
                    BaseEnc_get_type_checker(BASE_ENC(bdd_enc)),
                    constraints, Nil)) {
        error_type_system_violation();
      }

      /* here constraints are intended in context Nil */
      bool_constraints = Compile_detexpr2bexpr(bdd_enc, constraints);

      result = Bmc_Conv_Bexp2Be(be_enc, bool_constraints);
      if ((Expr_ptr*) NULL != node_expr) *node_expr = constraints;
    }
    FAIL {
      result = (be_ptr) NULL;
      if ((Expr_ptr*) NULL != node_expr) *node_expr = (Expr_ptr) NULL;
    }
  }

  if ((be_ptr) NULL == result) {
    rpterr("Conversion from expression to BE (aka RBC) failed.");
  }

  return result;
}
