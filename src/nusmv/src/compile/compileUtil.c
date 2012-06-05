/**CFile***********************************************************************

  FileName    [compileUtil.c]

  PackageName [compile]

  Synopsis    [Routines for model computation.]

  Description [This file contains the code for the compilation of the
  flattened hierarchy into BDD:
  <ul>
  <li> Creation of the boolean variables.</li>
  <li> Creation of the BDD representing the inertia of the system when
       there are processes. In fact when a process is running the
       other processes are stopped, and their state variables don't
       change.</li>
  <li> Creation of the BDD representing what does not change in the
       system, i.e. the set of invariance. These are introduced in the
       model by the keyword "<tt>INVAR</tt>" or by the <em>normal
       assignments</em> (i.e. "<tt>ASSIGN x : = y & z;</tt>"). These
       states are not stored in the transition relation, they are
       stored in an a doc variable.
  <li> Creation of the BDD representing the set of initial states.
  <li> Creation of the BDD representing the transition relation. 
       Various ways of representing the transition relation are offered
       the users.
       <ul>
       <li> <em>Monolithic</em>: the monolithic transition relation is
            computed.</li>
       <li> <em>Conjunctive Partioned (Threshold)</em>: the transition 
            relation is stored as an implicitly conjoined list of 
            transition relation. This kind of partitioning can be 
            used only if the model considered is a synchronous one.</li>
       <li> <em>Conjunctive Partioned IWLS95</em>: as the above, but the
            heuristic proposed in \[1\] is used to order partition clusters. </li>
       </ul>
  <li> Computes the fairness constraints. I.e. each fairness constraint
       (which can be a CTL formula) is evaluated and the resulting BDD
       is stored in the list <tt>fairness_constraints_bdd</tt> to be
       then used in the model checking phase.
  </ul>
  \[1\] R. K. Ranjan and A. Aziz and B. Plessier and C. Pixley and R. K. Brayton,
      "Efficient BDD Algorithms for FSM Synthesis and Verification,
      IEEE/ACM Proceedings International Workshop on Logic Synthesis,
      Lake Tahoe (NV), May 1995.</li>
  ]

  SeeAlso     []

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
#include "compile/compile.h"
#include "compileInt.h" 
#include "compile/symb_table/ResolveSymbol.h"
#include "parser/symbols.h"
#include "utils/ustring.h"
#include "utils/error.h"
#include "utils/WordNumber.h"


static char rcsid[] UTIL_UNUSED = "$Id: compileUtil.c,v 1.29.4.12.4.8.4.15 2009-11-17 15:29:03 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef enum {
  State_Instantiation_Mode,
  Input_Instantiation_Mode
} Instantiation_Vars_Mode_Type;



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static node_ptr Compile_pop_distrib_ops_recurse ARGS((node_ptr prop));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Builds an internal representation for a given string.]

  Description        [Builds an internal representation for a given
  string. If the conversion has been performed in the past, then the
  hashed value is returned back, else a new one is created, hashed and
  returned. We hash this in order to allow the following:
  <pre>
  VAR
     x : {a1, a2, a3};
     y : {a3, a4, a5};

  ASSIGN
     next(x) := case
                 x = y    : a2;
                 !(x = y) : a1;
                 1        : a3;
                esac;
  </pre>
  i.e. to allow the equality test between x and y. This can be
  performed because we internally have a unique representation of the
  atom <tt>a3</tt>.]

  SideEffects        []

  SeeAlso            [find_atom]

******************************************************************************/
node_ptr sym_intern(char *s)
{
  return find_node(ATOM, (node_ptr)find_string(s), Nil);
}



/**Macro***********************************************************************
  Synopsis           [Private macros for the sake of readability of the function
  Compile_pop_global]

  Description        []
******************************************************************************/
/* G a, AG a, H a */ 
#define IS_AND_DISTRIB_OP(prop)             \
   ((OP_GLOBAL == node_get_type(prop)) ||   \
    (AG == node_get_type(prop)) ||          \
    (OP_HISTORICAL == node_get_type(prop)))

/* F a, EF a, O a */ 
#define IS_OR_DISTRIB_OP(prop)              \
   ((OP_FUTURE == node_get_type(prop)) ||   \
    (EF == node_get_type(prop))        ||   \
    (OP_ONCE == node_get_type(prop)))

/*
 * G (a) & G (b) ------> G (a & b)
 * AG (a) & AG (b) ------> AG (a & b)
 * H (a) & H (b) ------> H (a & b)
 */
#define ARE_AND_DISTRIB_OPS(prop1, prop2)         \
   (IS_AND_DISTRIB_OP(prop1) &&                   \
    IS_AND_DISTRIB_OP(prop2) &&                   \
    node_get_type(prop1) == node_get_type(prop2))

/*
 * F (a) | F (b) ------> F (a | b)
 * EF (a) | EF (b) ------> EF (a | b)
 * O (a) | O (b) ------> O (a | b)
 */
#define ARE_OR_DISTRIB_OPS(prop1, prop2)         \
   (IS_OR_DISTRIB_OP(prop1) &&                   \
    IS_OR_DISTRIB_OP(prop2) &&                   \
    node_get_type(prop1) == node_get_type(prop2))



/**Function********************************************************************

  Synopsis           [Simplifies the given property by exploiting 
  the distributivity of G, AG and H over AND, and distributivity of F, AF and O 
  over OR]

  Description        [Transformation rules are:
   1) <OP> <OP> a           :-> <OP> a
   2) (<OP> a) * (<OP> b)   :-> <OP> (a * b);
   3) (<OP> (a * <OP> b))   :-> <OP> (a * b);
   4) (<OP> (<OP> a * b))   :-> <OP> (a * b);
   5) (<OP> (<OP> a * <OP> b)) :-> <OP> (a * b); 

   Where <OP> can be either:
     G|AG|H for * := &
     F|AF|O for * := |

   Given property can be both flattened or unflattened.
   ]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr Compile_pop_distrib_ops(node_ptr prop) {
  node_ptr result;

  result = Compile_pop_distrib_ops_recurse(prop);

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 6)) {
    if (prop == result) {
     fprintf(nusmv_stderr, "-- No simplification occured\n");
    }
    else {
      fprintf(nusmv_stderr, "-- The simplified formula is: \"");
      print_node(nusmv_stderr, result);
      fprintf(nusmv_stderr, "\"\n");
    }
  }
  return result;
}

static node_ptr Compile_pop_distrib_ops_recurse(node_ptr prop) 
{
  if ((node_ptr) NULL == prop) return (node_ptr) NULL;

  /* Base cases */
  switch (node_get_type(prop)) {
  case FAILURE:
  case TRUEEXP: 
  case FALSEEXP:
  case SELF: 
  case BOOLEAN: 
  case ATOM:
  case DOT:
  case ARRAY:
  case NUMBER:
  case NUMBER_UNSIGNED_WORD:
  case UWCONST: case SWCONST:
  case NUMBER_SIGNED_WORD:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case BIT:
    return prop;

  case CONTEXT:
    return find_node(CONTEXT, car(prop), Compile_pop_distrib_ops_recurse(cdr(prop)));

  default: break;
  }

  /* 1) <OP> <OP> a :-> <OP> a */
  if (ARE_AND_DISTRIB_OPS(prop, car(prop)) || 
      ARE_OR_DISTRIB_OPS(prop, car(prop))) {
    return Compile_pop_distrib_ops_recurse(car(prop));
  }

  /* 2) (<OP> a) * (<OP> b) :-> <OP> (a * b); */
  if (AND == node_get_type(prop) || 
      OR == node_get_type(prop)) {
    node_ptr l = Compile_pop_distrib_ops_recurse(car(prop));
    node_ptr r = Compile_pop_distrib_ops_recurse(cdr(prop));

    if ((ARE_AND_DISTRIB_OPS(l, r) && AND == node_get_type(prop)) ||
	(ARE_OR_DISTRIB_OPS(l, r) && OR == node_get_type(prop))) {
      return Compile_pop_distrib_ops_recurse(find_node(node_get_type(l),
		  find_node(node_get_type(prop), car(l), car(r)), Nil));
    }
    else return find_node(node_get_type(prop), l, r);
  }

  /* 3) (<OP> (a * <OP> b))   :-> <OP> (a * b);
     4) (<OP> (<OP> a * b))   :-> <OP> (a * b);
     5) (<OP> (<OP> a * <OP> b)) :-> <OP> (a * b); */

  if (IS_AND_DISTRIB_OP(prop) || IS_OR_DISTRIB_OP(prop)) {
    int op = node_get_type(car(prop));
    if (AND == op || OR == op) {
      node_ptr l = Compile_pop_distrib_ops_recurse(car(car(prop)));
      node_ptr r = Compile_pop_distrib_ops_recurse(cdr(car(prop)));

      if ( ((AND == op) && ARE_AND_DISTRIB_OPS(prop, l) && 
	    ARE_AND_DISTRIB_OPS(prop, r)) 
	   ||
	   ((OR == op) && ARE_OR_DISTRIB_OPS(prop, l) && 
	    ARE_OR_DISTRIB_OPS(prop, r)) ) { /* 5 */

        return Compile_pop_distrib_ops_recurse(find_node(node_get_type(prop),
					 find_node(op, car(l), car(r)), Nil));
      }
      else if ( ((AND == op) && ARE_AND_DISTRIB_OPS(prop, l)) 
		|| 
		((OR == op) && ARE_OR_DISTRIB_OPS(prop, l)) ) { /* 4 */
        return Compile_pop_distrib_ops_recurse(find_node(node_get_type(prop), 
					    find_node(op, car(l), r), Nil));
      }
      else if ( ((AND == op) && ARE_AND_DISTRIB_OPS(prop, r)) 
		|| 
		((OR == op) && ARE_OR_DISTRIB_OPS(prop, r)) ) { /* 3 */
        return Compile_pop_distrib_ops_recurse(find_node(node_get_type(prop),
					    find_node(op, l, car(r)), Nil));
      }
      return find_node(node_get_type(prop), find_node(op, l, r), Nil);
    }
  }

  /* fall back case */
  return find_node(node_get_type(prop), 
		   Compile_pop_distrib_ops_recurse(car(prop)), 
		   Compile_pop_distrib_ops_recurse(cdr(prop)));
}


/**Function********************************************************************

  Synopsis [This function creates a new list of variables that will
  contain the same symbols into 'vars', but ordered wrt to
  'vars_order' content]

  Description [This function can be used to construct an ordered list
  of symbols. The set of symbols is provided by the input list 'vars',
  whereas the ordering is provided by the 'vars_order' list, that can
  be an intersecting set over 'vars'. The resulting list will
  contain those symbols that occur in vars_order (respecting their
  order), plus all the symbols in vars that do not occur in vars_order,
  pushed at the end of the list. All duplicates (if any) will not occur
  into the resulting list. The returned set must be destroyed by the
  caller.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t 
Compile_make_sorted_vars_list_from_order(const SymbTable_ptr st, 
                                         const NodeList_ptr vars, 
                                         const NodeList_ptr vars_order)
{
  Set_t res;
  ListIter_ptr iter;

  res = Set_MakeEmpty();

  /* pushes all the names in vars_order that belong to the intersection
     of vars and vars_order */
  NODE_LIST_FOREACH(vars_order, iter) {
    ResolveSymbol_ptr rs;
    node_ptr name;

    rs = SymbTable_resolve_symbol(st,
                          NodeList_get_elem_at(vars_order, iter), Nil);
    name = ResolveSymbol_get_resolved_name(rs);

    if (NodeList_belongs_to(vars, name)) {
      res = Set_AddMember(res, (Set_Element_t) name);
    }
  }

  /* pushes all the remaining symbols at the end of the resulting list */
  NODE_LIST_FOREACH(vars, iter) {
    ResolveSymbol_ptr rs;
    node_ptr name;

    rs = SymbTable_resolve_symbol(st,
                          NodeList_get_elem_at(vars, iter), Nil);
    name = ResolveSymbol_get_resolved_name(rs);

    res = Set_AddMember(res, name);
  }
  
  return res;  
}



/**Function********************************************************************

  Synopsis           [Checks if bdd model has been constructed]

  Description [Returns 0 if constructed, 1 otherwise. If given file is
  not NULL, an error message is also printed out to it (typically, you
  will use nusmv_stderr). Use this function from commands that require 
  the model to be constructed for being executed.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Compile_check_if_model_was_built(FILE* err, boolean forced)
{
  if (cmp_struct_get_build_model(cmps)) return 0;

  if (Compile_check_if_encoding_was_built(err)) return 1;

  if (cmp_struct_get_build_model(cmps) == 0 && 
      opt_cone_of_influence(OptsHandler_get_instance()) && !forced) return 0;

  if (err != (FILE*) NULL) {
    if (opt_cone_of_influence(OptsHandler_get_instance())) {
      fprintf(err, 
	"Model construction was delayed due to the use of Cone Of Influence.\n"
	"Use the command \"build_model -f\" to force the model construction.\n");
    }
    else fprintf(err,
	   "A model must be built before. Use the \"build_model\" command.\n");        
  }
  
  return 1;
}


/**Function********************************************************************

  Synopsis           [Checks if boolean model has been constructed]

  Description [Returns 0 if constructed, 1 otherwise. If given file is
  not NULL, an error message is also printed out to it (typically, you
  will use nusmv_stderr). If forced is true, thatn the model is
  requested to be built even when COI is enabled.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Compile_check_if_bool_model_was_built(FILE* err, boolean forced)
{
  if (cmp_struct_get_build_bool_model(cmps)) return 0;
  if (cmp_struct_get_build_bool_model(cmps) == 0 && 
      cmp_struct_get_encode_variables(cmps) && 
      opt_cone_of_influence(OptsHandler_get_instance()) && !forced) return 0;

  if (Compile_check_if_encoding_was_built(err)) return 1;

  if (err != (FILE*) NULL) {
    if (cmp_struct_get_build_bool_model(cmps) == 0) {
      fprintf(err, "The boolean model must be built before.\n");
      if (opt_cone_of_influence(OptsHandler_get_instance()) && forced) {
	fprintf(err, "(Use the command \"build_boolean_model -f\" as Cone Of Influence is enabled.)\n");
      }
      else fprintf(err, "(Use the command \"build_boolean_model\")\n");
    }
  }

  return 1;
}



/**Function********************************************************************

  Synopsis           [Checks if flat model has been constructed]

  Description [Returns 0 if constructed, 1 otherwise. If given file is
  not NULL, an error message is also printed out to it (typically, you
  will use nusmv_stderr). If forced is true, than the model is
  requested to be built even when COI is enabled.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Compile_check_if_flat_model_was_built(FILE* err, boolean forced)
{
  if (cmp_struct_get_build_flat_model(cmps)) return 0;

  if (cmp_struct_get_build_flat_model(cmps) == 0 && 
      opt_cone_of_influence(OptsHandler_get_instance()) && !forced) return 0;

  if (Compile_check_if_flattening_was_built(err)) return 1;

  if (err != (FILE*) NULL) {
    if (cmp_struct_get_build_flat_model(cmps) == 0) {
      fprintf(err, "The flat model must be built before "\
	      "(Use the command \"build_flat_model\")\n");
    }
  }

  return 1;
}


/**Function********************************************************************

  Synopsis           [Checks if the variables enconding has been constructed]

  Description [Returns 0 if constructed, 1 otherwise. If given file is
  not NULL, an error message is also printed out to it (typically, you
  will use nusmv_stderr)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Compile_check_if_encoding_was_built(FILE* err)
{
  if (cmp_struct_get_encode_variables(cmps)) return 0;

  
  if (Compile_check_if_flattening_was_built(err)) return 1;
  //if (Compile_check_if_flat_model_was_built(err, false)) return 1;

  if (err != (FILE*) NULL) {
    fprintf(err, 
	    "The variables must be built before. Use the "\
	    "\"encode_variables\" command.\n");
  }

  return 1;
}


/**Function********************************************************************

Synopsis           [Checks if the flattening has been carried out]

  Description [Returns 0 if constructed, 1 otherwise. If given file is
  not NULL, an error message is also printed out to it (typically, you
  will use nusmv_stderr)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Compile_check_if_flattening_was_built(FILE* err)
{
  if (cmp_struct_get_flatten_hrc(cmps)) return 0;

  if (err != (FILE*) NULL) {
    if (cmp_struct_get_read_model(cmps) == 0) {
      fprintf(err,
	      "A model must be read before. Use the \"read_model\" command.\n");
    }

    else fprintf(err,
		 "The hierarchy must be flattened before. Use the "\
		 "\"flatten_hierarchy\" command.\n");
  }

  return 1;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

