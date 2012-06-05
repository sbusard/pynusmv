/**CFile***********************************************************************

  FileName    [compileCheck.c]

  PackageName [compile]

  Synopsis    [Performs semantic checks on the model.]

  Description [The routines to perform some the semantic checks.<br>
  The performed checks are:
  <ul>
  <li>undefined symbols</li>
  <li>multiple definition of symbols</li>
  <li>circular definition of symbols</li>
  <li>assignment to input variables</li>
  </ul>]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2.
  Copyright (C) 1998-2001 by CMU and FBK-irst.

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

#include "compileInt.h"

#include "compile/symb_table/SymbTable.h"

#include "parser/symbols.h"
#include "utils/ustring.h"
#include "utils/error.h"
#include "utils/assoc.h"
#include "utils/error.h"
#include "utils/utils_io.h"
#include "compile/symb_table/ResolveSymbol.h"

static char rcsid[] UTIL_UNUSED = "$Id: compileCheck.c,v 1.5.2.24.2.1.2.19.4.18 2010-01-18 17:06:10 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define CHECK_NEXT_HAS_NO_NEXT 1
#define CHECK_NEXT_HAS_NEXT 2
#define CHECK_NEXT_EVALUATING 3



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [The constant hash used in the checking phase.]

  Description [The role is similar to that of <tt>constant_hash</tt>,
  but the associated value is not an ADD, it's the atom itself. It is
  used only for checking purpose.]

  SeeAlso     [constant_hash]

******************************************************************************/
static hash_ptr check_constant_hash;
void init_check_constant_hash() {
  check_constant_hash = new_assoc();
 }
static void insert_check_constant_hash(node_ptr x, node_ptr y) {
  insert_assoc(check_constant_hash, x, y);
}
static node_ptr lookup_check_constant_hash(node_ptr x) {
  return(find_assoc(check_constant_hash, x));
}
void clear_check_constant_hash() {
  clear_assoc(check_constant_hash);
}

void deinit_check_constant_hash() {
  free_assoc(check_constant_hash);
}

/**Variable********************************************************************

  Synopsis    [The hash table for Compile_check_next memoization]

  Description [The hash table for Compile_check_next memoization]

  SeeAlso     []

******************************************************************************/
static hash_ptr check_next_hash = (hash_ptr)NULL;
void init_check_next_hash()
{
  check_next_hash = new_assoc();
}
static void insert_check_next_hash(node_ptr k, node_ptr v)
{
  insert_assoc(check_next_hash, k, v);
}

static node_ptr lookup_check_next_hash(node_ptr k)
{
  return find_assoc(check_next_hash, k);
}

void clear_check_next_hash()
{
  clear_assoc(check_next_hash);
}

void deinit_check_next_hash()
{
  free_assoc(check_next_hash);
}

/**Variable********************************************************************

  Synopsis    [Hash used to check assignments.]

  Description [This hash is used to detect multiple definitions. It
  associates to each symbol of the form <em>next(x)</em>,
  <em>init(x)</em>, <em>x</em>, the line number of the input file
  where a value is assigned to it (eg. <em>next(x) := expr</em>).]

  SeeAlso     []

******************************************************************************/
static hash_ptr global_assign_hash;
static void init_global_assign_hash() { global_assign_hash = new_assoc(); }
static void insert_global_assign_hash(node_ptr x, node_ptr y) {
  insert_assoc(global_assign_hash, x, y);
}
static node_ptr lookup_global_assign_hash(node_ptr x) {
  return(find_assoc(global_assign_hash, x));
}
static void clear_global_assign_hash() {clear_assoc(global_assign_hash);}
static void deinit_global_assign_hash() { free_assoc(global_assign_hash); }

/**Variable********************************************************************

  Synopsis    [Hash used to check multiple and circular assignments.]

  Description [This hash is used in two different phases of the
  checking.
  <ol>
  <li>The first deal with multiple definition. During this phase
      the data associated to symbols of the form <em>next(x)</em>,
      <em>init(x)</em> is the body of the assignment (eg, if you have the
      following assignment <em>next(x) := x & y;</em>, then the data
      associated to the symbol <em>next(x)</em> is <em>x & y</em>).</li>
  <li>The second deal with circular definition. In this phase the data
       associated to each symbol is extracted to check the body, and it is
       replaced with <tt>FAILURE_NODE</tt> or <tt>CLOSED_NODE</tt>.</li>
  </ol>]

  SeeAlso     []

******************************************************************************/
static hash_ptr assign_hash;
static void init_assign_hash() { assign_hash = new_assoc(); }
static void insert_assign_hash(node_ptr x, node_ptr y) { insert_assoc(assign_hash, x, y);}
static node_ptr lookup_assign_hash(node_ptr x) {return(find_assoc(assign_hash, x));}
static void clear_assign_hash() {clear_assoc(assign_hash);}
static void deinit_assign_hash() { free_assoc(assign_hash); }


/**Variable********************************************************************

  Synopsis    [The hash table to record which expressions already checked
  for correct use of input vars.]

  Description [ Only compileCheckForInputVars is allowed to initialize
  and free the hash table and only compileCheckNoNextInputs
  (through compileCheckTransForInputVars and
  compileCheckAssignForInputVars) is allowed to work with the hash table.

  The association is exp->constant, where constant can be 0 (exp has
  not been processed yet), 1 (exp has been processed).]

  SeeAlso     []

******************************************************************************/
static hash_ptr check_inputs_hash = (hash_ptr)NULL;


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void compileCheckInitForInputVars ARGS((SymbTable_ptr, node_ptr));
static void compileCheckInvarForInputVars ARGS((SymbTable_ptr, node_ptr));
static void compileCheckTransForInputVars ARGS((SymbTable_ptr, node_ptr));
static void compileCheckAssignForInputVars ARGS((SymbTable_ptr, node_ptr,
                                                 FlatHierarchy_ptr hierarchy));
static void compileCheckNoNextInputs ARGS((SymbTable_ptr symb_table,
                                           node_ptr expr, node_ptr ctx));


static void check_circular_assign ARGS((const SymbTable_ptr symb_table, node_ptr,
                                        node_ptr, boolean, boolean, boolean));

static void init_check_program ARGS((node_ptr l));
static void check_circ ARGS((const SymbTable_ptr symb_table, node_ptr n,
                             node_ptr context, boolean, boolean));

static boolean check_next ARGS((const SymbTable_ptr symb_table, node_ptr n,
                                node_ptr context, boolean is_next_allowed,
                                boolean is_next));

static void check_case ARGS((node_ptr expr));

static void check_assign ARGS((const SymbTable_ptr symb_table, node_ptr n,
                               node_ptr context, int mode));

static void check_assign_both ARGS((node_ptr v, int node_type, int lineno));
static void error_circular_assign ARGS((node_ptr n));
static void error_nested_next ARGS((node_ptr s));
static void error_unexpected_next ARGS((node_ptr s));
static void compile_check_print_io_atom_stack_assign ARGS((FILE *));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Semantic checks on assignments of the module.]

  Description        [
  The function checks that there are no multiple assignments and
  circular definitions.<br> Then the functions tries to detect
  multiple assignments between different modules.]

  SideEffects  []

******************************************************************************/
void Compile_CheckAssigns(const SymbTable_ptr symb_table, node_ptr procs)
{
  node_ptr procs_list = procs;

  /* Initialization of the hashes */
  init_global_assign_hash();
  init_assign_hash();

  /* Initializes check_constant_hash with process_selector elements. */
  init_check_program(map(car, procs));

  while (procs_list) { /* Loops over processes */
    node_ptr context = car(car(procs_list));
    node_ptr assign_expr = cdr(car(procs_list));

    /* Checks for multiple assignments: */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      print_in_process("checking for multiple assignments", context);
    }

    check_assign(symb_table, assign_expr, Nil, 0);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stdout, "Done\n");
    }

    /* Checks for circular assignments: */
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      print_in_process("checking for circular assignments", context);
    }

    check_assign(symb_table, assign_expr, Nil, 1);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stdout, "Done\n");
    }

    clear_assign_hash();

    procs_list = cdr(procs_list);
  }

  {
    SymbTableIter iter;

    /* checks state variables */
    SYMB_TABLE_FOREACH(symb_table, iter, STT_STATE_VAR | STT_FROZEN_VAR) {
      node_ptr v = SymbTable_iter_get_symbol(symb_table, &iter);
      int lineno = NODE_TO_INT(lookup_global_assign_hash(v));

      if (lineno != 0) {
        check_assign_both(v, NEXT, lineno);
        check_assign_both(v, SMALLINIT, lineno);
      }
    }
  }

  clear_global_assign_hash();
  clear_assign_hash();
  deinit_global_assign_hash();
  deinit_assign_hash();

  if (opt_batch(OptsHandler_get_instance())) { clear_check_constant_hash(); }
}


/**Function********************************************************************

  Synopsis           [Checks expressions for illegal occurrences of input vars]

  Description        [Checks the TRANS, INIT, INVAR and ASSIGN statements to
  make sure that input variables are not used where they should not be. That
  is, anywhere in a TRANS, INIT or INVAR statement and within next expressions
  in the init and next sections of an ASSIGN statement.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void compileCheckForInputVars(const SymbTable_ptr symb_table,
                              node_ptr trans_expr,
                              node_ptr init_expr,
                              node_ptr invar_expr,
                              node_ptr assign_expr,
                              FlatHierarchy_ptr hierarchy)
{
  compileCheckInitForInputVars(symb_table, init_expr);
  compileCheckInvarForInputVars(symb_table, invar_expr);

  check_inputs_hash = new_assoc(); /* memoization for below functions */

  compileCheckTransForInputVars(symb_table, trans_expr);
  compileCheckAssignForInputVars(symb_table, assign_expr, hierarchy);

  free_assoc(check_inputs_hash);
  check_inputs_hash = (hash_ptr)NULL; /* for debugging */
}


void Compile_check_case(node_ptr expr)
{
  check_case(expr);
}


/**Function********************************************************************

  Synopsis [Checks that given expression contains either no nested
  next, or no next operator at all.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Compile_check_next(const SymbTable_ptr st,
                        node_ptr expr, node_ptr context,
                        boolean is_one_next_allowed)
{
  extern int yylineno;
  int cur_lineno = yylineno;

  if (expr == Nil) return;
  yylineno = node_get_lineno(expr);
  check_next(st, expr, context, is_one_next_allowed, false);
  yylineno = cur_lineno;
}


/**Function********************************************************************

  Synopsis [Checks that given expression contains either no input
  variables in next.]

  Description        [It outputs an error message (and rises an exception)
  iff the expression contains a next statement which itself has an
  input variable in it.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Compile_check_input_next(const SymbTable_ptr st,
                              node_ptr expr, node_ptr context)
{
  nusmv_assert((hash_ptr) NULL == check_inputs_hash);
  check_inputs_hash = new_assoc(); /* memoization for below functions */

  CATCH {
    compileCheckNoNextInputs(st, expr, context);
  }
  FAIL {
    free_assoc(check_inputs_hash);
    check_inputs_hash = (hash_ptr)NULL; /* for debugging */
    rpterr(NULL); /* rethrow */
  }

  free_assoc(check_inputs_hash);
  check_inputs_hash = (hash_ptr)NULL; /* for debugging */
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Checks flattened init statement for input variables]

  Description        [If the flattened init statement contains input
  variables then this function will print out an error message.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void compileCheckInitForInputVars(SymbTable_ptr symb_table,
                                         node_ptr init)
{
  Set_t deps = Formula_GetDependencies(symb_table, init, Nil);

  if (SymbTable_list_contains_input_var(symb_table, Set_Set2List(deps))) {
    error_init_exp_contains_input_vars(init);
  }
  Set_ReleaseSet(deps);
}



/**Function********************************************************************

  Synopsis           [Checks flattened invar statement for input variables]

  Description        [If the flattened invar statement contains input
  variables then this function will print out an error message.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void compileCheckInvarForInputVars(SymbTable_ptr symb_table,
                                          node_ptr invar)
{
  Set_t deps = Formula_GetDependencies(symb_table, invar, Nil);

  if (SymbTable_list_contains_input_var(symb_table, Set_Set2List(deps))) {
    error_invar_exp_contains_input_vars(invar);
  }
  Set_ReleaseSet(deps);
}


/**Function********************************************************************

  Synopsis           [Checks flattened trans statement for input variables]

  Description        [If the flattened trans statement contains input
  variables within next() statements then this function will print out an
  error message.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void compileCheckTransForInputVars(SymbTable_ptr symb_table,
                                          node_ptr trans)
{
  if (trans != Nil) {
    compileCheckNoNextInputs(symb_table, trans, Nil);
  }
}


/**Function********************************************************************

  Synopsis           [Checks flattened assign statement for input variables]

  Description        [If the flattened assign statement contains input
  variables then this function will print out an error message. Note that
  input variables are allowed in some parts of an assign statement. They're
  not allowed anywhere in an init section and cannot be contained within a
  next statement inside a next declaration.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void compileCheckAssignForInputVars(SymbTable_ptr symb_table,
                                           node_ptr assign,
                                           FlatHierarchy_ptr hierarchy)
{
  if (assign == Nil) return;

  switch (node_get_type(assign)) {
  case CONS:
  case AND:
    compileCheckAssignForInputVars(symb_table, car(assign), hierarchy);
    compileCheckAssignForInputVars(symb_table, cdr(assign), hierarchy);
    break;

  case DOT:
  case ARRAY: /* process name => skip it */
    break;

  case EQDEF: {
    node_ptr stored;
    node_ptr name = car(assign);
    yylineno = node_get_lineno(assign);

    switch (node_get_type(name)) {
    case ARRAY:
      if (SymbTable_is_symbol_input_var(symb_table, name)) {
        error_assign_exp_contains_input_vars(name);
      }
      break;

    case NEXT:
      /* We don't care about presence of input vars in next assign,
         but we check for the presence of references to next of input
         variables. Defines are taken into account by expanding them
         before performing this check. */
      name = find_atom(name);
      stored = FlatHierarchy_lookup_assign(hierarchy, name);

      if (Nil != stored) {
        /* checks that the right value does not contain next(inputs) */
        compileCheckNoNextInputs(symb_table, stored, Nil);
      }

      break;

    case DOT: /* only resolved identifiers can be here */
    case SMALLINIT:
      if (SMALLINIT == node_get_type(name)) name = find_atom(name);
      /* For normal assignments and init assignments we verify the rhs
         does not contain input variables. In this respect we have to
         look at the flattened assign, since from flattened symbols we
         can see whether they are input or state variables. */
      stored = FlatHierarchy_lookup_assign(hierarchy, name);

      if (Nil != stored) {
        Set_t deps = Formula_GetDependencies(symb_table, stored, Nil);
        if (SymbTable_list_contains_input_var(symb_table, Set_Set2List(deps))) {
          error_assign_exp_contains_input_vars(name);
        }
        Set_ReleaseSet(deps);
      }
      break;

    default:
      fprintf(nusmv_stderr,
              "compileCheckAssignForInputVars: unrecognised token (%d)\n",
              node_get_type(name));
      internal_error("");
    } /* internal (EQDEF) switch */

    break;
  }

  default:
    fprintf(nusmv_stderr, "compileCheckAssignForInputVars: unknown token (%d)\n",
            node_get_type(assign));
    internal_error("");

  } /* switch */
}


/**Function********************************************************************

  Synopsis    [Checks expression for input variables in next statements]

  Description [It outputs an error message (and rises an exception)
  iff the expression contains a next statement which itself has an
  input variable in it.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void compileCheckNoNextInputs(SymbTable_ptr symb_table,
                                     node_ptr expr, node_ptr ctx)
{
  node_ptr expr_to_remember = Nil; /* expression which will be memoized */

  if (expr == Nil) return;

  switch (node_get_type(expr)) {
  case FAILURE:
  case NUMBER:
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case UWCONST:
  case SWCONST:
    return;

  case ATOM:
  case BIT:
  case DOT:
  case ARRAY:
    {
      ResolveSymbol_ptr rs;
      node_ptr resName;

      rs = SymbTable_resolve_symbol(symb_table, expr, Nil);
      resName = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_error(rs)) ResolveSymbol_throw_error(rs);

      /* dotted constants or a variable, ar an array */
      if (ResolveSymbol_is_constant(rs) ||
          ResolveSymbol_is_var(rs) ||
          ResolveSymbol_is_array(rs)) {
        return;
      }

      /* this identifier is a complex expression and may have
         been processed already */
      expr_to_remember = resName;
      if (find_assoc(check_inputs_hash, expr_to_remember)) return;

      /* is this a define ? -> recur into flattened body */
      if (ResolveSymbol_is_define(rs)) {
        expr = SymbTable_get_define_flatten_body(symb_table, resName);
      }
      /* is this a array define ? -> recur into flattened body */
      else if (ResolveSymbol_is_array_def(rs)) {
        expr = SymbTable_get_array_define_flatten_body(symb_table, resName);
      }
      /* or a parameter ? */
      else if (ResolveSymbol_is_parameter(rs)) {
        expr = SymbTable_get_flatten_actual_parameter(symb_table, resName);
      }
      /* unknown kind of identifier */
      else {
        internal_error("impossible code in function compileCheckNoNextInputs");
      }

      compileCheckNoNextInputs(symb_table, expr, ctx);
    }
    break;

  case NEXT:
    {
      /* this expr may have been processed already */
      expr_to_remember = find_node(CONTEXT, ctx, expr);
      if (find_assoc(check_inputs_hash, expr_to_remember)) return;

      Set_t deps = Formula_GetDependencies(symb_table, expr, ctx);
      boolean res = SymbTable_list_contains_input_var(symb_table,
                                                      Set_Set2List(deps));
      Set_ReleaseSet(deps);

      if (res) {
        extern int yylineno;
        yylineno = node_get_lineno(expr);
        error_next_exp_contains_input_vars(expr);
        /* this code does not return */
      }
    }
    break;

  default:
    /* this expr may have been processed already */
    expr_to_remember = find_node(CONTEXT, ctx, expr);
    if (find_assoc(check_inputs_hash, expr_to_remember)) return;

    compileCheckNoNextInputs(symb_table, car(expr), ctx);
    compileCheckNoNextInputs(symb_table, cdr(expr), ctx);
    break;
  }

  /* remember the expression already processed.  Note: here only
     complex expressions are memorized. The simple expressions such as
     numbers, constants and vars are not in hash (hopefully decreasing such
     a ways the size of the hash). */
  insert_assoc(check_inputs_hash, expr_to_remember, NODE_PTR(1));

  nusmv_assert(expr_to_remember != Nil); /* the expr has to be a real expression */
  return;
}


/**Function********************************************************************

  Synopsis           [Initializes the data structure to perform semantic
  checks.]

  Description        [The input should be a list of processes names.
  Loops over the list of process names
  and inserts the process symbolic name in the <tt>check_constant_hash</tt>.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void init_check_program(node_ptr l)
{
  if (l == Nil) internal_error("init_check_program: l = Nil");
  if (cdr(l) == Nil) {
    node_ptr v = find_atom(car(l));
    if (lookup_check_constant_hash(v)) return;
    if (v && (node_get_type(v) == ATOM)) insert_check_constant_hash(v, v);
    return;
  }
  else {
    init_check_program(odd_elements(l));
    init_check_program(even_elements(l));
  }
  return;
}

/**Function********************************************************************

  Synopsis           [Checks for circular definitions.]

  Description        [This function checks for circular definition of
  any kind. This function is able to detect circularity of the
  following kinds:
  <ul>
     <li><code>next(x) := alpha(next(x))<code></li>
     <li><code>next(x) := next(alpha(x))<code></li<
     <li>any combination of the two above.</li>
     <li><code>x := alpha(x)</code>
  </ul>
  where <code>alpha(x)</code> (<code>alpha(next(x))</code>) is a
  formula in which the variable <code>x</code> (<code>next(x)</code>)
  occurs. Notice that <code>next(alpha(x))</code> can be rewritten in
  term of <code>next(x)</code>, since the next operator distributes
  along all kind of operators.<br>

  Here we check also the case where we have next(x), and x is a symbol
  declared as DEFINE whose body contain a next(v).  These kind of
  formulas cannot be checked at parsing time, since, it would require
  to knowledge of part of the model that might be possibly parsed
  later. And removing next from the body of DEFINE is a too
  restrictive choice.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void check_circ(const SymbTable_ptr symb_table, node_ptr n,
                       node_ptr context,
                       boolean is_next, boolean lhs_is_next)
{
  if (n == Nil) return;

  switch (node_get_type(n)) {

  case FAILURE:
  case NUMBER:
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case UWCONST:
  case SWCONST:
    return;

  case BIT:
  case DOT:
  case ATOM: {
    ResolveSymbol_ptr rs;
    node_ptr name;

    rs = SymbTable_resolve_symbol(symb_table, n, context);
    name = ResolveSymbol_get_resolved_name(rs);

    if (ResolveSymbol_is_error(rs)) ResolveSymbol_throw_error(rs);

    if (ResolveSymbol_is_parameter(rs)) {
      node_ptr par = SymbTable_get_actual_parameter(symb_table, name);
      node_ptr ctx = SymbTable_get_actual_parameter_context(symb_table, name);
      check_circ(symb_table, par, ctx, is_next, lhs_is_next);
      return;
    }

    if (ResolveSymbol_is_constant(rs)) return;

    check_circular_assign(symb_table, name, context, is_next, false,
                          lhs_is_next);
    return;
  } /* end of case ATOM */

  case ARRAY:
    {
      ResolveSymbol_ptr rs;
      node_ptr t;

      rs = SymbTable_resolve_symbol(symb_table, n, context);
      t = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_undefined(rs)) {
        /* this is not identifier but array expression */
        check_circ(symb_table, car(n), context, is_next, lhs_is_next);
        check_circ(symb_table, cdr(n), context, is_next, lhs_is_next);
      }
      else {
        /* this is identifier-with-brackets */
        check_circular_assign(symb_table, t, context, is_next, false,
                              lhs_is_next);
      }
      return;
    }

  case CONTEXT:
    check_circ(symb_table, cdr(n), car(n), is_next, lhs_is_next);
    return;

  case NEXT:
    /* handling of hidden next not easy to detect syntactically */
    if (is_next) { error_nested_next(n); }
    if (!lhs_is_next) { error_unexpected_next(n); }

    check_circ(symb_table, car(n), context, true, lhs_is_next);
    return;

  default:
    check_circ(symb_table, car(n), context, is_next, lhs_is_next);
    check_circ(symb_table, cdr(n), context, is_next, lhs_is_next);
  }

}


/**Function********************************************************************

  Synopsis           [Performs circular assignment checking]

  Description        [Checks for circular assignments in the model. If
  there are any, then an error is generated. NEXT operator, if any,
  must be stripped away from given expression 'n', and in that case is_next
  must be set to true. Parameter is_lhs is true at the first call (done
  with the first left-hand-side value (the assigned value)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void check_circular_assign(const SymbTable_ptr symb_table, node_ptr n,
                                  node_ptr context, boolean is_next,
                                  boolean is_lhs, boolean lhs_is_next)
{
  node_ptr t;
  node_ptr next_n;
  boolean is_rhs_next;

  if ((n != Nil) && (is_next) && (node_get_type(n) == NUMBER)) return;

  next_n = find_node(NEXT, n, Nil);
  if (is_next) {
    t = lookup_assign_hash(next_n);
  }
  else {
    /* check if it is a next assignment or a normal assignment */
    t = lookup_assign_hash(n);
    if (t == Nil) {
      /* check if it is an init assignment */
      t = lookup_assign_hash(find_node(SMALLINIT, n, Nil));;
    }
  }

  if (t == CLOSED_NODE) return;
  if (t == FAILURE_NODE) { error_circular_assign(n); }

  if (t == Nil) {
    /* it might be a define: */
    if (SymbTable_is_symbol_define(symb_table, n)) {
      /* switch to define ctx and body, and continue: */
      context = SymbTable_get_define_context(symb_table, n);
      t = SymbTable_get_define_body(symb_table, n);
      is_rhs_next = false; /* this actually is a don't care */
    }
    else return;
  }
  else {
    is_rhs_next = (node_get_type(t) == NEXT);
    if (!is_lhs && is_next && is_rhs_next) { error_nested_next(n); }

    is_lhs = true; /* we found an assignment: restart the search */
  }

  if (t == Nil) {
    if (SymbTable_is_symbol_constant(symb_table, n)) return;
    else error_undefined(n);
  }
  /* single elements of array may depend on other elements of the same array.
     Probably, if array is in dependency then every element has to be
     added to dependency as well.
   */

  insert_assign_hash(is_next ? next_n : n, FAILURE_NODE);
  io_atom_push(is_next? next_n : n);

  /* if this is the first time this function is called, rhs decides if
     there is NEXT operator, otherwise keeps the current mode */
  check_circ(symb_table, t, context, is_lhs? is_rhs_next : is_next,
             lhs_is_next);

  io_atom_pop();
  insert_assign_hash(is_next ? next_n : n, CLOSED_NODE);
}

/**Function********************************************************************

  Synopsis           [Checks for multiple or circular assignments.]

  Description        [This function detects either multiple or
  circular assignments in "context" regarding to "mode".
  If mode is equal to 0 (zero) then it checks for multiple assignments
  or symbols redefinition. Otherwise it performs checks for circular
  assignments.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void check_assign(const SymbTable_ptr symb_table, node_ptr n,
                         node_ptr context, int mode)
{
  if (n == Nil) return;
  yylineno = node_get_lineno(n);

  switch (node_get_type(n)) {
  case AND:
    check_assign(symb_table, car(n), context, mode);
    check_assign(symb_table, cdr(n), context, mode);
    break;

  case CONTEXT:
    check_assign(symb_table, cdr(n), car(n), mode);
    break;

  case EQDEF:
    {
      node_ptr t1, t2;
      ResolveSymbol_ptr rs;

      if ((node_get_type(car(n)) == SMALLINIT) ||
          (node_get_type(car(n)) == NEXT)) {
        rs = SymbTable_resolve_symbol(symb_table, car(car(n)), context);

        t1 = ResolveSymbol_get_resolved_name(rs);
        t2 = find_node(node_get_type(car(n)), t1, Nil);
      }
      else {
        rs = SymbTable_resolve_symbol(symb_table, car(n), context);

        t1 = t2 = ResolveSymbol_get_resolved_name(rs);
      }

      if (mode == 0) {
        /* Checking for multiple assignments */
        if (! SymbTable_is_symbol_declared(symb_table, t1)) { error_undefined(t1); }
        if (SymbTable_is_symbol_input_var(symb_table, t1)) {
          error_assign_input_var(car(n));
        }
        if (SymbTable_is_symbol_frozen_var(symb_table, t1) &&
            SMALLINIT != node_get_type(car(n))) {
          error_assign_frozen_var(car(n));
        }
        if (! SymbTable_is_symbol_state_frozen_var(symb_table, t1)) {
          /* How it can be not state or frozen variable ?*/
          error_redefining(t1);
        }

        if (lookup_assign_hash(t2)) { error_multiple_assignment(t2); }
        insert_assign_hash(t2, find_node(CONTEXT, context, cdr(n)));
        insert_global_assign_hash(t2, NODE_FROM_INT(yylineno));
      }
      else { /* Checking for circular assignments */
        if (node_get_type(t2) == NEXT) {
          check_circular_assign(symb_table, car(t2), context, true, true, true);
        }
        else {
          check_circular_assign(symb_table, t2, context, false, true, false);
        }
      }
      break;
    }

  default:
    internal_error("check_assign: type = %d", node_get_type(n));
  }
}

/**Function********************************************************************

  Synopsis           [Given a variable, it checks if there are
  multiple assignments to it.]

  Description        [Checks if there exists in the model an
  assignments of type <tt>node_type</tt> for variable <tt>v</tt>. If
  such an assignment exists, then an error is generated.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void check_assign_both(node_ptr v, int node_type, int lineno)
{
  node_ptr v1 = find_node(node_type, v, Nil);
  int lineno2 = NODE_TO_INT(lookup_global_assign_hash(v1));

  if (lineno2) error_assign_both(v, v1, lineno, lineno2);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void compile_check_print_io_atom_stack_assign(FILE * fd){
  while(!io_atom_isempty()){
    node_ptr s = io_atom_head();

    io_atom_pop();
    fprintf(fd, "in definition of ");
    print_node(fd, s);
    {
      int lineno = NODE_TO_INT(lookup_global_assign_hash(s));

      if (lineno) fprintf(fd," at line %d", lineno);
      fprintf(fd, "\n");
    }
  }
}

static void error_circular_assign(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "recursively defined: ");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr,"\n");
  compile_check_print_io_atom_stack_assign(nusmv_stderr);
  clear_assign_hash();
  finish_parsing_err();
}

static void error_nested_next(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "nested NEXT operators: ");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr,"\n");
  compile_check_print_io_atom_stack_assign(nusmv_stderr);
  clear_assign_hash();
  finish_parsing_err();
}


static void error_unexpected_next(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "found unexpected next operator: ");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr,"\n");
  compile_check_print_io_atom_stack_assign(nusmv_stderr);
  clear_assign_hash();
  finish_parsing_err();
}


static void check_case(node_ptr expr)
{
  if (Nil == expr) return;

  switch (node_get_type(expr)) {

  case ATOM:
  case DOT:
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case UWCONST:
  case SWCONST:
    return;

  case FAILURE: internal_error("%s:%d:%s %s", __FILE__, __LINE__, __func__,
                               "unexpected FAILURE node");

  case IFTHENELSE:
  case CASE:
      if (node_get_type(cdr(expr)) == FAILURE) {
        /* checks that the last condition is 1 */
        nusmv_assert((node_get_type(car(expr)) == COLON));
        if (!(((node_get_type(car(car(expr))) == NUMBER) &&
               (node_get_int(car(car(expr))) == 1))
              ||
              (node_get_type(car(car(expr))) == TRUEEXP)))
          warning_case_not_exhaustive(cdr(expr));
      }
      check_case(cdr(car(expr)));

  default :
    if (Nil != cdr(expr)) check_case(cdr(expr));
    if (Nil != car(expr)) check_case(car(expr));
  }
}

static boolean check_next(const SymbTable_ptr symb_table, node_ptr n,
                          node_ptr context, boolean is_next_allowed,
                          boolean is_next)
{
  boolean result = false;

  if (Nil == n) return result;

  if (node_is_leaf(n)) return result;

  switch (node_get_type(n)) {

    /* Array is treated as a normal expression: we are just checking
       for next operators!! */
  case DOT:
  case BIT:
  case ATOM:
    {
      ResolveSymbol_ptr rs;
      node_ptr res;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, n, context);
      name = ResolveSymbol_get_resolved_name(rs);

      res = lookup_check_next_hash(name);

      if (res != Nil) {
        int has_next = NODE_TO_INT(res);
        /* If the expression contains next and we are already in a
           next operator, or next are not allowed */
        if (CHECK_NEXT_HAS_NEXT == has_next) {
          if (is_next) { 
            clear_check_next_hash();
            rpterr("Nested next operator.\n"); 
          }
          if (!is_next_allowed) { 
            clear_check_next_hash();
            rpterr("Unexpected next operator.\n"); 
          }

          /* return TRUE, expression contains a next. */
          return true;
        }
        if (CHECK_NEXT_EVALUATING == has_next) {
          clear_check_next_hash();
          yylineno = node_get_lineno(n);
          error_circular(n);
        }
        return false;
      }

      if (ResolveSymbol_is_ambiguous(rs)) { 
        clear_check_next_hash();
        error_ambiguous(name); 
      }

      if (ResolveSymbol_is_constant(rs)) { return false; }

      /* while evaluating the node, mark it to find circular
         dependencies */
      insert_check_next_hash(name, NODE_FROM_INT(CHECK_NEXT_EVALUATING)); 

      if (ResolveSymbol_is_defined(rs)) {
        if (ResolveSymbol_is_define(rs)) {
          node_ptr ctx, body;
          body = SymbTable_get_define_body(symb_table, name);
          ctx = SymbTable_get_define_context(symb_table, name);
          result = check_next(symb_table, body, ctx,
                              is_next_allowed, is_next);
        }
        else if (ResolveSymbol_is_parameter(rs)) {
          node_ptr ctx, body;
          body = SymbTable_get_actual_parameter(symb_table, name);
          ctx = SymbTable_get_actual_parameter_context(symb_table, name);
          result = check_next(symb_table, body, ctx,
                              is_next_allowed, is_next);
        }
        else if (ResolveSymbol_is_array_def(rs)) {
          node_ptr ctx, body;
          body = SymbTable_get_array_define_body(symb_table, name);
          ctx = SymbTable_get_array_define_context(symb_table, name);
          result = check_next(symb_table, body, ctx,
                              is_next_allowed, is_next);
        }
        /* If the assertion fails, it may be that a new kind of symbol
           has been added to the symbol table. add the proper case in
           the condition below: */
        else if (ResolveSymbol_is_var(rs) ||
                 ResolveSymbol_is_function(rs) ||
                 ResolveSymbol_is_array(rs) ||
                 ResolveSymbol_is_constant(rs)) {
          /* variables/constant/function cannot have "next" */
        }
        else {
          fprintf(nusmv_stderr,
                  "Compile_check_next: Unsupported symbol found.\n");
          error_unreachable_code();
        }
      }
      else {
        /* The identifier is not declared. We do not rise an assertion
           because an identifier introduced by PSL forall operator is
           not declared anywhere. We assume that such an identifier is
           met.

           If in future PSL forall identifier is declared then
           here "error_unreachable_code();" should be added.
        */
      }

      /* stores the result in the cache */
      insert_check_next_hash(name, NODE_FROM_INT(result ?       \
                               CHECK_NEXT_HAS_NEXT : CHECK_NEXT_HAS_NO_NEXT));
    }
    break;

  case CONTEXT:
    result = check_next(symb_table, cdr(n), car(n),
                        is_next_allowed, is_next);
    break;

  case NEXT:
    /* handling of hidden next not easy to detect syntactically */
    if (is_next) { 
      clear_check_next_hash();
      rpterr("Nested next operator.\n"); 
    }
    if (!is_next_allowed) { 
      clear_check_next_hash();
      rpterr("Unexpected next operator.\n"); 
    }

    check_next(symb_table, car(n), context, is_next_allowed, true);
    result = true;
    break;

  default:
    result = check_next(symb_table, car(n), context,
                        is_next_allowed, is_next);
    result |= check_next(symb_table, cdr(n), context,
                         is_next_allowed, is_next);
    break;
  } /* switch(node_get_type(n)) */

  return result;
}

