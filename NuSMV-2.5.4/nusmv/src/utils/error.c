/**CFile***********************************************************************

FileName    [error.c]

PackageName [utils]

Synopsis    [Error routines]

Description [This file conatins the error routines. This file is
partitioned in two parts. The first contains general routines, the
second contains specific error routines.]

SeeAlso     []

Author      [Marco Roveri]

Copyright   [
This file is part of the ``utils'' package of NuSMV version 2.
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

#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>

#include "utils/error.h"
#include "utils/utils.h"
#include "utils/ustring.h"
#include "node/node.h"
#include "opt/opt.h"
#include "mc/mc.h"
#include "cinit/cinit.h"
#include "utils/utils_io.h" /* for indent_node */
#include "compile/compile.h"
#include "parser/symbols.h"
#include "parser/psl/pslNode.h"
#include "bmc/bmcUtils.h" /* for Bmc_Utils_ConvertLoopFromInteger */
static char rcsid[] UTIL_UNUSED = "$Id: error.c,v 1.8.2.14.4.31.4.22 2010-02-16 10:53:49 nusmv Exp $";

extern FILE * nusmv_stderr;
extern FILE * nusmv_stdout;
extern cmp_struct_ptr cmps;

#ifndef NUSMV_PACKAGE_BUGREPORT
#define NUSMV_PACKAGE_BUGREPORT "nusmv-users@fbk.eu"
#endif


/*
  PART 1: generic error routines.
*/

/**Variable********************************************************************

   Synopsis    [The name of the currently evaluated expression.]

   Description [This variable is used to store the currently evalauted
   expression to perform it's printing.]

   SeeAlso     []

******************************************************************************/
static node_ptr the_node;
void init_the_node() { the_node = Nil;}
void set_the_node(node_ptr n) { the_node = n;}
node_ptr get_the_node() { return(the_node);}

/**Variable********************************************************************

   Synopsis    [Stack used to print out multiple or circular defined symbols.]

   Description [Stack used to print out where multiple or circular
   defined symbols occurs in the input file.]

   SeeAlso     []

******************************************************************************/
static node_ptr io_atom_stack = Nil;
int io_atom_isempty(void) {return(io_atom_stack == Nil);}
void io_atom_push(node_ptr s) { io_atom_stack = cons(s,io_atom_stack);}
void io_atom_pop() {
  node_ptr temp;

  if (io_atom_stack == Nil) internal_error("io_atom_pop: stack empty");
  temp = cdr(io_atom_stack);
  free_node(io_atom_stack);
  io_atom_stack = temp;
}
node_ptr io_atom_head() {
  if (io_atom_stack == Nil) internal_error("io_atom_pop: stack empty");
  return(car(io_atom_stack));
}
void print_io_atom_stack(FILE * fd){
  while(!io_atom_isempty()){
    node_ptr s = car(io_atom_stack);

    io_atom_stack = cdr(io_atom_stack);
    fprintf(fd, "in definition of ");
    print_node(fd, s);
    if (node_get_lineno(s)) fprintf(fd," at line %d", node_get_lineno(s));
    fprintf(fd, "\n");
  }
}

/**Variable********************************************************************

   Synopsis    [Stack context for non-local goto]

   Description [This variable is used to save the stack environment for
   later use.]

   SeeAlso     [util_newlongjmp util_longjmp]

******************************************************************************/
#define MAXJMP 20
static JMPBUF jmp_buf_arr[MAXJMP];
static int jmp_buf_pos = 0;



/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis           [Save stack context for non-local goto]

   Description        [Saves the stack environment in the global
   array <code>jmp_buf_arr</code> for later use by <code>util_longjmp</code>.]

   SideEffects        []

   SeeAlso            [util_longjmp util_cancellongjmp]

******************************************************************************/
JMPBUF * util_newlongjmp(void)
{
  nusmv_assert(jmp_buf_pos < MAXJMP);
  /* fprintf(nusmv_stdout, "PUSH  ==> POS = %d\n", jmp_buf_pos+1); */
  return (&(jmp_buf_arr[jmp_buf_pos++]));
}

/**Function********************************************************************

   Synopsis           [Restore the environment saved in <code>jmp_buf</code>.]

   Description        [Restores the environment saved by the last call of
   <code>SETJMP(*(util_newlongjmp()), 1)</code>. After
   <code>util_longjmp()</code> is completed, program execution
   continues as if the corresponding call of <code>SETJMP()</code>
   had just returned a value different from <code>0</code> (zero).]

   SideEffects        []

   SeeAlso            [util_newlongjmp util_cancellongjmp]

******************************************************************************/
void util_longjmp(void) {
  if (jmp_buf_pos > 0) {
    /* fprintf(nusmv_stdout, "JMP   ==> POS = %d\n", jmp_buf_pos); */
    LONGJMP(jmp_buf_arr[--jmp_buf_pos], 2);
  }
}

/**Function********************************************************************

   Synopsis           [Pop one of the environments saved in <code>jmp_buf</code>.]

   Description        [Removes the last envirnoment saved in
   <code>jmp_buf</code> by a <code>SETJMP(*(util_newlongjmp()), 1)</code> call.]

   SideEffects        []

   SeeAlso            [util_newlongjmp util_longjmp]

******************************************************************************/
void util_cancellongjmp(void) {
  assert(jmp_buf_pos > 0);
  /* fprintf(nusmv_stdout, "POP   ==> POS = %d\n", jmp_buf_pos); */
  jmp_buf_pos--;
}

/**Function********************************************************************

   Synopsis           [Reset environment saved in <code>jmp_buf</code>.]

   Description        [Resets the environment saved by the calls to
   <code>SETJMP(*(util_newlongjmp()), 1)<code>. After
   this call, all the longjump points previously stored are
   cancelled.]

   SideEffects        []

   SeeAlso            [util_newlongjmp]

******************************************************************************/
void util_resetlongjmp(void) {
  jmp_buf_pos = 0;
  /* fprintf(nusmv_stdout, "RESET ==> POS = %d\n", jmp_buf_pos); */
}

/**Function********************************************************************

   Synopsis           [General routine to start error reporting.]

   Description        [This is a general routine to be called by error
   reporting routines as first call. The file name and corresponding
   line number of the token that has generated the error (which is
   retrieved by <code>get_the_node()</code> are printed out.]

   SideEffects        []

   SeeAlso            [finish_parsing_err]

******************************************************************************/
void start_parsing_err()
{
  extern int yylineno;

  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr,"\n");
  if (get_input_file(OptsHandler_get_instance()))
    fprintf(nusmv_stderr, "file %s: ", get_input_file(OptsHandler_get_instance()));
  else
    fprintf(nusmv_stderr, "file stdin: ");
  if (yylineno) fprintf(nusmv_stderr, "line %d: ", yylineno);
}


/**Function********************************************************************

   Synopsis           [General exit routine.]

   Description        [If non local goto are anebaled, instead of
   exiting from the program, then the non local goto is executed.]

   SideEffects        []

   SeeAlso            [util_setjmp util_longjmp]

******************************************************************************/
void nusmv_exit(int n)
{
  util_longjmp();
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0) )
    fprintf(nusmv_stderr, "%s: exit(%d)\n", get_pgm_name(OptsHandler_get_instance()), n);
  fflush(NULL); /* to flush all existing messages */
  exit(n);
}

/**Function********************************************************************

   Synopsis           [General error reporting routine.]

   Description        [Produces a message on the
   <code>nusmv_stderr</code>. The arguments are similar to those of the
   <code>printf</code>, but only if fmt is not NULL or the empty string]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void rpterr(const char* fmt, ...)
{
  if ((fmt != (char*) NULL) && strlen(fmt) > 0) {
    va_list args;

    start_parsing_err();
    va_start(args, fmt);
    (void) vfprintf(nusmv_stderr, fmt, args);
    va_end(args);
  }

  finish_parsing_err();
}

/**Function********************************************************************

   Synopsis           [General error reporting routine.]

   Description        [Produces a message on the
   <code>nusmv_stderr</code>. The arguments are similar to those of the
   <code>printf</code>, except argument "node" which is output at the
   end of the message with function print_node. ]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void rpterr_node(node_ptr node, const char* fmt, ...)
{
  if ((fmt != (char*) NULL) && strlen(fmt) > 0) {
    va_list args;

    start_parsing_err();
    va_start(args, fmt);
    (void) vfprintf(nusmv_stderr, fmt, args);
    va_end(args);
    print_node(nusmv_stderr, node);
  }

  finish_parsing_err();
}

/**Function********************************************************************

   Synopsis           [Prints out an internal error.]

   Description        [Produces a message on the <code>nusmv_stderr</code>.
   The message is considered an internal error. The arguments are
   similar to those of the <code>printf</code>.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void internal_error(const char * fmt, ...)
{
  va_list args;

  fflush(NULL); /* to flush all existing messages before outputting */
  va_start(args, fmt);
  fprintf(nusmv_stderr, "\n\n*** internal error *** \n");
  (void) vfprintf(nusmv_stderr, fmt, args);
  va_end(args);
  fprintf(nusmv_stderr, "\nPlease report this error to <%s>\n",
          NUSMV_PACKAGE_BUGREPORT);
  fprintf(nusmv_stderr, "Send a copy of this output and your input.\n");
  nusmv_exit(1);
}

/**Function********************************************************************

   Synopsis           [Initializes the memory routines.]

   Description        [This function deals with the memory routines
   taken form the CUDD. It initializes the pointer to function
   <tt>MMoutOfMemory</tt> which is used by the memory allocation
   functions when the <tt>USE_MM</tt> macro is not defined (the
   default). This pointer specifies the function to call when the
   allocation routines fails to allocate memory.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void init_memory()
{
#ifndef USE_MM
  MMoutOfMemory = (void (*)())error_out_of_memory;
#endif
}

/*
  PART 2: Specific error routines.
*/
void error_multiple_substitution(node_ptr nodep)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Multiple substitution for ");
  print_node(nusmv_stderr, nodep);
  finish_parsing_err();
}

/* n must be a FAILURE node */
void report_failure_node(node_ptr n)
{
  extern int yylineno;
  nusmv_assert(Nil != n && node_get_type(n) == FAILURE);

  yylineno = failure_get_lineno(n); /* set the line number of the error */

  start_parsing_err();
  fprintf(nusmv_stderr, "%s\n", failure_get_msg(n));
  finish_parsing_err();
}

void warning_failure_node(node_ptr n)
{
  nusmv_assert(Nil != n && node_get_type(n) == FAILURE);

  fprintf(nusmv_stderr, "WARNING: line: %d: %s\n",
          failure_get_lineno(n),
          failure_get_msg(n));
}


void warning_case_not_exhaustive(node_ptr failure)
{
  fprintf(nusmv_stderr,
          "Warning: at line %d CASE expression might contain not exhaustive conditions.\n"\
          "Use the BDD-based engine to check all CASEs as current version cannot check\n"\
          "exhaustivity of boolean CASEs by using Bounded Model Checking.\n\n",
          failure_get_lineno(failure));
}

void warning_possible_div_by_zero(node_ptr failure)
{
  fprintf(nusmv_stderr,
          "Warning: at line %d expression might contain a division by zero.\n\n",

          failure_get_lineno(failure));
}

void warning_possible_array_out_of_bounds(node_ptr failure)
{
  fprintf(nusmv_stderr,
          "Warning: at line %d expression might result in array subscripting out of bounds.\n\n",

          failure_get_lineno(failure));
}

void error_array_out_of_bounds(int index, int low, int high)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "array index has value %d which is outside of "
                 "allowed range [%d, %d]\n",
          index, low, high);
  finish_parsing_err();
}

void error_lhs_of_index_is_not_array()
{
  start_parsing_err();
  fprintf(nusmv_stderr,
          "left hand side of index sub-scripting is not an array");
  finish_parsing_err();
}

void error_div_by_zero(node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Division by zero: ");
  print_node(nusmv_stderr, expr);
  finish_parsing_err();
}

void error_div_by_nonconst(node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Division by non constant expression: ");
  print_node(nusmv_stderr, expr);
  finish_parsing_err();
}


void type_error(node_ptr n)
{
  start_parsing_err();
  indent_node(nusmv_stderr, "type error: value = ", n, "");
  finish_parsing_err();
}

void range_error(node_ptr n, node_ptr var)
{
  start_parsing_err();
  indent_node(nusmv_stderr, "cannot assign value ", n, " to variable ");
  print_node(nusmv_stderr, var);
  finish_parsing_err();
}

void range_warning(node_ptr n, node_ptr var)
{
  fprintf(nusmv_stderr, "Warning: ");
  indent_node(nusmv_stderr, "cannot assign value ", n, " to variable ");
  print_node(nusmv_stderr, var);
  fprintf(nusmv_stderr, "\n");
}

void error_multiple_assignment(node_ptr t1)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "multiply assigned: ");
  print_node(nusmv_stderr, t1);
  finish_parsing_err();
}

void error_empty_range(node_ptr name, int dim1, int dim2)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "empty range type %d..%d for ", dim1, dim2);
  print_node(nusmv_stderr, name);
  finish_parsing_err();
}

void error_not_word_wsizeof(node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr,
          "sizeof operator applied to a non-word operand: ");
  print_node(nusmv_stderr, expr);
  fprintf(nusmv_stderr, "\n");
  error_reset_and_exit(1);
}

void error_not_constant_extend_width(const node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr,
          "extend expression with non constant size specifier: ");
  print_node(nusmv_stderr, expr);
  fprintf(nusmv_stderr, "\n");
  error_reset_and_exit(1);
}

void error_not_constant_resize_width(const node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr,
          "resize expression with non constant size specifier: ");
  print_node(nusmv_stderr, expr);
  fprintf(nusmv_stderr, "\n");
  error_reset_and_exit(1);
}

void error_not_constant_wtoint(const node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr,
          "toint expression with non constant argument: ");
  print_node(nusmv_stderr, expr);
  fprintf(nusmv_stderr, "\n");
  error_reset_and_exit(1);
}

void error_not_constant_width_of_word_type(node_ptr name)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "the width of a word variable ");
  print_node(nusmv_stderr, name);
  fprintf(nusmv_stderr, " is not a constant");
  finish_parsing_err();
}

void error_not_constant_width_of_word_array_type(node_ptr name)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "at least one of the width of the Word Array variable ");
  print_node(nusmv_stderr, name);
  fprintf(nusmv_stderr, " is not a constant");
  finish_parsing_err();
}

void error_not_constant_width_of_array_type(node_ptr name)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "at least one of the width of the Array variable ");
  print_node(nusmv_stderr, name);
  fprintf(nusmv_stderr, " is not a constant");
  finish_parsing_err();
}

void error_wrong_word_operand(const char* msg, node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Error: %s\n", msg);
  print_node(nusmv_stderr, expr);
  finish_parsing_err();
}

void error_assign_both(node_ptr v, node_ptr v1, int lineno, int lineno2)
{
  extern int yylineno;

  yylineno = lineno;
  start_parsing_err();
  fprintf(nusmv_stderr, "assigned ");
  print_node(nusmv_stderr,v);
  fprintf(nusmv_stderr,", line %d: assigned ", lineno2);
  print_node(nusmv_stderr, v1);
  finish_parsing_err();
}

void error_unknown_var_in_order_file(node_ptr n)
{
  start_parsing_err();
  indent_node(nusmv_stderr, "unknown variable in order file :", n, "");
  finish_parsing_err();
}

void warning_variable_not_declared(node_ptr vname) {
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\nThe variable: ");
  (void) print_node(nusmv_stderr, vname);
  fprintf(nusmv_stderr, "\nhas not been declared in the source file,\n");
  fprintf(nusmv_stderr, "but it appear in the input ordering file.\n");
  fprintf(nusmv_stderr, "Ignoring it.\n");
  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}

void warning_missing_variable(node_ptr vname)
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\nThe variable: ");
  (void) print_node(nusmv_stderr, vname);
  fprintf(nusmv_stderr, "\nhas not been specified in the ordering file.\n");
  fprintf(nusmv_stderr, "It has been positioned at the end of the ordering.\n");
  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}

void warning_missing_variables(NodeList_ptr vars_list)
{
  fflush(NULL); /* to flush all existing messages before outputting */
  if (NodeList_get_length(vars_list) > 1) {
    fprintf(nusmv_stderr, "\n********   WARNING   ********\nThe variables: ");
  }
  else {
    fprintf(nusmv_stderr, "\n********   WARNING   ********\nThe variable: ");
  }

  NodeList_print_nodes(vars_list, nusmv_stderr);

  if (NodeList_get_length(vars_list) > 1) {
    fprintf(nusmv_stderr, "\nhave not been specified in the ordering file.\n");
    fprintf(nusmv_stderr, "They have been positioned at the end of the ordering.\n");
  }
  else {
    fprintf(nusmv_stderr, "\nhas not been specified in the ordering file.\n");
    fprintf(nusmv_stderr, "It has been positioned at the end of the ordering.\n");
  }

  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}

void warning_non_ag_only_spec(Prop_ptr prop)
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\nThe ");
  print_spec(nusmv_stderr, prop);
  fprintf(nusmv_stderr, "\nis not an AG-only formula: Skipped\n");
  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}


void warning_ag_only_without_reachables()
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
  fprintf(nusmv_stderr,
          "AG-only requires the calculation of reachable states to be effective.\n" \
          "Since reeachable states have not been computed, the standard algorithm\n" \
          "will be used instead. \n");
  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}


void warning_fsm_init_empty()
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
  fprintf(nusmv_stderr,
          "The initial states set of the finite state machine is empty.\n"
          "This might make results of model checking not trustable.\n");
  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}


void warning_fsm_invar_empty()
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
  fprintf(nusmv_stderr,
          "The states set of the finite state machine is empty.\n"
          "This might make results of model checking not trustable.\n");
  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}


void warning_fsm_fairness_empty()
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
  fprintf(nusmv_stderr,
          "Fair states set of the finite state machine is empty.\n"
          "This might make results of model checking not trustable.\n");
  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}


void warning_fsm_init_and_fairness_empty()
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
  fprintf(nusmv_stderr,
          "The intersection of initial set of states and the set of fair states");
  fprintf(nusmv_stderr, " of the finite state machine is empty.\n"
          "This might make results of model checking not trustable.\n");
  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}



void error_var_appear_twice_in_order_file(node_ptr n)
{
  start_parsing_err();
  indent_node(nusmv_stderr, "variable appears twice in order file:", n, "");
  finish_parsing_err();
}

void warning_var_appear_twice_in_order_file(node_ptr n)
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
  (void) indent_node(nusmv_stderr, "variable appears twice in order file:", n, "\n");
}

void warning_id_appears_twice_in_idlist_file(node_ptr n)
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
  (void) indent_node(nusmv_stderr, "ID appears twice in id list file :", n, "\n");
}

void error_var_not_in_order_file(node_ptr n)
{
  start_parsing_err();
  indent_node(nusmv_stderr, "not in order file: ", n, "");
  finish_parsing_err();
}

void error_not_proper_number(const char* op, node_ptr n)
{
  start_parsing_err();

  fprintf(nusmv_stderr, "not suitable constant for operator '%s' : ", op);
  print_node(nusmv_stderr, n);

  finish_parsing_err();
}

void error_not_proper_numbers(const char* op, node_ptr n1, node_ptr n2)
{
  start_parsing_err();

  fprintf(nusmv_stderr, "not suitable constants for operator '%s' : ", op);
  print_node(nusmv_stderr, n1);
  fprintf(nusmv_stderr, " and ");
  print_node(nusmv_stderr, n2);

  finish_parsing_err();
}

void error_ambiguous(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "\"");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr, "\" ambiguous");
  finish_parsing_err();
}

void error_undefined(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "\"");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr, "\" undefined");
  finish_parsing_err();
}

void error_shadowing(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "multiple declaration of identifier "
          "(previously declared as parameter): ");
  print_node(nusmv_stderr, s);
  finish_parsing_err();
}

void error_redefining(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "multiple declaration of identifier: ");
  print_node(nusmv_stderr, s);
  finish_parsing_err();
}

void error_redefining_operational_symbol(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "operational symbol \"");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr, "\" is redefined by a user");
  finish_parsing_err();
}

void error_redefining_input_var(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "redefining or assigning input variable \"");
  print_node(nusmv_stderr, s);
  fprintf(nusmv_stderr, "\"");
  finish_parsing_err();
}

void error_reassigning(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "variable is assigned more than once: ");
  print_node(nusmv_stderr, s);
  finish_parsing_err();
}

void error_assign_input_var(node_ptr s)
{
  node_ptr tmp = car(s);
  start_parsing_err();
  if (node_get_type(s) == SMALLINIT) {
    fprintf(nusmv_stderr, "illegal assignment of initial value to input variable \"");
  }
  else if (node_get_type(s) == NEXT) {
    fprintf(nusmv_stderr, "illegal assignment of next value to input variable \"");
  }
  else {
    fprintf(nusmv_stderr, "illegal assignment of value to input variable \"");
    tmp = s;
  }
  print_node(nusmv_stderr, tmp);
  fprintf(nusmv_stderr, "\"");
  finish_parsing_err();
}

void error_assign_frozen_var(node_ptr s)
{
  start_parsing_err();
  nusmv_assert(node_get_type(s) != SMALLINIT); /* init assign is legal=>cannot be here*/
  fprintf(nusmv_stderr, "illegal assignment to frozen variable \"");
  print_node(nusmv_stderr, NEXT == node_get_type(s) ? car(s) : s);
  fprintf(nusmv_stderr, "\". Assignment of an initial value only is allowed.");
  finish_parsing_err();
}

void error_assign_expected_var(node_ptr s)
{
  start_parsing_err();
  nusmv_assert(EQDEF == node_get_type(s));
  fprintf(nusmv_stderr, "A variable is expected in left-hand-side of assignment:\n");
  print_node(nusmv_stderr, s);
  finish_parsing_err();
}

void error_circular(node_ptr s)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "recursively defined: ");
  print_node(nusmv_stderr, s);
  finish_parsing_err();
}

void error_too_many_vars()
{
  start_parsing_err();
  fprintf(nusmv_stderr, "too many variables");
  finish_parsing_err();
}

void error_out_of_memory(size_t size)
{
  fflush(NULL); /* to flush all existing messages before outputting */
  (void) fprintf(nusmv_stderr,
                 "\n##################################################\n");
  (void) fprintf(nusmv_stderr,
                 "### Out of memory allocating %" PRIuPTR " bytes\n", size);
  (void) fprintf(nusmv_stderr,
                 "##################################################\n");
  nusmv_exit(1);
}

void error_invalid_subrange(node_ptr range)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Invalid subrange: ");
  print_node(nusmv_stderr, range);
  finish_parsing_err();
}

void error_invalid_bool_cast(node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr,
          "Invalid boolean cast (Expected word1, boolean"
          " or integer expression): ");
  print_node(nusmv_stderr, expr);
  finish_parsing_err();
}

void error_out_of_bounds_word_toint_cast(node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr,
          "Invalid word to int cast. (Max word width is 32 for signed words, "
          "31 for unsigned words): ");
  print_node(nusmv_stderr, expr);
  finish_parsing_err();
}

void error_invalid_toint_cast(node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr,
          "Invalid integer cast (Expected word, boolean"
          " or integer expression): ");
  print_node(nusmv_stderr, expr);
  finish_parsing_err();
}

void error_invalid_count_operator(node_ptr expr)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Expected boolean expressions in count operator: ");
  print_node(nusmv_stderr, expr);
  finish_parsing_err();
}

void error_invalid_enum_value(node_ptr value)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Invalid enumerative value: ");
  print_node(nusmv_stderr, value);
  finish_parsing_err();
}

void error_game_definition_contains_input_vars(node_ptr var_name)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "The game difinition contains an input variable: ");
  print_node(nusmv_stderr, var_name);
  finish_parsing_err();
}

void error_property_contains_input_vars(Prop_ptr prop)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Property contains input variables:\n");
  Prop_print_db(prop, nusmv_stderr, PROPDB_PRINT_FMT_DEFAULT);
  finish_parsing_err();
}

void error_assign_exp_contains_input_vars(node_ptr exp)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Assignment of ");
  print_node(nusmv_stderr, exp);
  fprintf(nusmv_stderr, " contains input variables, which is not allowed.");
  finish_parsing_err();
}

void error_next_exp_contains_input_vars(node_ptr exp)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "\"next\" expression contains input variables:\n");
  print_node(nusmv_stderr, exp);
  finish_parsing_err();
}
void error_invar_exp_contains_input_vars(node_ptr exp)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "INVAR expression contains input variables:\n");
  print_node(nusmv_stderr, exp);
  finish_parsing_err();
}
void error_init_exp_contains_input_vars(node_ptr exp)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "INIT expression contains input variables:\n");
  print_node(nusmv_stderr, exp);
  finish_parsing_err();
}
void error_second_player_var(node_ptr exp)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Prohibited use of a variable of "
          "the second player in the expressions of the first player : ");
  print_node(nusmv_stderr, exp);
  finish_parsing_err();
}
void error_second_player_next_var(node_ptr exp)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Prohibited use of a (next-state) variable of "
          "the second player in the expressions of the first player : ");
  print_node(nusmv_stderr, exp);
  finish_parsing_err();
}

void error_unknown_preprocessor(const char* prep_name)
{
  fprintf(nusmv_stderr, "Unknown preprocessor: %s\n", prep_name);
  nusmv_exit(1);
}
void error_type_system_violation()
{
  extern int yylineno;
  yylineno = 0; /* to suppress line-number output */
  start_parsing_err();
  fprintf(nusmv_stderr, "Type System Violation detected\n");
  finish_parsing_err();
}

void error_psl_not_supported_feature()
{
  start_parsing_err();
  fprintf(nusmv_stderr, "The specified PSL property requires features " \
          "that are not implemented yet.\n");
  finish_parsing_err();
}
void error_psl_not_supported_feature_next_number()
{
  fprintf(nusmv_stderr, "expr in next*[expr] must be a constant positive "\
          "integer number.\n");
  error_psl_not_supported_feature();
}
void error_not_supported_feature(const char* msg)
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Error: unsupported feature.\n");
  fprintf(nusmv_stderr, "The requested feature has not been implemented yet:\n ");
  fprintf(nusmv_stderr, "%s", msg);
  fprintf(nusmv_stderr, "\n");
  finish_parsing_err();
}

void error_expected_number()
{
  start_parsing_err();
  fprintf(nusmv_stderr, "A number was expected but not found.\n");
  finish_parsing_err();
}


void warning_psl_not_supported_feature(node_ptr psl_spec, int index)
{
  fflush(NULL); /* to flush all existing messages before outputting */
  fprintf(nusmv_stderr, "\n********   WARNING   ********\n");
  fprintf(nusmv_stderr, "The following PSL property:\n");
  print_node(nusmv_stderr, psl_spec);
  fprintf(nusmv_stderr, "\nrequires features that are not yet available.\n");
  fprintf(nusmv_stderr,
          "The property will be inserted in the properties database at index %d,\n",
          index);
  fprintf(nusmv_stderr,
          "but the user will be not allowed to perform any model checking on it.\n");
  fprintf(nusmv_stderr,
          "Furthermore, no type nor definitions checking will be performed\n" \
          "on this property.\n");
  fprintf(nusmv_stderr, "******** END WARNING ********\n");
}

void error_psl_repeated_replicator_id()
{
  start_parsing_err();
  fprintf(nusmv_stderr, "Nested forall contains an ID that is "\
          "already used by an outer forall.\n");
  finish_parsing_err();
}



/**Function********************************************************************

   Synopsis           [General routine to terminate error reporting.]

   Description        [This is the general routine to be called as last
   routine in specific error reporting routines.
   If error happens during flattening, the system is also reset.
   Finally, a call to <code>nusmv_exit()</code> is performed.]

   SideEffects        []

   SeeAlso            [start_parsing_err]

******************************************************************************/
void finish_parsing_err()
{
  fprintf(nusmv_stderr, "\n");
  print_io_atom_stack(nusmv_stderr);

  error_reset_and_exit(1);
}


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void error_reset_and_exit(int val)
{
  /* Resets the system if flattening hasn't been successfully performed */
  if ((cmp_struct_get_flatten_hrc(cmps) == 0)
      && (cmp_struct_get_read_model(cmps) == 1)) {
    NuSMVCore_reset();
  }

  nusmv_exit(val);
}


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void error_invalid_number(const char* szNumber)
{
  if (szNumber != (const char*) NULL) {
    fprintf(nusmv_stderr,
            "Error: string \"%s\" is not valid. An integer was expected.\n",
            szNumber);
  }
  else {
    fprintf(nusmv_stderr,
            "Error: you have given an empty string when integer was "\
            "expected.\n");
  }
}


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void error_bmc_invalid_k_l(const int k, const int l)
{
  char buf[16];

  Bmc_Utils_ConvertLoopFromInteger(l, buf, sizeof(buf));
  fprintf(nusmv_stderr,
          "Error: length=%d and loopback=%s are not compatible.\n",
          k, buf);
}


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void error_property_already_specified()
{
  fprintf(nusmv_stderr,
          "Error: It is not possible to specify multiple properties to be" \
          " checked \n");
  fprintf(nusmv_stderr,
          "      using \"-p <formula>\" and/or \"-n <property_idx>\" " \
          "options\n\n");
}

/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void error_invalid_numeric_value(int value, const char* reason)
{
  start_parsing_err();

  fprintf(nusmv_stderr,
          "Error: invalid numeric value: %d\n", value);
  if (reason != (char*) NULL) {
    fprintf(nusmv_stderr, "%s", reason);
    fprintf(nusmv_stderr, "\n");
  }

  finish_parsing_err();
}


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void error_file_not_found(const char* filename)
{
  fprintf(nusmv_stderr,
          "Error: file '%s' not found.\n", filename);
  nusmv_exit(1);
}


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
void warning_processes_deprecated()
{
  fprintf(nusmv_stderr,
          "WARNING *** Processes are still supported, but deprecated.      ***\n");
  fprintf(nusmv_stderr,
          "WARNING *** In the future processes may be no longer supported. ***\n\n");
}


/**Function********************************************************************

   Synopsis           [Builder for FAILURE nodes]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
node_ptr failure_make(const char* msg, FailureKind kind, int lineno)
{
  return find_node(FAILURE,
                   find_node(COLON,
                             (node_ptr) find_string((char*) msg),
                             (node_ptr) kind),
                   NODE_FROM_INT(lineno));
}


/**Function********************************************************************

   Synopsis           [Returns the message string associated to the
   failure node]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
const char* failure_get_msg(node_ptr failure)
{
  if (failure != (node_ptr) NULL &&
      node_get_type(failure) == FAILURE &&
      car(failure) != (node_ptr) NULL &&
      node_get_type(car(failure)) == COLON)
    return (const char*) get_text((string_ptr) car(car(failure)));
  return "Unknown";
}


/**Function********************************************************************

   Synopsis           [Returns the failure kind associated to the
   failure node]

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
FailureKind failure_get_kind(node_ptr failure)
{
  if (failure != (node_ptr) NULL &&
      node_get_type(failure) == FAILURE &&
      car(failure) != Nil &&
      node_get_type(car(failure)) == COLON)
    return (FailureKind) cdr(car(failure));
  return FAILURE_UNSPECIFIED;
}


/**Function********************************************************************

   Synopsis           []

   Description        []

   SideEffects        []

   SeeAlso            []

******************************************************************************/
int failure_get_lineno(node_ptr failure)
{
  nusmv_assert(failure != (node_ptr) NULL &&
               node_get_type(failure) == FAILURE &&
               cdr(failure) != Nil);
  return NODE_TO_INT(cdr(failure));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

