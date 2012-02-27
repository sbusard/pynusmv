/**CFile***********************************************************************

FileName    [FlatHierarchy.c]

PackageName [compile]

Synopsis    [The class is used to store results of flattening a hierarchy]

Description [See description in FlatHierarchy.h]

Author      [Andrei Tchaltsev]

Copyright   [
This file is part of the ``compile'' package of NuSMV version 2.
Copyright (C) 2005 by FBK-irst.

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
#include "FlatHierarchy.h"
#include "utils/utils.h"
#include "utils/assoc.h"
#include "utils/NodeList.h"
#include "parser/symbols.h"
#include "fsm/sexp/Expr.h"
#include "utils/error.h"


static char rcsid[] UTIL_UNUSED = "$Id: FlatHierarchy.c,v 1.1.2.1.6.37 2010-01-25 19:43:34 nusmv Exp $";



/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

Synopsis    [Data structure used to store the results of compilation.]

Description []

******************************************************************************/
struct FlatHierarchy
{
  SymbTable_ptr st;
  boolean vars_constr_built; /* whether vars constrains have been built */

  node_ptr init_expr;
  node_ptr invar_expr;
  node_ptr trans_expr;
  node_ptr input_expr;
  node_ptr assign_expr;
  node_ptr justice_expr;
  node_ptr compassion_expr;
  node_ptr spec_expr;
  node_ptr ltlspec_expr;
  node_ptr invarspec_expr;
  node_ptr pslspec_expr;
  node_ptr compute_expr;

  node_ptr pred_list;
  node_ptr mirror_list;

  Set_t var_set; /* variables declared in the given hierarchy */

  hash_ptr assign_hash; /* a hash table that for every variable
                           'var_name' the hash contains the following associations:
                           init(var_name) -> CONS(rhs, init_list)
                           where rhs is the right handsides of init-assignments of the
                           given variable
                           init-list is a list of INIT expressions of this hierarchy,
                           which contain the variable
                           next(var_name) -> CONS( rhs, trans-list)
                           where rhs can be
                           case module-1.running : rhs-1;
                           case module-2.running : rhs-2;
                           ...
                           case 1: var_name;

                           where rhs-n is the right hand side of
                           the next-assignment in the process module-n.
                           If there are no processes then the structure degrades
                           to just one 'rhs'.
                           trans-list is a list of TRANS of this hierarchy,
                           which contain var_name or expression next(var_name)

                           var_name : the structure is the same as for init(var_name) except
                           that the rhs-n are the right handside of invar-assignment, and
                           init-list is a list of INVAR expressions  */

  hash_ptr const_constr_hash;

  /* Hash table used for properties names univocity */
  hash_ptr property_hash;
};


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void flat_hierarchy_init ARGS((FlatHierarchy_ptr self,
                                      SymbTable_ptr st));
static void flat_hierarchy_deinit ARGS((FlatHierarchy_ptr self));
static void flat_hierarchy_copy ARGS((const FlatHierarchy_ptr self,
                                      FlatHierarchy_ptr other));
static void
flat_hierarchy_mergeinto ARGS((FlatHierarchy_ptr self,
                               FlatHierarchy_ptr other));

/* This will be removed when assignment of bit selection is implemented */
void error_bit_selection_assignment_not_supported ARGS((node_ptr name));

static void
flat_hierarchy_calc_var_const_aux ARGS((FlatHierarchy_ptr self,
                                        node_ptr expr, int type));

static void
flat_hierarchy_calc_var_const_recur ARGS((FlatHierarchy_ptr self,
                                          node_ptr expr, int type));

static boolean flat_hierarchy_check_const_deps ARGS((FlatHierarchy_ptr self,
                                                     node_ptr expr, int type));

static void
flat_hiearchy_self_check_expr ARGS((const FlatHierarchy_ptr self, node_ptr expr));

static const char* constr_type_to_string ARGS((int type));

static void flat_hierarchy_visit_dag ARGS((hash_ptr outbounds, hash_ptr visited,
                                           node_ptr var, NodeList_ptr result));

static assoc_retval
flat_hierarchy_free_assign_fun ARGS((char * key, char * data, char * arg));
/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

Synopsis           [The constructor]

Description        [The class is used to store information
obtained after flattening module hierarchy.
These class stores:
the list of TRANS, INIT, INVAR, ASSIGN, SPEC, COMPUTE, LTLSPEC,
PSLSPEC, INVARSPEC, JUSTICE, COMPASSION,
a full list of variables declared in the hierarchy,
a hash table associating variables to their assignments and constrains.

NOTE: this structure is filled in by compileFlatten.c routines. There are
a few assumptions about the content stored in this class:
1. All expressions are stored in the same order as in the input
file (in module body or module instantiation order).
2. Assigns are stored as a list of pairs
{process instance name, assignments in it}.
3. Variable list contains only vars declared in this hierarchy.
4. The association var->assignments should be for assignments of
this hierarchy only.
Note that var may potentially be from another hierarchy. For
example, with Games of the GAME package an assignment in the body of
one hierarchy (one player) may have on the left hand side a variable from
another hierarchy (another player).
See FlatHierarchy_lookup_assign, FlatHierarchy_insert_assign
5. The association var->constrains (init, trans, invar) should be
for constrains of this hierarchy only. Similar to
var->assignment association (see above) a variable may
potentially be from another hierarchy.
See FlatHierarchy_lookup_constrains, FlatHierarchy_add_constrains
]

SideEffects        []

******************************************************************************/
FlatHierarchy_ptr FlatHierarchy_create(SymbTable_ptr st)
{
  FlatHierarchy_ptr self = ALLOC(struct FlatHierarchy, 1);
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  flat_hierarchy_init(self, st);
  return self;
}


/**Function********************************************************************

Synopsis           [Class FlatHierarcy destructorUtility constructor]

Description        [Use this constructor to set the main hierarchy members]

SideEffects        [FlatHierarchy_create]

******************************************************************************/
EXTERN FlatHierarchy_ptr
FlatHierarchy_create_from_members(SymbTable_ptr st,
                                  node_ptr init, node_ptr invar,
                                  node_ptr trans, node_ptr input,
                                  node_ptr justice, node_ptr compassion)
{
  FlatHierarchy_ptr self = FlatHierarchy_create(st);

  FlatHierarchy_set_init(self, init);
  FlatHierarchy_set_invar(self, invar);
  FlatHierarchy_set_trans(self, trans);
  FlatHierarchy_set_input(self, input);
  FlatHierarchy_set_justice(self, justice);
  FlatHierarchy_set_compassion(self, compassion);

  return self;
}


/**Function********************************************************************

Synopsis           [Class FlatHierarcy destructor]

Description        [The destoructor does not destroy the nodes
given to it with access functions.]

SideEffects        [FlatHierarchy_create]

******************************************************************************/
void FlatHierarchy_destroy(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  flat_hierarchy_deinit(self);
  FREE(self);
}


/**Function********************************************************************

Synopsis           [Returns a newly created instance that is a copy of self]

Description        []

SideEffects        []

******************************************************************************/
FlatHierarchy_ptr FlatHierarchy_copy(const FlatHierarchy_ptr self)
{
  FlatHierarchy_ptr res;

  FLAT_HIERARCHY_CHECK_INSTANCE(self);

  res = FlatHierarchy_create(self->st);
  flat_hierarchy_copy(self, res);
  return res;
}

/**Function********************************************************************

Synopsis           [Merges the contents of other into self (leaves other
intact)]

Description        []

SideEffects        [flat_hierarchy_mergeinto,
SexpFsm_apply_synchronous_product]

******************************************************************************/
void FlatHierarchy_mergeinto(FlatHierarchy_ptr self,
                             const FlatHierarchy_ptr other)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  FLAT_HIERARCHY_CHECK_INSTANCE(other);

  flat_hierarchy_mergeinto(self, other);
}


/**Function********************************************************************

Synopsis           [Returns the associated symbol table]

Description        []

SideEffects        []

******************************************************************************/
SymbTable_ptr FlatHierarchy_get_symb_table(const FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return self->st;
}


/**Function********************************************************************

Synopsis           [Returns the associated symbol table]

Description        []

SideEffects        []

******************************************************************************/
void FlatHierarchy_set_symb_table(const FlatHierarchy_ptr self,
                                  SymbTable_ptr symb_table)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->st = symb_table;
}


/**Function********************************************************************

Synopsis           [A set of functions accessing the fields of the class]

Description        []

SideEffects        []

******************************************************************************/
node_ptr FlatHierarchy_get_init(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->init_expr);
}
void FlatHierarchy_set_init(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);

  /* later recalculates vars constr if needed */
  self->vars_constr_built = self->vars_constr_built & (self->init_expr == n);
  self->init_expr = n;
}

node_ptr FlatHierarchy_get_invar(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->invar_expr);
}
void FlatHierarchy_set_invar(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  /* later recalculates vars constr if needed */
  self->vars_constr_built = self->vars_constr_built & (self->invar_expr == n);
  self->invar_expr = n;
}

node_ptr FlatHierarchy_get_trans(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->trans_expr);
}
void FlatHierarchy_set_trans(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  /* later recalculates vars constr if needed */
  self->vars_constr_built = self->vars_constr_built & (self->trans_expr == n);
  self->trans_expr = n;
}

node_ptr FlatHierarchy_get_input(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->input_expr);
}
void FlatHierarchy_set_input(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->input_expr = n;
}

/* Warning: after flattening is done this function returns has a list
   of pairs <process-name, its-assignments>, where its-assignments
   consists of the original assignments from the input without the
   wrap of CASE with condition over "running" variable.  To access the
   actual assignments used in FSM it is necessary to access FlatHierarchy_lookup_assign
   as it is done in, for example, compile_write_flat_asgn.
*/
node_ptr FlatHierarchy_get_assign(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->assign_expr);
}
void FlatHierarchy_set_assign(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->assign_expr = n;
}

node_ptr FlatHierarchy_get_justice(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->justice_expr);
}
void FlatHierarchy_set_justice(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->justice_expr = n;
}

node_ptr FlatHierarchy_get_compassion(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->compassion_expr);
}
void FlatHierarchy_set_compassion(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->compassion_expr = n;
}

boolean FlatHierarchy_add_property_name(FlatHierarchy_ptr self,
                                        node_ptr name)
{
  /* Return false if the name is already in the hash. This means the
     name is duplicate */
  if (NODE_FROM_INT(1) == find_assoc(self->property_hash, name)){
    return false;
  }

  insert_assoc(self->property_hash, name, NODE_FROM_INT(1));
  return true;
}

node_ptr FlatHierarchy_get_spec(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return (self->spec_expr);
}

void FlatHierarchy_set_spec(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->spec_expr = n;
}

node_ptr FlatHierarchy_get_ltlspec(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->ltlspec_expr);
}
void FlatHierarchy_set_ltlspec(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->ltlspec_expr = n;
}

node_ptr FlatHierarchy_get_invarspec(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->invarspec_expr);
}
void FlatHierarchy_set_invarspec(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->invarspec_expr = n;
}

node_ptr FlatHierarchy_get_pslspec(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->pslspec_expr);
}
void FlatHierarchy_set_pslspec(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->pslspec_expr = n;
}

node_ptr FlatHierarchy_get_compute(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return(self->compute_expr);
}
void FlatHierarchy_set_compute(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->compute_expr = n;
}


/**Function********************************************************************

Synopsis [Returns the set of variables declared in the given hierarchy]

Description        [Do not destroy or change returned set]

SideEffects        []

SeeAlso            [FlatHierarchy_add_var]

******************************************************************************/
Set_t FlatHierarchy_get_vars(const FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return self->var_set;
}

/**Function********************************************************************

Synopsis           [Returns an ordered list of variables]

Description        [Starting from hierarchy assignments, creates a DAG
                    and returns the topological sort of it. The set of nodes
                    contained in this dag is the union of the dependencies
                    of all assings expressions of the hierarchy.
                    If not NULL, outbound_edges will contain a map between vars
                    and their respective outgoing edges to other variables]

SideEffects        []

SeeAlso            []

******************************************************************************/
NodeList_ptr
FlatHierarchy_get_ordered_vars(const FlatHierarchy_ptr self,
                               hash_ptr* outbound_edges)
{
  NodeList_ptr result;
  SymbTable_ptr symb_table = self->st;
  Set_t nodes = Set_MakeEmpty();
  hash_ptr outbounds = new_assoc();
  assoc_iter iter;
  node_ptr var, assignment;

  /* Foreach assign variable, get it's assignment expression and build
     the DAG */
  ASSOC_FOREACH(self->assign_hash, iter, &var, &assignment) {
    boolean is_normal_assignment = false;
    Set_t deps;

    if (SMALLINIT == node_get_type(var)) {
      var = car(var);
    }
    else if (!(NEXT == node_get_type(var))) {
      is_normal_assignment = true;
    }

    deps = Formula_GetDependenciesByType(symb_table,
                                         node_normalize(assignment),
                                         Nil, VFT_ALL, true);

    nodes = Set_AddMember(nodes, (Set_Element_t)var);
    if (is_normal_assignment) {
      /* Normal Assignment: We add both init(x) and next(x) */
      nodes = Set_AddMember(nodes, (Set_Element_t)Expr_next(var, symb_table));
    }

    {
      Set_Iterator_t iter;
      SET_FOREACH(deps, iter) {
        node_ptr dep = Set_GetMember(deps, iter);
        Set_t out = (Set_t)find_assoc(outbounds, dep);

        if ((Set_t)NULL == out) {
          out = Set_MakeEmpty();
        }

        /* Create the edge */
        if (dep != var) {
          out = Set_AddMember(out, (Set_Element_t)var);
          if (is_normal_assignment) {
            /* Normal Assignment: We add both init(x) and next(x) */
            out = Set_AddMember(out, (Set_Element_t)Expr_next(var, symb_table));
          }
        }
        insert_assoc(outbounds, dep, (Set_Element_t)out);
        nodes = Set_AddMember(nodes, (Set_Element_t)dep);
      }
    }

    Set_ReleaseSet(deps);
  }

  /* Topological sort */
  {
    result = NodeList_create();
    hash_ptr visited = new_assoc();
    Set_Iterator_t iter;

    SET_FOREACH(nodes, iter) {
      node_ptr var = NODE_PTR(Set_GetMember(nodes, iter));
      flat_hierarchy_visit_dag(outbounds, visited, var, result);
    }

    free_assoc(visited);
  }

  Set_ReleaseSet(nodes);

  if ((hash_ptr*)NULL != outbound_edges) {
    *outbound_edges = outbounds;
  }
  /* If the caller did not request the hash map, free it */
  else {
    assoc_iter iter;
    node_ptr tmp;
    Set_t set;

    ASSOC_FOREACH(outbounds, iter, &tmp, &set) {
      Set_ReleaseSet(set);
    }
  }

  return result;
}


/**Function********************************************************************

Synopsis           [Add a variable name to the list of variables
declared in the given hierarchy]

Description        []

SideEffects        []

SeeAlso            [FlatHierarchy_get_vars]

******************************************************************************/
void FlatHierarchy_add_var(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->var_set = Set_AddMember(self->var_set, (Set_Element_t) n);
}


/**Function********************************************************************

Synopsis           [Remove a variable name to the list of variables
                    declared in the given hierarchy]

Description        []

SideEffects        []

SeeAlso            [FlatHierarchy_get_vars]

******************************************************************************/
void FlatHierarchy_remove_var(FlatHierarchy_ptr self, node_ptr n)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->var_set = Set_RemoveMember(self->var_set, (Set_Element_t) n);
}


/**Function********************************************************************

Synopsis           [Returns the right handside of an assignment which has
"name" as the left handside.]

Description        [The name can be a usual variable name, init(variable
name) or next(variable name). The name should be fully resolved (and
created with find_node).

NB: All returned assignments are supposed to be declared in the
given hierarchy.]

SideEffects        []

SeeAlso            [FlatHierarchy_insert_assign]

******************************************************************************/
node_ptr FlatHierarchy_lookup_assign(FlatHierarchy_ptr self, node_ptr name)
{
  node_ptr res;
  nusmv_assert(self != NULL);

  /* Currently, bit selection is not allowed on the left side of
     assignments */
  if (node_get_type(name) == BIT_SELECTION ||
      ((node_get_type(name) == NEXT || node_get_type(name) == SMALLINIT) &&
       node_get_type(car(name)) == BIT_SELECTION)) {
    error_bit_selection_assignment_not_supported(name);
  }

  /* common consistency check : the name should be correctly formed */
  nusmv_assert(SMALLINIT == node_get_type(name) ||
               NEXT == node_get_type(name) ||
               DOT == node_get_type(name) || ATOM == node_get_type(name) ||
               ARRAY == node_get_type(name) || BIT == node_get_type(name));

  res = find_assoc(self->assign_hash, name);
  if (Nil == res) return Nil;

  nusmv_assert(CONS == node_get_type(res));

  return car(res);
}


/**Function********************************************************************

Synopsis           [Insert the right handside of an assignment which
has "name" as the left handside]

Description        [The name can be a usual variable name, init(var-name) or
next(var-name).
The variable name should be fully resolved (and created  with find_node).

NB: All given assignments should have been declared in the given hierarchy.]

SideEffects        []

SeeAlso            [FlatHierarchy_lookup_assign]

******************************************************************************/
void FlatHierarchy_insert_assign(FlatHierarchy_ptr self, node_ptr name,
                                 node_ptr assign)
{
  node_ptr cont;

  nusmv_assert(self != NULL);
  cont = find_assoc(self->assign_hash, name);

  if (Nil == cont) { /* there was no container before => create a new one */
    cont = cons(Nil, Nil);
    insert_assoc(self->assign_hash, name, cont);
  }

  /* [AT] I think, this is true:
     name and init(name) can have only one assignment
  */
  nusmv_assert( (node_get_type(name) != SMALLINIT &&
                 node_get_type(name) != DOT) || Nil == car(cont));

  /* If change this, we *MUST* also change simp/fsm/logic_elimination.c
     which uses this feature to overwrite next after a copy.
   */

  /* set car is allowed here as the node was created with new_node */
  setcar(cont, assign);
}


/**Function********************************************************************

Synopsis           [Returns  a list of constrains which contain
a variable of the given name.]

Description        [
If the parameter "name" is a usual variable name then
the INVAR expressions are returned.
If the parameter "name" has a form init(var-name) then
the INIT expressions are returned.
If the parameter "name" has a form next(var-name) then
the TRANS expressions are returned.

The name should be fully resolved (and created with find_node).

NB: All returned expressions are supposed to be declared in the
given hierarchy.]

SideEffects        []

SeeAlso            [FlatHierarchy_add_constrains]

******************************************************************************/
node_ptr FlatHierarchy_lookup_constrains(FlatHierarchy_ptr self,
                                         node_ptr name)
{
  node_ptr res;
  FLAT_HIERARCHY_CHECK_INSTANCE(self);

  /* common consistency check : the name should be correctly formed */
  nusmv_assert(SMALLINIT == node_get_type(name) || NEXT == node_get_type(name) ||
               DOT == node_get_type(name) ||
               ARRAY == node_get_type(name) || BIT == node_get_type(name) ||
               ATOM == node_get_type(name));

  /* if not previously calculated (or if invalidated by later
     settings) triggers calculation of var constrains */
  if (!self->vars_constr_built) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "Triggering postponed "\
              "calculation of vars constrains\n");
    }
    FlatHierarchy_calculate_vars_constrains(self);
  }

  res = find_assoc(self->assign_hash, name);
  if (Nil == res) return Nil;

  nusmv_assert(CONS == node_get_type(res));

  return cdr(res);
}

/**Function********************************************************************

Synopsis           [Adds the given expressions to the list
of constrains associated to the given variable]

Description        [
The parameter "name" can be a usual variable name then
an expression is expected to be INVAR body.
The parameter "name" can have a form init(var-name) then
an expression is expected to be INIT body.
The parameter "name" can have a form next(var-name) then
an expression is expected to be TRANS body.

In any case the variable name should be fully resolved (and created
with find_node).

NB: All given expressions should have been declared in the given hierarchy.]


SideEffects        []

SeeAlso            [FlatHierarchy_lookup_constrains]

******************************************************************************/
void FlatHierarchy_add_constrains(FlatHierarchy_ptr self, node_ptr name,
                                  node_ptr expr)
{
  node_ptr cont;

  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  cont = find_assoc(self->assign_hash, name);

  if (Nil == cont) { /* there was no container before => create a new one */
    cont = cons(Nil, Nil);
    insert_assoc(self->assign_hash, name, cont);
  }

  if (Nil == cdr(cont)) setcdr(cont, expr);
  else setcdr(cont, find_node(AND, cdr(cont), expr));
}

/**Function********************************************************************

Synopsis           [Retrieves the list of constrains associated to constants]

Description        [Retrieves the list of constrains associated to constants
                    for the given hierarchy section.
                    Type must be one of "INIT, INVAR or TRANS"]


SideEffects        []

SeeAlso            [FlatHierarchy_add_constant_constrains]

******************************************************************************/
node_ptr FlatHierarchy_lookup_constant_constrains(FlatHierarchy_ptr self,
                                                  int type)
{
  node_ptr res;

  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  nusmv_assert(INIT == type || TRANS == type || INVAR == type);

  /* if not previously calculated (or if invalidated by later
     settings) triggers calculation of var constrains */
  if (!self->vars_constr_built) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "Triggering postponed "\
              "calculation of vars constrains\n");
    }
    FlatHierarchy_calculate_vars_constrains(self);
  }

  res = find_assoc(self->const_constr_hash, NODE_FROM_INT(type));
  return res;
}

/**Function********************************************************************

Synopsis           [Adds the given expressions to the list
                    of constrains associated to constants]

Description        [Adds the given expressions to the list
                    of constrains associated to constants.
                    Type must be one of "INIT, INVAR or TRANS"]


SideEffects        []

SeeAlso            [FlatHierarchy_lookup_constant_constrains]

******************************************************************************/
void FlatHierarchy_add_constant_constrains(FlatHierarchy_ptr self,
                                           node_ptr expr,
                                           int type)
{
  node_ptr tmp;
  FLAT_HIERARCHY_CHECK_INSTANCE(self);

  nusmv_assert(INIT == type || TRANS == type || INVAR == type);

  tmp = find_assoc(self->const_constr_hash, NODE_FROM_INT(type));

  /* If a constrain already exist, put it in AND with the new one.. */
  if (Nil != tmp) { tmp = Expr_and(tmp, expr); }
  else {
    tmp = expr;
  }

  insert_assoc(self->const_constr_hash, NODE_FROM_INT(type), tmp);
}


/**Function********************************************************************

Synopsis           [This function returns the hash table storing the
                    association between the hierarchy section and
                    constants expressions]

Description        [self retains ownership of returned value.
                    Note: you should know what you are doing when
                    performing modifications on the table.  ]

SideEffects        []

SeeAlso            []

******************************************************************************/
hash_ptr FlatHierarchy_get_constants_associations(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return self->const_constr_hash;
}

/**Function********************************************************************

Synopsis           [This function sets the hash table storing the
                    association between the hierarchy section and
                    constants expressions]

Description        [self retains ownership of h.
                    Note: you should know what you are doing when
                    performing modifications on the table.  ]

SideEffects        []

SeeAlso            []

******************************************************************************/
void FlatHierarchy_set_constants_associations(FlatHierarchy_ptr self,
                                              hash_ptr h)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->const_constr_hash = h;
}

/**Function********************************************************************

Synopsis           [Clears the association between hierarchy sections
                    and constant expressions]

Description        [Clears the association between hierarchy sections
                    and constant expressions]

SideEffects        []

SeeAlso            []

******************************************************************************/
void FlatHierarchy_clear_constants_associations(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  clear_assoc(self->const_constr_hash);
}


/**Function********************************************************************

Synopsis    [Creates association between variables and all the expressions
the variables occur in.]

Description [For every variable var-name in the given expressions the
function creates association between:
1. var-name and and the INVAR expression list the variable occur.
2. init(var-name) and INIT expression list
3. next(var-name) and TRANS expression list.
The result is remembered by flatHierarchy.

The function compileFlattenProcess works similarly but with assignments.]

SideEffects        []

SeeAlso            [compileFlattenProcess]

******************************************************************************/
void FlatHierarchy_calculate_vars_constrains(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "FlatHierarchy: "         \
            "calculating vars constrains...\n");
  }

  flat_hierarchy_calc_var_const_aux(self, FlatHierarchy_get_init(self), INIT);
  flat_hierarchy_calc_var_const_aux(self, FlatHierarchy_get_invar(self), INVAR);
  flat_hierarchy_calc_var_const_aux(self, FlatHierarchy_get_trans(self), TRANS);

  self->vars_constr_built = true;
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "FlatHierarchy: vars constrains calculated\n");
  }
}


/**Function********************************************************************

Synopsis           [This function returns the hash table storing the
association between a variable name and expressions the variable is used in.]

Description        [self retains ownership of returned value.

Note: you should know what you are doing when performing modifications
on the table.
]


SideEffects        []

SeeAlso            []

******************************************************************************/
hash_ptr FlatHierarchy_get_var_expr_associations(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  return self->assign_hash;
}


/**Function********************************************************************

Synopsis           [This function sets the hash table storing the
association between a variable name and expressions the variable is used in
to h.]

Description        [self obtains ownership of h.

Note: you should know what you are doing when using this function. You are
fully responsible that the contents of h make sense - no checks are performed
whatsoever.
]


SideEffects        []

SeeAlso            []

******************************************************************************/
void FlatHierarchy_set_var_expr_associations(FlatHierarchy_ptr self,
                                             hash_ptr h)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  self->assign_hash = h;
}


/**Function********************************************************************

Synopsis           [This function cleans the association between a variable name
and expressions the variable is used in.]

Description        [
Practically, this function cleans association created by
FlatHierarchy_insert_assign and FlatHierarchy_add_constrains such that
functions FlatHierarchy_lookup_assign and FlatHierarchy_lookup_constrains
will return Nil for any var name.

Note: you should know what you are doing when invoke this function since
it makes COI and various checks of FSM incorrect.
]


SideEffects        []

SeeAlso            []

******************************************************************************/
void FlatHierarchy_clear_var_expr_associations(FlatHierarchy_ptr self)
{
  FLAT_HIERARCHY_CHECK_INSTANCE(self);
  clear_assoc(self->assign_hash);
}


node_ptr FlatHierarchy_get_preds(FlatHierarchy_ptr cmp)
{
  return cmp->pred_list;
}

void FlatHierarchy_add_pred(FlatHierarchy_ptr cmp, node_ptr n)
{
  cmp->pred_list = cons(n, cmp->pred_list);
}

void FlatHierarchy_set_pred(FlatHierarchy_ptr cmp, node_ptr n)
{
  cmp->pred_list = n;
}

node_ptr FlatHierarchy_get_mirrors(FlatHierarchy_ptr cmp)
{
  return cmp->mirror_list;
}

void FlatHierarchy_add_mirror(FlatHierarchy_ptr cmp, node_ptr n)
{
  cmp->mirror_list = cons(n, cmp->mirror_list);
}

void FlatHierarchy_set_mirror(FlatHierarchy_ptr cmp, node_ptr n)
{
  cmp->mirror_list = n;
}


/**Function********************************************************************

Synopsis           [Performs a self check of the instance content wrt the
                    set of language self was declared to contain]

Description        []

SideEffects        []

SeeAlso            []

******************************************************************************/
void FlatHierarchy_self_check(const FlatHierarchy_ptr self)
{
  flat_hiearchy_self_check_expr(self, self->init_expr);
  flat_hiearchy_self_check_expr(self, self->invar_expr);
  flat_hiearchy_self_check_expr(self, self->trans_expr);
  flat_hiearchy_self_check_expr(self, self->input_expr);
  flat_hiearchy_self_check_expr(self, self->assign_expr);
  flat_hiearchy_self_check_expr(self, self->justice_expr);
  flat_hiearchy_self_check_expr(self, self->compassion_expr);
  flat_hiearchy_self_check_expr(self, self->spec_expr);
  flat_hiearchy_self_check_expr(self, self->ltlspec_expr);
  flat_hiearchy_self_check_expr(self, self->invarspec_expr);
  flat_hiearchy_self_check_expr(self, self->compute_expr);
}


/*---------------------------------------------------------------------------*/
/* Static function definitions                                               */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [initialisation function used by the constructor]

Description        []

SideEffects        []

******************************************************************************/
static void flat_hierarchy_init(FlatHierarchy_ptr self, SymbTable_ptr st)
{
  self->st                   = st;
  self->vars_constr_built    = false;

  self->init_expr            = Nil;
  self->invar_expr           = Nil;
  self->trans_expr           = Nil;
  self->input_expr           = Nil;
  self->assign_expr          = Nil;
  self->justice_expr         = Nil;
  self->compassion_expr      = Nil;
  self->spec_expr            = Nil;
  self->compute_expr         = Nil;
  self->ltlspec_expr         = Nil;
  self->pslspec_expr         = Nil;
  self->invarspec_expr       = Nil;
  self->var_set              = Set_MakeEmpty();
  self->pred_list            = Nil;
  self->mirror_list          = Nil;

  self->assign_hash = new_assoc();
  self->property_hash = new_assoc();
  self->const_constr_hash = new_assoc();
}

static assoc_retval
flat_hierarchy_free_assign_fun(char * key, char * data, char * arg)
{
  node_ptr n = NODE_PTR(data);

  if (Nil != n) {
    free_node(n);
  }

  return ASSOC_DELETE;
}


/**Function********************************************************************

Synopsis           [de-initialisation function used by the destructor]

Description        []

SideEffects        []

******************************************************************************/
static void flat_hierarchy_deinit(FlatHierarchy_ptr self)
{
  Set_ReleaseSet(self->var_set);

  clear_assoc_and_free_entries(self->assign_hash,
                               flat_hierarchy_free_assign_fun);
  free_assoc(self->assign_hash);
  free_assoc(self->property_hash);
  free_assoc(self->const_constr_hash);
}


/**Function********************************************************************

Synopsis           [Copies self's data into other, so that other contains
the same information]

Description        []

SideEffects        []

******************************************************************************/
static void
flat_hierarchy_copy(const FlatHierarchy_ptr self, FlatHierarchy_ptr other)
{
  other->st                   = self->st;
  other->vars_constr_built    = self->vars_constr_built;
  other->init_expr            = self->init_expr;
  other->invar_expr           = self->invar_expr;
  other->trans_expr           = self->trans_expr;
  other->input_expr           = self->input_expr;
  other->assign_expr          = self->assign_expr;
  other->justice_expr         = self->justice_expr;
  other->compassion_expr      = self->compassion_expr;
  other->spec_expr            = self->spec_expr;
  other->compute_expr         = self->compute_expr;
  other->ltlspec_expr         = self->ltlspec_expr;
  other->pslspec_expr         = self->pslspec_expr;
  other->invarspec_expr       = self->invarspec_expr;
  other->var_set              = Set_Copy(self->var_set);
  other->pred_list            = self->pred_list;
  other->mirror_list          = self->mirror_list;

  /* Make a valid copy of the assignments hash. Note that all key
     associated values are CONS nodes, and need to be copied in order
     to not share any node instance. Sharing nodes means that
     modifying the "self" flat hierarchy instance, will have
     side-effects on the "other" flat hierarchy instance */
  clear_assoc(other->assign_hash);
  {
    node_ptr old_cons;
    node_ptr tmp;
    assoc_iter iter;

    ASSOC_FOREACH(self->assign_hash, iter, &tmp, &old_cons) {
      node_ptr new_cons = cons(car(old_cons), cdr(old_cons));
      insert_assoc(other->assign_hash, tmp, new_cons);
    }
  }

  free_assoc(other->property_hash);
  other->property_hash = copy_assoc(self->property_hash);

  free_assoc(other->const_constr_hash);
  other->const_constr_hash = copy_assoc(self->const_constr_hash);
}


/**Function********************************************************************

Synopsis           [Performs the actual merging.]

Description        []

SideEffects        []

******************************************************************************/
static void
flat_hierarchy_mergeinto(FlatHierarchy_ptr self,
                         const FlatHierarchy_ptr other)
{
  nusmv_assert(self->st == other->st); /* same symbol table */

  /* exprs: conjunct or concatenate (components of) the two */
  self->init_expr       = NODE_PTR(Expr_and_nil(EXPR(self->init_expr),
                                                EXPR(other->init_expr)));
  self->invar_expr      = NODE_PTR(Expr_and_nil(EXPR(self->invar_expr),
                                                EXPR(other->invar_expr)));
  self->trans_expr      = NODE_PTR(Expr_and_nil(EXPR(self->trans_expr),
                                                EXPR(other->trans_expr)));
  self->input_expr      = NODE_PTR(Expr_and_nil(EXPR(self->input_expr),
                                                EXPR(other->input_expr)));

/* in assignments first search if each module in other exists in
 * self. In this case adds that module assiments to self. Otherwise
 * append simply the module assigments to self */
  {
    /* this hash is used to reduce complexy from N^2 to linear in
       size of other+self */
    hash_ptr other_assgns = new_assoc();
    node_ptr iter, name, assgn;
    assoc_iter aiter;

    /* 1. Here assignments in other are collected */
    for (iter=other->assign_expr; iter != Nil; iter=cdr(iter)) {
      node_ptr assgn, inst_assgn;
      nusmv_assert(node_get_type(iter) == CONS);

      assgn = car(iter);
      nusmv_assert(node_get_type(assgn) == CONS);

      /* inst name in in car, inst assign in cdr */
      inst_assgn = cdr(assgn);
      if (inst_assgn != Nil && !Expr_is_true(inst_assgn)) {
        /* stores the assignement to be used later when traversing self */
        node_ptr inst_name = car(assgn);
        node_ptr old_assgn = Expr_and_nil(find_assoc(other_assgns, inst_name),
                                          inst_assgn);
        insert_assoc(other_assgns, inst_name, old_assgn);
      }
    }

    /* 2. How traverses assignments in self and conjunct those
     * found in other */
    for (iter=self->assign_expr; iter != Nil; iter=cdr(iter)) {
      node_ptr assgn, inst_name, other_assgn;
      nusmv_assert(node_get_type(iter) == CONS);

      assgn = car(iter);
      nusmv_assert(node_get_type(assgn) == CONS);

      /* inst name in in car, inst assign in cdr */
      inst_name = car(assgn);

      /* is this in other as well? */
      other_assgn = find_assoc(other_assgns, inst_name);
      if (other_assgn != Nil) {
        node_ptr inst_assgn = cdr(assgn);
        inst_assgn = Expr_and_nil(inst_assgn, other_assgn);
        setcdr(assgn, inst_assgn);

        /* reset assign in hash so at the end all remaining non-nil
         * values will be appended at the end, as they are those
         * assignments in other that do not occur in self */
        insert_assoc(other_assgns, inst_name, Nil);
      }
    }

    /* 3. Finally adds remaining (new) assigments. true here is
          essential to avoid previously already handled
          assignments */
    ASSOC_FOREACH(other_assgns, aiter, &name, &assgn) {
      self->assign_expr = cons(cons(name, assgn), self->assign_expr);
    }

    free_assoc(other_assgns);
  } /* end of assignments merging */

  /* Constant constrains merging */
  {
    node_ptr tmp = FlatHierarchy_lookup_constant_constrains(other, INIT);
    FlatHierarchy_add_constant_constrains(self, tmp, INIT);

    tmp = FlatHierarchy_lookup_constant_constrains(other, INVAR);
    FlatHierarchy_add_constant_constrains(self, tmp, INVAR);

    tmp = FlatHierarchy_lookup_constant_constrains(other, TRANS);
    FlatHierarchy_add_constant_constrains(self, tmp, TRANS);
  }

#if 0 /* RC: substituted code */
  self->assign_expr     =
    find_node(CONS,
              find_node(CONS,
                        NODE_PTR(Expr_and_nil(EXPR(car(car(self->assign_expr))),
                                              EXPR(car(car(other->assign_expr))))),
                        NODE_PTR(Expr_and_nil(EXPR(cdr(car(self->assign_expr))),
                                              EXPR(cdr(car(other->assign_expr)))))),
              NODE_PTR(Expr_and_nil(EXPR(cdr(self->assign_expr)),
                                    EXPR(cdr(other->assign_expr)))));
#endif

  self->justice_expr    = append_ns(self->justice_expr,
                                    other->justice_expr);
  self->compassion_expr = append_ns(self->compassion_expr,
                                    other->compassion_expr);
  self->spec_expr       = append_ns(self->spec_expr,
                                    other->spec_expr);
  self->ltlspec_expr    = append_ns(self->ltlspec_expr,
                                    other->ltlspec_expr);
  self->invarspec_expr  = append_ns(self->invarspec_expr,
                                    other->invarspec_expr);
  self->pslspec_expr    = append_ns(self->pslspec_expr,
                                    other->pslspec_expr);
  self->compute_expr    = append_ns(self->compute_expr,
                                    other->compute_expr);


  /* pred_list: append other to self */
  self->pred_list = append_ns(self->pred_list,
                              other->pred_list);

  /*
     assign_hash: merge other in; deal with assign_hash before
     var_set to still have both var_set available.  Additional
     checking, can be disabled for performance; follows [AT]s comment
     above.
  */
  {
    Set_t intersect = Set_Copy(self->var_set);
    intersect = Set_Intersection(intersect, other->var_set);

    Set_Iterator_t iter;
    SET_FOREACH(intersect, iter) {
      node_ptr var = Set_GetMember(intersect, iter);
      node_ptr init_var = find_node(SMALLINIT, var, Nil);
      node_ptr next_var = find_node(NEXT, var, Nil);
      boolean self_has_init, self_has_next, self_has_normal;
      boolean other_has_init, other_has_next, other_has_normal;

      self_has_init = (Nil != FlatHierarchy_lookup_assign(self, init_var));
      self_has_next = (Nil != FlatHierarchy_lookup_assign(self, next_var));
      self_has_normal = (Nil != FlatHierarchy_lookup_assign(self, var));

      other_has_init = (Nil != FlatHierarchy_lookup_assign(other, init_var));
      other_has_next = (Nil != FlatHierarchy_lookup_assign(other, next_var));
      other_has_normal = (Nil != FlatHierarchy_lookup_assign(other, var));

      /* No duplicate init assignments */
      nusmv_assert(!(self_has_init && other_has_init));
      /* No duplicate next assignments */
      nusmv_assert(!(self_has_next && other_has_next));

      /* No assignments at all if normal assignment
         other_has_normal -> !self_has_init & !self_has_next
         self_has_normal -> !other_has_init & !other_has_next
       */
      nusmv_assert(!(self_has_normal && other_has_normal));
      nusmv_assert((!other_has_normal) |
                   ((!self_has_init) && (!self_has_next)));
      nusmv_assert((!self_has_normal) |
                   ((!other_has_init) && (!other_has_next)));

    }

    Set_ReleaseSet(intersect);
  }

  /* appends the var list of other to self */
  self->var_set = Set_Union(self->var_set, other->var_set);

  {
    Set_t vars = FlatHierarchy_get_vars(other);
    Set_Iterator_t vars_iter;

  /* merge assigns and constrains
   * [VS] note that no simplification occurs. MR said that should be ok */
    SET_FOREACH(vars, vars_iter) {
      node_ptr var, ivar, nvar, ass, con;

      /* var */
      var = Set_GetMember(vars, vars_iter);
      ass = FlatHierarchy_lookup_assign(other, var);
      if (Nil != ass) {
        FlatHierarchy_insert_assign(self, var, ass);
      }
      con = FlatHierarchy_lookup_constrains(other, var);
      if (Nil != con) {
        FlatHierarchy_add_constrains(self, var, con);
      }

      /* init(var) */
      ivar = find_node(SMALLINIT, var, Nil);
      ass = FlatHierarchy_lookup_assign(other, ivar);
      if (Nil != ass) {
        FlatHierarchy_insert_assign(self, ivar, ass);
      }
      con = FlatHierarchy_lookup_constrains(other, ivar);
      if (Nil != con) {
        FlatHierarchy_add_constrains(self, ivar, con);
      }

      /* next(var) */
      nvar = find_node(NEXT, var, Nil);
      ass = FlatHierarchy_lookup_assign(other, nvar);
      if (Nil != ass) {
        FlatHierarchy_insert_assign(self, nvar, ass);
      }
      con = FlatHierarchy_lookup_constrains(other, nvar);
      if (Nil != con) {
        FlatHierarchy_add_constrains(self, nvar, con);
      }
    } /* loop over other's vars */
  }

  {
    assoc_iter aiter;
    node_ptr name;
    ASSOC_FOREACH(other->property_hash, aiter, &name, NULL) {
      nusmv_assert(NODE_FROM_INT(1) != find_assoc(self->property_hash, name));
      insert_assoc(self->property_hash, name, NODE_FROM_INT(1));
    }
  }
}


/**Function********************************************************************

Synopsis           []

Description        [see compileFlattenSexpModel]

SideEffects        []

SeeAlso            []

******************************************************************************/
static void
flat_hierarchy_calc_var_const_aux(FlatHierarchy_ptr self,
                                  node_ptr expr, int type)
{
  int saved_yylineno = yylineno;
  if (expr == Nil) return;
  yylineno = node_get_lineno(expr);
  flat_hierarchy_calc_var_const_recur(self, expr, type);
  yylineno = saved_yylineno;
}


/**Function********************************************************************

Synopsis           []

Description        [see compileFlattenSexpModel]

SideEffects        []

SeeAlso            []

******************************************************************************/
static void
flat_hierarchy_calc_var_const_recur(FlatHierarchy_ptr self,
                                    node_ptr expr, int type)
{
  if (expr == Nil) return;

  if (node_get_type(expr) == AND) {
    flat_hierarchy_calc_var_const_aux(self, car(expr), type);
    flat_hierarchy_calc_var_const_aux(self, cdr(expr), type);
  }
  else {
    Set_t deps;
    Set_Iterator_t iter;
    deps = Formula_GetDependencies(self->st, expr, Nil);

    /* If there are no variables in the dependencies set, and the
       expression is not simply "TRUE", add it to the set of constant
       constrains. */
    if (Set_IsEmpty(deps) &&
        flat_hierarchy_check_const_deps(self, expr, type)) {
      FlatHierarchy_add_constant_constrains(self, expr, type);
    }

    SET_FOREACH(deps, iter) {
      node_ptr var = (node_ptr) Set_GetMember(deps, iter);
      node_ptr index;

      switch (type) {
      case INIT: index = find_node(SMALLINIT, var, Nil); break;
      case INVAR: index = var; break;
      case TRANS: index = find_node(NEXT, var, Nil); break;
      default:
        fprintf(nusmv_stderr,
                "flat_hierarchy_calc_var_const_recur: Unknown expr type\n");
        error_reset_and_exit(1);
        break;
      }
      FlatHierarchy_add_constrains(self, index, expr);
    } /* for loop*/

    Set_ReleaseSet(deps);
  }
}



/**Function********************************************************************

Synopsis           [Called when a constant has been found in INVAR, INIT or
TRANS]

Description         [If the constant is trivially true, false is returned.
                     In all other cases, this function returns true
                     NB: This function has been keept ONLY for the
                     verbosity it produces]

SideEffects        []

SeeAlso            []

******************************************************************************/
static boolean flat_hierarchy_check_const_deps(FlatHierarchy_ptr self,
                                               node_ptr expr, int type)
{
  boolean keep = false;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
    fprintf(nusmv_stderr, "Constant expression found in a %s statement in",
            constr_type_to_string(type));
    start_parsing_err();

#if NUSMV_HAVE_CEGAR
    fprintf(nusmv_stderr, " (Omitted)");

#else
    fprintf(nusmv_stderr, " The expression is \"");
    print_node(nusmv_stderr, expr);
    fprintf(nusmv_stderr, "\"");
#endif
  }

  if (Expr_is_true(expr)) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, " (Skipped)\n");
    }
    keep = false;
  }
  else {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "\n");
    }
    keep = true;
  }

  return keep;
}



/**Function********************************************************************

   Synopsis           [Internal service to self-check that a given
                       expression's language is contained within
                       the set of variables which this FH was
                       declared to have at construction time ]

   Description        [optional]

   SideEffects        []

   SeeAlso            [optional]

******************************************************************************/
static void
flat_hiearchy_self_check_expr(const FlatHierarchy_ptr self, node_ptr expr)
{
  Set_t deps = Formula_GetDependencies(self->st, self->init_expr, Nil);
  if (! Set_Contains(self->var_set, deps)) {
    internal_error("FlatHierachy failed self-check.");
  }
  Set_ReleaseSet(deps);
}


/**Function********************************************************************

Synopsis           [required]

Description        [optional]

SideEffects        []

SeeAlso            [optional]

******************************************************************************/
static const char* constr_type_to_string(int type)
{
  switch (type) {
  case INIT: return "INIT";
  case INVAR: return "INVAR";
  case TRANS: return "TRANS";
  default:
    error_unreachable_code();
  }
}

typedef struct dag_TAG {
  node_ptr var;
  Set_t inbounds;
  Set_t outbounds;
} dag;

static void flat_hierarchy_visit_dag(hash_ptr outbounds, hash_ptr visited,
                                     node_ptr var, NodeList_ptr result)
{
  /* Not not yet visited */
  if (Nil == find_assoc(visited, var)) {
    Set_Iterator_t iter;
    Set_t outb = (Set_t)find_assoc(outbounds, var);

    /* Set the node to visited */
    insert_assoc(visited, var, NODE_FROM_INT(1));
    SET_FOREACH(outb, iter) {
      node_ptr out = (node_ptr)Set_GetMember(outb, iter);
      flat_hierarchy_visit_dag(outbounds, visited, out, result);
    }
    NodeList_prepend(result, var);
  }
}
