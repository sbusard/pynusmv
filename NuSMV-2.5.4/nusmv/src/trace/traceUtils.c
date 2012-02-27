/**CFile***********************************************************************

  FileName    [traceUtils.c]

  PackageName [trace]

  Synopsis    [This module contains support functions to the trace class.]

  Description [optional]

  SeeAlso     [optional]

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK.

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
#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif
#include "pkg_trace.h"
#include "parser/symbols.h"

#include "fsm/bdd/bdd.h"
#include "enc/enc.h"
#include "compile/compile.h"
#include "bmc/bmc.h"

static char rcsid[] UTIL_UNUSED = "$Id: $";

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


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Extracts assignments in (trace, step) to a set of symbols]

  Description [Builds a bdd representing the assignments from a given
               step in trace. The symbols to be assigned are picked
               according to \"iter_type\". Refer to documentation of
               the TraceIteratorType for possible sets.

               Remarks: returned bdd is referenced]

  SideEffects []

  SeeAlso     []

*****************************************************************************/
bdd_ptr TraceUtils_fetch_as_bdd(Trace_ptr trace, TraceIter step,
                                TraceIteratorType iter_type,
                                BddEnc_ptr bdd_enc)
{
  TRACE_CHECK_INSTANCE(trace);
  BDD_ENC_CHECK_INSTANCE(bdd_enc);

  DdManager* dd = BddEnc_get_dd_manager(bdd_enc);
  TraceStepIter iter;
  node_ptr var, val;
  SymbTable_ptr symb_table = Trace_get_symb_table(trace);

  bdd_ptr res = bdd_true(dd);
  TRACE_STEP_FOREACH(trace, step, iter_type, iter, var, val) {
    bdd_ptr tmp = BddEnc_expr_to_bdd(bdd_enc,
                                     Expr_equal(var, val, symb_table),
                                     Nil);
    bdd_and_accumulate(dd, &res, tmp);
    bdd_free(dd, tmp);
  }

  return res;
}


/**Function********************************************************************

  Synopsis    [Extracts assignments in (trace, step) to a set of symbols]

  Description [Builds a be representing the assignments from a given
               step in trace. The symbols to be assigned are picked
               according to \"iter_type\". Refer to documentation of
               the TraceIteratorType for possible sets.]

  SideEffects []

  SeeAlso     []

*****************************************************************************/
be_ptr TraceUtils_fetch_as_be(Trace_ptr trace, TraceIter step,
                              TraceIteratorType iter_type,
                              BeEnc_ptr be_enc, BddEnc_ptr bdd_enc)
{
  TraceStepIter iter;
  node_ptr var, val;
  Be_Manager_ptr be_mgr;

  TRACE_CHECK_INSTANCE(trace);
  BE_ENC_CHECK_INSTANCE(be_enc);
  BDD_ENC_CHECK_INSTANCE(bdd_enc);

  be_mgr = BeEnc_get_be_manager(be_enc);
  bdd_ptr res = Be_Truth(be_mgr);
  TRACE_STEP_FOREACH(trace, step, iter_type, iter, var, val) {
    be_ptr tmp = Bmc_Conv_Bexp2Be(be_enc, Compile_detexpr2bexpr(bdd_enc,
                                                        Expr_equal(var, val, SYMB_TABLE(NULL))));
    res = Be_And(be_mgr, res, tmp);
  }

  return res;
}


/**Function********************************************************************

  Synopsis    [Extracts assignments in (trace, step) to a set of symbols]

  Description [Builds a sexp representing the assignments from a given
               step in trace. The symbols to be assigned are picked
               according to \"iter_type\". Refer to documentation of
               the TraceIteratorType for possible sets.

               Remarks: returned expression is find-node'd]

  SideEffects []

  SeeAlso     [TraceUtils_fetch_as_big_and]

*****************************************************************************/
Expr_ptr TraceUtils_fetch_as_sexp(Trace_ptr trace, TraceIter step,
                                  TraceIteratorType iter_type)
{
  TRACE_CHECK_INSTANCE(trace);

  Expr_ptr res = Expr_true();
  node_ptr var, val;
  TraceStepIter iter;
  SymbTable_ptr symb_table = Trace_get_symb_table(trace);

  TRACE_STEP_FOREACH(trace, step, iter_type, iter, var, val) {
    res = Expr_and(Expr_equal(var, val, symb_table), res);
  }

  return res;
}

/**Function********************************************************************

  Synopsis    [Extracts assignments in (trace, step) to a set of symbols]

  Description [Do the same thing as TraceUtils_fetch_as_sexp, but do not
               simplify or reorder the pointers of expressions created.]

  SideEffects []

  SeeAlso     [TraceUtils_fetch_as_sexp]

*****************************************************************************/
Expr_ptr TraceUtils_fetch_as_big_and(Trace_ptr trace, TraceIter step,
                                     TraceIteratorType iter_type)
{
  TRACE_CHECK_INSTANCE(trace);

  Expr_ptr res = NULL;
  node_ptr var, val;
  TraceStepIter iter;
  SymbTable_ptr symb_table = Trace_get_symb_table(trace);


  TRACE_STEP_FOREACH(trace, step, iter_type, iter, var, val) {
    if ((Expr_ptr)NULL == res) {
      res = find_node(EQUAL, var, val);
    }
    else res = find_node(AND, find_node(EQUAL, var, val), res);
  }

  return res;
}
/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

