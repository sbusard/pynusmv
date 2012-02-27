/**CFile***********************************************************************

  FileName    [traceTest.c]

  PackageName [trace]

  Synopsis    [Automated tests suite for the trace package]

  Description [This module contains self-testing code for the trace package]

  SeeAlso     []

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

#include "TraceManager.h"
#include "TraceOpt.h"
#include "Trace.h"
#include "pkg_traceInt.h"
#include "pkg_trace.h"
#include "prop/propPkg.h"
#include "enc/enc.h"
#include "cmd/cmd.h"
#include "utils/ucmd.h"
#include "exec/traceExec.h"
#include "bmc/bmcCmd.h"
#include "parser/symbols.h"
#include "enc/bool/BoolEnc.h"

static char rcsid[] UTIL_UNUSED = "$Id: $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef int (*trace_pkg_test_ptr)(FILE* out, FILE* err);

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
static int trace_test_setup ARGS((FILE* out, FILE* err));
static int trace_test_cleanup ARGS((FILE* out, FILE* err));

/* create a trace, perform a few basic tests on metadata */
static int trace_test_creation ARGS((FILE* out, FILE* err));

/* create a trace, perform language queries */
static int trace_test_language ARGS((FILE* out, FILE* err));

/* create a trace, perform queries on assignments */
static int trace_test_population ARGS((FILE* out, FILE* err));

/* perform trace copy */
static int trace_test_copy ARGS((FILE* out, FILE* err));

/* perform trace concatenation */
static int trace_test_concat ARGS((FILE* out, FILE* err));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
int TracePkg_test_package (FILE* out, FILE* err)
{
  int passed = 0, failed = 0;
  int i, res;
  trace_pkg_test_ptr tests[] = {
    trace_test_creation,
    trace_test_language,
    trace_test_population,
    trace_test_copy,
    trace_test_concat,
  };

  for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++ i) {
    trace_pkg_test_ptr f = tests[i];
    nusmv_assert( (trace_pkg_test_ptr)(NULL) !=  f);

    fprintf(out, "\n***** TEST %d: *****\n", i);

    /* run test */
    res = trace_test_setup(out, err) ||                          \
      (*f)(out, err) ||                                          \
      trace_test_cleanup(out, err);

    if (0 == res) {
      fprintf(out, "PASSED\n\n");
      ++ passed;
    }
    else {
      fprintf(out, "FAILED\n\n");
      ++ failed;
    }
  }

  fprintf(out, "%d tests performed, %d passed, %d failed.\n",
          (unsigned)(sizeof(tests) / sizeof(tests[0])), passed, failed);

  return ! (0 == failed);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
static int trace_test_setup(FILE* out, FILE* err)
{
  int res =  Cmd_CommandExecute("go");

  if (0 == res) {
    SymbTable_ptr st = Compile_get_global_symb_table();
    BoolEnc_ptr benc = BoolEncClient_get_bool_enc(\
                                     BOOL_ENC_CLIENT(Enc_get_bdd_encoding()));
    SymbTableIter iter;

    fprintf(out, "symbols: ");

    SymbTable_gen_iter(st, &iter, STT_VAR | STT_DEFINE);
    while (!SymbTable_iter_is_end(st, &iter)) {
      node_ptr symb = SymbTable_iter_get_symbol(st, &iter);

      if (BoolEnc_is_var_bit(benc, symb)) continue;
      print_node(out, symb); fprintf(out, " ");
      SymbTable_iter_next(st, &iter);
    }
  }
  else {
    fprintf(err, "%s:%d:%s: test setup failed\n",
            __FILE__, __LINE__, __func__);
  }

  return res;
}

static int trace_test_cleanup(FILE* out, FILE* err)
{
  return (Cmd_CommandExecute("reset"));
}

static int trace_test_creation (FILE* out, FILE* err)
{
  const char* desc = "My test trace #1";
  fprintf(out, "\n## Trace creation ##\n");

  Trace_ptr trace = TRACE(NULL);

  SexpFsm_ptr sexp_fsm = \
    PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());

  SymbTable_ptr st = SexpFsm_get_symb_table(sexp_fsm);

  SEXP_FSM_CHECK_INSTANCE(sexp_fsm);

  int i;

  /* try it a few times ... */
  for (i = 0; i < 10; ++ i ) {

    trace = Trace_create(st, desc, TRACE_TYPE_SIMULATION,
                         SexpFsm_get_symbols_list(sexp_fsm), true);

    /* a newly created traces has 0 length */
    if (0 != Trace_get_length(trace)) return 1;

    /* retrieve TRACE TYPE */
    if (TRACE_TYPE_SIMULATION != Trace_get_type(trace)) return 1;

    /* ... and description */
    if (0 != strcmp(Trace_get_desc(trace), desc)) return 1;

    /* a newly created trace is unregistered */
    if (Trace_is_registered(trace)) return 1;

    /* destroy it */
    Trace_destroy(trace);
  }

  return  0;
}

static int trace_test_language(FILE* out, FILE* err)
{
  fprintf(out, "\n## Trace language ##\n");

  Trace_ptr trace = TRACE(NULL);

  SexpFsm_ptr sexp_fsm = \
    PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());

  SymbTable_ptr st = SexpFsm_get_symb_table(sexp_fsm);

  SEXP_FSM_CHECK_INSTANCE(sexp_fsm);

  trace = Trace_create(st, NIL(char), TRACE_TYPE_SIMULATION,
                       SexpFsm_get_symbols_list(sexp_fsm), true);

  { /* test fsm symbols */
    NodeList_ptr symbs = SexpFsm_get_symbols_list(sexp_fsm);
    ListIter_ptr liter;

    NODE_LIST_FOREACH(symbs, liter) {
      node_ptr symb = NodeList_get_elem_at(symbs, liter);

      if (!Trace_symbol_in_language(trace, symb)) {
        return 1;
      }
    }
  }

  /* destroy it */
  Trace_destroy(trace);

  return 0;
}

static int trace_test_population (FILE* out, FILE* err)
{
  fprintf(out, "\n## Trace population ##\n");

  SexpFsm_ptr sexp_fsm = \
    PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());

  SymbTable_ptr st = SexpFsm_get_symb_table(sexp_fsm);

  Trace_ptr trace = TRACE(NULL);

  SEXP_FSM_CHECK_INSTANCE(sexp_fsm);

  trace = Trace_create(st, NIL(char), TRACE_TYPE_SIMULATION,
                       SexpFsm_get_symbols_list(sexp_fsm), true);

  int i; /* make up a few steps... */
  for (i = 0; i < 100; ++ i) {
    Trace_append_step(trace);
  }

  if (100 != Trace_get_length(trace)) {
    return 1;
  }

  { /* populate the trace with default values */
    TraceIter step;
    const TraceIter first = Trace_first_iter(trace);
    BoolEnc_ptr bool_enc = Enc_get_bool_encoding();
    i = 0;
    TRACE_FOREACH(trace, step) {
      TraceSymbolsIter sym_iter;
      node_ptr var;

      ++ i;
      TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_ALL_VARS, sym_iter, var) {
        node_ptr val = Trace_step_get_value(trace, step, var);
        if (Nil == val) { /* not assigned */
          BitValues_ptr bv  = BIT_VALUES(NULL);

          if (!SymbType_is_boolean(SymbTable_get_var_type(st, var))) {

            if (BoolEnc_is_var_bit(bool_enc, var)) {
              var = BoolEnc_get_scalar_var_from_bit(bool_enc, var);
              bv = BitValues_create(bool_enc, var);
            }
            else bv = BitValues_create(bool_enc, var);
          }

          /* no inputs on first step */
          if (!SymbTable_is_symbol_input_var(st, var) || (first != step)) {
            int tmp = Trace_step_put_value(trace, step, var,
                                 BIT_VALUES(NULL) != bv
                                 ? BoolEnc_get_value_from_var_bits(bool_enc,
                                                                   bv)
                                 : Expr_false());

            if (!tmp) {
              fprintf(nusmv_stderr, "halt\n");
              return 1;
            }
          }

          if (BIT_VALUES(NULL) != bv) { FREE(bv); }
        }
      }
    }
  }

  /* destroy it */
  Trace_destroy(trace);

  return 0;
}

static int trace_test_copy (FILE* out, FILE* err)
{
  fprintf(out, "\n## Trace copy ##\n");

  SexpFsm_ptr sexp_fsm = \
    PropDb_master_get_scalar_sexp_fsm(PropPkg_get_prop_database());

  SymbTable_ptr st = SexpFsm_get_symb_table(sexp_fsm);

  SEXP_FSM_CHECK_INSTANCE(sexp_fsm);

  Trace_ptr trace = Trace_create(st, NIL(char), TRACE_TYPE_SIMULATION,
                                 SexpFsm_get_symbols_list(sexp_fsm), true);

  int i; /* make up a few steps... */
  for (i = 0; i < 10; ++ i) {
    Trace_append_step(trace);
  }

  if (10 != Trace_get_length(trace)) {
    return 1;
  }

  { /* populate the trace with default values */
    TraceIter step;
    const TraceIter first = Trace_first_iter(trace);
    BoolEnc_ptr bool_enc = Enc_get_bool_encoding();
    i = 0;
    TRACE_FOREACH(trace, step) {
      TraceSymbolsIter sym_iter;
      node_ptr var;

      ++ i;
      TRACE_SYMBOLS_FOREACH(trace, TRACE_ITER_ALL_VARS, sym_iter, var) {
        node_ptr val = Trace_step_get_value(trace, step, var);
        if (Nil == val) { /* not assigned */
          BitValues_ptr bv  = BIT_VALUES(NULL);
          if (!SymbType_is_boolean(SymbTable_get_var_type(st, var))) {

            if (BoolEnc_is_var_bit(bool_enc, var)) {
              var = BoolEnc_get_scalar_var_from_bit(bool_enc, var);
              bv = BitValues_create(bool_enc, var);
            }
            else bv = BitValues_create(bool_enc, var);
          }

          /* no inputs on first step */
          if (!SymbTable_is_symbol_input_var(st, var) || (first != step)) {
            Trace_step_put_value(trace, step, var,
                                 BIT_VALUES(NULL) != bv
                                 ? BoolEnc_get_value_from_var_bits(bool_enc,
                                                                   bv)
                                 : Expr_false());
          }

          if (BIT_VALUES(NULL) != bv) { FREE(bv); }
        }
      }
    }
  }

  { /* make a full copy and compare the two traces */
    Trace_ptr copy = Trace_copy(trace, TRACE_END_ITER, true);
    if (!Trace_equals(trace, copy)) return 1;
    Trace_destroy(copy);
  }

  { /* do it again with frozen traces */
    Trace_freeze(trace);
    Trace_step_force_loopback(trace, Trace_ith_iter(trace, 3));

    Trace_ptr copy = Trace_copy(trace, TRACE_END_ITER, true);
    if (!Trace_equals(trace, copy)) return 1;

    Trace_destroy(copy);
  }

  Trace_destroy(trace);

  return 0;
}

static int trace_test_concat (FILE* out, FILE* err)
{
  fprintf(out, "\n## Trace concat ##\n");
  return 0;
}




