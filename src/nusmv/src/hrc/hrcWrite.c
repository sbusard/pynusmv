/**CFile***********************************************************************

  FileName    [hrcWrite.c]

  PackageName [hrc]

  Synopsis    [Creation of an SMV file of an Hrc structure]

  Description [Creates a SMV file from the hrc
  structure.

  The exported function Hrc_WriteModel allows to print a HrcNode_ptr
  structure on a file.

  The file contains static functions needed to print an SMV file given
  the hrc structure.]

  SeeAlso     []

  Author      [Sergio Mover]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK.

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


#include "hrc.h"
#include "HrcNode.h"
#include "parser/symbols.h"
#include "utils/Slist.h"
#include "utils/assoc.h"
#include "utils/ustring.h"
#include "compile/compile.h" /* for Compile_print_array_define */


static char rcsid[] UTIL_UNUSED = "$Id: hrcWrite.c,v 1.1.2.5 2009-11-03 11:29:18 nusmv Exp $";

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

/**Macro***********************************************************************

  Synopsis     [Suffix used to rename module names and module variables]

  Description  [Suffix used to rename module names and module variables]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define HRC_WRITE_MODULE_SUFFIX "_hrc"

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static boolean hrc_write_expr_split ARGS((FILE* out,
                                          node_ptr n,
                                          const char* s));

static boolean hrc_write_spec_split ARGS((FILE* out,
                                          node_ptr n,
                                          const char* s));

static void hrc_write_expr ARGS((FILE* out, node_ptr n, const char* s));

static void hrc_write_spec ARGS((FILE* out, node_ptr spec, const char* msg));

static boolean hrc_write_assign_list ARGS((FILE* out,
                                           int assign_node_type,
                                           node_ptr assign_list));

static void hrc_write_print_assign ARGS((FILE * out,
                                         node_ptr lhs,
                                         node_ptr rhs));

static void hrc_write_module_instance ARGS((FILE* ofile,
                                            HrcNode_ptr hrcNode,
                                            st_table* printed_module_map,
                                            boolean append_suffix));

static void hrc_write_parameters ARGS((FILE* ofile,
                                       node_ptr parameters_list));

static void
hrc_write_declare_module_variables ARGS((FILE* ofile,
                                         HrcNode_ptr child,
                                         st_table* printed_module_map,
                                         boolean append_suffix));

static void hrc_write_print_vars ARGS((FILE* out, HrcNode_ptr hrcNode));

static void hrc_write_print_var_list ARGS((FILE* out, node_ptr var_list));

static void hrc_write_print_defines ARGS((FILE* out, HrcNode_ptr hrcNode));

static void hrc_write_print_array_defines ARGS((FILE* out, HrcNode_ptr hrcNode));

static void hrc_write_specifications ARGS((FILE* out, HrcNode_ptr hrcNode));

static void hrc_write_spec_pair_list ARGS((FILE* out,
                                           node_ptr pair_list,
                                           char* section_name));

static boolean hrc_write_constants ARGS((FILE* out, node_ptr constants));

static void print_variable_type ARGS((FILE* out, node_ptr node));

static void print_scalar_type ARGS ((FILE* out, node_ptr node));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Prints the SMV module for the hrcNode.]

  Description        [Prints the SMV module for the hrcNode. If the
  flag append_suffix is true then the suffix HRC_WRITE_MODULE_SUFFIX
  is appended when a module type is printed. So
  HRC_WRITE_MODULE_SUFFIX is appended to the module name in module
  declarations and to the module name in a module instantiation. The
  feature is needed for testing to avoid name clash among modules
  names when the original model and the model generated from hrc are
  merged.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Hrc_WriteModel(HrcNode_ptr hrcNode, FILE * ofile, boolean append_suffix)
{
  HRC_NODE_CHECK_INSTANCE(hrcNode);
  nusmv_assert((FILE *)NULL != ofile);
  st_table* printed_module_map; /* hash table used to keep track of
                                   previously printed modules. */

  printed_module_map = new_assoc();

  /* call the recursive creation of the modules */
  hrc_write_module_instance(ofile,
                            hrcNode,
                            printed_module_map,
                            append_suffix);

  clear_assoc(printed_module_map);
  free_assoc(printed_module_map);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Writes the SMV translation of the instance
  module contained in hrcNode on file.]

  Description        [Writes the SMV translation of the instance
  module contained in hrcNode on file.]

  SideEffects        [printed_module_map is changed to keep track of
  printed modules.]

  SeeAlso            []

******************************************************************************/
static void hrc_write_module_instance(FILE * ofile,
                                      HrcNode_ptr hrcNode,
                                      st_table* printed_module_map,
                                      boolean append_suffix)
{
  Siter iter;
  node_ptr module_name;
  Slist_ptr rev_child_stack;

  module_name = HrcNode_get_name(hrcNode);

  /* Set the module as printed  */
  insert_assoc(printed_module_map, module_name, PTR_FROM_INT(node_ptr, 1));

  /* prints MODEL name(formal_parameters) */
  fprintf(ofile, "MODULE ");
  print_node(ofile, module_name);

  /* main module is never changed with the suffix */
  if ((HRC_NODE(NULL) != HrcNode_get_parent(hrcNode)) &&
      append_suffix) {
    fprintf(ofile, "%s", HRC_WRITE_MODULE_SUFFIX);
  }

  /* print formal parameters of module */
  hrc_write_parameters(ofile,
                       HrcNode_get_formal_parameters(hrcNode));
  fprintf(ofile, "\n\n");


  /* Iterates over all children of this node, creating variables and
     assigning module names.
     Children stack is reversed in order to preserve order.
  */
  rev_child_stack =
    Slist_copy_reversed(HrcNode_get_child_hrc_nodes(hrcNode));


  /* All the children of the current nodes are visited instantiating
     each module variable.
  */
  if (! Slist_is_empty(rev_child_stack)) {
    fprintf(ofile, "VAR\n");
  }
  SLIST_FOREACH(rev_child_stack, iter) {
    HrcNode_ptr child;

    child = HRC_NODE(Siter_element(iter));

    /* Declares the new module variables */
    hrc_write_declare_module_variables(ofile,
                                       child,
                                       printed_module_map,
                                       append_suffix);
  }
  if (! Slist_is_empty(rev_child_stack)) {
    fprintf(ofile, "\n");
  }

  /* Prints state, invar and frozen variables */
  hrc_write_print_vars(ofile, hrcNode);

  /* Prints define */
  hrc_write_print_defines(ofile, hrcNode);

  /* Prints array define */
  hrc_write_print_array_defines(ofile, hrcNode);

  /* CONSTANTS */
  if (hrc_write_constants(ofile,
                          HrcNode_get_constants(hrcNode))) {
    fprintf(ofile, "\n");
  }

  /* ASSIGN: invar, init, next  */
  if (hrc_write_assign_list(ofile,
                            -1,
                            HrcNode_get_invar_assign_exprs(hrcNode))) {
    fprintf(ofile, "\n");
  }

  if (hrc_write_assign_list(ofile,
                            SMALLINIT,
                            HrcNode_get_init_assign_exprs(hrcNode))) {
    fprintf(ofile, "\n");
  }

  if (hrc_write_assign_list(ofile,
                            NEXT,
                            HrcNode_get_next_assign_exprs(hrcNode))) {
    fprintf(ofile, "\n");
  }

  /* CONSTRAINS: INIT, INVAR, TRANS */
  if (hrc_write_expr_split(ofile,
                           HrcNode_get_init_exprs(hrcNode),
                           "INIT\n")) {
    fprintf(ofile, "\n");
  }

  if (hrc_write_expr_split(ofile,
                           HrcNode_get_invar_exprs(hrcNode),
                           "INVAR\n")) {
    fprintf(ofile, "\n");
  }

  if (hrc_write_expr_split(ofile,
                           HrcNode_get_trans_exprs(hrcNode),
                           "TRANS\n")) {
    fprintf(ofile, "\n");
  }

  /* JUSTICE/FAIRNESS and COMPASSION */
  hrc_write_expr_split(ofile, HrcNode_get_justice_exprs(hrcNode), "JUSTICE\n");
  hrc_write_spec_pair_list(ofile,
                           HrcNode_get_compassion_exprs(hrcNode),
                           "COMPASSION\n");

  /* Writes specifications (INVARSPEC CTLSPEC LTLSPEC PSLSPEC
     COMPUTE) */
  hrc_write_specifications(ofile, hrcNode);

  /* Recursive creation of child modules.
     Reversed children stack is used to preserve child definition order.
  */
  SLIST_FOREACH(rev_child_stack, iter) {
    HrcNode_ptr child;
    node_ptr assoc_key;
    node_ptr child_module_name;

    child = HRC_NODE(Siter_element(iter));
    child_module_name = HrcNode_get_name(child);

    /* Avoids to print the module multiple times */
    assoc_key = find_assoc(printed_module_map, child_module_name);
    if (Nil == assoc_key) {
      hrc_write_module_instance(ofile,
                                child,
                                printed_module_map,
                                append_suffix);
    }
  } /* end loop on children */

  Slist_destroy(rev_child_stack);
}


/**Function********************************************************************

  Synopsis           [Declare a module variables, setting the module
  to use and the actual parameters.]

  Description        [Declare a module variables, setting the module
  to use and the actual parameters.]

  SideEffects        [printed_module_map is changed in the recursive
  calls of the the function.]

  SeeAlso            []

******************************************************************************/
static void hrc_write_declare_module_variables(FILE * ofile,
                                               HrcNode_ptr child,
                                               st_table* printed_module_map,
                                               boolean append_suffix)
{
  node_ptr instance_name;
  node_ptr child_module_name;

  child_module_name = HrcNode_get_name(child);

  /* Declare module variable */
  instance_name = HrcNode_get_instance_name(child);
  print_node(ofile, instance_name);
  fprintf(ofile, " : ");
  print_node(ofile, child_module_name);

  if (append_suffix) {
    fprintf(ofile, "%s", HRC_WRITE_MODULE_SUFFIX);
  }

  /* Prints actual parameters */
  hrc_write_parameters(ofile,
                       HrcNode_get_actual_parameters(child));

  fprintf(ofile, ";\n");
}

/**Function********************************************************************

  Synopsis           [Prints a list of parameters for module
  declaration or instantiation.]

  Description        [Prints a list of parameters for module
  declaration or instantiation.

  The parameter list is printed enclosed by brackets, every parameter
  is separated by colon.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_parameters(FILE* ofile, node_ptr parameters_list)
{
  boolean first_parameter, has_parameters;

  has_parameters = Nil != parameters_list;

  if (has_parameters) {
    fprintf(ofile, "(");
  }

  first_parameter = true;

  /* Loops all parameters list and print them */
  while (Nil != parameters_list) {
    node_ptr parameter;
    node_ptr parameter_name;
    node_ptr parameter_type;

    parameter = car(parameters_list);
    nusmv_assert(Nil != parameter);
    nusmv_assert(CONS == node_get_type(parameter));

    parameter_name = car(parameter);
    nusmv_assert(Nil != parameter_name);

    if (! first_parameter) {
      fprintf(ofile, ", ");
    }
    print_node(ofile, parameter_name);


    parameter_type = cdr(parameter);

    /* Parameter type can be Nil now */
    if (Nil != parameter_type) {
      fprintf(ofile, ": ");
      print_node(ofile, parameter_type);
    }

    first_parameter = false;
    parameters_list = cdr(parameters_list);
  }

  if (has_parameters) {
    fprintf(ofile, ")");
  }

  return;

}

/**Function********************************************************************

  Synopsis           [Prints the variable of the module contained in hrcNode.]

  Description        [Prints the variable of the module contained in
  hrcNode.
  The sections printed are VAR, IVAR and FROZENVAR.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_print_vars(FILE* out, HrcNode_ptr hrcNode)
{

  if (Nil != HrcNode_get_state_variables(hrcNode)) {
    fprintf(out, "VAR\n");
    hrc_write_print_var_list(out, HrcNode_get_state_variables(hrcNode));
    fprintf(out, "\n");
  }

  if (Nil != HrcNode_get_input_variables(hrcNode)) {
    fprintf(out, "IVAR\n");
    hrc_write_print_var_list(out, HrcNode_get_input_variables(hrcNode));
    fprintf(out, "\n");
  }

  if (Nil != HrcNode_get_frozen_variables(hrcNode)) {
    fprintf(out, "FROZENVAR\n");
    hrc_write_print_var_list(out, HrcNode_get_frozen_variables(hrcNode));
    fprintf(out, "\n");
  }
}


/**Function********************************************************************

  Synopsis           [Prints a list of variables.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_print_var_list(FILE* out, node_ptr var_list)
{
  node_ptr reversed_list, list_iterator;

  /* reverse the the input list */
  reversed_list = reverse_ns(var_list);

  /* Visit the list printing each variable */
  list_iterator = reversed_list;
  while (Nil != list_iterator) {
    node_ptr variable_node;
    node_ptr variable_name;
    node_ptr variable_type;

    variable_node = car(list_iterator);
    assert(Nil != variable_node);

    /* prints the variable name */
    variable_name = car(variable_node);
    nusmv_assert(Nil != variable_name);
    print_node(out, variable_name);
    fprintf(out, " : ");

    /* prints the variable type */
    variable_type = cdr(variable_node);
    print_variable_type(out, variable_type);
    fprintf(out, ";\n");

    /* visit next variable */
    list_iterator = cdr(list_iterator);
  }

  free_list(reversed_list);

  return;
}

/**Function********************************************************************

  Synopsis           [Writes DEFINE declarations in SMV format on a
  file.]

  Description        [Writes DEFINE declarations in SMV format on a
  file.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_print_defines(FILE* out, HrcNode_ptr hrcNode)
{
  node_ptr define_list;
  boolean has_define;

  define_list = HrcNode_get_defines(hrcNode);

  has_define = Nil != define_list;

  if (has_define) {
    fprintf(out, "DEFINE\n");
  }

  /* Visit the list printing each define statement */
  while (Nil != define_list) {
    node_ptr define_node;
    node_ptr define_name;
    node_ptr define_body;

    define_node = car(define_list);
    nusmv_assert(Nil != define_node);

    /* prints the define name */
    define_name = car(define_node);
    nusmv_assert(Nil != define_name);
    print_node(out, define_name);
    fprintf(out, " := ");

    /* prints the body of the define */
    define_body = cdr(define_node);
    nusmv_assert(Nil != define_body);
    print_node(out, define_body);
    fprintf(out, ";\n");

    /* visit next define */
    define_list = cdr(define_list);
  }

  if (has_define) {
    fprintf(out, "\n");
  }

  return;
}
/**Function********************************************************************

  Synopsis           [Writes the ARRAY DEFINE declarations contained in hrcNode.]

  Description        [Writes the ARRAY DEFINE declarations contained in hrcNode.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_print_array_defines(FILE* out, HrcNode_ptr hrcNode)
{
  node_ptr array_define_list;
  boolean has_array_define;

  array_define_list = HrcNode_get_array_defines(hrcNode);

  has_array_define = Nil != array_define_list;

  if (has_array_define) {
    fprintf(out, "MDEFINE\n");
  }

  /* Visit the list printing each array define statement */
  while (Nil != array_define_list) {
    node_ptr array_define_node;
    node_ptr array_define_name;
    node_ptr array_define_body;

    array_define_node = car(array_define_list);
    nusmv_assert(Nil != array_define_node);

    /* prints the array define name */
    array_define_name = car(array_define_node);
    nusmv_assert(Nil != array_define_name);
    print_node(out, array_define_name);
    fprintf(out, " := ");

    /* prints the body of the array define */
    array_define_body = cdr(array_define_node);
    nusmv_assert(Nil != array_define_body);
    Compile_print_array_define(out, array_define_body);
    fprintf(out, ";\n");

    /* visit next array define */
    array_define_list = cdr(array_define_list);
  }

  if (has_array_define) {
    fprintf(out, "\n");
  }

  return;
}

/**Function********************************************************************

  Synopsis           [Writes an expression in SMV format on a file.]

  Description        [Writes a generic expression prefixed by a given
  string in SMV format on a file.

  Returns true if at least one expression was printed.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean hrc_write_expr_split(FILE* out, node_ptr n, const char* s)
{
  if (Nil == n) {
    return false;
  }

  switch (node_get_type(n)) {
  case CONS:
    /* Expressions are inserted in reverse order in the Hrc
       structure, so we need to visit the right branch before.
    */
    hrc_write_expr_split(out, cdr(n), s);

    hrc_write_expr_split(out, car(n), s);

    break;

  default:
    hrc_write_expr(out, n, s);
  } /* switch */

  return true;
}

/**Function********************************************************************

  Synopsis           [Writes a specification list in SMV format on a file.]

  Description        [Writes a specification list prefixed by a given
  string in SMV format on a file.

  Returns true if at least one specification was printed.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean hrc_write_spec_split(FILE* out, node_ptr n, const char* s)
{
  if (Nil == n) {
    return false;
  }

  switch (node_get_type(n)) {
  case CONS:
  case AND:

    /* Specifications are inserted in reverse order in the Hrc
       structure, so we need to visit the right branch before.
    */
    hrc_write_spec_split(out, cdr(n), s);

    hrc_write_spec_split(out, car(n), s);

    break;

  default:
    hrc_write_spec(out, n, s);
  } /* switch */

  return true;
}

/**Function********************************************************************

  Synopsis           [Writes expression in SMV format on a file.]

  Description        [Writes a generic expression prefixed by a given
  string in SMV format on a file.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_expr(FILE* out, node_ptr n, const char* s)
{
  fprintf(out, "%s ", s);
  print_node(out, n);
  fprintf(out, "\n");

  return;
}

/**Function********************************************************************

  Synopsis           [Prints the given specification.]

  Description        [Prints in out file the specification.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_spec(FILE* out, node_ptr spec, const char* msg)
{
  nusmv_assert((SPEC == node_get_type(spec)) ||
               (LTLSPEC == node_get_type(spec)) ||
               (INVARSPEC == node_get_type(spec)) ||
               (PSLSPEC == node_get_type(spec)) ||
               (COMPUTE == node_get_type(spec)));

  node_ptr expr = car(spec);
  node_ptr name = cdr(spec);

  fprintf(out, "%s ", msg);

  /* Support for property Names: Old property structure is in car(n),
     property name is in cdr(n).  */
  if (Nil != name){
    fprintf(out, "NAME ");
    print_node(out, name);
    fprintf(out, " := ");
  }

  print_node(out, expr);
  /* semicolon is needed for PSLSPEC */
  fprintf(out, ";");
  fprintf(out, "\n");

  return;
}

/**Function********************************************************************

  Synopsis           [Writes all the specifications of a module instance.]

  Description        [Writes all the specifications of a module instance.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_specifications(FILE* out,
                                     HrcNode_ptr hrcNode)
{

  if (hrc_write_spec_split(out,
                           HrcNode_get_ctl_properties(hrcNode),
                           "CTLSPEC\n")) {
    fprintf(out, "\n");
  }

  if (hrc_write_spec_split(out,
                           HrcNode_get_ltl_properties(hrcNode),
                           "LTLSPEC\n")) {
    fprintf(out, "\n");
  }

  if (hrc_write_spec_split(out,
                           HrcNode_get_compute_properties(hrcNode),
                           "COMPUTE\n")) {
    fprintf(out, "\n");
  }

  if (hrc_write_spec_split(out,
                           HrcNode_get_invar_properties(hrcNode),
                           "INVARSPEC\n")) {
    fprintf(out, "\n");
  }

  if (hrc_write_spec_split(out,
                           HrcNode_get_psl_properties(hrcNode),
                           "PSLSPEC\n")) {
    fprintf(out, "\n");
  }
}

/**Function********************************************************************

  Synopsis           [Writes ASSIGN declarations in SMV format on a file.]

  Description        [Writes ASSIGN declarations in SMV format on a
  file.

  Function returns true if at least an assign was written]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean hrc_write_assign_list(FILE* out,
                                     int assign_node_type,
                                     node_ptr assign_list)
{
  boolean has_assign;

  node_ptr reversed_list;
  node_ptr list_iterator;

  reversed_list = reverse_ns(assign_list);

  has_assign = false;


  list_iterator = reversed_list;
  while (Nil != list_iterator) {
    node_ptr assign_expression;
    node_ptr assign_lhs_name;
    node_ptr assign_lhs_node;
    node_ptr assign_rhs_node;

    if (! has_assign) {
      has_assign = true;
      fprintf(out, "ASSIGN\n");
    }

    assign_expression = car(list_iterator);
    nusmv_assert(Nil != assign_expression);

    /* The node to be printed for an assign can be:
       next(assign_lhs_name) for next assign
       init(assign_lhs_name) for init assign
       assign_lhs_name for invar assign

       This is decided by the caller of the function.
    */
    assign_lhs_name = car(assign_expression);
    nusmv_assert(Nil != assign_lhs_name);

    if (assign_node_type < 0) {
      assign_lhs_node = assign_lhs_name;
    } else {
      assign_lhs_node = find_node(assign_node_type, assign_lhs_name, Nil);
    }

    /* Get assign right expression */
    assign_rhs_node = cdr(assign_expression);
    nusmv_assert(Nil != assign_rhs_node);

    hrc_write_print_assign(out, assign_lhs_node, assign_rhs_node);

    list_iterator = cdr(list_iterator);
  }

  free_list(reversed_list);

  return has_assign;
}


/**Function********************************************************************

  Synopsis           [Prints an assignement statement]

  Description        [Prints an assignement statement]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_print_assign(FILE * out, node_ptr lhs, node_ptr rhs)
{
  print_node(out, lhs);
  fprintf(out, " := ");
  print_node(out, rhs);
  fprintf(out, ";\n");
}

/**Function********************************************************************

  Synopsis           [Writes a list of specification that contains
  pairs in SMV format.]

  Description        [Writes a list of specification that contains
  pairs in SMV format.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_write_spec_pair_list(FILE* out,
                                     node_ptr pair_list,
                                     char* section_name)
{
  boolean has_specifications;
  node_ptr reversed_list, list_iterator;

  has_specifications = Nil != pair_list;

  reversed_list = reverse_ns(pair_list);

  /* Visit the list printing each statement */
  list_iterator = reversed_list;
  while (Nil != list_iterator) {
    node_ptr pair;

    pair = car(list_iterator);

    nusmv_assert(Nil != pair);
    nusmv_assert(CONS == node_get_type(pair));

    fprintf(out, "%s(", section_name);
    print_node(out, car(pair));
    fprintf(out, ", ");
    print_node(out, cdr(pair));
    fprintf(out, ")\n");

    /* visit next pair */
    list_iterator = cdr(list_iterator);
  }

  if (has_specifications) {
    fprintf(out, "\n");
  }

  free_list(reversed_list);

  return ;
}

/**Function********************************************************************

  Synopsis           [Prints in the output file the SMV
  representations of constants list.]

  Description        [Prints in the output file the SMV
  representations of constants list.

  Function returns true if at least a constant was printed.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean hrc_write_constants(FILE *out, node_ptr constants_list)
{
  boolean first;

  first = true;

  while (Nil != constants_list) {
    node_ptr constant;

    nusmv_assert(CONS == node_get_type(constants_list));

    constant = car(constants_list);

    if (first) {
      fprintf(out, "CONSTANTS\n");
    } else {
      fprintf(out, ", ");
    }

    print_node(out, constant);

    first = false;
    constants_list = cdr(constants_list);
  }

  if (! first) {
    fprintf(out, ";\n");
  }

  return (! first);
}


/**Function********************************************************************

  Synopsis           [Prints the type of a variable.]

  Description        [Prints the type of a variable. The printers used
  in compileWrite.c in compile package cannot be used in hrc, unless
  symbol table is used.

  The printer manages the following types: BOOLEAN, INTEGER, REAL,
  UNSIGNED_WORD, SIGNED_WORD, SCALAR, WORD_ARRAY and  ARRAY_TYPE.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void print_variable_type(FILE* out, node_ptr node)
{
  nusmv_assert(Nil != node);

  int node_type;

  node_type = node_get_type(node);

  switch(node_type) {
  case BOOLEAN:
  case TWODOTS: /* range */
    print_node(out, node);
    break;

  case INTEGER:
    fprintf(out, "integer");
    break;

  case REAL:
    fprintf(out, "real");
    break;

  case SIGNED_WORD:
    fprintf(out, "signed word[");
    print_node(out, car(node));
    fprintf(out, "]");
    break;

  case UNSIGNED_WORD:
    fprintf(out, "unsigned word[");
    print_node(out, car(node));
    fprintf(out, "]");
    break;

  case SCALAR:
    print_scalar_type(out, node);
    break;

  case WORDARRAY:
    fprintf(out, "array ");

    fprintf(out, "word[");
    print_node(out, car(node));
    fprintf(out, "]");

    fprintf(out, " of ");

    fprintf(out, "word[");
    print_node(out, cdr(node));
    fprintf(out, "]");
    break;

  case ARRAY_TYPE:

    fprintf(out, "array ");

    /* Prints subrange of array n..m */
    print_node(out, car(node));

    fprintf(out, " of ");

    /* recursively prints the array type */
    print_variable_type(out, cdr(node));

    break;
  default:
    fprintf(stderr, "Type %d not supported by hrc emitter.\n", node_type);
  }

  return;
}

/**Function********************************************************************

  Synopsis           [Prints the scalar type of a variable.]

  Description        [Prints the scalar type of a variable. The
  printer takes care of reversing the CONS list that contains the
  enumeration to preserve the order of the literals in the source
  model.]

  SideEffects        []

  SeeAlso            [print_variable_type]

******************************************************************************/
static void print_scalar_type(FILE* out, node_ptr node)
{
  nusmv_assert(Nil != node);

  int node_type;
  node_ptr reversed_literals;
  node_ptr iterator;
  boolean first_literal;

  node_type = node_get_type(node);
  nusmv_assert(SCALAR == node_type);

  fprintf(out, "{");

  /* reverse the literals of the enumerations to preserve their
     original order */
  reversed_literals = reverse_ns(car(node));

  iterator = reversed_literals;
  first_literal = true;
  while (Nil != iterator) {
    node_ptr literal;

    literal = car(iterator);
    nusmv_assert(Nil != literal);

    if (! first_literal) {
      fprintf(out, ", ");
    }
    print_node(out, literal);


    first_literal = false;
    iterator = cdr(iterator);
  }

  fprintf(out, "}");

  free_list(reversed_literals);

  return;
}
