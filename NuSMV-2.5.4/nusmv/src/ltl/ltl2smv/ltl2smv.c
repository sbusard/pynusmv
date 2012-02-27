/**CFile***********************************************************************

  FileName    [ltl2smv.c]

  PackageName [ltl2smv]

  Synopsis    [Functions performing conversion of LTL formula to
  SMV module]

  Description [see the header of file ltl2smv.h]

  SeeAlso     []

  Author      [Marco Roveri, Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``ltl2smv'' package of NuSMV version 2.
  Copyright (C) 1998-2005 by CMU and FBK-irst.

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

#include "ltl2smv.h"
#include "utils/utils.h"
#include "parser/symbols.h"
#include "utils/assoc.h"
#include "utils/ustring.h"
#include "node/node.h"
#include "fsm/sexp/Expr.h"
#include "utils/error.h"

#include <stdarg.h>

static char rcsid[] UTIL_UNUSED = "$Id: ltl2smv.c,v 1.1.4.10.4.16 2009-12-14 18:15:29 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* MACRO declaration                                                         */
/*---------------------------------------------------------------------------*/
/* Set or unset the debugging mode (value 1 and 0 respectively).*/
#define OUTPUT_DEBUGGING 0

/*
   The NO_DEFINE_FOR_OR_and_NOT_OPT define controls whether in the
   tableau construction it is created or not a DEFINE for each OR and
   NOT occurring in the rewritten formula. Originally, a DEFINE was
   created for each OR and NOT occurring in the rewritten
   formula. Now, by default this creation is disabled. The reason
   being that on large LTL properties having disabled such creation
   results in a drammatic reduction of the size of the generated
   internal representation of the MODULE corresponding to the tableau,
   and as a consequence also the size of the generated file, and in
   its subsequent handling by the flattener. To re-enable the old
   behavior comment the following define.
*/
#define NO_DEFINE_FOR_OR_and_NOT_OPT

#define PRE_PREFIX "LTL_"
#define PREFIXNAME "_SPECF_"
/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

EXTERN FILE* nusmv_stdout;

/* the body of the module being created during transformation.
 See transform_ltl_expression for more info
*/
static node_ptr define_declarations;
static node_ptr trans_declarations;
static node_ptr var_declarations;
static node_ptr justice_declarations;
static node_ptr init_declarations;

/* node to string association.
 The string is the string representation of the node (for leaf nodes)
 or a unique name (for all other nodes participating in the transformation)
*/
static hash_ptr expr_to_name_hash = NULL;

/* used to generate unique positive numbers (for variable name)*/
static unsigned int uniquePositiveNumber = 0;

/* a positive number identifying particular conversion.
  Provided as a parameter to the function ltl2smv
*/
static unsigned int specificationNumber = 0;


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void initialise_transformation ARGS((unsigned int specificationNumber_));
static void deinitialise_transformation ARGS((void));

static node_ptr normalise_formula ARGS((node_ptr t));
static node_ptr perform_memory_sharing ARGS((node_ptr t));
static node_ptr transform_ltl_expression ARGS((node_ptr t, boolean polarity));
static node_ptr generate_smv_module ARGS((node_ptr t));

static node_ptr expr_to_name ARGS((node_ptr node));
static node_ptr add_to_list ARGS((node_ptr node, node_ptr list));
static char* generate_string ARGS((const char *format, ...));
static node_ptr expand_case_body ARGS((node_ptr));
#if OUTPUT_DEBUGGING
static void ltl2smv_print_module ARGS((FILE* ostream, node_ptr module));
#endif

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The main routine converting an LTL formula to am SMV Module.]

  Description [The parameters are:
  uniqueId - is a positive integer number, which is converted to string
            representation and then used as a part of the generated
            SMV models name.  (_LTL_uniqueId_SPECF_N_)
  in_ltl_expr - the input LTL Formula in the form of node_ptr. The
            expression should be flattened, rewritten wrt input
            variables, negated, i.e. be ready for conversion without
            any additional transformations.


  The function also performs memory-sharing on the input expression,
  since the returned module may be smaller if the memory of
  expression is shared. So DO NOT modify the output expressions.

  But the invoker can modify the declarations and module itself (but
  not their expressions). See generate_smv_module for more info.

  The return value is the created SMV module in the form of node_ptr.
  ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
node_ptr ltl2smv(unsigned int uniqueId, node_ptr in_ltl_expr)
{
  nusmv_assert(uniqueId >= 0);

  {
    /* the line number is set to LTL expression's line number.  */
    extern int yylineno;
    yylineno = node_get_lineno(in_ltl_expr);
  }


  /* DEBUGGING */
#if OUTPUT_DEBUGGING
#warning Debugging LTL2SMV facilities are compiled in!
  fprintf(nusmv_stdout, "-- original LTL expression : ");
  print_node(nusmv_stdout, in_ltl_expr);
  fprintf(nusmv_stdout, "\n");
#endif

  in_ltl_expr = normalise_formula(in_ltl_expr);

  /* DEBUGGING */
#if OUTPUT_DEBUGGING
#warning Debugging LTL2SMV facilities are compiled in!
  fprintf(nusmv_stdout, "-- normalised LTL expression : ");
  print_node(nusmv_stdout, in_ltl_expr);
  fprintf(nusmv_stdout, "\n");
#endif

  initialise_transformation(uniqueId);

  in_ltl_expr = transform_ltl_expression(in_ltl_expr, false);
  in_ltl_expr = generate_smv_module(in_ltl_expr);

  deinitialise_transformation();

#if OUTPUT_DEBUGGING
#warning Debugging LTL2SMV facilities are compiled in!
  ltl2smv_print_module(stdout, in_ltl_expr);
#endif

  return in_ltl_expr;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Initialises the conversion performed by this package]

  Description [The specificationNumber is the same operand provided to ltl2smv]

  SideEffects [initialises all the data structures required for conversion]

  SeeAlso     [ltl2smv]

******************************************************************************/
static void initialise_transformation(unsigned int specificationNumber_)
{
  specificationNumber = specificationNumber_;

  nusmv_assert(NULL == expr_to_name_hash);
  expr_to_name_hash = new_assoc();

  uniquePositiveNumber = 0; /* zero the counter for unique name generation */

  define_declarations = Nil;
  trans_declarations = Nil;
  init_declarations = Nil;
  justice_declarations = Nil;
  var_declarations = Nil;
}


/**Function********************************************************************

  Synopsis    [deinitialises the package]

  Description [The only required thing is to free the hash table: exp -> name]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void deinitialise_transformation()
{
  nusmv_assert(NULL != expr_to_name_hash);
  free_assoc(expr_to_name_hash);
  expr_to_name_hash = (hash_ptr)NULL;

  /* the lists of declarations are not freed, because they are returned
   as a part of the module
  */
}


/**Function********************************************************************

  Synopsis           [Normalises the formula]

  Description        [Normalisations will create a new formula with
  the following transformations:
   "a & b"   => "!(!a & !b)"
   "a -> b"  => "!a | b"
   "a <-> b"  => "!(!a | !b) | !(a | b)"
   "a xnor b" => "!(!a | !b) | !(a | b)"
   "a xor b"  => "!(!a | b) | !(!a | b)"
   "Z a" => "! Y !a"
   "F a" => "1 U a"
   "G a" => "!(1 U !a)"
   "a V b" => "!(!a U !b)"
   "O a" => "1 S a"
   "H a" => "!(1 S !a)"
   "a T b" => "!(!a S !b)"

   If two consecutive NOT are met => both removed.

   The expression is also memory-shared, i.e. find_atom
   of find_node is invoked on every(!) expressions, including
   on the leaf-nodes (i.e. nodes not participating in the conversion directly).
   ]

  SideEffects []

  SeeAlso            []

******************************************************************************/
static node_ptr normalise_formula(node_ptr t)
{
  node_ptr left = Nil, right = Nil;
  node_ptr or, or1, or2, tmp;

  if (Nil == t) return Nil;

  /* proceed with every kind of expression separately*/
  switch (node_get_type(t)) {

  case NOT: /* ! */
    left = normalise_formula(car(t));
    return Expr_not(left);
  case OP_NEXT: /* X */
  case OP_PREC: /* Y */
    left = normalise_formula(car(t));
    return find_node(node_get_type(t), left, Nil);

  case OR: /* | */
    left = normalise_formula(car(t));
    right = normalise_formula(cdr(t));
    return Expr_or(left, right);

  case UNTIL: /* U */
  case SINCE: /* S */
    left = normalise_formula(car(t));
    right = normalise_formula(cdr(t));
    return find_node(node_get_type(t), left, right);

  case AND: /* & */
    left = normalise_formula(car(t));
    right = normalise_formula(cdr(t));
    or  = Expr_or(Expr_not(left), Expr_not(right));
    return Expr_not(or);

  case IMPLIES: /* ->  */
    left = normalise_formula(car(t));
    right = normalise_formula(cdr(t));
    return Expr_or(Expr_not(left), right);

  case IFF: /* <-> */
  case XNOR: /* xnor */
    /* a <-> b  =>  (!(!a | !b) | !(a | b)) */
    left = normalise_formula(car(t));
    right = normalise_formula(cdr(t));
    or1 = Expr_or(Expr_not(left), Expr_not(right));
    or2 = Expr_or(left, right);
    return Expr_or(Expr_not(or1), Expr_not(or2));

  case XOR: /* xor */
    /* a xor b   =>   (!(!a | b) | !(a | !b)) */
    left = normalise_formula(car(t));
    right = normalise_formula(cdr(t));
    or1 = Expr_or(Expr_not(left), right);
    or2 = Expr_or(left, Expr_not(right));
    return Expr_or(Expr_not(or1), Expr_not(or2));

  case OP_NOTPRECNOT: /* Z  */
    left = normalise_formula(car(t));
    tmp = find_node(OP_PREC, Expr_not(left), Nil);
    return Expr_not(tmp);

  case OP_FUTURE: /* F */
    left = normalise_formula(car(t));
    return find_node(UNTIL, Expr_true(), left);

  case OP_GLOBAL: /* G  */
    left = normalise_formula(car(t));
    tmp = find_node(UNTIL, Expr_true(), Expr_not(left));
    return Expr_not(tmp);

  case OP_ONCE: /* O */
    left = normalise_formula(car(t));
    return  find_node(SINCE, Expr_true(), left);

  case OP_HISTORICAL: /* H */
    left = normalise_formula(car(t));
    tmp = find_node(SINCE, Expr_true(), Expr_not(left));
    return Expr_not(tmp);

  case TRIGGERED: /* T */
  case RELEASES: /* V */
    error_unreachable_code(); /* T and V were transformed by the parser */

    /* Leafs, i.e. all the usual kinds of expressions.
       They all considered as an atomic expressions and not
       participate in LTL -> Module transformation.
       Just perform memory-sharing.
    */
    /* In previous version of this function the leafs were expected to
       be already shared, but now they are obligitory shared here
       (the sharing makes the return module potentially smaller).
    */
  case CASE: /* Case are expanded as the logical expansion of
                chains of ITE */
  case IFTHENELSE:
  case FAILURE: case FALSEEXP:  case TRUEEXP:
  case NUMBER:  case NUMBER_UNSIGNED_WORD:  case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC:  case NUMBER_REAL:
  case NUMBER_EXP:
  case TWODOTS:  case ATOM: case SELF:  case DOT:  case ARRAY:
  case BIT_SELECTION:  case CONCATENATION: case EXTEND:
  case WSIZEOF: case WRESIZE:  case CAST_TOINT:
  case CAST_BOOL:  case CAST_WORD1:  case CAST_SIGNED:  case CAST_UNSIGNED:
  case TIMES: case DIVIDE: case PLUS :case MINUS: case MOD: case UMINUS:
  case LSHIFT: case RSHIFT: case LROTATE: case RROTATE:
  case UNION: case SETIN:
  case EQUAL: case NOTEQUAL: case LT: case GT: case LE: case GE:
    return perform_memory_sharing(t);

  default:
    error_unreachable_code(); /*  unknown operator */
    return Nil;
  }
}


/**Function********************************************************************

  Synopsis           [Make all sub-expressions of the expression
  to share as much memory as possible, i.e. the same sub-expressions
  will have the same pointer.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr perform_memory_sharing(node_ptr t)
{
  if (t == (node_ptr) NULL) return (node_ptr) NULL;

  switch (node_get_type(t)) {
    /* 0-arity */
  case FAILURE: case FALSEEXP: case TRUEEXP:
  case NUMBER: case NUMBER_UNSIGNED_WORD: case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC: case NUMBER_REAL:
  case NUMBER_EXP:
  case ATOM: case SELF:
    return find_atom(t);
    /* not, unary minus */
  case NOT:
  case UMINUS:
    if (node_get_type(t) == node_get_type(car(t))) {
      /* remove duplicated NOT and UMINUS */
      return perform_memory_sharing(car(car(t)));
    }
    /* else behave as a usual unary operator */
    /* 1-arity */
  case CAST_BOOL:  case CAST_WORD1: case CAST_SIGNED: case CAST_UNSIGNED:
    return Expr_resolve(SYMB_TABLE(NULL), node_get_type(t),
                        perform_memory_sharing(car(t)), cdr(t));
    /* 1 or 2 arity */
  case DOT:
    if (Nil == car(t)) {
      return find_node(node_get_type(t),
                       Nil,
                       perform_memory_sharing(cdr(t)));
    }
    else { /* skip to 2-arity expressions */ }
    /* 2-arity */
  case OR: case AND: case XOR: case XNOR: case IFF: case IMPLIES:
  case TWODOTS: case ARRAY:
  case BIT_SELECTION:  case CONCATENATION: case EXTEND:
  case WSIZEOF: case WRESIZE: case CAST_TOINT:
  case CASE: /* CASE is no longer a part of LTL formula - just a usual exp */
  case IFTHENELSE:
  case COLON:
  case TIMES: case DIVIDE: case PLUS :case MINUS: case MOD:
  case LSHIFT: case RSHIFT: case LROTATE: case RROTATE:
  case UNION: case SETIN:
  case EQUAL: case NOTEQUAL: case LT: case GT: case LE: case GE:
    return Expr_resolve(SYMB_TABLE(NULL), node_get_type(t),
                        perform_memory_sharing(car(t)),
                        perform_memory_sharing(cdr(t)));
  }

  error_unreachable_code(); /* unknown kind of the expression */
  return Nil;
}

/**Function********************************************************************

  Synopsis           [The main conversion function.]

  Description [It fills in the lists : var_declarations,
  define_declarations, trans_declarations, init_declarations,
  justice_declarations. These lists are list of nodes.
  var_declarations: a list of ATOM (future VAR declarations)
  define_declarations: a list of DEFINES body, i.e. EQDEF (see syntax
  of DEFINE) trans_declarations: a list of TRANS expressions (but not
  TRANS itself) init_declarations: a list of INIT expressions (but not
  INIT itself) justice_declarations: a list of JUSTICE expressions(but
  not JUSTICE itself)

  NB: The elements of the lists use memory-sharing so
  pointers can be compare to check their uniqueness.
  (But the lists themselves do not use memory-sharing)

  Returns the name of the input expression (see expr_to_name
  for more details).

  NB: The memory of expression is expected to be shared. Then the
  same name will be used for the same sub-expression.
  ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr transform_ltl_expression(node_ptr t, boolean polarity)
{
  node_ptr name;
  node_ptr nameLeft;
  node_ptr nameRight;
  node_ptr result;
  node_ptr nameXY; /* an additional X or Y expression */

  switch(node_get_type(t)) {
  case OR:
#ifndef NO_DEFINE_FOR_OR_and_NOT_OPT
    name = expr_to_name(t);
    nameLeft = transform_ltl_expression(car(t), polarity);
    nameRight = transform_ltl_expression(cdr(t), polarity);
    /* generate a new DEFINE */
    result = find_node(EQDEF, name, Expr_or(nameLeft, nameRight));
    define_declarations = add_to_list(result, define_declarations);
    return name;
#else
    nameLeft = transform_ltl_expression(car(t), polarity);
    nameRight = transform_ltl_expression(cdr(t), polarity);
    return Expr_or(nameLeft, nameRight);
#endif

  case NOT:
#ifndef NO_DEFINE_FOR_OR_and_NOT_OPT
    name = expr_to_name(t);
    nameLeft = transform_ltl_expression(car(t), !polarity);
    /* generate a new DEFINE */
    result = find_node(EQDEF, name, Expr_not(nameLeft));
    define_declarations = add_to_list(result, define_declarations);
    return name;
#else
    nameLeft = transform_ltl_expression(car(t), !polarity);
    return Expr_not(nameLeft);
#endif

  case OP_NEXT:  /* X */
    name = expr_to_name(t);
    /* add to VAR */
    var_declarations = add_to_list(name, var_declarations);

    nameLeft = transform_ltl_expression(car(t), polarity);

    /* generate a new TRANS */
    result = Expr_equal(Expr_next(nameLeft, SYMB_TABLE(NULL)),
                        name,
                        SYMB_TABLE(NULL));
    trans_declarations = add_to_list(result, trans_declarations);
    return name;

  case OP_PREC: /* Y */
    name = expr_to_name(t);
    /* add to VAR */
    var_declarations = add_to_list(name, var_declarations);

    nameLeft = transform_ltl_expression(car(t), polarity);

    /* generate a new TRANS */
    /* Note that the TRANS is similar to X's TRANS, but the NEXT operator
       is applied to a different term */
    result = Expr_equal(nameLeft,
                        Expr_next(name, SYMB_TABLE(NULL)),
                        SYMB_TABLE(NULL));
    trans_declarations = add_to_list(result, trans_declarations);
    /* generate a new INIT */
    result = Expr_equal(name, Expr_false(), SYMB_TABLE(NULL));
    init_declarations = add_to_list(result, init_declarations);
    return name ;

  case UNTIL:
    name = expr_to_name(t);
    /* generate X (expr) and add to VAR */
    nameXY = expr_to_name(find_node(OP_NEXT, t, Nil));
    var_declarations = add_to_list(nameXY, var_declarations);

    nameLeft = transform_ltl_expression(car(t), polarity);
    nameRight = transform_ltl_expression(cdr(t), polarity);


    /* generate a new DEFINE */
    result = find_node(EQDEF, name, Expr_or(nameRight,
                                            Expr_and(nameLeft, nameXY)));
    define_declarations = add_to_list(result, define_declarations);
    /* generate a new TRANS */
    result = Expr_equal(Expr_next(name, SYMB_TABLE(NULL)),
                        nameXY,
                        SYMB_TABLE(NULL));
    trans_declarations = add_to_list(result, trans_declarations);
    /* if the occurrence of the formula is positive, generate a new JUSTICE */
    if (polarity == false) {
      result = Expr_or(Expr_not(name), nameRight);
      justice_declarations = add_to_list(result,
                                         justice_declarations);
    }
    return name;

  case SINCE:
    name = expr_to_name(t);
    /* generate Y (expr) and add to VAR */
    nameXY = expr_to_name(find_node(OP_PREC, t, Nil));
    var_declarations = add_to_list(nameXY, var_declarations);

    nameLeft = transform_ltl_expression(car(t), polarity);
    nameRight = transform_ltl_expression(cdr(t), polarity);

    /* generate a new DEFINE */
    result = find_node(EQDEF, name, Expr_or(nameRight,
                                            Expr_and(nameLeft, nameXY)));
    define_declarations = add_to_list(result, define_declarations);
    /* generate a new TRANS */
    result = Expr_equal(name,
                        Expr_next(nameXY, SYMB_TABLE(NULL)),
                        SYMB_TABLE(NULL));
    trans_declarations = add_to_list(result, trans_declarations);
    /* generate a new INIT */
    result = Expr_equal(nameXY, Expr_false(), SYMB_TABLE(NULL));
    init_declarations = add_to_list(result, init_declarations);
    return name;

    /* Leafs. Skip them */
  case FAILURE: case FALSEEXP:  case TRUEEXP:
  case NUMBER:  case NUMBER_UNSIGNED_WORD: case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC:  case NUMBER_REAL:
  case NUMBER_EXP:
  case TWODOTS:  case ATOM:  case SELF: case DOT:  case ARRAY:
  case BIT_SELECTION:  case CONCATENATION: case EXTEND:
  case WSIZEOF: case WRESIZE: case CAST_TOINT:
  case CASE: /* CASE is no longer a part of LTL formula - just a usual exp */
  case IFTHENELSE:
  case CAST_BOOL:  case CAST_WORD1: case CAST_SIGNED: case CAST_UNSIGNED:
  case TIMES: case DIVIDE: case PLUS :case MINUS: case MOD:
  case LSHIFT: case RSHIFT: case LROTATE: case RROTATE:
  case UNION: case SETIN:
  case EQUAL: case NOTEQUAL: case LT: case GT: case LE: case GE:
    return t;
  default:
    error_unreachable_code(); /* unknown expression. This is for debugging only */
  }
  return Nil;
}


/**Function********************************************************************

  Synopsis           [Returns a name of the expression or
  the expression itself]

  Description        [If the expression is an LTL leaf node (i.e. not
  a node participating in the transformation) then the node itself is
  returned.
  If the node is a node participating in the transformation then
  the ATOM node with a unique name is returned.

  NB: The memory sharing for expression will make the whole algorithm
  more efficient because the same name will be returned
  for the same subexpression]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr expr_to_name(node_ptr node)
{
  node_ptr result = find_assoc(expr_to_name_hash, node);

  if (Nil != result) return result; /* the association already exists */

  /* no association has been created =>  create a new association */
  switch (node_get_type(node)) {
    /* operators participating in the conversion => create a name */
  case NOT: /* ! */
  case OR: /* | */
  case OP_NEXT: /* X */
  case OP_PREC: /* Y */
  case UNTIL: /* U */
  case SINCE: /* S */{
    char* str = generate_string("%s%u%s%d", PRE_PREFIX, specificationNumber,
                                PREFIXNAME, uniquePositiveNumber++);
    result = find_node(ATOM, (node_ptr)find_string(str), Nil);
    break;
  }

    /* Leafs. just return the input node  */
  case FAILURE: case FALSEEXP:  case TRUEEXP:
  case NUMBER:  case NUMBER_UNSIGNED_WORD:  case NUMBER_SIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_FRAC:  case NUMBER_REAL:
  case NUMBER_EXP:
  case TWODOTS:  case ATOM: case SELF: case DOT:  case ARRAY:
  case BIT_SELECTION:  case CONCATENATION: case EXTEND:
  case WSIZEOF: case WRESIZE: case CAST_TOINT:
  case CASE: /* CASE is no longer a part of LTL formula - just a usual exp */
  case IFTHENELSE:
  case CAST_BOOL:  case CAST_WORD1:  case CAST_SIGNED:  case CAST_UNSIGNED:
  case TIMES: case DIVIDE: case PLUS :case MINUS: case MOD:
  case LSHIFT: case RSHIFT: case LROTATE: case RROTATE:
  case UNION: case SETIN:
  case EQUAL: case NOTEQUAL: case LT: case GT: case LE: case GE:
    result = node;
    break;
  default: error_unreachable_code(); /* unknown expression: just for debugging */
  }
  /* remember the node and its name */
  insert_assoc(expr_to_name_hash, node, result);

  return result;
}


/**Function********************************************************************

  Synopsis           [Add a node to a list. If the node is already in the list,
  nothing happens]

  Description        [Returns a new list (probably with the given node.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr add_to_list(node_ptr node, node_ptr list)
{
  node_ptr iter = list;

  while (Nil != iter) {
    /* the node is the list */
    if (node == car(iter)) return list;
    iter = cdr(iter);
  }
  /* the node is NOT in the list => add it */
  return cons(node, list);
}


/**Function********************************************************************

  Synopsis           [Behaves similar to standard sprintf, but will allocates
  the required memory by itself]

  Description        [Warning:
  1. memory belongs to the function. Do not modify the returned
     string. Consecutive invocations damage the previously returned
     strings.

  2. The limit of generated strings is set to 100 bytes. Do not try
  to generate a bigger string]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static char* generate_string(const char* format, ...)
{
  static char buffer[100]; /* 100 bytes should be enough */

  int len;
  va_list ap;

  va_start(ap, format);
  len = vsnprintf(buffer, 100, format, ap); /* 100 - buffer length */
  va_end(ap);

  nusmv_assert(len >= 0); /* an error in the vsnprintf */
  nusmv_assert(len < 100); /* buffer overflow */
  return buffer;

#if 0
  !!! HERE is the implementation of the string with arbitrary length. !!!
  !!! Apparently this implementation is less efficient !!!!

  /* the buffer is not deallocated every. This is correct */
  static char* buffer = NULL;
  static int bufferSize = 0;

  int len;
  char* returnString;

  do {
    /* try to print with the standard vsnprintf */
    va_list ap;
    va_start(ap, format);
    len = vsnprintf(buffer, bufferSize, format, ap);
    va_end(ap);

    nusmv_assert(len >= 0); /* an error in the vsnprintf */

    /* buffer is too small */
    if (len == bufferSize) {
      bufferSize = 100 + bufferSize * 2; /* increase the buffer */
      buffer = REALLOC(char, buffer, bufferSize);
      len = bufferSize;
    }
  } while(len == bufferSize); /* buffer was increased. Try one more time */

  /* allocate the memory which will be returned */
  returnString = ALLOC(char, len + 1);
  strncpy(returnString, buffer, len + 1);
  return returnString;
#endif
}


/**Function********************************************************************

  Synopsis           [After invocation of transform_ltl_expression, this
  function generates SMV modules from the obtained lists
  of DEFINEs, INITs, etc.]

  Description        [The parameter whole_expression_name
  is the name (see expr_to_name) of the whole LTL expression.

  All expressions in the module are memory-shared, but
  the module itself is not. The invoker may modify or delete
  all the declarations (and lists insided VAR and DEFINE), but
  should not modify the expressions (including EQDEF in DEFINEs).]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr generate_smv_module(node_ptr whole_expression_name)
{
  node_ptr all_declr;
  node_ptr iter;
  node_ptr tmp;

  /* add the INIT with the name of the whole expression */
  all_declr = cons(new_node(INIT, whole_expression_name, Nil), Nil);

  /* add the JUSTICEs to the list of all declarations */
  iter = justice_declarations;
  tmp = Nil;
  while (Nil != iter) {
    tmp = cons(new_node(JUSTICE, car(iter), Nil), tmp);
    iter = cdr(iter);
  }
  all_declr = append(all_declr, reverse(tmp));

  /* add the INITs to the list of all declarations */
  iter = init_declarations;
  tmp = Nil;
  while (Nil != iter) {
    tmp = cons(new_node(INIT, car(iter), Nil), tmp);
    iter = cdr(iter);
  }
  all_declr = append(all_declr, reverse(tmp));

  /* add the TRANSs to the list of all declarations */
  iter = trans_declarations;
  tmp = Nil;
  while (Nil != iter) {
    tmp = cons(new_node(TRANS, car(iter), Nil), tmp);
    iter = cdr(iter);
  }
  all_declr = append(all_declr, reverse(tmp));

  /* add the DEFINESs to the list of all declarations */
  {
/*     node_ptr shared_defines = Nil; /\* defines with shared memory *\/ */
/*     iter = define_declarations; */
/*     while (Nil != iter) { */
/*       shared_defines = find_node(CONS, car(iter), shared_defines); */
/*       iter = cdr(iter); */
/*     } */
/*     all_declr = find_node(CONS, */
/*                           find_node(DEFINE, shared_defines, Nil), all_declr); */
    all_declr = cons(new_node(DEFINE, define_declarations, Nil), all_declr);
  }

  /* add the VARs to the list of all declarations */
  {
    node_ptr new_vars = Nil;/* all the variable declaration */
    node_ptr boolean = find_node(BOOLEAN, Nil, Nil);
    iter = var_declarations;
    while (Nil != iter) {
      new_vars = cons(new_node(COLON, car(iter), boolean), new_vars);
      iter = cdr(iter);
    }
    all_declr = cons(new_node(VAR, new_vars, Nil), all_declr);
  }

  /* create the module name */
  {
    char* name = generate_string("%s%u", LTL_MODULE_BASE_NAME,
                                 specificationNumber);
    iter = find_node(ATOM, (node_ptr)find_string(name), Nil);
  }

  return new_node(MODULE, new_node(MODTYPE, iter, Nil), all_declr);
}


/**Function********************************************************************

  Synopsis           [Expands the body of a case]

  Description        [Given a case expression, of the form
  <textarea>
  case
     c1 : e1;
     c2 : e2;
     ...
  esac;
  </textarea>
  it returns <tt> (c1 and e1) or (!c1 and ((c2 and e2) or (!c2 and (....)))) </tt>
]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr expand_case_body(node_ptr expr) {
  nusmv_assert(Nil != expr);

  if ((CASE == node_get_type(expr)) ||
      (IFTHENELSE == node_get_type(expr))) {
    node_ptr c, t, e;

    nusmv_assert((COLON == node_get_type(car(expr))));

    c = car(car(expr));
    t = cdr(car(expr));
    e = expand_case_body(cdr(expr));

    return Expr_or(Expr_and(c, t), Expr_and(Expr_not(c), e));
  }

  return expr;
}



#if OUTPUT_DEBUGGING
#warning Debugging LTL2SMV facilities are compiled in!
/**Function********************************************************************

  Synopsis           [Prints the tableau in SMV format]

  Description        [Prints the tableau in SMV format]

  SideEffects        [None]

******************************************************************************/
static void ltl2smv_print_module(FILE* ostream, node_ptr module)
{
  node_ptr iter;

  nusmv_assert(Nil != module);
  nusmv_assert(MODULE == node_get_type(module));
  /* print the name */
  nusmv_assert(MODTYPE == node_get_type(car(module)));
  nusmv_assert(ATOM == node_get_type(car(car(module))));
  nusmv_assert(Nil == cdr(car(module)));

  fprintf(ostream, "MODULE %s\n", get_text((string_ptr)car(car(car(module)))));

  iter = cdr(module);
  while (Nil != iter) {
    nusmv_assert(CONS == node_get_type(iter));
    switch (node_get_type(car(iter))) {

    case VAR: { /* variable declarations */
      node_ptr var;
      var = car(car(iter));
      if ( Nil != var) {
        fprintf(ostream, "VAR\n");
        while (Nil != var) { /* iterate over variable declarations */

          nusmv_assert(CONS == node_get_type(var));
          nusmv_assert(COLON == node_get_type(car(var)));
          nusmv_assert(ATOM == node_get_type(car(car(var))));
          nusmv_assert(BOOLEAN == node_get_type(cdr(car(var))));

          fprintf(ostream, "   %s : boolean;\n",
                  get_text((string_ptr)car(car(car(var)))));

          var = cdr(var);
        }
      }
      break;
    }

    case DEFINE: { /* define declarations */
      node_ptr def;
      def = car(car(iter));
      if ( Nil != def) {
        fprintf(ostream, "DEFINE\n");
        while (Nil != def) { /* iterate over define declarations */

          nusmv_assert(CONS == node_get_type(def));
          nusmv_assert(EQDEF == node_get_type(car(def)));

          fprintf(ostream, "   ");
          print_node(ostream, car(def));
          fprintf(ostream, ";\n");

          def = cdr(def);
        }
      } /* if */
      break;
    }

    case INIT: /* INIT  declarations */
      fprintf(ostream, "INIT\n   ");
      print_node(ostream, car(car(iter)));
      fprintf(ostream, "\n");
      break;

    case TRANS: /* TRANS  declarations */
      fprintf(ostream, "TRANS\n   ");
      print_node(ostream, car(car(iter)));
      fprintf(ostream, "\n");
      break;

    case JUSTICE: /* JUSTICE  declarations */
      fprintf(ostream, "JUSTICE\n   ");
      print_node(ostream, car(car(iter)));
      fprintf(ostream, "\n");
      break;
    default: error_unreachable_code(); /* unexpected node */
    } /*switch */

    iter = cdr(iter);
  } /* while */

  return;
}
#endif
